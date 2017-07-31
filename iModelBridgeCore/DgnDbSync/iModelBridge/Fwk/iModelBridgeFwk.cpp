/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/iModelBridgeFwk.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux)
#include <unistd.h>
#endif
#include <iModelBridge/iModelBridgeFwk.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTextFile.h>
#include <Logging/bentleylogging.h>
#include <iModelBridge/iModelBridgeBimHost.h>
#include "cpl_spawn.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LOGGING

#define RETURN_STATUS_SUCCESS           0
#define RETURN_STATUS_USAGE_ERROR       1
#define RETURN_STATUS_CONVERTER_ERROR   2
#define RETURN_STATUS_SERVER_ERROR      3
#define RETURN_STATUS_LOCAL_ERROR       4

#define MUSTBEDBRESULT(stmt,RESULT) {auto rc=stmt; if (RESULT!=rc) {return rc;}}
#define MUSTBEOK(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_OK)
#define MUSTBEROW(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_ROW)
#define MUSTBEDONE(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_DONE)

#ifdef _WIN32
#define EXE_EXT L".exe"
#else
#define EXE_EXT L""
#endif

static ProfileVersion s_schemaVer(1,0,0,0);
static BeSQLite::PropertySpec s_schemaVerPropSpec("SchemaVersion", "be_iModelBridgeFwk", BeSQLite::PropertySpec::Mode::Normal, BeSQLite::PropertySpec::Compress::No);
static BeSQLite::PropertySpec s_briefcaseIdPropSpec("BriefcaseId", "be_iModelBridgeFwk", BeSQLite::PropertySpec::Mode::Normal, BeSQLite::PropertySpec::Compress::No);
//static BeSQLite::PropertySpec s_parentRevisionIdPropSpec("ParentRevisionId", "be_iModelBridgeFwk", BeSQLite::PropertySpec::Mode::Normal, BeSQLite::PropertySpec::Compress::No);

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridgeFwk"))

BEGIN_BENTLEY_DGN_NAMESPACE

#ifdef _WIN32
static int s_maxWaitForMutex = 60000;
#endif

static bmap<BeFileName, CPLSpawnedProcess*> s_affinityCalculators;

#if defined(__unix__)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void* iModelBridgeFwk::GetBridgeFunction(BeFileNameCR bridgeDllName, Utf8CP funcName)
    {
    BeAssert(false && "TBD");
    return nullptr;
    }

