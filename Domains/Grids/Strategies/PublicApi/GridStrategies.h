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

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(GridPlanarSurfaceManipulationStrategy)

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(LineGridSurfacePlacementStrategy)
GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(LineGridSurfaceManipulationStrategy)

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(ArcGridSurfaceManipulationStrategy)
GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(ArcGridSurfacePlacementStrategy)

#include "SketchGridSurfaceManipulationStrategy.h"
#include "SketchGridSurfacePlacementStrategy.h"

#include "GridPlanarSurfaceManipulationStrategy.h"

#include "LineGridSurfaceManipulationStrategy.h"
#include "LineGridSurfacePlacementStrategy.h"

#include "ArcGridSurfaceManipulationStrategy.h"
#include "ArcGridSurfacePlacementStrategy.h"