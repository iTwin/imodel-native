/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityPlatformUtil.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

typedef uint32_t PtHandle;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
//&&JFC TODO: Create a mechanism that will initialize the libraries only once per session.
// Init GCS
// Init Pointools
// Init IPP
// ...
struct SessionManager
    {
    static bool InitBaseGCS();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void GetBaseDirOfExecutingModule(WStringR baseDir);

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Marc.Bedard                     04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct RasterFacility
    {
    static void ConvertThePixels(size_t pi_Width, size_t pi_Height, const HFCPtr<HRPPixelType>& pi_rpSrcPixelType,
        const HFCPtr<HCDPacket>& pi_rpSrcPacket, const HFCPtr<HRPPixelType>& pi_rpDstPixelType,
        HFCPtr<HCDPacket>& po_rpDstPacket);

    static void CreateHBitmapFromHRFThumbnail(HBITMAP* pThumbnailBmp, HFCPtr<HRFThumbnail>&  pThumbnail, HFCPtr<HRPPixelType>& pPixelType);

    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE