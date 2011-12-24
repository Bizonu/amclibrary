////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AES_CPU.cpp
///  Description: The implementation of the internal interface for the AES implementation on the CPU.
///  Author:      Chiuta Adrian Marius
///  Created:     08-11-2009
///
///  Licensed under the Apache License, Version 2.0 (the "License");
///  you may not use this file except in compliance with the License.
///  You may obtain a copy of the License at
///  http://www.apache.org/licenses/LICENSE-2.0
///  Unless required by applicable law or agreed to in writing, software
///  distributed under the License is distributed on an "AS IS" BASIS,
///  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
///  See the License for the specific language governing permissions and
///  limitations under the License.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "platform.h"
#include "AES_CPU_Internal.h"
#include "AES_CPU_Impl_O0.h"
#include "AES_CPU_Impl_O1.h"
#include "AES_CPU_Impl_O2.h"
#include "AES_CPU_Impl_O3.h"
#include "omp.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AES_CPU_Description : public IAlgorithmDescription
{
public:
    AES_CPU_Description()
    {
        // Here can be made some checks to see if the current AES implementation can work.
        m_IsOK = true;
    }
    bool IsOK() { return m_IsOK; };

    const TCHAR* ClassName() { return _T("AES_CPU"); };
    const TCHAR* ClassDescription() { return _T("Implements the AES encryption/decryption algorithm using the CPU resources."); };
    CLASS_ID ClassID() { return AES_CPU_ALGORITHM_CLASS_ID; }
    CLASS_ID InterfaceID() { return AES_ALGORITHM_CLASS_ID; }
    IAlgorithm* Create()
    {
        return new AES_CPU_Internal();
    }
private:
    bool m_IsOK;
};

