////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AES_CPU_Impl_O3.cpp
///  Description: The CPU implementation for the AES using full optimizations + loop unroll
///               ( SubBytes, ShiftRows, MixColumns and AddRoundKey in one step,
///                 also working only with UINT32 ).
///  Author:      Chiuta Adrian Marius
///  Created:     17-11-2009
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
#include "AES_CPU_Impl_O3.h"
#include "AES_CPU_Impl_O2.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ENCRYPT_ROUND(so1, so2, so3, so4, si1, si2, si3, si4, round)\
{                                                                   \
    /* Make SubBytes, ShiftRows, MixColumns and AddRoundKey */      \
    /* in the same step and put the result back the state */        \
    so1 = sBoxMixColumn_a[si1 & 0xFF]           ^                   \
          sBoxMixColumn_b[(si2 >> 8) & 0xFF]    ^                   \
          sBoxMixColumn_c[(si3 >> 16) & 0xFF]   ^                   \
          sBoxMixColumn_d[si4 >> 24]            ^                   \
          r_key[(round * 4) + 0];                                   \
                                                                    \
    so2 = sBoxMixColumn_a[si2 & 0xFF]           ^                   \
          sBoxMixColumn_b[(si3 >> 8) & 0xFF]    ^                   \
          sBoxMixColumn_c[(si4 >> 16) & 0xFF]   ^                   \
          sBoxMixColumn_d[si1 >> 24]            ^                   \
          r_key[(round * 4) + 1];                                   \
                                                                    \
    so3 = sBoxMixColumn_a[si3 & 0xFF]           ^                   \
          sBoxMixColumn_b[(si4 >> 8) & 0xFF]    ^                   \
          sBoxMixColumn_c[(si1 >> 16) & 0xFF]   ^                   \
          sBoxMixColumn_d[si2 >> 24]            ^                   \
          r_key[(round * 4) + 2];                                   \
                                                                    \
    so4 = sBoxMixColumn_a[si4 & 0xFF]           ^                   \
          sBoxMixColumn_b[(si1 >> 8) & 0xFF]    ^                   \
          sBoxMixColumn_c[(si2 >> 16) & 0xFF]   ^                   \
          sBoxMixColumn_d[si3 >> 24]            ^                   \
          r_key[(round * 4) + 3];                                   \
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECRYPT_ROUND(so1, so2, so3, so4, si1, si2, si3, si4, round)\
{                                                                   \
   /*Make InvShiftRows, InvSubBytes, InvMixColumns and AddRoundKey*/\
   /* in the same step and put the result back in the state */      \
    so1 = rsBoxInvMixColumn_a[si1 & 0xFF]           ^               \
          rsBoxInvMixColumn_b[(si4 >> 8) & 0xFF]    ^               \
          rsBoxInvMixColumn_c[(si3 >> 16) & 0xFF]   ^               \
          rsBoxInvMixColumn_d[si2 >> 24]            ^               \
          r_key[(round * 4) + 0];                                   \
                                                                    \
    so2 = rsBoxInvMixColumn_a[si2 & 0xFF]           ^               \
          rsBoxInvMixColumn_b[(si1 >> 8) & 0xFF]    ^               \
          rsBoxInvMixColumn_c[(si4 >> 16) & 0xFF]   ^               \
          rsBoxInvMixColumn_d[si3 >> 24]            ^               \
          r_key[(round * 4) + 1];                                   \
                                                                    \
    so3 = rsBoxInvMixColumn_a[si3 & 0xFF]           ^               \
          rsBoxInvMixColumn_b[(si2 >> 8) & 0xFF]    ^               \
          rsBoxInvMixColumn_c[(si1 >> 16) & 0xFF]   ^               \
          rsBoxInvMixColumn_d[si4 >> 24]            ^               \
          r_key[(round * 4) + 2];                                   \
                                                                    \
    so4 = rsBoxInvMixColumn_a[si4 & 0xFF]           ^               \
          rsBoxInvMixColumn_b[(si3 >> 8) & 0xFF]    ^               \
          rsBoxInvMixColumn_c[(si2 >> 16) & 0xFF]   ^               \
          rsBoxInvMixColumn_d[si1 >> 24]            ^               \
          r_key[(round * 4) + 3];                                   \
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AES_CPU_Impl_O3::EncryptBlock(UINT8 *dst, const UINT8 *src)
{
    UINT32 st1, st2, st3, st4;
    UINT32 *r_key = (UINT32*)m_keyEnc,
           *src32 = (UINT32*)src,
           *dst32 = (UINT32*)dst;

    // First round will copy the data in the state and add the round key
    UINT32 s1 = src32[0] ^ r_key[0];
    UINT32 s2 = src32[1] ^ r_key[1];
    UINT32 s3 = src32[2] ^ r_key[2];
    UINT32 s4 = src32[3] ^ r_key[3];

    // Do the rest of the rounds
    ENCRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 1);
    ENCRYPT_ROUND(s1, s2, s3, s4, st1, st2, st3, st4, 2);
    ENCRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 3);
    ENCRYPT_ROUND(s1, s2, s3, s4, st1, st2, st3, st4, 4);
    ENCRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 5);
    ENCRYPT_ROUND(s1, s2, s3, s4, st1, st2, st3, st4, 6);
    ENCRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 7);
    ENCRYPT_ROUND(s1, s2, s3, s4, st1, st2, st3, st4, 8);
    ENCRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 9);
    r_key += 40;
    if(m_Nr > 10)
    {
        ENCRYPT_ROUND(s1, s2, s3, s4, st1, st2, st3, st4, 0);
        ENCRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 1);
        r_key += 8;
        if(m_Nr > 12)
        {
            ENCRYPT_ROUND(s1, s2, s3, s4, st1, st2, st3, st4, 0);
            ENCRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 1);
            r_key += 8;
        }
    }

    // The last round is special because does not have MixColumns
    // Make SubBytes, ShiftRows and AddRoundKey in the same step and copy the result in destination
    dst32[0] = ( sbox[st1 & 0xFF]               |
                (sbox[(st2 >> 8) & 0xFF] << 8)  |
                (sbox[(st3 >> 16) & 0xFF] << 16)|
                (sbox[st4 >> 24] << 24)
               ) ^ r_key[0];
    dst32[1] = ( sbox[st2 & 0xFF]               |
                (sbox[(st3 >> 8) & 0xFF] << 8)  |
                (sbox[(st4 >> 16) & 0xFF] << 16)|
                (sbox[st1 >> 24] << 24)
               ) ^ r_key[1];
    dst32[2] = ( sbox[st3 & 0xFF]               |
                (sbox[(st4 >> 8) & 0xFF] << 8)  |
                (sbox[(st1 >> 16) & 0xFF] << 16)|
                (sbox[st2 >> 24] << 24)
               ) ^ r_key[2];
    dst32[3] = ( sbox[st4 & 0xFF]               |
                (sbox[(st1 >> 8) & 0xFF] << 8)  |
                (sbox[(st2 >> 16) & 0xFF] << 16)|
                (sbox[st3 >> 24] << 24)
               ) ^ r_key[3];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AES_CPU_Impl_O3::DecryptBlock(UINT8 *dst, const UINT8 *src)
{
    UINT32 st1, st2, st3, st4;
    UINT32 *r_key = (UINT32*)m_keyDec,
           *src32 = (UINT32*)src,
           *dst32 = (UINT32*)dst;

    // First round will copy the data in the state and add the round key
    UINT32 s1 = src32[0] ^ r_key[0];
    UINT32 s2 = src32[1] ^ r_key[1];
    UINT32 s3 = src32[2] ^ r_key[2];
    UINT32 s4 = src32[3] ^ r_key[3];

    // Do the rest of the rounds
    DECRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 1);
    DECRYPT_ROUND(s1, s2, s3, s4, st1, st2, st3, st4, 2);
    DECRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 3);
    DECRYPT_ROUND(s1, s2, s3, s4, st1, st2, st3, st4, 4);
    DECRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 5);
    DECRYPT_ROUND(s1, s2, s3, s4, st1, st2, st3, st4, 6);
    DECRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 7);
    DECRYPT_ROUND(s1, s2, s3, s4, st1, st2, st3, st4, 8);
    DECRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 9);
    r_key += 40;
    if(m_Nr > 10)
    {
        DECRYPT_ROUND(s1, s2, s3, s4, st1, st2, st3, st4, 0);
        DECRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 1);
        r_key += 8;
        if(m_Nr > 12)
        {
            DECRYPT_ROUND(s1, s2, s3, s4, st1, st2, st3, st4, 0);
            DECRYPT_ROUND(st1, st2, st3, st4, s1, s2, s3, s4, 1);
            r_key += 8;
        }
    }

    // The last round is special because does not have InvMixColumns
    // Make InvShiftRows, InvSubBytes and AddRoundKey in the same step and copy the result in destination
    dst32[0] = ( rsbox[st1 & 0xFF]                  |
                (rsbox[(st4 >> 8) & 0xFF] << 8)     |
                (rsbox[(st3 >> 16) & 0xFF] << 16)   |
                (rsbox[st2 >> 24] << 24)
               ) ^ r_key[0];
    dst32[1] = ( rsbox[st2 & 0xFF]                  |
                (rsbox[(st1 >> 8) & 0xFF] << 8)     |
                (rsbox[(st4 >> 16) & 0xFF] << 16)   |
                (rsbox[st3 >> 24] << 24)
               ) ^ r_key[1];
    dst32[2] = ( rsbox[st3 & 0xFF]                  |
                (rsbox[(st2 >> 8) & 0xFF] << 8)     |
                (rsbox[(st1 >> 16) & 0xFF] << 16)   |
                (rsbox[st4 >> 24] << 24)
               ) ^ r_key[2];
    dst32[3] = ( rsbox[st4 & 0xFF]                  |
                (rsbox[(st3 >> 8) & 0xFF] << 8)     |
                (rsbox[(st2 >> 16) & 0xFF] << 16)   |
                (rsbox[st1 >> 24] << 24)
               ) ^ r_key[3];
}
