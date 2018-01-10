/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/GeometryManipulationStrategiesApi.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingShared/BuildingSharedMacros.h>
#include <DgnPlatform/DgnPlatformApi.h>

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(GeometryManipulationStrategyBase)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(GeometryManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(CurvePrimitiveManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(CurveVectorManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ChildCurveVectorManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LineManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LineStringManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(EllipseManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(GeometryPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(CurveVectorPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(CurvePrimitivePlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ChildCurveVectorPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcStartCenterPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcCenterStartPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcStartMidEndPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcStartEndMidPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LinePlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LinePointsPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LinePointLengthAnglePlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LineMetesAndBoundsPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LinePointsLengthPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LinePointsAnglePlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LineStringPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LineStringPointsPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LineStringMetesAndBoundsPlacementStrategy)

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ElementManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ElementPlacementStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE
enum class LinePlacementStrategyType;
enum class ArcPlacementStrategyType;
enum class LineStringPlacementStrategyType;
END_BUILDING_SHARED_NAMESPACE

#include "GeometryManipulationStrategyBase.h"
#include "GeometryManipulationStrategy.h"
#include "CurvePrimitiveManipulationStrategy.h"
#include "CurveVectorManipulationStrategy.h"
#include "ChildCurveVectorManipulationStrategy.h"
#include "LineManipulationStrategy.h"
#include "LineStringManipulationStrategy.h"
#include "EllipseManipulationStrategy.h"
#include "ArcManipulationStrategy.h"
#include "GeometryPlacementStrategy.h"
#include "CurveVectorPlacementStrategy.h"
#include "CurvePrimitivePlacementStrategy.h"
#include "ChildCurveVectorPlacementStrategy.h"
#include "LinePlacementStrategy.h"
#include "LineStringPlacementStrategy.h"
#include "LineStringPointsPlacementStrategy.h"
#include "LineStringMetesAndBoundsPlacementStrategy.h"
#include "ArcPlacementStrategy.h"
#include "ArcStartCenterPlacementStrategy.h"
#include "ArcCenterStartPlacementStrategy.h"
#include "ArcStartMidEndPlacementStrategy.h"
#include "ArcStartEndMidPlacementStrategy.h"

#include "ElementManipulationStrategy.h"
#include "ElementPlacementStrategy.h"