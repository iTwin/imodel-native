/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/OffscreenRendering/OffscreenRendering.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// OffscreenRendering.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "CPUProcessing.h"
#include "D3DProcessing.h"
#include "ImageFile.h"
#include "DirectComputeProcessing.h"


#define ENABLE_PROFILER
#include "Profiler.h"

HFCPtr<HGF2DWorldCluster> gWorldClusterP(new HGFHMRStdWorldCluster());

//--------------------------------------------------------------------------------------
void PrintTestHeader( const char* TestName )
{
    printf("--------------------------------------------------------------------\n");
    printf("[%s] \n", TestName);
}

//--------------------------------------------------------------------------------------
void PrintError( const char* errortext )
{
    printf( errortext );
    printf( "\nPress any key to exit.\n" );
    getchar();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void TestRGBtoLUV(HFCPtr<HRABitmap> pTile, D3DProcessing& myD3DContext, CPUProcessing& cpuProcessor, int loopCount)
    {
    assert(pTile->GetPixelType()->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID));
    assert(pTile->GetPacket()->GetDataSize() == TILE_SIZE_BYTES); // a RGBA tile.

    double* pOutBufferCPU = new double[TILE_SIZE*TILE_SIZE*3];  // L, U, V
    double* pOutBufferGPU = new double[TILE_SIZE*TILE_SIZE*3];  // L, U, V
    
    Profiler gpuProfile("GPU");
    Profiler cpuProfile("CPU");

    D3DXMATRIX pixelTransformation;
	D3DXMatrixIdentity(&pixelTransformation);	

    bool atLeastOneBadResut = false;
    for(int i=0; i < loopCount; ++i)
        {           
        // GPU
            {
//             gpuProfile.Start();
//             myD3DContext.RenderToBuffer(pOutBufferGPU, pTile->GetPacket()->GetBufferAddress(), 0x00, pixelTransformation);
//             gpuProfile.End();
            }

        //CPU
            {
            memset(pOutBufferCPU, 0, TILE_SIZE*TILE_SIZE*3);
            cpuProfile.Start();
            cpuProcessor.ConvertToLUV(pOutBufferCPU, pTile);
            cpuProfile.End();
            }

//         Validate result between CPU and GPU
//                     for(int pixel=0; pixel < TILE_SIZE*TILE_SIZE; ++pixel)
//                         {
//                         // CPU trunc and GPU round so a difference of 1 is allowed
//                         if(abs(pOutBufferCPU[pixel* 4 + 0] - pOutBufferGPU[pixel* 4 + 0]) > 1 ||
//                            abs(pOutBufferCPU[pixel* 4 + 1] - pOutBufferGPU[pixel* 4 + 1]) > 1 || 
//                            abs(pOutBufferCPU[pixel* 4 + 2] - pOutBufferGPU[pixel* 4 + 2]) > 1 ||
//                            abs(pOutBufferCPU[pixel* 4 + 3] - pOutBufferGPU[pixel* 4 + 3]) > 1)
//                             {
//                             atLeastOneBadResut = true;
//                             break;
//                             }
//                         }                     
        }

    gpuProfile.Print();
    cpuProfile.Print();
    if(atLeastOneBadResut)
        printf("At least one difference between CPU and GPU\n");
    

    delete [] pOutBufferGPU;
    delete [] pOutBufferCPU;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void TestResample(HFCPtr<HRABitmap> pTile, D3DProcessing& d3dProcessing, DirectComputeProcessing& dcProcessing, CPUProcessing& cpuProcessing, int loopCount, UINT backgroundColorARGB)
    {
    assert(pTile->GetPixelType()->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID));
    assert(pTile->GetPacket()->GetDataSize() == TILE_SIZE_BYTES); // a RGBA tile.

    UCHAR* pOutBufferGPU = new UCHAR[TILE_SIZE_BYTES];
    UCHAR* pOutBufferGPUDirectCompute = new UCHAR[TILE_SIZE_BYTES];
    UCHAR* pOutBufferCPU = new UCHAR[TILE_SIZE_BYTES];

    Profiler gpuProfile("GPU");
    Profiler dcProfile("DirectCompute");
    Profiler cpuProfile("CPU");

    D3DXMATRIX pixelTransformation;
	D3DXMatrixIdentity(&pixelTransformation);	
    D3DXMatrixRotationZ(&pixelTransformation, (float)D3DXToRadian(45.0));

    HGF2DDisplacement MyDisp(-127.5, -127.5);
    HFCPtr<HGF2DTranslation> pTranslation = new HGF2DTranslation(MyDisp);

    double TheAngle((float)D3DXToRadian(45.0));
    
    double Anortho(0.0);
    HFCPtr<HGF2DAffine> pRotation = new HGF2DAffine(HGF2DDisplacement(1.0, 1.0), TheAngle, 1.0, 1.0, Anortho);

    HFCPtr<HGF2DTransfoModel> pTransfo = pTranslation->ComposeInverseWithDirectOf(*pRotation);
    pTransfo = pTransfo->ComposeInverseWithInverseOf(*pTranslation);

    bool atLeastOneBadResut = false;
    for(int i=0; i < loopCount; ++i)
        {           
        // GPU
            {
            gpuProfile.Start();
            d3dProcessing.RenderToBuffer(pOutBufferGPU, pTile->GetPacket()->GetBufferAddress(), backgroundColorARGB, pixelTransformation);
            gpuProfile.End();
            }

         //DirectCompute
            {
            dcProfile.Start();
            dcProcessing.RenderToBuffer(pOutBufferGPUDirectCompute, pTile->GetPacket()->GetBufferAddress(), backgroundColorARGB, pixelTransformation);
            dcProfile.End();
            }

        //CPU
            {
            cpuProfile.Start();
            cpuProcessing.Resample(pOutBufferCPU, pTile, backgroundColorARGB, *pTransfo);
            cpuProfile.End();
            }


        if(0 == i)
            {
            ImageFile::CreateTiffFromRGBATile(pTile->GetPacket()->GetBufferAddress(), L"!input_256x256_RGBA.tif");

            ImageFile::CreateTiffFromRGBATile(pOutBufferGPU, L"!GPU_256x256_RGBA.tif");

            ImageFile::CreateTiffFromRGBATile(pOutBufferCPU, L"!CPU_256x256_RGBA.tif");

            ImageFile::CreateTiffFromRGBATile(pOutBufferGPUDirectCompute, L"!DirectCompute_256x256_RGBA.tif");
            }

        // Validate result between CPU and GPU
        for(int pixel=0; pixel < TILE_SIZE*TILE_SIZE; ++pixel)
            {
            // CPU trunc and GPU round so a difference of 1 is allowed
            if(abs(pOutBufferCPU[pixel* 4 + 0] - pOutBufferGPU[pixel* 4 + 0]) > 1 ||
               abs(pOutBufferCPU[pixel* 4 + 1] - pOutBufferGPU[pixel* 4 + 1]) > 1 || 
               abs(pOutBufferCPU[pixel* 4 + 2] - pOutBufferGPU[pixel* 4 + 2]) > 1 ||
               abs(pOutBufferCPU[pixel* 4 + 3] - pOutBufferGPU[pixel* 4 + 3]) > 1)
                {
                atLeastOneBadResut = true;
                break;
                }
            }                     
        }

    gpuProfile.Print();
    dcProfile.Print();
    cpuProfile.Print();
    if(atLeastOneBadResut)
        printf("At least one difference between CPU and GPU\n");
    
    delete [] pOutBufferGPU;
    delete [] pOutBufferCPU;
    delete [] pOutBufferGPUDirectCompute;
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void TestConvertToGray(HFCPtr<HRABitmap> pTile, D3DProcessing& d3dProcessing, CPUProcessing& cpuProcessing, int loopCount, UINT backgroundColorARGB)
    {
    assert(pTile->GetPixelType()->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID));
    assert(pTile->GetPacket()->GetDataSize() == TILE_SIZE_BYTES); // a RGBA tile.

    UCHAR* pOutBufferGPU = new UCHAR[TILE_SIZE_BYTES];
    UCHAR* pOutBufferCPU = new UCHAR[TILE_SIZE_BYTES];
    UCHAR* pOutBufferCPUIntermediate = new UCHAR[TILE_SIZE_BYTES];

//     float srcC = 47.0f / 255.0f;
//     float srcA = 132.0f / 255.0f;
// 
//     float dstC = 127.0f / 255.0f;
//     float dstA = 127.0f / 255.0f;
// 
//     //http://en.wikipedia.org/wiki/Alpha_compositing Porter and Duff 
//     float newA = srcA + dstA * (1 - srcA);
//     float newC = (srcC * srcA + dstC * dstA * (1 - srcA)) / newA;
// 
//     // IPP
//     // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
//     float newC_ipp = srcA * (srcC - (dstA * dstC)) + (dstA * dstC);
//     // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
//     float newA_ipp = 1 - ( (1 - srcA) * (1 - dstA) ) ;

    Profiler gpuProfile("GPU");
    Profiler cpuProfile("CPU");

    D3DXMATRIX pixelTransformation;
	D3DXMatrixIdentity(&pixelTransformation);	
    D3DXMatrixRotationZ(&pixelTransformation, (float)D3DXToRadian(45.0));

    HGF2DDisplacement MyDisp(-127.5, -127.5);
    HFCPtr<HGF2DTranslation> pTranslation = new HGF2DTranslation(MyDisp);

    double TheAngle((float)D3DXToRadian(315.0));
    
    double Anortho(0.0);
    HFCPtr<HGF2DAffine> pRotation = new HGF2DAffine(HGF2DDisplacement(1.0, 1.0), TheAngle, 1.0, 1.0, Anortho);

    HFCPtr<HGF2DTransfoModel> pTransfo = pTranslation->ComposeInverseWithDirectOf(*pRotation);
    pTransfo = pTransfo->ComposeInverseWithInverseOf(*pTranslation);

    bool atLeastOneBadResut = false;
    for(int i=0; i < loopCount; ++i)
        {
//         if(!useSourceFile)
//             {
//             //         int seedR = rand();
//     //         int seedG = rand();
//     //         int seedB = rand();
//     //         int seedA = rand();
//             int seedR, seedG, seedB, seedA;
//             seedR = seedG = seedB = 47;
//             seedA = 132;
//             // Fill in src buffer.
//             for(int pixel=0; pixel < TILE_SIZE*TILE_SIZE; ++pixel)
//                 {
//                 pInBuffer[pixel* 4 + 0] = (i + seedR) % 256;
//                 pInBuffer[pixel* 4 + 1] = (i + seedG) % 256;
//                 pInBuffer[pixel* 4 + 2] = (i + seedB) % 256;
//                 pInBuffer[pixel* 4 + 3] = (i + seedA) % 256;
//                 }
//             }
            
        // GPU
            {
            gpuProfile.Start();
            d3dProcessing.RenderToBuffer(pOutBufferGPU, pTile->GetPacket()->GetBufferAddress(), backgroundColorARGB, pixelTransformation);
            gpuProfile.End();
            }

        //CPU
            {
            cpuProfile.Start();

            for(int i=0; i < TILE_SIZE*TILE_SIZE; ++i)
                ((ULONG*)pOutBufferCPU)[i] = backgroundColorARGB;
                        
            cpuProcessing.Render(pOutBufferCPUIntermediate, pTile->GetPacket()->GetBufferAddress(), TILE_SIZE, *pTransfo);
            cpuProcessing.Blend(pOutBufferCPU, pOutBufferCPUIntermediate, TILE_SIZE);
            cpuProfile.End();
            }

        if(0 == i)
            {
            ImageFile::CreateTiffFromRGBATile(pTile->GetPacket()->GetBufferAddress(), L"!input_256x256_RGBA.tif");

            ImageFile::CreateTiffFromRGBATile(pOutBufferGPU, L"!GPU_256x256_RGBA.tif");

            ImageFile::CreateTiffFromRGBATile(pOutBufferCPU, L"!CPU_256x256_RGBA.tif");
            }

        // Validate result between CPU and GPU
        for(int pixel=0; pixel < TILE_SIZE*TILE_SIZE; ++pixel)
            {
            // CPU trunc and GPU round so a difference of 1 is allowed
            if(abs(pOutBufferCPU[pixel* 4 + 0] - pOutBufferGPU[pixel* 4 + 0]) > 1 ||
               abs(pOutBufferCPU[pixel* 4 + 1] - pOutBufferGPU[pixel* 4 + 1]) > 1 || 
               abs(pOutBufferCPU[pixel* 4 + 2] - pOutBufferGPU[pixel* 4 + 2]) > 1 ||
               abs(pOutBufferCPU[pixel* 4 + 3] - pOutBufferGPU[pixel* 4 + 3]) > 1)
                {
                atLeastOneBadResut = true;
                break;
                }
            }                     
        }

    gpuProfile.Print();
    cpuProfile.Print();
    if(atLeastOneBadResut)
        printf("At least one difference between CPU and GPU\n");
    
    delete [] pOutBufferGPU;
    delete [] pOutBufferCPU;
    delete [] pOutBufferCPUIntermediate;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int _tmain(int argc, _TCHAR* argv[])
{
    ID3D10Device* pDevice = NULL;
    DWORD dwCreateFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        dwCreateFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif
    HRESULT hr;
    hr = D3D10CreateDevice( NULL, D3D10_DRIVER_TYPE_HARDWARE, ( HMODULE )0, dwCreateFlags, D3D10_SDK_VERSION, &pDevice );
    if( FAILED( hr ) )
        {
        hr = D3D10CreateDevice( NULL, D3D10_DRIVER_TYPE_REFERENCE, ( HMODULE )0, dwCreateFlags, D3D10_SDK_VERSION, &pDevice );
        if( FAILED( hr ) )
            {
            PrintError( "A suitable D3D10 device could not be created.\n" );
            return 1;
            }
        }

    wstring filename(L"C:\\images\\iTIFF\\24bit\\IMG0035.iTIFF");

    ImageFile imageFile(filename);

    if(!imageFile.IsValid())
        {
        printf("Could not load %s", filename.c_str());
        PrintError("\n" );
        return 1;
        }

    // Read first tile

    HGF2DStretch Stretch(HGF2DDisplacement(0.0, 0.0), 
                             1.0, 
                             1.0);

    HFCPtr<HRABitmap> pTile = new HRABitmap(256, 256, &Stretch, imageFile.GetRaster()->GetPhysicalCoordSys(), new HRPPixelTypeV32R8G8B8A8());

    HRACopyFromOptions copyFromOpts;
    pTile->CopyFrom(imageFile.GetRaster().GetPtr(), copyFromOpts); 

    D3DProcessing myD3DContext(pDevice);

    // Look at this example on how to setup thing per stage: dxManager::initialize(dxManager.cpp)
    // http://takinginitiative.net/2010/04/09/directx-10-tutorial-6-transparency-and-alpha-blending/
    myD3DContext.LoadEffectFile(L"OffscreenRenderingEffect.fx");
    myD3DContext.CreateTechnique();
    myD3DContext.CreateRenderTargetAndStagingResource();
    myD3DContext.CreateSourceTexture();
    myD3DContext.CreateTextureReceiver();
    myD3DContext.PostInitSetup();

    CPUProcessing cpuProcessor;
    DirectComputeProcessing dcProcessor;
    dcProcessor.Init();

    for(int i=0; i < 10; ++i)
        {
        PrintTestHeader("Initial run to create static stuff on GPU");
        TestResample(pTile, myD3DContext, dcProcessor, cpuProcessor, 1, 0xff0000ff/*0x7f7f7f7f*//*grayBackground*/); // semi-transparency gray(127) will trigger blending.
        }

    PrintTestHeader("TestResample with semi transparent gray background");
    TestResample(pTile, myD3DContext, dcProcessor, cpuProcessor, 1, 0x000000ff/*0x7f7f7f7f*//*grayBackground*/); // semi-transparency gray(127) will trigger blending.
// 
//     PrintTestHeader("TestResample with semi transparent gray background");
//     TestResample(pTile, myD3DContext, cpuProcessor, 10, 0x000000ff/*0x7f7f7f7f*//*grayBackground*/); // semi-transparency gray(127) will trigger blending.
// 
//     PrintTestHeader("TestResample with semi transparent gray background");
//     TestResample(pTile, myD3DContext, cpuProcessor, 10, 0x000000ff/*0x7f7f7f7f*//*grayBackground*/); // semi-transparency gray(127) will trigger blending.
// 
//     PrintTestHeader("TestRGBtoLUV");
//     TestRGBtoLUV(pTile, myD3DContext, cpuProcessor, 2);


    printf( "\nPress any key to exit.\n" );
    getchar();
	return 0;
}

