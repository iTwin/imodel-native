/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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



