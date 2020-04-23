/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Bentley/BeTest.h>
#include <Logging/bentleylogging.h>
#include <Bentley/BeDirectoryIterator.h>

#include <DgnPlatform/UnitTests/ScopedDgnHost.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitTestLineStyleAdmin : public DgnPlatformLib::Host::LineStyleAdmin
    {
    bool    _GetLocalLineStylePaths(WStringR str) override
        {
        BeFileName path;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(path);
        str = path.GetName();
        return true;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestingConfigurationAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
    {
    BeFileName  m_tmp;
    BeFileName  m_appDir;

    TestingConfigurationAdmin()
        {
        BeTest::GetHost().GetTempDir(m_tmp);
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(m_appDir);
        }

    BeFileNameCR _GetLocalTempDirectoryBaseName() override {return m_tmp;}
    BeFileNameCR _GetDgnPlatformAssetsDirectory() override {return m_appDir;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProxyRepositoryAdmin : Dgn::DgnPlatformLib::Host::RepositoryAdmin
{
    DEFINE_T_SUPER(RepositoryAdmin);

    RepositoryAdmin* m_impl;

    ProxyRepositoryAdmin() : m_impl(nullptr) {}
    IBriefcaseManagerPtr _CreateBriefcaseManager(DgnDbR db) const override
        {
        return nullptr != m_impl ? m_impl->_CreateBriefcaseManager(db) : T_Super::_CreateBriefcaseManager(db);
        }
    IRepositoryManagerP _GetRepositoryManager(DgnDbR db) const override
        {
        return nullptr != m_impl ? m_impl->_GetRepositoryManager(db) : T_Super::_GetRepositoryManager(db);
        }
};

/*---------------------------------------------------------------------------------**//**
* Here is the real implementation of ScopeDgnHost. Registers itself as a DgnHost in its
* constructor. Supplies key admins that direct DgnPlatform to the files in the
* directories delivered with the unit test framework.
* @bsiclass                                     Sam.Wilson                      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BEGIN_BENTLEY_DGN_NAMESPACE
struct ScopedDgnHostImpl : DgnPlatformLib::Host
{
    bool        m_isInitialized;

    ScopedDgnHostImpl();
    ~ScopedDgnHostImpl();
    LineStyleAdmin& _SupplyLineStyleAdmin() override;
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;
    RepositoryAdmin& _SupplyRepositoryAdmin() override {return *new ProxyRepositoryAdmin();}
    void _SupplyProductName(Utf8StringR s) override {s="BeTest";}
    L10N::SqlangFiles _SupplySqlangFiles() override {return L10N::SqlangFiles(BeFileName());} // users must have already initialized L10N to use ScopedDgnHost

    void SetRepositoryAdmin(DgnPlatformLib::Host::RepositoryAdmin* admin) {((ProxyRepositoryAdmin*)m_repositoryAdmin)->m_impl = admin;}
    DgnPlatformLib::Host::RepositoryAdmin* GetRepositoryAdmin() {return ((ProxyRepositoryAdmin*)m_repositoryAdmin)->m_impl;}
};
END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedDgnHost::ScopedDgnHost()
    {
    m_pimpl = new ScopedDgnHostImpl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedDgnHost::~ScopedDgnHost()
    {
    delete m_pimpl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ScopedDgnHost::SetRepositoryAdmin(DgnPlatformLib::Host::RepositoryAdmin* admin)
    {
    m_pimpl->SetRepositoryAdmin(admin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformLib::Host::RepositoryAdmin* ScopedDgnHost::GetRepositoryAdmin()
    {
    return m_pimpl->GetRepositoryAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedDgnHostImpl::ScopedDgnHostImpl()  : m_isInitialized(false)
    {
    DgnPlatformLib::StaticInitialize();

    BeAssert((DgnPlatformLib::QueryHost() == NULL) && L"This means an old host is still registered. You should have terminated it first before creating a new host.");

    DgnPlatformLib::Initialize(*this);
    m_isInitialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedDgnHostImpl::~ScopedDgnHostImpl()
    {
    if (m_isInitialized)
        Terminate(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     02/13
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::LineStyleAdmin&       ScopedDgnHostImpl::_SupplyLineStyleAdmin() {return *new UnitTestLineStyleAdmin();}
DgnPlatformLib::Host::IKnownLocationsAdmin& ScopedDgnHostImpl::_SupplyIKnownLocationsAdmin() {return *new TestingConfigurationAdmin();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt TestDataManager::FindTestData(BeFileName& fullFileName, WCharCP fileName, BeFileName const& searchLeafDir)
    {
    BeFileName dir;
    BeTest::GetHost().GetDocumentsRoot(dir);
    dir.AppendToPath(searchLeafDir);

    for (;;)
        {
        fullFileName = dir;
        fullFileName.AppendToPath(fileName);
        if (BeFileName::DoesPathExist(fullFileName))
            return SUCCESS;

        dir.PopDir();
        if (!*dir.GetName())
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestDataManager::SetAsStandaloneDb(DgnDbPtr& db, DgnDb::OpenMode openMode) {
    if (db->IsWriteableStandalone() || openMode == DgnDb::OpenMode::Readonly && db->IsStandalone())
        return;

    if (openMode == DgnDb::OpenMode::ReadWrite) {
        auto val = Json::Value();
        val["txns"] = true;
        db->SaveStandaloneEditFlags(val);
    }

    db->ResetBriefcaseId(BeBriefcaseId(BeBriefcaseId::Standalone()));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TestDataManager::OpenTestFile(bool needTxns)
    {
    DbResult stat;
    DgnDb::OpenParams params(m_openMode);
    m_dgndb = DgnDb::OpenDgnDb(&stat, BeFileName(m_fileName), params);
    if (m_dgndb == NULL)
        {
        if (stat == BE_SQLITE_ERROR_ProfileTooOld)
            {
            NativeLogging::LoggingManager::GetLogger(L"BeTest")->errorv(L"HORNSWAGGLED! \"%ls\"", m_fileName.c_str());
            BeAssert(false && "HORNSWAGGLED!");
            }
        else
            {
            NativeLogging::LoggingManager::GetLogger(L"BeTest")->errorv(L"failed to open project fullFileName=\"%ls\" (status=%x)", m_fileName.c_str(), stat);
            BeAssert(false && "failed to open test input project file");
            }
        return ERROR;
        }

    if (needTxns)
        SetAsStandaloneDb(m_dgndb, m_openMode);

    for (ModelIteratorEntryCR entry : m_dgndb->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_Model)))
        {
        DgnModelPtr dgnModel = m_dgndb->Models().GetModel(entry.GetModelId());
        if (m_model == NULL)
            m_model = dgnModel.get();
        }

    if (NULL == m_model)
        {
        NativeLogging::LoggingManager::GetLogger(L"BeTest")->errorv(L"failed to load any model from project \"%ls\"", m_fileName.c_str());
        BeAssert(false && "failed to load any model from test input project file");
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    TestDataManager::CloseTestFile()
    {
    m_model = nullptr;
    m_dgndb = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TestDataManager::TestDataManager(WCharCP fullFileName, Db::OpenMode dbOpenMode, bool needTxns, bool fill)
    {
    m_model     = NULL;
    m_fileName  = fullFileName;
    m_openMode  = dbOpenMode;
    m_fill      = fill;

    OpenTestFile(needTxns);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TestDataManager::~TestDataManager()
    {
    m_dgndb = NULL;   // then release/close the project
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP TestDataManager::GetDgnModelP() const {return m_model;}

