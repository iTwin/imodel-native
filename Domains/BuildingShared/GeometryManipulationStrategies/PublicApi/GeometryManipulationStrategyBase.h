/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/GeometryManipulationStrategyBase.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

struct GeometryManipulationStrategyBase : RefCountedBase
    {
    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT GeometryManipulationStrategyBase();
    };

END_BUILDING_SHARED_NAMESPACE