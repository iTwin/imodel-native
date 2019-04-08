/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/GeometryManipulationStrategiesApi.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingShared/BuildingSharedMacros.h>
#include <Geom/GeomApi.h>

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(GeometryManipulationStrategyBase)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(GeometryManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(NullGeometryManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(CurvePrimitiveManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(CurveVectorManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ChildCurveVectorManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LineManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LineStringManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(EllipseManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(GeometryPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(NullGeometryPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(CurveVectorPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(CurvePrimitivePlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ChildCurveVectorPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(IArcPlacementMethod)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcStartCenterPlacementMethod)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcCenterStartPlacementMethod)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcStartMidEndPlacementMethod)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcStartEndMidPlacementMethod)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LinePlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(SplinePlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(SplineControlPointsPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(SplineThroughPointsPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(SplineManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(SplineControlPointsManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(SplineThroughPointsManipulationStrategy)

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

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(SolidPrimitiveManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ExtrusionManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(SolidPrimitivePlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ExtrusionPlacementStrategy)

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(RectanglePlacementStrategy)

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(PointManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(PointPlacementStrategy)

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(DynamicStateBase)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(BooleanDynamicState)
BUILDING_SHARED_TYPEDEFS(IResettableDynamic)

BEGIN_BUILDING_SHARED_NAMESPACE
enum class LinePlacementStrategyType;
enum class ArcPlacementMethod;
enum class LineStringPlacementStrategyType;
enum class SplinePlacementStrategyType;
END_BUILDING_SHARED_NAMESPACE

#include "ConstructionGeometry.h"
#include "GeometryManipulationStrategyBase.h"
#include "GeometryManipulationStrategy.h"
#include "NullGeometryManipulationStrategy.h"
#include "CurvePrimitiveManipulationStrategy.h"
#include "CurveVectorManipulationStrategy.h"
#include "ChildCurveVectorManipulationStrategy.h"
#include "LineManipulationStrategy.h"
#include "SplineManipulationStrategy.h"
#include "LineStringManipulationStrategy.h"
#include "EllipseManipulationStrategy.h"
#include "ArcManipulationStrategy.h"
#include "GeometryPlacementStrategy.h"
#include "NullGeometryPlacementStrategy.h"
#include "CurveVectorPlacementStrategy.h"
#include "CurvePrimitivePlacementStrategy.h"
#include "ChildCurveVectorPlacementStrategy.h"
#include "LinePlacementStrategy.h"
#include "SplinePlacementStrategy.h"
#include "LineStringPlacementStrategy.h"
#include "LineStringPointsPlacementStrategy.h"
#include "LineStringMetesAndBoundsPlacementStrategy.h"

#include "ArcPlacementStrategy.h"
#include "ArcStartCenterPlacementMethod.h"
#include "ArcCenterStartPlacementMethod.h"
#include "ArcStartMidEndPlacementMethod.h"
#include "ArcStartEndMidPlacementMethod.h"

#include "IArcElementKeyPointContainer.h"
#include "ElementManipulationStrategy.h"
#include "ElementPlacementStrategy.h"

#include "SolidPrimitiveManipulationStrategy.h"
#include "ExtrusionManipulationStrategy.h"
#include "SolidPrimitivePlacementStrategy.h"
#include "ExtrusionPlacementStrategy.h"

#include "RectanglePlacementStrategy.h"

#include "PointManipulationStrategy.h"
#include "PointPlacementStrategy.h"

#include "ScopedDynamicKeyPointResetter.h"