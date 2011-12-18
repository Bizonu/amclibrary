////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        IAlgorithmAES.h
///  Description: The interface of the AES algorithms.
///  Author:      Chiuta Adrian Marius
///  Created:     23-10-2009
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
#ifndef __INCLUDED_IALGORITHMAES_H__
#define __INCLUDED_IALGORITHMAES_H__

#include "AMCLibrary.h"
#include "AES_Key.h"

#define AES_ALGORITHM_CLASS_ID 0xAE51AE51

/// The interface of the AES algorithms
class IAlgorithmAES : public IAlgorithm
{
public:
    /// The size in bits of the block that will be encrypted/decrypted.
    static const UINT32 BlockSize = 128;

    /// The size in bytes of the block that will be encrypted/decrypted.
    static const UINT32 BlockSizeBytes = 16;

    /// The possible modes of operation for AES encryption/decryption.
    enum AES_Modes
    {
        /// Standard AES mode of operation. Every block is encrypted/decrypted separately.
        AES_ECB     = 1,
        /// Cipher-Block Chaining (CBC) mode, each block of plain-text is XORed
        /// with the previous cipher-text block before being encrypted.
        AES_CBC     = 2,
        /// Propagating Cipher-Block Chaining
        AES_PCBC    = 4,
        /// Cipher FeedBack (CFB) mode, makes a block cipher into a self-synchronizing stream cipher
        AES_CFB     = 8,
        /// Output FeedBack (OFB) mode makes a block cipher into a synchronous stream cipher
        AES_OFB     = 16,
        /// Counter mode turns a block cipher into a stream cipher
        AES_CTR     = 32
    };

    /// The status code specific for AES algorithms
    enum AES_Status
    {
        /// Everything is OK
        AES_Status_OK           = 0,
        /// The operation failed, unspecified reason
        AES_Status_Fail         = 1,
        /// The specified operation is not supported
        AES_Status_NotSupported = 2,
        /// One of the arguments is not correct
        AES_Status_InvalidArgs  = 3,
        /// The state of the algorithm is not valid
        AES_Status_InvalidState = 4
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Returns all the modes supported by the algorithm.
    ///
    /// @param Encryption
    ///                 If true, then the returned modes are available
    ///                 for encryption, else for decryption.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual UINT32 GetSupportedModes(bool Encryption = true) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Sets the mode of operation for the algorithm, and if needed, also sets 
    /// the IV (Initialization Vector) needed by the mode.
    ///
    /// @param mode
    ///                 The mode of operation.
    /// @param IV
    ///                 If IV is NULL, then all the values from IV are considered to be zero.
    ///                 Node that the size of the IV is the same as the &ref BlockSize.
    /// @return
    ///                 AES_Status_OK if everything is OK.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual AES_Status SetMode(AES_Modes mode, const UINT8 *IV = NULL) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Sets the key that will be used for encryption/decryption.
    ///
    /// @param key
    ///                 The key used for encryption/decryption.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual AES_Status SetKey(const AES_Key *key) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Encrypts the data found at src and store the result at dst.
    /// The size of the data should be multiple of block size (padding will be done by the caller).
    /// Note that @ref dst can be equal to @ref src.
    ///
    /// @param dst
    ///                 The address where the encrypted data will be put. 
    /// @param src
    ///                 The address from where the plain text will be read.
    /// @param size
    ///                 The size in bytes of the data.
    /// @return
    ///                 The status of the operation.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual AES_Status Encrypt(UINT8 *dst, const UINT8 *src, UINT32 size) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Decrypts the data found at src and store the result at dst. 
    /// The size of the data should be multiple of block size (un-padding will be done by the caller).
    /// Note that @ref dst can be equal to @ref src.
    ///
    /// @param dst
    ///                 The address where the decrypted data will be put. 
    /// @param src
    ///                 The address from where the encrypted data will be read.
    /// @param size
    ///                 The size in bytes of the data.
    /// @return
    ///                 The status of the operation.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual AES_Status Decrypt(UINT8 *dst, const UINT8 *src, UINT32 size) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// @return         The name of the device on which the encryption/decryption take place.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const TCHAR* GetDeviceName() = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// @return         The ID of the interface/class from which this class is derived.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CLASS_ID GetInterfaceID() { return AES_ALGORITHM_CLASS_ID; }
};

#endif // __INCLUDED_IALGORITHMAES_H__
