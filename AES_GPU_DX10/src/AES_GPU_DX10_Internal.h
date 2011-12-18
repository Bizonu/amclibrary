////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AES_GPU_DX10_Internal.h
///  Description: The implementation of the internal interface for the AES implementation on the GPU using DX10.
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
#ifndef __INCLUDED_AES_GPU_DX10_INTERNAL_H__
#define __INCLUDED_AES_GPU_DX10_INTERNAL_H__

#include "AES_GPU_DX10.h"
#include <d3d10.h>
#include <d3dx10.h>
#include <D3Dcompiler.h>

// The size of the texture used to hold the data that will be encrypted/decrypted.
// We will need to allocate two textures (destination and source).
#define TEXTURE_SIZEX 2048
#define TEXTURE_SIZEY 2048

typedef HRESULT ( WINAPI* LPCREATEDXGIFACTORY )( REFIID, void** );
typedef HRESULT ( WINAPI* LPD3D10CREATEDEVICE )( IDXGIAdapter*, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT32, ID3D10Device** );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// The high level implementation of AES algorithm on GPU
class AES_GPU_DX10_Internal : public AES_GPU_DX10
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// The function that will create the object needed for AES encryption/decryption on GPU
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static AES_GPU_DX10_Internal* Create();

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
    /// @return The name of the device on which the encryption/decryption take place.
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const TCHAR* GetDeviceName();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// The destructor used to release the resources
    ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~AES_GPU_DX10_Internal();

protected:
    AES_GPU_DX10_Internal();

    /// The list of supported modes
    UINT32 m_SupportedModes;

    /// The current selected mode for encryption/decryption
    IAlgorithmAES::AES_Modes m_Mode;

    /// The size of the used key
    AES_Key::AES_KeySize m_keySize;

    /// The size in bytes of the entire key data
    UINT32 m_keyDataSize;

    /// The key data used for encryption
    /// ( maximum key data is for a key size of 256 bits => 14 rounds + initial key)
    __declspec(align(16)) UINT8 m_keyEnc[15 * IAlgorithmAES::BlockSizeBytes];

    /// The key data used for decryption 
    /// ( maximum key data is for a key size of 256 bits => 14 rounds + initial key)
    __declspec(align(16)) UINT8 m_keyDec[15 * IAlgorithmAES::BlockSizeBytes];

    /// The Initial Vector used by the algorithm
    __declspec(align(16)) UINT8 m_IV[IAlgorithmAES::BlockSizeBytes];

    /// The Initial Vector used by the encryption algorithm and updated after each encrypted block
    __declspec(align(16)) UINT8 m_internalEncIV[IAlgorithmAES::BlockSizeBytes];

    /// The Initial Vector used by the decryption algorithm and updated after each decrypted block
    __declspec(align(16)) UINT8 m_internalDecIV[IAlgorithmAES::BlockSizeBytes];

    /// The name of the device, or NULL if not initialized
    TCHAR *mDeviceName;

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////DirectX 10 specific implementation///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
private:
    void SetShaderIV(UINT8 *IV);
    void SetShaderKey();
    void CopyDataToGPU(const UINT8 *src, UINT32 sizeX, UINT32 sizeY, UINT32 lastLineBytes = 0);
    void CopyDataFromGPU(UINT8 *dst, UINT32 sizeX, UINT32 sizeY, UINT32 lastLineBytes = 0);
    void RunEncryption();
    void RunDecryption();
private:
    /// The handle to the D3D10.dll
    HMODULE                             m_hModD3D10;
    /// The device for Direct3D 10
    ID3D10Device                        *m_pDevice;

    /// The interface to the effect file
    ID3D10Effect                        *m_pEffect10;

    /// The layout of the quad geometry
    ID3D10InputLayout                   *m_pQuadVertexLayout;

    /// The buffer for the vertices of the quad
    ID3D10Buffer                        *m_pQuadVB;

    /// The techniques currently selected for encryption/decryption
    ID3D10EffectTechnique               *m_pEncrypt, *m_pDecrypt;

    /// The techniques used for encryption/decryption in ECB mode
    ID3D10EffectTechnique               *m_pEncryptECB, *m_pDecryptECB;

    /// The techniques used for encryption and decryption in CTR mode and decryption in CBC mode
    ID3D10EffectTechnique               *m_pEncDecCTR, *m_pDecryptCBC;

    /// The source and destination texture
    ID3D10Texture2D                     *m_pSourceTexture, *m_pDestTexture;

    /// The source texture view
    ID3D10ShaderResourceView            *m_pSourceTexRV;

    /// The destination render target view
    ID3D10RenderTargetView              *m_pDestRTV;

    // The texture used to copy data from m_pDestTexture to the memory addressable by the CPU
    ID3D10Texture2D                     *m_pStagingDstTexture;

    // The texture used to copy data from CPU addressable memory to m_pSourceTexture
    ID3D10Texture2D                     *m_pStagingSrcTexture;

    /// The interface to the shader variable that holds the IV
    ID3D10EffectVectorVariable          *m_pShaderIV;

    /// The interface to the shader variable that holds the key of encryption
    ID3D10EffectVectorVariable          *m_pShaderKeyEnc;

    /// The interface to the shader variable that holds the key of decryption
    ID3D10EffectVectorVariable          *m_pShaderKeyDec;

    /// The interface to the shader variable that holds the number of rounds
    ID3D10EffectScalarVariable          *m_pShaderKeySize;

    /// The texture that contains only constants for the algorithm
    ID3D10Texture2D                     *m_pTexConstants;

    /// The texture view for the constants texture
    ID3D10ShaderResourceView            *m_pTexConstantsRV;
};

#endif // __INCLUDED_AES_GPU_DX10_INTERNAL_H__
