/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/iModelBridgeFwk.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
#include <Bentley/BeGetProcAddress.h>
#include <Logging/bentleylogging.h>
#include <iModelBridge/iModelBridgeBimHost.h>
#include <iModelBridge/iModelBridgeRegistry.h>
#include "../iModelBridgeHelpers.h"
#include <iModelBridge/Fwk/IModelClientForBridges.h>
#include <BentleyLog4cxx/log4cxx.h>

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

static iModelBridge* s_bridgeForTesting;
static IModelBridgeRegistry* s_registryForTesting;

static int s_maxWaitForMutex = 60000;

void iModelBridgeFwk::SetBridgeForTesting(iModelBridge& b)
    {
    s_bridgeForTesting = &b;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      10/17
//---------------------------------------------------------------------------------------
void* iModelBridgeFwk::GetBridgeFunction(BeFileNameCR bridgeDllName, Utf8CP funcName)
    {
    BeFileName pathname(BeFileName::FileNameParts::DevAndDir, bridgeDllName);

    BeGetProcAddress::SetLibrarySearchPath(pathname);
    auto hinst = BeGetProcAddress::LoadLibrary(bridgeDllName);
    if (!hinst)
        {
        LOG.fatalv(L"%ls: not found or could not be loaded", bridgeDllName.c_str());
        return nullptr;
        }

    return BeGetProcAddress::GetProcAddress (hinst, funcName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      10/17
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      10/17
//---------------------------------------------------------------------------------------
T_iModelBridge_releaseInstance* iModelBridgeFwk::JobDefArgs::ReleaseBridge()
    {
    auto getInstance = (T_iModelBridge_releaseInstance*) GetBridgeFunction(m_bridgeLibraryName, "iModelBridge_releaseInstance");
    if (!getInstance)
        {
        LOG.errorv(L"%ls: Does not export a function called 'iModelBridge_releaseInstance'", m_bridgeLibraryName.c_str());
        return nullptr;
        }

    return getInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
WString         iModelBridgeFwk::getArgValueW(WCharCP arg)
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
Utf8String      iModelBridgeFwk::getArgValue(WCharCP arg)
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
    IModelHubArgs::PrintUsage();
    IModelBankArgs::PrintUsage();
    DmsServerArgs::PrintUsage();
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
        L"--fwk-argsJson=             (optional)  Additional arguments in JSON format.\n"
        L"--fwk-max-wait=milliseconds (optional)  The maximum amount of time to wait for other instances of this job to finish. Default value is 60000ms\n"
        L"--fwk-jobrun-guid=          (optional)  A unique GUID that identifies this job run for activity tracking. This will be passed along to all dependant services and logs.\n"
        L"--fwk-assetsDir=            (optional)  Asset directory for the iModelBridgeFwk resources if default location is not suitable.\n"
        L"--fwk-bridgeAssetsDir=      (optional)  Asset directory for the iModelBridge resources if default location is not suitable.\n"
        L"--fwk-imodelbank-url=       (optional)  The URL of the iModelBank server to use. If none is provided, then iModelHub will be used.\n"
        L"--fwk-jobrequest-guid=      (optional)  A unique GUID that identifies this job run for correlation. This will be limited to the native callstack.\n"
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
        if (NULL == m_logProvider)
            m_logProvider = new NativeLogging::Provider::Log4cxxProvider();
        NativeLogging::LoggingConfig::ActivateProvider(m_logProvider);
        return;
        }

    fprintf(stderr, "Logging.config.xml not specified. Activating default logging using console provider.\n");
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity(L"iModelBridge", NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity(L"iModelBridgeFwk", NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity(L"iModelHub", NativeLogging::LOG_INFO);
    //NativeLogging::LoggingConfig::SetSeverity(L"Performance", NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity(L"ECDb", NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity(L"DgnCore", NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity(L"DgnV8Converter", NativeLogging::LOG_TRACE);
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
    fwkAssetsDirRaw.AppendToPath(L"Assets");                        // we want:         blah/iModelBridgeFwk/assets
    if (fwkAssetsDirRaw.DoesPathExist())
        return fwkAssetsDirRaw;

    //Try default assets folder relative the program directory
    fwkAssetsDirRaw = Desktop::FileSystem::GetExecutableDir();
    fwkAssetsDirRaw.AppendToPath(L"Assets");
    return fwkAssetsDirRaw;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeFwk::JobDefArgs::JobDefArgs()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      10/17
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

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-skip-assignment-check"))//undocumented, meant for pp based atp pupose
            {
            m_skipAssignmentCheck = true;
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

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-max-wait="))
            {
            wscanf(getArgValueW(argv[iArg]).c_str(), L"%d", &s_maxWaitForMutex);
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-argsJson="))
            {
            m_argsJson = Json::Value::From(getArgValue(argv[iArg]));
            continue;
            }
                
        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-bridgeAssetsDir="))
            {
            BeFileName assetsDir(getArgValueW(argv[iArg]));
            if (assetsDir.DoesPathExist())
                m_bridgeAssetsDir = assetsDir;
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-jobrun-guid="))
            {
            m_jobRunCorrelationId = getArgValue(argv[iArg]);
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-jobrequest-guid="))
            {
            m_jobRequestId = getArgValue(argv[iArg]);
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
// @bsimethod                                   Sam.Wilson                      10/17
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
BentleyStatus iModelBridgeFwk::ParseDocProps()
    {
    // The iModelBridge Job Configuration UI allows the user to specify things like the transform for a root model.
    // Merge that data into the params, so that it is passed through to the bridge like any other data.

    if (m_jobEnvArgs.m_inputFileName.empty())
        return BSISUCCESS;

    iModelBridgeDocumentProperties docProps;

    try
        {
        _GetDocumentProperties(docProps, m_jobEnvArgs.m_inputFileName);
        }
    catch (const std::exception& ex)
        {
        fprintf(stdout, "ERROR - %s\n", ex.what());
        return BSIERROR;
        }
    catch (const std::string& exdesc)
        {
        fprintf(stdout, "ERROR - %s\n", exdesc.c_str());
        return BSIERROR;
        }
    catch (...)
        {
        fwprintf(stdout, L"Unknown exception thrown from iModelBridgeRegistry!");
        return BSIERROR;
        }

    if (docProps.m_spatialRootTransformJSON.empty())
        return BSISUCCESS;

    Json::Value jsonValue = Json::Value::From(docProps.m_spatialRootTransformJSON);
    if (jsonValue.isNull())
        {
        fprintf(stderr, "%s - invalid JSON syntax\n", docProps.m_spatialRootTransformJSON.c_str());
        return BSIERROR;
        }

    m_jobEnvArgs.m_argsJson = jsonValue;

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

    bvector<WCharCP> serverRawArgPtrs;        // pare down the args once again, removing the server-specific args and leaving the rest for the bridge
    std::swap(serverRawArgPtrs, m_bargptrs);

    m_bargptrs.push_back(argv[0]);

    IModelBankArgs bankArgs;
    if ((BSISUCCESS != bankArgs.ParseCommandLine(m_bargptrs, (int) serverRawArgPtrs.size(), serverRawArgPtrs.data())) || (BSISUCCESS != bankArgs.Validate((int) serverRawArgPtrs.size(), serverRawArgPtrs.data())))
        {
        PrintUsage(argv[0]);
        return BSIERROR;
        }
    
    if (!bankArgs.ParsedAny())
        {
        m_bargptrs.clear();
        m_bargptrs.push_back(argv[0]);
        }

    IModelHubArgs hubArgs;
    if ((BSISUCCESS != hubArgs.ParseCommandLine(m_bargptrs, (int) serverRawArgPtrs.size(), serverRawArgPtrs.data())) || (BSISUCCESS != hubArgs.Validate((int) serverRawArgPtrs.size(), serverRawArgPtrs.data())))
        {
        PrintUsage(argv[0]);
        return BSIERROR;
        }

    if (bankArgs.ParsedAny() && hubArgs.ParsedAny())
        {
        fwprintf (stderr, L"Specify either imodel-bank arguments or server arguments but not both\n");
        return BSIERROR;
        }

    bool dmsCredentialsAreEncrypted = false;
    if (bankArgs.ParsedAny())
        {
        m_useIModelHub = false;
        m_iModelBankArgs = new IModelBankArgs(bankArgs);
        m_briefcaseBasename = m_iModelBankArgs->GetBriefcaseBasename();
        m_maxRetryCount = m_iModelBankArgs->m_maxRetryCount;
        dmsCredentialsAreEncrypted = m_iModelBankArgs->m_dmsCredentialsEncrypted;
        }
    else
        {
        m_useIModelHub = true;
        m_iModelHubArgs = new IModelHubArgs(hubArgs);
        m_briefcaseBasename = m_iModelHubArgs->m_repositoryName;
        m_maxRetryCount = m_iModelHubArgs->m_maxRetryCount;
        dmsCredentialsAreEncrypted = m_iModelHubArgs->m_isEncrypted;
        }

    bvector<WCharCP> dmsRawArgPtrs;        // pare down the args once again, removing the dms server-specific args and leaving the rest for the bridge
    std::swap(dmsRawArgPtrs, m_bargptrs);

    m_bargptrs.push_back(argv[0]);

    if ((BSISUCCESS != m_dmsServerArgs.ParseCommandLine(m_bargptrs, (int) dmsRawArgPtrs.size(), dmsRawArgPtrs.data(), dmsCredentialsAreEncrypted)) || (BSISUCCESS != m_dmsServerArgs.Validate((int) dmsRawArgPtrs.size(), dmsRawArgPtrs.data())))
        {
        PrintUsage(argv[0]);
        return BSIERROR;
        }

    // Now that we have the server arguments (including the repository name), we can access and parse the arguments that are parked in the registry db
    if (BSISUCCESS != ParseDocProps())
        return BSIERROR;

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
    GetLogger().infov("bridge:%s iModel:%s - acquiring briefcase.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());
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
        ReportIssue("iModel not found"); // *** WIP translate
        GetLogger().fatalv("%s - repository not found in project. Create option was not specified.\n", m_briefcaseBasename.c_str());
        return BSIERROR;
        }

    //
    //  We need to create a new repository.
    //

    //  Initialize the bridge to do the creation.
    //  Note: iModelBridge::_Initialize will register domains, and some of them may be required.
    if (BSISUCCESS != InitBridge())
        return BentleyStatus::ERROR;
    if (true)
        {
        iModelBridgeCallTerminate callTerminate(*m_bridge);

        //  Create a new repository. (This will import all required domains and their schemas.)
        m_briefcaseDgnDb = m_bridge->DoCreateDgnDb(m_modelsInserted, nullptr);
        if (!m_briefcaseDgnDb.IsValid())
            {
            // Hopefully, this is a recoverable error. Stay in Initial state, so that we can try again.
            return BSIERROR;
            }
        callTerminate.m_status = BSISUCCESS;
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
BentleyStatus iModelBridgeFwk::IModelHub_DoCreatedLocalDb()
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

    if (BSISUCCESS != Briefcase_IModelHub_CreateRepository())
        return BSIERROR;

    SetState(BootstrappingState::CreatedRepository);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::IModelHub_DoCreatedRepository()
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
        ReportIssue("iModel create failed"); // *** WIP translate
        GetLogger().fatalv("CreateRepository failed or acquireBriefcase failed for repositoryname=%s, stagingdir=%s",
                            m_briefcaseBasename.c_str(), Utf8String(m_jobEnvArgs.m_stagingDir).c_str());
        return BSIERROR;
        }

    SetState(BootstrappingState::NewBriefcaseNeedsLocks);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::IModelHub_DoNewBriefcaseNeedsLocks()
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
            case BootstrappingState::CreatedLocalDb:            status = IModelHub_DoCreatedLocalDb(); createdNewRepo = true; break;
            case BootstrappingState::CreatedRepository:         status = IModelHub_DoCreatedRepository(); break;
            case BootstrappingState::NewBriefcaseNeedsLocks:    status = IModelHub_DoNewBriefcaseNeedsLocks(); break;
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
void iModelBridgeFwk::SetBridgeParams(iModelBridge::Params& params, FwkRepoAdmin* ra)
    {
    Briefcase_MakeBriefcaseName(); // => defines m_briefcaseName
    params.m_briefcaseName = m_briefcaseName;
    params.SetReportFileName();
    params.GetReportFileName().BeDeleteFile();
    params.m_assetsDir = m_jobEnvArgs.m_bridgeAssetsDir;
    params.m_libraryDir = m_jobEnvArgs.m_bridgeLibraryName.GetDirectoryName();
    params.m_repoAdmin = ra;
    params.m_inputFileName = m_jobEnvArgs.m_inputFileName;
    if (!m_jobEnvArgs.m_drawingAndSheetFiles.empty())
        params.m_drawingAndSheetFiles = m_jobEnvArgs.m_drawingAndSheetFiles;
    else
        {
        GetRegistry()._QueryAllFilesAssignedToBridge(params.m_drawingAndSheetFiles, m_jobEnvArgs.m_bridgeRegSubKey.c_str());
        }
    if (!m_jobEnvArgs.m_skipAssignmentCheck)
        params.SetDocumentPropertiesAccessor(*this);
    params.SetBridgeRegSubKey(m_jobEnvArgs.m_bridgeRegSubKey);
    params.ParseJsonArgs(m_jobEnvArgs.m_argsJson, true);
    params.m_jobRunCorrelationId = m_jobEnvArgs.m_jobRunCorrelationId;
    //Set up Dms files would have loaded the DMS accesor. Set it on the params for the Dgnv8 Bridge
    params.m_dmsSupport = m_dmsSupport;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::LoadBridge()
    {
    if (s_bridgeForTesting)
        {
        m_bridge = s_bridgeForTesting;
        return BentleyStatus::SUCCESS;
        }

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
* @bsimethod                                    Abeesh.Basheer                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::ReleaseBridge()
    {
    if (nullptr == m_bridge)
        return BentleyStatus::SUCCESS;

    auto releaseFunc = m_jobEnvArgs.ReleaseBridge();
    if (nullptr == releaseFunc)
        {
        return BentleyStatus::ERROR;
        }

    BentleyStatus status = releaseFunc(m_bridge);
    m_bridge = NULL;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::InitBridge()
    {
    GetLogger().infov("bridge:%s iModel:%s - Initializing bridge.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());

    SetBridgeParams(m_bridge->_GetParams(), m_repoAdmin);

    if (BentleyStatus::SUCCESS != m_bridge->_ParseCommandLine((int)m_bargptrs.size(), m_bargptrs.data()))
        {
        fprintf(stderr, "bridge _ParseCommandLine failed\n");
        m_bridge->_PrintUsage();
        return BentleyStatus::ERROR;
        }

    SetBridgeParams(m_bridge->_GetParams(), m_repoAdmin);    // make sure that MY definition of these params is used!

    if (BSISUCCESS != m_bridge->_Initialize((int)m_bargptrs.size(), m_bargptrs.data()))
        return BentleyStatus::ERROR;

    BeAssert((m_bridge->_GetParams().GetRepositoryAdmin() == m_repoAdmin) && "Bridge must use the RepositoryAdmin that the fwk supplies");

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
DbResult iModelBridgeFwk::SaveBriefcaseId()
    {
    DbResult rc;
    auto db = DgnDb::OpenDgnDb(&rc, m_briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    if (!db.IsValid())
        {
        return rc;
        }
    uint32_t bcid = db->GetBriefcaseId().GetValue();
    m_stateDb.SaveProperty(s_briefcaseIdPropSpec, &bcid, sizeof(bcid));
    m_stateDb.SaveChanges();
    return BE_SQLITE_OK;
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

struct HostTerminator
    {
    ~HostTerminator()
        {
        DgnViewLib::GetHost().Terminate(true);
        iModelBridge::L10N::Terminate();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct LoggingContext //This class allows to pass in a logging sequence id to rest of the callers for a bridge job run
    {
    private:
    WString                                      m_jobRunCorrelationId;
    WString                                      m_jobRequestId;
    NativeLogging::Provider::Log4cxxProvider*    m_provider;
    WString                                      m_connectProjectId;
    WString                                      m_iModelId;
    WString                                      m_bridgeName;
    public:
    LoggingContext(iModelBridgeFwk::JobDefArgs const& jobDef, Utf8StringCR projectId, Utf8StringCR iModelId, NativeLogging::Provider::Log4cxxProvider* provider)
        :m_jobRunCorrelationId(jobDef.m_jobRunCorrelationId.c_str(), true), m_provider(provider), m_connectProjectId(projectId.c_str(), true), m_iModelId(iModelId.c_str(), true),
        m_jobRequestId(jobDef.m_jobRunCorrelationId.c_str(), true), m_bridgeName(jobDef.m_bridgeRegSubKey)
        {
        if (NULL == m_provider)
            return;
        if (!m_jobRunCorrelationId.empty())
            m_provider->AddContext(L"ActivityId", m_jobRunCorrelationId.c_str());
        if (!m_jobRequestId.empty())
            m_provider->AddContext(L"RequestId", m_jobRequestId.c_str());
        if(!m_connectProjectId.empty())
            m_provider->AddContext(L"ConnectProjectId", m_connectProjectId.c_str());
        if (!m_iModelId.empty())
            m_provider->AddContext(L"iModelId", m_iModelId.c_str());
        if (!m_bridgeName.empty())
            m_provider->AddContext(L"iModelBridgeName", m_bridgeName.c_str());
        }

    ~LoggingContext()
        {
        if (NULL == m_provider)
            return;
        if (!m_jobRequestId.empty())
            m_provider->RemoveContext(L"RequestId");
        if (!m_jobRunCorrelationId.empty())
            m_provider->RemoveContext(L"ActivityId");
        if (!m_jobRequestId.empty())
            m_provider->RemoveContext(L"ConnectProjectId");
        if (!m_jobRunCorrelationId.empty())
            m_provider->RemoveContext(L"iModelId");
        if (!m_bridgeName.empty())
            m_provider->RemoveContext(L"iModelBridgeName");
        }
    };

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
    StopWatch setUpTimer(true);
    Utf8String connectProjectId, iModelId;
    if (m_useIModelHub && NULL != m_iModelHubArgs)
        {
        iModelId = m_iModelHubArgs->m_repositoryName;
        connectProjectId = m_iModelHubArgs->m_bcsProjectId;
        }
    else if (NULL != m_iModelBankArgs)
        {
        iModelId = m_iModelBankArgs->m_iModelId;
        }
    LoggingContext logContext(m_jobEnvArgs, connectProjectId, iModelId, m_logProvider);

    DbResult dbres;

    iModelBridgeSacAdapter::InitCrt(false);

    Briefcase_MakeBriefcaseName();
    BeFileName::BeDeleteFile(ComputeReportFileName(m_briefcaseName));  // delete any old issues file hanging round from the previous run

    //  Open our state db.
    dbres = OpenOrCreateStateDb();
    if (BE_SQLITE_OK != dbres)
        {
        LOG.fatal("OpenOrCreateStateDb failed");
        return RETURN_STATUS_LOCAL_ERROR;
        }

    //  Resolve the bridge to run
    if (m_jobEnvArgs.m_bridgeLibraryName.empty())
        {
        GetRegistry()._FindBridgeInRegistry(m_jobEnvArgs.m_bridgeLibraryName, m_jobEnvArgs.m_bridgeAssetsDir, m_jobEnvArgs.m_bridgeRegSubKey);
        if (m_jobEnvArgs.m_bridgeLibraryName.empty())
            {
            LOG.fatalv(L"%s - no bridge with this subkey is in the state db", m_jobEnvArgs.m_bridgeRegSubKey.c_str());
            return RETURN_STATUS_LOCAL_ERROR;
            }
        }

    if (m_jobEnvArgs.m_bridgeAssetsDir.empty())
        m_jobEnvArgs.m_bridgeAssetsDir = findBridgeAssetsDir(m_jobEnvArgs.m_bridgeLibraryName.GetDirectoryName());

    // Put out this info message, so that we can relate all subsequent logging messages to this bridge.
    // The log on this machine may have messages from many bridge jobs.
    LOG.infov(L"Running bridge [%ls] staging dir [%ls] input [%ls]", m_jobEnvArgs.m_bridgeLibraryName.c_str(),
              m_jobEnvArgs.m_stagingDir.c_str(), m_jobEnvArgs.m_inputFileName.c_str());

    // Load the bridge ... but don't initialize it.
    // *** TRICKY: Do not call InitBridge until AFTER we acquire the briefcase (or decide to create a new DgnDb).
    if (BentleyStatus::SUCCESS != LoadBridge())
        return RETURN_STATUS_CONVERTER_ERROR;

    // Initialize the DgnViewLib Host.
    m_repoAdmin = new FwkRepoAdmin(*this);  // TRICKY: This is ultimately passed to the host as a host variable, and host terimation will delete it.
    iModelBridge::Params params;
    SetBridgeParams(params, m_repoAdmin);

    BeFileName fwkAssetsDir(m_jobEnvArgs.m_fwkAssetsDir);
    BeFileName fwkDb3 = fwkAssetsDir;
    fwkDb3.AppendToPath(L"sqlang");
    fwkDb3.AppendToPath(L"iModelBridgeFwk_en-US.sqlang.db3");

    Dgn::iModelBridgeBimHost host(m_repoAdmin, fwkAssetsDir, fwkDb3, Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str());
    DgnViewLib::Initialize(host, true);

    //  Initialize the bridge-specific L10N
    BeFileName bridgeSqlangPath(params.GetAssetsDir());
    bridgeSqlangPath.AppendToPath(m_bridge->_SupplySqlangRelPath().c_str());
    iModelBridge::L10N::Initialize(BeSQLite::L10N::SqlangFiles(bridgeSqlangPath));

    HostTerminator terminateHostOnReturn;

    static PrintfProgressMeter s_meter;
    T_HOST.SetProgressMeter(&s_meter);

    LogPerformance(setUpTimer, "Initialized iModelBridge Fwk");
    
    LOG.tracev(L"Logging into iModel Hub");
    {
    StopWatch iModelHubSignIn(true);
    //  Sign into the iModelHub
    if (BSISUCCESS != Briefcase_Initialize(argc, argv))
        return RETURN_STATUS_SERVER_ERROR;

    LogPerformance(setUpTimer, "Logging into iModelHub");
    }
    LOG.tracev(L"Logging into iModel Hub : Done");

    // Stage the workspace and input file if  necessary.
    LOG.tracev(L"Setting up workspace for standalone bridges");
    if (BSISUCCESS != SetupDmsFiles())
        return RETURN_STATUS_SERVER_ERROR;

    LOG.tracev(L"Setting up workspace for standalone bridges  : Done");

    //  Make sure we have a briefcase.
    Briefcase_MakeBriefcaseName(); // => defines m_briefcaseName
    {
    LOG.tracev(L"Setting up iModel Briefcase for processing");
    StopWatch briefcaseTime(true);

    bool createdNewRepo = false;
    if (BSISUCCESS != BootstrapBriefcase(createdNewRepo))
        {
        m_briefcaseDgnDb = nullptr;
        return RETURN_STATUS_SERVER_ERROR;
        }

    //  "Bootstrapping" the briefcase might have created a new repository. In that case, there is no need to go through the update logic and pullmergepush.
    if (createdNewRepo)
        return RETURN_STATUS_SUCCESS;

    LogPerformance(setUpTimer, "Getting iModel Briefcase from iModelHub");
    LOG.tracev(L"Setting up iModel Briefcase for processing  : Done");
    }
    
    //  The repo already exists. Run the bridge to update it and then push the changeset to the iModel.
    int status;
    try
        {
        status = UpdateExistingBim();
        }
    catch (...)
        {
        LOG.fatal("UpdateExistingBim failed");
        status = RETURN_STATUS_LOCAL_ERROR;
        }

    s_meter.Hide();

    if (m_briefcaseDgnDb.IsValid())     // must make sure briefcase dgndb is closed before tearing down host!
        {
        Briefcase_ReleaseAllPublicLocks();  // regardless of the success or failure of the bridge, we must not hold onto any locks that are not the private property of the bridge

        if (BSISUCCESS != status)
            m_briefcaseDgnDb->AbandonChanges();

        m_briefcaseDgnDb = nullptr;
        }

    //We are done processing the dgn db file. Release the bridge
    if (SUCCESS != ReleaseBridge())
        LOG.errorv(L"%s - Memory leak. This bridge was not released properly.", m_jobEnvArgs.m_bridgeRegSubKey.c_str());

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::ProcessSchemaChange()
    {
    GetLogger().infov("bridge:%s iModel:%s - Processing schema change.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());

    //  Push the pending schema change to iModelHub in its own changeset
    m_briefcaseDgnDb->SaveChanges();
    if (BSISUCCESS != Briefcase_PullMergePush("schema changes"))
        return RETURN_STATUS_SERVER_ERROR;
    Briefcase_ReleaseAllPublicLocks();

    // >------> pullmergepush *may* have pulled schema changes -- close and re-open the briefcase in order to merge them in <-----------<

    DbResult dbres;
    bool madeSchemaChanges = false;
    m_briefcaseDgnDb = nullptr; // close the current connection to the briefcase db before attempting to reopen it!
    m_briefcaseDgnDb = iModelBridge::OpenBimAndMergeSchemaChanges(dbres, madeSchemaChanges, m_briefcaseName);
    if (!m_briefcaseDgnDb.IsValid())
        {
        ReportIssue(BeSQLite::Db::InterpretDbResult(dbres));
        GetLogger().fatalv("OpenDgnDb failed with error %s (%x)", BeSQLite::Db::InterpretDbResult(dbres), dbres);
        return BentleyStatus::ERROR;
        }

    BeAssert(!madeSchemaChanges && "No further domain schema changes were expected.");
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String   iModelBridgeFwk::GetRevisionComment()
    {
    //Revision comment override from command line has the first priority
    Utf8String trunCastedCommentString;
    //if (!m_jobEnvArgs.m_revisionComment.empty())
        trunCastedCommentString = m_jobEnvArgs.m_revisionComment;

    //See TFS#819945: IMBridgeFwk - must truncate changeset description < 400 chars
    if (trunCastedCommentString.size() > 399)
        trunCastedCommentString.erase(399, std::string::npos);
    return trunCastedCommentString;
    /* Disabled until iModelHub changeset suppports a json blob property for the info below.
    bvector<BeFileName> inputFiles;
    GetRegistry()._QueryAllFilesAssignedToBridge(inputFiles, m_jobEnvArgs.m_bridgeRegSubKey.c_str());

    if (0 == inputFiles.size())
        return Utf8String();

    Json::Value auditArray;
    for (BeFileNameCR file : inputFiles)
        {
        iModelBridgeDocumentProperties prop;
        if (SUCCESS != GetRegistry()._GetDocumentProperties(prop, file))
            continue;

        if (prop.m_changeHistoryJSON.empty())
            continue;

        Json::Value auditLogs = Json::Value::From(prop.m_changeHistoryJSON);
        if (auditLogs.isNull())
            continue;

        auditArray.append(auditLogs);
        }
    return auditArray.ToString();
    */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::TryOpenBimWithBisSchemaUpgrade()
    {
    GetLogger().trace("Entering TryOpenBimWithBisSchemaUpgrade");
    bool madeSchemaChanges = false;
    DbResult dbres;
    m_briefcaseDgnDb = iModelBridge::OpenBimAndMergeSchemaChanges(dbres, madeSchemaChanges, m_briefcaseName);
    uint8_t retryopenII = 0;
    while (!m_briefcaseDgnDb.IsValid() && (DbResult::BE_SQLITE_ERROR_SchemaUpgradeFailed == dbres) && (++retryopenII < m_maxRetryCount) && IModelClientBase::SleepBeforeRetry())
        {
        // The upgrade may have failed because we could not get the schema lock, and that may have failed because
        // another briefcase pushed schema changes after this briefcase last pulled.
        // If so, then we have to pull before re-trying.
        GetLogger().infov("SchemaUpgrade failed. Pulling.");
        DgnDb::OpenParams oparams(DgnDb::OpenMode::ReadWrite);
        oparams.GetSchemaUpgradeOptionsR().SetUpgradeFromDomains(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck);
        m_briefcaseDgnDb = DgnDb::OpenDgnDb(&dbres, m_briefcaseName, oparams);
        Briefcase_PullMergePush("");    // TRICKY Only Briefcase_PullMergePush contains the mergeschemachanges logic. Briefcase_PullAndMerge does not.
        m_briefcaseDgnDb = nullptr;

        GetLogger().infov("Retrying SchemaUpgrade (if still necessary).");
        m_briefcaseDgnDb = iModelBridge::OpenBimAndMergeSchemaChanges(dbres, madeSchemaChanges, m_briefcaseName);
        }
    if (!m_briefcaseDgnDb.IsValid())
        {
        ReportIssue(BeSQLite::Db::InterpretDbResult(dbres));
        GetLogger().fatalv("OpenDgnDb failed with error %s (%x)", BeSQLite::Db::InterpretDbResult(dbres), dbres);
        return BentleyStatus::ERROR;
        }
    //                                       *** NB: CALLER CLEANS UP m_briefcaseDgnDb! ***
    if (madeSchemaChanges)
        {
        if (0 != ProcessSchemaChange())  // pullmergepush + re-open
            return BSIERROR;
        }
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
//! Method to store common information applicable to all bridges.
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int             iModelBridgeFwk::StoreHeaderInformation()
    {
    BeGuid guid = m_briefcaseDgnDb->QueryProjectGuid();
    if (guid.IsValid())
        return SUCCESS;

    if (!m_useIModelHub || NULL == m_iModelHubArgs)
        return SUCCESS;

    BeGuid connectProjectId;
    if (SUCCESS != connectProjectId.FromString(m_iModelHubArgs->m_bcsProjectId.c_str()))
        return ERROR;
    
    m_briefcaseDgnDb->SaveProjectGuid(connectProjectId);
    return SUCCESS;
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
    
    
    //                                      ************************************************
    //                                       *** NB: CALLER CLEANS UP m_briefcaseDgnDb! ***
    //                                      ************************************************
    //                                      
    //                            *** Do not close it. Do not set it to null                            ***
    //                            *** Even in case of error, do not attempt to clean up m_briefcaseDgnDb. ***
    //                            *** The caller does all cleanup, including releasing all public locks. ***

    GetLogger().infov("bridge:%s iModel:%s - Opening briefcase I.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());

    // ***
    // *** TRICKY: Do not call InitBridge until we have done PullMergePush
    // ***          That is because the bridge may register a new or upgraded schema, which will require DgnDb::OpenDgnDb to do an import or upgrade.
    // ***          At that time, we must ensure that the BIM does not contain any lingering local Txns and that it is as the tip (so that we can get the schema lock).
    // ***          So, we need to be able to open the BIM just in order to pull/merge/push, before we allow the bridge to add a schema import/upgrade into the mix.
    // ***
    //  By getting the BIM to the tip, this initial pull also helps ensure that we will be able to get locks for the other changes that that bridge will make later.
    LOG.tracev(L"TryOpenBimWithBisSchemaUpgrade");
    if (BSISUCCESS != TryOpenBimWithBisSchemaUpgrade())
        return BentleyStatus::ERROR;
    LOG.tracev(L"TryOpenBimWithBisSchemaUpgrade  : Done");

    if (BSISUCCESS != Briefcase_PullMergePush(""))
        return RETURN_STATUS_SERVER_ERROR;

    //                                       *** NB: CALLER CLEANS UP m_briefcaseDgnDb! ***

    // >------> pullmergepush *may* have pulled schema changes -- close and re-open the briefcase in order to merge them in <-----------<

    m_briefcaseDgnDb->SaveChanges();
    Briefcase_ReleaseAllPublicLocks();
    m_briefcaseDgnDb = nullptr;             // This is safe, because we released all locks.

    // Now initialize the bridge.
    if (BSISUCCESS != InitBridge())
        return BentleyStatus::ERROR;

    if (true)
        {
        iModelBridgeCallTerminate callTerminate(*m_bridge);

        GetLogger().infov("bridge:%s iModel:%s - Opening briefcase II.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());

        // Open the briefcase in the normal way, allowing domain schema changes to be pulled in.
        bool madeSchemaChanges = false;
        m_briefcaseDgnDb = nullptr; // close the current connection to the briefcase db before attempting to reopen it!
        
        if (BSISUCCESS != TryOpenBimWithBisSchemaUpgrade())
            return BentleyStatus::ERROR;

        BeAssert(!anyTxnsInFile(*m_briefcaseDgnDb));

        //  Tell the bridge that the briefcase is now open and ask it to open the source file(s).
        iModelBridgeCallOpenCloseFunctions callCloseOnReturn(*m_bridge, *m_briefcaseDgnDb);
        if (!callCloseOnReturn.IsReady())
            {
            LOG.fatalv("Bridge is not ready or could not open source file");
            return BentleyStatus::ERROR;
            }

        m_briefcaseDgnDb->SaveChanges(); // If the _OnOpenBim or _OpenSource callbacks did things like attaching syncinfo, we need to commit that before going on.
                                        // This also prevents a call to AbandonChanges in _MakeSchemaChanges from undoing what the open calls did.


        //                                       *** NB: CALLER CLEANS UP m_briefcaseDgnDb! ***

        if (m_briefcaseDgnDb->Txns().HasChanges() || anyTxnsInFile(*m_briefcaseDgnDb)) // if bridge made any changes, they must be pushed and cleared out before we can make schema changes
            {
            if (BSISUCCESS != Briefcase_PullMergePush("initialization changes"))
                return RETURN_STATUS_SERVER_ERROR;
            Briefcase_ReleaseAllPublicLocks();
            }

        //                                       *** NB: CALLER CLEANS UP m_briefcaseDgnDb! ***

        //  Let the bridge generate schema changes
        GetLogger().infov("bridge:%s iModel:%s - MakeSchemaChanges.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());

        int bridgeSchemaChangeStatus = m_bridge->_MakeSchemaChanges();
        if (BSISUCCESS != bridgeSchemaChangeStatus)
            {
            uint8_t retryAttempt = 0;
            while ((BSISUCCESS != bridgeSchemaChangeStatus) && (++retryAttempt < m_maxRetryCount) && IModelClientBase::SleepBeforeRetry())
                {
                GetLogger().infov("_MakeSchemaChanges failed. Retrying.");
                callCloseOnReturn.CallCloseFunctions(); // re-initialize the bridge, to clear out the side-effects of the previous failed attempt
                m_briefcaseDgnDb->AbandonChanges();
                if (BSISUCCESS != ProcessSchemaChange())    // make sure that we are at the tip and that we have absorbed any schema changes from the server
                    return RETURN_STATUS_SERVER_ERROR;
                callCloseOnReturn.CallOpenFunctions(*m_briefcaseDgnDb);
                bridgeSchemaChangeStatus = m_bridge->_MakeSchemaChanges();
                }
            }
        if (BSISUCCESS != bridgeSchemaChangeStatus)
            {
            LOG.fatalv("Bridge _MakeSchemaChanges failed");
            return BentleyStatus::ERROR;
            }

        madeSchemaChanges = m_briefcaseDgnDb->Txns().HasChanges() || anyTxnsInFile(*m_briefcaseDgnDb); // see if bridge actually made any changes
        if (madeSchemaChanges)
            {
            callCloseOnReturn.CallCloseFunctions();
            if (0 != ProcessSchemaChange())  // pullmergepush + re-open
                return BSIERROR;
            callCloseOnReturn.CallOpenFunctions(*m_briefcaseDgnDb);
            }

        //                                       *** NB: CALLER CLEANS UP m_briefcaseDgnDb! ***

        BeAssert(!anyTxnsInFile(*m_briefcaseDgnDb));
        GetLogger().tracev("bridge:%s iModel:%s - Storing iModel Bridge Header Data.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());
        if (SUCCESS != StoreHeaderInformation())
            GetLogger().warningv("bridge:%s iModel:%s - Storing iModel Bridge Header Data Failed.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());

        //  Now, finally, we can convert data
        GetLogger().infov("bridge:%s iModel:%s - Convert Data.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());

        BentleyStatus bridgeCvtStatus = m_bridge->DoConvertToExistingBim(*m_briefcaseDgnDb, true);
    
        if (BSISUCCESS != bridgeCvtStatus)
            {
            GetLogger().errorv("Bridge::DoConvertToExistingBim failed with status %d", bridgeCvtStatus);
            return RETURN_STATUS_CONVERTER_ERROR;
            }

        callTerminate.m_status = callCloseOnReturn.m_status = BSISUCCESS;
        }

    DbResult dbres = m_briefcaseDgnDb->SaveChanges();

    //                                       *** NB: CALLER CLEANS UP m_briefcaseDgnDb! ***

    if (BeSQLite::BE_SQLITE_OK != dbres)
        {
        GetLogger().errorv("Db::SaveChanges failed with status %d", dbres);
        return RETURN_STATUS_LOCAL_ERROR;
        }

    //  PullMergePush
    //  Note: We may still be holding shared locks that we need to release. If we detect this, we must try again to release them.
    if (!anyTxnsInFile(*m_briefcaseDgnDb) && (SyncState::Initial == GetSyncState()))
        {
        GetLogger().info("No changes were detected and there are no Txns waiting to be pushed or shared locks to be released.");
        }
    else
        {
        BeAssert(!m_briefcaseDgnDb->Txns().HasChanges());

        GetLogger().infov("bridge:%s iModel:%s - Pushing Data Changeset.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());

        if (BSISUCCESS != Briefcase_PullMergePush(GetRevisionComment().c_str()))
            return RETURN_STATUS_SERVER_ERROR;
        // (Retain shared locks, so that we can re-try our push later.)

        BeAssert(!anyTxnsInFile(*m_briefcaseDgnDb));

        if (BSISUCCESS != Briefcase_ReleaseAllPublicLocks())
            return RETURN_STATUS_SERVER_ERROR;
        }

    //                                       *** NB: CALLER CLEANS UP m_briefcaseDgnDb! ***


    // If we got here, we completed the update and pushed it.

    // POST-CONDITIONS
    BeAssert((!anyTxnsInFile(*m_briefcaseDgnDb) && (SyncState::Initial == GetSyncState())) && "Local changes should have been pushed");

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

    // Write contents of "stderr" capture file to the log
    WString wstr;
    readEntireFile(wstr, m_stderrFileName);
    if (!wstr.empty())
        GetLogger().errorv(L"%ls", wstr.c_str());

    if (!m_briefcaseName.empty())
        {
        // Write contents of the issues file to the log
        wstr.clear();
        BeFileName issuesFileName = ComputeReportFileName(m_briefcaseName);
        readEntireFile(wstr, issuesFileName);
        if (!wstr.empty())
            GetLogger().errorv(L"%ls", wstr.c_str());
        }

//    BeFileName::BeDeleteFile(m_stdoutFileName.c_str());
    BeFileName::BeDeleteFile(m_stderrFileName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeFwk::iModelBridgeFwk()
:m_logProvider(NULL), m_dmsSupport(NULL)
    {
    m_client = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeFwk::~iModelBridgeFwk()
    {
    Briefcase_Shutdown();
    LogStderr();
    if (NULL != m_logProvider)
        delete m_logProvider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName iModelBridgeFwk::ComputeReportFileName(BeFileNameCR bcName)
    {
    BeFileName reportFileName(bcName);
    reportFileName.append(L"-fwk-issues");
    return reportFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::ReportIssue(WStringCR msg)
    {
    if (m_briefcaseName.empty()) // too early in the initialization process?
        {
        fputws(msg.c_str(), stderr);
        return;
        }

    auto issuesFileName = ComputeReportFileName(m_briefcaseName);
    BeFileStatus _status;
    auto tf = BeTextFile::Open(_status, issuesFileName.c_str(), TextFileOpenType::Append, TextFileOptions::None);
    if (!tf.IsValid())
        {
        BeAssert(false);
        fputws(msg.c_str(), stderr);
        return;
        }
    tf->PutLine(msg.c_str(), true);
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
        fprintf(stderr, "%ls - cannot create mutex", GetMutexName().c_str());
        return -1;
        }
    HRESULT hr = ::WaitForSingleObject(mutex, s_maxWaitForMutex);
    if (WAIT_OBJECT_0 != hr)
        {
        if (WAIT_TIMEOUT == hr)
            fprintf(stderr, "%ls - Another job is taking a long time. Try again later", GetMutexName().c_str());
        else
            fprintf(stderr, "%ls - Error getting named mutex. Try again later", GetMutexName().c_str());
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
* @bsimethod                                    Sam.Wilson                      08/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool iModelBridgeFwk::_IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey)
    {
    return GetRegistry()._IsFileAssignedToBridge(fn, bridgeRegSubKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::_AssignFileToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey)
    {
    return GetRegistry()._AssignFileToBridge(fn, bridgeRegSubKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::_GetDocumentProperties(iModelBridgeDocumentProperties& props, BeFileNameCR fn)
    {
    return GetRegistry()._GetDocumentProperties(props, fn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::_GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFilePath, BeGuid const& guid)
    {
    return GetRegistry()._GetDocumentPropertiesByGuid(props, localFilePath, guid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SetRegistryForTesting(IModelBridgeRegistry& reg)
    {
    s_registryForTesting = &reg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/17
+---------------+---------------+---------------+---------------+---------------+------*/
IModelBridgeRegistry& iModelBridgeFwk::GetRegistry()
    {
    if (m_registry.IsValid())
        return *m_registry;
    
    if (nullptr != s_registryForTesting)
        return *(m_registry = s_registryForTesting);
     
    BeSQLite::DbResult res;
    m_registry = iModelBridgeRegistry::OpenForFwk(res, m_jobEnvArgs.m_stagingDir, m_briefcaseBasename);

    if (!m_registry.IsValid())
        {
        std::string str (Utf8String(iModelBridgeRegistry::MakeDbName(m_jobEnvArgs.m_stagingDir, m_briefcaseBasename)).c_str());
        str.append(": ");
        str.append(BeSQLite::Db::InterpretDbResult(res));
        throw str;
        }

    return *m_registry;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::LogPerformance(StopWatch& stopWatch, Utf8CP description, ...)
    {
    stopWatch.Stop();
    const NativeLogging::SEVERITY severity = NativeLogging::LOG_INFO;
    NativeLogging::ILogger* logger = NativeLogging::LoggingManager::GetLogger("iModelBridge.Performance");
    if (NULL == logger)
        return;

    if (logger->isSeverityEnabled(severity))
        {
        va_list args;
        va_start(args, description);
        Utf8String formattedDescription;
        formattedDescription.VSprintf(description, args);
        va_end(args);

        logger->messagev(severity, "%s|%.0f millisecs", formattedDescription.c_str(), stopWatch.GetElapsedSeconds() * 1000.0);
        }
    }
