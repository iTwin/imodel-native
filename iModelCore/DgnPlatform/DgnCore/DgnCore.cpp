/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <ECDb/ECDb.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/DgnECSymbolProvider.h>
#include <BRepCore/PSolidUtil.h>

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
        case ThreadId::IoPool:      return L"IoPool";
        case ThreadId::CpuPool:     return L"CpuPool";
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
DgnPlatformLib::Host::ExceptionHandler& DgnPlatformLib::Host::_SupplyExceptionHandler() {return *new ExceptionHandler();}

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
    static size_t s_highestKey = 0;
    BeAssert(key.m_key >= 0 && key.m_key<=s_highestKey); // make sure we're given a valid key

    if (0 == key.m_key)
        {
        BeSystemMutexHolder lock;

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

static DgnPlatformLib::Host* s_host = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformLib::Host* DgnPlatformLib::QueryHost() {return s_host;}
DgnPlatformLib::Host& DgnPlatformLib::GetHost() {return *s_host;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::StaticInitialize()
    {
    BeSystemMutexHolder holdBeSystemMutexInScope;

    static bool s_staticInitialized = false;
    if (s_staticInitialized)
        return;

    bentleyAllocator_enableLowFragmentationCRTHeap();
    s_staticInitialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/08
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Initialize(Host& host)
    {
    StaticInitialize();

    if (nullptr != s_host)
        {
        BeAssert(false && "only call DgnPlatformLib::Initialize once");
        return;
        }

    s_host = &host;

    DgnDb::SetThreadId(DgnDb::ThreadId::Client);

    host.Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* *Private* method called by Dgn::Host::Initialize.
* @bsimethod                                    Keith.Bentley                   09/08
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::Initialize()
    {
    BeAssert(NULL == m_knownLocationsAdmin); m_knownLocationsAdmin = &_SupplyIKnownLocationsAdmin();
    BeAssert(NULL == m_exceptionHandler); m_exceptionHandler = &_SupplyExceptionHandler();
    BeAssert(NULL == m_geoCoordAdmin); m_geoCoordAdmin = &_SupplyGeoCoordinationAdmin();

    auto assetDir = m_knownLocationsAdmin->GetDgnPlatformAssetsDirectory();

    BeStringUtilities::Initialize(assetDir);
    ECDb::Initialize(m_knownLocationsAdmin->GetLocalTempDirectoryBaseName(),
                      &assetDir,
                       BeSQLiteLib::LogErrors::No);
    L10N::Initialize(_SupplySqlangFiles());

    GeoCoordinates::BaseGCS::Initialize(GetGeoCoordinationAdmin()._GetDataDirectory().c_str());

    DgnDomains::RegisterDomain(BisCoreDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    DgnDomains::RegisterDomain(GenericDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);

    _SupplyProductName(m_productName);

    BeAssert(NULL == m_txnAdmin); m_txnAdmin = &_SupplyTxnAdmin();

    // ECSchemaReadContext::GetStandardPaths will append ECSchemas/ for us.
    ECN::ECSchemaReadContext::Initialize(assetDir);

    // Register Symbol Provider for ECExpressions
    IECSymbolProvider::RegisterExternalSymbolPublisher(&DgnECSymbolProvider::ExternalSymbolPublisher);

    BeAssert(NULL == m_fontAdmin);             m_fontAdmin             = &_SupplyFontAdmin();
    BeAssert(NULL == m_lineStyleAdmin);        m_lineStyleAdmin        = &_SupplyLineStyleAdmin();
    BeAssert(NULL == m_pointCloudAdmin);       m_pointCloudAdmin       = &_SupplyPointCloudAdmin();
    BeAssert(NULL == m_repositoryAdmin);       m_repositoryAdmin       = &_SupplyRepositoryAdmin();

    m_fontAdmin->Initialize();

    auto tempDirBase = m_knownLocationsAdmin->GetLocalTempDirectoryBaseName();
    PSolidKernelManager::Initialize(assetDir, tempDirBase);
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

    for (ObjEntry& obj : m_hostObj)
        ON_HOST_TERMINATE(obj.m_val, onProgramExit);

    m_hostObj.clear();
    m_hostVar.clear();

    ON_HOST_TERMINATE(m_exceptionHandler, onProgramExit);
    ON_HOST_TERMINATE(m_knownLocationsAdmin, onProgramExit);

    s_host = nullptr;

    BeAssert(NULL == m_fontAdmin);
    BeAssert(NULL == m_lineStyleAdmin);
    BeAssert(NULL == m_pointCloudAdmin);
    BeAssert(NULL == m_geoCoordAdmin);
    BeAssert(NULL == m_txnAdmin);
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
DgnPlatformLib::Host::FontAdmin&            DgnPlatformLib::Host::_SupplyFontAdmin()            {return *new FontAdmin();}
DgnPlatformLib::Host::LineStyleAdmin&       DgnPlatformLib::Host::_SupplyLineStyleAdmin()       {return *new LineStyleAdmin();}
DgnPlatformLib::Host::TxnAdmin&             DgnPlatformLib::Host::_SupplyTxnAdmin()             {return *new TxnAdmin();}
DgnPlatformLib::Host::RepositoryAdmin&      DgnPlatformLib::Host::_SupplyRepositoryAdmin()      {return *new RepositoryAdmin();}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      10/2014
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::GeoCoordinationAdmin& DgnPlatformLib::Host::_SupplyGeoCoordinationAdmin()
    {
    BeFileName geo = GetIKnownLocationsAdmin().GetGeoCoordinateDataDirectory();
    BeFileName path(geo);
    path.AppendToPath(L"DgnGeoCoord");
    if (!path.DoesPathExist())
        path = geo;
    return *DgnGeoCoordinationAdmin::Create(path);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      07/14
//---------------------------------------------------------------------------------------
void DgnPlatformLib::ForwardAssertionFailures(BeAssertFunctions::T_BeAssertHandler* h)
    {
    BeAssertFunctions::SetBeAssertHandler(h);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnPlatformLib::Host::_IsFeatureEnabled(Utf8CP featureName)
    {
    // Applications must opt-in to gated features
    return false;
    }

void DgnProgressMeter::AddTasks(uint32_t numTasksToAdd) {_AddTasks(numTasksToAdd);}
void DgnProgressMeter::AddSteps(uint32_t numSteps) {_AddSteps(numSteps);}
void DgnProgressMeter::SetCurrentStepName(Utf8CP newName) {_SetCurrentStepName(newName);}
void DgnProgressMeter::SetCurrentTaskName(Utf8CP newName) {_SetCurrentTaskName(newName);}
DgnProgressMeter::Abort DgnProgressMeter::ShowProgress() {return _ShowProgress();}
void DgnProgressMeter::Hide() {_Hide();}

static const wchar_t s_spinner[] = L" /-\\|";
static const size_t s_spinnerSize = _countof(s_spinner)-1;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool PrintfProgressMeter::HasDescription() const
    {
    return m_taskName.find(':') != Utf8String::npos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProgressMeter::Abort PrintfProgressMeter::_ShowProgress()
    {
    if (m_aborted)
        return ABORT_Yes;

    auto now = BeTimeUtilities::QuerySecondsCounter();

    if ((now - m_timeOfLastSpinnerUpdate) < 0.25) // don't do printf's more than a few times per second -- too slow and not useful
        return ABORT_No;

    m_timeOfLastSpinnerUpdate = now;

    m_spinCount++;

    bool justShowSpinner = false;

    if ((now - m_timeOfLastUpdate) < 0.5)
        justShowSpinner = true;         // don't push out full messages more than a couple times per second -- too slow and not useful
    else
        justShowSpinner = (FmtMessage() == m_lastMessage);

    if (justShowSpinner)
        {
        printf("[%c]\r", s_spinner[m_spinCount%s_spinnerSize]);
        return ABORT_No;
        }

    ForceNextUpdateToDisplay();
    UpdateDisplay();
    return ABORT_No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void PrintfProgressMeter::_Hide()
    {
    Utf8PrintfString msg("    %-123.123s %-16.16s", "", "");
    printf("%s\r", msg.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void PrintfProgressMeter::UpdateDisplay0(Utf8StringCR msgLeft)
    {
    m_lastMessage = msgLeft;

    // Display the number of tasks remaining. Not all major tasks have a task count.
    Utf8String tbd;
    if (m_stepsRemaining || m_tasksRemaining)
        tbd = Utf8PrintfString(":%d/%d", m_stepsRemaining, m_tasksRemaining);

    // Display the spinner and the task.
    Utf8PrintfString msg("[%c] %-123.123s %-16.16s", s_spinner[m_spinCount%s_spinnerSize], msgLeft.c_str(), tbd.c_str());
    printf("%s\r", msg.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PrintfProgressMeter::FmtMessage() const
    {
    Utf8String msg(m_stepName);
    msg.append(": ");
    msg.append(m_taskName);
    return msg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void PrintfProgressMeter::UpdateDisplay()
    {
    auto now = BeTimeUtilities::QuerySecondsCounter();

    if ((now - m_timeOfLastUpdate) < 1.0)
        return;

    m_timeOfLastUpdate = now;

    UpdateDisplay0(FmtMessage());
    }

/*---------------------------------------------------------------------------------**//**
* Sets the current task name, within the current step. Also indicates that the previous task was complete.
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void PrintfProgressMeter::_SetCurrentTaskName(Utf8CP newName)
    {
    T_Super::_SetCurrentTaskName(newName); // decrements task count

    if (newName && m_taskName == newName)
        return;

    m_taskName = newName? newName: "";
    m_spinCount=0;
    ForceNextUpdateToDisplay();
    UpdateDisplay();
    }

/*---------------------------------------------------------------------------------**//**
* Sets the current task name, within the current step. Also indicates that the previous task was complete.
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void PrintfProgressMeter::_SetCurrentStepName(Utf8CP stepName)
    {
    T_Super::_SetCurrentStepName(stepName); // decrements step count

    if (NULL == stepName)
        {
        m_stepName.clear();
        return;
        }
    if (m_stepName.Equals(stepName))
        return;

    m_stepName = stepName;
    m_taskName.clear();
    m_spinCount=0;
    ForceNextUpdateToDisplay();
    UpdateDisplay();
    }


