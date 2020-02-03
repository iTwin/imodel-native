/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/OffscreenRendering/CPUProcessing.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include "CPUProcessing.h"

// static const double  REDFACTOR   = 0.2125;
// static const double  GREENFACTOR = 0.7154;
// static const double  BLUEFACTOR  = 0.0721;



CPUProcessing::CPUProcessing(void)
    {
    }


CPUProcessing::~CPUProcessing(void)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT CPUProcessing::Render(UCHAR* pTileOut, UCHAR const* pTileIn, UINT TileSize, HGF2DTransfoModel const& pixelTrans)
    {
    UINT rowPitch = TileSize*4;

    for( UINT row = 0; row < TileSize; ++row)
        {
        UINT rowStart = row * rowPitch;
        for( UINT col = 0; col < TileSize; ++col )
            {
            UINT colStart = rowStart + col * 4;

            double xPixel = col + 0.5;
            double yPixel = row + 0.5;

            pixelTrans.ConvertInverse(&xPixel, &yPixel);

            UINT pixelPosX = (UINT)xPixel;   // rnd?
            UINT pixelPosY = (UINT)yPixel;   // rnd?
            if(IN_RANGE(pixelPosX, 0, 255) && 
               IN_RANGE(pixelPosY, 0, 255))
                {
                UINT srcRowStart = pixelPosY * rowPitch;
                UINT srcColStart = srcRowStart + pixelPosX * 4;

                pTileOut[colStart + 0] = pTileIn[srcColStart + 0];
                pTileOut[colStart + 1] = pTileIn[srcColStart + 1];
                pTileOut[colStart + 2] = pTileIn[srcColStart + 2];
                pTileOut[colStart + 3] = 0xff; 
                }
            else
                {
                pTileOut[colStart + 0] = 0x0; // Red
                pTileOut[colStart + 1] = 0xff; // Green
                pTileOut[colStart + 2] = 0xff;  // Blue
                pTileOut[colStart + 3] = 0xff;  // Alpha
                }
            }
        }

    return S_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT CPUProcessing::RenderToGray(UCHAR* pTileOut, UCHAR const* pTileIn, UINT TileSize)
    {
    UINT rowPitch = TileSize*4;

    for( UINT row = 0; row < TileSize; ++row)
        {
        UINT rowStart = row * rowPitch;
        for( UINT col = 0; col < TileSize; ++col )
            {
            UINT colStart = rowStart + col * 4;
            UCHAR gray = (UCHAR)(pTileIn[colStart + 0] * REDFACTOR +
                                 pTileIn[colStart + 1] * GREENFACTOR +
                                 pTileIn[colStart + 2] * BLUEFACTOR);

                pTileOut[colStart + 0] = gray; // Red
                pTileOut[colStart + 1] = gray; // Green
                pTileOut[colStart + 2] = gray;  // Blue
                pTileOut[colStart + 3] = pTileIn[colStart + 3];  // Alpha

            }
        }

    return S_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT CPUProcessing::Blend(UCHAR* pTileOut, UCHAR const* pTileIn, UINT TileSize)
    {
    ULONG pi_PixelsCount = TileSize*TileSize;

    Byte const* pSourceComposite =  (Byte*)pTileIn;
    Byte* pDestComposite = (Byte*)pTileOut;
    Byte PremultDestColor;

    HFCMath*pQuotients = HFCMath::GetInstance();

    // Copy entire bytes
    while(pi_PixelsCount)
        {
        // If source pixel is fully transparent, destination is unaltered
        if (pSourceComposite[3] != 0)
            {
            if (pDestComposite[3] == 0 ||
                pSourceComposite[3] == 255)
                {
                // Destination pixel is fully transparent, or source pixel
                // is fully opaque. Copy source pixel,
                *((uint32_t*)pDestComposite) = *((uint32_t*)pSourceComposite);
                }
            else
                {
                // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                PremultDestColor = (Byte)pQuotients->DivideBy255(pDestComposite[3] * pDestComposite[0]);
                pDestComposite[0] = pQuotients->DivideBy255(pSourceComposite[3] * (pSourceComposite[0] - PremultDestColor)) + PremultDestColor;

                PremultDestColor = (Byte)pQuotients->DivideBy255(pDestComposite[3] * pDestComposite[1]);
                pDestComposite[1] = pQuotients->DivideBy255(pSourceComposite[3] * (pSourceComposite[1] - PremultDestColor)) + PremultDestColor;

                PremultDestColor = (Byte)pQuotients->DivideBy255(pDestComposite[3] * pDestComposite[2]);
                pDestComposite[2] = pQuotients->DivideBy255(pSourceComposite[3] * (pSourceComposite[2] - PremultDestColor)) + PremultDestColor;

                // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                // --> Transparency percentages are multiplied
                pDestComposite[3] = 255 - (pQuotients->DivideBy255((255 - pSourceComposite[3]) * (255 - pDestComposite[3])));
                }
            }

        --pi_PixelsCount;
        pDestComposite += 4;
        pSourceComposite += 4;
        }
            
    return S_OK;
    }

                
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT CPUProcessing::Resample(UCHAR* pTileOut, HFCPtr<HRABitmap> pTileIn, UINT backgroundColorARGB, HGF2DTransfoModel const& pixelTrans)
    {
    HFCPtr<HRABitmap> pOutBitmap = new HRABitmap(TILE_SIZE, TILE_SIZE, &pixelTrans, pTileIn->GetPhysicalCoordSys(), new HRPPixelTypeV32R8G8B8A8());

    HFCPtr<HCDPacket> pPacket = new HCDPacket(new HCDCodecIdentity(), pTileOut, TILE_SIZE_BYTES, TILE_SIZE_BYTES);
    pPacket->SetBufferOwnership(false);
    pOutBitmap->SetPacket(pPacket);

    HRAClearOptions clearOpt;
    clearOpt.SetRawDataValue(&backgroundColorARGB);
    pOutBitmap->Clear(clearOpt);

    HRACopyFromOptions copyOpts;
    pOutBitmap->CopyFrom(pTileIn.GetPtr());

    return S_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT CPUProcessing::ConvertToLUV(double* pTileOut, HFCPtr<HRABitmap> pTileIn)
    {
    Byte* pRBuffer = new Byte[TILE_SIZE*TILE_SIZE];
    Byte* pGBuffer = new Byte[TILE_SIZE*TILE_SIZE];
    Byte* pBBuffer = new Byte[TILE_SIZE*TILE_SIZE];

    Byte const* pInBuffer = pTileIn->GetPacket()->GetBufferAddress();
    for(int i=0; i < TILE_SIZE*TILE_SIZE; ++i)
        {
        pRBuffer[i + 0] = pInBuffer[i*4 + 0];
        pGBuffer[i + 0] = pInBuffer[i*4 + 1];
        pBBuffer[i + 0] = pInBuffer[i*4 + 2];
        }

    HGFLuvColorSpace converter;

    size_t Offset = TILE_SIZE*TILE_SIZE;
    converter.ConvertArrayFromRGB(pRBuffer, pGBuffer, pBBuffer, pTileOut, pTileOut+Offset, pTileOut+Offset+Offset, TILE_SIZE*TILE_SIZE);

    delete [] pRBuffer;
    delete [] pGBuffer;
    delete [] pBBuffer;
    return S_OK;
    }