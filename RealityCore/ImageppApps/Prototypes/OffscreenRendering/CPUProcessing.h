/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/OffscreenRendering/CPUProcessing.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
class CPUProcessing
    {
    public:
        CPUProcessing(void);
        ~CPUProcessing(void);


        HRESULT RenderToGray(UCHAR* pTileOut, UCHAR const* pTileIn, UINT TileSize);
        HRESULT Render(UCHAR* pTileOut, UCHAR const* pTileInt, UINT TileSize, HGF2DTransfoModel const& pixelTrans);
        HRESULT Blend(UCHAR* pTileOut, UCHAR const* pTileIn, UINT TileSize);


        HRESULT Resample(UCHAR* pTileOut, HFCPtr<HRABitmap> pTileIn, UINT backgroundColorARGB, HGF2DTransfoModel const& pixelTrans);

        HRESULT ConvertToLUV(double* pTileOut, HFCPtr<HRABitmap> pTileIn);
    };

