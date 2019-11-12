/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

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
