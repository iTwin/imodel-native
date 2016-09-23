/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/PublicAPI/AutomaticGroundDetection/GroundDetectionManager.h $
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
     static void Register(IGroundDetectionServices* pGroundDetectionServicesImpl);
     static void UnRegister(IGroundDetectionServices* pGroundDetectionServicesImpl);

     static IGroundDetectionServices*   GetServices();
};  // GeoCoordinationManager

END_GROUND_DETECTION_NAMESPACE



