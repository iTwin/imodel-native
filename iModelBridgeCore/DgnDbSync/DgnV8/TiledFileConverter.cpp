/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/TiledFileConverter.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

// We enter this namespace in order to avoid having to qualify all of the types, such as bmap, that are common
// to bim and v8. The problem is that the V8 Bentley namespace is shifted in.
BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledFileConverter::_SetChangeDetector(bool isUpdating)
    {
    if (!isUpdating)
        m_changeDetector.reset(new CreatorChangeDetector);
    else
        {
        BeAssert((nullptr != GetRootV8File()) && "_SetChangeDetector should be called after _OpenSource");
        m_changeDetector.reset(new ChangeDetectorForTiles(*GetRootV8File())); 
        m_skipECContent = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::ModelId TiledFileConverter::GetDefaultModelId(DgnV8FileR v8File)
    {
    if (!IsV8Format(v8File))
        return DgnV8Api::DEFAULTMODEL;

    return v8File.GetDefaultModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::DgnFileStatus TiledFileConverter::_InitRootModel()
    {
    // *** NB: Do not create elements (or models) in here. This is running as part of the initialization phase.
    //          Only schema changes are allowed in this phase.

    //  Open the root V8File
    DgnV8Api::DgnFileStatus openStatus;    
    m_rootFile = OpenDgnV8File(openStatus, GetRootFileName());
    if (!m_rootFile.IsValid())
        return openStatus;
    
    //  Identify the root model
    DgnV8Api::ModelId id = GetDefaultModelId(*m_rootFile);

    //  Load the root model
    m_rootModelRef = m_rootFile->LoadRootModelById((Bentley::StatusInt*)&openStatus, id, /*fillCache*/true, /*loadRefs*/true, /*processAffected*/false);
    if (NULL == m_rootModelRef)
        return openStatus;

    //  Note that, for now, we do not support reprojection in a tiled file conversion
    ComputeTransformAndGlobalOriginFromRootModel(*m_rootModelRef->GetDgnModelP(), true);

    if (!ShouldConvertToPhysicalModel(*GetRootModelP()))
        {
        ReportError(Converter::IssueCategory::Unsupported(), Converter::Issue::RootModelMustBePhysical(), Converter::IssueReporter::FmtModel(*GetRootModelP()).c_str());
        OnFatalError();
        return DgnV8Api::DGNFILE_STATUS_UnknownError;
        }

    SetLineStyleConverterRootModel(m_rootModelRef->GetDgnModelP());

    CreateProvenanceTables(); // WIP_EXTERNAL_SOURCE_INFO - stop using so-called model provenance
    GetRepositoryLinkId(*GetRootV8File()); // DynamicSchemaGenerator et al need to assume that all V8 files are recorded in syncinfo

    return WasAborted() ? DgnV8Api::DGNFILE_STATUS_UnknownError: DgnV8Api::DGNFILE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping TiledFileConverter::_GetResolvedModelMapping(DgnV8ModelRefCR v8ModelRef, TransformCR)
    {
    if (IsUpdating())
        {
        ResolvedModelMapping res = FindModelByExternalAspect(v8ModelRef, m_rootTrans);
        if (res.IsValid())
            {
            GetChangeDetector()._OnModelSeen(*this, res);
            return res;
            }

        // not found in syncinfo => treat as insert
        }

    DgnV8ModelR v8Model = *v8ModelRef.GetDgnModelP();

    Utf8String newModelName = _ComputeModelName(v8Model).c_str();

    // Map in the DgnV8 model.
    DgnModelId modelId = _MapModelIntoProject(v8Model, newModelName.c_str(), v8ModelRef.AsDgnAttachmentCP());
    if (!modelId.IsValid())
        {
        return ResolvedModelMapping();
        }

    if (_WantModelProvenanceInBim())
        {
        DgnV8FileP file = v8Model.GetDgnFileP();
        BeSQLite::BeGuid guid = GetDocumentGUIDforFile(*file);
        if (SUCCESS == DgnV8FileProvenance::Find(nullptr, nullptr, guid, GetDgnDb()))
            {
            DgnV8ModelProvenance::ModelProvenanceEntry entry;
            entry.m_dgnv8ModelId = v8Model.GetModelId();
            entry.m_modelId = modelId;
            entry.m_modelName = Utf8String(v8Model.GetModelName());
            entry.m_trans = m_rootTrans;
            DgnV8ModelProvenance::Insert(guid, entry, GetDgnDb());
            }
        }

    DgnModelPtr model = m_dgndb->Models().GetModel(modelId);
    if (!model.IsValid())
        {
        ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), IssueReporter::FmtModel(v8Model).c_str());
        OnFatalError();
        return ResolvedModelMapping();
        }
    BeAssert(model->GetRefCount() > 0); // DgnModels holds references to all models that it loads

    auto modeledElement = m_dgndb->Elements().GetElement(model->GetModeledElementId())->CopyForEdit();
    auto modelAspect = SyncInfo::V8ModelExternalSourceAspect::CreateAspect(v8Model, m_rootTrans, *this);
    modelAspect.AddAspect(*modeledElement);
    modeledElement->Update();

    auto v8mm = ResolvedModelMapping(*model, v8Model, modelAspect, v8ModelRef.AsDgnAttachmentCP());

    GetChangeDetector()._OnModelInserted(*this, v8mm);
    GetChangeDetector()._OnModelSeen(*this, v8mm);
    m_monitor->_OnModelInserted(v8mm);

    return v8mm;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping TiledFileConverter::MapDgnV8ModelToDgnDbModel(DgnV8ModelR v8Model, DgnModelId targetModelId)
    {
    BeAssert(&v8Model != GetRootModelP());

    if (IsUpdating())
        {
        ResolvedModelMapping res = FindModelByExternalAspect(v8Model, m_rootTrans);
        if (res.IsValid())
            {
            GetChangeDetector()._OnModelSeen(*this, res);
            return res;
            }

        // not found in syncinfo => treat as insert
        }

    GetRepositoryLinkId(*v8Model.GetDgnFileP());

    auto model = m_dgndb->Models().GetModel(targetModelId);
    if (!model.IsValid())
        return ResolvedModelMapping();

    auto modeledElement = m_dgndb->Elements().GetElement(model->GetModeledElementId())->CopyForEdit();
    auto modelAspect = SyncInfo::V8ModelExternalSourceAspect::CreateAspect(v8Model, m_rootTrans, *this);
    modelAspect.AddAspect(*modeledElement);
    auto updatedModelElement = modeledElement->Update();
    BeAssert(updatedModelElement.IsValid());

    ResolvedModelMapping v8mm(*model, v8Model, modelAspect, nullptr);

    GetChangeDetector()._OnModelInserted(*this, v8mm);
    GetChangeDetector()._OnModelSeen(*this, v8mm);

    return v8mm;
    }

/*---------------------------------------------------------------------------------**//**
* Convert all of the elements in a DgnV8Model into Elements
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledFileConverter::_ConvertElementsInModel(ResolvedModelMapping const& v8mm)
    {
    BeAssert(&m_rootModelMapping.GetDgnModel() == &v8mm.GetDgnModel());

    GetChangeDetector()._OnModelSeen(*this, v8mm);

    DgnV8Api::DgnModel& v8Model = v8mm.GetV8Model();

    auto* v8File = v8Model.GetDgnFileP();
    BeFileName baseName(BeFileName::NameAndExt, v8File->GetFileName().c_str());

    v8Model.FillCache(DgnV8Api::DgnModelSections::Model);

    ReportProgress();

    m_currIdPolicy = GetIdPolicyFromAppData(*v8File);
    
    ConvertElementList(v8Model.GetControlElementsP(), v8mm);
    ConvertElementList(v8Model.GetGraphicElementsP(), v8mm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledFileConverter::ConvertRootModel()
    {
    CorrectSpatialTransform(m_rootModelMapping);

    m_rootTrans = m_rootModelMapping.GetTransform();

    _OnConversionStart();

    if (!m_rootFile.IsValid() || nullptr == m_rootModelRef)
        {
        BeAssert(false);
        _OnFatalError();
        return;
        }

    if (!m_rootModelMapping.IsValid())
        {
        Utf8String rootName(GetRootFileName());
        Utf8String outName(GetDgnDb().GetFileName());
        OnFatalError(Converter::IssueCategory::Sync(), Converter::Issue::SeedFileMismatch(), rootName.c_str(), outName.c_str());
        }

    if (IsUpdating())
        {
        if (GetChangeDetector()._ShouldSkipFileByName(*this, GetRootFileName())
         || GetChangeDetector()._ShouldSkipFile(*this, *m_rootFile))
            return;
        }

    _BeginConversion();
    _ConvertLineStyles();
    _ConvertSpatialLevels();
    _ConvertSpatialViews();

    _ConvertElementsInModel(m_rootModelMapping);
    m_hadAnyChanges = true;
    _OnFileComplete(*m_rootFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledFileConverter::ConvertTile(BeFileNameCR tileFileName)
    {
    if (tileFileName.EqualsI(GetRootFileName()) || _FilterTileByName(tileFileName))
        return;

    if (IsUpdating() && GetChangeDetector()._ShouldSkipFileByName(*this, tileFileName))
        return;

    DgnV8Api::DgnFileStatus openStatus;
    DgnFilePtr tileFile = OpenDgnV8File(openStatus, tileFileName);
    if (!tileFile.IsValid())
        {
        ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::CorruptData(), Converter::Issue::NotRecognizedFormat(), nullptr, Utf8String(tileFileName).c_str());
        return;
        }

    if (IsUpdating() && GetChangeDetector()._ShouldSkipFile(*this, *tileFile))
        return;

    BeAssert(_GetIdPolicy(*m_rootFile) == _GetIdPolicy(*tileFile));
    GetRepositoryLinkId(*tileFile);

    _ConvertSpatialLevelTable(*tileFile); // This actually checks that there are NO levels to be converted -- that all levels in the tile's level table were already found in the root's level table. 

    Bentley::DgnModelPtr thisModel = tileFile->LoadRootModelById(nullptr, GetDefaultModelId(*tileFile));
    if (!thisModel.IsValid())
        {
        ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::CorruptData(), Converter::Issue::CannotLoadModel(), nullptr, Utf8String(tileFileName).c_str());
        return;
        }

    auto v8mm = MapDgnV8ModelToDgnDbModel(*thisModel, m_rootModelMapping.GetDgnModel().GetModelId());

    _ConvertElementsInModel(v8mm);

    _OnFileComplete(*tileFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledFileConverter::_ConvertSpatialLevels()
    {
    _ConvertSpatialLevelTable(*m_rootFile);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
void TiledFileConverter::_ConvertLineStyles()
    {
    ConvertAllLineStyles(*m_rootFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool TiledFileConverter::ChangeDetectorForTiles::_ShouldSkipLevel(DgnCategoryId&, Converter& converter, DgnV8Api::LevelHandle const& v8Level, 
                                              DgnV8FileR v8File, Utf8StringCR dbCategoryName)
    {
    if (&m_rootFile == &v8File)
        return false;

    converter.ReportIssueV(Converter::IssueSeverity::Fatal, Converter::IssueCategory::CorruptData(), Converter::Issue::LevelNotFoundInRoot(), 
                             nullptr, Utf8String(v8Level.GetName()).c_str(),
                            Utf8String((WCharCP)v8File.GetFileName().c_str()).c_str(), Utf8String(m_rootFile.GetFileName().c_str()).c_str());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledFileConverter::_ConvertSpatialViews()
    {
    auto viewGroup = m_rootFile->GetViewGroupsR().FindByModelId(GetRootModelP()->GetModelId(), true, -1);
    if (viewGroup.IsNull())
        {
        DgnV8Api::ViewGroupStatus vgStatus;
        if (DgnV8Api::VG_Success != (vgStatus = DgnV8Api::ViewGroup::Create(viewGroup, *GetRootModelP(), true, NULL, true)))
            return;
        }

    SpatialViewFactory vf(*this);
    ConvertViewGroup(m_defaultViewId, *viewGroup, m_rootTrans, vf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledFileConverter::_OnFileComplete(DgnV8FileR v8File)
    {
    if (IsUpdating())
        {
        GetChangeDetector()._DetectDeletedElementsInFile(*this, v8File);
        GetChangeDetector()._DetectDeletedElementsEnd(*this);
        GetChangeDetector()._DetectDeletedModelsEnd(*this);

        auto rlinkEd = GetDgnDb().Elements().GetForEdit<RepositoryLink>(GetRepositoryLinkId(v8File));
        auto rlinkXsa = SyncInfo::RepositoryLinkExternalSourceAspect::GetAspectForEdit(*rlinkEd);
        rlinkXsa.Update(GetSyncInfo().ComputeFileInfo(v8File));
        rlinkEd->Update();
        }

    GetDgnDb().Elements().ClearCache();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledFileConverter::_BeginConversion()
    {
    if (!GetImportJob().IsValid() || (GetImportJob().GetConverterType() != ResolvedImportJob::ConverterType::TiledFile))
        {
        OnFatalError();
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledFileConverter::_FinishConversion()
    {
    if (!IsUpdating())
        {
        EmbedFilesInSource(GetRootFileName());
        }
    }

END_DGNDBSYNC_DGNV8_NAMESPACE