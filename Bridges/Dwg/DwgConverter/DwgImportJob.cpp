/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgImporter.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::ImportJobCreateStatus   DwgImporter::InitializeJob (Utf8CP comments)
    {
    // will create a new ImportJob - don't call this if updating!
    if (this->IsUpdating())
        return ImportJobCreateStatus::FailedExistingRoot;

    if (!m_dwgdb.IsValid())
        {
        BeAssert (false && "Root DWG file not open!");
        return ImportJobCreateStatus::FailedInsertFailure;
        }

    // open the modelspace block
    DwgDbBlockTableRecordPtr    modelspaceBlock(m_dwgdb->GetModelspaceId(), DwgDbOpenMode::ForRead);
    if (DwgDbStatus::Success != modelspaceBlock.OpenStatus())
        {
        BeAssert (false && "The ModelSpace block not available for the import job!");
        return ImportJobCreateStatus::FailedInsertFailure;
        }

    Utf8String  mastermodelName = this->_ComputeModelName (*modelspaceBlock.get());
    Utf8String  jobName = this->_ComputeImportJobName (*modelspaceBlock.get());
    BeAssert (!jobName.empty() && "Bridge job not defined!");

    // Create RepositoryLink for the root DWG file
    auto rootlinkId = this->CreateOrUpdateRepositoryLink ();
    if (!rootlinkId.IsValid())
        {
        BeAssert(false);
        return ImportJobCreateStatus::FailedExistingNonRootModel;
        }

    // Bootstrap the SourceMasterModel subject element. (Note: another bridge might have created it already and it may not be from DWG.)
    auto sourceMasterModelSubject = this->FindSourceMasterModelSubject(rootlinkId, mastermodelName, true);
    if (!sourceMasterModelSubject.IsValid())
        return ImportJobCreateStatus::FailedExistingNonRootModel;

    // since this is a create job, an existing import job under SourceMasterModel subject is not expected - bail out the job!
    if (this->FindSoleImportJobForSourceMasterModel(*sourceMasterModelSubject).IsValid())
        return ImportJobCreateStatus::FailedExistingRoot;

    // set default change detector (no-op):
    this->_SetChangeDetector (false);

    // Create a import job subject, as a child of the SourceMasterModel subject
    SubjectPtr  newSubject = Subject::Create (*sourceMasterModelSubject, jobName);
    if (!newSubject.IsValid())
        return ImportJobCreateStatus::FailedInsertFailure;

    // add some properties to the subject
    Utf8String  versionInfo;
    if (DwgHelper::GetImporterModuleVersion(versionInfo) != BSISUCCESS)
        versionInfo.Sprintf ("%d", DwgHelper::GetDwgImporterVersion()); 

    Utf8String  bridgeType(GetOptions().GetBridgeRegSubKey().c_str());
    if (bridgeType.empty())
        bridgeType.assign ("DWG");

    Json::Value jobProp(Json::objectValue);
    jobProp["BridgeType"] = bridgeType.c_str();
    jobProp["BridgeVersion"] = versionInfo.c_str();
    JobSubjectUtils::InitializeProperties(*newSubject, this->GetOptions().GetBridgeRegSubKeyUtf8(), comments, &jobProp);

    SubjectCPtr jobSubject = newSubject->InsertT<Subject>();
    if (!jobSubject.IsValid())
        return ImportJobCreateStatus::FailedInsertFailure;

    // Set up m_importJob with the subject.
    m_importJob = ResolvedImportJob (*jobSubject);

    // Create the job-specific stuff in the DgnDb (relative to the job subject).
    this->GetOrCreateJobPartitions ();

    // Create the root model's "hierarchy" subject. The root model's physical partition will be a child of that.
    m_spatialParentSubject = this->GetOrCreateModelSubject(this->GetJobSubject(), mastermodelName, ModelSubjectType::Hierarchy);
    if (!m_spatialParentSubject.IsValid())
        {
        BeAssert (false && "Failed creating parent model subject");
        return ImportJobCreateStatus::FailedInsertFailure;
        }

    // Map the root model into the DgnDb. Note that this will generally create a partition, which is relative to the job subject,
    //    So, the job subject and its framework must be created first.
    m_rootDwgModelMap = this->_GetOrCreateRootModel (nullptr);
    if (!m_rootDwgModelMap.IsValid())
        {
        BeAssert (false && "No root DWG model!");
        return ImportJobCreateStatus::FailedInsertFailure;
        }

    // ready to set the SourceMasterModel subject
    this->SetRootModelAspectIdInSourceMasterModelSubject(*sourceMasterModelSubject);

    return ImportJobCreateStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::ImportJobLoadStatus DwgImporter::FindJob ()
    {
    // called when updating bim (i.e. an incremental job)
    DwgDbBlockTableRecordPtr    modelspaceBlock(m_dwgdb->GetModelspaceId(), DwgDbOpenMode::ForRead);
    if (DwgDbStatus::Success != modelspaceBlock.OpenStatus())
        {
        BeAssert (false && "The ModelSpace block not available the import job!");
        return ImportJobLoadStatus::FailedNotFound;
        }

    Utf8String  mastermodelName = this->_ComputeModelName (*modelspaceBlock.get());

    // ensure a repository for the root file
    auto rootlinkId = this->CreateOrUpdateRepositoryLink();
    if (!rootlinkId.IsValid())
        return ImportJobLoadStatus::FailedNotFound;
    
    // find the SourceMasterModel subject - do not create one
    auto sourceMasterModelSubject = this->FindSourceMasterModelSubject(rootlinkId, mastermodelName);
    if (!sourceMasterModelSubject.IsValid())
        return ImportJobLoadStatus::FailedNotFound;

    auto rootModelAspect = this->GetRootModelAspectFromSourceMasterModelSubject(*sourceMasterModelSubject);
    if (rootModelAspect.GetDwgModelHandle() != modelspaceBlock->GetObjectId().GetHandle())
        {
        auto    modelId = rootModelAspect.GetModelId().GetValue();
        this->ReportError (IssueCategory::Sync(), Issue::Error(), Utf8PrintfString("SourceMasterModel %I64d is not owned by DwgBridge", modelId).c_str());
        }

    // find the import job under MasterSourceModel subject
    m_importJob = this->FindSoleImportJobForSourceMasterModel(*sourceMasterModelSubject);
    if (!m_importJob.IsValid())
        return ImportJobLoadStatus::FailedNotFound;

    // *** TRICKY: If this is called by the framework as a check *after* it calls _IntializeJob, then don't change the change detector!
    if (IsUpdating() || !this->_HaveChangeDetector())
        this->_SetChangeDetector (true);

    this->GetOrCreateJobPartitions ();
    m_rootDwgModelMap = this->_GetOrCreateRootModel (&rootModelAspect);
    if (!m_rootDwgModelMap.IsValid())
        {
        this->ReportError (IssueCategory::Sync(), Issue::Error(), "Failed creating the root spatial model");
        return ImportJobLoadStatus::FailedNotFound;
        }

    this->CheckSameRootModelAndUnits ();

    // Do not apply job transform here - wait till existing models retreived from the syncInfo, see _ImportDwgModels!

    // There's only one hierarchy subject for a job. Look it up.
    auto found = this->GetJobHierarchySubject ();
    if (found.IsValid())
        this->SetSpatialParentSubject (*found);
    else
        return ImportJobLoadStatus::FailedNotFound;

    return ImportJobLoadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/18
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionModelPtr  DwgImporter::GetOrCreateJobDefinitionModel ()
    {
    static Utf8CP   s_definitionPartitionName = "DwgDefinitionModel";
    if (m_jobDefinitionModel.IsValid() && m_jobDefinitionModel->GetModelId().IsValid())
        return  m_jobDefinitionModel;

    // get or create the importer job partition
    auto const& jobSubject = this->GetJobSubject ();
    Utf8String  utf8Name (m_rootFileName.GetFileNameWithoutExtension());
    Utf8PrintfString    partitionName("%s:%s", s_definitionPartitionName, utf8Name.c_str());

    auto partitionCode = DefinitionPartition::CreateCode (jobSubject, partitionName);
    auto partitionId = m_dgndb->Elements().QueryElementIdByCode (partitionCode);
    if (!partitionId.IsValid())
        {
        // create a new partition
        auto partition = DefinitionPartition::CreateAndInsert (jobSubject, partitionCode.GetValueUtf8CP());
        if (!partition.IsValid())
            {
            this->ReportError (IssueCategory::Unknown(), Issue::CantCreateModel(), partitionCode.GetValueUtf8CP());
            return  nullptr;
            }
        partitionId = partition->GetElementId ();
        }
    if (!partitionId.IsValid())
        return nullptr;

    // if the model exists, we are done!
    m_jobDefinitionModel = m_dgndb->Models().Get<DefinitionModel> (DgnModelId(partitionId.GetValueUnchecked()));
    if (m_jobDefinitionModel.IsValid())
        return m_jobDefinitionModel;
    
    auto defPartition = m_dgndb->Elements().Get<DefinitionPartition> (partitionId);
    if (!defPartition.IsValid())
        return nullptr;

    // create & insert a new DefinitionModel in our partition
    m_jobDefinitionModel = DefinitionModel::CreateAndInsert (*defPartition);
    if (!m_jobDefinitionModel.IsValid() || !m_jobDefinitionModel->GetModelId().IsValid())
        {
        BeAssert (false && "GeometryParts model is not created or not inserted successfully!");
        m_jobDefinitionModel = nullptr;
        }
    return m_jobDefinitionModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr DwgImporter::GetOrCreateModelSubject (SubjectCR parent, Utf8StringCR modelName, ModelSubjectType stype)
    {
    Json::Value modelProps(Json::nullValue);
    switch (stype)
        {
        case ModelSubjectType::Hierarchy:
            modelProps["Type"] = "Hierarchy";
            break;
        case ModelSubjectType::References:
            modelProps["Type"] = "References";
            break;
        default:
            BeAssert (false && "Unsupported ModelSubjectType!!");
            return  nullptr;
        }

    for (auto childid : parent.QueryChildren())
        {
        auto subj = GetDgnDb().Elements().Get<Subject>(childid);
        if (subj.IsValid() && subj->GetCode().GetValue().Equals(modelName.c_str()) && (modelProps == subj->GetSubjectJsonProperties().GetMember(Subject::json_Model())))
            return subj;
        }

    BeAssert((!IsUpdating() || (ModelSubjectType::Hierarchy != stype)) && "You create a hierarchy subject once when you create the job");

    SubjectPtr ed = Subject::Create(parent, modelName.c_str());

    ed->SetSubjectJsonProperties(Subject::json_Model(), modelProps);

    // set user label to help the element name display in Navigator's version comparison - TFS 915733:
    Utf8PrintfString    userLabel("%s %s %s", modelProps["Type"].asCString(), ModelSubjectType::Hierarchy==stype ? "of" : "in", modelName.c_str());
    ed->SetUserLabel (userLabel.c_str());

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
void DwgImporter::ComputeDefaultImportJobName (Utf8StringCR rootModelName) const
    {
    DgnElements&    bimElements = this->GetDgnDb().Elements ();

    Utf8String  rootFileName (BeFileName::GetFileNameWithoutExtension(this->GetRootDwgFileName()).c_str());
    size_t      dotAt = rootFileName.find (".");
    if (dotAt != Utf8String::npos)
        rootFileName.erase(dotAt);

    Utf8String  jobName = Utf8String(GetOptions().GetBridgeRegSubKey()) + Utf8String(":") + rootFileName + Utf8String(", ") + rootModelName;
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
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  DwgImporter::_ComputeImportJobName (DwgDbBlockTableRecordCR modelspaceBlock) const
    {
    Utf8String  jobName = this->GetOptions().GetBridgeJobName ();
    if (jobName.empty())
        {
        Utf8String  modelspaceName(modelspaceBlock.GetName().c_str());
        if (modelspaceName.empty())
            modelspaceName.assign ("Model");
        else if (modelspaceName.StartsWith("*"))
            modelspaceName = modelspaceName.substr(1, modelspaceName.size());

        this->ComputeDefaultImportJobName (modelspaceName);
        jobName = this->GetOptions().GetBridgeJobName();
        }
    else
        {
        if (!jobName.StartsWithI(this->GetOptions().GetBridgeRegSubKeyUtf8().c_str()))
            {
            jobName = this->GetOptions().GetBridgeRegSubKeyUtf8();
            jobName.append(":");
            jobName.append(this->GetOptions().GetBridgeJobName());
            }
        }
    return  jobName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgSourceAspects::ModelAspect DwgImporter::GetRootModelAspectFromSourceMasterModelSubject(SubjectCR subject) const
    {
    auto id = this->GetRootModelAspectIdFromSourceMasterModelSubject(subject);
    return DwgSourceAspects::ModelAspect::GetByAspectId(*m_dgndb, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgImporter::SetRootModelAspectIdInSourceMasterModelSubject(SubjectCR subject) const
    {
    auto sourceMasterModelSubjectEd = subject.MakeCopy<Subject>();
    auto sourceMasterModelSubjectProps = sourceMasterModelSubjectEd->GetSubjectJsonProperties(Subject::json_Model());
    sourceMasterModelSubjectProps["rootModelExternalAspectInstanceId"] = iModelExternalSourceAspect::UInt64ToString(m_rootDwgModelMap.GetModelAspect().GetECInstanceId().GetValue());
    sourceMasterModelSubjectEd->SetSubjectJsonProperties(Subject::json_Model(), sourceMasterModelSubjectProps);
    sourceMasterModelSubjectEd->Update();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceId DwgImporter::GetRootModelAspectIdFromSourceMasterModelSubject(SubjectCR subject) const
    {
    auto sourceMasterModelSubjectProps = subject.GetSubjectJsonProperties(Subject::json_Model());
    auto idvalue = iModelExternalSourceAspect::UInt64FromString(sourceMasterModelSubjectProps["rootModelExternalAspectInstanceId"].asCString());
    auto id = BeSQLite::EC::ECInstanceId(idvalue);
    BeAssert(id.IsValid() && "We have a sourceMasterModelSubject, but it has an invalid rootModelExternalAspectInstanceId JSON property value");
    return id;
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedImportJob   DwgImporter::FindSoleImportJobForSourceMasterModel (SubjectCR masterModelSubject) const
    {
    // each bridge can have only one job subject under a SorceMasterModel subject which is a child of the root subject
    auto stmt = m_dgndb->GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_Subject) " WHERE (Parent.Id=? AND json_extract(JsonProperties, '$.Subject.Job.Bridge') = ?)");
    stmt->BindId(1, masterModelSubject.GetElementId());
    stmt->BindText(2, this->GetOptions().GetBridgeRegSubKeyUtf8().c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes); // e.g., "iModelBridgeForMstn" or "ABDBridge", etc.
    if (BE_SQLITE_ROW != stmt->Step())
        return ResolvedImportJob();

    auto jobSubjectId = stmt->GetValueId<DgnElementId>(0);
    auto jobSubject = GetDgnDb().Elements().Get<Subject>(jobSubjectId);
    if (!jobSubject.IsValid())
        {
        BeAssert(false);
        return ResolvedImportJob();
        }
    
    return ResolvedImportJob(*jobSubject);
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
* @bsimethod                                                    Don.Fu          04/19
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr DwgImporter::FindSourceMasterModelSubject (DgnElementId rootRepLinkId, Utf8StringCR modelName, bool createIfNotFound) const
    {
    /*-----------------------------------------------------------------------------------
    A root subject may have multiple SourceMasterModel subjects, each of which cooresponds 
    to a source master file. Find a SourceMasterModel subject for this root master DWG file.
    -----------------------------------------------------------------------------------*/
    auto stmt = m_dgndb->GetPreparedECSqlStatement(
        "SELECT subject.ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_Subject) " subject "
        " JOIN " BIS_SCHEMA(BIS_CLASS_RepositoryLink) " rlink USING " BIS_SCHEMA(BIS_REL_ElementHasLinks) 
        " WHERE (rlink.ECInstanceId=? AND subject.Parent.Id=? AND json_extract(subject.JsonProperties, '$.Subject.Model.Type') = 'SourceMasterModel')");
    stmt->BindId(1, rootRepLinkId);
    stmt->BindId(2, m_dgndb->Elements().GetRootSubjectId());

    auto rootDwgModelId = m_dwgdb->GetModelspaceId().GetHandle ();

    // loop through all SourceMasterModel subjects and find the one matching this root DWG model
    SubjectCPtr sourceMasterModelSubject;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto masterModelSubjectId = stmt->GetValueId<DgnElementId>(0);
        sourceMasterModelSubject = m_dgndb->Elements().Get<Subject>(masterModelSubjectId);

        // make sure we get the SourceMasterModel subject for this DWG file
        auto rootModelAspect = this->GetRootModelAspectFromSourceMasterModelSubject(*sourceMasterModelSubject);
        if (!rootModelAspect.IsValid() || rootModelAspect.GetDwgModelHandle() != rootDwgModelId)
            continue;
        return sourceMasterModelSubject;
        }

    if (createIfNotFound)
        {
        // create a new SoureceMasterModel subject
        Utf8String sourceMasterModelName("DwgBridge master model ");
        sourceMasterModelName.append(this->_GetUniqueNameForFile(m_rootFileName));
        sourceMasterModelName.append(", ").append(modelName);

        auto code = createUniqueSubjectCode(*m_dgndb->Elements().GetRootSubject(), sourceMasterModelName.c_str());
        auto sourceMasterModelId = m_dgndb->Elements().QueryElementIdByCode(code);
        if (sourceMasterModelId.IsValid())
            {
            BeAssert(false && "SourceMasterModel Subject element not found via db.Elements().QueryElementIdByCode()!");
            return m_dgndb->Elements().Get<Subject>(sourceMasterModelId);
            }

        auto subject = Subject::Create(*m_dgndb->Elements().GetRootSubject(), code.GetValueUtf8());

        Utf8String  label(m_rootFileName.GetBaseName().c_str());
        subject->SetUserLabel(label.c_str());

        Json::Value modelProps(Json::nullValue);
        modelProps["Type"] = "SourceMasterModel";
        modelProps["rootModelExternalAspectInstanceId"] = "<tbd>";      // to be filled in by InitializeJob
        subject->SetSubjectJsonProperties(Subject::json_Model(), modelProps);

        sourceMasterModelSubject = dynamic_cast<Subject const*>(subject->Insert().get());
        auto subjectId = sourceMasterModelSubject->GetElementId();

        iModelBridge::InsertElementHasLinksRelationship(*m_dgndb, subjectId, rootRepLinkId);
        return sourceMasterModelSubject;
        }

    return nullptr;
    }

