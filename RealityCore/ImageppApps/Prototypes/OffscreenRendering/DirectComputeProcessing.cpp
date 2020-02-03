/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/OffscreenRendering/DirectComputeProcessing.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include "DirectComputeProcessing.h"

#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11.h>

// to move as member and delete them.
ID3D11Device*               g_pDevice = NULL;
ID3D11DeviceContext*        g_pContext = NULL;
ID3D11ComputeShader*        g_pCS = NULL;

ID3D11Buffer*               g_pBuf0 = NULL;
ID3D11Buffer*               g_pBufResult = NULL;
ID3D11Buffer*               g_debugbuf = NULL;


ID3D11ShaderResourceView*   g_pBuf0SRV = NULL;
ID3D11UnorderedAccessView*  g_pBufResultUAV = NULL;


//--------------------------------------------------------------------------------------
// Create the D3D device and device context suitable for running Compute Shaders(CS)
//--------------------------------------------------------------------------------------
HRESULT CreateComputeDevice( ID3D11Device** ppDeviceOut, ID3D11DeviceContext** ppContextOut, BOOL bForceRef )
{    
    *ppDeviceOut = NULL;
    *ppContextOut = NULL;
    
    HRESULT hr = S_OK;

    UINT uCreationFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined(DEBUG) || defined(_DEBUG)
    uCreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL flOut;
    static const D3D_FEATURE_LEVEL flvl[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
    
    BOOL bNeedRefDevice = FALSE;
    if ( !bForceRef )
    {
        hr = D3D11CreateDevice( NULL,                        // Use default graphics card
                                        D3D_DRIVER_TYPE_HARDWARE,    // Try to create a hardware accelerated device
                                        NULL,                        // Do not use external software rasterizer module
                                        uCreationFlags,              // Device creation flags
                                        flvl,
                                        sizeof(flvl) / sizeof(D3D_FEATURE_LEVEL),
                                        D3D11_SDK_VERSION,           // SDK version
                                        ppDeviceOut,                 // Device out
                                        &flOut,                      // Actual feature level created
                                        ppContextOut );              // Context out
        
        if ( SUCCEEDED( hr ) )
        {
            // A hardware accelerated device has been created, so check for Compute Shader support

            // If we have a device >= D3D_FEATURE_LEVEL_11_0 created, full CS5.0 support is guaranteed, no need for further checks
            if ( flOut < D3D_FEATURE_LEVEL_11_0 )            
            {
#ifdef TEST_DOUBLE
                bNeedRefDevice = TRUE;
                printf( "No hardware Compute Shader 5.0 capable device found (required for doubles), trying to create ref device.\n" );
#else
                // Otherwise, we need further check whether this device support CS4.x (Compute on 10)
                D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hwopts;
                (*ppDeviceOut)->CheckFeatureSupport( D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &hwopts, sizeof(hwopts) );
                if ( !hwopts.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x )
                {
                    bNeedRefDevice = TRUE;
                    printf( "No hardware Compute Shader capable device found, trying to create ref device.\n" );
                }
#endif
            }

#ifdef TEST_DOUBLE
            else
            {
                // Double-precision support is an optional feature of CS 5.0
                D3D11_FEATURE_DATA_DOUBLES hwopts;
                (*ppDeviceOut)->CheckFeatureSupport( D3D11_FEATURE_DOUBLES, &hwopts, sizeof(hwopts) );
                if ( !hwopts.DoublePrecisionFloatShaderOps )
                {
                    bNeedRefDevice = TRUE;
                    printf( "No hardware double-precision capable device found, trying to create ref device.\n" );
                }
            }
#endif
        }
    }
    
    if ( bForceRef || FAILED(hr) || bNeedRefDevice )
    {
        // Either because of failure on creating a hardware device or hardware lacking CS capability, we create a ref device here

        SAFE_RELEASE( *ppDeviceOut );
        SAFE_RELEASE( *ppContextOut );
        
        hr = D3D11CreateDevice( NULL,                        // Use default graphics card
                                        D3D_DRIVER_TYPE_REFERENCE,   // Try to create a hardware accelerated device
                                        NULL,                        // Do not use external software rasterizer module
                                        uCreationFlags,              // Device creation flags
                                        flvl,
                                        sizeof(flvl) / sizeof(D3D_FEATURE_LEVEL),
                                        D3D11_SDK_VERSION,           // SDK version
                                        ppDeviceOut,                 // Device out
                                        &flOut,                      // Actual feature level created
                                        ppContextOut );              // Context out
        if ( FAILED(hr) )
        {
            printf( "Reference rasterizer device create failure\n" );
            return hr;
        }
    }

    return hr;
}

//--------------------------------------------------------------------------------------
// Compile and create the CS
//--------------------------------------------------------------------------------------
HRESULT CreateComputeShader( LPCWSTR pSrcFile, LPCSTR pFunctionName, 
                             ID3D11Device* pDevice, ID3D11ComputeShader** ppShaderOut )
{
    HRESULT hr;

    // Finds the correct path for the shader file.
    // This is only required for this sample to be run correctly from within the Sample Browser,
    // in your own projects, these lines could be removed safely
    WCHAR str[MAX_PATH] = L"myFile";        //todo
    wcscpy(str, pSrcFile);
//     hr = FindDXSDKShaderFileCch( str, MAX_PATH, pSrcFile );
//     if ( FAILED(hr) )
//         return hr;
    
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    const D3D_SHADER_MACRO defines[] = 
    {
#ifdef USE_STRUCTURED_BUFFERS
        "USE_STRUCTURED_BUFFERS", "1",
#endif

#ifdef TEST_DOUBLE
        "TEST_DOUBLE", "1",
#endif
        NULL, NULL
    };

    // We generally prefer to use the higher CS shader profile when possible as CS 5.0 is better performance on 11-class hardware
    LPCSTR pProfile = ( pDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0 ) ? "cs_5_0" : "cs_4_0";

    ID3DBlob* pErrorBlob = NULL;
    ID3DBlob* pBlob = NULL;
    hr = D3DX11CompileFromFile( str, defines, NULL, pFunctionName, pProfile, 
        dwShaderFlags, NULL, NULL, &pBlob, &pErrorBlob, NULL );
    if ( FAILED(hr) )
    {
        if ( pErrorBlob )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );

        SAFE_RELEASE( pErrorBlob );
        SAFE_RELEASE( pBlob );    

        return hr;
    }    

    hr = pDevice->CreateComputeShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, ppShaderOut );

