/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/OffscreenRendering/D3DProcessing.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include "D3DProcessing.h"

#define ENABLE_PROFILER
#include "Profiler.h"

struct SimpleVertex
    {
    D3DXVECTOR3 Pos;  // Position
    D3DXVECTOR2 texCoord;
    };

D3DProcessing::D3DProcessing(ID3D10Device* pDevice)
    :m_pDevice(pDevice),
        m_pEffect(NULL),
        m_pRenderTileTechnique(NULL),
        m_pRenderTileTechniqueRGBtoLUV(NULL),
        m_pRenderTileLayout(NULL),
        m_pShaderSrcTexture(NULL),
        m_pPixelTransformation(NULL),
        m_pRenderTarget(NULL),
        m_pRenderTargetView(NULL),
        m_pStagingResource(NULL),
        m_pSrcTexture(NULL),
        m_pSrcTextureShaderResView(NULL),
        m_pTexReceiver(NULL)
    {
    assert(m_pDevice != NULL);
    }

D3DProcessing::~D3DProcessing()
    {
    SAFE_RELEASE(m_pRenderTileLayout);


    SAFE_RELEASE(m_pRenderTargetView);
    SAFE_RELEASE(m_pRenderTarget);
        
    SAFE_RELEASE(m_pStagingResource);
    SAFE_RELEASE(m_pSrcTexture);
    SAFE_RELEASE(m_pSrcTextureShaderResView);

    SAFE_RELEASE(m_pTexReceiver);       

    m_pRenderTileTechnique = 0; // access pointer hold by m_pEffect
    m_pRenderTileTechniqueRGBtoLUV = 0;
    m_pShaderSrcTexture = 0;    // access pointer hold by m_pEffect
    m_pPixelTransformation = 0; // access pointer hold by m_pEffect
    SAFE_RELEASE(m_pEffect);
        
    m_pDevice->Release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT D3DProcessing::LoadEffectFile(wstring const& filename)
    {
    // Create the effect
    ID3DBlob* pErrorBlob = NULL;
    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif
    HRESULT hr = D3DX10CreateEffectFromFileW( filename.c_str(), NULL, NULL, "fx_4_0", dwShaderFlags, 0, m_pDevice, 
                                                NULL, NULL, &m_pEffect, &pErrorBlob, NULL );
    if( FAILED( hr ) )
        {
        if( pErrorBlob != NULL )
            {
            MessageBoxA(NULL, (char*)pErrorBlob->GetBufferPointer(), "Error", MB_OK);
            pErrorBlob->Release();
            return hr;
            }
        MessageBoxW(NULL, L"The FX file cannot be located.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
        }

    return hr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT D3DProcessing::CreateTechnique()
    {
    // Obtain the technique
    m_pRenderTileTechnique = m_pEffect->GetTechniqueByName( "Render" );
    assert(m_pRenderTileTechnique != NULL);

    m_pRenderTileTechniqueRGBtoLUV = m_pEffect->GetTechniqueByName( "RGBtoLUV" );
    assert(m_pRenderTileTechniqueRGBtoLUV != NULL);

    // Define the input layout
    D3D10_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },  
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D10_INPUT_PER_VERTEX_DATA, 0 }
        };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    D3D10_PASS_DESC PassDesc;
    m_pRenderTileTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
    HRESULT hr = m_pDevice->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature,
                                                PassDesc.IAInputSignatureSize, &m_pRenderTileLayout );
    if( FAILED( hr ) )
        return hr;

    // Not sure it belong here. It is a post init step
    m_pShaderSrcTexture = m_pEffect->GetVariableByName( "tex2D" )->AsShaderResource();
    m_pPixelTransformation = m_pEffect->GetVariableByName( "pixelTransformation" )->AsMatrix();

    return hr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT D3DProcessing::CreateRenderTargetAndStagingResource()
    {
    // ---- BEGIN: Create our render target.  
    D3D10_TEXTURE2D_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.Width = TILE_SIZE;
    desc.Height = TILE_SIZE;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D10_USAGE_DEFAULT;
    desc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;

    HRESULT hr = m_pDevice->CreateTexture2D( &desc, NULL, &m_pRenderTarget );
    assert(SUCCEEDED(hr));

    // Textures cannot be bound for rendering to the pipeline directly; use a render-target view
    D3D10_RENDER_TARGET_VIEW_DESC rtDesc;
    rtDesc.Format = desc.Format;
    rtDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
    rtDesc.Texture2D.MipSlice = 0;
    hr = m_pDevice->CreateRenderTargetView( m_pRenderTarget, &rtDesc, &m_pRenderTargetView );
    assert(SUCCEEDED(hr));

    // Similarly to how a render-target view must be created so that the render target can be bound for output to the pipeline, 
    // a shader-resource view must be created so that the render target can be bound to the pipeline as an input. 
//         D3D10_SHADER_RESOURCE_VIEW_DESC srDesc;
//         srDesc.Format = desc.Format;
//         srDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
//         srDesc.Texture2D.MostDetailedMip = 0;
//         srDesc.Texture2D.MipLevels = 1;
//         ID3D10ShaderResourceView *pShaderResView = NULL;
//         hr = m_pDevice->CreateShaderResourceView( m_pRenderTarget, &srDesc, &pShaderResView );
//         assert(SUCCEEDED(hr));
    // ---- END: Create our render target.  

    // From this render target create a compatible staging buffer so we can read back GPU result.
    // ----- BEGIN: Create a staging resource to get our data back to the CPU
    D3D10_TEXTURE2D_DESC dstex;
    m_pRenderTarget->GetDesc( &dstex );
    dstex.Usage = D3D10_USAGE_STAGING;                      // Staging allows us to copy to and from the GPU
    dstex.BindFlags = 0;                                    // Staging resources cannot be bound to ANY part of the pipeline
    dstex.CPUAccessFlags = D3D10_CPU_ACCESS_READ;           // We want to read from this resource
    hr = m_pDevice->CreateTexture2D( &dstex, NULL, &m_pStagingResource );
    assert(SUCCEEDED(hr));

    // ----- END: Create a staging resource to get our data back to the CPU
    return hr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT D3DProcessing::CreateSourceTexture()
    {
    // ----- BEGIN: Create texture resource from memory
    D3D10_TEXTURE2D_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.Width = TILE_SIZE;
    desc.Height = TILE_SIZE;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D10_USAGE_DYNAMIC;     
    desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;     
    
    HRESULT hr = m_pDevice->CreateTexture2D( &desc, NULL, &m_pSrcTexture);
    assert(SUCCEEDED(hr));

    D3D10_SHADER_RESOURCE_VIEW_DESC srDesc;
    srDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    srDesc.Texture2D.MostDetailedMip = 0;
    srDesc.Texture2D.MipLevels = 1;
    hr = m_pDevice->CreateShaderResourceView(m_pSrcTexture, &srDesc, &m_pSrcTextureShaderResView );
    assert(SUCCEEDED(hr));

    // ----- END: Create texture resource from memory

    return hr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT D3DProcessing::CreateTextureReceiver()
    {
    // ---- BEGIN: Create triangles to map our texture.

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        D3DXVECTOR3( -1.0f, -1.0f, 0.0f ),  D3DXVECTOR2(0.0f, 1.0f),
        D3DXVECTOR3( -1.0f, 1.0f, 0.0f ),   D3DXVECTOR2(0.0f, 0.0f), 
        D3DXVECTOR3( 1.0f, -1.0f, 0.0f ),   D3DXVECTOR2(1.0f, 1.0f),
        D3DXVECTOR3( 1.0f, 1.0f, 0.0f ),    D3DXVECTOR2(1.0f, 0.0f),
    };

    D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_IMMUTABLE;
    bd.ByteWidth = sizeof( vertices );  //bd.ByteWidth = sizeof( SimpleVertex ) * 4/*the number of vector in vertices*/;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA buffInitData;
    buffInitData.pSysMem = vertices;
    HRESULT hr = m_pDevice->CreateBuffer( &bd, &buffInitData, &m_pTexReceiver );
    if( FAILED( hr ) )
        return hr;

// ---- END: Create triangles to map our texture.
    return hr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT D3DProcessing::UpdateSourceTexture(UCHAR const* pixels)
    {
    D3D10_MAPPED_TEXTURE2D mappedTex;
    HRESULT hr = m_pSrcTexture->Map( D3D10CalcSubresource(0, 0, 1), D3D10_MAP_WRITE_DISCARD, 0, &mappedTex );

    memcpy(mappedTex.pData, pixels, TILE_SIZE_BYTES); // RGBA 32 bits.
        
    m_pSrcTexture->Unmap( D3D10CalcSubresource(0, 0, 1) );
    return hr;
    }

void  D3DProcessing::PostInitSetup()
    {
    m_pShaderSrcTexture->SetResource(m_pSrcTextureShaderResView);

    // Bind the render target view and depth stencil buffer to the output render pipeline.
	m_pDevice->OMSetRenderTargets(1, &m_pRenderTargetView, NULL/*No stencil/depthBuffer*/);

    D3D10_VIEWPORT viewport;    // create a struct to hold the viewport data
    ZeroMemory(&viewport, sizeof(D3D10_VIEWPORT));    // clear out the struct for use

    viewport.TopLeftX = 0;    // set the left to 0
    viewport.TopLeftY = 0;    // set the top to 0
    viewport.Width = 256;    // set the width to the window's width
    viewport.Height = 256;    // set the height to the window's height

    m_pDevice->RSSetViewports(1, &viewport);    // set the viewport

    // Set the input layout
    m_pDevice->IASetInputLayout( m_pRenderTileLayout );

    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    m_pDevice->IASetVertexBuffers( 0, 1, &m_pTexReceiver, &stride, &offset );

    // Set primitive topology
    m_pDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void D3DProcessing::Render(UINT backgroundColorARGB, D3DXMATRIX& matrix)
    {
    m_pPixelTransformation->SetMatrix(matrix);
            
    // Clear the back buffer.
	m_pDevice->ClearRenderTargetView(m_pRenderTargetView, D3DXCOLOR(backgroundColorARGB));

    D3D10_TECHNIQUE_DESC techDesc;
    m_pRenderTileTechnique->GetDesc( &techDesc );

    for( UINT p = 0; p < techDesc.Passes; ++p )
        {
        //apply technique
        m_pRenderTileTechnique->GetPassByIndex( p )->Apply( 0 );

        //draw
        m_pDevice->Draw( 4, 0 );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT D3DProcessing::RenderToBuffer(UCHAR* pOutBuffer, UCHAR const* pSrcBuffer, UINT backgroundColorARGB, D3DXMATRIX& matrix)
    { 
    HRESULT hr = S_OK;

    UpdateSourceTexture(pSrcBuffer);

    Render(backgroundColorARGB, matrix);

   // Profiler copyResource("GPU CopyResource");

  //  copyResource.Start();

    //return S_OK;
    m_pDevice->CopyResource( m_pStagingResource, m_pRenderTarget);

    // Read result from GPU
    D3D10_MAPPED_TEXTURE2D map;
    hr = m_pStagingResource->Map( 0, D3D10_MAP_READ, NULL, &map );
    memcpy(pOutBuffer, map.pData, TILE_SIZE_BYTES);
    m_pStagingResource->Unmap( 0 );

   // copyResource.End();
   // copyResource.Print();

    return hr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT D3DProcessing::ConvertToLUV(double* pTileOut, HFCPtr<HRABitmap> pTileIn)
    {

    //UpdateSourceTexture(pSrcBuffer);


    return S_OK;
    }