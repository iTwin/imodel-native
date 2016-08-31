/*--------------------------------------------------------------------------------------+
|
|     $Source: Alignment.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

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
HorizontalAlignmentCPtr Alignment::QueryHorizontal() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT HorizontalAlignment FROM " RRA_CLASS_Alignment " WHERE ElementId = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return nullptr;

    return HorizontalAlignment::Get(GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalAlignmentCPtr Alignment::QueryMainVertical() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT VerticalAlignment FROM " RRA_CLASS_Alignment " WHERE ElementId = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return nullptr;

    return VerticalAlignment::Get(GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet Alignment::QueryVerticalAlignmentIds() const
    {

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