#else

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
static void setDllSearchPathFor(BeFileNameCR bridgeDllName)
    {
    BeFileName pathname(BeFileName::FileNameParts::DevAndDir, bridgeDllName);

    ::SetDllDirectoryW(pathname.c_str());

    WString newPath(L"PATH=");
    newPath.append(pathname);
    newPath.append(L";");
    newPath.append(::_wgetenv(L"PATH"));
    _wputenv(newPath.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void* iModelBridgeFwk::GetBridgeFunction(BeFileNameCR bridgeDllName, Utf8CP funcName)
    {
    setDllSearchPathFor(bridgeDllName);

    auto hinst = ::LoadLibraryW(bridgeDllName.c_str());
    if (!hinst)
        {
        LOG.fatalv(L"%ls: not found or could not be loaded", bridgeDllName.c_str());
        return nullptr;
        }

    return GetProcAddress (hinst, funcName);
    }

#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
T_iModelBridge_getInstance* iModelBridgeFwk::JobDefArgs::LoadBridge()
    {
    auto getInstance = (T_iModelBridge_getInstance*) GetBridgeFunction(m_bridgeLibraryName, "iModelBridge_getInstance");
    if (!getInstance)
        {
        LOG.fatalv(L"%ls: Does not export a function called 'iModelBridge_getInstance'", m_bridgeLibraryName.c_str());
        return nullptr;
        }

    return getInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static WString getArgValueW(WCharCP arg)
    {
    WString argValue(arg);
    argValue = argValue.substr(argValue.find_first_of('=', 0) + 1);
    argValue.Trim(L"\"");
    argValue.Trim();
    return argValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getArgValue(WCharCP arg)
    {
    return Utf8String(getArgValueW(arg));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus readEntireFile(WStringR contents, BeFileNameCR fileName)
    {
    BeFile errfile;
    if (BeFileStatus::Success != errfile.Open(fileName.c_str(), BeFileAccess::Read))
        return BSIERROR;

    bvector<Byte> bytes;
    if (BeFileStatus::Success != errfile.ReadEntireFile(bytes))
        return BSIERROR;

    if (bytes.empty())
        return BSISUCCESS;

    bytes.push_back('\0'); // End of stream

    const Byte utf8BOM[] = {0xef, 0xbb, 0xbf};
    if (bytes[0] == utf8BOM[0] || bytes[1] == utf8BOM[1] || bytes[2] == utf8BOM[2])
        contents.AssignUtf8((Utf8CP) (bytes.data() + 3));
    else
        contents.AssignA((char*) bytes.data());

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static bool anyTxnsInFile(DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(db, "SELECT Id FROM " DGN_TABLE_Txns " LIMIT 1");
    return (BE_SQLITE_ROW == stmt.Step());
    }

END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::PrintUsage(WCharCP programName)
    {
    WString exeName = BeFileName::GetFileNameAndExtension(programName);

    fwprintf (stderr, L"Usage: %ls\n", exeName.c_str());

    JobDefArgs::PrintUsage();
    ServerArgs::PrintUsage();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::JobDefArgs::PrintUsage()
    {
    fwprintf (stderr,
        L"JOB ENVIRONMENT:\n"
        L"--fwk-bridge-library=       (optional)  The full path to the bridge library. Specify this instead of --fwk-bridge-regsubkey.\n"
        L"--fwk-bridge-regsubkey=     (optional)  The registry subkey of a bridge that is installed on this machine. Specify this instead of --fwk-bridge-library.\n"
        L"--fwk-staging-dir=          (required)  The staging directory.\n"
        L"--fwk-input=                (required)  Input file name. Specify only one.\n"
        L"--fwk-input-sheet=          (required)  Input sheet file name. Can be more than one.\n"
        L"--fwk-revision-comment=     (optional)  The revision comment. Can be more than one.\n"
        L"--fwk-logging-config-file=  (optional)  The name of the logging configuration file.\n"
#ifdef COMMENT_OUT // *** we don't plan to support these as direct inputs in the framework BAS GUI
        L"--fwk-max-wait=milliseconds (optional)  The maximum amount of time to wait for other instances of this job to finish.\n"
        L"--fwk-input-gcs=gcsspec     (optional)  Specifies the GCS of the input DGN root model. Ignored if DGN root model already has a GCS.\n"
        L"--fwk-geoCalculation=       (optional)  If a new model is added with the --update option, sets the geographic coordinate calculation method. Possible Values are:\n"
        L"                                           Default   - Base the calculation method on the source GCS. GCS's calculated from Placemarks are transformed, others are reprojected\n"
        L"                                           Reproject - Do full geographic reprojection of each point\n"
        L"                                           Transform - use the source GCS and DgnDb GCS to construct and use a linear transform\n"
        L"                                           TransformScaled - Similar to Transform, except the transform is scaled to account for the GCS grid to ground scale (also known as K factor).\n"
#endif
        );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::InitLogging()
    {
    if (!m_jobEnvArgs.m_loggingConfigFileName.empty() && m_jobEnvArgs.m_loggingConfigFileName.DoesPathExist())
        {
        NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, m_jobEnvArgs.m_loggingConfigFileName);
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        return;
        }

    fprintf(stderr, "Logging.config.xml not specified. Activating default logging using console provider.\n");
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity(L"iModelBridge", NativeLogging::LOG_INFO);
    NativeLogging::LoggingConfig::SetSeverity(L"iModelBridgeFwk", NativeLogging::LOG_INFO);
    NativeLogging::LoggingConfig::SetSeverity(L"Performance", NativeLogging::LOG_TRACE);
    //NativeLogging::LoggingConfig::SetSeverity(L"DgnCore", NativeLogging::LOG_TRACE);
    //NativeLogging::LoggingConfig::SetSeverity(L"BeSQLite", NativeLogging::LOG_TRACE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName  GetDefaultAssetsDirectory()
    {
    BeFileName fwkAssetsDirRaw = Desktop::FileSystem::GetExecutableDir(); // this will be:    blah/iModelBridgeFwk/lib/x64

    fwkAssetsDirRaw.AppendToPath(L"..");
    fwkAssetsDirRaw.AppendToPath(L"..");
    fwkAssetsDirRaw.AppendToPath(L"assets");                        // we want:         blah/iModelBridgeFwk/assets
    if (fwkAssetsDirRaw.DoesPathExist())
        return fwkAssetsDirRaw;

    //Try default assets folder relative the program directory
    fwkAssetsDirRaw = Desktop::FileSystem::GetExecutableDir();
    fwkAssetsDirRaw.AppendToPath(L"assets");
    return fwkAssetsDirRaw;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridgeFwk::JobDefArgs::ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[])
    {
    BeFileName fwkAssetsDirRaw = GetDefaultAssetsDirectory();
    for (int iArg = 1; iArg < argc; ++iArg)
        {
        if (argv[iArg][0] == '@')
            {
            BeFileName rspFileName(argv[iArg]+1);
            WString wargs;
            if (BSISUCCESS != readEntireFile(wargs, rspFileName))
                {
                fwprintf(stderr, L"%ls - response file not found\n", rspFileName.c_str());
                return BSIERROR;
                }

            bvector<WString> strings;
            bvector<WCharCP> ptrs;
            BeStringUtilities::ParseArguments(strings, wargs.c_str(), L"\n\r");

            if (!strings.empty())
                {
                ptrs.push_back(argv[0]);
                for (auto const& str: strings)
                    {
                    if (!str.empty())
                        ptrs.push_back(str.c_str());
                    }

                if (BSISUCCESS != ParseCommandLine(bargptrs, (int)ptrs.size(), &ptrs.front()))
                    return BSIERROR;
                }

            continue;
            }

        if (0 != BeStringUtilities::Wcsnicmp(argv[iArg], L"--fwk", 5))
            {
            // Not a fwk argument. We will forward it to the bridge.
            m_bargs.push_back(argv[iArg]);  // Keep the string alive
            bargptrs.push_back(m_bargs.back().c_str());
            continue;
            }

        // this is just to allow for testing in a standalone setup
        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-bridge-library"))
            {
            if (!m_bridgeLibraryName.empty())
                {
                fwprintf(stderr, L"The --fwk-bridge-library= option may appear only once.\n");
                return BSIERROR;
                }

            m_bridgeLibraryName.SetName(getArgValueW(argv[iArg]));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-bridge-regsubkey"))
            {
            if (!m_bridgeRegSubKey.empty())
                {
                fwprintf(stderr, L"The --fwk-bridge-regsubkey= option may appear only once.\n");
                return BSIERROR;
                }

            m_bridgeRegSubKey = getArgValueW(argv[iArg]);
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-create-repository-if-necessary")) // undocumented
            {
            m_createRepositoryIfNecessary = true;
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-staging-dir="))
            {
            if (!m_stagingDir.empty())
                {
                fwprintf(stderr, L"The --fwk-staging-dir= option may appear only once.\n");
                return BSIERROR;
                }
            m_stagingDir.SetName(getArgValueW(argv[iArg]));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-input="))
            {
            if (!m_inputFileName.empty())
                {
                fwprintf(stderr, L"The --fwk-input= option may appear only once.\n");
                return BSIERROR;
                }
            BeFileName::FixPathName (m_inputFileName, getArgValueW(argv[iArg]).c_str());
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-input-sheet="))
            {
            BeFileName fn;
            BeFileName::FixPathName (fn, getArgValueW(argv[iArg]).c_str());
            m_drawingAndSheetFiles.push_back(fn);
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-revision-comment="))
            {
            m_revisionComment.append(getArgValue(argv[iArg]).c_str());
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-logging-config-file="))
            {
            m_loggingConfigFileName.SetName(getArgValueW(argv[iArg]));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-assetsDir="))
            {
            BeFileName assetsDir(getArgValueW(argv[iArg]));
            if (assetsDir.DoesPathExist())
                fwkAssetsDirRaw = assetsDir;

            continue;
            }

#ifdef _WIN32
        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-max-wait="))
            {
            wscanf(getArgValueW(argv[iArg]).c_str(), L"%d", &s_maxWaitForMutex);
            continue;
            }
#endif

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-input-gcs="))
            {
            iModelBridge::GCSDefinition gcs;
            if (BSISUCCESS != iModelBridge::Params::ParseGcsSpec(m_inputGcs, getArgValue(argv[iArg])))
                return BSIERROR;
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-geoCalculation="))
            {
            if (BSISUCCESS != iModelBridge::Params::ParseGCSCalculationMethod(m_gcsCalculationMethod, getArgValue(argv[iArg])))
                return BSIERROR;
            continue;
            }
        
        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-bridgeAssetsDir="))
            {
            BeFileName assetsDir(getArgValueW(argv[iArg]));
            if (assetsDir.DoesPathExist())
                m_bridgeAssetsDir = assetsDir;
            continue;
            }

        BeAssert(false);
        fwprintf(stderr, L"%ls: unrecognized fwk argument\n", argv[iArg]);
        return BSIERROR;
        }

    BeFileName::FixPathName(m_fwkAssetsDir, fwkAssetsDirRaw.c_str());
    m_fwkAssetsDir.BeGetFullPathName();

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridgeFwk::JobDefArgs::Validate(int argc, WCharCP argv[])
    {
    /*Commented out this assert for ease of debugging when not running against automation services.
    if (m_bridgeLibraryName.empty() && m_bridgeRegSubKey.empty()
     || !m_bridgeLibraryName.empty() && !m_bridgeRegSubKey.empty())
        {
        fwprintf(stderr, L"Either --fwk-bridge-library or --fwk-bridge-regsubkey must be specified, but not both.\n");
        return BSIERROR;
        }*/

    if (!m_bridgeLibraryName.empty() && !m_bridgeLibraryName.DoesPathExist())
        {
        fwprintf(stderr, L"[%ls: not found. The --fwk-bridge-library argument must be the name of a shared library.\n", m_bridgeLibraryName.c_str());
        return BSIERROR;
        }

    if (m_stagingDir.empty())
        {
        fwprintf(stderr, L"The staging directory name is required.\n");
        return BSIERROR;
        }

    if (!m_stagingDir.IsDirectory())
        {
        fwprintf(stderr, L"[%ls] is not a directory. The --fwk-staging-dir argument must be the name of the directory where the briefcase and other bridge-specific files are stored.\n", m_stagingDir.c_str());
        return BSIERROR;
        }

    if (m_loggingConfigFileName.empty())
        {
        m_loggingConfigFileName = Desktop::FileSystem::GetExecutableDir();
        m_loggingConfigFileName.AppendToPath(L"iModelBridgeFwk.logging.config.xml");
        }

    if (!m_loggingConfigFileName.DoesPathExist())
        {
        fwprintf(stdout, L"%ls: file not found.\n", m_loggingConfigFileName.c_str());
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::ParseCommandLine(int argc, WCharCP argv[])
    {
    if (argc < 2)
        {
        PrintUsage(argv[0]);
        return BSIERROR;
        }

    m_bargptrs.push_back(argv[0]);

    if ((BSISUCCESS != m_jobEnvArgs.ParseCommandLine(m_bargptrs, argc, argv)) || (BSISUCCESS != m_jobEnvArgs.Validate(argc, argv)))
        {
        PrintUsage(argv[0]);
        return BSIERROR;
        }

    // *After* verifying that m_stagingDir exists,
    // Redirect stderr to a file in m_stagingDir. This is how we report some errors back to the calling program.
    RedirectStderr();

    InitLogging();

    bvector<WCharCP> rawArgPtrs;        // pare down the args once again, removing the server-specific args and leaving the rest for the bridge
    std::swap(rawArgPtrs, m_bargptrs);

    m_bargptrs.push_back(argv[0]);

    if ((BSISUCCESS != m_serverArgs.ParseCommandLine(m_bargptrs, (int)rawArgPtrs.size(), rawArgPtrs.data())) || (BSISUCCESS != m_serverArgs.Validate((int)rawArgPtrs.size(), rawArgPtrs.data())))
        {
        PrintUsage(argv[0]);
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SetSyncState(SyncState st)
    {
    m_stateDb.SaveBriefcaseLocalValue("be_iModelBridgeFwk_Sync_State", (uint64_t)st);
    m_stateDb.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeFwk::SyncState iModelBridgeFwk::GetSyncState()
    {
    uint64_t val;
    if (BE_SQLITE_ROW != m_stateDb.QueryBriefcaseLocalValue(val, "be_iModelBridgeFwk_Sync_State"))
        return SyncState::Initial;
    return (SyncState)(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SaveNewModelIds()
    {
    if (m_modelsInserted.empty())
        return;

    auto stmt = m_stateDb.GetCachedStatement("INSERT INTO fwk_CreatedModels (ModelId) VALUES(?)");
    for (auto mid : m_modelsInserted)
        {
        stmt->Reset();
        stmt->ClearBindings();
        stmt->BindId(1, mid);
        stmt->Step();
        }
    m_stateDb.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::ReadNewModelIds()
    {
    BeAssert(m_modelsInserted.empty());

    auto stmt = m_stateDb.GetCachedStatement("SELECT ModelId FROM fwk_CreatedModels");
    while (BE_SQLITE_ROW == stmt->Step())
        {
        m_modelsInserted.push_back(stmt->GetValueId<DgnModelId>(0));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult iModelBridgeFwk::OpenOrCreateStateDb()
    {
    BeFileName tempDir;
    Desktop::FileSystem::BeGetTempPath(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    Briefcase_MakeBriefcaseName();
    BeFileName stateFileName(m_briefcaseName);
    stateFileName.append(L".fwk-state.db");
    if (stateFileName.DoesPathExist())
        {
        MUSTBEOK(m_stateDb.OpenBeSQLiteDb(stateFileName, Db::OpenParams(Db::OpenMode::ReadWrite)));

        // Double-check that this really is a fwk state db
        Utf8String propStr;
        if (BE_SQLITE_ROW != m_stateDb.QueryProperty(propStr, s_schemaVerPropSpec))
            {
            LOG.fatalv(L"%ls - this is an invalid fwk state db. Re-creating ...", stateFileName.c_str());
            m_stateDb.CloseDb();
            stateFileName.BeDeleteFile();
            }
        }

    if (!stateFileName.DoesPathExist())
        {
        MUSTBEOK(m_stateDb.CreateNewDb(stateFileName));
        MUSTBEOK(m_stateDb.CreateTable("fwk_CreatedModels", "ModelId BIGINT"));
        MUSTBEOK(m_stateDb.CreateTable("fwk_InstalledBridges", "Name TEXT NOT NULL UNIQUE COLLATE NoCase, \
                                                                BridgeLibraryPath TEXT UNIQUE COLLATE NoCase, \
                                                                AffinityLibraryPath TEXT UNIQUE COLLATE NoCase, \
                                                                IsPowerPlatformBased BOOLEAN \
                                                                "));
        MUSTBEOK(m_stateDb.CreateTable("fwk_BridgeAssignments", "SourceFile TEXT NOT NULL UNIQUE COLLATE NoCase PRIMARY KEY, \
                                                                 Bridge BIGINT"));  // Bridge --foreign key--> fwk_InstalledBridges

        MUSTBEOK(m_stateDb.SavePropertyString(s_schemaVerPropSpec, s_schemaVer.ToJson()));

        MUSTBEOK(m_stateDb.SaveChanges());
        }
    else
        {
        Utf8String propStr;
        MUSTBEROW(m_stateDb.QueryProperty(propStr, s_schemaVerPropSpec));

        ProfileVersion ver(propStr.c_str());
        if (ver.IsEmpty() || ver.GetMajor() != s_schemaVer.GetMajor())
            {
            LOG.fatalv(L"%ls - version %ls is too old", stateFileName.c_str(), WString(ver.ToString().c_str(), true).c_str());
            return BE_SQLITE_ERROR_ProfileTooOld;
            }
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SetState(BootstrappingState st)
    {
    m_stateDb.SaveBriefcaseLocalValue("be_iModelBridgeFwk_BootStrapping_State", (uint64_t)st);
    m_stateDb.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeFwk::BootstrappingState iModelBridgeFwk::GetState()
    {
    uint64_t val;
    if (BE_SQLITE_ROW != m_stateDb.QueryBriefcaseLocalValue(val, "be_iModelBridgeFwk_BootStrapping_State"))
        return BootstrappingState::Initial;
    return (BootstrappingState)(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::CleanJobWorkdir()
    {
    BeAssert(!m_briefcaseName.empty());
    BeAssert(m_briefcaseName.DoesPathExist());

    bvector<BeFileName> files;
    WString glob = m_briefcaseName.GetFileNameWithoutExtension();
    glob.append(L"*");
//    GetLogger().infov("Deleting existing files [%s/%s]", Utf8String(m_briefcaseName.GetDirectoryName()).c_str(), Utf8String(glob).c_str());
    BeDirectoryIterator::WalkDirsAndMatch(files, m_briefcaseName.GetDirectoryName(), glob.c_str(), false);
    for (auto const& file: files)
        BeFileName::BeDeleteFile(file);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::AssertPreConditions()
    {
    if (m_briefcaseName.empty())
        {
        BeAssert(false);
        return BSIERROR;
        }

    if (m_briefcaseName.DoesPathExist() && m_briefcaseName.IsDirectory())
        {
        BeAssert(false);
        LOG.fatalv(L"%ls - is a directory name. I need to create a file by this name.", m_briefcaseName.c_str());
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::DoInitial()
    {
    // ***
	// ***
	// *** DO NOT CHANGE THE ORDER OF THE STEPS BELOW
	// *** Talk to Sam Wilson if you need to make a change.
	// ***
	// ***

    // ***
    // *** TRICKY: Do not call InitBridge until AFTER we try to acquire the briefcase.
    // ***

    BeAssert((nullptr != DgnPlatformLib::QueryHost()) && "framework must initialize the host before calling this.");

    bool fileExists = m_briefcaseName.DoesPathExist();
    bool haveBriefcase = fileExists && Briefcase_IsBriefcase();

    if (haveBriefcase)
        {
        BeDataAssert(false && "We shouldn't be in Initial when we have a briefcase. Maybe somebody deleted the state file. We can go on from here...");
        SetState(BootstrappingState::HaveBriefcase);
        return BSISUCCESS;
        }

    //  We don't have a briefcase

    if (fileExists)
        {
        // We have a db, but it's not a briefcase.
        // Since I'm in the Initial state, I know that a previous attempt to bootstrap a repository must have failed
        // while or after creating the local Db. (Otherwise, I'd be in the CreatedLocalDb state.)
        // Since there's a good chance that the local Db is corrupt, just delete it and start over.
        CleanJobWorkdir();
        }

    // Maybe we just need to acquire a briefcase for an existing repository.
    if (BSISUCCESS == Briefcase_AcquireBriefcase())
        {
        SetState(BootstrappingState::HaveBriefcase);
        return BSISUCCESS;
        }

    //  Can't acquire a briefcase
    if (EffectiveServerError::iModelDoesNotExist != m_lastServerError)
        {
        // Probably some kind of communications failure, or incorrect credentials.
        // This is a recoverable error. Stay in Initial state.
        return BSIERROR;
        }

    //  Repository does not exist
    if (!m_jobEnvArgs.m_createRepositoryIfNecessary)
        {
        GetLogger().fatalv("%s - repository not found in project. Create option was not specified.\n", m_serverArgs.m_repositoryName.c_str());
        return BSIERROR;
        }

    //
    //  We need to create a new repository.
    //

    //  Initialize the bridge to do the creation.
    //  Note: iModelBridge::_Initialize will register domains, and some of them may be required.
    if (BSISUCCESS != InitBridge())
        return BentleyStatus::ERROR;

    //  Create a new repository. (This will import all required domains and their schemas.)
    m_briefcaseDgnDb = m_bridge->DoCreateDgnDb(m_modelsInserted, nullptr);
    if (!m_briefcaseDgnDb.IsValid())
        {
        // Hopefully, this is a recoverable error. Stay in Initial state, so that we can try again.
        return BSIERROR;
        }

    auto rc = m_briefcaseDgnDb->SaveChanges();
    if (BeSQLite::BE_SQLITE_OK != rc)
        {
        // Hopefully, this is a recoverable error. Stay in Initial state, so that we can try again.
        LOG.fatalv("SaveChanges failed with %s", m_briefcaseDgnDb->GetLastError().c_str());
        m_briefcaseDgnDb->AbandonChanges();
        m_briefcaseDgnDb = nullptr;
        m_briefcaseName.BeDeleteFile();
        m_bridge->_DeleteSyncInfo();
        return BentleyStatus::ERROR;
        }

    BeAssert(!m_briefcaseDgnDb->Txns().HasChanges());

    SaveNewModelIds();
    m_modelsInserted.clear();   // *** CLEAR, SO THAT WE TEST WRITE/READ CODE

    // Close the DgnDb that converter just created. The next states all start with a filename. Also, we
    // must be sure that all changes are flushed to disk.
    m_briefcaseDgnDb = nullptr;

    SetState(BootstrappingState::CreatedLocalDb);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::DoCreatedLocalDb()
    {
    bool fileExists = m_briefcaseName.DoesPathExist();
    if (!fileExists)
        {
        BeDataAssert(false && "CreatedLocalDb state assumes localDb was created");
        SetState(BootstrappingState::Initial); // start over
        return BSIERROR;
        }

    if (Briefcase_IsBriefcase())
        {
        BeAssert(false && "Invalid state. If we have a briefcase, we should not be in CreatedLocalDb");
        SetState(BootstrappingState::HaveBriefcase);
        return BSIERROR;
        }

    if (BSISUCCESS != Briefcase_CreateRepository())
        return BSIERROR;

    SetState(BootstrappingState::CreatedRepository);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::DoCreatedRepository()
    {
    bool fileExists = m_briefcaseName.DoesPathExist();
    if (fileExists)
        {
        // This is normal. The local file that we created with the converter and then uploaded
        // is still sitting around. Delete it now, to make way for the briefcase.
        if (BeFileNameStatus::Success != m_briefcaseName.BeDeleteFile())
            {
            BeAssert(false);
            return BSIERROR;
            }
        }

    if (BSISUCCESS != Briefcase_AcquireBriefcase())
        {
        BeAssert(false && "we think we have created a repository, but it must have failed");
        if (EffectiveServerError::iModelDoesNotExist == m_lastServerError)
            {
            SetState(BootstrappingState::Initial);
            return BSISUCCESS;
            }
        GetLogger().fatalv("CreateRepository failed or acquireBriefcase failed for repositoryname=%s, stagingdir=%s",
                            m_serverArgs.m_repositoryName.c_str(), Utf8String(m_jobEnvArgs.m_stagingDir).c_str());
        return BSIERROR;
        }

    SetState(BootstrappingState::NewBriefcaseNeedsLocks);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::DoNewBriefcaseNeedsLocks()
    {
    // I just created a repository and pulled a briefcase for it.
    // I must now manually get an exclusive lock on all of the models that I created
#ifdef DEBUG_BIM_BRIDGE
    bvector<DgnModelId> check;
    std::swap(check, m_modelsInserted);
#endif
    if (m_modelsInserted.empty())
        ReadNewModelIds();

#ifdef DEBUG_BIM_BRIDGE
    BeAssert(check == m_modelsInserted);
#endif

    if (BSISUCCESS != Briefcase_AcquireExclusiveLocks())
        return BSIERROR;

    //DeleteNewModelIdsFile();  *** NEEDS WORK: What did I intend here? Am I trying to clean up after a crash??

    SetState(BootstrappingState::HaveBriefcase);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::BootstrapBriefcase(bool& createdNewRepo)
    {
    BootstrappingState state;
    int count = 0;
    createdNewRepo = false;
    while (BootstrappingState::HaveBriefcase != (state = GetState()))
        {
        GetLogger().infov("State=%d", (int)state);

        if (BSISUCCESS != AssertPreConditions())
            return BSIERROR;

        BentleyStatus status = BSISUCCESS;
        switch (state)
            {
            case BootstrappingState::Initial:                   status = DoInitial(); break;
            case BootstrappingState::CreatedLocalDb:            status = DoCreatedLocalDb(); createdNewRepo = true; break;
            case BootstrappingState::CreatedRepository:         status = DoCreatedRepository(); break;
            case BootstrappingState::NewBriefcaseNeedsLocks:    status = DoNewBriefcaseNeedsLocks(); break;
            }

        if (BSISUCCESS != status)
            return BSIERROR;

        if (count++ > (int)BootstrappingState::LastState)
            {
            // We seem to be cycling.
            GetLogger().error("Unable to recover from errors");
            return BSIERROR;
            }
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName findBridgeAssetsDir(BeFileNameCR bridgeLibDir)
    {
    BeFileName assetsDir(bridgeLibDir);
    assetsDir.AppendToPath(L"Assets");
    if (assetsDir.DoesPathExist())
        return assetsDir;

    assetsDir = bridgeLibDir;
    assetsDir.AppendToPath(L"../../Assets");
    if (assetsDir.DoesPathExist())
        return assetsDir;

    BeAssert(false && "bridge product directory structure does not follow the convention of putting library in /lib/<arch> subdir");
    return assetsDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SetBridgeParams(iModelBridge::Params& params)
    {
    Briefcase_MakeBriefcaseName(); // => defines m_briefcaseName
    params.m_briefcaseName = m_briefcaseName;
    params.SetReportFileName();
    params.GetReportFileName().BeDeleteFile();
    params.m_assetsDir = m_jobEnvArgs.m_bridgeAssetsDir;
    params.m_libraryDir = m_jobEnvArgs.m_bridgeLibraryName.GetDirectoryName();
    params.m_repoAdmin = &m_repoAdmin;
    params.m_inputFileName = m_jobEnvArgs.m_inputFileName;
    params.m_gcsCalculationMethod = m_jobEnvArgs.m_gcsCalculationMethod;
    params.m_inputGcs = m_jobEnvArgs.m_inputGcs;
    params.m_drawingAndSheetFiles = m_jobEnvArgs.m_drawingAndSheetFiles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::LoadBridge()
    {
    auto getInstance = m_jobEnvArgs.LoadBridge();
    if (nullptr == getInstance)
        return BentleyStatus::ERROR;

    m_bridge = getInstance(m_jobEnvArgs.m_bridgeRegSubKey.c_str());
    if (nullptr == m_bridge)
        {
        LOG.fatalv(L"%ls: iModelBridge_getInstance function returned a nullptr", m_jobEnvArgs.m_bridgeLibraryName.c_str());
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::InitBridge()
    {
    SetBridgeParams(m_bridge->_GetParams());

    if (BentleyStatus::SUCCESS != m_bridge->_ParseCommandLine((int)m_bargptrs.size(), m_bargptrs.data()))
        {
        fprintf(stderr, "bridge _ParseCommandLine failed\n");
        m_bridge->_PrintUsage();
        return BentleyStatus::ERROR;
        }

    SetBridgeParams(m_bridge->_GetParams());    // make sure that MY definition of these params is used!

    if (BSISUCCESS != m_bridge->_Initialize((int)m_bargptrs.size(), m_bargptrs.data()))
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

#ifdef COMMENT_OUT // if we do this up front, then we'll always pay the performance cost, even if there are no updates
                    // to the shared models. All in all, it's better to let the briefcase manager detect the need for
                    // such locks, along with all other locking requirements, and then try to acquire them at the end.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void addModelLockRequest(LockRequest& req, DgnDbR db, DgnModelId mid)
    {
    auto model = db.Models().GetModel(mid);
    if (!model.IsValid())
        return;
    req.Insert(*model, LockLevel::Shared);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static RepositoryStatus acquireSharedLocks(DgnDbR db)
    {
    BeAssert(db.IsBriefcase());

    //  Get a shared lock on some internal models. I may write to these models if I create new models or categories, etc.
    LockRequest req;
    addModelLockRequest(req, db, DgnModel::DictionaryId());
    addModelLockRequest(req, db, DgnModel::RepositoryModelId());
    /*
    addModelLockRequest(req, db, m_drawingListModelId);
    addModelLockRequest(req, db, m_sheetListModelId);
    addModelLockRequest(req, db, m_groupModelId);
    */
    if (db.Elements().Get<DefinitionPartition>(db.Elements().GetRealityDataSourcesPartitionId()).IsValid())
        addModelLockRequest(req, db, db.Elements().Get<DefinitionPartition>(db.Elements().GetRealityDataSourcesPartitionId())->GetSubModelId());

    return db.BriefcaseManager().AcquireLocks(req).Result();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SaveBriefcaseId()
    {
    auto db = DgnDb::OpenDgnDb(nullptr, m_briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    if (!db.IsValid())
        {
        BeAssert(false);
        return;
        }
    uint32_t bcid = db->GetBriefcaseId().GetValue();
    m_stateDb.SaveProperty(s_briefcaseIdPropSpec, &bcid, sizeof(bcid));
    m_stateDb.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
//void iModelBridgeFwk::SaveParentRevisionId()
//    {
//    auto tipRevId = m_briefcaseDgnDb->Revisions().GetParentRevisionId();
//    m_stateDb.SavePropertyString(s_parentRevisionIdPropSpec, tipRevId);
//    m_stateDb.SaveChanges();
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::RunExclusive(int argc, WCharCP argv[])
    {
    // ***
	// ***
	// *** DO NOT CHANGE THE ORDER OF THE STEPS IN THIS FUNCTION.
	// *** Talk to Sam Wilson if you need to make a change.
	// ***
	// ***
    DbResult dbres;

    iModelBridgeSacAdapter::InitCrt(false);

    //  Open our state db. (That's where bridge assignments are, too.)
    dbres = OpenOrCreateStateDb();
    if (BE_SQLITE_OK != dbres)
        return RETURN_STATUS_LOCAL_ERROR;

    //  Resolve the bridge to run
    if (m_jobEnvArgs.m_bridgeLibraryName.empty())
        {
        FindBridgeInRegistry(m_jobEnvArgs.m_bridgeLibraryName, m_jobEnvArgs.m_bridgeAssetsDir, m_jobEnvArgs.m_bridgeRegSubKey);
        if (m_jobEnvArgs.m_bridgeLibraryName.empty())
            {
            LOG.fatalv(L"%s - no bridge with this subkey is in the state db", m_jobEnvArgs.m_bridgeRegSubKey.c_str());
            return RETURN_STATUS_LOCAL_ERROR;
            }
        }

    if (m_jobEnvArgs.m_bridgeAssetsDir.empty())
        {
        m_jobEnvArgs.m_bridgeAssetsDir = findBridgeAssetsDir(m_jobEnvArgs.m_bridgeLibraryName.GetDirectoryName());

        }

    // Put out this info message, so that we can relate all subsequent logging messages to this bridge.
    // The log on this machine may have messages from many bridge jobs.
    LOG.infov(L"Running bridge [%ls] staging dir [%ls] input [%ls]", m_jobEnvArgs.m_bridgeLibraryName.c_str(),
              m_jobEnvArgs.m_stagingDir.c_str(), m_jobEnvArgs.m_inputFileName.c_str());

    // Load the bridge ... but don't initialize it.
    // *** TRICKY: Do not call InitBridge until AFTER we acquire the briefcase (or decide to create a new DgnDb).
    if (BentleyStatus::SUCCESS != LoadBridge())
        return RETURN_STATUS_CONVERTER_ERROR;

    // Initialize the DgnViewLib Host.
    iModelBridge::Params params;
    SetBridgeParams(params);

    BeFileName fwkAssetsDir(m_jobEnvArgs.m_fwkAssetsDir);
    BeFileName fwkDb3 = fwkAssetsDir;
    fwkDb3.AppendToPath(L"sqlang");
    fwkDb3.AppendToPath(L"iModelBridgeFwk_en-US.sqlang.db3");

    Dgn::iModelBridgeBimHost host(params.GetRepositoryAdmin(), fwkAssetsDir, fwkDb3, ""); // *** TBD: product name = job name
    DgnViewLib::Initialize(host, true);

    //  Initialize the bridge-specific L10N
    BeFileName bridgeSqlangPath(params.GetAssetsDir());
    bridgeSqlangPath.AppendToPath(m_bridge->_SupplySqlangRelPath().c_str());
    iModelBridge::L10N::Initialize(BeSQLite::L10N::SqlangFiles(bridgeSqlangPath));

    static PrintfProgressMeter s_meter;
    T_HOST.SetProgressMeter(&s_meter);

    //  Sign into the iModelHub
    if (BSISUCCESS != Briefcase_Initialize(argc, argv))
        return RETURN_STATUS_SERVER_ERROR;

    //  Make sure we have a briefcase.
    Briefcase_MakeBriefcaseName(); // => defines m_briefcaseName
    bool createdNewRepo = false;
    if (BSISUCCESS != BootstrapBriefcase(createdNewRepo))
        {
        m_briefcaseDgnDb = nullptr;
        return RETURN_STATUS_SERVER_ERROR;
        }

    //  "Bootstrapping" the briefcase might have created a new repository. In that case, there is no need to go through the update logic and pullmergepush.
    if (createdNewRepo)
        return RETURN_STATUS_SUCCESS;

    //  The repo already exists. Run the bridge to update it and then push the changeset to the iModel.
    auto status = UpdateExistingBim();

    s_meter.Hide();

    if (m_briefcaseDgnDb.IsValid())     // must make sure briefcase dgndb is closed before tearing down host!
        m_briefcaseDgnDb = nullptr;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::UpdateExistingBim()
    {
    // PRE-CONDITIONS
    BeAssert(nullptr != m_bridge);
    BeAssert(nullptr != DgnPlatformLib::QueryHost());
    BeAssert(m_stateDb.IsDbOpen());
    BeAssert(Briefcase_IsInitialized());

    // ***
	// ***
	// *** DO NOT CHANGE THE ORDER OF THE STEPS IN THIS FUNCTION.
	// *** Talk to Sam Wilson if you need to make a change.
	// ***
	// ***
    DbResult dbres;

    // ***
    // *** TRICKY: Do not call InitBridge until we have done PullMergePush
    // ***          That is because the bridge may register a new or upgraded schema, which will require DgnDb::OpenDgnDb to do an import or upgrade.
    // ***          At that time, we must ensure that the BIM does not contain any lingering local Txns and that it is as the tip (so that we can get the schema lock).
    // ***          So, we need to be able to open the BIM just in order to pull/merge/push, before we allow the bridge to add a schema import/upgrade into the mix.
    // ***
    //  By getting the BIM to the tip, this initial pull also helps ensure that we will be able to get locks for the other changes that that bridge will make later.
    m_briefcaseDgnDb = DgnDb::OpenDgnDb(&dbres, m_briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    if (!m_briefcaseDgnDb.IsValid())
        return BentleyStatus::ERROR;

    if (BSISUCCESS != Briefcase_PullMergePush(""))  // *** WIP_BRIDGE: create revision comment from stored descriptions of local Txns?
        return RETURN_STATUS_SERVER_ERROR;

    m_briefcaseDgnDb->SaveChanges();
    Briefcase_ReleaseSharedLocks();
    m_briefcaseDgnDb = nullptr;

    // Now initialize the bridge.
    if (BSISUCCESS != InitBridge())
        return BentleyStatus::ERROR;

    // Open the briefcase
    // Note that iModelBridge::_Initialize may have registered new or changed domains, so we have to permit schema changes.
    bool madeSchemaChanges = false;
    m_briefcaseDgnDb = m_bridge->OpenBim(dbres, madeSchemaChanges);
    if (!m_briefcaseDgnDb.IsValid())
        {
        GetLogger().fatalv("OpenDgnDb failed with error %x", dbres);
        return BentleyStatus::ERROR;
        }

    if (madeSchemaChanges)
        {
        // We must isolate schema changes in their own revision, before we let the bridge move on to making data changes.
        m_briefcaseDgnDb->SaveChanges();
        if (BSISUCCESS != Briefcase_PullMergePush("schema changes"))
            return RETURN_STATUS_SERVER_ERROR;
        Briefcase_ReleaseSharedLocks();
        }

#ifdef COMMENT_OUT
    // if we acquire shared locks up front, then we can fail fast if we can't get them. However, we will also
    // incur a performance penalty, even if there are no updates
    // to any shared models. All in all, it's better to let the briefcase manager detect the need for
    // such locks, along with all other locking requirements, and then try to acquire them at the end.
    // Failure then will be more expensive -- we'll have to rollback and then retry the entire conversion later --
    // but this should be rare for a correctly written bridge that does not try to write to shared models.
    if (RepositoryStatus::Success != acquireSharedLocks(*m_briefcaseDgnDb))
        {
        m_briefcaseDgnDb = nullptr;
        return RETURN_STATUS_SERVER_ERROR;
        }
#endif

    //  Tell the bridge to update the BIM
    if (BSISUCCESS != m_bridge->DoConvertToExistingBim(*m_briefcaseDgnDb))
        {
        m_briefcaseDgnDb->AbandonChanges();
        m_briefcaseDgnDb = nullptr;
        return RETURN_STATUS_CONVERTER_ERROR;
        }

    dbres = m_briefcaseDgnDb->SaveChanges();

    if (BeSQLite::BE_SQLITE_OK != dbres)
        {
        m_briefcaseDgnDb->AbandonChanges();
        m_briefcaseDgnDb = nullptr;
        return RETURN_STATUS_LOCAL_ERROR;
        }

    //  PullMergePush
    //  Note: We may still be holding shared locks that we need to release. If we detect this, we must try again to release them.
    if (!anyTxnsInFile(*m_briefcaseDgnDb) && (SyncState::Initial == GetSyncState()))
        {
        GetLogger().info("No changes were detected and there are no Txns waiting to be pushed or shared locks to be released.\n");
        }
    else
        {
        BeAssert(!m_briefcaseDgnDb->Txns().HasChanges());

        if (BSISUCCESS != Briefcase_PullMergePush(m_jobEnvArgs.m_revisionComment.c_str()))
            return RETURN_STATUS_SERVER_ERROR;
        // (Retain shared locks, so that we can re-try our push later.)

        BeAssert(!anyTxnsInFile(*m_briefcaseDgnDb));

        if (BSISUCCESS != Briefcase_ReleaseSharedLocks())
            return RETURN_STATUS_SERVER_ERROR;
        }

    // If we got here, we completed the update and pushed it.

    // POST-CONDITIONS
    BeAssert((!anyTxnsInFile(*m_briefcaseDgnDb) && (SyncState::Initial == GetSyncState())) && "Local changes should have been pushed");
    // *** TBD: briefcase should not be holding any shared locks

    m_briefcaseDgnDb = nullptr;

    return RETURN_STATUS_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::RedirectStderr()
    {
    //m_stdoutFileName = m_stagingDir;
    //m_stdoutFileName.AppendToPath(L"stdout");

    m_stderrFileName = m_jobEnvArgs.m_stagingDir;
    m_stderrFileName.AppendToPath(L"stderr");

    //BeFileName::BeDeleteFile(m_stdoutFileName.c_str());
    BeFileName::BeDeleteFile(m_stderrFileName.c_str());

    //_wfreopen (m_stdoutFileName.c_str(), L"w+", stdout);
    freopen (Utf8String(m_stderrFileName).c_str(), "w+", stderr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::LogStderr()
    {
    fflush(stdout);
    fflush(stderr);

    WString wstr;
    readEntireFile(wstr, m_stderrFileName);
    if (!wstr.empty())
        GetLogger().errorv(L"%ls", wstr.c_str());

    wstr.clear();
    BeFileName issuesFileName(m_briefcaseName);
    issuesFileName.append(L"-issues");
    readEntireFile(wstr, issuesFileName);
    if (!wstr.empty())
        GetLogger().errorv(L"%ls", wstr.c_str());

//    BeFileName::BeDeleteFile(m_stdoutFileName.c_str());
    BeFileName::BeDeleteFile(m_stderrFileName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeFwk::iModelBridgeFwk() : m_repoAdmin(*this)
    {
    m_clientUtils = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeFwk::~iModelBridgeFwk()
    {
    Briefcase_Shutdown();
    LogStderr();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
WString iModelBridgeFwk::GetMutexName()
    {
    WString mname(m_jobEnvArgs.m_stagingDir);
    mname.ReplaceAll(L"\\", L"_");
    if (mname.length() > MAX_PATH)
        mname = mname.substr(mname.length() - (MAX_PATH-1));
    return mname;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::Run(int argc, WCharCP argv[])
    {
#ifdef _WIN32
    auto mutex = ::CreateMutexW(nullptr, false, GetMutexName().c_str());
    if (nullptr == mutex)
        {
        GetLogger().fatalv(L"%ls - cannot create mutex", GetMutexName().c_str());
        return -1;
        }
    HRESULT hr = ::WaitForSingleObject(mutex, s_maxWaitForMutex);
    if (WAIT_OBJECT_0 != hr)
        {
        if (WAIT_TIMEOUT == hr)
            GetLogger().fatalv(L"%ls - Another job is taking a long time. Try again later", GetMutexName().c_str());
        else
            GetLogger().fatalv(L"%ls - Error getting named mutex. Try again later", GetMutexName().c_str());
        return -1;
        }
#endif

    int res = -2;
    try
        {
        res = RunExclusive(argc, argv);
        }
    catch(...)
        {
#ifdef _WIN32
        ::ReleaseMutex(mutex);
        ::CloseHandle(mutex);
#endif
        }

    return res;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
#ifdef WIP_TEST
static DgnRevisionPtr createRevision(DgnDbR db)
    {
    DgnRevisionPtr revision = db.Revisions().StartCreateRevision();
    if (!revision.IsValid())
        return nullptr;

    RevisionStatus status = db.Revisions().FinishCreateRevision();
    if (RevisionStatus::Success != status)
        {
        BeAssert(false);
        return nullptr;
        }

    return revision;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName iModelBridgeFwk::QueryBridgeLibraryPathByName(uint64_t* rowid, WStringCR bridgeName)
    {
    auto stmt = m_stateDb.GetCachedStatement("SELECT ROWID, BridgeLibraryPath FROM fwk_InstalledBridges WHERE Name=?");
    stmt->BindText(1, Utf8String(bridgeName), Statement::MakeCopy::Yes);
    if (BE_SQLITE_ROW != stmt->Step())
        return BeFileName();

    if (nullptr != rowid)
        *rowid = stmt->GetValueInt64(0);
    return BeFileName(stmt->GetValueText(1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::QueryBridgeAssignedToDocument(BeFileNameR libPath, WStringR name, BeFileNameCR docName)
    {
    auto stmt = m_stateDb.GetCachedStatement("SELECT b.BridgeLibraryPath, b.Name FROM fwk_BridgeAssignments a, fwk_InstalledBridges b WHERE (a.SourceFile=?) AND (b.ROWID = a.Bridge)");
    stmt->BindText(1, Utf8String(docName), Statement::MakeCopy::Yes);
    if (BE_SQLITE_ROW != stmt->Step())
        return BSIERROR;

    libPath.SetNameUtf8(stmt->GetValueText(0));
    name.AssignUtf8(stmt->GetValueText(1));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
static CPLSpawnedProcess* findOrStartAffinityCalculator(BeFileNameCR affinityLibraryPath)
    {
    auto iFound = s_affinityCalculators.find(affinityLibraryPath);
    if (iFound != s_affinityCalculators.end())
        return iFound->second;

    auto calcexe = Desktop::FileSystem::GetExecutableDir();
    calcexe.AppendToPath(L"iModelBridgeGetAffinityHost" EXE_EXT);
    BeAssert(calcexe.DoesPathExist());

    Utf8String calcexeU(calcexe);
    Utf8String libU(affinityLibraryPath);
    libU.insert(0, "\"");
    libU.append("\"");
    const char* callData[] = { calcexeU.c_str(), libU.c_str(), nullptr };

    return s_affinityCalculators[affinityLibraryPath] = CPLSpawnAsync(NULL, callData, true, true, false, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void killAllAffinityCalculators()
    {
    for (auto i = s_affinityCalculators.begin(); i != s_affinityCalculators.end(); ++i)
        {
        CPLSpawnAsyncFinish(i->second, false, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::ComputeBridgeAffinityToDocument(iModelBridge::BridgeAffinity& affinity, BeFileNameCR affinityLibraryPath, BeFileNameCR filePath)
    {
    auto calc = findOrStartAffinityCalculator(affinityLibraryPath);
    if (nullptr == calc)
        {
        LOG.errorv(L"%ls - not found or could not be executed", affinityLibraryPath.c_str());
        return BSIERROR;
        }

    Utf8String filePathU(filePath);
    filePathU.append("\n");
    if (!CPLPipeWrite(CPLSpawnAsyncGetOutputFileHandle(calc), filePathU.c_str(), filePathU.size()))
        {
        LOG.errorv(L"%ls - has crashed", affinityLibraryPath.c_str());
        return BSIERROR;
        }

    auto responseHandle = CPLSpawnAsyncGetInputFileHandle(calc);

    // I expect exactly two lines of text: [0]affinity (integer) [1]bridge name (string)
    Utf8String line0, line1;
    if ((BSISUCCESS != CPLPipeReadLine(line0, responseHandle))
     || (BSISUCCESS != CPLPipeReadLine(line1, responseHandle)))
        {
        LOG.errorv(L"%ls - has crashed", affinityLibraryPath.c_str());
        return BSIERROR;
        }

    int v;
    if (1 != sscanf(line0.c_str(), "%d", &v))
        {
        LOG.errorv(L"%ls - \"%ls\" is an invalid affinity value?!", affinityLibraryPath.c_str(), WString(line0.c_str(), true).c_str());
        return BSIERROR;
        }

    affinity.m_affinity = (iModelBridge::Affinity)v;
    affinity.m_bridgeRegSubKey.AssignUtf8(line1.c_str());
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::SearchForBridgeToAssignToDocument(BeFileNameCR sourceFilePath)
    {
    BeFileName a; WString b;
    if (BSISUCCESS == QueryBridgeAssignedToDocument(a, b, sourceFilePath)) // If we have already assigned a bridge to this document, then stick with that.
        return BSISUCCESS;

    LOG.tracev(L"SearchForBridgeToAssignToDocument %ls", sourceFilePath.c_str());

    iModelBridge::BridgeAffinity bestBridge;

    auto iterateInstalled = m_stateDb.GetCachedStatement("SELECT BridgeLibraryPath, AffinityLibraryPath FROM fwk_InstalledBridges");
    while (BE_SQLITE_ROW == iterateInstalled->Step())
        {
        BeFileName bridgePath(iterateInstalled->GetValueText(0));
        BeFileName affinityLibraryPath(iterateInstalled->GetValueText(1));
        if (affinityLibraryPath.empty())
            affinityLibraryPath = bridgePath;
        if (affinityLibraryPath.empty())
            continue;

        iModelBridge::BridgeAffinity thisBridge;
        if (BSISUCCESS != ComputeBridgeAffinityToDocument(thisBridge, affinityLibraryPath, sourceFilePath))
            continue;

        LOG.tracev(L"%ls -> (%ls,%d)", affinityLibraryPath.c_str(), thisBridge.m_bridgeRegSubKey.c_str(), (int)thisBridge.m_affinity);

        if (thisBridge.m_affinity > bestBridge.m_affinity)
            bestBridge = thisBridge;
        }

    if ((bestBridge.m_bridgeRegSubKey.empty()) || (bestBridge.m_affinity == iModelBridge::Affinity::None))
        {
        LOG.infov(L"%ls - no installed bridge can convert this document", sourceFilePath.c_str());
        return BSIERROR;
        }

    uint64_t bestBridgeRowid = 0;
    QueryBridgeLibraryPathByName(&bestBridgeRowid, bestBridge.m_bridgeRegSubKey);
    BeAssert(0 != bestBridgeRowid);

    auto insertAssignment = m_stateDb.GetCachedStatement("INSERT INTO fwk_BridgeAssignments (SourceFile,Bridge) VALUES(?,?)");
    insertAssignment->BindText(1, Utf8String(sourceFilePath), Statement::MakeCopy::Yes);
    insertAssignment->BindInt64(2, bestBridgeRowid);
    auto rc = insertAssignment->Step();
    BeAssert(BE_SQLITE_DONE == rc);

    LOG.tracev(L"%ls := (%ls,%d)", sourceFilePath.c_str(), bestBridge.m_bridgeRegSubKey.c_str(), (int)bestBridge.m_affinity);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SearchForBridgesToAssignToDocumentsInDir(BeFileNameCR topDirIn)
    {
    BeFileName topDir(topDirIn);
    topDir.AppendSeparator();

    BeFileName entryName;
    bool        isDir;
    for (BeDirectoryIterator dirs (topDir); dirs.GetCurrentEntry (entryName, isDir) == SUCCESS; dirs.ToNext())
        {
        if (entryName.GetBaseName().EndsWithI(L".prp"))         // Filter out some control files that we know we should ignore
            continue;

        if (!isDir)
            SearchForBridgeToAssignToDocument(entryName);
        else
            SearchForBridgesToAssignToDocumentsInDir(entryName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SearchForBridgesToAssignToDocuments()
    {
    // There are no documents in the staging directory itself. The docs are all in subdirectories (where the
    // subdir name corresponds to the PW doc GUID).
    BeFileName entryName;
    bool        isDir;
    for (BeDirectoryIterator dirs (m_jobEnvArgs.m_stagingDir); dirs.GetCurrentEntry (entryName, isDir) == SUCCESS; dirs.ToNext())
        {
        if (isDir)
            SearchForBridgesToAssignToDocumentsInDir(entryName);
        }

    killAllAffinityCalculators();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::WriteBridgesFile()
    {
    BeFileName bridgesFileName(m_jobEnvArgs.m_stagingDir);
    bridgesFileName.AppendToPath(L"bridges.txt");
    BeFileStatus status;
    BeTextFilePtr bridgesFile = BeTextFile::Open(status, bridgesFileName.c_str(), TextFileOpenType::Write, TextFileOptions::KeepNewLine);
    if (!bridgesFile.IsValid())
        {
        LOG.fatalv(L"%ls - error writing bridges file", bridgesFileName.c_str());
        return BSIERROR;
        }
    auto stmt = m_stateDb.GetCachedStatement("SELECT DISTINCT b.Name, b.IsPowerPlatformBased, a.SourceFile FROM fwk_BridgeAssignments a, fwk_InstalledBridges b WHERE (b.ROWID = a.Bridge)");
    while (BE_SQLITE_ROW == stmt->Step())
        bridgesFile->PrintfTo(false, L"%ls;%ls;%d\n", WString(stmt->GetValueText(0), true).c_str(), WString(stmt->GetValueText(2), true).c_str(), stmt->GetValueBoolean(1));

    bridgesFile->Close();
    bridgesFile = nullptr;
    return BSISUCCESS;
    }

#ifdef _WIN32
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
LONG GetStringRegKey(WStringR strValue, HKEY hKey, WStringCR strValueName, WStringCR strDefaultValue)
    {
    strValue = strDefaultValue;
    WCHAR szBuffer[2048];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError;
    nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS == nError)
        {
        strValue = szBuffer;
        }
    return nError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool iModelBridgeFwk::QueryAnyInstalledBridges()
    {
    auto stmt = m_stateDb.GetCachedStatement("SELECT BridgeLibraryPath FROM fwk_InstalledBridges");
    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (!Utf8String::IsNullOrEmpty(stmt->GetValueText(0)))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::DiscoverInstalledBridges()
    {
    LOG.tracev(L"DiscoverInstalledBridges");

    HKEY iModelBridgesKey;
    long result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Bentley\\iModelBridges", 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE, &iModelBridgesKey);
    if (result != ERROR_SUCCESS)
        {
        LOG.info(L"HKLM\\Software\\Bentley\\iModelBridges not found -- no iModelBridges are installed on this machine.");
        return;
        }

    wchar_t iModelBridgeName[1024];
    DWORD iModelBridgeNameLen = _countof(iModelBridgeName), dwIdx = 0;
    DWORD index = 0;
    while ( ERROR_SUCCESS == RegEnumKeyExW(iModelBridgesKey, index, iModelBridgeName, &iModelBridgeNameLen, NULL, NULL, NULL, NULL) )
        {
        // iModelBridgeName buffer is contains key iModelBridgeName here
        HKEY subKey;
        if (ERROR_SUCCESS != RegOpenKeyExW(iModelBridgesKey, iModelBridgeName, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE, &subKey))
            continue;

        auto stmt = m_stateDb.GetCachedStatement("INSERT INTO fwk_InstalledBridges (Name,BridgeLibraryPath,AffinityLibraryPath,IsPowerPlatformBased) VALUES(?,?,?,?)");

        BeFileName bridgeLibraryPath, affinityLibraryPath;
        WString isPowerPlatformBased;
        GetStringRegKey(bridgeLibraryPath, subKey, L"BridgeLibraryPath", L"");
        GetStringRegKey(affinityLibraryPath, subKey, L"AffinityLibraryPath", L"");
        GetStringRegKey(isPowerPlatformBased, subKey, L"IsPowerPlatformBased", L"");

        stmt->BindText(1, Utf8String(iModelBridgeName).c_str(), Statement::MakeCopy::Yes);

        if (!bridgeLibraryPath.empty())
            stmt->BindText(2, Utf8String(bridgeLibraryPath).c_str(), Statement::MakeCopy::Yes);
        else
            stmt->BindNull(2);

        if (!affinityLibraryPath.empty())
            stmt->BindText(3, Utf8String(affinityLibraryPath).c_str(), Statement::MakeCopy::Yes);
        else
            stmt->BindNull(3);

        bool isPPBased = false;
        if (!isPowerPlatformBased.empty())
            {
            isPPBased= 0 == isPowerPlatformBased.CompareToI(L"True");
            stmt->BindBoolean(4, isPPBased);
            }
        else
            stmt->BindNull(4);

        stmt->Step(); // may fail with constraint if bridge library is already registered.

        // Get ready for the next subkey
        iModelBridgeNameLen = _countof(iModelBridgeName); // restore iModelBridgeNameLen after it is set to key's actual length by RegEnumKeyExW
        ++index; // increment subkey index

        LOG.tracev(L"%ls -> %ls %ls %d", iModelBridgeName, bridgeLibraryPath.c_str(), affinityLibraryPath.c_str(), isPPBased);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::FindBridgeInRegistry(BeFileNameR bridgeLibraryPath, BeFileNameR bridgeAssetsDir, WStringCR bridgeName)
    {
    WString bridgePath(L"Software\\Bentley\\iModelBridges");
    bridgePath.append(L"\\");
    bridgePath.append(bridgeName.c_str());

    HKEY iModelBridgeKey;
    long result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, bridgePath.c_str(), 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE, &iModelBridgeKey);
    if (result != ERROR_SUCCESS)
        return BSIERROR;

    GetStringRegKey(bridgeLibraryPath, iModelBridgeKey, L"BridgeLibraryPath", L"");
    GetStringRegKey(bridgeAssetsDir, iModelBridgeKey, L"BridgeAssetsDir", L"");

    return BSISUCCESS;
    }

#else
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::DiscoverInstalledBridges()
    {
    BeAssert(false && "TBD");
    }

BentleyStatus iModelBridgeFwk::FindBridgeInRegistry(BeFileNameR bridgeLibrary, BeFileNameR bridgeAssetsDir, WStringCR bridgeName)
    {
    BeAssert(false && "TBD");
    return BSIERROR;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::ComputeAffinityMain(int argc, WCharCP argv[])
    {
    iModelBridgeSacAdapter::InitCrt(false);

    if (argc != 2)
        {
        fprintf(stderr, "syntax: iModelBridgeGetAffinityHost affinityLibraryName\n");
        return -1;
        }

    BeFileName affinityLibraryPath(argv[1]);

    auto getAffinity = (T_iModelBridge_getAffinity*)GetBridgeFunction(affinityLibraryPath, "iModelBridge_getAffinity");
    if (nullptr == getAffinity)
        {
        LOG.errorv(L"%ls - does not export the iModelBridge_getAffinity function. That is probably an error.", affinityLibraryPath.c_str());
        return -1;
        }

    char filePathUtf8[MAX_PATH*2];
    while (fgets(filePathUtf8, sizeof(filePathUtf8), stdin))
        {
        BeFileName filePath(filePathUtf8, true);
        iModelBridge::BridgeAffinity bridge;
        getAffinity(bridge, affinityLibraryPath, filePath);
        fprintf(stdout, "%d\n%s\n", (int)bridge.m_affinity, Utf8String(bridge.m_bridgeRegSubKey).c_str());
        fflush(stdout);
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::AssignMain(int argc, WCharCP argv[])
    {
    iModelBridgeSacAdapter::InitCrt(false);

    Dgn::iModelBridgeFwk app;
    app.m_jobEnvArgs.m_bridgeRegSubKey = L"<dummy>";
    if (BSISUCCESS != app.ParseCommandLine(argc, argv))
        return -1;

    if (app.m_jobEnvArgs.m_stagingDir.empty() || !app.m_jobEnvArgs.m_stagingDir.DoesPathExist()
     || app.m_serverArgs.m_repositoryName.empty())
        {
        fwprintf(stderr, L"syntax: %ls --server-repository=<reponame> --fwk-staging-dir=<dirname>\n", argv[0]);
        return -1;
        }

    auto dbres = app.OpenOrCreateStateDb();
    if (BE_SQLITE_OK != dbres)
        return RETURN_STATUS_LOCAL_ERROR;

    app.DiscoverInstalledBridges();
    app.SearchForBridgesToAssignToDocuments();
    app.m_stateDb.SaveChanges();

    app.WriteBridgesFile();

    app.m_stateDb.CloseDb();

    return 0;
    }
