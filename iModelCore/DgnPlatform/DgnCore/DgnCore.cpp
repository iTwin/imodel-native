/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCore.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnProgressMeter.h>
#include <DgnPlatform/DgnCore/DgnMarkupProject.h>
#include <RmgrTools/RscMgr/rmgrsubs.h>
#include <BeSQLite/ECDb/ECDb.h>

USING_NAMESPACE_BENTLEY_SQLITE

BeThreadLocalStorage    g_hostForThread;

/*----------------------------------------------------------------------+
|                                                                       |
|  Element Type Masks - Can be used to set up type masks for scanner    |
|                       or element modification commands                |
|                                                                       |
+----------------------------------------------------------------------*/
// The following masks must be 'OR'ed with typemask[0]
#define ELMBITMSK(elmNum)       (1<<((elmNum-1)%16))
#define TMSK0_CELL_HEADER       ELMBITMSK (CELL_HEADER_ELM)
#define TMSK0_LINE              ELMBITMSK (LINE_ELM)
#define TMSK0_LINE_STRING       ELMBITMSK (LINE_STRING_ELM)
#define TMSK0_GROUP_DATA        ELMBITMSK (GROUP_DATA_ELM)
#define TMSK0_SHAPE             ELMBITMSK (SHAPE_ELM)
#define TMSK0_TEXT_NODE         ELMBITMSK (TEXT_NODE_ELM)
#define TMSK0_DIG_SETDATA       ELMBITMSK (DIG_SETDATA_ELM)
#define TMSK0_DGNFIL_HEADER     ELMBITMSK (DGNFIL_HEADER_ELM)
#define TMSK0_LEV_SYM           ELMBITMSK (LEV_SYM_ELM)
#define TMSK0_CURVE             ELMBITMSK (CURVE_ELM)
#define TMSK0_CMPLX_STRING      ELMBITMSK (CMPLX_STRING_ELM)
#define TMSK0_CONIC             ELMBITMSK (CONIC_ELM)
#define TMSK0_CMPLX_SHAPE       ELMBITMSK (CMPLX_SHAPE_ELM)
#define TMSK0_ELLIPSE           ELMBITMSK (ELLIPSE_ELM)
#define TMSK0_ARC               ELMBITMSK (ARC_ELM)

// These following masks must be 'OR'ed with typemask[1]
#define TMSK1_TEXT              ELMBITMSK (TEXT_ELM)
#define TMSK1_SURFACE           ELMBITMSK (SURFACE_ELM)
#define TMSK1_SOLID             ELMBITMSK (SOLID_ELM)
#define TMSK1_BSPLINE_POLE      ELMBITMSK (BSPLINE_POLE_ELM)
#define TMSK1_POINT_STRING      ELMBITMSK (POINT_STRING_ELM)
#define TMSK1_CONE              ELMBITMSK (CONE_ELM)
#define TMSK1_BSPLINE_SURFACE   ELMBITMSK (BSPLINE_SURFACE_ELM)
#define TMSK1_BSURF_BOUNDARY    ELMBITMSK (BSURF_BOUNDARY_ELM)
#define TMSK1_BSPLINE_KNOT      ELMBITMSK (BSPLINE_KNOT_ELM)
#define TMSK1_BSPLINE_CURVE     ELMBITMSK (BSPLINE_CURVE_ELM)
#define TMSK1_BSPLINE_WEIGHT    ELMBITMSK (BSPLINE_WEIGHT_ELM)

// These following masks must be 'OR'ed with typemask[2]
#define TMSK2_DIMENSION         ELMBITMSK (DIMENSION_ELM)
#define TMSK2_SHAREDCELL_DEF    ELMBITMSK (SHAREDCELL_DEF_ELM)
#define TMSK2_SHARED_CELL       ELMBITMSK (SHARED_CELL_ELM)
#define TMSK2_MULTILINE         ELMBITMSK (MULTILINE_ELM)
#define TMSK2_ATTRIBUTE         ELMBITMSK (ATTRIBUTE_ELM)
#define TMSK2_DGNSTORE_HDR      ELMBITMSK (DGNSTORE_HDR)