#if defined(DEBUG) || defined(PROFILE)
    if ( *ppShaderOut )
        (*ppShaderOut)->SetPrivateData( WKPDID_D3DDebugObjectName, lstrlenA(pFunctionName), pFunctionName );
#endif

    SAFE_RELEASE( pErrorBlob );
    SAFE_RELEASE( pBlob );

    return hr;
}

//--------------------------------------------------------------------------------------
// Create Structured Buffer
//--------------------------------------------------------------------------------------
HRESULT CreateStructuredBuffer( ID3D11Device* pDevice, D3D11_USAGE usage, UINT cpuFlag, UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut )
{
    *ppBufOut = NULL;

    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    desc.ByteWidth = uElementSize * uCount;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    //desc.Usage = usage;
    //desc.CPUAccessFlags = cpuFlag;
    desc.StructureByteStride = uElementSize;

    if ( pInitData )
    {
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = pInitData;
        return pDevice->CreateBuffer( &desc, &InitData, ppBufOut );
    } else
        return pDevice->CreateBuffer( &desc, NULL, ppBufOut );
}

//--------------------------------------------------------------------------------------
// Create Shader Resource View for Structured or Raw Buffers
//--------------------------------------------------------------------------------------
HRESULT CreateBufferSRV( ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut )
{
    D3D11_BUFFER_DESC descBuf;
    ZeroMemory( &descBuf, sizeof(descBuf) );
    pBuffer->GetDesc( &descBuf );

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    desc.BufferEx.FirstElement = 0;

    if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS )
    {
        // This is a Raw Buffer
        desc.Format = DXGI_FORMAT_R32_TYPELESS;
        desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
        desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
    } else
    if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED )
    {
        // This is a Structured Buffer
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
    } else
    {
        return E_INVALIDARG;
    }

    return pDevice->CreateShaderResourceView( pBuffer, &desc, ppSRVOut );
}

//--------------------------------------------------------------------------------------
// Create Unordered Access View for Structured or Raw Buffers
//-------------------------------------------------------------------------------------- 
HRESULT CreateBufferUAV( ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut )
{
    D3D11_BUFFER_DESC descBuf;
    ZeroMemory( &descBuf, sizeof(descBuf) );
    pBuffer->GetDesc( &descBuf );
        
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;

    if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS )
    {
        // This is a Raw Buffer

        desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
        desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
        desc.Buffer.NumElements = descBuf.ByteWidth / 4; 
    } else
    if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED )
    {
        // This is a Structured Buffer

        desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
        desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride; 
    } else
    {
        return E_INVALIDARG;
    }
    
    return pDevice->CreateUnorderedAccessView( pBuffer, &desc, ppUAVOut );
}

