/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportXrefs.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportXReference (ElementImportResults& results, ElementImportInputs& inputs)
    {
    /*-----------------------------------------------------------------------------------
    This method imports an xref insert entity during entity importing phase.  Do not try
    to skip model during updating.

    All xref files should have been loaded during model discovering phase in the block section.
    If an xref file is not loaded, it is a serious error and we should not bother to try
    loading the file.

    However, xref instances may not all be seen in the block section, therefore there is 
    a chance that this xref insert could be new and a model will need to be created.
    It gets trickier if the root transform has been changed from iModelBridge as we have
    to invert the current root transform in order to find the model from the syncInfo.
    -----------------------------------------------------------------------------------*/
    DwgDbBlockReferenceP        xrefInsert = DwgDbBlockReference::Cast (inputs.GetEntityP());
    if (nullptr == xrefInsert)
        return  BSIERROR;
    DwgDbObjectId               xrefblockId = xrefInsert->GetBlockTableRecordId ();
    DwgDbBlockTableRecordPtr    xrefBlock(xrefblockId, DwgDbOpenMode::ForRead);
    if (xrefBlock.IsNull())
        {
        this->ReportError (IssueCategory::CorruptData(), Issue::CantOpenObject(), "xref block %ls");
        return  BSIERROR;
        }

    // skip overlaid xRef if it is nested in another xRef:
    if (xrefBlock->IsOverlayReference() && m_currentXref.IsValid() && m_currentXref.GetDatabaseP() != m_dwgdb.get())
        return  BSISUCCESS;

    auto xrefInsertId = xrefInsert->GetObjectId ();

    // save currentXref before recurse into a nested xref:
    DwgXRefHolder   savedCurrentXref = m_currentXref;

    auto found = this->FindXRefHolder (*xrefBlock);
    if (found != nullptr)
        {
        // this xref has been previously loaded - set it as current:
        m_currentXref = *found;
        }
    else
        {
        // the xref file has not been be loaded in block section - error out!
        if (xrefBlock->GetPath().Find(this->GetRootDwgFileName().GetFileNameAndExtension().c_str()) >= 0)
            this->ReportIssue (IssueSeverity::Warning, IssueCategory::UnexpectedData(), Issue::CircularXrefIgnored(), Utf8String(xrefBlock->GetPath().c_str()).c_str());
        else
            this->ReportError (IssueCategory::UnexpectedData(), Issue::ModelFilteredOut(), Utf8PrintfString("%ls, INSERT ID=%ls", xrefBlock->GetPath().c_str(), xrefInsertId.ToAscii().c_str()).c_str());
        return  BSIERROR;
        }

    // get or create a model for the xRefBlock with the blockReference's transformation:
    Transform   xtrans = inputs.GetTransform ();
    this->CompoundModelTransformBy (xtrans, *xrefInsert);
    
    /*-----------------------------------------------------------------------------------
    We take 3 steps in order to catch potential missing xRef inserts in model dicovery phase
    and also to take account of possible change of root transformation:

    Step 1 - try to look the model up in the cached model list only - an xref insert not 
    found in the list is a potential missing xref instance.
    -----------------------------------------------------------------------------------*/
    DgnModelP               model = nullptr;
    ResolvedModelMapping    modelMap= this->FindModel (xrefInsertId, xtrans, DwgSyncInfo::ModelSourceType::XRefAttachment);
    if (!modelMap.IsValid() || nullptr == (model = modelMap.GetModel()))
        {
        // Step2 - decide on if we need to look it up in syncInfo using old or new transform:
        RootTransformInfo const&    rootTransInfo = this->GetRootTransformInfo ();
        bool    searchByOldTransform = this->IsUpdating() && rootTransInfo.HasChanged ();
        // the change of the root transform has no impact on paperspace:
        if (searchByOldTransform && this->IsXrefInsertedInPaperspace(xrefInsertId))
            searchByOldTransform = false;

        Transform   searchTrans;
        if (searchByOldTransform)
            {
            Transform   fromNewToOld = rootTransInfo.GetChangeTransformFromNewToOld ();
            searchTrans.InitProduct (fromNewToOld, xtrans);
            }
        else
            {
            // new import or no change in root transform - search by current xref transform:
            searchTrans = xtrans;
            }

        // Step 3 - second try finding the model using the desired transform, create new one if not found:
        modelMap = this->GetOrCreateModelFromBlock (*xrefBlock.get(), searchTrans, xrefInsert, m_currentXref.GetDatabaseP());
        if (!modelMap.IsValid() || nullptr == (model = modelMap.GetModel()))
            {
            this->ReportError (IssueCategory::UnexpectedData(), Issue::CantCreateModel(), IssueReporter::FmtModel(*xrefBlock).c_str());
            m_currentXref = savedCurrentXref;
            return  BSIERROR;
            }

        if (searchByOldTransform)
            {
            // update model map with the new transform
            modelMap.SetTransform (xtrans);
            modelMap.GetMapping().Update (this->GetDgnDb());
            }
        }

    // and or update the loaded xref cache:
    if (m_currentspaceId == m_modelspaceId)
        m_modelspaceXrefs.insert (model->GetModelId());
    else
        m_paperspaceXrefs.push_back (DwgXRefInPaperspace(xrefInsertId, m_currentspaceId, model->GetModelId()));

    this->SetTaskName (ProgressMessage::TASK_IMPORTING_MODEL(), model->GetName().c_str());
    this->Progress ();

    // get the modelspace block from the xRef DwgDb
    DwgDbBlockTableRecordPtr    xModelspace (m_currentXref.GetModelspaceId(), DwgDbOpenMode::ForRead);
    if (xModelspace.OpenStatus() != DwgDbStatus::Success)
        {
        this->ReportError (IssueCategory::CorruptData(), Issue::CantOpenObject(), Utf8PrintfString("modelspace of the xref %s", m_currentXref.GetResolvedPath().c_str()).c_str());
        m_currentXref = savedCurrentXref;
        return  BSIERROR;
        }

    // clipped xReference
    DwgDbSpatialFilterPtr   spatialFilter;
    if (DwgDbStatus::Success == xrefInsert->OpenSpatialFilter(spatialFilter, DwgDbOpenMode::ForRead))
        LOG_ENTITY.tracev ("xRef %ls is clipped!", xrefBlock->GetPath().c_str());

    ElementImportInputs     childInputs (*model);
    childInputs.SetClassId (this->_GetElementType(*xrefBlock.get()));
    childInputs.SetTransform (xtrans);
    childInputs.SetSpatialFilter (spatialFilter.get());
    childInputs.SetModelMapping (modelMap);

    // SortEnts table
    DwgDbSortentsTablePtr   sortentsTable;
    if (DwgDbStatus::Success == xModelspace->OpenSortentsTable(sortentsTable, DwgDbOpenMode::ForRead))
        {
        // import entities in sorted order:
        DwgDbObjectIdArray  entities;
        if (DwgDbStatus::Success == sortentsTable->GetFullDrawOrder(entities))
            {
            for (DwgDbObjectIdCR id : entities)
                {
                childInputs.SetEntityId (id);
                this->OpenAndImportEntity (childInputs);
                }
            }
        }
    else
        {
        // import entities in database order:
        DwgDbBlockChildIteratorPtr  entityIter = xModelspace->GetBlockChildIterator ();
        if (entityIter.IsValid() && entityIter->IsValid())
            {
            // fill the xref model with entities
            for (entityIter->Start(); !entityIter->Done(); entityIter->Step())
                {
                childInputs.SetEntityId (entityIter->GetEntityId());
                this->OpenAndImportEntity (childInputs);
                }
            }
        }

    // done model filling - if it's an xref in a layout, create a SpatialView & a ViewAttachment:
    if (m_currentspaceId != m_modelspaceId)
        {
        LayoutXrefFactory   factory(*this, *xrefInsert);
        factory.SetPaperspace (m_currentspaceId);
        factory.SetXrefDatabase (m_currentXref.GetDatabaseP());
        factory.SetXrefModel (model);
        factory.SetXrefTransform (xtrans);
        factory.ConvertToBim (results, inputs);
        }
    
    // restore current xref
    m_currentXref = savedCurrentXref;

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::DwgXRefHolder::InitFrom (DwgDbBlockTableRecordCR xrefBlock, DwgImporter& importer)
    {
    /*-----------------------------------------------------------------------------------
    This xRef holder is initiated from root the block section of the root master file, 
    should guarantee no circular xRef blocks to be loaded by either toolkit, so we should
    not have to bother checking circular xref's here.
    -----------------------------------------------------------------------------------*/
    if (xrefBlock.IsExternalReference())
        {
        DwgImportHost&  host = DwgImportHost::GetHost ();
        BeFileName      found;

        // save originally stored path:
        m_savedPath.assign (xrefBlock.GetPath().c_str());
        // will start the search from originally stored path:
        m_resolvedPath.assign (m_savedPath);
        // try resolving the file path
        if (!m_resolvedPath.DoesPathExist() && DwgDbStatus::Success == host._FindFile(found, m_resolvedPath.c_str(), xrefBlock.GetDatabase().get(), AcadFileType::XRefDrawing))
            m_resolvedPath = found;

        if (m_resolvedPath.DoesPathExist())
            {
            // if the DWG file has been previously loaded, use it
            m_xrefDatabase = importer.FindLoadedXRef (m_resolvedPath);

            DwgFileVersion  version = DwgFileVersion::Invalid;

            // try creating a new DwgDb for the xref, but do not allow circular referencing:
            if (!m_xrefDatabase.IsValid() && !m_resolvedPath.EqualsI(importer.GetRootDwgFileName()) && DwgHelper::SniffDwgFile (m_resolvedPath, &version))
                {
                Utf8String  verstr = DwgHelper::GetStringFromDwgVersion (version);
                importer.SetStepName (ProgressMessage::STEP_OPENINGFILE(), m_resolvedPath.c_str(), verstr.c_str());
                m_xrefDatabase = host.ReadFile (m_resolvedPath, false, false, FileShareMode::DenyNo);
                }
            }

        if (m_xrefDatabase.IsValid())
            {
            // the nested block name should be propagated into the root file
            m_prefixInRootFile = xrefBlock.GetName().GetWCharCP ();
            m_blockIdInRootFile = xrefBlock.GetObjectId ();
            // will add DgnModels resolved from xref inserts
            m_dgnModels.clear ();
            return  BSISUCCESS;
            }

        importer.ReportError (IssueCategory::DiskIO(), Issue::FileFilteredOut(), Utf8String(m_savedPath.c_str()).c_str());
        }

    m_resolvedPath.clear ();
    m_savedPath.clear ();
    m_prefixInRootFile.clear ();
    m_blockIdInRootFile.SetNull ();
    m_dgnModels.clear ();

    return  BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbDatabaseP  DwgImporter::FindLoadedXRef (BeFileNameCR path)
    {
    // find DwgDbDatabase from m_loadedXrefFiles by resolved file path:
    struct FindXrefPredicate
        {
        BeFileName  m_filePath;
        FindXrefPredicate (BeFileNameCR inPath) : m_filePath (inPath) {}
        bool operator () (DwgXRefHolder const& xh) { return m_filePath.CompareTo(xh.GetResolvedPath()) == 0; }
        };

    FindXrefPredicate   pred(path);
    auto found = std::find_if (m_loadedXrefFiles.begin(), m_loadedXrefFiles.end(), pred);

    return found == m_loadedXrefFiles.end() ? nullptr : found->GetDatabaseP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::DwgXRefHolder* DwgImporter::FindXRefHolder (DwgDbBlockTableRecordCR xrefBlock)
    {
    // find an entry in m_loadedXrefFiles by either blockId or saved file name:
    struct FindXrefPredicate
        {
        DwgDbObjectId   m_parentBlockId;
        BeFileName      m_parentSavedPath;
        BeFileName      m_parentResolvedPath;
        DwgDbDatabaseP  m_parentDwg;
        BeFileName      m_rootMasterFile;

        FindXrefPredicate (DwgDbBlockTableRecordCR xrefBlock, BeFileNameCR rootFile)
            {
            // xref information from its immediate parent file:
            m_parentBlockId = xrefBlock.GetObjectId ();
            m_parentSavedPath.SetName (xrefBlock.GetPath().c_str());
            m_parentResolvedPath.clear ();
            m_parentDwg = xrefBlock.GetDatabase().get ();
            m_rootMasterFile.SetName (rootFile);
            }

        bool operator () (DwgXRefHolder const& xh)
            {
            // if it is a directly attached block, it should be matched by block ID:
            if ((m_parentBlockId.IsValid() && m_parentBlockId == xh.GetBlockIdInRootFile()))
                return  true;

            // or a nested xref matched by saved path:
            if (m_parentSavedPath.EqualsI(xh.GetSavedPath()))
                return  true;

            /*----------------------------------------------------------------------------------
            Handle a special case: ProjectWise changes dms path only in an immediate parent file, 
            not in the root master file, causing saved paths out of sync!  In this case, we try
            comparing resolved paths, with below consideration of:
                a) Only do this for PW files to minimize the need for expensive file search.
                b) Check and stop circular xref resolved back into the root master file.
            ----------------------------------------------------------------------------------*/
            if (m_parentSavedPath.StartsWith(L"..\\dms"))
                {
                if (m_parentResolvedPath.empty())
                    {
                    if (DwgImportHost::GetHost()._FindFile(m_parentResolvedPath, m_parentSavedPath.c_str(), m_parentDwg, AcadFileType::XRefDrawing) != DwgDbStatus::Success ||
                        m_parentResolvedPath.EqualsI(m_rootMasterFile))
                        {
                        m_parentResolvedPath.clear ();
                        return  false;
                        }
                    }
                return  m_parentResolvedPath.EqualsI(xh.GetResolvedPath());
                }
            return  false;
            }
        };  // FindXrefPredicate

    FindXrefPredicate  pred (xrefBlock, this->GetRootDwgFileName());
    auto found = std::find_if (m_loadedXrefFiles.begin(), m_loadedXrefFiles.end(), pred);
    return found == m_loadedXrefFiles.end() ? nullptr : found;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
LayoutXrefFactory::LayoutXrefFactory (DwgImporter& im, DwgDbBlockReferenceCR x) : m_importer(im), m_xrefInsert(x)
    {
    m_jobDefinitionModel = m_importer.GetOrCreateJobDefinitionModel().get ();
    if (nullptr == m_jobDefinitionModel)
        {
        m_importer.ReportError (IssueCategory::Unknown(), Issue::MissingJobDefinitionModel(), "SpatialView");
        m_jobDefinitionModel = &m_importer.GetDgnDb().GetDictionaryModel ();
        }
    m_xrefModel = nullptr;
    m_xrefDwg = nullptr;
    m_xrefTransform.InitIdentity ();
    m_paperspaceId.SetNull ();
    m_xrefRange.Init ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    LayoutXrefFactory::ValidateViewName ()
    {
    auto viewId = ViewDefinition::QueryViewId (*m_jobDefinitionModel, m_viewName);
    if (viewId.IsValid())
        {
        // deduplicate view name
        Utf8String  suffix;
        uint32_t    count = 1;
        Utf8String  fileName (m_importer.GetDwgDb().GetFileName().c_str());
        Utf8String  proposedName = m_viewName;

        do
            {
            suffix.Sprintf ("-%d", count++);
            m_viewName = m_importer.RemapNameString (fileName, proposedName, suffix);
            } while (ViewDefinition::QueryViewId(*m_jobDefinitionModel, m_viewName).IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool    LayoutXrefFactory::UpdateViewName ()
    {
    if (m_spatialView->GetName().Equals(m_viewName))
        return  false;

    this->ValidateViewName ();

    // change view name - caller shall update element
    auto code = m_spatialView->GetCode ();
    auto status = m_spatialView->SetCode (DgnCode::From(code.GetCodeSpecId(), code.GetScopeString(), m_viewName));
    if (status != DgnDbStatus::Success)
        {
        m_importer.ReportIssueV (DwgImporter::IssueSeverity::Error, IssueCategory::Briefcase(), Issue::CannotUpdateName(), "Layout View", m_spatialView->GetName().c_str(), m_viewName.c_str());
        return  false;
        }

    // change category selector name - caller shall update element
    auto userLabel = DataStrings::GetString (DataStrings::CategorySelector());
    auto& categorySelector = m_spatialView->GetCategorySelector ();
    m_importer.UpdateElementName (categorySelector, m_viewName, userLabel.c_str(), false);
    
    // change display style name - caller shall update element
    userLabel = DataStrings::GetString (DataStrings::DisplayStyle());
    auto& displayStyle = m_spatialView->GetDisplayStyle ();
    m_importer.UpdateElementName (displayStyle, m_viewName, userLabel.c_str(), false);

    // change model selector name - do not expect caller to update element
    auto spatialView = m_spatialView->ToSpatialViewP ();
    if (nullptr != spatialView)
        {
        userLabel = DataStrings::GetString (DataStrings::ModelSelector());
        m_importer.UpdateElementName (spatialView->GetModelSelector(), m_viewName, userLabel.c_str(), true);
        }

    return  status == DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  LayoutXrefFactory::GetXrefLayerPrefix () const
    {
    // get xRef file name
    DwgDbBlockTableRecordPtr block(m_xrefInsert.GetBlockTableRecordId(), DwgDbOpenMode::ForRead);
    if (block.OpenStatus() != DwgDbStatus::Success)
        return  Utf8String();

    // build xref base name only
    auto xrefName = BeFileName::GetFileNameWithoutExtension (block->GetPath().c_str());

    // layer prefix
    Utf8String  prefix = Utf8String(xrefName) + "_";
    DgnDbTable::ReplaceInvalidCharacters (prefix, DgnCategory::GetIllegalCharacters(), '_');

    return prefix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  LayoutXrefFactory::GetViewportFrozenLayers (DwgDbObjectIdArrayR frozenLayers) const
    {
    // get the overall viewport from the paperspace
    DwgDbBlockTableRecordPtr paperspace(m_paperspaceId, DwgDbOpenMode::ForRead);
    if (paperspace.OpenStatus() != DwgDbStatus::Success)
        return  0;
    
    DwgDbViewportPtr viewport0(LayoutFactory::FindOverallViewport(*paperspace), DwgDbOpenMode::ForRead);
    if (viewport0.OpenStatus() != DwgDbStatus::Success)
        return  0;
    
    if (viewport0->GetFrozenLayers(frozenLayers) != DwgDbStatus::Success)
        return  0;

    return  frozenLayers.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    LayoutXrefFactory::UpdateSpatialCategories (DgnCategoryIdSet& categoryIdSet) const
    {
    // build the xref name prefix in a layer name
    auto xrefPrefix = this->GetXrefLayerPrefix ();
    if (xrefPrefix.empty())
        return;

    // get viewport frozen layers from the paperspace viewport
    DwgDbObjectIdArray  frozenLayers;
    auto numFrozenLayers = this->GetViewportFrozenLayers (frozenLayers);

    // lookup current DWG file in syncInfo
    DwgDbDatabaseR          dwg = m_importer.GetDwgDb ();
    DwgSyncInfo::DwgFileId  fileId = DwgSyncInfo::DwgFileId::GetFrom (dwg);
    DwgSyncInfo&            syncInfo = m_importer.GetSyncInfo ();

    // add displayed xRef spatial categories to the view:
    for (ElementIteratorEntry entry : SpatialCategory::MakeIterator(m_importer.GetDgnDb()))
        {
        DgnCategoryId   categoryId = entry.GetId <DgnCategoryId> ();
        Utf8String      codeValue = entry.GetCodeValue ();

        if (codeValue.StartsWith(xrefPrefix.c_str()) || codeValue.Equals("0") || codeValue.EqualsI("defpoints"))
            {
            // this is a category from a layer of this xref - check on/off status
            bool isViewed = false;
            auto layerId = dwg.GetObjectId (syncInfo.FindLayerHandle(categoryId, fileId));

            DwgDbLayerTableRecordPtr    layer(layerId, DwgDbOpenMode::ForRead);
            if (layer.OpenStatus() == DwgDbStatus::Success && !layer->IsOff() && !layer->IsFrozen())
                {
                isViewed = true;
                // also check viewport frozen layer
                if (numFrozenLayers > 0)
                    {
                    auto found = std::find_if (frozenLayers.begin(), frozenLayers.end(), [&](DwgDbObjectId id){ return id == layerId; });
                    isViewed = found == frozenLayers.end();
                    }
                if (isViewed)
                    {
                    categoryIdSet.insert (categoryId);
                    continue;
                    }
                }
            }
        categoryIdSet.erase (categoryId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void LayoutXrefFactory::ComputeSpatialDisplayStyle (DisplayStyle3dR displayStyle) const
    {
    // a display style viewed through a sheet model
    Render::ViewFlags   viewFlags;

    viewFlags.SetRenderMode (RenderMode::Wireframe);
    viewFlags.SetShowVisibleEdges (true);
    viewFlags.SetShowHiddenEdges (true);
    viewFlags.SetShowCameraLights (false);
    viewFlags.SetShowSolarLight (false);
    viewFlags.SetShowSourceLights (false);
    viewFlags.SetShowShadows (false);
    displayStyle.SetViewFlags (viewFlags);

    // white background
    displayStyle.SetBackgroundColor (ColorDef::White());

    // neither sky box nor ground plane
    auto&   environmentDisplay = displayStyle.GetEnvironmentDisplayR ();
    environmentDisplay.m_groundPlane.m_enabled = false;
    environmentDisplay.m_skybox.m_enabled = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LayoutXrefFactory::ComputeSpatialView ()
    {
    // fit the xref model in the SpatialView
    auto spatialModel = m_xrefModel->ToSpatialModel ();
    if (nullptr == spatialModel)
        return  BSIERROR;

    auto range = spatialModel->QueryModelRange ();

    if (nullptr != m_xrefDwg)
        m_xrefRange.InitFrom (m_xrefDwg->GetEXTMIN(),  m_xrefDwg->GetEXTMAX());

    DwgDbSpatialFilterPtr   filter;
    if (m_xrefInsert.OpenSpatialFilter(filter, DwgDbOpenMode::ForRead) == DwgDbStatus::Success)
        {
        // WIP - clip xref in SpatialView, and update m_xrefRange to the clipped extents
        }
    
    m_spatialView->SetExtents (range.DiagonalVector());
    m_spatialView->SetOrigin (range.low);
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LayoutXrefFactory::ComputeViewAttachment (Placement2dR placement)
    {
    // get the range of the SpartialView
    auto extents = m_spatialView->GetExtents ();

    ElementAlignedBox2d box(0.0, 0.0, extents.x, extents.y);
    placement.SetElementBox (box);

    // find the center of inserted Xref in sheet model
    auto dwgCenter = DPoint3d::FromSumOf (m_xrefRange.low, m_xrefRange.high);

    dwgCenter.Scale (0.5);
    m_xrefTransform.Multiply (dwgCenter);

    // center of the SpatialView
    auto dbCenter = DPoint3d::From (-0.5 * extents.x, -0.5 * extents.y, 0.0);
    dbCenter.Add (dwgCenter);

    placement.GetOriginR().Init (dbCenter.x, dbCenter.y);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LayoutXrefFactory::UpdateSpatialView (bool updateBim)
    {
    if (!m_spatialView.IsValid())
        return  BSIERROR;

    // if the view name has been changed, reset it in affected elements which will be updated as follows
    this->UpdateViewName ();

    // update categories
    auto& categorySelector = m_spatialView->GetCategorySelector ();
    this->UpdateSpatialCategories (categorySelector.GetCategoriesR());

    // update display style:
    auto& displayStyle = m_spatialView->GetDisplayStyle3d ();
    this->ComputeSpatialDisplayStyle (displayStyle);

    // re-compute the view
    this->ComputeSpatialView ();

#ifdef NDEBUG
    m_spatialView->SetIsPrivate (true);
#else
    m_spatialView->SetIsPrivate (false);
#endif
    m_spatialView->SetUserLabel (DataStrings::GetString(DataStrings::XrefView()).c_str());

    if (updateBim)
        {
        categorySelector.Update ();
        displayStyle.Update ();
        m_spatialView->Update ();

        auto viewId = m_spatialView->GetViewId ();
        if (!viewId.IsValid())
            {
            m_importer.ReportError(IssueCategory::CorruptData(), Issue::Error(), m_viewName.c_str());
            return  BSIERROR;
            }
        m_importer._GetChangeDetector()._OnViewSeen (m_importer, viewId);
        m_importer.GetSyncInfo().UpdateView (viewId, m_xrefInsert.GetObjectId(), DwgSyncInfo::View::Type::XrefAttachment, m_spatialView->GetName());
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LayoutXrefFactory::CreateSpatialView ()
    {
    this->ValidateViewName ();

    // only add this xref model at this time
    ModelSelectorPtr modelSelector = new ModelSelector (*m_jobDefinitionModel, m_viewName.c_str());
    if (!modelSelector.IsValid())
        return  BSIERROR;
    modelSelector->AddModel (m_xrefModel->GetModelId());

    // will add spatial categories from the xRef in update
    CategorySelectorPtr categorySelector = new CategorySelector (*m_jobDefinitionModel, m_viewName.c_str());
    if (!categorySelector.IsValid())
        return  BSIERROR;

    // create a display style:
    DisplayStyle3dPtr   displayStyle = new DisplayStyle3d (*m_jobDefinitionModel, m_viewName.c_str());
    if (!displayStyle.IsValid())
        return  BSIERROR;

    // create a spatial view
    m_spatialView = new SpatialViewDefinition (*m_jobDefinitionModel, m_viewName, *categorySelector, *displayStyle, *modelSelector);

    // update the new SpatialView from the xref
    if (this->UpdateSpatialView(false) != BSISUCCESS)
        return  BSIERROR;

    // add the new SpatialView to db
    if (m_spatialView->Insert().IsNull())
        {
        m_importer.ReportError(IssueCategory::CorruptData(), Issue::Error(), m_viewName.c_str());
        return  BSIERROR;
        }

    // and add it to syncInfo
    m_importer._GetChangeDetector()._OnViewSeen (m_importer, m_spatialView->GetViewId());
    m_importer.GetSyncInfo().InsertView (m_spatialView->GetViewId(), m_xrefInsert.GetObjectId(), DwgSyncInfo::View::Type::XrefAttachment, m_spatialView->GetName());
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LayoutXrefFactory::UpdateViewAttachment ()
    {
    if (!m_viewAttachment.IsValid())
        {
        BeAssert (false && "ViewAttachment not created!");
        return  BSIERROR;
        }

    // calculate the placement point in the sheet view for the view attachment
    Placement2d     placement;
    this->ComputeViewAttachment (placement);
    m_viewAttachment->SetPlacement (placement);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LayoutXrefFactory::CreateViewAttachment (DgnModelR sheetModel)
    {
    if (!m_spatialView.IsValid())
        return  BSIERROR;

    // get or create a drawing category from the layer of this xRef insert:
    DgnSubCategoryId    subCategoryId;
    DwgDbObjectId   layerId = m_xrefInsert.GetLayerId ();
    DwgDbObjectId   layoutViewportId = m_importer._GetCurrentGeometryOptions().GetViewportId ();
    DgnCategoryId   categoryId = m_importer.GetOrAddDrawingCategory (subCategoryId, layerId, layoutViewportId, sheetModel);

    // will compute placement in update method:
    Placement2d placeHolder(DPoint2d::FromZero(), AngleInDegrees(), ElementAlignedBox2d(0,0,1,1));

    // create a view attachment in the sheet:
    m_viewAttachment = new Sheet::ViewAttachment (m_importer.GetDgnDb(), sheetModel.GetModelId(), m_spatialView->GetViewId(), categoryId, placeHolder);

    return  this->UpdateViewAttachment();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LayoutXrefFactory::ConvertToBim (DwgImporter::ElementImportResults& results, DwgImporter::ElementImportInputs& inputs)
    {
    if (nullptr == m_xrefModel || nullptr == m_xrefDwg || !m_paperspaceId.IsValid())
        return  BSIERROR;

    /*-----------------------------------------------------------------------------------
    Treat an xRef like a viewport - create a SpatialView of the xref model and attach that
    to a ViewAttachment, which is our pivital element for syncInfo.
    -----------------------------------------------------------------------------------*/
    bool    hasSpatialView = false;
    bool    hasViewAttachment = false;

    m_viewName.Sprintf ("%s%s", LayoutXrefFactory::GetSpatialViewNamePrefix(), m_xrefModel->GetName().c_str());

    // current target model should be a sheet model:
    auto& sheetModel = inputs.GetTargetModelR ();
    BeAssert (sheetModel.IsSheetModel());

    if (m_importer.IsUpdating())
        {
        auto attachId = results.GetExistingElement().GetElementId ();
        m_viewAttachment = m_importer.GetDgnDb().Elements().GetForEdit<Sheet::ViewAttachment> (attachId);

        if (m_viewAttachment.IsValid())
            {
            // update SpatialView
            m_spatialView = m_importer.GetDgnDb().Elements().GetForEdit<SpatialViewDefinition> (m_viewAttachment->GetAttachedViewId());
            if (m_spatialView.IsValid())
                {
                if (m_spatialView->ViewsModel(m_xrefModel->GetModelId()))
                    {
                    this->UpdateViewName ();
                    hasSpatialView = this->UpdateSpatialView(true) == BSISUCCESS;
                    }
                else
                    {
                    // a transform change of an xref results in a new model created - re-import all views.
                    m_spatialView = nullptr;
                    m_viewAttachment = nullptr;
                    hasSpatialView = false;
                    }
                }
            // update ViewAttachment
            if (hasSpatialView)
                hasViewAttachment = this->UpdateViewAttachment() == BSISUCCESS;
            }
        }

    if (!hasSpatialView)
        this->CreateSpatialView ();
    if (!hasViewAttachment)
        this->CreateViewAttachment (sheetModel);

    if (!m_viewAttachment.IsValid())
        return  BSIERROR;

    results.SetImportedElement (m_viewAttachment.get());
    return  BSISUCCESS;
    }

