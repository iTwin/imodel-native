/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Updater.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

static bool s_doFileSaveTimeCheck = true;

// We enter this namespace in order to avoid having to qualify all of the types, such as bmap, that are common
// to bim and v8. The problem is that the V8 Bentley namespace is shifted in.
BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_Prepare(Converter& c)
    {
    BeAssert (c.GetSyncInfo().IsValid());
    BeAssert (!c.WasAborted());

    c.PopulateRangePartIdMap(); // Populate range to partId map for update...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_Cleanup(Converter& c)
    {
    m_elementsSeen.clear();
    c.GetRangePartIdMap().clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2017
//---------------+---------------+---------------+---------------+---------------+-------
void ChangeDetector::PrepareIterators(DgnDbCR db)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeDetector::~ChangeDetector()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ElementExternalSourceAspect ChangeDetector::FindElementAspectById(Converter& converter, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, T_SyncInfoElementFilter* filter)
    {
    SyncInfo::V8ElementExternalSourceAspectIteratorByV8Id it(v8mm.GetDgnModel(), v8eh);
    auto found = it.begin();
    if (nullptr != filter)
        {
        while ((found != it.end()) && !(*filter)(found, converter))
            ++found;
        }
    return (found != it.end())? *found: SyncInfo::V8ElementExternalSourceAspect();
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ElementExternalSourceAspect ChangeDetector::FindElementAspectByChecksum(Converter& converter, SyncInfo::ElementHash const& hash, ResolvedModelMapping const& v8mm, T_SyncInfoElementFilter* filter)
    {
    SyncInfo::V8ElementExternalSourceAspectIteratorByChecksum it(v8mm.GetDgnModel(), hash);
    auto found = it.begin();
    if (nullptr != filter)
        {
        while ((found != it.end()) && !(*filter)(found, converter))
            ++found;
        }
    return (found != it.end())? *found: SyncInfo::V8ElementExternalSourceAspect();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeDetector::_IsElementChanged(SearchResults& res, Converter& converter, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, 
                                       T_SyncInfoElementFilter* filter)
    {
    res.m_currentElementProvenance = SyncInfo::ElementProvenance(v8eh, converter.GetSyncInfo(), converter.GetCurrentIdPolicy());
    
    if (converter.GetCurrentIdPolicy() == StableIdPolicy::ById)
        {
        res.m_v8ElementAspect = FindElementAspectById(converter, v8eh, v8mm, filter);
        }
    else
        {
        res.m_v8ElementAspect = FindElementAspectByChecksum(converter, res.m_currentElementProvenance.m_hash, v8mm, filter);
        }

    if (!res.m_v8ElementAspect.IsValid())
        {
        // we never saw this element before, treat it as a new element. 
        // or maybe it was previously discarded (SyncInfo::WasElementDiscarded). 
        // Even so, give the converter another shot at it. Maybe it will convert it this time.
        res.m_changeType = ChangeType::Insert;
        }
    else
        {
        //  This V8 element was previously mapped to at least one element in the BIM. See if the V8 element has changed.
        res.m_changeType = res.m_v8ElementAspect.DoesProvenanceMatch(res.m_currentElementProvenance)? ChangeType::None: ChangeType::Update;

        if (v8mm.GetDgnModel().IsSpatialModel() && converter.HasRootTransChanged())
            res.m_changeType = ChangeType::Update;

        // If the element is reality data, then need to check if the image file has changed
        Utf8String fileName;
        uint64_t existingLastModifiedTime;
        uint64_t existingFileSize;
        Utf8String existingEtag;
        Utf8String rdsId; // unnecessary but the method requires it
        if (converter.GetSyncInfo().TryFindImageryFile(res.m_v8ElementAspect.GetElementId(), fileName, existingLastModifiedTime, existingFileSize, existingEtag, rdsId))
            {
            uint64_t currentLastModifiedTime;
            uint64_t currentFileSize;
            Utf8String currentEtag;
            converter.GetSyncInfo().GetCurrentImageryInfo(fileName, currentLastModifiedTime, currentFileSize, currentEtag);

            if (!existingEtag.empty())
                {
                if (!existingEtag.Equals(currentEtag))
                    res.m_changeType = ChangeType::Update;
                }
            else if (currentLastModifiedTime != existingLastModifiedTime || currentFileSize != existingFileSize)
                res.m_changeType = ChangeType::Update;
            }
        }

    return (ChangeType::None != res.m_changeType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeDetector::_ShouldSkipFileByName(Converter& converter, BeFileNameCR file) 
    {
    if (!converter._GetParams().GetSkipUnchangedFiles())
        return false;

    if (converter.GetSyncInfo().HasDiskFileChanged(file))
        return false;

    if (converter.HasRootTransChanged())  // must re-visit all elements if root transform has changed
        return false;

    SyncInfo::RepositoryLinkExternalSourceAspect prov = converter.GetSyncInfo().FindFileByFileName(file);
    if (prov.IsValid())
        {
        SyncInfo::V8ModelExternalSourceAspectIterator it(converter.GetDgnDb(), prov, nullptr);
        for (auto entry = it.begin(); entry != it.end(); ++entry)
            {
            m_v8ModelsSkipped.insert(entry->GetModelId());
            }
        }

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("skip %s (disk file lastmod time unchanged)", Utf8String(file).c_str());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeDetector::_ShouldSkipFile(Converter& converter, DgnV8FileCR v8file)
    {
    if (converter.HasRootTransChanged())  // must re-visit all elements if root transform has changed
        return false;

    // if it hasn't changed per the "last saved time", don't bother with it.
    if (!s_doFileSaveTimeCheck || converter.GetSyncInfo().HasLastSaveTimeChanged(v8file) || converter.GetSyncInfo().ModelHasChangedImagery(converter.GetRepositoryLinkId(v8file)))
        return false;

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("skip %s (dgnfile lastmod time unchanged)", Utf8String((WCharCP)v8file.GetFileName().c_str()).c_str());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_OnModelInserted(Converter& converter, ResolvedModelMapping const& v8mm)
    {
    m_newlyDiscoveredModels.insert(v8mm.GetDgnModel().GetModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeDetector::_AreContentsOfModelUnChanged(Converter& converter, ResolvedModelMapping const& v8mm)
    {
    if (m_newlyDiscoveredModels.find(v8mm.GetDgnModel().GetModelId()) != m_newlyDiscoveredModels.end())
        return false;

    if (!_ShouldSkipFile(converter, *v8mm.GetV8Model().GetDgnFileP()))
        return false;

    m_v8ModelsSkipped.insert(v8mm.GetDgnModel().GetModelId());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_OnModelSeen(Converter& converter, ResolvedModelMapping const& v8mm)
    {
    m_v8ModelsSeen.insert(v8mm.GetDgnModel().GetModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_DeleteModel(DgnModelR model, SyncInfo::V8ModelExternalSourceAspect const& modelXsa)
    {
    LOG.infov("Delete model %llx (%s)", model.GetModelId().GetValue(), model.GetModeledElement()->GetCode().GetValueUtf8CP());
    GetMonitor()._OnModelDelete(model, modelXsa);
    auto& db = model.GetDgnDb();
    auto modeledElementId = model.GetModeledElementId();
    model.Delete();
    db.Elements().Delete(modeledElementId);

#ifdef WIP_OLD_MODEL_PROVENANCE
    if (_WantModelProvenanceInBim())
        DgnV8ModelProvenance::Delete(model.GetModelId(), GetDgnDb());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_DetectDeletedModels(Converter& converter, SyncInfo::V8ModelExternalSourceAspectIterator& iter)
    {
    // *** NB: This alogorithm *infers* that a model was deleted in V8 if we did not see it during processing.
    //          This inference is valid only if we know that a) we saw all models and/or files, and b) they were all registered in m_v8Files/ModelsSkipped or m_v8ModelsSeen.

    // iterate over all of the previously found models to determine if any of 
    // them were missing this time around. Those models and their constituent Models must to be deleted.
    for (auto wasModel=iter.begin(); wasModel!=iter.end(); ++wasModel)
        {
        if (!converter.IsBimModelAssignedToJobSubject(wasModel->GetModelId()))
            continue;

        if (m_v8ModelsSeen.find(wasModel->GetModelId()) == m_v8ModelsSeen.end())
            {
            if (m_v8ModelsSkipped.find(wasModel->GetModelId()) != m_v8ModelsSkipped.end())
                continue;   // we skipped this V8 model, so we don't expect to see it in m_v8ModelsSeen

            // not found, delete this model
            auto model = converter.GetDgnDb().Models().GetModel(wasModel->GetModelId());
            if (!model.IsValid())
                {
                BeAssert(false && "I found a Model *aspect*, and yet I cannot access the model that it points to. That's impossible.:");
                continue;
                }
            converter._DeleteModel(*model, *wasModel);

            // Note that DetectDeletedElements will take care of detecting and deleting the elements that were in the V8 model.
            }
        }    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_DeleteElement(DgnElementId eid)
    {
    LOG.tracev("Delete element %lld", eid.GetValue());

    _OnElementBeforeDelete(eid);
    GetDgnDb().Elements().Delete(eid);

    _OnElementConverted(eid, nullptr, Converter::ChangeOperation::Delete);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_DetectDeletedElements(Converter& converter, SyncInfo::V8ElementExternalSourceAspectIterator& iter)
    {
    // *** NB: This alogorithm *infers* that an element was deleted in V8 if we did not see it, its model, or its file during processing.
    //          This inference is valid only if we know that a) we saw all models and/or files, and b) they were all registered in m_v8Files/ModelsSkipped or m_elementsSeen.

    // iterate over all of the previously converted elements from the syncinfo.
    for (auto elementInSyncInfo=iter.begin(); elementInSyncInfo!=iter.end(); ++elementInSyncInfo)
        {
        auto previouslyConvertedElementId = elementInSyncInfo->GetElementId();
        if (m_elementsSeen.Contains(previouslyConvertedElementId)) // if update encountered at least one V8 element that was mapped to this BIM element,
            continue;   // keep this BIM element alive

        if (m_v8ModelsSkipped.find(elementInSyncInfo->GetModelId()) != m_v8ModelsSkipped.end()) // if we skipped this whole V8 model (e.g., because it was unchanged),
            continue;   // we don't expect any element from it to be in m_elementsSeen. Keep them all alive.

#ifdef WIP_EXTERNAL_SOURCE_ASPECT // detect if any parent is in m_elementsSeen
#endif

        // We did not encounter the V8 element that was mapped to this BIM element. We infer that the V8 element 
        // was deleted. Therefore, the update to the BIM is to delete the corresponding BIM element.
        converter._DeleteElement(previouslyConvertedElementId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_DetectDeletedElementsInFile(Converter& converter, DgnV8FileR v8File)
    {
    // iterate over all of the previously found elements from the syncinfo to determine if any of 
    // them were missing this time around. Those elements need to be deleted.
    SyncInfo::V8ModelExternalSourceAspectIterator modelsInFile(*converter.GetRepositoryLinkElement(v8File));
    for (auto modelInFile = modelsInFile.begin(); modelInFile != modelsInFile.end(); ++modelInFile)
        {
        if (!converter.IsBimModelAssignedToJobSubject(modelInFile->GetModelId()))
            continue;
        auto model = converter.GetDgnDb().Models().GetModel(modelInFile->GetModelId());
        if (!model.IsValid())
            continue; // could happen if modeled element still exists but submodel itself does not.
        SyncInfo::V8ElementExternalSourceAspectIterator elementsInModel(*model);
        _DetectDeletedElements(converter, elementsInModel);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_DetectDeletedModelsInFile(Converter& converter, DgnV8FileR v8File)
    {
    SyncInfo::V8ModelExternalSourceAspectIterator iter(*converter.GetRepositoryLinkElement(v8File));
    _DetectDeletedModels(converter, iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::IsDocumentInRegistry(Utf8StringCR docGuidStr, Utf8String localFileName)
    {
    BeGuid docGuid;
    if (BSISUCCESS == docGuid.FromString(docGuidStr.c_str()))
        return _GetParams().IsDocumentInRegistry(docGuidStr);
    
    // If we don't have a document registry, then we must be converting raw disk files. 
    // Pretend that the directory is the registry.
    if (BSISUCCESS == DgnV8Api::DgnFile::ParsePackagedName(nullptr, nullptr, nullptr, WString(localFileName.c_str()).c_str()))
        return true;

    return BeFileName(localFileName.c_str(), true).DoesPathExist();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_DeleteFileAndContents(RepositoryLinkId repositoryLinkId)
    {
    auto rlink = GetRepositoryLinkElement(repositoryLinkId);
    if (!rlink.IsValid())
        return;
    SyncInfo::V8ModelExternalSourceAspectIterator modelsInFile(*rlink);
    for (auto wasModel : modelsInFile)
        {
        auto model = GetDgnDb().Models().GetModel(wasModel.GetModelId());
        if (!model.IsValid())
            {
            BeAssert(false && "I found a Model *aspect*, and yet I cannot access the model that it points to. That's impossible.:");
            continue;
            }

        SyncInfo::V8ElementExternalSourceAspectIterator elementsInModel(*model);

        for (auto wasElement : elementsInModel)
            {
            _DeleteElement(wasElement.GetElementId());
            }

        _DeleteModel(*model, wasModel);
        }

    rlink->Delete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_DetectDeletedDocuments()
    {
    if (!IsUpdating())
        return;

    SyncInfo::RepositoryLinkExternalSourceAspectIterator files(GetDgnDb(), nullptr);
    for (auto file = files.begin(); file != files.end(); ++file)
        {
        if (!IsDocumentInRegistry(file->GetIdentifier(), file->GetFileName()))
            {
            _DeleteFileAndContents(file->GetRepositoryLinkId());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::_DetectDeletedDocuments()
    {
    if (!IsUpdating())
        return;

    T_Super::_DetectDeletedDocuments();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool CreatorChangeDetector::_IsElementChanged(SearchResults& res, Converter& converter, DgnV8EhCR v8eh, 
                                              ResolvedModelMapping const& v8mm, T_SyncInfoElementFilter*)
    {
    res.m_currentElementProvenance = SyncInfo::ElementProvenance(v8eh, converter.GetSyncInfo(), converter.GetCurrentIdPolicy());
    res.m_changeType = ChangeType::Insert;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
void ChangeDetector::_OnViewSeen(Converter&, DgnViewId viewId)
    {
    if (viewId.IsValid())
        m_viewsSeen.insert(viewId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
void Converter::DeleteView(DgnViewId previouslyConvertedViewId, SyncInfo& syncInfo)
    {
    auto&   elements = GetDgnDb().Elements();

    // a special case for a SpatialView: if attached to a Sheet::ViewAttachment, do not delete it:
    bool    isAttached = false;
    for (auto va : elements.MakeIterator(BIS_SCHEMA(BIS_CLASS_ViewAttachment)))
        {
        auto viewAttachment = elements.Get<Sheet::ViewAttachment>(va.GetElementId());
        if (viewAttachment.IsValid() && viewAttachment->GetAttachedViewId() == previouslyConvertedViewId)
            {
            isAttached = true;
            break;
            }
        }
    if (!isAttached)
        {
        ViewDefinitionCPtr tempView = GetDgnDb().Elements().Get<ViewDefinition>(previouslyConvertedViewId);
        DgnElementId categorySelectorId = tempView->GetCategorySelectorId();
        DgnElementId displayStyleId = tempView->GetDisplayStyleId();
        DgnElementId modelSelectorId;

        if (tempView->ToSpatialView() != nullptr)
                modelSelectorId = tempView->ToSpatialView()->GetModelSelectorId();

        elements.Delete(previouslyConvertedViewId); // Note that cascading delete takes care of deleting the ExternalSourceAspect, if any.

        // Clean up the component elements
        // Category Selector
        BeSQLite::EC::ECSqlStatement stmt;
        Utf8PrintfString sql("SELECT 1 FROM %s WHERE %s.Id = ?", BIS_SCHEMA(BIS_CLASS_ViewDefinition), BIS_CLASS_CategorySelector);
        stmt.Prepare(GetDgnDb(), sql.c_str());
        stmt.BindId(1, categorySelectorId);
        auto rc = stmt.Step();
        if (BE_SQLITE_DONE == rc)
            elements.Delete(categorySelectorId);

        BeSQLite::EC::ECSqlStatement stmt2;
        Utf8PrintfString sql2("SELECT 1 FROM %s WHERE %s.Id = ?", BIS_SCHEMA(BIS_CLASS_ViewDefinition), BIS_CLASS_DisplayStyle);
        stmt2.Prepare(GetDgnDb(), sql2.c_str());
        stmt2.BindId(1, displayStyleId);
        rc = stmt2.Step();
        if (BE_SQLITE_DONE == rc)
            elements.Delete(displayStyleId);

        if (modelSelectorId.IsValid())
            {
            BeSQLite::EC::ECSqlStatement stmt3;
            Utf8PrintfString sql3("SELECT 1 FROM %s WHERE %s.Id = ?", BIS_SCHEMA(BIS_CLASS_SpatialViewDefinition), BIS_CLASS_ModelSelector);
            stmt3.Prepare(GetDgnDb(), sql3.c_str());
            stmt3.BindId(1, modelSelectorId);
            rc = stmt3.Step();
            if (BE_SQLITE_DONE == rc)
                elements.Delete(modelSelectorId);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
void ChangeDetector::_DetectDeletedViews(Converter& converter, SyncInfo::ViewDefinitionExternalSourceAspectIterator &iter)
    {
    auto&   elements = converter.GetDgnDb().Elements();
    auto&   syncInfo = converter.GetSyncInfo();

    for (auto viewSyncInfo = iter.begin(); viewSyncInfo != iter.end(); ++viewSyncInfo)
        {
        auto previouslyConvertedViewId = viewSyncInfo->GetViewId();

        if (m_viewsSeen.find(previouslyConvertedViewId) != m_viewsSeen.end())
            continue;
        
        converter.DeleteView(previouslyConvertedViewId, syncInfo);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
void ChangeDetector::_DetectDeletedViewsInFile(Converter& converter, DgnV8FileR v8File)
    {
    auto scope = converter.GetRepositoryLinkId(v8File);
    auto iterator = SyncInfo::ViewDefinitionExternalSourceAspectIterator(converter.GetDgnDb(), scope);
    _DetectDeletedViews(converter, iterator);
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