//--------------------------------------------------------------------------------------
// Create a CPU accessible buffer and download the content of a GPU buffer into it
// This function is very useful for debugging CS programs
//-------------------------------------------------------------------------------------- 
ID3D11Buffer* CreateCopyBuf( ID3D11Device* pDevice, ID3D11Buffer* pBuffer )
{
    ID3D11Buffer* debugbuf = NULL;

    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    pBuffer->GetDesc( &desc );
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
    if ( SUCCEEDED(pDevice->CreateBuffer(&desc, NULL, &debugbuf)) )
    {
#if defined(DEBUG) || defined(PROFILE)
        debugbuf->SetPrivateData( WKPDID_D3DDebugObjectName, sizeof( "Debug" ) - 1, "Debug" );
#endif
    }

    return debugbuf;
}

DirectComputeProcessing::DirectComputeProcessing(void)
    {
    

    }


DirectComputeProcessing::~DirectComputeProcessing(void)
    {
    }

bool DirectComputeProcessing::Init()
    {
    printf( "Creating device..." );
    if ( FAILED( CreateComputeDevice( &g_pDevice, &g_pContext, FALSE ) ) )
        {
        printf( "Error creating device" );
        return false;
        }    
    printf( "done\n" );

    printf( "Creating Compute Shader..." );
    CreateComputeShader( L"BasicCompute11.hlsl", "CSMain", g_pDevice, &g_pCS );
    printf( "done\n" );

    CreateStructuredBuffer( g_pDevice, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, sizeof(uint32_t), TILE_SIZE*TILE_SIZE, NULL, &g_pBuf0 );
    CreateStructuredBuffer( g_pDevice, D3D11_USAGE_DEFAULT, 0x00, sizeof(uint32_t), TILE_SIZE*TILE_SIZE, NULL, &g_pBufResult );

    g_debugbuf = CreateCopyBuf(g_pDevice, g_pBufResult);

    CreateBufferSRV( g_pDevice, g_pBuf0, &g_pBuf0SRV );

    CreateBufferUAV( g_pDevice, g_pBufResult, &g_pBufResultUAV );

    return true;
    }


HRESULT DirectComputeProcessing::RenderToBuffer(UCHAR* pOutBuffer, UCHAR const* pSrcBuffer, UINT backgroundColorARGB, D3DXMATRIX& matrix)
    {
//     void RunComputeShader( ID3D11DeviceContext* pd3dImmediateContext,
//                       ID3D11ComputeShader* pComputeShader,
//                       UINT nNumViews, ID3D11ShaderResourceView** pShaderResourceViews, 
//                       ID3D11Buffer* pCBCS, void* pCSData, DWORD dwNumDataBytes,
//                       ID3D11UnorderedAccessView* pUnorderedAccessView,
//                       UINT X, UINT Y, UINT Z )

    g_pContext->CSSetShader( g_pCS, NULL, 0 );
    g_pContext->CSSetShaderResources( 0, 1, &g_pBuf0SRV );
    g_pContext->CSSetUnorderedAccessViews( 0, 1, &g_pBufResultUAV, NULL );
   
    // Copy our source pixels
        {
        g_pContext->UpdateSubresource( g_pBuf0, 0, NULL, pSrcBuffer, 0, 0 );
//         D3D11_MAPPED_SUBRESOURCE MappedResource;
//         HRESULT result = g_pContext->Map( g_pBuf0, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
//         memcpy( MappedResource.pData, pSrcBuffer, TILE_SIZE_BYTES );
//         g_pContext->Unmap( g_pBuf0, 0 );
        ID3D11Buffer* ppCB[1] = { g_pBuf0 };
        g_pContext->CSSetConstantBuffers( 0, 1, ppCB );
        }

    g_pContext->Dispatch( TILE_SIZE, TILE_SIZE, 1 );

 //   g_pContext->CSSetShader( NULL, NULL, 0 );

//     ID3D11UnorderedAccessView* ppUAViewNULL[1] = { NULL };
//     g_pContext->CSSetUnorderedAccessViews( 0, 1, ppUAViewNULL, NULL );
// 
//     ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL, NULL };
//     g_pContext->CSSetShaderResources( 0, 2, ppSRVNULL );
// 
//     ID3D11Buffer* ppCBNULL[1] = { NULL };
//     g_pContext->CSSetConstantBuffers( 0, 1, ppCBNULL );

    // Read back result
    g_pContext->CopyResource(g_debugbuf, g_pBufResult);

    D3D11_MAPPED_SUBRESOURCE MappedResource; 
    g_pContext->Map( g_debugbuf, 0, D3D11_MAP_READ, 0, &MappedResource );

    // Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
    // This is also a common trick to debug CS programs.
    memcpy(pOutBuffer, MappedResource.pData, TILE_SIZE_BYTES);

    g_pContext->Unmap( g_debugbuf, 0 );


    return S_OK;
    }
