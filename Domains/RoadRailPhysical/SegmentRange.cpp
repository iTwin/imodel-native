/*--------------------------------------------------------------------------------------+
|
|     $Source: SegmentRange.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(SegmentRangeElementHandler)
HANDLER_DEFINE_MEMBERS(RailRangeHandler)
HANDLER_DEFINE_MEMBERS(RoadRangeHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId SegmentRangeElement::QueryAlignmentId() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement(
        "SELECT TargetECInstanceId FROM " BRRP_SCHEMA(BRRP_REL_SegmentRangeRefersToAlignment) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnElementId();

    return stmtPtr->GetValueId<DgnElementId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SegmentRangeElement::SetAlignment(SegmentRangeElementCR roadRange, AlignmentCP alignment)
    {
    if (!roadRange.GetElementId().IsValid() || (alignment && !alignment->GetElementId().IsValid()))
        return DgnDbStatus::BadArg;

    auto delStmtPtr = roadRange.GetDgnDb().GetPreparedECSqlStatement(
        "DELETE FROM " BRRP_SCHEMA(BRRP_REL_SegmentRangeRefersToAlignment) " WHERE SourceECInstanceId = ?;");
    BeAssert(delStmtPtr.IsValid());

    delStmtPtr->BindId(1, roadRange.GetElementId());
    if (DbResult::BE_SQLITE_DONE != delStmtPtr->Step())
        return DgnDbStatus::WriteError;

    if (alignment)
        {
        auto insStmtPtr = roadRange.GetDgnDb().GetPreparedECSqlStatement(
            "INSERT INTO " BRRP_SCHEMA(BRRP_REL_SegmentRangeRefersToAlignment) " (SourceECInstanceId, TargetECInstanceId) VALUES (?,?);");
        BeAssert(insStmtPtr.IsValid());

        insStmtPtr->BindId(1, roadRange.GetElementId());
        insStmtPtr->BindId(2, alignment->GetElementId());

        if (DbResult::BE_SQLITE_DONE != insStmtPtr->Step())
            return DgnDbStatus::WriteError;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRangePtr RoadRange::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailPhysicalDomain::QueryRoadCategoryId(model.GetDgnDb()));

    return new RoadRange(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRangeCPtr RoadRange::InsertWithAlignment(AlignmentCR alignment, DgnDbStatus* status)
    {
    auto retVal = Insert(status);
    if (retVal.IsValid())
        {
        DgnDbStatus localStatus = SetAlignment(*retVal, &alignment);
        if (status)
            *status = localStatus;
        }
    
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RailRangePtr RailRange::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailPhysicalDomain::QueryTrackCategoryId(model.GetDgnDb()));

    return new RailRange(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RailRangeCPtr RailRange::InsertWithAlignment(AlignmentCR alignment, DgnDbStatus* status)
    {
    auto retVal = Insert(status);
    if (retVal.IsValid())
        *status = SetAlignment(*retVal, &alignment);
    
    return retVal;
    }