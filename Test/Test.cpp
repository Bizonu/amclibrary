////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        Test.cpp
///  Description: Command line testing application for AMC Library.
///  Author:      Chiuta Adrian Marius
///  Created:     10-11-2009
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
#include <time.h>
#include "AMCLibrary.h"
#include "IAlgorithmAES.h"
#include "AES_CPU.h"
#include "AES_GPU_DX10.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define BENCHMARK_START                             \
{                                                   \
    LARGE_INTEGER liFrequency;                      \
    LARGE_INTEGER liStartTime;                      \
    LARGE_INTEGER liCurrentTime;                    \
    QueryPerformanceFrequency ( &liFrequency );     \
    QueryPerformanceCounter ( &liStartTime );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define BENCHMARK_END(timeMs)                       \
    QueryPerformanceCounter ( &liCurrentTime );     \
    (timeMs) = ((double)( (liCurrentTime.QuadPart - liStartTime.QuadPart)* (double)1000.0/(double)liFrequency.QuadPart ));\
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK
ACMEnumeration(IAlgorithmDescription &moduleDescription)
{
    _tprintf(_T("Found new plugin:\n\t* Class Name: %s \n\t* Description: %s\n\n"), moduleDescription.ClassName(), moduleDescription.ClassDescription());

    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
Benchmark()
{
#define BENCH_DATA_SIZE 4096 * 4096 * 16
//#define BENCH_DATA_SIZE 1024 * 1024 * 16

    _tprintf(_T("Starting benchmark\n"));
    UINT8 *src = (UINT8*)malloc(BENCH_DATA_SIZE);
    UINT8 *tmp = (UINT8*)malloc(BENCH_DATA_SIZE);
    UINT8 *dst = (UINT8*)malloc(BENCH_DATA_SIZE);

    if( dst == NULL || src == NULL || tmp == NULL )
    {
        _tprintf(_T("Could not allocate %d MBytes of memory.\n"), 3 * BENCH_DATA_SIZE / (1024*1024));
        free(src);
        free(dst);
        return;
    }

    // Touch the memory, to be sure the whole memory is allocated and no paging will occur when we measure the time
    for( int i = 0; i < BENCH_DATA_SIZE / 4; i++ )
    {
        ((UINT32*)src)[i] = 0x2BADF00D;
        ((UINT32*)tmp)[i] = 0x2BADF00D;
        ((UINT32*)dst)[i] = 0x2BADF00D;
    }

    _tprintf(_T("Generating %d MBytes of random data...\n"), BENCH_DATA_SIZE / (1024*1024));
    srand( 0xAE5 );
    for( int i = 0; i < BENCH_DATA_SIZE / 2; i++ )
        ((UINT16*)src)[i] = rand();

    // Generate one key
    UINT8 keyData[16];
    srand( 0x1234 );
    for( UINT32 i = 0; i < 16; i++ )
        keyData[i] = rand();

    // Generate the IV
    UINT8 IV[16];
    srand( 0x4321 );
    for( UINT32 i = 0; i < 16; i++ )
        IV[i] = rand();

    // The CPU algorithm is used as fail-safe in case the GPU does not support a mode of operation
    IAlgorithmAES *aesAlgCPU = NULL;
    {
        IAlgorithmDescription *algDesc = AMCGetAlgorithmDescription(AES_ALGORITHM_CLASS_ID, AES_CPU_ALGORITHM_CLASS_ID);
        if( algDesc != NULL )
            aesAlgCPU = dynamic_cast<IAlgorithmAES*>(algDesc->Create());
    }

    static const CLASS_ID classID[] = 
    {
        AES_CPU_ALGORITHM_CLASS_ID,
        AES_GPU_DX10_ALGORITHM_CLASS_ID
    };

    for( int i = 0; i < sizeof(classID) / sizeof(classID[0]) ; i++ )
    {
        IAlgorithmDescription *algDesc = AMCGetAlgorithmDescription(AES_ALGORITHM_CLASS_ID, classID[i]);
        if( algDesc != NULL )
        {
            _tprintf(_T("\n\nCreating class %s...\n"), algDesc->ClassName());
            IAlgorithmAES *aesAlg = dynamic_cast<IAlgorithmAES*>(algDesc->Create());
            if( aesAlg != NULL )
            {
                static const TCHAR* modesName[] = { _T("ECB"), _T("CBC"), _T("CTR") };
                static const IAlgorithmAES::AES_Modes modes[] =
                {
                    IAlgorithmAES::AES_ECB,
                    IAlgorithmAES::AES_CBC,
                    IAlgorithmAES::AES_CTR
                };

/*              {
                    _tprintf(_T("Warming the caches...\n"));
                    AES_Key key(keyData, AES_Key::AES_Key128);
                    aesAlg->SetKey(&key);
                    aesAlg->Encrypt(tmp, src, BENCH_DATA_SIZE);
                    aesAlg->Decrypt(dst, tmp, BENCH_DATA_SIZE);
                }
*/
                for( int mode = 0; mode < sizeof(modes) / sizeof(modes[0]) ; mode++ )
                {
                    _tprintf(_T("\nStarting AES benchmark in %s mode using class %s.\n\n"), modesName[mode], algDesc->ClassName());
                    aesAlg->SetMode(modes[mode], IV);
                    if( aesAlgCPU )
                        aesAlgCPU->SetMode(modes[mode], IV);

                    static const AES_Key::AES_KeySize keySize[] = 
                    {
                        AES_Key::AES_Key128,
                        AES_Key::AES_Key192,
                        AES_Key::AES_Key256
                    };

                    {
                        _tprintf(_T("Warming the caches...\n"));
                        AES_Key key(keyData, keySize[0]);
                        aesAlg->SetKey(&key);
                        aesAlg->Encrypt(tmp, src, BENCH_DATA_SIZE);
                        aesAlg->Decrypt(dst, tmp, BENCH_DATA_SIZE);
                    }

                    for( int j = 0; j < sizeof(keySize) / sizeof(keySize[0]) ; j++ )
                    {
                        _tprintf(_T("Using key size of %d bits.\n"), keySize[j]);
                        AES_Key key(keyData, keySize[j]);
                        aesAlg->SetKey(&key);
                        if( aesAlgCPU )
                            aesAlgCPU->SetKey(&key);

                        double timeEncrypt = 1.0f, timeDecrypt = 1.0f;

                        if( (aesAlg->GetSupportedModes(true) & modes[mode]) == 0 )
                        {
                            if( aesAlgCPU == NULL )
                            {
                                _tprintf(_T("The mode %s is not supported by %s! Skipping....\n"), 
                                                modesName[mode], aesAlg->GetDescription().ClassName());
                                break;
                            }
                            // The mode is not supported by aesAlg
                            _tprintf(_T("The mode %s is not supported by %s, and will be used %s for encryption.\n"),
                                     modesName[mode], aesAlg->GetDescription().ClassName(), aesAlgCPU->GetDescription().ClassName());
                            BENCHMARK_START
                                aesAlgCPU->Encrypt(tmp, src, BENCH_DATA_SIZE);
                            BENCHMARK_END(timeEncrypt);
                        }
                        else
                        {
                            BENCHMARK_START
                                aesAlg->Encrypt(tmp, src, BENCH_DATA_SIZE);
                            BENCHMARK_END(timeEncrypt);
                        }

                        if( (aesAlg->GetSupportedModes(false) & modes[mode]) == 0 )
                        {
                            if( aesAlgCPU == NULL )
                            {
                                _tprintf(_T("The mode %s is not supported by %s! Skipping....\n"),
                                         modesName[mode], aesAlg->GetDescription().ClassName());
                                break;
                            }

                            // The mode is not supported by aesAlg
                            _tprintf(_T("The mode %s is not supported by %s, and will be used %s for decryption.\n"), 
                                     modesName[mode], aesAlg->GetDescription().ClassName(), aesAlgCPU->GetDescription().ClassName());
                            BENCHMARK_START
                                aesAlgCPU->Decrypt(dst, tmp, BENCH_DATA_SIZE);
                            BENCHMARK_END(timeDecrypt);
                        }
                        else
                        {
                            BENCHMARK_START
                                aesAlg->Decrypt(dst, tmp, BENCH_DATA_SIZE);
                            BENCHMARK_END(timeDecrypt);
                        }

                        _tprintf(_T("Encrypted %u bytes in %8.3f ms. (%8.3f MB/sec)\n"), BENCH_DATA_SIZE, timeEncrypt, ((1000.0f * BENCH_DATA_SIZE) / timeEncrypt) / (1024 * 1024) );
                        _tprintf(_T("Decrypted %u bytes in %8.3f ms. (%8.3f MB/sec)\n"), BENCH_DATA_SIZE, timeDecrypt, ((1000.0f * BENCH_DATA_SIZE) / timeDecrypt) / (1024 * 1024) );

                        _tprintf(_T("Please wait while verfying the results..."));
                        bool isOK = true;
                        for( int k = 0; k < BENCH_DATA_SIZE; k++ )
                        {
                            if( dst[i] != src[i] )
                            {
                                isOK = false;
                                break;
                            }
                        }

                        if( isOK )
                            _tprintf(_T("Results are OK.\n"));
                        else
                            _tprintf(_T("Results are wrong !!!\n"));
                    }
                }
                delete aesAlg;
            }
            else
                _tprintf(_T("Could not create the %s class.\n"), algDesc->ClassName());
        }
        else
            _tprintf(_T("Could not find the class with ID 0x%X.\n"), classID[i]);
    }

    delete aesAlgCPU;
    free(dst);
    free(src);
    free(tmp);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
PrintUsage()
{
    _tprintf(_T("Usage scenarios:\n\n")
             _T("1) -benchmark      - Will test all the components for speed.\n\n")
             _T("2) -enumerate      - Will enumerate all the components.\n\n")
             _T("3) -e/d ECB/CBC/CTR CPU/GPU keysize key infile outfile - Will encrypt/decrypt")
             _T("                     the contents of the input file using the key")
             _T("                     (max 16 characters), key size(128, 192, 256).")
             _T("                     Padding is done with zero.\n")
            );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
_tmain(int argc, _TCHAR* argv[])
{
    if( argc < 2 )
        PrintUsage();
    else
    {
        // Make all the parameters in lower case
        for( int i = 1; i < argc; i++ )
        {
            for( int j = 0; j < (int)_tcslen(argv[i]); j++ )
                if( _istascii(argv[i][j]) && _istupper(argv[i][j]) )
                    argv[i][j] = _totlower(argv[i][j]);
        }

        if( _tcscmp(argv[1], _T("-benchmark")) == 0 )
            Benchmark();
        else if( _tcscmp(argv[1], _T("-enumerate")) == 0 )
        {
            // Test the enumeration of plugins
            _tprintf(_T("Searching for plugins...\n\n"));
            AMCEnumerateAlgorithms(ACMEnumeration);
        }
        else
        {
            if( argc != 8 )
            {
                PrintUsage();
                return 0;
            }

            int EncDec = 0; // 1 encryption; -1 decryption
            if( _tcscmp(argv[1], _T("-e")) == 0 )
                EncDec = 1;
            else if( _tcscmp(argv[1], _T("-d")) == 0 )
                EncDec = -1;

            if( EncDec == 0 )
            {
                PrintUsage();
                return 0;
            }

            int Mode = 0; // 1 - ECB; 2 - CBC; 3 - CTR;
            if( _tcscmp(argv[2], _T("ecb")) == 0 )
                Mode = 1;
            else if( _tcscmp(argv[2], _T("cbc")) == 0 )
                Mode = 2;
            else if( _tcscmp(argv[2], _T("ctr")) == 0 )
                Mode = 3;

            if( Mode == 0 )
            {
                PrintUsage();
                return 0;
            }

            int CpuGPU = 0; // 1 CPU; -1 GPU
            if( _tcscmp(argv[3], _T("cpu")) == 0 )
                CpuGPU = 1;
            else if( _tcscmp(argv[3], _T("gpu")) == 0 )
                CpuGPU = -1;

            if( CpuGPU == 0 )
            {
                PrintUsage();
                return 0;
            }

            int keySize = _tstoi(argv[4]);
            if( keySize != 128 && keySize != 192 && keySize != 256 )
            {
                PrintUsage();
                return 0;
            }

            UINT8 keyData[16] = {0};
            INT32 nbKeyChars = (INT32)_tcslen(argv[5]);
            nbKeyChars = min(nbKeyChars, 16);
            for( int i = 0; i < nbKeyChars; i++ )
                keyData[i] = (UINT8)argv[5][i];

            FILE *fin, *fout;
            if( _tfopen_s(&fin, argv[6], _T("rb")) )
            {
                _tprintf(_T("Could not open file \"%s\".\n"), argv[6]);
                return 0;
            }

            if( _tfopen_s(&fout, argv[7], _T("wb")) )
            {
                _tprintf(_T("Could not open file \"%s\".\n"), argv[7]);
                fclose(fin);
                return 0;
            }

            // Create the key
            AES_Key key(keyData, (AES_Key::AES_KeySize)keySize);

            IAlgorithmDescription *algDesc;
            if( CpuGPU > 0)
                algDesc = AMCGetAlgorithmDescription(AES_ALGORITHM_CLASS_ID, AES_CPU_ALGORITHM_CLASS_ID);
            else
                algDesc = AMCGetAlgorithmDescription(AES_ALGORITHM_CLASS_ID, AES_GPU_DX10_ALGORITHM_CLASS_ID);
            if( algDesc == NULL )
            {
                _tprintf(_T("Could not find the specified class.\n"));
                return 1;
            }
            IAlgorithmAES *aesAlg = dynamic_cast<IAlgorithmAES*>(algDesc->Create());
            if( aesAlg == NULL )
            {
                _tprintf(_T("Could not create the %s class.\n"), algDesc->ClassName());
                return 1;
            }

            UINT32 supportedModes = aesAlg->GetSupportedModes(EncDec > 0);
            static const IAlgorithmAES::AES_Modes modes[] =
            {
                IAlgorithmAES::AES_ECB,
                IAlgorithmAES::AES_CBC,
                IAlgorithmAES::AES_CTR
            };
            UINT32 aesMode = modes[Mode - 1];

            if( (supportedModes & aesMode) == 0 )
            {
                _tprintf(_T("The specified block chiper mode is not supported by the class %s\n"), algDesc->ClassName());
                return 1;
            }

            aesAlg->SetMode(IAlgorithmAES::AES_ECB);
            aesAlg->SetKey(&key);

        #define BUFF_SIZE 1024 * 1024 * 16
            INT64 fileSize, dataSize;
            _fseeki64(fin, 0, SEEK_END);
            fileSize = _ftelli64(fin);
            _fseeki64(fin, 0, SEEK_SET);

            // Pad with zero the input data
            dataSize = (fileSize + 15) & (~15);
            UINT8 *buff = (UINT8*)malloc((UINT32)min(dataSize, BUFF_SIZE));

            while( dataSize > 0)
            {
                UINT32 blockSize = (UINT32)min(dataSize, BUFF_SIZE);
                // Hack for fast zero padding
                memset(buff + blockSize - 16, 0, 16);
                fread(buff, 1, blockSize, fin);
                if( EncDec > 0 )
                    aesAlg->Encrypt(buff, buff, blockSize);
                else
                    aesAlg->Decrypt(buff, buff, blockSize);
                fwrite(buff, 1, blockSize, fout);
                dataSize -= blockSize;
            }

            fclose(fin);
            fclose(fout);
            free(buff);
            delete aesAlg;
        }
    }
    return 0;
}