static AES_CPU_Description class_description;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AES_CPU_API IAlgorithmDescription*
GetDescription(UINT32 acmVersion)
{
    if( class_description.IsOK() && acmVersion == AMCLIB_VERSION )
        return &class_description;
    else
        return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AES_CPU_Internal::AES_CPU_Internal()
{
    m_SupportedModes = IAlgorithmAES::AES_ECB |
                       IAlgorithmAES::AES_CBC |
                       IAlgorithmAES::AES_CTR;

    m_Mode = IAlgorithmAES::AES_ECB;

    m_keySize = AES_Key::AES_KeyInvalid;

    memset(m_IV, 0, IAlgorithmAES::BlockSizeBytes);
    memset(m_internalEncIV, 0, IAlgorithmAES::BlockSizeBytes);
    memset(m_internalDecIV, 0, IAlgorithmAES::BlockSizeBytes);

    // Select the fastest implementation for AES
    m_AES_Impl = new AES_CPU_Impl_O3();

    // Get the number of processors on this machine
    TCHAR sNbProc[5] = _T("1");
    GetEnvironmentVariable(_T("NUMBER_OF_PROCESSORS"), sNbProc, 5);
    m_maxNbThreads = _tstoi(sNbProc);
    if( m_maxNbThreads <= 0 )
        m_maxNbThreads = 1;

    m_nbMinBlocksForParallel = 10240 / IAlgorithmAES::BlockSizeBytes;
    m_nbThreads = m_maxNbThreads;
    omp_set_num_threads(m_nbThreads);

    HKEY key;
    if( ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), &key) )
    {
        DWORD dataSize;
        RegQueryValueEx(key, _T("ProcessorNameString"), NULL, NULL, NULL, &dataSize);
        mDeviceName = (TCHAR*)new UINT8[dataSize];
        RegQueryValueEx(key, _T("ProcessorNameString"), NULL, NULL, (LPBYTE)mDeviceName, &dataSize);
        RegCloseKey(key);
    }
    else
    {
        static TCHAR defaultDeviceName[] = _T("Unknown CPU");
        mDeviceName = (TCHAR*)new UINT8[sizeof(defaultDeviceName)];
        memcpy(mDeviceName, defaultDeviceName, sizeof(defaultDeviceName));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AES_CPU_Internal::~AES_CPU_Internal()
{
    delete m_AES_Impl;
    delete mDeviceName;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IAlgorithmDescription&
AES_CPU_Internal::GetDescription()
{
    return class_description;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const TCHAR*
AES_CPU_Internal::GetDeviceName()
{
    return mDeviceName;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32
AES_CPU_Internal::GetSupportedModes(bool Encryption)
{
    return m_SupportedModes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IAlgorithmAES::AES_Status 
AES_CPU_Internal::SetMode(AES_Modes mode, const UINT8 *IV)
{
    if( (mode & m_SupportedModes) == 0 )
        return IAlgorithmAES::AES_Status_NotSupported;

    /// Check if only one mode was passed ( must be a power of 2 )
    if( (mode & (mode - 1))  != 0 )
        return IAlgorithmAES::AES_Status_InvalidArgs;
    m_Mode = mode;

    if( mode != IAlgorithmAES::AES_ECB )
    {   
        /// All the modes beside ECB needs IV
        if( IV == NULL )
        {
            memset(m_IV, 0, IAlgorithmAES::BlockSizeBytes);
            memset(m_internalEncIV, 0, IAlgorithmAES::BlockSizeBytes);
            memset(m_internalDecIV, 0, IAlgorithmAES::BlockSizeBytes);
        }
        else
        {
            memcpy(m_IV, IV, IAlgorithmAES::BlockSizeBytes);
            memcpy(m_internalEncIV, IV, IAlgorithmAES::BlockSizeBytes);
            memcpy(m_internalDecIV, IV, IAlgorithmAES::BlockSizeBytes);
        }
    }

    return IAlgorithmAES::AES_Status_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IAlgorithmAES::AES_Status
AES_CPU_Internal::SetKey(const AES_Key *key)
{
    if( key == NULL || key->GetKeySize() == AES_Key::AES_KeyInvalid )
        return IAlgorithmAES::AES_Status_InvalidArgs;

    m_keySize = key->GetKeySize();
    m_keyDataSize = key->GetKeySizeBytes();

    memcpy(m_key, key->GetKeyData(), m_keyDataSize);

    m_AES_Impl->SetKeyData(m_key, m_keyDataSize, m_keySize);

    // Also restore the IV that we use
    if( m_Mode != IAlgorithmAES::AES_ECB )
    {
        memcpy(m_internalEncIV, m_IV, IAlgorithmAES::BlockSizeBytes);
        memcpy(m_internalDecIV, m_IV, IAlgorithmAES::BlockSizeBytes);
    }

    return IAlgorithmAES::AES_Status_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AES_CPU_Internal::SetOptimization(AES_CPU_Optimization opt)
{
    delete m_AES_Impl;

    switch(opt)
    {
    case AES_CPU_O0:
        m_AES_Impl = new AES_CPU_Impl_O0();
        break;

    case AES_CPU_O1:
        m_AES_Impl = new AES_CPU_Impl_O1();
        break;

    case AES_CPU_O2:
        m_AES_Impl = new AES_CPU_Impl_O2();
        break;

    case AES_CPU_O3:
        m_AES_Impl = new AES_CPU_Impl_O3();
        break;
    }

    if( m_keySize != AES_Key::AES_KeyInvalid )
        m_AES_Impl->SetKeyData(m_key, m_keyDataSize, m_keySize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32
AES_CPU_Internal::SetNbThreads(UINT32 nbThreads, UINT32 minSizeForParallel)
{
    m_nbMinBlocksForParallel = minSizeForParallel / IAlgorithmAES::BlockSizeBytes;

    if( nbThreads == 0 )
        nbThreads = 1;

    if( nbThreads > m_maxNbThreads )
        nbThreads = m_maxNbThreads;

    if( nbThreads == m_nbThreads )
        return nbThreads;

    m_nbThreads = nbThreads;

    return m_nbThreads;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32
AES_CPU_Internal::SetNbThreads(UINT32 nbThreads)
{
    return SetNbThreads(nbThreads, m_nbMinBlocksForParallel * IAlgorithmAES::BlockSizeBytes);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IAlgorithmAES::AES_Status
AES_CPU_Internal::Encrypt(UINT8 *dst, const UINT8 *src, UINT32 size)
{
    // Check for NULL pointers, and if the size of the data is multiple of block size
    // ( padding of the data must be done by the caller )
    if( dst == NULL || src == NULL || size % IAlgorithmAES::BlockSizeBytes != 0 )
        return IAlgorithmAES::AES_Status_InvalidArgs;

    if( m_keySize == AES_Key::AES_KeyInvalid )
        return IAlgorithmAES::AES_Status_InvalidState;

    UINT32 nbBlocks = size / IAlgorithmAES::BlockSizeBytes;
    if( nbBlocks != 0 )
    {
        switch(m_Mode)
        {
            case IAlgorithmAES::AES_ECB:
                {
                    if( m_nbThreads > 1 && nbBlocks >= m_nbMinBlocksForParallel )
                    {
                        #pragma omp parallel for num_threads(m_nbThreads)
                        for( INT32 i = 0; i < (signed)nbBlocks; i++ )
                        {
                            m_AES_Impl->EncryptBlock(dst + i * IAlgorithmAES::BlockSizeBytes,
                                                     src + i * IAlgorithmAES::BlockSizeBytes);
                        }
                    }
                    else
                    {
                        for( INT32 i = 0; i < (signed)nbBlocks; i++ )
                        {
                            m_AES_Impl->EncryptBlock(dst + i * IAlgorithmAES::BlockSizeBytes,
                                                     src + i * IAlgorithmAES::BlockSizeBytes);
                        }
                    }
                }break;

            case IAlgorithmAES::AES_CBC:
                {
                    UINT64 *src64 = (UINT64*)src,
                           *dst64 = (UINT64*)dst;

                    dst64[0] = src64[0] ^ ((UINT64*)m_internalEncIV)[0];
                    dst64[1] = src64[1] ^ ((UINT64*)m_internalEncIV)[1];
                    m_AES_Impl->EncryptBlock((UINT8*)dst64, (UINT8*)dst64);
                    src64 += 2;
                    dst64 += 2;

                    for( INT32 i = 1; i < (signed)nbBlocks; i++ )
                    {
                        dst64[0] = src64[0] ^ dst64[-2];
                        dst64[1] = src64[1] ^ dst64[-1];

                        m_AES_Impl->EncryptBlock((UINT8*)dst64, (UINT8*)dst64);
                        src64 += 2;
                        dst64 += 2;
                    }
                    ((UINT64*)m_internalEncIV)[0] = dst64[-2];
                    ((UINT64*)m_internalEncIV)[1] = dst64[-1];
                }break;

            case IAlgorithmAES::AES_CTR:
                {
                    if( m_nbThreads > 1 && nbBlocks >= m_nbMinBlocksForParallel )
                    {
                        #pragma omp parallel for num_threads(m_nbThreads)
                        for(INT32 i = 0; i < (signed)nbBlocks; i++)
                        {
                            UINT64  IV64[2], encoded[2],
                                    *src64 = ((UINT64*)src) + i * 2,
                                    *dst64 = ((UINT64*)dst) + i * 2;

                            IV64[0] = ((UINT64*)m_internalEncIV)[0] + i;
                            IV64[1] = ((UINT64*)m_internalEncIV)[1];
                            if( IV64[0] < ((UINT64*)m_internalEncIV)[0] )
                                IV64[1]++;

                            m_AES_Impl->EncryptBlock((UINT8*)encoded, (UINT8*)IV64);

                            dst64[0] = encoded[0] ^ src64[0];
                            dst64[1] = encoded[1] ^ src64[1];
                        }
                    }
                    else
                    {
                        for( INT32 i = 0; i < (signed)nbBlocks; i++ )
                        {
                            UINT64  IV64[2], encoded[2],
                                    *src64 = ((UINT64*)src) + i * 2,
                                    *dst64 = ((UINT64*)dst) + i * 2;

                            IV64[0] = ((UINT64*)m_internalEncIV)[0] + i;
                            IV64[1] = ((UINT64*)m_internalEncIV)[1];
                            if( IV64[0] < ((UINT64*)m_internalEncIV)[0] )
                                IV64[1]++;

                            m_AES_Impl->EncryptBlock((UINT8*)encoded, (UINT8*)IV64);

                            dst64[0] = encoded[0] ^ src64[0];
                            dst64[1] = encoded[1] ^ src64[1];
                        }
                    }

                    // Save the IV for the next run
                    if( ((UINT64*)m_internalEncIV)[0] + nbBlocks < ((UINT64*)m_internalEncIV)[0] )
                    {
                        ((UINT64*)m_internalEncIV)[0] += nbBlocks;
                        ((UINT64*)m_internalEncIV)[1]++;
                    }
                    else
                        ((UINT64*)m_internalEncIV)[0] += nbBlocks;
                }break;

            default:
                return IAlgorithmAES::AES_Status_NotSupported;
        }
    }

    return IAlgorithmAES::AES_Status_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IAlgorithmAES::AES_Status
AES_CPU_Internal::Decrypt(UINT8 *dst, const UINT8 *src, UINT32 size)
{
    // Check for NULL pointers, and if the size of the data is multiple of block size
    // ( padding of the data must be done by the caller )
    if( dst == NULL || src == NULL || size % IAlgorithmAES::BlockSizeBytes != 0 )
        return IAlgorithmAES::AES_Status_InvalidArgs;

    if( m_keySize == AES_Key::AES_KeyInvalid )
        return IAlgorithmAES::AES_Status_InvalidState;

    UINT32 nbBlocks = size / IAlgorithmAES::BlockSizeBytes;
    if( nbBlocks != 0 )
    {
        switch(m_Mode)
        {
        case IAlgorithmAES::AES_ECB:
            {
                if( m_nbThreads > 1 && nbBlocks >= m_nbMinBlocksForParallel )
                {
                    #pragma omp parallel for num_threads(m_nbThreads)
                    for( INT32 i = 0; i < (signed)nbBlocks; i++ )
                    {
                        m_AES_Impl->DecryptBlock(dst + i * IAlgorithmAES::BlockSizeBytes,
                                                 src + i * IAlgorithmAES::BlockSizeBytes);
                    }
                }
                else
                {
                    for( INT32 i = 0; i < (signed)nbBlocks; i++ )
                    {
                        m_AES_Impl->DecryptBlock(dst + i * IAlgorithmAES::BlockSizeBytes,
                                                 src + i * IAlgorithmAES::BlockSizeBytes);
                    }
                }
            }break;

        case IAlgorithmAES::AES_CBC:
            {
                if( src != dst )
                {
                    UINT64 *tdst64 = (UINT64*)dst;

                    m_AES_Impl->DecryptBlock((UINT8*)tdst64, src);
                    tdst64[0] ^= ((UINT64*)m_internalDecIV)[0];
                    tdst64[1] ^= ((UINT64*)m_internalDecIV)[1];

                    // Save the Initial Vector for the next run
                    UINT64 *tsrc64 = ((UINT64*)src) + (nbBlocks - 1) * 2;
                    ((UINT64*)m_internalDecIV)[0] = tsrc64[0];
                    ((UINT64*)m_internalDecIV)[1] = tsrc64[1];

                    if( m_nbThreads > 1 && nbBlocks >= m_nbMinBlocksForParallel )
                    {
                        #pragma omp parallel for num_threads(m_nbThreads)
                        for( INT32 i = 1; i < (signed)nbBlocks; i++ )
                        {
                            UINT64 *src64 = ((UINT64*)src) + i * 2;
                            UINT64 *dst64 = ((UINT64*)dst) + i * 2;
                            m_AES_Impl->DecryptBlock((UINT8*)dst64, (UINT8*)src64);
                            src64 -= 2;
                            dst64[0] ^= src64[0];
                            dst64[1] ^= src64[1];
                        }
                    }
                    else
                    {
                        for( INT32 i = 1; i < (signed)nbBlocks; i++ )
                        {
                            UINT64 *src64 = ((UINT64*)src) + i * 2;
                            UINT64 *dst64 = ((UINT64*)dst) + i * 2;
                            m_AES_Impl->DecryptBlock((UINT8*)dst64, (UINT8*)src64);
                            src64 -= 2;
                            dst64[0] ^= src64[0];
                            dst64[1] ^= src64[1];
                        }
                    }
                }
                else
                {
                    // We can't process the blocks in parallel because each one writes it's results 
                    // over the input and the input may also be needed by another thread

                    // Save the Initial Vector for the next run
                    UINT64 tIV[2];
                    UINT64 *tsrc64 = ((UINT64*)src) + (nbBlocks - 1) * 2;
                    tIV[0] = tsrc64[0];
                    tIV[1] = tsrc64[1];

                    for( INT32 i = (signed)nbBlocks - 1; i >= 1; i-- )
                    {
                        UINT64 *src64 = ((UINT64*)src) + i * 2;
                        UINT64 *dst64 = ((UINT64*)dst) + i * 2;
                        UINT64 tmpdst[2];
                        m_AES_Impl->DecryptBlock((UINT8*)tmpdst, (UINT8*)src64);
                        src64 -= 2;
                        dst64[0] = tmpdst[0] ^ src64[0];
                        dst64[1] = tmpdst[1] ^ src64[1];
                    }

                    UINT64 *tdst64 = (UINT64*)dst;
                    m_AES_Impl->DecryptBlock(dst, src);
                    tdst64[0] ^= ((UINT64*)m_internalDecIV)[0];
                    tdst64[1] ^= ((UINT64*)m_internalDecIV)[1];

                    ((UINT64*)m_internalDecIV)[0] = tIV[0];
                    ((UINT64*)m_internalDecIV)[1] = tIV[1];
                }
            }break;

        case IAlgorithmAES::AES_CTR:
            {
                if( m_nbThreads > 1 && nbBlocks >= m_nbMinBlocksForParallel )
                {
                    #pragma omp parallel for num_threads(m_nbThreads)
                    for( INT32 i = 0; i < (signed)nbBlocks; i++ )
                    {
                        UINT64  IV64[2], encoded[2],
                                *src64 = ((UINT64*)src) + i * 2,
                                *dst64 = ((UINT64*)dst) + i * 2;

                        IV64[0] = ((UINT64*)m_internalDecIV)[0] + i;
                        IV64[1] = ((UINT64*)m_internalDecIV)[1];
                        if( IV64[0] < ((UINT64*)m_internalDecIV)[0] )
                            IV64[1]++;

                        m_AES_Impl->EncryptBlock((UINT8*)encoded, (UINT8*)IV64);

                        dst64[0] = encoded[0] ^ src64[0];
                        dst64[1] = encoded[1] ^ src64[1];
                    }
                }
                else
                {
                    for( INT32 i = 0; i < (signed)nbBlocks; i++ )
                    {
                        UINT64  IV64[2], encoded[2],
                                *src64 = ((UINT64*)src) + i * 2,
                                *dst64 = ((UINT64*)dst) + i * 2;

                        IV64[0] = ((UINT64*)m_internalDecIV)[0] + i;
                        IV64[1] = ((UINT64*)m_internalDecIV)[1];
                        if( IV64[0] < ((UINT64*)m_internalDecIV)[0] )
                            IV64[1]++;

                        m_AES_Impl->EncryptBlock((UINT8*)encoded, (UINT8*)IV64);

                        dst64[0] = encoded[0] ^ src64[0];
                        dst64[1] = encoded[1] ^ src64[1];
                    }
                }

                // Save the IV for the next run
                if( ((UINT64*)m_internalDecIV)[0] + nbBlocks < ((UINT64*)m_internalDecIV)[0] )
                {
                    ((UINT64*)m_internalDecIV)[0] += nbBlocks;
                    ((UINT64*)m_internalDecIV)[1]++;
                }
                else
                    ((UINT64*)m_internalDecIV)[0] += nbBlocks;
            }break;

        default:
            return IAlgorithmAES::AES_Status_NotSupported;
        }
    }

    return IAlgorithmAES::AES_Status_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL APIENTRY
DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
