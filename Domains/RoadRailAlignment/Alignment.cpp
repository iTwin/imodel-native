/*--------------------------------------------------------------------------------------+
|
|     $Source: Alignment.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

HANDLER_DEFINE_MEMBERS(AlignmentHandler)
HANDLER_DEFINE_MEMBERS(AlignmentHorizontalHandler)
HANDLER_DEFINE_MEMBERS(AlignmentVerticalHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPtr Alignment::Create(AlignmentModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    return new Alignment(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), 
        RoadRailAlignmentDomain::QueryAlignmentCategoryId(model.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double Alignment::_GetLength() const
    {
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentHorizontalCPtr Alignment::QueryHorizontal() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BRRA_SCHEMA(BRRA_CLASS_AlignmentHorizontal) " WHERE ParentId = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return nullptr;

    return AlignmentHorizontal::Get(GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentVerticalCPtr Alignment::QueryMainVertical() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT MainVerticalAlignment FROM " BRRA_SCHEMA(BRRA_CLASS_Alignment) " WHERE ECInstanceId = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return nullptr;

    return AlignmentVertical::Get(GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet Alignment::QueryAlignmentVerticalIds() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT TargetECInstanceId FROM " BRRA_SCHEMA(BRRA_REL_AlignmentOwnsVerticals) " WHERE SourceECInstanceId = ?");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, GetElementId());

    DgnElementIdSet retVal;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        retVal.insert(stmt.GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Alignment::SetMainVertical(AlignmentVerticalCR vertical)
    {
    if (!vertical.GetElementId().IsValid())
        return DgnDbStatus::BadElement;

    if (vertical.GetParentId() != GetElementId())
        return DgnDbStatus::WrongElement;

    ECValue ecVal(vertical.GetElementId().GetValue());
    SetPropertyValue(BRRA_PROP_Alignment_MainVerticalAlignment, ecVal);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentHorizontalPtr AlignmentHorizontal::Create(AlignmentCR alignment, CurveVectorCR horizontalGeometry)
    {
    if (!alignment.GetElementId().IsValid())
        return nullptr;

    CreateParams createParams(alignment.GetDgnDb(), alignment.GetModelId(), QueryClassId(alignment.GetDgnDb()), 
        RoadRailAlignmentDomain::QueryAlignmentCategoryId(alignment.GetDgnDb()));
    createParams.SetParentId(alignment.GetElementId());

    return new AlignmentHorizontal(createParams, horizontalGeometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentVerticalPtr AlignmentVertical::Create(AlignmentCR alignment, CurveVectorCR verticalGeometry)
    {
    if (!alignment.GetElementId().IsValid())
        return nullptr;

    CreateParams createParams(alignment.GetDgnDb(), alignment.GetModelId(), QueryClassId(alignment.GetDgnDb()), 
        RoadRailAlignmentDomain::QueryAlignmentCategoryId(alignment.GetDgnDb()));
    createParams.SetParentId(alignment.GetElementId());

    return new AlignmentVertical(createParams, verticalGeometry);
    }