// These following masks must be 'OR'ed with typemask[4]
#define TMSK4_MICROSTATION_ELM  ELMBITMSK (MICROSTATION_ELM)

// These following masks must be 'OR'ed with typemask[5]
#define TMSK5_RASTER_HDR            ELMBITMSK (RASTER_HDR)
#define TMSK5_RASTER_COMP           ELMBITMSK (RASTER_COMP)
#define TMSK5_RASTER_LINK           ELMBITMSK (RASTER_REFERENCE_ELM)
#define TMSK5_RASTER_REFCMPN        ELMBITMSK (RASTER_REFERENCE_COMP)
#define TMSK5_RASTER_HIERARCHY      ELMBITMSK (RASTER_HIERARCHY_ELM)
#define TMSK5_RASTER_HIERARCHYCMPN  ELMBITMSK (RASTER_HIERARCHY_COMP)
#define TMSK5_RASTER_FRAME          ELMBITMSK (RASTER_FRAME_ELM)
#define TMSK5_TABLE_ENTRY           ELMBITMSK (TABLE_ENTRY_ELM)
#define TMSK5_TABLE                 ELMBITMSK (TABLE_ELM)

// These following masks must be 'OR'ed with typemask[6]
#define TMSK6_MATRIX_HEADER             ELMBITMSK (MATRIX_HEADER_ELM)
#define TMSK6_MATRIX_INT_DATA           ELMBITMSK (MATRIX_INT_DATA_ELM)
#define TMSK6_MATRIX_DOUBLE_DATA        ELMBITMSK (MATRIX_DOUBLE_DATA_ELM)
#define TMSK6_MESH_HEADER               ELMBITMSK (MESH_HEADER_ELM)
#define TMSK6_EXTENDED                  ELMBITMSK (EXTENDED_ELM)
#define TMSK6_EXTENDED_NONGRAPHIC_ELM   ELMBITMSK (EXTENDED_NONGRAPHIC_ELM)
#define TMSK6_VIEW_GROUP                ELMBITMSK (VIEW_GROUP_ELM)
#define TMSK6_VIEW                      ELMBITMSK (VIEW_ELM)

#if defined (NEEDS_WORK_DGNITEM)
UShort element_drawn[8] =
                                {
                                TMSK0_CELL_HEADER | TMSK0_LINE | TMSK0_LINE_STRING |
                                TMSK0_SHAPE | TMSK0_TEXT_NODE | TMSK0_CURVE |
                                TMSK0_CMPLX_STRING | TMSK0_CONIC | TMSK0_CMPLX_SHAPE |
                                TMSK0_ELLIPSE | TMSK0_ARC,

                                TMSK1_TEXT | TMSK1_SURFACE | TMSK1_SOLID |
                                TMSK1_BSPLINE_POLE | TMSK1_POINT_STRING | TMSK1_CONE |
                                TMSK1_BSPLINE_SURFACE | TMSK1_BSURF_BOUNDARY |
                                TMSK1_BSPLINE_KNOT | TMSK1_BSPLINE_CURVE |
                                TMSK1_BSPLINE_WEIGHT,

                                TMSK2_DIMENSION | TMSK2_SHARED_CELL |
                                TMSK2_MULTILINE,
                                0x0000,
                                0x0000,
                                TMSK5_RASTER_HDR | TMSK5_RASTER_COMP | TMSK5_RASTER_FRAME,
                                TMSK6_MESH_HEADER | TMSK6_EXTENDED ,
                                0x0000
                                };
#endif

