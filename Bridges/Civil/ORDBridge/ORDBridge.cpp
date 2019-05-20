/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDBridge.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ORDBridgeInternal.h"
#include <windows.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger(L"ORDBridge"))
#define DefaultPhysicalPartitionName    "Physical"
#define DefaultRoadRailNetworkName      "Road/Rail Network"
#define DomainModelsPrivate             true

BEGIN_ORDBRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge::CmdLineArgStatus ORDBridge::_ParseCommandLineArg(int iArg, int argc, WCharCP argv[])
    {
    // This method gets called by the offline/test bridge runner.  See _ParseCommandLine() for the version
    // that gets called during iModelBridgeFwk.exe run.
    if (argv[iArg] == wcsstr(argv[iArg], L"--root-model="))
        {
        BentleyApi::Dgn::DgnDbSync::DgnV8::RootModelConverter::RootModelChoice rmc;

        Utf8String value = GetArgValue(argv[iArg]);
        if (value.EqualsI(".default"))
            rmc.SetUseDefaultModel();
        else if (value.EqualsI(".active"))
            rmc.SetUseActiveViewGroup();
        else
            rmc = BentleyApi::Dgn::DgnDbSync::DgnV8::RootModelConverter::RootModelChoice(value);

        m_params.SetRootModelChoice(rmc);
        return iModelBridge::CmdLineArgStatus::Success;
        }
    else
        {
        WString arg(argv[iArg]);
        if (arg.StartsWith(L"--DGN"))
            return iModelBridge::CmdLineArgStatus::Success;
        }

    return iModelBridge::CmdLineArgStatus::NotRecognized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Jonathan.DeCarlo                   10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDBridge::_ParseCommandLine(int argc, WCharCP argv[])
    {
    // This method gets called during a run of iModelBridgeFwk.exe. See _ParseCommandLineArg()
    // for the version that gets called during the offline/test runner.
    for (int i = 1; i < argc; i++)
        {
        if (argv[i] == wcsstr(argv[i], L"--root-model="))
            {
            BentleyApi::Dgn::DgnDbSync::DgnV8::RootModelConverter::RootModelChoice rmc;

            Utf8String value = GetArgValue(argv[i]);
            if (value.EqualsI(".default"))
                rmc.SetUseDefaultModel();
            else if (value.EqualsI(".active"))
                rmc.SetUseActiveViewGroup();
            else
                rmc = BentleyApi::Dgn::DgnDbSync::DgnV8::RootModelConverter::RootModelChoice(value);

            m_params.SetRootModelChoice(rmc);
            }
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ORDBridge::_Initialize(int argc, WCharCP argv[])
    {
    if (_GetParams().GetBridgeRegSubKey().empty())
        _GetParams().SetBridgeRegSubKey(GetRegistrySubKey());

    m_isUnitTesting = CheckIfUnitTesting(argc, argv);

    // The call to iModelBridge::_Initialize is the time to register domains.
    DgnDomains::RegisterDomain(LinearReferencingDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    DgnDomains::RegisterDomain(AlignmentBim::RoadRailAlignmentDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    DgnDomains::RegisterDomain(RoadRailBim::RoadRailPhysicalDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    DgnDomains::RegisterDomain(DgnV8ORDBim::DgnV8OpenRoadsDesignerDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);

    if (!_GetParams().GetInputFileName().DoesPathExist() || _GetParams().GetInputFileName().IsDirectory())
        {
        fwprintf(stderr, L"%ls: not found or not an OpenRoads Designer DGN file.\n", _GetParams().GetInputFileName().c_str());
        return BentleyStatus::ERROR;
        }

    DgnDbSync::DgnV8::Converter::Initialize(_GetParams().GetLibraryDir(), _GetParams().GetAssetsDir(), BeFileName(L"DgnV8"), nullptr, false, argc, argv, nullptr);
    if (m_isUnitTesting)
        {
        //When unit testing, we need to make an additional call to SetProcessingDisabled in order for CIF backpointers to be created in ProcessAffected()
        DependencyManager::SetProcessingDisabled(false);
        }
    AppendCifSdkToDllSearchPath(_GetParams().GetLibraryDir());

    // Initialize Cif SDK
    DgnPlatformCivilLib::InitializeWithDefaultHost();
    GeometryModelDgnECDataBinder::GetInstance().Initialize();

    m_params.SetConsiderNormal2dModelsSpatial(true);
    m_params.SetProcessAffected(true);
    m_params.SetConvertViewsOfAllDrawings(true);

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
    DependencyManager::SetProcessingDisabled(false);
    auto initStat = m_converter->InitRootModel();
    DependencyManager::SetProcessingDisabled(true);

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

        SubjectCPtr physicalSubjectCPtr = &subjectCR;
        AlignmentBim::RoadRailAlignmentDomain::GetDomain().SetUpDefinitionPartitions(subjectCR);
        for (auto childId : subjectCR.QueryChildren())
            {
            auto childElmCPtr = subjectCR.GetDgnDb().Elements().GetElement(childId);
            if (auto childSubjectCP = dynamic_cast<SubjectCP>(childElmCPtr.get()))
                {
                if (0 == childSubjectCP->GetCode().GetValueUtf8().CompareTo(DefaultPhysicalPartitionName))
                    {
                    physicalSubjectCPtr = childSubjectCP;
                    break;
                    }
                }
            }

        RoadRailBim::RoadRailPhysicalDomain::GetDomain().SetUpDefinitionPartitions(subjectCR);

        auto physicalPartitionCPtr = RoadRailBim::PhysicalModelUtilities::CreateAndInsertPhysicalPartitionAndModel(
            *physicalSubjectCPtr, DefaultPhysicalPartitionName);
        BeAssert(physicalPartitionCPtr.IsValid());

        auto roadRailNetworkCPtr = RoadRailBim::RoadRailNetwork::Insert(*physicalPartitionCPtr->GetSubModel()->ToPhysicalModelP(), DefaultRoadRailNetworkName);

        // IMODELBRIDGE REQUIREMENT: Relate this model to the source document
        m_converter->SetRoadRailNetwork(*roadRailNetworkCPtr);

        InsertElementHasLinksRelationship(GetDgnDbR(), physicalPartitionCPtr->GetElementId(), m_converter->GetRepositoryLinkId(*m_converter->GetRootV8File()));

        auto designAlignmentsCPtr = AlignmentBim::DesignAlignments::Query(*roadRailNetworkCPtr->GetNetworkModel());
        auto designAlignmentModelPtr = designAlignmentsCPtr->GetAlignmentModel();
        InsertElementHasLinksRelationship(GetDgnDbR(), designAlignmentModelPtr->GetModeledElementId(), m_converter->GetRepositoryLinkId(*m_converter->GetRootV8File()));

        if (DomainModelsPrivate)
            {
            physicalPartitionCPtr->GetSubModel()->SetIsPrivate(true);
            physicalPartitionCPtr->GetSubModel()->Update();

            roadRailNetworkCPtr->GetNetworkModel()->SetIsPrivate(true);
            roadRailNetworkCPtr->GetNetworkModel()->Update();

            designAlignmentModelPtr->SetIsPrivate(true);
            designAlignmentModelPtr->Update();

            auto horizontalAlignmentsCPtr = AlignmentBim::HorizontalAlignments::Query(*designAlignmentModelPtr);
            horizontalAlignmentsCPtr->GetHorizontalModel()->SetIsPrivate(true);
            horizontalAlignmentsCPtr->GetHorizontalModel()->Update();
            }

        m_converter->SetUpModelFormatters();

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
    // This if statement's work is also done in _InitializeJob(), but when the bridge goes off for a second time (an update),
    // _InitializeJob() is not called. It's possible that this work can (should?) be just done here and NOT in _InitializeJob(),
    // but I'm not sure yet.
    if (!m_converter->GetRoadRailNetwork())
        {
        // IMODELBRIDGE REQUIREMENT: Relate this model to the source document
        auto physicalNetworkModelPtr = RoadRailBim::PhysicalModelUtilities::QueryPhysicalNetworkModel(jobSubject,
            DefaultPhysicalPartitionName, DefaultRoadRailNetworkName);
        m_converter->SetRoadRailNetwork(*dynamic_cast<RoadRailBim::RoadRailNetworkCP>(physicalNetworkModelPtr->GetModeledElement().get()));
        }

    auto changeDetectorPtr = GetSyncInfo().GetChangeDetectorFor(*this);

    // IMODELBRIDGE REQUIREMENT: Keep information about the source document up to date.
    iModelBridgeSyncInfoFile::ConversionResults docLink = RecordDocument(*changeDetectorPtr, _GetParams().GetInputFileName(), nullptr, "DocumentWithBeGuid", jobSubject.GetElementId().GetValue());
    auto fileScopeId = jobSubject.GetElementId().GetValue();

    // IMODELBRIDGE REQUIREMENT: Note job transform and react when it changes
    ORDConverter::Params params(_GetParams(), jobSubject, *changeDetectorPtr, fileScopeId, m_converter->GetRootModelUnitSystem(), GetSyncInfo());
    params.domainModelsPrivate = DomainModelsPrivate;

    Transform _old, _new;
    params.spatialDataTransformHasChanged = DetectSpatialDataTransformChange(_new, _old, *changeDetectorPtr, fileScopeId, "JobTrans", "JobTrans");
    params.isCreatingNewDgnDb = IsCreatingNewDgnDb();
    params.isUpdating = _GetParams().IsUpdating();

    m_converter->SetORDParams(&params);
    ConvertORDElementXDomain convertORDXDomain(*m_converter);
    Dgn::DgnDbSync::DgnV8::XDomain::Register(convertORDXDomain);

    m_converter->SetIsProcessing(true);
    m_converter->MakeDefinitionChanges();
    m_converter->ConvertData();
    m_converter->SetIsProcessing(false);

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

bool ORDBridge::CheckIfUnitTesting(int argc, WCharCP argv[])
{
    //check argv[] for '--unit-testing' parameter
    for (int i = 0; i < argc; i++)
    {
        if (argv[i] == wcsstr(argv[i], L"--unit-testing"))
        {
            return true;
        }
    }
    return false;
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

    GetDgnDbR().Schemas().CreateClassViewsInDb(); // For debugging purposes

    return ((BSISUCCESS != status) || m_converter->WasAborted()) ? BSIERROR : BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDBridge::_OnOpenBim(DgnDbR db)
    {
    //if (m_converter != nullptr)
        //delete m_converter;
    if (m_converter == nullptr)
        {
        m_converter = new ORDConverter(m_params);
        }
    m_converter->SetDgnDb(db);
    if (BentleyStatus::SUCCESS != m_converter->AttachSyncInfo())
        return BentleyStatus::ERROR;

    return T_Super::_OnOpenBim(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDBridge::_OnCloseBim(BentleyStatus status, ClosePurpose purpose)
    {
    // this also has the side effect of closing the source files
    if (m_converter != nullptr)
        {
        //calling delete on the ORDConverter will result in source files closing, which is important for the unit tests to run properly
            __try
            {
                // TODO: Some CIF object smart pointers are blowing up after deleting 
                // the converter instance.  Look into this!
                delete m_converter;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
            }
            m_converter = nullptr;
        }

        T_Super::_OnCloseBim(status, purpose);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDBridge::_DetectDeletedDocuments()
    {
    m_converter->_DetectDeletedDocuments();

    return m_converter->WasAborted() ? BSIERROR : BSISUCCESS;
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