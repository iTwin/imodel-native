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
USING_NAMESPACE_DGNDBSYNC_DWG

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportXReference (DwgDbBlockReferenceCR xrefInsert, ElementImportInputs& inputs)
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
    DwgDbObjectId               xrefblockId = xrefInsert.GetBlockTableRecordId ();
    DwgDbBlockTableRecordPtr    xrefBlock(xrefblockId, DwgDbOpenMode::ForRead);
    if (xrefBlock.IsNull())
        {
        this->ReportError (IssueCategory::CorruptData(), Issue::CantOpenObject(), "xref block %ls");
        return  BSIERROR;
        }

    // skip overlaid xRef if it is nested in another xRef:
    if (xrefBlock->IsOverlayReference() && m_currentXref.IsValid() && m_currentXref.GetDatabaseP() != m_dwgdb.get())
        return  BSISUCCESS;

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
            this->ReportIssue (DwgImporter::IssueSeverity::Warning, IssueCategory::UnexpectedData(), DwgImporter::Issue::CircularXrefIgnored(), Utf8String(xrefBlock->GetPath().c_str()).c_str());
        else
            this->ReportError (IssueCategory::UnexpectedData(), Issue::ModelFilteredOut(), Utf8PrintfString("%ls, INSERT ID=%ls", xrefBlock->GetPath().c_str(), xrefInsert.GetObjectId().ToAscii().c_str()).c_str());
        return  BSIERROR;
        }

    // get or create a model for the xRefBlock with the blockReference's transformation:
    Transform   xtrans = inputs.GetTransform ();
    this->CompoundModelTransformBy (xtrans, xrefInsert);
    
    /*-----------------------------------------------------------------------------------
    We take 3 steps in order to catch potential missing xRef inserts in model dicovery phase
    and also to take account of possible change of root transformation:

    Step 1 - try to look the model up in the cached model list only - an xref insert not 
    found in the list is a potential missing xref instance.
    -----------------------------------------------------------------------------------*/
    DgnModelP               model = nullptr;
    ResolvedModelMapping    modelMap= this->FindModel (xrefInsert.GetObjectId(), xtrans, DwgSyncInfo::ModelSourceType::XRefAttachment);
    if (!modelMap.IsValid() || nullptr == (model = modelMap.GetModel()))
        {
        // Step2 - decide on if we need to look it up in syncInfo using old or new transform:
        RootTransformInfo const&    rootTransInfo = this->GetRootTransformInfo ();
        bool    searchByOldTransform = this->IsUpdating() && rootTransInfo.HasChanged ();
        // the change of the root transform has no impact on paperspace:
        if (searchByOldTransform && this->IsXrefInsertedInPaperspace(xrefInsert.GetObjectId()))
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
        modelMap = this->GetOrCreateModelFromBlock (*xrefBlock.get(), searchTrans, &xrefInsert, m_currentXref.GetDatabaseP());
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
        m_paperspaceXrefs.push_back (DwgXRefInPaperspace(xrefInsert.GetObjectId(), m_currentspaceId, model->GetModelId()));

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
    if (DwgDbStatus::Success == xrefInsert.OpenSpatialFilter(spatialFilter, DwgDbOpenMode::ForRead))
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
        DwgDbBlockChildIterator     entityIter = xModelspace->GetBlockChildIterator ();
        if (entityIter.IsValid())
            {
            // fill the xref model with entities
            for (entityIter.Start(); !entityIter.Done(); entityIter.Step())
                {
                childInputs.SetEntityId (entityIter.GetEntityId());
                this->OpenAndImportEntity (childInputs);
                }
            }
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
                importer.SetStepName (DwgImporter::ProgressMessage::STEP_OPENINGFILE(), m_resolvedPath.c_str(), verstr.c_str());
                m_xrefDatabase = host.ReadFile (m_resolvedPath, false, false, FileShareMode::DenyNo);
                }
            }

        if (m_xrefDatabase.IsValid())
            {
            // the nested block name should be propagated into the root file
            m_prefixInRootFile = xrefBlock.GetName().GetWCharCP ();
            m_blockIdInRootFile = xrefBlock.GetObjectId ();
            return  BSISUCCESS;
            }

        importer.ReportError (DwgImporter::IssueCategory::DiskIO(), DwgImporter::Issue::FileFilteredOut(), Utf8String(m_savedPath.c_str()).c_str());
        }

    m_resolvedPath.clear ();
    m_savedPath.clear ();
    m_prefixInRootFile.clear ();
    m_blockIdInRootFile.SetNull ();

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
