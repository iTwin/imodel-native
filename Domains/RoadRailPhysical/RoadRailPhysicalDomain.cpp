/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RoadRailPhysicalDomain.h>
#include <RoadRailPhysical/Corridor.h>
#include <RoadRailPhysical/DesignSpeed.h>
#include <RoadRailPhysical/RoadRailCategory.h>

#define INSERT_CODESPEC(x) \
    { auto codeSpecPtr = x; \
    BeAssert(codeSpecPtr.IsValid()); \
    if (codeSpecPtr.IsValid()) { \
        codeSpecPtr->Insert(); \
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid()); } \
    }

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

HANDLER_DEFINE_MEMBERS(RoadRailNetworkHandler)
DOMAIN_DEFINE_MEMBERS(RoadRailPhysicalDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRailPhysicalDomain::RoadRailPhysicalDomain() : DgnDomain(BRRP_SCHEMA_NAME, "Bentley RoadRailPhysical Domain", 2)
    {    
    RegisterHandler(RoadRailNetworkHandler::GetHandler());
    RegisterHandler(CorridorHandler::GetHandler());    
    RegisterHandler(CorridorRangeHandler::GetHandler());

    RegisterHandler(CorridorPortionElementHandler::GetHandler());
    RegisterHandler(PathwayElementHandler::GetHandler());
    RegisterHandler(RailwayHandler::GetHandler());
    RegisterHandler(RoadwayHandler::GetHandler());    

    RegisterHandler(PathwayDesignCriteriaHandler::GetHandler());
    RegisterHandler(DesignSpeedDefinitionHandler::GetHandler());
    RegisterHandler(DesignSpeedHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus createRoadwayStandardsPartition(SubjectCR subject)
    {
    DgnDbStatus status;

    auto roadwayStandardsPartitionPtr = DefinitionPartition::Create(subject, RoadRailPhysicalDomain::GetRoadwayStandardsPartitionName());
    if (roadwayStandardsPartitionPtr->Insert(&status).IsNull())
        return status;

    auto roadwayStandardsModelPtr = DefinitionModel::Create(*roadwayStandardsPartitionPtr);

    if (DgnDbStatus::Success != (status = roadwayStandardsModelPtr->Insert()))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus createRailwayStandardsPartition(SubjectCR subject)
    {
    DgnDbStatus status;

    auto railwayStandardsPartitionPtr = DefinitionPartition::Create(subject, RoadRailPhysicalDomain::GetRailwayStandardsPartitionName());
    if (railwayStandardsPartitionPtr->Insert(&status).IsNull())
        return status;

    auto railwayStandardsModelPtr = DefinitionModel::Create(*railwayStandardsPartitionPtr);

    if (DgnDbStatus::Success != (status = railwayStandardsModelPtr->Insert()))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RoadRailPhysicalDomain::SetUpDefinitionPartitions(SubjectCR subject)
    {
    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = createRailwayStandardsPartition(subject)))
        return status;

    if (DgnDbStatus::Success != (status = createRoadwayStandardsPartition(subject)))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void createCodeSpecs(DgnDbR dgndb)
    {
    INSERT_CODESPEC(CodeSpec::Create(dgndb, BRRP_CODESPEC_RoadRailNetwork, CodeScopeSpec::CreateModelScope()));
    INSERT_CODESPEC(CodeSpec::Create(dgndb, BRRP_CODESPEC_Corridor, CodeScopeSpec::CreateModelScope()));
    INSERT_CODESPEC(CodeSpec::Create(dgndb, BRRP_CODESPEC_CorridorRange, CodeScopeSpec::CreateModelScope()));
    INSERT_CODESPEC(CodeSpec::Create(dgndb, BRRP_CODESPEC_Pathway, CodeScopeSpec::CreateModelScope()));
    INSERT_CODESPEC(CodeSpec::Create(dgndb, BRRP_CODESPEC_DesignSpeedDefinition, CodeScopeSpec::CreateModelScope()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailPhysicalDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    createCodeSpecs(dgndb);
    RoadRailCategory::InsertDomainCategories(dgndb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
CategorySelectorPtr getSpatialCategorySelector(DefinitionModelR model, bvector<DgnCategoryId> const* additionalCategories)
    {
    Utf8String selectorName = "Default Spatial Road/Rail Categories";
    auto selectorId = model.GetDgnDb().Elements().QueryElementIdByCode(CategorySelector::CreateCode(model, selectorName));
    auto selectorPtr = model.GetDgnDb().Elements().GetForEdit<CategorySelector>(selectorId);
    if (selectorPtr.IsValid())
        return selectorPtr;

    selectorPtr = new CategorySelector(model, selectorName);
    selectorPtr->AddCategory(AlignmentCategory::GetAlignment(model.GetDgnDb()));
    selectorPtr->AddCategory(AlignmentCategory::GetLinear(model.GetDgnDb()));
    selectorPtr->AddCategory(RoadRailCategory::GetCorridor(model.GetDgnDb()));
    selectorPtr->AddCategory(RoadRailCategory::GetRoadway(model.GetDgnDb()));
    selectorPtr->AddCategory(RoadRailCategory::GetRailway(model.GetDgnDb()));

    if (additionalCategories)
        {
        for (auto categoryId : *additionalCategories)
            selectorPtr->AddCategory(categoryId);
        }

    return selectorPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
CategorySelectorPtr getDrawingCategorySelector(DefinitionModelR model)
    {
    Utf8String selectorName = "Default Drawing Road/Rail Categories";
    auto selectorId = model.GetDgnDb().Elements().QueryElementIdByCode(CategorySelector::CreateCode(model, selectorName));
    auto selectorPtr = model.GetDgnDb().Elements().GetForEdit<CategorySelector>(selectorId);
    if (selectorPtr.IsValid())
        return selectorPtr;

    selectorPtr = new CategorySelector(model, selectorName);
    selectorPtr->AddCategory(AlignmentCategory::GetVertical(model.GetDgnDb()));
    return selectorPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
ModelSelectorPtr getModelSelector(DefinitionModelR definitionModel, Utf8StringCR name)
    {
    return new ModelSelector(definitionModel, name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DisplayStyle2dPtr getDisplayStyle2d(DefinitionModelR model)
    {
    Utf8String styleName = "Default 2D Style - Road/Rail";
    auto styleId = model.GetDgnDb().Elements().QueryElementIdByCode(DisplayStyle2d::CreateCode(model, styleName));
    auto stylePtr = model.GetDgnDb().Elements().GetForEdit<DisplayStyle2d>(styleId);
    if (stylePtr.IsValid())
        return stylePtr;

    stylePtr = new DisplayStyle2d(model, styleName);
    stylePtr->SetBackgroundColor(ColorDef::White());
    return stylePtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DisplayStyle3dPtr getDisplayStyle3d(DefinitionModelR model)
    {
    Utf8String styleName = "Default 3D Style - Road/Rail";
    auto styleId = model.GetDgnDb().Elements().QueryElementIdByCode(DisplayStyle3d::CreateCode(model, styleName));
    auto stylePtr = model.GetDgnDb().Elements().GetForEdit<DisplayStyle3d>(styleId);
    if (stylePtr.IsValid())
        return stylePtr;

    stylePtr = new DisplayStyle3d(model, "Default 3D Style - Road/Rail");
    stylePtr->SetBackgroundColor(ColorDef::Black());
    stylePtr->SetSkyBoxEnabled(false);
    stylePtr->SetGroundPlaneEnabled(false);
    Render::ViewFlags viewFlags = stylePtr->GetViewFlags();
    viewFlags.SetRenderMode(Render::RenderMode::SmoothShade);
    viewFlags.SetMonochrome(false);
    viewFlags.SetShowFill(true);
    viewFlags.SetShowMaterials(true);
    viewFlags.SetShowPatterns(true);
    viewFlags.SetShowShadows(true);
    stylePtr->SetViewFlags(viewFlags);
    return stylePtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
DgnViewId create3dView(DefinitionModelR model, Utf8CP viewName,
    CategorySelectorR categorySelector, ModelSelectorR modelSelector, DisplayStyle3dR displayStyle)
    {
    DgnDbR db = model.GetDgnDb();

    OrthographicViewDefinition view(model, viewName, categorySelector, displayStyle, modelSelector);

    DgnViewId viewId;
    DgnViewId existingViewId = ViewDefinition::QueryViewId(db, view.GetCode());
    if (existingViewId.IsValid())
        viewId = existingViewId;
    else
        {
        view.SetStandardViewRotation(StandardView::Top);
        view.LookAtVolume(db.GeoLocation().GetProjectExtents());        

        if (!view.Insert().IsValid())
            return viewId;

        viewId = view.GetViewId();
        }

    if (!viewId.IsValid())
        return viewId;

    DgnViewId defaultViewId;
    if (db.QueryProperty(&defaultViewId, sizeof(defaultViewId), DgnViewProperty::DefaultView()) != BeSQLite::DbResult::BE_SQLITE_ROW)
        db.SaveProperty(DgnViewProperty::DefaultView(), &viewId, (uint32_t) sizeof(viewId));

    return viewId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus create2dView(DefinitionModelR model, Utf8CP viewName,
    CategorySelectorR categorySelector, DgnModelId modelToDisplay, DisplayStyle2dR displayStyle)
    {
    DgnDbR db = model.GetDgnDb();

    DrawingViewDefinition view(model, viewName, modelToDisplay, categorySelector, displayStyle);

    DgnViewId viewId;
    DgnViewId existingViewId = ViewDefinition::QueryViewId(db, view.GetCode());
    if (existingViewId.IsValid())
        viewId = existingViewId;
    else
        {
        view.SetStandardViewRotation(StandardView::Top);
        view.LookAtVolume(db.GeoLocation().GetProjectExtents());

        if (!view.Insert().IsValid())
            return BentleyStatus::ERROR;

        viewId = view.GetViewId();
        }

    if (!viewId.IsValid())
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId RoadRailPhysicalDomain::SetUpDefaultViews(SubjectCR subject, PhysicalModelR physicalNetworkModel, bvector<DgnCategoryId> const* additionalCategoriesForSelector)
    {
    auto& dgnDb = subject.GetDgnDb();

    auto configurationModelPtr = RoadRailAlignmentDomain::QueryConfigurationModel(subject);
    
    auto displayStyle3dPtr = getDisplayStyle3d(*configurationModelPtr);
    auto spatialCategorySelectorPtr = getSpatialCategorySelector(*configurationModelPtr, additionalCategoriesForSelector);
    auto model3dSelectorPtr = getModelSelector(*configurationModelPtr, "3D - Road/Rail");
    model3dSelectorPtr->AddModel(physicalNetworkModel.GetModelId());

    return create3dView(*configurationModelPtr, "3D - Road/Rail", *spatialCategorySelectorPtr, *model3dSelectorPtr, *displayStyle3dPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RoadRailNetwork::CreateCode(PhysicalModelCR scopeModel, Utf8StringCR networkCode)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_RoadRailNetwork, scopeModel, networkCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRailNetworkCPtr RoadRailNetwork::Insert(PhysicalModelR parentModel, Utf8StringCR networkName)
    {
    if (!parentModel.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(parentModel.GetDgnDb(), parentModel.GetModelId(), QueryClassId(parentModel.GetDgnDb()),
        RoadRailCategory::GetCorridor(parentModel.GetDgnDb()));
    createParams.m_code = CreateCode(parentModel, networkName);

    RoadRailNetworkPtr newPtr(new RoadRailNetwork(createParams));
    auto networkCPtr = parentModel.GetDgnDb().Elements().Insert<RoadRailNetwork>(*newPtr);
    if (networkCPtr.IsNull())
        return nullptr;

    auto networkPhysicalModelPtr = PhysicalModel::Create(*networkCPtr);
    if (networkPhysicalModelPtr.IsValid())
        {
        if (DgnDbStatus::Success != networkPhysicalModelPtr->Insert())
            return nullptr;
        }

    return networkCPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalPartitionCPtr PhysicalModelUtilities::CreateAndInsertPhysicalPartitionAndModel(SubjectCR subject, Utf8CP physicalPartitionName)
    {
    DgnDbStatus status;

    auto physicalPartitionPtr = PhysicalPartition::Create(subject, physicalPartitionName);
    auto physicalPartitionCPtr = physicalPartitionPtr->Insert(&status);
    if (physicalPartitionCPtr.IsNull())
        return nullptr;

    auto& physicalModelHandlerR = dgn_ModelHandler::Physical::GetHandler();
    auto physicalModelPtr = physicalModelHandlerR.Create(DgnModel::CreateParams(subject.GetDgnDb(), subject.GetDgnDb().Domains().GetClassId(physicalModelHandlerR),
        physicalPartitionCPtr->GetElementId()));

    if (DgnDbStatus::Success != (status = physicalModelPtr->Insert()))
        return nullptr;

    return dynamic_cast<PhysicalPartitionCP>(physicalPartitionCPtr.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet PhysicalModelUtilities::QueryPhysicalPartitions(SubjectCR subject)
    {
    ECSqlStatement stmt;
    stmt.Prepare(subject.GetDgnDb(), "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_PhysicalPartition)
        " WHERE Parent.Id = ?;");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, subject.GetElementId());

    DgnElementIdSet retVal;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        retVal.insert(stmt.GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
-* @bsimethod                                    Diego.Diaz                      06/2017
-+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr PhysicalModelUtilities::QueryPhysicalNetworkModel(SubjectCR parentSubject, Utf8CP physicalPartitionName, Utf8CP roadRailNetworkName)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = PhysicalPartition::CreateCode(parentSubject, physicalPartitionName);
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    PhysicalPartitionCPtr partition = db.Elements().Get<PhysicalPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;

    auto parentModelCPtr = partition->GetSubModel()->ToPhysicalModel();
    DgnCode networkCode = RoadRailNetwork::CreateCode(*parentModelCPtr, roadRailNetworkName);
    DgnElementId networkId = db.Elements().QueryElementIdByCode(networkCode);
    auto networkCPtr = RoadRailNetwork::Get(db, networkId);
    if (!networkCPtr.IsValid())
        return nullptr;

    return networkCPtr->GetSubModel()->ToPhysicalModelP();
    }

/*---------------------------------------------------------------------------------**//**
-* @bsimethod                                    Diego.Diaz                      04/2018
-+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr PhysicalModelUtilities::GetParentSubject(PhysicalModelCR model)
    {
    auto partitionCP = dynamic_cast<PhysicalPartitionCP>(model.GetModeledElement().get());
    BeAssert(partitionCP != nullptr);

    return model.GetDgnDb().Elements().Get<Subject>(partitionCP->GetParentId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionModelCPtr RoadwayStandardsModelUtilities::Query(SubjectCR parentSubject)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = DefinitionPartition::CreateCode(parentSubject, RoadRailPhysicalDomain::GetRoadwayStandardsPartitionName());
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    DefinitionPartitionCPtr partition = db.Elements().Get<DefinitionPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return partition->GetSubModel()->ToDefinitionModel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionModelCPtr RailwayStandardsModelUtilities::Query(SubjectCR parentSubject)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = DefinitionPartition::CreateCode(parentSubject, RoadRailPhysicalDomain::GetRailwayStandardsPartitionName());
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    DefinitionPartitionCPtr partition = db.Elements().Get<DefinitionPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return partition->GetSubModel()->ToDefinitionModel();
    }

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE