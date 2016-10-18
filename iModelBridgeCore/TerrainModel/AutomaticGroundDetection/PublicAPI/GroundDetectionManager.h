/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/PublicAPI/GroundDetectionManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include    "IGroundDetectionServices.h"

GROUND_DETECTION_TYPEDEF(GroundDetectionManager)

BEGIN_GROUND_DETECTION_NAMESPACE

/*=================================================================================**//**
* The GroundDetectionManager provides access to certain Point Cloud Ground Detection services.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GroundDetectionManager : NonCopyableClass
{
public:
    //we can dynamically provide an implementation
     GROUND_DETECTION_EXPORT static void Register(IGroundDetectionServices* pGroundDetectionServicesImpl);
     GROUND_DETECTION_EXPORT static void UnRegister(IGroundDetectionServices* pGroundDetectionServicesImpl);

     GROUND_DETECTION_EXPORT static IGroundDetectionServices*   GetServices();
};  // GeoCoordinationManager

END_GROUND_DETECTION_NAMESPACE



