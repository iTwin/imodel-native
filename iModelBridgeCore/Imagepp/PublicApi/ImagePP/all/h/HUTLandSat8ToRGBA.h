//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRARaster.h"
#include "HRFRasterFile.h"

BEGIN_IMAGEPP_NAMESPACE

IMAGEPP_EXPORT HFCPtr<HRARaster> HUTLandSat8ToRGBA(HFCPtr<HRARaster>& pRaster, HFCPtr<HRFRasterFile>& pRasterFile, uint32_t pageNumber,
                                                   double  const* pNodataValue = NULL, double HistoCut = 0.0, bool UseSameMinMax = true);

END_IMAGEPP_NAMESPACE



