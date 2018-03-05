/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ArcManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

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
    {
    std::fill_n(std::back_inserter(GetKeyPointsR()), 4, INVALID_POINT);
    }

#define KEY_POINT_ACCESSOR_IMPL(name, index) \
    bool ArcManipulationStrategy::Is##name##Set() const {return !GetKeyPoints()[index].AlmostEqual(INVALID_POINT);} \
    DPoint3d ArcManipulationStrategy::Get##name() const {BeAssert(Is##name##Set()); return GetKeyPoints()[index];} \
    void ArcManipulationStrategy::Set##name(DPoint3dCR newValue) {ReplaceKeyPoint(newValue, index);} \
    void ArcManipulationStrategy::Reset##name() {ReplaceKeyPoint(INVALID_POINT, index);} \
    void ArcManipulationStrategy::SetDynamic##name(DPoint3dCR newValue) {UpdateDynamicKeyPoint(newValue, index);} \
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
    strategy->SetCenter(arc.center);
    strategy->SetStart(arc.FractionToPoint(0));
    strategy->SetMid(arc.FractionToPoint(0.5));
    strategy->SetEnd(arc.FractionToPoint(1));

    return strategy;
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

        DVec3d centerToStart = GetStart() - GetCenter();
        DVec3d centerToMid = DVec3d::FromRotateVectorAroundVector(centerToStart, normal, Angle::FromRadians(m_sweep / 4));
        DVec3d centerToEnd = DVec3d::FromRotateVectorAroundVector(centerToStart, normal, Angle::FromRadians(m_sweep / 2));

        DPoint3d start = GetStart();
        DPoint3d mid = GetCenter() + centerToMid;
        DPoint3d end = GetCenter() + centerToEnd;

        DEllipse3d arc = DEllipse3d::FromPointsOnArc(start, mid, end);
        arc.sweep = m_sweep;
        return ICurvePrimitive::CreateArc(arc);
        }

    if (IsStartSet() && IsMidSet() && IsEndSet())
        {
        DEllipse3d arc = DEllipse3d::FromPointsOnArc(GetStart(), GetMid(), GetEnd());
        if (m_useSweep)
            arc.sweep = m_sweep;

        return ICurvePrimitive::CreateArc(arc);
        }

    if (IsCenterSet() && IsStartSet() && IsEndSet())
        {
        DVec3d centerStart = DVec3d::FromStartEnd(GetCenter(), GetStart());
        if (DoubleOps::AlmostEqual(centerStart.Magnitude(), 0)) // 0 radius arc
            return nullptr;

        DVec3d centerEnd = DVec3d::FromStartEnd(GetCenter(), GetEnd());

        bool startEndCenterInLine = DoubleOps::AlmostEqual(DVec3d::FromCrossProduct(centerStart, centerEnd).Magnitude(), 0);
        if (startEndCenterInLine)
            {
            DVec3d normal;
            if (BentleyStatus::SUCCESS != _TryGetProperty(prop_Normal(), normal))
                return nullptr;

            double centerEndMagnitude = centerEnd.Magnitude();
            DEllipse3d arc = DEllipse3d::FromCenterNormalRadius(GetCenter(), normal, GetCenter().Distance(GetStart()));
            if (DoubleOps::AlmostEqual(centerEndMagnitude, 0) ||
                DoubleOps::AlmostEqual(centerStart.AngleTo(centerEnd), 0))
                {
                return ICurvePrimitive::CreateArc(arc);
                }
            else if (!DoubleOps::AlmostEqual(centerEndMagnitude, 0))
                {
                arc.sweep = GetSweep();
                if (DoubleOps::AlmostEqual(arc.sweep, 0))
                    arc.sweep = Angle::Pi();
                return ICurvePrimitive::CreateArc(arc);
                }
            }

        DEllipse3d tmpArc = DEllipse3d::FromArcCenterStartEnd(GetCenter(), GetStart(), GetEnd());
        if (DoubleOps::AlmostEqual(GetSweep(), tmpArc.sweep))
            return ICurvePrimitive::CreateArc(tmpArc);
        else
            {
            tmpArc.ComplementSweep();
            return ICurvePrimitive::CreateArc(tmpArc);
            }
        }

    return nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcManipulationStrategy::_OnKeyPointsChanged()
    {
    if (!IsStartSet() || !IsEndSet())
        return;

    if (IsCenterSet())
        UpdateSweep(GetStart(), GetCenter(), GetEnd());
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