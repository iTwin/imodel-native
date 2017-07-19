/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Updater.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

static bool s_doFileSaveTimeCheck = true;

// We enter this namespace in order to avoid having to qualify all of the types, such as bmap, that are common
// to bim and v8. The problem is that the V8 Bentley namespace is shifted in.
BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_Prepare(Converter& c)
    {
    m_byIdIter   = new SyncInfo::ByV8ElementIdIter(c.GetDgnDb());
    m_byHashIter = new SyncInfo::ByHashIter(c.GetDgnDb());

    BeAssert (c.GetSyncInfo().IsValid());
    BeAssert (!c.WasAborted());

    c.PopulateRangePartIdMap(); // Populate range to partId map for update...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_Cleanup(Converter& c)
    {
    DELETE_AND_CLEAR(m_byHashIter);
    DELETE_AND_CLEAR(m_byIdIter);
    m_elementsSeen.clear();
    c.GetRangePartIdMap().clear();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeDetector::~ChangeDetector()
    {
    // ensure that we release the statements held by the "Iter" member variables, in case of abort.
    DELETE_AND_CLEAR(m_byHashIter);
    DELETE_AND_CLEAR(m_byIdIter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::CheckNoECSchemaChanges()
    {
    bmap<Utf8String, uint32_t> syncInfoChecksums;
    GetSyncInfo().RetrieveECSchemaChecksums(syncInfoChecksums);

    for (auto& modelMapping : m_v8ModelMappings)
        CheckECSchemasForModel(modelMapping.GetV8Model(), syncInfoChecksums);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
void Converter::CheckECSchemasForModel(DgnV8ModelR v8Model, bmap<Utf8String, uint32_t>& syncInfoChecksums)
    {
    auto& dgnv8EC = DgnV8Api::DgnECManager::GetManager();
    const DgnV8Api::ReferencedModelScopeOption modelScopeOption = DgnV8Api::REFERENCED_MODEL_SCOPE_None;

    Bentley::bvector<DgnV8Api::SchemaInfo> v8SchemaInfos;
    dgnv8EC.DiscoverSchemasForModel(v8SchemaInfos, v8Model, DgnV8Api::ECSCHEMAPERSISTENCE_Stored, modelScopeOption);
    if (v8SchemaInfos.empty())
        return;

    for (auto& v8SchemaInfo : v8SchemaInfos)
        {
        bmap<Utf8String, uint32_t>::const_iterator syncEntry = syncInfoChecksums.find(Utf8String(v8SchemaInfo.GetSchemaName()));
        if (syncEntry == syncInfoChecksums.end())
            {
            Utf8PrintfString msg("v8 schema '%s' not found in original DgnDb", Utf8String(v8SchemaInfo.GetSchemaName()).c_str());
            ReportSyncInfoIssue(IssueSeverity::Fatal, IssueCategory::InconsistentData(), Issue::Error(), msg.c_str());
            OnFatalError(IssueCategory::InconsistentData());
            return;
            }

        Bentley::Utf8String schemaXml;
        uint32_t checksum = -1;
        if (v8SchemaInfo.IsStoredSchema())
            {
            Bentley::WString schemaXmlW;
            auto stat = dgnv8EC.LocateSchemaXmlInModel(schemaXmlW, v8SchemaInfo, ECObjectsV8::SCHEMAMATCHTYPE_Exact, v8Model, modelScopeOption);
            if (stat != BentleyApi::SUCCESS)
                {
                Utf8PrintfString msg("Could not read v8 ECSchema XML for '%s'.", Utf8String(v8SchemaInfo.GetSchemaName()).c_str());
                ReportSyncInfoIssue(IssueSeverity::Fatal, IssueCategory::MissingData(), Issue::Error(), msg.c_str());
                OnFatalError(IssueCategory::MissingData());
                return;
                }

            schemaXml = Bentley::Utf8String(schemaXmlW);
            const size_t xmlByteSize = schemaXml.length() * sizeof(Utf8Char);
            checksum = BECN::ECSchema::ComputeSchemaXmlStringCheckSum(schemaXml.c_str(), xmlByteSize);
            }
        else
            {
            // handle external schemas
            //TBD: DgnECManager doesn't seem to allow to just return the schema xml (Is this possible somehow?)
            //So we need to get the ECSchema and then serialize it to a string.
            //(we need the string anyways as this is the only way to marshal the schema from v8 to Graphite)
            auto externalSchema = dgnv8EC.LocateExternalSchema(v8SchemaInfo, ECObjectsV8::SCHEMAMATCHTYPE_Exact);
            if (externalSchema == nullptr)
                {
                Utf8PrintfString msg("Could not locate external v8 ECSchema '%s'", Utf8String(v8SchemaInfo.GetSchemaName()).c_str());
                ReportSyncInfoIssue(IssueSeverity::Fatal, IssueCategory::MissingData(), Issue::Error(), msg.c_str());
                OnFatalError(IssueCategory::MissingData());
                return;
                }
            if (ECObjectsV8::SCHEMA_WRITE_STATUS_Success != externalSchema->WriteToXmlString(schemaXml))
                {
                Utf8PrintfString msg("Could not serialize external v8 ECSchema '%s'", Utf8String(v8SchemaInfo.GetSchemaName()).c_str());
                ReportSyncInfoIssue(IssueSeverity::Fatal, IssueCategory::CorruptData(), Issue::Error(), "Could not serialize external v8 ECSchema.");
                OnFatalError(IssueCategory::CorruptData());
                return;
                }
            checksum = v8SchemaInfo.GetSchemaKey().m_checkSum;
            }
        if (checksum != syncEntry->second)
            {
            Utf8PrintfString msg("v8 ECSchema '%s' checksum is different from stored schema", Utf8String(v8SchemaInfo.GetSchemaName()).c_str());
            ReportSyncInfoIssue(IssueSeverity::Fatal, IssueCategory::InconsistentData(), Issue::ConvertFailure(), msg.c_str());
            OnFatalError(IssueCategory::InconsistentData());
            return;
            }


        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedImportJob Converter::GetResolvedImportJob(SyncInfo::ImportJob const& importJob)
    {
    auto jobSubject = GetDgnDb().Elements().Get<Subject>(importJob.GetSubjectId());
    if (!jobSubject.IsValid())
        {
        // *** WIP_IMPORT_JOB -- what to do if somebody has deleted the subject?
        return ResolvedImportJob();
        }

    return ResolvedImportJob(importJob, *jobSubject);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::CheckModelUnitsUnchanged(DgnV8ModelR rootModel, TransformCR rootTrans)
    {
    // Note: FindV8ModelMapping will fail if rootTrans (i.e., units scaling) is different from what it was at create time.

    SyncInfo::V8ModelMapping modelInfo;
    if (BSISUCCESS != GetSyncInfo().FindModel(&modelInfo, rootModel, &rootTrans, GetCurrentIdPolicy())
        || modelInfo.GetV8ModelSyncInfoId() != GetImportJob().GetV8ModelSyncInfoId())
        {
        ReportSyncInfoIssue(IssueSeverity::Fatal, IssueCategory::Sync(), Issue::RootModelChanged(), "");
        OnFatalError();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeDetector::_IsElementChanged(SearchResults& res, Converter& converter, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, 
                                       T_SyncInfoElementFilter* filter)
    {
    res.m_currentElementProvenance = SyncInfo::ElementProvenance(v8eh, converter.GetSyncInfo(), converter.GetCurrentIdPolicy());
    
    SyncInfo::ElementIterator* iter;
    if (converter.GetCurrentIdPolicy() == StableIdPolicy::ById)
        {
        iter = m_byIdIter;
        m_byIdIter->Bind(v8mm.GetV8ModelSyncInfoId(), v8eh.GetElementId());
        }
    else
        {
        iter = m_byHashIter;
        m_byHashIter->Bind(v8mm.GetV8ModelSyncInfoId(), res.m_currentElementProvenance.m_hash);
        }

    auto found = iter->begin();
    if (nullptr != filter)
        {
        while ((found != iter->end()) && !(*filter)(found, converter))
            ++found;
        }
    if (found == iter->end())
        {
        // we never saw this element before, treat it as a new element. 
        // or maybe it was previously discarded (SyncInfo::WasElementDiscarded). 
        // Even so, give the converter another shot at it. Maybe it will convert it this time.
        res.m_changeType = ChangeType::Insert;
        }
    else
        {
        //  This V8 element was previously mapped to at least one element in the BIM. See if the V8 element has changed.
        res.m_v8ElementMapping = found.GetV8ElementMapping();
        res.m_changeType = found.GetProvenance().IsSame(res.m_currentElementProvenance)? ChangeType::None: ChangeType::Update;
        }

    iter->GetStatement()->Reset();  // NB: don't leave the iterator in an active state!

    return (ChangeType::None != res.m_changeType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeDetector::_ShouldSkipFileByName(Converter& converter, BeFileNameCR file) 
    {
    if (!converter._GetParams().GetSkipUnchangedFiles())
        return false;

    if (converter.GetSyncInfo().HasDiskFileChanged(file))
        return false;

    SyncInfo::V8FileProvenance prov(file, converter.GetSyncInfo(), converter.GetCurrentIdPolicy());
    if (prov.FindByName(false))
        {
        SyncInfo::ModelIterator it(converter.GetDgnDb(), "V8FileSyncInfoId=?");
        it.GetStatement()->BindInt(1, prov.m_syncId.GetValue());
        for (auto entry = it.begin(); entry != it.end(); ++entry)
            {
            m_v8ModelsSkipped.insert(entry.GetV8ModelSyncInfoId());
            }
        }

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("skip %s (disk file lastmod time unchanged)", Utf8String(file).c_str());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeDetector::_ShouldSkipFile(Converter& converter, DgnV8FileCR v8file)
    {
    // if it hasn't changed per the "last saved time", don't bother with it.
    if (!s_doFileSaveTimeCheck || converter.GetSyncInfo().HasLastSaveTimeChanged(v8file))
        return false;

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("skip %s (dgnfile lastmod time unchanged)", Utf8String((WCharCP)v8file.GetFileName().c_str()).c_str());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_OnModelInserted(Converter& converter, ResolvedModelMapping const& v8mm, DgnV8Api::DgnAttachment const*)
    {
    m_newlyDiscoveredModels.insert(v8mm.GetV8ModelSyncInfoId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeDetector::_AreContentsOfModelUnChanged(Converter& converter, ResolvedModelMapping const& v8mm)
    {
    if (m_newlyDiscoveredModels.find(v8mm.GetV8ModelSyncInfoId()) != m_newlyDiscoveredModels.end())
        return false;

    if (!_ShouldSkipFile(converter, *v8mm.GetV8Model().GetDgnFileP()))
        return false;

    m_v8ModelsSkipped.insert(v8mm.GetV8ModelSyncInfoId());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_OnModelSeen(Converter& converter, ResolvedModelMapping const& v8mm)
    {
    m_v8ModelsSeen.insert(v8mm.GetV8ModelSyncInfoId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_DetectDeletedModels(Converter& converter, SyncInfo::ModelIterator& iter)
    {
    // *** NB: This alogorithm *infers* that a model was deleted in V8 if we did not see it during processing.
    //          This inference is valid only if we know that a) we saw all models and/or files, and b) they were all registered in m_v8Files/ModelsSkipped or m_v8ModelsSeen.

    // iterate over all of the previously found models to determine if any of 
    // them were missing this time around. Those models and their constituent Models must to be deleted.
    for (auto wasModel=iter.begin(); wasModel!=iter.end(); ++wasModel)
        {
        if (m_v8ModelsSeen.find(wasModel.GetV8ModelSyncInfoId()) == m_v8ModelsSeen.end())
            {
            if (m_v8ModelsSkipped.find(wasModel.GetV8ModelSyncInfoId()) != m_v8ModelsSkipped.end())
                continue;   // we skipped this V8 model, so we don't expect to see it in m_v8ModelsSeen

            // not found, delete this model
            DgnModelId deleteModelId = wasModel.GetModelId();
            LOG.tracev("Delete model %lld", deleteModelId.GetValue());
            auto model = converter.GetDgnDb().Models().GetModel(deleteModelId);
            converter.GetMonitor()._OnModelDelete(*model, wasModel.GetMapping());
            model->Delete();
            converter.GetSyncInfo().DeleteModel(wasModel.GetV8ModelSyncInfoId());

            if (converter._WantProvenanceInBim())
                DgnV8ModelProvenance::Delete(deleteModelId, converter.GetDgnDb());

            // Note that DetectDeletedElements will take care of detecting and deleting the elements that were in the V8 model.
            }
        }    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_DetectDeletedElements(Converter& converter, SyncInfo::ElementIterator& iter)
    {
    // *** NB: This alogorithm *infers* that an element was deleted in V8 if we did not see it, its model, or its file during processing.
    //          This inference is valid only if we know that a) we saw all models and/or files, and b) they were all registered in m_v8Files/ModelsSkipped or m_elementsSeen.

    // iterate over all of the previously converted elements from the syncinfo.
    for (auto elementInSyncInfo=iter.begin(); elementInSyncInfo!=iter.end(); ++elementInSyncInfo)
        {
        auto previouslyConvertedElementId = elementInSyncInfo.GetElementId();
        if (m_elementsSeen.Contains(previouslyConvertedElementId)) // if update encountered at least one V8 element that was mapped to this BIM element,
            continue;   // keep this BIM element alive

        if (m_v8ModelsSkipped.find(elementInSyncInfo.GetV8ModelSyncInfoId()) != m_v8ModelsSkipped.end()) // if we skipped this whole V8 model (e.g., because it was unchanged),
            continue;   // we don't expect any element from it to be in m_elementsSeen. Keep them all alive.

        if (converter.GetSyncInfo().IsMappedToSameV8Element(previouslyConvertedElementId, m_elementsSeen)) // if update encountered at least one V8 element that this element was mapped to, 
            continue;   // infer that this is a child of an assembly, and the assembly parent is in m_elementsSeen.

        // We did not encounter the V8 element that was mapped to this BIM element. We infer that the V8 element 
        // was deleted. Therefore, the update to the BIM is to delete the corresponding BIM element.
        LOG.tracev("Delete element %lld", previouslyConvertedElementId.GetValue());

        converter._OnElementBeforeDelete(previouslyConvertedElementId);
        converter.GetDgnDb().Elements().Delete(previouslyConvertedElementId);
        converter.GetSyncInfo().DeleteElement(previouslyConvertedElementId);

        if (converter._WantProvenanceInBim())
            DgnV8ElementProvenance::Delete(previouslyConvertedElementId, converter.GetDgnDb());

        converter._OnElementConverted(elementInSyncInfo.GetElementId(), nullptr, Converter::ChangeOperation::Delete);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_DetectDeletedElementsInFile(Converter& converter, DgnV8FileR v8File)
    {
    // iterate over all of the previously found elements from the syncinfo to determine if any of 
    // them were missing this time around. Those elements need to be deleted.
    SyncInfo::ModelIterator modelsInFile(converter.GetDgnDb(), "V8FileSyncInfoId=?");
    modelsInFile.GetStatement()->BindInt(1, Converter::GetV8FileSyncInfoIdFromAppData(v8File).GetValue());
    for (auto modelInFile = modelsInFile.begin(); modelInFile != modelsInFile.end(); ++modelInFile)
        {
        SyncInfo::ElementIterator elementsInModel(converter.GetDgnDb(), "V8ModelSyncInfoId=?");
        elementsInModel.GetStatement()->BindInt(1, modelInFile.GetV8ModelSyncInfoId().GetValue());
        _DetectDeletedElements(converter, elementsInModel);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetector::_DetectDeletedModelsInFile(Converter& converter, DgnV8FileR v8File)
    {
    SyncInfo::ModelIterator iter(converter.GetDgnDb(), "V8FileSyncInfoId=?");
    iter.GetStatement()->BindInt(1, Converter::GetV8FileSyncInfoIdFromAppData(v8File).GetValue());
    _DetectDeletedModels(converter, iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_OnSourceFileDeleted()
    {
    Utf8String uniqueName = GetSyncInfo().GetUniqueName(_GetParams().GetInputFileName());

    SyncInfo::FileIterator files(GetDgnDb(), nullptr);
    for (auto file = files.begin(); file != files.end(); ++file)
        {
        if (file.GetUniqueName() == uniqueName)
            {
            SyncInfo::ModelIterator modelsInFile(GetDgnDb(), "V8FileSyncInfoId=?");
            modelsInFile.GetStatement()->BindInt(1, file.GetV8FileSyncInfoId().GetValue());
            GetChangeDetector()._DetectDeletedModels(*this, modelsInFile);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool CreatorChangeDetector::_IsElementChanged(SearchResults& res, Converter& converter, DgnV8EhCR v8eh, 
                                              ResolvedModelMapping const& v8mm, T_SyncInfoElementFilter*)
    {
    res.m_currentElementProvenance = SyncInfo::ElementProvenance(v8eh, converter.GetSyncInfo(), converter.GetCurrentIdPolicy());
    res.m_changeType = ChangeType::Insert;
    return true;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
