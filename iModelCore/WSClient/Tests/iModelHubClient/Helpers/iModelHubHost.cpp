/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iModelHubHost.h"
#include <Bentley/BeTest.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

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
struct iModelHubHostImpl : DgnPlatformLib::Host
    {
    bool m_isInitialized;

    iModelHubHostImpl();
    ~iModelHubHostImpl();

    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new TestKnownLocationsAdmin(); }
    virtual void _SupplyProductName(Utf8StringR name) override { name.assign("iModelHubHost"); }
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); }
    RepositoryAdmin& _SupplyRepositoryAdmin() override { return *new ProxyServerAdmin(); }
    void SetRepositoryAdmin(DgnPlatformLib::Host::RepositoryAdmin* admin) { ((ProxyServerAdmin*) m_repositoryAdmin)->m_impl = admin; }
    };


iModelHubHost& iModelHubHost::Instance()
    {
    static iModelHubHost s_instance;
    return s_instance;
    }

void iModelHubHost::SetCustomOutputDir(BeFileName outputDir)
    {
    m_customOutputDir = outputDir;
    }

void iModelHubHost::SetRepositoryAdmin(DgnPlatformLib::Host::RepositoryAdmin* admin)
    {
    m_pimpl->SetRepositoryAdmin(admin);
    }

iModelHubHost::iModelHubHost()
    {
    m_pimpl = nullptr;
    DgnPlatformLib::Host* currentHost = DgnPlatformLib::QueryHost();
    if (nullptr != currentHost)
        {
        m_pimpl = dynamic_cast<iModelHubHostImpl*>(currentHost);
        }
    if (nullptr == m_pimpl)
        {
        if (nullptr != currentHost)
            {
            currentHost->Terminate(false);
            }
        m_pimpl = new iModelHubHostImpl();
        }
    }

iModelHubHost::~iModelHubHost()
    {
    delete m_pimpl;
    }

void iModelHubHost::CleanOutputDirectory()
    {
    BeFileName outputDir = GetOutputDirectory();
    BeFileName::EmptyAndRemoveDirectory(outputDir.c_str());
    BeFileName::CreateNewDirectory(outputDir.c_str());
    }

BeFileName iModelHubHost::GetDocumentsDirectory()
    {
    BeFileName documentsDir;
    BeTest::GetHost().GetDocumentsRoot(documentsDir);
    return documentsDir;
    }

BeFileName iModelHubHost::GetTempDirectory()
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    return tempDir;
    }

BeFileName iModelHubHost::GetOutputDirectory()
    {
    if (!m_customOutputDir.IsEmpty())
        return m_customOutputDir;

    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    outputDir.AppendToPath(L"iModelHub");

    if (!BeFileName::DoesPathExist(outputDir))
        BeFileName::CreateNewDirectory(outputDir);

    return outputDir;
    }

BeFileName iModelHubHost::GetDgnPlatformAssetsDirectory()
    {
    BeFileName assetsRootDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsRootDirectory);
    return assetsRootDirectory;
    }

BeFileName iModelHubHost::BuildDbFileName(Utf8StringCR baseName)
    {
    WString wFileName;
    BeStringUtilities::Utf8ToWChar(wFileName, (baseName + ".bim").c_str());
    BeFileName projectFileName = GetOutputDirectory();
    projectFileName.AppendToPath(wFileName.c_str());
    return projectFileName;
    }

DgnDbPtr iModelHubHost::CreateTestDb(Utf8StringCR name)
    {
    DgnDbPtr db;
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(true);
    createProjectParams.SetRootSubjectName(name.c_str());
    createProjectParams.SetRootSubjectDescription((name + " is created by iModelHubHost").c_str());
    db = DgnDb::CreateDgnDb(nullptr, BuildDbFileName(name), createProjectParams);
    BeAssert(db.IsValid());
    return db;
    }

iModelHubHostImpl::iModelHubHostImpl() : m_isInitialized(false)
    {
    BeAssert((DgnPlatformLib::QueryHost() == NULL) && L"This means an old host is still registered. You should have terminated it first before creating a new host.");
    DgnPlatformLib::Initialize(*this);
    m_isInitialized = true;
    }

iModelHubHostImpl::~iModelHubHostImpl()
    {
    if (m_isInitialized)
        {
        Terminate(false);
        }
    }
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
