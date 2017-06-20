/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/TiledFileConverter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
SyncInfo::ImportJob TiledFileConverter::GenerateImportJobInfo()
    {
    SyncInfo::ImportJob importJob;
    importJob.SetV8ModelSyncInfoId(m_rootModelMapping.GetV8ModelSyncInfoId());
    importJob.SetPrefix(m_params.GetNamePrefix());
    importJob.SetType(SyncInfo::ImportJob::Type::TiledFile);
    return importJob;
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
    //  Open the root V8File
    DgnV8Api::DgnFileStatus openStatus;    
    m_rootFile = OpenDgnV8File(openStatus, GetRootFileName());
    if (!m_rootFile.IsValid())
        return openStatus;
    
    //  Identify the root model
    DgnV8Api::ModelId id = GetDefaultModelId(*m_rootFile);

    //  Load the root model
    m_rootModelRef = m_rootFile->LoadRootModelById((Bentley::StatusInt*)&openStatus, id);
    if (NULL == m_rootModelRef)
        return openStatus;

    //  Note that, for now, we do not support reprojection in a tiled file conversion
    ComputeTransformAndGlobalOriginFromRootModel(*m_rootModelRef->GetDgnModelP());

    if (!ShouldConvertToPhysicalModel(*GetRootModelP()))
        {
        ReportError(Converter::IssueCategory::Unsupported(), Converter::Issue::RootModelMustBePhysical(), Converter::IssueReporter::FmtModel(*GetRootModelP()).c_str());
        OnFatalError();
        return DgnV8Api::DGNFILE_STATUS_UnknownError;
        }

    SetLineStyleConverterRootModel(m_rootModelRef->GetDgnModelP());

    return WasAborted() ? DgnV8Api::DGNFILE_STATUS_UnknownError: DgnV8Api::DGNFILE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping TiledFileConverter::_GetModelForDgnV8Model(DgnV8ModelRefCR v8ModelRef, TransformCR)
    {
    if (IsUpdating())
        {
        ResolvedModelMapping res = GetModelFromSyncInfo(v8ModelRef, m_rootTrans);
        if (res.IsValid())
            {
            GetChangeDetector()._OnModelSeen(*this, res);
            return res;
            }

        // not found in syncinfo => treat as insert
        }

    DgnV8ModelR v8Model = *v8ModelRef.GetDgnModelP();

    ResolvedModelMapping unresolvedMapping(v8Model);

    Utf8String newModelName = _ComputeModelName(v8Model).c_str();

    // Map in the DgnV8 model.
    DgnModelId modelId = _MapModelIntoProject(v8Model, newModelName.c_str(), v8ModelRef.AsDgnAttachmentCP());
    if (!modelId.IsValid())
        {
        return unresolvedMapping;
        }

    SyncInfo::V8ModelMapping mapping;
    auto rc = m_syncInfo.InsertModel(mapping, modelId, v8Model, m_rootTrans);
    if (SUCCESS != rc)
        {
        BeAssert(false);
        ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), IssueReporter::FmtModel(v8Model).c_str());
        OnFatalError();
        return unresolvedMapping;
        }

    if (_WantProvenanceInBim())
        DgnV8ModelProvenance::Insert(modelId, mapping.GetV8FileSyncInfoId().GetValue(), mapping.GetV8ModelId().GetValue(), mapping.GetV8Name(), GetDgnDb());

    DgnModelPtr model = m_dgndb->Models().GetModel(mapping.GetModelId());
    if (!model.IsValid())
        {
        ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), IssueReporter::FmtModel(v8Model).c_str());
        OnFatalError();
        return unresolvedMapping;
        }
    BeAssert(model->GetRefCount() > 0); // DgnModels holds references to all models that it loads

    auto v8mm = ResolvedModelMapping(*model, v8Model, mapping);

    GetChangeDetector()._OnModelInserted(*this, v8mm, v8ModelRef.AsDgnAttachmentCP());
    GetChangeDetector()._OnModelSeen(*this, v8mm);
    m_monitor->_OnModelInserted(v8mm, v8ModelRef.AsDgnAttachmentCP());

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
        ResolvedModelMapping res = GetModelFromSyncInfo(v8Model, m_rootTrans);
        if (res.IsValid())
            {
            GetChangeDetector()._OnModelSeen(*this, res);
            return res;
            }

        // not found in syncinfo => treat as insert
        }

    _GetV8FileIntoSyncInfo(*v8Model.GetDgnFileP(), _GetIdPolicy(*v8Model.GetDgnFileP()));

    ResolvedModelMapping unresolved(v8Model);

    SyncInfo::V8ModelMapping mapping;
    auto rc = m_syncInfo.InsertModel(mapping, targetModelId, v8Model, m_rootTrans);
    BeAssert(SUCCESS == rc);

    auto model = m_dgndb->Models().GetModel(targetModelId);
    if (!model.IsValid())
        return unresolved;

    ResolvedModelMapping v8mm(*model, v8Model, mapping);

    GetChangeDetector()._OnModelInserted(*this, v8mm, nullptr);
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

    GetDgnDb().Memory().PurgeUntil(1024*1024);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledFileConverter::ConvertRootModel()
    {
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
    _GetV8FileIntoSyncInfo(*tileFile, _GetIdPolicy(*m_rootFile));   // make sure the tile file is registered

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

        SyncInfo::V8FileProvenance prov(v8File, GetSyncInfo(), GetCurrentIdPolicy());
        prov.Update();
        }

    GetDgnDb().Memory().PurgeUntil(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledFileConverter::_BeginConversion()
    {
    if (!GetImportJob().IsValid() || (GetImportJob().GetConverterType() != SyncInfo::ImportJob::Type::TiledFile))
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