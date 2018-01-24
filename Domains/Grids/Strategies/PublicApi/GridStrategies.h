/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/GridStrategies.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingShared/BuildingSharedApi.h>

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(SketchGridSurfacePlacementStrategy)
GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(SketchGridSurfaceManipulationStrategy)

#include "SketchGridSurfaceManipulationStrategy.h"
#include "SketchGridSurfacePlacementStrategy.h"

#include "SketchGridPlacementStrategy.h"
#include "ArcGridPlacementStrategy.h"
#include "LineGridPlacementStrategy.h"