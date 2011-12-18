////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AES_CPU_Internal.h
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
#ifndef __INCLUDED_AES_CPU_INTERNAL_H__
#define __INCLUDED_AES_CPU_INTERNAL_H__

#include "AES_CPU.h"

/// The high level implementation of AES algorithm
class AES_CPU_Internal : public AES_CPU
{
public:
    AES_CPU_Internal();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Returns a bitfield that contains all the modes supported by the algorithm.
    ///
    /// @param Encryption
    ///                 If true, then the returned modes are available for encryption, else for decryption.
    /// @return
    ///                 A bitfield that contains all the modes supported by the algorithm.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual UINT32 GetSupportedModes(bool Encryption = true);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Sets the mode of operation for the algorithm, and if needed,
    /// also sets the IV (Initialization Vector) needed by the mode.
    ///
    /// @param mode
    ///                 The mode of operation.
    /// @param IV
    ///                 If @a IV is @a NULL, then all the values from @a IV are considered to be zero.
    ///                 Node that the size of the @a IV must be the same as the IAlgorithmAES::BlockSize.
    /// @return
    ///                 AES_Status_OK if everything is OK.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual AES_Status SetMode(AES_Modes mode, const UINT8 *IV = NULL);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Sets the key that will be used for encryption/decryption.
    ///
    /// @param key
    ///                 The key used for encryption/decryption.
    /// @return
    ///                 AES_Status_OK if everything is OK.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual AES_Status SetKey(const AES_Key *key);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Encrypts the data found at @a src and store the result at @a dst.
    /// Note that @a dst can be equal to @a src to perform in-place encryption.
    ///
    /// @param dst
    ///                 The address where the encrypted data will be put.
    /// @param src
    ///                 The address from where the plain text will be read.
    /// @param size
    ///                 The size in bytes of the data.
    ///                 The size of the data should be multiple of block size (padding will be done by the caller).
    /// @return
    ///                 The status of the operation.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual AES_Status Encrypt(UINT8 *dst, const UINT8 *src, UINT32 size);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Decrypts the data found at @a src and store the result at @a dst.
    /// Note that @a dst can be equal to @a src to perform in-place decryption.
    ///
    /// @param dst
    ///                 The address where the decrypted data will be put. 
    /// @param src
    ///                 The address from where the encrypted data will be read.
    /// @param size
    ///                 The size in bytes of the data.
    ///                 The size of the data should be multiple of block size (un-padding will be done by the caller).
    /// @return
    ///                 The status of the operation.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual AES_Status Decrypt(UINT8 *dst, const UINT8 *src, UINT32 size);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// @return The description of the class.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual IAlgorithmDescription& GetDescription();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Set the optimizations that will be used for encryption/decryption.
    ///
    /// @param opt
    ///                 The optimization level that will be used by the encryption/decryption algorithm.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void SetOptimization(AES_CPU_Optimization opt);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Set the number of threads that will be used by the encryption/decryption algorithm.
    ///
    /// @param nbThreads
    ///                 The number of threads to be used by the encryption/decryption algorithm.
    /// @param minSizeForParallel
    ///                 Specify the minimum size of the data that will be encrypted/decrypted,
    ///                 for the parallelization to occur (default value is 10240).
    /// @return
    ///                 The actual number of running threads.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual UINT32 SetNbThreads(UINT32 nbThreads, UINT32 minSizeForParallel);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Set the number of threads that will be used by the encryption/decryption,
    /// without modifying the needed minimum size of the data, for the parallelization to occur.
    ///
    /// @param nbThreads
    ///                 The number of threads to be used by the encryption/decryption algorithm.
    /// @return
    ///                 The actual number of running threads.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual UINT32 SetNbThreads(UINT32 nbThreads);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// @return The name of the device on which the encryption/decryption take place.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const TCHAR* GetDeviceName();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// The destructor used to release the resources
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~AES_CPU_Internal();

protected:
    /// The list of supported modes
    UINT32 m_SupportedModes;

    /// The current selected mode for encryption/decryption
    IAlgorithmAES::AES_Modes m_Mode;

    /// The size of the used key
    AES_Key::AES_KeySize m_keySize;

    /// The size in bytes of the entire key data
    UINT32 m_keyDataSize;

    /// The actual implementation of AES algorithm
    class AES_CPU_Impl *m_AES_Impl;

    /// The maximum number of threads that will be created
    UINT32 m_maxNbThreads;

    // The actual number of threads that will be used
    UINT32 m_nbThreads;

    // The number of blocks needed to use the parallelization
    UINT32 m_nbMinBlocksForParallel;

    /// The key data used for encryption/decryption
    /// ( maximum key data is for a key size of 256 bits => 14 rounds + initial key)
    __declspec(align(16)) UINT8 m_key[15 * IAlgorithmAES::BlockSizeBytes];

    /// The Initial Vector used by the algorithm
    __declspec(align(16)) UINT8 m_IV[IAlgorithmAES::BlockSizeBytes];

    /// The Initial Vector used by the encryption algorithm and updated after each encrypted block
    __declspec(align(16)) UINT8 m_internalEncIV[IAlgorithmAES::BlockSizeBytes];

    /// The Initial Vector used by the decryption algorithm and updated after each decrypted block
    __declspec(align(16)) UINT8 m_internalDecIV[IAlgorithmAES::BlockSizeBytes];

    /// The name of the device, or NULL if not initialized
    TCHAR *mDeviceName;
};

/// The interface of the actual implementation of AES algorithm for CPU
class AES_CPU_Impl
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Sets the key data for the algorithm
    ///
    /// @param keyData
    ///                 The pointer to the bytes that form the key.
    /// @param keyDataSize
    ///                 The size in bytes of the keyData.
    /// @param keySize
    ///                 The size in bits of the key (is not keyDataSize * 8 !!!).
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void SetKeyData(const UINT8 *keyData, UINT32 keyDataSize, AES_Key::AES_KeySize keySize) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Encrypts one block of data (16 bytes).
    ///
    /// @param dst
    ///                 The pointer to where the encrypted block will be stored.
    /// @param src
    ///                 The pointer to the block of data that will be encrypted.
    ///                 This can be equal to dst, to perform in-place encryption.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void EncryptBlock(UINT8 *dst, const UINT8 *src) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Decrypts one block of data (16 bytes).
    ///
    /// @param dst
    ///                 The pointer to where the decrypted block will be stored.
    /// @param src
    ///                 The pointer to the block of data that will be decrypted.
    ///                 This can be equal to dst, to perform in-place decryption.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void DecryptBlock(UINT8 *dst, const UINT8 *src) = 0;
};

#endif // __INCLUDED_AES_CPU_INTERNAL_H__
