/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
PointManipulationStrategyPtr PointManipulationStrategy::Create
(
    DPoint3dCR point
)
    {
    PointManipulationStrategyPtr strategy = Create();
    strategy->AppendKeyPoint(point);
    return strategy;
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
void PointManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    if (_IsComplete())
        return;

    T_Super::_AppendKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
bool PointManipulationStrategy::_IsComplete() const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    return !_IsDynamicKeyPointSet() && !keyPoints.empty();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
bool PointManipulationStrategy::_CanAcceptMorePoints() const
    {
    return !_IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<ConstructionGeometry> PointManipulationStrategy::_FinishConstructionGeometry() const
    {
    return bvector<ConstructionGeometry>();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
void PointManipulationStrategy::_AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint)
    {
    if (_IsComplete())
        return;

    T_Super::_AppendDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr PointManipulationStrategy::_FinishPrimitive() const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if (keyPoints.empty())
        return nullptr;

    return ICurvePrimitive::CreatePointString(&keyPoints.front(), 1);
    }