//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// HRFUtility
//-----------------------------------------------------------------------------
// This is a utilities tools for HRF.
//-----------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HRFRasterFile;
class HRFCacheFileCreator;
class HRFClipShape;
class HFCURL;
class HRFRasterFileCapabilities;
class HRFThumbnail;
class HCDCodec;
class HFCBinStream;
class HCDPacket;
END_IMAGEPP_NAMESPACE

//----------------------------------------------------------------------------
#include "HFCPtr.h"
#include "HFCProgressIndicator.h"
#include "HFCMacros.h"
#include "HPMAttributeSet.h"
#include "HFCMatrix.h"

//----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
class HRFThumbnailProgressIndicator : public HFCProgressIndicator
    {
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFThumbnailProgressIndicator)

    // Disabled methodes
    HRFThumbnailProgressIndicator();
    };

//----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Shape importing
//-----------------------------------------------------------------------------
IMAGEPP_EXPORT HRFClipShape* ImportShapeFromArrayOfDouble(const double*           pi_pClipShape,
                                                  size_t                   pi_ClipShapeLength);


//-----------------------------------------------------------------------------
// Shape Exporting
//-----------------------------------------------------------------------------
IMAGEPP_EXPORT double*    ExportClipShapeToArrayOfDouble(const HRFClipShape&  pi_ClipShape,
                                                  size_t*              po_pShapeLength,
                                                  double              pi_SurfaceVersionToUse = 1.0);

//-----------------------------------------------------------------------------
// GenericImprove to redefine by your application
//-----------------------------------------------------------------------------
IMAGEPP_EXPORT HFCPtr<HRFRasterFile>  GenericImprove(HFCPtr<HRFRasterFile>       pi_rpRasterFile,
                                             const HRFCacheFileCreator*  pi_pCreator,
                                             bool                       pi_PageFileOverwrite=false,
                                             bool                       pi_ApplyPageFile=true);

//-----------------------------------------------------------------------------
// GenericImprove to redefine by your application
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile>  GenericImprove(HFCPtr<HRFRasterFile>       pi_rpRasterFile, 
                                      const HRFCacheFileCreator*  pi_pCreator,
                                      bool                        pi_PageFileOverwrite,
                                      bool                        pi_ApplyPageFile, 
                                      double                      pi_DefaultRatioToMeterForPageFile);

//-----------------------------------------------------------------------------
// HRFThumbnail
//-----------------------------------------------------------------------------
IMAGEPP_EXPORT HFCPtr<HRFThumbnail> HRFThumbnailMaker(HFCPtr<HRFRasterFile>& pi_rpSource,
                                                       uint32_t              pi_Page,
                                                       uint32_t*             pio_pPreferedWidth,
                                                       uint32_t*             pio_pPreferedHeight,
                                                       bool                  pi_UseBestQuality);

//-----------------------------------------------------------------------------
// IsValidMatrix
//-----------------------------------------------------------------------------
bool IsValidMatrix(const HFCMatrix<3, 3>& pi_rMatrix);

//-----------------------------------------------------------------------------
// WriteEmptyFile
//-----------------------------------------------------------------------------
IMAGEPP_EXPORT void WriteEmptyFile(HFCPtr<HRFRasterFile>& pi_prFile,
                                   Byte*                  pi_pRGBDefaultColor);

//-----------------------------------------------------------------------------
// Create PRJ(WKT) Sister file: Creates a sister file to a raster file to
// externally set the Geocoding. The sister file has a .prj extension and contains
// the definition of the Geographic Coordinate System in WKT format.
// filename: Name of the raster file to create a sister file for.
// coordinateSystemKeyName: The keyname of the Geographic Coordinate System
//     from which is created the sister file content.
//-----------------------------------------------------------------------------
IMAGEPP_EXPORT bool CreateSisterFile(Utf8String fileName, Utf8String coordinateSystemKeyName);

END_IMAGEPP_NAMESPACE
