/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

// Stub out these macros. We should not suppress exceptions within a bridge. Only fwk s/ do that.
#define IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS
#define IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG(REPORTING_CODE)

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
* @bsimethod                                                    Don.Fu          12/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_MakeSchemaChanges ()
    {
    if (!m_dgndb.IsValid() || !m_dwgdb.IsValid())
        return  static_cast<BentleyStatus>(DgnDbStatus::NotOpen);

    m_paperspaceBlockIds.clear ();
    m_loadedXrefFiles.clear ();
    m_unresolvedXrefFiles.clear ();

    /*-----------------------------------------------------------------------------------
    Iterate the block table once for multiple tasks in an import job:
        1) Load xRef files from xRef block table records
        2) Cache layout block object ID's from paperspace block table records
        3) Collect attribute definitions from regular block table records
    -----------------------------------------------------------------------------------*/
    XRefLoader  xrefLoader(*this);
    xrefLoader.LoadXrefsInMasterFile ();
    xrefLoader.CacheUnresolvedXrefs ();
    auto attrdefSchema = xrefLoader.GetAttrdefSchema ();

    if (attrdefSchema.IsValid() && attrdefSchema->GetClassCount() > 0)
        {
        if (m_dgndb->BriefcaseManager().LockSchemas().Result() != RepositoryStatus::Success)
            return  static_cast<BentleyStatus>(DgnDbStatus::LockNotHeld);

        this->ImportAttributeDefinitionSchema (*attrdefSchema);
        }

    return  BentleyStatus::SUCCESS;
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
ResolvedModelMapping    DwgImporter::GetModelFromSyncInfo (DwgDbObjectIdCR id, DwgDbDatabaseR dwg, TransformCR trans)
    {
    // retrieve a model by file & model instance ID of a model/paperspace block, xref insert or a raster image.
    if (!id.IsValid())
        return ResolvedModelMapping();

    // the input file is the source file of the model - for an xref model it is the xref file:
    auto aspect = m_sourceAspects.FindModelAspect(id, dwg, trans);
    if (aspect.IsValid())
        {
        auto model = m_dgndb->Models().GetModel(aspect.GetModelId());
        if (model.IsValid())
            return  ResolvedModelMapping(id, model.get(), aspect);
        }

    return ResolvedModelMapping();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping   DwgImporter::FindModel (DwgDbObjectIdCR dwgModelId, TransformCR trans, DwgSourceAspects::ModelAspect::SourceType sourceType)
    {
    // find cached model mapping by DWG model object ID, matching transform, and source type
    ResolvedModelMapping    unresolved (dwgModelId);
    static double   s_angleTolerance = 1.0e-3;

    auto    range = m_dwgModelMap.equal_range (unresolved);
    for (auto modelMap = range.first; modelMap != range.second; ++modelMap)
        {
        // a DgnModel exists, check transform & source type!
        if (modelMap->GetTransform().IsEqual(trans, s_angleTolerance, m_sizeTolerance) && modelMap->GetModelAspect().GetSourceType() == sourceType) 
            return *modelMap;
        }

    return  ResolvedModelMapping();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping   DwgImporter::FindModel (DwgDbObjectIdCR dwgModelId, DwgSourceAspects::ModelAspect::SourceType sourceType)
    {
    // find cached model mapping by DWG model ID & source type only, ignoring transform
    ResolvedModelMapping    unresolved (dwgModelId);

    auto    range = m_dwgModelMap.equal_range (unresolved);
    for (auto modelMap = range.first; modelMap != range.second; ++modelMap)
        {
        // a DgnModel exists, check source type!
        if (modelMap->GetModelAspect().GetSourceType() == sourceType) 
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
Utf8String      DwgImporter::_ComputeModelName (DwgDbBlockTableRecordCR block, DwgDbHandleCP idAsSuffix)
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

    // append an ID suffix for an xRef insert entity
    Utf8CP      inSuffix = nullptr;
    Utf8String  suffix;
    if (nullptr != idAsSuffix && !idAsSuffix->IsNull())
        {
        suffix.Sprintf ("(%llx)", idAsSuffix->AsUInt64());
        suffix = suffix.c_str ();
        }

    return  this->ComputeModelName (modelName, baseFilename, refPath, inSuffix, modelType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::ComputeModelName (Utf8StringR proposedName, BeFileNameCR baseFilename, BeFileNameCR refPath, Utf8CP inSuffix, DgnClassId modelType)
    {
    bool        hasRule = false;
    ImportRule  modelMergeEntry;
    if (BSISUCCESS == this->SearchForMatchingRule(modelMergeEntry, proposedName, baseFilename))
        {
        Utf8String      computedName;
        if (BSISUCCESS == modelMergeEntry.ComputeNewName(computedName, proposedName, baseFilename))
            {
            proposedName = computedName;
            hasRule = true;
            }
        }

    // unless user has a rule for a layout, append the base file name:
    if (!hasRule && m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SheetModel) == modelType)
        proposedName += "[" + Utf8String(baseFilename) + "]";

    DgnModels&  models = m_dgndb->Models ();
    models.ReplaceInvalidCharacters (proposedName, models.GetIllegalCharacters(), '_');

    Utf8String  uniqueName(proposedName);
    if (nullptr != inSuffix && inSuffix[0] != 0)
        uniqueName += inSuffix;

    if (m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalModel) != modelType)
        return  uniqueName;

    // unique physical model names will be later handled by PhysicalPartition::CreateUniqueCode

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
    this->GetRootTransform().IsRigidScale (toMeters);
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

        DwgDbSymbolTableIteratorPtr iter = viewportTable->NewIterator ();
        if (!iter.IsValid() || !iter->IsValid())
            return  BSIERROR;

        viewport.OpenObject (iter->GetRecordId(), DwgDbOpenMode::ForRead);
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
    Utf8String      descr (block.GetComments().c_str());
    SchemaManagerCR dgndbSchemas = m_dgndb->Schemas ();
    DgnElementId    modelElementId;

    if (dgndbSchemas.GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalModel) == classId)
        {
        // modelspace or xref model
        SubjectCPtr             rootSubject = this->GetSpatialParentSubject ();
        DgnCode                 partitionCode = PhysicalPartition::CreateUniqueCode (*rootSubject, modelName.c_str());
        PhysicalPartitionCPtr   partition = PhysicalPartition::CreateAndInsert (*rootSubject, partitionCode.GetValueUtf8CP(), descr.c_str());
        if (partition.IsValid())
            modelElementId = partition->GetElementId();
        else
            this->ReportError (IssueCategory::Unknown(), Issue::CantCreateModel(), modelName.c_str());
        }
    else if (dgndbSchemas.GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SheetModel) == classId)
        {
        // paperspace model, or app remapped modelspace or xref model
        DocumentListModelPtr    sheetListModel = m_dgndb->Models().Get<DocumentListModel>(m_sheetListModelId);
        if (!sheetListModel.IsValid())
            return modelElementId;

        DPoint2d    sheetSize = DPoint2d::From (1.0, 1.0);
        double      scale = 1.0;
        if (block.IsPaperspace())
            {
            LayoutFactory   factory (*this, block.GetLayoutId());
            scale = factory.GetUserScale ();
            factory.CalculateSheetSize (sheetSize);
            }
        else
            {
            DRange3d    range;
            if (DwgDbStatus::Success == const_cast<DwgDbBlockTableRecordR>(block).ComputeRange(range))
                {
                auto units = this->GetModelUnitsFromBlock (scale, block);
                double toMeters = units.ToMeters ();
                sheetSize.Init (range.XLength(), range.YLength());
                sheetSize.Scale (toMeters);
                }
            }

        DgnCode             sheetCode = Sheet::Element::CreateUniqueCode(*sheetListModel, modelName.c_str());
        Sheet::ElementPtr   sheet = Sheet::Element::Create (*sheetListModel, scale, sheetSize, sheetCode.GetValueUtf8CP());
        if (!sheet.IsValid())
            return modelElementId;

        sheet->SetUserLabel (modelName.c_str());

        m_dgndb->Elements().Insert <Sheet::Element> (*sheet);

        if (sheet.IsValid())
            modelElementId = sheet->GetElementId ();
        else
            this->ReportError (IssueCategory::Unknown(), Issue::CantCreateModel(), modelName.c_str());
        }
    else if (dgndbSchemas.GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingModel) == classId)
        {
        // app remapped modelspace or xref model
        DocumentListModelPtr drawingListModel = m_dgndb->Models().Get<DocumentListModel>(m_drawingListModelId);
        if (!drawingListModel.IsValid())
            return modelElementId;

        DgnCode     drawingCode = Drawing::CreateUniqueCode (*drawingListModel, modelName.c_str());
        DrawingPtr  drawing = Drawing::Create (*drawingListModel, drawingCode.GetValueUtf8CP());
        if (!drawing.IsValid())
            return modelElementId;

        drawing->SetUserLabel (modelName.c_str());

        m_dgndb->Elements().Insert <Drawing> (*drawing);
        if (drawing.IsValid())
            modelElementId = drawing->GetElementId ();
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
static DwgSourceAspects::ModelAspect::SourceType GetModelSourceType (DwgDbBlockTableRecordCR block, bool expectXref)
    {
    if (expectXref)
        return DwgSourceAspects::ModelAspect::SourceType::XRefAttachment;
    return block.IsModelspace() ? DwgSourceAspects::ModelAspect::SourceType::ModelSpace : DwgSourceAspects::ModelAspect::SourceType::PaperSpace;
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
    ResolvedModelMapping    modelMap;
    auto    sourceType = GetModelSourceType (dwgBlock, nullptr != xrefInsert);
    if (DwgSourceAspects::ModelAspect::SourceType::XRefAttachment == sourceType && nullptr == xrefDwg)
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

    // if a known insert entity is given, use its handle value as model suffix:
    DwgDbHandle insertHandle;
    if (nullptr != xrefInsert)
        insertHandle = xrefInsert->GetObjectId().GetHandle();

    // build a consistent & valid model name for all DWG models:
    auto modelName = this->_ComputeModelName (dwgBlock, &insertHandle);

    if (this->IsUpdating())
        {
        if (DwgSourceAspects::ModelAspect::SourceType::XRefAttachment == sourceType)
            modelMap = this->GetModelFromSyncInfo (dwgModelId, *xrefDwg, trans);
        else
            modelMap = this->GetModelFromSyncInfo (dwgModelId, this->GetDwgDb(), trans);

        if (modelMap.IsValid())
            {
            // update model name if changed
            auto model = modelMap.GetModel ();
            if (nullptr != model && !model->GetName().Equals(modelName))
                {
                auto modelElement = this->GetDgnDb().Elements().GetForEdit<DgnElement>(model->GetModeledElementId());
                if (modelElement.IsValid())
                    this->UpdateElementName (*modelElement, modelName, modelName.c_str());
                }

            // add the model to cache if not already added(need the check for a multiset):
            if (!this->FindModel(dwgModelId, trans, sourceType).IsValid())
                this->AddToDwgModelMap (modelMap);
            // tell the updater about the coming DWG model object:
            this->_GetChangeDetector()._OnModelSeen (*this, modelMap);
            return  modelMap;
            }
        // not found in syncinfo - may have to create a new one.
        }

    // see if this model has already been created
    modelMap = this->FindModel (dwgModelId, trans, sourceType);
    if (modelMap.IsValid())
        return  modelMap;

    // create a new model
    DgnClassId  modelType = this->_GetModelType (dwgBlock);
    DgnModelId  modelId = this->CreateModel (dwgBlock, modelName.c_str(), modelType);
    if (!modelId.IsValid())
        return ResolvedModelMapping();

    DgnModelP   model = m_dgndb->Models().GetModel(modelId).get ();
    if (nullptr == model)
        {
        this->ReportError (IssueCategory::Unknown(), Issue::ImportFailure(), IssueReporter::FmtModel(dwgBlock).c_str());
        this->OnFatalError ();
        return modelMap;
        }

    // add a model map entry to syncInfo:
    modelMap = this->CreateAndInsertModelMap (model, dwgBlock, trans, xrefInsert, xrefDwg);

    // add an ElementHasLinks relationship for the model & its RepositoryLink:
    this->InsertElementHasLinks (*model, nullptr == xrefDwg ? *m_dwgdb : *xrefDwg);

    return  modelMap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping DwgImporter::CreateAndInsertModelMap (DgnModelP model, DwgDbBlockTableRecordCR dwgBlock, TransformCR trans, DwgDbBlockReferenceCP xrefInsert, DwgDbDatabaseP xrefDwg)
    {
    ResolvedModelMapping    modelMap;
    if (nullptr == model)
        return  modelMap;

    // save the model info to syncInfo:
    DwgDbObjectId   dwgModelId = xrefInsert == nullptr ? dwgBlock.GetObjectId() : xrefInsert->GetObjectId();
    DgnModelId      dgnModelId = model->GetModelId ();

    auto modelAspect = m_sourceAspects.AddModelAspect (*model, dwgBlock, trans, xrefInsert, xrefDwg);
    if (!modelAspect.IsValid())
        return  modelMap;

    modelMap.SetModelInstanceId (dwgModelId);
    modelMap.SetModel (model);
    modelMap.SetModelAspect (modelAspect);

    if (LOG_MODEL_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
        LOG_MODEL.tracev("+ %s 0x%x -> %s %d", modelAspect.GetDwgModelName().c_str(), modelAspect.GetDwgModelHandle().AsUInt64(), model->GetName().c_str(), dgnModelId.GetValue());

    // save in our list of known model mappings.
    this->AddToDwgModelMap (modelMap);
    // tell the updater about the newly discovered model
    this->_GetChangeDetector()._OnModelInserted (*this, modelMap, xrefDwg);
    this->_GetChangeDetector()._OnModelSeen (*this, modelMap);

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
ResolvedModelMapping    DwgImporter::_GetOrCreateRootModel (DwgSourceAspects::ModelAspectCP rootModelAspect)
    {
    ResolvedModelMapping    rootModelMap;
    if (m_dwgdb.IsNull())
        {
        BeAssert (false && "DWG file is not opened yet!");
        return  rootModelMap;
        }

    // do this only once per session
    rootModelMap = this->GetRootModel ();
    if (rootModelMap.IsValid())
        return  rootModelMap;

    if (m_modelspaceId.IsNull())
        m_modelspaceId = m_dwgdb->GetModelspaceId ();

    // apply the spatial model transform initiated from the iModelBridge:
    Transform   jobTransform = iModelBridge::GetSpatialDataTransform(this->GetOptions(), this->GetJobSubject());
    m_rootTransformInfo.SetJobTransform (jobTransform);

    TransformR  rootTransform = m_rootTransformInfo.GetRootTransformR ();
    if (!rootTransform.IsEqual(jobTransform, 0.001, jobTransform.Translation().Magnitude() * 0.001))
        rootTransform.InitProduct (jobTransform, rootTransform);

    // set the root model transformation from the modelspace block, compounded by the job tranform.
    DwgDbBlockTableRecordPtr    modelspaceBlock(m_modelspaceId, DwgDbOpenMode::ForRead);
    if (modelspaceBlock.IsNull())
        {
        this->ReportError (IssueCategory::Unknown(), Issue::CannotOpenModelspace(), "when creating the root model!");
        this->OnFatalError ();
        return rootModelMap;
        }
    Transform   newTransform = m_rootTransformInfo.GetRootTransform ();
    this->ScaleModelTransformBy (newTransform, *modelspaceBlock.get());
    m_rootTransformInfo.SetRootTransform (newTransform);

    // set size tolerance for DgnModel discovering - FindModel depends on reasonable tolerances:
    DRange3d    dwgRange = DRange3d::From(m_dwgdb->GetEXTMIN(), m_dwgdb->GetEXTMAX());
    m_sizeTolerance = dwgRange.DiagonalDistance() * 1.0e-7;
    m_sizeTolerance *= this->GetScaleToMeters();

    if (rootModelAspect != nullptr)
        {
        // updating - try retrieving the root model from the syncInfo per current import job:
        rootModelMap.SetModelAspect (*rootModelAspect);
        rootModelMap.SetModelInstanceId (m_modelspaceId);
        rootModelMap.SetModel (this->GetDgnDb().Models().GetModel(rootModelAspect->GetModelId()).get());
        if (rootModelMap.IsValid())
            {
            this->_GetChangeDetector()._OnModelSeen (*this, rootModelMap);
            this->AddToDwgModelMap (rootModelMap);

            // check if the root model transformation has been changed from previous import:
            auto oldTransform = rootModelMap.GetTransform ();
            auto pointTol = oldTransform.Translation().Magnitude() * 0.01;

            m_rootTransformInfo.SetHasChanged (!newTransform.IsEqual(oldTransform, 0.01, pointTol));

            if (m_rootTransformInfo.HasChanged())
                {
                // use previous transform as the effective root transform, so we can find the models - will apply new transform after model dicovery phase.
                m_rootTransformInfo.SetRootTransform (oldTransform);
                // calculate and save the change that will transform models from old to new:
                Transform   fromOldToNew = Transform::FromProduct (newTransform, oldTransform.ValidatedInverse().Value());
                m_rootTransformInfo.SetChangeTransformFromOldToNew (fromOldToNew);
                }

            return  rootModelMap;
            }
        }

    // try creating the root model from the modelspace block:
    rootModelMap = this->GetOrCreateModelFromBlock (*modelspaceBlock.get(), m_rootTransformInfo.GetRootTransform());
    if (!rootModelMap.IsValid())
        this->ReportError (IssueCategory::MissingData(), Issue::CantCreateModel(), IssueReporter::FmtModel(*modelspaceBlock).c_str());
    else
        this->SetModelPropertiesFromModelspaceViewport (rootModelMap.GetModel());

    return rootModelMap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping DwgImporter::_ImportLayoutModel (DwgDbBlockTableRecordCR block)
    {
    // Units of paperspace models are from plot settings.
    Transform   layoutTransform = Transform::FromIdentity ();
    this->ScaleModelTransformBy (layoutTransform, block);
    // WIP - when/if Sheet::Border supports location in the future, switching code to set the origin, instead of moving geometry:
    this->AlignSheetToPaperOrigin (layoutTransform, block.GetLayoutId());

    auto modelMap = this->GetOrCreateModelFromBlock (block, layoutTransform);
    if (!modelMap.IsValid() || modelMap.GetModel() == nullptr)
        this->ReportError (IssueCategory::MissingData(), Issue::CantCreateModel(), IssueReporter::FmtModel(block).c_str());
    else
        this->SetModelPropertiesFromLayout (modelMap.GetModel(), block.GetLayoutId());
    return  modelMap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping DwgImporter::_ImportXrefModel (DwgDbBlockTableRecordCR block, DwgDbObjectIdCR insertId, DwgDbDatabaseP xRefDwg)
    {
    ResolvedModelMapping    modelMap;
    DwgDbBlockReferencePtr  insert(insertId, DwgDbOpenMode::ForRead);
    if (insert.OpenStatus() != DwgDbStatus::Success)
        return  modelMap;

    // open & check parent of this insert entity:
    DwgDbObjectId               parentId = insert->GetOwnerId ();
    DwgDbBlockTableRecordPtr    parentBlock (parentId, DwgDbOpenMode::ForRead);
    if (parentBlock.OpenStatus() != DwgDbStatus::Success)
        {
        this->ReportError (IssueCategory::MissingData(), Issue::CantOpenObject(), Utf8PrintfString("the owner block of an xRef %ls", block.GetName().c_str()).c_str());
        return  modelMap;
        }

    Transform   xtrans = this->GetRootTransform ();
    DgnModelP   parentModel = nullptr;

    // if this xref is inserted in a paperspace block, import paperspace first, then use paperspace transform:
    bool    isParentPaperspace = parentBlock->IsLayout() && !parentBlock->IsModelspace() && m_modelspaceId != parentId;
    if (isParentPaperspace)
        {
        // get or create the paperspace model if not already created:
        xtrans = Transform::FromIdentity ();
        this->ScaleModelTransformBy (xtrans, *parentBlock.get());
        // WIP - when/if Sheet::Border supports location in the future, switching code to set the origin, instead of moving geometry:
        this->AlignSheetToPaperOrigin (xtrans, parentBlock->GetLayoutId());

        modelMap = this->GetOrCreateModelFromBlock (*parentBlock.get(), xtrans);
        if (!modelMap.IsValid() || (parentModel = modelMap.GetModel()) == nullptr)
            {
            this->ReportError (IssueCategory::MissingData(), Issue::CantCreateModel(), IssueReporter::FmtModel(*parentBlock).c_str());
            return  modelMap;
            }
        // we now have a paperspace transform for current xref instance...
        }

    this->CompoundModelTransformBy (xtrans, *insert.get());

    modelMap = this->GetOrCreateModelFromBlock (block, xtrans, insert.get(), xRefDwg);

    DgnModelP   model = nullptr;
    if (!modelMap.IsValid() || (model = modelMap.GetModel()) == nullptr)
        {
        this->ReportError (IssueCategory::MissingData(), Issue::CantCreateModel(), IssueReporter::FmtModel(block).c_str());
        return  modelMap;
        }

    // if the instance is in the modelspace, add it to modelspace xref list:
    if (m_modelspaceId == parentId)
        {
        m_modelspaceXrefs.insert (model->GetModelId());
        return  modelMap;
        }

    // if the instance is in a paperspace, add the info into paperspace xref list:
    if (isParentPaperspace && nullptr != parentModel)
        {
        m_paperspaceXrefs.push_back (DwgXRefInPaperspace(insertId, parentId, parentModel->GetModelId()));
        return  modelMap;
        }

    // close the parent block for the explode operation:
    parentBlock->Close ();

    // the instance is neither in the modelspace nor in a paperspace - drop it into the parent block:
    if (DwgDbStatus::Success != insert->ExplodeToOwnerSpace())
        this->ReportError (IssueCategory::Unknown(), Issue::Error(), Utf8PrintfString("failed dropping nested xRef %ls", block.GetName().c_str()).c_str());

    return  modelMap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::ImportXrefModelsFrom (DwgXRefHolder& xref, SubjectCR parentSubject, bool& hasPushedReferencesSubject)
    {
    DwgDbBlockTableRecordPtr    block(xref.GetBlockIdInRootFile(), DwgDbOpenMode::ForRead);
    if (block.OpenStatus() != DwgDbStatus::Success)
        {
        this->ReportError (IssueCategory::CorruptData(), Issue::CantOpenObject(), "xRef block");
        return  BSIERROR;
        }

    auto name = block->GetName ();
    if (name.IsEmpty())
        {
        this->ReportIssueV (IssueSeverity::Info, IssueCategory::UnexpectedData(), Issue::ConfigUsingDefault(), IssueReporter::FmtModel(*block).c_str());
        name = L"Xref";
        }

    // expect an xRef block
    if (!block->IsExternalReference())
        {
        this->ReportError (IssueCategory::UnexpectedData(), Issue::CantCreateModel(), IssueReporter::FmtModel(*block).c_str());
        return  BSIERROR;
        }

    DwgDbObjectIdArray  ids;
    if (DwgDbStatus::Success != block->GetBlockReferenceIds(ids) || ids.empty())
        {
        /*------------------------------------------------------------------------------------------------
        There could be legitimate reasons as well as file errors that no instances found for an xref block.
        A nested xref would not have instances in the outermost master file for example.  Whatever has led
        us here is not important - we will not add a model for an xref block that has no instances.  We will
        create a model when we see an xref attachment while importing entities.
        Give the updater a chance to cache skipped models linked to this file.
        ------------------------------------------------------------------------------------------------*/
        this->_GetChangeDetector()._ShouldSkipFileAndModels (*this, xref.GetDatabase());
        return  BSISUCCESS;
        }

    if (!ids.empty())
        LOG_MODEL.tracev("Creating DgnModel(s) from DWG xRef block %ls with %d instances", name.c_str(), ids.size());

    // the xref block has instances - create models for each of them:
    for (auto const& id : ids)
        {
        auto handle = id.GetHandle ();
        auto modelName = this->_ComputeModelName(*block, &handle);

        // get & set refs subject
        SubjectCPtr refsSubject = this->GetOrCreateModelSubject (parentSubject, modelName, ModelSubjectType::References);
        if (refsSubject.IsValid())
            {
            // set the root model subject as spatial parent
            this->SetSpatialParentSubject (*refsSubject);
            hasPushedReferencesSubject = true;
            }
        else
            {
            ReportError (IssueCategory::Unsupported(), Issue::Error(), Utf8PrintfString("Failed to create references subject. XRef[%s].", IssueReporter::FmtXReference(*block).c_str()).c_str());
            BeAssert (false && "Failed creating xrefrences subject!");
            }

        DgnModelP   model;
        auto modelMap = this->_ImportXrefModel (*block, id, xref.GetDatabaseP());
        if (!modelMap.IsValid() || (model = modelMap.GetModel()) == nullptr)
            continue;

        // cache the mapped model in the xref holder:
        xref.AddDgnModel (model->GetModelId());

        // give the updater a chance to cache skipped models as we may not see xref inserts during importing phase:
        this->_GetChangeDetector()._ShouldSkipModel (*this, modelMap, xref.GetDatabaseP());
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportDwgModels ()
    {
    // should have created root model when initializing or finding the importer job:
    BeAssert (this->GetRootModel().IsValid());

    SubjectCPtr parentSubject = this->GetSpatialParentSubject ();
    if (!parentSubject.IsValid())
        {
        BeAssert (false && "parent subject for spatial models not set yet!!");
        return  BSIERROR;
        }

    // create models for xRef block references:
    bool    hasPushedReferencesSubject = false;
    for (auto& xref : m_loadedXrefFiles)
        this->ImportXrefModelsFrom (xref, *parentSubject, hasPushedReferencesSubject);

    if (hasPushedReferencesSubject)
        this->SetSpatialParentSubject(*parentSubject);

    // create models for paperspaces
    for (auto paperspaceId : m_paperspaceBlockIds)
        {
        DwgDbBlockTableRecordPtr    block(paperspaceId, DwgDbOpenMode::ForRead);
        if (block.OpenStatus() == DwgDbStatus::Success && block->IsLayout())
            {
            LOG_MODEL.tracev("Creating DgnModel from DWG layout block %ls", block->GetName().c_str());
            this->_ImportLayoutModel (*block);
            }
        else
            {
            this->ReportError (IssueCategory::CorruptData(), Issue::CantOpenObject(), "paperspace block");
            }
        }

    /*-----------------------------------------------------------------------------------
    Now we have passed the model discovery phase, we can update model transforms if the 
    root transform has been changed from previous import.  Raster models and some xRef
    instances that are missing from DwgDbBlockTableRecord::GetBlockReferenceIds will have
    to be treated individually when we process their instances.
    -----------------------------------------------------------------------------------*/
    if (this->IsUpdating() && this->HasRootTransformChanged())
        {
        // will update impacted models from the old transform to the new transform:
        Transform   fromOldToNew = m_rootTransformInfo.GetChangeTransformFromOldToNew ();

        for (auto& modelMap : m_dwgModelMap)
            {
            // update transform for all models but paperspace's, as well as xref's attached in them:
            DwgSourceAspects::ModelAspect::SourceType sourceType = modelMap.GetModelAspect().GetSourceType ();
            if (sourceType == DwgSourceAspects::ModelAspect::SourceType::PaperSpace)
                continue;

            if (sourceType == DwgSourceAspects::ModelAspect::SourceType::XRefAttachment || sourceType == DwgSourceAspects::ModelAspect::SourceType::RasterAttachment)
                {
                // don't update xref's and rasters in a paperspace:
                auto xrefInsertId = modelMap.GetModelInstanceId ();
                if (this->IsXrefInsertedInPaperspace(xrefInsertId))
                    continue;
                }
                
            // update the model map to the new transform by applying the delta transform:
            modelMap.SetTransform (Transform::FromProduct(fromOldToNew, modelMap.GetTransform()));
            }

        // apply the new root transform for future process:
        Transform   newRootTransform = Transform::FromProduct (fromOldToNew, m_rootTransformInfo.GetRootTransform());
        m_rootTransformInfo.SetRootTransform (newRootTransform);
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::IsXrefInsertedInPaperspace (DwgDbObjectIdCR xrefInsertId) const
    {
    // find an xref insert from a paperspace
    auto found = std::find_if (m_paperspaceXrefs.begin(), m_paperspaceXrefs.end(), [&](DwgXRefInPaperspace const& entry)
        {
        return  entry.GetXrefInsertId() == xrefInsertId;
        });
    return  found != m_paperspaceXrefs.end();
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
    //IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS
        {
        this->_ImportTextStyleSection ();
        }
    //IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG(this->ReportError(IssueCategory::Unknown(), Issue::Exception(), "failed processing text styles"));
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Text Styles");

    timer.Start();
    //IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS
        {
        this->_ImportLineTypeSection ();
        }
    //IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG(this->ReportError(IssueCategory::Unknown(), Issue::Exception(), "failed processing line types"));
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Linetype Section");

    timer.Start();
    //IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS
        {
        this->_ImportMaterialSection ();
        }
    //IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG(this->ReportError(IssueCategory::Unknown(), Issue::Exception(), "failed processing materials"));
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Material Section");

    timer.Start();
    //IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS
        {
        this->_ImportLayerSection ();
        }
    //IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG(this->ReportError(IssueCategory::Unknown(), Issue::Exception(), "failed processing layers"));
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
    //IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS
        {
        this->_ImportDwgModels ();
        }
    //IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG(this->ReportError(IssueCategory::Unknown(), Issue::Exception(), "failed processing models"));
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Create Models");

    timer.Start();
    this->_ImportModelspaceViewports ();
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Modelspace Viewports");

    timer.Start();
    //IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS
        {
        this->_ImportEntitySection ();
        }
    //IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG(this->ReportError(IssueCategory::Unknown(), Issue::Error(), "failed processing entity section"));
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Entity Section/ModelSpace");

    timer.Start();
    IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS
        {
        this->_ImportLayouts ();
        }
    IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG(this->ReportError(IssueCategory::Unknown(), Issue::Error(), "failed processing layouts"));
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Layouts");

    timer.Start();
    //IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS
        {
        this->_ImportGroups ();
        }
    //IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG(this->ReportError(IssueCategory::Unknown(), Issue::Error(), "failed processing groups"));
    if (this->WasAborted())
        return BSIERROR;

    DwgImportLogging::LogPerformance(timer, "Import Groups");

    timer.Start ();
    //IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS
        {
        this->_FinishImport ();
        }
    //IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS_AND_LOG(this->ReportError(IssueCategory::Unknown(), Issue::Error(), "failed cleaning up the importer"));
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
    m_currIdPolicy = StableIdPolicy::ById;
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
