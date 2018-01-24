/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ArcPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcPlacementStrategy::ArcPlacementStrategy
(
    ArcManipulationStrategyP manipulationStrategy
)
    : T_Super()
    , m_manipulationStrategy(manipulationStrategy)
    {
    BeAssert(m_manipulationStrategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcPlacementStrategyPtr ArcPlacementStrategy::Create
(
    ArcPlacementStrategyType strategyType
)
    {
    switch (strategyType)
        {
        case ArcPlacementStrategyType::CenterStart :
            return ArcCenterStartPlacementStrategy::Create();
        case ArcPlacementStrategyType::StartCenter :
            return ArcStartCenterPlacementStrategy::Create();
        case ArcPlacementStrategyType::StartMidEnd :
            return ArcStartMidEndPlacementStrategy::Create();
        case ArcPlacementStrategyType::StartEndMid :
            return ArcStartEndMidPlacementStrategy::Create();
        default :
            BeAssert(false);
            return nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcPlacementStrategyPtr ArcPlacementStrategy::Create
(
    ArcPlacementStrategyType strategyType, 
    ArcManipulationStrategyR manipulationStrategy
)
    {
    ArcPlacementStrategyPtr placementStrategy = Create(strategyType);
    if (placementStrategy.IsNull())
        return nullptr;

    placementStrategy->m_manipulationStrategy = &manipulationStrategy;
    return placementStrategy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d ArcPlacementStrategy::CalculateVec90KeyPoint
(
    DPoint3dCR endPoint
) const
    {
    bvector<DPoint3d> const& keyPoints = _GetKeyPoints();
    if (keyPoints.size() < 2)
        {
        BeAssert(false);
        return {0,0,0};
        }

    DPoint3d vec0Point = keyPoints[0];
    DPoint3d center = keyPoints[1];
    DVec3d vec0 = DVec3d::FromStartEnd(center, vec0Point);
    DVec3d endVec = DVec3d::FromStartEnd(center, endPoint);
    DVec3d axis = DVec3d::FromCrossProduct(vec0, endVec);
    if (DoubleOps::AlmostEqual(axis.Magnitude(), 0))
        axis = DVec3d::From(0, 0, 1); // if all points are inline - fallback to default axis.
    DVec3d vec90 = DVec3d::FromRotate90Around(vec0, DVec3d::FromCrossProduct(vec0, endVec));
    vec90.ScaleToLength(vec0.Magnitude());
    DPoint3d vec90Point = center;
    vec90Point.Add(vec90);
    return vec90Point;
    }