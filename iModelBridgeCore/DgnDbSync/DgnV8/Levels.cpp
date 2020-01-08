/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

USING_NAMESPACE_BENTLEY_LOGGING

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::IsDrawingCategory(DgnCategoryId categoryId)
    {
    auto cat = DgnCategory::Get(GetDgnDb(), categoryId);
    if (!cat.IsValid())
        return false;
    return nullptr != dynamic_cast<DrawingCategory const*>(cat.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::IsSpatialCategory(DgnCategoryId categoryId)
    {
    auto cat = DgnCategory::Get(GetDgnDb(), categoryId);
    if (!cat.IsValid())
        return false;
    return nullptr != dynamic_cast<SpatialCategory const*>(cat.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::IsDefaultLevel(DgnV8Api::LevelHandle const& level)
    {
    return (level->GetLevelId() == DGNV8_LEVEL_DEFAULT_LEVEL_ID) && !Utf8String(level->GetName()).Equals("0") && _GetNamePrefix().empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void appendToList(Utf8StringR lst, Utf8CP item)
    {
    if (!lst.empty())
        lst.append(", ");
    lst.append(item);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename ITEMTYPE> static int compareItems(Utf8StringR result, Utf8CP desc, ITEMTYPE const& v0, ITEMTYPE const& v1)
    {
    if (v0 == v1)
        return 0;
        
    appendToList(result, Utf8PrintfString("%s (%d vs. %d)", desc, v0, v1).c_str());
    return (v0 < v1)? -1: 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
template<> int compareItems(Utf8StringR result, Utf8CP desc, double const& v0, double const& v1)
    {
    int i = BeNumerical::Compare(v0, v1);
    if (i == 0)
        return 0;
        
    appendToList(result, Utf8PrintfString("%s (%lf vs. %lf)", desc, v0, v1).c_str());
    return i;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int compareAppearance(Utf8StringR result, DgnSubCategory::Appearance const& a0, DgnSubCategory::Appearance const& a1)
    {
    if (0 != compareItems(result, "color",  a0.GetColor().GetValue(), a1.GetColor().GetValue())
     || 0 != compareItems(result, "weight", a0.GetWeight(), a1.GetWeight())
     || 0 != compareItems(result, "style",  a0.GetStyle().GetValueUnchecked(), a1.GetStyle().GetValueUnchecked())
     || 0 != compareItems(result, "displayPriority",  a0.GetDisplayPriority(), a1.GetDisplayPriority())
     || 0 != compareItems(result, "material",  a0.GetRenderMaterial().GetValueUnchecked(), a1.GetRenderMaterial().GetValueUnchecked())
     || 0 != compareItems(result, "locate",  a0.GetDontLocate(), a1.GetDontLocate())
     || 0 != compareItems(result, "snap",  a0.GetDontSnap(), a1.GetDontSnap())
     || 0 != compareItems(result, "transparency",  a0.GetTransparency(), a1.GetTransparency()))
        return 1;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ComputeSubCategoryAppearanceFromLevel(DgnSubCategory::Appearance& appear, DgnV8Api::LevelHandle const& level)
    {
    // Note: we do not import the "override" symbology and on/off flags. We use view-specific level overrides to give the user an equivalent way to customize symbology.
    // *** WIP_V10_LEVELS: How to carry over existing override symbology to new view-oriented system?

    DgnV8Api::IntColorDef v8ColorDef;
    DgnV8Api::DgnColorMap::ExtractElementColorInfo(&v8ColorDef, nullptr, nullptr, nullptr, nullptr, level->GetByLevelColor().GetColor(), *level->GetByLevelColor().GetDefinitionFile());

    auto material   = m_syncInfo.FindMaterialByV8Id(level->GetByLevelMaterial().GetId(), *level->GetByLevelMaterial().GetDefinitionFile(), *level->GetByLevelMaterial().GetDefinitionModel());
    auto color      = ColorDef(v8ColorDef.m_int);
    auto weight     = level->GetByLevelWeight();
    //  WIP_V10_LEVELS -- Do we care about whether there is a line style scale?
    double unitsScale = 0;
    bool foundStyle = false;
    auto lineStyleId = m_syncInfo.FindLineStyle(unitsScale, foundStyle, SyncInfo::V8StyleId(GetRepositoryLinkId(*level->GetByLevelLineStyle().GetDefinitionFile()), (uint32_t) level->GetByLevelLineStyle().GetStyle()));

    appear.SetInvisible(false); // It never makes sense to define a SubCategory that is invisible. If a level is off in a view, then we will turn off this SubCategory in that view.
    appear.SetDisplayPriority(level->GetDisplayPriority());
    appear.SetTransparency(level->GetTransparency());
    appear.SetDontPlot(!level->GetPlot());
    appear.SetDontSnap(!level->GetSnap());
    appear.SetDontLocate(!level->GetLocate());
    appear.SetColor(color);
    appear.SetWeight(weight);
    appear.SetStyle(lineStyleId);
    if (material.IsValid())
        appear.SetRenderMaterial(material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId Converter::FindFirstModelInSyncInfo(DgnV8ModelCR v8Model) const
    {
    SyncInfo::V8ModelExternalSourceAspectIteratorByV8Id it(*GetRepositoryLinkElement(*v8Model.GetDgnFileP()), v8Model);
    auto entry=it.begin();
    if (entry != it.end())
        return entry->GetModelId();
    return DgnModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId Converter::ConvertLevelToSubCategory(DgnV8Api::LevelHandle const& level, DgnV8ModelCR v8Model, DgnCategoryId catid)
    {
    if (!catid.IsValid())
        return DgnSubCategoryId();

    //  When we create a model-specific subcategory, we use the model's name as the subcategory code. That makes it easy to identify the subcategory, and it's unique.
    DgnModel* dgnModel = m_dgndb->Models().GetModel(FindFirstModelInSyncInfo(v8Model)).get();
    if (nullptr == dgnModel)
        {
        BeAssert(false);
        return DgnSubCategoryId();
        }

    Utf8String dbSubCategoryName = dgnModel->GetName();
    // *** NEEDS WORK - we should not have models with no names!
    // *** vvvTEMPORARY FIXvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    if (dbSubCategoryName.empty())
        dbSubCategoryName = Utf8PrintfString("model_%llu", dgnModel->GetModeledElementId().GetValue());
    // *** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    dbSubCategoryName.Trim(); // in DgnDb, we don't allow leading or trailing blanks
    dbSubCategoryName.insert(0, _GetNamePrefix().c_str());
    DgnDbTable::ReplaceInvalidCharacters(dbSubCategoryName, DgnCategory::GetIllegalCharacters(), '_');

    DgnSubCategoryId dbSubCategoryId = DgnSubCategory::QuerySubCategoryId(GetDgnDb(), DgnSubCategory::CreateCode(GetDgnDb(), catid, dbSubCategoryName));
    if (dbSubCategoryId.IsValid())
        {
        //  We've already created a subcategory by this name. Map this V8 level to it.
        if (LOG_LEVEL_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
            LOG_LEVEL.tracev("merged level [%s] (%d) -sub-> [%s] (%d)", Utf8String(level.GetName()).c_str(), level.GetLevelId(), dbSubCategoryName.c_str(), dbSubCategoryId.GetValue());
        }
    else
        {
        //  First time we've seen a level for a model with this name. Create a subcategory for it.
        DgnSubCategory::Appearance appear;
        ComputeSubCategoryAppearanceFromLevel(appear, level);

        DgnSubCategory newSubCategoryData(DgnSubCategory::CreateParams(GetDgnDb(), catid, dbSubCategoryName, appear, Utf8String(level->GetDescription())));

        /*
        if (_WantProvenanceInBim())
            {
            auto aspect = SyncInfo::LevelExternalSourceAspect::CreateAspect(level, v8Model, *this);
            aspect.AddAspect(newSubCategoryData);
            }
            */

        auto newSubCategory = newSubCategoryData.Insert();
        if (!newSubCategory.IsValid())
            {
            if (LOG_LEVEL_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
                LOG_LEVEL.tracev("failed to insert subcategory for level [%s] (%d) as [%s]", Utf8String(level.GetName()).c_str(), level.GetLevelId(), newSubCategoryData.GetSubCategoryName().c_str());
            BeAssert(false);
            return DgnSubCategoryId();
            }

        dbSubCategoryId = newSubCategory->GetSubCategoryId();
        BeAssert(dbSubCategoryId.IsValid());

        if (LOG_LEVEL_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
            LOG_LEVEL.tracev("inserted level [%s] (%d) -> [%s] (%d)", Utf8String(level.GetName()).c_str(), level.GetLevelId(), newSubCategory->GetSubCategoryName().c_str(), dbSubCategoryId.GetValue());
        }

    m_syncInfo.InsertLevel(dbSubCategoryId, v8Model, level);
    return dbSubCategoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId Converter::ConvertLevelToCategory(DgnV8Api::LevelHandle const& level, DgnV8FileR v8File, bool is3d)
    {
    // Always map levels -> categories by name. We will create model-specific sub-categories later on in the conversion.
    Utf8String dbCategoryName(level.GetName());
    dbCategoryName.Trim(); // in DgnDb, we don't allow leading or trailing blanks
    dbCategoryName.insert(0, _GetNamePrefix().c_str());
    DgnDbTable::ReplaceInvalidCharacters(dbCategoryName, DgnCategory::GetIllegalCharacters(), '_');

    DefinitionModelPtr definitionModel = ShouldMergeDefinitions()? &m_dgndb->GetDictionaryModel(): GetJobDefinitionModel();
    if (!definitionModel.IsValid())
        return DgnCategoryId();

    if (definitionModel.get() == &m_dgndb->GetDictionaryModel())
        {
        if (BSISUCCESS != MustBeInSharedChannel("ConvertLevelToCategory may only merge levels to the DictionaryModel in the shared channel."))
            return DgnCategoryId();
        }
    else
        {
        if (BSISUCCESS != MustBeInNormalChannel("ConvertLevelToCategory may only write levels to the Job's definition model in the normal channel."))
            return DgnCategoryId();
        }

    DgnCode categoryCode = is3d ? SpatialCategory::CreateCode(*definitionModel, dbCategoryName.c_str()) : DrawingCategory::CreateCode(*definitionModel, dbCategoryName.c_str());
    DgnCategoryId dbCategoryId = DgnCategory::QueryCategoryId(GetDgnDb(), categoryCode);
    if (dbCategoryId.IsValid())
        {
        //  We've already created a category by this name. Map this V8 level to it.
        if (LOG_LEVEL_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
            LOG_LEVEL.tracev("merged level [%s] (%d) -> [%s] (%d)", Utf8String(level.GetName()).c_str(), level.GetLevelId(), dbCategoryName.c_str(), dbCategoryId.GetValue());
        }
    else
        {
        if (GetChangeDetector()._ShouldSkipLevel(dbCategoryId, *this, level, v8File, dbCategoryName)) // allow sub-class to disallow (or tweak) level conversion. The TiledFileConverter limits conversion to levels in root file.
            return dbCategoryId; 

        //  First time we've seen a level by this name. Create a category for it.
        DgnSubCategory::Appearance appear;
        DgnDbStatus result;
        ComputeSubCategoryAppearanceFromLevel(appear, level);

        DgnCategoryCPtr newCategory;
        if (is3d)
            {
            SpatialCategory category(*definitionModel, dbCategoryName, DgnCategory::Rank::User, Utf8String(level->GetDescription()));
            newCategory = category.Insert(appear, &result);
            }
        else
            {
            DrawingCategory category(*definitionModel, dbCategoryName, DgnCategory::Rank::User, Utf8String(level->GetDescription()));
            newCategory = category.Insert(appear, &result);
            }

        BeAssert((DgnDbStatus::LockNotHeld != result) && "Failed to get or retain necessary locks");
        BeAssert((DgnDbStatus::Success == result) && newCategory.IsValid());

        if (DgnDbStatus::Success != result || !newCategory.IsValid())
            {
            BeAssert(DgnDbStatus::DuplicateCode != result && "Caller is responsible for providing a unique level name");

            if (LOG_LEVEL_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
                LOG_LEVEL.tracev("failed to insert level [%s] (%d) as [%s]", Utf8String(level.GetName()).c_str(), level.GetLevelId(), dbCategoryName.c_str());

            //    importer.ReportIssueV (ISSUE_SEVERITY_Warning, IssueReportCategory::CorruptData, importer.GetCurrentUpgradeFile()->GetDgnV8FileName().c_str(), ImportIssue::LevelFailedToImport, level->GetCategoryId(), level->GetName());
            BeAssert(false);
            return DgnCategoryId();
            }
        
        dbCategoryId = newCategory->GetCategoryId();

        if (LOG_LEVEL_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
            LOG_LEVEL.tracev("inserted level [%s] (%d) -> [%s] (%lld)%s", Utf8String(level.GetName()).c_str(), level.GetLevelId(), dbCategoryName.c_str(), dbCategoryId.GetValue(), is3d? " (Spatial)":" (Drawing)");
        }

    // Even if this is the second time we've seen a level by this name and we decide above not to insert a new copy,
    // we still create a LevelExternalSourceAspect for this source. That will help us, later, to detect display differences
    // and to create source-specific SubCategories if necessary. See ConvertLevelMask where we call GetSyncInfo().FindSubCategory -- that must succeed.
    m_syncInfo.InsertLevel(DgnCategory::GetDefaultSubCategoryId(dbCategoryId), v8File.GetDictionaryModel(), level);

    return dbCategoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertAllSpatialLevels(DgnV8FileR v8file)
    {
    if (LOG_LEVEL_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
        LOG_LEVEL.tracev(L"importing level table from file %ls", v8file.GetFileName().c_str());

    DgnV8Api::FileLevelCache& cache = v8file.GetLevelCacheR();

    uint32_t count=0;
    for (DgnV8Api::LevelHandle const& level : cache)
        {
        if (IsUpdating())
            {
            DgnCategoryId categoryId = GetSyncInfo().FindCategory(level.GetLevelId(), v8file, SyncInfo::LevelExternalSourceAspect::Type::Spatial);
            if (categoryId.IsValid())
                {
                if (IsSpatialCategory(categoryId)) // go with previously converted level ... as long as it's a spatial level
                    {
                    _OnUpdateLevel(level, categoryId, v8file);
                    continue;
                    }
                }
            }

        if ((count++ % 100) == 0)
            ReportProgress();

        if (IsDefaultLevel(level))
            {
            continue;   // This must be converted in _ConvertBridgeSpecificLevelsInLevelTable
            }

        ConvertLevelToCategory(level, v8file, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::MapDefaultLevelToUncategorized(DgnV8FileR v8file)
    {
    if (BSISUCCESS != MustBeInNormalChannel("MapDefaultLevelToUncategorized must be called only in the normal channel."))
        return;

    if (LOG_LEVEL_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
        LOG_LEVEL.tracev(L"importing bridge-specific levels from level table from file %ls", v8file.GetFileName().c_str());

    DgnV8Api::FileLevelCache& cache = v8file.GetLevelCacheR();

    uint32_t count=0;
    for (DgnV8Api::LevelHandle const& level : cache)
        {
        if (IsUpdating())
            {
            DgnCategoryId categoryId = GetSyncInfo().FindCategory(level.GetLevelId(), v8file, SyncInfo::LevelExternalSourceAspect::Type::Spatial);
            if (categoryId.IsValid())
                continue;
            }

        //  Only the default level is bridge-specific
        if (!IsDefaultLevel(level))
            continue;

        GetSyncInfo().InsertLevel(DgnCategory::GetDefaultSubCategoryId(GetUncategorizedCategory()), v8file.GetDictionaryModel(), level);
        GetSyncInfo().InsertLevel(DgnCategory::GetDefaultSubCategoryId(GetUncategorizedDrawingCategory()), v8file.GetDictionaryModel(), level);

        if (LOG_LEVEL_IS_SEVERITY_ENABLED (NativeLogging::LOG_TRACE))
            LOG_LEVEL.tracev("Mapped default level [%s] (%d) -> %d", Utf8String(level.GetName()).c_str(), level.GetLevelId(), GetUncategorizedCategory().GetValue());
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::_ConvertSpatialLevelTable(DgnV8FileR v8file)
    {
    ConvertAllSpatialLevels(v8file);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::HandleLevelAppearanceInconsistency(ViewDefinitionR theView, DgnAttachmentCR v8Attachment, DgnV8Api::LevelId v8LevelId, DgnCategoryId categoryId, DgnSubCategoryId subcatId, bool isV8LevelOn)
    {
    // If this level is not actually used in the attached model, then its symbology or on/off status does not in fact affect
    // any view and therefore does not require a subcategory override.
#ifdef TFS_620759 // level usage stats are not accurate for shared cell def in embedded reference file in i.dgn!
    if (!v8Attachment.GetDgnModelP() || 0 == v8Attachment.GetDgnModelP()->LevelUsageTree()->GetUsageCount(v8LevelId))
        return;
#endif

    // Tricky: if we are making attachment-specific SubCategories, then each Category must be displayed by default.
    //          Then we can turn attachment-specific SubCategories off whenever we see a 0 in the attachment level display mask.
    if (isV8LevelOn || !GetParams().NeverCopyLevel())
        theView.GetCategorySelector().AddCategory(categoryId);

    DgnV8Api::LevelCache const& v8AttachmentLevelCache = v8Attachment.GetLevelCache();
    DgnV8Api::LevelHandle v8Level = v8AttachmentLevelCache.GetLevel(v8LevelId);
    if (!v8Level.IsValid()) // looks like the level has disappeared from the v8 file!
        return;

    bool maybeMakeSubCategories = !GetParams().NeverCopyLevel(); 
    IssueSeverity issueSeverity = maybeMakeSubCategories? IssueSeverity::Info: IssueSeverity::Error;

    if (DgnV8Api::LevelCacheErrorCode::CannotFindLevel == v8Level->GetStatus())
        {
        if (IssueSeverity::Error == issueSeverity)
            ReportIssueV(issueSeverity, IssueCategory::VisualFidelity(), Issue::LevelDisplayInconsistent(), nullptr, Utf8String(v8Level.GetName()).c_str(), IssueReporter::FmtAttachment(v8Attachment).c_str());
        return;
        }

    bool forceInvisible = false;
    if (!isV8LevelOn && theView.GetCategorySelector().IsCategoryViewed(categoryId))
        {
        forceInvisible = true;
		if (IssueSeverity::Error == issueSeverity)
            ReportIssueV(IssueSeverity::Info, IssueCategory::VisualFidelity(), Issue::LevelDisplayInconsistent(), nullptr, Utf8String(v8Level.GetName()).c_str(), IssueReporter::FmtAttachment(v8Attachment).c_str());
        }

    //  See if this appearance has its own symbology
    bool appearanceIsDifferent = false;
    DgnSubCategoryCPtr dgnDbSubCategory = DgnSubCategory::Get(GetDgnDb(), subcatId);
    if (dgnDbSubCategory.IsValid())
        {
        DgnSubCategory::Appearance appearanceInAttachment;
        ComputeSubCategoryAppearanceFromLevel(appearanceInAttachment, v8Level);
        if (!dgnDbSubCategory->GetAppearance().IsEqual(appearanceInAttachment))
            {
            appearanceIsDifferent = true;
            if (v8LevelId != DGNV8_LEVEL_DEFAULT_LEVEL_ID)  // We expect "Default" to be defined differently in different files. Only report on real levels that differ.
                {
                Utf8String diff;
                compareAppearance(diff, dgnDbSubCategory->GetAppearance(), appearanceInAttachment);
		        if (IssueSeverity::Error == issueSeverity)
	                ReportIssueV(IssueSeverity::Info, IssueCategory::VisualFidelity(), Issue::LevelSymbologyInconsistent(), nullptr, Utf8String(v8Level.GetName()).c_str(), IssueReporter::FmtAttachment(v8Attachment).c_str(), diff.c_str());
                }
            }
        }

    //  If the user allows it and there are differences in appearance, then make a new subcategory and map this use of the level in this attachment to it.
    if (GetParams().AlwaysCopyLevel() || (GetParams().CopyLevelIfDifferent() && (forceInvisible || appearanceIsDifferent)))
        {
        DgnSubCategoryId newSubCategoryId = ConvertLevelToSubCategory(v8Level, *v8Attachment.GetDgnModelP(), categoryId);

        if (forceInvisible)
            MakeSubCategoryInvisibleInView(theView, newSubCategoryId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::CheckNoLevelChange(DgnV8Api::LevelHandle const& v8Level, DgnCategoryId categoryId, DgnV8FileR v8File)
    {
    DgnSubCategoryCPtr dgnDbSubCategory = DgnSubCategory::Get(GetDgnDb(), DgnCategory::GetDefaultSubCategoryId(categoryId));

    DgnSubCategory::Appearance newAppearance;
    ComputeSubCategoryAppearanceFromLevel(newAppearance, v8Level);

    if (dgnDbSubCategory.IsNull() || newAppearance.IsEqual(dgnDbSubCategory->GetAppearance()))
        return;

    Utf8String diff;
    compareAppearance(diff, dgnDbSubCategory->GetAppearance(), newAppearance);
    ReportIssueV(IssueSeverity::Error, IssueCategory::VisualFidelity(), Issue::LevelDefinitionChange(), nullptr, Bentley::Utf8String(v8Level.GetName()).c_str(), Bentley::Utf8String(v8File.GetFileName()).c_str(), diff.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertAttachmentLevelMask(ViewDefinitionR theView, DgnV8ViewInfoCR viewInfo, DgnAttachmentCR v8Attachment, SyncInfo::LevelExternalSourceAspect::Type ltype)
    {
    DgnV8FileP v8ReferenceFile = v8Attachment.GetDgnFileP();
    auto& v8Model = v8ReferenceFile->GetDictionaryModel();

    Bentley::BitMaskCP v8LevelMask = nullptr;
    if (DgnV8Api::VI_Success == viewInfo.GetEffectiveLevelDisplayMask(v8LevelMask, const_cast<DgnV8Api::DgnAttachment*>(&v8Attachment)) || nullptr==v8LevelMask)
        {
        for (uint32_t i=0; i<v8LevelMask->GetCapacity(); ++i)
            {
            DgnV8Api::LevelId levelId = (i+1);

            DgnV8Api::LevelCache const& v8AttachmentLevelCache = v8Attachment.GetLevelCache();
            DgnV8Api::LevelHandle v8Level = v8AttachmentLevelCache.GetLevel(levelId);
            if (!v8Level.IsValid()) // looks like the level has disappeared from the v8 file!
                continue;

            DgnSubCategoryId subcatId = GetSyncInfo().FindSubCategory(levelId, v8Model, ltype);   // Look up the default SubCategory for this level
            if (!subcatId.IsValid())
                continue;

            DgnCategoryId categoryId = DgnSubCategory::QueryCategoryId(GetDgnDb(), subcatId);
            bool isV8LevelOn = v8LevelMask->Test(i);

            HandleLevelAppearanceInconsistency(theView, v8Attachment, levelId, categoryId, subcatId, isV8LevelOn);
            }        
        }

    if (v8Attachment.GetDgnAttachmentsP() != nullptr)
        {
        for (auto nestedAttachment : *v8Attachment.GetDgnAttachmentsP())
            {
            if (nullptr == nestedAttachment->GetDgnModelP())
                continue; // missing reference or blocked reference conversion.

            if (ShouldConvertToPhysicalModel(*v8Attachment.GetDgnModelP()) != ShouldConvertToPhysicalModel(*nestedAttachment->GetDgnModelP()))
                continue; // 3D->2D attachments not supported

            ConvertAttachmentLevelMask(theView, viewInfo, *nestedAttachment, ltype);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::MakeSubCategoryInvisibleInView(ViewDefinitionR viewDef, DgnSubCategoryId subcatId)
    {
    BeAssert(viewDef.GetCategorySelector().IsCategoryViewed(DgnSubCategory::QueryCategoryId(*m_dgndb, subcatId)) && "Use this (expensive) way to turn off a SubCategory only if its Category is in the view's display set");
    DgnSubCategory::Override ovr;
    ovr.SetInvisible(true);
    viewDef.GetDisplayStyle().OverrideSubCategory(subcatId, ovr);

    if (LOG_LEVEL_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
        LOG_LEVEL.tracev("turn off subcategory %d", subcatId.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertLevelMask(ViewDefinitionR theView, DgnV8ViewInfoCR viewInfo, DgnV8ModelRefP v8ModelRef)
    {
    auto& categories = theView.GetCategorySelector();
    
    bool handleInconsistencies = theView.IsSpatialView(); // only spatial views process references and so only spatial views can have attachments disagreeing about level symbology or visibility
    auto levelType = theView.IsSpatialView()? SyncInfo::LevelExternalSourceAspect::Type::Spatial: SyncInfo::LevelExternalSourceAspect::Type::Drawing;

    //  Start by adding all categories to the view that are on in the V8 root model level mask.
    Bentley::BitMaskCP v8LevelMask = nullptr;
    if (DgnV8Api::VI_Success != viewInfo.GetEffectiveLevelDisplayMask(v8LevelMask, v8ModelRef) || nullptr==v8LevelMask)
        {
        // *** NEEDS WORK: add drawing categories for non-spatial view
        AddAllSpatialCategories(categories.GetCategoriesR());
        }
    else
        {
        DgnV8Api::LevelMaskTree& lmTree = viewInfo.GetLevelMasksR();
        DgnV8Api::LMTreeEntry* entry;
        if (NULL != (entry = lmTree.FindEntry(NULL, v8ModelRef, false, NULL)))
            {
            bool newLevels;
            entry->CorrectViewLevelMask(newLevels, false, v8ModelRef);
            v8LevelMask = entry->GetViewLevelMaskCP();
            }
        for (uint32_t i = 0; i < v8LevelMask->GetCapacity(); ++i)
            {
            DgnV8Api::LevelId levelId = (i + 1);
            DgnSubCategoryId subcatId = GetSyncInfo().FindSubCategory(levelId, *v8ModelRef->GetDgnFileP(), levelType);
            if (!subcatId.IsValid())
                continue;

            auto catId = DgnSubCategory::QueryCategoryId(GetDgnDb(), subcatId);

            if (v8LevelMask->Test(i))
                {
                // The V8 level is on for the root model in this view, so display the Category.
                // (Elements in the root model will use default SubCategory, and so they will be on.)
                categories.AddCategory(catId);
                }
            else if (handleInconsistencies)
                {
                // The V8 level is off in the root model.
                if (!GetParams().NeverCopyLevel() && (nullptr != v8ModelRef->GetDgnAttachmentsP()))
                    {
                    //  Attachments might want this level to be on. Therefore:
                    
                    //  Force the Category to be on. If an attachment wants it to be off, it will create its 
                    //  own unique SubCategory and turn it off. The default must be on.
                    categories.AddCategory(catId);

                    if (!GetSyncInfo().FindSubCategory(levelId, *v8ModelRef->GetDgnModelP(), levelType).IsValid())
                        {
                        //  Make a unique SubCategory for the root model and turn it off.
                        //  (Elements in the root model will use this new SubCategory, and so they will be off.)
                        DgnV8Api::LevelCache const& lc = v8ModelRef->GetLevelCache();
                        DgnV8Api::LevelHandle level = lc.GetLevel(levelId);
                        if (!level.IsValid())
                            continue;
                        auto rootSubCatId = ConvertLevelToSubCategory(level, *v8ModelRef->GetDgnModelP(), catId);
                        MakeSubCategoryInvisibleInView(theView, rootSubCatId);
                        }
                    }
                }
            }
        }
    
    //  Now make sure that categories are on for levels that are on in attachments, and handle level appearance inconsistencies
    if (nullptr == v8ModelRef->GetDgnAttachmentsP())
        return;

    for (auto v8attachment : *v8ModelRef->GetDgnAttachmentsP())
        {
        if (nullptr == v8attachment->GetDgnModelP())
            continue; // missing reference

        if (!theView.IsSpatialView() && v8attachment->Is3d())  // when converting the levelmask for a view of a drawing, only consider the nested 2d attachments
            continue;

        ConvertAttachmentLevelMask(theView, viewInfo, *v8attachment, levelType);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::AddAllSpatialCategories(DgnCategoryIdSet& categories)
    {
    for (ElementIteratorEntry categoryEntry : SpatialCategory::MakeIterator(*m_dgndb))
        categories.insert(categoryEntry.GetId<DgnCategoryId>());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId Converter::GetUncategorizedCategory()
    {
    if (m_uncategorizedCategoryId.IsValid())
        return m_uncategorizedCategoryId;

    DefinitionModelPtr definitionModel = GetJobDefinitionModel();
    if (!definitionModel.IsValid())
        {
        BeAssert(false);
        return DgnCategoryId();
        }

    m_uncategorizedCategoryId = SpatialCategory::QueryCategoryId(*definitionModel, CATEGORY_NAME_Uncategorized);

    if (m_uncategorizedCategoryId.IsValid())
        return m_uncategorizedCategoryId;

    SpatialCategory category(*definitionModel, CATEGORY_NAME_Uncategorized, DgnCategory::Rank::Application);
    if (!category.Insert(DgnSubCategory::Appearance()).IsValid())
        {
        BeAssert(false);
        }

    m_uncategorizedCategoryId = category.GetCategoryId();

    return m_uncategorizedCategoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId Converter::GetUncategorizedDrawingCategory()
    {
    if (m_uncategorizedDrawingCategoryId.IsValid())
        return m_uncategorizedDrawingCategoryId;

    DefinitionModelPtr definitionModel = GetJobDefinitionModel();
    if (!definitionModel.IsValid())
        {
        BeAssert(false);
        return DgnCategoryId();
        }

    m_uncategorizedDrawingCategoryId = DrawingCategory::QueryCategoryId(*definitionModel, CATEGORY_NAME_Uncategorized);

    if (m_uncategorizedDrawingCategoryId.IsValid())
        return m_uncategorizedDrawingCategoryId;

    DrawingCategory category(*definitionModel, CATEGORY_NAME_Uncategorized, DgnCategory::Rank::Application);
    if (!category.Insert(DgnSubCategory::Appearance()).IsValid())
        {
        BeAssert(false);
        }

    m_uncategorizedDrawingCategoryId = category.GetCategoryId();

    return m_uncategorizedDrawingCategoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId Converter::ConvertDrawingLevel(DgnV8FileR v8file, DgnV8Api::LevelId levelId)
    {
    DgnV8Api::FileLevelCache& cache = v8file.GetLevelCacheR();

    DgnV8Api::LevelHandle level = cache.GetLevel(levelId);
    if (!level.IsValid())
        return GetUncategorizedDrawingCategory();

    DgnCategoryId categoryId = m_syncInfo.FindCategory(levelId, v8file, SyncInfo::LevelExternalSourceAspect::Type::Drawing);
    if (categoryId.IsValid())
        {
        if (IsDrawingCategory(categoryId)) // go with previously converted level ... as long as it's a drawing level
            {
            if (IsUpdating())
                _OnUpdateLevel(level, categoryId, v8file);
            return categoryId;
            }
        }

    if (IsDefaultLevel(level))
        {
        return DgnCategoryId(); // This must be done in the normal channel by GetUncategorizedDrawingCategory!
        }

    return ConvertLevelToCategory(level, v8file, false);
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
