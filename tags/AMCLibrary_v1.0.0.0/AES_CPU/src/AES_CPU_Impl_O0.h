////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AES_CPU_Impl_O0.h
///  Description: The CPU implementation for the AES using no optimizations at all
///               (based solely on the FIPS 197 Advanced Encryption Standard - AES).
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
#ifndef __INCLUDED_AES_CPU_IMPL_O0_H__
#define __INCLUDED_AES_CPU_IMPL_O0_H__

#include "AES_CPU_Internal.h"

// The class that implements the AES CPU encryption/decryption using no optimizations.
class AES_CPU_Impl_O0 : public AES_CPU_Impl
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

private:
    /// Adds the round key to the state
    void AddRoundKey(UINT8 m_state[4][4], UINT32 round);

    /// Substitutes the values in the state matrix with values in an S-box.
    void SubBytes(UINT8 m_state[4][4]);

    /// Substitutes the values in the state matrix with values in an inverse S-box.
    void InvSubBytes(UINT8 m_state[4][4]);

    /// Shifts the rows in the state to the left. Each row is shifted with different offset.
    /// Offset = Row number. So the first row is not shifted.
    void ShiftRows(UINT8 m_state[4][4]);

    /// Shifts the rows in the state to the right. Each row is shifted with different offset.
    /// Offset = Row number. So the first row is not shifted.
    void InvShiftRows(UINT8 m_state[4][4]);

    /// Mixes the columns of the state matrix
    void MixColumns(UINT8 m_state[4][4]);

    /// Mixes the columns of the state matrix.
    void InvMixColumns(UINT8 m_state[4][4]);

protected:
    /// The size of the used key
    AES_Key::AES_KeySize m_keySize;

    /// The number of columns comprising a state in AES. This is a constant in AES.
    static const UINT32 m_Nb = 4;

    /// The number of rounds in AES Cipher.
    UINT32 m_Nr;

    /// The number of 32 bit words in the key.
    UINT32 m_Nk;

    /// The key data used for encryption/decryption
    /// ( maximum key data is for a key size of 256 bits => 14 rounds + initial key)
    __declspec(align(16)) UINT8 m_key[15 * IAlgorithmAES::BlockSizeBytes];
};

/// Finds the product of {02} and the argument to xtime modulo {1b}  
#define xtime(x)   (((x) << 1) ^ ((((x) >> 7) & 1) * 0x1b))

/// Multiply numbers in the field GF(2^8)
#define Multiply(x,y) ((((y) & 1) * (x)) ^ ((((y) >> 1) & 1) * xtime(x)) ^ ((((y) >> 2) & 1) * xtime(xtime(x))) ^ ((((y) >> 3) & 1) * xtime(xtime(xtime(x)))) ^ ((((y) >> 4) & 1) * xtime(xtime(xtime(xtime(x))))))

/// The S box used to encrypt. Make it external because we will use it also in other files.
extern __declspec(align(16)) const UINT8 sbox[256];

/// The reversed S box used to decrypt. Make it external because we will use it also in other files.
extern __declspec(align(16)) const UINT8 rsbox[256];

#endif // __INCLUDED_AES_CPU_IMPL_O0_H__
