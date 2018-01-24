/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDBridge.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ORDBridgeInternal.h"
#include <windows.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger(L"ORDBridge"))

BEGIN_ORDBRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge::CmdLineArgStatus ORDBridge::_ParseCommandLineArg(int iArg, int argc, WCharCP argv[])
    {
    WString arg(argv[iArg]);
    if (arg.StartsWith(L"--DGN"))
        return iModelBridge::CmdLineArgStatus::Success;

    return iModelBridge::CmdLineArgStatus::NotRecognized;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ORDBridge::_Initialize(int argc, WCharCP argv[])
    {
    if (_GetParams().GetBridgeRegSubKey().empty())
        _GetParams().SetBridgeRegSubKey(GetRegistrySubKey());

    // The call to iModelBridge::_Initialize is the time to register domains.
    DgnDomains::RegisterDomain(LinearReferencingDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    DgnDomains::RegisterDomain(AlignmentBim::RoadRailAlignmentDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    DgnDomains::RegisterDomain(RoadRailBim::RoadRailPhysicalDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);

    if (!_GetParams().GetInputFileName().DoesPathExist() || _GetParams().GetInputFileName().IsDirectory())
        {
        fwprintf(stderr, L"%ls: not found or not an OpenRoads Designer DGN file.\n", _GetParams().GetInputFileName().c_str());
        return BentleyStatus::ERROR;
        }

    DgnDbSync::DgnV8::Converter::Initialize(_GetParams().GetLibraryDir(), _GetParams().GetAssetsDir(), BeFileName(L"DgnV8"), nullptr, false, argc, argv);
    AppendCifSdkToDllSearchPath(_GetParams().GetLibraryDir());

    // Initialize Cif SDK
    DgnPlatformCivilLib::InitializeWithDefaultHost();
    GeometryModelDgnECDataBinder::GetInstance().Initialize();

#ifndef TARGET_2DMODEL
    m_params.SetConsiderNormal2dModelsSpatial(true);
#endif

    m_params.SetWantThumbnails(true);

    BentleyApi::BeFileName configFileName = m_params.GetAssetsDir();
    configFileName.AppendToPath(L"ImportConfig.xml");
    m_params.SetConfigFile(configFileName);

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ORDBridge::_OpenSource()
    {
    auto initStat = m_converter->InitRootModel();
    if (DgnV8Api::DGNFILE_STATUS_Success != initStat)
        {
        switch (initStat)
            {
            case DgnV8Api::DGNOPEN_STATUS_FileNotFound:
                LOG.fatalv(L"%ls - file not found", _GetParams().GetInputFileName().GetName());
                fwprintf(stderr, L"%ls - file not found\n", _GetParams().GetInputFileName().GetName());
                break;

            default:
                m_converter->ReportDgnV8FileOpenError(initStat, _GetParams().GetInputFileName().c_str());
                LOG.fatalv(L"%ls - cannot find or load root model. See %ls-issues for more information.", _GetParams().GetInputFileName().GetName(), _GetParams().GetBriefcaseName().GetName());
            }

        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::SubjectCPtr ORDBridge::CreateAndInsertJobSubject(DgnDbR db, Utf8CP jobName)
    {
    db.Schemas().CreateClassViewsInDb();

    auto subjectObj = Subject::Create(*db.Elements().GetRootSubject(), jobName);

    Json::Value jobProps(Json::nullValue);
    jobProps["Converter"] = "OpenRoads/Rail Designer BIM Bridge";
    jobProps["InputFile"] = Utf8String(_GetParams().GetInputFileName());

    subjectObj->SetSubjectJsonProperties(Subject::json_Job(), jobProps);

    return subjectObj->InsertT<Subject>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::SubjectCPtr ORDBridge::QueryJobSubject(DgnDbR db, Utf8CP jobName)
    {
    DgnCode jobCode = Subject::CreateCode(*db.Elements().GetRootSubject(), jobName);
    auto jobId = db.Elements().QueryElementIdByCode(jobCode);
    return db.Elements().Get<Subject>(jobId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Utf8String ORDBridge::ComputeJobSubjectName(Utf8StringCR docId)
    {
    Utf8String name(GetRegistrySubKey());
    name.append(":");
    name.append(docId.c_str());
    return name;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Utf8String ORDBridge::ComputeJobSubjectName()
    {
    Utf8String docId;
    BeSQLite::BeGuid docGuid = QueryDocumentGuid(_GetParams().GetInputFileName());
    if (docGuid.IsValid())
        docId = docGuid.ToString();                                 // Use the document GUID, if available, to ensure a stable and unique Job subject name.
    else
        docId = Utf8String(_GetParams().GetInputFileName());        // fall back on using local file name -- not as stable!

    return ComputeJobSubjectName(docId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
SubjectCPtr ORDBridge::_FindJob()
    {
    auto status = m_converter->FindJob();
    return (DgnDbSync::DgnV8::RootModelConverter::ImportJobLoadStatus::Success == status) ? &m_converter->GetImportJob().GetSubject() : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
SubjectCPtr ORDBridge::_InitializeJob()
    {
    BeAssert(m_converter->IsFileAssignedToBridge(*m_converter->GetRootV8File()) && "The bridge assigned to the root file/model must be the bridge that creates the root subject");

    auto status = m_converter->InitializeJob();

    //  Make sure that we really should be converting this model as our root
    if (DgnDbSync::DgnV8::RootModelConverter::ImportJobCreateStatus::Success != status)
        {
        if (DgnDbSync::DgnV8::RootModelConverter::ImportJobCreateStatus::FailedExistingNonRootModel == status)
            {
            // This model was converted by some other job and not as its root.
            // This is probably a user error. If we were to use this as a root, we could end up creating duplicates of it and its references, possibly
            //  using different transforms. That would probably only cause confusion.
            LOG.fatalv(L"%ls - error - the selected root model [%ls] was previously converted, not as a root but as a reference attachment.", _GetParams().GetBriefcaseName().GetName(), m_converter->GetRootModelP()->GetModelName());
            return nullptr;
            }

        BeAssert(DgnDbSync::DgnV8::RootModelConverter::ImportJobCreateStatus::FailedExistingRoot != status); // If the root was previously converted, then we should be doing an update!
        }

    if (DgnDbSync::DgnV8::RootModelConverter::ImportJobCreateStatus::Success == status)
        {
        auto& subjectCR = m_converter->GetImportJob().GetSubject();

        AlignmentBim::RoadRailAlignmentDomain::GetDomain().SetUpModelHierarchy(subjectCR, ORDBRIDGE_AlignmentModelName);
        RoadRailBim::RoadRailPhysicalDomain::GetDomain().SetUpModelHierarchy(subjectCR, ORDBRIDGE_PhysicalModelName);

        // IMODELBRIDGE REQUIREMENT: Relate this model to the source document
        auto physicalModelPtr = RoadRailBim::RoadRailPhysicalDomain::QueryPhysicalModel(subjectCR, ORDBRIDGE_PhysicalModelName);
        InsertElementHasLinksRelationship(GetDgnDbR(), physicalModelPtr->GetModeledElementId(), m_converter->GetRepositoryLinkFromAppData(*m_converter->GetRootV8File()));
        return &subjectCR;
        }
    else
        return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ORDBridge::_ConvertToBim(SubjectCR jobSubject)
    {
    auto changeDetectorPtr = GetSyncInfo().GetChangeDetectorFor(*this);

    // IMODELBRIDGE REQUIREMENT: Keep information about the source document up to date.
    iModelBridgeSyncInfoFile::ConversionResults docLink = RecordDocument(*changeDetectorPtr, _GetParams().GetInputFileName());
    auto fileScopeId = docLink.m_syncInfoRecord.GetROWID();

    // IMODELBRIDGE REQUIREMENT: Note job transform and react when it changes
    ORDConverter::Params params(_GetParams(), jobSubject, *changeDetectorPtr, fileScopeId);
    Transform _old, _new;
    params.spatialDataTransformHasChanged = DetectSpatialDataTransformChange(_new, _old, *changeDetectorPtr, fileScopeId, "JobTrans", "JobTrans");
    params.isCreatingNewDgnDb = IsCreatingNewDgnDb();

    ConvertORDElementXDomain convertORDXDomain(*m_converter, params);
    Dgn::DgnDbSync::DgnV8::XDomain::Register(convertORDXDomain);

    m_converter->Process();

    convertORDXDomain.CreateRoadRailElements();

    Dgn::DgnDbSync::DgnV8::XDomain::UnRegister(convertORDXDomain);
    return m_converter->WasAborted() ? BSIERROR : BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDBridge::AppendCifSdkToDllSearchPath(BeFileNameCR libraryDir)
    {
    BeFileName dllDirectory(libraryDir);
    dllDirectory.AppendToPath(L"Cif");

    WString newPath(L"PATH=");
    newPath.append(dllDirectory);
    newPath.append(L";");
    newPath.append(::_wgetenv(L"PATH"));
    _wputenv(newPath.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    diego.diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDBridge::_OnDocumentDeleted(Utf8StringCR documentId, Dgn::iModelBridgeSyncInfoFile::ROWID documentSyncId)
    {
    // TODO
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDBridge::_MakeSchemaChanges()
    {
    auto status = m_converter->MakeSchemaChanges();
    return ((BSISUCCESS != status) || m_converter->WasAborted()) ? BSIERROR : BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDBridge::CreateSyncInfoIfNecessary()
    {
    //  If I am creating a new local file or if I just acquired a briefcase for an existing repository, then I will have to bootstrap syncinfo.
    if (!DgnDbSync::DgnV8::SyncInfo::GetDbFileName(_GetParams().GetBriefcaseName()).DoesPathExist())
        {
        if (BSISUCCESS != DgnDbSync::DgnV8::SyncInfo::CreateEmptyFile(DgnDbSync::DgnV8::SyncInfo::GetDbFileName(_GetParams().GetBriefcaseName()))) // Bootstrap the V8 converter by pairing an empty syncinfo file with the briefcase
            return BSIERROR;
        }

    BeAssert(DgnDbSync::DgnV8::SyncInfo::GetDbFileName(_GetParams().GetBriefcaseName()).DoesPathExist());

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDBridge::_OnOpenBim(DgnDbR db)
    {
    m_converter.reset(new ORDConverter(m_params));
    m_converter->SetDgnDb(db);
    CreateSyncInfoIfNecessary();
    if (BentleyStatus::SUCCESS != m_converter->AttachSyncInfo())
        return BentleyStatus::ERROR;

    return T_Super::_OnOpenBim(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDBridge::_OnCloseBim(BentleyStatus)
    {
    m_converter.reset(nullptr); // this also has the side effect of closing the source files
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDBridge::_DetectDeletedDocuments()
    {
    m_converter->_DetectDeletedDocuments();
    return m_converter->WasAborted() ? BSIERROR : BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carl.Hinkle                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDBridge::_DeleteSyncInfo()
    {
    T_Super::_DeleteSyncInfo();

    BeFileName briefcaseName = _GetParams().GetBriefcaseName();
    briefcaseName = briefcaseName.AppendExtension(L"syncinfo");
    if (!briefcaseName.DoesPathExist())
        return;
    briefcaseName.BeDeleteFile();    
    }

END_ORDBRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" BentleyStatus iModelBridge_releaseInstance(BentleyApi::Dgn::iModelBridge* bridge)
    {
    #if defined (BENTLEYCONFIG_PARASOLID)
    if (PSolidKernelManager::IsSessionStarted())
        PSolidKernelManager::StopSession();
    #endif
        
    delete bridge;
    return SUCCESS;
    }

USING_NAMESPACE_BENTLEY_ORDBRIDGE

bool DummyClass::DummyMethod() { return true; }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
extern "C" BentleyApi::Dgn::iModelBridge* iModelBridge_getInstance(wchar_t const* bridgeName)
    {
    BeAssert(0 == BentleyApi::BeStringUtilities::Wcsicmp(bridgeName, ORDBridge::GetRegistrySubKey()));
    return new ORDBridge();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" wchar_t const* iModelBridge_getRegistrySubKey()
    {
    return ORDBridge::GetRegistrySubKey();
    }