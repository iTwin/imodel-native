/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void applyToTileTree(DgnViewportR vp, DgnModelId const& modelId, T func)
    {
    auto target = vp.GetRenderTarget();
    auto system = nullptr != target ? &target->GetSystem() : nullptr;
    if (nullptr == system) // typically, because the viewport is being destroyed...
        return;

    auto model = vp.GetViewController().GetDgnDb().Models().Get<SpatialModel>(modelId);
    auto tree = model.IsValid() ? model->GetTileTree(system) : nullptr;
    if (nullptr != tree)
        func(*tree);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
SubjectViewController::SubjectViewController(SpatialViewDefinition const& view, SubjectColorMap const& subjectColors)
    : SpatialViewController(view)
    {
    m_subjectColors = subjectColors;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SubjectViewController::_OnAttachedToViewport(DgnViewportR vp)
    {
    T_Super::_OnAttachedToViewport(vp);

    // NB: This applies to every spatial model, because models may be added or removed from this view controller's viewed model list during alignment workflow.
    // Possibly m_subjectColors contains the actual models of interest?

    forEachSpatialModel(GetDgnDb(), [&](DgnModelId const& modelId)
            {
            applyToTileTree(vp, modelId, [&](TileTree::Root& tree)
                {
                tree.SetIgnoreChanges(true);
                });
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SubjectViewController::_OnDetachedFromViewport(DgnViewportR vp)
    {
    forEachSpatialModel(GetDgnDb(), [&](DgnModelId const& modelId)
        {
        applyToTileTree(vp, modelId, [&](TileTree::Root& tree)
            {
            tree.SetIgnoreChanges(false);
            tree.SetDisplayTransform(nullptr);
            });
        });

    T_Super::_OnDetachedFromViewport(vp);
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
    Transform displayTransform = m_transformCache.find(subjectId) != m_transformCache.end() ? m_transformCache[subjectId] : Transform::FromIdentity();
    BeAssert(nullptr != m_vp);
    for (auto const& modelId : modelIds)
        applyToTileTree(*m_vp, modelId, [&](TileTree::Root& tree) { tree.SetDisplayTransform(&displayTransform); });
    
    m_vp->InvalidateScene();
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     12/17 
 //-------------------------------------------------------------------------------------------
void SubjectViewController::ApplyDynamicTransform(DgnElementId const& subjectId, DgnModelIdSet const& modelIds, TransformCP incrementalTransform)
    {
    Transform displayTransform = m_transformCache.find(subjectId) != m_transformCache.end() ? m_transformCache[subjectId] : Transform::FromIdentity();
    displayTransform = Transform::FromProduct(*incrementalTransform, displayTransform);
    BeAssert(nullptr != m_vp);
    for (auto const& modelId : modelIds)
        applyToTileTree(*m_vp, modelId, [&](TileTree::Root& tree) { tree.SetDisplayTransform(&displayTransform); });
    
    m_vp->InvalidateScene();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SubjectViewController::SetTransform(DgnElementId const& subjectId, DgnModelIdSet const& modelIds, TransformCP transform)
    {
    BeAssert(nullptr != m_vp);
    for (auto const& modelId : modelIds)
        applyToTileTree(*m_vp, modelId, [&](TileTree::Root& tree) { tree.SetDisplayTransform(transform); });
    
    m_transformCache[subjectId] = transform != nullptr ? *transform : Transform::FromIdentity();

    m_vp->InvalidateScene();
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
void    SubjectViewController::_AddFeatureOverrides(Render::FeatureSymbologyOverrides& ovrs) const
    {
    for (auto mapping : m_subjectColors)
        ovrs.OverrideModel(mapping.first, mapping.second);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     10/17
//-------------------------------------------------------------------------------------------
void    SubjectViewController::ToggleVisibility(DgnModelId modelId, bool isVisible)
    {
    if (m_subjectColors.find(modelId) == m_subjectColors.end())
        return;

    ChangeModelDisplay(modelId, isVisible);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     11/17
//-------------------------------------------------------------------------------------------
bool     SubjectViewController::IsVisible(DgnModelId modelId) const
    {
    if (m_subjectColors.find(modelId) == m_subjectColors.end())
        {
        BeAssert(false && "Subject not found in View Controller.");
        return false;
        }
    
    auto& models = GetSpatialViewDefinition().GetModelSelector().GetModelsR();
    return models.Contains(modelId);
    }
