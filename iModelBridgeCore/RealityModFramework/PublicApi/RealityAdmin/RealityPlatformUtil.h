/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityAdmin/RealityPlatformUtil.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include <Imagepp/h/ImageppAPI.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRPPixelTypeV32B8G8R8X8.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

typedef uint32_t PtHandle;

//=====================================================================================
//! //&&JFC TODO: Create a mechanism that will initialize the libraries only once per session.
//! Init GCS
//! Init Pointools
//! Init IPP
//! ...
//!
//! @bsiclass                                   Jean-Francois.Cote              03/2015
//=====================================================================================
struct SessionManager
    {
    static bool InitBaseGCS();
    };

//=====================================================================================
//! @bsiclass                                   Marc.Bedard                     04/2013
//=====================================================================================
struct RasterFacility
    {
    REALITYDATAPLATFORM_EXPORT static void ConvertThePixels(size_t pi_Width, 
                                                            size_t pi_Height,
                                                            const ImagePP::HFCPtr<ImagePP::HRPPixelType>& pi_rpSrcPixelType,
                                                            const ImagePP::HFCPtr<ImagePP::HCDPacket>& pi_rpSrcPacket,
                                                            const ImagePP::HFCPtr<ImagePP::HRPPixelType>& pi_rpDstPixelType,
                                                            ImagePP::HFCPtr<ImagePP::HCDPacket>& po_rpDstPacket);

    REALITYDATAPLATFORM_EXPORT static void CreateHBitmapFromHRFThumbnail(HBITMAP* pThumbnailBmp, 
                                                                         ImagePP::HFCPtr<ImagePP::HRFThumbnail>& pThumbnail, 
                                                                         ImagePP::HFCPtr<ImagePP::HRPPixelType>& pPixelType);

    REALITYDATAPLATFORM_EXPORT static ImagePP::HFCPtr<ImagePP::HRFRasterFile> GetRasterFile(Utf8CP inFilename);

    REALITYDATAPLATFORM_EXPORT static bool CreateSisterFile(Utf8CP fileName, Utf8CP coordinateSystemKeyName);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE