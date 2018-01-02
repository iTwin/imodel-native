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
    DPoint3dCR ArcManipulationStrategy::Get##name() const {BeAssert(Is##name##Set()); return GetKeyPoints()[index];} \
    void ArcManipulationStrategy::Set##name(DPoint3dCR newValue) {ReplaceKeyPoint(newValue, index);} \
    void ArcManipulationStrategy::Reset##name() {ReplaceKeyPoint(INVALID_POINT, index);} \
    void ArcManipulationStrategy::SetDynamic##name(DPoint3dCR newValue) {UpdateDynamicKeyPoint(newValue, index);}

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
        return ICurvePrimitive::CreateArc(DEllipse3d::FromArcCenterStartEnd(GetCenter(), GetStart(), GetEnd()));

    return nullptr;
    }