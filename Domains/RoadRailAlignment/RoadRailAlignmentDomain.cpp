/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/RoadRailAlignmentDomain.h>
#include <RoadRailAlignment/AlignmentCategory.h>
#include <RoadRailAlignment/Alignment.h>
#include <RoadRailAlignment/AlignmentReferent.h>
#include <DgnPlatform/DgnPlatformLib.h>

#define DEFAULT_VIEWDEF_ASPECT_RATIO_SKEW 10.0 // For Profile and XS view definitions
#define INSERT_CODESPEC(x) \
    { auto codeSpecPtr = x; \
    BeAssert(codeSpecPtr.IsValid()); \
    if (codeSpecPtr.IsValid()) { \
        codeSpecPtr->Insert(); \
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid()); } \
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

END_UNNAMED_NAMESPACE

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

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void createDomainCategoriesPartition(SubjectCR subject)
    {
    DgnDbStatus status;
    auto categoryPartitionPtr = DefinitionPartition::Create(subject, RoadRailAlignmentDomain::GetDomainCategoriesPartitionName());
    if (categoryPartitionPtr->Insert(&status).IsNull())
        {
        BeAssert(false);
        }

    auto modelPtr = DefinitionModel::Create(*categoryPartitionPtr);

    if (!modelPtr.IsValid() || (DgnDbStatus::Success != modelPtr->Insert()))
        {
        BeAssert(false);
        }

    AlignmentCategory::InsertDomainCategories(subject.GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void createCodeSpecs(DgnDbR dgndb)
    {
    INSERT_CODESPEC(CodeSpec::Create(dgndb, BRRA_CODESPEC_Alignment, CodeScopeSpec::CreateModelScope()));
    INSERT_CODESPEC(CodeSpec::Create(dgndb, BRRA_CODESPEC_DesignAlignments, CodeScopeSpec::CreateModelScope()));
    INSERT_CODESPEC(CodeSpec::Create(dgndb, BRRA_CODESPEC_HorizontalAlignment, CodeScopeSpec::CreateModelScope()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailAlignmentDomain::OnSchemaImported(SubjectCR subject)
    {
    SetParentSubject(subject);
    createCodeSpecs(subject.GetDgnDb());
    createDomainCategoriesPartition(subject);
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

struct RoadRailCategoriesAppData : BeSQLite::Db::AppData
{
    struct RoadRailCategoriesKey : BeSQLite::Db::AppData::Key {};

public:
    RoadRailCategoriesAppData(SubjectCR subject): m_parentSubjectId(subject.GetElementId()) {}

    static RoadRailCategoriesKey s_key;
    Dgn::DgnElementId m_parentSubjectId;
}; // RoadRailCategoriesAppData

RoadRailCategoriesAppData::RoadRailCategoriesKey RoadRailCategoriesAppData::s_key;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId RoadRailAlignmentDomain::QueryCategoryModelId(DgnDbR db)
    {
    auto appDataPtr = db.FindAppData(RoadRailCategoriesAppData::s_key);
    BeAssert(appDataPtr.IsValid());

    auto parentSubjectCPtr = db.Elements().Get<Subject>(dynamic_cast<RoadRailCategoriesAppData*>(appDataPtr.get())->m_parentSubjectId);
    DgnCode partitionCode = DefinitionPartition::CreateCode(*parentSubjectCPtr, RoadRailAlignmentDomain::GetDomainCategoriesPartitionName());
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
* @bsimethod                                    Diego.Diaz                      12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailAlignmentDomain::SetParentSubject(SubjectCR subject)
    {
    subject.GetDgnDb().AddAppData(RoadRailCategoriesAppData::s_key, new RoadRailCategoriesAppData(subject));
    }