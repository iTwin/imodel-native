//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HUTLandSat8ToRGBA.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRARaster.h"
#include "HRFRasterFile.h"

BEGIN_IMAGEPP_NAMESPACE

IMAGEPP_EXPORT HFCPtr<HRARaster> HUTLandSat8ToRGBA(HFCPtr<HRARaster>& pRaster, HFCPtr<HRFRasterFile>& pRasterFile, uint32_t pageNumber,
                                                   double  const* pNodataValue = NULL, double HistoCut = 0.0, bool UseSameMinMax = true);

END_IMAGEPP_NAMESPACE



