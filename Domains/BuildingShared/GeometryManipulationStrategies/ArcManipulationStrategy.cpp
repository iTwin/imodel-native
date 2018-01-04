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
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr ArcManipulationStrategy::_FinishPrimitive() const
    {
    if (IsStartSet() && IsMidSet() && IsEndSet())
        return ICurvePrimitive::CreateArc(DEllipse3d::FromPointsOnArc(GetStart(), GetMid(), GetEnd()));

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
    return IsStartSet() && !IsStartDynamic() && IsEndSet() && !IsEndDynamic() &&
        ((IsMidSet() && !IsMidDynamic()) || (IsCenterSet() && !IsCenterDynamic()));
    }