/*--------------------------------------------------------------------------------------+
|
|     $Source: Thumbnails/ThumbnailUtil.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <Imagepp/h/hstdcpp.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFSLOStripAdapter.h>
#include <Imagepp/all/h/HRFThumbnail.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFPageFileFactory.h>
#include <Imagepp/all/h/HRFRasterFilePageDecorator.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRFBmpFile.h>
#include <Imagepp/all/h/HRFCalsFile.h>
#include <Imagepp/all/h/HRFGeoTiffFile.h>
#include <Imagepp/all/h/HRFHMRFile.h>
#include <Imagepp/all/h/HRFImgRGBFile.h>
#include <Imagepp/all/h/HRFIntergraphCITFile.h>
#include <Imagepp/all/h/HRFIntergraphCOT29File.h>
#include <Imagepp/all/h/HRFIntergraphCotFile.h>
#include <Imagepp/all/h/HRFIntergraphRGBFile.h>
#include <Imagepp/all/h/HRFIntergraphRLEFile.h>
#include <Imagepp/all/h/HRFIntergraphTG4File.h>
#include <Imagepp/all/h/HRFIntergraphC30File.h>
#include <Imagepp/all/h/HRFIntergraphC31File.h>
#include <Imagepp/all/h/HRFiTiffFile.h>
#include <Imagepp/all/h/HRFJpegFile.h>
#include <Imagepp/all/h/HRFPngFile.h>
#include <Imagepp/all/h/HRFTiffFile.h>
#include <Imagepp/all/h/HRFGifFile.h>
#include <Imagepp/all/h/HRFRLCFile.h>
#include <Imagepp/all/h/HRFImgMappedFile.h>
#include <Imagepp/all/h/HRFcTiffFile.h>
#include <Imagepp/all/h/HRFTgaFile.h>
#include <Imagepp/all/h/HRFPcxFile.h>
#include <Imagepp/all/h/HRFSunRasterFile.h>
#include <Imagepp/all/h/HRFTiffIntgrFile.h>
#include <Imagepp/all/h/HRFBilFile.h>
         
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRPPixelTypeV32B8G8R8X8.h>
#include <Imagepp/all/h/HFCStat.h>


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
THUMBNAILS_EXPORT void GetBaseDirOfExecutingModule(WStringR baseDir);



/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Chantal.Poulin                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
class CriticalSectionHelper
    {
    static CRITICAL_SECTION s_criticalSection;
    
    public:
        THUMBNAILS_EXPORT CriticalSectionHelper();
        THUMBNAILS_EXPORT ~CriticalSectionHelper();
        THUMBNAILS_EXPORT static void Init();
    };

struct RasterFacility
    {
    static void ConvertThePixels(size_t pi_Width, size_t pi_Height, const HFCPtr<HRPPixelType>& pi_rpSrcPixelType,
        const HFCPtr<HCDPacket>& pi_rpSrcPacket, const HFCPtr<HRPPixelType>& pi_rpDstPixelType,
        HFCPtr<HCDPacket>& po_rpDstPacket);

    static void CreateHBitmapFromHRFThumbnail(HBITMAP* pThumbnailBmp, HFCPtr<HRFThumbnail>&  pThumbnail, HFCPtr<HRPPixelType>& pPixelType);

    };
