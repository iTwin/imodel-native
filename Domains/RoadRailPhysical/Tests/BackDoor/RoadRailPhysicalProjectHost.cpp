/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    createDgnDbParams.SetRootSubjectName("RoadRailPhysicalProject");
    createDgnDbParams.SetRootSubjectDescription("Created by RoadRailPhysicalProjectHost");
    createDgnDbParams.SetStartDefaultTxn(DefaultTxn::Exclusive);

    DbResult createStatus;
    BeFileName fileName = BuildProjectFileName(baseName);
    DgnDbPtr projectPtr = DgnDb::CreateDgnDb(&createStatus, fileName, createDgnDbParams);
    if (!projectPtr.IsValid() || DbResult::BE_SQLITE_OK != createStatus)
        return nullptr;

    auto schemaReadContextPtr = ECN::ECSchemaReadContext::CreateContext(false, true);

    auto assetsDir = GetDgnPlatformAssetsDirectory();
    BeFileName ecdbDir = assetsDir;
    ecdbDir.AppendToPath(L"ECSchemas");
    ecdbDir.AppendToPath(L"ECDb");

    BeFileName dgnDir = assetsDir;
    dgnDir.AppendToPath(L"ECSchemas");
    dgnDir.AppendToPath(L"Dgn");

    BeFileName domainDir = assetsDir;
    domainDir.AppendToPath(L"ECSchemas");
    domainDir.AppendToPath(L"Domain");

    schemaReadContextPtr->AddSchemaPath(ecdbDir);
    schemaReadContextPtr->AddSchemaPath(dgnDir);
    schemaReadContextPtr->AddSchemaPath(domainDir);

    BeFileName lrSchemaPath = assetsDir;
    lrSchemaPath.AppendToPath(LinearReferencingDomain::GetSchemaRelativePath());

    ECSchemaPtr lrSchemaPtr;
    if (SchemaReadStatus::Success != ECSchema::ReadFromXmlFile(lrSchemaPtr, lrSchemaPath.c_str(), *schemaReadContextPtr))
        return nullptr;

    BeFileName rraSchemaPath = assetsDir;
    rraSchemaPath.AppendToPath(RoadRailAlignmentDomain::GetSchemaRelativePath());

    ECSchemaPtr rraSchemaPtr;
    if (SchemaReadStatus::Success != ECSchema::ReadFromXmlFile(rraSchemaPtr, rraSchemaPath.c_str(), *schemaReadContextPtr))
        return nullptr;

    BeFileName rrpSchemaPath = assetsDir;
    rrpSchemaPath.AppendToPath(RoadRailPhysicalDomain::GetSchemaRelativePath());

    ECSchemaPtr rrpSchemaPtr;
    if (SchemaReadStatus::Success != ECSchema::ReadFromXmlFile(rrpSchemaPtr, rrpSchemaPath.c_str(), *schemaReadContextPtr))
        return nullptr;

    BeFileName rdpSchemaPath = assetsDir;
    rdpSchemaPath.AppendToPath(RoadPhysicalDomain::GetSchemaRelativePath());

    ECSchemaPtr rdpSchemaPtr;
    if (SchemaReadStatus::Success != ECSchema::ReadFromXmlFile(rdpSchemaPtr, rdpSchemaPath.c_str(), *schemaReadContextPtr))
        return nullptr;

    BeFileName rlpSchemaPath = assetsDir;
    rlpSchemaPath.AppendToPath(RailPhysicalDomain::GetSchemaRelativePath());

    ECSchemaPtr rlpSchemaPtr;
    if (SchemaReadStatus::Success != ECSchema::ReadFromXmlFile(rlpSchemaPtr, rlpSchemaPath.c_str(), *schemaReadContextPtr))
        return nullptr;

    bvector<ECSchemaCP> schemas;
    schemas.push_back(lrSchemaPtr.get());
    schemas.push_back(rraSchemaPtr.get());
    schemas.push_back(rrpSchemaPtr.get());
    schemas.push_back(rdpSchemaPtr.get());
    schemas.push_back(rlpSchemaPtr.get());

    if (SchemaStatus::Success != projectPtr->ImportSchemas(schemas))
        return nullptr;

    BeAssert(BentleyStatus::SUCCESS == projectPtr->Schemas().CreateClassViewsInDb());

    auto subjectCPtr = projectPtr->Elements().GetRootSubject();
    RoadRailAlignmentDomain::OnSchemaImported(*subjectCPtr);
    RoadRailPhysicalDomain::OnSchemaImported(*subjectCPtr);

    RoadRailAlignmentDomain::SetUpDefinitionPartitions(*subjectCPtr);
    RailPhysicalDomain::SetUpDefinitionPartition(*subjectCPtr);
    RoadPhysicalDomain::SetUpDefinitionPartition(*subjectCPtr);

    auto physicalPartitionCPtr = PhysicalModelUtilities::CreateAndInsertPhysicalPartitionAndModel(*subjectCPtr, "Physical");

    auto roadNetworkCPtr = RoadNetwork::Insert(*physicalPartitionCPtr->GetSubModel()->ToPhysicalModelP(),
        "Road Network");

    DesignAlignments::Insert(*roadNetworkCPtr->GetNetworkModel(), "Design Alignments");

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

    RoadRailAlignmentDomain::SetParentSubject(*projectPtr->Elements().GetRootSubject());
    return projectPtr;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
RoadRailPhysicalProjectHostImpl::RoadRailPhysicalProjectHostImpl() : m_isInitialized(false)
    {
    BeAssert((DgnPlatformLib::QueryHost() == NULL) && L"This means an old host is still registered. You should have terminated it first before creating a new host.");

    DgnPlatformLib::Initialize(*this);
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
        s_currentProject->ResetBriefcaseId(BeBriefcaseId(1));
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
        s_currentProject->ResetBriefcaseId(BeBriefcaseId(1));
        s_currentProject->SaveChanges();
        s_currentProject->CloseDb();
        s_currentProject = m_host->OpenProject(baseName);
        }

    return s_currentProject;
    }