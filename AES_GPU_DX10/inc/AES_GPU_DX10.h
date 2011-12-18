////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AES_GPU_DX10.h
///  Description: The interface of the AES implementation on the GPU.
///  Author:      Chiuta Adrian Marius
///  Created:     25-11-2009
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
#ifndef __INCLUDED_AES_GPU_DX10_H__
#define __INCLUDED_AES_GPU_DX10_H__

#include "AMCLibrary.h"

#ifdef AES_GPU_DX10_EXPORTS
#define AES_GPU_DX10_API __declspec(dllexport)
#else
#define AES_GPU_DX10_API __declspec(dllimport)
#endif

#define AES_GPU_DX10_ALGORITHM_CLASS_ID 0xAE5D3D10

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// The function that returns the description of the algorithm that implements the AES on the GPU.
///
/// @param acmVersion
///                 The version of the ACM library.
/// @return
///                 The pointer to the description of the library. Also the pointer can be NULL.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AES_GPU_DX10_API IAlgorithmDescription*
GetDescription(UINT32 acmVersion);

/// The interface to the GPU implementation of the AES algorithm.
class AES_GPU_DX10 : public IAlgorithmAES
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// @return The ID of the class.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CLASS_ID GetID() { return AES_GPU_DX10_ALGORITHM_CLASS_ID; }
};

#endif // __INCLUDED_AES_GPU_DX10_H__
