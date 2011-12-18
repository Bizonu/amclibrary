////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AES_CPU_Impl_O1.h
///  Description: The CPU implementation for the AES using medium optimizations
///               ( MixColumns using LUT and other functions made in same
///                 step, also decryption is done using Equivalent Inverse Cipher )
///  Author:      Chiuta Adrian Marius
///  Created:     12-11-2009
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
#ifndef __INCLUDED_AES_CPU_IMPL_O1_H__
#define __INCLUDED_AES_CPU_IMPL_O1_H__

#include "AES_CPU_Impl_O0.h"

// The class that implements the AES CPU encryption/decryption using medium optimizations.
class AES_CPU_Impl_O1 : public AES_CPU_Impl
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
    virtual void SetKeyData(const UINT8 *keyData, UINT32 keyDataSize, AES_Key::AES_KeySize keySize);

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
    virtual void EncryptBlock(UINT8 *dst, const UINT8 *src);

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
    virtual void DecryptBlock(UINT8 *dst, const UINT8 *src);

protected:
    /// The size of the used key
    AES_Key::AES_KeySize m_keySize;

    /// The number of columns comprising a state in AES. This is a constant in AES.
    static const UINT32 m_Nb = 4;

    /// The number of rounds in AES Cipher.
    UINT32 m_Nr;

    /// The number of 32 bit words in the key.
    UINT32 m_Nk;

    /// The key data used for encryption
    /// ( maximum key data is for a key size of 256 bits => 14 rounds + initial key)
    __declspec(align(16)) UINT8 m_keyEnc[15 * IAlgorithmAES::BlockSizeBytes];

    /// The key data used for decryption
    /// ( maximum key data is for a key size of 256 bits => 14 rounds + initial key)
    __declspec(align(16)) UINT8 m_keyDec[15 * IAlgorithmAES::BlockSizeBytes];
};

#endif // __INCLUDED_AES_CPU_IMPL_O1_H__
