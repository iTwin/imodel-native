/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DwgImportInternal.h"

BEGIN_DWG_NAMESPACE

static bset<Utf8String> s_loggedFileNames;
static bset<Utf8String> s_loggedModelNames;

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
void UpdaterChangeDetector::_Prepare (DwgImporter& importer)
    {
#ifdef DEBUG_CHECKENTITY
    DwgSourceAspects::ObjectAspectIterator  all(importer.GetDgnDb(), nullptr);
    for (auto el = all.begin(); el != all.end(); ++el)
        LOG_ENTITY.debugv("Seen elementId=%lld, entityId=%lld[%x], syncModelId=%d", el.GetElementId(), el.GetObjectHandle().AsUInt64(), el.GetObjectHandle().AsUInt64(), el.GetModelId().GetValue());
#endif  // DEBUG_CHECKENTITY
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_Cleanup (DwgImporter& importer)
    {
    // release memory
    m_elementsSeen.clear();
    m_dwgModelsSeen.clear ();
    m_viewsSeen.clear ();
    m_dwgModelsSkipped.clear ();
    m_newlyDiscoveredModels.clear ();
    m_subjectsToRemove.clear ();
    m_elementsDiscarded = 0;

    s_loggedFileNames.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
UpdaterChangeDetector::~UpdaterChangeDetector ()
    {
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
    Transform   trans = GetRootTransform ();
    auto modelAspect = this->GetSourceAspects().FindModelAspect(modelspace->GetObjectId(), GetDwgDb(), trans);
    if (!modelAspect.IsValid())
        OnFatalError (IssueCategory::Sync(), Issue::RootModelChanged(), "maybe different units?");
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
        // also update the aspect
        auto writeAspect = DwgSourceAspects::LinetypeAspect::GetForEdit(*el);
        if (writeAspect.IsValid())
            writeAspect.Update(name);
        else
            return  BSIERROR;
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool    CreatorChangeDetector::_IsElementChanged (DetectionResults& results, DwgImporter& importer, DwgDbObjectCR obj, ResolvedModelMapping const& modelMap, DwgSourceAspects::T_ObjectAspectFilter* filter)
    {
    // will always insert the new element
    DwgSourceAspects::ObjectProvenance  newProv(obj, importer);
    results.SetCurrentProvenance (newProv);
    results.SetChangeType (ChangeType::Insert);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool CreatorChangeDetector::_IsElementChanged (DetectionResults& results, DwgImporter& importer, DwgDbHandleCR handle, DgnModelCR model, DwgSourceAspects::T_ObjectAspectFilter* filter)
    {
    // will always insert the new element
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
bool    UpdaterChangeDetector::_IsElementChanged (DetectionResults& results, DwgImporter& importer, DwgDbObjectCR obj, ResolvedModelMappingCR modelMap, DwgSourceAspects::T_ObjectAspectFilter* filter)
    {
    DgnModelCP  model = modelMap.GetModel();
    if (model == nullptr)
        {
        BeAssert(false && "Null target model, likely an unintialized modelMap!");
        return  false;
        }
    auto objHandle = obj.GetObjectId().GetHandle ();

    DwgSourceAspects::ObjectProvenance  newProv(obj, importer);
    results.SetCurrentProvenance (newProv);

#ifdef DEBUG_CHECKENTITY
    uint64_t    entityId = objHandle.AsUInt64();
    DwgString   name = obj.GetDxfName ();
    LOG_ENTITY.debugv ("checking entity[%ls]: %lld[%llx] in model %lld[%s]", name.c_str(), entityId, entityId, model->GetModelId().GetValue(), modelMap.GetModelAspect().GetDwgModelName().c_str());
#endif

    auto changed = this->_IsElementChanged(results, importer, objHandle, *model, filter);

#ifdef DUMPHASH
    if (LOG_ENTITY_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
        {
        Utf8String  oldHash = results.GetObjectAspect().GetHashString ();
        if (oldHash.empty())
            oldHash.assign ("<null>");
        Utf8String  newHash;
        newProv.GetHash().AsHexString (newHash);
        LOG_ENTITY.tracev("Hash %s ? %s for entity %ls[%lld]", oldHash.c_str(), newHash.c_str(), obj.GetDwgClassName().c_str(), obj.GetObjectId().ToUInt64());
        }
#endif

    if (this->IsUpdateRequired(results, importer, obj))
        results.SetChangeType (ChangeType::Update);

    return  changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool UpdaterChangeDetector::_IsElementChanged (DetectionResults& results, DwgImporter& importer, DwgDbHandleCR sourceHandle, DgnModelCR model, DwgSourceAspects::T_ObjectAspectFilter* filter)
    {
    auto newProv = results.GetCurrentProvenance();
    if (newProv.IsNull())
        {
        LOG_ENTITY.debugv ("Caller must call DetectionResults::SetCurrentProvenance before detecting changes!");
        return  false;
        }

    DwgSourceAspects::ObjectAspect  aspect;
    if (importer.GetCurrentIdPolicy() == StableIdPolicy::ById)
        aspect = importer.GetSourceAspects().FindObjectAspect (sourceHandle, model, filter);
    else
        aspect = importer.GetSourceAspects().FindObjectAspect (newProv, model, filter);

    if (!aspect.IsValid())
        {
        // we never saw this entity before, treat it as a new entity. 
        // or maybe it was previously discarded (DwgSyncInfo::WasElementDiscarded). Even so, give the converter another shot at it. Maybe it will convert it this time.
        results.SetChangeType (ChangeType::Insert);
        return  true;
        }

    // This entity was previously mapped to at least one element in the BIM. See if the entity has changed.
    bool isSame = aspect.IsProvenanceEqual(newProv);
    results.SetChangeType (isSame ? ChangeType::None : ChangeType::Update);
    results.SetObjectAspect (aspect);

    // if new & old don't match, we will have to either import anew or update it:
    return (results.GetChangeType() != ChangeType::None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DwgImporter::UpdateResults (ElementImportResults& results, DgnElementId existingElementId, DwgSourceAspects::ObjectAspect::SourceDataCR source)
    {
    auto importedElement = results.GetImportedElement();
    if (importedElement == nullptr || !existingElementId.IsValid())
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

    if (existingEl->GetElementClassId() != importedElement->GetElementClassId())
        {
        this->ReportIssueV (IssueSeverity::Error, IssueCategory::Unsupported(), Issue::UpdateDoesNotChangeClass(), nullptr,
            m_issueReporter.FmtElement (*existingEl).c_str(), importedElement->GetElementClass()->GetECSqlName().c_str());
        }

    // get an editable element
    DgnElementPtr   writeEl = existingEl->CopyForEdit();   // *** this copying is necessary only because existingEl has the ElementId and results.m_importedElement does not. Inefficient!!
    if (!writeEl.IsValid())
        return  DgnDbStatus::WriteError;

    writeEl->CopyFrom(*importedElement);

    // reset the code if changed
    DgnCode code = existingEl->GetCode ();
    DgnCode newCode = writeEl->GetCode();
    if (newCode.IsValid() && newCode != code)
        code = newCode;
    writeEl->SetCode (code);

    // update the ObjectAspect from the source object, with pre-calculated prevenence
    results.m_existingElementMapping = m_sourceAspects.AddOrUpdateObjectAspect (*writeEl, source);
    BeAssert (results.m_existingElementMapping.IsValid() && "Failed updating a ExternalSourceAspect for a DWG object");
    
    DgnDbStatus     status = DgnDbStatus::Success;
    DgnElementCPtr  ret = GetDgnDb().Elements().Update (*writeEl, &status);
    if (!ret.IsValid())
        return status;

    // Note that we don't plan to modify the result after this. We just
    // want the output to reflect the outcome. Since, we have a non-const
    // pointer, we have to make a copy.
    results.m_importedElement = ret->CopyForEdit ();

    // tell the change detector that we have seen this element
    auto& changeDetector = this->_GetChangeDetector ();
    changeDetector._OnElementSeen (*this, results.m_importedElement->GetElementId());

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
                this->UpdateResults (childResults, existingChildElementId, source);
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
        // set the parent element as the existing element for children, VSTS 127172:
        auto childResults = results.m_childElements.at(i);
        this->UpdateResults (childResults, existingChildren.at(i), source);
        // *** WIP_CONVERTER - bail out if any child update fails?
        }

    // insert the left overs as new
    for (; i < results.m_childElements.size(); ++i)
        {
        auto childResults = results.m_childElements.at(i);
        this->InsertResults (childResults, source);
        changeDetector._OnElementSeen (*this, childResults.GetImportedElementId());
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool HasModelNameBeenLogged (Utf8StringCR modelname)
    {
    if (s_loggedModelNames.find(modelname) != s_loggedModelNames.end())
        return  true;
    s_loggedModelNames.insert (modelname);
    return  false;
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
    if (importer.GetOptions().GetSyncDwgVersionGuid() && !importer.GetSourceAspects().HasVersionGuidChanged(dwg))
        return  true;
    
    // if it hasn't changed per the "last saved time", don't bother with it.
    if (importer.GetSourceAspects().HasLastSaveTimeChanged(dwg))
        return false;

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        {
        Utf8String  fn(dwg.GetFileName().c_str());
        if (!HasFileNameBeenLogged(fn))
            LOG.tracev("skip file %s (last saved time unchanged)", fn.c_str());
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool UpdaterChangeDetector::_ShouldSkipModel (DwgImporter& importer, ResolvedModelMapping const& modelMap, DwgDbDatabaseCP xref)
    {
    if (!modelMap.IsValid())
        {
        BeAssert (false && "attempt to update a null DWG model!");
        return  false;
        }

    // update all models if the root transformation has been changed - even in paperspaces, to prevent models to be deleted:
    if (importer.HasRootTransformChanged())
        return  false;

    // if the model is new, do not skip it:
    auto modelId = modelMap.GetModelId ();
    if (m_newlyDiscoveredModels.find(modelId) != m_newlyDiscoveredModels.end())
        return  false;

    // model is not new, check the file of the object(for an xref attachment, it's the input xref DWG):
    DwgDbDatabaseCP dwg = xref == nullptr ? modelMap.GetModelInstanceId().GetDatabase() : xref;
    if (dwg == nullptr)
        {
        BeAssert (false && L"model object is not a database resident!");
        return  false;
        }

    if (!this->_ShouldSkipFile(importer, *dwg))
        return false;

    if (LOG_IS_SEVERITY_ENABLED(LOG_DEBUG))
        {
        uint64_t    dwgId = modelMap.GetModelInstanceId().ToUInt64 ();
        Utf8String  name = modelMap.GetModel()->GetName();
        if (!HasModelNameBeenLogged(name))
            LOG_MODEL.debugv("Skip model %lld (name=%s, handle=%llx)", modelMap.GetModelId().GetValue(), name.c_str(), dwgId);
        }

    // skip this model
    m_dwgModelsSkipped.insert (modelId);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool UpdaterChangeDetector::_ShouldSkipFileAndModels (DwgImporterR importer, DwgDbDatabaseCR dwg)
    {
    // detect file
    if (!this->_ShouldSkipFile(importer, dwg))
        return  false;

    // find and skip models linked by the file
    BeFileName  filename(dwg.GetFileName().c_str());
    auto rlinkAspect = importer.GetSourceAspects().FindFileByFileName (filename);
    if (rlinkAspect.IsValid())
        {
        DwgSourceAspects::ModelAspectIterator modelAspects(importer.GetDgnDb(), rlinkAspect.GetRepositoryLinkId());
        for (auto modelAspect : modelAspects)
            {
            if (LOG_IS_SEVERITY_ENABLED(LOG_DEBUG))
                LOG_MODEL.debugv("Skip model %lld (name=%s, handle=%llx)", modelAspect.GetModelId().GetValue(), modelAspect.GetDwgModelName().c_str(), modelAspect.GetDwgModelHandle().AsUInt64());
            m_dwgModelsSkipped.insert (modelAspect.GetModelId());
            }
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_OnModelInserted (DwgImporter& importer, ResolvedModelMappingCR modelMap, DwgDbDatabaseCP xRef)
    {
    if (modelMap.IsValid())
        m_newlyDiscoveredModels.insert (modelMap.GetModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_OnModelSeen (DwgImporter& importer, ResolvedModelMappingCR modelMap)
    {
    if (modelMap.IsValid())
        m_dwgModelsSeen.insert (modelMap.GetModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DeleteModel(DgnModelR model)
    {
    auto modelElementId = model.GetModeledElementId();
    auto name = model.GetName ();
    auto& db = model.GetDgnDb ();

    LOG.tracev("Delete model %lld [%s]", model.GetModelId().GetValue(), name.empty() ? "no name" : name.c_str());

    model.Delete();
    db.Elements().Delete (modelElementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DeleteElement (DgnDbR db, DwgSourceAspects::ObjectAspectCR aspect)
    {
    auto elementId = aspect.GetElementId ();
    auto modelId = aspect.GetModelId ();
    auto handle = aspect.GetObjectHandle().AsAscii ();

    LOG.tracev ("Delete element %lld [entity %ls] in model %lld", elementId.GetValue(), handle.c_str(), modelId.GetValue());

    db.Elements().Delete (elementId);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DetectDeletedModels(DwgImporterR importer, DwgSourceAspects::ModelAspectIteratorR iter)
    {
    // *** NB: This alogorithm *infers* that a model was deleted in DWG if we did not see it or its file during processing.
    //          This inference is valid only if we know that a) we saw all models and/or files, and b) they were all registered in Files/ModelsSkipped or m_dwgModelsSeen.

    // iterate over all of the previously found models to determine if any of 
    // them were missing this time around. Those models and their constituent Models must to be deleted.
    for (auto wasModel=iter.begin(); wasModel!=iter.end(); ++wasModel)
        {
        auto modelId = wasModel->GetModelId();
        if (m_dwgModelsSeen.find(modelId) == m_dwgModelsSeen.end())
            {
            if (m_dwgModelsSkipped.find(modelId) != m_dwgModelsSkipped.end())
                continue;   // we skipped this DWG model, so we don't expect to see it in m_dwgModelsSeen

            auto model = importer.GetDgnDb().Models().GetModel(modelId);
            if (!model.IsValid())
                continue;   // should never happen!

            // only delete models owned by this import job
            auto modelElementId = model->GetModeledElementId();
            if (!DwgHelper::IsElementOwnedByJobSubject(model->GetDgnDb(), modelElementId, importer.GetImportJob().GetSubject().GetElementId()))
                continue;

            // not found, delete this model
            this->_DeleteModel (*model);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DetectDeletedModelsInFile (DwgImporter& importer, DwgDbDatabaseR dwg)
    {
    auto rlinkId = importer.GetRepositoryLink (&dwg);
    auto repositoryLink = importer.GetDgnDb().Elements().Get<RepositoryLink>(rlinkId);
    if (repositoryLink.IsValid())
        {
        DwgSourceAspects::ModelAspectIterator iter(*repositoryLink);
        _DetectDeletedModels (importer, iter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DetectDeletedElements (DwgImporterR importer, DwgSourceAspects::ObjectAspectIteratorR iter)
    {
    // *** NB: This alogorithm *infers* that an element was deleted in DWG if we did not see it, its model, or its file during processing.
    //          This inference is valid only if we know that a) we saw all models and/or files, and b) they were all registered in Files/ModelsSkipped or m_elementsSeen.

    auto& db = importer.GetDgnDb ();

    // iterate over all of the previously converted elements from the syncinfo.
    for (auto elementInSyncInfo=iter.begin(); elementInSyncInfo!=iter.end(); ++elementInSyncInfo)
        {
        auto previouslyConvertedElementId = elementInSyncInfo->GetElementId();
        if (m_elementsSeen.Contains(previouslyConvertedElementId)) // if update encountered at least one DWG entity that was mapped to this BIM element,
            continue;   // keep this BIM element alive

        if (m_dwgModelsSkipped.find(elementInSyncInfo->GetModelId()) != m_dwgModelsSkipped.end()) // if we skipped this whole DWG model (e.g., because it was unchanged),
            continue;   // we don't expect any element from it to be in m_elementsSeen. Keep them all alive.

        // We did not encounter the DWG entity that was mapped to this BIM element. We infer that the DWG entity 
        // was deleted. Therefore, the update to the BIM is to delete the corresponding BIM element.
        this->_DeleteElement (db, *elementInSyncInfo);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DetectDeletedElementsInFile (DwgImporter& importer, DwgDbDatabaseR dwg)
    {
    // iterate over all of the previously found elements from the syncinfo to determine if any of 
    // them were missing this time around. Those elements need to be deleted.
    auto rlinkId = importer.GetRepositoryLink (&dwg);
    auto repositoryLink = importer.GetDgnDb().Elements().Get<RepositoryLink>(rlinkId);
    if (!repositoryLink.IsValid())
        return;
    DwgSourceAspects::ModelAspectIterator  modelsInFile (*repositoryLink);
    for (auto modelInFile = modelsInFile.begin(); modelInFile != modelsInFile.end(); ++modelInFile)
        {
        auto model = importer.GetDgnDb().Models().GetModel(modelInFile->GetModelId());
        if (!model.IsValid())
            continue;
        // only delete models owned by this import job
        if (!DwgHelper::IsElementOwnedByJobSubject(model->GetDgnDb(), model->GetModeledElementId(), importer.GetImportJob().GetSubject().GetElementId()))
            continue;
        DwgSourceAspects::ObjectAspectIterator elementsInModel(*model);
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
    auto scopeId = importer.GetRepositoryLink(&importer.GetDwgDb());
    DwgSourceAspects::MaterialAspectIterator syncMaterials (importer.GetDgnDb(), scopeId);
    // collect imported/seen materials
    DwgImporter::T_MaterialIdMap& importedMaterials = importer.GetImportedDgnMaterials ();

    // collect the delete list from the sync info list which are not seen in the db list.
    bset<RenderMaterialId>  deleteList;
    for (auto entry : syncMaterials)
        {
        RenderMaterialId    id(entry.GetElementId().GetValueUnchecked());

        auto found = std::find_if (importedMaterials.begin(), importedMaterials.end(), [&](DwgImporter::T_DwgRenderMaterialId map){return map.second == id;});
        if (importedMaterials.end() == found)
            deleteList.insert (id);
        }

    if (!deleteList.empty())
        {
        // delete unseen materials from both db and syncinfo
        DgnElements&    dbElements = importer.GetDgnDb().Elements ();

        for (auto id : deleteList)
            {
            LOG.tracev ("Delete material %lld", id.GetValue());
            dbElements.Delete (id);
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

    for (auto groupId : groupModel->MakeIterator().BuildIdSet<DgnElementId>())
        {
        if (m_groupsSeen.find(groupId) == m_groupsSeen.end())
            {
            auto group = elements.Get<DgnElement> (groupId);
            LOG.tracev ("Delete group %s (ID=%lld)", group.IsValid() ? group->GetUserLabel() : "??", groupId.GetValue());
            elements.Delete (groupId);
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
    auto&   db = importer.GetDgnDb();
    auto&   elements = db.Elements ();
    auto    jobModelId = importer.GetOrCreateJobDefinitionModel()->GetModelId ();

    for (auto const& entry : ViewDefinition::MakeIterator(importer.GetDgnDb()))
        {
        auto viewId = entry.GetId ();
        if (m_viewsSeen.find(viewId) == m_viewsSeen.end())
            {
            // don't delete views not created by us:
            auto view = ViewDefinition::Get (db, viewId);
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
                elements.Delete (viewId);
            }
        }

    m_viewsSeen.clear ();
    }


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/19
+===============+===============+===============+===============+===============+======*/
struct XRefNode : public RefCountedBase
{
private:
    DwgDbDatabaseP  m_xref;
    DwgDbObjectIdArray  m_insertIds;
    bvector<RefCountedPtr<XRefNode>>    m_children;
    XRefNode*   m_parent;
    size_t  m_nestingDepth;

public:
    explicit XRefNode(DwgDbDatabaseP dwg) : m_xref(dwg), m_parent(nullptr), m_nestingDepth(0) { m_insertIds.clear(); }

#define DEBUG_XREF_TREE
#ifdef DEBUG_XREF_TREE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetDebuggingTabs () const
    {
    Utf8String  tabs;
    for (size_t i = 0; i < m_nestingDepth; ++i)
        tabs += "\t";
    return  tabs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DebugAddedChild () const
    {
    auto tabs = this->GetDebuggingTabs();
    LOG.debugv ("%sAdded Xref node %ls", tabs.c_str(), m_xref->GetFileName().c_str());
    for (auto id : m_insertIds)
        LOG.debugv ("%sXref insert Id = %llx", tabs.c_str(), id.ToUInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DebugFoundChild (DwgDbDatabaseP xref) const
    {
    Utf8String  ids;
    for (auto id : m_insertIds)
        ids += Utf8PrintfString(" %llx", id.ToUInt64());
    auto tabs = this->GetDebuggingTabs ();
    LOG.debugv ("%sFound inserts[%s]!", tabs.c_str(), ids.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DebugFindChild (DwgDbDatabaseP xref) const
    {
    auto tabs = this->GetDebuggingTabs();
    LOG.debugv ("%sChecking Xref %ls attached in file %ls", tabs.c_str(), xref->GetFileName().c_str(), m_xref->GetFileName().c_str());
    }
#endif  // DEBUG_XREF_TREE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
static RefCountedPtr<XRefNode> Create(DwgDbDatabaseP xref)
    {
    return new XRefNode(xref);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SetParent (XRefNode* parent)
    {
    m_parent = parent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
void AddChild (DwgDbDatabaseP xref, DwgDbObjectIdArrayCR insertIds, DwgImporterR importer)
    {
    if (xref != nullptr && !insertIds.empty())
        {
        m_children.push_back (XRefNode::Create(xref));
        auto child = m_children.back ();
        if (child.IsValid())
            {
            child->SetParent (this);
            child->AddBlockInserts (insertIds);
#ifdef DEBUG_XREF_TREE
            child->SetNestingDepth (m_nestingDepth + 1);
            child->DebugAddedChild ();
#endif
            child->AddNestedChildren (importer, m_nestingDepth + 1);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
size_t AddBlockInserts (DwgDbObjectIdArrayCR insertIds)
    {
    m_insertIds.assign (insertIds.begin(), insertIds.end());
    return  m_insertIds.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectIdArrayCR GetBlockInserts () const
    {
    return  m_insertIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SetNestingDepth (size_t depth)
    {
    m_nestingDepth = depth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
size_t AddNestedChildren (DwgImporterR importer, size_t nestingDepth)
    {
    this->SetNestingDepth (nestingDepth);

    // drill into the block table and add xRef nodes it contains as nested children
    auto thisSavedPath = m_xref->GetFileName ();
    DwgDbBlockTablePtr  blockTable(m_xref->GetBlockTableId(), DwgDbOpenMode::ForRead);
    if (blockTable.OpenStatus() != DwgDbStatus::Success)
        {
        BeAssert (false && "Unable to open Xref's block table for read!");
        return 0;
        }

    auto iter = blockTable->NewIterator ();
    if (!iter.IsValid() || !iter->IsValid())
        {
        BeAssert (false && "Unable to create a block iterator!");
        return 0;
        }

    for (iter->Start(); !iter->Done(); iter->Step())
        {
        DwgDbBlockTableRecordPtr block(iter->GetRecordId(), DwgDbOpenMode::ForRead);
        if (block.OpenStatus() != DwgDbStatus::Success || !block->IsExternalReference())
            continue;

        auto xref = importer.FindXRefHolder (*block, false);
        if (xref != nullptr)
            {
            DwgDbObjectIdArray  insertIds;
            if (block->GetBlockReferenceIds(insertIds) == DwgDbStatus::Success)
                this->AddChild (xref->GetDatabaseP(), insertIds, importer);
            }
        }
    return  m_children.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsSameNode (DwgDbDatabaseP xref) const
    {
    return  xref == m_xref;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool FindChild (DwgDbDatabaseP xref, DwgDbHandleArrayR outHandles) const
    {
    if (xref == m_xref)
        {
#ifdef DEBUG_XREF_TREE
        this->DebugFoundChild (xref);
#endif
        for (auto id : m_insertIds)
            outHandles.push_back (id.GetHandle());
        return  true;
        }

#ifdef DEBUG_XREF_TREE
    this->DebugFindChild (xref);
#endif

    for (auto node : m_children)
        {
        node->FindChild(xref, outHandles);
        }
    return  !outHandles.empty();
    }
}; // XRefNode
DEFINE_REF_COUNTED_PTR(XRefNode)
typedef bvector<XRefNodePtr>    T_XRefNodes;


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/19
+===============+===============+===============+===============+===============+======*/
struct XRefTree
{
private:
    DwgImporterR m_importer;
    T_XRefNodes m_nodes;
    bool    m_isValid;

public:
    explicit XRefTree(DwgImporterR importer) : m_importer(importer), m_isValid(false) { m_nodes.clear(); }
    bool IsValid () const { return m_isValid; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchXrefAttachments (DwgDbDatabaseP xref, DwgDbHandleArrayR outHandles)
    {
    // demand build the tree
    if (!this->IsValid())
        this->Build ();
    
    // drill into children and search for matching xRef nodes, collecting all xRef insert handles
    for (auto node : m_nodes)
        {
        if (node->IsSameNode(xref))
            {
            // found a node - append attachments to output
            auto inserts = node->GetBlockInserts ();
            for (auto id : inserts)
                outHandles.push_back (id.GetHandle());
            continue;
            }

        node->FindChild (xref, outHandles);
        }
    return  !outHandles.empty();
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AddRootChild (DwgImporter::DwgXRefHolder& xref)
    {
    if (!xref.IsValid())
        return  BSIERROR;

    // add a root child only if it has instances
    DwgDbObjectIdArray  inserts;
    DwgDbBlockTableRecordPtr block(xref.GetBlockIdInRootFile(), DwgDbOpenMode::ForRead);
    if (block.OpenStatus() != DwgDbStatus::Success || block->GetBlockReferenceIds(inserts) != DwgDbStatus::Success || inserts.empty())
        return  BSIERROR;

    m_nodes.push_back (XRefNode::Create(xref.GetDatabaseP()));
    auto node = m_nodes.back ();
    if (node.IsValid())
        {
        node->AddBlockInserts (inserts);
#ifdef DEBUG_XREF_TREE
        LOG.debugv ("Added root child Xref node %ls", xref.GetSavedPath().c_str());
        inserts = node->GetBlockInserts ();
        for (auto id : inserts)
            LOG.debugv ("Xref insert Id = %llx", id.ToUInt64());
#endif
        node->AddNestedChildren (m_importer, 0);
        return  BSISUCCESS;
        }
    return  BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
void Build ()
    {
    auto loadedXrefs = m_importer.GetLoadedXrefs ();
    for (auto xref : loadedXrefs)
        this->AddRootChild(xref);
    m_isValid = true;
    }
};  // XRefTree


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/19
+===============+===============+===============+===============+===============+======*/
struct XRefDetector
{
private:
    DwgImporterR    m_importer;
    XRefTree    m_loadedXrefTree;

public:
    explicit XRefDetector(DwgImporterR importer) : m_importer(importer), m_loadedXrefTree(importer) {}
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbDatabaseP FindXrefByRepositoryLink (DgnElementId repLinkId)
    {
    auto&   loadedXrefs = m_importer.GetLoadedXrefs ();
    for (auto& xref : loadedXrefs)
        {
        auto dwg = xref.GetDatabaseP ();
        if (nullptr != dwg && m_importer.GetRepositoryLink(dwg) == repLinkId)
            return  dwg;
        }
    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool DetectDetachedXrefsForFile(DgnElementId repLinkId, DwgDbHandleArrayCR newXrefInserts, DgnModelIdSet& detachedModels)
    {
    // compare old xRef models against current xRef inserts - old models not seen in current xRef list are detached
    bool hasNoAttachments = newXrefInserts.empty();
    DwgSourceAspects::ModelAspectIterator oldModels(m_importer.GetDgnDb(), repLinkId);
    for (auto oldModel : oldModels)
        {
        auto oldXrefHandle = oldModel.GetDwgModelHandle ();
        if (hasNoAttachments)
            {
            // all old xRef models are detached
            detachedModels.insert (oldModel.GetModelId());
            }
        else
            {
            // one or more old xRef models not seen in current xRef list is/are detached
            auto found = std::find_if (newXrefInserts.begin(), newXrefInserts.end(), [&](DwgDbHandleCR h){ return oldXrefHandle == h; });
            if (found == newXrefInserts.end())
                detachedModels.insert (oldModel.GetModelId());
            }
        }
    return  !detachedModels.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool DetectDetachedXrefsForFile (DgnElementId replinkId, BeFileNameCR filename, DgnModelIdSet& deletedModels)
    {
    deletedModels.clear ();

    // find the source xRef from which the input repository link was originally created
    auto xrefDwg = this->FindXrefByRepositoryLink (replinkId);
    if (xrefDwg == nullptr)
        return  false;

    // search the tree for all attached instances of the source xRef
    DwgDbHandleArray xrefInserts;
    m_loadedXrefTree.SearchXrefAttachments (xrefDwg, xrefInserts);
    this->DetectDetachedXrefsForFile (replinkId, xrefInserts, deletedModels);

    return !deletedModels.empty();
    }
};  // XRefDetector


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DeleteXrefModels (DwgImporterR importer, DwgSourceAspects::RepositoryLinkAspectCR file, DgnModelIdSet const& detachedModels)
    {
    auto&   db = importer.GetDgnDb ();
    auto    rootSubject = importer.GetSpatialParentSubject ();
    auto    rlinkId = file.GetRepositoryLinkId ();
    size_t  numModels = DwgSourceAspects::ModelAspectIterator(db, rlinkId).Count ();

    // first delete elements in the model in the scope of this file
    for (auto modelId : detachedModels)
        {
        auto model = db.Models().GetModel(modelId);
        if (model.IsValid())
            {
            // the xRef has been detected as detached - if it was previously recorded as skipped model, remove it from the list:
            m_dwgModelsSkipped.erase (model->GetModelId());

            DwgSourceAspects::ObjectAspectIterator elementsInModel(*model);
            _DetectDeletedElements (importer, elementsInModel);

            // mark the references subject of this xRef for later deletection
            if (rootSubject.IsValid())
                {
                Json::Value modelProps(Json::nullValue);
                modelProps["Type"] = "References";
                auto subject = DwgHelper::FindModelSubject (*rootSubject, model->GetName(), modelProps, db);
                if (subject.IsValid())
                    m_subjectsToRemove.insert (subject->GetElementId());
                }

            _DeleteModel (*model);
            }
        }

    // delete the repository link of the xRef file only if all models linked to this file are deleted
    if (detachedModels.size() == numModels)
        {
        BeFileName  filename(file.GetDwgName().c_str());
        LOG.tracev ("Delete repository link %lld of xRef %ls", rlinkId.GetValue(), filename.c_str());
        db.Elements().Delete (rlinkId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdaterChangeDetector::_DetectDetachedXrefs (DwgImporter& importer)
    {
    auto&   db = importer.GetDgnDb ();
    auto    rootfileId = importer.GetRepositoryLink(&importer.GetDwgDb());

    DgnElementIdSet subjectsToRemove;
    XRefDetector detector(importer);

    DwgSourceAspects::RepositoryLinkAspectIterator files(db, nullptr);
    for (auto file : files)
        {
        // skip the root file and files that do not belong to the root file(i.e. those created from different jobs)
        auto rlinkId = file.GetRepositoryLinkId();
        if (rlinkId == rootfileId || rootfileId != file.GetRootRepositoryLinkId())
            continue;
        
        BeFileName  filename(file.GetFileName());
        DgnModelIdSet   detachedModels;

        detector.DetectDetachedXrefsForFile(rlinkId, filename, detachedModels);
        if (!detachedModels.empty())
            _DeleteXrefModels (importer, file, detachedModels);
        }

    // now delete references subjects whose xRef models have been deleted
    for (auto id : m_subjectsToRemove)
        {
        auto subject = db.Elements().Get<Subject>(id);
        if (!subject.IsValid() || !subject->IsPersistent())
            continue;
        LOG.tracev ("Delete references subject %lld [%s]", id.GetValue(), subject->GetCode().GetValueUtf8CP());
        db.Elements().Delete (*subject);
        }
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
    DwgSourceAspects::RepositoryLinkAspectIterator files(db, nullptr);

    for (auto file : files)
        {
        // check the file GUID per PW, or check the local file path (when run on command line, VSTS 54783):
        bool    docExists = false;
        BeGuid  docGuid;
        auto name = file.GetUniqueName ();
        if (docGuid.FromString(name.c_str()) == BSISUCCESS)
            docExists = this->GetOptions().IsDocumentInRegistry(name);
        else
            docExists = BeFileName(file.GetDwgName().c_str(), true).DoesPathExist ();

        if (docExists)
            continue;

        LOG.tracev ("Document %s has been detected for deletion.", name.c_str());

        // need to delete this file - walk through all model mappings in the sync info:
        auto rlinkId = file.GetRepositoryLinkId();
        auto repositoryLink = this->GetDgnDb().Elements().Get<RepositoryLink>(rlinkId);
        if (!repositoryLink.IsValid())
            continue;   // should not happen
        DwgSourceAspects::ModelAspectIterator  modelMaps(*repositoryLink);
        for (auto modelMap : modelMaps)
            {
            // delete elements in DgnModel:
            auto model = db.Models().GetModel(modelMap.GetModelId());
            if (!model.IsValid())
                continue;
            DwgSourceAspects::ObjectAspectIterator elements(*model);
            detector._DetectDeletedElements (*this, elements);

            // delete DgnModel from db:
            model->Delete();
            }
        }

    if (!detectorPresent)
        detector._Cleanup (*this);

    return BSISUCCESS;
    }

END_DWG_NAMESPACE
