/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
LineManipulationStrategyPtr LineManipulationStrategy::Create
(
    DSegment3dCR line
)
    {
    DPoint3d start, end;
    line.GetEndPoints(start, end);
    return Create(start, end);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
LineManipulationStrategyPtr LineManipulationStrategy::Create
(
    DPoint3dCR start, 
    DPoint3dCR end
)
    {
    LineManipulationStrategyPtr strategy = Create();
    strategy->AppendKeyPoints({start, end});
    return strategy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr LineManipulationStrategy::_FinishPrimitive() const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if (keyPoints.size() < 2)
        return nullptr;
    
    return ICurvePrimitive::CreateLine(DSegment3d::From(keyPoints[0], keyPoints[1]));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void LineManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if ((IsDynamicKeyPointSet() && keyPoints.size() <= 2) ||
        (!IsDynamicKeyPointSet() && keyPoints.size() < 2))
        T_Super::_AppendKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool LineManipulationStrategy::_IsComplete() const 
    {
    return GetKeyPoints().size() == 2;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool LineManipulationStrategy::_CanAcceptMorePoints() const 
    {
    return !_IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool LineManipulationStrategy::_IsContinious() const
    {
    return false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitiveManipulationStrategyPtr LineManipulationStrategy::_Clone() const
    {
    return Create();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyPtr LineManipulationStrategy::_CreateDefaultPlacementStrategy()
    {
    return LinePointsPlacementStrategy::Create(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
LinePlacementStrategyPtr LineManipulationStrategy::_CreateLinePlacementStrategy
(
    LinePlacementStrategyType strategyType
)
    {
    return LinePlacementStrategy::Create(strategyType, *this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<ConstructionGeometry> LineManipulationStrategy::_FinishConstructionGeometry() const
    {
    return bvector<ConstructionGeometry>();
    }