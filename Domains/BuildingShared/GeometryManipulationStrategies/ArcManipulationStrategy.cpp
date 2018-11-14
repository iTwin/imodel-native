/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ArcManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"
#include <BuildingShared/Utils/UtilsApi.h>

#define INVALID_POINT DPoint3d::From(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max())

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcManipulationStrategy::ArcManipulationStrategy()
    : T_Super()
    , m_useSweep(false)
    , m_useRadius(false)
    , m_sweep(0)
    , m_radius(0)
    , m_lastArc(nullptr)
    {
    std::fill_n(std::back_inserter(GetKeyPointsR()), 4, INVALID_POINT);
    }

#define KEY_POINT_ACCESSOR_IMPL(name, index) \
    bool ArcManipulationStrategy::Is##name##Set() const {return !GetKeyPoints()[index].AlmostEqual(INVALID_POINT);} \
    size_t ArcManipulationStrategy::Get##name##Index() const {return index;} \
    DPoint3d ArcManipulationStrategy::Get##name() const {BeAssert(Is##name##Set()); return GetKeyPoints()[index];} \
    void ArcManipulationStrategy::Set##name(DPoint3dCR newValue) {ReplaceKeyPoint(newValue, index);} \
    void ArcManipulationStrategy::Reset##name() {ReplaceKeyPoint(INVALID_POINT, index);} \
    void ArcManipulationStrategy::SetDynamic##name(DPoint3dCR newValue) {_UpdateDynamicKeyPoint(newValue, index);_OnKeyPointsChanged();} \
    bool ArcManipulationStrategy::Is##name##Dynamic() const {return !GetAcceptedKeyPoints()[index].AlmostEqual(GetKeyPoints()[index]);}

