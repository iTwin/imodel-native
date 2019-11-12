/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/SubjectViewController.h>
#include <DgnPlatform/DgnDbTables.h>

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void forEachSpatialModel(DgnDbR db, T func)
    {
    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId FROM Bis.SpatialModel");
    while (BE_SQLITE_ROW == stmt->Step())
        func(stmt->GetValueId<DgnModelId>(0));
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
SubjectViewController::SubjectViewController(SpatialViewDefinition const& view) : SpatialViewController(view)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectViewController::~SubjectViewController()
    {
    //
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     12/17
//-------------------------------------------------------------------------------------------
void SubjectViewController::ResetDynamicTransform(DgnElementId const& subjectId, DgnModelIdSet const& modelIds)
    {
    // ###TODO remove me
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     12/17 
 //-------------------------------------------------------------------------------------------
void SubjectViewController::ApplyDynamicTransform(DgnElementId const& subjectId, DgnModelIdSet const& modelIds, TransformCP incrementalTransform)
    {
    // ###TODO remove me
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SubjectViewController::SetTransform(DgnElementId const& subjectId, DgnModelIdSet const& modelIds, TransformCP transform)
    {
    // ###TODO remove me
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     11/17
//-------------------------------------------------------------------------------------------
void    SubjectViewController::SetModelsAndCategoriesVisibility(bool visible)
    {
    forEachSpatialModel(GetDgnDb(), [&](DgnModelId const& modelId) { ChangeModelDisplay(modelId, visible); });

    CachedECSqlStatementPtr categoryStmt = GetDgnDb().GetPreparedECSqlStatement("SELECT c.ECInstanceId FROM Bis.Category c");
    while (categoryStmt->Step() == BE_SQLITE_ROW)
        ChangeCategoryDisplay(categoryStmt->GetValueId<DgnCategoryId>(0), visible);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
void    SubjectViewController::ToggleVisibility(DgnModelId modelId, bool isVisible)
    {
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     11/17
//-------------------------------------------------------------------------------------------
bool     SubjectViewController::IsVisible(DgnModelId modelId) const
    {
    auto& models = GetSpatialViewDefinition().GetModelSelector().GetModelsR();
    return models.Contains(modelId);
    }
