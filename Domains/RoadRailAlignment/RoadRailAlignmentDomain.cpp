/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/RoadRailAlignmentDomain.h>
#include <RoadRailAlignment/AlignmentCategory.h>
#include <RoadRailAlignment/Alignment.h>
#include <RoadRailAlignment/AlignmentReferent.h>
//#include <RoadRailAlignment/AlignmentProfileViewDefinition.h>
//#include <RoadRailAlignment/ClipPlanesViewDefinition.h>

#define DEFAULT_VIEWDEF_ASPECT_RATIO_SKEW 10.0 // For Profile and XS view definitions

DOMAIN_DEFINE_MEMBERS(RoadRailAlignmentDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRailAlignmentDomain::RoadRailAlignmentDomain() : DgnDomain(BRRA_SCHEMA_NAME, "Bentley RoadRailAlignment Domain", 2)
    {
    RegisterHandler(AlignmentHandler::GetHandler());
    RegisterHandler(DesignAlignmentsHandler::GetHandler());
    RegisterHandler(HorizontalAlignmentsHandler::GetHandler());
    RegisterHandler(HorizontalAlignmentHandler::GetHandler());    
    //RegisterHandler(AlignmentProfileViewDefinitionHandler::GetHandler());
    RegisterHandler(AlignmentStationHandler::GetHandler());
    //RegisterHandler(ClipPlanesViewDefinitionHandler::GetHandler());
    RegisterHandler(VerticalAlignmentModelHandler::GetHandler());
    RegisterHandler(VerticalAlignmentHandler::GetHandler());    
    }

BEGIN_UNNAMED_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        09/2017
// Turn on all public categories for a given CategorySelector
//---------------------------------------------------------------------------------------
void setupCategorySelector(CategorySelectorR cSelector)
    {
    ElementIterator catIt = cSelector.GetDgnDb().Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialCategory), "WHERE IsPrivate=FALSE");
    const bvector<DgnCategoryId> categoryIds = catIt.BuildIdList<DgnCategoryId>();
    for (auto const& catId : categoryIds)
        {
        cSelector.ChangeCategoryDisplay(catId, true);
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        09/2017
//---------------------------------------------------------------------------------------
void setupDisplayStyle(DisplayStyle3dR dStyle)
    {
    dStyle.SetBackgroundColor(ColorDef::Black());

    auto& env = dStyle.GetEnvironmentDisplayR();
    env.GetGroundPlane().m_enabled = false;
    env.GetSkyBox().m_enabled = false;

    Render::ViewFlags flags;
    flags.SetRenderMode(Render::RenderMode::SmoothShade);
    flags.SetShowCameraLights(false);
    flags.SetShowSourceLights(false);
    flags.SetShowSolarLight(false);
    flags.SetShowShadows(false);
    flags.SetShowVisibleEdges(true);
    flags.SetShowHiddenEdges(true);
    dStyle.SetViewFlags(flags);
    dStyle.SetIsPrivate(true);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        09/2017
// Turn on all public models for a given ModelSelector
//---------------------------------------------------------------------------------------
void setupModelSelector(ModelSelectorR mSelector)
    {
    ModelIterator modelIt = mSelector.GetDgnDb().Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalModel), "WHERE IsPrivate=FALSE");
    const bvector<DgnModelId> modelIds = modelIt.BuildIdList();
    for (auto const& modelId : modelIds)
        {
        mSelector.AddModel(modelId);
        }
    mSelector.SetIsPrivate(true);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        09/2017
//---------------------------------------------------------------------------------------
/*template <typename VIEWDEF_T>
RefCountedPtr<VIEWDEF_T> createViewDefinition(ConfigurationModelR model, Utf8StringCR viewName)
    {
    Utf8String cSelectorName = viewName;
    cSelectorName.append("CategorySelector");

    Utf8String dStyleName = viewName;
    dStyleName.append("DisplayStyle");

    Utf8String mSelectorName = viewName;
    mSelectorName.append("ModelSelector");

    CategorySelectorPtr cSelector = new CategorySelector(model, cSelectorName);
    DisplayStyle3dPtr dStyle = new DisplayStyle3d(model, dStyleName);
    ModelSelectorPtr mSelector = new ModelSelector(model, mSelectorName);
    if (!cSelector.IsValid() || !dStyle.IsValid() || !mSelector.IsValid())
        return nullptr;

    setupCategorySelector(*cSelector);
    setupDisplayStyle(*dStyle);
    setupModelSelector(*mSelector);

    RefCountedPtr<VIEWDEF_T> viewDefinition = new VIEWDEF_T(model, viewName, *cSelector, *dStyle, *mSelector);
    if (viewDefinition.IsValid())
        {
        viewDefinition->SetIsPrivate(true);
        viewDefinition->SetAspectRatioSkew(DEFAULT_VIEWDEF_ASPECT_RATIO_SKEW);
        }

    return viewDefinition;
    }*/

END_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        09/2017
// Inserts the 'system' private views for ClipPlanesViewDefinition and AlignmentProfileViewDefinition classes
//---------------------------------------------------------------------------------------
/*DgnDbStatus RoadRailAlignmentDomain::InsertViewDefinitions(ConfigurationModelR model)
    {
    AlignmentProfileViewDefinitionPtr profileDefinition = createViewDefinition<AlignmentProfileViewDefinition>(model, AlignmentProfileViewDefinition::SYSTEM_VIEW_NAME);
    ClipPlanesViewDefinitionPtr xsDefinition = createViewDefinition<ClipPlanesViewDefinition>(model, ClipPlanesViewDefinition::SYSTEM_VIEW_NAME);

    if (!profileDefinition.IsValid() || !xsDefinition.IsValid())
        return DgnDbStatus::BadElement;

    DgnDbStatus status;
    if (!profileDefinition->Insert(&status).IsValid())
        return status;

    if (!xsDefinition->Insert(&status).IsValid())
        return status;

    return DgnDbStatus::Success;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RoadRailAlignmentDomain::SetUpDefinitionPartitions(SubjectCR subject)
    {
    DgnDbStatus status;
    auto configurationPartitionPtr = DefinitionPartition::Create(subject, GetConfigurationPartitionName());

    IBriefcaseManager::Request req;
    auto stat = configurationPartitionPtr->PopulateRequest(req, BeSQLite::DbOpcode::Insert);
    if (RepositoryStatus::Success == stat)
        Dgn::IBriefcaseManager::Response response = configurationPartitionPtr->GetDgnDb().BriefcaseManager().Acquire(req);
           
    if (configurationPartitionPtr->Insert(&status).IsNull())
        {
        BeAssert(false);
        }

    auto configModelPtr = DefinitionModel::Create(*configurationPartitionPtr);

    stat = configModelPtr->PopulateRequest(req, BeSQLite::DbOpcode::Insert);
    if (RepositoryStatus::Success == stat)
        Dgn::IBriefcaseManager::Response response = configModelPtr->GetDgnDb().BriefcaseManager().Acquire(req);

    if (!configModelPtr.IsValid() || (DgnDbStatus::Success != configModelPtr->Insert()))
        {
        BeAssert(false);
        }

    /*if (DgnDbStatus::Success != (status = InsertViewDefinitions(*configModelPtr)))
        return status;*/

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void createDomainCategoriesPartition(DgnDbR db)
    {
    DgnDbStatus status;
    auto categoryPartitionPtr = DefinitionPartition::Create(*db.Elements().GetRootSubject(), RoadRailAlignmentDomain::GetDomainCategoriesPartitionName());
    if (categoryPartitionPtr->Insert(&status).IsNull())
        {
        BeAssert(false);
        }

    auto modelPtr = DefinitionModel::Create(*categoryPartitionPtr);

    if (!modelPtr.IsValid() || (DgnDbStatus::Success != modelPtr->Insert()))
        {
        BeAssert(false);
        }

    AlignmentCategory::InsertDomainCategories(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void createCodeSpecs(DgnDbR dgndb)
    {
    auto codeSpec = CodeSpec::Create(dgndb, BRRA_CODESPEC_Alignment, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpec.IsValid());
    if (codeSpec.IsValid())
        {
        codeSpec->Insert();
        BeAssert(codeSpec->GetCodeSpecId().IsValid());
        }

    codeSpec = CodeSpec::Create(dgndb, BRRA_CODESPEC_HorizontalAlignment, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpec.IsValid());
    if (codeSpec.IsValid())
        {
        codeSpec->Insert();
        BeAssert(codeSpec->GetCodeSpecId().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailAlignmentDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    createCodeSpecs(dgndb);
    createDomainCategoriesPartition(dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId RoadRailAlignmentDomain::QueryAlignmentCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRA_CODESPEC_Alignment);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RoadRailAlignmentDomain::CreateCode(DgnModelCR scopeModel, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRA_CODESPEC_Alignment, scopeModel, value);
    }
	
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId RoadRailAlignmentDomain::QueryConfigurationModelId(SubjectCR subject)
    {
    DgnCode partitionCode = DefinitionPartition::CreateCode(subject, GetConfigurationPartitionName());
    return subject.GetDgnDb().Models().QuerySubModelId(partitionCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionModelPtr RoadRailAlignmentDomain::QueryConfigurationModel(SubjectCR subject)
    {
    DefinitionModelPtr model = subject.GetDgnDb().Models().Get<DefinitionModel>(QueryConfigurationModelId(subject));
    BeAssert(model.IsValid());
    return model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId RoadRailAlignmentDomain::QueryCategoryModelId(DgnDbR db)
    {
    DgnCode partitionCode = DefinitionPartition::CreateCode(*db.Elements().GetRootSubject(), RoadRailAlignmentDomain::GetDomainCategoriesPartitionName());
    return db.Models().QuerySubModelId(partitionCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionModelPtr RoadRailAlignmentDomain::QueryCategoryModel(DgnDbR db)
    {
    DefinitionModelPtr model = db.Models().Get<DefinitionModel>(QueryCategoryModelId(db));
    BeAssert(model.IsValid());
    return model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
/*SubjectCPtr ConfigurationModel::GetParentSubject() const
    {
    auto partitionCP = dynamic_cast<DefinitionPartitionCP>(GetModeledElement().get());
    BeAssert(partitionCP != nullptr);

    return GetDgnDb().Elements().Get<Subject>(partitionCP->GetParentId());
    }*/