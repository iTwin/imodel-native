/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionElementUsageInfoPtr DefinitionElementUsageInfo::Create(DgnDbR db, BeSQLite::IdSet<DgnElementId> const& elementIds)
    {
    DefinitionElementUsageInfoPtr context = new DefinitionElementUsageInfo(db);
    if (!context.IsValid())
        return nullptr;

    context->Initialize(elementIds);
    context->QueryUsage();
    return context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void idSetToJson(BeJsValue array, BeIdSet const& ids)
    {
    array.SetEmptyArray();
    Json::ArrayIndex i = 0;
    for (auto const& id : ids)
        array[i++] = id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElementUsageInfo::ToJson(BeJsValue value) const
    {
    if (!m_spatialCategoryIds.empty()) idSetToJson(value[json_spatialCategoryIds()], m_spatialCategoryIds.GetBeIdSet());
    if (!m_drawingCategoryIds.empty()) idSetToJson(value[json_drawingCategoryIds()], m_drawingCategoryIds.GetBeIdSet());
    if (!m_subCategoryIds.empty()) idSetToJson(value[json_subCategoryIds()], m_subCategoryIds.GetBeIdSet());
    if (!m_categorySelectorIds.empty()) idSetToJson(value[json_categorySelectorIds()], m_categorySelectorIds.GetBeIdSet());
    if (!m_modelSelectorIds.empty()) idSetToJson(value[json_modelSelectorIds()], m_modelSelectorIds.GetBeIdSet());
    if (!m_displayStyleIds.empty()) idSetToJson(value[json_displayStyleIds()], m_displayStyleIds.GetBeIdSet());
    if (!m_geometryPartIds.empty()) idSetToJson(value[json_geometryPartIds()], m_geometryPartIds.GetBeIdSet());
    if (!m_renderMaterialIds.empty()) idSetToJson(value[json_renderMaterialIds()], m_renderMaterialIds.GetBeIdSet());
    if (!m_lineStyleIds.empty()) idSetToJson(value[json_lineStyleIds()], m_lineStyleIds.GetBeIdSet());
    if (!m_textureIds.empty()) idSetToJson(value[json_textureIds()], m_textureIds.GetBeIdSet());
    if (!m_viewDefinitionIds.empty()) idSetToJson(value[json_viewDefinitionIds()], m_viewDefinitionIds.GetBeIdSet());
    if (!m_otherDefinitionElementIds.empty()) idSetToJson(value[json_otherDefinitionElementIds()], m_otherDefinitionElementIds.GetBeIdSet());
    if (!m_usedIds.empty()) idSetToJson(value[json_usedIds()], m_usedIds.GetBeIdSet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElementUsageInfo::Initialize(BeSQLite::IdSet<DgnElementId> const& elementIds)
    {
    for (DgnElementId elementId : elementIds)
        {
        DgnElementCPtr element = m_db.Elements().GetElement(elementId);
        if (!element.IsValid())
            continue; // invalid ElementIds are skipped

        DefinitionElementCP definitionElement = element->ToDefinitionElement();
        if (nullptr == definitionElement)
            continue; // non-DefinitionElements are skipped

        SpatialCategoryCP spatialCategory = dynamic_cast<SpatialCategoryCP>(definitionElement);
        if (nullptr != spatialCategory)
            {
            DgnCategoryId categoryId = spatialCategory->GetCategoryId();
            m_spatialCategoryIds.insert(categoryId);
            if (IsSpatialCategoryUsed(categoryId))
                m_usedIds.insert(categoryId);
            continue;
            }

        DrawingCategoryCP drawingCategory = dynamic_cast<DrawingCategoryCP>(definitionElement);
        if (nullptr != drawingCategory)
            {
            DgnCategoryId categoryId = drawingCategory->GetCategoryId();
            m_drawingCategoryIds.insert(categoryId);
            if (IsDrawingCategoryUsed(categoryId))
                m_usedIds.insert(categoryId);
            continue;
            }

        DgnSubCategoryCP subCategory = dynamic_cast<DgnSubCategoryCP>(definitionElement);
        if (nullptr != subCategory)
            {
            DgnSubCategoryId subCategoryId = subCategory->GetSubCategoryId();
            m_subCategoryIds.insert(subCategoryId);
            if (subCategory->IsDefaultSubCategory())
                m_usedIds.insert(subCategoryId);
            continue;
            }

        CategorySelectorCP categorySelector = dynamic_cast<CategorySelectorCP>(definitionElement);
        if (nullptr != categorySelector)
            {
            DgnElementId categorySelectorId = categorySelector->GetElementId();
            m_categorySelectorIds.insert(categorySelectorId);
            if (IsCategorySelectorUsed(categorySelectorId))
                m_usedIds.insert(categorySelectorId);
            continue;
            }

        ModelSelectorCP modelSelector = dynamic_cast<ModelSelectorCP>(definitionElement);
        if (nullptr != modelSelector)
            {
            DgnElementId modelSelectorId = modelSelector->GetElementId();
            m_modelSelectorIds.insert(modelSelectorId);
            if (IsModelSelectorUsed(modelSelectorId))
                m_usedIds.insert(modelSelectorId);
            continue;
            }

        DisplayStyleCP displayStyle = dynamic_cast<DisplayStyleCP>(definitionElement);
        if (nullptr != displayStyle)
            {
            DgnElementId displayStyleId = displayStyle->GetElementId();
            m_displayStyleIds.insert(displayStyleId);
            if (IsDisplayStyleUsed(displayStyleId))
                m_usedIds.insert(displayStyleId);
            continue;
            }

        DgnGeometryPartCP geometryPart = element->ToGeometryPart();
        if (nullptr != geometryPart)
            {
            DgnGeometryPartId geometryPartId = geometryPart->GetId();
            m_geometryPartIds.insert(geometryPartId);
            continue;
            }

        RenderMaterialCP renderMaterial = dynamic_cast<RenderMaterialCP>(definitionElement);
        if (nullptr != renderMaterial)
            {
            RenderMaterialId renderMaterialId = renderMaterial->GetMaterialId();
            m_renderMaterialIds.insert(renderMaterialId);
            continue;
            }

        LineStyleElementCP lineStyle = dynamic_cast<LineStyleElementCP>(definitionElement);
        if (nullptr != lineStyle)
            {
            DgnStyleId lineStyleId = lineStyle->GetId();
            m_lineStyleIds.insert(lineStyleId);
            continue;
            }

        DgnTextureCP texture = dynamic_cast<DgnTextureCP>(definitionElement);
        if (nullptr != texture)
            {
            DgnTextureId textureId = texture->GetTextureId();
            m_textureIds.insert(textureId);
            continue;
            }

        ViewDefinitionCP viewDefinition = dynamic_cast<ViewDefinitionCP>(definitionElement);
        if (nullptr != viewDefinition)
            {
            DgnViewId viewDefinitionId = viewDefinition->GetViewId();
            m_viewDefinitionIds.insert(viewDefinitionId);
            if (IsViewDefinitionUsed(viewDefinitionId))
                m_usedIds.insert(viewDefinitionId);
            continue;
            }

        m_otherDefinitionElementIds.insert(elementId); // unknown or unhandled Element Class
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefinitionElementUsageInfo::IsFullScanRequired() const
    {
    return !m_geometryPartIds.empty()
        || !m_lineStyleIds.empty()
        || !m_renderMaterialIds.empty()
        || !m_textureIds.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElementUsageInfo::QueryUsage()
    {
    if (IsFullScanRequired())
        {
        ScanGeometryStreams();
        }
    else if (!m_subCategoryIds.empty())
        {
        std::shared_ptr<BeSQLite::IdSet<BeInt64Id>> categoriesToScan = std::make_shared<BeSQLite::IdSet<BeInt64Id>>();
        for (DgnSubCategoryId subCategoryId : m_subCategoryIds)
            {
            DgnSubCategoryCPtr subCategory = m_db.Elements().Get<DgnSubCategory>(subCategoryId);
            if (subCategory.IsValid())
                categoriesToScan->insert(subCategory->GetCategoryId());
            }

        ScanGeometryStreams(categoriesToScan);
        }

    if (!m_geometryPartIds.empty())
        ScanLineStyles();

    if (!m_textureIds.empty())
        ScanDisplayStyles();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefinitionElementUsageInfo::IsSpatialCategoryUsed(DgnCategoryId categoryId) const
    {
    Utf8CP sql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " WHERE Category.Id=? LIMIT 1";
    CachedECSqlStatementPtr statement = m_db.GetPreparedECSqlStatement(sql);
    statement->BindId(1, categoryId);
    return BE_SQLITE_ROW == statement->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefinitionElementUsageInfo::IsDrawingCategoryUsed(DgnCategoryId categoryId) const
    {
    Utf8CP sql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement2d) " WHERE Category.Id=? LIMIT 1";
    CachedECSqlStatementPtr statement = m_db.GetPreparedECSqlStatement(sql);
    statement->BindId(1, categoryId);
    return BE_SQLITE_ROW == statement->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefinitionElementUsageInfo::IsDisplayStyleUsed(DgnElementId displayStyleId) const
    {
    Utf8CP sql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewDefinition) " WHERE DisplayStyle.Id=? LIMIT 1";
    CachedECSqlStatementPtr statement = m_db.GetPreparedECSqlStatement(sql);
    statement->BindId(1, displayStyleId);
    return BE_SQLITE_ROW == statement->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefinitionElementUsageInfo::IsCategorySelectorUsed(DgnElementId categorySelectorId) const
    {
    Utf8CP sql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewDefinition) " WHERE CategorySelector.Id=? LIMIT 1";
    CachedECSqlStatementPtr statement = m_db.GetPreparedECSqlStatement(sql);
    statement->BindId(1, categorySelectorId);
    return BE_SQLITE_ROW == statement->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefinitionElementUsageInfo::IsModelSelectorUsed(DgnElementId modelSelectorId) const
    {
    Utf8CP sql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_SpatialViewDefinition) " WHERE ModelSelector.Id=? LIMIT 1";
    CachedECSqlStatementPtr statement = m_db.GetPreparedECSqlStatement(sql);
    statement->BindId(1, modelSelectorId);
    return BE_SQLITE_ROW == statement->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefinitionElementUsageInfo::IsViewDefinitionUsed(DgnViewId viewDefinitionId) const
    {
    if (m_db.Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_SectionDrawing)->GetPropertyP("SpatialView") != nullptr)
        {
        Utf8CP sectionDrawingSql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_SectionDrawing) " WHERE SpatialView.Id=? LIMIT 1";
        CachedECSqlStatementPtr sectionDrawingStatement = m_db.GetPreparedECSqlStatement(sectionDrawingSql);
        sectionDrawingStatement->BindId(1, viewDefinitionId);
        if (BE_SQLITE_ROW == sectionDrawingStatement->Step())
            return true;
        }

    Utf8CP viewAttachmentSql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE View.Id=? LIMIT 1";
    CachedECSqlStatementPtr viewAttachmentStatement = m_db.GetPreparedECSqlStatement(viewAttachmentSql);
    viewAttachmentStatement->BindId(1, viewDefinitionId);
    if (BE_SQLITE_ROW == viewAttachmentStatement->Step())
        return true;

    if (m_db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SectionDrawingLocation).IsValid())
        {
        Utf8CP sectionDrawingLocationSql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_SectionDrawingLocation) " WHERE SectionView.Id=? LIMIT 1";
        CachedECSqlStatementPtr sectionDrawingLocationStatement = m_db.GetPreparedECSqlStatement(sectionDrawingLocationSql);
        sectionDrawingLocationStatement->BindId(1, viewDefinitionId);
        if (BE_SQLITE_ROW == sectionDrawingLocationStatement->Step())
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElementUsageInfo::ScanGeometryStreams(std::shared_ptr<BeSQLite::IdSet<BeInt64Id>> categoriesToScan)
    {
    ScanGeometricElement3ds(categoriesToScan);
    ScanGeometricElement2ds(categoriesToScan);
    ScanGeometryParts();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElementUsageInfo::ScanGeometricElement3ds(std::shared_ptr<BeSQLite::IdSet<BeInt64Id>> categoriesToScan)
    {
    Utf8String sql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " WHERE GeometryStream IS NOT NULL";
    if (nullptr != categoriesToScan)
        sql.append(" AND (InVirtualSet(?, Category.Id))");

    CachedECSqlStatementPtr statement = m_db.GetPreparedECSqlStatement(sql.c_str());
    if (nullptr != categoriesToScan)
        {
        statement->BindVirtualSet(1, categoriesToScan);
        }

    while (BE_SQLITE_ROW == statement->Step())
        {
        DgnElementId elementId = statement->GetValueId<DgnElementId>(0);
        ScanGeometricElement(elementId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElementUsageInfo::ScanGeometricElement2ds(std::shared_ptr<BeSQLite::IdSet<BeInt64Id>> categoriesToScan)
    {
    Utf8String sql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement2d) " WHERE GeometryStream IS NOT NULL";
    if (nullptr != categoriesToScan)
        sql.append(" AND (InVirtualSet(?, Category.Id))");

    CachedECSqlStatementPtr statement = m_db.GetPreparedECSqlStatement(sql.c_str());
    if (nullptr != categoriesToScan)
        {
        statement->BindVirtualSet(1, categoriesToScan);
        }

    while (BE_SQLITE_ROW == statement->Step())
        {
        DgnElementId elementId = statement->GetValueId<DgnElementId>(0);
        ScanGeometricElement(elementId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElementUsageInfo::ScanGeometricElement(DgnElementId elementId)
    {
    DgnElementCPtr element = m_db.Elements().GetElement(elementId);
    GeometrySourceCP geometrySource = element.IsValid() ? element->ToGeometrySource() : nullptr;
    if ((nullptr != geometrySource) && geometrySource->HasGeometry())
        ScanGeometryStream(geometrySource->GetGeometryStream());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElementUsageInfo::ScanGeometryParts()
    {
    Utf8CP sql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_GeometryPart) " WHERE GeometryStream IS NOT NULL";
    CachedECSqlStatementPtr statement = m_db.GetPreparedECSqlStatement(sql);
    while (BE_SQLITE_ROW == statement->Step())
        {
        DgnElementId elementId = statement->GetValueId<DgnElementId>(0);
        DgnGeometryPartCPtr geometryPart = m_db.Elements().Get<DgnGeometryPart>(elementId);
        if (geometryPart.IsValid())
            ScanGeometryStream(geometryPart->GetGeometryStream());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElementUsageInfo::ScanLineStyles()
    {
    Utf8CP sql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_LineStyle);
    CachedECSqlStatementPtr statement = m_db.GetPreparedECSqlStatement(sql);
    while (BE_SQLITE_ROW == statement->Step())
        {
        DgnElementId lineStyleElementId = statement->GetValueId<DgnElementId>(0);
        LineStyleElementCPtr lineStyleElement = m_db.Elements().Get<LineStyleElement>(lineStyleElementId);
        if (!lineStyleElement.IsValid())
            continue;

        Json::Value dataObj(Json::objectValue);
        if (!Json::Reader::Parse(lineStyleElement->GetData(), dataObj))
            continue;

        LsComponentId lineStyleComponentId = LsDefinition::GetComponentId(dataObj);
        LsComponentPtr lineStyleComponent = m_db.LineStyles().GetLsComponent(lineStyleComponentId);
        if (!lineStyleComponent.IsValid())
            continue;

        ScanLineStyleComponent(lineStyleComponent.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElementUsageInfo::ScanLineStyleComponent(LsComponentCP lineStyleComponent)
    {
    switch (lineStyleComponent->GetComponentType())
        {
        case LsComponentType::PointSymbol:
            {
            LsSymbolComponentCP symbolComponent = dynamic_cast<LsSymbolComponentCP>(lineStyleComponent);
            if (nullptr != symbolComponent)
                {
                DgnGeometryPartId geometryPartId(symbolComponent->GetGeometryPartId());
                if (geometryPartId.IsValid() && m_geometryPartIds.Contains(geometryPartId))
                    m_usedIds.insert(geometryPartId);
                }
            break;
            }

        case LsComponentType::LinePoint:
            {
            LsPointComponentCP pointComponent = dynamic_cast<LsPointComponentCP>(lineStyleComponent);
            if (nullptr != pointComponent)
                {
                for (uint32_t i = 0; i < pointComponent->GetNumberSymbols(); ++i)
                    {
                    DgnGeometryPartId geometryPartId(pointComponent->GetSymbolCP(i)->GetSymbolComponentCP()->GetGeometryPartId());
                    if (geometryPartId.IsValid() && m_geometryPartIds.Contains(geometryPartId))
                        m_usedIds.insert(geometryPartId);
                    }
                }
            break;
            }

        case LsComponentType::Compound:
            {
            LsCompoundComponentCP compoundComponent = dynamic_cast<LsCompoundComponentCP>(lineStyleComponent);
            if (nullptr != compoundComponent)
                {
                for (size_t i = 0; i < compoundComponent->GetNumComponents(); i++)
                    ScanLineStyleComponent(compoundComponent->GetComponentCP(i));
                }
            }
            break;

        default:
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElementUsageInfo::ScanDisplayStyles()
    {
    Utf8CP sql = "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_DisplayStyle3d);
    CachedECSqlStatementPtr statement = m_db.GetPreparedECSqlStatement(sql);
    while (BE_SQLITE_ROW == statement->Step())
        {
        DgnElementId displayStyleId = statement->GetValueId<DgnElementId>(0);
        DisplayStyle3dCPtr displayStyle = m_db.Elements().Get<DisplayStyle3d>(displayStyleId);
        if (displayStyle.IsValid())
            ScanDisplayStyle(*displayStyle);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElementUsageInfo::ScanDisplayStyle(DisplayStyle3dCR displayStyle)
    {
    displayStyle.GetEnvironmentDisplay().m_skybox.m_image.DiscloseTextureIds([&](DgnTextureId textureId)
        {
        if (textureId.IsValid() && m_textureIds.Contains(textureId))
            m_usedIds.insert(textureId);
        });
    }
