/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void IArcPlacementMethod::AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    _AddKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void IArcPlacementMethod::PopKeyPoint()
    {
    _PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void IArcPlacementMethod::AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    _AddDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void IArcPlacementMethod::AddDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    _AddDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcPlacementMethod IArcPlacementMethod::GetMethod() const
    {
    return _GetMethod();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> IArcPlacementMethod::GetKeyPoints() const
    {
    return _GetKeyPoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void IArcPlacementStrategy::SetPlacementMethod
(
    ArcPlacementMethod method
)
    {
    _SetPlacementMethod(method);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void IArcPlacementStrategy::SetUseSweep
(
    bool useSweep
)
    {
    _SetUseSweep(useSweep);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void IArcPlacementStrategy::SetSweep
(
    double sweep
)
    {
    _SetSweep(sweep);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void IArcPlacementStrategy::SetUseRadius
(
    bool useRadius
)
    {
    _SetUseRadius(useRadius);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void IArcPlacementStrategy::SetRadius
(
    double radius
)
    {
    _SetRadius(radius);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
bool IArcKeyPointContainer::TryGetStartKeyPoint
(
    DPoint3dR startKeyPoint
) const
    {
    return _TryGetStartKeyPoint(startKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
bool IArcKeyPointContainer::TryGetCenterKeyPoint
(
    DPoint3dR centerKeyPoint
) const
    {
    return _TryGetCenterKeyPoint(centerKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
bool IArcKeyPointContainer::TryGetMidKeyPoint
(
    DPoint3dR midKeyPoint
) const
    {
    return _TryGetMidKeyPoint(midKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
bool IArcKeyPointContainer::TryGetEndKeyPoint
(
    DPoint3dR endKeyPoint
) const
    {
    return _TryGetEndKeyPoint(endKeyPoint);
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
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_AddDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    m_placementMethod->AddDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_SetPlacementMethod
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> ArcPlacementStrategy::_GetKeyPoints() const
    {
    return m_placementMethod->GetKeyPoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_SetUseSweep
(
    bool useSweep
)
    {
    _SetProperty(prop_UseSweep(), useSweep);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_SetSweep
(
    double sweep
)
    {
    _SetProperty(prop_Sweep(), sweep);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_SetUseRadius
(
    bool useRadius
)
    {
    _SetProperty(prop_UseRadius(), useRadius);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_SetRadius
(
    double radius
)
    {
    _SetProperty(prop_Radius(), radius);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
template<bool(ArcManipulationStrategy::*isSet)() const, DPoint3d(ArcManipulationStrategy::*get)() const>
bool ArcPlacementStrategy::TryGetKeyPoint
(
    ArcManipulationStrategyCP container,
    DPoint3dR pointToSet
) const
    {
    BeAssert(nullptr != container);

    if (!(container->*isSet)())
        return false;

    pointToSet = (container->*get)();
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ArcPlacementStrategy::_TryGetStartKeyPoint
(
    DPoint3dR startKeyPoint
) const
    {
    return TryGetKeyPoint<&ArcManipulationStrategy::IsStartSet, &ArcManipulationStrategy::GetStart>(m_manipulationStrategy.get(), startKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ArcPlacementStrategy::_TryGetCenterKeyPoint
(
    DPoint3dR centerKeyPoint
) const
    {
    return TryGetKeyPoint<&ArcManipulationStrategy::IsCenterSet, &ArcManipulationStrategy::GetCenter>(m_manipulationStrategy.get(), centerKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ArcPlacementStrategy::_TryGetMidKeyPoint
(
    DPoint3dR midKeyPoint
) const
    {
    return TryGetKeyPoint<&ArcManipulationStrategy::IsMidSet, &ArcManipulationStrategy::GetMid>(m_manipulationStrategy.get(), midKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ArcPlacementStrategy::_TryGetEndKeyPoint
(
    DPoint3dR endKeyPoint
) const
    {
    return TryGetKeyPoint<&ArcManipulationStrategy::IsEndSet, &ArcManipulationStrategy::GetEnd>(m_manipulationStrategy.get(), endKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_CopyKeyPointsTo
(
    ArcPlacementStrategyR other
) const
    {
    BeAssert(!IsDynamicKeyPointSet());
    switch (other.GetPlacementMethod())
        {
        case ArcPlacementMethod::StartEndMid:
        case ArcPlacementMethod::StartCenter:
        case ArcPlacementMethod::StartMidEnd:
            {
            if (!m_manipulationStrategy->IsStartSet())
                return;

            other.AddKeyPoint(m_manipulationStrategy->GetStart());
            break;
            }
        default:
            break;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
template <typename T>
void copy2PointsTo
(
    ArcManipulationStrategyCR manipulationStrategy,
    ArcPlacementMethod placementMethod,
    T& other
)
    {
    if (!manipulationStrategy.IsStartSet())
        return;

    other.AddKeyPoint(manipulationStrategy.GetStart());
    switch (placementMethod)
        {
        case ArcPlacementMethod::StartEndMid:
            {
            if (manipulationStrategy.IsEndSet())
                other.AddKeyPoint(manipulationStrategy.GetEnd());
            break;
            }
        case ArcPlacementMethod::StartMidEnd:
            {
            if (manipulationStrategy.IsMidSet())
                other.AddKeyPoint(manipulationStrategy.GetMid());
            break;
            }
        default:
            break;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_CopyKeyPointsTo
(
    LinePlacementStrategyR other
) const
    {
    BeAssert(!IsDynamicKeyPointSet());
    if (!m_manipulationStrategy->IsStartSet())
        return;

    other.AddKeyPoint(m_manipulationStrategy->GetStart());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_CopyKeyPointsTo
(
    LineStringPlacementStrategyR other
) const
    {
    BeAssert(!IsDynamicKeyPointSet());
    copy2PointsTo(*m_manipulationStrategy, GetPlacementMethod(), other);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_CopyKeyPointsTo
(
    SplineControlPointsPlacementStrategyR other
) const
    {
    BeAssert(!IsDynamicKeyPointSet());
    copy2PointsTo(*m_manipulationStrategy, GetPlacementMethod(), other);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void ArcPlacementStrategy::_CopyKeyPointsTo
(
    SplineThroughPointsPlacementStrategyR other
) const
    {
    BeAssert(!IsDynamicKeyPointSet());
    copy2PointsTo(*m_manipulationStrategy, GetPlacementMethod(), other);
    }