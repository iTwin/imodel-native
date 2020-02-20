/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

#include <ScalableMesh/ScalableMeshLib.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include "RuleSetEmbedder.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

// We enter this namespace in order to avoid having to qualify all of the types, such as bmap, that are common
// to bim and v8. The problem is that the V8 Bentley namespace is shifted in.
BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertSpatialViews()
    {
    if (!IsFileAssignedToBridge(*GetRootV8File()))
        return;

    auto viewTypes = m_config.GetXPathString("/ImportConfig/Views/@viewTypes", "rootviewgroup rootsavedviews");
    viewTypes.ToLower();
    bool wantViewGroups = viewTypes.find("rootviewgroup") != Utf8String::npos;
    bool wantSavedViews = viewTypes.find("rootsavedviews") != Utf8String::npos;

    if (!wantViewGroups && !wantSavedViews)
        return;

    SpatialViewFactory vf(*this);

    if (wantViewGroups)
        {
        if (!m_viewGroup.IsValid())
            m_viewGroup = m_rootFile->GetViewGroupsR().FindByModelId(GetRootModelP()->GetModelId(), true, -1);
        if (!m_viewGroup.IsValid())
            {
            DgnV8Api::ViewGroupStatus vgStatus;
            if (DgnV8Api::VG_Success != (vgStatus = DgnV8Api::ViewGroup::Create(m_viewGroup, *GetRootModelP(), true, NULL, true)))
                return;
            }

        DgnViewId firstView;
        ConvertViewGroup(firstView, *m_viewGroup, m_rootTrans, vf);

        if (firstView.IsValid() && !m_defaultViewId.IsValid())
            m_defaultViewId = firstView;
        }

    if (wantSavedViews)
        {
        NamedViewCollectionCR namedViews = GetRootV8File()->GetNamedViews();
        for (DgnV8Api::NamedViewPtr namedView : namedViews)
            ConvertNamedView(*namedView, m_rootTrans, vf);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::DgnFileStatus RootModelConverter::_InitRootModel()
    {
    // *** NB: Do not create elements (or models) in here. This is running as part of the initialization phase.
    //          Only schema changes are allowed in this phase.

    // don't bother to convert a DWG master file - let DwgImporter do the job.
    BeFileNameCR rootFileName = GetRootFileName();
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
        {
        LOG.errorv(L"Error opening the file %s (Error code: %d)", rootFileName.GetName(), openStatus);
        return openStatus;
        }
    else
        LOG.tracev(L"Opened root model file %ls", rootFileName.GetName());

    //  Identify the root model
    auto rootModelId = _GetRootModelId();

    //  Load the root model and all of its reference attachments. Let V8 do this, so that we know that it's done correctly and in the same way that MicroStation would do it.
    m_rootModelRef = m_rootFile->LoadRootModelById((Bentley::StatusInt*)&openStatus, rootModelId, /*fillCache*/true, /*loadRefs*/true, GetParams().GetProcessAffected());
    if (NULL == m_rootModelRef)
        return openStatus;

    RealityMeshAttachmentConversion::ForceClassifierAttachmentLoad (*m_rootModelRef);

    if (DgnV8Api::DGNFILE_STATUS_Success != (openStatus = _ComputeCoordinateSystemTransform()))
        return openStatus;

    m_isRootModelSpatial = ShouldConvertToPhysicalModel(*GetRootModelP());

    if (!m_isRootModelSpatial)
        ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Unsupported(), Converter::Issue::RootModelMustBePhysical(), Converter::IssueReporter::FmtModel(*GetRootModelP()).c_str());

    if (WasAborted())
        return DgnV8Api::DGNFILE_STATUS_UnknownError;
    
    // Detect all V8 models. This process also classifies 2d design models and loads and fills drawings and sheets.
    // The of models that we find will be fed into the ECSchema conversion logic. These functions will ALSO enroll the v8files that they find in syncinfo.
    GetRepositoryLinkId(*m_rootFile);
    
    for (auto xdomain : XDomainRegistry::s_xdomains)
        xdomain->_OnBeginConversion(*this, *m_rootModelRef->GetDgnModelP());
    
    FindSpatialV8Models(*GetRootModelP());
    FindV8DrawingsAndSheets();

#ifndef NDEBUG
    BeAssert((m_v8Files.size() >= 1) && "FindSpatialV8Models should have populated m_v8Files");
    for (auto f : m_v8Files)
        {
        auto rlink = GetRepositoryLinkElement(*f);
        BeAssert(rlink.IsValid() && "We should have cached the RepositoryLinkId for each V8 file that we found");

        auto aspect = SyncInfo::RepositoryLinkExternalSourceAspect::GetAspect(*rlink);
        BeAssert(aspect.IsValid() && "We should be able to look up V8 files in syncinfo by their RepositoryLinkId's");
        }
#endif

    if (GetParams().IgnoreStaleFiles() || GetParams().ErrorOnStaleFiles())
        {
        bool anyStale = false;
        for (auto v8File : m_v8Files)
            {
            DateTime ftime, xtime;
            if (GetSyncInfo().IsStaleFile(&ftime, &xtime, *v8File))
                {
                anyStale = true;
                auto sev = GetParams().ErrorOnStaleFiles() ? Converter::IssueSeverity::Fatal : Converter::IssueSeverity::Error;
                ReportIssueV(sev, Converter::IssueCategory::InconsistentData(), Converter::Issue::FileIsStale(), Utf8String(v8File->GetFileName().c_str()).c_str(),
                            xtime.ToString().c_str());
                }
            }
        if (anyStale && GetParams().ErrorOnStaleFiles())
            {
            _OnFatalError();
            }
        }

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

    GetRepositoryLinkId(*GetRootV8File());  // Make sure the RepositoryLink element for the root file exists and its ID is cached in appdata.

    auto sourceMasterModelSubject = FindSourceMasterModelSubject(*GetRootModelP());
    if (!sourceMasterModelSubject.IsValid())
        {
        return ImportJobLoadStatus::FailedNotFound;
        }
    auto rootModelAspect = GetRootModelAspectFromSourceMasterModelSubject(*sourceMasterModelSubject);
    BeAssert(rootModelAspect.GetV8ModelId() == GetRootModelP()->GetModelId());

    // Look for a Job Subject for this bridge under this SourceMasterModel Subject.
    // If we don't find one, that means that this bridge has never been run against this source master file.
    m_importJob = FindSoleJobSubjectForSourceMasterModel(*sourceMasterModelSubject);
    if (!m_importJob.IsValid())
        return ImportJobLoadStatus::FailedNotFound;

    if (BSISUCCESS != CheckJobBelongsToMe(m_importJob.GetSubject(), *sourceMasterModelSubject))
        return ImportJobLoadStatus::FailedNotFound;

    // *** TRICKY: If this is called by the framework as a check *after* it calls _IntializeJob, then don't change the change detector!
    if (!_HaveChangeDetector() || IsUpdating())
        _SetChangeDetector(true);

    auto rootBimModel = GetDgnDb().Models().GetModel(rootModelAspect.GetModelId());
    m_rootModelMapping = ResolvedModelMapping(*rootBimModel, *GetRootModelP(), rootModelAspect, nullptr);
    _AddResolvedModelMapping(m_rootModelMapping);
    _GetChangeDetector()._OnModelSeen(*this, m_rootModelMapping);

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
void SpatialConverterBase::DetectRootTransformChange()
    {
    // Detect two possible sources of change to the root transform:
    // 1. The caller has specified a new and different "job transform" to be pre-multiplied into the usual units/gcs transform
    // 2. The units/gcs transform has changed in the source file.
    // If either (or both) changed, then we must set m_rootTransHasChanged=true and m_rootTransChange to a corrective transform that
    // must be applied to all spatial models.

    // Note that m_rootTrans was set by _ComputeCoordinateSystemTransform prior to this function being called. So, if the source
    // units or GCS changed, then the effects will be reflected in m_rootTrans.

    // Also note that m_rootModelMapping.GetTransform() tells us what the root transform was the last time we converted this root.
    // Therefore, m_rootModelMapping.GetTransform() is the basis for comparison, when we try to find out if the all-in root transform
    // has changed or not.

    // Compute the root transform that the caller would like me to apply to all elements
    Transform jobTrans = iModelBridge::GetSpatialDataTransform(_GetParams(), m_importJob.GetSubject()); // TRICKY! GetSpatialDataTransform reads the GetSpatialDataTransform property of the params and *updates* the Editable Transform property of the JobSubject if they differ!
    if (!jobTrans.IsIdentity())
        m_rootTrans = BentleyApi::Transform::FromProduct(jobTrans, m_rootTrans); // Standing orders

    // Is this different from what I applied last time I converted?
    m_rootTransHasChanged = !Converter::IsTransformEqualWithTolerance(m_rootModelMapping.GetTransform(), m_rootTrans);
    if (!m_rootTransHasChanged)
        return;

    // The root transform has changed.

    // We will have to correct all model transforms (later on in the conversion).
    // So, we want the factor, rtc, such that: 
    //  r1 = rtc * r0    
    // Where "r1" is the new root trans, and "r0" is the old root trans . Therefore, 
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

    // Set the corrected transform in the copy of the aspect that we have cached in memory
    rmm.SetTransform(Transform::FromProduct(m_rootTransChange, rmm.GetTransform())); // See DetectRootTransformChange for how m_rootTransChange was computed
    
    // Also, write the corrected transform to the persistent aspect in the BIM
    DgnElementPtr drawingEl;
    SyncInfo::V8ModelExternalSourceAspect aspect;
    std::tie(drawingEl, aspect) = SyncInfo::V8ModelExternalSourceAspect::GetAspectForEdit(rmm.GetDgnModel());
    aspect.SetTransform(rmm.GetTransform());
    drawingEl->Update();

    // we can now look up the model mapping in the iModel using the new transform
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::CorrectSpatialTransforms()
    {
    if (!m_rootTransHasChanged)
        return;

    CorrectSpatialTransform(m_rootModelMapping);

    m_rootTrans = m_rootModelMapping.GetTransform();

    for (auto& rmm : m_v8ModelMappings)
        {
        if (rmm.GetDgnModel().GetModelId() == m_rootModelMapping.GetDgnModel().GetModelId())
            continue;
        if (!IsFileAssignedToBridge(*rmm.GetV8Model().GetDgnFileP()))
            continue;
        CorrectSpatialTransform(rmm);
        }

    m_spatialTransformCorrectionsApplied = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::ComputeDefaultImportJobName(SubjectCR sourceMasterModelSubject)
    {
    if (nullptr == GetRootModelP())
        {
        BeAssert(false && "Call InitRootModel first");
        return;
        }

    /*
    Utf8String jobName = iModelBridge::ComputeJobSubjectName(sourceMasterModelSubject, _GetParams(), Utf8String(GetRootModelP()->GetModelName()));
    */
    Utf8String jobName = _GetParams().GetBridgeRegSubKeyUtf8();
    _GetParamsR().SetBridgeJobName(jobName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SpatialConverterBase::GetJobName(SubjectCR sourceMasterModelSubject)
    {
    Utf8String jobName = _GetParams().GetBridgeJobName();
    if (jobName.empty())
        {
        ComputeDefaultImportJobName(sourceMasterModelSubject);
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
    return jobName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnCode createUniqueSubjectCode(SubjectCR parent, Utf8CP baseName)
    {
    DgnDbR db = parent.GetDgnDb();
    DgnCode code = Subject::CreateCode(parent, baseName);
    if (!db.Elements().QueryElementIdByCode(code).IsValid())
        return code;

    int counter=1;
    do  {
        Utf8PrintfString name("%s-%d", baseName, counter);
        code = Subject::CreateCode(parent, name.c_str());
        counter++;
        }
    while (db.Elements().QueryElementIdByCode(code).IsValid());

    return code;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr SpatialConverterBase::CreateAndInsertSourceMasterModelSubject(DgnV8ModelR v8RootModel)
    {
    auto existing = FindSourceMasterModelSubject(v8RootModel);
    if (existing.IsValid())
        {
        BeAssert(false && "Caller should have checked for existing SourceMasterModel Subject");
        return existing;
        }

    // Make sure the code is unique among Subjects that are children of the root subject.
    // Make sure the code is *the same* for all bridges based on the V8 converter. For example, IModelBridgeForMstn and ABD bridge must use the same code,
    // as their bridge job subjects must be children of the same master model subject.
    Utf8String sourceMasterModelName("IModelBridgeForMstn master model ");     
    sourceMasterModelName.append(GetSyncInfo().GetUniqueNameForFile(*v8RootModel.GetDgnFileP()));
    sourceMasterModelName.append(", ").append(Utf8String(v8RootModel.GetModelName()));

    Utf8String sourceMasterModelUserLabel(IssueReporter::FmtFileBaseName(*v8RootModel.GetDgnFileP()));

    auto code = createUniqueSubjectCode(*GetDgnDb().Elements().GetRootSubject(), sourceMasterModelName.c_str());
    auto sourceMasterModelId = GetDgnDb().Elements().QueryElementIdByCode(code);
    if (sourceMasterModelId.IsValid())
        {
        BeAssert(false && "how could I find an existing SourceMasterModel Subject element if I constructed a unique code?");
        return GetDgnDb().Elements().Get<Subject>(sourceMasterModelId); //??
        }

    auto ed = Subject::Create(*GetDgnDb().Elements().GetRootSubject(), code.GetValueUtf8());

    ed->SetUserLabel(sourceMasterModelUserLabel.c_str());
    
    Json::Value modelProps(Json::nullValue);
    modelProps["Type"] = "SourceMasterModel";
    modelProps["rootModelExternalAspectInstanceId"] = "<tbd>";      // to be filled in by InitializeJob
    ed->SetSubjectJsonProperties(Subject::json_Model(), modelProps);

    auto sourceMasterModelSubject = dynamic_cast<Subject const*>(ed->Insert().get());

    iModelBridge::InsertElementHasLinksRelationship(GetDgnDb(), sourceMasterModelSubject->GetElementId(), GetRepositoryLinkId(*v8RootModel.GetDgnFileP()));

    return sourceMasterModelSubject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::SetRootModelAspectIdInSourceMasterModelSubject(SubjectCR sourceMasterModelSubject)
    {
    auto sourceMasterModelSubjectEd = sourceMasterModelSubject.MakeCopy<Subject>();
    auto sourceMasterModelSubjectProps = sourceMasterModelSubjectEd->GetSubjectJsonProperties(Subject::json_Model());
    sourceMasterModelSubjectProps["rootModelExternalAspectInstanceId"] = iModelExternalSourceAspect::UInt64ToString(m_rootModelMapping.GetAspect().GetECInstanceId().GetValue());
    sourceMasterModelSubjectEd->SetSubjectJsonProperties(Subject::json_Model(), sourceMasterModelSubjectProps);
    sourceMasterModelSubjectEd->Update();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceId SpatialConverterBase::GetRootModelAspectIdFromSourceMasterModelSubject(SubjectCR sourceMasterModelSubject) const
    {
    auto sourceMasterModelSubjectProps = sourceMasterModelSubject.GetSubjectJsonProperties(Subject::json_Model());
    auto idvalue = iModelExternalSourceAspect::UInt64FromString(sourceMasterModelSubjectProps["rootModelExternalAspectInstanceId"].asCString());
    auto id = BeSQLite::EC::ECInstanceId(idvalue);
    BeAssert(id.IsValid() && "We have a sourceMasterModelSubject, but it has an invalid rootModelExternalAspectInstanceId JSON property value");
    return id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::ComputeV8AttachmentDescription(DgnAttachmentCR attachment)
    {
    BeAssert(nullptr != attachment.GetDgnModelP());
    auto& v8Model = *attachment.GetDgnModelP();
    auto& v8File = *v8Model.GetDgnFileP();

    Bentley::WString v8FileName;
    if (BSISUCCESS != DgnV8Api::DgnFile::ParsePackagedName(nullptr, nullptr, &v8FileName, v8File.GetFileName().c_str()))
        v8FileName = v8File.GetFileName();
    
    Utf8String refFileName(BeFileName::GetFileNameAndExtension(v8FileName.c_str()).c_str());

    auto modelName = _ComputeModelName(v8Model);

    auto logicalName = attachment.GetLogicalName();

    // The name/code of the attachment subject element should be what displays in the References dialog in MicroStation:
    //      [logicalname,] filename[, modelname]
    //  modelname is omitted if the attachment is to the Default model.

    Utf8String refname;

    if (0 != *logicalName)
        refname.append(Utf8String(logicalName)).append(", ");;
    
    refname.append(refFileName);

    if (!modelName.EqualsI(GetFileBaseName(v8File)))
        refname.append(", ").append(modelName);

    return refname;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::ComputeV8AttachmentPathDescription(DgnAttachmentCR att)
    {
    Utf8String path;
    auto parentAttachment = att.GetParentDgnAttachmentCP();
    if (nullptr != parentAttachment)
        {
        path = ComputeV8AttachmentPathDescription(*parentAttachment);
        path.append(" / ");
        }
    path.append(ComputeV8AttachmentDescription(att));
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::ComputeV8AttachmentPathDescriptionAsJson(DgnAttachmentCR att)
    {
    auto attachmentInfo = ComputeV8AttachmentPathDescription(att);
    
    rapidjson::Document json(rapidjson::kObjectType);
    auto& allocator = json.GetAllocator();
    json.AddMember("v8AttachmentInfo", rapidjson::Value(attachmentInfo.c_str(), allocator), allocator);
    
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);

    return buffer.GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::ComputeV8AttachmentIdPath(DgnAttachmentCR att)
    {
    Utf8String path;
    auto parentAttachment = att.GetParentDgnAttachmentCP();
    if (nullptr != parentAttachment)
        {
        path = ComputeV8AttachmentIdPath(*parentAttachment);
        path.append("/");
        }
    path.append(SyncInfo::V8ElementExternalSourceAspect::FormatSourceId(att.GetElementId()));
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::ComputeV8ElementIdPath(DgnV8EhCR eh)
    {
    Utf8String path;
    auto attachment = eh.GetModelRef()->AsDgnAttachmentCP();
    if (nullptr != attachment)
        {
        path = ComputeV8AttachmentIdPath(*attachment);
        path.append("/");
        }
    path.append(SyncInfo::V8ElementExternalSourceAspect::FormatSourceId(eh.GetElementId()));
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::ComputeV8ElementIdPath(DgnV8Api::DisplayPath const& dpath)
    {
    Utf8String path;
    auto attachment = dpath.GetRoot()->AsDgnAttachmentCP();
    if (nullptr != attachment)
        {
        path = ComputeV8AttachmentIdPath(*attachment);
        path.append("/");
        }
    path.append(SyncInfo::V8ElementExternalSourceAspect::FormatSourceId(dpath.GetTailElem()->GetElementId()));
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ComputeXSAInfo(Utf8StringR idPath, Utf8StringR v8AttachmentJson, DgnV8EhCR eh, DgnAttachmentCP att)
    {
    // If the element is *known* to be a DgnAttachment, then format it.
    if (nullptr != att)
        {
        idPath = ComputeV8AttachmentIdPath(*att);
        v8AttachmentJson = ComputeV8AttachmentPathDescriptionAsJson(*att);
        return;
        }

    // This is normal element. It may be in an attached model.
    idPath = ComputeV8ElementIdPath(eh);

    att = eh.GetModelRef()->AsDgnAttachmentCP();
    if (nullptr != att)
        v8AttachmentJson = ComputeV8AttachmentPathDescriptionAsJson(*att);
    else
        v8AttachmentJson.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialConverterBase::ImportJobCreateStatus SpatialConverterBase::InitializeJob(Utf8CP comments, ResolvedImportJob::ConverterType jtype)
    {
    BeAssert(m_rootFile.IsValid() && "Must define root file before creating the job");
    BeAssert((nullptr != m_rootModelRef) && "Must define root model before creating the job");

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

    GetRepositoryLinkId(*GetRootV8File());

    // ***
    // ***  See Job Subject XSA.jpg for a diagram of how the MasterModel, BridgeJob, Hierarchy Subject hierarchy works,
    // ***  how PhysicalPartitions are plugged in, and how ExternalSourceAspects (aka XSAs) create new relationships.
    // ***

    // Bootstrap the source master model subject element. (Note: another bridge might have created it already.)
    auto sourceMasterModelSubject = FindSourceMasterModelSubject(*GetRootModelP());
    bool alreadyHaveSourceMasterModelSubject = sourceMasterModelSubject.IsValid();
    if (!alreadyHaveSourceMasterModelSubject)
        sourceMasterModelSubject = CreateAndInsertSourceMasterModelSubject(*GetRootModelP());

    // Create a BridgeJob Subject as a child of the sourceMasterModel Subject
    Utf8String jobName = GetJobName(*sourceMasterModelSubject);

    BeAssert(!jobName.empty());

    // Check that it's OK to initialize this as a new bridge joblet.
    if (FindSoleJobSubjectForSourceMasterModel(*sourceMasterModelSubject).IsValid())
        return ImportJobCreateStatus::FailedExistingRoot;

    if (IsFileAssignedToBridge(*GetRootV8File()))   // If this joblet is supposed to convert this master model, then we must assert that it wasn't already converted by some other joblet
        {                                           // (On the other hand, this joblet may only be traversing through this master model in order to find some reference file to convert.)
        SyncInfo::V8ModelExternalSourceAspectIteratorByV8Id modelIt(*GetRepositoryLinkElement(*GetRootV8File()), *GetRootModelP());
        if (modelIt.begin() != modelIt.end())
            return ImportJobCreateStatus::FailedExistingNonRootModel;
        }

    // If we are creating a JobSubject, then we know that we are not updating the results of a previous run of this joblet
    _SetChangeDetector(false);

    // Create the "job" subject, as a child of the source master model subject
    SubjectCPtr jobSubject;
    if (true)
        {
        SubjectPtr ed = Subject::Create(*sourceMasterModelSubject, jobName);

        Json::Value v8JobProps(Json::objectValue);      // V8Bridge-specific job properties - information that is not recorded anywhere else.
        v8JobProps["BridgeType"] = "IModelBridgeForMstn";
        v8JobProps["BridgeVersion"] = _GetParams().GetBridgeVersion().ToString().c_str();
        v8JobProps["ConverterType"] = (int)jtype;
        JobSubjectUtils::InitializeProperties(*ed, _GetParams().GetBridgeRegSubKeyUtf8(), comments, &v8JobProps);
        JobSubjectUtils::SetTransform(*ed, BentleyApi::Transform::FromIdentity());
        
        jobSubject = ed->InsertT<Subject>();
        }
    if (!jobSubject.IsValid())
        return ImportJobCreateStatus::FailedInsertFailure;

    // Set up m_importJob with the subject. Do this before calling GetOrCreateJobPartitions.
    m_importJob = ResolvedImportJob(*jobSubject, jtype);

    // Create DocumentPartitions, GroupInformationPartition, and other organizer partitions, all as children of the jobSubject that we just created.
    // Write some default Categories and CodeSpecs into the models under these partitions.
    GetOrCreateJobPartitions();

    // Create the root model's "hierarchy" subject as a child of the jobSubject.
    SubjectCPtr hsubj = GetOrCreateModelSubject(GetJobSubject(), SubjectPhysicalBreakdownStructureCode, ModelSubjectType::Hierarchy);
    if (!hsubj.IsValid())
        {
        BeAssert(false);
        return ImportJobCreateStatus::FailedInsertFailure;
        }

    // Create a model into the DgnDb to represent the source master/root model.
    // Note that this will generally create a PhysicalPartition. 
    // We call SetSpatialParentSubject first, passing it the hierarchy subject that we just created, so that the the PhysicalPartition will be a child of the hierarchy subject.
    SetSpatialParentSubject(*hsubj);
    m_rootModelMapping = GetResolvedModelMapping(*m_rootModelRef->GetDgnModelP(), m_rootTrans);

    if (!alreadyHaveSourceMasterModelSubject)
        {
        // Finish creation of the SourceMasterModelSubject.
        // Go back and fix up the reference from the SourceMasterModel Subject to the root model's V8ModelExternalSourceAspect.
        // This is what will allow _GetRootModelId and FindJob to find the source master model (aka "root model").
        // We use an XSA ECInstanceId in order to identify the BIM root model unambiguously. 
        // This will work even if multiple source models are mapped into the iModel model that (also) represents the source master model (as TileFileConverter does).
        // That is because this V8ModelExternalSourceAspect is specific to one source model (and one specific transform of it).
        SetRootModelAspectIdInSourceMasterModelSubject(*sourceMasterModelSubject);
        }

    BeAssert(FindSourceMasterModelSubject(*GetRootModelP()).IsValid()); // had to wait until RootModelAspectId was filled in to do this check

    /*
    iModelExternalSourceAspect::Dump(*GetRepositoryLinkElement(*m_rootFile), nullptr, NativeLogging::SEVERITY::LOG_DEBUG);
    iModelExternalSourceAspect::Dump(*sourceMasterModelSubject, nullptr, NativeLogging::SEVERITY::LOG_DEBUG);
    iModelExternalSourceAspect::Dump(m_importJob.GetSubject(), nullptr, NativeLogging::SEVERITY::LOG_DEBUG);
    iModelExternalSourceAspect::Dump(*hsubj, nullptr, NativeLogging::SEVERITY::LOG_DEBUG);
    iModelExternalSourceAspect::Dump(*m_rootModelMapping.GetDgnModel().GetModeledElement(), nullptr, NativeLogging::SEVERITY::LOG_DEBUG);
	*/

    return ImportJobCreateStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr SpatialConverterBase::FindSourceMasterModelSubject(DgnV8ModelCR v8Model)
    {
    // *** See Job Subject XSA.jpg

    auto repositoryLinkId = GetRepositoryLinkId(*v8Model.GetDgnFileP());
        
    // Look up the SourceMasterModel Subject
    // Note: we only support a single master *model* per master *file*.
    // So, there will be one and only one master model subject for this file.
    auto stmt = GetDgnDb().GetPreparedECSqlStatement(
        "SELECT subject.ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_Subject) " subject "
        " JOIN " BIS_SCHEMA(BIS_CLASS_RepositoryLink) " rlink USING " BIS_SCHEMA(BIS_REL_ElementHasLinks) 
        " WHERE (rlink.ECInstanceId=? AND subject.Parent.Id=? AND json_extract(subject.JsonProperties, '$.Subject.Model.Type') = 'SourceMasterModel')");
    stmt->BindId(1, repositoryLinkId);
    stmt->BindId(2, GetDgnDb().Elements().GetRootSubjectId());
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto masterModelSubjectId = stmt->GetValueId<DgnElementId>(0);
        auto sourceMasterModelSubject = GetDgnDb().Elements().Get<Subject>(masterModelSubjectId);

        if (!v8Model.IsDictionaryModel())
            {
            // Check that this subject points at the specified v8 model NEEDS WORK: Can probably add a json_extract to the WHERE clause to do this...
            auto rootModelAspect = GetRootModelAspectFromSourceMasterModelSubject(*sourceMasterModelSubject);
            if (!rootModelAspect.IsValid() || (rootModelAspect.GetV8ModelId() != v8Model.GetModelId()))
                {
                continue;
                }
            }
        return sourceMasterModelSubject;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedImportJob SpatialConverterBase::FindSoleJobSubjectForSourceMasterModel(SubjectCR masterModelSubject)
    {
    // *** See Job Subject XSA.jpg

    // Each V8-based bridge is a child of the masterModelSubject. Find the one with the correct name. There will only be one.
    auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_Subject) " WHERE (Parent.Id=? AND json_extract(JsonProperties, '$.Subject.Job.Bridge') = ?)");
    stmt->BindId(1, masterModelSubject.GetElementId());
    stmt->BindText(2, _GetParams().GetBridgeRegSubKeyUtf8().c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes); // e.g., "iModelBridgeForMstn" or "ABDBridge", etc.
    if (BE_SQLITE_ROW != stmt->Step())
        return ResolvedImportJob();
    auto jobSubjectId = stmt->GetValueId<DgnElementId>(0);
    auto jobSubject = GetDgnDb().Elements().Get<Subject>(jobSubjectId);
    if (!jobSubject.IsValid())
        {
        BeAssert(false);
        return ResolvedImportJob();
        }
    
    auto jtype = (ResolvedImportJob::ConverterType)(JobSubjectUtils::GetProperty(*jobSubject, "Properties")["ConverterType"].asInt());

    return ResolvedImportJob(*jobSubject, jtype);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SpatialConverterBase::CheckJobBelongsToMe(SubjectCR jobSubject, SubjectCR masterModelSubject)
    {
    auto thisJobName = GetJobName(masterModelSubject);
    auto foundJobName = jobSubject.GetCode().GetValueUtf8();
    return (foundJobName.Equals(thisJobName))? BentleyStatus::SUCCESS: BentleyStatus::ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr SpatialConverterBase::GetOrCreateModelSubject(SubjectCR parent, Utf8StringCR modelName, ModelSubjectType stype)
    {
    Json::Value modelProps(Json::nullValue);
    modelProps["Type"] = (ModelSubjectType::Hierarchy == stype) ? "Hierarchy" : "References";

    for (auto childid : parent.QueryChildren())
        {
        auto subj = GetDgnDb().Elements().Get<Subject>(childid);
        if (subj.IsValid() && modelName.Equals(subj->GetCode().GetValueUtf8CP()) && (modelProps == subj->GetSubjectJsonProperties().GetMember(Subject::json_Model())))
            return subj;
        }

    BeAssert((!IsUpdating() || (ModelSubjectType::Hierarchy != stype)) && "You create a hierarchy subject once when you create the job");

    SubjectPtr ed = Subject::Create(parent, modelName.c_str());
    LOG.tracev("Created model subject %s for parentId %" PRIu64, modelName.c_str(), parent.GetElementId().GetValue());

    ed->SetSubjectJsonProperties(Subject::json_Model(), modelProps);

    return ed->InsertT<Subject>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertModels()
    {
    SetStepName(Converter::ProgressMessage::STEP_FINDING_MODELS(), Utf8String(GetDgnDb().GetFileName()).c_str());

    if (nullptr != m_rootModelRef && m_isRootModelSpatial)
        {
        bool haveFoundSpatialRoot = false;
        ImportSpatialModels(haveFoundSpatialRoot, *m_rootModelRef, m_rootTrans);
        if (WasAborted())
            return;
        CorrectSpatialTransforms();
        }

    _ImportDrawingAndSheetModels(m_rootModelMapping); // we also need to convert all drawing models now, so that we can analyze them for EC content.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::IsFileAssignedToBridge(DgnV8FileCR v8File) const
    {
    //1. If the file belongs to the bridge just return it.
    BeFileName fn(v8File.GetFileName().c_str());
    if (_GetParams().IsFileAssignedToBridge(fn))
        return true;

    // 2. Is it is an iModel we have two cases
    if (v8File.IsIModel())
        {
        if (v8File.IsEmbeddedFile()) //Embedded file assignments are owned by the parent.
            {
            Bentley::WString packageName = const_cast<Bentley::DgnFileP>(&v8File)->GetPackageName();
            if (_GetParams().IsFileAssignedToBridge(BeFileName(packageName.c_str())))
                return true;
            }

        return false;
        }

    //3. Now lets check for foreign file formats that do not have a bridge assignment yet.
    bool isMyFile = false;
    // Before we get the bridge affinity work for references of foreign file formats, treat them as owned, so they get processed - TFS's 916434,921023.
    auto rootFilename = _GetParams().GetInputFileName ();
    if (!DgnV8Api::DgnFile::IsSameFile(fn.c_str(), rootFilename.c_str(), DgnV8Api::FileCompareMask::BaseNameAndExtension))
        {
        DgnV8Api::DgnFileFormatType   format;
        if (Bentley::dgnFileObj_validateFile(&format, nullptr, nullptr, nullptr, nullptr, nullptr, fn.c_str()))
            {
            switch (format)
                {
                // These formats have bridges supporting their affinity:
                case DgnV8Api::DgnFileFormatType::V8:
                case DgnV8Api::DgnFileFormatType::V7:
                case DgnV8Api::DgnFileFormatType::DWG:
                case DgnV8Api::DgnFileFormatType::DXF:
                case DgnV8Api::DgnFileFormatType::RFA:
                    break;
                // Other formats currently do not have bridges and need to be owned by "this" bridge:
                default:
                    isMyFile = _GetParams().IsFileAssignedToBridge(rootFilename);
                }
            }
        }
        
    return  isMyFile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::FindSpatialV8Models(DgnV8ModelRefR thisModelRef)
    {
    DgnV8ModelR thisV8Model = *thisModelRef.GetDgnModelP();

    if (!ShouldConvertToPhysicalModel(thisV8Model))
        return;

    DgnV8FileR thisV8File = *thisV8Model.GetDgnFileP();

    GetRepositoryLinkId(thisV8File); // Register this FILE in syncinfo. Also populates m_v8files
    LOG.tracev("Found spatial model %s", IssueReporter::FmtModelRef(thisModelRef).c_str());
    m_spatialModelsSeen.insert(&thisV8Model);   // Note that we may very well encounter the same model via more than one attachment path. Each path may have a different setting for nesting depth, so keep going.

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

        FindSpatialV8Models(*attachment);
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

    RepositoryLinkId v8FileId = GetRepositoryLinkId(thisV8File);

    // NB: We must not try to skip entire files if we need to follow reference attachments. Instead, we can skip 
    //      the elements in a model if the model (i.e., the file) is unchanged.

    // Map this v8 model into the project
    ResolvedModelMapping v8mm = GetResolvedModelMapping(thisModelRef, trans);

    if (nullptr == thisModelRef.GetDgnAttachmentsP())
        return;

	// FindSpatialV8Models has already forced children of a spatial root to be spatial

    SubjectCPtr parentRefsSubject = &_GetSpatialParentSubject();

    bool hasPushedReferencesSubject = false;
    for (DgnV8Api::DgnAttachment* attachment : *thisModelRef.GetDgnAttachmentsP())
        {
        if (nullptr == attachment->GetDgnModelP() || !_WantAttachment(*attachment))
            continue; // missing reference 

        if (!ShouldConvertToPhysicalModel(*attachment->GetDgnModelP()))
            {
            // 3D model referencing a 2D model
            // *** WIP_TEMPLATES convert the 2D model to a template and insert an instance of it at the ref attachment point
            ReportError(IssueCategory::Unsupported(), Issue::Error(), Utf8PrintfString("2D attachments to 3D are not supported. 3D:[%s]. 2D:[%s]",
                                                                                       IssueReporter::FmtModel(thisV8Model).c_str(),
                                                                                       IssueReporter::FmtAttachment(*attachment).c_str()).c_str());
            continue;
            }

        SubjectCPtr myRefsSubject = GetOrCreateModelSubject(*parentRefsSubject, ComputeV8AttachmentDescription(*attachment), ModelSubjectType::References);
        if (!myRefsSubject.IsValid())
            {
            BeAssert(false);
            ReportError(IssueCategory::Unsupported(), Issue::Error(), Utf8PrintfString("Failed to create references subject. Parent[%s]. Attachment[%s].",
                                                                                        IssueReporter::FmtModel(thisV8Model).c_str(),
                                                                                        IssueReporter::FmtAttachment(*attachment).c_str()).c_str());
            continue;
            }
        // iModelExternalSourceAspect::Dump(*myRefsSubject, nullptr, NativeLogging::SEVERITY::LOG_DEBUG);

        SetSpatialParentSubject(*myRefsSubject); // >>>>>>>>>> Push spatial parent subject

        Transform refTrans = ComputeAttachmentTransform(trans, *attachment);
        ImportSpatialModels(haveFoundSpatialRoot, *attachment, refTrans);
        }

    if (hasPushedReferencesSubject)
        SetSpatialParentSubject(*parentRefsSubject); // <<<<<<<<<<<< Pop spatial parent subject
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertSpatialLevels()
    {
    for (DgnV8FileP v8File : m_v8Files)
        {
        if (IsFileAssignedToBridge(*v8File))
            _ConvertSpatialLevelTable(*v8File);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertBridgeSpecificLevels()
    {
    for (DgnV8FileP v8File : m_v8Files)
        {
        if (IsFileAssignedToBridge(*v8File))
            _ConvertBridgeSpecificLevelsInLevelTable(*v8File);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2019
//---------------+---------------+---------------+---------------+---------------+-------
void RootModelConverter::_ConvertDrawingLevels()
    {
    if (SUCCESS != MustBeInSharedChannel("_ConvertDrawingLevels must be called in the definitions phase"))
        return;

    for (auto v8Model : m_nonSpatialModelsInModelIndexOrder)
        {
        if (!IsFileAssignedToBridge(*v8Model->GetDgnFileP()))
            continue;
        DgnV8Api::PersistentElementRefList* graphicElements = v8Model->GetGraphicElementsP();
        if (nullptr != graphicElements)
            {
            for (DgnV8Api::PersistentElementRef* v8Element : *graphicElements)
                {
                ConvertDrawingLevels(DgnV8Api::ElementHandle(v8Element));
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
void RootModelConverter::_ConvertLineStyles()
    {
    for (DgnV8FileP v8File : m_v8Files)
        {
        if (IsFileAssignedToBridge(*v8File))
            ConvertAllLineStyles(*v8File);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::ModelId RootModelConverter::_GetRootModelIdFromViewGroup()
    {
    // try to find a spatial view in the viewgroup
    for (uint32_t iView = 0; iView < DgnV8Api::MAX_VIEWS; ++iView)
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
    if (IsUpdating() && !m_params.GetRootModelChoice().IsSet()) // (Note: Some of our unit tests convert various models from the same file.)
        {
        auto sourceMasterModelSubject = FindSourceMasterModelSubject(m_rootFile->GetDictionaryModel()); // we must assume that there is only one SourceMasterModel Subject for this file.
        if (sourceMasterModelSubject.IsValid())
            {
            auto aspect = GetRootModelAspectFromSourceMasterModelSubject(*sourceMasterModelSubject);
            return aspect.GetV8ModelId();
            }
        }

    if (!IsV8Format(*m_rootFile))
        return DgnV8Api::DEFAULTMODEL;

    if (RootModelChoice::Method::ById == m_params.GetRootModelChoice().m_method)
        return m_params.GetRootModelChoice().m_id;

    if (RootModelChoice::Method::UseDefaultModel == m_params.GetRootModelChoice().m_method)
        return m_rootFile->GetDefaultModelId();

    if (RootModelChoice::Method::ByName == m_params.GetRootModelChoice().m_method)
        return m_rootFile->FindModelIdByName(WString(m_params.GetRootModelChoice().m_name.c_str(), BentleyCharEncoding::Utf8).c_str());

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
    m_consider3dElementsAsGraphics = (GetConfig().GetOptionValueBool("Consider3dElementsAsGraphics", false) || m_params.GetConsider3dElementsAsGraphics());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
RootModelConverter::~RootModelConverter()
    {
    for (auto file : m_v8Files)
        {
        // TFS#803958 - Don't leave stale appdata on the files. That is mainly a problem in the case where a new
        //              RootModelConverter will be created to process the same set of DgnFiles that were already processed
        //              (at least partially) earlier in the same session. The second instance of the converter needs to run
        //              the FindSpatialV8Models logic, which must populate the m_v8Files vector. It won't do that if the
        //              files all have appdata saying that the converter already knows about them.
        DiscardV8FileSyncInfoAppData(*file);
        }

    m_v8Files.clear();
    m_filesKeepAlive.clear();
    m_viewGroup = nullptr;
    m_rootFile = nullptr;
    m_drawingModelsKeepAlive.clear();
    if (!m_params.GetKeepHostAlive())
        {
        ClearV8ProgressMeter();
        Terminate(m_params);
        }
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
        {
        m_unchangedModels.insert(v8mm.GetDgnModel().GetModelId());
        return;
        }
    m_hadAnyChanges = true;

    DgnV8Api::DgnModel& v8Model = v8mm.GetV8Model();

    if (!v8Model.Is3D() && GetRootModelP() != &v8Model) // if this is a drawing, then it won't already be filled.
        v8Model.FillCache();

    ReportProgress();

    m_currIdPolicy = GetIdPolicyFromAppData(*v8Model.GetDgnFileP());

    uint32_t        preElementsConverted = m_elementsConverted;
    
    ConvertElementList(v8Model.GetControlElementsP(), v8mm);
    ConvertElementList(v8Model.GetGraphicElementsP(), v8mm);

    if (preElementsConverted == m_elementsConverted)
        m_unchangedModels.insert(v8mm.GetDgnModel().GetModelId());
    else
        {
        if (_GetParams().GetPushIntermediateRevisions() == iModelBridge::Params::PushIntermediateRevisions::ByModel)
            PushChangesForModel(v8mm.GetV8Model());
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Keith.Bentley                   02 / 15
//---------------------------------------------------------------------------------------
void RootModelConverter::ConvertElementsInModelWithExceptionHandling(ResolvedModelMapping const& v8mm)
    {
    ConvertElementsInModel(v8mm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::DoConvertSpatialElements()
    {
    bmap<DgnV8FileP, bmultiset<ResolvedModelMapping>> spatialModels;
    int32_t count = 0;
    for (auto& v8mm : m_v8ModelMappings)
        {
        if (!IsFileAssignedToBridge(*v8mm.GetV8Model().GetDgnFileP()))
            continue;

        if (v8mm.GetDgnModel().Is3dModel())
            {
            ++count;
            spatialModels[v8mm.GetV8Model().GetDgnFileP()].insert(v8mm);
            }
        }

    if (spatialModels.empty())
        return;

    AddTasks(count);
    for (auto& v8FileGroup: spatialModels)
        {
        for (auto& modelMapping : v8FileGroup.second)
            {
            if (WasAborted())
                return;

            SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_MODEL(), modelMapping.GetDgnModel().GetName().c_str());

            StopWatch timer(true);
            uint32_t start = GetElementsConverted();

            ConvertElementsInModelWithExceptionHandling(modelMapping);

            uint32_t convertedElementCount = (uint32_t) GetElementsConverted() - start;
            ConverterLogging::LogPerformance(timer, "Convert Spatial Elements> Model '%s' (%" PRIu32 " element(s))",
                                             modelMapping.GetDgnModel().GetName().c_str(),
                                             convertedElementCount);

            if (convertedElementCount != 0 && m_monitor.IsValid())
                m_monitor->_OnModelConverted(modelMapping, convertedElementCount);
            }

        if (_GetParams().GetPushIntermediateRevisions() == iModelBridge::Params::PushIntermediateRevisions::ByFile)
            PushChangesForFile(*v8FileGroup.first, ConverterDataStrings::SpatialData());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::ConvertNamedGroupsAndECRelationships()
    {
    StopWatch timer(true);
    uint32_t start = GetElementsConverted();

    AddTasks((int32_t) (m_v8Files.size()));
    //convert dictionary model named groups
    for (DgnV8FileP v8File : m_v8Files)
        {
        if (!IsFileAssignedToBridge(*v8File))
            continue;

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
    if (m_haveDroppedIndexDdl)
        RecreateElementRefersToElementsIndices(m_indexDdlList);
    ConverterLogging::LogPerformance(timer, "Convert Elements> ECRelationships (total)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
void RootModelConverter::UpdateCalculatedProperties()
    {
    // Need to register the ECDb expression context in order to parse the GetRelatedInstance expression
    BeSQLite::EC::ECDbExpressionSymbolContext context(*m_dgndb);

    CachedStatementPtr stmt = nullptr;
    auto stat = m_dgndb->GetCachedStatement(stmt, "SELECT Name, StrData FROM " BEDB_TABLE_Property " where Namespace='dgn_V8Expression'");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement.");
        return;
        }

    while (BE_SQLITE_ROW == stmt->Step())
        {
        Utf8String fullName = stmt->GetValueText(0);
        Utf8String schemaName, className;
        ECN::ECClass::ParseClassName(schemaName, className, fullName);
        BECN::ECClassCP ecClass = m_dgndb->Schemas().GetClass(schemaName, className);

        rapidjson::Document expressions(rapidjson::kObjectType);
        expressions.Parse(stmt->GetValueText(1));

        Utf8PrintfString sql("SELECT ECClassId, ECInstanceId, * FROM %s", ecClass->GetECSqlName().c_str());

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
            for (rapidjson::Value::ConstMemberIterator iter = expressions.MemberBegin(); iter != expressions.MemberEnd(); ++iter)
                {
                ECN::CalculatedPropertySpecificationPtr spec = ECN::CalculatedPropertySpecification::Create(iter->value.GetString(), ecClass->GetPropertyP(iter->name.GetString())->GetAsPrimitivePropertyP()->GetType());
                if (spec.IsValid())
                    {
                    BECN::ECValue v;
                    selectedInstance->GetValue(v, iter->name.GetString());

                    selectedInstance->SetValue(iter->name.GetString(), nullValue);
                    ECN::ECValue calcValue;
                    spec->Evaluate(calcValue, v, *selectedInstance, iter->name.GetString());
                    element->SetPropertyValue(iter->name.GetString(), calcValue);
                    }
                }
            element->Update();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2018
//---------------+---------------+---------------+---------------+---------------+-------
void RootModelConverter::CreatePresentationRulesWithExceptionHandling()
    {
    CreatePresentationRules();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2018
//---------------+---------------+---------------+---------------+---------------+-------
void RootModelConverter::CreatePresentationRules()
    {
    if (!m_dgndb->Schemas().ContainsSchema("IFC2x3"))
        return;

    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("IFC2x3", 1, 0, true, "IFC2x3 modifiers", "IFC2x3", "", false);
    ContentModifierP modifier = new ContentModifier("IFC2x3", "IfcObject");
    ruleset->AddPresentationRule(*modifier);
    RelatedPropertiesSpecification *spec = new RelatedPropertiesSpecification(RequiredRelationDirection_Backward, "IFC2x3:IfcRelDefinesProperties_RelatedObjects", "IFC2x3:IfcRelDefinesByPropertiesProperties", "_none_", RelationshipMeaning::SameInstance);
    modifier->AddRelatedProperty(*spec);
    spec->AddNestedRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "IFC2x3:IfcRelDefinesByPropertiesProperties_RelatingPropertyDefinition", "IFC2x3:IfcPropertySetDefinition", "", RelationshipMeaning::RelatedInstance, true));

    RulesetEmbedder embedder(*m_dgndb);
    embedder.InsertRuleset(*ruleset);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   03/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus RootModelConverter::ConvertNamedGroupsRelationships()
    {
    for (auto const& modelMapping : m_v8ModelMappings)
        {
        if (!IsFileAssignedToBridge(*modelMapping.GetV8Model().GetDgnFileP()))
            continue;

        if (BentleyApi::SUCCESS != ConvertNamedGroupsRelationshipsInModel(modelMapping.GetV8Model()))
            return BSIERROR;
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   04/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus RootModelConverter::ConvertNamedGroupsRelationshipsInModel(DgnV8ModelR v8Model)
    {
    struct NamedGroupConverter : DgnV8Api::INamedGroupMemberVisitor
        {
        private:
            RootModelConverter& m_converter;
            DgnElementId const& m_parentId;
            bool m_namedGroupOwnsMembers;
            bset<DgnElementId> m_visitedMembers;
            bmap<Utf8String, Utf8String>& m_indexDdlList;

            virtual DgnV8Api::MemberTraverseStatus VisitMember(DgnV8Api::NamedGroupMember const* member, DgnV8Api::NamedGroup const* ng, UInt32 index) override
                {
                DgnV8Api::ElementHandle memberEh(member->GetElementRef());

                DgnElementId childElementId;
                if (!m_converter.TryFindElement(childElementId, memberEh))
                    {
                    Utf8String error;
                    error.Sprintf("No BIS grouping relationship created for v8 NamedGroup Member element (%s) because the member element was not converted.",
                                  Converter::IssueReporter::FmtElement(memberEh).c_str());
                    m_converter.ReportIssue(IssueSeverity::Info, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
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

                GroupInformationElementPtr group = elementTable.GetForEdit<GroupInformationElement>(m_parentId);
                bool isInGroup = m_converter.GetSyncInfo().IsElementInNamedGroup(m_parentId, childElementId);
                if (group.IsValid() && !isInGroup)
                    {
                    if (Utf8String::IsNullOrEmpty(group->GetUserLabel()))
                        {
                        group->SetUserLabel(Utf8String(ng->GetName().c_str()).c_str());
                        group->Update();
                        }
                    DgnDbStatus status;
                    if (m_indexDdlList.size() == 0)
                        {
                        m_converter.DropElementRefersToElementsIndices(m_indexDdlList, "uix_bis_ElementGroupsMembers_sourcetarget");
                        m_converter.DropElementRefersToElementsIndices(m_indexDdlList, "uix_bis_ElementRefersToElements_sourcetargetclassid");
                        }
                    if (DgnDbStatus::BadRequest == (status = ElementGroupsMembers::Insert(*group, *child, 0)))
                        {
                        Utf8String error;
                        error.Sprintf("Failed to add child element %s to group %" PRIu64 "", Converter::IssueReporter::FmtElement(memberEh).c_str(), m_parentId.GetValue());
                        m_converter.ReportIssue(IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
                        }
                    if (BentleyApi::SUCCESS != m_converter.GetSyncInfo().AddElementToNamedGroup(m_parentId, childElementId))
                        {
                        Utf8String info;
                        info.Sprintf("Failed to add %" PRIu64 "(%s) to group %" PRIu64, childElementId.GetValue(), Converter::IssueReporter::FmtElement(memberEh).c_str(), m_parentId.GetValue());
                        LOG.warning(info.c_str());
                        }
                    }

                return DgnV8Api::MemberTraverseStatus::Continue;
                }

        public:
            NamedGroupConverter(RootModelConverter& converter, DgnElementId const& parentId, bool namedGroupOwnsMembers, bmap<Utf8String, Utf8String> indexDdlList)
                : m_converter(converter), m_parentId(parentId), m_namedGroupOwnsMembers(namedGroupOwnsMembers), m_indexDdlList(indexDdlList)
                {}

            bool WasVisited(DgnElementId member)
                {
                return m_visitedMembers.find(member) != m_visitedMembers.end();
                }
        };

    if (WasAborted())
        return BentleyApi::SUCCESS;

    //TODO: do we need to load the model and its attachments or can we consider them loaded already?

    RepositoryLinkId v8FileId = GetRepositoryLinkId(*v8Model.GetDgnFileP());
    bset<DgnV8Api::ElementId> const* namedGroupsWithOwnershipHintPerFile = nullptr;
    V8NamedGroupInfo::TryGetNamedGroupsWithOwnershipHint(namedGroupsWithOwnershipHintPerFile, v8FileId);

    bmap<Utf8String, Utf8String> indexDdlList;

    for (auto v8El : *v8Model.GetControlElementsP())
        {
        BeAssert(v8El != nullptr);
        DgnV8Api::ElementHandle v8eh(v8El);
        if (V8ElementTypeHelper::GetType(v8eh) != V8ElementType::NamedGroup)
            continue;

        DgnV8Api::DgnModel* ngRootModel = &v8Model;
        if (ngRootModel->IsDictionaryModel()) // *** TBD: Check that file was orginally DWG
            {
            DgnV8Api::DgnModel* defaultModel = v8Model.GetDgnFileP()->FindLoadedModelById(v8Model.GetDgnFileP()->GetDefaultModelId());
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
        if (!TryFindElement(ngElementId, v8eh))
            {
            Utf8String error;
            error.Sprintf("No BIS grouping created for v8 NamedGroup element (%s) because the NamedGroup was not converted.",
                          Converter::IssueReporter::FmtElement(v8eh).c_str());
            ReportIssue(IssueSeverity::Info, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
            continue;
            }

        const bool namedGroupOwnsMembersFlag = namedGroupsWithOwnershipHintPerFile != nullptr && namedGroupsWithOwnershipHintPerFile->find(v8eh.GetElementId()) != namedGroupsWithOwnershipHintPerFile->end();
        NamedGroupConverter ngConverter(*this, ngElementId, namedGroupOwnsMembersFlag, indexDdlList);
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
    if (indexDdlList.size() > 0)
        RecreateElementRefersToElementsIndices(indexDdlList);

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// As all v8 relationships end up in the same table (bis_ElementRefersToElements)
// it gets a lot of indexes. These hurt performance a lot, so we drop the indexes before the bulk insert
// and re-add them later.
// @bsimethod                                   Carole.MacDonald            12/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyApi::BentleyStatus RootModelConverter::DropElementRefersToElementsIndices(bmap<Utf8String, Utf8String>& indexDdlList, Utf8String search)
    {
    if (!IsUpdating())
        {
        StopWatch timer(true);
        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(*m_dgndb, "SELECT name, sql FROM sqlite_master WHERE tbl_name='bis_ElementRefersToElements' AND type='index'"))
            return BentleyApi::ERROR;

        while (stmt.Step() == BE_SQLITE_ROW)
            {
            Utf8String index(stmt.GetValueText(1));
            if (!index.Contains(search))
                continue;
            indexDdlList[Utf8String(stmt.GetValueText(0))] = Utf8String(stmt.GetValueText(1));
            break;
            }

        stmt.Finalize();

        for (auto const& index : indexDdlList)
            {
            Utf8String sql("DROP INDEX ");
            sql.append(index.first);
            if (BE_SQLITE_OK != GetDgnDb().ExecuteSql(sql.c_str()))
                return BentleyApi::ERROR;
            }

        Utf8PrintfString msg("Convert Elements> ECRelationships: Dropped %d indices for bulk insertion into BisCore:ElementRefersToElements class hierarchy for index %s.", indexDdlList.size(), search.c_str());
        ConverterLogging::LogPerformance(timer, msg.c_str());
        }
    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyApi::BentleyStatus RootModelConverter::RecreateElementRefersToElementsIndices(bmap<Utf8String, Utf8String>& indexDdlList)
    {
    //recreate indexes that were previously dropped
    if (!IsUpdating())
        {
        StopWatch timer(true);
        for (auto const& index : indexDdlList)
            {
            DbResult result = GetDgnDb().TryExecuteSql(index.second.c_str());
            if (BE_SQLITE_OK != result)
                {
                // If we have a constraint violation, try to delete all of the duplicate rows and then try re-applying the index
                if (BE_SQLITE_CONSTRAINT_UNIQUE == result)
                    {
                    bvector<Utf8String> tokens;
                    BeStringUtilities::Split(index.second.c_str(), "=", tokens);
                    if (tokens.size() == 2)
                        {
                        uint64_t classId;
                        BeStringUtilities::ParseUInt64(classId, tokens[1].c_str());
                        if (classId != 0)
                            {
                            Utf8PrintfString sql("delete from bis_ElementRefersToElements where rowid not in (select min(rowid) from bis_ElementRefersToElements where ECClassId=% " PRIu64 " group by SourceId, TargetId) and ECClassId=% " PRIu64, classId, classId);
                            result = GetDgnDb().TryExecuteSql(sql.c_str());
                            if (BE_SQLITE_OK == result)
                                result = GetDgnDb().TryExecuteSql(index.second.c_str());
                            }
                        }
                    else
                        {
                        CachedStatementPtr stmt = nullptr;
                        auto stat = m_dgndb->GetCachedStatement(stmt, "select SourceId, TargetId, ECClassId from bis_ElementRefersToElements group by SourceId, TargetId, ECClassId having count(*) > 1 ");
                        while (BE_SQLITE_ROW == stmt->Step())
                            {
                            LOG.tracev("Looking for rowids for %" PRIu64 ", %" PRIu64 ", %" PRIu64, stmt->GetValueId<DgnElementId>(0).GetValue(), stmt->GetValueId<DgnElementId>(1).GetValue(), stmt->GetValueId<ECN::ECClassId>(2).GetValue());
                            CachedStatementPtr stmt2 = nullptr;
                            auto stat2 = m_dgndb->GetCachedStatement(stmt2, "select rowid from bis_ElementRefersToElements where SourceId=? and TargetId=? and ECClassId=?");
                            stmt2->BindId(1, stmt->GetValueId<DgnElementId>(0));
                            stmt2->BindId(2, stmt->GetValueId<DgnElementId>(1));
                            stmt2->BindId(3, stmt->GetValueId<ECN::ECClassId>(2));
                            stmt2->Step(); // skip the first result as we need to keep one
                            while (BE_SQLITE_ROW == stmt2->Step())
                                {
                                CachedStatementPtr stmt3 = nullptr;
                                m_dgndb->GetCachedStatement(stmt3, "delete from bis_ElementRefersToElements where rowid=?");
                                stmt3->BindInt64(1, stmt2->GetValueInt64(0));
                                result = stmt3->Step();
                                LOG.tracev("Deleting rowid %ld", stmt2->GetValueInt64(0));
                                }
                            }
                        result = GetDgnDb().TryExecuteSql(index.second.c_str());
                        }
                    }

                // If we didn't succeed, fatally end as we can't make schema changes at this point in the process
                if (BE_SQLITE_OK != result)
                    {
                    Utf8String error;
                    error.Sprintf("Failed to recreate index '%s' for BisCore:ElementRefersToElements class hierarchy: %s", index.second.c_str(), GetDgnDb().GetLastError().c_str());
                    ReportIssue(IssueSeverity::Fatal, IssueCategory::Sync(), Issue::Message(), error.c_str());
                	OnFatalError(Converter::IssueCategory::CorruptData());
                    return BentleyApi::ERROR;
                    }
                }
            }

        ConverterLogging::LogPerformance(timer, "Convert Elements> ECRelationships: Recreated indices for BisCore:ElementRefersToElements class hierarchy.");
        }
    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus RootModelConverter::ConvertECRelationships()
    {
    if (m_skipECContent)
        return BentleyApi::SUCCESS;

    bvector<DgnV8ModelP> visitedModels;
    for (auto& modelMapping : m_v8ModelMappings)
        {
        if (!IsFileAssignedToBridge(*modelMapping.GetV8Model().GetDgnFileP()))
            continue;

        ConvertECRelationshipsInModel(modelMapping.GetV8Model());
        if (WasAborted())
            return BentleyApi::ERROR;
        visitedModels.push_back(&modelMapping.GetV8Model());
        }

    //analyze named groups in dictionary models
    for (DgnV8FileP v8File : m_v8Files)
        {
        if (!IsFileAssignedToBridge(*v8File))
            continue;

        DgnV8ModelR dictionaryModel = v8File->GetDictionaryModel();
        if (std::find(visitedModels.begin(), visitedModels.end(), &dictionaryModel) == visitedModels.end())
            ConvertECRelationshipsInModel(dictionaryModel);
        if (WasAborted())
            return BentleyApi::ERROR;
        }

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyApi::BentleyStatus RootModelConverter::ConvertECRelationshipsInModel(DgnV8ModelR v8Model)
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
    if (m_beginConversionCalled)
        {
        BeAssert(false && "Call _BeginConversion only once");
        return;
        }
    m_beginConversionCalled = true;

    if (!GetImportJob().IsValid() || (GetImportJob().GetConverterType() != ResolvedImportJob::ConverterType::RootModel))
        {
        OnFatalError();
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::UnmapModelsNotAssignedToBridge()
    {
    // TFS#922840 - work around the fact that a bridge will create empty models as it traverses
    //                  references that belong to other bridges.
    bvector<ResolvedModelMapping> mappingsToRemove;
    bvector<BentleyApi::Dgn::DgnModelPtr> keepAlive;
    DgnV8ModelP rootModel = GetRootModelP();
    for (auto& modelMapping : m_v8ModelMappings)
        {
        if (IsFileAssignedToBridge(*modelMapping.GetV8Model().GetDgnFileP()))
            continue;

        BentleyApi::Dgn::DgnModelPtr mref = &modelMapping.GetDgnModel();
        DgnModelId modelId = mref->GetModelId();
        if (!IsBimModelAssignedToJobSubject(modelId))
            continue;
        
        DgnElementId partition = mref->GetModeledElementId();
        //Check whether the root models partition element is the same as this models parition rppt
        keepAlive.push_back(mref);
        
        DgnElementCPtr element = GetDgnDb().Elements().GetElement(partition);
        bool isRootModel = &modelMapping.GetV8Model() == rootModel;
        if (!isRootModel)
            {
            DgnModelId modelId = mref->GetModelId();
            mref->Delete();
            if (element.IsValid())
                element->Delete();
            }
        else
            {
            mref->SetIsPrivate(true);
            mref->Update();
            }
        mappingsToRemove.push_back(modelMapping);

        Utf8PrintfString msg("Unmapped %ls in %ls not owned by %ls", modelMapping.GetV8Model().GetModelName(), modelMapping.GetV8Model().GetDgnFileP()->GetFileName().c_str(), _GetParams().GetBridgeRegSubKey().c_str());
        ReportIssue(IssueSeverity::Info, IssueCategory::Filtering(), Issue::Message(), msg.c_str());
        
        }
    for (auto const& mappingToRemove : mappingsToRemove)
        {
        m_v8ModelMappings.erase(mappingToRemove);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_FinishConversion()
    {
    if (!m_beginConversionCalled)
        {
        LOG.error ("_FinishConversion called without _BeginConversion");
        BeAssert(false && "_FinishConversion called without _BeginConversion");
        return;
        }
    UnmapModelsNotAssignedToBridge(); // just in case any snuck back in

    ConvertNamedGroupsAndECRelationships();   // Now that we know all elements, work on the relationships between elements.
    if (WasAborted())
        return;

    if (IsUpdating())
        UpdateCalculatedProperties();
    _RemoveUnusedMaterials();

    m_linkConverter->PurgeOrphanedLinks();

    EmbedSpecifiedFiles();

    for (auto f : m_finishers)
        {
        LOG.tracev ("calling DgnDbElementHandlerExtension::_OnFinishConversion for %p", f);
        f->_OnFinishConversion(*this);
        LOG.tracev ("called DgnDbElementHandlerExtension::_OnFinishConversion for %p", f);
        }
	//We need to wait until call out extension handler sets up the relationships for view attachments
    _ConvertDynamicViews();
    for (auto xdomain : XDomainRegistry::s_xdomains)
        {
        LOG.tracev ("calling XDomain::_OnFinishConversion for %p", xdomain);
        xdomain->_OnFinishConversion(*this);
        LOG.tracev ("called XDomain::_OnFinishConversion for %p", xdomain);

        LOG.tracev ("calling XDomain::_GetManuallyCreatedElementIds for %p", xdomain);

        // Add elements that were manually created by the grid domain to the seen element list
        for (DgnElementId const& elementId : xdomain->_GetManuallyCreatedElementIds(*this))
            GetChangeDetector()._OnElementSeen(*this, elementId);

        LOG.tracev ("called XDomain::_GetManuallyCreatedElementIds for %p", xdomain);
        }

    if (!IsUpdating())
        {
        EmbedFilesInSource(GetRootFileName());
        }
    else
        {
        if (_GetParams().DoDetectDeletedModelsAndElements())
            {
            SetStepName(ProgressMessage::STEP_DETECT_DELETIONS());
            StopWatch timer(true);

            // Detect deletions in the V8 files that we processed. (Don't assume we saw all V8 files.)
            for (DgnV8FileP v8File : m_v8Files)
                {
                if (!IsFileAssignedToBridge(*v8File))
                    continue;

                GetChangeDetector()._DetectDeletedElementsInFile(*this, *v8File);
                GetChangeDetector()._DetectDeletedViewsInFile(*this, *v8File);
                GetChangeDetector()._DetectDeletedModelsInFile(*this, *v8File);    // NB: call this *after* DetectDeletedElements!
                }
            GetChangeDetector()._DetectDeletedElementsEnd(*this);
            GetChangeDetector()._DetectDeletedViewsEnd(*this);
            GetChangeDetector()._DetectDeletedModelsEnd(*this);
            ConverterLogging::LogPerformance(timer, "Remove deleted items");
            }
        }

    // See "When to mark file as processed" comment below
    for (DgnFileP v8File : m_v8Files)
        {
        if (!IsFileAssignedToBridge(*v8File))
            continue;
        // Note: Bridges do not retain their locks on RepositoryLink elements. So, even if another job created this element,
        // it is safe for this bridge job to update it.
        auto rlinkEd = GetDgnDb().Elements().GetForEdit<RepositoryLink>(GetRepositoryLinkId(*v8File));
        BeFileName localFileName = GetLocalFileName(*v8File);
        auto anyChanges = iModelBridge::UpdateRepositoryLinkDocumentProperties(rlinkEd.get(), *m_dgndb, GetParams(), localFileName);
        auto rlinkXsa = SyncInfo::RepositoryLinkExternalSourceAspect::GetAspectForEdit(*rlinkEd);
        if (rlinkXsa.IsValid())
            anyChanges |= rlinkXsa.Update(GetSyncInfo().ComputeFileInfo(*v8File), _GetParams().IgnoreStaleFiles());

        if (anyChanges)
            rlinkEd->Update();
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
    DefinitionModelP        m_definitionModel;

    ConvertModelACSTraverser(ResolvedModelMapping const& v8mm, DefinitionModelP dm, double toMeters) : m_v8mm(v8mm), m_toMeters(toMeters), m_definitionModel(dm) {}
    UInt32 _GetACSTraversalOptions() override { return 0; }

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

        AuxCoordSystemPtr   acsElm = AuxCoordSystem::CreateNew(m_v8mm.GetDgnModel(), m_definitionModel, acsName);
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
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootModelConverter::DoBeginConversion()
    {
    _OnConversionStart();
    _BeginConversion();
    return WasAborted()? BSIERROR: BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootModelConverter::DoFinishConversion()
    {
    _FinishConversion();
    if (WasAborted())
        {
        LOG.error ("DoFinishConversion was aborted");
        return BSIERROR;
        }

    _OnConversionComplete();

    if (ShouldCreateIntermediateRevisions())
        PushChangesForFile(*GetRootV8File(), ConverterDataStrings::GlobalProperties());

    if (WasAborted())
        {
        LOG.error ("DoFinishConversion was aborted!");//note the exclamation, it is a hint where it got aborted.
        return BSIERROR;
        }
    
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootModelConverter::MakeDefinitionChanges()
    {
    if (BSISUCCESS != MustBeInSharedChannel("MakeDefinitionChanges must be called only in the shared channel."))
        return BSIERROR;

    if (!m_beginConversionCalled)
        {
        if (SUCCESS != DoBeginConversion() || WasAborted())     // must call this first, to initialize the ChangeDetector, which MakeDefinitionChanges will use
            {
            return BSIERROR;
            }
        }

    SetStepName(Converter::ProgressMessage::STEP_CONVERTING_STYLES());
    _ConvertLineStyles();
    if (WasAborted())
        return ERROR;

    if (!m_isRootModelSpatial)
        return BSISUCCESS;

    for (auto v8Model : m_spatialModelsInAttachmentOrder)
        {
        if (IsFileAssignedToBridge(*v8Model->GetDgnFileP()))
            {
            SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_MATERIALS());
            ConvertModelMaterials(*v8Model);
            }
        }

    _ConvertSpatialLevels();
    _ConvertDrawingLevels();


    for (auto xdomain : XDomainRegistry::s_xdomains)
        {
        LOG.tracev("calling XDomain::_MakeDefinitionChanges for %p", xdomain);
        xdomain->_MakeDefinitionChanges(*this);
        LOG.tracev("called XDomain::_MakeDefinitionChanges for %p", xdomain);
        }

    // NB: It is up to ConvertData to call DoEndConversion. Don't do that here!

    // The framework (iModelBridgeFwk or SACAdapter) guarantees that ConvertData will be called in the same session. 
    // The bim and source will NOT be closed and re-opened between the definition and data conversion steps. 
    // That means I can assume that all current state, including the existing ChangeDetector, m_newlyDiscoveredModels, 
    // etc. will be carried over to ConvertData.

    CreatePresentationRulesWithExceptionHandling();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  RootModelConverter::ConvertData()
    {
    if (BSISUCCESS != MustBeInNormalChannel("ConvertData must be called in the normal channel."))
        return BSIERROR;

    if (!m_beginConversionCalled)
        {
        if (SUCCESS != DoBeginConversion() || WasAborted())     // must call this first, to initialize the ChangeDetector, which MakeDefinitionChanges will use
            {
            return BSIERROR;
            }
        }

    AddSteps(10);

    StopWatch totalTimer(true);

    StopWatch timer(true);

    ConverterLogging::LogPerformance(timer, "Begin Conversion");

    _ConvertBridgeSpecificLevels();

    timer.Start();

    _ConvertModels();       // This is where we discover just about all of the V8 files and models that we'll need to mine for data in the subsequent steps
    if (WasAborted())
        return ERROR;

    UnmapModelsNotAssignedToBridge();

    ConverterLogging::LogPerformance(timer, "Convert Models");

    AddTasks((int32_t) (m_v8ModelMappings.size()));
    for (auto& modelMapping : m_v8ModelMappings)
        {
        if (WasAborted())
            return ERROR;

        if (!IsFileAssignedToBridge(*modelMapping.GetV8Model().GetDgnFileP()))
            continue;

        ConvertModelACSTraverser acsTraverser(modelMapping, GetJobDefinitionModel().get(), ComputeUnitsScaleFactor(modelMapping.GetV8Model()));
        DgnV8Api::IACSManager::GetManager().Traverse(acsTraverser, &modelMapping.GetV8Model());

        // If we're updating, need to see if the master model's subject needs updating, and if the model names need updating.  This will
        // happen when a filename has been modified and we're relying on the PW doc guid to identify the file.
        if (IsUpdating())
            {
            SubjectCPtr sourceMasterModelSubject = FindSourceMasterModelSubject(modelMapping.GetV8Model());
            if (!sourceMasterModelSubject.IsValid())
                continue;
            Utf8String sourceMasterModelUserLabel(IssueReporter::FmtFileBaseName(*modelMapping.GetV8Model().GetDgnFileP()));
            if (!sourceMasterModelUserLabel.Equals(sourceMasterModelSubject->GetUserLabel()))
                {
                auto subject = GetDgnDb().Elements().GetForEdit<DgnElement>(sourceMasterModelSubject->GetElementId());
                subject->SetUserLabel(sourceMasterModelUserLabel.c_str());
                subject->Update();
                }

            Utf8String newModelName = _ComputeModelName(modelMapping.GetV8Model()).c_str();
            Utf8String originalModelName = modelMapping.GetDgnModel().GetName();
            if (!newModelName.Equals(originalModelName))
                {
                auto modeledElement = m_dgndb->Elements().GetElement(modelMapping.GetDgnModel().GetModeledElementId())->CopyForEdit();
                DgnCodeCR code = modeledElement->GetCode();
                DgnCode newCode(code.GetCodeSpecId(), code.GetScopeElementId(GetDgnDb()), newModelName);
                modeledElement->SetCode(newCode);
                if (originalModelName.Equals(modeledElement->GetUserLabel()))
                    modeledElement->SetUserLabel(newModelName.c_str());
                modeledElement->Update();
                }
            }
        }

    if (m_isRootModelSpatial)
        {
        timer.Start();
        _ConvertSpatialViews();
        if (WasAborted())
            return ERROR;

        if (ShouldCreateIntermediateRevisions())
            PushChangesForFile(*GetRootV8File(), ConverterDataStrings::ViewsAndModels());

        ConverterLogging::LogPerformance(timer, "Convert Spatial Views");

        timer.Start();
        _ConvertSpatialElements();
        if (WasAborted())
            return ERROR;

        ConverterLogging::LogPerformance(timer, "Convert Spatial Elements (total)");
        }
    else
        {
        if (ShouldCreateIntermediateRevisions())
            PushChangesForFile(*GetRootV8File(), ConverterDataStrings::ViewsAndModels());
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

    if (ShouldCreateIntermediateRevisions())
        PushChangesForFile(*GetRootV8File(), ConverterDataStrings::Sheets());

    ConverterLogging::LogPerformance(timer, "Convert Sheets (total)");

    if (BSISUCCESS != DoFinishConversion())
        return BSIERROR;

    ConverterLogging::LogPerformance(totalTimer, "Total data conversion time (%" PRIu32 " element(s))", (uint32_t) GetElementsConverted());
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

    CheckNoLevelChange(v8Level, cat, v8File);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_SetChangeDetector(bool isUpdating)
    {
    BeAssert(isUpdating == IsUpdating());

    m_changeDetector.reset(new ChangeDetector);
    // m_skipECContent = m_config.GetOptionValueBool("SkipECContent", false); [CGM] - should be no reason to need to change this value
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
ResolvedModelMapping RootModelConverter::_FindResolvedModelMappingByModelId(DgnModelId mid)
    {
    for (auto& rmm : m_v8ModelMappings)
        {
        if (rmm.GetDgnModel().GetModelId() == mid)
            return rmm;
        }
    return ResolvedModelMapping();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::PushChangesForFile(DgnV8FileR file, BentleyApi::Utf8StringCR whatData)
    {
    BentleyApi::Utf8String comment;
    if (&file == GetRootV8File())
        comment = whatData;
    else
        {
        BeFileName filename(file.GetFileName().c_str());
        comment = BentleyApi::Utf8PrintfString("%s - %s", Utf8String(filename.GetBaseName()).c_str(), whatData.c_str());
        }
    iModelBridge::PushChanges(*m_dgndb, _GetParams(), comment.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::PushChangesForFile(DgnV8FileR file, ConverterDataStrings::StringId whatDataNo)
    {
    PushChangesForFile(file, ConverterDataStrings::GetString(whatDataNo));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::PushChangesForModel(DgnV8ModelRefCR model)
    {
    PushChangesForFile(*model.GetDgnFileP(), BentleyApi::Utf8String(model.GetModelNameCP()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      4/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8FileCP RootModelConverter::_GetPackageFileOf(DgnV8FileCR f)
    {
    if (!f.IsEmbeddedFile())
        return &f;
    auto packageFilename = const_cast<DgnV8FileR>(f).GetPackageName();
    return GetV8FileByName(BentleyApi::BeFileName(packageFilename.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootModelConverter::_WasEmbeddedFileSeen(Utf8StringCR uniqueName) const
    {
    return m_embeddedFilesSeen.find(uniqueName) != m_embeddedFilesSeen.end();
    }

END_DGNDBSYNC_DGNV8_NAMESPACE

/* "When to mark file as processed"

A RepositoryLink element represents an external file, and the ExternalSourceAspect (XSA) 
for a RepositoryLink captures information about the external file.

The XSA's lastModifiedFile and lastSaveTime properties record the last-modified
time of the file as of the last time that it was converted. These XSA properties are used 
by the ChangeDetector to decide if the file can be skipped.

The XSA's lastModifiedFile and lastSaveTime properties must be updated to match the file's 
current state only *after* all content in the file has been processed. By updating these properties
at the end, we ensure that the converter will process the file again in the case where it 
crashed part way through and is then re-run.

We had a bug where we were setting the up-to-date values for these properties in the XSA
at the time that we first discovered a file and created the XSA. We were getting away with that
most of the time because we also added the file's model to a special list of newly discovered models.
But that list is in memory only. It is lost after a crash. Bug #171870.

*/