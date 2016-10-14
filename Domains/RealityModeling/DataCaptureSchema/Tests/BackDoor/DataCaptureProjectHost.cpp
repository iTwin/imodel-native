/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Tests/BackDoor/DataCaptureProjectHost.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicApi/BackDoor/DataCapture/BackDoor.h"
#include <DgnPlatform/DgnPlatformLib.h>

#include <DgnPlatform\DgnGeoCoord.h>

BEGIN_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TestKnownLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
{
BeFileName m_tempDir;
BeFileName m_assetsDir;

virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override { return m_tempDir; }
virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override { return m_assetsDir; }

TestKnownLocationsAdmin()
    {
    BeTest::GetHost().GetTempDir(m_tempDir);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(m_assetsDir);
    }
};

#if 0
//=======================================================================================
// @bsiclass
//! Refer to http://bsw-wiki.bentley.com/bin/view.pl/Main/DgnDbServerClientAPIExamples
//! This implementation bypasses all the checks for locks and codes
//=======================================================================================
struct RepositoryManager : IRepositoryManager
{
protected:
    virtual Response _ProcessRequest(Request const& req, DgnDbR db, bool queryOnly) override { return Response(queryOnly ? IBriefcaseManager::RequestPurpose::Query : IBriefcaseManager::RequestPurpose::Acquire, req.Options(), RepositoryStatus::Success); }
    virtual RepositoryStatus _Demote(DgnLockSet const&, DgnCodeSet const&, DgnDbR) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _Relinquish(Resources, DgnDbR) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _QueryStates(DgnLockInfoSet&, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) override { return RepositoryStatus::Success; }
};

//=======================================================================================
// @bsiclass                            Alexandre.Gagnon                        04/2016
//=======================================================================================
struct TestRepositoryAdmin : DgnPlatformLib::Host::RepositoryAdmin
    {
    private:
        mutable RepositoryManager m_client;

    protected:
        virtual IRepositoryManagerP _GetRepositoryManager(DgnDbR db) const override { return &m_client; }
    };

#endif


//=======================================================================================
// @bsiclass
//=======================================================================================
struct DataCaptureProjectHostImpl : DgnPlatformLib::Host
    {
    bool m_isInitialized;

    DataCaptureProjectHostImpl();
    ~DataCaptureProjectHostImpl();

    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new TestKnownLocationsAdmin(); }
    //virtual RepositoryAdmin & _SupplyRepositoryAdmin() override { return *new TestRepositoryAdmin(); }

    //virtual HandlerAdmin& _SupplyHandlerAdmin() override { return *new TestHandlerAdmin; }
    virtual void _SupplyProductName(Utf8StringR name) override { name.assign("DataCaptureProjectHost"); }
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); } // no translatable strings
    };

