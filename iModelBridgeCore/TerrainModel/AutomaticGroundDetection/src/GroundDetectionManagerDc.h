/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/GroundDetectionManagerDc.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//#include "GroundDetectionManager.h"
#include <TerrainModel\AutomaticGroundDetection\IGroundDetectionServices.h>

GROUND_DETECTION_TYPEDEF(PCGroundTIN)

BEGIN_GROUND_DETECTION_NAMESPACE

struct GroundDetectionManagerDc : public IGroundDetectionServices
    {
      static IGroundDetectionServices*   GetServices();
    StatusInt _DoGroundDetection(GroundDetectionParameters& params, IGroundDetectionProgressListener* pProgressListener = NULL) override;
    StatusInt _GetSeedPointsFromTIN(bvector<DPoint3d>& seedpoints, GroundDetectionParameters& params, IGroundDetectionProgressListener* pProgressListener = NULL) override;
    StatusInt _UpdateTINFileFromPoints(const bvector<DPoint3d>& seedpoints, GroundDetectionParameters& params, IGroundDetectionProgressListener* pProgressListener = NULL) override;

    private:
    PCGroundTINPtr m_PCGroundTIN;
    };

END_GROUND_DETECTION_NAMESPACE

/*__PUBLISH_SECTION_END__*/