/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgImporter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DGNDBSYNC_DWG

static DgnPlatformLib::Host*        s_dgndbHost = nullptr;
static const Utf8CP                 s_codeSpecName = "DWG";    // TBD: One authority per DWG file?

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::InitializeDgnHost (DgnPlatformLib::Host& dgndbHost)
    {
    // Initialize DgnPlatform host - DWG host is initialized in the constructor.
    if (!dgndbHost.IsInitialized())
        DgnPlatformLib::Initialize (dgndbHost, false);

    s_dgndbHost = &dgndbHost;

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
void            DwgImporter::ImportAttributeDefinitionSchema (ECSchemaR attrdefSchema)
    {
    ECSchemaCachePtr        schemaCache = ECSchemaCache::Create ();
    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext ();
    if (!schemaCache.IsValid() || !schemaContext.IsValid())
        return;

    this->SetStepName (ProgressMessage::STEP_IMPORTING_ATTRDEFSCHEMA(), attrdefSchema.GetClassCount());

    // import attrdef schema
    schemaContext->AddSchemaLocater (m_dgndb->GetSchemaLocater());
    schemaContext->AddSchema (attrdefSchema);
    schemaContext->RemoveSchemaLocater (m_dgndb->GetSchemaLocater());

    m_dgndb->ImportSchemas(schemaContext->GetCache().GetSchemas());

    // get back newly added schema:
    m_attributeDefinitionSchema = m_dgndb->Schemas().GetSchema (attrdefSchema.GetName().c_str(), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::CreateNewDgnDb (BeFileNameCR projectName)
    {
    this->SetStepName (ProgressMessage::STEP_CREATING(), Utf8String(projectName).c_str());

    BeFileName::BeDeleteFile (projectName);

    CreateDgnDbParams   createParams;
    createParams.SetOverwriteExisting (true);
    createParams.SetRootSubjectDescription (m_options.GetDescription().c_str());
    createParams.SetExpirationDate(m_options.GetExpirationDate());

    Utf8String  subjectName(projectName.GetFileNameWithoutExtension());
    createParams.SetRootSubjectName (subjectName.c_str());

    DbResult    status = BE_SQLITE_ERROR;
    m_dgndb = DgnDb::CreateDgnDb (&status, projectName, createParams);

    if (!m_dgndb.IsValid())
        {
        this->ReportDbFileStatus (status, projectName);
        return this->OnFatalError ();
        }

    if (BSISUCCESS != m_syncInfo.CreateEmptyFile(DwgSyncInfo::GetDbFileName(*m_dgndb)))
        {
        this->ReportSyncInfoIssue (DwgImporter::IssueSeverity::Fatal, DwgImporter::IssueCategory::Sync(), DwgImporter::Issue::CantCreateSyncInfo(), "");
        return this->OnFatalError();
        }

    m_dgndb->AddIssueListener (m_issueReporter.GetECDbIssueListener());

    // Create the "uncategorized" category for elements that can't be categorized without application/domain help
    this->InitUncategorizedCategory ();
    this->InitBusinessKeyCodeSpec ();
    this->InitSheetListModel ();

    return this->AttachSyncInfo ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::OpenExistingDgnDb (BeFileNameCR projectName)
    {
    DbResult    status = BE_SQLITE_ERROR;
    m_dgndb = DgnDb::OpenDgnDb (&status, projectName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));

    if (!m_dgndb.IsValid())
        {
        this->ReportDbFileStatus (status, projectName);
        return this->OnFatalError ();
        }

    m_dgndb->AddIssueListener (m_issueReporter.GetECDbIssueListener());

    this->InitUncategorizedCategory ();
    this->InitBusinessKeyCodeSpec ();
    this->InitSheetListModel ();

    return this->AttachSyncInfo ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::InitSheetListModel ()
    {
    Utf8CP  partitionName = "Converted Sheets";
    DgnCode partitionCode = GroupInformationPartition::CreateCode(*m_dgndb->Elements().GetRootSubject(), partitionName);
    DgnElementId partitionId = m_dgndb->Elements().QueryElementIdByCode(partitionCode);
    m_sheetListModelId = DgnModelId(partitionId.GetValueUnchecked());
    
    if (m_sheetListModelId.IsValid())
        return  BSISUCCESS;

    DocumentPartitionCPtr partition = DocumentPartition::CreateAndInsert(*m_dgndb->Elements().GetRootSubject(), partitionName);
    if (!partition.IsValid())
        return  BSIERROR;

    DocumentListModelPtr sheetListModel = DocumentListModel::CreateAndInsert(*partition);
    if (!sheetListModel.IsValid())
        return  BSIERROR;

    m_sheetListModelId = sheetListModel->GetModelId();

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode         DwgImporter::CreateCode (Utf8StringCR value) const
    {
    auto auth = m_dgndb->CodeSpecs().GetCodeSpec(m_businessKeyCodeSpecId);
    BeDataAssert(auth.IsValid());
    return auth.IsValid() ? auth->CreateCode(value) : DgnCode();
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
    auto category = DwgImporter::IssueCategory::DiskIO();
    auto issue = DwgImporter::Issue::Error();
    switch (fileStatus)
        {
        case BE_SQLITE_ERROR_FileNotFound:
            issue = DwgImporter::Issue::FileNotFound();
            break;

        case BE_SQLITE_CORRUPT:
        case BE_SQLITE_ERROR_NoPropertyTable:
            issue = DwgImporter::Issue::NotADgnDb();
            break;

        case BE_SQLITE_ERROR_InvalidProfileVersion:
        case BE_SQLITE_ERROR_ProfileUpgradeFailed:
        case BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite:
        case BE_SQLITE_ERROR_ProfileTooOld:
            category = DwgImporter::IssueCategory::Compatibility();
            issue = DwgImporter::Issue::Error();
            break;
        }

    this->ReportError(category, issue, projectFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::AttachSyncInfo ()
    {
    // Create and then attach the .syncInfo file to the project
    BeFileName syncInfoFileName = DwgSyncInfo::GetDbFileName(*m_dgndb);

    BentleyStatus   status = m_syncInfo.AttachToProject (this->GetDgnDb(), syncInfoFileName);
    if (BentleyApi::SUCCESS != status)
        {
        Utf8CP  reason = "";
        if (DgnDbStatus::VersionTooNew == static_cast<DgnDbStatus>(status))
            reason = "file newer than program version";
        else if (DgnDbStatus::VersionTooOld == static_cast<DgnDbStatus>(status))
            reason = "file version too old";

        this->ReportSyncInfoIssue(DwgImporter::IssueSeverity::Fatal, DwgImporter::IssueCategory::Sync(), DwgImporter::Issue::CantOpenSyncInfo(), reason);
        BeFileName::BeDeleteFile(syncInfoFileName.c_str());
        return this->OnFatalError (IssueCategory::Sync(), Issue::FatalError(), reason);
        }

    BeAssert(m_syncInfo.IsValid());
    BeAssert(!WasAborted());

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::_AddFileInSyncInfo (DwgDbDatabaseP dwg, StableIdPolicy policy)
    {
    if (nullptr == dwg)
        {
        BeAssert (false && L"null DwgDbDatabase!");
        return;
        }

    DwgSyncInfo::FileProvenance provenance(*dwg, m_syncInfo, policy);
    if (!provenance.FindByName(true))
        {
        provenance.Insert();

        if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
            LOG.tracev("+ %s => %lld", DwgHelper::ToUtf8CP(dwg->GetFileName()), provenance.m_syncId.GetValue());
        }

    dwg->SetFileIdPolicy (provenance.m_syncId.GetValue(), static_cast<uint32_t>(provenance.m_idPolicy));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
StableIdPolicy  DwgImporter::_GetDwgFileIdPolicy (DwgDbDatabaseCR dwg) const
    {
    if (StableIdPolicy::ByHash == m_options.GetStableIdPolicy())
        StableIdPolicy::ByHash;

    return StableIdPolicy::ById;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::OpenDwgFile (BeFileNameCR dwgdxfName)
    {
    this->SetStepName (ProgressMessage::STEP_OPENINGFILE(), dwgdxfName.c_str());

    // load fonts before we start reading the DWG file as the toolkit may search for them via the hostApp:
    m_loadedFonts.LoadFonts ();

    WString  password;
    password.AppendUtf8 (this->GetOptions().GetPassword().c_str());

    m_dwgdb = DwgImportHost::GetHost().ReadFile (dwgdxfName, false, false, FileShareMode::DenyNo, password);
    if (!m_dwgdb.IsValid())
        return  BSIERROR;

    m_rootFileName = dwgdxfName;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::ResolvedModelMapping   DwgImporter::GetModelFromSyncInfo (DwgDbObjectIdCR id, DwgDbDatabaseR dwg, TransformCR trans)
    {
    // retrieve a model by file & model instance ID of a model/paperspace block, xref insert or a raster image.
    if (!id.IsValid())
        return ResolvedModelMapping();

    // the input file is the source file of the model - for an xref model it is the xref file:
    DwgSyncInfo::DwgFileId  fileId = DwgSyncInfo::DwgFileId::GetFrom (dwg);

    DwgSyncInfo::ModelIterator  it(*m_dgndb, "DwgFileId=? AND DwgInstanceId=?");
    it.GetStatement()->BindInt (1, fileId.GetValue());
    it.GetStatement()->BindInt64 (2, id.ToUInt64());

    for (auto entry=it.begin(); entry!=it.end(); ++entry)
        {
        if (entry.GetTransform().IsEqual(trans))
            {
            auto    model = m_dgndb->Models().GetModel (entry.GetModelId());
            if (model.IsValid())
                return  ResolvedModelMapping(id, model.get(), entry.GetMapping());
            }
        }

    return ResolvedModelMapping();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::ResolvedModelMapping   DwgImporter::FindModel (DwgDbObjectIdCR dwgModelId, TransformCR trans, DwgSyncInfo::ModelSourceType sourceType)
    {
    // find cached model mapping by DWG model object ID
    ResolvedModelMapping    unresolved (dwgModelId);

    auto    range = m_dwgModelMap.equal_range (unresolved);
    for (auto modelMap = range.first; modelMap != range.second; ++modelMap)
        {
        // a DgnModel exists, check transform & source type!
        if (modelMap->GetTransform().IsEqual(trans) && modelMap->GetMapping().GetSourceType() == sourceType) 
            return *modelMap;
        }

    return  ResolvedModelMapping();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::RemapNameString(Utf8String filename, Utf8StringCR name, Utf8StringCR suffix)
    {
    Utf8String fileBase = filename.substr(0, filename.find_first_of(".")) + suffix;
    return name + " [" + fileBase + "]";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::RemapModelName (Utf8StringCR modelName, BeFileNameCR filePath, Utf8StringCR suffix)
    {
    Utf8String  fileName (filePath.GetFileNameWithoutExtension().c_str());
    size_t      dotAt = fileName.find (".");
    if (dotAt != Utf8String::npos)
        fileName.erase (dotAt);

    return  this->RemapNameString(fileName, modelName, suffix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::_ComputeModelName (DwgDbBlockTableRecordCR block, Utf8CP inSuffix)
    {
    Utf8String  modelName;
    if (BSISUCCESS != DwgHelper::GetLayoutOrBlockName(modelName, block))
        {
        this->ReportError (IssueCategory::Unknown(), Issue::CantCreateModel(), "failed extracting layout/block name!");
        modelName.assign ("Unnamed");
        }

    // get DWG base file name
    BeFileName          baseFilename;
    DwgDbDatabasePtr    dwg = block.GetDatabase ();
    if (dwg.IsNull())
        BeAssert (false && L"null DwgDbDatabasePtr!");
    else
        baseFilename = BeFileName (BeFileName::Basename, dwg->GetFileName().c_str());

    // get either the block name or xref path
    BeFileName  refPath(block.GetPath().c_str());
    DgnClassId  modelType = this->_GetModelType (block);

    return  this->ComputeModelName (modelName, baseFilename, refPath, inSuffix, modelType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::ComputeModelName (Utf8StringR proposedName, BeFileNameCR baseFilename, BeFileNameCR refPath, Utf8CP inSuffix, DgnClassId modelType)
    {
    ImportRule  modelMergeEntry;
    if (BSISUCCESS == this->SearchForMatchingRule(modelMergeEntry, proposedName, baseFilename))
        {
        Utf8String      computedName;
        if (SUCCESS == modelMergeEntry.ComputeNewName(computedName, proposedName, baseFilename))
            proposedName = computedName;
        }

    DgnModels&  models = m_dgndb->Models ();
    models.ReplaceInvalidCharacters (proposedName, models.GetIllegalCharacters(), '_');

    Utf8String  uniqueName(proposedName);
    if (nullptr != inSuffix && inSuffix[0] != 0)
        uniqueName += inSuffix;

    if (m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalModel) != modelType)
        return  uniqueName;

    // unique phisical model names will later handled by PhysicalPartition::CreateUniqueCode

    return uniqueName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId      DwgImporter::_GetModelType (DwgDbBlockTableRecordCR block)
    {
    // spatial models for modelspace and xRef, sheet models for layouts:
    Utf8String  className = (block.IsModelspace() || block.IsExternalReference()) ? BIS_CLASS_PhysicalModel : BIS_CLASS_SheetModel;
    DgnClassId  classId (m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, className.c_str()));

    BeAssert(classId.IsValid());
    return classId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition  DwgImporter::GetModelUnitsFromBlock (double& unitScale, DwgDbBlockTableRecordCR block)
    {
    unitScale = 1.0;

    DwgDbUnits      insUnit = block.GetINSUNITS ();
    if (block.IsLayout() && !block.IsModelspace())
        {
        // this is a layout block - use the units of plot settings:
        DwgDbLayoutPtr  layout (block.GetLayoutId(), DwgDbOpenMode::ForRead);
        if (layout.OpenStatus() == DwgDbStatus::Success)
            {
            DwgDbUnits  paperUnit = layout->GetPlotPaperUnits ();
            if (paperUnit != DwgDbUnits::Undefined)
                insUnit = paperUnit;
            unitScale = layout->IsStandardScale() ? layout->GetStandardScale() : layout->GetCustomScale();
            }
        }

    StandardUnit    blockUnit = insUnit == DwgDbUnits::Undefined ? m_options.GetUnspecifiedBlockUnits() : DwgHelper::GetStandardUnitFromDwgUnit(insUnit);

    UnitDefinition  modelUnit = UnitDefinition::GetStandardUnit (blockUnit);
    if (!modelUnit.IsValid())
        {
        Utf8PrintfString    err("invalid units for model from block %ls - using Meters!", block.GetName().c_str());
        this->ReportError (IssueCategory::MissingData(), Issue::Error(), err.c_str());
        modelUnit = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
        }

    return  modelUnit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::_SetModelUnits (GeometricModel::Formatter& displayInfo, DwgDbBlockTableRecordCR block)
    {
    // set model display info from a block before the model is inserted into the DgbDb
    double          unitScale = 1.0;
    UnitDefinition  modelUnit = this->GetModelUnitsFromBlock (unitScale, block);

    // set linear units and unit format
    UnitDefinition  subUnit = modelUnit;
    DgnUnitFormat   unitFormat = DgnUnitFormat::MU;

    DwgDbDatabasePtr    dwg = block.GetDatabase ();
    if (dwg.IsNull())
        dwg = m_dwgdb;

    DwgDbLUnitFormat    lunits = dwg->GetLUNITS ();
    if ((lunits == DwgDbLUnitFormat::Architectural || lunits == DwgDbLUnitFormat::Engineering) && modelUnit.IsStandardUnit() == StandardUnit::EnglishInches)
        {
        // promote model unit as Feet and keep sub unit as Inches
        modelUnit = UnitDefinition::GetStandardUnit (StandardUnit::EnglishFeet);
        unitFormat = DgnUnitFormat::MUSU;
        }

    displayInfo.SetUnits (modelUnit, subUnit);
    displayInfo.SetLinearUnitMode (unitFormat);

    // set linear unit precision from DWG's luprec & lunits
    PrecisionType   precisionType = PrecisionType::Decimal;
    Byte            precisionByte = 4;
    int16_t         linearPrecision = dwg->GetLUPREC ();
    if (linearPrecision < 0)
        linearPrecision = 0;
    else if (linearPrecision > 8)
        linearPrecision = 8;

    switch (lunits)
        {
        case DwgDbLUnitFormat::Scientific:
            precisionType = PrecisionType::Scientific;
            precisionByte = static_cast<Byte> (linearPrecision);
            break;
        case DwgDbLUnitFormat::Decimal:
        case DwgDbLUnitFormat::Engineering:
            precisionType = PrecisionType::Decimal;
            precisionByte = static_cast<Byte> (linearPrecision);
            break;
        case DwgDbLUnitFormat::Fractional:
        case DwgDbLUnitFormat::Architectural:
            precisionType = PrecisionType::Fractional;
            precisionByte = static_cast<Byte> ((1 << linearPrecision) - 1);
            break;
        }
    displayInfo.SetLinearPrecision (DoubleFormatter::ToPrecisionEnum(precisionType, precisionByte));

    // set angular & directional info:
    int16_t             auprec = dwg->GetAUPREC ();
    AngleMode           angleMode = AngleMode::Degrees;
    AnglePrecision      anglePrecision = AnglePrecision::Whole;
    DirectionMode       directionMode = DirectionMode::Azimuth;

    switch (dwg->GetAUNITS())
        {
        case DwgDbAngularUnits::DecimalDegrees:
            angleMode = AngleMode::Degrees;
            break;
        case DwgDbAngularUnits::DegreesMinutesSeconds:
            anglePrecision = DwgHelper::GetAngularUnits (&angleMode, auprec);
            break;
        case DwgDbAngularUnits::Gradians:
            angleMode = AngleMode::Centesimal;
            break;
        case DwgDbAngularUnits::Radians:
            angleMode = AngleMode::Radians;
            break;
        case DwgDbAngularUnits::Bearing:
            directionMode = DirectionMode::Bearing;
            anglePrecision = DwgHelper::GetAngularUnits (&angleMode, auprec);
            break;
        }
    displayInfo.SetAngularMode (angleMode);
    displayInfo.SetAngularPrecision (anglePrecision);
    displayInfo.SetDirectionMode (directionMode);
    displayInfo.SetDirectionBaseDir (dwg->GetANGBASE() * msGeomConst_degreesPerRadian);
    displayInfo.SetDirectionClockwise (dwg->GetANGDIR());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          DwgImporter::GetScaleToMeters () const
    {
    double  toMeters = 1.0;
    m_rootTransform.IsRigidScale (toMeters);
    return  toMeters;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::SetModelProperties (DgnModelP model, DPoint2dCR snaps)
    {
    GeometricModelP     geometricModel;
    if (nullptr == model || nullptr == (geometricModel = model->ToGeometricModelP()))
        return  BSIERROR;

    double      toMeters = this->GetScaleToMeters ();

    // have a valid viewport - set model disply infor from the viewport:
    DPoint2d    snapDistances = DPoint2d::FromScale (snaps, toMeters);
    double      snapRatio = fabs(snapDistances.x) < 1.0e-8 ? 1.0 : snapDistances.y / snapDistances.x;

    GeometricModel::Formatter&  displayInfo = geometricModel->GetFormatterR ();
    displayInfo.SetRoundoffUnit (snapDistances.x, snapRatio);

    return static_cast<BentleyStatus> (model->Update());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::SetModelPropertiesFromModelspaceViewport (DgnModelP model)
    {
    // some model properties need to be set from a DWG viewport
    DwgDbViewportTableRecordPtr viewport(m_dwgdb->GetActiveModelspaceViewportId(), DwgDbOpenMode::ForRead);
    if (viewport.IsNull())
        {
        // try using the first entry in the viewport table
        DwgDbViewportTablePtr   viewportTable (m_dwgdb->GetViewportTableId(), DwgDbOpenMode::ForRead);
        if (viewportTable.IsNull())
            return  BSIERROR;

        DwgDbSymbolTableIterator    iter = viewportTable->NewIterator ();
        if (!iter.IsValid())
            return  BSIERROR;

        viewport.OpenObject (iter.GetRecordId(), DwgDbOpenMode::ForRead);
        if (viewport.IsNull())
            return  BSIERROR;
        }

    return  this->SetModelProperties(model, viewport->GetSnapIncrements());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::SetModelPropertiesFromLayout (DgnModelP model, DwgDbObjectIdCR layoutId)
    {
    // some model properties need to be set from a DWG viewport
    DwgDbLayoutPtr  layout (layoutId, DwgDbOpenMode::ForRead);
    if (layout.IsNull())
        return  BSIERROR;

    DwgDbObjectIdArray  viewports;
    if (layout->GetViewports(viewports) == 0)
        return  BSIERROR;
    
    // the first in the list is the paperspace viewport:
    DwgDbViewportPtr    viewport (viewports.front(), DwgDbOpenMode::ForRead);
    if (viewport.IsNull())
        return  BSIERROR;

    return  this->SetModelProperties(model, viewport->GetSnapIncrements());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    DwgImporter::CreateModelElement (DwgDbBlockTableRecordCR block, Utf8StringCR modelName, DgnClassId classId)
    {
    Utf8String          descr (block.GetComments().c_str());
    SchemaManagerCR dgndbSchemas = m_dgndb->Schemas ();
    DgnElementId        modelElementId;

    if (dgndbSchemas.GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalModel) == classId)
        {
        // modelspace or xref model
        SubjectCPtr             rootSubject = m_dgndb->Elements().GetRootSubject ();
        DgnCode                 partitionCode = PhysicalPartition::CreateUniqueCode (*rootSubject, modelName.c_str());
        PhysicalPartitionCPtr   partition = PhysicalPartition::CreateAndInsert (*rootSubject, partitionCode.GetValueCP());
        if (partition.IsValid())
            modelElementId = partition->GetElementId();
        else
            this->ReportError (IssueCategory::Unknown(), Issue::CantCreateModel(), modelName.c_str());
        }
    else if (dgndbSchemas.GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SheetModel) == classId)
        {
        // paperspace model
        DocumentListModelPtr    sheetListModel = m_dgndb->Models().Get<DocumentListModel>(m_sheetListModelId);
        if (!sheetListModel.IsValid())
            return modelElementId;

        LayoutFactory   factory (*this, block.GetLayoutId());
        double          scale = factory.GetUserScale ();
        DPoint2d        sheetSize = DPoint2d::From (1.0, 1.0);

        factory.CalculateSheetSize (sheetSize);

        DgnCode             sheetCode = Sheet::Element::CreateUniqueCode(*sheetListModel, modelName.c_str());
        Sheet::ElementPtr   sheet = Sheet::Element::Create (*sheetListModel, scale, sheetSize, sheetCode.GetValueCP());
        if (!sheet.IsValid())
            return modelElementId;

        m_dgndb->Elements().Insert <Sheet::Element> (*sheet);

        if (sheet.IsValid())
            modelElementId = sheet->GetElementId ();
        else
            this->ReportError (IssueCategory::Unknown(), Issue::CantCreateModel(), modelName.c_str());
        }
    else
        {
        BeAssert (false && "Unsupported model type!");
        this->ReportError (IssueCategory::Unsupported(), Issue::CantCreateModel(), modelName.c_str());
        }

    return  modelElementId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId      DwgImporter::CreateModel (DwgDbBlockTableRecordCR block, Utf8CP modelName, DgnClassId classId)
    {
    ModelHandlerP   handler = dgn_ModelHandler::Model::FindHandler(*m_dgndb, classId);
    if (nullptr == handler)
        {
        BeAssert(false);
        ReportError(IssueCategory::Unknown(), Issue::ImportFailure(), "bad model type");
        return DgnModelId();
        }

    DgnElementId    modelElementId = this->CreateModelElement (block, modelName, classId);

    DgnModelPtr model = handler->Create (DgnModel::CreateParams(*m_dgndb, classId, modelElementId, nullptr));
    if (!model.IsValid())
        {
        BeAssert(false);
        ReportError(IssueCategory::Unknown(), Issue::CantCreateModel(), modelName);
        return DgnModelId();
        }

    GeometricModelP geometricModel = model->ToGeometricModelP();
    if (nullptr != geometricModel)
        this->_SetModelUnits (geometricModel->GetFormatterR(), block);

    auto        status = model->Insert ();
    if (status != DgnDbStatus::Success)
        {
        BeAssert(false);
        ReportError(IssueCategory::Unknown(), Issue::ImportFailure(), Utf8String(block.GetDatabase()->GetFileName().c_str()).c_str());
        return DgnModelId();
        }

    return model->GetModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::ResolvedModelMapping DwgImporter::GetOrCreateModelFromBlock (DwgDbBlockTableRecordCR dwgBlock, TransformCR trans, DwgDbBlockReferenceCP xrefInsert, DwgDbDatabaseP xrefDwg)
    {
    /*-----------------------------------------------------------------------------------
    This method either gets an existing DgnModel or creates a new one for a modelspace, a
    paperspace, or an xRef attachment, with required transformation.
    -----------------------------------------------------------------------------------*/
    ResolvedModelMapping            modelMap;
    DwgSyncInfo::ModelSourceType    sourceType = xrefInsert == nullptr ? DwgSyncInfo::ModelSourceType::ModelOrPaperSpace : DwgSyncInfo::ModelSourceType::XRefAttachment;
    if (DwgSyncInfo::ModelSourceType::XRefAttachment == sourceType && nullptr == xrefDwg)
        {
        BeAssert (false && "an xRef is not loaded!");
        return  modelMap;
        }

    /*-----------------------------------------------------------------------------------
    A ResolvedModelMapping uses a model instance ID as the key - for modelspace or paperspace, 
    it is the block ID; for an xref or raster attachment, it is the entity ID.
    -----------------------------------------------------------------------------------*/
    DwgDbObjectId   dwgModelId = xrefInsert == nullptr ? dwgBlock.GetObjectId() : xrefInsert->GetObjectId();
    DwgDbObjectCP   dwgModel = xrefInsert == nullptr ? DwgDbObject::Cast(&dwgBlock) : DwgDbObject::Cast(xrefInsert);
    if (nullptr == dwgModel || !dwgModelId.IsValid())
        {
        BeAssert (false && "unsupported DWG model type!");
        return  modelMap;
        }

    if (this->_IsUpdating())
        {
        if (DwgSyncInfo::ModelSourceType::XRefAttachment == sourceType)
            modelMap = this->GetModelFromSyncInfo (dwgModelId, *xrefDwg, trans);
        else
            modelMap = this->GetModelFromSyncInfo (dwgModelId, this->GetDwgDb(), trans);

        if (modelMap.IsValid())
            {
            // add the model to cache if not already added(need the check for a multiset):
            if (!this->FindModel(dwgModelId, trans, sourceType).IsValid())
                this->AddToDwgModelMap (modelMap);
            // tell the updater about the coming DWG model object:
            this->_OnModelSeen (modelMap);
            return  modelMap;
            }
        // not found in syncinfo - may have to create a new one.
        }

    // see if this model has already been created
    modelMap = this->FindModel (dwgModelId, trans, sourceType);
    if (modelMap.IsValid())
        return  modelMap;

    // if a known insert entity is given, use its handle value as model suffix:
    Utf8CP      suffix = nullptr;
    Utf8String  idSuffix;
    if (nullptr != xrefInsert)
        {
        idSuffix.Sprintf ("(%llx)", xrefInsert->GetObjectId().ToUInt64());
        suffix = idSuffix.c_str ();
        }
    
    // create a new model
    Utf8String  modelName = this->_ComputeModelName(dwgBlock, suffix).c_str ();
    DgnClassId  modelType = this->_GetModelType (dwgBlock);
    DgnModelId  modelId = this->CreateModel (dwgBlock, modelName.c_str(), modelType);
    if (!modelId.IsValid())
        return nullptr;

    DgnModelP   model = m_dgndb->Models().GetModel(modelId).get ();
    if (nullptr == model)
        {
        this->ReportError (IssueCategory::Unknown(), Issue::ImportFailure(), IssueReporter::FmtModel(dwgBlock).c_str());
        this->OnFatalError ();
        return modelMap;
        }

    // save the model info to syncInfo:
    DwgSyncInfo::DwgModelMapping    mapping;
    BentleyStatus                   status = BSISUCCESS;
    if (nullptr == xrefInsert)
        status = m_syncInfo.InsertModel (mapping, modelId, dwgBlock, trans);
    else
        status = m_syncInfo.InsertModel (mapping, modelId, *xrefInsert, *xrefDwg, trans);
    BeAssert(status==BSISUCCESS);

    modelMap.SetModelInstanceId (dwgModelId);
    modelMap.SetModel (model);
    modelMap.SetMapping (mapping);

    if (LOG_MODEL_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
        LOG_MODEL.tracev("+ %s %d -> %s %d", mapping.GetDwgName().c_str(), mapping.GetDwgModelId().GetValue(), model->GetName().c_str(), model->GetModelId().GetValue());

    // save in our list of known model mappings.
    this->AddToDwgModelMap (modelMap);
    // tell the updater about the newly discovered model
    this->_OnModelInserted (modelMap);

    return  modelMap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::ScaleModelTransformBy (TransformR trans, DwgDbBlockTableRecordCR block)
    {
    double          unitScale = 1.0;
    UnitDefinition  modelUnits = this->GetModelUnitsFromBlock (unitScale, block);
    double          toMeters = modelUnits.ToMeters() * unitScale;
    trans.ScaleMatrixColumns (toMeters, toMeters, toMeters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::CompoundModelTransformBy (TransformR trans, DwgDbBlockReferenceCR insert)
    {
    Transform   insertTrans;
    insert.GetBlockTransform (insertTrans);
    trans.InitProduct (trans, insertTrans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportDwgModels ()
    {
    // Create models from layout and xRef block definitions
    DwgDbBlockTablePtr  blockTable (m_dwgdb->GetBlockTableId(), DwgDbOpenMode::ForRead);
    if (DwgDbStatus::Success != blockTable.OpenStatus())
        return  BSIERROR;

    DwgDbSymbolTableIterator    iter = blockTable->NewIterator ();

    if (!iter.IsValid())
        return  BSIERROR;

    // create the root model from modelspace block first
    if (m_modelspaceId.IsNull())
        m_modelspaceId = blockTable->GetModelspaceId ();

    ResolvedModelMapping    modelMap = this->FindModel(m_modelspaceId, m_rootTransform, DwgSyncInfo::ModelSourceType::ModelOrPaperSpace);
    if (!modelMap.IsValid())
        {
        DwgDbBlockTableRecordPtr    block(m_modelspaceId, DwgDbOpenMode::ForRead);
        if (block.IsNull())
            {
            this->ReportError (IssueCategory::Unknown(), Issue::CannotOpenModelspace(), "when creating the root model!");
            this->OnFatalError ();
            return BSIERROR;
            }

        // set root model transformation from modelspace block
        this->ScaleModelTransformBy (m_rootTransform, *block.get());

        modelMap = this->GetOrCreateModelFromBlock (*block.get(), m_rootTransform);
        if (!modelMap.IsValid())
            this->ReportError (IssueCategory::MissingData(), Issue::CantCreateModel(), IssueReporter::FmtModel(*block).c_str());
        else
            this->SetModelPropertiesFromModelspaceViewport (modelMap.GetModel());
        }

    ECSchemaPtr     attrdefSchema;

    // walk through all blocks and create models for paperspace and xref blocks:
    for (iter.Start(); !iter.Done(); iter.Step())
        {
        DwgDbObjectId   blockId = iter.GetRecordId ();
        if (!blockId.IsValid() || blockId == m_modelspaceId)
            continue;

        DwgDbBlockTableRecordPtr    block(blockId, DwgDbOpenMode::ForRead);
        if (block.IsNull())
            continue;

        DwgString       name = block.get()->GetName ();
        if (name.IsEmpty())
            {
            this->ReportIssueV (IssueSeverity::Info, IssueCategory::UnexpectedData(), Issue::ConfigUsingDefault(), IssueReporter::FmtModel(*block).c_str());
            name = L"Default";
            }

        // create layout models
        if (block->IsLayout())
            {
            LOG_MODEL.tracev("Creating DgnModel from DWG layout block %ls", name.c_str());

            // Units of paperspace models are from plot settings.
            Transform   layoutTransform = Transform::FromIdentity ();
            this->ScaleModelTransformBy (layoutTransform, *block.get());

            modelMap= this->GetOrCreateModelFromBlock (*block.get(), layoutTransform);
            if (!modelMap.IsValid() || modelMap.GetModel() == nullptr)
                this->ReportError (IssueCategory::MissingData(), Issue::CantCreateModel(), IssueReporter::FmtModel(*block).c_str());
            else
                this->SetModelPropertiesFromLayout (modelMap.GetModel(), block->GetLayoutId());
            continue;
            }

        // create xref models
        if (block->IsExternalReference() && !block->IsOverlayReference())
            {
            // load the DWG xRef file:
            DwgXRefHolder   xref(*block.get(), *this);
            if (xref.IsValid())
                {
                // add the new xref in local cache as well as in the syncInfo:
                m_loadedXrefFiles.push_back (xref);
                this->_AddFileInSyncInfo (xref.GetDatabase(), this->_GetDwgFileIdPolicy(*xref.GetDatabase()));
                }
            else
                {
                // can't load the xRef file - error out
                Utf8String  filename = xref.GetPath().GetNameUtf8 ();
                if (filename.empty())
                    filename = Utf8String (name.c_str());
                this->ReportError (IssueCategory::DiskIO(), Issue::FileNotFound(), filename.c_str());
                continue;
                }

            DwgDbObjectIdArray  ids;
            if (DwgDbStatus::Success != block->GetBlockReferenceIds(ids))
                {
                /*------------------------------------------------------------------------------------------------
                There could be legitimate reasons as well as file errors that no instances found for an xref block.
                A nested xref would not have instances in the outermost master file for example.  Whatever has led
                us here is not important - we will not add a model for an xref block that has no instances.  We will
                create a model when we see an xref attachment while importing entities.
                ------------------------------------------------------------------------------------------------*/
                continue;
                }

            LOG_MODEL.tracev("Creating DgnModel from DWG xRef block %ls", name.c_str());

            // the xref block has instances - create models for each of them:
            for (auto const& id : ids)
                {
                DwgDbBlockReferencePtr  insert(id, DwgDbOpenMode::ForRead);
                if (!insert.IsNull())
                    {
                    // open & check parent of this insert entity:
                    DwgDbObjectId               parentId = insert->GetOwnerId ();
                    DwgDbBlockTableRecordPtr    parentBlock (parentId, DwgDbOpenMode::ForRead);
                    if (parentBlock.OpenStatus() != DwgDbStatus::Success)
                        {
                        this->ReportError (IssueCategory::MissingData(), Issue::CantOpenObject(), Utf8PrintfString("the owner block of an xRef %ls", name.c_str()).c_str());
                        continue;
                        }

                    Transform   xtrans = m_rootTransform;
                    DgnModelP   parentModel = nullptr;

                    // if this xref is inserted in a paperspace block, import paperspace first, then use paperspace transform:
                    bool    isParentPaperspace = parentBlock->IsLayout() && m_modelspaceId != parentId;
                    if (isParentPaperspace)
                        {
                        // get or create the paperspace model if not already created:
                        this->ScaleModelTransformBy (xtrans, *parentBlock.get());

                        modelMap = this->GetOrCreateModelFromBlock (*parentBlock.get(), xtrans);
                        if (!modelMap.IsValid() || (parentModel = modelMap.GetModel()) == nullptr)
                            {
                            this->ReportError (IssueCategory::MissingData(), Issue::CantCreateModel(), IssueReporter::FmtModel(*parentBlock).c_str());
                            continue;
                            }
                        // we now have a paperspace transform for current xref instance...
                        }

                    this->CompoundModelTransformBy (xtrans, *insert.get());

                    modelMap = this->GetOrCreateModelFromBlock (*block.get(), xtrans, insert.get(), xref.GetDatabase());

                    DgnModelP   model = nullptr;
                    if (!modelMap.IsValid() || (model = modelMap.GetModel()) == nullptr)
                        {
                        this->ReportError (IssueCategory::MissingData(), Issue::CantCreateModel(), IssueReporter::FmtModel(*block).c_str());
                        continue;
                        }

                    xref.AddDgnModelId (model->GetModelId());

                    // give the updater a chance to cache skipped models as we may not see xref inserts during importing phapse:
                    this->_ShouldSkipModel (modelMap);

                    // if the instance is in the modelspace, add it to modelspace xref list:
                    if (m_modelspaceId == parentId)
                        {
                        m_modelspaceXrefs.insert (model->GetModelId());
                        continue;
                        }

                    // if the instance is in a paperspace, add the info into paperspace xref list:
                    if (isParentPaperspace && nullptr != parentModel)
                        {
                        m_paperspaceXrefs.push_back (DwgXRefInPaperspace(id, parentId, parentModel->GetModelId()));
                        continue;
                        }

                    parentBlock->Close ();

                    // the instance is neither in the modelspace nor in a paperspace - drop it into the parent block:
                    if (DwgDbStatus::Success != insert->ExplodeToOwnerSpace())
                        this->ReportError (IssueCategory::Unknown(), Issue::Error(), Utf8PrintfString("failed dropping nested xRef %ls", name.c_str()).c_str());
                    }
                }
            continue;
            }

        // create an attrdef ECClass from the block:
        if (block->HasAttributeDefinitions())
            this->AddAttrdefECClassFromBlock(attrdefSchema, *block.get());
        }
    
    if (attrdefSchema.IsValid() && attrdefSchema->GetClassCount() > 0)
        this->ImportAttributeDefinitionSchema (*attrdefSchema);

    m_dgndb->SaveSettings ();
    m_dgndb->SaveChanges ();
    m_syncInfo.SetValid (true);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportSpaces ()
    {
    // DWG global origin is at 0,0,0
    DPoint3d    globalOrigin = DPoint3d::From (0.0, 0.0, 0.0);
    m_dgndb->GeoLocation().SetGlobalOrigin (globalOrigin);
    m_dgndb->GeoLocation().Save ();

    // save off effective DWG file units
    m_modelspaceUnits = DwgHelper::GetStandardUnitFromDwgUnit (m_dwgdb->GetINSUNITS());
    if (m_modelspaceUnits == StandardUnit::None)
        m_modelspaceUnits = m_options.GetUnspecifiedBlockUnits ();

    // add current DWG file into syncinfo:
    StableIdPolicy  policy = this->_GetDwgFileIdPolicy (*m_dwgdb);
    this->_AddFileInSyncInfo (m_dwgdb.get(), policy);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::_AddDefaultRootGuestSyncInfo ()
    {
    DwgSyncInfo::DwgModelSource rootSource(m_modelspaceId);
    DwgSyncInfo::ImportJob          importJob(rootSource, DwgSyncInfo::ImportJob::Type::RootModels);

    m_syncInfo.InsertImportJob(importJob);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::_FinishImport ()
    {
    _PostProcessViewports ();
    _EmbedFonts ();
    _AddDefaultRootGuestSyncInfo ();

    m_dgndb->GeoLocation().InitializeProjectExtents();

    if (m_defaultViewId.IsValid() && _IsCreatingNewDgnDb())
        GetDgnDb().SaveProperty(DgnViewProperty::DefaultView(), &m_defaultViewId, sizeof(m_defaultViewId));

    // WIP - thumbnails
    // GenerateThumbnails ();

    if (WasAborted())
        {
        m_dgndb->AbandonChanges();
        return ;
        }

    m_dgndb->SaveSettings ();

    auto rc = m_dgndb->SaveChanges();
    if (BE_SQLITE_OK != rc)
        {
        ReportIssueV(IssueSeverity::Fatal, IssueCategory::DiskIO(), Issue::SaveError(), nullptr, m_dgndb->GetLastError().c_str());
        m_dgndb->AbandonChanges();
        return ;
        }

    if (m_config.GetOptionValueBool("CompactDatabase", true))
        {
        SetStepName(ProgressMessage::STEP_COMPACTING());
        m_dgndb->CompactFile();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::Process ()
    {
    StopWatch totalTimer (true);
    StopWatch timer (true);

    if (m_dwgdb.IsNull() || m_dgndb.IsNull())
        {
        BeAssert (false && L"Both DwgDbDatabase and DgnDb must be created prior to import or update DWG!");
        return  BSIERROR;
        }

    m_modelspaceId = m_dwgdb->GetModelspaceId ();

    this->_BeginImport ();
    if (this->WasAborted())
        return BSIERROR;

    this->_ImportSpaces ();
    DwgImportLogging::LogPerformance(timer, "Create Spaces");

    timer.Start();
    this->_ImportDwgModels ();
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Create Models");

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
DwgImporter::DwgImporter (DwgImporter::Options& options) : m_syncInfo(*this), m_config(options.GetConfigFile(), *this), m_dgndb(), m_dwgdb(), m_loadedFonts(*this), m_issueReporter(options.GetReportFile())
    {
    m_wasAborted = false;
    m_options = options;
    m_rootFileName.Clear ();
    m_rootTransform.InitIdentity ();
    m_fileCount = 0;
    m_wasAborted = false;
    m_currIdPolicy = StableIdPolicy::ById;
    m_modelspaceXrefs.clear ();
    m_paperspaceXrefs.clear ();
    m_importedTextstyles.clear ();
    m_importedLinestyles.clear ();
    m_importedMaterials.clear ();
    m_sharedGeometryPartList.clear ();
    m_entitiesImported = 0;
    m_attributeDefinitionSchema = 0;
    m_constantBlockAttrdefList.clear ();
    m_modelspaceUnits = StandardUnit::None;
    m_modelspaceId.SetNull ();
    m_currentspaceId.SetNull ();
    m_dwgModelMap.clear ();
    m_isProcessingDwgModelMap = false;

    // read config file, if supplied
    if (!m_config.GetInstanceFilename().empty())
        m_config.ReadFromXmlFile();

    T_Utf8StringVector  userObjectEnablers;
    this->ParseConfigurationFile (userObjectEnablers);

    DwgImportHost::GetHost().Initialize (*this);
    DwgImportHost::GetHost().NewProgressMeter ();
    DwgImporter::RegisterProtocalExtensions ();

    // try loading user object enablers if specified
    for (auto& oe : userObjectEnablers)
        DwgImportHost::GetHost().LoadObjectEnabler (BeFileName(oe.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::~DwgImporter ()
    {
    if (m_issueReporter.HasIssues())
        m_issueReporter.CloseReport ();

    if (nullptr != m_options.GetProgressMeter())
        DgnPlatformLib::QueryHost()->SetProgressMeter (nullptr);

    m_modelspaceXrefs.clear ();
    m_paperspaceXrefs.clear ();
    m_paperspaceViews.clear ();
    m_loadedXrefFiles.clear ();
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
uint32_t        DwgImporter::Options::GetDgnLineWeight (DwgDbLineWeight dwgWeight)
    {
    auto    found = m_lineweightMapping.find (dwgWeight);
    if (found != m_lineweightMapping.end())
        return  found->second;

    return  0;
    }