KEY_POINT_ACCESSOR_IMPL(Start, s_startIndex)
KEY_POINT_ACCESSOR_IMPL(Center, s_centerIndex)
KEY_POINT_ACCESSOR_IMPL(Mid, s_midPointIndex)
KEY_POINT_ACCESSOR_IMPL(End, s_endIndex)

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcManipulationStrategyPtr ArcManipulationStrategy::Create
(
    ICurvePrimitiveCR arcPrimitive
)
    {
    DEllipse3d arc;
    if (!arcPrimitive.TryGetArc(arc))
        return nullptr;

    ArcManipulationStrategyPtr strategy = Create();
    strategy->Init(arcPrimitive);

    return strategy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcManipulationStrategyPtr ArcManipulationStrategy::Create() 
    { 
    return new ArcManipulationStrategy(); 
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcManipulationStrategy::Init
(
    ICurvePrimitiveCR arcPrimitive
)
    {
    BeAssert(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == arcPrimitive.GetCurvePrimitiveType());

    DEllipse3d arc;
    if (!arcPrimitive.TryGetArc(arc))
        return;

    SetCenter(arc.center);
    SetStart(arc.FractionToPoint(0));
    SetMid(arc.FractionToPoint(0.5));
    SetEnd(arc.FractionToPoint(1));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ArcManipulationStrategy::PointsOnLine
(
    DPoint3dCR p1, 
    DPoint3dCR p2, 
    DPoint3dCR p3
)
    {
    if (p1.AlmostEqual(p2) || p1.AlmostEqual(p3) || p2.AlmostEqual(p3))
        return true;

    DVec3d p1ToP2 = p2 - p1;
    DVec3d p1ToP3 = p3 - p1;

    return DoubleOps::AlmostEqual(DVec3d::FromCrossProduct(p1ToP2, p1ToP3).Magnitude(), 0);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr ArcManipulationStrategy::_FinishPrimitive() const
    {
    if (IsStartSet() && IsCenterSet() && m_useSweep)
        {
        DVec3d normal;
        if (BentleyStatus::SUCCESS != _TryGetProperty(prop_Normal(), normal))
            return nullptr;

        DVec3d centerToStart = GetArcStart() - GetCenter();
        if (DoubleOps::AlmostEqual(centerToStart.Magnitude(), 0))
            return nullptr;

        DVec3d centerToMid = GeometryUtils::CreateVectorFromRotateVectorAroundVector(centerToStart, normal, Angle::FromRadians(m_sweep / 4));
        DVec3d centerToEnd = GeometryUtils::CreateVectorFromRotateVectorAroundVector(centerToStart, normal, Angle::FromRadians(m_sweep / 2));

        DPoint3d start = GetArcStart();
        DPoint3d mid = GetCenter() + centerToMid;
        DPoint3d end = GetCenter() + centerToEnd;

        DEllipse3d arc = DEllipse3d::FromPointsOnArc(start, mid, end);
        arc.sweep = m_sweep;
        return ICurvePrimitive::CreateArc(arc);
        }

    if (IsStartSet() && IsMidSet() && IsEndSet())
        {
        if (PointsOnLine(GetStart(), GetMid(), GetEnd()))
            return nullptr;

        DEllipse3d arc = DEllipse3d::FromPointsOnArc(GetStart(), GetMid(), GetEnd());
        if (m_useSweep)
            arc.sweep = m_sweep;

        return ICurvePrimitive::CreateArc(arc);
        }

    if (IsCenterSet() && IsStartSet() && IsEndSet())
        {
        DVec3d normal;
        if (BentleyStatus::SUCCESS != _TryGetProperty(prop_Normal(), normal))
            return nullptr;

        DVec3d centerStart = DVec3d::FromStartEnd(GetCenter(), GetArcStart());
        if (DoubleOps::AlmostEqual(centerStart.Magnitude(), 0)) // 0 radius arc
            return nullptr;

        DVec3d centerEnd = DVec3d::FromStartEnd(GetCenter(), GetEnd());
        if (centerEnd.Magnitude() > 0)
            centerEnd.ScaleToLength(centerStart.Magnitude());

        DVec3d vec90;
        vec90.CrossProduct(normal, centerStart);
        vec90.ScaleToLength(centerStart.Magnitude());

        double angle = centerStart.PlanarAngleTo(centerEnd, normal);
        if (DoubleOps::AlmostEqual(angle, 0))
            {
            double sweep = Angle::TwoPi();
            if (m_lastArc.IsValid())
                {
                DEllipse3d arc;
                m_lastArc->TryGetArc(arc);
                if (arc.sweep < 0)
                    sweep = -Angle::TwoPi();
                }
            DEllipse3d newArc = DEllipse3d::FromVectors(GetCenter(), centerStart, vec90, 0.0, sweep);
            return ICurvePrimitive::CreateArc(newArc);
            }
        else
            {
            double lastSweep = 0;
            if (m_lastArc.IsValid())
                {
                DEllipse3d arc;
                m_lastArc->TryGetArc(arc);
                lastSweep = arc.sweep;
                }

            double newSweep = angle;
            if (angle < 0.0 && lastSweep > 0.0)
                newSweep = Angle::TwoPi() + angle;
            else if (angle > 0.0 && lastSweep < 0.0)
                newSweep = -(Angle::TwoPi() - angle);

            if (fabs(newSweep - lastSweep) > Angle::Pi() || DoubleOps::AlmostEqual(fabs(lastSweep), Angle::TwoPi()))
                newSweep = angle;

            DEllipse3d newArc = DEllipse3d::FromVectors(GetCenter(), centerStart, vec90, 0.0, newSweep);
            return ICurvePrimitive::CreateArc(newArc);
            }
        }

    return nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<IGeometryPtr> ArcManipulationStrategy::_FinishConstructionGeometry() const
    {
    ICurvePrimitivePtr arcPrimitive = _FinishPrimitive();
    if (arcPrimitive.IsValid())
        {
        DEllipse3d arc;
        if (!arcPrimitive->TryGetArc(arc))
            {
            BeAssert(false);
            return bvector<IGeometryPtr>();
            }

        DPoint3d center = arc.center;
        DPoint3d arcStart, arcEnd;
        arc.EvaluateEndPoints(arcStart, arcEnd);

        if (arc.IsFullEllipse())
            {
            return {IGeometry::Create(ICurvePrimitive::CreateLineString(bvector<DPoint3d>({center, arcStart})))};
            }
        else
            {
            return {IGeometry::Create(ICurvePrimitive::CreateLineString(bvector<DPoint3d>({arcEnd, center, arcStart})))};
            }
        }
    else
        {
        if (!IsStartSet() || !IsCenterSet())
            return bvector<IGeometryPtr>();

        DVec3d normal;
        if (BentleyStatus::SUCCESS != _TryGetProperty(prop_Normal(), normal))
            return bvector<IGeometryPtr>();

        DPoint3d center = GetCenter();
        DPoint3d start = GetArcStart();
        DVec3d centerStart = start - center;

        if (m_useRadius)
            {
            centerStart.ScaleToLength(m_radius);
            }
        if (DoubleOps::AlmostEqual(centerStart.Magnitude(), 0))
            return bvector<IGeometryPtr>();

        DPoint3d arcStart = center + centerStart;

        IGeometryPtr arcGeometry = IGeometry::Create(ICurvePrimitive::CreateArc(DEllipse3d::FromCenterNormalRadius(center, normal, centerStart.Magnitude())));
        IGeometryPtr lineStringGeometry = IGeometry::Create(ICurvePrimitive::CreateLineString(bvector<DPoint3d>({center, arcStart})));
        return {arcGeometry, lineStringGeometry};
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcManipulationStrategy::_OnKeyPointsChanged()
    {
    if (!IsStartSet() || !IsEndSet())
        return;

    if (IsCenterSet())
        UpdateSweep(GetArcStart(), GetCenter(), GetEnd());
    else if(IsMidSet())
        {
        DEllipse3d tmpArc = DEllipse3d::FromPointsOnArc(GetStart(), GetMid(), GetEnd());
        UpdateSweep(GetStart(), tmpArc.center, GetEnd());
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ArcManipulationStrategy::_IsComplete() const
    {
    if (IsStartSet() && IsCenterSet() && m_useSweep)
        return true;

    return IsStartSet() && !IsStartDynamic() && IsEndSet() && !IsEndDynamic() &&
        ((IsMidSet() && !IsMidDynamic()) || (IsCenterSet() && !IsCenterDynamic()));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyPtr ArcManipulationStrategy::_CreateDefaultPlacementStrategy()
    {
    return ArcPlacementStrategy::Create(ArcPlacementMethod::StartMidEnd, *this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ArcManipulationStrategy::_IsEmpty() const
    {
    return !IsStartSet() && !IsMidSet() && !IsCenterSet() && !IsEndSet();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ArcManipulationStrategy::_IsSingleKeyPointLeft() const
    {
    int validPointCounter = 0;

    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    for (DPoint3dCR keyPoint : keyPoints)
        {
        if (!keyPoint.AlmostEqual(INVALID_POINT))
            ++validPointCounter;
        }

    return validPointCounter == 1;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d ArcManipulationStrategy::_GetLastKeyPoint() const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    for (auto rit = keyPoints.rbegin(); keyPoints.rend() != rit; ++rit)
        {
        if (!rit->AlmostEqual(INVALID_POINT))
            return *rit;
        }

    return DPoint3d();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d ArcManipulationStrategy::_GetFirstKeyPoint() const
    {
    bvector <DPoint3d> const& keyPoints = GetKeyPoints();
    for (auto it = keyPoints.begin(); keyPoints.end() != it; ++it)
        {
        if (!it->AlmostEqual(INVALID_POINT))
            return *it;
        }

    return DPoint3d();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitiveManipulationStrategyPtr ArcManipulationStrategy::_Clone() const
    {
    return Create();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcPlacementStrategyPtr ArcManipulationStrategy::_CreateArcPlacementStrategy
(
    ArcPlacementMethod method
)
    {
    return ArcPlacementStrategy::Create(method, *this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcManipulationStrategy::_Clear()
    {
    ResetCenter();
    ResetEnd();
    ResetMid();
    ResetStart();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    bool const& value
)
    {
    if (0 == strcmp(key, prop_UseSweep()))
        m_useSweep = value;
    else if (0 == strcmp(key, prop_UseRadius()))
        m_useRadius = value;

    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ArcManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    bool& value
) const
    {
    if (0 == strcmp(key, prop_UseSweep()))
        {
        value = m_useSweep;
        return BentleyStatus::SUCCESS;
        }
    else if (0 == strcmp(key, prop_UseRadius()))
        {
        value = m_useRadius;
        return BentleyStatus::SUCCESS;
        }

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    int const& value
)
    {
    if (0 == strcmp(key, prop_Radius()))
        _SetProperty(key, static_cast<double>(value));
    else if (0 == strcmp(key, prop_Sweep()))
        _SetProperty(key, static_cast<double>(value));

    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    double const& value
)
    {
    if (0 == strcmp(key, prop_Radius()))
        m_radius = value;
    else if (0 == strcmp(key, prop_Sweep()))
        m_sweep = value;

    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ArcManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    double& value
) const
    {
    if (0 == strcmp(key, prop_Radius()))
        {
        value = m_radius;
        return BentleyStatus::SUCCESS;
        }
    else if (0 == strcmp(key, prop_Sweep()))
        {
        value = m_sweep;
        return BentleyStatus::SUCCESS;
        }

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcManipulationStrategy::UpdateLastArc()
    {
    ICurvePrimitivePtr arc = _FinishPrimitive();
    
    if (arc.IsNull())
        return;

    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != arc->GetCurvePrimitiveType())
        return;

    m_lastArc = arc;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcManipulationStrategy::_ReplaceKeyPoint
(
    DPoint3dCR newKeyPoint, 
    size_t index
)
    {
    UpdateLastArc();
    T_Super::_ReplaceKeyPoint(newKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcManipulationStrategy::_UpdateDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint, 
    size_t index
)
    {
    UpdateLastArc();
    T_Super::_UpdateDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcManipulationStrategy::_ResetDynamicKeyPoint()
    {
    UpdateLastArc();
    T_Super::_ResetDynamicKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d ArcManipulationStrategy::GetArcStart() const
    {
    // We need start and center keypoints, since we need to keep the radius,
    // which is determined by the distance between those two points.
    if (!IsStartSet() || !IsCenterSet())
        return INVALID_POINT;

    DPoint3d startKeyPoint = GetStart();
    DPoint3d centerKeyPoint = GetCenter();
    double radius = centerKeyPoint.Distance(startKeyPoint);

    DVec3d normal;
    // Vector from center to start should be perpendicular to normal,
    // so we cannot calculate the arc start point without it.
    if (_TryGetProperty(prop_Normal(), normal) != BentleyStatus::SUCCESS)
        return startKeyPoint;
    
    DPlane3d workingPlane = DPlane3d::FromOriginAndNormal(centerKeyPoint, normal);

    DPoint3d tmpArcStartPoint;
    workingPlane.ProjectPoint(tmpArcStartPoint, startKeyPoint);
    // Can't do anything if the projection is the same as the center point.
    // We would get a zero vector with which we couldn't do much.
    if (tmpArcStartPoint.AlmostEqual(centerKeyPoint))
        return tmpArcStartPoint;

    DVec3d centerToArcStart = DVec3d::FromStartEnd(centerKeyPoint, tmpArcStartPoint);
    centerToArcStart.ScaleToLength(radius);

    DPoint3d arcStartPoint = centerKeyPoint;
    arcStartPoint.Add(centerToArcStart);
    return arcStartPoint;
    }