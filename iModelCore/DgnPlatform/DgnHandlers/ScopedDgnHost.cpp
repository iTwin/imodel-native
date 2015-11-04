/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnHandlers/ScopedDgnHost.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    bool    _GetLocalLineStylePaths     (WStringR str) override
        {
        BeFileName path;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory (path);
        str = path.GetName();
        return true;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LoggingNotificationAdmin : DgnPlatformLib::Host::NotificationAdmin
{
    virtual StatusInt _OutputMessage (NotifyMessageDetails const& msg) override
        {
        NativeLogging::LoggingManager::GetLogger(L"NOTIFICATION-ADMIN")->warningv ("MESSAGE: %s %s\n", msg.GetBriefMsg().c_str(), msg.GetDetailedMsg().c_str());
        return SUCCESS;
        }

    virtual NotificationManager::MessageBoxValue _OpenMessageBox (NotificationManager::MessageBoxType t, Utf8CP msg, NotificationManager::MessageBoxIconType iconType) override
        {
        NativeLogging::LoggingManager::GetLogger(L"NOTIFICATION-ADMIN")->warningv ("MESSAGEBOX: %s\n", msg);
        printf ("<<NOTIFICATION MessageBox: %s >>\n", msg);
        return NotificationManager::MESSAGEBOX_VALUE_Ok;
        }

    virtual void      _OutputPrompt (Utf8CP msg) override
        { // Log this as an error because we cannot prompt while running a unit test!
        NativeLogging::LoggingManager::GetLogger(L"NOTIFICATION-ADMIN")->errorv ("PROMPT (IGNORED): %s\n", msg);
        }

    virtual bool _GetLogSQLiteErrors() override 
        {
        return NativeLogging::LoggingManager::GetLogger("BeSQLite")->isSeverityEnabled (NativeLogging::LOG_INFO);
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
        BeTest::GetHost ().GetTempDir (m_tmp);
        BeTest::GetHost().GetDgnPlatformAssetsDirectory (m_appDir);
        }

    virtual BeFileNameCR _GetLocalTempDirectoryBaseName () override {return m_tmp;}
    virtual BeFileNameCR _GetDgnPlatformAssetsDirectory () override {return m_appDir;}
    };

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      07/15
//=======================================================================================
struct TestingDgnScriptingAdmin : Dgn::DgnPlatformLib::Host::ScriptAdmin
{
    ScopedDgnHost::FetchScriptCallback* m_callback;

    TestingDgnScriptingAdmin() {m_callback=nullptr;}

    DgnDbStatus _FetchScript(Utf8StringR sText, DgnScriptType& stypeFound, DgnDbR db, Utf8CP sName, DgnScriptType stypePreferred) override
        {
        if (nullptr == m_callback)
            return DgnDbStatus::NotEnabled;
        return m_callback->_FetchScript(sText, stypeFound, db, sName, stypePreferred);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProxyLocksAdmin : Dgn::DgnPlatformLib::Host::LocksAdmin
{
    DEFINE_T_SUPER(LocksAdmin);

    LocksAdmin* m_impl;

    ProxyLocksAdmin() : m_impl(nullptr) { }
    virtual ILocksManagerPtr _CreateLocksManager(DgnDbR db) const override
        {
        return nullptr != m_impl ? m_impl->_CreateLocksManager(db) : T_Super::_CreateLocksManager(db);
        }
    virtual ILocksServerP _GetLocksServer(DgnDbR db) const override
        {
        return nullptr != m_impl ? m_impl->_GetLocksServer(db) : T_Super::_GetLocksServer(db);
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
    NotificationAdmin& _SupplyNotificationAdmin () override;
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;
    ScriptAdmin& _SupplyScriptingAdmin() override {return *new TestingDgnScriptingAdmin();}
    LocksAdmin& _SupplyLocksAdmin() override {return *new ProxyLocksAdmin();}
    void _SupplyProductName(Utf8StringR s) override {s="BeTest";}
    L10N::SqlangFiles _SupplySqlangFiles() override {return L10N::SqlangFiles(BeFileName());} // users must have already initialized L10N to use ScopedDgnHost

    void SetFetchScriptCallback(ScopedDgnHost::FetchScriptCallback* cb) {((TestingDgnScriptingAdmin*)m_scriptingAdmin)->m_callback = cb;}
    void SetLocksAdmin(DgnPlatformLib::Host::LocksAdmin* admin) {((ProxyLocksAdmin*)m_locksAdmin)->m_impl = admin;}
};
END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
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
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ScopedDgnHost::SetLocksAdmin(DgnPlatformLib::Host::LocksAdmin* admin)
    {
    m_pimpl->SetLocksAdmin(admin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedDgnHostImpl::ScopedDgnHostImpl()  : m_isInitialized (false)
    {
    DgnPlatformLib::StaticInitialize();
    
    BeAssert ((DgnPlatformLib::QueryHost() == NULL) && L"This means an old host is still registered. You should have terminated it first before creating a new host.");

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
DgnPlatformLib::Host::NotificationAdmin&    ScopedDgnHostImpl::_SupplyNotificationAdmin () {return *new LoggingNotificationAdmin();}
DgnPlatformLib::Host::IKnownLocationsAdmin& ScopedDgnHostImpl::_SupplyIKnownLocationsAdmin() {return *new TestingConfigurationAdmin();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt TestDataManager::FindTestData (BeFileName& fullFileName, WCharCP fileName, BeFileName const& searchLeafDir)
    {
    BeFileName dir;
    BeTest::GetHost().GetDocumentsRoot (dir);
    dir.AppendToPath (searchLeafDir);

    for (;;)
        {
        fullFileName = dir;
        fullFileName.AppendToPath (fileName);
        if (BeFileName::DoesPathExist (fullFileName))
            return SUCCESS;

        dir.PopDir();
        if (!*dir.GetName())
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/07
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnModelPtr getAndFill(DgnDbR db, DgnModelId modelID, bool fillCache)
    {
    DgnModelPtr dgnModel = db.Models().GetModel (modelID);
    if (dgnModel == NULL)
        return NULL;

    if (fillCache)
        dgnModel->FillModel();

    return  dgnModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TestDataManager::OpenTestFile ()
    {
    DbResult stat;
    DgnDb::OpenParams params(m_openMode);
    m_dgndb = DgnDb::OpenDgnDb(&stat, BeFileName(m_fileName), params);
    if (m_dgndb == NULL)
        {
        if (stat == BE_SQLITE_ERROR_ProfileTooOld || stat == BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite)
            {
            NativeLogging::LoggingManager::GetLogger (L"BeTest")->errorv (L"HORNSWAGGLED! \"%ls\"", m_fileName.c_str());
            BeAssert (false && "HORNSWAGGLED!");
            }
        else
            {
            NativeLogging::LoggingManager::GetLogger (L"BeTest")->errorv (L"failed to open project fullFileName=\"%ls\" (status=%x)", m_fileName.c_str(), stat);
            BeAssert (false && "failed to open test input project file");
            }
        return ERROR;
        }


    for (auto const& entry : m_dgndb->Models().MakeIterator())
        {
        DgnModelPtr dgnModel = getAndFill(*m_dgndb, entry.GetModelId(), m_fill);
        if (m_model == NULL)
            m_model = dgnModel.get();
        }

    if (NULL == m_model)
        {
        NativeLogging::LoggingManager::GetLogger (L"BeTest")->errorv (L"failed to load any model from project \"%ls\"", m_fileName.c_str());
        BeAssert (false && "failed to load any model from test input project file");
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    TestDataManager::CloseTestFile ()
    {
    m_model = nullptr;
    m_dgndb = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TestDataManager::TestDataManager (WCharCP fullFileName, Db::OpenMode dbOpenMode, bool fill)
    {
    m_model     = NULL;
    m_fileName  = fullFileName;
    m_openMode  = dbOpenMode;
    m_fill      = fill;

    OpenTestFile ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TestDataManager::~TestDataManager ()
    {
    m_dgndb = NULL;   // then release/close the project
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP TestDataManager::GetDgnModelP() const {return m_model;}

