/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgUpdater.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DwgImportInternal.h"

BEGIN_DWG_NAMESPACE

static bset<Utf8String> s_loggedFileNames;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgImporter::_SetChangeDetector (bool updating)
    {
    if (updating)
        m_changeDetector.reset (new UpdaterChangeDetector());
    else
        m_changeDetector.reset (new CreatorChangeDetector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedImportJob DwgImporter::GetResolvedImportJob (DwgSyncInfo::ImportJob const& importJob)
    {
    auto jobSubject = GetDgnDb().Elements().Get<Subject>(importJob.GetSubjectId());
    if (jobSubject.IsValid())
        return ResolvedImportJob (importJob, *jobSubject);
    return ResolvedImportJob();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_Prepare (DwgImporter& importer)
    {
    m_byIdIter   = new DwgSyncInfo::ByDwgObjectIdIter (importer.GetDgnDb());
    m_byHashIter = new DwgSyncInfo::ByHashIter (importer.GetDgnDb());

#ifdef DEBUG_CHECKENTITY
    DwgSyncInfo::ElementIterator  all(importer.GetDgnDb(), nullptr);
    for (auto el = all.begin(); el != all.end(); ++el)
        LOG_ENTITY.debugv("Seen elementId=%lld, entityId=%lld[%x], syncModelId=%d", el.GetElementId(), el.GetDwgObjectId(), el.GetDwgObjectId(), el.GetDwgModelSyncInfoId().GetValue());
#endif  // DEBUG_CHECKENTITY
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_Cleanup (DwgImporter& importer)
    {
    // release memory
    DELETE_AND_CLEAR (m_byHashIter);
    DELETE_AND_CLEAR (m_byIdIter);
    m_elementsSeen.clear();
    m_dwgModelsSeen.clear ();
    m_viewsSeen.clear ();
    m_dwgModelsSkipped.clear ();
    m_newlyDiscoveredModels.clear ();
    m_elementsDiscarded = 0;

    s_loggedFileNames.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
UpdaterChangeDetector::~UpdaterChangeDetector ()
    {
    // ensure that we release the statements held by the "Iter" member variables, in case of abort.
    DELETE_AND_CLEAR (m_byHashIter);
    DELETE_AND_CLEAR (m_byIdIter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgImporter::CheckSameRootModelAndUnits ()
    {
    DwgDbBlockTableRecordPtr    modelspace(GetModelSpaceId(), DwgDbOpenMode::ForRead);
    if (modelspace.IsNull())
        {
        OnFatalError ();
        return;
        }

    // Note: FindModelInDwgSyncInfo will fail if m_rootTrans (i.e., units scaling) is different from what it was at create time.
    DwgSyncInfo::DwgModelMapping    modelInfo;
    Transform                       trans = GetRootTransform ();
    if (BSISUCCESS != this->GetSyncInfo().FindModel(&modelInfo, modelspace->GetObjectId(), &trans) || 
        modelInfo.GetDwgModelSyncInfoId() != this->GetImportJob().GetDwgModelSyncInfoId())
        {
        ReportSyncInfoIssue (IssueSeverity::Fatal, IssueCategory::Sync(), Issue::RootModelChanged(), "");
        OnFatalError ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_OnUpdateLayer (DgnCategoryId& id, DwgDbLayerTableRecordCR layer)
    {
    DwgDbDatabasePtr    dwg = layer.GetDatabase ();
    if (dwg.IsNull())
        return  BSIERROR;

    if (this->_GetChangeDetector()._ShouldSkipFile(*this, *dwg.get()))
        return  BSISUCCESS;
        
    SpatialCategoryCPtr cat = SpatialCategory::Get (this->GetDgnDb(), id);
    if (!cat.IsValid())
        return  BSIERROR;
    
    DgnElementPtr   writeEl = cat->CopyForEdit ();
    DgnCode         categoryCode = cat->GetCode ();
    DgnDbStatus     status;
    bool            changed = false;

    // layer 0 uses Unrecoginized category - don't bother checking the name
    if (id != this->GetUncategorizedCategory() && layer.GetObjectId() != dwg->GetLayer0Id())
        {
        Utf8String  name(DwgHelper::ToUtf8CP(layer.GetName(), true));
        DgnDbTable::ReplaceInvalidCharacters(name, DgnCategory::GetIllegalCharacters(), '_');
        name.Trim ();

        if (!categoryCode.GetValue().Equals(name.c_str()))
            {
            categoryCode = DgnCode (categoryCode.GetCodeSpecId(), categoryCode.GetScopeElementId(this->GetDgnDb()), name);
            writeEl->SetCode (categoryCode);
            changed = true;
            }
        }

    DgnSubCategoryCPtr  subCat = DgnSubCategory::Get (GetDgnDb(), cat->GetDefaultSubCategoryId());
    if (subCat.IsValid())
        {
        DgnSubCategory::Appearance  appear;
        this->GetLayerAppearance (appear, layer);

        if (!appear.IsEqual(subCat->GetAppearance()))
            {
            DgnSubCategoryPtr  writeSubcat = subCat->MakeCopy<DgnSubCategory> ();
            if (writeSubcat.IsValid())
                {
                writeSubcat->GetAppearanceR() = appear;
                if (writeSubcat->Update(&status).IsValid())
                    changed = true;
                }
            }
        }

    if (changed)
        {
        DgnElementCPtr  newCat = writeEl->Update (&status);
        if (!newCat.IsValid())
            return  BSIERROR;
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_OnUpdateLineType (DgnStyleId& id, DwgDbLinetypeTableRecordCR linetype)
    {
    // don't bother with linetypes CONTINUOUS, ByLayer and ByBlock
    if (linetype.IsContinuous() || linetype.IsByLayer() || linetype.IsByBlock())
        return  BSISUCCESS;

    DwgDbDatabasePtr    dwg = linetype.GetDatabase ();
    if (dwg.IsNull())
        return  BSIERROR;

    if (this->_GetChangeDetector()._ShouldSkipFile(*this, *dwg.get()))
        return  BSISUCCESS;
        
    // WIP - currently loading LineStyleElement will hit the assertion of alloc'ed bytes in Element::Destroy() while destructing DwgUpdater.
    LineStyleElementPtr el = LineStyleElement::GetForEdit (this->GetDgnDb(), id);
    if (!el.IsValid() || el->GetData().empty())
        return  BSIERROR;
    
    Utf8String  name (linetype.GetName().c_str());
    if (!name.EqualsI(el->GetName()))
        {
        // linetype name has been changed, update the element
        el->SetName (name.c_str());
        if (!el->Update().IsValid())
            return  BSIERROR;
        // update the mapping in SyncInfo:
        return m_syncInfo.UpdateLinetype (id, linetype);
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool    CreatorChangeDetector::_IsElementChanged (DetectionResults& results, DwgImporter& importer, DwgDbObjectCR obj, ResolvedModelMapping const& modelMap, T_DwgSyncInfoElementFilter* filter)
    {
    // will always insert the new element
    DwgSyncInfo::DwgObjectProvenance    newProv(obj, importer.GetSyncInfo(), importer.GetCurrentIdPolicy(), importer.GetOptions().GetSyncBlockChanges());
    results.SetObjectProvenance (newProv);
    results.SetChangeType (ChangeType::Insert);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool    UpdaterChangeDetector::IsUpdateRequired (DetectionResults& results, DwgImporter& importer, DwgDbObjectCR obj) const
    {
    /*-----------------------------------------------------------------------------------
    Cases we need to force an element update:

    1) If the root transform has been changed, we need to update all elements, even those in 
        a papserspace.  A paperspace element is not impacted by the root transform change, but
        we have to record it as a seen element so it won't get deleted at the end.
    2) For an Xref insert entity in a paperspace, we need to recurse into xref's entity section
        such that we can detect element changes made in the xref file.
    -----------------------------------------------------------------------------------*/
    if (results.GetChangeType() == ChangeType::None)
        {
        // case 1 - root transform has changed
        if (importer.HasRootTransformChanged())
            return  true;

        // case 2 - an xref insert in a paperspace
        if (importer.GetCurrentSpaceId() != importer.GetModelSpaceId())
            {
            DwgDbBlockReferenceP insert = DwgDbBlockReference::Cast (&obj);
            return insert != nullptr && insert->IsXAttachment();
            }
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool    UpdaterChangeDetector::_IsElementChanged (DetectionResults& results, DwgImporter& importer, DwgDbObjectCR obj, ResolvedModelMapping const& modelMap, T_DwgSyncInfoElementFilter* filter)
    {
    DwgSyncInfo::DwgObjectProvenance    newProv(obj, importer.GetSyncInfo(), importer.GetCurrentIdPolicy(), importer.GetOptions().GetSyncBlockChanges());
    results.SetObjectProvenance (newProv);

#ifdef DEBUG_CHECKENTITY
    uint64_t    entityId = obj.GetObjectId().ToUInt64 ();
    DwgString   name = obj.GetDxfName ();
    DwgSyncInfo::DwgModelSyncInfoId modelSyncId = modelMap.GetModelSyncInfoId ();
    LOG_ENTITY.debugv ("checking entity[%ls]: %lld[%llx] in model %lld[%s]", name.c_str(), entityId, entityId, modelSyncId.GetValue(), modelMap.GetMapping().GetDwgName().c_str());
#endif

    DwgSyncInfo::ElementIterator*       iter = nullptr;
    if (importer.GetCurrentIdPolicy() == StableIdPolicy::ById)
        {
        iter = m_byIdIter;
        m_byIdIter->Bind (modelMap.GetModelSyncInfoId(), obj.GetObjectId().ToUInt64());
        }
    else
        {
        iter = m_byHashIter;
        m_byHashIter->Bind (modelMap.GetModelSyncInfoId(), newProv.GetPrimaryHash(), newProv.GetSecondaryHash());
        }

    auto found = iter->begin();
    if (nullptr != filter)
        {
        // filter supplied, try it
        while ((found != iter->end()) && !(*filter)(found, importer))
            ++found;
        }
    if (found == iter->end())
        {
        // we never saw this entity before, treat it as a new entity. 
        // or maybe it was previously discarded (DwgSyncInfo::WasElementDiscarded). Even so, give the converter another shot at it. Maybe it will convert it this time.
        results.SetChangeType (ChangeType::Insert);
        iter->GetStatement()->Reset();  // NB: don't leave the iterator in an active state!
        return  true;
        }

    // This entity was previously mapped to at least one element in the BIM. See if the entity has changed.
    DwgSyncInfo::DwgObjectProvenance    oldProv = found.GetProvenance ();

    results.SetChangeType (newProv.IsSame(oldProv) ? ChangeType::None : ChangeType::Update);
    results.SetObjectMapping (found.GetObjectMapping());

    iter->GetStatement()->Reset (); // NB: don't leave the iterator in an active state!

#ifdef DUMPHASH
    if (LOG_ENTITY_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
        {
        Utf8String  oldHash, newHash;
        oldProv.m_primaryHash.AsHexString (oldHash);
        newProv.m_primaryHash.AsHexString (newHash);
        LOG_ENTITY.tracev("Primary hash %s ? %s for entity %ls[%lld]", oldHash.c_str(), newHash.c_str(), obj.GetClassName().c_str(), obj.GetObjectId().ToUInt64());

        if (hashBlocks)
            {
            oldProv.m_secondaryHash.AsHexString (oldHash);
            newProv.m_secondaryHash.AsHexString (newHash);
            LOG_ENTITY.tracev("Secondary hash %s ? %s", oldHash.c_str(), newHash.c_str(), obj.GetClassName().c_str(), obj.GetObjectId().ToUInt64());
            }
        }
#endif

    if (this->IsUpdateRequired(results, importer, obj))
        results.SetChangeType (ChangeType::Update);

    // if new & old don't match, we will have to either import anew or update it:
    return (results.GetChangeType() != ChangeType::None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DwgImporter::UpdateResults (ElementImportResults& results, DgnElementId existingElementId)
    {
    if (!results.m_importedElement.IsValid() || !existingElementId.IsValid())
        {
        BeAssert(false && L"invalid element imported!");
        return DgnDbStatus::BadArg;
        }

    DgnElementCPtr existingEl = GetDgnDb().Elements().GetElement (existingElementId);
    if (!existingEl.IsValid())
        {
        BeAssert(false && L"Invalid existing element!");
        return DgnDbStatus::BadArg;
        }

    if (existingEl->GetElementClassId() != results.m_importedElement->GetElementClassId())
        {
        this->ReportIssueV (IssueSeverity::Error, IssueCategory::Unsupported(), Issue::UpdateDoesNotChangeClass(), nullptr,
            m_issueReporter.FmtElement (*existingEl).c_str(), results.m_importedElement->GetElementClass()->GetECSqlName().c_str());
        }

    // get an editable element
    DgnElementPtr   writeEl = existingEl->CopyForEdit();   // *** this copying is necessary only because existingEl has the ElementId and results.m_importedElement does not. Inefficient!!
    if (!writeEl.IsValid())
        return  DgnDbStatus::WriteError;

    writeEl->CopyFrom(*results.m_importedElement);

    // reset the code if changed
    DgnCode code = existingEl->GetCode ();
    DgnCode newCode = writeEl->GetCode();
    if (newCode.IsValid() && newCode != code)
        code = newCode;
    writeEl->SetCode (code);

    DgnDbStatus     status = DgnDbStatus::Success;
    DgnElementCPtr  ret = GetDgnDb().Elements().Update (*writeEl, &status);
    if (!ret.IsValid())
        return status;

    // Note that we don't plan to modify the result after this. We just
    // want the output to reflect the outcome. Since, we have a non-const
    // pointer, we have to make a copy.
    results.m_importedElement = ret->CopyForEdit ();

    if (results.m_childElements.empty())
        return  status;

    // now update results for children
    DgnElementId    parentId = results.m_importedElement->GetElementId ();
    for (auto& child : results.m_childElements)
        child.m_importedElement->SetParentId (parentId, m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
    
    // While we could just delete all existing children and insert all new ones, we try to do better.
    // If we can figure out how the new children map to existing children, we can update them. 

    // Note that in the update logic below we don't delete existing children that were not mapped. 
    // Instead, we just refrain from calling the change detector's _OnElementSeen method on unmatched child elements. 
    // That will allow the updater in its final phase to infer that they should be deleted.

    // The best way is if an extension sets up the DgnElementId of the child elements in parentConversionResults. 
    auto const& firstChild = results.m_childElements.front();
    if (firstChild.m_importedElement.IsValid() && firstChild.m_importedElement->GetElementId().IsValid())
        {
        auto existingChildIdSet = results.m_importedElement->QueryChildren ();
        for (auto& childResults : results.m_childElements)
            {
            if (!childResults.m_importedElement.IsValid())
                continue;

            auto existingChildElementId = childResults.m_importedElement->GetElementId();
            auto found = existingChildIdSet.find (existingChildElementId);
            if (found != existingChildIdSet.end())
                {
                this->UpdateResults (childResults, existingChildElementId);
                // *** WIP_CONVERTER - bail out if any child update fails?
                }
            }
        return DgnDbStatus::Success;
        }

    // If we have to guess, we just try to match them up 1:1 in sequence. 
    bvector<DgnElementId>   existingChildren;
    CachedStatementPtr      stmt = GetDgnDb().Elements().GetStatement("SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ParentId=?");
    stmt->BindId (1, parentId);
    while (BE_SQLITE_ROW == stmt->Step())
        existingChildren.push_back (stmt->GetValueId<DgnElementId>(0));

    size_t  existingCount = existingChildren.size ();
    if (existingCount > results.m_childElements.size())
        existingCount = results.m_childElements.size ();

    // update existing children
    size_t  i = 0;
    for (; i < existingCount; ++i)
        {
        this->UpdateResults (results.m_childElements.at(i), existingChildren.at(i));
        // *** WIP_CONVERTER - bail out if any child update fails?
        }

    // insert the left overs as new
    for (; i < results.m_childElements.size(); ++i)
        {
        this->InsertResults (results.m_childElements.at(i));
        // *** WIP_CONVERTER - bail out if any child insertion fails?
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool HasFileNameBeenLogged (Utf8StringCR filename)
    {
    if (s_loggedFileNames.find(filename) != s_loggedFileNames.end())
        return  true;
    s_loggedFileNames.insert (filename);
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool UpdaterChangeDetector::_ShouldSkipFile (DwgImporter& importer, DwgDbDatabaseCR dwg)
    {
    // if a DWG file is changed via an AutoCAD based product, its version GUID must have been changed:
    if (importer.GetOptions().GetSyncDwgVersionGuid() && !importer.GetSyncInfo().HasVersionGuidChanged(dwg))
        return  true;
    
    // if it hasn't changed per the "last saved time", don't bother with it.
    if (importer.GetSyncInfo().HasLastSaveTimeChanged(dwg))
        return false;

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        {
        Utf8String  fn(dwg.GetFileName().c_str());
        if (!HasFileNameBeenLogged(fn))
            LOG.tracev("skip %s (dgnfile lastmod time unchanged)", fn.c_str());
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool UpdaterChangeDetector::_ShouldSkipModel (DwgImporter& importer, ResolvedModelMapping const& modelMap, DwgDbDatabaseCP xrefDwg)
    {
    if (!modelMap.IsValid())
        {
        BeAssert (false && "attempt to update a null DWG model!");
        return  false;
        }

#ifdef DEBUG_SKIP_MODELS
    uint64_t    dwgId = modelMap.GetModelInstanceId().ToUInt64 ();
    LOG_MODEL.debugv ("Testing model from syncInfo %lld, InstanceId=%lld[%llx], %s", modelMap.GetModelSyncInfoId().GetValue(), dwgId, dwgId, modelMap.GetMapping().GetDwgName().c_str());
#endif

    // update all models if the root transformation has been changed - even in paperspaces, to prevent models to be deleted:
    if (importer.HasRootTransformChanged())
        return  false;

    // if the model is new, do not skip it:
    DwgSyncInfo::DwgModelSyncInfoId entryId = modelMap.GetModelSyncInfoId ();
    if (m_newlyDiscoveredModels.find(entryId) != m_newlyDiscoveredModels.end())
        return  false;

    // model is not new, check the file of the object((for an xref it's the insert entity):
    DwgDbDatabasePtr    dwg = modelMap.GetModelInstanceId().GetDatabase ();
    if (dwg.IsNull())
        {
        BeAssert (false && L"model object is not a database resident!");
        return  false;
        }

    if (!this->_ShouldSkipFile(importer, *dwg))
        return false;

    // for an xRef, also check the xRef DWG file, in addition to the above DWG that owns the insert entity:
    if (nullptr != xrefDwg && !this->_ShouldSkipFile(importer, *xrefDwg))
        return  false;

    // skip this model
    m_dwgModelsSkipped.insert (entryId);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_OnModelInserted (DwgImporter& importer, ResolvedModelMapping const& modelMap, DwgDbDatabaseCP xRef)
    {
    if (modelMap.IsValid())
        m_newlyDiscoveredModels.insert (modelMap.GetModelSyncInfoId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_OnModelSeen (DwgImporter& importer, ResolvedModelMapping const& modelMap)
    {
    if (modelMap.IsValid())
        m_dwgModelsSeen.insert (modelMap.GetModelSyncInfoId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DetectDeletedModels(DwgImporter& importer, DwgSyncInfo::ModelIterator& iter)
    {
    // *** NB: This alogorithm *infers* that a model was deleted in DWG if we did not see it or its file during processing.
    //          This inference is valid only if we know that a) we saw all models and/or files, and b) they were all registered in Files/ModelsSkipped or m_dwgModelsSeen.

    // iterate over all of the previously found models to determine if any of 
    // them were missing this time around. Those models and their constituent Models must to be deleted.
    for (auto wasModel=iter.begin(); wasModel!=iter.end(); ++wasModel)
        {
        DwgSyncInfo::DwgModelSyncInfoId modelRowId = wasModel.GetDwgModelSyncInfoId ();
        if (m_dwgModelsSeen.find(modelRowId) == m_dwgModelsSeen.end())
            {
            if (m_dwgModelsSkipped.find(modelRowId) != m_dwgModelsSkipped.end())
                continue;   // we skipped this DWG model, so we don't expect to see it in m_dwgModelsSeen

            // not found, delete this model
            Utf8CP  name = wasModel.GetDwgName ();
            LOG.tracev("Delete model %lld [%s]", wasModel.GetModelId().GetValue(), nullptr == name ? "no name" : name);
            auto model = importer.GetDgnDb().Models().GetModel(wasModel.GetModelId());
            model->Delete();
            importer.GetSyncInfo().DeleteModel (wasModel.GetDwgModelSyncInfoId());

            // Note that DetectDeletedElements will take care of detecting and deleting the elements that were in the DWG model.
            }
        }
    // do not clear m_dwgModelsSeen until _DetectDeletedModelsEnd is called!
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DetectDeletedModelsInFile (DwgImporter& importer, DwgDbDatabaseR dwg)
    {
    DwgSyncInfo::ModelIterator iter (importer.GetDgnDb(), "DwgFileId=?");
    iter.GetStatement()->BindInt(1, DwgSyncInfo::GetDwgFileId(dwg).GetValue());
    _DetectDeletedModels (importer, iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DetectDeletedElements (DwgImporter& importer, DwgSyncInfo::ElementIterator& iter)
    {
    // *** NB: This alogorithm *infers* that an element was deleted in DWG if we did not see it, its model, or its file during processing.
    //          This inference is valid only if we know that a) we saw all models and/or files, and b) they were all registered in Files/ModelsSkipped or m_elementsSeen.

    // iterate over all of the previously converted elements from the syncinfo.
    for (auto elementInSyncInfo=iter.begin(); elementInSyncInfo!=iter.end(); ++elementInSyncInfo)
        {
        auto previouslyConvertedElementId = elementInSyncInfo.GetElementId();
        if (m_elementsSeen.Contains(previouslyConvertedElementId)) // if update encountered at least one DWG entity that was mapped to this BIM element,
            continue;   // keep this BIM element alive

        if (m_dwgModelsSkipped.find(elementInSyncInfo.GetDwgModelSyncInfoId()) != m_dwgModelsSkipped.end()) // if we skipped this whole DWG model (e.g., because it was unchanged),
            continue;   // we don't expect any element from it to be in m_elementsSeen. Keep them all alive.

        if (importer.GetSyncInfo().IsMappedToSameDwgObject(previouslyConvertedElementId, m_elementsSeen)) // if update encountered at least one DWG entity that this element was mapped to, 
            continue;   // infer that this is a child of an assembly, and the assembly parent is in m_elementsSeen.

        // We did not encounter the DWG entity that was mapped to this BIM element. We infer that the DWG entity 
        // was deleted. Therefore, the update to the BIM is to delete the corresponding BIM element.
        LOG.tracev ("Delete element %lld", previouslyConvertedElementId.GetValue());

        // importer._OnElementBeforeDelete (previouslyConvertedElementId);

        importer.GetDgnDb().Elements().Delete (previouslyConvertedElementId);
        importer.GetSyncInfo().DeleteElement (previouslyConvertedElementId);

        //if (importer._WantProvenanceInBim())
        //    DgnV8ElementProvenance::Delete (previouslyConvertedElementId, importer.GetDgnDb());
        //importer._OnElementConverted (elementInSyncInfo.GetElementId(), nullptr, importer::ChangeOperation::Delete);
        }
    // Let _DetectDeletedElementsEnd clear m_elementsSeen!
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DetectDeletedElementsInFile (DwgImporter& importer, DwgDbDatabaseR dwg)
    {
    // iterate over all of the previously found elements from the syncinfo to determine if any of 
    // them were missing this time around. Those elements need to be deleted.
    DwgSyncInfo::ModelIterator  modelsInFile (importer.GetDgnDb(), "DwgFileId=?");
    modelsInFile.GetStatement()->BindInt (1, DwgSyncInfo::GetDwgFileId(dwg).GetValue());
    for (auto modelInFile = modelsInFile.begin(); modelInFile != modelsInFile.end(); ++modelInFile)
        {
        DwgSyncInfo::ElementIterator elementsInModel(importer.GetDgnDb(), "DwgModelSyncInfoId=?");
        elementsInModel.GetStatement()->BindInt(1, modelInFile.GetDwgModelSyncInfoId().GetValue());
        _DetectDeletedElements (importer, elementsInModel);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_OnElementSeen (DwgImporter& importer, DgnElementId elementId) 
    {
    if (elementId.IsValid())
        {
        m_elementsSeen.insert (elementId);
#ifdef DEBUG_DELETE_ELEMENTS
        LOG_ENTITY.debugv ("Seen element [%lld]", elementId);
#endif
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DetectDeletedMaterials (DwgImporter& importer)
    {
    // collect materials from syncInfo:
    DwgSyncInfo::MaterialIterator syncMaterials (importer.GetDgnDb());
    // collect imported/seen materials
    DwgImporter::T_MaterialIdMap& importedMaterials = importer.GetImportedDgnMaterials ();

    // collect the delete list from the sync info list which are not seen in the db list.
    bset<RenderMaterialId>  deleteList;
    for (auto entry : syncMaterials)
        {
        RenderMaterialId   id = entry.GetRenderMaterialId ();

        auto found = std::find_if (importedMaterials.begin(), importedMaterials.end(), [&](DwgImporter::T_DwgRenderMaterialId map){return map.second == id;});
        if (importedMaterials.end() == found)
            deleteList.insert (id);
        }

    if (!deleteList.empty())
        {
        // delete unseen materials from both db and syncinfo
        DgnElements&    dbElements = importer.GetDgnDb().Elements ();
        DwgSyncInfo&    syncInfo = importer.GetSyncInfo ();

        for (auto id : deleteList)
            {
            dbElements.Delete (id);
            syncInfo.DeleteMaterial (id);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_OnGroupSeen (DwgImporter& importer, DgnElementId groupId)
    {
    if (groupId.IsValid())
        m_groupsSeen.insert (groupId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DetectDeletedGroups (DwgImporter& importer)
    {
    auto groupModel = importer.GetDgnDb().Models().Get<GenericGroupModel>(importer.GetGroupModelId());
    if (!groupModel.IsValid())
        return;

    auto&   elements = importer.GetDgnDb().Elements ();
    auto&   syncInfo = importer.GetSyncInfo ();

    for (auto groupId : groupModel->MakeIterator().BuildIdSet<DgnElementId>())
        {
        if (m_groupsSeen.find(groupId) == m_groupsSeen.end())
            {
            auto group = elements.Get<DgnElement> (groupId);
            LOG.tracev ("Delete group %s (ID=%lld)", group.IsValid() ? group->GetUserLabel() : "??", groupId.GetValue());
            elements.Delete (groupId);
            syncInfo.DeleteGroup (groupId);
            }
        }

    m_groupsSeen.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_OnViewSeen (DwgImporter& importer, DgnViewId viewId)
    {
    if (viewId.IsValid())
        m_viewsSeen.insert (viewId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DetectDeletedViews (DwgImporter& importer)
    {
    auto&   elements = importer.GetDgnDb().Elements ();
    auto&   syncInfo = importer.GetSyncInfo ();
    auto    jobModelId = importer.GetOrCreateJobDefinitionModel()->GetModelId ();

    for (auto const& entry : ViewDefinition::MakeIterator(importer.GetDgnDb()))
        {
        auto viewId = entry.GetId ();
        if (m_viewsSeen.find(viewId) == m_viewsSeen.end())
            {
            // don't delete views not created by us:
            auto view = entry.GetSpatialViewDefinition ();
            if (view.IsValid() && view->GetModel()->GetModelId() != jobModelId)
                continue;

            // a special case for a SpatialView: if attached to a Sheet::ViewAttachment, do not delete it:
            bool    isAttached = false;
            for (auto va : elements.MakeIterator(BIS_SCHEMA(BIS_CLASS_ViewAttachment)))
                {
                auto viewAttachment = elements.Get<Sheet::ViewAttachment>(va.GetElementId());
                if (viewAttachment.IsValid() && viewAttachment->GetAttachedViewId() == viewId)
                    {
                    isAttached = true;
                    break;
                    }
                }
            if (!isAttached)
                {
                elements.Delete (viewId);
                syncInfo.DeleteView (viewId);
                }
            }
        }

    m_viewsSeen.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgImporter::_DetectDeletedDocuments ()
    {
    if (!this->IsUpdating())
        return  BSISUCCESS;

    // this is called after the process is done, so, setup change detector again:
    bool detectorPresent = this->_HaveChangeDetector ();
    if (!detectorPresent)
        {
        this->_SetChangeDetector (true);
        this->_GetChangeDetector()._Prepare (*this);
        }

    auto& detector = this->_GetChangeDetector ();
    auto& db = this->GetDgnDb ();
    auto& syncInfo = this->GetSyncInfo ();  
    DwgSyncInfo::FileIterator files(db, nullptr);

    for (auto file : files)
        {
        // check the file GUID per PW, or check the local file path (when run on command line, VSTS 54783):
        bool    docExists = false;
        BeGuid  docGuid;
        auto name = file.GetUniqueName ();
        if (docGuid.FromString(name.c_str()) == BSISUCCESS)
            docExists = this->GetOptions().IsDocumentAssignedToJob(name);
        else
            docExists = BeFileName(file.GetDwgName().c_str(), true).DoesPathExist ();

        if (docExists)
            continue;

        LOG.tracev ("Document %s has been detected for deletion.", name.c_str());

        // need to delete this file - walk through all model mappings in the sync info:
        DwgSyncInfo::ModelIterator  modelMaps(db, "DwgFileId=?");
        modelMaps.GetStatement()->BindInt (1, file.GetSyncId().GetValue());
        for (auto modelMap : modelMaps)
            {
            // delete elements in DgnModel:
            DwgSyncInfo::ElementIterator elements(db, "DwgModelSyncInfoId=?");
            elements.GetStatement()->BindInt(1, modelMap.GetDwgModelSyncInfoId().GetValue());
            detector._DetectDeletedElements (*this, elements);

            // delete DgnModel from db:
            auto model = db.Models().GetModel (modelMap.GetModelId());
            model->Delete();

            // delele the model mapping from the sync info:
            syncInfo.DeleteModel (modelMap.GetDwgModelSyncInfoId());
            }
        // delete the file mapping from the sync info:
        syncInfo.DeleteFile (file.GetSyncId());
        }

    if (!detectorPresent)
        detector._Cleanup (*this);

    return BSISUCCESS;
    }

END_DWG_NAMESPACE
