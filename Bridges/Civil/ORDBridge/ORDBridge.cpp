/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDBridge.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ORDBridgeInternal.h"
#include <windows.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>

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
    AppendCifSdkToDllSearchPath(_GetParams().GetLibraryDir());

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

    // Initialize Cif SDK
    DgnPlatformCivilLib::InitializeWithDefaultHost();
    GeometryModelDgnECDataBinder::GetInstance().Initialize();

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ORDBridge::_OpenSource()
    {
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
    Utf8String jobName(ComputeJobSubjectName());
    return QueryJobSubject(GetDgnDbR(), jobName.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
SubjectCPtr ORDBridge::_InitializeJob()
    {
    Utf8String jobName(ComputeJobSubjectName());

    SubjectCPtr jobSubject = CreateAndInsertJobSubject(GetDgnDbR(), jobName.c_str());
    if (!jobSubject.IsValid())
        return nullptr;

    // IMODELBRIDGE REQUIREMENT: Store information about the source document
    iModelBridgeSyncInfoFile::ConversionResults docLink = RecordDocument(*GetSyncInfo().GetChangeDetectorFor(*this), _GetParams().GetInputFileName());
    auto repositoryLinkId = docLink.m_element->GetElementId();

    AlignmentBim::RoadRailAlignmentDomain::GetDomain().SetUpModelHierarchy(*jobSubject, ORDBRIDGE_AlignmentModelName);
    RoadRailBim::RoadRailPhysicalDomain::GetDomain().SetUpModelHierarchy(*jobSubject, ORDBRIDGE_PhysicalModelName);

    // IMODELBRIDGE REQUIREMENT: Relate this model to the source document
    auto physicalModelPtr = RoadRailBim::RoadRailPhysicalDomain::QueryPhysicalModel(*jobSubject, ORDBRIDGE_PhysicalModelName);
    InsertElementHasLinksRelationship(GetDgnDbR(), physicalModelPtr->GetModeledElementId(), repositoryLinkId);

    return jobSubject;
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

    ORDConverter::Params params(_GetParams(), jobSubject, *changeDetectorPtr, fileScopeId);

    // IMODELBRIDGE REQUIREMENT: Note job transform and react when it changes
    Transform _old, _new;
    params.spatialDataTransformHasChanged = DetectSpatialDataTransformChange(_new, _old, *changeDetectorPtr, fileScopeId, "JobTrans", "JobTrans");
    params.isCreatingNewDgnDb = IsCreatingNewDgnDb();

    ORDConverter converter;
    converter.ConvertORDData(params);

    // Infer deletions
    changeDetectorPtr->DeleteElementsNotSeenInScope(fileScopeId);

    return BentleyStatus::SUCCESS;
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
extern "C" iModelBridge* iModelBridge_getInstance(wchar_t const* bridgeName)
    {
    BeAssert(0 == BeStringUtilities::Wcsicmp(bridgeName, ORDBridge::GetRegistrySubKey()));
    return new ORDBridge();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" BentleyStatus iModelBridge_releaseInstance(iModelBridge* bridge)
    {
    #if defined (BENTLEYCONFIG_PARASOLID)
    if (PSolidKernelManager::IsSessionStarted())
        PSolidKernelManager::StopSession();
    #endif
        
    delete bridge;
    return SUCCESS;
    }

END_ORDBRIDGE_NAMESPACE