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

#include <DgnPlatform/DgnHandlers/UnitTests/ScopedDgnHost.h>

//=======================================================================================
// @bsiclass                                                    MattGooding     02/13
//=======================================================================================
struct UnitTestGeoCoordAdmin : DgnPlatformLib::Host::GeoCoordinationAdmin
{
public:
    virtual WString _GetDataDirectory () override
        {
        BeFileName path;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory (path);
        path.AppendToPath (L"GeoCoordData");
        return path.GetName();
        }
};

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

/*---------------------------------------------------------------------------------**//**
* Here is the real implementation of ScopeDgnHost. Registers itself as a DgnHost in its
* constructor. Supplies key admins that direct DgnPlatform to the files in the 
* directories delivered with the unit test framework.
* @bsiclass                                     Sam.Wilson                      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
struct ScopedDgnHostImpl : DgnPlatformLib::Host
{
    bool        m_isInitialized;

    ScopedDgnHostImpl();
    ~ScopedDgnHostImpl();
    virtual LineStyleAdmin& _SupplyLineStyleAdmin() override;
    virtual GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin() override;
    virtual NotificationAdmin& _SupplyNotificationAdmin () override;
    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;
    virtual void _SupplyProductName(Utf8StringR s) override {s="BeTest";}
    virtual L10N::SqlangFiles _SupplySqlangFiles() override {return L10N::SqlangFiles(BeFileName());} // users must have already initialized L10N to use ScopedDgnHost
};
END_BENTLEY_DGNPLATFORM_NAMESPACE

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
DgnPlatformLib::Host::GeoCoordinationAdmin& ScopedDgnHostImpl::_SupplyGeoCoordinationAdmin() {return *new UnitTestGeoCoordAdmin();}
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
static DgnModelP getAndFill(DgnDbR db, DgnModelId modelID, bool fillCache)
    {
    DgnModelP dgnModel = db.Models().GetModelById (modelID);
    if (dgnModel == NULL)
        return NULL;

    if (fillCache)
        dgnModel->FillModel();

    return  dgnModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TestDataManager::TestDataManager (WCharCP fullFileName, Db::OpenMode dbOpenMode, bool fill)
    {
    m_model = NULL;

    DbResult stat;
    DgnDb::OpenParams params(dbOpenMode);
    m_dgndb = DgnDb::OpenDgnDb(&stat, BeFileName(fullFileName), params);
    if (m_dgndb == NULL)
        {
        if (stat == BE_SQLITE_ERROR_ProfileTooOld || stat == BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite)
            {
            NativeLogging::LoggingManager::GetLogger (L"BeTest")->errorv (L"HORNSWAGGLED! \"%ls\"", fullFileName);
            BeAssert (false && "HORNSWAGGLED!");
            }
        else
            {
            NativeLogging::LoggingManager::GetLogger (L"BeTest")->errorv (L"failed to open project fullFileName=\"%ls\" (status=%x)", fullFileName, stat);
            BeAssert (false && "failed to open test input project file");
            }
        return;
        }


    for (auto const& entry : m_dgndb->Models().MakeIterator())
        {
        DgnModelP dgnModel = getAndFill(*m_dgndb, entry.GetModelId(), fill);
        dgnModel->SetReadOnly (Db::OPEN_Readonly == dbOpenMode);
        if (m_model == NULL)
            m_model = dgnModel;
        }

    if (NULL == m_model)
        {
        NativeLogging::LoggingManager::GetLogger (L"BeTest")->errorv (L"failed to load any model from project \"%ls\"", fullFileName);
        BeAssert (false && "failed to load any model from test input project file");
        return;
        }
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

