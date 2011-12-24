////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AES_GPU_DX10_SHADER.fx
///  Description: The shaders used for AES encryption on the GPU.
///  Author:      Chiuta Adrian Marius
///  Created:     30-11-2009
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
#define ALLOW_ONLY_TX_CB

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The global variables used by the shaders
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
cbuffer cbOften0
{
    // The initial vector
    uint4 IV;
};

cbuffer cbOften1
{
    // The encryption key for each round
    uint4 key[15];
    // The decryption key for each round
    uint4 rkey[15];
    // The number of rounds ( can be 10, 12 and 14 )
    uint keySize;
};

cbuffer ConstantTextureSize
{
    // The size of the texture
    float2  txSize;
}

// The constants used by the algorithm ( the same used for AES implementation on CPU )
cbuffer cbNeverChanges
{
    uint sBoxMixColumn_a[256];
    uint sBoxMixColumn_b[256];
    uint sBoxMixColumn_c[256];
    uint sBoxMixColumn_d[256];
    uint rsBoxInvMixColumn_a[256];
    uint rsBoxInvMixColumn_b[256];
    uint rsBoxInvMixColumn_c[256];
    uint rsBoxInvMixColumn_d[256];
    uint sBox[256];
    uint rsBox[256];
};

// The texture from which we will read the input
Texture2D<uint4> txSource;

// The constants used by the algorithm, mapped as a texture
// Line 0: sBoxMixColumn_a
// Line 1: sBoxMixColumn_b
// Line 2: sBoxMixColumn_c
// Line 3: sBoxMixColumn_d
// Line 4: rsBoxInvMixColumn_a
// Line 5: rsBoxInvMixColumn_b
// Line 6: rsBoxInvMixColumn_c
// Line 7: rsBoxInvMixColumn_d
// Line 8: sBox
// Line 9: rsBox
Texture2D<uint>  txConstants;

// The texture buffers which contains the constants used by the algorithm
tbuffer tb1 { uint tsBoxMixColumn_a[256]; };
tbuffer tb2 { uint tsBoxMixColumn_b[256]; };
tbuffer tb3 { uint tsBoxMixColumn_c[256]; };
tbuffer tb4 { uint tsBoxMixColumn_d[256]; };
tbuffer tb5 { uint tsBox[256]; };
tbuffer tb6 { uint trsBoxInvMixColumn_a[256]; };
tbuffer tb7 { uint trsBoxInvMixColumn_b[256]; };
tbuffer tb8 { uint trsBoxInvMixColumn_c[256]; };
tbuffer tb9 { uint trsBoxInvMixColumn_d[256]; };
tbuffer tb10 { uint trsBox[256]; };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The structures that defines the input for vertex and pixel shader
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct VSQuadIn
{
    float3 pos  : POSITION; // Position of the vertex [-1, 1]
    float2 tex  : TEXTURE0; // Texture coordinate [0, 1]
};

