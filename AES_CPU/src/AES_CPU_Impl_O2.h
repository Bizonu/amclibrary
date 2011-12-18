////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AES_CPU_Impl_O2.h
///  Description: The CPU implementation for the AES using full optimizations
///               ( SubBytes, ShiftRows, MixColumns and AddRoundKey in one step,
///                 also working only with UINT32 ).
///  Author:      Chiuta Adrian Marius
///  Created:     15-11-2009
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
#ifndef __INCLUDED_AES_CPU_IMPL_O2_H__
#define __INCLUDED_AES_CPU_IMPL_O2_H__

#include "AES_CPU_Impl_O1.h"

// The class that implements the AES CPU encryption/decryption using full optimizations.
class AES_CPU_Impl_O2 : public AES_CPU_Impl_O1
{
public:
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
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern __declspec(align(16)) const UINT32 sBoxMixColumn_a[256];
extern __declspec(align(16)) const UINT32 sBoxMixColumn_b[256];
extern __declspec(align(16)) const UINT32 sBoxMixColumn_c[256];
extern __declspec(align(16)) const UINT32 sBoxMixColumn_d[256];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern __declspec(align(16)) const UINT32 rsBoxInvMixColumn_a[256];
extern __declspec(align(16)) const UINT32 rsBoxInvMixColumn_b[256];
extern __declspec(align(16)) const UINT32 rsBoxInvMixColumn_c[256];
extern __declspec(align(16)) const UINT32 rsBoxInvMixColumn_d[256];

#endif // __INCLUDED_AES_CPU_IMPL_O2_H__
