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
void   DwgImporter::GetOrCreateJobPartitions ()
    {
    this->InitUncategorizedCategory ();
    this->InitBusinessKeyCodeSpec ();
    this->InitSheetListModel ();
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
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode         DwgImporter::CreateCode (Utf8StringCR value) const
    {
    auto auth = m_dgndb->CodeSpecs().GetCodeSpec(m_businessKeyCodeSpecId);
    BeDataAssert(auth.IsValid());
    return auth.IsValid() ? auth->CreateCode(GetImportJob().GetSubject(), value) : DgnCode();
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

    m_dgndb->SaveChanges ();

    BeAssert(m_syncInfo.IsValid());
    BeAssert(!WasAborted());

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgFileId  DwgImporter::_AddFileInSyncInfo (DwgDbDatabaseR dwg, StableIdPolicy policy)
    {
    DwgSyncInfo::DwgFileId  fileId;

    // create file provenance and insert it into the sync info
    DwgSyncInfo::FileProvenance provenance(dwg, m_syncInfo, policy);
    if (!provenance.FindByName(true))
        {
        provenance.Insert();

        if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
            LOG.tracev("+ %s => %lld", DwgHelper::ToUtf8CP(dwg.GetFileName()), provenance.GetDwgFileId().GetValue());
        }

    fileId = provenance.GetDwgFileId ();
    policy = provenance.GetIdPolicy ();

    // set file id & policy in DwgDbSummaryInfo
    dwg.SetFileIdPolicy (fileId.GetValue(), static_cast<uint32_t>(policy));

    return  fileId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSyncInfo::DwgFileId  DwgImporter::GetDwgFileId (DwgDbDatabaseR dwg, bool setIfNotExist)
    {
    DwgSyncInfo::DwgFileId  fileId = DwgSyncInfo::GetDwgFileId (dwg);
    if (!fileId.IsValid() && setIfNotExist)
        {
        fileId = this->_AddFileInSyncInfo (dwg, this->_GetDwgFileIdPolicy());
        BeAssert (fileId.IsValid() && "Unable to set/get DwgFileId from DWG file!");
        }
    return  fileId;
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
    DwgFileVersion  dwgVersion = DwgFileVersion::Invalid;
    if (!DwgHelper::SniffDwgFile(dwgdxfName, &dwgVersion) && (DwgFileVersion::Newer == dwgVersion || !DwgHelper::SniffDxfFile(dwgdxfName, &dwgVersion)))
        {
        this->ReportError (IssueCategory::DiskIO(), DwgFileVersion::Newer == dwgVersion ? Issue::NewerDwgVersion() : Issue::NotRecognizedFormat(), dwgdxfName.c_str());
        return  static_cast<BentleyStatus> (DwgFileVersion::Newer == dwgVersion ? DgnDbStatus::VersionTooNew : DgnDbStatus::NotOpen);
        }
    
    // load fonts before we start reading the DWG file as the toolkit may search for them via the hostApp:
    m_loadedFonts.LoadFonts ();

    WString  password;
    password.AppendUtf8 (this->GetOptions().GetPassword().c_str());

    Utf8String  dispVersion = DwgHelper::GetStringFromDwgVersion (dwgVersion);
    this->SetStepName (ProgressMessage::STEP_OPENINGFILE(), dwgdxfName.c_str(), dispVersion.c_str());

    m_dwgdb = DwgImportHost::GetHost().ReadFile (dwgdxfName, false, false, FileShareMode::DenyNo, password);
    if (!m_dwgdb.IsValid())
        return  BSIERROR;

    m_rootFileName = dwgdxfName;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping    DwgImporter::GetModelFromSyncInfo (DwgDbObjectIdCR id, DwgDbDatabaseR dwg, TransformCR trans)
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
ResolvedModelMapping   DwgImporter::FindModel (DwgDbObjectIdCR dwgModelId, TransformCR trans, DwgSyncInfo::ModelSourceType sourceType)
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
        PhysicalPartitionCPtr   partition = PhysicalPartition::CreateAndInsert (*rootSubject, partitionCode.GetValueUtf8CP());
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
        Sheet::ElementPtr   sheet = Sheet::Element::Create (*sheetListModel, scale, sheetSize, sheetCode.GetValueUtf8CP());
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
ResolvedModelMapping DwgImporter::GetOrCreateModelFromBlock (DwgDbBlockTableRecordCR dwgBlock, TransformCR trans, DwgDbBlockReferenceCP xrefInsert, DwgDbDatabaseP xrefDwg)
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

    IDwgChangeDetector& changeDetector = this->_GetChangeDetector ();
    if (this->IsUpdating())
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
            changeDetector._OnModelSeen (*this, modelMap);
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
    changeDetector._OnModelInserted (*this, modelMap, xrefDwg);

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
ResolvedModelMapping    DwgImporter::GetOrCreateRootModel ()
    {
    if (m_dwgdb.IsNull())
        {
        BeAssert (false && "DWG file is not opened yet!");
        return  ResolvedModelMapping();
        }

    if (m_rootDwgModelMap.IsValid())
        return  m_rootDwgModelMap;
        
    if (m_modelspaceId.IsNull())
        m_modelspaceId = m_dwgdb->GetModelspaceId ();

    m_rootDwgModelMap = this->FindModel(m_modelspaceId, m_rootTransform, DwgSyncInfo::ModelSourceType::ModelOrPaperSpace);
    if (!m_rootDwgModelMap.IsValid())
        {
        DwgDbBlockTableRecordPtr    block(m_modelspaceId, DwgDbOpenMode::ForRead);
        if (block.IsNull())
            {
            this->ReportError (IssueCategory::Unknown(), Issue::CannotOpenModelspace(), "when creating the root model!");
            this->OnFatalError ();
            return m_rootDwgModelMap;
            }

        // set root model transformation from modelspace block
        this->ScaleModelTransformBy (m_rootTransform, *block.get());

        m_rootDwgModelMap = this->GetOrCreateModelFromBlock (*block.get(), m_rootTransform);
        if (!m_rootDwgModelMap.IsValid())
            this->ReportError (IssueCategory::MissingData(), Issue::CantCreateModel(), IssueReporter::FmtModel(*block).c_str());
        else
            this->SetModelPropertiesFromModelspaceViewport (m_rootDwgModelMap.GetModel());
        }

    return m_rootDwgModelMap;
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
    ResolvedModelMapping    modelMap = this->GetOrCreateRootModel ();
    if (!modelMap.IsValid())
        return  BSIERROR;

    IDwgChangeDetector&     changeDetector = this->_GetChangeDetector ();
    bool        hasPushedReferencesSubject = false;
    SubjectCPtr parentSubject = this->GetSpatialParentSubject ();
    BeAssert (parentSubject.IsValid() && "parent subject for spatial models not set yet!!");

    // will collect attribute definitions from regular blocks
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
                this->_AddFileInSyncInfo (xref.GetDatabaseR(), this->_GetDwgFileIdPolicy());
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

                    modelMap = this->GetOrCreateModelFromBlock (*block.get(), xtrans, insert.get(), xref.GetDatabaseP());

                    DgnModelP   model = nullptr;
                    if (!modelMap.IsValid() || (model = modelMap.GetModel()) == nullptr)
                        {
                        this->ReportError (IssueCategory::MissingData(), Issue::CantCreateModel(), IssueReporter::FmtModel(*block).c_str());
                        continue;
                        }

                    xref.AddDgnModelId (model->GetModelId());

                    // give the updater a chance to cache skipped models as we may not see xref inserts during importing phapse:
                    changeDetector._ShouldSkipModel (*this, modelMap);

                    if (!hasPushedReferencesSubject && parentSubject.IsValid())
                        {
                        SubjectCPtr refsSubject = this->GetOrCreateModelSubject (*parentSubject, model->GetName(), ModelSubjectType::References);
                        if (refsSubject.IsValid())
                            {
                            // set the root model subject as spatial parent
                            this->SetSpatialParentSubject (*refsSubject);
                            hasPushedReferencesSubject = true;
                            }
                        else
                            {
                            ReportError (IssueCategory::Unsupported(), Issue::Error(), Utf8PrintfString("Failed to create references subject. Parent[%s]. XRef[%s].", IssueReporter::FmtModel(*parentBlock).c_str(), IssueReporter::FmtXReference(*block).c_str()).c_str());
                            BeAssert (false && "Failed creating xrefrences subject!");
                            }
                        }

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

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr DwgImporter::GetOrCreateModelSubject (SubjectCR parent, Utf8StringCR modelName, ModelSubjectType stype)
    {
    Json::Value modelProps(Json::nullValue);
    modelProps["Type"] = (ModelSubjectType::Hierarchy==stype)? "Hierarchy": "References";

    for (auto childid : parent.QueryChildren())
        {
        auto subj = GetDgnDb().Elements().Get<Subject>(childid);
        if (subj.IsValid() && subj->GetCode().GetValue().Equals(modelName.c_str()) && (modelProps == subj->GetSubjectJsonProperties().GetMember(Subject::json_Model())))
            return subj;
        }

    BeAssert((!IsUpdating() || (ModelSubjectType::Hierarchy != stype)) && "You create a hierarchy subject once when you create the job");

    SubjectPtr ed = Subject::Create(parent, modelName.c_str());

    ed->SetSubjectJsonProperties(Subject::json_Model(), modelProps);

    return ed->InsertT<Subject>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr DwgImporter::GetJobHierarchySubject ()
    {
    auto const& jobsubj = GetJobSubject();
    auto childids = jobsubj.QueryChildren();
    for (auto childid : childids)
        {
        auto subj = GetDgnDb().Elements().Get<Subject>(childid);
        if (subj.IsValid() && subj->GetSubjectJsonProperties(Subject::json_Model()).GetMember("Type") == "Hierarchy")
            return subj;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgImporter::ValidateJob ()
    {
    auto const& jobsubj = GetJobSubject();
    if (!jobsubj.GetElementId().IsValid())
        {
        BeAssert(false && "job subject must be persistent in the BIM");
        _OnFatalError();
        return;
        }
    auto jchildids = jobsubj.QueryChildren();
    auto hcount = 0;
    for (auto jchildid : jchildids)
        {
        auto subj = GetDgnDb().Elements().Get<Subject>(jchildid);
        if (subj.IsValid() && subj->GetSubjectJsonProperties(Subject::json_Model()).GetMember("Type") == "Hierarchy")
            ++hcount;
        }
    if (hcount != 1)
        {
        BeAssert(false && "there should be exactly 1 job hierarchy subject under the job subject");
        _OnFatalError();
        return;
        }

    auto hchildids = jobsubj.QueryChildren();
    auto rcount = 0;
    for (auto hchildid : hchildids)
        {
        auto subj = GetDgnDb().Elements().Get<Subject>(hchildid);
        if (subj.IsValid() && subj->GetSubjectJsonProperties(Subject::json_Model()).GetMember("Type") == "References")
            ++rcount;
        }
    if ((rcount != 0) && (rcount != 1))
        {
        BeAssert(false && "there should be 0 or 1 references subject under the hierarchy subject");
        _OnFatalError();
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgImporter::ComputeDefaultImportJobName (Utf8StringCR rootModelName)
    {
    DgnElements&    bimElements = this->GetDgnDb().Elements ();

    Utf8String  rootFileName (BeFileName::GetFileNameWithoutExtension(this->GetRootDwgFileName()).c_str());
    size_t      dotAt = rootFileName.find (".");
    if (dotAt != Utf8String::npos)
        rootFileName.erase(dotAt);

    Utf8String  jobName = iModelBridge::str_BridgeType_DWG() + Utf8String(":") + rootFileName + Utf8String(", ") + rootModelName;
    DgnCode     code = Subject::CreateCode (*bimElements.GetRootSubject(), jobName.c_str());

    // create a unique job name
    size_t  count = 0;
    while (bimElements.QueryElementIdByCode(code).IsValid())
        {
        Utf8String  uniqueJobName(jobName);
        uniqueJobName.append (Utf8PrintfString("%d", ++count).c_str());
        code = Subject::CreateCode (*bimElements.GetRootSubject(), uniqueJobName.c_str());
        }

    m_options.SetBridgeJobName (jobName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::ImportJobCreateStatus   DwgImporter::InitializeJob (Utf8CP comments, DwgSyncInfo::ImportJob::Type jobType)
    {
    // will create a new ImportJob - don't call this if updating!
    if (this->IsUpdating())
        return ImportJobCreateStatus::FailedExistingRoot;

    if (!m_dwgdb.IsValid())
        {
        BeAssert (false && "Root DWG file not open!");
        return ImportJobCreateStatus::FailedInsertFailure;
        }

    // get the modelspace name
    DwgDbBlockTableRecordPtr    modelspaceBlock(m_dwgdb->GetModelspaceId(), DwgDbOpenMode::ForRead);
    if (DwgDbStatus::Success != modelspaceBlock.OpenStatus())
        {
        BeAssert (false && "The ModelSpace block not available the import job!");
        return ImportJobCreateStatus::FailedInsertFailure;
        }

    Utf8String  modelspaceName(modelspaceBlock->GetName().c_str());
    if (modelspaceName.empty())
        modelspaceName.assign ("Model");

    Utf8String  jobName = this->GetOptions().GetBridgeJobName ();
    if (jobName.empty())
        {
        this->ComputeDefaultImportJobName (modelspaceName);
        jobName = this->GetOptions().GetBridgeJobName();
        }
    else
        {
        if (!jobName.StartsWithI(iModelBridge::str_BridgeType_DWG()))
            {
            jobName = iModelBridge::str_BridgeType_DWG();
            jobName.append(":");
            jobName.append(this->GetOptions().GetBridgeJobName());
            }
        }
    BeAssert (!jobName.empty() && "Bridge job not defined!");

    if (this->FindSoleImportJobForFile(this->GetDwgDb()).IsValid())
        return ImportJobCreateStatus::FailedExistingRoot;

    // 1. Map in the root file.
    auto    fileId = this->GetDwgFileId (this->GetDwgDb());
    // NB! file might already be in syncinfo! The logic that tries to detect an existing Job puts it there!
    DwgSyncInfo::FileById   syncInfoFiles (this->GetDgnDb(), fileId);
    DwgSyncInfo::FileIterator::Entry syncInfoFile = syncInfoFiles.begin();
    if (syncInfoFile == syncInfoFiles.end())
        {
        BeAssert(false);
        return ImportJobCreateStatus::FailedExistingNonRootModel;
        }

    // set default change detector (no-op):
    this->_SetChangeDetector (false);

    // 2. Create a subject, as a child of the rootsubject
    SubjectPtr  newSubject = Subject::Create (*this->GetDgnDb().Elements().GetRootSubject(), jobName);
    if (!newSubject.IsValid())
        return ImportJobCreateStatus::FailedInsertFailure;

    Json::Value dwgJobProps(Json::objectValue);
    dwgJobProps["ImporterType"] = (int)jobType;
    dwgJobProps["NamePrefix"] = this->GetImportJobNamePrefix ();
    dwgJobProps["DwgFile"] = syncInfoFile.GetUniqueName();

    Json::Value jobProps(Json::objectValue);
    if (!Utf8String::IsNullOrEmpty(comments))
        jobProps["Comments"] = comments;
    jobProps[iModelBridge::str_BridgeType_DWG()] = dwgJobProps;

    newSubject->SetSubjectJsonProperties(Subject::json_Job(), jobProps);

    SubjectCPtr jobSubject = newSubject->InsertT<Subject>();
    if (!jobSubject.IsValid())
        return ImportJobCreateStatus::FailedInsertFailure;

    // 3. Set up m_importJob with the subject. That leaves out the syncinfo part, but we don't need that yet. 
    //      We do need m_importJob to be defined and it must have its subject at this point, as GetOrCreateJobPartitions 
    //      and GetModelForDgnV8Model refer to the subject in it.
    m_importJob = ResolvedImportJob (*jobSubject);

    // 4. Create the job-specific stuff in the DgnDb (relative to the job subject).
    this->GetOrCreateJobPartitions ();

    //  ... and create and push the root model's "hierarchy" subject. The root model's physical partition will be a child of that.
    Utf8String  mastermodelName = this->_ComputeModelName (*modelspaceBlock.get());
    m_spatialParentSubject = this->GetOrCreateModelSubject(this->GetJobSubject(), mastermodelName, ModelSubjectType::Hierarchy);
    if (!m_spatialParentSubject.IsValid())
        {
        BeAssert (false && "Failed creating parent model subject");
        return ImportJobCreateStatus::FailedInsertFailure;
        }

    // 5. Map the root model into the DgnDb. Note that this will generally create a partition, which is relative to the job subject,
    //    So, the job subject and its framework must be created first.
    this->GetOrCreateRootModel ();
    if (!m_rootDwgModelMap.IsValid())
        {
        BeAssert (false && "No root DWG model!");
        return ImportJobCreateStatus::FailedInsertFailure;
        }

    // 6. Now that we have the root model's syncinfo id, we can define the syncinfo part of the importjob.
    DwgSyncInfo::ImportJob  importJob;
    importJob.SetDwgModelSyncInfoId (m_rootDwgModelMap.GetModelSyncInfoId());
    importJob.SetPrefix (this->GetImportJobNamePrefix());
    importJob.SetType (jobType);
    importJob.SetSubjectId (jobSubject->GetElementId());
    if (BSISUCCESS != this->GetSyncInfo().InsertImportJob(importJob))
        return ImportJobCreateStatus::FailedExistingRoot;

    m_importJob.GetImportJob() = importJob;     // Update the syncinfo part of the import job

    return ImportJobCreateStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::ImportJobLoadStatus DwgImporter::FindJob ()
    {
    // there is only one root model for a DWG file.
    m_importJob = this->FindSoleImportJobForFile (this->GetDwgDb());
    if (!m_importJob.IsValid())
        return ImportJobLoadStatus::FailedNotFound;

    // *** TRICKY: If this is called by the framework as a check *after* it calls _IntializeJob, then don't change the change detector!
    if (IsUpdating())
        this->_SetChangeDetector (true);

    this->GetDwgFileId (this->GetDwgDb(), true);
    this->GetOrCreateJobPartitions ();
    this->GetOrCreateRootModel ();
    this->CheckSameRootModelAndUnits ();

    // There's only one hierarchy subject for a job. Look it up.
    auto found = this->GetJobHierarchySubject ();
    if (found.IsValid())
        this->SetSpatialParentSubject (*found);
    else
        return ImportJobLoadStatus::FailedNotFound;

    return ImportJobLoadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::_BeginImport ()
    {
    this->_GetChangeDetector()._Prepare (*this);
    m_dgndb->AddIssueListener (m_issueReporter.GetECDbIssueListener());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::_FinishImport ()
    {
    _PostProcessViewports ();
    _EmbedFonts ();

    m_dgndb->GeoLocation().InitializeProjectExtents();

    if (m_defaultViewId.IsValid() && IsCreatingNewDgnDb())
        GetDgnDb().SaveProperty(DgnViewProperty::DefaultView(), &m_defaultViewId, sizeof(m_defaultViewId));

    IDwgChangeDetector& changeDetector = _GetChangeDetector ();
    if (IsUpdating())
        {
        DwgDbDatabaseP  dwg = m_dwgdb.get ();

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
        // done deletion
        changeDetector._DetectDeletedElementsEnd (*this);
        changeDetector._DetectDeletedModelsEnd (*this);

        // update syncinfo for master DWG file
        DwgSyncInfo&    syncInfo = GetSyncInfo ();
        StableIdPolicy  policy = GetCurrentIdPolicy ();
        DwgSyncInfo::FileProvenance     masterProv(*m_dwgdb.get(), syncInfo, policy);
        masterProv.Update ();

        // update syncinfo for xref files
        for (auto& xref : m_loadedXrefFiles)
            {
            if (nullptr != (dwg = xref.GetDatabaseP()))
                {
                DwgSyncInfo::FileProvenance xrefProv(*dwg, syncInfo, policy);
                xrefProv.Update ();
                }
            }
        }
    changeDetector._Cleanup (*this);

    ValidateJob ();

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

    // Initializing ImportJob should have set the root model ID, but check it anyway.
    if (m_modelspaceId.IsNull())
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
DwgImporter::DwgImporter (DwgImporter::Options const& options) : m_options(options), m_syncInfo(*this), m_config(options.GetConfigFile(), *this), m_dgndb(), m_dwgdb(), m_loadedFonts(*this), m_issueReporter(options.GetReportFileName())
    {
    m_wasAborted = false;
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
    m_materialSearchPaths.clear ();
    m_isProcessingDwgModelMap = false;

    this->SetStepName (ProgressMessage::STEP_INITIALIZING());

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

