/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnHandlers/ScopedDgnHost.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
struct LoggingNotificationAdmin : DgnPlatformLib::Host::NotificationAdmin
{
    StatusInt _OutputMessage(NotifyMessageDetails const& msg) override
        {
        NativeLogging::LoggingManager::GetLogger(L"NOTIFICATION-ADMIN")->warningv("MESSAGE: %s %s\n", msg.GetBriefMsg().c_str(), msg.GetDetailedMsg().c_str());
        return SUCCESS;
        }

    NotificationManager::MessageBoxValue _OpenMessageBox(NotificationManager::MessageBoxType t, Utf8CP msg, NotificationManager::MessageBoxIconType iconType) override
        {
        NativeLogging::LoggingManager::GetLogger(L"NOTIFICATION-ADMIN")->warningv("MESSAGEBOX: %s\n", msg);
        printf("<<NOTIFICATION MessageBox: %s >>\n", msg);
        return NotificationManager::MESSAGEBOX_VALUE_Ok;
        }

    void      _OutputPrompt(Utf8CP msg) override
        {// Log this as an error because we cannot prompt while running a unit test!
        NativeLogging::LoggingManager::GetLogger(L"NOTIFICATION-ADMIN")->errorv("PROMPT (IGNORED): %s\n", msg);
        }

    bool _GetLogSQLiteErrors() override 
        {
        return NativeLogging::LoggingManager::GetLogger("BeSQLite")->isSeverityEnabled(NativeLogging::LOG_INFO);
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

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      07/15
//=======================================================================================
struct TestingDgnScriptingAdmin : Dgn::DgnPlatformLib::Host::ScriptAdmin
{
    ScopedDgnHost::FetchScriptCallback* m_callback;

    TestingDgnScriptingAdmin() {m_callback=nullptr;}

    DgnDbStatus _FetchScript(Utf8StringR sText, DgnScriptType& stypeFound, DateTime& lmt, DgnDbR db, Utf8CP sName, DgnScriptType stypePreferred) override
        {
        if (nullptr == m_callback)
            return DgnDbStatus::NotEnabled;
        return m_callback->_FetchScript(sText, stypeFound, lmt, db, sName, stypePreferred);
        }
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
    NotificationAdmin& _SupplyNotificationAdmin() override;
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;
    ScriptAdmin& _SupplyScriptingAdmin() override {return *new TestingDgnScriptingAdmin();}
    RepositoryAdmin& _SupplyRepositoryAdmin() override {return *new ProxyRepositoryAdmin();}
    void _SupplyProductName(Utf8StringR s) override {s="BeTest";}
    L10N::SqlangFiles _SupplySqlangFiles() override {return L10N::SqlangFiles(BeFileName());} // users must have already initialized L10N to use ScopedDgnHost

    void SetFetchScriptCallback(ScopedDgnHost::FetchScriptCallback* cb) {((TestingDgnScriptingAdmin*)m_scriptingAdmin)->m_callback = cb;}
    void SetCodeAdmin(DgnPlatformLib::Host::CodeAdmin* admin) {delete m_codeAdmin; m_codeAdmin = admin;}
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
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ScopedDgnHost::SetFetchScriptCallback(FetchScriptCallback* cb)
    {
    m_pimpl->SetFetchScriptCallback(cb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ScopedDgnHost::SetCodeAdmin(DgnPlatformLib::Host::CodeAdmin* admin)
    {
    m_pimpl->SetCodeAdmin(admin);
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

    DgnPlatformLib::Initialize(*this,true);
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
DgnPlatformLib::Host::NotificationAdmin&    ScopedDgnHostImpl::_SupplyNotificationAdmin() {return *new LoggingNotificationAdmin();}
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
void TestDataManager::MustBeBriefcase(DgnDbPtr& db, DgnDb::OpenMode mode)
    {
    if (db->IsBriefcase())
        return;

    BeFileName name(db->GetFileName());

    db->ChangeBriefcaseId(BeBriefcaseId(BeBriefcaseId::Standalone()));
    db->SaveChanges();
    db->CloseDb();

    DbResult result = BE_SQLITE_OK;
    db = DgnDb::OpenDgnDb(&result, name, DgnDb::OpenParams(mode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TestDataManager::OpenTestFile(bool needBriefcase)
    {
    DbResult stat;
    DgnDb::OpenParams params(m_openMode);
    m_dgndb = DgnDb::OpenDgnDb(&stat, BeFileName(m_fileName), params);
    if (m_dgndb == NULL)
        {
        if (stat == BE_SQLITE_ERROR_ProfileTooOld || stat == BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite)
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

    if (needBriefcase)
        MustBeBriefcase(m_dgndb, m_openMode);

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
TestDataManager::TestDataManager(WCharCP fullFileName, Db::OpenMode dbOpenMode, bool needBriefcase, bool fill)
    {
    m_model     = NULL;
    m_fileName  = fullFileName;
    m_openMode  = dbOpenMode;
    m_fill      = fill;

    OpenTestFile(needBriefcase);
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

