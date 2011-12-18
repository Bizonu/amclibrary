////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AES_CPU.h
///  Description: The interface of the AES implementation on the CPU.
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
#ifndef __INCLUDED_AES_CPU_H__
#define __INCLUDED_AES_CPU_H__

#include "AMCLibrary.h"

#ifdef AES_CPU_EXPORTS
    #define AES_CPU_API __declspec(dllexport)
#else
    #define AES_CPU_API __declspec(dllimport)
#endif

#define AES_CPU_ALGORITHM_CLASS_ID 0xAE510000

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// The function that returns the description of the algorithm that implements the AES on the CPU.
///
/// @param acmVersion
///                 The version of the ACM library.
/// @return
///                 The pointer to the description of the library. Also the pointer can be NULL.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AES_CPU_API IAlgorithmDescription*
GetDescription(UINT32 acmVersion);

/// The interface to the CPU implementation of the AES algorithm.
class AES_CPU : public IAlgorithmAES
{
public:
    enum AES_CPU_Optimization
    {
        // No optimizations at all ( for debugging / learning - very slow decryption )
        AES_CPU_O0,
        // Medium optimizations ( MixColumns using LUT and other functions made in same
        // step, also decryption is done using Equivalent Inverse Cipher )
        AES_CPU_O1,
        // Full optimizations ( SubBytes, ShiftRows, MixColumns and AddRoundKey in one step, also working only with UINT32 )
        AES_CPU_O2,
        // Full optimizations + loop unroll
        AES_CPU_O3
    };

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// @return The ID of the class.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CLASS_ID GetID() { return AES_CPU_ALGORITHM_CLASS_ID; }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Set the optimizations that will be used for encryption/decryption.
    ///
    /// @param opt
    ///                 The type of the optimizations that will be used by the algorithm.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void SetOptimization(AES_CPU_Optimization opt) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Set the number of threads that will be used by the encryption/decryption.
    ///
    /// @param nbThreads
    ///                 The number of threads that will be used for encryption/decryption.
    /// @param minSizeForParallel
    ///                 Specify the minimum size of the data that will be encrypted/decrypted,
    ///                 for the parallelization to occur (default value is 10240).
    /// @return
    ///                 The actual number of running threads.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual UINT32 SetNbThreads(UINT32 nbThreads, UINT32 minSizeForParallel) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Set the number of threads that will be used by the encryption/decryption.
    ///
    /// @param nbThreads
    ///                 The number of threads that will be used for encryption/decryption.
    /// @return
    ///                 The actual number of running threads.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual UINT32 SetNbThreads(UINT32 nbThreads) = 0;
};

#endif // __INCLUDED_AES_CPU_H__
