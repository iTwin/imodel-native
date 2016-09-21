/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentReferent.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

HANDLER_DEFINE_MEMBERS(AlignmentReferentElementHandler)
HANDLER_DEFINE_MEMBERS(AlignmentStationHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentStation::AlignmentStation(CreateParams const& params) :
    T_Super(params), m_atLocationAspectId(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentStation::AlignmentStation(CreateParams const& params, DistanceExpressionCR distanceExpression, double restartValue) :
    T_Super(params, restartValue), m_atLocationAspectId(0)
    {
    _AddLinearlyReferencedLocation(*LinearlyReferencedAtLocation::Create(distanceExpression));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentStationPtr AlignmentStation::Create(AlignmentCR alignment, DistanceExpressionCR distanceExpression, double restartValue)
    {
    if (!alignment.GetModelId().IsValid() || !alignment.GetElementId().IsValid())
        return nullptr;

    CreateParams params(alignment.GetDgnDb(), alignment.GetModelId(), QueryClassId(alignment.GetDgnDb()),
        RoadRailAlignmentDomain::QueryAlignmentCategoryId(alignment.GetDgnDb()));
    params.SetParentId(alignment.GetElementId());

    AlignmentStationPtr retVal(new AlignmentStation(params, distanceExpression, restartValue));
    retVal->_SetLinearElementId(alignment.GetElementId());
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceExpressionP AlignmentStation::GetAtPositionP()
    {
    // TODO: Handle access to an un-persisted AlignmentStation
    BeAssert(GetElementId().IsValid());

    if (m_atLocationAspectId == 0)
        {
        auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement(
            "SELECT ECInstanceId FROM " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedAtLocation) " WHERE ElementId = ?;");
        BeAssert(stmtPtr.IsValid());

        stmtPtr->BindId(1, GetElementId());
        if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
            {
            BeAssert(false);
            return nullptr;
            }

        m_atLocationAspectId = stmtPtr->GetValueId<ECInstanceId>(0).GetValue();
        }

    auto locationP = DgnElement::MultiAspect::GetP<LinearlyReferencedAtLocation>(
        *this, *LinearlyReferencedAtLocation::QueryClass(GetDgnDb()), ECInstanceId(m_atLocationAspectId));
    BeAssert(locationP);

    return &locationP->GetAtPositionR();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceExpressionCR AlignmentStation::GetAtPosition() const
    {
    return *const_cast<AlignmentStationP>(this)->GetAtPositionP();
    }