struct PSQuadIn
{
    float4 pos  : SV_Position;
    float2 tex  : TEXTURE0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The states of the pipeline that are being used
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

RasterizerState DisableCulling
{
    CullMode = NONE;
    ScissorEnable = TRUE;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The vertex shader is the same for all the techniques
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PSQuadIn VSQuad( VSQuadIn input )
{
    PSQuadIn output;

    // Output our final position
    output.pos.xy   = input.pos.xy; //input comes in [-1..1] range
    output.pos.z    = 1.0f;
    output.pos.w    = 1.0f;
    
    // Rescale the texture coordinate so we will have the real UVs
    output.tex = input.tex * txSize;

    // Pass them to the pixel shader
    return output;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint4 ROL(uint4 value, uint bits)
{
    return (value << bits) | (value >> (32 - bits));
}

uint4 ROR(uint4 value, uint bits)
{
    return (value >> bits) | (value << (32 - bits));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////AES ECB ENCRYPTION PIXEL SHADERS///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ALLOW_ONLY_TX_CB

// Encrypt in ECB mode using the constant buffers ( Very slow on Geforce !!! )
uint4 PSEncode_ECB_cb( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ key[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(sBoxMixColumn_a[istate.x], 
                       sBoxMixColumn_a[istate.y], 
                       sBoxMixColumn_a[istate.z], 
                       sBoxMixColumn_a[istate.w]);

        istate = (state >> 8) & 0xFF;
        tstate^= uint4(sBoxMixColumn_b[istate.y], 
                       sBoxMixColumn_b[istate.z], 
                       sBoxMixColumn_b[istate.w], 
                       sBoxMixColumn_b[istate.x]);

        istate = (state >> 16) & 0xFF;
        tstate^= uint4(sBoxMixColumn_c[istate.z], 
                       sBoxMixColumn_c[istate.w], 
                       sBoxMixColumn_c[istate.x], 
                       sBoxMixColumn_c[istate.y]);

        istate = state >> 24;
        tstate^= uint4(sBoxMixColumn_d[istate.w], 
                       sBoxMixColumn_d[istate.x], 
                       sBoxMixColumn_d[istate.y], 
                       sBoxMixColumn_d[istate.z]);

        state = tstate ^ key[i];
    }

    istate = state & 0xFF;
    tstate = uint4(sBox[istate.x], 
                   sBox[istate.y], 
                   sBox[istate.z], 
                   sBox[istate.w]);

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(sBox[istate.y], 
                   sBox[istate.z], 
                   sBox[istate.w], 
                   sBox[istate.x]) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(sBox[istate.z], 
                   sBox[istate.w], 
                   sBox[istate.x], 
                   sBox[istate.y]) << 16;

    istate = state >> 24;
    tstate|= uint4(sBox[istate.w], 
                   sBox[istate.x], 
                   sBox[istate.y], 
                   sBox[istate.z]) << 24;

    return tstate ^ key[keySize];
}

// Encrypt in ECB mode using the constant buffers ( On Geforce is faster than PSEncode_ECB_cb )
uint4 PSEncode_ECB_cb_ROL( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ key[0];

    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(sBoxMixColumn_a[istate.x], 
                       sBoxMixColumn_a[istate.y], 
                       sBoxMixColumn_a[istate.z], 
                       sBoxMixColumn_a[istate.w]);

        istate = (state >> 8) & 0xFF;
        tstate^= ROL(uint4(sBoxMixColumn_a[istate.y], 
                           sBoxMixColumn_a[istate.z], 
                           sBoxMixColumn_a[istate.w], 
                           sBoxMixColumn_a[istate.x]), 8);

        istate = (state >> 16) & 0xFF;
        tstate^= ROL(uint4(sBoxMixColumn_a[istate.z], 
                           sBoxMixColumn_a[istate.w], 
                           sBoxMixColumn_a[istate.x], 
                           sBoxMixColumn_a[istate.y]), 16);

        istate = state >> 24;
        tstate^= ROL(uint4(sBoxMixColumn_a[istate.w], 
                           sBoxMixColumn_a[istate.x], 
                           sBoxMixColumn_a[istate.y], 
                           sBoxMixColumn_a[istate.z]), 24);

        state = tstate ^ key[i];
    }

    istate = state & 0xFF;
    tstate = uint4(sBox[istate.x], 
                   sBox[istate.y], 
                   sBox[istate.z], 
                   sBox[istate.w]);

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(sBox[istate.y], 
                   sBox[istate.z], 
                   sBox[istate.w], 
                   sBox[istate.x]) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(sBox[istate.z], 
                   sBox[istate.w], 
                   sBox[istate.x], 
                   sBox[istate.y]) << 16;

    istate = state >> 24;
    tstate|= uint4(sBox[istate.w], 
                   sBox[istate.x], 
                   sBox[istate.y], 
                   sBox[istate.z]) << 24;

    return tstate ^ key[keySize];
}

// Encrypt in ECB mode using the texture buffers
uint4 PSEncode_ECB_tb( PSQuadIn input ) : SV_Target
{
    uint x, y, z, w;
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ key[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        x = tsBoxMixColumn_a[istate.x];
        y = tsBoxMixColumn_a[istate.y];
        z = tsBoxMixColumn_a[istate.z];
        w = tsBoxMixColumn_a[istate.w];
        tstate = uint4(x, y, z, w); 

        istate = (state >> 8) & 0xFF;
        x = tsBoxMixColumn_b[istate.y];
        y = tsBoxMixColumn_b[istate.z];
        z = tsBoxMixColumn_b[istate.w];
        w = tsBoxMixColumn_b[istate.x];
        tstate^= uint4(x, y, z, w);

        istate = (state >> 16) & 0xFF;
        x = tsBoxMixColumn_c[istate.z];
        y = tsBoxMixColumn_c[istate.w];
        z = tsBoxMixColumn_c[istate.x];
        w = tsBoxMixColumn_c[istate.y];
        tstate^= uint4(x, y, z, w); 

        istate = state >> 24;
        x = tsBoxMixColumn_d[istate.w];
        y = tsBoxMixColumn_d[istate.x];
        z = tsBoxMixColumn_d[istate.y];
        w = tsBoxMixColumn_d[istate.z];
        tstate^= uint4(x, y, z, w); 

        state = tstate ^ key[i];
    }

    istate = state & 0xFF;
    x = tsBox[istate.x];
    y = tsBox[istate.y];
    z = tsBox[istate.z];
    w = tsBox[istate.w];
    tstate = uint4(x, y, z, w); 

    istate = (state >> 8) & 0xFF;
    x = tsBox[istate.y];
    y = tsBox[istate.z];
    z = tsBox[istate.w];
    w = tsBox[istate.x];
    tstate|= uint4(x, y, z, w) << 8;

    istate = (state >> 16) & 0xFF;
    x = tsBox[istate.z];
    y = tsBox[istate.w];
    z = tsBox[istate.x];
    w = tsBox[istate.y];
    tstate|= uint4(x, y, z, w) << 16;

    istate = state >> 24;
    x = tsBox[istate.w];
    y = tsBox[istate.x];
    z = tsBox[istate.y];
    w = tsBox[istate.z];
    tstate|= uint4(x, y, z, w) << 24;

    return tstate ^ key[keySize];
}

// Encrypt in ECB mode using the texture buffers and constant buffers
uint4 PSEncode_ECB_tb_cb( PSQuadIn input ) : SV_Target
{
    uint x, y, z, w;
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ key[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        x = tsBoxMixColumn_a[istate.x];
        y = tsBoxMixColumn_a[istate.y];
        z = tsBoxMixColumn_a[istate.z];
        w = tsBoxMixColumn_a[istate.w];
        tstate = uint4(x, y, z, w); 

        istate = (state >> 8) & 0xFF;
        x = tsBoxMixColumn_b[istate.y];
        y = tsBoxMixColumn_b[istate.z];
        z = tsBoxMixColumn_b[istate.w];
        w = tsBoxMixColumn_b[istate.x];
        tstate^= uint4(x, y, z, w);

        istate = (state >> 16) & 0xFF;
        x = tsBoxMixColumn_c[istate.z];
        y = tsBoxMixColumn_c[istate.w];
        z = tsBoxMixColumn_c[istate.x];
        w = tsBoxMixColumn_c[istate.y];
        tstate^= uint4(x, y, z, w); 

        istate = state >> 24;
        x = tsBoxMixColumn_d[istate.w];
        y = tsBoxMixColumn_d[istate.x];
        z = tsBoxMixColumn_d[istate.y];
        w = tsBoxMixColumn_d[istate.z];
        tstate^= uint4(x, y, z, w); 

        state = tstate ^ key[i];
    }

    istate = state & 0xFF;
    tstate = uint4(sBox[istate.x], 
                   sBox[istate.y], 
                   sBox[istate.z], 
                   sBox[istate.w]);

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(sBox[istate.y], 
                   sBox[istate.z], 
                   sBox[istate.w], 
                   sBox[istate.x]) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(sBox[istate.z], 
                   sBox[istate.w], 
                   sBox[istate.x], 
                   sBox[istate.y]) << 16;

    istate = state >> 24;
    tstate|= uint4(sBox[istate.w], 
                   sBox[istate.x], 
                   sBox[istate.y], 
                   sBox[istate.z]) << 24;

    return tstate ^ key[keySize];
}

// Encrypt in ECB mode using the constant textures ( On Geforce is faster than PSEncode_ECB_cb_ROL )
uint4 PSEncode_ECB_tx( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ key[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(txConstants.Load(int3(istate.x, 0, 0)), 
                       txConstants.Load(int3(istate.y, 0, 0)), 
                       txConstants.Load(int3(istate.z, 0, 0)), 
                       txConstants.Load(int3(istate.w, 0, 0)));

        istate = (state >> 8) & 0xFF;
        tstate^= uint4(txConstants.Load(int3(istate.y, 1, 0)), 
                       txConstants.Load(int3(istate.z, 1, 0)), 
                       txConstants.Load(int3(istate.w, 1, 0)), 
                       txConstants.Load(int3(istate.x, 1, 0)));

        istate = (state >> 16) & 0xFF;
        tstate^= uint4(txConstants.Load(int3(istate.z, 2, 0)), 
                       txConstants.Load(int3(istate.w, 2, 0)), 
                       txConstants.Load(int3(istate.x, 2, 0)), 
                       txConstants.Load(int3(istate.y, 2, 0)));

        istate = state >> 24;
        tstate^= uint4(txConstants.Load(int3(istate.w, 3, 0)), 
                       txConstants.Load(int3(istate.x, 3, 0)), 
                       txConstants.Load(int3(istate.y, 3, 0)), 
                       txConstants.Load(int3(istate.z, 3, 0)));

        state = tstate ^ key[i];
    }

    istate = state & 0xFF;
    tstate = uint4(txConstants.Load(int3(istate.x, 8, 0)), 
                   txConstants.Load(int3(istate.y, 8, 0)), 
                   txConstants.Load(int3(istate.z, 8, 0)), 
                   txConstants.Load(int3(istate.w, 8, 0)));

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(txConstants.Load(int3(istate.y, 8, 0)), 
                   txConstants.Load(int3(istate.z, 8, 0)), 
                   txConstants.Load(int3(istate.w, 8, 0)), 
                   txConstants.Load(int3(istate.x, 8, 0))) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(txConstants.Load(int3(istate.z, 8, 0)), 
                   txConstants.Load(int3(istate.w, 8, 0)), 
                   txConstants.Load(int3(istate.x, 8, 0)), 
                   txConstants.Load(int3(istate.y, 8, 0))) << 16;

    istate = state >> 24;
    tstate|= uint4(txConstants.Load(int3(istate.w, 8, 0)), 
                   txConstants.Load(int3(istate.x, 8, 0)), 
                   txConstants.Load(int3(istate.y, 8, 0)), 
                   txConstants.Load(int3(istate.z, 8, 0))) << 24;

    return tstate ^ key[keySize];
}
#endif // #ifndef ALLOW_ONLY_TX_CB

// Encrypt in ECB mode using the constant textures ( the fastest version for Geforce )
uint4 PSEncode_ECB_tx_cb( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ key[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(txConstants.Load(int3(istate.x, 0, 0)), 
                       txConstants.Load(int3(istate.y, 0, 0)), 
                       txConstants.Load(int3(istate.z, 0, 0)), 
                       txConstants.Load(int3(istate.w, 0, 0)));

        istate = (state >> 8) & 0xFF;
        tstate^= uint4(txConstants.Load(int3(istate.y, 1, 0)), 
                       txConstants.Load(int3(istate.z, 1, 0)), 
                       txConstants.Load(int3(istate.w, 1, 0)), 
                       txConstants.Load(int3(istate.x, 1, 0)));

        istate = (state >> 16) & 0xFF;
        tstate^= uint4(txConstants.Load(int3(istate.z, 2, 0)), 
                       txConstants.Load(int3(istate.w, 2, 0)), 
                       txConstants.Load(int3(istate.x, 2, 0)), 
                       txConstants.Load(int3(istate.y, 2, 0)));

        istate = state >> 24;
        tstate^= uint4(txConstants.Load(int3(istate.w, 3, 0)), 
                       txConstants.Load(int3(istate.x, 3, 0)), 
                       txConstants.Load(int3(istate.y, 3, 0)), 
                       txConstants.Load(int3(istate.z, 3, 0)));

        state = tstate ^ key[i];
    }

    istate = state & 0xFF;
    tstate = uint4(sBox[istate.x], 
                   sBox[istate.y], 
                   sBox[istate.z], 
                   sBox[istate.w]);

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(sBox[istate.y], 
                   sBox[istate.z], 
                   sBox[istate.w], 
                   sBox[istate.x]) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(sBox[istate.z], 
                   sBox[istate.w], 
                   sBox[istate.x], 
                   sBox[istate.y]) << 16;

    istate = state >> 24;
    tstate|= uint4(sBox[istate.w], 
                   sBox[istate.x], 
                   sBox[istate.y], 
                   sBox[istate.z]) << 24;

    return tstate ^ key[keySize];
}

#ifndef ALLOW_ONLY_TX_CB

// Encrypt in ECB mode using the constant textures ( is slower than PSEncode_ECB_tx )
uint4 PSEncode_ECB_tx_ROL( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ key[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(txConstants.Load(int3(istate.x, 0, 0)), 
                       txConstants.Load(int3(istate.y, 0, 0)), 
                       txConstants.Load(int3(istate.z, 0, 0)), 
                       txConstants.Load(int3(istate.w, 0, 0)));

        istate = (state >> 8) & 0xFF;
        tstate^= ROL(uint4(txConstants.Load(int3(istate.y, 0, 0)), 
                           txConstants.Load(int3(istate.z, 0, 0)), 
                           txConstants.Load(int3(istate.w, 0, 0)), 
                           txConstants.Load(int3(istate.x, 0, 0))), 8);

        istate = (state >> 16) & 0xFF;
        tstate^= ROL(uint4(txConstants.Load(int3(istate.z, 0, 0)), 
                           txConstants.Load(int3(istate.w, 0, 0)), 
                           txConstants.Load(int3(istate.x, 0, 0)), 
                           txConstants.Load(int3(istate.y, 0, 0))), 16);

        istate = state >> 24;
        tstate^= ROL(uint4(txConstants.Load(int3(istate.w, 0, 0)), 
                           txConstants.Load(int3(istate.x, 0, 0)), 
                           txConstants.Load(int3(istate.y, 0, 0)), 
                           txConstants.Load(int3(istate.z, 0, 0))), 24);

        state = tstate ^ key[i];
    }

    istate = state & 0xFF;
    tstate = uint4(txConstants.Load(int3(istate.x, 8, 0)), 
                   txConstants.Load(int3(istate.y, 8, 0)), 
                   txConstants.Load(int3(istate.z, 8, 0)), 
                   txConstants.Load(int3(istate.w, 8, 0)));

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(txConstants.Load(int3(istate.y, 8, 0)), 
                   txConstants.Load(int3(istate.z, 8, 0)), 
                   txConstants.Load(int3(istate.w, 8, 0)), 
                   txConstants.Load(int3(istate.x, 8, 0))) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(txConstants.Load(int3(istate.z, 8, 0)), 
                   txConstants.Load(int3(istate.w, 8, 0)), 
                   txConstants.Load(int3(istate.x, 8, 0)), 
                   txConstants.Load(int3(istate.y, 8, 0))) << 16;

    istate = state >> 24;
    tstate|= uint4(txConstants.Load(int3(istate.w, 8, 0)), 
                   txConstants.Load(int3(istate.x, 8, 0)), 
                   txConstants.Load(int3(istate.y, 8, 0)), 
                   txConstants.Load(int3(istate.z, 8, 0))) << 24;

    return tstate ^ key[keySize];
}

// Encrypt in ECB mode using the constant textures ( is slower than PSEncode_ECB_tx_cb )
uint4 PSEncode_ECB_tx_cb_ROL( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ key[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(txConstants.Load(int3(istate.x, 0, 0)), 
                       txConstants.Load(int3(istate.y, 0, 0)), 
                       txConstants.Load(int3(istate.z, 0, 0)), 
                       txConstants.Load(int3(istate.w, 0, 0)));

        istate = (state >> 8) & 0xFF;
        tstate^= ROL(uint4(txConstants.Load(int3(istate.y, 0, 0)), 
                           txConstants.Load(int3(istate.z, 0, 0)), 
                           txConstants.Load(int3(istate.w, 0, 0)), 
                           txConstants.Load(int3(istate.x, 0, 0))), 8);

        istate = (state >> 16) & 0xFF;
        tstate^= ROL(uint4(txConstants.Load(int3(istate.z, 0, 0)), 
                           txConstants.Load(int3(istate.w, 0, 0)), 
                           txConstants.Load(int3(istate.x, 0, 0)), 
                           txConstants.Load(int3(istate.y, 0, 0))), 16);

        istate = state >> 24;
        tstate^= ROL(uint4(txConstants.Load(int3(istate.w, 0, 0)), 
                           txConstants.Load(int3(istate.x, 0, 0)), 
                           txConstants.Load(int3(istate.y, 0, 0)), 
                           txConstants.Load(int3(istate.z, 0, 0))), 24);

        state = tstate ^ key[i];
    }

    istate = state & 0xFF;
    tstate = uint4(sBox[istate.x], 
                   sBox[istate.y], 
                   sBox[istate.z], 
                   sBox[istate.w]);

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(sBox[istate.y], 
                   sBox[istate.z], 
                   sBox[istate.w], 
                   sBox[istate.x]) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(sBox[istate.z], 
                   sBox[istate.w], 
                   sBox[istate.x], 
                   sBox[istate.y]) << 16;

    istate = state >> 24;
    tstate|= uint4(sBox[istate.w], 
                   sBox[istate.x], 
                   sBox[istate.y], 
                   sBox[istate.z]) << 24;

    return tstate ^ key[keySize];
}
#endif // #ifndef ALLOW_ONLY_TX_CB

// Encrypt and Decrypt in CTR mode using the constant textures ( the fastest version for Geforce )
uint4 PSEncodeDecode_CTR_tx_cb( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    int3 txCoord = int3(input.tex, 0);

    int blockID = txCoord.x + txCoord.y * txSize.x;
    state = IV;
    state.x += blockID;
    if( state.x < IV.x )
    {
        state.y ++;
        if( state.y == 0 )
        {
            state.z ++;
            if( state.z == 0 )
                state.w ++;
        }
    }

    state ^= key[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(txConstants.Load(int3(istate.x, 0, 0)), 
                       txConstants.Load(int3(istate.y, 0, 0)), 
                       txConstants.Load(int3(istate.z, 0, 0)), 
                       txConstants.Load(int3(istate.w, 0, 0)));

        istate = (state >> 8) & 0xFF;
        tstate^= uint4(txConstants.Load(int3(istate.y, 1, 0)), 
                       txConstants.Load(int3(istate.z, 1, 0)), 
                       txConstants.Load(int3(istate.w, 1, 0)), 
                       txConstants.Load(int3(istate.x, 1, 0)));

        istate = (state >> 16) & 0xFF;
        tstate^= uint4(txConstants.Load(int3(istate.z, 2, 0)), 
                       txConstants.Load(int3(istate.w, 2, 0)), 
                       txConstants.Load(int3(istate.x, 2, 0)), 
                       txConstants.Load(int3(istate.y, 2, 0)));

        istate = state >> 24;
        tstate^= uint4(txConstants.Load(int3(istate.w, 3, 0)), 
                       txConstants.Load(int3(istate.x, 3, 0)), 
                       txConstants.Load(int3(istate.y, 3, 0)), 
                       txConstants.Load(int3(istate.z, 3, 0)));

        state = tstate ^ key[i];
    }

    istate = state & 0xFF;
    tstate = uint4(sBox[istate.x], 
                   sBox[istate.y], 
                   sBox[istate.z], 
                   sBox[istate.w]);

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(sBox[istate.y], 
                   sBox[istate.z], 
                   sBox[istate.w], 
                   sBox[istate.x]) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(sBox[istate.z], 
                   sBox[istate.w], 
                   sBox[istate.x], 
                   sBox[istate.y]) << 16;

    istate = state >> 24;
    tstate|= uint4(sBox[istate.w], 
                   sBox[istate.x], 
                   sBox[istate.y], 
                   sBox[istate.z]) << 24;

    return tstate ^ key[keySize] ^ txSource.Load(txCoord);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////AES ECB DECRYPTION PIXEL SHADERS///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ALLOW_ONLY_TX_CB

// Decrypt in ECB mode using the constant buffers ( Very slow on Geforce !!! )
uint4 PSDecode_ECB_cb( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ rkey[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(rsBoxInvMixColumn_a[istate.x], 
                       rsBoxInvMixColumn_a[istate.y], 
                       rsBoxInvMixColumn_a[istate.z], 
                       rsBoxInvMixColumn_a[istate.w]);

        istate = (state >> 8) & 0xFF;
        tstate^= uint4(rsBoxInvMixColumn_b[istate.w], 
                       rsBoxInvMixColumn_b[istate.x], 
                       rsBoxInvMixColumn_b[istate.y], 
                       rsBoxInvMixColumn_b[istate.z]);

        istate = (state >> 16) & 0xFF;
        tstate^= uint4(rsBoxInvMixColumn_c[istate.z], 
                       rsBoxInvMixColumn_c[istate.w], 
                       rsBoxInvMixColumn_c[istate.x], 
                       rsBoxInvMixColumn_c[istate.y]);

        istate = state >> 24;
        tstate^= uint4(rsBoxInvMixColumn_d[istate.y], 
                       rsBoxInvMixColumn_d[istate.z], 
                       rsBoxInvMixColumn_d[istate.w], 
                       rsBoxInvMixColumn_d[istate.x]);

        state = tstate ^ rkey[i];
    }

    istate = state & 0xFF;
    tstate = uint4(rsBox[istate.x], 
                   rsBox[istate.y], 
                   rsBox[istate.z], 
                   rsBox[istate.w]);

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(rsBox[istate.w], 
                   rsBox[istate.x], 
                   rsBox[istate.y], 
                   rsBox[istate.z]) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(rsBox[istate.z], 
                   rsBox[istate.w], 
                   rsBox[istate.x], 
                   rsBox[istate.y]) << 16;

    istate = state >> 24;
    tstate|= uint4(rsBox[istate.y], 
                   rsBox[istate.z], 
                   rsBox[istate.w], 
                   rsBox[istate.x]) << 24;

    return tstate ^ rkey[keySize];
}

// Decrypt in ECB mode using the constant buffers ( On Geforce is faster than PSDecode_ECB_cb )
uint4 PSDecode_ECB_cb_ROL( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ rkey[0];

    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(rsBoxInvMixColumn_a[istate.x], 
                       rsBoxInvMixColumn_a[istate.y], 
                       rsBoxInvMixColumn_a[istate.z], 
                       rsBoxInvMixColumn_a[istate.w]);

        istate = (state >> 8) & 0xFF;
        tstate^= ROL(uint4(rsBoxInvMixColumn_a[istate.w], 
                           rsBoxInvMixColumn_a[istate.x], 
                           rsBoxInvMixColumn_a[istate.y], 
                           rsBoxInvMixColumn_a[istate.z]), 8);

        istate = (state >> 16) & 0xFF;
        tstate^= ROL(uint4(rsBoxInvMixColumn_a[istate.z], 
                           rsBoxInvMixColumn_a[istate.w], 
                           rsBoxInvMixColumn_a[istate.x], 
                           rsBoxInvMixColumn_a[istate.y]), 16);

        istate = state >> 24;
        tstate^= ROL(uint4(rsBoxInvMixColumn_a[istate.y], 
                           rsBoxInvMixColumn_a[istate.z], 
                           rsBoxInvMixColumn_a[istate.w], 
                           rsBoxInvMixColumn_a[istate.x]), 24);

        state = tstate ^ rkey[i];
    }

    istate = state & 0xFF;
    tstate = uint4(rsBox[istate.x], 
                   rsBox[istate.y], 
                   rsBox[istate.z], 
                   rsBox[istate.w]);

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(rsBox[istate.w], 
                   rsBox[istate.x], 
                   rsBox[istate.y], 
                   rsBox[istate.z]) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(rsBox[istate.z], 
                   rsBox[istate.w], 
                   rsBox[istate.x], 
                   rsBox[istate.y]) << 16;

    istate = state >> 24;
    tstate|= uint4(rsBox[istate.y], 
                   rsBox[istate.z], 
                   rsBox[istate.w], 
                   rsBox[istate.x]) << 24;

    return tstate ^ rkey[keySize];
}

// Decrypt in ECB mode using the constant textures ( On Geforce is faster than PSDecode_ECB_cb_ROL )
uint4 PSDecode_ECB_tx( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ rkey[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(txConstants.Load(int3(istate.x, 4, 0)), 
                       txConstants.Load(int3(istate.y, 4, 0)), 
                       txConstants.Load(int3(istate.z, 4, 0)), 
                       txConstants.Load(int3(istate.w, 4, 0)));

        istate = (state >> 8) & 0xFF;
        tstate^= uint4(txConstants.Load(int3(istate.w, 5, 0)), 
                       txConstants.Load(int3(istate.x, 5, 0)), 
                       txConstants.Load(int3(istate.y, 5, 0)), 
                       txConstants.Load(int3(istate.z, 5, 0)));

        istate = (state >> 16) & 0xFF;
        tstate^= uint4(txConstants.Load(int3(istate.z, 6, 0)), 
                       txConstants.Load(int3(istate.w, 6, 0)), 
                       txConstants.Load(int3(istate.x, 6, 0)), 
                       txConstants.Load(int3(istate.y, 6, 0)));

        istate = state >> 24;
        tstate^= uint4(txConstants.Load(int3(istate.y, 7, 0)), 
                       txConstants.Load(int3(istate.z, 7, 0)), 
                       txConstants.Load(int3(istate.w, 7, 0)), 
                       txConstants.Load(int3(istate.x, 7, 0)));

        state = tstate ^ rkey[i];
    }

    istate = state & 0xFF;
    tstate = uint4(txConstants.Load(int3(istate.x, 9, 0)), 
                   txConstants.Load(int3(istate.y, 9, 0)), 
                   txConstants.Load(int3(istate.z, 9, 0)), 
                   txConstants.Load(int3(istate.w, 9, 0)));

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(txConstants.Load(int3(istate.w, 9, 0)), 
                   txConstants.Load(int3(istate.x, 9, 0)), 
                   txConstants.Load(int3(istate.y, 9, 0)), 
                   txConstants.Load(int3(istate.z, 9, 0))) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(txConstants.Load(int3(istate.z, 9, 0)), 
                   txConstants.Load(int3(istate.w, 9, 0)), 
                   txConstants.Load(int3(istate.x, 9, 0)), 
                   txConstants.Load(int3(istate.y, 9, 0))) << 16;

    istate = state >> 24;
    tstate|= uint4(txConstants.Load(int3(istate.y, 9, 0)), 
                   txConstants.Load(int3(istate.z, 9, 0)), 
                   txConstants.Load(int3(istate.w, 9, 0)), 
                   txConstants.Load(int3(istate.x, 9, 0))) << 24;

    return tstate ^ rkey[keySize];
}
#endif // #ifndef ALLOW_ONLY_TX_CB

// Decrypt in ECB mode using the constant textures ( the fastest version for Geforce )
uint4 PSDecode_ECB_tx_cb( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ rkey[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(txConstants.Load(int3(istate.x, 4, 0)), 
                       txConstants.Load(int3(istate.y, 4, 0)), 
                       txConstants.Load(int3(istate.z, 4, 0)), 
                       txConstants.Load(int3(istate.w, 4, 0)));

        istate = (state >> 8) & 0xFF;
        tstate^= uint4(txConstants.Load(int3(istate.w, 5, 0)), 
                       txConstants.Load(int3(istate.x, 5, 0)), 
                       txConstants.Load(int3(istate.y, 5, 0)), 
                       txConstants.Load(int3(istate.z, 5, 0)));

        istate = (state >> 16) & 0xFF;
        tstate^= uint4(txConstants.Load(int3(istate.z, 6, 0)), 
                       txConstants.Load(int3(istate.w, 6, 0)), 
                       txConstants.Load(int3(istate.x, 6, 0)), 
                       txConstants.Load(int3(istate.y, 6, 0)));

        istate = state >> 24;
        tstate^= uint4(txConstants.Load(int3(istate.y, 7, 0)), 
                       txConstants.Load(int3(istate.z, 7, 0)), 
                       txConstants.Load(int3(istate.w, 7, 0)), 
                       txConstants.Load(int3(istate.x, 7, 0)));

        state = tstate ^ rkey[i];
    }

    istate = state & 0xFF;
    tstate = uint4(rsBox[istate.x], 
                   rsBox[istate.y], 
                   rsBox[istate.z], 
                   rsBox[istate.w]);

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(rsBox[istate.w], 
                   rsBox[istate.x], 
                   rsBox[istate.y], 
                   rsBox[istate.z]) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(rsBox[istate.z], 
                   rsBox[istate.w], 
                   rsBox[istate.x], 
                   rsBox[istate.y]) << 16;

    istate = state >> 24;
    tstate|= uint4(rsBox[istate.y], 
                   rsBox[istate.z], 
                   rsBox[istate.w], 
                   rsBox[istate.x]) << 24;

    return tstate ^ rkey[keySize];
}

#ifndef ALLOW_ONLY_TX_CB
// Decrypt in ECB mode using the constant textures ( is slower than PSDecode_ECB_tx )
uint4 PSDecode_ECB_tx_ROL( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ rkey[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(txConstants.Load(int3(istate.x, 4, 0)), 
                       txConstants.Load(int3(istate.y, 4, 0)), 
                       txConstants.Load(int3(istate.z, 4, 0)), 
                       txConstants.Load(int3(istate.w, 4, 0)));

        istate = (state >> 8) & 0xFF;
        tstate^= ROL(uint4(txConstants.Load(int3(istate.w, 4, 0)), 
                           txConstants.Load(int3(istate.x, 4, 0)), 
                           txConstants.Load(int3(istate.y, 4, 0)), 
                           txConstants.Load(int3(istate.z, 4, 0))), 8);

        istate = (state >> 16) & 0xFF;
        tstate^= ROL(uint4(txConstants.Load(int3(istate.z, 4, 0)), 
                           txConstants.Load(int3(istate.w, 4, 0)), 
                           txConstants.Load(int3(istate.x, 4, 0)), 
                           txConstants.Load(int3(istate.y, 4, 0))), 16);

        istate = state >> 24;
        tstate^= ROL(uint4(txConstants.Load(int3(istate.y, 4, 0)), 
                           txConstants.Load(int3(istate.z, 4, 0)), 
                           txConstants.Load(int3(istate.w, 4, 0)), 
                           txConstants.Load(int3(istate.x, 4, 0))), 24);

        state = tstate ^ rkey[i];
    }

    istate = state & 0xFF;
    tstate = uint4(txConstants.Load(int3(istate.x, 9, 0)), 
                   txConstants.Load(int3(istate.y, 9, 0)), 
                   txConstants.Load(int3(istate.z, 9, 0)), 
                   txConstants.Load(int3(istate.w, 9, 0)));

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(txConstants.Load(int3(istate.w, 9, 0)), 
                   txConstants.Load(int3(istate.x, 9, 0)), 
                   txConstants.Load(int3(istate.y, 9, 0)), 
                   txConstants.Load(int3(istate.z, 9, 0))) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(txConstants.Load(int3(istate.z, 9, 0)), 
                   txConstants.Load(int3(istate.w, 9, 0)), 
                   txConstants.Load(int3(istate.x, 9, 0)), 
                   txConstants.Load(int3(istate.y, 9, 0))) << 16;

    istate = state >> 24;
    tstate|= uint4(txConstants.Load(int3(istate.y, 8, 0)), 
                   txConstants.Load(int3(istate.z, 8, 0)), 
                   txConstants.Load(int3(istate.w, 8, 0)), 
                   txConstants.Load(int3(istate.x, 8, 0))) << 24;

    return tstate ^ rkey[keySize];
}

// Decrypt in ECB mode using the constant textures ( is slower than PSDecode_ECB_tx_cb )
uint4 PSDecode_ECB_tx_cb_ROL( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    state = txSource.Load(int3(input.tex, 0)) ^ rkey[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(txConstants.Load(int3(istate.x, 4, 0)), 
                       txConstants.Load(int3(istate.y, 4, 0)), 
                       txConstants.Load(int3(istate.z, 4, 0)), 
                       txConstants.Load(int3(istate.w, 4, 0)));

        istate = (state >> 8) & 0xFF;
        tstate^= ROL(uint4(txConstants.Load(int3(istate.w, 4, 0)), 
                           txConstants.Load(int3(istate.x, 4, 0)), 
                           txConstants.Load(int3(istate.y, 4, 0)), 
                           txConstants.Load(int3(istate.z, 4, 0))), 8);

        istate = (state >> 16) & 0xFF;
        tstate^= ROL(uint4(txConstants.Load(int3(istate.z, 4, 0)), 
                           txConstants.Load(int3(istate.w, 4, 0)), 
                           txConstants.Load(int3(istate.x, 4, 0)), 
                           txConstants.Load(int3(istate.y, 4, 0))), 16);

        istate = state >> 24;
        tstate^= ROL(uint4(txConstants.Load(int3(istate.y, 4, 0)), 
                           txConstants.Load(int3(istate.z, 4, 0)), 
                           txConstants.Load(int3(istate.w, 4, 0)), 
                           txConstants.Load(int3(istate.x, 4, 0))), 24);

        state = tstate ^ rkey[i];
    }

    istate = state & 0xFF;
    tstate = uint4(rsBox[istate.x], 
                   rsBox[istate.y], 
                   rsBox[istate.z], 
                   rsBox[istate.w]);

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(rsBox[istate.w], 
                   rsBox[istate.x], 
                   rsBox[istate.y], 
                   rsBox[istate.z]) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(rsBox[istate.z], 
                   rsBox[istate.w], 
                   rsBox[istate.x], 
                   rsBox[istate.y]) << 16;

    istate = state >> 24;
    tstate|= uint4(rsBox[istate.y], 
                   rsBox[istate.z], 
                   rsBox[istate.w], 
                   rsBox[istate.x]) << 24;

    return tstate ^ rkey[keySize];
}
#endif // #ifndef ALLOW_ONLY_TX_CB

// Decrypt in CBC mode using the constant textures ( the fastest version for Geforce )
uint4 PSDecode_CBC_tx_cb( PSQuadIn input ) : SV_Target
{
    uint4 state, istate, tstate;
    int3 txCoord = int3(input.tex, 0);

    state = txSource.Load(txCoord) ^ rkey[0];
    
    for(uint i = 1; i < keySize; i++)
    {
        istate = state & 0xFF;
        tstate = uint4(txConstants.Load(int3(istate.x, 4, 0)), 
                       txConstants.Load(int3(istate.y, 4, 0)), 
                       txConstants.Load(int3(istate.z, 4, 0)), 
                       txConstants.Load(int3(istate.w, 4, 0)));

        istate = (state >> 8) & 0xFF;
        tstate^= uint4(txConstants.Load(int3(istate.w, 5, 0)), 
                       txConstants.Load(int3(istate.x, 5, 0)), 
                       txConstants.Load(int3(istate.y, 5, 0)), 
                       txConstants.Load(int3(istate.z, 5, 0)));

        istate = (state >> 16) & 0xFF;
        tstate^= uint4(txConstants.Load(int3(istate.z, 6, 0)), 
                       txConstants.Load(int3(istate.w, 6, 0)), 
                       txConstants.Load(int3(istate.x, 6, 0)), 
                       txConstants.Load(int3(istate.y, 6, 0)));

        istate = state >> 24;
        tstate^= uint4(txConstants.Load(int3(istate.y, 7, 0)), 
                       txConstants.Load(int3(istate.z, 7, 0)), 
                       txConstants.Load(int3(istate.w, 7, 0)), 
                       txConstants.Load(int3(istate.x, 7, 0)));

        state = tstate ^ rkey[i];
    }

    istate = state & 0xFF;
    tstate = uint4(rsBox[istate.x], 
                   rsBox[istate.y], 
                   rsBox[istate.z], 
                   rsBox[istate.w]);

    istate = (state >> 8) & 0xFF;
    tstate|= uint4(rsBox[istate.w], 
                   rsBox[istate.x], 
                   rsBox[istate.y], 
                   rsBox[istate.z]) << 8;

    istate = (state >> 16) & 0xFF;
    tstate|= uint4(rsBox[istate.z], 
                   rsBox[istate.w], 
                   rsBox[istate.x], 
                   rsBox[istate.y]) << 16;

    istate = state >> 24;
    tstate|= uint4(rsBox[istate.y], 
                   rsBox[istate.z], 
                   rsBox[istate.w], 
                   rsBox[istate.x]) << 24;

    txCoord += (txCoord.x == 0) ? int3(txSize.x - 1, -1, 0) : int3(-1, 0, 0);

    return tstate ^ rkey[keySize] ^ txSource.Load(txCoord);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////DEFINITION OF TECHNIQUES/////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The definition of the techniques used to encrypt in ECB mode ( from the fastest to the slowest )
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
technique10 AES_Encrypt_ECB_tx_cb
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSEncode_ECB_tx_cb() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}

#ifndef ALLOW_ONLY_TX_CB
technique10 AES_Encrypt_ECB_tx
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSEncode_ECB_tx() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}

technique10 AES_Encrypt_ECB_tx_cb_ROL
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSEncode_ECB_tx_cb_ROL() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}

technique10 AES_Encrypt_ECB_tx_ROL
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSEncode_ECB_tx_ROL() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}

