/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

static const Utf8CP                 s_codeSpecName = "DWG";    // TBD: One authority per DWG file?

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::Initialize (BeFileNameCP toolkitDir)
    {
#ifdef BENTLEY_WIN32
    // set search path for an installed toolkit, RealDWG or OpenDWG:
    if (nullptr != toolkitDir && toolkitDir->DoesPathExist())
        ::SetDllDirectoryW (toolkitDir->c_str());
#endif

    DgnDomains::RegisterDomain (Raster::RasterDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::TerminateDwgHost ()
    {
    /*-------------------------------------------------------------------------------------------------
    Ideally we'd simply let ~DwgImporter terminate the toolkit host.  But for RealDWG we choose to extend 
    DwgDbDatabasePtr from RefCountedPtr which has a private member m_p holding our templated database. 
    As such, we cannot effectively control the sequence of deleting m_dwgdb, followed by terminating the
    toolkit's host, and then stopping ~RefCountedPtr from deleting the already deleted database again.
    We can do this for OpenDWG because OdSmartPtr has a release method that allows us to carry the
    aforementioned sequence. But since we want this code to support both toolkits in the same manner, we 
    choose to let the application terminate the toolkit.
    -------------------------------------------------------------------------------------------------*/
    DwgImportHost::TerminateToolkit ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaStatus    DwgImporter::ImportDwgSchemas (bvector<ECSchemaPtr>& dwgSchemas)
    {
    ECSchemaCachePtr        schemaCache = ECSchemaCache::Create ();
    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext ();
    if (!schemaCache.IsValid() || !schemaContext.IsValid())
        return  SchemaStatus::SchemaImportFailed;

    auto nSchemas = dwgSchemas.size ();
    auto nClasses = 0;

    // import schemas
    schemaContext->AddSchemaLocater (m_dgndb->GetSchemaLocater());
    for (auto dwgSchema : dwgSchemas)
        {
        schemaContext->AddSchema (*dwgSchema);
        nClasses += dwgSchema->GetClassCount();
        }
    schemaContext->RemoveSchemaLocater (m_dgndb->GetSchemaLocater());

    this->SetStepName (ProgressMessage::STEP_IMPORTING_SCHEMAS(), nSchemas, nSchemas < 2 ? "," : "s,", nClasses);

    auto status = m_dgndb->ImportSchemas (schemaContext->GetCache().GetSchemas());
    if (status != SchemaStatus::Success)
        {
        this->ReportError (IssueCategory::Unknown(), Issue::SchemaImportError(), Utf8PrintfString("%s and/or %s, SchemaStatus=%d", SCHEMAName_AttributeDefinitions, SCHEMAName_AecPropertySets, (int)status).c_str());
        return  status;
        }

    // get back newly imported schemas:
    m_attributeDefinitionSchema = nullptr;
    m_aecPropertySetSchema = nullptr;
    for (auto dwgSchema : dwgSchemas)
        {
        auto schemaName = dwgSchema->GetName();
        auto importedSchema = m_dgndb->Schemas().GetSchema (schemaName.c_str(), true);

        if (schemaName.StartsWith(SCHEMAName_AttributeDefinitions))
            m_attributeDefinitionSchema = importedSchema;
        else if (schemaName.Equals(SCHEMAName_AecPropertySets))
            m_aecPropertySetSchema = importedSchema;
        }

    return (m_attributeDefinitionSchema == nullptr && m_aecPropertySetSchema == nullptr) ? SchemaStatus::SchemaNotFound : SchemaStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_MakeSchemaChanges ()
    {
    if (!m_dgndb.IsValid() || !m_dwgdb.IsValid())
        return  static_cast<BentleyStatus>(DgnDbStatus::NotOpen);

    m_paperspaceBlockIds.clear ();
    m_loadedXrefFiles.clear ();
    m_unresolvedXrefFiles.clear ();

    bvector<ECSchemaPtr>    dwgSchemas;

    this->SetStepName (ProgressMessage::TASK_LOADING_XREFS());

    StopWatch totalTimer (true);
    StopWatch timer (true);

    timer.Start();

    /*-----------------------------------------------------------------------------------
    Iterate the block table once for multiple tasks in an import job:
        1) Load xRef files from xRef block table records
        2) Cache layout block object ID's from paperspace block table records
        3) Collect attribute definitions from regular block table records
        4) Create DwgAppData schema for Bentley.CCBlockInfo if the regapp present on a block
    -----------------------------------------------------------------------------------*/
    XRefLoader  xrefLoader(*this);
    xrefLoader.LoadXrefsInMasterFile ();
    xrefLoader.CacheUnresolvedXrefs ();

    // create attribute definitions schema
    auto attrdefSchema = xrefLoader.GetAttrdefSchema ();
    if (attrdefSchema.IsValid() && attrdefSchema->GetClassCount() > 0)
        {
        if (m_dgndb->BriefcaseManager().LockSchemas().Result() != RepositoryStatus::Success)
            return  static_cast<BentleyStatus>(DgnDbStatus::LockNotHeld);
        dwgSchemas.push_back (attrdefSchema);
        }

    // create AEC property set definitions schema
    auto aecpsetSchema = this->_CreateAecPropertySetSchema ();
    if (aecpsetSchema.IsValid() && aecpsetSchema->GetClassCount() > 0)
        {
        if (m_dgndb->BriefcaseManager().LockSchemas().Result() != RepositoryStatus::Success)
            return  static_cast<BentleyStatus>(DgnDbStatus::LockNotHeld);
        dwgSchemas.push_back (aecpsetSchema);
        }

    // create DwgAppData schema
    auto dwgappdataSchema = xrefLoader.GetDwgAppDataSchema ();
    if (dwgappdataSchema.IsValid() && dwgappdataSchema->GetClassCount() > 0)
        {
        if (m_dgndb->BriefcaseManager().LockSchemas().Result() != RepositoryStatus::Success)
            return  static_cast<BentleyStatus>(DgnDbStatus::LockNotHeld);
        dwgSchemas.push_back (dwgappdataSchema);
        }

    DwgImportLogging::LogPerformance(timer, "Creating schemas");

    // import schemas into db
    if (!dwgSchemas.empty())
        {
        timer.Start();

        auto status = this->ImportDwgSchemas (dwgSchemas);

        DwgImportLogging::LogPerformance(timer, "Importing schemas");

        if (status != SchemaStatus::Success)
            return  static_cast<BentleyStatus>(status);
        }

    // create/update all repository links after schemas imported; otherwise ImportSchema fails
    return  this->CreateOrUpdateRepositoryLinks ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void   DwgImporter::GetOrCreateJobPartitions ()
    {
    this->InitUncategorizedCategory ();
    this->InitBusinessKeyCodeSpec ();
    this->InitSheetListModel ();
    this->InitDrawingListModel ();
    this->InitGroupModel ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgImporter::InitGroupModel ()
    {
    static Utf8CP s_partitionName = "Imported Groups";
    DgnCode partitionCode = GroupInformationPartition::CreateCode(GetJobSubject(), s_partitionName);
    DgnElementId partitionId = m_dgndb->Elements().QueryElementIdByCode(partitionCode);
    m_groupModelId = DgnModelId(partitionId.GetValueUnchecked());
    
    if (m_groupModelId.IsValid())
        return BentleyStatus::BSISUCCESS;

    GroupInformationPartitionPtr ed = GroupInformationPartition::Create(GetJobSubject(), s_partitionName);
    GroupInformationPartitionCPtr partition = ed->InsertT<GroupInformationPartition>();
    if (!partition.IsValid())
        return BentleyStatus::BSIERROR;

    GenericGroupModelPtr groupModel = GenericGroupModel::CreateAndInsert(*partition);
    if (!groupModel.IsValid())
        return BentleyStatus::BSIERROR;

    m_groupModelId = groupModel->GetModelId();
    return BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::InitSheetListModel ()
    {
    Utf8CP  partitionName = "Imported Sheets";
    DgnCode partitionCode = GroupInformationPartition::CreateCode(GetJobSubject(), partitionName);
    DgnElementId partitionId = m_dgndb->Elements().QueryElementIdByCode(partitionCode);
    m_sheetListModelId = DgnModelId(partitionId.GetValueUnchecked());
    
    if (m_sheetListModelId.IsValid())
        return  BSISUCCESS;

    DocumentPartitionCPtr partition = DocumentPartition::CreateAndInsert(GetJobSubject(), partitionName);
    if (!partition.IsValid())
        return  BSIERROR;

    DocumentListModelPtr sheetListModel = DocumentListModel::CreateAndInsert(*partition);
    if (!sheetListModel.IsValid())
        return  BSIERROR;

    m_sheetListModelId = sheetListModel->GetModelId();

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::InitDrawingListModel ()
    {
    Utf8CP partitionName = "Imported Drawings";
    DgnCode partitionCode = GroupInformationPartition::CreateCode(GetJobSubject(), partitionName);
    DgnElementId partitionId = m_dgndb->Elements().QueryElementIdByCode(partitionCode);
    m_drawingListModelId = DgnModelId(partitionId.GetValueUnchecked());
    
    if (m_drawingListModelId.IsValid())
        return BentleyStatus::SUCCESS;

    DocumentPartitionPtr ed = DocumentPartition::Create(GetJobSubject(), partitionName);
    DocumentPartitionCPtr partition = ed->InsertT<DocumentPartition>();
    if (!partition.IsValid())
        return BentleyStatus::BSIERROR;

    DocumentListModelPtr drawingListModel = DocumentListModel::CreateAndInsert(*partition);
    if (!drawingListModel.IsValid())
        return BentleyStatus::BSIERROR;

    m_drawingListModelId = drawingListModel->GetModelId();
    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode         DwgImporter::CreateCode (Utf8StringCR value) const
    {
    auto auth = m_dgndb->CodeSpecs().GetCodeSpec(m_businessKeyCodeSpecId);
    BeDataAssert(auth.IsValid());
    return auth.IsValid() ? auth->CreateCode(GetImportJob().GetSubject(), value) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus     DwgImporter::UpdateElementName (DgnElementR editElement, Utf8StringCR newValue, Utf8CP label, bool save)
    {
    // change both the code value, and optional a user lable as well, of an element.
    auto code = editElement.GetCode ();
    auto status = editElement.SetCode (DgnCode::From(code.GetCodeSpecId(), code.GetScopeString(), newValue));
    if (status == DgnDbStatus::Success)
        {
        if (nullptr != label)
            editElement.SetUserLabel (label);
        if (save)
            editElement.Update (&status);
        }
    if (status != DgnDbStatus::Success)
        this->ReportIssueV (IssueSeverity::Error, IssueCategory::Briefcase(), Issue::CannotUpdateName(), "Element", code.GetValueUtf8().c_str(), newValue.c_str());
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::InitBusinessKeyCodeSpec ()
    {
    m_businessKeyCodeSpecId = m_dgndb->CodeSpecs().QueryCodeSpecId (s_codeSpecName);
    if (!m_businessKeyCodeSpecId.IsValid())
        {
        CodeSpecPtr auth = CodeSpec::Create (*m_dgndb, s_codeSpecName);
        BeAssert(auth.IsValid());
        if (auth.IsValid())
            {
            auth->Insert();
            m_businessKeyCodeSpecId = auth->GetCodeSpecId();
            }
        }

    BeAssert (m_businessKeyCodeSpecId.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::ReportDbFileStatus (DbResult fileStatus, BeFileNameCR projectFileName)
    {
    auto category = IssueCategory::DiskIO();
    auto issue = Issue::Error();
    switch (fileStatus)
        {
        case BE_SQLITE_ERROR_FileNotFound:
            issue = Issue::FileNotFound();
            break;

        case BE_SQLITE_CORRUPT:
        case BE_SQLITE_ERROR_NoPropertyTable:
            issue = Issue::NotADgnDb();
            break;

        case BE_SQLITE_ERROR_InvalidProfileVersion:
        case BE_SQLITE_ERROR_ProfileUpgradeFailed:
        case BE_SQLITE_ERROR_ProfileTooOldForReadWrite:
        case BE_SQLITE_ERROR_ProfileTooOld:
            category = IssueCategory::Compatibility();
            issue = Issue::Error();
            break;
        }

    this->ReportError(category, issue, projectFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgImporter::SetDgnDb (DgnDbR db)
    {
    m_dgndb = &db;
    m_sourceAspects.Initialize (*m_dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
StableIdPolicy  DwgImporter::_GetDwgFileIdPolicy () const
    {
    // let the bridge decide the ID policy
    return m_options.GetStableIdPolicy();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::OpenDwgFile (BeFileNameCR dwgdxfName)
    {
    if (!dwgdxfName.DoesPathExist())
        {
        this->ReportError (IssueCategory::DiskIO(), Issue::FileNotFound(), dwgdxfName.c_str());
        return  static_cast<BentleyStatus> (DgnDbStatus::FileNotFound);
        }

    DwgFileVersion  dwgVersion = DwgFileVersion::Invalid;
    if (!DwgHelper::SniffDwgFile(dwgdxfName, &dwgVersion) && (DwgFileVersion::Newer == dwgVersion || !DwgHelper::SniffDxfFile(dwgdxfName, &dwgVersion)))
        {
        this->ReportError (IssueCategory::DiskIO(), DwgFileVersion::Newer == dwgVersion ? Issue::NewerDwgVersion() : Issue::NotRecognizedFormat(), dwgdxfName.c_str());
        return  static_cast<BentleyStatus> (DwgFileVersion::Newer == dwgVersion ? DgnDbStatus::VersionTooNew : DgnDbStatus::NotOpen);
        }
    
    WString  password;
    password.AppendUtf8 (this->GetOptions().GetPassword().c_str());

    Utf8String  dispVersion = DwgHelper::GetStringFromDwgVersion (dwgVersion);
    this->SetStepName (ProgressMessage::STEP_OPENINGFILE(), dwgdxfName.c_str(), dispVersion.c_str());

    /*-----------------------------------------------------------------------------------
    Apparently file open mode DenyNo somehow has a negative impact on nested xref blocks: 
    there have been cases in which certain nested xref blocks are not seen in a master 
    file's block table.  See unit test BasicTests.AttachXrefs for one such a case.  
    Strange as it seems, file open mode DenyWrite does allow all nested xref blocks to be 
    iterated through in a master file's block table.  Since the importer does not need to 
    change the file on disc, i.e. delete etc, we opt for the file open mode DenyWrite.  
    We can still make changes on objects and save changes back into database.

    However, if someone else has already had the file open for write, we resort to DenyNo.
    -----------------------------------------------------------------------------------*/
    auto openMode = DwgHelper::CanOpenForWrite(dwgdxfName) ? FileShareMode::DenyWrite : FileShareMode::DenyNo;

    m_dwgdb = DwgImportHost::GetHost().ReadFile (dwgdxfName, false, false, openMode, password);
    if (!m_dwgdb.IsValid())
        return  BSIERROR;

    m_rootFileName = dwgdxfName;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportSpaces ()
    {
    // save off effective DWG file units
    m_modelspaceUnits = DwgHelper::GetStandardUnitFromDwgUnit (m_dwgdb->GetINSUNITS());
    if (m_modelspaceUnits == StandardUnit::None)
        m_modelspaceUnits = m_options.GetUnspecifiedBlockUnits ();

    // save off active viewport - either a modelspace viewport(timemode=1) or a layout viewport(tilemode=0):
    if (m_dwgdb->GetTILEMODE())
        {
        // modelspace is active
        m_activeViewportId = m_dwgdb->GetActiveModelspaceViewportId();
        }
    else if (this->IsUpdating())
        {
        // a layout is active, try find the active layout viewport if updating:
        auto manager = DwgImportHost::GetHost().GetLayoutManager ();
        if (!manager.IsNull() && manager->IsValid())
            {
            DwgDbLayoutPtr layout (manager->FindActiveLayout(m_dwgdb.get()), DwgDbOpenMode::ForRead);
            if (layout.OpenStatus() == DwgDbStatus::Success)
                {
                DwgDbObjectIdArray  viewports;
                if (layout->GetViewports(viewports) > 0)
                    {
                    // layout has the overal viewport exists
                    m_activeViewportId = viewports.front ();
                    }
                else
                    {
                    // layout not visited - try finding the first viewport
                    DwgDbBlockTableRecordPtr block(manager->GetActiveLayoutBlock(m_dwgdb.get()), DwgDbOpenMode::ForRead);
                    if (block.OpenStatus() == DwgDbStatus::Success)
                        m_activeViewportId = LayoutFactory::FindOverallViewport (*block);
                    }
                }
            }

        }

    // if updating, we may not even have a chance to process layouts, so find and set active viewport now:
    if (this->IsUpdating() && m_activeViewportId.IsValid())
        {
        auto viewtype = m_dwgdb->GetTILEMODE() ? DwgSourceAspects::ViewAspect::SourceType::ModelSpaceViewport : DwgSourceAspects::ViewAspect::SourceType::PaperSpaceViewport;
        m_defaultViewId = m_sourceAspects.FindView (m_activeViewportId, viewtype);
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::_BeginImport ()
    {
    this->_GetChangeDetector()._Prepare (*this);
    m_dgndb->AddIssueListener (m_issueReporter.GetECDbIssueListener());

    // get imported DWG schemas
    auto schemaName = DwgHelper::GetAttrdefECSchemaName (m_dwgdb.get());
    m_attributeDefinitionSchema = m_dgndb->Schemas().GetSchema (schemaName.c_str(), true);
    m_aecPropertySetSchema = m_dgndb->Schemas().GetSchema (SCHEMAName_AecPropertySets, true);
    m_dwgAppDataSchema = m_dgndb->Schemas().GetSchema (SCHEMAName_DwgAppData, true);

    m_hasBegunProcessing = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::_FinishImport ()
    {
    // initialize project extents before processing views
    m_dgndb->GeoLocation().InitializeProjectExtents();

    _EmbedFonts ();
    _EmbedPresentationRules ();

    if (m_defaultViewId.IsValid())
        {
        /*-------------------------------------------------------------------------------
        Set the DefaultView ID property only if:
            a) the property is not found in DB, or
            b) m_defaultViewId was set from a valid active viewport.
        -------------------------------------------------------------------------------*/
        PropertySpec    prop = DgnViewProperty::DefaultView ();
        DgnElementId    existingId;
        if (m_dgndb->QueryProperty(&existingId, sizeof(existingId), prop) != DbResult::BE_SQLITE_OK || m_activeViewportId.IsValid())
            m_dgndb->SaveProperty (prop, &m_defaultViewId, sizeof(m_defaultViewId));
        }

    IDwgChangeDetector& changeDetector = _GetChangeDetector ();
    if (IsUpdating())
        {
        DwgDbDatabaseP  dwg = m_dwgdb.get ();

        if (this->GetOptions().DoDetectDeletedModelsAndElements())
            {
            // begin element/mode deletion marked up by the change detector:
            if (nullptr != dwg)
                {
                changeDetector._DetectDeletedElementsInFile (*this, *dwg);
                changeDetector._DetectDeletedModelsInFile (*this, *dwg);
                }
            for (auto& xref : m_loadedXrefFiles)
                {
                if (nullptr != (dwg = xref.GetDatabaseP()))
                    {
                    changeDetector._DetectDeletedElementsInFile (*this, *dwg);
                    changeDetector._DetectDeletedModelsInFile (*this, *dwg);
                    }
                }
            changeDetector._DetectDeletedMaterials (*this);
            changeDetector._DetectDeletedViews (*this);
            changeDetector._DetectDeletedGroups (*this);
            changeDetector._DetectDetachedXrefs (*this);
            changeDetector._DetectDeletedElementsEnd (*this);
            changeDetector._DetectDeletedModelsEnd (*this);
            }

        // update ExternalSourceAspect for master DWG file
        dwg = m_dwgdb.get ();
        auto sourceAspects = this->GetSourceAspects ();
        StableIdPolicy  policy = GetCurrentIdPolicy ();
        auto repLink = GetDgnDb().Elements().GetForEdit<RepositoryLink>(GetRepositoryLink(dwg));
        auto aspect = DwgSourceAspects::RepositoryLinkAspect::GetForEdit(*repLink);
        if (aspect.IsValid())
            {
            aspect.Update(DwgSourceAspects::DwgFileInfo(*dwg, *this));
            repLink->Update();
            }

        // update ExternalSourceAspects for xref files
        for (auto& xref : m_loadedXrefFiles)
            {
            if (nullptr != (dwg = xref.GetDatabaseP()))
                {
                auto repLink = GetDgnDb().Elements().GetForEdit<RepositoryLink>(GetRepositoryLink(dwg));
                if (repLink.IsValid())
                    {
                    auto aspect = DwgSourceAspects::RepositoryLinkAspect::GetForEdit(*repLink);
                    if (aspect.IsValid())
                        {
                        aspect.Update(DwgSourceAspects::DwgFileInfo(*dwg, *this));
                        repLink->Update();
                        }
                    }
                }
            }
        }
    changeDetector._Cleanup (*this);

    // now that not seen views are deleted, update survived views and generate thumbnails as needed:
    _PostProcessViewports ();
    ValidateJob ();

    if (WasAborted())
        {
        m_dgndb->AbandonChanges();
        return ;
        }

#ifdef DEBUG_DELETE_DOCUMENTS
    this->_DetectDeletedDocuments ();
#endif
    m_hasBegunProcessing = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_MakeDefinitionChanges (SubjectCR jobSubject)
    {
    // only import dictionary tables that are shared with other bridges
    if (!m_hasBegunProcessing)
        {
        this->_BeginImport ();
        if (this->WasAborted())
            return BSIERROR;
        }

    StopWatch totalTimer (true);
    StopWatch timer (true);

    timer.Start();
    this->_ImportTextStyleSection ();
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Text Styles");

    timer.Start();
    this->_ImportLineTypeSection ();
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Linetype Section");

    timer.Start();
    this->_ImportMaterialSection ();
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Material Section");

    timer.Start();
    this->_ImportLayerSection ();
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Layer Section");

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::Process ()
    {
    // import everything else that has not been imported in previous MakeDefinitionChanges
    StopWatch totalTimer (true);
    StopWatch timer (true);

    if (m_dwgdb.IsNull() || m_dgndb.IsNull())
        {
        BeAssert (false && L"Both DwgDbDatabase and DgnDb must be created prior to import or update DWG!");
        return  BSIERROR;
        }

    if (!m_hasBegunProcessing)
        {
        this->_BeginImport ();
        if (this->WasAborted())
            return BSIERROR;
        }

    this->_ImportSpaces ();
    DwgImportLogging::LogPerformance(timer, "Create Spaces");

    timer.Start();
    this->_ImportDwgModels ();
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Create Models");

    timer.Start();
    this->_ImportModelspaceViewports ();
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Modelspace Viewports");

    timer.Start();
    this->_ImportEntitySection ();
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Entity Section/ModelSpace");

    timer.Start();
    this->_ImportLayouts ();
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Layouts");

    timer.Start();
    this->_ImportGroups ();
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Groups");

    timer.Start ();
    this->_FinishImport ();
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Finish importing");

    DwgImportLogging::LogPerformance(totalTimer, "Total conversion time");

    return this->WasAborted () ? BSIERROR : BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::DwgImporter (DwgImporter::Options& options) : m_options(options), m_sourceAspects(*this), m_config(options.GetConfigFile(), *this), m_dgndb(), m_dwgdb(), m_loadedFonts(*this), m_issueReporter(options.GetReportFileName())
    {
    m_wasAborted = false;
    m_rootFileName.Clear ();
    m_fileCount = 0;
    m_currIdPolicy = options.GetStableIdPolicy ();
    m_modelspaceXrefs.clear ();
    m_paperspaceXrefs.clear ();
    m_layersInSync.clear ();
    m_importedTextstyles.clear ();
    m_importedLinestyles.clear ();
    m_importedMaterials.clear ();
    m_blockPartsMap.clear ();
    m_entitiesImported = 0;
    m_layersImported = 0;
    m_errorCount = 0;
    m_attributeDefinitionSchema = 0;
    m_aecPropertySetSchema = 0;
    m_dwgAppDataSchema = 0;
    m_constantBlockAttrdefList.clear ();
    m_modelspaceUnits = StandardUnit::None;
    m_modelspaceId.SetNull ();
    m_currentspaceId.SetNull ();
    m_dwgModelMap.clear ();
    m_materialSearchPaths.clear ();
    m_hasBegunProcessing = false;
    m_isProcessingDwgModelMap = false;
    m_presentationRuleContents.clear ();
    m_sizeTolerance = 0.0;
    m_displayPriority = 0;

    this->SetStepName (ProgressMessage::STEP_INITIALIZING());

    // read config file, if supplied
    if (!m_config.GetInstanceFilename().empty())
        m_config.ReadFromXmlFile();

    T_Utf8StringVector  userObjectEnablers;
    this->ParseConfigurationFile (userObjectEnablers);

    DwgImportHost::GetHost().Initialize (*this);
    DwgImportHost::GetHost().NewProgressMeter ();
    DwgImporter::RegisterProtocolExtensions ();

    // try loading user object enablers if specified
    for (auto& oe : userObjectEnablers)
        DwgImportHost::GetHost().LoadObjectEnabler (BeFileName(oe.c_str()));

    // load fonts before we start reading the DWG file as the toolkit may search for them via the hostApp:
    m_loadedFonts.LoadFonts ();

#if defined (BENTLEYCONFIG_PARASOLID)
    PSolidKernelManager::StartSession ();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::~DwgImporter ()
    {
    if (m_issueReporter.HasIssues())
        m_issueReporter.CloseReport ();

    m_modelspaceXrefs.clear ();
    m_paperspaceXrefs.clear ();
    m_paperspaceViews.clear ();
    m_loadedXrefFiles.clear ();
    m_presentationRuleContents.clear ();
    m_reportedIssues.clear ();

    DwgImporter::UnRegisterProtocolExtensions ();

    DwgImportHost::GetHost().Terminate ();
#if defined (BENTLEYCONFIG_PARASOLID)
    PSolidKernelManager::StopSession ();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::ParseConfigurationFile (T_Utf8StringVectorR userObjectEnablers)
    {
    BeXmlDomP   xmlDom = m_config.GetDom ();
    if (nullptr == xmlDom)
        {
        this->ReportError (IssueCategory::ConfigXml(), Issue::Error(), "invalid configurations!");
        return;
        }

    // parse line weight mapping
    Options::T_DwgWeightMap     weightMap;
    BeXmlDom::IterableNodeSet   nodes;
    xmlDom->SelectNodes (nodes, "/ConvertConfig/LineWeightMapping/MapEntry", nullptr);
    for (auto node : nodes)
        {
        Utf8String          dwgWeight;
        uint32_t            dgnWeight = 0;
        if (BeXmlStatus::BEXML_Success == node->GetAttributeStringValue(dwgWeight, "DwgWeight") && BeXmlStatus::BEXML_Success == node->GetAttributeUInt32Value(dgnWeight, "DgnWeight"))
            {
            Utf8String  token (::strtok(const_cast<Utf8P>(dwgWeight.c_str()), " \t\n"));
            if (!token.empty())
                {
                DwgDbLineWeight     dwgWeightEnum = DwgHelper::GetDwgLineWeightFromWeightName (token);
                weightMap.insert (bpair<DwgDbLineWeight,uint32_t>(dwgWeightEnum, dgnWeight));
                }
            }
        }

    if (weightMap.empty())
        this->ReportError (IssueCategory::ConfigXml(), Issue::Error(), "failed parsing lineweight mapping!");
    else
        m_options.SetLineWeightMapping (weightMap);

    // retrieve fallback model units:
    Utf8String      str = m_config.GetXPathString("/ConvertConfig/Models/UnspecifiedUnits/@units", "");
    StandardUnit    units = StandardUnit::MetricMeters;
    if (!str.empty())
        {
        Utf8String  token (::strtok(const_cast<Utf8P>(str.c_str()), " \t\n"));
        if (!token.empty())
            units = DwgHelper::GetStandardUnitFromUnitName (token);
        }
    m_options.SetUnspecifiedBlockUnits (units);

    // retrieve fallback model import rule:
    xmlDom->SelectNodes(nodes, "/ConvertConfig/Models/ImportRules/If", nullptr);
    for (auto node : nodes)
        {
        ImportRule rule;
        rule.InitFromXml(*node);
        m_modelImportRules.push_back(rule);
        }

    // retrieve the modelspace options
    str = m_config.GetXPathString("/ConvertConfig/Models/Modelspace/@TreatAs", "");
    if (str.EqualsI("2D"))
        m_options.SetTreatModelspaceAs2D (true);
    else
        m_options.SetTreatModelspaceAs2D (false);

    // parse layer options
    if (m_options.GetCopyLayer() != Options::CopyLayer::UseConfig)
        return;

    Options::CopyLayer  v = Options::CopyLayer::Never;
    str = m_config.GetXPathString("/ConvertConfig/Layers/@copy", "");
    if (str.EqualsI("always"))
        v = Options::CopyLayer::Always;
    else if (str.EqualsI("ifdifferent"))
        v = Options::CopyLayer::IfDifferent;
    m_options.SetCopyLayer(v);

    // check general options
    xmlDom->SelectNodes (nodes, "/ConvertConfig/Options/OptionBool", nullptr);
    for (auto node : nodes)
        {
        bool        boolValue = false;
        if (BeXmlStatus::BEXML_Success == node->GetAttributeStringValue(str, "name") && BeXmlStatus::BEXML_Success == node->GetAttributeBooleanValue(boolValue, "value"))
            {
            if (str.EqualsI("SyncBlockChanges"))
                m_options.SetSyncBlockChanges (boolValue);
            else if (str.EqualsI("PreferRenderableGeometry"))
                m_options.SetPreferRenderableGeometry (boolValue);
            else if (str.EqualsI("ConvertAsmAsParasolid"))
                m_options.SetAsmAsParasolid (boolValue);
            else if (str.EqualsI("ConvertBlockSharedParts"))
                m_options.SetBlockAsSharedParts(boolValue);
            else if (str.EqualsI("ConvertFilledHatch"))
                m_options.SetFilledHatchAsFilledElement(boolValue);
            else if (str.EqualsI("SyncDwgVersionGuid"))
                m_options.SetSyncDwgVersionGuid (boolValue);
            else if (str.EqualsI("SyncAsmBodyInFull"))
                m_options.SetSyncAsmBodyInFull(boolValue);
            else if (str.EqualsI("SyncDependentObjects"))
                m_options.SetSyncDependentObjects(boolValue);
            }
        }

    // check raster import options
    str = m_config.GetXPathString("/ConvertConfig/Raster/@ImportAttachments", "");
    m_options.SetImportRasterAttachments (str.EqualsI("true"));
    
    // check point clouds import options
    xmlDom->SelectNodes (nodes, "/ConvertConfig/PointClouds", nullptr);
    for (auto node : nodes)
        {
        if (BeXmlStatus::BEXML_Success == node->GetAttributeStringValue(str, "ImportPointClouds"))
            m_options.SetImportPointClouds (str.EqualsI("true"));
            
        uint32_t    lod = 0;
        if (BeXmlStatus::BEXML_Success == node->GetAttributeUInt32Value(lod, "LevelOfDetails"))
            m_options.SetPointCloudLevelOfDetails (static_cast<uint16_t>(lod));
        }

    // check for user object enablers that need to be explicitly loaded
    xmlDom->SelectNodes (nodes, "/ConvertConfig/ObjectEnablers/PreLoadModules/Module", nullptr);
    for (auto node : nodes)
        {
        if (BeXmlStatus::BEXML_Success == node->GetAttributeStringValue(str, "name") && !str.empty())
            userObjectEnablers.push_back (str);
        }   

    // check material search paths
    xmlDom->SelectNodes (nodes, "/ConvertConfig/Materials", nullptr);
    for (auto node : nodes)
        {
        // collect explicit search paths
        BeXmlDom::IterableNodeSet   childNodes;
        node->SelectChildNodes (childNodes, "SearchPaths/Path");
        for (auto childNode : childNodes)
            {
            if (BeXmlStatus::BEXML_Success == childNode->GetAttributeStringValue(str, "name"))
                {
                BeFileName  dir(str.c_str(), BentleyCharEncoding::Utf8);
                if (dir.IsDirectory())
                    {
                    dir.AppendSeparator ();
                    m_materialSearchPaths.push_back (dir);
                    }
                }
            }
        // check if the input DWG path should also be searched, in addition to explicit search paths
        bool    boolValue = false;
        node->SelectChildNodes (childNodes, "SearchOptions");
        for (auto childNode : childNodes)
            {
            if (BeXmlStatus::BEXML_Success == childNode->GetAttributeBooleanValue(boolValue, "IncludeDwgPath"))
                m_options.SetDwgPathInMaterialSearch (boolValue);
            }
        }

    xmlDom->SelectNodes (nodes, "/ConvertConfig/ElementClassMapping/AecPropertySet", nullptr);
    for (auto node : nodes)
        {
        // collect element class map
        Utf8String  str2, str3;
        if (BeXmlStatus::BEXML_Success == node->GetAttributeStringValue(str, "FromPropertyName") &&
            BeXmlStatus::BEXML_Success == node->GetAttributeStringValue(str2, "FromPropertyValue") &&
            BeXmlStatus::BEXML_Success == node->GetAttributeStringValue(str3, "ToClassName"))
            {
            // skip empty key names
            if (str.empty() || str3.empty())
                continue;

            Utf8String  str4;
            if (BeXmlStatus::BEXML_Success != node->GetAttributeStringValue(str4, "ElementLabelFrom"))
                str4.clear ();
            m_elementClassMap.push_back (ElementClassMap(str, str2, str3, str4));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::SearchForMatchingRule (ImportRule& entryOut, Utf8StringCR modelName, BeFileNameCR baseFilename)
    {
    if (modelName.empty() || baseFilename.empty())
        return  BSIERROR;

    for (auto const& rule : m_modelImportRules)
        {
        if (rule.Matches(modelName, baseFilename))
            {
            entryOut = rule;
            LOG_MODEL.tracev("Model %s <- Rule %s", modelName.c_str(), rule.ToString().c_str());
            return BSISUCCESS;
            }
        }

    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t queryXmlRuleStringAttribute(Utf8String& value, BeXmlNode& xml, Utf8CP optionName)
    {
    if (BEXML_Success == xml.GetAttributeStringValue(value, optionName))
        return 1;
    Utf8String notOptionName("not_");
    notOptionName.append(optionName);
    return (BEXML_Success == xml.GetAttributeStringValue(value, notOptionName.c_str())) ? 2 : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            ImportRule::InitFromXml(BeXmlNode& xml)
    {
    m_matchOnBase.file = queryXmlRuleStringAttribute(m_file, xml, "file");
    m_file.ToLower();

    m_matchOnBase.name = queryXmlRuleStringAttribute(m_name, xml, "name");
    m_name.ToLower();

    auto thenClause = xml.GetFirstChild();
    if (thenClause == nullptr)
        return;

    m_hasNewName = (BEXML_Success == thenClause->GetAttributeStringValue(m_newName, "newName"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String ruleToString(Utf8CP name, uint32_t wantMatch, Utf8StringCR value)
    {
    if (wantMatch==0)
        return "";

    Utf8String str(" ");
    str.append(name);
    str.append((wantMatch==1)? "=": "!=");
    str.append(value);
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
static bool matchesString(Utf8StringCR pattern, uint32_t wantMatch, Utf8StringCR str)
    {
    if (wantMatch==0)
        return true;        // wantMatch==0 means that this property should not be checked. Return true, since we have nothing to say about it.

    if (pattern.find_first_of("*?") == Utf8String::npos)
        return (wantMatch==1) == pattern.EqualsI(str);

    return (wantMatch==1) == FileNamePattern::MatchesGlob(BeFileName(str.c_str(),true), WString(pattern.c_str(),true).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ImportRule::Matches(Utf8StringCR name, BeFileNameCR baseFilename) const
    {
    if (m_matchOnBase.file && !matchesString(m_file, m_matchOnBase.file, Utf8String(baseFilename)))
        return false;

    if (m_matchOnBase.name && !matchesString(m_name, m_matchOnBase.name, name))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      ImportRule::ToString() const
    {
    Utf8String str("If ");
    str.append(ruleToString("file", m_matchOnBase.file, m_file));
    str.append(ruleToString("name", m_matchOnBase.name, m_name));
    str.append(" then");

    if (m_hasNewName)
        str.append(" newName=").append(m_newName);

    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ImportRule::ComputeNewName(Utf8StringR newName, BeFileNameCR baseFilename) const
    {
    if (!m_hasNewName)
        return BSIERROR;

    if (newName.empty())
        {
        if (m_newName.empty())
            return BSIERROR;

        newName = m_newName;
        newName.Trim();
        }

    static Utf8CP fileStr = "%file";
    size_t pos = newName.find(fileStr);
    if (pos != Utf8String::npos)
        newName.replace(pos, strlen(fileStr), Utf8String(baseFilename));

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ImportRule::ComputeNewName(Utf8StringR newName, Utf8StringCR modelName, BeFileNameCR baseFilename) const
    {
    if (!m_hasNewName)
        return BSIERROR;

    newName = m_newName;
    newName.Trim();

    Utf8String oldName(modelName.c_str());

    static Utf8CP modelStr = "%model";
    size_t pos = newName.find(modelStr);
    if (pos != Utf8String::npos)
        newName.replace(pos, strlen(modelStr), oldName);

    return ComputeNewName(newName, baseFilename);
    }                       

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        DwgImporter::Options::GetDgnLineWeight (DwgDbLineWeight dwgWeight) const
    {
    auto    found = m_lineweightMapping.find (dwgWeight);
    if (found != m_lineweightMapping.end())
        return  found->second;

    return  0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::ElementImportInputs::ElementImportInputs (DgnModelR model) 
    : m_targetModel(model), m_spatialFilter(nullptr), m_parentEntity(nullptr), m_templateEntity(nullptr)
    {
    m_transformToDgn.InitIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::ElementImportInputs::ElementImportInputs (DgnModelR model, DwgDbEntityP entity, ElementImportInputs const& other) 
    : m_targetModel(model)
    {
    this->SetClassId (other.GetClassId());
    this->SetTransform (other.GetTransform());
    this->SetEntityId (other.GetEntityId());
    this->SetParentEntity (other.GetParentEntity());
    this->SetTemplateEntity (other.GetTemplateEntity());
    this->SetSpatialFilter (other.GetSpatialFilter());
    this->SetModelMapping (other.GetModelMapping());
    this->GetEntityPtrR() = entity;
    }
