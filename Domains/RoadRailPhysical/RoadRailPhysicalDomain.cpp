/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadRailPhysicalDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
#include <RoadRailPhysical/TypicalSection.h>
#include <RoadRailPhysical/TypicalSectionPoint.h>

DOMAIN_DEFINE_MEMBERS(RoadRailPhysicalDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRailPhysicalDomain::RoadRailPhysicalDomain() : DgnDomain(BRRP_SCHEMA_NAME, "Bentley RoadRailPhysical Domain", 1)
    {    
    RegisterHandler(RoadRailCategoryModelHandler::GetHandler());

    RegisterHandler(OverallTypicalSectionBreakDownModelHandler::GetHandler());
    RegisterHandler(TypicalSectionPortionBreakDownModelHandler::GetHandler());

    RegisterHandler(TypicalSectionPortionDefinitionElementHandler::GetHandler());
    RegisterHandler(OverallTypicalSectionHandler::GetHandler());
    RegisterHandler(TravelwayDefinitionElementHandler::GetHandler());
    RegisterHandler(RoadTravelwayDefinitionHandler::GetHandler());
    RegisterHandler(TravelwaySideDefinitionHandler::GetHandler());
    RegisterHandler(TravelwayStructureDefinitionHandler::GetHandler());

    RegisterHandler(TypicalSectionComponentElementHandler::GetHandler());
    RegisterHandler(TravelwayStructureComponentElementHandler::GetHandler());
    RegisterHandler(TravelwaySideComponentElementHandler::GetHandler());
    RegisterHandler(TravelwayComponentElementHandler::GetHandler());
    RegisterHandler(BarrierComponentHandler::GetHandler());
    RegisterHandler(BufferComponentHandler::GetHandler());
    RegisterHandler(CurbComponentHandler::GetHandler());
    RegisterHandler(RoadLaneComponentHandler::GetHandler());
    RegisterHandler(RoadShoulderComponentHandler::GetHandler());
    RegisterHandler(PavementComponentHandler::GetHandler());

    RegisterHandler(TypicalSectionPointNameHandler::GetHandler());
    RegisterHandler(TypicalSectionPointHandler::GetHandler());
    RegisterHandler(TypicalSectionPointPlaceHolderHandler::GetHandler());
    RegisterHandler(TypicalSectionConstraintSourceHandler::GetHandler());
    RegisterHandler(TypicalSectionConstraintWithOffsetHandler::GetHandler());
    RegisterHandler(TypicalSectionHorizontalConstraintHandler::GetHandler());
    RegisterHandler(TypicalSectionVerticalConstraintHandler::GetHandler());
    RegisterHandler(TypicalSectionSlopeConstraintHandler::GetHandler());
    RegisterHandler(TypicalSectionConstraintOffsetHandler::GetHandler());
    RegisterHandler(TypicalSectionConstraintConstantOffsetHandler::GetHandler());
    RegisterHandler(TypicalSectionConstraintSlopeHandler::GetHandler());
    RegisterHandler(TypicalSectionConstraintConstantSlopeHandler::GetHandler());

    RegisterHandler(PathwayElementHandler::GetHandler());
    RegisterHandler(TravelwaySegmentElementHandler::GetHandler());
    RegisterHandler(RegularTravelwaySegmentHandler::GetHandler());
    RegisterHandler(TravelwayTransitionHandler::GetHandler());
    RegisterHandler(TravelwayIntersectionSegmentElementHandler::GetHandler());
    
    RegisterHandler(RoadwayStandardsModelHandler::GetHandler());
    RegisterHandler(RoadClassDefinitionTableHandler::GetHandler());
    RegisterHandler(RoadClassDefinitionModelHandler::GetHandler());
    RegisterHandler(RoadClassDefinitionHandler::GetHandler());
    RegisterHandler(RoadClassHandler::GetHandler());

    RegisterHandler(DesignSpeedDefinitionTableHandler::GetHandler());
    RegisterHandler(DesignSpeedDefinitionModelHandler::GetHandler());
    RegisterHandler(DesignSpeedDefinitionElementHandler::GetHandler());
    RegisterHandler(RoadDesignSpeedDefinitionHandler::GetHandler());
    RegisterHandler(DesignSpeedHandler::GetHandler());
    
    RegisterHandler(RailwayHandler::GetHandler());
    RegisterHandler(RoadwayHandler::GetHandler());    
    RegisterHandler(RoadIntersectionLegElementHandler::GetHandler());    

    RegisterHandler(LinearlyLocatedStatusHandler::GetHandler());
    RegisterHandler(StatusAspectHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus createPhysicalPartition(SubjectCR subject, Utf8CP physicalPartitionName)
    {
    DgnDbStatus status;

    auto physicalPartitionPtr = PhysicalPartition::Create(subject, physicalPartitionName);
    if (physicalPartitionPtr->Insert(&status).IsNull())
        return status;

    auto& physicalModelHandlerR = dgn_ModelHandler::Physical::GetHandler();
    auto physicalModelPtr = physicalModelHandlerR.Create(DgnModel::CreateParams(subject.GetDgnDb(), subject.GetDgnDb().Domains().GetClassId(physicalModelHandlerR),
        physicalPartitionPtr->GetElementId()));

    if (DgnDbStatus::Success != (status = physicalModelPtr->Insert()))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr RoadRailPhysicalDomain::QueryPhysicalModel(Dgn::SubjectCR parentSubject, Utf8CP modelName)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = PhysicalPartition::CreateCode(parentSubject, modelName);
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    PhysicalPartitionCPtr partition = db.Elements().Get<PhysicalPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus createRoadwayStandardsPartition(SubjectCR subject)
    {
    DgnDbStatus status;

    auto roadwayStandardsPartitionPtr = DefinitionPartition::Create(subject, "Roadway Standards");
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
DgnDbStatus RoadRailPhysicalDomain::SetUpModelHierarchy(Dgn::SubjectCR subject, Utf8CP physicalPartitionName)
    {
    DgnDbStatus status;

    if (DgnDbStatus::Success != (status = createRoadwayStandardsPartition(subject)))

        return status;

    if (DgnDbStatus::Success != (status = createPhysicalPartition(subject, physicalPartitionName)))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void createCodeSpecs(DgnDbR dgndb)
    {
    auto codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_RoadTravelway, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_OverallTypicalSection, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_TypicalSectionParameter, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_TypicalSectionPointName, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_TypicalSectionPoint, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        auto sequenceSpec = CodeFragmentSpec::FromSequence();
        sequenceSpec.SetStartNumber(1);
        codeSpecPtr->GetFragmentSpecsR().push_back(sequenceSpec);
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_RoadClassDefinitionTable, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_RoadClassDefinition, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_DesignSpeedDefinitionTable, CodeScopeSpec::CreateModelScope());
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
    RoadRailCategoryModel::SetUp(dgndb);

    DgnDbStatus status = SetUpModelHierarchy(*dgndb.Elements().GetRootSubject(), GetDefaultPhysicalPartitionName());
    if (DgnDbStatus::Success != status)
        {
        BeAssert(false);
        }

    createCodeSpecs(dgndb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
CategorySelectorPtr getSpatialCategorySelector(DefinitionModelR model)
    {
    Utf8String selectorName = "Default Spatial Road/Rail Categories";
    auto selectorId = model.GetDgnDb().Elements().QueryElementIdByCode(CategorySelector::CreateCode(model, selectorName));
    auto selectorPtr = model.GetDgnDb().Elements().GetForEdit<CategorySelector>(selectorId);
    if (selectorPtr.IsValid())
        return selectorPtr;

    selectorPtr = new CategorySelector(model, selectorName);
    selectorPtr->AddCategory(AlignmentCategory::Get(model.GetDgnDb()));
    selectorPtr->AddCategory(RoadRailCategory::GetRoad(model.GetDgnDb()));
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
    selectorPtr->AddCategory(AlignmentCategory::GetHorizontal(model.GetDgnDb()));
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
DisplayStyle2dPtr getDisplayStyle2d(Dgn::DefinitionModelR model)
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
    stylePtr->SetBackgroundColor(ColorDef::White());
    Render::ViewFlags viewFlags = stylePtr->GetViewFlags();
    viewFlags.SetRenderMode(Render::RenderMode::SmoothShade);
    stylePtr->SetViewFlags(viewFlags);
    return stylePtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus create3dView(DefinitionModelR model, Utf8CP viewName,
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
            return BentleyStatus::ERROR;

        viewId = view.GetViewId();
        }

    if (!viewId.IsValid())
        return BentleyStatus::ERROR;

    DgnViewId defaultViewId;
    if (db.QueryProperty(&defaultViewId, sizeof(defaultViewId), DgnViewProperty::DefaultView()) != BeSQLite::DbResult::BE_SQLITE_ROW)
        db.SaveProperty(DgnViewProperty::DefaultView(), &viewId, (uint32_t) sizeof(viewId));

    return BentleyStatus::SUCCESS;
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
DgnDbStatus RoadRailPhysicalDomain::SetUpDefaultViews(SubjectCR subject, Utf8CP alignmentPartitionName, Utf8CP physicalPartitionName)
    {
    auto& dgnDb = subject.GetDgnDb();

    auto alignmentModelPtr =
        AlignmentModel::Query(subject, (alignmentPartitionName) ? alignmentPartitionName : RoadRailAlignmentDomain::GetDefaultPartitionName());
    auto physicalModelPtr = RoadRailPhysicalDomain::QueryPhysicalModel(subject, (physicalPartitionName) ? physicalPartitionName :
        RoadRailPhysicalDomain::GetDefaultPhysicalPartitionName());

    auto& subjectName = subject.GetCode().GetValue();

#ifndef NDEBUG
    Utf8String view2dName = "2D - " + subjectName.GetUtf8();
    auto displayStyle2dPtr = getDisplayStyle2d(dgnDb.GetDictionaryModel());
    auto drawingCategorySelectorPtr = getDrawingCategorySelector(dgnDb.GetDictionaryModel());
    create2dView(dgnDb.GetDictionaryModel(), view2dName.c_str(), *drawingCategorySelectorPtr,
        HorizontalAlignmentModel::QueryBreakDownModelId(*alignmentModelPtr), *displayStyle2dPtr);
#endif

    auto displayStyle3dPtr = getDisplayStyle3d(dgnDb.GetDictionaryModel());
    auto spatialCategorySelectorPtr = getSpatialCategorySelector(dgnDb.GetDictionaryModel());
    auto model3dSelectorPtr = getModelSelector(dgnDb.GetDictionaryModel(), subjectName.GetUtf8());
    model3dSelectorPtr->AddModel(alignmentModelPtr->GetModelId());
    model3dSelectorPtr->AddModel(physicalModelPtr->GetModelId());

    auto& view3dName = subjectName;
    create3dView(dgnDb.GetDictionaryModel(), view3dName.GetUtf8CP(),
        *spatialCategorySelectorPtr, *model3dSelectorPtr, *displayStyle3dPtr);

    return DgnDbStatus::Success;
    }