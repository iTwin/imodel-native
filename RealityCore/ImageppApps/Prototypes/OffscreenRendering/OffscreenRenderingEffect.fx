//--------------------------------------------------------------------------------------
// File: Tutorial02.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


static const float  REDFACTOR   = 0.2125f;
static const float  GREENFACTOR = 0.7154f;
static const float  BLUEFACTOR  = 0.0721f;

Texture2D<float4> tex2D;
Texture2D<float4> tex2DTarget;
matrix pixelTransformation;

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
};

struct VS_INPUT
{
	float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
};

SamplerState linearSampler
{
    //Filter = MIN_MAG_MIP_LINEAR;  // nearest
    //Filter = ANISOTROPIC;             // little slower.
    Filter = MIN_MAG_MIP_POINT;    // average?
    AddressU = Wrap;
    AddressV = Wrap;
    //AddressU = Border;
    //AddressV = Border;
    //BorderColor = float4(0,1.0, 0.0, 1.0);
};

//http://en.wikipedia.org/wiki/Alpha_compositing Porter and Duff 
//float newA = srcA + dstA * (1 - srcA);        
//    >>> SrcBlendAlpha = ONE;
//        DestBlendAlpha = INV_SRC_ALPHA;
//        BlendOpAlpha = ADD;

//float newC = (srcC * srcA + dstC * dstA * (1 - srcA)) / newA;

// Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)        == a premultiplied color.
// Adst' = 1 - ( (1 - Asrc) * (1 - Adst) ) == SrcA + dstA * (1-srcA)

// DirectX Blend equation 
// NewColor = (srcColor * srcBlendFactor) BlendOp (destColor * destBlendFactor)

BlendState SrcAlphaBlendingAdd
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = ONE;
    DestBlendAlpha = INV_SRC_ALPHA;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

BlendState SrcAlphaBlendingAdd_original
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = SRC_ALPHA;
    DestBlendAlpha = INV_SRC_ALPHA;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

BlendState AdditiveAlphaBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = ONE;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

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

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
};

RasterizerState DisableCulling
{
	CullMode = NONE;
};

RasterizerState EnableCulling
{
	CullMode = BACK;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output;

    output.Pos = mul(input.Pos, pixelTransformation);
    output.Tex = input.Tex;

    return output;
}        

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input ) : SV_Target
{
   //     float4 colorRGBA;
//     colorRGBA.rgba = 0.5;
//     return colorRGBA;
    // -- Transform RGB to gray, preserve alpha
    //float4 colorRGBA = tex2D.Load(input.Tex);
    float4 colorRGBA = tex2D.Sample(linearSampler, input.Tex);

//     colorRGBA.rgb = colorRGBA.r * REDFACTOR +
//                     colorRGBA.g * GREENFACTOR +
//                     colorRGBA.b * BLUEFACTOR;

    return colorRGBA;

    // -- Here is how we load from our texture with sampling.
    // return tex2D.Sample(linearSampler, Pos);
    
    // -- Load from texture without sampling. 
    //return tex2D.Load(Pos);

    // -- An arbritay color
    //return float4( 0.0f, 1.0f, 0.0f, 1.0f );  
}

// --------------------------------------------------------------------------------------
// Vertex Shader
// --------------------------------------------------------------------------------------
// float4 VS_indentity( float4 Pos : POSITION ) : SV_POSITION
// {
//     return Pos;
// }        
// 
// --------------------------------------------------------------------------------------
// Pixel Shader
// --------------------------------------------------------------------------------------
// float4 PS_RemovePremultiplied( float4 Pos : SV_POSITION ) : SV_Target
// {
//     // -- Transform RGB to gray, preserve alpha
//     float4 colorRGBA = tex2DTarget.Load(Pos);
//  
//  //   printf ("this is a test");
//  
//     colorRGBA.r = 1.0f;
//     colorRGBA.g = colorRGBA.g / colorRGBA.a;
//     colorRGBA.b = colorRGBA.a;
// 
//     return colorRGBA;
//     //return float4(0.0f, 1.0f, 0.0f, 1.0f );
// }

//--------------------------------------------------------------------------------------
technique10 Render
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );

        //SetBlendState( SrcAlphaBlendingAdd, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        //SetBlendState( AdditiveAlphaBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        
        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }

//     pass P1
//     {
//         SetVertexShader( CompileShader( vs_4_0, VS_indentity() ) );
//         SetGeometryShader( NULL );
//         SetPixelShader( CompileShader( ps_4_0, PS_RemovePremultiplied() ) );
// 
//     }
}


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS_Identity( VS_INPUT input )
{
    return input;
}        

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS_RGBtoLUV( PS_INPUT input ) : SV_Target
{
    float4 colorRGBA = tex2D.Load(input.Pos);

    float X = 0.412453*colorRGBA.r + 0.35758 *colorRGBA.g + 0.180423*colorRGBA.b;
    float Y = 0.212671*colorRGBA.r + 0.71516 *colorRGBA.g + 0.072169*colorRGBA.b;
    float Z = 0.019334*colorRGBA.r + 0.119193*colorRGBA.g + 0.950227*colorRGBA.b;

    float xn = 0.312713;
    float yn = 0.329016;

    float Yn = 1.0;

    float un = 4*xn / (-2*xn + 12*yn + 3);
    float vn = 9*yn / (-2*xn + 12*yn + 3);
    float u = 4*X / (X + 15*Y + 3*Z);
    float v = 9*Y / (X + 15*Y + 3*Z);
    float L = 116 * pow(Y/Yn, 1/3) - 16;
    float U = 13*L*(u-un);
    float V = 13*L*(v-vn);

    return float4(L, U, V, 1.0f );
}

technique10 RGBtoLUV
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );

        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( DisableCulling );
    }

}