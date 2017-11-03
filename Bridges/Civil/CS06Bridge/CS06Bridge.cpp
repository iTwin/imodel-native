/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/CS06Bridge.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CS06BridgeInternal.h"
#include <windows.h>

BEGIN_CS06BRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge::CmdLineArgStatus CS06Bridge::_ParseCommandLineArg(int iArg, int argc, WCharCP argv[])
    {
    WString arg(argv[iArg]);
    if (arg.StartsWith(L"--DGN"))
        return iModelBridge::CmdLineArgStatus::Success;

    return iModelBridge::CmdLineArgStatus::NotRecognized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CS06Bridge::_Initialize(int argc, WCharCP argv[])
    {
    /*AppendCifSdkToDllSearchPath(_GetParams().GetLibraryDir());

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
    GeometryModelDgnECDataBinder::GetInstance().Initialize();*/

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CS06Bridge::_OpenSource()
    {
    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SubjectCPtr CS06Bridge::CreateAndInsertJobSubject(DgnDbR db, Utf8CP jobName)
    {
    //db.Schemas().CreateClassViewsInDb();

    auto subjectObj = Subject::Create(*db.Elements().GetRootSubject(), jobName);

    Json::Value jobProps(Json::nullValue);
    jobProps["Converter"] = "OpenRoads/Rail ConceptStation BIM Bridge";
    jobProps["InputFile"] = Utf8String(_GetParams().GetInputFileName());

    subjectObj->SetSubjectJsonProperties(Subject::json_Job(), jobProps);

    return subjectObj->InsertT<Subject>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SubjectCPtr CS06Bridge::QueryJobSubject(DgnDbR db, Utf8CP jobName)
    {
    DgnCode jobCode = Subject::CreateCode(*db.Elements().GetRootSubject(), jobName);
    auto jobId = db.Elements().QueryElementIdByCode(jobCode);
    return db.Elements().Get<Subject>(jobId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CS06Bridge::ComputeJobSubjectName()
    {
    return Utf8String (_GetParams().GetInputFileName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr CS06Bridge::_FindJob()
    {
    Utf8String jobName(ComputeJobSubjectName());
    return QueryJobSubject(GetDgnDbR(), jobName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr CS06Bridge::_InitializeJob()
    {
    Utf8String jobName(ComputeJobSubjectName());

    SubjectCPtr jobSubject = CreateAndInsertJobSubject(GetDgnDbR(), jobName.c_str());
    if (!jobSubject.IsValid())
        return nullptr;

    //AlignmentBim::RoadRailAlignmentDomain::GetDomain().SetUpModelHierarchy(*jobSubject, ORDBRIDGE_AlignmentModelName);
    //RoadRailBim::RoadRailPhysicalDomain::GetDomain().SetUpModelHierarchy(*jobSubject, ORDBRIDGE_PhysicalModelName);

    return jobSubject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CS06Bridge::UpdateProjectExtents(SpatialModelR spatialModel)
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

void CS06Bridge::_OnDocumentDeleted(Utf8StringCR docId, iModelBridgeSyncInfoFile::ROWID docSyncInfoid)
	{
	// TODO: Implement this.
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CS06Bridge::_ConvertToBim(SubjectCR jobSubject)
    {
    //auto changeDetectorPtr = GetSyncInfo().GetChangeDetectorFor(*this);

    /*ORDConverter converter;
    converter.ConvertORDData(_GetParams().GetInputFileName(), jobSubject, *changeDetectorPtr);

    auto alignmentModelPtr = AlignmentBim::AlignmentModel::Query(jobSubject, ORDBRIDGE_AlignmentModelName);
    auto horizontalAlignmentModelId = AlignmentBim::HorizontalAlignmentModel::QueryBreakDownModelId(*alignmentModelPtr);
    auto physicalModelPtr = RoadRailBim::RoadRailPhysicalDomain::QueryPhysicalModel(jobSubject, ORDBRIDGE_PhysicalModelName);

    UpdateProjectExtents(*alignmentModelPtr);
    UpdateProjectExtents(*physicalModelPtr);

    RoadRailBim::RoadRailPhysicalDomain::SetUpDefaultViews(jobSubject, ORDBRIDGE_AlignmentModelName, ORDBRIDGE_PhysicalModelName);*/

    // Infer deletions
    //changeDetectorPtr->_DeleteElementsNotSeen();

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" Dgn::iModelBridge* iModelBridge_getInstance(wchar_t const* bridgeName)
    {
	BeAssert(0 == BeStringUtilities::Wcsicmp(bridgeName, CS06Bridge::GetRegistrySubKey()));
    return new CS06Bridge();
    }

END_CS06BRIDGE_NAMESPACE
