/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RoadRailPhysicalDomain.h>
#include <RoadRailPhysical/ElementAspects.h>
#include <RoadRailPhysical/Pathway.h>
#include <RoadRailPhysical/RoadClass.h>
#include <RoadRailPhysical/RoadDesignSpeed.h>
#include <RoadRailPhysical/RoadRailCategory.h>
#include <RoadRailPhysical/RoadSegment.h>
#include <RoadRailPhysical/TravelwaySegment.h>
#include <RoadRailPhysical/TravelwaySideSegment.h>
#include <RoadRailPhysical/TravelwayStructureSegment.h>
#include <RoadRailPhysical/TypicalSection.h>
#include <RoadRailPhysical/TypicalSectionPoint.h>

HANDLER_DEFINE_MEMBERS(RoadRailNetworkHandler)
DOMAIN_DEFINE_MEMBERS(RoadRailPhysicalDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRailPhysicalDomain::RoadRailPhysicalDomain() : DgnDomain(BRRP_SCHEMA_NAME, "Bentley RoadRailPhysical Domain", 1)
    {    
    RegisterHandler(RoadRailNetworkHandler::GetHandler());
    RegisterHandler(CorridorHandler::GetHandler());    

    RegisterHandler(TypicalSectionPointDefinitionHandler::GetHandler());
    RegisterHandler(GenericTypicalSectionPointDefinitionHandler::GetHandler());
    
    RegisterHandler(RailwayStandardsModelHandler::GetHandler());
    RegisterHandler(RoadwayStandardsModelHandler::GetHandler());

    RegisterHandler(DesignSpeedDefinitionHandler::GetHandler());
    RegisterHandler(DesignSpeedElementHandler::GetHandler());
    RegisterHandler(DesignSpeedHandler::GetHandler());
    RegisterHandler(DesignSpeedTransitionHandler::GetHandler());

    RegisterHandler(CorridorPortionElementHandler::GetHandler());
    RegisterHandler(PathwayElementHandler::GetHandler());
    RegisterHandler(RailwayHandler::GetHandler());
    RegisterHandler(RoadwayHandler::GetHandler());    

    RegisterHandler(AssociatedFacetAspectHandler::GetHandler());
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

    auto roadwayStandardsModelPtr = RoadwayStandardsModel::Create(
        RoadwayStandardsModel::CreateParams(subject.GetDgnDb(), roadwayStandardsPartitionPtr->GetElementId()));

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

    auto railwayStandardsModelPtr = RailwayStandardsModel::Create(
        RailwayStandardsModel::CreateParams(subject.GetDgnDb(), railwayStandardsPartitionPtr->GetElementId()));

    if (DgnDbStatus::Success != (status = railwayStandardsModelPtr->Insert()))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RoadRailPhysicalDomain::SetUpModelHierarchy(SubjectCR subject, Utf8CP physicalPartitionName, Utf8CP networkName)
    {
    auto physicalPartitionId = subject.GetDgnDb().Elements().QueryElementIdByCode(PhysicalPartition::CreateCode(subject, physicalPartitionName));
    if (!physicalPartitionId.IsValid())
        return DgnDbStatus::BadArg;

    auto physicalPartitionCPtr = subject.GetDgnDb().Elements().Get<PhysicalPartition>(physicalPartitionId);
    auto physicalModelP = physicalPartitionCPtr->GetSubModel()->ToPhysicalModelP();
    if (!physicalModelP)
        return DgnDbStatus::BadModel;

    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = createRailwayStandardsPartition(subject)))
        return status;

    if (DgnDbStatus::Success != (status = createRoadwayStandardsPartition(subject)))
        return status;

    PhysicalModelPtr breakDownModelPtr;
    if (RoadRailNetwork::Insert(*physicalModelP, RoadRailNetwork::CreateCode(*physicalModelP, networkName), breakDownModelPtr).IsNull() ||
        breakDownModelPtr.IsNull())
        return DgnDbStatus::WriteError;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void createCodeSpecs(DgnDbR dgndb)
    {
    auto codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_RoadRailNetwork, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_Corridor, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_Pathway, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_TypicalSectionPointDefinition, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_DesignSpeedDefinition, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }
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
CategorySelectorPtr getSpatialCategorySelector(ConfigurationModelR model, bvector<DgnCategoryId> const* additionalCategories)
    {
    Utf8String selectorName = "Default Spatial Road/Rail Categories";
    auto selectorId = model.GetDgnDb().Elements().QueryElementIdByCode(CategorySelector::CreateCode(model, selectorName));
    auto selectorPtr = model.GetDgnDb().Elements().GetForEdit<CategorySelector>(selectorId);
    if (selectorPtr.IsValid())
        return selectorPtr;

    selectorPtr = new CategorySelector(model, selectorName);
    selectorPtr->AddCategory(AlignmentCategory::GetAlignment(model.GetDgnDb()));
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
CategorySelectorPtr getDrawingCategorySelector(ConfigurationModelR model)
    {
    Utf8String selectorName = "Default Drawing Road/Rail Categories";
    auto selectorId = model.GetDgnDb().Elements().QueryElementIdByCode(CategorySelector::CreateCode(model, selectorName));
    auto selectorPtr = model.GetDgnDb().Elements().GetForEdit<CategorySelector>(selectorId);
    if (selectorPtr.IsValid())
        return selectorPtr;

    selectorPtr = new CategorySelector(model, selectorName);
    selectorPtr->AddCategory(AlignmentCategory::GetHorizontal(model.GetDgnDb()));
    selectorPtr->AddCategory(AlignmentCategory::GetVertical(model.GetDgnDb()));
    return selectorPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
ModelSelectorPtr getModelSelector(ConfigurationModelR definitionModel, Utf8StringCR name)
    {
    return new ModelSelector(definitionModel, name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DisplayStyle2dPtr getDisplayStyle2d(ConfigurationModelR model)
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
DisplayStyle3dPtr getDisplayStyle3d(ConfigurationModelR model)
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
DgnViewId create3dView(ConfigurationModelR model, Utf8CP viewName,
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
BentleyStatus create2dView(ConfigurationModelR model, Utf8CP viewName,
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
    // unused - auto& dgnDb = subject.GetDgnDb();

    auto configurationModelPtr = ConfigurationModel::Query(subject);
    
    auto displayStyle3dPtr = getDisplayStyle3d(*configurationModelPtr);
    auto spatialCategorySelectorPtr = getSpatialCategorySelector(*configurationModelPtr, additionalCategoriesForSelector);
    auto model3dSelectorPtr = getModelSelector(*configurationModelPtr, "3D - Road/Rail");
    model3dSelectorPtr->AddModel(physicalNetworkModel.GetModelId());

    return create3dView(*configurationModelPtr, "3D - Road/Rail", *spatialCategorySelectorPtr, *model3dSelectorPtr, *displayStyle3dPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RoadRailPhysicalDomain::SetGeometricElementAsBoundingContentForSheet(GeometricElementCR boundingElm, Sheet::ElementCR sheet)
    {
    if (!boundingElm.GetElementId().IsValid() || !sheet.GetElementId().IsValid())
        return DgnDbStatus::BadArg;

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != boundingElm.GetDgnDb().InsertLinkTableRelationship(insKey,
        *boundingElm.GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, BRRP_REL_GeometricElementBoundsContentForSheet)->GetRelationshipClassCP(),
        ECInstanceId(boundingElm.GetElementId().GetValue()), ECInstanceId(sheet.GetElementId().GetValue())))
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet RoadRailPhysicalDomain::QueryElementIdsBoundingContentForSheets(DgnDbCR dgnDb)
    {
    auto stmtPtr = dgnDb.GetPreparedECSqlStatement("SELECT DISTINCT SourceECInstanceId FROM "
        BRRP_SCHEMA(BRRP_REL_GeometricElementBoundsContentForSheet) " WHERE TargetECInstanceId NOT IS NULL;");
    BeAssert(stmtPtr.IsValid());

    DgnElementIdSet retVal;
    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        retVal.insert(stmtPtr->GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet RoadRailPhysicalDomain::QuerySheetIdsBoundedBy(GeometricElementCR boundingElm)
    {
    auto stmtPtr = boundingElm.GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM "
        BRRP_SCHEMA(BRRP_REL_GeometricElementBoundsContentForSheet) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, boundingElm.GetElementId());

    DgnElementIdSet retVal;
    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        retVal.insert(stmtPtr->GetValueId<DgnElementId>(0));

    return retVal;
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
RoadRailNetworkCPtr RoadRailNetwork::Insert(PhysicalModelR parentModel, DgnCodeCR networkCode, PhysicalModelPtr& breakDownModelPtr)
    {
    if (!parentModel.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(parentModel.GetDgnDb(), parentModel.GetModelId(), QueryClassId(parentModel.GetDgnDb()),
        RoadRailCategory::GetCorridor(parentModel.GetDgnDb()));
    createParams.m_code = networkCode;

    RoadRailNetworkPtr newPtr(new RoadRailNetwork(createParams));
    auto networkCPtr = parentModel.GetDgnDb().Elements().Insert<RoadRailNetwork>(*newPtr);
    if (networkCPtr.IsNull())
        return nullptr;

    auto& physicalModelHandlerR = dgn_ModelHandler::Physical::GetHandler();
    auto newDgnModelPtr = physicalModelHandlerR.Create(DgnModel::CreateParams(parentModel.GetDgnDb(), parentModel.GetDgnDb().Domains().GetClassId(physicalModelHandlerR),
        networkCPtr->GetElementId()));

    if (newDgnModelPtr.IsValid())
        {
        if (DgnDbStatus::Success == newDgnModelPtr->Insert())
            breakDownModelPtr = newDgnModelPtr->ToPhysicalModelP();
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