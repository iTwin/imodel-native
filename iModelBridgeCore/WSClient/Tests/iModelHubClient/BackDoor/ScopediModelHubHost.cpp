/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/iModelHubClient/BackDoor/ScopediModelHubHost.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/WebServices/iModelHub/BackDoor.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTest.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

namespace {

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TestKnownLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
{
    BeFileName m_tempDir;
    BeFileName m_assetsDir;

    virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override {return m_tempDir;}
    virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override {return m_assetsDir;}

    TestKnownLocationsAdmin()
        {
        BeTest::GetHost().GetTempDir(m_tempDir);
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(m_assetsDir);
        }
};

}


/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProxyServerAdmin : Dgn::DgnPlatformLib::Host::RepositoryAdmin
    {
    DEFINE_T_SUPER(RepositoryAdmin);

    RepositoryAdmin* m_impl;

    ProxyServerAdmin() : m_impl(nullptr) {}
    virtual IBriefcaseManagerPtr _CreateBriefcaseManager (DgnDbR db) const override
        {
        return nullptr != m_impl ? m_impl->_CreateBriefcaseManager (db) : T_Super::_CreateBriefcaseManager (db);
        }
    virtual IRepositoryManagerP _GetRepositoryManager (DgnDbR db) const override
        {
        return nullptr != m_impl ? m_impl->_GetRepositoryManager(db) : T_Super::_GetRepositoryManager(db);
        }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE

struct ScopediModelHubHostImpl : DgnPlatformLib::Host
{
    bool m_isInitialized;

    ScopediModelHubHostImpl();
    ~ScopediModelHubHostImpl();

    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override {return *new TestKnownLocationsAdmin();}
    virtual void _SupplyProductName(Utf8StringR name) override {name.assign("ScopediModelHubHost");}
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() {return BeSQLite::L10N::SqlangFiles(BeFileName());}
    RepositoryAdmin& _SupplyRepositoryAdmin() override { return *new ProxyServerAdmin(); }
    void SetRepositoryAdmin(DgnPlatformLib::Host::RepositoryAdmin* admin) { ((ProxyServerAdmin*)m_repositoryAdmin)->m_impl = admin; }
};


namespace BackDoor
{
}
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE

void ScopediModelHubHost::SetRepositoryAdmin(DgnPlatformLib::Host::RepositoryAdmin* admin)
    {
    m_pimpl->SetRepositoryAdmin(admin);
    }

ScopediModelHubHost::ScopediModelHubHost()
    {
    m_pimpl = nullptr;
    DgnPlatformLib::Host* currentHost = DgnPlatformLib::QueryHost();
    if (nullptr != currentHost)
        {
        m_pimpl = dynamic_cast<ScopediModelHubHostImpl*>(currentHost);
        }
    if (nullptr == m_pimpl)
        {
        if (nullptr != currentHost)
            {
            currentHost->Terminate(false);
            }
        m_pimpl = new ScopediModelHubHostImpl();
        }
    }

ScopediModelHubHost::~ScopediModelHubHost()
    {
    delete m_pimpl;
    }

void ScopediModelHubHost::CleanOutputDirectory()
    {
    BeFileName outputDir = GetOutputDirectory();
    BeFileName::EmptyAndRemoveDirectory(outputDir.c_str());
    BeFileName::CreateNewDirectory(outputDir.c_str());
    }

BeFileName ScopediModelHubHost::GetDocumentsDirectory()
    {
    BeFileName documentsDir;
    BeTest::GetHost().GetDocumentsRoot(documentsDir);
    return documentsDir;
    }

BeFileName ScopediModelHubHost::GetTempDirectory()
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    return tempDir;
    }

BeFileName ScopediModelHubHost::GetOutputDirectory()
    {
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    outputDir.AppendToPath(L"iModelHub");
    return outputDir;
    }

BeFileName ScopediModelHubHost::GetDgnPlatformAssetsDirectory()
    {
    BeFileName assetsRootDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsRootDirectory);
    return assetsRootDirectory;
    }

BeFileName ScopediModelHubHost::BuildDbFileName(Utf8StringCR baseName)
    {
    WString wFileName;
    BeStringUtilities::Utf8ToWChar(wFileName, (baseName + ".bim").c_str());
    BeFileName projectFileName = GetOutputDirectory();
    projectFileName.AppendToPath(wFileName.c_str());
    return projectFileName;
    }

DgnDbPtr ScopediModelHubHost::CreateTestDb(Utf8StringCR name)
    {
    DgnDbPtr db;
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(true);
    createProjectParams.SetRootSubjectName(name.c_str());
    createProjectParams.SetRootSubjectDescription((name + " is created by ScopediModelHubHost").c_str());
    db = DgnDb::CreateDgnDb(nullptr, BuildDbFileName(name), createProjectParams);
    BeAssert(db.IsValid());
    return db;
    }

ScopediModelHubHostImpl::ScopediModelHubHostImpl() : m_isInitialized(false)
    {
    BeAssert((DgnPlatformLib::QueryHost() == NULL) && L"This means an old host is still registered. You should have terminated it first before creating a new host.");
    DgnPlatformLib::Initialize(*this, false);
    m_isInitialized = true;
    }

ScopediModelHubHostImpl::~ScopediModelHubHostImpl()
    {
    if (m_isInitialized)
        {
        Terminate(false);
        }
    }
