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
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void IArcPlacementMethod::AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    _AddKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void IArcPlacementMethod::PopKeyPoint()
    {
    _PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void IArcPlacementMethod::AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    _AddDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcPlacementMethod IArcPlacementMethod::GetMethod() const
    {
    return _GetMethod();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcPlacementStrategy::ArcPlacementStrategy
(
    ArcManipulationStrategyR manipulationStrategy,
    IArcPlacementMethodR placementMethod
)
    : T_Super()
    , m_manipulationStrategy(&manipulationStrategy)
    , m_placementMethod(&placementMethod)
    {
    BeAssert(m_manipulationStrategy.IsValid());
    BeAssert(m_placementMethod.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
IArcPlacementMethodPtr ArcPlacementStrategy::CreatePlacementMethod
(
    ArcPlacementMethod method,
    ArcManipulationStrategyR manipulationStrategy
)
    {
    switch (method)
        {
        case ArcPlacementMethod::CenterStart:
            return ArcCenterStartPlacementMethod::Create(manipulationStrategy);
        case ArcPlacementMethod::StartCenter:
            return ArcStartCenterPlacementMethod::Create(manipulationStrategy);
        case ArcPlacementMethod::StartMidEnd:
            return ArcStartMidEndPlacementMethod::Create(manipulationStrategy);
        case ArcPlacementMethod::StartEndMid:
            return ArcStartEndMidPlacementMethod::Create(manipulationStrategy);
        default:
            BeAssert(false);
            return nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcPlacementStrategyPtr ArcPlacementStrategy::Create
(
    ArcPlacementMethod method
)
    {
    ArcManipulationStrategyPtr manipulationStrategy = ArcManipulationStrategy::Create();
    IArcPlacementMethodPtr placementMethod = CreatePlacementMethod(method, *manipulationStrategy);

    if (manipulationStrategy.IsNull() || placementMethod.IsNull())
        return nullptr;

    return new ArcPlacementStrategy(*manipulationStrategy, *placementMethod);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcPlacementStrategyPtr ArcPlacementStrategy::Create
(
    ArcPlacementMethod method, 
    ArcManipulationStrategyR manipulationStrategy
)
    {
    IArcPlacementMethodPtr placementMethod = CreatePlacementMethod(method, manipulationStrategy);
    
    if (placementMethod.IsNull())
        return nullptr;

    return new ArcPlacementStrategy(manipulationStrategy, *placementMethod);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    m_placementMethod->AddKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_PopKeyPoint()
    {
    m_placementMethod->PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    m_placementMethod->AddDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::SetPlacementMethod
(
    ArcPlacementMethod method
)
    {
    if (m_placementMethod->GetMethod() == method)
        return;

    m_manipulationStrategy->Clear();
    m_placementMethod = CreatePlacementMethod(method, *m_manipulationStrategy);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcPlacementMethod ArcPlacementStrategy::GetPlacementMethod() const
    {
    return m_placementMethod->GetMethod();
    }