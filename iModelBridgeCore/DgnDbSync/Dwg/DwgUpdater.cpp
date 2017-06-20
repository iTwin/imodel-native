/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgUpdater.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DwgImportInternal.h"

BEGIN_DGNDBSYNC_DWG_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgUpdater::_BeginImport ()
    {
    m_byIdIter   = new DwgSyncInfo::ByDwgObjectIdIter (GetDgnDb());
    m_byHashIter = new DwgSyncInfo::ByHashIter (GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgUpdater::_FinishImport ()
    {
    T_Super::_FinishImport ();

    // empty deleted elements & models from the syncInfo
    DwgSyncInfo::ElementIterator    allElementsInSyncInfo(GetDgnDb(), nullptr);
    DetectDeletedElements (allElementsInSyncInfo);
    
    DwgSyncInfo::ModelIterator      allModelsInSyncInfo(GetDgnDb(), nullptr);
    DetectDeletedModels (allModelsInSyncInfo);

    DeleteDeletedMaterials ();

    // update syncinfo for input DWG file
    DwgSyncInfo&    syncInfo = GetSyncInfo ();
    StableIdPolicy  idPolicy = GetCurrentIdPolicy ();
    DwgSyncInfo::FileProvenance     masterProv(GetDwgDb(), syncInfo, idPolicy);
    masterProv.Update ();

    // update syncinfo for xref files
    for (auto& xref : m_loadedXrefFiles)
        {
        DwgDbDatabaseCP     dwg = xref.GetDatabase ();
        if (nullptr != dwg)
            {
            DwgSyncInfo::FileProvenance xrefProv(*dwg, syncInfo, idPolicy);
            xrefProv.Update ();
            }
        }

    // release memory
    DELETE_AND_CLEAR (m_byHashIter);
    DELETE_AND_CLEAR (m_byIdIter);
    m_elementsSeen.clear();
    m_dwgModelsSeen.clear ();
    m_dwgModelsSkipped.clear ();
    m_newlyDiscoveredModels.clear ();
    m_elementsDiscarded = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgUpdater::CheckSameRootModelAndUnits ()
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
    if (BSISUCCESS != this->GetSyncInfo().FindModel(&modelInfo, modelspace->GetObjectId(), &trans) || modelInfo.GetSource() != this->GetRootGuest().m_dwgRootModel)
        {
        ReportSyncInfoIssue (IssueSeverity::Fatal, IssueCategory::Sync(), Issue::RootModelChanged(), "");
        OnFatalError ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgUpdater::_OnUpdateLayer (DgnCategoryId& id, DwgDbLayerTableRecordCR layer)
    {
    DwgDbDatabasePtr    dwg = layer.GetDatabase ();
    if (dwg.IsNull())
        return  BSIERROR;

    if (this->ShouldSkipFile(*dwg.get()))
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

        if (!name.EqualsI(categoryCode.GetValue()))
            {
            categoryCode = DgnCode (categoryCode.GetCodeSpecId(), name, categoryCode.GetScope());
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
BentleyStatus   DwgUpdater::_OnUpdateLineType (DgnStyleId& id, DwgDbLinetypeTableRecordCR linetype)
    {
    // don't bother with linetypes CONTINUOUS, ByLayer and ByBlock
    if (linetype.IsContinuous() || linetype.IsByLayer() || linetype.IsByBlock())
        return  BSISUCCESS;

    DwgDbDatabasePtr    dwg = linetype.GetDatabase ();
    if (dwg.IsNull())
        return  BSIERROR;

    if (this->ShouldSkipFile(*dwg.get()))
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
bool DwgUpdater::CheckEntity (DgnElementId& oldId, ElementImportInputs& inputs, DwgSyncInfo::DwgObjectProvenance const& newProv, bool hashBlocks)
    {
    oldId.Invalidate ();

    DwgDbEntityP    entity = inputs.GetEntityP ();
    if (nullptr == entity)
        {
        BeAssert (false && L"bad object!");
        return  false;
        }

#ifdef DEBUG_CHECKENTITY
    uint64_t    entityId = entity->GetObjectId().ToUInt64 ();
    DwgSyncInfo::DwgModelSyncInfoId modelSyncId = inputs.GetModelMapping().GetModelSyncInfoId ();
    LOG_ENTITY.debugv ("checking entity: %lld[%llx] in model %lld[%s]", entityId, entityId, modelSyncId.GetValue(), inputs.GetModelMapping().GetMapping().GetDwgName().c_str());
#endif

    DwgSyncInfo::ElementIterator*       iter = nullptr;

    if (GetCurrentIdPolicy() == StableIdPolicy::ById)
        {
        iter = m_byIdIter;
        m_byIdIter->Bind (inputs.GetModelMapping().GetModelSyncInfoId(), entity->GetObjectId().ToUInt64());
        }
    else
        {
        iter = m_byHashIter;
        m_byHashIter->Bind (inputs.GetModelMapping().GetModelSyncInfoId(), newProv.m_primaryHash, newProv.m_secondaryHash);
        }

    auto found = iter->begin();
    if (found == iter->end())
        {
        // we never saw this entity before, treat it as a new entity. 
        // or maybe it was previously discarded (DwgSyncInfo::WasElementDiscarded). Even so, give the converter another shot at it. Maybe it will convert it this time.
        iter->GetStatement()->Reset();  // NB: don't leave the iterator in an active state!
        // caller calls OnElementSeen after element created.
        return  false;
        }

    // This entity was previously mapped to an element(s) in the DgnDb. See if it has changed and if the target(s) must be updated.
    DwgSyncInfo::DwgObjectProvenance    oldProv = found.GetProvenance ();
    oldId = found.GetElementId ();

    iter->GetStatement()->Reset (); // NB: don't leave the iterator in an active state!

    OnElementSeen (oldId);

#ifdef DUMPHASH
    if (LOG_ENTITY_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
        {
        Utf8String  oldHash, newHash;
        oldProv.m_primaryHash.AsHexString (oldHash);
        newProv.m_primaryHash.AsHexString (newHash);
        LOG_ENTITY.tracev("Primary hash %s ? %s for entity %ls[%lld]", oldHash.c_str(), newHash.c_str(), entity->GetClassName().c_str(), entity->GetObjectId().ToUInt64());

        if (hashBlocks)
            {
            oldProv.m_secondaryHash.AsHexString (oldHash);
            newProv.m_secondaryHash.AsHexString (newHash);
            LOG_ENTITY.tracev("Secondary hash %s ? %s", oldHash.c_str(), newHash.c_str(), entity->GetClassName().c_str(), entity->GetObjectId().ToUInt64());
            }
        }
#endif

    if (oldProv.IsSame(newProv, hashBlocks))
        return  true; // nothing changed, we can skip this element

    // new & old don't match - re-import it:
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgUpdater::_ImportEntity (ElementImportResults& results, ElementImportInputs& inputs)
    {
    DwgDbEntityP    entity = inputs.GetEntityP ();
    DwgDbObjectP    obj = DwgDbObject::Cast (entity);
    if (nullptr == entity || nullptr == obj)
        {
        BeAssert (false && L"bad object!");
        return  BSIERROR;
        }

    DwgSyncInfo::DwgObjectProvenance    newProv(*obj, GetSyncInfo(), GetCurrentIdPolicy(), false);

    DgnElementId    oldId;
    if (this->CheckEntity(oldId, inputs, newProv, false))
        return  static_cast<BentleyStatus>(DgnDbStatus::IdExists);

    results.SetIsUpdating  (true);

    BentleyStatus   status = T_Super::_ImportEntity (results, inputs);
    if (BSISUCCESS != status)
        return  status;

    if (oldId.IsValid())
        {
        // update the re-imported element
        results.m_existingElementId = oldId;
        UpdateResults (results, DgnElementId());
        }
    else
        {
        // add the new element
        if (DgnDbStatus::Success == T_Super::_InsertResults(results, DgnElementId()))
            OnElementSeen (results.m_importedElement.get());
        else
            BeAssert (false && L"Failed inserting an element!");
        }

    UpdateResultsInSyncInfo (results, inputs, newProv);

    return  static_cast<BentleyStatus>(DgnDbStatus::AlreadyLoaded);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgUpdater::_ImportBlockReference (ElementImportResults& results, ElementImportInputs& inputs)
    {
    DwgDbEntityP    entity = inputs.GetEntityP ();
    DwgDbObjectP    obj = DwgDbObject::Cast (entity);
    if (nullptr == entity || nullptr == obj)
        {
        BeAssert (false && L"bad object!");
        return  BSIERROR;
        }

    bool    hashBlocks = this->GetOptions().GetSyncBlockChanges() && DwgDbBlockReference::Cast(entity) != nullptr;

    DwgSyncInfo::DwgObjectProvenance    newProv(*obj, GetSyncInfo(), GetCurrentIdPolicy(), hashBlocks);

    DgnElementId    oldId;
    if (this->CheckEntity(oldId, inputs, newProv, hashBlocks))
        return  static_cast<BentleyStatus>(DgnDbStatus::IdExists);
    
    results.SetIsUpdating  (oldId.IsValid());

    BentleyStatus   status = T_Super::_ImportBlockReference (results, inputs);
    if (BSISUCCESS != status)
        return  status;

    if (oldId.IsValid())
        {
        // update the re-imported element
        OnElementSeen (oldId);
        results.m_existingElementId = oldId;
        UpdateResults (results, DgnElementId());
        }
    else
        {
        // add the new element
        if (DgnDbStatus::Success == T_Super::_InsertResults(results, DgnElementId()))
            OnElementSeen (results.m_importedElement.get());
        else
            BeAssert (false && L"Failed inserting an element!");
        }

    UpdateResultsInSyncInfo (results, inputs, newProv);

    return  static_cast<BentleyStatus>(DgnDbStatus::AlreadyLoaded);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DwgUpdater::UpdateResults (ElementImportResults& results, DgnElementId parentId)
    {
    if (!results.m_importedElement.IsValid())
        {
        BeAssert(false && L"invalid element imported!");
        return DgnDbStatus::BadArg;
        }

    if (!results.m_existingElementId.IsValid())
        return T_Super::_InsertResults(results, parentId);

    DgnElementCPtr existingEl = GetDgnDb().Elements().GetElement (results.m_existingElementId);

    if (!existingEl.IsValid())
        {
        BeAssert(false && L"Invalid existing element!");
        return DgnDbStatus::BadArg;
        }

    DgnElementPtr   writeEl = existingEl->CopyForEdit();   // *** this copying is necessary only because existingEl has the ElementId and results.m_importedElement does not. Inefficient!!
    writeEl->CopyFrom(*results.m_importedElement);      // ***          "                               "                           "
    writeEl->SetCode (existingEl->GetCode());              // *** TRICKY! CopyFrom zeroes out the Code (since results didn't have one)

    DgnDbStatus     status = DgnDbStatus::Success;
    DgnElementCPtr  ret = GetDgnDb().Elements().Update (*writeEl, &status);
    if (!ret.IsValid())
        return status;

    // as of today I'm seeing writeEl to be updated in above call so I have to cast the returned element:
    results.m_importedElement = const_cast <DgnElementP> (ret.get());

    for (ElementImportResults& child : results.m_childElements)
        {
        status = UpdateResults (child, results.m_importedElement->GetElementId());
        if (DgnDbStatus::Success != status)
            return status;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DwgUpdater::UpdateResultsInSyncInfo (ElementImportResults& results, ElementImportInputs& inputs, DwgSyncInfo::DwgObjectProvenance const& prov)
    {
    DwgDbEntityCR   entity = inputs.GetEntity ();
    DwgSyncInfo&    syncInfo = GetSyncInfo ();

    if (!results.m_importedElement.IsValid() || !results.m_importedElement->GetElementId().IsValid())
        {
        ++m_elementsDiscarded;
        syncInfo.InsertDiscardedDwgObject (entity, inputs.GetModelSyncInfoId());
        return  DgnDbStatus::Success;
        }

    // save the element -> element mapping in syncinfo
    if (!results.m_existingElementId.IsValid())
        {
        syncInfo.InsertElement (results.m_importedElement->GetElementId(), entity, prov, inputs.GetModelSyncInfoId());
        }
    else
        {
        BeAssert (results.m_existingElementId == results.m_importedElement->GetElementId());
        syncInfo.UpdateElement (results.m_importedElement->GetElementId(), entity, prov);
        }

    for (ElementImportResults& child : results.m_childElements)
        UpdateResultsInSyncInfo (child, inputs, prov);

    return  DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgUpdater::ShouldSkipFile (DwgDbDatabaseCR dwg)
    {
    // if a DWG file is changed via RealDWG or OpenDWG, its version GUID must have been changed:
    if (!GetSyncInfo().HasVersionGuidChanged(dwg))
        return  true;
    
    // if it hasn't changed per the "last saved time", don't bother with it.
    if (GetSyncInfo().HasLastSaveTimeChanged(dwg))
        return false;

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("skip %s (dgnfile lastmod time unchanged)", Utf8String((WCharCP)dwg.GetFileName().c_str()).c_str());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgUpdater::_ShouldSkipModel (ResolvedModelMapping const& modelMap)
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

    // if the model is new, do not skip it:
    DwgSyncInfo::DwgModelSyncInfoId entryId = modelMap.GetModelSyncInfoId ();
    if (m_newlyDiscoveredModels.find(entryId) != m_newlyDiscoveredModels.end())
        return  false;

    // model is not new, check the file of the object(for an xref it's the insert entity):
    DwgDbDatabasePtr    dwg = modelMap.GetModelInstanceId().GetDatabase ();
    if (dwg.IsNull())
        {
        BeAssert (false && L"model object is not a database resident!");
        return  false;
        }

    if (!ShouldSkipFile(*dwg))
        return false;

    // skip this model
    m_dwgModelsSkipped.insert (entryId);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgUpdater::_OnModelInserted (ResolvedModelMapping const& modelMap)
    {
    if (modelMap.IsValid())
        m_newlyDiscoveredModels.insert (modelMap.GetModelSyncInfoId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgUpdater::_OnModelSeen (ResolvedModelMapping const& modelMap)
    {
    if (modelMap.IsValid())
        m_dwgModelsSeen.insert (modelMap.GetModelSyncInfoId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgUpdater::DetectDeletedModels(DwgSyncInfo::ModelIterator& iter)
    {
    // *** NB: This alogorithm *infers* that a model was deleted in DWG if we did not see it or its file during processing.
    //          This inference is valid only if we know that a) we saw all models and/or files, and b) they were all registered in m_dwgFiles/ModelsSkipped or m_dwgModelsSeen.

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
            LOG.tracev("Delete model %lld", wasModel.GetModelId().GetValue());
            auto model=GetDgnDb().Models().GetModel(wasModel.GetModelId());
            model->Delete();
            GetSyncInfo().DeleteModel (wasModel.GetDwgModelSyncInfoId());

            // Note that DetectDeletedElements will take care of detecting and deleting the elements that were in the DWG model.
            }
        }

    m_dwgModelsSeen.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgUpdater::DetectDeletedElements(DwgSyncInfo::ElementIterator& iter)
    {
    // *** NB: This alogorithm *infers* that an element was deleted in DWG if we did not see it, its model, or its file during processing.
    //          This inference is valid only if we know that a) we saw all models and/or files, and b) they were all registered in m_dwgFiles/ModelsSkipped or m_elementsSeen.

    // iterate over all of the previously found elements from the syncinfo to determine if any of 
    // them were missing this time around. Those elements need to be deleted.
    for (auto wasElement=iter.begin(); wasElement!=iter.end(); ++wasElement)
        {
#ifdef DEBUG_DELETE_ELEMENTS
        LOG_ENTITY.debugv ("Element in SyncInfo[%lld]...", wasElement.GetElementId());
#endif
        if (!m_elementsSeen.Contains(wasElement.GetElementId()))
            {
            DwgSyncInfo::DwgModelSyncInfoId modelRowId = wasElement.GetDwgModelSyncInfoId ();
            if (m_dwgModelsSkipped.find(modelRowId) != m_dwgModelsSkipped.end())
                continue;   // we skipped this DWG model, so we don't expect to see any element from it in m_elementsSeen

            // not found, delete this entity
            LOG.tracev("Delete element %lld", wasElement.GetElementId().GetValue());

            GetDgnDb().Elements().Delete(wasElement.GetElementId());
            GetSyncInfo().DeleteElement(wasElement.GetElementId());
            }
        }
    
    m_elementsSeen.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgUpdater::OnElementSeen (DgnElementId elementId) 
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
void DwgUpdater::DeleteDeletedMaterials ()
    {
    // collect materials from syncInfo:
    DwgSyncInfo::MaterialIterator syncMaterials (this->GetDgnDb());

    // collect the delete list from the sync info list which are not seen in the db list.
    bset<DgnMaterialId>  deleteList;
    for (auto entry : syncMaterials)
        {
        DgnMaterialId   id = entry.GetDgnMaterialId ();

        auto found = std::find_if (m_importedMaterials.begin(), m_importedMaterials.end(), [&](T_DwgDgnMaterialId map){return map.second == id;});
        if (m_importedMaterials.end() == found)
            deleteList.insert (id);
        }

    if (!deleteList.empty())
        {
        // delete unseen materials from both db and syncinfo
        DgnElements&    dbElements = this->GetDgnDb().Elements ();
        DwgSyncInfo&    syncInfo = this->GetSyncInfo ();

        for (auto id : deleteList)
            {
            dbElements.Delete (id);
            syncInfo.DeleteMaterial (id);
            }
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgProtocalExtension::_ConvertToBim (ProtocalExtensionContext& context, DwgUpdater& updater)
    {
    /*-----------------------------------------------------------------------------------
    This is the default implementation of _UpdateBim - it assumes that both the input entity
    and the output element are singles.  Complex types should use the default.
    -----------------------------------------------------------------------------------*/
    DwgDbObjectCP   obj = DwgDbObject::Cast (&context.GetEntity());
    if (nullptr == obj)
        return  BSIERROR;

    DwgSyncInfo::DwgObjectProvenance    newProv (*obj, updater.GetSyncInfo(), updater.GetCurrentIdPolicy(), false);
    DwgImporter::ElementImportInputs&   inputs = context.GetElementInputsR ();
    DwgImporter::ElementImportResults&  results = context.GetElementResultsR ();

    DgnElementId    oldId;
    if (updater.CheckEntity(oldId, inputs, newProv, false))
        return  static_cast<BentleyStatus>(DgnDbStatus::IdExists);
    
    results.SetIsUpdating (oldId.IsValid());

    BentleyStatus   status = this->_ToBim (context, updater);
    if (BSISUCCESS != status)
        return  status;

    if (oldId.IsValid())
        {
        // update the re-imported view attachment
        updater.OnElementSeen (oldId);
        results.m_existingElementId = oldId;
        updater.UpdateResults (results, DgnElementId());
        }
    else
        {
        // add the new view attachment
        if (DgnDbStatus::Success == updater._InsertResults(results, DgnElementId()))
            updater.OnElementSeen (results.m_importedElement.get());
        else
            BeAssert (false && L"Failed inserting an element!");
        }

    updater.UpdateResultsInSyncInfo (results, inputs, newProv);

    return  static_cast<BentleyStatus>(DgnDbStatus::AlreadyLoaded);
    }

END_DGNDBSYNC_DWG_NAMESPACE
