/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/RootModelConverter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

// We enter this namespace in order to avoid having to qualify all of the types, such as bmap, that are common
// to bim and v8. The problem is that the V8 Bentley namespace is shifted in.
BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus Converter::ConvertECRelationships(DgnV8Api::ElementHandle const& v8Element)
    {
    auto& v8ECManager = DgnV8Api::DgnECManager::GetManager();
    DgnV8Api::RelationshipEntryVector relationships;
    v8ECManager.FindRelationshipEntriesOnElement(v8Element.GetElementRef(), relationships);

    DgnDbR dgndb = GetDgnDb();
    SyncInfo::V8FileSyncInfoId fileId = GetV8FileSyncInfoIdFromAppData(*v8Element.GetDgnFileP());

    for (DgnV8Api::RelationshipEntry const& entry : relationships)
        {
        //schemas not captured in sync info are system schemas which we don't consider during conversion
        if (!GetSyncInfo().ContainsECSchema(Utf8String(entry.RelationshipSchemaName.c_str()).c_str()))
            continue;

        V8ECInstanceKey v8SourceKey(ECClassName(Utf8String(entry.SourceSchemaName.c_str()).c_str(), Utf8String(entry.SourceClassName.c_str()).c_str()),
                                    entry.SourceInstanceId.c_str());
        V8ECInstanceKey v8TargetKey(ECClassName(Utf8String(entry.TargetSchemaName.c_str()).c_str(), Utf8String(entry.TargetClassName.c_str()).c_str()),
                                    entry.TargetInstanceId.c_str());
        ECClassName v8RelName(Utf8String(entry.RelationshipSchemaName.c_str()).c_str(), Utf8String(entry.RelationshipClassName.c_str()).c_str());
        Utf8String v8RelFullClassName = v8RelName.GetClassFullName();

        BisConversionRule rule;
        bool hasSecondary;
        if (!V8ECClassInfo::TryFind(rule, dgndb, v8RelName, hasSecondary))
            {
            BeAssert(false && "V8ECClassInfo should exist for relationship classes.");
            continue;
            }

        if (BisConversionRuleHelper::IgnoreInstance(rule))
            {
            Utf8String errorMsg;
            errorMsg.Sprintf("Skipped v8 '%s' relationship ECInstance because its class was ignored during schema conversion. See ECSchema conversion log entries above.",
                             v8RelName.GetClassFullName().c_str());
            ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), errorMsg.c_str());
            continue;
            }

        bool sourceInstanceIsElement = false;
        BeSQLite::EC::ECInstanceKey sourceInstanceKey = ECInstanceInfo::Find(sourceInstanceIsElement, dgndb, fileId, v8SourceKey);
        bool targetInstanceIsElement = false;
        BeSQLite::EC::ECInstanceKey targetInstanceKey = ECInstanceInfo::Find(targetInstanceIsElement, dgndb, fileId, v8TargetKey);

        if (IsUpdating())
            {
            if (DoesRelationshipExist(v8RelFullClassName, sourceInstanceKey, targetInstanceKey))
                {
                continue;
                }
            }
        ECDiagnostics::LogV8RelationshipDiagnostics(dgndb, v8RelName, v8SourceKey, sourceInstanceKey.IsValid(), sourceInstanceIsElement, v8TargetKey, targetInstanceKey.IsValid(), targetInstanceIsElement);

        if (!sourceInstanceKey.IsValid() || !targetInstanceKey.IsValid())
            {
            Utf8CP failingEndStr = nullptr;
            if (!sourceInstanceKey.IsValid() && !targetInstanceKey.IsValid())
                failingEndStr = "source and target ECInstances";
            else
                failingEndStr = !sourceInstanceKey.IsValid() ? "source ECInstance" : "target ECInstance";

            Utf8String errorMsg;
            errorMsg.Sprintf("Could not find %s for relationship '%s' (Source: %s|%s Target %s|%s).",
                             failingEndStr, v8RelFullClassName.c_str(),
                             v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                             v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId());
            ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), errorMsg.c_str());
            continue;
            }

        ECN::ECClassCP relClass = GetDgnDb().Schemas().GetClass(v8RelName.GetSchemaName(), v8RelName.GetClassName());
        if (relClass == nullptr || !relClass->IsRelationshipClass())
            {
            Utf8String error;
            error.Sprintf("Failed to convert instance of ECRelationshipClass %s. The class doesn't exist in the BIM file.", v8RelFullClassName.c_str());
            ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                        error.c_str());
            continue;
            }

        // If the relationship class inherits from one of the two biscore base relationship classes, then it is a link table relationship, and can use the API
        if (relClass->Is(BIS_ECSCHEMA_NAME, BIS_REL_ElementRefersToElements) || relClass->Is(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsMultiAspects))
            {
            BeSQLite::EC::ECInstanceKey relKey;
            if (BE_SQLITE_OK != GetDgnDb().InsertLinkTableRelationship(relKey, *relClass->GetRelationshipClassCP(), sourceInstanceKey.GetInstanceId(), targetInstanceKey.GetInstanceId()))
                {
                Utf8String dgndbError = GetDgnDb().GetLastError();
                Utf8String errorMsg;
                errorMsg.Sprintf("Failed to convert ECRelationship '%s' from element %" PRIu64 " in file '%s' "
                                 "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). "
                                 "Insertion into target BIM file failed.%s%s",
                                 v8RelFullClassName.c_str(),
                                 v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
                                 v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                                 sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                                 v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
                                 targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str(),
                                 dgndbError.empty() ? "" : " Reason: ",
                                 dgndbError.empty() ? "" : dgndbError.c_str());

                ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                            errorMsg.c_str());
                }
            continue;
            }

        ECN::ECClassCP targetClass = GetDgnDb().Schemas().GetClass(targetInstanceKey.GetClassId());
        // Otherwise, the converter should have created a navigation property on the target class, so we need to set the target instance's ECValue
        ECN::ECPropertyP prop = targetClass->GetPropertyP(relClass->GetName().c_str());
        if (nullptr == prop)
            {
            Utf8String errorMsg;
            errorMsg.Sprintf("Unable to find NavigationECProperty '%s' on Target-Constraint ECClass '%s'.  This relationship is not derived from a BisCore link table relationship "
                             "and therefore the conversion process should have created a NavigationECProperty on the ECClass."
                             "Failed to convert ECRelationship '%s' from element %" PRIu64 " in file '%s' "
                             "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). "
                             "Insertion into target BIM file failed.",
                             relClass->GetName().c_str(), targetClass->GetFullName(),
                             v8RelFullClassName.c_str(),
                             v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
                             v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                             sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                             v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
                             targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());
            ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                        errorMsg.c_str());

            continue;
            }
        ECN::NavigationECPropertyP navProp = prop->GetAsNavigationPropertyP();
        if (nullptr == navProp)
            {
            Utf8String errorMsg;
            errorMsg.Sprintf("Unable to find NavigationECProperty '%s' on Target-Constraint ECClass '%s'.  This relationship is not derived from a BisCore link table relationship "
                             "and therefore the conversion process should have created a NavigationECProperty on the ECClass."
                             "Failed to convert ECRelationship '%s' from element %" PRIu64 " in file '%s' "
                             "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). "
                             "Insertion into target BIM file failed.",
                             relClass->GetName().c_str(), targetClass->GetFullName(),
                             v8RelFullClassName.c_str(),
                             v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
                             v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                             sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                             v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
                             targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());
            ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                        errorMsg.c_str());

            continue;
            }
        ECN::ECValue val;
        val.SetNavigationInfo((BeInt64Id) targetInstanceKey.GetInstanceId().GetValue(), relClass->GetRelationshipClassCP());

        DgnElementPtr element = m_dgndb->Elements().GetForEdit<DgnElement>(DgnElementId(targetInstanceKey.GetInstanceId().GetValue()));
        if (targetClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementAspect))
            {
            DgnElement::MultiAspect* aspect = DgnElement::MultiAspect::GetAspectP(*element, *targetClass, targetInstanceKey.GetInstanceId());
            if (nullptr == aspect)
                {
                Utf8String errorMsg;
                errorMsg.Sprintf("Unable to get ElementAspect."
                                 "and therefore the conversion process should have created a NavigationECProperty on the ECClass."
                                 "Failed to convert ECRelationship '%s' from element %" PRIu64 " in file '%s' "
                                 "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). "
                                 "Insertion into target BIM file failed.",
                                 v8RelFullClassName.c_str(),
                                 v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
                                 v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                                 sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                                 v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
                                 targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());
                ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                            errorMsg.c_str());
                continue;
                }
            if (DgnDbStatus::Success != aspect->SetPropertyValue(navProp->GetName().c_str(), val))
                {
                Utf8String errorMsg;
                errorMsg.Sprintf("Failed to set NavigationECProperty on Target ElementAspect ECInstance for ECRelationship '%s' from element %" PRIu64 " in file '%s' "
                                 "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). ",
                                 v8RelFullClassName.c_str(),
                                 v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
                                 v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                                 sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                                 v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
                                 targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());

                ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                            errorMsg.c_str());
                continue;
                }
            element->Update();
            }
        else
            {
            if (DgnDbStatus::Success != element->SetPropertyValue(navProp->GetName().c_str(), val))
                {
                Utf8String errorMsg;
                errorMsg.Sprintf("Failed to set NavigationECProperty on Target ECInstance for ECRelationship '%s' from element %" PRIu64 " in file '%s' "
                                 "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). ",
                                 v8RelFullClassName.c_str(),
                                 v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
                                 v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                                 sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                                 v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
                                 targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());

                ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                            errorMsg.c_str());
                continue;
                }
            element->Update();
            }
        }

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
bool Converter::DoesRelationshipExist(Utf8StringCR relName, BeSQLite::EC::ECInstanceKey const& sourceInstanceKey, BeSQLite::EC::ECInstanceKey const& targetInstanceKey)
    {

    Utf8String ecsql("SELECT COUNT(*) FROM ");
    ecsql.append(relName).append(" WHERE SourceECInstanceId=? AND TargetECInstanceId=?");
    BeSQLite::EC::CachedECSqlStatementPtr stmt = GetDgnDb().GetPreparedECSqlStatement(ecsql.c_str());
    //Utf8String qplan = GetDgnDb().ExplainQuery();

    if (!stmt.IsValid())
        {
        ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::CorruptData(), Converter::Issue::Message(),
            Utf8PrintfString("%s - failed to prepare", ecsql.c_str()).c_str());
        return false;
        }

    if (BeSQLite::EC::ECSqlStatus::Success != stmt->BindId(1, sourceInstanceKey.GetInstanceId()))
        {
        Utf8PrintfString error("Failed to search for ECRelationship %s. Binding value to SourceECInstanceId (%s) failed. (%s:%s -> %s:%s)", relName.c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                               sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(), targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());
        ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
        return false;
        }

    if (BeSQLite::EC::ECSqlStatus::Success != stmt->BindId(2, targetInstanceKey.GetInstanceId()))
        {
        Utf8PrintfString error("Failed to search for ECRelationship %s. Binding value to TargetECInstanceId (%s) failed.  (%s:%s -> %s:%s)", relName.c_str(), targetInstanceKey.GetInstanceId().ToString().c_str(),
                               sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(), targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());
        ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
        return false;
        }

    if (BeSQLite::BE_SQLITE_ROW != stmt->Step())
        {
        Utf8String errorMsg;
        errorMsg.Sprintf("Failed to search for ECRelationship '%s' "
                         "(Source: (%s:%s) Target (%s:%s)). "
                         "Execution of ECSQL SELECT '%s' failed. (Native SQL: %s)\n",
                         relName.c_str(),
                         sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                         targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str(),
                         stmt->GetECSql(), stmt->GetNativeSql());

        ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                    errorMsg.c_str());
        return false;
        }

    return stmt->GetValueInt(0) > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertSpatialViews()
    {
    if (IsUpdating())
        return;         // pity, but we don't have the logic to *update* previously converted views.

    if (!m_viewGroup.IsValid())
        m_viewGroup = m_rootFile->GetViewGroupsR().FindByModelId(GetRootModelP()->GetModelId(), true, -1);
    if (!m_viewGroup.IsValid())
        {
        DgnV8Api::ViewGroupStatus vgStatus;
        if (DgnV8Api::VG_Success != (vgStatus = DgnV8Api::ViewGroup::Create(m_viewGroup, *GetRootModelP(), true, NULL, true)))
            return;
        }

    SpatialViewFactory vf(*this);

    DgnViewId firstView;
    ConvertViewGroup(firstView, *m_viewGroup, m_rootTrans, vf);

    if (firstView.IsValid() && !m_defaultViewId.IsValid())
        m_defaultViewId = firstView;

    NamedViewCollectionCR namedViews = GetRootV8File()->GetNamedViews();
    for (DgnV8Api::NamedViewPtr namedView : namedViews)
        ConvertNamedView(*namedView, m_rootTrans, vf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::DgnFileStatus RootModelConverter::_InitRootModel()
    {
    // *** NB: Do not create elements (or models) in here. This is running as part of the initialization phase.
    //          Only schema changes are allowed in this phase.

    m_newFilesOk = true;

    // don't bother to convert a DWG master file - let DwgImporter do the job.
    BeFileNameCR rootFileName = GetRootFileName ();
    if (Converter::IsDwgOrDxfFile(rootFileName))
        {
        ReportError (IssueCategory::Unsupported(), Converter::Issue::DwgFileIgnored(), Utf8String(rootFileName.c_str()).c_str());
        return  DgnV8Api::DGNFILE_ERROR_UnknownFormat;
        }

    SetStepName(ProgressMessage::STEP_LOADING_V8());

    m_rootTrans.InitIdentity();
                
    //  Open the root V8File
    DgnV8Api::DgnFileStatus openStatus;    
    m_rootFile = OpenDgnV8File(openStatus, rootFileName);
    if (!m_rootFile.IsValid())
        return openStatus;

    //  Identify the root model
    auto rootModelId = _GetRootModelId();

    //  Load the root model and all of its reference attachments. Let V8 do this, so that we know that it's done correctly and in the same way that MicroStation would do it.
    m_rootModelRef = m_rootFile->LoadRootModelById((Bentley::StatusInt*)&openStatus, rootModelId, /*fillCache*/true, /*loadRefs*/true, /*processAffected*/false);
    if (NULL == m_rootModelRef)
        return openStatus;

    if (DgnV8Api::DGNFILE_STATUS_Success != (openStatus = _ComputeCoordinateSystemTransform()))
        return openStatus;

    m_isRootModelSpatial = ShouldConvertToPhysicalModel(*GetRootModelP());

    if (!m_isRootModelSpatial)
        ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Unsupported(), Converter::Issue::RootModelMustBePhysical(), Converter::IssueReporter::FmtModel(*GetRootModelP()).c_str());

    SetLineStyleConverterRootModel(m_rootModelRef->GetDgnModelP());

    if (WasAborted())
        return DgnV8Api::DGNFILE_STATUS_UnknownError;
    
    // Detect all V8 models. This process also classifies 2d design models and loads and fills drawings and sheets.
    // The of models that we find will be fed into the ECSchema conversion logic. These functions will ALSO enroll the v8files that they find in syncinfo.
    CreateProvenanceTables(); // TRICKY: Call this before anyone calls _GetV8FileIntoSyncInfo
    _GetV8FileIntoSyncInfo(*m_rootFile, _GetIdPolicy(*m_rootFile)); // TRICKY: Before looking for models, register the root file in syncinfo. This starts the process of populating m_v8files. Do NOT CALL GetV8FileSyncInfoId as that will fail to populate m_v8Files in some cases.
    FindSpatialV8Models(*GetRootModelP());
    FindV8DrawingsAndSheets();

#ifndef NDEBUG
    BeAssert((m_v8Files.size() >= 1) && "FindSpatialV8Models should have populated m_v8Files");
    for (auto f : m_v8Files)
        {
        auto cachedSfid = GetV8FileSyncInfoIdFromAppData(*f);
        BeAssert(cachedSfid.IsValid() && "We should have cached the V8FileSyncInfoId for each V8 file that we found");

        SyncInfo::FileById syncInfoFiles(GetDgnDb(), cachedSfid);
        SyncInfo::FileIterator::Entry syncInfoFile = syncInfoFiles.begin();
        BeAssert((syncInfoFile != syncInfoFiles.end()) && "We should be able to look up V8 files in syncinfo by their V8FileSyncInfoId's");
        }
#endif

    m_newFilesOk = false;

    return WasAborted() ? DgnV8Api::DGNFILE_STATUS_UnknownError: DgnV8Api::DGNFILE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialConverterBase::ImportJobLoadStatus SpatialConverterBase::FindJob()
    {
    if (_GetParams().GetBridgeRegSubKey().empty())
        {
        BeAssert(false && "Job registry subkey is a required property of iModelBridge::Params");
        return ImportJobLoadStatus::FailedNotFound;
        }
    BeAssert(m_rootFile.IsValid() && "Must define root file before loading the job");
    BeAssert((nullptr != m_rootModelRef) && "Must define root model before loading the job");

    BeAssert(!GetRepositoryLinkFromAppData(*GetRootV8File()).IsValid());
    WriteRepositoryLink(*GetRootV8File());  // Find the RepositoryLink element for the root file now. This is the order in which the older converter did it.

    m_importJob = FindImportJobForModel(*GetRootModelP());

    if (!m_importJob.IsValid())
        return ImportJobLoadStatus::FailedNotFound;
    
    // *** TRICKY: If this is called by the framework as a check *after* it calls _IntializeJob, then don't change the change detector!
    if (!_HaveChangeDetector() || IsUpdating())
        _SetChangeDetector(true);

    _GetV8FileIntoSyncInfo(*m_rootFile, _GetIdPolicy(*m_rootFile)); // (on update, this just looks up the existing mapping and caches it in memory)

    if (BSISUCCESS != FindRootModelFromImportJob())
        return ImportJobLoadStatus::FailedNotFound;

    ApplyJobTransformToRootTrans();

    DetectRootTransformChange();

    GetOrCreateJobPartitions(); // (on update, this just looks up and caches existing modelids)

    auto hsubj = GetJobHierarchySubject(); // There's only one hierarchy subject for a job. Look it up.
    if (!hsubj.IsValid())
        {
        BeAssert(false);
        return ImportJobLoadStatus::FailedNotFound;
        }
    SetSpatialParentSubject(*hsubj);

    return ImportJobLoadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SpatialConverterBase::FindRootModelFromImportJob()
    {
    // Look up the root model in syncinfo, using the ID saved in the ImportJob record. We can't use GetModelForDgnV8Model, because m_rootTrans might have changed.
    SyncInfo::V8ModelMapping syncInfoModelMapping;
    auto status = GetSyncInfo().GetModelBySyncInfoId(syncInfoModelMapping, m_importJob.GetV8ModelSyncInfoId());
    if (BSISUCCESS != status)
        {
        BeAssert(false);
        ReportError(IssueCategory::CorruptData(), Issue::Error(), "ImportJob has bad V8ModelSyncInfoId");
        return BSIERROR;
        }
    auto rootBimModel = GetDgnDb().Models().GetModel(syncInfoModelMapping.GetModelId());
    if (!rootBimModel.IsValid())
        {
        BeAssert(false);
        ReportError(IssueCategory::CorruptData(), Issue::Error(), "V8ModelMapping has bad DgnModelId");
        return BSIERROR;
        }
    m_rootModelMapping = ResolvedModelMapping(*rootBimModel, *GetRootModelP(), syncInfoModelMapping, nullptr);
    _AddResolvedModelMapping(m_rootModelMapping);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::ApplyJobTransformToRootTrans()
    {
    Transform jobTrans = iModelBridge::GetSpatialDataTransform(_GetParams(), m_importJob.GetSubject());
    if (jobTrans.IsIdentity())
        return;

    // Incorporate the job transform into the root transform. The job transform is just one more ingredient in computing the root transform.

    m_rootTrans = BentleyApi::Transform::FromProduct(jobTrans, m_rootTrans); // NB: pre-multiply!

    auto matrixTolerance = Angle::TinyAngle();
    auto pointTolerance = 10*BentleyApi::BeNumerical::NextafterDelta(jobTrans.ColumnXMagnitude());

    if (!jobTrans.IsEqual(m_importJob.GetImportJob().GetTransform(), matrixTolerance, pointTolerance))
        {
        m_importJob.GetImportJob().SetTransform(jobTrans);
        GetSyncInfo().UpdateImportJob(m_importJob.GetImportJob()); // update syncinfo to record the new baseline
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::DetectRootTransformChange()
    {
    auto matrixTolerance = Angle::TinyAngle();
    auto pointTolerance = 10*BentleyApi::BeNumerical::NextafterDelta(m_rootTrans.ColumnXMagnitude());

    //  Detect if anything about the root GCS/units transform has changed (including the computed root trans and the job trans).
    m_rootTransHasChanged  = !m_rootModelMapping.GetTransform().IsEqual(m_rootTrans, matrixTolerance, pointTolerance);

    if (!m_rootTransHasChanged)
        return;

    // The root transform has changed.

    // We will have to correct all model transforms (later on in the conversion).
    // So, we want the factor, rtc, such that: 
    //  r1 = rtc * r0    
    // Where "r1" is the new root trans, and "r0" is the old root trans. Therefore, 
    //  rtc = r1 * inverse(r0)
    // We can then pre-multiply *all* spatial model transforms by rtc, in order to base them on the new root transform.
    // For example, suppose the transform for a given attachment was 
    //                      r0*a1*...*an
    // We can rebase it on r1 by pre-multiplying by rtc:
    //                  rtc*r0*a1*...*an
    // =   (r1*inverse(r0))*r0*a1*...*an
    // =                    r1*a1*...*an
    // See CorrectSpatialTransform for where we do this.
    auto r0inv = m_rootModelMapping.GetTransform().ValidatedInverse();  // The inverse of the old root trans, aka r0
    if (!r0inv.IsValid())
        {
        BeAssert(false);
        ReportError(IssueCategory::Unsupported(), Issue::Error(), "Root transform cannot be modified - inverse failed");
        return;
        }
    m_rootTransChange = Transform::FromProduct(m_rootTrans, r0inv.Value()); // rtc = r1 * inverse(r0)

    // Now adopt the old root trans to start with.
    // We will use the old root transform in ImportSpatialModels in order to find all of the existing spatial models.
    // Then, we will correct their transforms in CorrectSpatialTransforms.
    m_rootTrans = m_rootModelMapping.GetTransform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::CorrectSpatialTransform(ResolvedModelMapping& rmm)
    {
    if (!m_rootTransHasChanged || !rmm.GetDgnModel().IsSpatialModel())
        return;

    rmm.SetTransform(Transform::FromProduct(m_rootTransChange, rmm.GetTransform())); // See DetectRootTransformChange for how m_rootTransChange was computed
    rmm.GetV8ModelMapping().Update(GetDgnDb()); // (we can now look up the model mapping in syncinfo using the new transform)
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::CorrectSpatialTransforms()
    {
    CorrectSpatialTransform(m_rootModelMapping);

    m_rootTrans = m_rootModelMapping.GetTransform();

    for (auto& rmm : m_v8ModelMappings)
        CorrectSpatialTransform(rmm);

    m_spatialTransformCorrectionsApplied = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::ComputeDefaultImportJobName()
    {
    if (nullptr == GetRootModelP())
        {
        BeAssert(false && "Call InitRootModel first");
        return;
        }

    // Use the document GUID, if available, to ensure a unique Job subject name.
    Utf8String docIdStr;
    auto docGuid = _GetParams().QueryDocumentGuid(_GetParams().GetInputFileName());
    if (docGuid.IsValid())
        docIdStr = docGuid.ToString();
    else
        docIdStr = Utf8String(_GetParams().GetInputFileName());

    Utf8String jobName(_GetParams().GetBridgeRegSubKey());
    jobName.append(":");
    jobName.append(docIdStr.c_str());
    jobName.append(", ");
    jobName.append(Utf8String(GetRootModelP()->GetModelName()));

    DgnCode code = Subject::CreateCode(*GetDgnDb().Elements().GetRootSubject(), jobName.c_str());
    int i=0;
    while (GetDgnDb().Elements().QueryElementIdByCode(code).IsValid())
        {
        Utf8String uniqueJobName(jobName);
        uniqueJobName.append(Utf8PrintfString("%d", ++i).c_str());
        code = Subject::CreateCode(*GetDgnDb().Elements().GetRootSubject(), uniqueJobName.c_str());
        }

    _GetParamsR().SetBridgeJobName(jobName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialConverterBase::ImportJobCreateStatus SpatialConverterBase::InitializeJob(Utf8CP comments, SyncInfo::ImportJob::Type jtype)
    {
    if (_GetParams().GetBridgeRegSubKey().empty())
        {
        BeAssert(false && "Job registry subkey is a required property of iModelBridge::Params");
        return ImportJobCreateStatus::FailedExistingRoot;
        }

    if (IsUpdating())
        {
        BeAssert(false);
        return ImportJobCreateStatus::FailedExistingRoot;
        }

    BeAssert(!GetRepositoryLinkFromAppData(*GetRootV8File()).IsValid());
    WriteRepositoryLink(*GetRootV8File());  // Write the RepositoryLink element for the root file now. This is the order in which the older converter did it.

    Utf8String jobName = _GetParams().GetBridgeJobName();
    if (jobName.empty())
        {
        ComputeDefaultImportJobName();
        jobName = _GetParams().GetBridgeJobName();
        }
    else
        {
        if (!jobName.StartsWithI(_GetParams().GetBridgeRegSubKeyUtf8().c_str()))
            {
            jobName = _GetParams().GetBridgeRegSubKeyUtf8();
            jobName.append(":");
            jobName.append(_GetParams().GetBridgeJobName());
            }
        }

    BeAssert(m_rootFile.IsValid() && "Must define root file before creating the job");
    BeAssert((nullptr != m_rootModelRef) && "Must define root model before creating the job");
    BeAssert(!jobName.empty());

    // Make sure that we don't already have a job for this root ...
    if (FindImportJobForModel(*GetRootModelP()).IsValid())
        return ImportJobCreateStatus::FailedExistingRoot;

    // ... and that we don't already have this V8 model mapped in as a reference to some other root.
    SyncInfo::V8ModelMapping mapping;
    if (BSISUCCESS == GetSyncInfo().FindModel(&mapping, *GetRootModelP(), &m_rootTrans, GetCurrentIdPolicy()))
        return ImportJobCreateStatus::FailedExistingNonRootModel;

    // 1. Look up the root file's syncinfoid. (_InitRootModel has already mapped in the root file.)
    auto fileId = GetV8FileSyncInfoId(*m_rootFile);
#ifndef NDEBUG
    SyncInfo::FileById syncInfoFiles(GetDgnDb(), fileId);
    BeAssert(syncInfoFiles.begin() != syncInfoFiles.end());
#endif

    _SetChangeDetector(false);

    // 2. Create a subject, as a child of the rootsubject
    SubjectPtr ed = Subject::Create(*GetDgnDb().Elements().GetRootSubject(), jobName);

    Json::Value v8JobProps(Json::objectValue);      // V8Bridge-specific job properties - information that is not recorded anywhere else.
    v8JobProps["RootModel"] = Utf8String(m_rootModelRef->GetDgnModelP()->GetModelName());

    JobSubjectUtils::InitializeProperties(*ed, _GetParams().GetBridgeRegSubKeyUtf8(), comments, &v8JobProps);

    SubjectCPtr jobSubject = ed->InsertT<Subject>();
    if (!jobSubject.IsValid())
        return ImportJobCreateStatus::FailedInsertFailure;

    // 3. Set up m_importJob with the subject. That leaves out the syncinfo part, but we don't need that yet. 
    //      We do need m_importJob to be defined and it must have its subject at this point, as GetOrCreateJobPartitions 
    //      and GetModelForDgnV8Model refer to the subject in it.

    m_importJob = ResolvedImportJob(*jobSubject);

    // 4. Create the job-specific stuff in the DgnDb (relative to the job subject).
    GetOrCreateJobPartitions();

    //  ... and create and push the root model's "hierarchy" subject. The root model's physical partition will be a child of that.
    Utf8String mastermodelName = _ComputeModelName(*m_rootModelRef->GetDgnModelP());
    SubjectCPtr hsubj = GetOrCreateModelSubject(GetJobSubject(), mastermodelName, ModelSubjectType::Hierarchy);
    if (!hsubj.IsValid())
        {
        BeAssert(false);
        return ImportJobCreateStatus::FailedInsertFailure;
        }
    SetSpatialParentSubject(*hsubj);

    // 5. Map the root model into the DgnDb. Note that this will generally create a partition, which is relative to the job subject,
    //    So, the job subject and its framework must be created first.
    m_rootModelMapping = GetModelForDgnV8Model(*m_rootModelRef->GetDgnModelP(), m_rootTrans);

    // 6. Now that we have the root model's syncinfo id, we can define the syncinfo part of the importjob.
    SyncInfo::ImportJob importJob;
    importJob.SetV8ModelSyncInfoId(m_rootModelMapping.GetV8ModelSyncInfoId());
    importJob.SetPrefix(_GetNamePrefix());
    importJob.SetType(jtype);
    importJob.SetSubjectId(jobSubject->GetElementId());
    importJob.SetTransform(BentleyApi::Transform::FromIdentity());
    if (BSISUCCESS != GetSyncInfo().InsertImportJob(importJob))
        return ImportJobCreateStatus::FailedExistingRoot;

    m_importJob.GetImportJob() = importJob;     // Update the syncinfo part of the import job

    return ImportJobCreateStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr SpatialConverterBase::GetOrCreateModelSubject(SubjectCR parent, Utf8StringCR modelName, ModelSubjectType stype)
    {
    Json::Value modelProps(Json::nullValue);
    modelProps["Type"] = (ModelSubjectType::Hierarchy==stype)? "Hierarchy": "References";

    for (auto childid : parent.QueryChildren())
        {
        auto subj = GetDgnDb().Elements().Get<Subject>(childid);
        if (subj.IsValid() && modelName.Equals(subj->GetCode().GetValueUtf8CP()) && (modelProps == subj->GetSubjectJsonProperties().GetMember(Subject::json_Model())))
            return subj;
        }

    BeAssert((!IsUpdating() || (ModelSubjectType::Hierarchy != stype)) && "You create a hierarchy subject once when you create the job");

    SubjectPtr ed = Subject::Create(parent, modelName.c_str());

    ed->SetSubjectJsonProperties(Subject::json_Model(), modelProps);

    return ed->InsertT<Subject>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertModels()
    {
    SetStepName(IsUpdating() ? Converter::ProgressMessage::STEP_UPDATING() : 
                               Converter::ProgressMessage::STEP_CREATING(), Utf8String(GetDgnDb().GetFileName()).c_str());

    if (nullptr != m_rootModelRef && m_isRootModelSpatial)
        {
        bool haveFoundSpatialRoot = false;
        ImportSpatialModels(haveFoundSpatialRoot, *m_rootModelRef, m_rootTrans);
        if (WasAborted())
            return;
        CorrectSpatialTransforms();
        }

    _ImportDrawingAndSheetModels(m_rootModelMapping); // we also need to convert all drawing models now, so that we can analyze them for EC content.

    m_syncInfo.SetValid(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::IsFileAssignedToBridge(DgnV8FileCR v8File) const 
    {
    BeFileName fn(v8File.GetFileName().c_str());
    return _GetParams().IsFileAssignedToBridge(fn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::FindSpatialV8Models(DgnV8ModelRefR thisModelRef, bool haveFoundSpatialRoot)
    {
    DgnV8ModelR thisV8Model = *thisModelRef.GetDgnModelP();

    if (!ShouldConvertToPhysicalModel(thisV8Model))
        return;

    DgnV8FileR thisV8File = *thisV8Model.GetDgnFileP();

    GetV8FileSyncInfoId(thisV8File); // Register this FILE in syncinfo. Also populates m_v8files

    bool isThisMyFile = IsFileAssignedToBridge(thisV8File);

    if (haveFoundSpatialRoot && !isThisMyFile)
        return; // we only go through another bridge's territory in order to find our own spatial root. don't follow any references from my spatial root into another bridge's file.

    if (isThisMyFile)
        haveFoundSpatialRoot = true;

    if (!m_spatialModelsSeen.insert(&thisV8Model).second)
        return;

    m_spatialModelsInAttachmentOrder.push_back(&thisV8Model);

    ClassifyNormal2dModels (thisV8File); // Tells me which 2d design models should be treated as spatial models

    if (nullptr == thisModelRef.GetDgnAttachmentsP())
        return; 

    // All attachments to a spatial model are spatial, unless they are specifically DgnModelType::Drawing or DgnModelType::Sheet, which BIM doesn't support.
    ForceAttachmentsToSpatial (*thisModelRef.GetDgnAttachmentsP());

    for (DgnV8Api::DgnAttachment* attachment : *thisModelRef.GetDgnAttachmentsP())
        {                  
        if (nullptr == attachment->GetDgnModelP() || !_WantAttachment(*attachment))
            continue; // missing reference 

        FindSpatialV8Models(*attachment, haveFoundSpatialRoot);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::FindNonSpatialModel(DgnV8ModelRefR v8ModelRef, bool andRefs)
    {
    if (nullptr == v8ModelRef.GetDgnModelP())
        return;

    DgnV8FileR thisV8File = *v8ModelRef.GetDgnFileP();

    if (!IsFileAssignedToBridge(thisV8File))
        return;

    DgnV8ModelR thisV8Model = *v8ModelRef.GetDgnModelP();

    if (ShouldConvertToPhysicalModel(thisV8Model))          // Not going to convert this to a sheet or drawing?
        return;

    if (!m_nonSpatialModelsSeen.insert(&thisV8Model).second)   // Already seen this model?
        return;

    // Build up a list of non-spatial models in the order in which they were found. This is 
    // so that we will (later) import those models in that same order. That is necessary so that
    // the converter will produce the same results -- the same rows in the same order -- as the older converter did.
    // And that is necessary so that we can verify the new converter by matching its results with the results of the old converter.
    m_nonSpatialModelsInModelIndexOrder.push_back(&thisV8Model);
    
    _OnDrawingModelFound(thisV8Model);  // keep this model alive

    if (!GetV8FileSyncInfoIdFromAppData(thisV8File).IsValid())  // Register this FILE in syncinfo.
        {
        GetV8FileSyncInfoId(thisV8File); // populates m_v8files
        _KeepFileAlive(thisV8File);     // keep the file alive
        }

    if (!andRefs)
        return;

    GetAttachments(thisV8Model);

    if (nullptr == v8ModelRef.GetDgnAttachmentsP())
        return;

    for (DgnV8Api::DgnAttachment* attachment : *v8ModelRef.GetDgnAttachmentsP())
        {                  
        if (nullptr == attachment->GetDgnModelP())
            continue; // missing reference 

        FindNonSpatialModel(*attachment, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnV8FileP findOpenV8FileByName(bvector<DgnV8FileP> const& files, BeFileNameCR fn)
    {
    for (auto v8File : files)
        {
        if (fn.EqualsI(v8File->GetFileName().c_str()))
            return v8File;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::FindV8DrawingsAndSheets()
    {
    //  Build a list of dgn files to search for sheets and drawings 
    bvector<DgnFilePtr> tempKeepAlive;
    bvector<DgnV8FileP> filesToSearch;

    for (auto v8 : m_v8Files) // start with the files that we already found when we searched for spatial models
        {
        if (IsFileAssignedToBridge(*v8))
            filesToSearch.push_back(v8);
        }

    for (auto const& fn : m_params.GetDrawingAndSheetFiles()) // include the list of other files to search for drawings and sheets
        {
        if (nullptr != findOpenV8FileByName(filesToSearch, fn))
            continue;

        DgnV8Api::DgnFileStatus openStatus;
        Bentley::DgnFilePtr v8File = OpenDgnV8File(openStatus, fn);
        if (v8File.IsValid() && IsFileAssignedToBridge(*v8File))
            {
            tempKeepAlive.push_back(v8File);
            filesToSearch.push_back(v8File.get());
            }
        }

    for (auto fileToSearch : filesToSearch)
        ClassifyNormal2dModels (*fileToSearch); // This tells us whether a given 2d design model should become a drawing or a spatial model

    // *** EXTREMELY TRICKY: See "DgnModel objects and Sheet attachments" for why we need TWO PASSES
    Bentley::DgnModelPtr v8model;

    // 1. Sheets
    for (auto v8File : filesToSearch)
        {
        for (DgnV8Api::ModelIndexItem const& item : v8File->GetModelIndex())
            {
            if ((DgnV8Api::DgnModelType::Sheet == item.GetModelType()) && ((v8model = v8File->LoadModelById(item.GetModelId())).IsValid()))
                FindNonSpatialModel(*v8model, true);
            }
        }

    // Note: We follow and register the 3d reference attachments to sheets. That is so that we discover all drawings referenced to a sheet
    //  via the sheet's attachment. See "DgnModel objects and Sheet attachments".
    // For drawing models, however, we do not follow reference attachments. They must be found either directly
    //  from the model index (or as attachments to some sheet).


    // 2. Drawings
    for (auto v8File : filesToSearch)
        {
        for (DgnV8Api::ModelIndexItem const& item : v8File->GetModelIndex())
            {
            if (IsV8DrawingModel(*v8File, item) && ((v8model = v8File->LoadModelById(item.GetModelId())).IsValid()))
                FindNonSpatialModel(*v8model, false); 
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* The purpose of this function is to *discover* spatial models, not to convert their contents.
* The outcome of this function is to a) create an (empty) BIM spatial model for each discovered V8 spatial model,
* and b) to discover and record the spatial transform that should be applied when (later) converting
* the elements in each model.
* This function also has the side effect of creating or updating the job subject hierarchy.
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::ImportSpatialModels(bool& haveFoundSpatialRoot, DgnV8ModelRefR thisModelRef, BentleyApi::TransformCR trans)
    {
    // NB: Don't filter out files or models here. We need to know about the existence of all of them 
    // (e.g., so that we can infer deleted models). We will do any applicable filtering in ConvertElementsInModel.

    DgnV8ModelR thisV8Model = *thisModelRef.GetDgnModelP();
    DgnV8FileR  thisV8File  = *thisV8Model.GetDgnFileP();

    BeAssert((m_spatialModelsSeen.find(&thisV8Model) != m_spatialModelsSeen.end()) && "All spatial models should have been found by FindSpatialV8Models");
    BeAssert((std::find(m_v8Files.begin(), m_v8Files.end(), &thisV8File) != m_v8Files.end()) && "All V8 files should have been found by FindV8DrawingsAndSheets and FindSpatialV8Models");

    bool isThisMyFile = IsFileAssignedToBridge(thisV8File);
    if (isThisMyFile)
        haveFoundSpatialRoot = true;

    // FindSpatialV8Models has already called ClassifyNormal2dModels (thisV8File);

    SyncInfo::V8FileSyncInfoId v8FileId = GetV8FileSyncInfoId(thisV8File);
    WriteRepositoryLink(thisV8File);    // write the RepositoryLink element for this v8 file now. This the order in which the older converter did it.

    // NB: We must not try to skip entire files if we need to follow reference attachments. Instead, we can skip 
    //      the elements in a model if the model (i.e., the file) is unchanged.

    // Map this v8 model into the project
    ResolvedModelMapping v8mm = GetModelForDgnV8Model(thisModelRef, trans);

    if (nullptr == thisModelRef.GetDgnAttachmentsP())
        return; 

	// FindSpatialV8Models has already forced children of a spatial root to be spatial

    SubjectCR parentRefsSubject = _GetSpatialParentSubject();

    bool hasPushedReferencesSubject = false;
    for (DgnV8Api::DgnAttachment* attachment : *thisModelRef.GetDgnAttachmentsP())
        {                  
        if (nullptr == attachment->GetDgnModelP() || !_WantAttachment(*attachment))
            continue; // missing reference 

        if (haveFoundSpatialRoot && !IsFileAssignedToBridge(*attachment->GetDgnModelP()->GetDgnFileP()))
            continue; // this leads to models in another bridge's territory. Stay out.

        if (!hasPushedReferencesSubject)
            {
            SubjectCPtr myRefsSubject = GetOrCreateModelSubject(parentRefsSubject, v8mm.GetDgnModel().GetName(), ModelSubjectType::References);
            if (!myRefsSubject.IsValid())
                {
                BeAssert(false);
                ReportError(IssueCategory::Unsupported(), Issue::Error(), Utf8PrintfString("Failed to create references subject. Parent[%s]. Attachment[%s].",
                                                                                           IssueReporter::FmtModel(thisV8Model).c_str(),
                                                                                           IssueReporter::FmtAttachment(*attachment).c_str()).c_str());
                continue;
                }
            SetSpatialParentSubject(*myRefsSubject); // >>>>>>>>>> Push spatial parent subject
            hasPushedReferencesSubject = true;
            }

        if (!ShouldConvertToPhysicalModel(*attachment->GetDgnModelP()))
            {
            // 3D model referencing a 2D model
            // *** WIP_TEMPLATES convert the 2D model to a template and insert an instance of it at the ref attachment point
            ReportError(IssueCategory::Unsupported(), Issue::Error(), Utf8PrintfString("2D attachments to 3D are not supported. 3D:[%s]. 2D:[%s]",
                                                                                       IssueReporter::FmtModel(thisV8Model).c_str(),
                                                                                       IssueReporter::FmtAttachment(*attachment).c_str()).c_str());
            continue;
            }

        Transform refTrans = ComputeAttachmentTransform(trans, *attachment);
        ImportSpatialModels(haveFoundSpatialRoot, *attachment, refTrans);
        }

    if (hasPushedReferencesSubject)
        SetSpatialParentSubject(parentRefsSubject); // <<<<<<<<<<<< Pop spatial parent subject
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertSpatialLevels()
    {
    for (DgnV8FileP v8File : m_v8Files)
        _ConvertSpatialLevelTable(*v8File);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
void RootModelConverter::_ConvertLineStyles()
    {
    BeAssert(GetRootModelP() != nullptr);
    if (GetRootModelP() == nullptr)
        return;     //  the line style converter uses m_rootModel

    for (DgnV8FileP v8File : m_v8Files)
        ConvertAllLineStyles(*v8File);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::ModelId RootModelConverter::_GetRootModelIdFromViewGroup()
    {
    // try to find a spatial view in the viewgroup
    for (uint32_t iView=0; iView < DgnV8Api::MAX_VIEWS; ++iView)
        {
        ViewInfoR viewInfo = m_viewGroup->GetViewInfoR(iView);
        Bentley::ViewPortInfoCR viewPortInfo = m_viewGroup->GetViewPortInfo(iView);

        if (!viewInfo.GetViewFlags().on_off || !viewPortInfo.m_wasDefined)
            continue;

        auto modelId = viewInfo.GetRootModelId();
        Bentley::StatusInt openStatus;    
        auto model = m_rootFile->LoadRootModelById(&openStatus, modelId);
        if (nullptr == model)
            continue;

        if (!ShouldConvertToPhysicalModel(*model))
            continue;

        return modelId;
        }
    return INVALID_MODELID;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::ModelId RootModelConverter::_GetRootModelId()
    {
    if (IsUpdating() && !m_params.GetRootModelChoice().IsSet())
        {
        auto importJob = FindSoleImportJobForFile(*m_rootFile);
        if (importJob.IsValid())
            return GetSyncInfo().GetV8ModelIdFromV8ModelSyncInfoId(importJob.GetV8ModelSyncInfoId());
        }

    if (!IsV8Format(*m_rootFile))
        return DgnV8Api::DEFAULTMODEL;

    if (RootModelChoice::Method::ById == m_params.GetRootModelChoice().m_method)
        return m_params.GetRootModelChoice().m_id;

    if (RootModelChoice::Method::UseDefaultModel == m_params.GetRootModelChoice().m_method)
        return m_rootFile->GetDefaultModelId();

    if (RootModelChoice::Method::ByName == m_params.GetRootModelChoice().m_method)
        return m_rootFile->FindModelIdByName(WString(m_params.GetRootModelChoice().m_name.c_str(),BentleyCharEncoding::Utf8).c_str());

    BeAssert(RootModelChoice::Method::FromActiveViewGroup == m_params.GetRootModelChoice().m_method);

    DgnV8Api::ModelId defaultModelId = m_rootFile->GetDefaultModelId(); // We'll fall back on the default model if we can't find an eligible root from the viewgroups

    // Start with the root of the active viewgroup. That's what the user was looking at the last time he did save settings.
    ViewGroupCollectionR viewGroups = m_rootFile->GetViewGroupsR();
    m_viewGroup = viewGroups.FindByElementId(m_rootFile->GetActiveViewGroupId());

    // Problem: the viewgroup stored in the tcb no longer exists. Use the most recently modified one instead.
    if (!m_viewGroup.IsValid())
        m_viewGroup = viewGroups.FindLastModifiedMatchingModel(INVALID_ELEMENTID, INVALID_MODELID, false, -1);

    if (!m_viewGroup.IsValid()) // there must not be any viewgroup in the file. 
        DgnV8Api::ViewGroup::Create(m_viewGroup, defaultModelId, *m_rootFile, false, nullptr, true);

    if (!m_viewGroup.IsValid())
        {
        BeAssert(false);
        return defaultModelId;
        }

    auto rootModelId = _GetRootModelIdFromViewGroup();
    if (INVALID_MODELID == rootModelId)
        {
        // The active or most recently modified ViewGroup does not have a spatial root. 
        // See if *any* viewgroup has a spatial root.
        for (auto vg : viewGroups)
            {
            m_viewGroup = vg;
            if (INVALID_MODELID != (rootModelId = _GetRootModelIdFromViewGroup()))
                break;
            }    
        }

    if (INVALID_MODELID != rootModelId)
        return rootModelId;

    return defaultModelId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
RootModelConverter::RootModelConverter(RootModelSpatialParams& params) 
    : T_Super(params), m_params(params)
    {
    // We do the Config map lookup here and save the result to this variable.
    m_considerNormal2dModelsSpatial = (GetConfig().GetOptionValueBool("Consider2dModelsSpatial", false) || m_params.GetConsiderNormal2dModelsSpatial());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertSpatialElements()
    {
    SetStepName(Converter::ProgressMessage::STEP_CONVERTING_ELEMENTS());
    DoConvertSpatialElements();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Keith.Bentley                   02 / 15
//---------------------------------------------------------------------------------------
void RootModelConverter::ConvertElementsInModel(ResolvedModelMapping const& v8mm)
    {
    DgnModelR targetModel = v8mm.GetDgnModel();

    if (!targetModel.Is3d())
        {
        BeAssert(false);
        return;
        }

    if (GetChangeDetector()._AreContentsOfModelUnChanged(*this, v8mm))
        return;

    DgnV8Api::DgnModel& v8Model = v8mm.GetV8Model();

    if (!v8Model.Is3D() && GetRootModelP() != &v8Model) // if this is a drawing, then it won't already be filled.
        v8Model.FillCache();

    ReportProgress();

    m_currIdPolicy = GetIdPolicyFromAppData(*v8Model.GetDgnFileP());

    ConvertElementList(v8Model.GetControlElementsP(), v8mm);
    ConvertElementList(v8Model.GetGraphicElementsP(), v8mm);

    GetDgnDb().Memory().PurgeUntil(1024*1024);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::DoConvertSpatialElements()
    {
    bmultiset<ResolvedModelMapping> spatialModels;
    for (auto v8mm : m_v8ModelMappings)
        {
        if (v8mm.GetDgnModel().Is3dModel())
            spatialModels.insert(v8mm);
        }

    if (spatialModels.empty())
        return;

    AddTasks((int32_t)(spatialModels.size()));
    for (auto& modelMapping : spatialModels)
        {
        if (WasAborted())
            return;

        SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_MODEL(), modelMapping.GetDgnModel().GetName().c_str());

        StopWatch timer(true);
        uint32_t start = GetElementsConverted();

        ConvertElementsInModel(modelMapping);

        uint32_t convertedElementCount = (uint32_t) GetElementsConverted() - start;
        ConverterLogging::LogPerformance(timer, "Convert Spatial Elements> Model '%s' (%" PRIu32 " element(s))", 
                                         modelMapping.GetDgnModel().GetName().c_str(),
                                         convertedElementCount);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::ConvertNamedGroupsAndECRelationships()
    {
    StopWatch timer(true);
    uint32_t start = GetElementsConverted();

    AddTasks((int32_t)(m_v8Files.size()));
    //convert dictionary model named groups
    for (DgnV8FileP v8File : m_v8Files)
        {
        SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_MODEL(), Converter::IssueReporter::FmtFileBaseName(*v8File).c_str());

        if (WasAborted())
            return;

        ResolvedModelMapping v8DictionaryModelMapping;

        for (DgnV8Api::PersistentElementRef* controlElem : *v8File->GetDictionaryModel().GetControlElementsP())
            {
            if (V8ElementType::NamedGroup == V8ElementTypeHelper::GetType(*controlElem))
                {
                if (!v8DictionaryModelMapping.IsValid())
                    v8DictionaryModelMapping = MapDgnV8ModelToDgnDbModel(v8File->GetDictionaryModel(), Transform::FromIdentity(), GetDgnDb().GetDictionaryModel().GetModelId());
                DgnV8Api::EditElementHandle v8eeh(controlElem);
                ElementConversionResults res;
                _ConvertControlElement(res, v8eeh, v8DictionaryModelMapping);
                }
            }
        }

    uint32_t convertedElementCount = (uint32_t) GetElementsConverted() - start;
    ConverterLogging::LogPerformance(timer, "Convert NamedGroups in dictionary (%" PRIu32 " element(s))", convertedElementCount);

    AddTasks(1);
    SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_RELATIONSHIPS());
    ConvertNamedGroupsRelationships();
    ConverterLogging::LogPerformance(timer, "Convert Elements> NamedGroups");

    timer.Start();
    ConvertECRelationships();
    ConverterLogging::LogPerformance(timer, "Convert Elements> ECRelationships (total)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
void RootModelConverter::UpdateCalculatedProperties()
    {

    CachedStatementPtr stmt = nullptr;
    auto stat = m_dgndb->GetCachedStatement(stmt, "SELECT p.ClassId, p.Name FROM ec_Property p, ec_CustomAttribute ca, ec_Class c WHERE ca.ClassId = c.Id AND c.Name='CalculatedECPropertySpecification' AND ca.ContainerId=p.Id AND ca.Instance LIKE '%GetRelatedInstance%'");
    //auto stat = m_dgndb->GetCachedStatement(stmt, "SELECT p.ClassId, p.Name FROM ec_Property p, ec_CustomAttribute ca, ec_Class c WHERE ca.ClassId = c.Id AND c.Name='CalculatedECPropertySpecification' AND ca.ContainerId=p.Id");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement.");
        return;
        }

    bmap<BECN::ECClassId, bvector<Utf8String>> classMap;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        BECN::ECClassId classId = stmt->GetValueId<BECN::ECClassId>(0);
        bmap<BECN::ECClassId, bvector<Utf8String>>::iterator iter = classMap.find(classId);
        classMap[classId].push_back(stmt->GetValueText(1));
        }

    for (bmap<BECN::ECClassId, bvector<Utf8String>>::iterator iter = classMap.begin(); iter != classMap.end(); iter++)
        {
        BECN::ECClassCP ecClass = m_dgndb->Schemas().GetClass(iter->first);

        Utf8String propertyNames;
        bvector<Utf8String> properties = iter->second;
        for (int i = 0; i < properties.size(); i++)
            {
            if (i != 0)
                propertyNames.append(", ");
            propertyNames.append(properties[i]);
            }
        Utf8PrintfString sql("SELECT ECClassId, ECInstanceId, * FROM %s"/*, propertyNames.c_str()*/, ecClass->GetECSqlName().c_str());

        BeSQLite::EC::CachedECSqlStatementPtr instanceStmt = m_dgndb->GetPreparedECSqlStatement(sql.c_str());
        BeSQLite::EC::ECInstanceECSqlSelectAdapter dataAdapter(*instanceStmt);
        while (BE_SQLITE_ROW == instanceStmt->Step())
            {
            BECN::IECInstancePtr selectedInstance = dataAdapter.GetInstance();
            uint64_t id;
            BeStringUtilities::ParseUInt64(id, selectedInstance->GetInstanceId().c_str());
            DgnElementPtr element = m_dgndb->Elements().GetForEdit<DgnElement>(DgnElementId(id));
            BECN::ECValue nullValue;
            nullValue.SetToNull();
            for (int i = 0; i < properties.size(); i++)
                {
                BECN::ECValue v;
                selectedInstance->SetValue(properties[i].c_str(), nullValue);
                selectedInstance->GetValue(v, properties[i].c_str());
                element->SetPropertyValue(properties[i].c_str(), v);
                }
            element->Update();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   03/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus RootModelConverter::ConvertNamedGroupsRelationships()
    {
    for (auto const& modelMapping : m_v8ModelMappings)
        {
        if (BentleyApi::SUCCESS != ConvertNamedGroupsRelationshipsInModel(modelMapping.GetV8Model()))
            return BSIERROR;
        }

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   04/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus Converter::ConvertNamedGroupsRelationshipsInModel(DgnV8ModelR v8Model)
    {
    struct NamedGroupConverter : DgnV8Api::INamedGroupMemberVisitor
        {
        private:
            Converter& m_converter;
            DgnElementId const& m_parentId;
            bool m_namedGroupOwnsMembers;
            bset<DgnElementId> m_visitedMembers;

            virtual DgnV8Api::MemberTraverseStatus VisitMember(DgnV8Api::NamedGroupMember const* member, DgnV8Api::NamedGroup const* ng, UInt32 index) override
                {
                DgnV8Api::ElementHandle memberEh(member->GetElementRef());
                DgnElementId childElementId;
                if (!m_converter.GetSyncInfo().TryFindElement(childElementId, memberEh))
                    {
                    Utf8String error;
                    error.Sprintf("No BIS grouping relationship created for v8 NamedGroup Member element (%s) because the member element was not converted.",
                                  Converter::IssueReporter::FmtElement(memberEh).c_str());
                    m_converter.ReportIssue(IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
                    return DgnV8Api::MemberTraverseStatus::Continue;
                    }
                m_visitedMembers.insert(childElementId);

                DgnElements& elementTable = m_converter.GetDgnDb().Elements();

                DgnElementCPtr child = elementTable.GetElement(childElementId);
                if (!child.IsValid())
                    return DgnV8Api::MemberTraverseStatus::Continue;

                if (m_namedGroupOwnsMembers)
                    {
                    if (child->GetParentId() != m_parentId)
                        {
                        DgnElementPtr childEdit = child->CopyForEdit();
                        DgnClassId parentRelClassId = m_converter.GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);
                        childEdit->SetParentId(m_parentId, parentRelClassId);
                        elementTable.Update(*childEdit);
                        }
                    return DgnV8Api::MemberTraverseStatus::Continue;
                    }

                GroupInformationElementCPtr group = elementTable.Get<GroupInformationElement>(m_parentId);
                if (group.IsValid() && !ElementGroupsMembers::HasMember(*group, *child))
                    {
                    if (DgnDbStatus::Success != ElementGroupsMembers::Insert(*group, *child, 0))
                        {
                        Utf8String error;
                        error.Sprintf("Failed to add child element %s to group %" PRIu64 "", Converter::IssueReporter::FmtElement(memberEh).c_str(), m_parentId.GetValue());
                        m_converter.ReportIssue(IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
                        }
                    }

                return DgnV8Api::MemberTraverseStatus::Continue;
                }

        public:
            NamedGroupConverter(Converter& converter, DgnElementId const& parentId, bool namedGroupOwnsMembers)
                : m_converter(converter), m_parentId(parentId), m_namedGroupOwnsMembers(namedGroupOwnsMembers)
                {}

            bool WasVisited(DgnElementId member)
                {
                return m_visitedMembers.find(member) != m_visitedMembers.end();
                }
        };

    if (WasAborted())
        return BentleyApi::SUCCESS;

    //TODO: do we need to load the model and its attachments or can we consider them loaded already?

    SyncInfo::V8FileSyncInfoId v8FileId = GetV8FileSyncInfoIdFromAppData(*v8Model.GetDgnFileP());
    bset<DgnV8Api::ElementId> const* namedGroupsWithOwnershipHintPerFile = nullptr;
    V8NamedGroupInfo::TryGetNamedGroupsWithOwnershipHint(namedGroupsWithOwnershipHintPerFile, v8FileId);

    for (auto v8El : *v8Model.GetControlElementsP())
        {
        BeAssert(v8El != nullptr);
        DgnV8Api::ElementHandle v8eh(v8El);
        if (V8ElementTypeHelper::GetType(v8eh) != V8ElementType::NamedGroup)
            continue;

        DgnV8Api::DgnModel* ngRootModel = &v8Model;
        if (ngRootModel->IsDictionaryModel()) // *** TBD: Check that file was orginally DWG
            {
            DgnV8Api::DgnModel* defaultModel = v8Model.GetDgnFileP()->FindLoadedModelById (v8Model.GetDgnFileP()->GetDefaultModelId());
            if (nullptr != defaultModel)
                ngRootModel = defaultModel;
            }

        DgnV8Api::NamedGroupPtr ng = nullptr;
        if (DgnV8Api::NamedGroupStatus::NG_Success != DgnV8Api::NamedGroup::Create(ng, v8eh, ngRootModel))
            {
            BeAssert(false && "Could not instantiate NamedGroup object");
            return BSIERROR;
            }

        DgnElementId ngElementId;
        if (!GetSyncInfo().TryFindElement(ngElementId, v8eh))
            {
            Utf8String error;
            error.Sprintf("No BIS grouping created for v8 NamedGroup element (%s) because the NamedGroup was not converted.",
                          Converter::IssueReporter::FmtElement(v8eh).c_str());
            ReportIssue(IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
            continue;
            }

        const bool namedGroupOwnsMembersFlag = namedGroupsWithOwnershipHintPerFile != nullptr && namedGroupsWithOwnershipHintPerFile->find(v8eh.GetElementId()) != namedGroupsWithOwnershipHintPerFile->end();
        NamedGroupConverter ngConverter(*this, ngElementId, namedGroupOwnsMembersFlag);
        //if traversal fails, still continue with next element, so return value doesn't matter here.
        ng->TraverseMembers(&ngConverter, DgnV8Api::MemberTraverseType::Enumerate, false, false);

        if (!IsUpdating())
            continue;

        DgnElements& elementTable = GetDgnDb().Elements();
        if (namedGroupOwnsMembersFlag)
            {
            DgnElementCPtr header = elementTable.GetElement(ngElementId);
            ;
            for (DgnElementId childId : header->QueryChildren())
                {
                if (ngConverter.WasVisited(childId))
                    continue;
                auto child = elementTable.GetElement(childId);
                if (child.IsValid())
                    {
                    auto editChild = child->CopyForEdit();
                    DgnClassId parentRelClassId = GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);
                    editChild->SetParentId(DgnElementId(), parentRelClassId);
                    editChild->Update();
                    }
                }
            }
        else
            {
            GenericGroupCPtr group = elementTable.Get<GenericGroup>(ngElementId);
            if (group.IsValid())
                {
                for (DgnElementId memberId : ElementGroupsMembers::QueryMembers(*group))
                    {
                    if (ngConverter.WasVisited(memberId))
                        continue;
                    DgnElementCPtr child = elementTable.GetElement(memberId);
                    if (child.IsValid())
                        group->RemoveMember(*child);
                    }
                }
            }
        }

    GetDgnDb().Memory().PurgeUntil(1024*1024);
    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus RootModelConverter::ConvertECRelationships()
    {
    if (m_skipECContent)
        return BentleyApi::SUCCESS;

    //As all v8 relationships end up in the same table (bis_ElementRefersToElements)
    //it gets a lot of indexes. These hurt performance a lot, so we drop the indexes before the bulk insert
    //and re-add them later.
    bmap<Utf8String, Utf8String> indexDdlList;
    if (!IsUpdating())
        {
        StopWatch timer(true);
        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(*m_dgndb, "SELECT name, sql FROM sqlite_master WHERE tbl_name='bis_ElementRefersToElements' AND type='index'"))
            return BentleyApi::ERROR;

        while (stmt.Step() == BE_SQLITE_ROW)
            {
            indexDdlList[Utf8String(stmt.GetValueText(0))] = Utf8String(stmt.GetValueText(1));
            }

        stmt.Finalize();

        for (auto const& index : indexDdlList)
            {
            Utf8String sql("DROP INDEX ");
            sql.append(index.first);
            if (BE_SQLITE_OK != GetDgnDb().ExecuteSql(sql.c_str()))
                return BentleyApi::ERROR;
            }

        ConverterLogging::LogPerformance(timer, "Convert Elements> ECRelationships: Dropped indices for bulk insertion into BisCore:ElementRefersToElements class hierarchy.");
        }

    for (auto& modelMapping : m_v8ModelMappings)
        {
        ConvertECRelationshipsInModel(modelMapping.GetV8Model());
        if (WasAborted())
            return BentleyApi::ERROR;
        }

    //analyze named groups in dictionary models
    for (DgnV8FileP v8File : m_v8Files)
        {
        DgnV8ModelR dictionaryModel = v8File->GetDictionaryModel();
        ConvertECRelationshipsInModel(dictionaryModel);
        if (WasAborted())
            return BentleyApi::ERROR;
        }

    //recreate indexes that were previously dropped
    if (!IsUpdating())
        {
        StopWatch timer(true);
        for (auto const& index : indexDdlList)
            {
            if (BE_SQLITE_OK != GetDgnDb().TryExecuteSql(index.second.c_str()))
                {
                Utf8String error;
                error.Sprintf("Failed to recreate index '%s' for BisCore:ElementRefersToElements class hierarchy: %s", index.second.c_str(), GetDgnDb().GetLastError().c_str());
                ReportIssue(IssueSeverity::Info, IssueCategory::Sync(), Issue::Message(), error.c_str());
                CachedStatementPtr stmt = GetDgnDb().GetCachedStatement("DELETE FROM ec_Index WHERE Name=?");
                if (stmt == nullptr)
                    {
                    BeAssert(false && "Could not retrieve cached statement.");
                    return BentleyApi::ERROR;
                    }
                if (BE_SQLITE_OK != stmt->BindText(1, index.first, Statement::MakeCopy::No))
                    {
                    BeAssert(false && "Could not bind to statement.");
                    return BentleyApi::ERROR;
                    }

                if (BE_SQLITE_DONE != stmt->Step())
                    {
                    error.Sprintf("Failed to delete index '%s' from table ec_Index: %s", index.first.c_str(), GetDgnDb().GetLastError().c_str());
                    ReportError(IssueCategory::Sync(), Issue::Message(), error.c_str());
                    return BentleyApi::ERROR;
                    }
                }
            }

        ConverterLogging::LogPerformance(timer, "Convert Elements> ECRelationships: Recreated indices for BisCore:ElementRefersToElements class hierarchy.");
        }

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyApi::BentleyStatus Converter::ConvertECRelationshipsInModel(DgnV8ModelR v8Model)
    {
    if (m_skipECContent)
        return BentleyApi::SUCCESS;

    DgnV8Api::PersistentElementRefList* graphicElements = v8Model.GetGraphicElementsP();
    if (nullptr != graphicElements)
        {
        for (DgnV8Api::PersistentElementRef* v8Element : *graphicElements)
            {
            DgnV8Api::ElementHandle v8eh(v8Element);
            //TODO if (_FilterElement(v8eh))
            //TODO     continue;

            ConvertECRelationships(v8eh);
            }
        }

    DgnV8Api::PersistentElementRefList* controlElems = v8Model.GetControlElementsP();
    if (nullptr != controlElems)
        {
        for (DgnV8Api::PersistentElementRef* v8Element : *controlElems)
            {
            DgnV8Api::ElementHandle v8eh(v8Element);
            //TODO if (_FilterElement(v8eh))
            //TODO     continue;

            ConvertECRelationships(v8eh);
            }
        }
    return BentleyApi::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_BeginConversion()
    {
    if (!GetImportJob().IsValid() || (GetImportJob().GetConverterType() != SyncInfo::ImportJob::Type::RootModels))
        {
        OnFatalError();
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_FinishConversion()
    {
    ConvertNamedGroupsAndECRelationships();   // Now that we know all elements, work on the relationships between elements.
    if (IsUpdating())
        UpdateCalculatedProperties();
    _RemoveUnusedMaterials();

    m_linkConverter->PurgeOrphanedLinks();
    
    EmbedSpecifiedFiles();

    for (auto f: m_finishers)
        {
        f->_OnFinishConversion(*this);
        }

    if (!IsUpdating())
        {
        EmbedFilesInSource(GetRootFileName());
        }
    else
        {
        // Detect deletions in the V8 files that we processed. (Don't assume we saw all V8 files.)
        for (DgnV8FileP v8File : m_v8Files)
            {
            GetChangeDetector()._DetectDeletedElementsInFile(*this, *v8File);
            GetChangeDetector()._DetectDeletedModelsInFile(*this, *v8File);    // NB: call this *after* DetectDeletedElements!
            }
        GetChangeDetector()._DetectDeletedElementsEnd(*this);
        GetChangeDetector()._DetectDeletedModelsEnd(*this);

        // Update syncinfo for all V8 files *** WIP_UPDATER - really only need to update syncinfo for changed files, but we don't keep track of that.
        for (DgnFileP v8File : m_v8Files)
            {
            SyncInfo::V8FileProvenance prov(*v8File, GetSyncInfo(), GetCurrentIdPolicy());
            prov.Update();
            }
        }

    CopyExpirationDate(*m_rootFile);

    ValidateJob();
    }

/*=================================================================================**//**
* @bsiclass                                     Brien.Bastings                  03/17
+===============+===============+===============+===============+===============+======*/
struct ConvertModelACSTraverser : DgnV8Api::IACSTraversalHandler
{
ResolvedModelMapping    m_v8mm;
double                  m_toMeters;

ConvertModelACSTraverser(ResolvedModelMapping const& v8mm, double toMeters) : m_v8mm(v8mm), m_toMeters(toMeters) {}
UInt32 _GetACSTraversalOptions () override {return 0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool _HandleACSTraversal(DgnV8Api::IAuxCoordSys& v8acs) override
    {
    if (DgnV8Api::ACSType::Extended == v8acs.GetType())
        return false; // Entering lat/long will not be handled by requiring a special ACS in Bim002...
        
    Bentley::WString acsNameV8 = v8acs.GetName();

    if (Bentley::WString::IsNullOrEmpty(acsNameV8.c_str()))
        return false;

    // See if this ACS already exists...if same name from different models, just keep first one...
    Utf8String    acsName(acsNameV8.c_str());
    DgnCode       acsCode = AuxCoordSystem::CreateCode(m_v8mm.GetDgnModel(), nullptr, acsName);
    DgnElementId  acsId = m_v8mm.GetDgnModel().GetDgnDb().Elements().QueryElementIdByCode(acsCode);

    if (acsId.IsValid()) // Do we need to do something here for update???
        return false;

    AuxCoordSystemPtr   acsElm = AuxCoordSystem::CreateNew(m_v8mm.GetDgnModel(), nullptr, acsName);
    Bentley::DPoint3d   acsOrigin;
    Bentley::RotMatrix  acsRMatrix;

    v8acs.GetRotation(acsRMatrix);
    v8acs.GetOrigin(acsOrigin);
    acsOrigin.Scale(acsOrigin, m_toMeters); // Account for uor->meter scale...

    acsElm->SetType((ACSType) v8acs.GetType());
    acsElm->SetOrigin((DPoint3dCR) acsOrigin);
    acsElm->SetRotation((RotMatrixCR) acsRMatrix);

    Bentley::WString acsDescrV8 = v8acs.GetDescription();

    if (!Bentley::WString::IsNullOrEmpty(acsDescrV8.c_str()))
        {
        Utf8String acsDescr;

        acsDescr.Assign(acsDescrV8.c_str());
        acsElm->SetDescription(acsDescr.c_str());
        }

    DgnDbStatus acsStatus;
    acsElm->Insert(&acsStatus);

    return false;
    }

}; // ConvertModelACSTraverser

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  RootModelConverter::Process()
    {
    AddSteps(9);

    StopWatch totalTimer(true);
   
    StopWatch timer(true);

    _OnConversionStart();

    _BeginConversion();
    if (WasAborted())
        return ERROR;

    ConverterLogging::LogPerformance(timer, "Begin Conversion");

    timer.Start();
    
    _ConvertModels();       // This is where we discover just about all of the V8 files and models that we'll need to mine for data in the subsequent steps
    if (WasAborted())
        return ERROR;
    
    ConverterLogging::LogPerformance(timer, "Convert Models");

    SetStepName(Converter::ProgressMessage::STEP_CONVERTING_STYLES());

    timer.Start();
    _ConvertLineStyles();
    if (WasAborted())
        return ERROR;
    
    ConverterLogging::LogPerformance(timer, "Convert Line Styles");

    AddTasks((int32_t)(m_v8ModelMappings.size()));
    for (auto& modelMapping : m_v8ModelMappings)
        {
        if (WasAborted())
            return ERROR;

        SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_MATERIALS(), modelMapping.GetDgnModel().GetName().c_str());
        ConvertModelMaterials (modelMapping.GetV8Model());

        ConvertModelACSTraverser acsTraverser(modelMapping, ComputeUnitsScaleFactor(modelMapping.GetV8Model()));
        DgnV8Api::IACSManager::GetManager().Traverse(acsTraverser, &modelMapping.GetV8Model());
        }

    if (m_isRootModelSpatial)
        {
        timer.Start();
        _ConvertSpatialLevels();
        if (WasAborted())
            return ERROR;

        ConverterLogging::LogPerformance(timer, "Convert Spatial Levels");

        timer.Start();
        _ConvertSpatialViews();
        if (WasAborted())
            return ERROR;

        ConverterLogging::LogPerformance(timer, "Convert Spatial Views");

        timer.Start();
        _ConvertSpatialElements();
        if (WasAborted())
            return ERROR;

        ConverterLogging::LogPerformance(timer, "Convert Spatial Elements (total)");
        }

    timer.Start();
    _ConvertDrawings();
    if (WasAborted())
        return ERROR;

    ConverterLogging::LogPerformance(timer, "Convert Drawings (total)");

    timer.Start();
    _ConvertSheets();
    if (WasAborted())
        return ERROR;

    ConverterLogging::LogPerformance(timer, "Convert Sheets (total)");

    timer.Start();
    _FinishConversion();
    if (WasAborted())
        return ERROR;

    _OnConversionComplete();
    ConverterLogging::LogPerformance(timer, "Finish conversion");

    ConverterLogging::LogPerformance(totalTimer, "Total conversion time");
    return WasAborted() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::_OnUpdateLevel(DgnV8Api::LevelHandle const& v8Level, DgnCategoryId cat, DgnV8FileR v8File)
    {
    // We can only check the root file's levels for changes. If a reference file defines a level with the same name as
    // some other file but with a different definition, the converter will have made an attachment-specific copy of the
    // level. We need the DgnAttachment in order to find that copy and check ... or even to know if a copy was made.
    if (v8Level.GetLevelId() == DGNV8_LEVEL_DEFAULT_LEVEL_ID || GetRootModelP()->GetDgnFileP() != &v8File)
        {
        // we will check that all attachment-specific subcategories are unchanged when we go through the view level mask.
        return;
        }

    CheckNoLevelChange(v8Level,cat,v8File);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_SetChangeDetector(bool isUpdating)
    {
    BeAssert(isUpdating == IsUpdating());

    if (!isUpdating)
        m_changeDetector.reset(new CreatorChangeDetector);
    else
        {
        m_changeDetector.reset(new ChangeDetector); 
        m_skipECContent = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::RootModelSpatialParams::Legacy_Converter_Init(BeFileNameCR bcName)
    {
    m_briefcaseName = bcName;
    SetReportFileName();
    m_isCreatingNewDb = true;
    m_isUpdating = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/17
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping RootModelConverter::_FindResolvedModelMappingBySyncId(SyncInfo::V8ModelSyncInfoId sid)
    {
    for (auto& rmm : m_v8ModelMappings)
        {
        if (rmm.GetV8ModelSyncInfoId() == sid)
            return rmm;
        }
    return ResolvedModelMapping();
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
