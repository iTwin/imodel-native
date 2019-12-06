/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ORDBridgeInternal.h"
#include "ConvertOBMElementXDomain.h"
#include <windows.h>
#include <BRepCore/PSolidUtil.h>
#include "ScalableMeshWrapper.h"

static constexpr Utf8CP DefaultPhysicalPartitionName                = "Physical";
static constexpr Utf8CP DefaultDesignAlignmentsName                 = "Road/Rail Design Alignments";
static constexpr Utf8CP DefaultRoadNetworkName                      = "Road Network";
static constexpr Utf8CP DefaultRailNetworkName                      = "Rail Network";
static constexpr Utf8CP SubjectCivilGraphicsBreakdownStructureCode  = "Civil Designer Products";

#define DomainModelsPrivate     true

USING_NAMESPACE_BENTLEY_OBMNET_GEOMETRYMODEL_SDK

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

    ORDBRIDGE_LOGI(L"%ls - not recognized.", GetArgValue(argv[iArg]).c_str());

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
    ScalableMeshWrapper::RegisterDomain();
    DgnDomains::RegisterDomain(Structural::StructuralPhysicalDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
	DgnDomains::RegisterDomain(DgnV8ORDBim::DgnV8OpenRoadsDesignerDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);

    if (!_GetParams().GetInputFileName().DoesPathExist() || _GetParams().GetInputFileName().IsDirectory())
        {
        fwprintf(stderr, L"%ls: not found or not an OpenRoads Designer DGN file.\n", _GetParams().GetInputFileName().c_str());
        return BentleyStatus::ERROR;
        }

    DgnDbSync::DgnV8::Converter::Initialize(_GetParams().GetLibraryDir(), _GetParams().GetAssetsDir(), BeFileName(L"DgnV8"), nullptr, false, argc, argv, _GetParams().GetDmsSupportLibrary());
    if (m_isUnitTesting)
        {
        //When unit testing, we need to make an additional call to SetProcessingDisabled in order for CIF backpointers to be created in ProcessAffected()
        DependencyManager::SetProcessingDisabled(false);
        }
    AppendCifSdkToDllSearchPath(_GetParams().GetLibraryDir());
	AppendObmSdkToDllSearchPath(_GetParams().GetLibraryDir());

    // Initialize Cif SDK
    DgnPlatformCivilLib::InitializeWithDefaultHost();
    GeometryModelDgnECDataBinder::GetInstance().Initialize();

	// Initialize OBM SDK
	ObmGeometryConsensusSchemaProvider::GetInstance();

    m_params.SetConsiderNormal2dModelsSpatial(true);
    m_params.SetProcessAffected(true);
    m_params.SetConvertViewsOfAllDrawings(true);

    if (!_wgetenv(L"ORDBRIDGE_TESTING"))
        {
        m_params.SetDoTerrainModelConversion(true);
        m_params.SetDoRealityDataUpload(true);
        }

    m_params.SetDoTerrainModelConversion(false); // DO NOT COMMIT!
    m_params.SetWantThumbnails(true);

    BentleyApi::BeFileName configFileName = m_params.GetAssetsDir();
    configFileName.AppendToPath(L"CivilImportConfig.xml");
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
                ORDBRIDGE_LOG.fatalv(L"%ls - file not found", _GetParams().GetInputFileName().GetName());
                fwprintf(stderr, L"%ls - file not found\n", _GetParams().GetInputFileName().GetName());
                break;

            default:
                m_converter->ReportDgnV8FileOpenError(initStat, _GetParams().GetInputFileName().c_str());
                ORDBRIDGE_LOG.fatalv(L"%ls - cannot find or load root model. See %ls-issues for more information.", _GetParams().GetInputFileName().GetName(), _GetParams().GetBriefcaseName().GetName());
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
            ORDBRIDGE_LOG.fatalv(L"%ls - error - the selected root model [%ls] was previously converted, not as a root but as a reference attachment.", _GetParams().GetBriefcaseName().GetName(), m_converter->GetRootModelP()->GetModelName());
            return nullptr;
            }

        BeAssert(DgnDbSync::DgnV8::RootModelConverter::ImportJobCreateStatus::FailedExistingRoot != status); // If the root was previously converted, then we should be doing an update!
        }

    if (DgnDbSync::DgnV8::RootModelConverter::ImportJobCreateStatus::Success == status)
        {
        auto& importJobSubjectCR = m_converter->GetImportJob().GetSubject();

        auto physicalSubjectIdCreatedByConverter = GetDgnDbR().Elements().QueryElementIdByCode(Subject::CreateCode(importJobSubjectCR, "Physical"));
        if (physicalSubjectIdCreatedByConverter.IsValid())
            {
            auto graphicalSubjectPtr = GetDgnDbR().Elements().GetForEdit<Subject>(physicalSubjectIdCreatedByConverter);
            graphicalSubjectPtr->SetCode(Subject::CreateCode(importJobSubjectCR, SubjectCivilGraphicsBreakdownStructureCode));

            Json::Value graphicalSubjectProps = graphicalSubjectPtr->GetJsonProperties(Subject::json_Model()); // Making a copy
            graphicalSubjectProps["Perspective"] = "AppGraphics";
            graphicalSubjectPtr->SetJsonProperties(Subject::json_Model(), graphicalSubjectProps);

            graphicalSubjectPtr->Update();
            }
        
        return &importJobSubjectCR;
        }
    else
        return nullptr;
    }