technique10 AES_Encrypt_ECB_tb
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSEncode_ECB_tb() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}

technique10 AES_Encrypt_ECB_tb_cb
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSEncode_ECB_tb_cb() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}

technique10 AES_Encrypt_ECB_cb_ROL
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSEncode_ECB_cb_ROL() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}

technique10 AES_Encrypt_ECB_cb
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSEncode_ECB_cb() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}
#endif // #ifndef ALLOW_ONLY_TX_CB

technique10 AES_EncryptDecrypt_CTR_tx_cb
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSEncodeDecode_CTR_tx_cb() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The definition of the techniques used to decrypt in ECB mode ( from the fastest to the slowest )
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
technique10 AES_Decrypt_ECB_tx_cb
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSDecode_ECB_tx_cb() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}

#ifndef ALLOW_ONLY_TX_CB
technique10 AES_Decrypt_ECB_tx
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSDecode_ECB_tx() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}

technique10 AES_Decrypt_ECB_tx_cb_ROL
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSDecode_ECB_tx_cb_ROL() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}

technique10 AES_Decrypt_ECB_tx_ROL
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSDecode_ECB_tx_ROL() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}

technique10 AES_Decrypt_ECB_cb_ROL
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSDecode_ECB_cb_ROL() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}

technique10 AES_Decrypt_ECB_cb
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSDecode_ECB_cb() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}
#endif // #ifndef ALLOW_ONLY_TX_CB

technique10 AES_Decrypt_CBC_tx_cb
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSQuad() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSDecode_CBC_tx_cb() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }
}
