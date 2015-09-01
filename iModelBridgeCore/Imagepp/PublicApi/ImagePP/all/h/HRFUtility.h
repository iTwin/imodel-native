//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFUtility.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
                                              uint32_t               pi_Page,
                                              uint32_t*                pio_pPreferedWidth,
                                              uint32_t*                pio_pPreferedHeight,
                                              bool                  pi_UseBestQuality);

//-----------------------------------------------------------------------------
// HRFStretcher
//-----------------------------------------------------------------------------
Byte* HRFStretcher(HFCPtr<HRFRasterFile>& pi_rpSource,
                     uint32_t               pi_Page,
                     uint32_t*                pio_pPreferedWidth,
                     uint32_t*                pio_pPreferedHeight,
                     bool                  pi_UseBestQuality);

//-----------------------------------------------------------------------------
// IsValidMatrix
//-----------------------------------------------------------------------------
bool IsValidMatrix(const HFCMatrix<3, 3>& pi_rMatrix);

//-----------------------------------------------------------------------------
// WriteEmptyFile
//-----------------------------------------------------------------------------
IMAGEPP_EXPORT void WriteEmptyFile(HFCPtr<HRFRasterFile>& pi_prFile,
                           Byte*                 pi_pRGBDefaultColor);
END_IMAGEPP_NAMESPACE
