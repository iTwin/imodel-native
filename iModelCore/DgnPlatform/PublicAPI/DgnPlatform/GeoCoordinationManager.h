/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include    "IGeoCoordServices.h"

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
* The GeoCoordinationManager provides access to certain GeoCoordination services.
* @ingroup  GeoCoordination
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GeoCoordinationManager : NonCopyableClass
{
DGNPLATFORM_EXPORT  static IGeoCoordinateServicesP   GetServices();
};  // GeoCoordinationManager

END_BENTLEY_DGN_NAMESPACE



