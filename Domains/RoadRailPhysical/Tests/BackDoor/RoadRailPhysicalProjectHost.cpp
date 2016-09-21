#include "../BackDoor/PublicApi/BackDoor/RoadRailPhysical/BackDoor.h"
#include <DgnPlatform/DgnPlatformLib.h>

#include <DgnPlatform\DgnGeoCoord.h>

BEGIN_BENTLEY_ROADRAILPHYSICAL_UNITTESTS_NAMESPACE

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



//=======================================================================================
// @bsiclass
//=======================================================================================
struct RoadRailPhysicalProjectHostImpl : DgnPlatformLib::Host
    {
    bool m_isInitialized;

    RoadRailPhysicalProjectHostImpl();
    ~RoadRailPhysicalProjectHostImpl();

    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new TestKnownLocationsAdmin(); }
    virtual RepositoryAdmin & _SupplyRepositoryAdmin() override { return *new TestRepositoryAdmin(); }

    //virtual HandlerAdmin& _SupplyHandlerAdmin() override { return *new TestHandlerAdmin; }
    virtual void _SupplyProductName(Utf8StringR name) override { name.assign("RoadRailPhysicalProjectHost"); }
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); } // no translatable strings
    };

END_BENTLEY_ROADRAILPHYSICAL_UNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
RoadRailPhysicalProjectHost::RoadRailPhysicalProjectHost()
    {
    m_pimpl = new RoadRailPhysicalProjectHostImpl;
    CleanOutputDirectory();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
RoadRailPhysicalProjectHost::~RoadRailPhysicalProjectHost()
    {
    delete m_pimpl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
void RoadRailPhysicalProjectHost::CleanOutputDirectory()
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
BeFileName RoadRailPhysicalProjectHost::GetOutputDirectory()
    {
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    outputDir.AppendToPath(L"RoadRailPhysical");
    return outputDir;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName RoadRailPhysicalProjectHost::GetDgnPlatformAssetsDirectory()
    {
    BeFileName assetsRootDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsRootDirectory);
    return assetsRootDirectory;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName RoadRailPhysicalProjectHost::BuildProjectFileName(WCharCP baseName)
    {
    BeFileName projectFileName = GetOutputDirectory();
    projectFileName.AppendToPath(baseName);
    return projectFileName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
DgnDbPtr RoadRailPhysicalProjectHost::CreateProject(WCharCP baseName)
    {
    CreateDgnDbParams createDgnDbParams;
    createDgnDbParams.SetOverwriteExisting(true);
    createDgnDbParams.SetRootSubjectLabel("RoadRailPhysicalProject");
    createDgnDbParams.SetRootSubjectDescription("Created by RoadRailPhysicalProjectHost");
    createDgnDbParams.SetStartDefaultTxn(DefaultTxn::Exclusive);

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
    BeFileName lrSchemaFileName = schemaRootDir;
    lrSchemaFileName.AppendToPath(BLR_SCHEMA_FILE);    
    if (DgnDbStatus::Success != (status = LinearReferencing::LinearReferencingDomain::GetDomain().ImportSchema(*projectPtr, lrSchemaFileName)))
        return nullptr;

    BeFileName alignSchemaFileName = schemaRootDir;
    alignSchemaFileName.AppendToPath(BRRA_SCHEMA_FILE);
    if (DgnDbStatus::Success != (status = RoadRailAlignment::RoadRailAlignmentDomain::GetDomain().ImportSchema(*projectPtr, alignSchemaFileName)))
        return nullptr;

    BeFileName roadRailSchemaFileName = schemaRootDir;
    roadRailSchemaFileName.AppendToPath(BRRP_SCHEMA_FILE);
    if (DgnDbStatus::Success != (status = RoadRailPhysical::RoadRailPhysicalDomain::GetDomain().ImportSchema(*projectPtr, roadRailSchemaFileName)))
        return nullptr;

    projectPtr->Schemas().CreateECClassViewsInDb();

    auto& alignmentModelHandlerR = AlignmentModelHandler::GetHandler();
    auto alignmentModelPtr = alignmentModelHandlerR.Create(DgnModel::CreateParams(*projectPtr, AlignmentModel::QueryClassId(*projectPtr),
        projectPtr->Elements().GetRootSubjectId(), AlignmentModel::CreateModelCode("Test Alignment Model")));

    if (DgnDbStatus::Success != alignmentModelPtr->Insert())
        return nullptr;

    auto& physicalModelHandlerR = dgn_ModelHandler::Physical::GetHandler();
    auto physicalModelPtr = physicalModelHandlerR.Create(DgnModel::CreateParams(*projectPtr, projectPtr->Domains().GetClassId(physicalModelHandlerR),
        projectPtr->Elements().GetRootSubjectId(), PhysicalModel::CreateModelCode("Test Physical Model")));

    if (DgnDbStatus::Success != physicalModelPtr->Insert())
        return nullptr;

    return projectPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        04/2015
//---------------------------------------------------------------------------------------
DgnDbPtr RoadRailPhysicalProjectHost::OpenProject(WCharCP baseName)
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
RoadRailPhysicalProjectHostImpl::RoadRailPhysicalProjectHostImpl() : m_isInitialized(false)
    {
    BeAssert((DgnPlatformLib::QueryHost() == NULL) && L"This means an old host is still registered. You should have terminated it first before creating a new host.");

    DgnPlatformLib::Initialize(*this, false);
    DgnDomains::RegisterDomain(LinearReferencingDomain::GetDomain());
    DgnDomains::RegisterDomain(RoadRailAlignmentDomain::GetDomain());
    DgnDomains::RegisterDomain(RoadRailPhysicalDomain::GetDomain());
    m_isInitialized = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
RoadRailPhysicalProjectHostImpl::~RoadRailPhysicalProjectHostImpl()
    {
    if (m_isInitialized)
        {
        Terminate(false);
        }
    }


RoadRailPhysicalProjectHost* RoadRailPhysicalTestsFixture::m_host = nullptr;
DgnDbPtr RoadRailPhysicalTestsFixture::s_currentProject = DgnDbPtr(nullptr);
//---------------------------------------------------------------------------------------
// Automatically called by gTest framework before running every test
//---------------------------------------------------------------------------------------
void RoadRailPhysicalTestsFixture::SetUpTestCase()
    {
    m_host = new RoadRailPhysicalProjectHost();
    }
//---------------------------------------------------------------------------------------
// Automatically called by gTest framework after running every test
//---------------------------------------------------------------------------------------
void RoadRailPhysicalTestsFixture::TearDownTestCase()
    {
    if (s_currentProject.IsValid())
        s_currentProject->SaveChanges();

    s_currentProject = nullptr;

    delete m_host;
    m_host = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DgnModelId RoadRailPhysicalTestsFixture::QueryFirstAlignmentModelId(DgnDbR db)
    {
    for (auto const& modelEntry : db.Models().MakeIterator())
        {
        if ((DgnModel::RepositoryModelId() == modelEntry.GetModelId()) || (DgnModel::DictionaryId() == modelEntry.GetModelId()))
            continue;

        DgnModelPtr model = db.Models().GetModel(modelEntry.GetModelId());
        if (model->IsGeometricModel() && dynamic_cast<AlignmentModelP>(model.get()))
            return modelEntry.GetModelId();
        }

    BeAssert(false && "No AlignmentModel found");
    return DgnModelId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DgnModelId RoadRailPhysicalTestsFixture::QueryFirstPhysicalModelId(DgnDbR db)
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbPtr RoadRailPhysicalTestsFixture::CreateProject(WCharCP baseName, bool needsSetBriefcase)
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
Dgn::DgnDbPtr RoadRailPhysicalTestsFixture::OpenProject(WCharCP baseName, bool needsSetBriefcase)
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
