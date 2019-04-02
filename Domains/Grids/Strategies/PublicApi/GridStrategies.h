/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/GridStrategies.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingShared/BuildingSharedApi.h>
#include <Grids/Domain/GridsMacros.h>
#include <Grids/Elements/GridElementsAPI.h>

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(SketchGridSurfacePlacementStrategy)
GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(SketchGridSurfaceManipulationStrategy)

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(GridPlanarSurfaceManipulationStrategy)

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(LineGridSurfacePlacementStrategy)
GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(LineGridSurfaceManipulationStrategy)

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(SplineGridSurfacePlacementStrategy)
GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(SplineGridSurfaceManipulationStrategy)

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(ArcGridSurfaceManipulationStrategy)
GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(ArcGridSurfacePlacementStrategy)

#include "SketchGridSurfaceManipulationStrategy.h"
#include "SketchGridSurfacePlacementStrategy.h"

#include "GridPlanarSurfaceManipulationStrategy.h"

#include "LineGridSurfaceManipulationStrategy.h"
#include "LineGridSurfacePlacementStrategy.h"

#include "SplineGridSurfaceManipulationStrategy.h"
#include "SplineGridSurfacePlacementStrategy.h"

#include "ArcGridSurfaceManipulationStrategy.h"
#include "ArcGridSurfacePlacementStrategy.h"