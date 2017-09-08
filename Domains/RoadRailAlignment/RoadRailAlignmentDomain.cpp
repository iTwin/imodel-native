/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadRailAlignmentDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/RoadRailAlignmentDomain.h>

DOMAIN_DEFINE_MEMBERS(RoadRailAlignmentDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRailAlignmentDomain::RoadRailAlignmentDomain() : DgnDomain(BRRA_SCHEMA_NAME, "Bentley RoadRailAlignment Domain", 1)
    {
    RegisterHandler(AlignmentCategoryModelHandler::GetHandler());
    RegisterHandler(AlignmentModelHandler::GetHandler());
    RegisterHandler(AlignmentHandler::GetHandler());
    RegisterHandler(HorizontalAlignmentModelHandler::GetHandler());
    RegisterHandler(HorizontalAlignmentsPortionHandler::GetHandler());
    RegisterHandler(HorizontalAlignmentHandler::GetHandler());    
    RegisterHandler(AlignmentProfileViewDefinitionHandler::GetHandler());
    RegisterHandler(AlignmentReferentElementHandler::GetHandler());
    RegisterHandler(AlignmentStationHandler::GetHandler());
    RegisterHandler(AlignmentXSViewDefinitionHandler::GetHandler());
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
template <typename VIEWDEF_T>
RefCountedPtr<VIEWDEF_T> createViewDefinition(DictionaryModelR dictionary, Utf8StringCR viewName)
    {
    Utf8String cSelectorName = viewName;
    cSelectorName.append("CategorySelector");

    Utf8String dStyleName = viewName;
    dStyleName.append("DisplayStyle");

    Utf8String mSelectorName = viewName;
    mSelectorName.append("ModelSelector");

    CategorySelectorPtr cSelector = new CategorySelector(dictionary, cSelectorName);
    DisplayStyle3dPtr dStyle = new DisplayStyle3d(dictionary, dStyleName);
    ModelSelectorPtr mSelector = new ModelSelector(dictionary, mSelectorName);
    if (!cSelector.IsValid() || !dStyle.IsValid() || !mSelector.IsValid())
        return nullptr;

    setupCategorySelector(*cSelector);
    setupDisplayStyle(*dStyle);
    setupModelSelector(*mSelector);

    RefCountedPtr<VIEWDEF_T> viewDefinition = new VIEWDEF_T(dictionary, viewName, *cSelector, *dStyle, *mSelector);
    if (viewDefinition.IsValid())
        viewDefinition->SetIsPrivate(true);

    return viewDefinition;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        09/2017
// Inserts the 'system' private views for AlignmentXSViewDefinition and AlignmentProfileViewDefinition classes
//---------------------------------------------------------------------------------------
DgnDbStatus insertAlignmentViewDefinitions(DgnDbR db)
    {
    DictionaryModelR dictionary = db.GetDictionaryModel();

    AlignmentProfileViewDefinitionPtr profileDefinition = createViewDefinition<AlignmentProfileViewDefinition>(dictionary, AlignmentProfileViewDefinition::SYSTEM_VIEW_NAME);
    AlignmentXSViewDefinitionPtr xsDefinition = createViewDefinition<AlignmentXSViewDefinition>(dictionary, AlignmentXSViewDefinition::SYSTEM_VIEW_NAME);

    if (!profileDefinition.IsValid() || !xsDefinition.IsValid())
        return DgnDbStatus::BadElement;

    DgnDbStatus status;
    if (!profileDefinition->Insert(&status).IsValid())
        return status;

    if (!xsDefinition->Insert(&status).IsValid())
        return status;

    return DgnDbStatus::Success;
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RoadRailAlignmentDomain::SetUpModelHierarchy(SubjectCR subject, Utf8CP partitionName)
    {
    DgnDbStatus status;

    auto alignmentPartitionPtr = SpatialLocationPartition::Create(subject, partitionName);
    if (alignmentPartitionPtr->Insert(&status).IsNull())
        return status;

    auto alignmentModelPtr = AlignmentModel::Create(AlignmentModel::CreateParams(subject.GetDgnDb(), alignmentPartitionPtr->GetElementId()));


    if (DgnDbStatus::Success != (status = alignmentModelPtr->Insert()))
        return status;

    auto horizontalPartitionCPtr = HorizontalAlignmentsPortion::InsertPortion(*alignmentModelPtr);
    if (horizontalPartitionCPtr.IsNull())
        return DgnDbStatus::BadModel;

    auto horizontalBreakDownModelPtr = HorizontalAlignmentModel::Create(HorizontalAlignmentModel::CreateParams(subject.GetDgnDb(), horizontalPartitionCPtr->GetElementId()));

    if (DgnDbStatus::Success != (status = horizontalBreakDownModelPtr->Insert()))
        return status;


    if (DgnDbStatus::Success != (status = insertAlignmentViewDefinitions(subject.GetDgnDb())))
        return status;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailAlignmentDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    AlignmentCategoryModel::SetUp(dgndb);

    DgnDbStatus status = SetUpModelHierarchy(*dgndb.Elements().GetRootSubject(), GetDefaultPartitionName());
    if (DgnDbStatus::Success != status)
        {
        BeAssert(false);
        }

    auto codeSpec = CodeSpec::Create(dgndb, BRRA_CODESPEC_Alignment, CodeScopeSpec::CreateModelScope());
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