OBMConverter* s_obmConverter = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDBridge::InitializeAlignedPartitions(SubjectCR jobSubject)
    {
    auto alignedSubjectPtr = Subject::Create(jobSubject, m_converter->GetAlignedSubjectName());

    Json::Value modelProps(Json::nullValue);
    modelProps["Perspective"] = "Aligned";
    alignedSubjectPtr->SetJsonProperties(Subject::json_Model(), modelProps);
    SubjectCPtr alignedSubjectCPtr = dynamic_cast<SubjectCP>(alignedSubjectPtr->Insert().get());
    if (alignedSubjectCPtr.IsNull())
        {
        ORDBRIDGE_LOG.fatalv(L"%ls - error - creation of the aligned subject failed.", _GetParams().GetBriefcaseName().GetName());
        return BentleyStatus::ERROR;
        }

    RoadRailAlignment::RoadRailAlignmentDomain::OnSchemaImported(*alignedSubjectCPtr);
    RoadRailPhysical::RoadRailPhysicalDomain::OnSchemaImported(*alignedSubjectCPtr);
    BridgeStructuralPhysical::BridgeStructuralPhysicalDomain::OnSchemaImported(*alignedSubjectCPtr);

    AlignmentBim::RoadRailAlignmentDomain::SetUpDefinitionPartitions(*alignedSubjectCPtr);
    RailBim::RailPhysicalDomain::SetUpDefinitionPartition(*alignedSubjectCPtr);
    RoadBim::RoadPhysicalDomain::SetUpDefinitionPartition(*alignedSubjectCPtr);

    auto clippingsPartitionPtr = SpatialLocationPartition::Create(*alignedSubjectPtr, "TerrainClippings");
    auto clippingsPartitionCPtr = clippingsPartitionPtr->Insert();
    if (clippingsPartitionCPtr.IsNull())
        return BentleyStatus::ERROR;

    auto& spatialLocationModelHandlerR = dgn_ModelHandler::SpatialLocation::GetHandler();
    auto clippingsModelPtr = spatialLocationModelHandlerR.Create(
        Dgn::DgnModel::CreateParams(GetDgnDbR(), GetDgnDbR().Domains().GetClassId(spatialLocationModelHandlerR),
            clippingsPartitionCPtr->GetElementId()));
    clippingsModelPtr->SetIsPrivate(DomainModelsPrivate);
    if (DgnDbStatus::Success != clippingsModelPtr->Insert())
        return BentleyStatus::ERROR;

    m_converter->SetClippingsModelId(clippingsModelPtr->GetModelId());

    auto physicalPartitionCPtr = RoadRailBim::PhysicalModelUtilities::CreateAndInsertPhysicalPartitionAndModel(
        *alignedSubjectCPtr, DefaultPhysicalPartitionName);
    BeAssert(physicalPartitionCPtr.IsValid());

    auto roadNetworkCPtr = RoadBim::RoadNetwork::Insert(*physicalPartitionCPtr->GetSubModel()->ToPhysicalModelP(), DefaultRoadNetworkName);
    auto railNetworkCPtr = RailBim::RailNetwork::Insert(*physicalPartitionCPtr->GetSubModel()->ToPhysicalModelP(), DefaultRailNetworkName);

    // IMODELBRIDGE REQUIREMENT: Relate this model to the source document
    m_converter->SetRoadNetwork(*roadNetworkCPtr);
    m_converter->SetRailNetwork(*railNetworkCPtr);

    InsertElementHasLinksRelationship(GetDgnDbR(), physicalPartitionCPtr->GetElementId(), m_converter->GetRepositoryLinkId(*m_converter->GetRootV8File()));

    auto designAlignmentsCPtr = AlignmentBim::DesignAlignments::Insert(*roadNetworkCPtr->GetNetworkModel(), DefaultDesignAlignmentsName);
    auto designAlignmentModelPtr = designAlignmentsCPtr->GetAlignmentModel();

    if (DomainModelsPrivate)
        {
        AlignmentBim::RoadRailAlignmentDomain::QueryCategoryModel(GetDgnDbR())->SetIsPrivate(true);

        physicalPartitionCPtr->GetSubModel()->SetIsPrivate(true);
        physicalPartitionCPtr->GetSubModel()->Update();

        roadNetworkCPtr->GetNetworkModel()->SetIsPrivate(true);
        roadNetworkCPtr->GetNetworkModel()->Update();

        railNetworkCPtr->GetNetworkModel()->SetIsPrivate(true);
        railNetworkCPtr->GetNetworkModel()->Update();

        auto horizontalAlignmentsCPtr = AlignmentBim::HorizontalAlignments::Query(*roadNetworkCPtr->GetNetworkModel());
        if (horizontalAlignmentsCPtr.IsValid())
            {
            horizontalAlignmentsCPtr->GetHorizontalModel()->SetIsPrivate(true);
            horizontalAlignmentsCPtr->GetHorizontalModel()->Update();
            }

        designAlignmentModelPtr->SetIsPrivate(true);
        designAlignmentModelPtr->Update();

        horizontalAlignmentsCPtr = AlignmentBim::HorizontalAlignments::Query(*designAlignmentModelPtr);
        horizontalAlignmentsCPtr->GetHorizontalModel()->SetIsPrivate(true);
        horizontalAlignmentsCPtr->GetHorizontalModel()->Update();
        }

    m_converter->SetUpModelFormatters();

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ORDBridge::_ConvertToBim(SubjectCR jobSubject)
    {
    ORDBRIDGE_LOGI("ORDConverter ConvertToBim called.");
    
    if (m_converter->IsUpdating())
        {
        auto alignedSubjectCPtr = m_converter->GetAlignedSubject();
        RoadRailAlignment::RoadRailAlignmentDomain::SetParentSubject(*alignedSubjectCPtr);

        // IMODELBRIDGE REQUIREMENT: Relate this model to the source document
        auto physicalNetworkModelPtr = RoadRailBim::PhysicalModelUtilities::QueryRoadNetworkModel(*alignedSubjectCPtr,
            DefaultPhysicalPartitionName, DefaultRoadNetworkName);
        m_converter->SetRoadNetwork(*RoadBim::RoadNetwork::Get(GetDgnDbR(), physicalNetworkModelPtr->GetModeledElement()->GetElementId()));

        physicalNetworkModelPtr = RoadRailBim::PhysicalModelUtilities::QueryRailNetworkModel(*alignedSubjectCPtr,
            DefaultPhysicalPartitionName, DefaultRailNetworkName);
        m_converter->SetRailNetwork(*RailBim::RailNetwork::Get(GetDgnDbR(), physicalNetworkModelPtr->GetModeledElement()->GetElementId()));

        auto terrainClippingsPartitionCode = SpatialLocationPartition::CreateCode(*alignedSubjectCPtr, "TerrainClippings");
        auto clippingsModelId = GetDgnDbR().Models().QuerySubModelId(terrainClippingsPartitionCode);
        m_converter->SetClippingsModelId(clippingsModelId);
        }
    else
        {
        if (BentleyStatus::SUCCESS != InitializeAlignedPartitions(jobSubject))
            return BentleyStatus::ERROR;
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
    ConvertOBMElementXDomain convertOBMXDomain(*m_converter);
    Dgn::DgnDbSync::DgnV8::XDomain::Register(convertORDXDomain);
    //Dgn::DgnDbSync::DgnV8::XDomain::Register(convertOBMXDomain);

    ORDBRIDGE_LOGI("ORDConverter ConvertData started.");

    m_converter->SetIsProcessing(true);    
    m_converter->ConvertData();
    m_converter->SetIsProcessing(false);

    ORDBRIDGE_LOGI("ORDConverter ConvertData stopped.");

    Dgn::DgnDbSync::DgnV8::XDomain::UnRegister(convertORDXDomain);
    //Dgn::DgnDbSync::DgnV8::XDomain::UnRegister(convertOBMXDomain);

    ORDConverterExtension::UnRegister(*s_obmConverter);
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
* @bsimethod                                    Jonathan.DeCarlo            09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDBridge::AppendObmSdkToDllSearchPath(BeFileNameCR libraryDir)
    {
    BeFileName dllDirectory(libraryDir);
    dllDirectory.AppendToPath(L"Obm");

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
    ORDBRIDGE_LOGI(L"TODO OnDocumentDeleted %ls", documentId.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDBridge::_MakeDefinitionChanges(SubjectCR jobSubject)
    {
    BentleyStatus status = T_Super::_MakeDefinitionChanges(jobSubject);
    if (BentleyStatus::SUCCESS == status)
        {
        if (BentleyStatus::SUCCESS != (status = m_converter->MakeDefinitionChanges()))
            {
            ORDBRIDGE_LOGE("ORDConverter MakeDefinitionChanges failed.");
            return status;
            }
        }
    else
        {
        ORDBRIDGE_LOGE("iModelBridge MakeDefinitionChanges failed.");
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Diego.Diaz                      01/2018
 +---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDBridge::_MakeSchemaChanges(bool& hasMoreChanges)
    {
    BentleyStatus status = SUCCESS;
    if (m_schemaImportPhase == SchemaImportPhase::Base)
        {
        ORDBRIDGE_LOGI("ORDConverter MakeSchemaChanges Import Base.");
        status = m_converter->MakeSchemaChanges();
        m_schemaImportPhase = SchemaImportPhase::RoadRail;
        hasMoreChanges = true;
        }
    else if (m_schemaImportPhase == SchemaImportPhase::RoadRail)
        {
        ORDBRIDGE_LOGI("ORDConverter MakeSchemaChanges Import Road & Rail.");
        status = m_converter->MakeRoadRailSchemaChanges();
        m_schemaImportPhase = SchemaImportPhase::Dynamic;
        hasMoreChanges = true;
        }
    else if (m_schemaImportPhase == SchemaImportPhase::Dynamic)
        {
        ORDBRIDGE_LOGI("ORDConverter MakeSchemaChanges Import Dynamic.");
        status = m_converter->AddDynamicSchema();

        if (0 == m_converter->GetExtensionCount())
            {
            hasMoreChanges = false;
            m_schemaImportPhase = SchemaImportPhase::Done;
            GetDgnDbR().Schemas().CreateClassViewsInDb(); // For debugging purposes
            }
        else
            {
            m_schemaImportPhase = SchemaImportPhase::Extensions;
            hasMoreChanges = true;
            }
        }
    else if (m_schemaImportPhase == SchemaImportPhase::Extensions)
        {
        status = m_converter->AddExtensionSchema(hasMoreChanges);

        if (!hasMoreChanges)
            {
            m_schemaImportPhase = SchemaImportPhase::Done;
            GetDgnDbR().Schemas().CreateClassViewsInDb(); // For debugging purposes
            }
        }
    else
        {
        BeAssert(false);
        ORDBRIDGE_LOGE("Too many calls to _MakeSchemaChanges.");
        }

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

        s_obmConverter = new OBMConverter(*m_converter);
        ORDConverterExtension::UnRegisterAll();
        ORDConverterExtension::Register(*s_obmConverter);
        }
    m_converter->SetDgnDb(db);
    if (BentleyStatus::SUCCESS != m_converter->AttachSyncInfo())
        {
        ORDBRIDGE_LOGE("ORDConverter AttachSyncInfo failed.");
        return BentleyStatus::ERROR;
        }

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