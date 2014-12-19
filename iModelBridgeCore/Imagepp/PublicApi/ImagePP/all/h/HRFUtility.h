//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFUtility.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// HRFUtility
//-----------------------------------------------------------------------------
// This is a utilities tools for HRF.
//-----------------------------------------------------------------------------
#pragma once

class HRFRasterFile;
class HRFCacheFileCreator;
class HRFClipShape;
class HFCURL;
class HRFRasterFileCapabilities;
class HRFThumbnail;
class HCDCodec;
class HFCBinStream;

//----------------------------------------------------------------------------
#include "HFCPtr.h"
#include "HFCProgressIndicator.h"
#include "HFCMacros.h"
#include "HPMAttributeSet.h"
#include "HFCMatrix.h"

//----------------------------------------------------------------------------

class HRFThumbnailProgressIndicator : public HFCProgressIndicator
    {
private:
    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HRFThumbnailProgressIndicator)

    // Disabled methodes
    HRFThumbnailProgressIndicator();
    };

//----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Shape importing
//-----------------------------------------------------------------------------
_HDLLg HRFClipShape* ImportShapeFromArrayOfDouble(const double*           pi_pClipShape,
                                                  size_t                   pi_ClipShapeLength);


//-----------------------------------------------------------------------------
// Shape Exporting
//-----------------------------------------------------------------------------
_HDLLg double*    ExportClipShapeToArrayOfDouble(const HRFClipShape&  pi_ClipShape,
                                                  size_t*              po_pShapeLength,
                                                  double              pi_SurfaceVersionToUse = 1.0);

//-----------------------------------------------------------------------------
// GenericImprove to redefine by your application
//-----------------------------------------------------------------------------
_HDLLg HFCPtr<HRFRasterFile>  GenericImprove(HFCPtr<HRFRasterFile>       pi_rpRasterFile,
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
// CreateCombinedURLAndOffset
//-----------------------------------------------------------------------------
_HDLLg HFCPtr<HFCURL> CreateCombinedURLAndOffset(const HFCPtr<HFCURL>& pi_rpURL, uint64_t pi_Offset);


//-----------------------------------------------------------------------------
// HRFThumbnail
//-----------------------------------------------------------------------------
_HDLLg HFCPtr<HRFThumbnail> HRFThumbnailMaker(HFCPtr<HRFRasterFile>& pi_rpSource,
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
// ThrowFileException
//-----------------------------------------------------------------------------
_HDLLg /*IppImaging_Needs*/void ThrowFileExceptionIfError(HFCBinStream const* pi_pBinStream, const WString& pi_rURL);

//-----------------------------------------------------------------------------
// WriteEmptyFile
//-----------------------------------------------------------------------------
_HDLLg void WriteEmptyFile(HFCPtr<HRFRasterFile>& pi_prFile,
                           Byte*                 pi_pRGBDefaultColor);
