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
AlignmentPtr Alignment::Create(AlignmentModelR model)
    {
    return new Alignment(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), DgnCategoryId()));
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
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT HorizontalAlignment FROM " BRRA_SCHEMA(BRRA_CLASS_Alignment) " WHERE ECInstanceId = ?");
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
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT VerticalAlignment FROM " BRRA_SCHEMA(BRRA_CLASS_Alignment) " WHERE ECInstanceId = ?");
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
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentHorizontalPtr AlignmentHorizontal::Create(AlignmentCR alignment, CurveVectorCR horizontalGeometry)
    {
    if (!alignment.GetElementId().IsValid())
        return nullptr;

    CreateParams createParams(alignment.GetDgnDb(), alignment.GetModelId(), QueryClassId(alignment.GetDgnDb()), DgnCategoryId());
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

    CreateParams createParams(alignment.GetDgnDb(), alignment.GetModelId(), QueryClassId(alignment.GetDgnDb()), DgnCategoryId());
    createParams.SetParentId(alignment.GetElementId());

    return new AlignmentVertical(createParams, verticalGeometry);
    }