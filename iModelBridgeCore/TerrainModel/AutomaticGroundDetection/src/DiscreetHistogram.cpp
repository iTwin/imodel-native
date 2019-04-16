/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"

#include <TerrainModel/AutomaticGroundDetection/GroundDetectionMacros.h>
#include "DiscreetHistogram.h"

BEGIN_GROUND_DETECTION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DiscreetHistogramPtr DiscreetHistogram::Create(double minimum, double maximum, size_t entryCount)
    {
    return new DiscreetHistogram(minimum, maximum, entryCount);
    }

END_GROUND_DETECTION_NAMESPACE