double const fc_hugeVal      = 1e37;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/14
//---------------------------------------------------------------------------------------
void DgnPlatformLib::InitializeBentleyLogging (WCharCP configFileName)
    {
    if (configFileName != NULL && BeFileName::DoesPathExist (configFileName))
        {
        NativeLogging::LoggingConfig::SetOption (CONFIG_OPTION_CONFIG_FILE, configFileName);
        NativeLogging::LoggingConfig::ActivateProvider (NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        }
    else
        {
        NativeLogging::LoggingConfig::ActivateProvider (NativeLogging::CONSOLE_LOGGING_PROVIDER);
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
BentleyStatus DgnPlatformLib::Host::IKnownLocationsAdmin::GetLocalTempDirectory (BeFileNameR tempDir, WCharCP subDir)
    {
    tempDir = GetLocalTempDirectoryBaseName();
    
    if (NULL != subDir)
        {
        tempDir.AppendToPath (subDir);
        tempDir.AppendSeparator ();
        }

    BeFileNameStatus status = BeFileName::CreateNewDirectory (tempDir);
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
UInt32 DgnPlatformLib::Host::ExceptionHandler::_ResetFloatingPointExceptions (UInt32 newFpuMask)
    {
    return BeNumerical::ResetFloatingPointExceptions (newFpuMask);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                    04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void  DgnPlatformLib::Host::_OnAssert (WCharCP _Message, WCharCP _File, unsigned _Line)
    {
    GetExceptionHandler()._HandleAssert (_Message, _File, _Line);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnHost::GetKeyIndex (DgnHost::Key& key)
    {
    static size_t s_highestKey = 0; // MT: DgnCoreCriticalSection

    BeAssert (key.m_key >= 0 && key.m_key<=s_highestKey); // make sure we're given a valid key

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
    size_t keyIndex = GetKeyIndex (key);

    if (m_hostVar.size() < keyIndex+1)
        m_hostVar.resize(keyIndex+1, VarEntry());

    return  m_hostVar[keyIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnHost::ObjEntry& DgnHost::GetObjEntry(Key& key)
    {
    size_t keyIndex = GetKeyIndex (key);

    if (m_hostObj.size() < keyIndex+1)
        m_hostObj.resize(keyIndex+1, ObjEntry());

    return  m_hostObj[keyIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnHost::IHostObject* DgnHost::GetHostObject(Key& key)
    {
    return  GetObjEntry(key).m_val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnHost::SetHostObject (Key& key, IHostObject* val)
    {
    GetObjEntry(key).m_val = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void* DgnHost::GetHostVariable (Key& key)
    {
    return  GetVarEntry(key).m_val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnHost::SetHostVariable (Key& key, void* val)
    {
    GetVarEntry(key).m_val = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
void       NotificationManager::OutputPrompt (Utf8CP prompt) {return T_HOST.GetNotificationAdmin()._OutputPrompt(prompt);}
StatusInt  NotificationManager::OutputMessage (NotifyMessageDetails const& details) {return T_HOST.GetNotificationAdmin()._OutputMessage (details);}
void       DgnPlatformLib::Host::NotificationAdmin::ChangeAdmin (NotificationAdmin& newAdmin){T_HOST.ChangeNotificationAdmin(newAdmin);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
NotificationManager::MessageBoxValue NotificationManager::OpenMessageBox (MessageBoxType mbType, Utf8CP message, MessageBoxIconType icon)
    {
    return T_HOST.GetNotificationAdmin()._OpenMessageBox (mbType, message, icon);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
ITxn&           ITxnManager::SetCurrentTxn (ITxn& newTxn)
    {
    ITxn* old = m_currTxn;
    m_currTxn = &newTxn;
    return  *old;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/08
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::InteractiveHost::StartupInteractive()
    {
    BeAssert (NULL == m_sessionAdmin);   m_sessionAdmin  = &_SupplySessionAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnPlatformLib::AdoptHost (DgnPlatformLib::Host& host)
    {
    g_hostForThread.SetValueAsPointer (&host);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformLib::Host* DgnPlatformLib::QueryHost ()
    {
    return static_cast<Host*> (g_hostForThread.GetValueAsPointer());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::ForgetHost ()
    {
    g_hostForThread.SetValueAsPointer (NULL);
    }

/*---------------------------------------------------------------------------------**//**
* *Private* method called by DgnPlatform::Host::Initialize.
* @bsimethod                                    Keith.Bentley                   09/08
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::InitializeDgnCore ()
    {
    // NB: Assume that the caller has checked that we are not initializing the same thread twice.

    if (NULL != DgnPlatformLib::QueryHost())
        {
        BeAssert (false); // called more than once on the same thread
        return;
        }

    BeAssert (NULL == m_knownLocationsAdmin); m_knownLocationsAdmin = &_SupplyIKnownLocationsAdmin ();
    BeAssert (NULL == m_exceptionHandler); m_exceptionHandler = &_SupplyExceptionHandler ();
    // establish the NotificationAdmin first, in case other _Supply methods generate errors
    BeAssert (NULL == m_notificationAdmin); m_notificationAdmin = &_SupplyNotificationAdmin();
    BeAssert (NULL == m_realityDataAdmin); m_realityDataAdmin = &_SupplyRealityDataAdmin();

    BeStringUtilities::Initialize(m_knownLocationsAdmin->GetDgnPlatformAssetsDirectory());
    ECDb::Initialize (m_knownLocationsAdmin->GetLocalTempDirectoryBaseName (),
                      &m_knownLocationsAdmin->GetDgnPlatformAssetsDirectory (),
                      m_notificationAdmin->_GetLogSQLiteErrors () ? BeSQLiteLib::LogErrors::Yes : BeSQLiteLib::LogErrors::No);
    L10N::Initialize (_SupplySqlangFiles());

    AdoptHost (*this);
    BeAssert (NULL != DgnPlatformLib::QueryHost());

    _SupplyProductName(m_productName);

    m_acsManager = new IACSManager ();

    DgnPlatformLib::InteractiveHost* interactive = dynamic_cast <DgnPlatformLib::InteractiveHost*> (this);
    if (NULL != interactive)
        {
        interactive->StartupInteractive();
        }
    else
        {
        m_sessionAdmin = new DgnPlatformLib::SessionAdmin();
        }

    BeAssert (NULL == m_txnAdmin); m_txnAdmin = &_SupplyTxnAdmin();

    m_fontManager = new DgnFontManager ();
    m_lineStyleManager = new LineStyleManager ();

    ElementHandlerManager::OnHostInitialize ();
    DgnProject::InitializeTableHandlers();
    
    // ECSchemaReadContext::GetStandardPaths will append ECSchemas/ for us.
    ECN::ECSchemaReadContext::Initialize (T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TerminateDgnCore(bool onProgramExit)
    {
    if (NULL == DgnPlatformLib::QueryHost())
        {
        BeAssert (false);// && "Terminate called on a thread that is not associated with a host");
        return;
        }

    TERMINATE_HOST_OBJECT(m_txnAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_acsManager, onProgramExit);
    TERMINATE_HOST_OBJECT(m_sessionAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_fontManager, onProgramExit);
    TERMINATE_HOST_OBJECT(m_realityDataAdmin, onProgramExit);
    TERMINATE_HOST_OBJECT(m_lineStyleManager, onProgramExit);

    for (ObjEntry& obj : m_hostObj)
        TERMINATE_HOST_OBJECT(obj.m_val, onProgramExit);

    m_hostObj.clear();
    m_hostVar.clear();

    TERMINATE_HOST_OBJECT(m_exceptionHandler, onProgramExit);
    TERMINATE_HOST_OBJECT(m_knownLocationsAdmin, onProgramExit);

    ForgetHost ();
    BeAssert (NULL == DgnPlatformLib::QueryHost());

    BeAssert (NULL == m_fontAdmin);
    BeAssert (NULL == m_lineStyleAdmin);
    BeAssert (NULL == m_rasterAttachmentAdmin);
    BeAssert (NULL == m_pointCloudAdmin);
    BeAssert (NULL == m_notificationAdmin);
    BeAssert (NULL == m_elementHandlerLoader);
    BeAssert (NULL == m_graphicsAdmin);
    BeAssert (NULL == m_materialAdmin);
    BeAssert (NULL == m_solidsKernelAdmin);
    BeAssert (NULL == m_geoCoordAdmin);
    BeAssert (NULL == m_sessionAdmin);
    BeAssert (NULL == m_txnAdmin);
    BeAssert (NULL == m_fontManager);
    BeAssert (NULL == m_acsManager);
    BeAssert (NULL == m_lineStyleManager);
    BeAssert (NULL == m_formatterAdmin);
    BeAssert (NULL == m_realityDataAdmin);
    BeAssert (NULL == m_exceptionHandler);
    BeAssert (NULL == m_knownLocationsAdmin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnPlatformLib::Host::LineStyleAdmin::_GetLocalLinFilePaths (WStringR paths)
    {
#ifdef WIP_CFGVAR // MS_LINFILELIST
    return SUCCESS == ConfigurationManager::GetVariable (paths, L"MS_LINFILELIST");
#endif
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnPlatformLib::Host::LineStyleAdmin::_GetLocalLineStylePaths (WStringR paths)
    {
#ifdef WIP_CFGVAR // MS_SYMBRSRC, MS_LINESTYLEPATH
    ConfigurationManager::GetVariable (paths, L"MS_SYMBRSRC");

    WString lspath;
    if (SUCCESS == ConfigurationManager::GetVariable (lspath, L"MS_LINESTYLEPATH"))
        {
        if (0 != paths.length())
            paths.append (L";");
        paths.append (lspath);
        }

    return 0 != paths.length();
#endif
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformLib::Host::GraphicsAdmin&         DgnPlatformLib::Host::_SupplyGraphicsAdmin()         {return *new GraphicsAdmin();}
DgnPlatformLib::Host::SolidsKernelAdmin&     DgnPlatformLib::Host::_SupplySolidsKernelAdmin()     {return *new SolidsKernelAdmin();}
DgnPlatformLib::Host::MaterialAdmin&         DgnPlatformLib::Host::_SupplyMaterialAdmin()         {return *new MaterialAdmin();}
DgnPlatformLib::Host::GeoCoordinationAdmin&  DgnPlatformLib::Host::_SupplyGeoCoordinationAdmin () {return *new GeoCoordinationAdmin();}
DgnPlatformLib::Host::FontAdmin&             DgnPlatformLib::Host::_SupplyFontAdmin()             {return *new FontAdmin();}
DgnPlatformLib::Host::NotificationAdmin&     DgnPlatformLib::Host::_SupplyNotificationAdmin()     {return *new NotificationAdmin();}
DgnPlatformLib::Host::LineStyleAdmin&        DgnPlatformLib::Host::_SupplyLineStyleAdmin()        {return *new LineStyleAdmin();}
TxnAdmin& DgnPlatformLib::Host::_SupplyTxnAdmin() {return *new TxnAdmin();}
DgnPlatformLib::Host::FormatterAdmin&        DgnPlatformLib::Host::_SupplyFormatterAdmin()        {return *new FormatterAdmin();}
DgnPlatformLib::Host::RealityDataAdmin&      DgnPlatformLib::Host::_SupplyRealityDataAdmin()      {return *new RealityDataAdmin();}

DgnPlatformLib::SessionAdmin&   DgnPlatformLib::InteractiveHost::_SupplySessionAdmin() {return *new SessionAdmin();}

BeFileName DgnPlatformLib::Host::FontAdmin::_GetFontPath() 
    {
    BeFileName assets = DgnPlatformLib::GetHost().GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    return assets.AppendToPath(L"fonts");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      07/14
//---------------------------------------------------------------------------------------
void DgnPlatformLib::ForwardAssertionFailures (BeAssertFunctions::T_BeAssertHandler* h)
    {
    BeAssertFunctions::SetBeAssertHandler (h);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2012
//---------------------------------------------------------------------------------------
DgnProgressMeter::~DgnProgressMeter()
    {
    if (T_HOST.GetProgressMeter() == this)
        T_HOST.SetProgressMeter(NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2012
//---------------------------------------------------------------------------------------
void DgnProgressMeter::AddTasks(UInt32 numTasksToAdd) {_AddTasks(numTasksToAdd);}
void DgnProgressMeter::AddSteps(UInt32 numSteps) {_AddSteps(numSteps);}
void DgnProgressMeter::SetCurrentStepName(Utf8CP newName) {_SetCurrentStepName(newName);}
void DgnProgressMeter::SetCurrentTaskName(Utf8CP newName) {_SetCurrentTaskName(newName);}
DgnProgressMeter::Abort DgnProgressMeter::ShowProgress() {return _ShowProgress();}
void DgnProgressMeter::Hide() {_Hide();}

DEFINE_KEY_METHOD(DgnMarkupProject)
DEFINE_KEY_METHOD(RedlineModel)
DEFINE_KEY_METHOD(PhysicalRedlineModel)
DEFINE_KEY_METHOD(IPickGeom)
