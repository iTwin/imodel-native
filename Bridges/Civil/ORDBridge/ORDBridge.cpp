/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDBridge.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ORDBridgeInternal.h"
#include <windows.h>

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
Utf8String ORDBridge::ComputeJobSubjectName()
    {
    return Utf8String (_GetParams().GetInputFileName());
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

    iModelBridgeSyncInfoFile::ConversionResults docLink = RecordDocument(*GetSyncInfo().GetChangeDetectorFor(*this), _GetParams().GetInputFileName());
    auto repositoryLinkId = docLink.m_element->GetElementId();

    AlignmentBim::RoadRailAlignmentDomain::GetDomain().SetUpModelHierarchy(*jobSubject, ORDBRIDGE_AlignmentModelName);
    RoadRailBim::RoadRailPhysicalDomain::GetDomain().SetUpModelHierarchy(*jobSubject, ORDBRIDGE_PhysicalModelName);

    auto physicalModelPtr = RoadRailBim::RoadRailPhysicalDomain::QueryPhysicalModel(*jobSubject, ORDBRIDGE_PhysicalModelName);
    InsertPartitionOriginatesFromRepositoryRelationship(GetDgnDbR(), physicalModelPtr->GetModeledElementId(), repositoryLinkId);

    return jobSubject;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void ORDBridge::UpdateProjectExtents(SpatialModelR spatialModel)
    {
    DgnDbR db = spatialModel.GetDgnDb();

    AxisAlignedBox3d modelExtents = spatialModel.QueryModelRange();
    if (modelExtents.IsEmpty())
        return;

    modelExtents.Extend(0.5);

    if (IsCreatingNewDgnDb())
        {
        db.GeoLocation().SetProjectExtents(modelExtents);
        return;
        }

    // Make sure the project extents include the elements in the spatialModel, plus a margin
    AxisAlignedBox3d currentProjectExtents = db.GeoLocation().GetProjectExtents();
    if (!modelExtents.IsContained(currentProjectExtents))
        {
        currentProjectExtents.Extend(modelExtents);
        db.GeoLocation().SetProjectExtents(currentProjectExtents);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ORDBridge::_ConvertToBim(SubjectCR jobSubject)
    {
    auto changeDetectorPtr = GetSyncInfo().GetChangeDetectorFor(*this);

    iModelBridgeSyncInfoFile::ConversionResults docLink = RecordDocument(*changeDetectorPtr, _GetParams().GetInputFileName());
    auto fileScopeId = docLink.m_syncInfoRecord.GetROWID();

    ORDConverter converter;
    converter.ConvertORDData(_GetParams().GetInputFileName(), jobSubject, *changeDetectorPtr, fileScopeId);

    auto alignmentModelPtr = AlignmentBim::AlignmentModel::Query(jobSubject, ORDBRIDGE_AlignmentModelName);
    auto horizontalAlignmentModelId = AlignmentBim::HorizontalAlignmentModel::QueryBreakDownModelId(*alignmentModelPtr);
    auto physicalModelPtr = RoadRailBim::RoadRailPhysicalDomain::QueryPhysicalModel(jobSubject, ORDBRIDGE_PhysicalModelName);

    UpdateProjectExtents(*alignmentModelPtr);
    UpdateProjectExtents(*physicalModelPtr);

    RoadRailBim::RoadRailPhysicalDomain::SetUpDefaultViews(jobSubject, ORDBRIDGE_AlignmentModelName, ORDBRIDGE_PhysicalModelName);

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
extern "C" Dgn::iModelBridge* iModelBridge_getInstance()
    {
    return new ORDBridge();
    }

END_ORDBRIDGE_NAMESPACE