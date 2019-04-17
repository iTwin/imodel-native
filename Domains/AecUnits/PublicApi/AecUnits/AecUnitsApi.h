/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <DgnPlatform/ViewController.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/DgnDb.h>

#include "AecUnitsDefinitions.h"
#include "AecUnitsDomain.h"


BEGIN_BENTLEY_NAMESPACE

namespace AecUnits
    {


    //=======================================================================================
    // @bsiclass                                    BentleySystems
    //=======================================================================================
    struct  AecUnitsUtilities
        {
        AEC_UNITS_EXPORT static ECN::ECUnitCP                                    GetUnitCPFromProperty(Dgn::DgnElementCR element, Utf8StringCR propertyName);
        AEC_UNITS_EXPORT static BentleyStatus                                    SetDoublePropertyUsingUnitString(Dgn::DgnElementR element, Utf8StringCR propertyName, Utf8StringCR unitString, double value);
        AEC_UNITS_EXPORT static BentleyStatus                                    GetDoublePropertyUsingUnitString(Dgn::DgnElementCR element, Utf8StringCR propertyName, Utf8StringCR unitString, double& value);
        };
    }

END_BENTLEY_NAMESPACE

