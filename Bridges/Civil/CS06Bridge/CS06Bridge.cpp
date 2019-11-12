/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "CS06BridgeInternal.h"
#include <windows.h>

BEGIN_CS06BRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge::CmdLineArgStatus CS06Bridge::_ParseCommandLineArg(int iArg, int argc, WCharCP argv[])
    {
    // Process any command-line arguments specific to the CS06Bridge here.

    return iModelBridge::CmdLineArgStatus::NotRecognized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WString CS06Bridge::_SupplySqlangRelPath()
    {
    // TODO: Find out how I should create my own db3 and what it needs to contain.  Where do the framework strings come from?
    return L"sqlang/iModelBridgeFwk_en-US.sqlang.db3";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CS06Bridge::_Initialize(int argc, WCharCP argv[])
    {
    Teleporter::AppendDgnDb06SdkToDllSearchPath(_GetParams().GetLibraryDir());

    // The call to iModelBridge::_Initialize is the time to register domains.
    Teleporter::RegisterDomains();

    if (!_GetParams().GetInputFileName().DoesPathExist() || _GetParams().GetInputFileName().IsDirectory())
        {
        // TODO: Localize this string.
        // TODO: Write this out to the log file.
        fwprintf(stderr, L"%ls: not found or not an OpenRoads ConceptStation file.\n", _GetParams().GetInputFileName().c_str());
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CS06Bridge::_OpenSource()
    {
    BentleyStatus status = 
        static_cast<BentleyStatus>(Teleporter::OpenSourceFile(MarshalHelper::MarshalBimBeFileNameTo06BeFileName(_GetParams().GetInputFileName())));
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CS06Bridge::_CloseSource(BentleyStatus)
    {
    Teleporter::CloseSourceFile();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SubjectCPtr CS06Bridge::CreateAndInsertJobSubject(DgnDbR db, Utf8CP jobName)
    {
    auto subjectObj = Subject::Create(*db.Elements().GetRootSubject(), jobName);
    JobSubjectUtils::InitializeProperties(*subjectObj, _GetParams().GetBridgeRegSubKeyUtf8());
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

    iModelBridgeSyncInfoFile::ConversionResults docLink = RecordDocument(*GetSyncInfo().GetChangeDetectorFor(*this), _GetParams().GetInputFileName());
    auto repositoryLinkId = docLink.m_element->GetElementId();

    AlignmentBim::RoadRailAlignmentDomain::GetDomain().SetUpModelHierarchy(*jobSubject, CS06BRIDGE_AlignmentModelName);
    RoadRailBim::RoadRailPhysicalDomain::GetDomain().SetUpModelHierarchy(*jobSubject, CS06BRIDGE_PhysicalModelName);

    auto physicalModelPtr = RoadRailBim::RoadRailPhysicalDomain::QueryPhysicalModel(*jobSubject, CS06BRIDGE_PhysicalModelName);
    InsertElementHasLinksRelationship(GetDgnDbR(), physicalModelPtr->GetModeledElementId(), repositoryLinkId);

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CS06Bridge::_OnDocumentDeleted(Utf8StringCR docId, iModelBridgeSyncInfoFile::ROWID docSyncInfoid)
	{
	// TODO: Is this the way we should be handle document deletion?

    // Look up the document-specific job subject element.
    auto jobSubject = QueryJobSubject(GetDgnDbR(), ComputeJobSubjectName().c_str());
    if (!jobSubject.IsValid())
        return;

    auto alignmentModelPtr = AlignmentBim::RoadRailAlignmentDomain::QueryAlignmentModel(*jobSubject, CS06BRIDGE_AlignmentModelName);
    if (alignmentModelPtr.IsValid())
        alignmentModelPtr->Delete();

    auto physicalModelPtr = RoadRailBim::RoadRailPhysicalDomain::QueryPhysicalModel(*jobSubject, CS06BRIDGE_PhysicalModelName);
    if (physicalModelPtr.IsValid())
        physicalModelPtr->Delete();

    jobSubject->Delete();
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CS06Bridge::_ConvertToBim(SubjectCR jobSubject)
    {
    // TODO: Implement this.

    auto changeDetectorPtr = GetSyncInfo().GetChangeDetectorFor(*this);

    iModelBridgeSyncInfoFile::ConversionResults docLink = RecordDocument(*changeDetectorPtr, _GetParams().GetInputFileName());
    auto fileScopeId = docLink.m_syncInfoRecord.GetROWID();

    ChangeDetectorFacadePtr changeDetectorFacade = new ChangeDetectorFacade(changeDetectorPtr.get(), fileScopeId);
    Teleporter::ConvertDgnDbToBim(jobSubject.GetDgnDb(), true, jobSubject, changeDetectorFacade.get());

    // Infer deletions
    changeDetectorPtr->DeleteElementsNotSeenInScope(fileScopeId);

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
