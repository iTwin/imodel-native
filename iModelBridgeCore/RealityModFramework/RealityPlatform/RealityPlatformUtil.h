/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityPlatformUtil.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void GetBaseDirOfExecutingModule(WStringR baseDir);

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Chantal.Poulin                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
class CriticalSectionHelper
    {
    static CRITICAL_SECTION s_criticalSection;
    
    public:
        CriticalSectionHelper();
        ~CriticalSectionHelper();
        static void Init();
    };

struct RasterFacility
    {
    static void ConvertThePixels(size_t pi_Width, size_t pi_Height, const HFCPtr<HRPPixelType>& pi_rpSrcPixelType,
        const HFCPtr<HCDPacket>& pi_rpSrcPacket, const HFCPtr<HRPPixelType>& pi_rpDstPixelType,
        HFCPtr<HCDPacket>& po_rpDstPacket);

    static void CreateHBitmapFromHRFThumbnail(HBITMAP* pThumbnailBmp, HFCPtr<HRFThumbnail>&  pThumbnail, HFCPtr<HRPPixelType>& pPixelType);

    };
