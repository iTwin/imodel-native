/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCore.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <DgnPlatform/DgnMarkupProject.h>
#include <ECDb/ECDb.h>
#include <DgnPlatform/DgnGeoCoord.h>

BeThreadLocalStorage g_hostForThread;
BeThreadLocalStorage g_threadId;

double const fc_hugeVal = 1e37;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDb::ThreadId DgnDb::GetThreadId() {return (ThreadId) g_threadId.GetValueAsInteger();}
void DgnDb::SetThreadId(ThreadId id) {g_threadId.SetValueAsInteger((int) id);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP DgnDb::GetThreadIdName()
    {
    switch (GetThreadId())
        {
        case ThreadId::Client:      return L"ClientThread";
        case ThreadId::Render:      return L"RenderThread";
        case ThreadId::Query:       return L"QueryThread";
        case ThreadId::RealityData: return L"RealityData";
        default:                    return L"UnknownThread";
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/14
//---------------------------------------------------------------------------------------
void DgnPlatformLib::InitializeBentleyLogging(WCharCP configFileName)
    {
    if (configFileName != NULL && BeFileName::DoesPathExist(configFileName))
        {
        NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, configFileName);
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        }
    else
        {
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformLib::Host::ExceptionHandler& DgnPlatformLib::Host::_SupplyExceptionHandler() { return *new ExceptionHandler(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR DgnPlatformLib::Host::IKnownLocationsAdmin::GetLocalTempDirectoryBaseName() 
    {
    return _GetLocalTempDirectoryBaseName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnPlatformLib::Host::IKnownLocationsAdmin::GetLocalTempDirectory(BeFileNameR tempDir, WCharCP subDir)
    {
    tempDir = GetLocalTempDirectoryBaseName();
    
    if (NULL != subDir)
        {
        tempDir.AppendToPath(subDir);
        tempDir.AppendSeparator();
        }

    BeFileNameStatus status = BeFileName::CreateNewDirectory(tempDir);
    if (status != BeFileNameStatus::Success && status != BeFileNameStatus::AlreadyExists)
        return ERROR;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    06/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR DgnPlatformLib::Host::IKnownLocationsAdmin::GetDgnPlatformAssetsDirectory()
    {
    return _GetDgnPlatformAssetsDirectory();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DgnPlatformLib::Host::ExceptionHandler::_ResetFloatingPointExceptions(uint32_t newFpuMask)
    {
    return BeNumerical::ResetFloatingPointExceptions(newFpuMask);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                    04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::_OnAssert(WCharCP _Message, WCharCP _File, unsigned _Line)
    {
    GetExceptionHandler()._HandleAssert(_Message, _File, _Line);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnHost::GetKeyIndex(DgnHost::Key& key)
    {
    static size_t s_highestKey = 0; // MT: DgnCoreCriticalSection

    BeAssert(key.m_key >= 0 && key.m_key<=s_highestKey); // make sure we're given a valid key

    if (0 == key.m_key)
        {
        ___DGNPLATFORM_SERIALIZED___;

        if (0 == key.m_key)
            key.m_key = ++s_highestKey;
        }

    return key.m_key;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnHost::VarEntry& DgnHost::GetVarEntry(Key& key)
    {
    size_t keyIndex = GetKeyIndex(key);

    if (m_hostVar.size() < keyIndex+1)
        m_hostVar.resize(keyIndex+1, VarEntry());

    return m_hostVar[keyIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnHost::ObjEntry& DgnHost::GetObjEntry(Key& key)
    {
    size_t keyIndex = GetKeyIndex(key);

    if (m_hostObj.size() < keyIndex+1)
        m_hostObj.resize(keyIndex+1, ObjEntry());

    return m_hostObj[keyIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnHost::IHostObject* DgnHost::GetHostObject(Key& key)
    {
    return GetObjEntry(key).m_val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnHost::SetHostObject(Key& key, IHostObject* val)
    {
    GetObjEntry(key).m_val = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void* DgnHost::GetHostVariable(Key& key)
    {
    return GetVarEntry(key).m_val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnHost::SetHostVariable(Key& key, void* val)
    {
    GetVarEntry(key).m_val = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
void NotificationManager::OutputPrompt(Utf8CP prompt) {return T_HOST.GetNotificationAdmin()._OutputPrompt(prompt);}
StatusInt NotificationManager::OutputMessage(NotifyMessageDetails const& details) {return T_HOST.GetNotificationAdmin()._OutputMessage(details);}
void DgnPlatformLib::Host::NotificationAdmin::ChangeAdmin(NotificationAdmin& newAdmin){T_HOST.ChangeNotificationAdmin(newAdmin);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
NotificationManager::MessageBoxValue NotificationManager::OpenMessageBox(MessageBoxType mbType, Utf8CP message, MessageBoxIconType icon)
    {
    return T_HOST.GetNotificationAdmin()._OpenMessageBox(mbType, message, icon);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::AdoptHost(DgnPlatformLib::Host& host)
    {
    g_hostForThread.SetValueAsPointer(&host);
    DgnDb::SetThreadId(DgnDb::ThreadId::Client);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformLib::Host* DgnPlatformLib::QueryHost()
    {
    return static_cast<Host*>(g_hostForThread.GetValueAsPointer());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::ForgetHost()
    {
    g_hostForThread.SetValueAsPointer(NULL);
    }

/*---------------------------------------------------------------------------------**//**
* *Private* method called by Dgn::Host::Initialize.
* @bsimethod                                    Keith.Bentley                   09/08
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::InitializeDgnCore()
    {
    // NB: Assume that the caller has checked that we are not initializing the same thread twice.

    if (NULL != DgnPlatformLib::QueryHost())
        {
        BeAssert(false); // called more than once on the same thread
        return;
        }

    BeAssert(NULL == m_knownLocationsAdmin); m_knownLocationsAdmin = &_SupplyIKnownLocationsAdmin();
    BeAssert(NULL == m_exceptionHandler); m_exceptionHandler = &_SupplyExceptionHandler();
    // establish the NotificationAdmin first, in case other _Supply methods generate errors
    BeAssert(NULL == m_notificationAdmin); m_notificationAdmin = &_SupplyNotificationAdmin();
    BeAssert (NULL == m_geoCoordAdmin); m_geoCoordAdmin = &_SupplyGeoCoordinationAdmin();

    BeStringUtilities::Initialize(m_knownLocationsAdmin->GetDgnPlatformAssetsDirectory());
    ECDb::Initialize(m_knownLocationsAdmin->GetLocalTempDirectoryBaseName(),
                      &m_knownLocationsAdmin->GetDgnPlatformAssetsDirectory(),
                      m_notificationAdmin->_GetLogSQLiteErrors() ? BeSQLiteLib::LogErrors::Yes : BeSQLiteLib::LogErrors::No);
    L10N::Initialize(_SupplySqlangFiles());

    GeoCoordinates::BaseGCS::Initialize (GetGeoCoordinationAdmin()._GetDataDirectory().c_str());

    AdoptHost(*this);
    BeAssert(NULL != DgnPlatformLib::QueryHost());

    DgnDomains::RegisterDomain(BisCoreDomain::GetDomain());
    DgnDomains::RegisterDomain(GenericDomain::GetDomain());

    _SupplyProductName(m_productName);

    m_acsManager = new IACSManager();

    BeAssert(NULL == m_txnAdmin); m_txnAdmin = &_SupplyTxnAdmin();

    // ECSchemaReadContext::GetStandardPaths will append ECSchemas/ for us.
    ECN::ECSchemaReadContext::Initialize(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TerminateDgnCore(bool onProgramExit)
    {
    if (NULL == DgnPlatformLib::QueryHost())
        {
        BeAssert(false);// && "Terminate called on a thread that is not associated with a host");
        return;
        }

    ON_HOST_TERMINATE(m_txnAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_acsManager, onProgramExit);

    for (ObjEntry& obj : m_hostObj)
        ON_HOST_TERMINATE(obj.m_val, onProgramExit);

    m_hostObj.clear();
    m_hostVar.clear();

    ON_HOST_TERMINATE(m_exceptionHandler, onProgramExit);
    ON_HOST_TERMINATE(m_knownLocationsAdmin, onProgramExit);

    ForgetHost();
    BeAssert(NULL == DgnPlatformLib::QueryHost());

    BeAssert(NULL == m_fontAdmin);
    BeAssert(NULL == m_lineStyleAdmin);
    BeAssert(NULL == m_rasterAttachmentAdmin);
    BeAssert(NULL == m_pointCloudAdmin);
    BeAssert(NULL == m_notificationAdmin);
    BeAssert(NULL == m_geoCoordAdmin);
    BeAssert(NULL == m_txnAdmin);
    BeAssert(NULL == m_acsManager);
    BeAssert(NULL == m_formatterAdmin);
    BeAssert(NULL == m_scriptingAdmin);
    BeAssert(NULL == m_exceptionHandler);
    BeAssert(NULL == m_knownLocationsAdmin);
    BeAssert(NULL == m_repositoryAdmin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnPlatformLib::Host::LineStyleAdmin::_GetLocalLinFilePaths(WStringR paths)
    {
#ifdef WIP_CFGVAR // MS_LINFILELIST
    return SUCCESS == ConfigurationManager::GetVariable(paths, L"MS_LINFILELIST");
#endif
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnPlatformLib::Host::LineStyleAdmin::_GetLocalLineStylePaths(WStringR paths)
    {
#ifdef WIP_CFGVAR // MS_SYMBRSRC, MS_LINESTYLEPATH
    ConfigurationManager::GetVariable(paths, L"MS_SYMBRSRC");

    WString lspath;
    if (SUCCESS == ConfigurationManager::GetVariable(lspath, L"MS_LINESTYLEPATH"))
        {
        if (0 != paths.length())
            paths.append(L";");
        paths.append(lspath);
        }

    return 0 != paths.length();
#endif
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformLib::Host::FontAdmin&             DgnPlatformLib::Host::_SupplyFontAdmin()             {return *new FontAdmin();}
DgnPlatformLib::Host::NotificationAdmin&     DgnPlatformLib::Host::_SupplyNotificationAdmin()     {return *new NotificationAdmin();}
DgnPlatformLib::Host::LineStyleAdmin&        DgnPlatformLib::Host::_SupplyLineStyleAdmin()        {return *new LineStyleAdmin();}
DgnPlatformLib::Host::TxnAdmin& DgnPlatformLib::Host::_SupplyTxnAdmin() {return *new TxnAdmin();}
DgnPlatformLib::Host::FormatterAdmin&        DgnPlatformLib::Host::_SupplyFormatterAdmin()        {return *new FormatterAdmin();}
DgnPlatformLib::Host::ScriptAdmin&           DgnPlatformLib::Host::_SupplyScriptingAdmin()        {return *new ScriptAdmin();}
DgnPlatformLib::Host::RepositoryAdmin&       DgnPlatformLib::Host::_SupplyRepositoryAdmin()       {return *new RepositoryAdmin();}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      10/2014
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::GeoCoordinationAdmin& DgnPlatformLib::Host::_SupplyGeoCoordinationAdmin()
    {
    BeFileName path = GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    path.AppendToPath (L"DgnGeoCoord");
    return *DgnGeoCoordinationAdmin::Create (path);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      07/14
//---------------------------------------------------------------------------------------
void DgnPlatformLib::ForwardAssertionFailures(BeAssertFunctions::T_BeAssertHandler* h)
    {
    BeAssertFunctions::SetBeAssertHandler(h);
    }

void DgnProgressMeter::AddTasks(uint32_t numTasksToAdd) {_AddTasks(numTasksToAdd);}
void DgnProgressMeter::AddSteps(uint32_t numSteps) {_AddSteps(numSteps);}
void DgnProgressMeter::SetCurrentStepName(Utf8CP newName) {_SetCurrentStepName(newName);}
void DgnProgressMeter::SetCurrentTaskName(Utf8CP newName) {_SetCurrentTaskName(newName);}
DgnProgressMeter::Abort DgnProgressMeter::ShowProgress() {return _ShowProgress();}
void DgnProgressMeter::Hide() {_Hide();}

DEFINE_KEY_METHOD(DgnMarkupProject)