END_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
DataCaptureProjectHost::DataCaptureProjectHost()
    {
    m_pimpl = new DataCaptureProjectHostImpl;
    CleanOutputDirectory();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
DataCaptureProjectHost::~DataCaptureProjectHost()
    {
    delete m_pimpl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
void DataCaptureProjectHost::CleanOutputDirectory()
    {
    static bool wasOutputDirectoryCleaned = false;
    if (!wasOutputDirectoryCleaned)
        {
        // clean up files from last run
        BeFileName outputDir = GetOutputDirectory();
        BeFileName::EmptyAndRemoveDirectory(outputDir.GetName());
        BeFileName::CreateNewDirectory(outputDir.GetName());
        wasOutputDirectoryCleaned = true;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName DataCaptureProjectHost::GetOutputDirectory()
    {
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    outputDir.AppendToPath(L"DataCapture");
    return outputDir;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName DataCaptureProjectHost::GetDgnPlatformAssetsDirectory()
    {
    BeFileName assetsRootDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsRootDirectory);
    return assetsRootDirectory;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName DataCaptureProjectHost::BuildProjectFileName(WCharCP baseName)
    {
    BeFileName projectFileName = GetOutputDirectory();
    projectFileName.AppendToPath(baseName);
    return projectFileName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
DgnDbPtr DataCaptureProjectHost::CreateProject(WCharCP baseName)
    {
    CreateDgnDbParams createDgnDbParams;
    createDgnDbParams.SetOverwriteExisting(true);
//     createDgnDbParams.SetRootSubjectLabel("DataCaptureProject");
//     createDgnDbParams.SetRootSubjectDescription("Created by DataCaptureProjectHost");
    createDgnDbParams.SetProjectName("DataCaptureProject");
    createDgnDbParams.SetProjectDescription("Created by DataCaptureProjectHost");
//     createDgnDbParams.SetStartDefaultTxn(DefaultTxn::Exclusive);

    DbResult createStatus;
    BeFileName fileName = BuildProjectFileName(baseName);
    DgnDbPtr projectPtr = DgnDb::CreateDgnDb(&createStatus, fileName, createDgnDbParams);
    if (!projectPtr.IsValid() || DbResult::BE_SQLITE_OK != createStatus)
        return nullptr;

    BeFileName assetsRootDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsRootDir);

    BeFileName schemaRootDir = assetsRootDir;
    schemaRootDir.AppendToPath(L"ECSchemas\\Domain\\");

     DgnDbStatus status;
//     BeFileName lrSchemaFileName = schemaRootDir;
//     lrSchemaFileName.AppendToPath(BLR_SCHEMA_FILE);    
//     if (DgnDbStatus::Success != (status = LinearReferencing::LinearReferencingDomain::GetDomain().ImportSchema(*projectPtr, lrSchemaFileName)))
//         return nullptr;

//     BeFileName alignSchemaFileName = schemaRootDir;
//     alignSchemaFileName.AppendToPath(BRRA_SCHEMA_FILE);
//     if (DgnDbStatus::Success != (status = DataCaptureAlignment::DataCaptureAlignmentDomain::GetDomain().ImportSchema(*projectPtr, alignSchemaFileName)))
//         return nullptr;

//     BeFileName costSchemaFileName = schemaRootDir;
//     costSchemaFileName.AppendToPath(BCST_SCHEMA_FILE);
//     if (DgnDbStatus::Success != (status = Costing::CostingDomain::GetDomain().ImportSchema(*projectPtr, costSchemaFileName)))
//         return nullptr;

//     BeFileName bridgeSchemaFileName = schemaRootDir;
//     bridgeSchemaFileName.AppendToPath(BBP_SCHEMA_FILE);
//     if (DgnDbStatus::Success != (status = BridgePhysical::BridgePhysicalDomain::GetDomain().ImportSchema(*projectPtr, bridgeSchemaFileName)))
//         return nullptr;

    BeFileName DataCaptureSchemaFileName = schemaRootDir;
    DataCaptureSchemaFileName.AppendToPath(BDCP_SCHEMA_FILE);
    if (DgnDbStatus::Success != (status = DataCapture::DataCaptureDomain::GetDomain().ImportSchema(*projectPtr, DataCaptureSchemaFileName)))
        return nullptr;

    projectPtr->Schemas().CreateECClassViewsInDb();

//     auto& alignmentModelHandlerR = AlignmentModelHandler::GetHandler();
//     auto alignmentModelPtr = alignmentModelHandlerR.Create(DgnModel::CreateParams(*projectPtr, AlignmentModel::QueryClassId(*projectPtr),
//         projectPtr->Elements().GetRootSubjectId(), AlignmentModel::CreateModelCode("Test Alignment Model")));
// 
//     if (DgnDbStatus::Success != alignmentModelPtr->Insert())
//         return nullptr;

//     auto& physicalModelHandlerR = dgn_ModelHandler::Physical::GetHandler();
//     auto physicalModelPtr = physicalModelHandlerR.Create(DgnModel::CreateParams(*projectPtr, projectPtr->Domains().GetClassId(physicalModelHandlerR),
//         projectPtr->Elements().GetRootSubjectId(), PhysicalModel::CreateModelCode("Test Physical Model")));

//     if (DgnDbStatus::Success != physicalModelPtr->Insert())
//         return nullptr;

    return projectPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        04/2015
//---------------------------------------------------------------------------------------
DgnDbPtr DataCaptureProjectHost::OpenProject(WCharCP baseName)
    {
    DbResult openStatus;
    BeFileName fileName = BuildProjectFileName(baseName);

    DgnDbPtr projectPtr = DgnDb::OpenDgnDb(&openStatus, fileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    if (!projectPtr.IsValid() || (DbResult::BE_SQLITE_OK != openStatus))
        return nullptr;

    return projectPtr;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
DataCaptureProjectHostImpl::DataCaptureProjectHostImpl() : m_isInitialized(false)
    {
    BeAssert((DgnPlatformLib::QueryHost() == NULL) && L"This means an old host is still registered. You should have terminated it first before creating a new host.");

    DgnPlatformLib::Initialize(*this, false);
//     DgnDomains::RegisterDomain(LinearReferencingDomain::GetDomain());
//     DgnDomains::RegisterDomain(DataCaptureAlignmentDomain::GetDomain());
//     DgnDomains::RegisterDomain(BridgePhysicalDomain::GetDomain());
    DgnDomains::RegisterDomain(DataCaptureDomain::GetDomain());
    m_isInitialized = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
DataCaptureProjectHostImpl::~DataCaptureProjectHostImpl()
    {
    if (m_isInitialized)
        {
        Terminate(false);
        }
    }


DataCaptureProjectHost* DataCaptureTestsFixture::m_host = nullptr;
DgnDbPtr DataCaptureTestsFixture::s_currentProject = DgnDbPtr(nullptr);
//---------------------------------------------------------------------------------------
// Automatically called by gTest framework before running every test
//---------------------------------------------------------------------------------------
void DataCaptureTestsFixture::SetUpTestCase()
    {
    m_host = new DataCaptureProjectHost();
    }
//---------------------------------------------------------------------------------------
// Automatically called by gTest framework after running every test
//---------------------------------------------------------------------------------------
void DataCaptureTestsFixture::TearDownTestCase()
    {
    if (s_currentProject.IsValid())
        s_currentProject->SaveChanges();

    s_currentProject = nullptr;

    delete m_host;
    m_host = nullptr;
    }

#if 0
//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DgnModelId DataCaptureTestsFixture::QueryFirstPhysicalModelId(DgnDbR db)
    {
    for (auto const& modelEntry : db.Models().MakeIterator())
        {
        if ((DgnModel::RepositoryModelId() == modelEntry.GetModelId()) || (DgnModel::DictionaryId() == modelEntry.GetModelId()))
            continue;

        DgnModelPtr model = db.Models().GetModel(modelEntry.GetModelId());
        if (model->IsGeometricModel() && dynamic_cast<PhysicalModelP>(model.get()))
            return modelEntry.GetModelId();
        }

    BeAssert(false && "No PhysicalModel found");
    return DgnModelId();
    }
#endif
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbPtr DataCaptureTestsFixture::CreateProject(WCharCP baseName, bool needsSetBriefcase)
    {
    BeAssert(nullptr != m_host);

    const WCharCP testSeedName = L"TestSeed.bim";
    const BeFileName testSeedPath = m_host->BuildProjectFileName(testSeedName);
    const BeFileName projectName = m_host->BuildProjectFileName(baseName);

    //! Create seed
    if (!testSeedPath.DoesPathExist())
        {
        DgnDbPtr seedProject = m_host->CreateProject(testSeedName);

        //! Error
        if (seedProject.IsNull())
            return nullptr;

        seedProject->CloseDb();
        }

    if (s_currentProject.IsValid())
        {
        s_currentProject->SaveChanges();
        s_currentProject->CloseDb();
        s_currentProject = nullptr;
        }

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile(testSeedPath.c_str(), projectName.c_str(), false))
        return nullptr;

    s_currentProject = m_host->OpenProject(baseName);
    if (s_currentProject.IsNull())
        return nullptr;

    if (needsSetBriefcase)
        {
        s_currentProject->ChangeBriefcaseId(BeBriefcaseId(1));
        s_currentProject->CloseDb();
        s_currentProject = m_host->OpenProject(baseName);
        }

    return s_currentProject;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Dgn::DgnDbPtr DataCaptureTestsFixture::OpenProject(WCharCP baseName, bool needsSetBriefcase)
    {
    BeAssert(nullptr != m_host);

    const BeFileName projectName = m_host->BuildProjectFileName(baseName);

    bool wantsCurrentProject = false;

    if (s_currentProject.IsValid())
        {
        Utf8String projectNameUtf(projectName.c_str());
        if (0 == projectNameUtf.CompareTo(s_currentProject->GetDbFileName()))
            wantsCurrentProject = true;
        }

    if (!wantsCurrentProject)
        s_currentProject = m_host->OpenProject(baseName);

    if (needsSetBriefcase && s_currentProject.IsValid())
        {
        s_currentProject->ChangeBriefcaseId(BeBriefcaseId(1));
        s_currentProject->SaveChanges();
        s_currentProject->CloseDb();
        s_currentProject = m_host->OpenProject(baseName);
        }

    return s_currentProject;
    }
