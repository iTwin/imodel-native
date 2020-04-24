/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined(_WIN32)
#include <windows.h>
#undef GetMessage
#elif defined(__linux)
#include <unistd.h>
#endif
#include <iModelBridge/iModelBridgeError.h>
#include <iModelBridge/iModelBridgeFwk.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTextFile.h>
#include <Bentley/BeGetProcAddress.h>
#include <Logging/bentleylogging.h>
#include <iModelBridge/iModelBridgeBimHost.h>
#include <iModelBridge/iModelBridgeRegistry.h>
#include "../iModelBridgeHelpers.h"
#include <iModelBridge/IModelClientForBridges.h>
#include <BentleyLog4cxx/log4cxx.h>
#include <iModelBridge/iModelBridgeLdClient.h>
#include <regex>
#include "../iModelBridgeSettings.h"
#include <WebServices/iModelHub/Client/Error.h>
#include "CrashProcessor.h"

PUSH_DISABLE_DEPRECATION_WARNINGS
#if defined (BENTLEYCONFIG_OS_WINDOWS)
#include <DgnPlatform\DesktopTools\w32tools.h>
#endif

#include "OidcSignInManager.h"
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/IGeoCoordServices.h>
#include <iModelBridge/iModelBridgeSyncInfoFile.h>
#include "iModelBridgeFwkInternal.h"

#include <csignal>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LOGGING

#define RETURN_STATUS_SUCCESS           0
#define RETURN_STATUS_USAGE_ERROR       1
#define RETURN_STATUS_CONVERTER_ERROR   2
#define RETURN_STATUS_SERVER_ERROR      3
#define RETURN_STATUS_LOCAL_ERROR       4
#define RETURN_STATUS_UNHANDLED_EXCEPTION -2

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
#define LOG (*LoggingManager::GetLogger(L"iModelBridge"))

BEGIN_BENTLEY_DGN_NAMESPACE

static iModelBridgeFwk::TestProbe* s_testProbe;

static iModelBridge* s_bridgeForTesting;
static IModelBridgeRegistry* s_registryForTesting;

static int s_maxWaitForMutex = 60000;

void iModelBridgeFwk::SetBridgeForTesting(iModelBridge& b)
    {
    s_bridgeForTesting = &b;
    }

void iModelBridgeFwk::SetTestProbe(TestProbe& p)
    {
    s_testProbe = &p;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/20
+---------------+---------------+---------------+---------------+---------------+------*/
static iModelBridgeFwk::SignalHandlerForTesting s_signalHandlerForTesting;
static BeFileName s_errorFileForSignalHandler;
static iModelBridgeFwk* s_fwkInstanceForSignalHandler;
void iModelBridgeFwk::OnTerminationSignal(int signal)
    {
    if (s_signalHandlerForTesting != nullptr)
        {
        s_signalHandlerForTesting(signal);
        return;
        }

    LOG.fatalv("Received termination signal %d.", signal);

    if (nullptr == s_fwkInstanceForSignalHandler)
        {
        std::abort();
        return;
        }

    s_fwkInstanceForSignalHandler->Briefcase_ReleaseAllPublicLocks(true);

    if (!s_errorFileForSignalHandler.empty())
        {
        iModelBridgeError errorContext;
        errorContext.m_id = iModelBridgeErrorId::Killed;
        errorContext.m_description = "Cancelled";
        errorContext.WriteErrorMessage(s_errorFileForSignalHandler);
        }

    std::abort();
    }

void iModelBridgeFwk::SetSignalHandlerForTesting(SignalHandlerForTesting handler)
    {
    s_signalHandlerForTesting = handler;
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
T_iModelBridge_getInstance* iModelBridgeFwk::JobDefArgs::LoadBridge(iModelBridgeError& errorContext)
    {
    auto getInstance = (T_iModelBridge_getInstance*) GetBridgeFunction(m_bridgeLibraryName, "iModelBridge_getInstance");
    if (!getInstance)
        {
        Utf8PrintfString errorMsg("%ls: Does not export a function called 'iModelBridge_getInstance'", m_bridgeLibraryName.c_str());
        errorContext.m_id = iModelBridgeErrorId::MissingFunctionExport;
        errorContext.m_message = errorMsg;
        LOG.fatalv(errorMsg.c_str());
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
BentleyStatus iModelBridgeFwk::ReadEntireFile(WStringR contents, BeFileNameCR fileName)
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
        L"--fwk-bridge-library=             (optional)  The full path to the bridge library. Specify this instead of --fwk-bridge-regsubkey.\n"
        L"--fwk-bridge-regsubkey=           (optional)  The registry subkey of a bridge that is installed on this machine. Specify this instead of --fwk-bridge-library.\n"
        L"--fwk-staging-dir=                (required)  The staging directory.\n"
        L"--fwk-input=                      (required)  Input file name. Specify only one.\n"
        L"--fwk-input-sheet=                (required)  Input sheet file name. Can be more than one.\n"
        L"--fwk-revision-comment=           (optional)  The revision comment. Can be more than one.\n"
        L"--fwk-logging-config-file=        (optional)  The name of the logging configuration file.\n"
        L"--fwk-argsJson=                   (optional)  Additional arguments in JSON format.\n"
        L"--fwk-max-wait=milliseconds       (optional)  The maximum amount of time to wait for other instances of this job to finish. Default value is 60000ms\n"
        L"--fwk-assetsDir=                  (optional)  Asset directory for the iModelBridgeFwk resources if default location is not suitable.\n"
        L"--fwk-bridgeAssetsDir=            (optional)  Asset directory for the iModelBridge resources if default location is not suitable.\n"
        L"--fwk-reality-data-dir=           (optional)  Path to where reality data tiles should be written to for upload to the Reality Data Server.\n"
        L"--fwk-job-subject-name=           (optional)  The unique name of the Job Subject element that the bridge must use.\n"
        L"--fwk-jobrun-guid=                (optional)  A unique GUID that identifies this job run for activity tracking. This will be passed along to all dependant services and logs.\n"
        L"--fwk-jobrequest-guid=            (optional)  A unique GUID that identifies this job run for correlation. This will be limited to the native callstack.\n"
        L"--fwk-ignore-stale-files          (optional)  Should bridges ignore any file whose last-saved-time is BEFORE that last-saved-time recorded for that file in the iModel?. The default is false (that is, process such files, looking for differences).\n"
        L"--fwk-error-on-stale-files        (optional)  Should bridges fail and report an error if they encounter a file whose last-saved-time is BEFORE that last-saved-time recorded for that file in the iModel?. The default is false (that is, don't fail on stale files).\n"
        L"--fwk-no-mergeDefinitions         (optional)  Do NOT merge definitions such as levels/layers and materials by name from different root models and bridges into the public dictionary model. Instead, keep definitions separate by job subject. The default is false (that is, merge definition).\n"
        L"--fwk-status-message-sink-url=    (optional)  The URL of a WebServer that will process progress meter and status messages\n"
        L"--fwk-status-message-interval=    (optional)  The number of milliseconds to wait before sending another status or progress meter message to the status message server. The default is 1000 milliseconds.\n"
        L"--fwk-enable-crash-reporting      (optional)  Opt-in to crash reporting and potential upload\n"
        L"--fwk-inputArgsJsonFile           (optional)  Input args serialized into a file stored as json\n"
        L"--fwk-skip-assignment-check       (optional)  Ignore the assignment check stored in the bridge settings.\n"
        L"--fwk-no-intermediate-pushes       (optional)  If set a single changeset is created per input file.\n"
        L"--fwk-snapshot=                   (optional)  Create a standalone \"snapshot\" iModel file with the specified name in the specified staging directory. The file will not be associated with any iModel on any iModel server.\n"
        );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::InitLogging()
    {
    // Was the logging config file name specified by --fwk-logging-config-file ?
    if (m_jobEnvArgs.m_loggingConfigFileName.empty() || (false == m_jobEnvArgs.m_loggingConfigFileName.DoesPathExist()))
        {
        // Check to see if one was defined with a Launch Darkly feature value
        GetLoggingConfigFileNameFromFeatureValue();
        }

    // If logging config file name still not defined check deployment directory
    if (m_jobEnvArgs.m_loggingConfigFileName.empty())
        {
        m_jobEnvArgs.m_loggingConfigFileName = Desktop::FileSystem::GetExecutableDir();
        m_jobEnvArgs.m_loggingConfigFileName.AppendToPath(L"iModelBridgeFwk.logging.config.xml");
        }

    if (!m_jobEnvArgs.m_loggingConfigFileName.DoesPathExist())
        {
        fwprintf(stdout, L"%ls: file not found.\n", m_jobEnvArgs.m_loggingConfigFileName.c_str());

        m_jobEnvArgs.m_loggingConfigFileName.Clear();
        }

    if (!m_jobEnvArgs.m_loggingConfigFileName.empty() && m_jobEnvArgs.m_loggingConfigFileName.DoesPathExist())
        {
        NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, m_jobEnvArgs.m_loggingConfigFileName);
        if (NULL == m_logProvider)
            m_logProvider = new NativeLogging::Provider::Log4cxxProvider();
        NativeLogging::LoggingConfig::ActivateProvider(m_logProvider);
        LOG.infov("Using logfile %s", m_jobEnvArgs.m_loggingConfigFileName.GetBaseName().GetNameUtf8().c_str());
        return;
        }

    // Default to the console logging provider.
    fprintf(stderr, "Logging.config.xml not specified. Activating default logging using console provider.\n");
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity(L"iModelBridge", NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity(L"iModelBridgeFwk", NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity(L"iModelHub", NativeLogging::LOG_INFO);
    //NativeLogging::LoggingConfig::SetSeverity(L"Performance", NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity(L"ECDb", NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity(L"DgnCore", NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity(L"DgnV8Converter", NativeLogging::LOG_TRACE);
    // NativeLogging::LoggingConfig::SetSeverity(L"Changeset", NativeLogging::LOG_TRACE);
    //NativeLogging::LoggingConfig::SetSeverity(L"BeSQLite", NativeLogging::LOG_TRACE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName  GetDefaultAssetsDirectory()
    {
    BeFileName fwkAssetsDirRaw = Desktop::FileSystem::GetExecutableDir();

    fwkAssetsDirRaw.AppendToPath(L"Assets");
    if (fwkAssetsDirRaw.DoesPathExist())
        return fwkAssetsDirRaw;

    fwkAssetsDirRaw = Desktop::FileSystem::GetExecutableDir();
    fwkAssetsDirRaw.AppendToPath(L"..");
    fwkAssetsDirRaw.AppendToPath(L"Assets");
    if (fwkAssetsDirRaw.DoesPathExist())
        return fwkAssetsDirRaw;

    fwkAssetsDirRaw = Desktop::FileSystem::GetExecutableDir();
    fwkAssetsDirRaw.AppendToPath(L"..");
    fwkAssetsDirRaw.AppendToPath(L"..");
    fwkAssetsDirRaw.AppendToPath(L"Assets");
    if (fwkAssetsDirRaw.DoesPathExist())
        return fwkAssetsDirRaw;

    return Desktop::FileSystem::GetExecutableDir();
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
BentleyStatus iModelBridgeFwk::JobDefArgs::ParseCommandLine(bvector<WString>& unrecognized, bvector<WString> const& args)
    {
    BeFileName fwkAssetsDirRaw = GetDefaultAssetsDirectory();
    auto argv = GetArgPtrs(args);   // TODO rewrite this function to work with WStrings, not an array of pointers
    int argc = (int)args.size();
    for (int iArg = 1; iArg < argc; ++iArg)
        {
        if (argv[iArg][0] == '@')
            {
            BeFileName rspFileName(argv[iArg]+1);
            WString wargs;
            if (BSISUCCESS != iModelBridgeFwk::ReadEntireFile(wargs, rspFileName))
                {
                fwprintf(stderr, L"%ls - response file not found\n", rspFileName.c_str());
                return BSIERROR;
                }

            bvector<WString> strings;
            BeStringUtilities::ParseArguments(strings, wargs.c_str(), L"\n\r");

            strings.erase(std::remove(strings.begin(), strings.end(), L""), strings.end());
            if (!strings.empty())
                {
                strings.insert(strings.begin(), args[0]);
                if (BSISUCCESS != ParseCommandLine(unrecognized, strings))
                    return BSIERROR;
                }

            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--registry-dir="))
            {
            if (!m_registryDir.empty())
                {
                fwprintf(stderr, L"The --registry-dir= option may appear only once.\n");
                return BSIERROR;
                }
            m_registryDir.SetName(getArgValueW(argv[iArg]));
            continue;
            }

        if (0 != BeStringUtilities::Wcsnicmp(argv[iArg], L"--fwk", 5))
            {
            // Not a fwk argument. We will forward it to the bridge.
            unrecognized.push_back(argv[iArg]);
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

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-snapshot="))
            {
            BeFileName::FixPathName(m_snapshotFileName, getArgValueW(argv[iArg]).c_str());
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

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-status-message-sink-url"))
            {
            m_statusMessageSinkUrl = getArgValue(argv[iArg]);
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-status-message-interval"))
            {
            int val;
            if (0 == swscanf(getArgValueW(argv[iArg]).c_str(), L"%d", &val) || val < 0)
                {
                fwprintf(stderr, L"The -fwk-status-message-interval= option takes an integer as its argument. The number of milliseconds between status message pushes.\n");
                return BSIERROR;
                }
            m_statusMessageInterval = val;
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
        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-job-subject-name="))
            {
            m_jobSubjectName = getArgValue(argv[iArg]);
            m_jobSubjectName.DropQuotes();
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-no-mergeDefinitions"))
            {
            m_mergeDefinitions = false;
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-ignore-stale-files"))
            {
            m_ignoreStaleFiles = true;
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-error-on-stale-files"))
            {
            m_errorOnStaleFiles = true;
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-all-docs-processed"))
            {
            m_allDocsProcessed = true;
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-enable-crash-reporting"))
            {
            m_isCrashReportingEnabled = true;
            continue;
            }
		if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-reality-data-dir="))
            {
            m_realityDataDir.SetName(getArgValueW(argv[iArg]));
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-inputArgsJsonFile"))
            {
            ProcessInputJson(unrecognized, getArgValue(argv[iArg]));
            continue;
            }
        // This argument was originally typoed.  In case anyone out in the field is using it, we still need to support the old spelling
        if ((argv[iArg] == wcsstr(argv[iArg], L"--fwk-no-intermdiate-pushes")) ||
            (argv[iArg] == wcsstr(argv[iArg], L"--fwk-no-intermediate-pushes"))) 
            {
            m_allowIntermdiatePushes = false;
            continue;
            }
        BeAssert(false);
        fwprintf(stderr, L"%ls: unrecognized fwk argument\n", argv[iArg]);
        return BSIERROR;
        }

    BeFileName::FixPathName(m_fwkAssetsDir, fwkAssetsDirRaw.c_str());
    m_fwkAssetsDir.BeGetFullPathName();

    ParseEnvironment(unrecognized);//We cannot call this at the top due to input file, staging directory argument behavior.

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::JobDefArgs::ProcessInputJson(bvector<WString>& unrecognized, Utf8StringCR jsonFileName)
    {
    BeFileName rspFileName(jsonFileName);
    WString wargs;
    if (BSISUCCESS != ReadEntireFile(wargs, rspFileName))
        {
        fwprintf(stderr, L"%ls - json file not found\n", rspFileName.c_str());
        return BSIERROR;
        }

    Json::Value jsonValue;
    Json::Reader::Parse(Utf8String(wargs), jsonValue);

    bvector<WString> params;
    params.push_back(rspFileName.c_str());
    Json::Value::Members     jsonParams = jsonValue.getMemberNames();
    for (auto& name : jsonParams)
        {
        auto& value = jsonValue[name];
        params.push_back(WPrintfString(L"%s=%s", name.c_str(), value.asCString()));
        }

    if (BSISUCCESS != ParseCommandLine(unrecognized, params))
        return BSIERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::JobDefArgs::ParseEnvironment(bvector<WString>& unrecognized)
    {
    Utf8String jsonFile;
    if (SetValueIfEmptyFromEnv(L"imbridge--fwk-inputArgsJsonFile", jsonFile))
        ProcessInputJson(unrecognized, jsonFile);

    SetValueIfEmptyFromEnv(L"imbridge_fwk_bridge_library", m_bridgeLibraryName);
    SetValueIfEmptyFromEnv(L"imbridge_fwk_staging_dir", m_stagingDir);
    SetValueIfEmptyFromEnv(L"imbridge--fwk-input", m_inputFileName);
    SetValueIfEmptyFromEnv(L"imbridge--fwk-assetsDir", m_fwkAssetsDir);
    SetValueIfEmptyFromEnv(L"imbridge--fwk-jobrun-guid", m_jobRunCorrelationId);
    SetValueIfEmptyFromEnv(L"imbridge--fwk-status-message-sink-url", m_statusMessageSinkUrl);
    if (!m_isCrashReportingEnabled)
        SetValueFromEnv(L"imbridge--fwk-enable-crash-reporting", m_isCrashReportingEnabled);
    /*
        TODO: Map these if neeeded
        L"--fwk-max-wait=milliseconds       (optional)  The maximum amount of time to wait for other instances of this job to finish. Default value is 60000ms\n"
        L"--fwk-bridgeAssetsDir=            (optional)  Asset directory for the iModelBridge resources if default location is not suitable.\n"
        L"--fwk-job-subject-name=           (optional)  The unique name of the Job Subject element that the bridge must use.\n"
        L"--fwk-jobrequest-guid=            (optional)  A unique GUID that identifies this job run for correlation. This will be limited to the native callstack.\n"
        L"--fwk-ignore-stale-files          (optional)  Should bridges ignore any file whose last-saved-time is BEFORE that last-saved-time recorded for that file in the iModel?. The default is false (that is, process such files, looking for differences).\n"
        L"--fwk-error-on-stale-files        (optional)  Should bridges fail and report an error if they encounter a file whose last-saved-time is BEFORE that last-saved-time recorded for that file in the iModel?. The default is false (that is, don't fail on stale files).\n"
        L"--fwk-no-mergeDefinitions         (optional)  Do NOT merge definitions such as levels/layers and materials by name from different root models and bridges into the public dictionary model. Instead, keep definitions separate by job subject. The default is false (that is, merge definition).\n"
        L"--fwk-status-message-sink-url=    (optional)  The URL of a WebServer that will process progress meter and status messages\n"
        L"--fwk-status-message-interval=    (optional)  The number of milliseconds to wait before sending another status or progress meter message to the status message server. The default is 1000 milliseconds.\n"
    */
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Majerle                 07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::GetLoggingConfigFileNameFromFeatureValue()
    {
    Utf8String loggingLevel = Utf8String("");

    // Check to see if logging config file is being configured by a feature value.
    if (BentleyApi::BSISUCCESS == GetFeatureValue(loggingLevel, "imodelbridge-logging-level"))
        {
        WPrintfString fileName(L"iModelBridgeFwk.%S.logging.config.xml", loggingLevel.c_str());

        BeFileName loggingConfigFileName = m_jobEnvArgs.m_bridgeAssetsDir;
        loggingConfigFileName.AppendToPath(fileName.c_str());

        if (!loggingConfigFileName.DoesPathExist())
            {
            loggingConfigFileName = m_jobEnvArgs.m_fwkAssetsDir;
            loggingConfigFileName.AppendToPath(fileName.c_str());
            }

        if (!loggingConfigFileName.empty() && loggingConfigFileName.DoesPathExist())
            {
            m_jobEnvArgs.m_loggingConfigFileName = loggingConfigFileName;

            }
        }
    // The selected logging.config.xml file requires FWK_STAGINGDIR be assigned to the staging directory
    putenv(Utf8PrintfString("FWK_STAGINGDIR=%ls", m_jobEnvArgs.m_stagingDir.c_str()).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      10/17
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridgeFwk::JobDefArgs::Validate(bvector<WString> const&)
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

    // Parse --fwk args and push all unrecognized args to m_bridgeArgs
    m_bridgeArgs.push_back(argv[0]);
    bvector<WString> args(argv, argv+argc);
    if ((BSISUCCESS != m_jobEnvArgs.ParseCommandLine(m_bridgeArgs, args)) || (BSISUCCESS != m_jobEnvArgs.Validate(args)))
        {
        PrintUsage(argv[0]);
        return BSIERROR;
        }

    // *After* verifying that m_stagingDir exists,
    // Redirect stderr to a file in m_stagingDir. This is how we report some errors back to the calling program.
    RedirectStderr();

    bvector<WString> unrecognizedArgs;          // Forward non-fwk args to next parser and get ready to accumulate the args that it does not recognize.
    std::swap(unrecognizedArgs, m_bridgeArgs);
    m_bridgeArgs.push_back(argv[0]);

    // Parse --imodel-bank args and push all unrecognized args to m_bridgeArgs
    IModelBankArgs bankArgs;
    if ((BSISUCCESS != bankArgs.ParseCommandLine(m_bridgeArgs, unrecognizedArgs)) || (BSISUCCESS != bankArgs.Validate(unrecognizedArgs)))
        {
        PrintUsage(argv[0]);
        return BSIERROR;
        }

    unrecognizedArgs.clear();                   // Forward non-bank args to next parser and get ready to accumulate the args that it does not recognize.
    std::swap(unrecognizedArgs, m_bridgeArgs);
    m_bridgeArgs.push_back(argv[0]);

    // Parse --server args and push all unrecognized args to m_bridgeArgs
    IModelHubArgs hubArgs;
    if ((BSISUCCESS != hubArgs.ParseCommandLine(m_bridgeArgs, unrecognizedArgs)) || (BSISUCCESS != hubArgs.Validate(unrecognizedArgs)))
        {
        PrintUsage(argv[0]);
        return BSIERROR;
        }

    unrecognizedArgs.clear();                   // Forward non-bank and non-server args to next parser and get ready to accumulate the args that it does not recognize.
    std::swap(unrecognizedArgs, m_bridgeArgs);
    m_bridgeArgs.push_back(argv[0]);

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
        m_maxRetryWait = m_iModelBankArgs->m_maxRetryWait;
        dmsCredentialsAreEncrypted = m_iModelBankArgs->m_dmsCredentialsEncrypted;
        }
    else
        {
        m_useIModelHub = true;
        m_iModelHubArgs = new IModelHubArgs(hubArgs);
        m_briefcaseBasename = m_iModelHubArgs->m_repositoryName;
        m_maxRetryCount = m_iModelHubArgs->m_maxRetryCount;
        m_maxRetryWait = m_iModelHubArgs->m_maxRetryWait;
        dmsCredentialsAreEncrypted = m_iModelHubArgs->m_isEncrypted;
        }

    // Parse --dms args and push all unrecognized args to m_bridgeArgs
    if ((BSISUCCESS != m_dmsServerArgs.ParseCommandLine(m_bridgeArgs, unrecognizedArgs, dmsCredentialsAreEncrypted)) || (BSISUCCESS != m_dmsServerArgs.Validate(unrecognizedArgs)))
        {
        PrintUsage(argv[0]);
        return BSIERROR;
        }

    // The args that are now in m_bridgeArgs will be fowarded to the bridge's _ParseCommandLine function.

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
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult iModelBridgeFwk::OpenOrCreateStateDb(iModelBridgeError& errorContext)
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
            errorContext.m_id = static_cast<iModelBridgeErrorId>(BE_SQLITE_ERROR_ProfileTooOld);
            Utf8PrintfString errorMessage("%ls - version %ls is too old", stateFileName.c_str(), ver.ToString().c_str());
            LOG.fatalv(errorMessage.c_str());
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
BentleyStatus iModelBridgeFwk::DoInitial(iModelBridgeFwk::FwkContext& context)
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
    if (BSISUCCESS == Briefcase_AcquireBriefcase(context))
        {
        SetState(BootstrappingState::HaveBriefcase);
        return BSISUCCESS;
        }

    //  Can't acquire a briefcase
    if (iModel::Hub::Error::Id::iModelDoesNotExist != static_cast<iModel::Hub::Error::Id>(context.m_error.m_id))
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


    SetState(BootstrappingState::CreatedLocalDb);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::IModelHub_DoCreatedLocalDb(iModelBridgeFwk::FwkContext& context)
    {
    if (Briefcase_IsBriefcase())
        {
        BeAssert(false && "Invalid state. If we have a briefcase, we should not be in CreatedLocalDb");
        SetState(BootstrappingState::HaveBriefcase);
        return BSIERROR;
        }

    if (BSISUCCESS != Briefcase_IModelHub_CreateRepository())
        return BSIERROR;

    if (context.m_iModelInfo.IsValid())
        context.m_settings.SetiModelId(context.m_iModelInfo->GetId());

    SetState(BootstrappingState::CreatedRepository);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::IModelHub_DoCreatedRepository(iModelBridgeFwk::FwkContext& context)
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

    if (BSISUCCESS != Briefcase_AcquireBriefcase(context))
        {
        BeAssert(false && "we think we have created a repository, but it must have failed");
        if (iModel::Hub::Error::Id::iModelDoesNotExist != static_cast<iModel::Hub::Error::Id>(context.m_error.m_id))
            {
            SetState(BootstrappingState::Initial);
            return BSISUCCESS;
            }
        ReportIssue("iModel create failed"); // *** WIP translate
        GetLogger().fatalv("CreateRepository failed or acquireBriefcase failed for repositoryname=%s, stagingdir=%s",
                            m_briefcaseBasename.c_str(), Utf8String(m_jobEnvArgs.m_stagingDir).c_str());
        return BSIERROR;
        }

    SetState(BootstrappingState::HaveBriefcase);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::BootstrapBriefcase(bool& createdNewRepo, iModelBridgeFwk::FwkContext& context)
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
            case BootstrappingState::Initial:                   status = DoInitial(context); break;
            case BootstrappingState::CreatedLocalDb:            status = IModelHub_DoCreatedLocalDb(context); createdNewRepo = true; break;
            case BootstrappingState::CreatedRepository:         status = IModelHub_DoCreatedRepository(context); break;
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

    if (!m_briefcaseName.DoesPathExist())
        {
        BeSQLite::BeBriefcaseId briefcaseId = GetBriefcaseId();
        if (!briefcaseId.IsValid())//We were not able to find a passed or cached briefcaseId. Let's check the settings service.
            {
            context.m_settings.GetBriefCaseId(m_dmsServerArgs.GetDocumentGuid(), briefcaseId);
            }
        if (briefcaseId.IsValid())
            {
            if (BSISUCCESS != m_client->RestoreBriefcase(m_briefcaseName, m_briefcaseBasename.c_str(), briefcaseId))
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
* @bsimethod                                    Sam.Wilson                      06/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void setEmbeddedFileIdRecipe(iModelBridge::Params& params)
    {
    auto tempVarCheck = getenv("iModelBridge_MatchOnEmbeddedFileBasename");     // TODO: Replace this with a settings service parameter check
    if ((nullptr == tempVarCheck) || (*tempVarCheck == '0'))
        return;

    Utf8String regexStr(tempVarCheck);

    if (regexStr.StartsWith("\"") && regexStr.EndsWith("\""))
        {
        regexStr.DropQuotes();
        }

    bool isRegex = false;
    if (!regexStr.Equals("1"))
        {
        // assume user has supplied a suffix regex to be used in a recipe.
        try {
            // but first make sure it's valid.
            std::regex rgx(regexStr.c_str());
            // If no exception, then go ahead with it.
            isRegex = true;
            }
        catch (...)
            {
            LOG.errorv(L"%s is an invalid regular expression. This was found as the value of the iModelBridge_MatchOnEmbeddedFileBasename environment variable.", tempVarCheck);
            isRegex = false; // default to old behavior - maybe the user set envvar to Yes or True or something
            }
        }

    if (!isRegex)
        {
        // Set up the recipe that we had been using
        iModelBridge::Params::FileIdRecipe recipe;
        recipe.m_ignorePackage = true;
        recipe.m_ignoreCase = false;
        recipe.m_ignoreExtension = false;
        recipe.m_ignorePwDocId = false;
        recipe.m_suffixRegex = "";
        params.SetEmbeddedFileIdRecipe(recipe);
        return;
        }

    // Set up a recipe with the new features, including a suffix recognizer
    iModelBridge::Params::FileIdRecipe recipe;
    recipe.m_ignorePackage = true;
    recipe.m_ignoreCase = true;
    recipe.m_ignoreExtension = true;
    recipe.m_ignorePwDocId = true;
    recipe.m_suffixRegex = regexStr.c_str();
    params.SetEmbeddedFileIdRecipe(recipe);
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
    if (m_bcMgrForBridges.IsValid())
        params.SetBriefcaseManager(*m_bcMgrForBridges);
    params.SetBridgeRegSubKey(m_jobEnvArgs.m_bridgeRegSubKey);
    params.ParseJsonArgs(m_jobEnvArgs.m_argsJson, true);
    params.m_jobRunCorrelationId = m_jobEnvArgs.m_jobRunCorrelationId;
    //Set up Dms files would have loaded the DMS accesor. Set it on the params for the Dgnv8 Bridge
    params.m_dmsSupport = m_dmsSupport;
    params.SetPushIntermediateRevisions(iModelBridge::Params::PushIntermediateRevisions::ByFile);
    if (!m_jobEnvArgs.m_jobSubjectName.empty())
        params.SetBridgeJobName(m_jobEnvArgs.m_jobSubjectName);
    params.SetMergeDefinitions(m_jobEnvArgs.m_mergeDefinitions);

    params.SetIgnoreStaleFiles(m_jobEnvArgs.m_ignoreStaleFiles);
    params.SetErrorOnStaleFiles(m_jobEnvArgs.m_errorOnStaleFiles);
    if (!m_jobEnvArgs.m_revisionComment.empty())
        params.SetRevisionComment(m_jobEnvArgs.m_revisionComment);

    setEmbeddedFileIdRecipe(params);

    if (params.GetEmbeddedFileIdRecipe() != nullptr)
        {
        auto recipe = params.GetEmbeddedFileIdRecipe();
        GetLogger().infov("bridge:%s iModel:%s - EmbeddedFileIdRecipe=C:%d X:%d P:%d I:%d R:%s",
            Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str(),
            recipe->m_ignoreCase, recipe->m_ignoreExtension, recipe->m_ignorePackage, recipe->m_ignorePwDocId, recipe->m_suffixRegex.c_str());
        }

    if (m_useIModelHub)
        {
        params.SetUrlEnvironment(m_iModelHubArgs->m_environment);
        params.SetiModelName(m_iModelHubArgs->m_repositoryName);
        params.SetUserName(m_iModelHubArgs->m_credentials.GetUsername());
        params.SetProjectGuid(m_iModelHubArgs->m_bcsProjectId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::LoadBridge(iModelBridgeError& errorContext)
    {
    if (s_bridgeForTesting)
        {
        m_bridge = s_bridgeForTesting;
        return BentleyStatus::SUCCESS;
        }

    auto getInstance = m_jobEnvArgs.LoadBridge(errorContext);
    if (nullptr == getInstance)
        return errorContext.GetBentleyStatus();

    m_bridge = getInstance(m_jobEnvArgs.m_bridgeRegSubKey.c_str());
    if (nullptr == m_bridge)
        {
        Utf8PrintfString errorMsg("%ls: iModelBridge_getInstance function returned a nullptr", m_jobEnvArgs.m_bridgeLibraryName.c_str());
        errorContext.m_id = iModelBridgeErrorId::MissingInstance;
        errorContext.m_message = errorMsg;
        LOG.fatalv(errorMsg.c_str());
        return errorContext.GetBentleyStatus();
        }

    m_bridge->_InitIdentity();

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
    StopWatch initBridge(true);
    SetBridgeParams(m_bridge->_GetParams(), m_repoAdmin);

    auto bargptrs = GetArgPtrs(m_bridgeArgs);
    if (BentleyStatus::SUCCESS != m_bridge->_ParseCommandLine((int)bargptrs.size(), bargptrs.data()))
        {
        fprintf(stderr, "bridge _ParseCommandLine failed\n");
        m_bridge->_PrintUsage();
        return BentleyStatus::ERROR;
        }

    SetBridgeParams(m_bridge->_GetParams(), m_repoAdmin);    // make sure that MY definition of these params is used!

    if (BSISUCCESS != m_bridge->TrackUsage())
        {
        LOG.error("Bridge Usage tracking failed. Please ignore if OIDC is not initialized.");
        }

    if (BSISUCCESS != m_bridge->_Initialize((int)bargptrs.size(), bargptrs.data()))
        return BentleyStatus::ERROR;

    BeAssert((m_bridge->_GetParams().GetRepositoryAdmin() == m_repoAdmin) && "Bridge must use the RepositoryAdmin that the fwk supplies");

    iModelBridge::LogPerformance(initBridge, "Inititalize the bridge.");
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
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void writeBriefcaseIdTxtFile(BeFileNameCR briefcaseName, BeSQLite::BeBriefcaseId const& bcId)
    {
    BeFileName fileName(briefcaseName);
    fileName.append(L"-briefcaseId");
    BeFileStatus status;
    BeTextFilePtr txtFile = BeTextFile::Open(status, fileName.c_str(), TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
    if (!txtFile.IsValid())
        return;

    txtFile->PrintfTo(false, L"%lu\n", bcId.GetValue());

    txtFile->Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult iModelBridgeFwk::SaveBriefcaseId(BeSQLite::BeBriefcaseId& briefcaseId)
    {
    DbResult rc;
    auto db = DgnDb::OpenDgnDb(&rc, m_briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    if (!db.IsValid())
        {
        return rc;
        }
    briefcaseId = db->GetBriefcaseId();
    uint32_t bcid = briefcaseId.GetValue();
    m_stateDb.SaveProperty(s_briefcaseIdPropSpec, &bcid, sizeof(bcid));
    m_stateDb.SaveChanges();

    writeBriefcaseIdTxtFile(m_briefcaseName, briefcaseId);

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
        DgnPlatformLib::GetHost().Terminate(true);
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
        m_jobRequestId(jobDef.m_jobRequestId.c_str(), true), m_bridgeName(jobDef.m_bridgeRegSubKey)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2019
//---------------------------------------------------------------------------------------
#if defined (BENTLEYCONFIG_OS_WINDOWS)

// N.B. The EXCEPTION_POINTERS are only available during this exception filter callback.
// We need to perform cleanup before terminating, so we need to create the dump here manually,
// and then continue to perform cleanup before termianting.
int windows_filterException(EXCEPTION_POINTERS* ptrs)
    {
    CrashProcessor* crashProc = CrashProcessor::GetInstance();
    if (nullptr != crashProc)
        crashProc->CreateDump(ptrs);

    win32Tools_resetFloatingPointExceptions(0); // Sam had this previously... not sure why... but keeping until proven otherwise
    return EXCEPTION_EXECUTE_HANDLER; // Ignore the exception for now so we can perform cleanup.
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::UpdateExistingBimWithExceptionHandling(iModelBridgeFwk::FwkContext& context)
{
#if defined (BENTLEYCONFIG_OS_WINDOWS)
    __try
        {
#endif
        return UpdateExistingBim(context);
#if defined (BENTLEYCONFIG_OS_WINDOWS)
        }
    __except (windows_filterException(GetExceptionInformation()))
        {
        fprintf(stderr, "Unhandled exception in UpdateExistingBim. Attempting to release public locks...\n");
        }

    return RETURN_STATUS_UNHANDLED_EXCEPTION;
#endif
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::InitializeLaunchDarklyClient()
    {
    if (!m_useIModelHub || nullptr == m_iModelHubArgs || m_jobEnvArgs.CreateSnapshot())
        return;

    iModelBridgeLdClient& client = iModelBridgeLdClient::GetInstance(m_iModelHubArgs->m_environment);

    if(!m_iModelHubArgs->m_callBackurl.empty())
        {
        OidcSignInManagerPtr oidcMgr = OidcSignInManager::FromCallBack(m_iModelHubArgs->m_callBackurl);
        client.SetUserName(oidcMgr->GetUserInfo().userId.c_str());
        }
    else if (!m_iModelHubArgs->m_accessToken.empty())
        {
        OidcSignInManagerPtr oidcMgr = OidcSignInManager::FromAccessToken(m_iModelHubArgs->m_accessToken);
        client.SetUserName(oidcMgr->GetUserInfo().userId.c_str());
        }
    else
        {
        client.SetUserName(m_iModelHubArgs->m_credentials.GetUsername().c_str());
        }

    client.SetProjectDetails(m_iModelHubArgs->m_repositoryName.c_str(), m_iModelHubArgs->m_bcsProjectId.c_str());
    if (SUCCESS != client.InitClient())
        LOG.errorv(L"Error initializing launch darkly.");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::ConfigureLaunchDarklyClient()
    {
    if (m_jobEnvArgs.CreateSnapshot())
        return;

    iModelBridgeLdClient& client = iModelBridgeLdClient::GetInstance(m_iModelHubArgs->m_environment);
    auto clientInfo = m_bridge->GetParamsCR().GetClientInfo();
    if (nullptr == clientInfo)
        return;

    client.SetBridgeDetails(clientInfo->GetApplicationProductId(), clientInfo->GetApplicationName(), clientInfo->GetApplicationVersion().ToString(VERSION_PARSE_FORMAT));
    client.RestartClient();
    }

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

    DbResult dbres;

    iModelBridgeError errorContext;
    iModelBridgeSacAdapter::InitCrt(false);

    InitializeLaunchDarklyClient();

    InitLogging();

    LoggingContext logContext(m_jobEnvArgs, connectProjectId, iModelId, m_logProvider);

    // Do this as a service to innards that call rand(). For example, the random wait times when acquiring locks to try and break logjams.
    srand(time(0));

    Briefcase_MakeBriefcaseName();
    BeFileName::BeDeleteFile(ComputeReportFileName(m_briefcaseName));  // delete any old issues file hanging round from the previous run
    BeFileName errorFile(m_briefcaseName);
    errorFile.append(L"-errors.json");
    BeFileName::BeDeleteFile(errorFile);  // delete any old error file hanging round from the previous run

    //  Open our state db.
    dbres = OpenOrCreateStateDb(errorContext);
    if (BE_SQLITE_OK != dbres)
        {
        LOG.fatal("OpenOrCreateStateDb failed");
        errorContext.WriteErrorMessage(errorFile);
        return errorContext.GetIntErrorId();
        }

    //  Resolve the bridge to run
    if (m_jobEnvArgs.m_bridgeLibraryName.empty())
        {
        GetRegistry()._FindBridgeInRegistry(m_jobEnvArgs.m_bridgeLibraryName, m_jobEnvArgs.m_bridgeAssetsDir, m_jobEnvArgs.m_bridgeRegSubKey);
        if (m_jobEnvArgs.m_bridgeLibraryName.empty())
            {
            Utf8PrintfString errorMsg("%ls - no bridge with this subkey is in the registry db", m_jobEnvArgs.m_bridgeRegSubKey.c_str());
            LOG.fatalv(errorMsg.c_str());
            errorContext.m_id = iModelBridgeErrorId::MissingBridgeInRegistry;
            errorContext.m_message = errorMsg;
            errorContext.WriteErrorMessage(errorFile);
            return errorContext.GetIntErrorId();
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
    if (BentleyStatus::SUCCESS != LoadBridge(errorContext))
        {
        errorContext.WriteErrorMessage(errorFile);
        return errorContext.GetIntErrorId();
        }

    ConfigureLaunchDarklyClient();

    // Initialize crash reporting.
    if (m_jobEnvArgs.m_isCrashReportingEnabled)
        {
        // We intentionally create BUDDI URLs matching bridge application names to get the Sentry endpoint.
        CrashProcessor& crashProc = CrashProcessor::CreateSentryInstance(m_bridge->_GetParams().GetClientInfo()->GetApplicationName().c_str());

        crashProc.SetAnnotation(CrashProcessor::CommonAnnotation::JOB_Id, m_jobEnvArgs.m_jobRequestId.c_str());
        crashProc.SetAnnotation(CrashProcessor::CommonAnnotation::JOB_CorrelationId, m_jobEnvArgs.m_jobRunCorrelationId.c_str());

        if (m_useIModelHub && !m_jobEnvArgs.CreateSnapshot())
            {
            crashProc.SetAnnotation(CrashProcessor::CommonAnnotation::IMH_UserName, m_iModelHubArgs->m_credentials.GetUsername().c_str());
            crashProc.SetAnnotation(CrashProcessor::CommonAnnotation::IMH_RpositoryName, m_iModelHubArgs->m_repositoryName.c_str());
            crashProc.SetAnnotation(CrashProcessor::CommonAnnotation::IMH_ProjectId, m_iModelHubArgs->m_bcsProjectId.c_str());
            }
        }

    s_fwkInstanceForSignalHandler = this;   // Hack - I must pass 'this' to my signal handler, but std::signal only takes a static function.
    s_errorFileForSignalHandler = errorFile;
    std::signal(SIGINT, OnTerminationSignal);
    std::signal(SIGTERM, OnTerminationSignal);
    // NB: Do not try to handle SIGABRT. OnTerminationSignal calls abort.

    // Initialize the DgnViewLib Host.
    m_repoAdmin = new FwkRepoAdmin(*this);  // TRICKY: This is ultimately passed to the host as a host variable, and host terimation will delete it.
    iModelBridge::Params params;
    SetBridgeParams(params, m_repoAdmin);

    BeFileName fwkAssetsDir(m_jobEnvArgs.m_fwkAssetsDir);
    BeFileName fwkDb3 = fwkAssetsDir;
    fwkDb3.AppendToPath(L"sqlang");
    fwkDb3.AppendToPath(L"iModelBridgeFwk_en-US.sqlang.db3");

    Dgn::iModelBridgeBimHost host(m_repoAdmin, fwkAssetsDir, fwkDb3, Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str());
    DgnPlatformLib::Initialize(host);

    //  Initialize the bridge-specific L10N
    BeFileName bridgeSqlangPath(params.GetAssetsDir());
    bridgeSqlangPath.AppendToPath(m_bridge->_SupplySqlangRelPath().c_str());
    iModelBridge::L10N::Initialize(BeSQLite::L10N::SqlangFiles(bridgeSqlangPath));

    HostTerminator terminateHostOnReturn;

    iModelBridgeBimHost_SetBridge _registerBridgeOnHost(*m_bridge);

    SetupProgressMeter();

    if (m_jobEnvArgs.CreateSnapshot())
        {
        auto status = CreateSnapshotWithExceptionHandling(errorContext);

        BeAssert(!m_briefcaseDgnDb.IsValid() && "CreateSnapshotWithExceptionHandling should manage the lifetime of the briefcase");

        LOG.tracev(L"ReleaseBridge...");
        if (SUCCESS != ReleaseBridge())
            LOG.errorv(L"%s - Memory leak. This bridge was not released properly.", m_jobEnvArgs.m_bridgeRegSubKey.c_str());
        LOG.tracev(L"ReleaseBridge         : Done");

        s_fwkInstanceForSignalHandler = nullptr;

        if (SUCCESS != status)
            {
            errorContext.WriteErrorMessage(errorFile);
            return errorContext.GetIntErrorId();
            }

        T_HOST.GetProgressMeter()->Hide();

/*<==*/ return status;
        }

    AddPhases(6);

    SetCurrentPhaseName("Initializing");
    GetProgressMeter().AddSteps(3);

    iModelBridge::LogPerformance(setUpTimer, "Initialized iModelBridge Fwk");

    LOG.tracev(L"Logging into iModel Hub");
    GetProgressMeter().SetCurrentStepName("Contacting the iModel Server");
    {
    StopWatch iModelHubSignIn(true);
    //  Sign into the iModelHub
    if (BSISUCCESS != Briefcase_Initialize(argc, argv, errorContext))
        {
        errorContext.WriteErrorMessage(errorFile);
        return errorContext.GetIntErrorId();
        }

    iModelBridge::LogPerformance(iModelHubSignIn, "Logging into iModelHub");
    }

    LOG.tracev(L"Logging into iModel Hub : Done");
    //Initialize the settings.
    Utf8String iModelGuid;
    if (m_iModelHubArgs)
         connectProjectId = m_client->GetProjectId();
    if (!connectProjectId.empty())
        m_iModelHubArgs->m_bcsProjectId = connectProjectId;//Store the guid instead of the name once we successfully log into iModelHub.
    auto iModelInfo = m_client->GetIModelInfo();
    if (iModelInfo.IsValid())
        iModelGuid = iModelInfo->GetId();


    iModelBridgeSettings settings(m_client->GetConnectSignInManager(), m_jobEnvArgs.m_jobRunCorrelationId, iModelGuid.c_str(), connectProjectId.c_str());
    FwkContext context(settings, errorContext, iModelInfo);
    // Stage the workspace and input file if  necessary.
    LOG.tracev(L"Setting up workspace for standalone bridges");
    GetProgressMeter().SetCurrentStepName("Setting up workspace for standalone bridges");
    if (BSISUCCESS != SetupDmsFiles(context))
        {
        errorContext.WriteErrorMessage(errorFile);
        return errorContext.GetIntErrorId();
        }
    LOG.tracev(L"Setting up workspace for standalone bridges  : Done");

    //  Make sure we have a briefcase.
    Briefcase_MakeBriefcaseName(); // => defines m_briefcaseName
    {
    LOG.tracev(L"Setting up iModel Briefcase for processing");
    GetProgressMeter().SetCurrentStepName("Setting up iModel briefcase");
    StopWatch briefcaseTime(true);

    if (m_bridge->TestFeatureFlag("imodel-bridge-reality-model-upload"))
        {
        m_bridge->_GetParams().SetDoRealityDataUpload(true);
        if (!m_jobEnvArgs.m_realityDataDir.empty())
            m_bridge->_GetParams().SetRealityDataDir(m_jobEnvArgs.m_realityDataDir);
        }

    if (m_bridge->TestFeatureFlag("imodel-bridge-update-geometry-parts"))
        m_bridge->_GetParams().SetUpdateGeometryParts(true);

    if (m_bridge->TestFeatureFlag("imodel-bridge-terrain-conversion"))
        m_bridge->_GetParams().SetDoTerrainModelConversion(true);

    bool doNotTrackRefs = false;
    TestFeatureFlag("imodel-bridge-do-not-track-references-subjects", doNotTrackRefs);
    m_bridge->_GetParams().SetDoNotTrackReferencesSubjects(doNotTrackRefs);

    bool createdNewRepo = false;
    if (BSISUCCESS != BootstrapBriefcase(createdNewRepo, context))
        {
        m_briefcaseDgnDb = nullptr;
        errorContext.WriteErrorMessage(errorFile);
        return errorContext.GetIntErrorId();
        }

    iModelBridge::LogPerformance(briefcaseTime, "Getting iModel Briefcase from iModelHub");
    LOG.tracev(L"Setting up iModel Briefcase for processing  : Done");
    }

    int status;
    m_lastError = &errorContext;
    try
        {
        LOG.tracev(L"UpdateExistingBim...");
        StopWatch updateExistingBim(true);
        status = UpdateExistingBimWithExceptionHandling(context);
        iModelBridge::LogPerformance(updateExistingBim, "Updating Existing Bim file.");
        LOG.tracev(L"UpdateExistingBim  : Done (status=%x)", status);
        }
    catch (...)
        {
        LOG.fatal("UpdateExistingBim failed");
        status = RETURN_STATUS_LOCAL_ERROR;
        }
    m_lastError = nullptr;

    SetCurrentPhaseName("Cleaning up");
    GetProgressMeter().AddSteps(2);

    if (m_briefcaseDgnDb.IsValid())     // must make sure briefcase dgndb is closed before tearing down host!
        {
        GetProgressMeter().SetCurrentStepName("Releasing locks");

        LOG.tracev(L"ReleaseAllPublicLocks...");
        Briefcase_ReleaseAllPublicLocks(SUCCESS != status);  // regardless of the success or failure of the bridge, we must not hold onto any locks that are not the private property of the bridge
        LOG.tracev(L"ReleaseAllPublicLocks : Done");

        if (BSISUCCESS != status)
            {
            LOG.tracev(L"AbandonChanges while cleaning up after update error");
            m_briefcaseDgnDb->AbandonChanges();
            }

        m_briefcaseDgnDb = nullptr;
        }

    //We are done processing the dgn db file. Release the bridge
    GetProgressMeter().SetCurrentStepName("Releasing bridge");
    LOG.tracev(L"ReleaseBridge...");
    if (SUCCESS != ReleaseBridge())
        LOG.errorv(L"%s - Memory leak. This bridge was not released properly.", m_jobEnvArgs.m_bridgeRegSubKey.c_str());
    LOG.tracev(L"ReleaseBridge         : Done");

    if (m_useIModelHub)
        {
        LOG.tracev(L"iModelBridgeLdClient close...");
        iModelBridgeLdClient::GetInstance(m_iModelHubArgs->m_environment).Close();
        LOG.tracev(L"iModelBridgeLdClient close: Done");
        }

    s_fwkInstanceForSignalHandler = nullptr;

    if (SUCCESS != status)
        {
        errorContext.WriteErrorMessage(errorFile);
        return errorContext.GetIntErrorId();
        }

    T_HOST.GetProgressMeter()->Hide();

    LOG.tracev(L"iModelBridgeFwk::RunExclusive:  Done");

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::PullMergeAndPushChange(Utf8StringCR description, bvector<Utf8String> const* changedFiles, iModel::Hub::ChangeSetKind  changes, bool releaseLocks, bool reopenDb)
    {
    GetLogger().infov("bridge:%s iModel:%s - PullMergeAndPushChange %s.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str(), description.c_str());

    //  Push the pending schema change to iModelHub in its own changeset
    if (BeSQLite::BE_SQLITE_OK != m_briefcaseDgnDb->SaveChanges())
        return RETURN_STATUS_SERVER_ERROR; // (probably a failure to obtain locks or reserve codes)

    if (BSISUCCESS != Briefcase_PullMergePush(description.c_str(), changedFiles, changes))
        return RETURN_STATUS_SERVER_ERROR;

    if (releaseLocks)
        Briefcase_ReleaseAllPublicLocks();

    // >------> pullmergepush *may* have pulled schema changes -- close and re-open the briefcase in order to merge them in <-----------<
    if (!reopenDb)
        return SUCCESS;

    DbResult dbres;
    bool madeSchemaChanges = false;
    auto channelProps = m_briefcaseDgnDb->BriefcaseManager().GetChannelProps();
    m_briefcaseDgnDb = nullptr; // close the current connection to the briefcase db before attempting to reopen it!
    m_briefcaseDgnDb = iModelBridge::OpenBimAndMergeSchemaChanges(dbres, madeSchemaChanges, m_briefcaseName);
    if (!m_briefcaseDgnDb.IsValid())
        {
        ReportIssue(BeSQLite::Db::InterpretDbResult(dbres));
        GetLogger().fatalv("OpenDgnDb failed with error %s (%x)", BeSQLite::Db::InterpretDbResult(dbres), dbres);
        return BentleyStatus::ERROR;
        }
    m_briefcaseDgnDb->BriefcaseManager().SetChannelProps(channelProps);

    BeAssert(!madeSchemaChanges && "No further domain schema changes were expected.");
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String   iModelBridgeFwk::GetRevisionComment()
    {
    //Revision comment override from command line has the first priority
    Utf8String truncatedCommentString;
    //if (!m_jobEnvArgs.m_revisionComment.empty())
        truncatedCommentString = m_jobEnvArgs.m_revisionComment;

    //See TFS#819945: IMBridgeFwk - must truncate changeset description < 400 chars
    if (truncatedCommentString.size() > 399)
        truncatedCommentString.erase(399, std::string::npos);
    return truncatedCommentString;
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
* @bsimethod                                    Sam.Wilson                      09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isUpgradeFailure(DbResult dbres)
    {
    return (DbResult::BE_SQLITE_ERROR_ProfileUpgradeFailed == dbres) || (DbResult::BE_SQLITE_ERROR_SchemaUpgradeFailed == dbres);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::TryOpenBimWithOptions(DgnDb::OpenParams& oparams)
    {
    if (m_briefcaseDgnDb.IsValid())
        return SUCCESS;

    GetLogger().trace("Entering TryOpenBimWithOptions");
    StopWatch openBimWithProfileUpgrade(true);
    bool madeSchemaChanges = false;
    DbResult dbres;
    m_briefcaseDgnDb = iModelBridge::OpenBimAndMergeSchemaChanges(dbres, madeSchemaChanges, m_briefcaseName, oparams);
    uint8_t retryopenII = 0;
    while (!m_briefcaseDgnDb.IsValid() && isUpgradeFailure(dbres) && (++retryopenII < m_maxRetryCount) && IModelClientBase::SleepBeforeRetry(m_maxRetryWait))
        {
        // The upgrade may have failed because we could not get the schema lock, and that may have failed because
        // another briefcase pushed schema changes after this briefcase last pulled.
        // If so, then we have to pull before re-trying.
        GetLogger().infov("SchemaUpgrade failed. Pulling.");
        DgnDb::OpenParams tmpParams(oparams);
        tmpParams.GetSchemaUpgradeOptionsR().SetUpgradeFromDomains(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck);
        tmpParams.SetProfileUpgradeOptions(EC::ECDb::ProfileUpgradeOptions::None);
        m_briefcaseDgnDb = DgnDb::OpenDgnDb(&dbres, m_briefcaseName, tmpParams);
        Briefcase_PullMergePush("", NULL, iModel::Hub::ChangeSetKind::NotSpecified);    // TRICKY Only Briefcase_PullMergePush contains the mergeschemachanges logic. Briefcase_PullAndMerge does not.
        m_briefcaseDgnDb = nullptr;

        GetLogger().infov("Retrying SchemaUpgrade (if still necessary).");
        m_briefcaseDgnDb = iModelBridge::OpenBimAndMergeSchemaChanges(dbres, madeSchemaChanges, m_briefcaseName,oparams);
        }
    if (!m_briefcaseDgnDb.IsValid())
        {
        ReportIssue(BeSQLite::Db::InterpretDbResult(dbres));
        GetLogger().fatalv("OpenDgnDb failed with error %s (%x)", BeSQLite::Db::InterpretDbResult(dbres), dbres);
        return BentleyStatus::ERROR;
        }

    writeBriefcaseIdTxtFile(m_briefcaseName, m_briefcaseDgnDb->GetBriefcaseId()); // write this as early as possible, so that caller can release locks in case of a crash that defeats fwk's own cleanup code.

    //                                       *** NB: CALLER CLEANS UP m_briefcaseDgnDb! ***
    if (madeSchemaChanges || iModelBridge::AnyChangesToPush(*m_briefcaseDgnDb))
        {
        if (0 != PullMergeAndPushChange(iModelBridgeFwkMessages::GetString(iModelBridgeFwkMessages::DOMAIN_SCHEMA_UPGRADE()), NULL, iModel::Hub::ChangeSetKind::Schema, true, oparams.GetProfileUpgradeOptions() != EC::ECDb::ProfileUpgradeOptions::Upgrade))  // pullmergepush + re-open
            return BSIERROR;
        }

    iModelBridge::LogPerformance(openBimWithProfileUpgrade, "TryOpenBimWithBisSchemaUpgrade");
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::TryOpenBimWithBimProfileUpgrade()
    {
    GetLogger().trace("Entering TryOpenBimWithBimProfileUpgrade");
    DgnDb::OpenParams oparams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Exclusive);
    oparams.GetSchemaUpgradeOptionsR().SetUpgradeFromDomains(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck);
    oparams.SetProfileUpgradeOptions(EC::ECDb::ProfileUpgradeOptions::Upgrade);
    return TryOpenBimWithOptions(oparams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::TryOpenBimWithBisSchemaUpgrade()
    {
    GetLogger().trace("Entering TryOpenBimWithBisSchemaUpgrade");
    DgnDb::OpenParams oparams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Exclusive);
    oparams.GetSchemaUpgradeOptionsR().SetUpgradeFromDomains(SchemaUpgradeOptions::DomainUpgradeOptions::CheckRecommendedUpgrades);
    return TryOpenBimWithOptions(oparams);
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
* @bsimethod                                    Sam.Wilson                      04/20
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::EnterNormalChannel(DgnElementId channelParent)
    {
    m_briefcaseDgnDb->BriefcaseManager().GetChannelPropsR().channelType = IBriefcaseManager::ChannelType::Normal;
    m_briefcaseDgnDb->BriefcaseManager().GetChannelPropsR().channelParentId = channelParent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/20
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::EnterSharedChannel()
    {
    m_briefcaseDgnDb->BriefcaseManager().GetChannelPropsR().channelType = IBriefcaseManager::ChannelType::Shared;
    m_briefcaseDgnDb->BriefcaseManager().GetChannelPropsR().channelParentId = m_briefcaseDgnDb->Elements().GetRootSubjectId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::GetSchemaLock()
    {
    GetLogger().infov("GetSchemaLock.");

    RepositoryStatus status = RepositoryStatus::Success;
    int retryAttempt = 0;
    do
        {
        if (retryAttempt > 0)
            {
            GetLogger().infov("GetSchemaLock failed. Retrying.");
            PostStatusMessage("GetSchemaLock failed. Retrying.");
            if (0 != PullMergeAndPushChange("GetSchemaLock", NULL, iModel::Hub::ChangeSetKind::NotSpecified, false, true))  // pullmergepush + re-open
                return BSIERROR;
            }
        auto response = m_briefcaseDgnDb->BriefcaseManager().LockSchemas();
        status = response.Result();
        SetLastError(response, "");
        } while ((RepositoryStatus::Success != status) && (++retryAttempt < m_maxRetryCount) && IModelClientBase::SleepBeforeRetry(m_maxRetryWait));

    if (RepositoryStatus::Success != status)
        GetLogger().warningv(L"GetSchemaLock failed with status %x. briefcase=%ls", status, m_briefcaseName.c_str());

    return (status != RepositoryStatus::Success) ? BSIERROR : BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::LockChannelParent(SubjectCR jobSubj)
    {
    auto& db = jobSubj.GetDgnDb();

    if (!db.BriefcaseManager().StayInChannel() || db.BriefcaseManager().IsNoChannel())
        return BSISUCCESS;

    if (jobSubj.GetElementId() != db.BriefcaseManager().GetChannelPropsR().channelParentId)
        {
        BeAssert(false);
        LOG.fatalv("Specified Job Subject element %llx is not the channel parent", jobSubj.GetElementId().GetValue());
        return BSIERROR;
        }

    GetLogger().infov("LockChannelParent %llx.", jobSubj.GetElementId().GetValue());

    RepositoryStatus status = RepositoryStatus::Success;
    int retryAttempt = 0;
    do
        {
        if (retryAttempt > 0)
            {
            GetLogger().infov("LockChannelParent failed. Retrying.");
            PostStatusMessage("LockChannelParent failed. Retrying.");
            if (0 != PullMergeAndPushChange("LockChannelParent", NULL, iModel::Hub::ChangeSetKind::NotSpecified, false, true))  // pullmergepush + re-open
                return BSIERROR;
            }
        auto resp = db.BriefcaseManager().LockChannelParent();
        auto errId = iModelBridgeErrorId::FailedToLockChannelParent;
        SetLastError(resp, "", &errId);
        status = resp.Result();
        } while ((RepositoryStatus::Success != status) && (++retryAttempt < m_maxRetryCount) && IModelClientBase::SleepBeforeRetry(m_maxRetryWait));

    if (RepositoryStatus::Success != status)
        {
        LOG.fatalv("Failed to acquire exclusive lock on the Job Subject element %llx. Status = %x", jobSubj.GetElementId().GetValue(), status);
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::LockChannelParent(DgnElementId channelParentId)
    {
    auto jobSubj = m_briefcaseDgnDb->Elements().Get<Subject>(channelParentId);
    if (!jobSubj.IsValid())
        {
        BeAssert(false);
        return BSIERROR;
        }
    return LockChannelParent(*jobSubj);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool iModelBridgeFwk::HoldsJobSubjectLock()
    {
    auto jobsubj = m_briefcaseDgnDb->Elements().Get<Subject>(m_briefcaseDgnDb->BriefcaseManager().GetChannelPropsR().channelParentId);
    if (!jobsubj.IsValid())
        return false;

    return iModelBridge::HoldsElementLock(*jobsubj);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::MustHoldJobSubjectLock()
    {
    if (!m_briefcaseDgnDb->BriefcaseManager().StayInChannel())
        return BSISUCCESS;

    if (!m_briefcaseDgnDb->BriefcaseManager().GetChannelPropsR().channelParentId.IsValid())
        {
        BeAssert(false);
        LOG.fatalv("Expected to be in a normal channel with parent id set.");
        return BSIERROR;
        }

    if (!HoldsJobSubjectLock())
        {
        BeAssert(false);
        LOG.fatalv("Expected to hold exclusive lock on Job Subject element %llx", m_briefcaseDgnDb->BriefcaseManager().GetChannelPropsR().channelParentId.GetValue());
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::MakeSchemaChanges(iModelBridgeCallOpenCloseFunctions& callCloseOnReturn)
    {
    BeAssert(m_briefcaseDgnDb->BriefcaseManager().IsSharedChannel());
    BeAssert(iModelBridge::HoldsSchemaLock(*m_briefcaseDgnDb));

    GetLogger().infov("bridge:%s iModel:%s - MakeSchemaChanges.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());
    bool importedAspectSchema = false;

    BeAssert(!m_briefcaseDgnDb->BriefcaseManager().IsBulkOperation());


    bool hasMoreSchemaChanges = false;
    do {
        m_briefcaseDgnDb->BriefcaseManager().StartBulkOperation();
        bool runningInBulkMode = m_briefcaseDgnDb->BriefcaseManager().IsBulkOperation();

        int bridgeSchemaChangeStatus = m_bridge->_MakeSchemaChanges(hasMoreSchemaChanges);
        if (BSISUCCESS != bridgeSchemaChangeStatus)
            {
            uint8_t retryAttempt = 0;
            while ((BSISUCCESS != bridgeSchemaChangeStatus) && (++retryAttempt < m_maxRetryCount) && IModelClientBase::SleepBeforeRetry(m_maxRetryWait))
                {
                GetLogger().infov("_MakeSchemaChanges failed. Retrying.");
                PostStatusMessage("_MakeSchemaChanges failed. Retrying.");
                callCloseOnReturn.CallCloseFunctions(iModelBridge::ClosePurpose::SchemaUpgrade); // re-initialize the bridge, to clear out the side-effects of the previous failed attempt
                m_briefcaseDgnDb->AbandonChanges();
                if (BSISUCCESS != PullMergeAndPushChange("dynamic schemas", NULL, iModel::Hub::ChangeSetKind::Schema, false, true))    // make sure that we are at the tip and that we have absorbed any schema changes from the server
                    return RETURN_STATUS_SERVER_ERROR;
                callCloseOnReturn.CallOpenFunctions(*m_briefcaseDgnDb);
                bridgeSchemaChangeStatus = m_bridge->_MakeSchemaChanges(hasMoreSchemaChanges);
                }
            }
        if (BSISUCCESS != bridgeSchemaChangeStatus)
            {
            LOG.fatalv("Bridge _MakeSchemaChanges failed");
            return BentleyStatus::ERROR;
            }

        BeAssert(!runningInBulkMode || m_briefcaseDgnDb->BriefcaseManager().IsBulkOperation());

        bool madeSchemaChanges = importedAspectSchema || iModelBridge::AnyChangesToPush(*m_briefcaseDgnDb);
        if (madeSchemaChanges)
            {
            BeAssert(iModelBridge::HoldsSchemaLock(*m_briefcaseDgnDb));

            callCloseOnReturn.CallCloseFunctions(iModelBridge::ClosePurpose::SchemaUpgrade);
            if (0 != PullMergeAndPushChange("dynamic schemas", NULL, iModel::Hub::ChangeSetKind::Schema, false, true))  // pullmergepush + re-open
                return BSIERROR;
            callCloseOnReturn.CallOpenFunctions(*m_briefcaseDgnDb);

            BeAssert(iModelBridge::HoldsSchemaLock(*m_briefcaseDgnDb));
            }
        else
            {
            if (runningInBulkMode)
                m_briefcaseDgnDb->BriefcaseManager().EndBulkOperation();
            }
        //!While opening a briefcase, the bridge could have made some changes. These may not be real changes
        // like GetDgnDb().GeoLocation().Save(); Dgnv8GeoCord.cpp. So save the file . This will clear the session and txns. Usually empty.
        //! Push the txns if there are any.
        DbResult dbres = m_briefcaseDgnDb->SaveChanges();
        if (BeSQLite::BE_SQLITE_OK != dbres)
            {
            GetLogger().errorv("Db::SaveChanges failed with status %d", dbres);
            return RETURN_STATUS_LOCAL_ERROR;
            }
        if (iModelBridge::AnyChangesToPush(*m_briefcaseDgnDb)) // if bridge made any changes, they must be pushed and cleared out before we can make schema changes
            {
            if (BSISUCCESS != Briefcase_PullMergePush(" File initialization changes", NULL, madeSchemaChanges ? iModel::Hub::ChangeSetKind::Schema: iModel::Hub::ChangeSetKind::NotSpecified))
                return RETURN_STATUS_SERVER_ERROR;
            }

        BeAssert(!m_briefcaseDgnDb->Txns().HasLocalChanges());//Put a breakpoint in filtertable to catch any changes.
        }while (hasMoreSchemaChanges);

    BeAssert(m_briefcaseDgnDb->BriefcaseManager().IsSharedChannel());
    BeAssert(iModelBridge::HoldsSchemaLock(*m_briefcaseDgnDb));
    BeAssert(!iModelBridge::AnyTxns(*m_briefcaseDgnDb)); // (Note that _OnOpenBim might have made IN-MEMORY changes to be_prop table.)

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::MakeDefinitionChanges(SubjectCPtr& jobsubj, iModelBridgeCallOpenCloseFunctions& callCloseOnReturn)
    {
    BeAssert(m_briefcaseDgnDb->BriefcaseManager().IsSharedChannel());
    BeAssert(iModelBridge::HoldsSchemaLock(*m_briefcaseDgnDb));

    GetLogger().infov("bridge:%s iModel:%s - MakeDefinitionChanges.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());

    int bridgeSchemaChangeStatus = m_bridge->DoMakeDefinitionChanges(jobsubj, *m_briefcaseDgnDb);
    if (BSISUCCESS != bridgeSchemaChangeStatus)
        {
        jobsubj = nullptr;
        LOG.fatalv("Bridge DoMakeDefinitionChanges failed");
        return BentleyStatus::ERROR;
        }

    DbResult dbres = m_briefcaseDgnDb->SaveChanges();
    if (BeSQLite::BE_SQLITE_OK != dbres)
        {
        GetLogger().errorv("Db::SaveChanges failed with status %d", dbres);
        return RETURN_STATUS_LOCAL_ERROR;
        }
    bool madeDefinitionChanges = iModelBridge::AnyChangesToPush(*m_briefcaseDgnDb);
    if (madeDefinitionChanges)
        {
        // DON'T CLOSE AND REOPEN.
        // We can be confident that the merged data will be irrelevant to the bridge.
        // Because we hold the schema lock, we know that merging cannot pull down schema changes or other bridges' definition changes.
        // We also know that bridge's private (exclusively locked) models cannot have been changed. So, the only
        // changes that we might get would be to other bridges' models and irrelevant changes to the public models,
        // such as new job subjects from other bridges.

        //!TODO: Should we attribute definition changes to all the files in the reference graph ?
        if (BSISUCCESS != Briefcase_PullMergePush(iModelBridgeFwkMessages::GetString(iModelBridgeFwkMessages::DEFINITIONS()).c_str(), NULL, iModel::Hub::ChangeSetKind::Definition))
            return BSIERROR;
        }

    BeAssert(m_briefcaseDgnDb->BriefcaseManager().IsSharedChannel());
    BeAssert(iModelBridge::HoldsSchemaLock(*m_briefcaseDgnDb));
    BeAssert(!iModelBridge::AnyChangesToPush(*m_briefcaseDgnDb));

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::DoNormalUpdate()
    {
    // ---------------------------------------------------
    //  Definition Changes => shared channel
    // ---------------------------------------------------
    EnterSharedChannel();

    SetCurrentPhaseName("Schemas and Definitions");
    GetProgressMeter().AddSteps(3);

    BeAssert(!m_briefcaseDgnDb->BriefcaseManager().StayInChannel() || !HoldsJobSubjectLock());

    GetProgressMeter().SetCurrentStepName("Lock Schema");

    if (BSISUCCESS != GetSchemaLock())  // must get schema lock preemptively. This ensures that only one bridge at a time can make schema and definition changes. That then allows me to pull/merge/push between the definition and data steps without closing and reopening
        {
        LOG.fatalv("Bridge cannot obtain schema lock.");
        return RETURN_STATUS_SERVER_ERROR;
        }
                                                                                             // === SCHEMA LOCK
    //  Tell the bridge that the briefcase is now open and ask it to open the source file(s).// === SCHEMA LOCK
    iModelBridgeCallOpenCloseFunctions callCloseOnReturn(*m_bridge, *m_briefcaseDgnDb);      // === SCHEMA LOCK
    if (!callCloseOnReturn.IsReady())                                                        // === SCHEMA LOCK
        {                                                                                    // === SCHEMA LOCK
        LOG.fatalv("Bridge is not ready or could not open source file");                     // === SCHEMA LOCK
        return BentleyStatus::ERROR;                                                         // === SCHEMA LOCK
        }                                                                                    // === SCHEMA LOCK
                                                                                             // === SCHEMA LOCK
    DbResult dbres = m_briefcaseDgnDb->SaveChanges(); // If the _OnOpenBim or _OpenSource callbacks did things like attaching syncinfo, we need to commit that before going on.
                                                        // This also prevents a call to AbandonChanges in _MakeSchemaChanges from undoing what the open calls did.
    if (BeSQLite::BE_SQLITE_OK != dbres)                                                     // === SCHEMA LOCK
        {                                                                                    // === SCHEMA LOCK
        GetLogger().errorv("Db::SaveChanges failed with status %d", dbres);                  // === SCHEMA LOCK
        return RETURN_STATUS_LOCAL_ERROR;                                                    // === SCHEMA LOCK
        }                                                                                    // === SCHEMA LOCK
    if (iModelBridge::AnyChangesToPush(*m_briefcaseDgnDb)) // if bridge made any changes, they must be pushed and cleared out before we can make schema changes
        {                                                                                    // === SCHEMA LOCK
        if (BSISUCCESS != Briefcase_PullMergePush(iModelBridgeFwkMessages::GetString(iModelBridgeFwkMessages::INITIALIZATION_CHANGES()).c_str(), NULL, iModel::Hub::ChangeSetKind::NotSpecified))                 // === SCHEMA LOCK
            return RETURN_STATUS_SERVER_ERROR;                                               // === SCHEMA LOCK
        }                                                                                    // === SCHEMA LOCK

    GetProgressMeter().SetCurrentStepName("Bridge Schema Changes");
                                                                                            // === SCHEMA LOCK
    if (BSISUCCESS != MakeSchemaChanges(callCloseOnReturn))                                  // === SCHEMA LOCK
        {                                                                                    // === SCHEMA LOCK
        GetLogger().errorv("MakeSchemaChanges failed");                                      // === SCHEMA LOCK
        return RETURN_STATUS_CONVERTER_ERROR;                                                // === SCHEMA LOCK
        }                                                                                    // === SCHEMA LOCK
                                                                                             // === SCHEMA LOCK
    BeAssert(!iModelBridge::AnyTxns(*m_briefcaseDgnDb));                                     // === SCHEMA LOCK
                                                                                             // === SCHEMA LOCK
    if (SUCCESS != StoreHeaderInformation())                                                 // === SCHEMA LOCK
        GetLogger().warningv("bridge:%s iModel:%s - Storing iModel Bridge Header Data Failed.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());

    GetProgressMeter().SetCurrentStepName("Bridge Definition Changes");
                                                                                             // === SCHEMA LOCK
    SubjectCPtr jobsubj;                                                                     // === SCHEMA LOCK
    if (SUCCESS != MakeDefinitionChanges(jobsubj, callCloseOnReturn))                        // === SCHEMA LOCK
        {                                                                                    // === SCHEMA LOCK
        GetLogger().errorv("Bridge::DoMakeDefinitionChanges failed");                        // === SCHEMA LOCK
        return RETURN_STATUS_CONVERTER_ERROR;                                                // === SCHEMA LOCK
        }                                                                                    // === SCHEMA LOCK
                                                                                             // === SCHEMA LOCK
    Briefcase_ReleaseAllPublicLocks();

    if (s_testProbe)
        s_testProbe->_ReportJobSubjectId(jobsubj->GetElementId());

    // ---------------------------------------------------
    //  Normal data changes => bridge's private channel
    // ---------------------------------------------------
    EnterNormalChannel(jobsubj->GetElementId());

    SetCurrentPhaseName("Elements and Models");
    GetProgressMeter().AddSteps(2);

    BeAssert(!iModelBridge::HoldsSchemaLock(*m_briefcaseDgnDb));
    BeAssert(!m_briefcaseDgnDb->BriefcaseManager().StayInChannel() || !HoldsJobSubjectLock());
    BeAssert(!iModelBridge::AnyTxns(*m_briefcaseDgnDb));
    GetLogger().infov("bridge:%s iModel:%s - Convert Data.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());

    GetProgressMeter().SetCurrentStepName("Lock Channel");

    // Get exlusive access to the channel BEFORE going into bulk mode.
    if (BSISUCCESS != LockChannelParent(*jobsubj))
        return RETURN_STATUS_SERVER_ERROR;

    BeAssert(BSISUCCESS == MustHoldJobSubjectLock());                                                   // ==== CHANNEL LOCK

    GetProgressMeter().SetCurrentStepName("Bridge Data");
                                                                                                        // ==== CHANNEL LOCK
    BentleyStatus bridgeCvtStatus = m_bridge->DoConvertToExistingBim(*m_briefcaseDgnDb, *jobsubj, true);// ==== CHANNEL LOCK
    if (BSISUCCESS != bridgeCvtStatus)                                                                  // ==== CHANNEL LOCK
        {                                                                                               // ==== CHANNEL LOCK
        GetLogger().errorv("Bridge::DoConvertToExistingBim failed with status %d", bridgeCvtStatus);    // ==== CHANNEL LOCK
        return RETURN_STATUS_CONVERTER_ERROR;                                                           // ==== CHANNEL LOCK
        }                                                                                               // ==== CHANNEL LOCK
                                                                                                        // ==== CHANNEL LOCK
    //Call save changes before the bridge is closed.                                                    // ==== CHANNEL LOCK
    dbres = m_briefcaseDgnDb->SaveChanges();                                                            // ==== CHANNEL LOCK
                                                                                                        // ==== CHANNEL LOCK
    callCloseOnReturn.m_status = BSISUCCESS;                                                            // ==== CHANNEL LOCK
                                                                                                        // ==== CHANNEL LOCK
    BeAssert(!iModelBridge::HoldsSchemaLock(*m_briefcaseDgnDb));                                        // ==== CHANNEL LOCK
    BeAssert(BSISUCCESS == MustHoldJobSubjectLock());                                                   // ==== CHANNEL LOCK
                                                                                                        // ==== CHANNEL LOCK
    return RETURN_STATUS_SUCCESS;                                                                       // ==== CHANNEL LOCK
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::LockAllJobSubjects()
    {
    if (!m_briefcaseDgnDb->BriefcaseManager().StayInChannel() || m_briefcaseDgnDb->BriefcaseManager().IsNoChannel())
        return BSISUCCESS;

    Utf8String thisBridge(m_jobEnvArgs.m_bridgeRegSubKey);

    EC::ECSqlStatement stmt;
    stmt.Prepare(*m_briefcaseDgnDb, "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_Subject) " WHERE json_extract(JsonProperties, '$.Subject.Job') is not null");
    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto subj = m_briefcaseDgnDb->Elements().Get<Subject>(stmt.GetValueId<DgnElementId>(0));
        if (subj.IsValid() && (JobSubjectUtils::GetBridge(*subj) == thisBridge))
            {
            m_briefcaseDgnDb->BriefcaseManager().GetChannelPropsR().channelParentId = subj->GetElementId();

            if (LockChannelParent(*subj) != BSISUCCESS)
                return BSIERROR;     // caller will relinquish all locks
            }
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::OnAllDocsProcessed(FwkContext& context)
    {
    EnterNormalChannel(DgnElementId()); // This operation applies to multiple channels

    //  Tell the bridge that the briefcase is now open. (Do NOT ask it to open a source file.)
    iModelBridgeBriefcaseCallOpenCloseFunctions callCloseOnReturn(*m_bridge, *m_briefcaseDgnDb);
    if (!callCloseOnReturn.IsReady())
        {
        LOG.fatalv("Bridge is not ready or could not open source file");
        return RETURN_STATUS_LOCAL_ERROR;
        }

    DbResult dbres = m_briefcaseDgnDb->SaveChanges(); // If the _OnOpenBim or _OpenSource callbacks did things like attaching syncinfo, we need to commit that before going on.
                                                        // This also prevents a call to AbandonChanges in _MakeSchemaChanges from undoing what the open calls did.
    if (BeSQLite::BE_SQLITE_OK != dbres)
        {
        GetLogger().errorv("Db::SaveChanges failed with status %d", dbres);
        return RETURN_STATUS_LOCAL_ERROR;
        }

    if (!m_jobEnvArgs.CreateSnapshot() && iModelBridge::HoldsSchemaLock(*m_briefcaseDgnDb))
        {
        BeAssert(false);
        GetLogger().error("OnAllDocsProcessed detected that this briefcase is holding the Schema Lock! That is an error. OnAllDocsProcessed should not be called while holding the schema lock, and there should not have been any pending schema imports or upgrades to process.");
        return RETURN_STATUS_CONVERTER_ERROR;
        }

    if (iModelBridge::AnyTxns(*m_briefcaseDgnDb))
        {
        BeAssert(false);
        GetLogger().error("All local changes should have been pushed before calling OnAllDocsProcessed");
        return RETURN_STATUS_CONVERTER_ERROR;
        }

    if (!m_jobEnvArgs.CreateSnapshot())
        {
        if (BSISUCCESS != LockAllJobSubjects())
            {
            GetLogger().error("Failed to get Channel Parent Subject locks before calling OnAllDocsProcessed");
            return RETURN_STATUS_SERVER_ERROR;
            }
        }
                                                                                                        // ==== CHANNEL LOCKS (caller will relinquish them all)
    GetLogger().infov("bridge:%s iModel:%s - OnAllDocsProcessed.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());
                                                                                                        // ==== CHANNEL LOCKS
    BentleyStatus bridgeCvtStatus = m_bridge->DoOnAllDocumentsProcessed(*m_briefcaseDgnDb);             // ==== CHANNEL LOCKS
    if (BSISUCCESS != bridgeCvtStatus)                                                                  // ==== CHANNEL LOCKS
        {                                                                                               // ==== CHANNEL LOCKS
        GetLogger().errorv("Bridge::DoOnAllDocumentsProcessed failed with status %d", bridgeCvtStatus); // ==== CHANNEL LOCKS
        return RETURN_STATUS_CONVERTER_ERROR;                                                           // ==== CHANNEL LOCKS
        }                                                                                               // ==== CHANNEL LOCKS

	// Technically we only need to do this if the files were converted with the 'skipExtents' flag, but to be safe we always do it
    UpdateProjectExtents(context);
                                                                                                        // ==== CHANNEL LOCKS
    //Call save changes before the bridge is closed.                                                    // ==== CHANNEL LOCKS
    if (m_briefcaseDgnDb->Txns().HasChanges())
        m_briefcaseDgnDb->SaveChanges();                                                                // ==== CHANNEL LOCKS

    if (!m_jobEnvArgs.CreateSnapshot())
        {
        PushDataChanges(iModelBridgeFwkMessages::GetString(iModelBridgeFwkMessages::EXTENT_CHANGES()), NULL, iModel::Hub::ChangeSetKind::GlobalProperties);     // ==== CHANNEL LOCKS
        callCloseOnReturn.m_status = BSISUCCESS;                                                        // ==== CHANNEL LOCKS
                                                                                                        // ==== CHANNEL LOCKS
        BeAssert(!iModelBridge::HoldsSchemaLock(*m_briefcaseDgnDb));                                    // ==== CHANNEL LOCKS
                                                                                                        // ==== CHANNEL LOCKS
        }
	return RETURN_STATUS_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
int   iModelBridgeFwk::PushDataChanges(Utf8StringCR pushCommentIn, bvector<Utf8String>* changedFiles, iModel::Hub::ChangeSetKind  changes)
    {
    if (!iModelBridge::AnyTxns(*m_briefcaseDgnDb) && (SyncState::Initial == GetSyncState()))
        {
        Briefcase_ReleaseAllPublicLocks();
        return SUCCESS;
        }

    BeAssert(!m_briefcaseDgnDb->Txns().HasChanges());

    GetLogger().infov("bridge:%s iModel:%s - Pushing Data Changeset.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());
    GetProgressMeter().SetCurrentStepName("Pushing Changes");
    Utf8String comment = GetRevisionComment().c_str();
    if (!comment.empty() && !pushCommentIn.empty())
        comment.append(" - ");

    comment.append(pushCommentIn);
    if (BSISUCCESS != Briefcase_PullMergePush(comment.c_str(), changedFiles, changes))
        return RETURN_STATUS_SERVER_ERROR; // (Retain shared locks, so that we can re-try our push later.)

    BeAssert(!iModelBridge::AnyTxns(*m_briefcaseDgnDb));

    GetProgressMeter().SetCurrentStepName("Releasing Locks");
    if (BSISUCCESS != Briefcase_ReleaseAllPublicLocks())
        return RETURN_STATUS_SERVER_ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::UpdateExistingBim(iModelBridgeFwk::FwkContext& context)
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

    SetCurrentPhaseName("Opening Briefcase");
    GetProgressMeter().AddSteps(2);

    GetLogger().infov("bridge:%s iModel:%s - Opening briefcase I.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());

    // ***
    // *** TRICKY: Do not call InitBridge until we have done PullMergePush
    // ***          That is because the bridge may register a new or upgraded schema, which will require DgnDb::OpenDgnDb to do an import or upgrade.
    // ***          At that time, we must ensure that the BIM does not contain any lingering local Txns and that it is as the tip (so that we can get the schema lock).
    // ***          So, we need to be able to open the BIM just in order to pull/merge/push, before we allow the bridge to add a schema import/upgrade into the mix.
    // ***
    //  By getting the BIM to the tip, this initial pull also helps ensure that we will be able to get locks for the other changes that that bridge will make later.
    if (EnableECProfileUpgrade())
        {
        GetProgressMeter().SetCurrentStepName("Checking for profile upgrade");
        LOG.tracev(L"TryOpenBimWithBimProfileUpgrade");
        if (BSISUCCESS != TryOpenBimWithBimProfileUpgrade())
            return BentleyStatus::ERROR;
        LOG.tracev(L"TryOpenBimWithBimProfileUpgrade  : Done");
        DbResult dbres = m_briefcaseDgnDb->SaveChanges();
        if (BeSQLite::BE_SQLITE_OK != dbres)
            {
            GetLogger().errorv("Db::SaveChanges failed with status %d", dbres);
            return RETURN_STATUS_LOCAL_ERROR;
            }

        Briefcase_ReleaseAllPublicLocks();
        m_briefcaseDgnDb = nullptr;             // This is safe, because we released all locks.
        }

    LOG.tracev(L"TryOpenBimWithBisSchemaUpgrade");
    GetProgressMeter().SetCurrentStepName("Checking for BIS schema upgrade");
    if (BSISUCCESS != TryOpenBimWithBisSchemaUpgrade())
        return BentleyStatus::ERROR;
    LOG.tracev(L"TryOpenBimWithBisSchemaUpgrade  : Done");

    LOG.tracev(L"Merge schema changes");
    LOG.tracev(L"Merge schema changes (PullMergePush)");
    if (BSISUCCESS != Briefcase_PullMergePush("", NULL, iModel::Hub::ChangeSetKind::Schema)) //TODO: I am not sure whether we need this since TryOpenBimWithBisSchemaUpgrade has already pushed the changes
        return RETURN_STATUS_SERVER_ERROR;

    //                                       *** NB: CALLER CLEANS UP m_briefcaseDgnDb! ***

    // >------> pullmergepush *may* have pulled schema changes -- close and re-open the briefcase in order to merge them in <-----------<

    LOG.tracev(L"Merge schema changes (SaveChanges)");
    DbResult dbres = m_briefcaseDgnDb->SaveChanges();
    if (BeSQLite::BE_SQLITE_OK != dbres)
        {
        GetLogger().errorv("Db::SaveChanges failed with status %d", dbres);
        return RETURN_STATUS_LOCAL_ERROR;
        }

    LOG.tracev(L"Merge schema changes (ReleaseAllPublicLocks)");
    Briefcase_ReleaseAllPublicLocks();
    m_briefcaseDgnDb = nullptr;             // This is safe, because we released all locks.

    LOG.tracev(L"Merge schema changes        : Done");

    // Now initialize the bridge.
    LOG.tracev(L"InitBridge");
    if (BSISUCCESS != InitBridge())
        return BentleyStatus::ERROR;
    LOG.tracev(L"InitBridge                  : Done");

    bool hadBridgeChanges = false;
    if (true)
        {
        iModelBridgeCallTerminate callTerminate(*m_bridge);

        GetLogger().infov("bridge:%s iModel:%s - Opening briefcase II.", Utf8String(m_jobEnvArgs.m_bridgeRegSubKey).c_str(), m_briefcaseBasename.c_str());

        ReportFeatureFlags();

        // Open the briefcase in the normal way, allowing domain schema changes to be pulled in.
        m_briefcaseDgnDb = nullptr; // close the current connection to the briefcase db before attempting to reopen it!

        // *** NEEDS WORK: There is a race condition here. See the comment in MstnBridgeTests.MultiBridgeSequencing.

        if (BSISUCCESS != TryOpenBimWithBisSchemaUpgrade())
            {
            LOG.fatalv("Bridge cannot open and perform Bis Schema Upgrade (may be denied schema lock).");
            return BentleyStatus::ERROR;
            }

        BeAssert(!iModelBridge::AnyTxns(*m_briefcaseDgnDb));


        int res;
        if (!m_jobEnvArgs.m_allDocsProcessed)
            {
            LOG.tracev(L"DoNormalUpdate");
            res = DoNormalUpdate();
            LOG.tracev(L"DoNormalUpdate              : Done (result=%x)", res);
            }
        else
            {
            LOG.tracev(L"OnAllDocsProcessed");
            res = OnAllDocsProcessed(context);
            LOG.tracev(L"OnAllDocsProcessed          : Done (result=%x)", res);
            }

        if (0 != res)
            return res;

        hadBridgeChanges = m_bridge->HadAnyChanges();
        callTerminate.m_status = BSISUCCESS;
        }

    LOG.tracev(L"PushDataChanges");
    bvector<Utf8String> changedFiles;
    changedFiles.push_back(m_jobEnvArgs.m_inputFileName.GetBaseName().GetNameUtf8());
    PushDataChanges(iModelBridgeFwkMessages::GetString(iModelBridgeFwkMessages::DATA_CHANGES()), &changedFiles, iModel::Hub::ChangeSetKind::Regular);
    LOG.tracev(L"PushDataChanges                :Done");

    //  Finalize changes in the shared channel
    SetCurrentPhaseName("Finalizing Changes");
    GetProgressMeter().AddSteps(2);

    EnterSharedChannel();

    if (BSISUCCESS != GetSchemaLock())  // must get schema lock preemptively. This ensures that only one bridge at a time can make schema and definition changes. That then allows me to pull/merge/push between the definition and data steps without closing and reopening
        {
        LOG.fatalv("Bridge cannot obtain schema lock.");
        return RETURN_STATUS_SERVER_ERROR;
        }

    if (!m_jobEnvArgs.m_allDocsProcessed)
        {
        LOG.tracev(L"DoFinalizationChanges");
        m_bridge->DoFinalizationChanges(*m_briefcaseDgnDb);
        LOG.tracev(L"DoFinalizationChanges          : Done");
        if (!m_jobEnvArgs.m_argsJson.isMember("skipExtents"))
            {
            LOG.tracev(L"UpdateProjectExtents");
            UpdateProjectExtents(context);
            LOG.tracev(L"UpdateProjectExtents       : Done");
            }
        //SetUpECEFLocation(context);
        }

    // Running ANALYZE allows SQLite to create optimize execution plans when running queries. It should be be included in changeset that bridges post.
    if (hadBridgeChanges)
        {
        LOG.tracev(L"ANALYZE");
        m_briefcaseDgnDb->ExecuteSql("ANALYZE");
        LOG.tracev(L"ANALYZE                        : Done");
        }

    LOG.tracev(L"Final Save and Push");
    dbres = m_briefcaseDgnDb->SaveChanges();

    //*** NB: CALLER CLEANS UP m_briefcaseDgnDb! ***

    if (BeSQLite::BE_SQLITE_OK != dbres)
        {
        GetLogger().errorv("Db::SaveChanges failed with status %d", dbres);
        return RETURN_STATUS_LOCAL_ERROR;
        }

    //  Done. Make sure that all changes are pushed and all shared locks are released.
    PushDataChanges(iModelBridgeFwkMessages::GetString(iModelBridgeFwkMessages::FINALIZATION()), &changedFiles, iModel::Hub::ChangeSetKind::GlobalProperties);

    LOG.tracev(L"Final Save and Push:           Done");

    // *** NB: CALLER CLEANS UP m_briefcaseDgnDb! ***

    // Set this value as a report of the Bridge/BriefcaseManager's policy, so that callers can find out afterwards.
    m_areCodesInLockedModelsReported = m_briefcaseDgnDb->BriefcaseManager().ShouldIncludeUsedLocksInChangeSet();
    m_retainedChannedlLockLevel = m_briefcaseDgnDb->BriefcaseManager().GetChannelPropsR().oneBriefcaseOwnsChannel ? BentleyApi::Dgn::LockLevel::Exclusive : BentleyApi::Dgn::LockLevel::None;

    // POST-CONDITIONS
    BeAssert((!iModelBridge::AnyTxns(*m_briefcaseDgnDb) && (SyncState::Initial == GetSyncState())) && "Local changes should have been pushed");

    return RETURN_STATUS_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::CreateNewSnapshot(iModelBridgeError& error, Utf8CP rootSubjectDescription)
    {
    // The caller has already done the following:
    // - Initialized the CRT, the DgnPlatformLib::Host, sqlang, and logging.
    // - Initialized the progress meter
    // - Initialized crash-reporting and signal-handler
    // - Set m_briefcaseName and m_briefcaseBasename
    // - Loaded the bridge (m_bridge)
    // - Initialized the bridge (InitBridge)
    // - Set up the bridge Parameters (including GetBriefcaseName)

    // The caller will:
    //  - DoFinalizationChanges, UpdateProjectExents
    //  - call SaveChanges or CancelChanges
    //  - Terminate the bridge

    // LEAVE m_briefcaseDgnDb DEFINED AND OPEN!

    error.m_id = iModelBridgeErrorId::Converter_Error;

    CreateDgnDbParams createProjectParams;
    if (nullptr != rootSubjectDescription)
        createProjectParams.SetRootSubjectDescription(rootSubjectDescription);

    Utf8String rootSubjName(m_bridge->_GetParams().GetBriefcaseName().GetBaseName());
    createProjectParams.SetRootSubjectName(rootSubjName.c_str());

    // Create the DgnDb file. All currently registered domain schemas are imported.
    BeSQLite::DbResult rc;
    m_briefcaseDgnDb = DgnDb::CreateDgnDb(&rc, m_bridge->_GetParams().GetBriefcaseName(), createProjectParams);
    if (!m_briefcaseDgnDb.IsValid())
        {
        LOG.fatalv(L"Failed to create repository [%s] with error %x", m_bridge->_GetParams().GetBriefcaseName().c_str(), rc);
        return BSIERROR;
        }

    if (nullptr != rootSubjectDescription)
        m_briefcaseDgnDb->SavePropertyString(DgnProjectProperty::Description(), rootSubjectDescription);

    m_bridge->_GetParams().SetIsCreatingNewDgnDb(true);
    m_bridge->_GetParams().SetIsUpdating(false);

    iModelBridgeCallOpenCloseFunctions callCloseOnReturn(*m_bridge, *m_briefcaseDgnDb);
    if (!callCloseOnReturn.IsReady())
        {
        LOG.fatalv("Bridge is not ready or could not open source file");
        return BSIERROR;
        }

    m_briefcaseDgnDb->SaveChanges(); // If the _OnOpenBim or _OpenSource callbacks did things like attaching syncinfo, we need to commit that before going on.
                       // This also prevents a call to AbandonChanges in _MakeSchemaChanges from undoing what the open calls did.

    // Tell the bridge to generate schemas
    bool hasMoreChanges = false;
    do  {
        if (BSISUCCESS != m_bridge->_MakeSchemaChanges(hasMoreChanges))
            {
            LOG.fatalv("_MakeSchemaChanges failed");
            return BSIERROR;
            }
        }
    while (hasMoreChanges);

    EnterSharedChannel();
    SubjectCPtr jobsubj;
    BentleyStatus bstatus = m_bridge->DoMakeDefinitionChanges(jobsubj, *m_briefcaseDgnDb);
    if (BSISUCCESS == bstatus)
        {
        EnterNormalChannel(jobsubj->GetElementId());
        bstatus = m_bridge->DoConvertToExistingBim(*m_briefcaseDgnDb, *jobsubj, true);
        }

    if (s_testProbe)
        s_testProbe->_ReportJobSubjectId(jobsubj->GetElementId());

    return callCloseOnReturn.m_status = bstatus;
    // Call bridge's _CloseSource and _OnConvertedToBim
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::CreateSnapshotSecondBridge(iModelBridgeError& error)
    {
    //  This function is like CreateNewSnapshot, except that it allows a second bridge to populate
    //  a new channel in an existing masterfile. It therefore imports additional schemas and updates
    //  existing ones. That is where all of the complication comes in.
    //  TODO: We know that the file is NOT a briefcase. Does that simplify the schema import/update problem??

    // The caller has already done the following:
    // - Initialized the CRT, the DgnPlatformLib::Host, sqlang, and logging.
    // - Initialized the progress meter
    // - Initialized crash-reporting and signal-handler
    // - Set m_briefcaseName and m_briefcaseBasename
    // - Loaded the bridge (m_bridge)
    // - Initialized the bridge (InitBridge)
    // - Set up the bridge Parameters (including GetBriefcaseName)

    // The caller will:
    //  - DoFinalizationChanges, UpdateProjectExents
    //  - call SaveChanges or CancelChanges
    //  - Terminate the bridge

    // LEAVE m_briefcaseDgnDb DEFINED AND OPEN!

    error.m_id = iModelBridgeErrorId::Converter_Error;

    BeSQLite::DbResult dbres;
    bool _hadDomainSchemaChanges = false;
    m_briefcaseDgnDb = m_bridge->OpenBimAndMergeSchemaChanges(dbres, _hadDomainSchemaChanges, m_bridge->_GetParams().GetBriefcaseName());
    if (!m_briefcaseDgnDb.IsValid())
        {
        LOG.fatalv(L"%ls - file not found or could not be opened (error %x)\n", m_bridge->_GetParams().GetBriefcaseName().GetName(), (int)dbres);
        return BSIERROR;
        }

    //  Tell the bridge that the briefcase is now open and ask it to open the source file(s).
    iModelBridgeCallOpenCloseFunctions callCloseOnReturn(*m_bridge, *m_briefcaseDgnDb);
    if (!callCloseOnReturn.IsReady())
        {
        LOG.fatalv("Bridge is not ready or could not open source file");
        return BentleyStatus::ERROR;
        }

    m_briefcaseDgnDb->SaveChanges(); // If the _OnOpenBim or _OpenSource callbacks did things like attaching syncinfo, we need to commit that before going on.
                        // This also prevents a call to AbandonChanges in _MakeSchemaChanges from undoing what the open calls did.

    //  Let the bridge generate schema changes
    bool hasMoreChanges = false;
    do  {
        m_bridge->_MakeSchemaChanges(hasMoreChanges);

        auto dbres = m_briefcaseDgnDb->SaveChanges();
        if (BeSQLite::BE_SQLITE_OK != dbres)
            {
            LOG.fatalv("Db::SaveChanges called after _MakeSchemaChanges failed with status %d", dbres);
            return callCloseOnReturn.m_status = BentleyStatus::ERROR;
            }
        }
    while (hasMoreChanges);

    bool madeDynamicSchemaChanges = m_briefcaseDgnDb->Txns().HasChanges(); // see if _MakeSchemaChanges made any changes.

    if (madeDynamicSchemaChanges) // if _MakeSchemaChanges made any dynamic schema changes, we close and re-open in order to accommodate them.
        {
        callCloseOnReturn.CallCloseFunctions(iModelBridge::ClosePurpose::SchemaUpgrade);

        _hadDomainSchemaChanges = false;
        m_briefcaseDgnDb = m_bridge->OpenBimAndMergeSchemaChanges(dbres, _hadDomainSchemaChanges, m_bridge->_GetParams().GetBriefcaseName());
        if (!m_briefcaseDgnDb.IsValid())
            {
            LOG.fatalv(L"%ls - open failed with error %x\n", m_bridge->_GetParams().GetInputFileName().GetName(), (int)dbres);
            return BentleyStatus::ERROR;
            }
        BeAssert(!_hadDomainSchemaChanges);

        callCloseOnReturn.CallOpenFunctions(*m_briefcaseDgnDb);
        }

    EnterSharedChannel();
    SubjectCPtr jobsubj;
    BentleyStatus bstatus = m_bridge->DoMakeDefinitionChanges(jobsubj, *m_briefcaseDgnDb);
    if (BSISUCCESS == bstatus)
        {
        EnterNormalChannel(jobsubj->GetElementId());
        bstatus = m_bridge->DoConvertToExistingBim(*m_briefcaseDgnDb, *jobsubj, true);
        }

    if (s_testProbe)
        s_testProbe->_ReportJobSubjectId(jobsubj->GetElementId());

    return callCloseOnReturn.m_status = bstatus;
    // Call bridge's _CloseSource and _OnConvertedToBim
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/20
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::CreateSnapshot(iModelBridgeError& error)
    {
    // The caller has already done the following:
    // - Initialized the CRT, the DgnPlatformLib::Host, sqlang, and logging.
    // - Initialized the progress meter
    // - Initialized crash-reporting and signal-handler
    // - Set up the bridge Parameters
    // - Loaded the bridge (m_bridge)

    iModelBridgeSettings settings(nullptr, m_jobEnvArgs.m_jobRunCorrelationId, "", "");
    FwkContext context(settings, error, nullptr);

    // Get the name of the output .bim file from the snapshot argument, not the --server-repository argument
    BeAssert(!m_jobEnvArgs.m_snapshotFileName.empty());
    if (m_jobEnvArgs.m_snapshotFileName.IsAbsolutePath())
        {
        m_briefcaseBasename.Assign(m_jobEnvArgs.m_snapshotFileName.GetBaseName().c_str());
        m_jobEnvArgs.m_stagingDir = m_jobEnvArgs.m_snapshotFileName.GetDirectoryName();
        }
    else
        {
        m_briefcaseBasename.Assign(m_jobEnvArgs.m_snapshotFileName.c_str());
        }

    Briefcase_MakeBriefcaseName();

    // TODO - do we need this for a snapshot?
    // if (m_bridge->TestFeatureFlag("imodel-bridge-terrain-conversion"))
    //     m_bridge->_GetParams().SetDoTerrainModelConversion(true);

    // Now initialize the bridge.
    if (BSISUCCESS != InitBridge())
        return BentleyStatus::ERROR;

    iModelBridgeCallTerminate callTerminate(*m_bridge);
    callTerminate.m_status = BSISUCCESS;

    ReportFeatureFlags();

    BentleyStatus status;
    if (!m_jobEnvArgs.m_allDocsProcessed)
        {
        if (!m_briefcaseName.DoesPathExist())
            {
            status = CreateNewSnapshot(error, m_briefcaseBasename.c_str());
            }
        else
            {
            status = CreateSnapshotSecondBridge(error);
            }

        if (BSISUCCESS == status)
            {
            EnterSharedChannel();

            m_bridge->DoFinalizationChanges(*m_briefcaseDgnDb);

            if (!m_jobEnvArgs.m_argsJson.isMember("skipExtents"))
                {
                UpdateProjectExtents(context);
                }
            //SetUpECEFLocation(context);
            }
        }
    else
        {
        DbResult rc;
        m_briefcaseDgnDb = DgnDb::OpenDgnDb(&rc, m_briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        if (!m_briefcaseDgnDb.IsValid())
            {
            callTerminate.m_status = BSIERROR;
            }
        else
            {
            OnAllDocsProcessed(context);

            // Do this last, to avoid creating Txns in DoCreateDgnDb
            m_briefcaseDgnDb->ResetBriefcaseId(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
            m_briefcaseDgnDb->ExecuteSql("ANALYZE");

            // TODO: Call vacuum?
            }
        }

    BentleyApi::Http::HttpClient::Uninitialize();

    if (callTerminate.m_status == BSISUCCESS)
        {
        callTerminate.m_status = (BentleyStatus)m_briefcaseDgnDb->SaveChanges();
        }
    else
        {
        m_briefcaseDgnDb->AbandonChanges();
        }

    m_briefcaseDgnDb = nullptr;

    return callTerminate.m_status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/20
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::CreateSnapshotWithExceptionHandling(iModelBridgeError& error)
{
#if defined (BENTLEYCONFIG_OS_WINDOWS)
    __try
        {
#endif
        return CreateSnapshot(error);
#if defined (BENTLEYCONFIG_OS_WINDOWS)
        }
    __except (windows_filterException(GetExceptionInformation()))
        {
        fprintf(stderr, "Unhandled exception in CreateSnapshot.\n");
        }

    return RETURN_STATUS_UNHANDLED_EXCEPTION;
#endif
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
    ReadEntireFile(wstr, m_stderrFileName);
    if (!wstr.empty())
        {
        GetLogger().error("BEGIN: errors from stderr");
        GetLogger().errorv(L"%ls", wstr.c_str());
        GetLogger().errorv("END: errors from stderr");
        }

    if (!m_briefcaseName.empty())
        {
        // Write contents of the issues file to the log
        wstr.clear();
        BeFileName issuesFileName = ComputeReportFileName(m_briefcaseName);
        ReadEntireFile(wstr, issuesFileName);
        if (!wstr.empty())
            {
            GetLogger().errorv("BEGIN: errors from issues file.");
            GetLogger().errorv(L"%ls", wstr.c_str());
            GetLogger().errorv("END: errors from issues file.");
            }
        }

//    BeFileName::BeDeleteFile(m_stdoutFileName.c_str());
    BeFileName::BeDeleteFile(m_stderrFileName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeFwk::iModelBridgeFwk()
:m_logProvider(nullptr), m_dmsSupport(nullptr), m_bridge(nullptr)
    {
    m_client = nullptr;
    m_bcMgrForBridges = new iModelBridgeFwkPush(*this);
    m_lastBridgePushStatus = iModelBridge::IBriefcaseManager::PushStatus::Success;

    m_useIModelHub = false;
    m_iModelHubArgs = nullptr;
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
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::ReportFeatureFlags()
    {
    if (m_bridge != nullptr)
        {
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::GetMutexName(wchar_t* buf, size_t bufLen)
    {
    WString mname(m_jobEnvArgs.m_stagingDir);
    mname.ReplaceAll(L"\\", L"_");
    wcsncpy(buf, mname.c_str(), bufLen-1);
    if (mname.length() > (bufLen-1))
        buf[bufLen-1] = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeFwk::Run(int argc, WCharCP argv[])
    {
#ifdef _WIN32
    wchar_t mutexName[256];
    GetMutexName(mutexName, sizeof(mutexName)/sizeof(mutexName[0]));
    auto mutex = ::CreateMutexW(nullptr, false, mutexName);
    if (nullptr == mutex)
        {
        fprintf(stderr, "%ls - cannot create mutex", mutexName);
        return -1;
        }
    HRESULT hr = ::WaitForSingleObject(mutex, s_maxWaitForMutex);
    if (WAIT_OBJECT_0 != hr)
        {
        if (WAIT_TIMEOUT == hr)
            fprintf(stderr, "%ls - Another job is taking a long time. Try again later", mutexName);
        else
            fprintf(stderr, "%ls - Error getting named mutex. Try again later", mutexName);
        return -1;
        }
#endif

    if (getenv("IMODEL_BRIDGE_FWK_PAUSE_ON_START"))
        {
        static int s_loop = 1;
        while (s_loop)
            {
            printf("Waiting...\n");
            BeThreadUtilities::BeSleep(1000);
            }
        }

    int res = RETURN_STATUS_UNHANDLED_EXCEPTION;

#if defined (BENTLEYCONFIG_OS_WINDOWS)
    __try
        {
#endif
        res = RunExclusive(argc, argv);
#if defined (BENTLEYCONFIG_OS_WINDOWS)
        }
    __except (windows_filterException(GetExceptionInformation()))
        {
        fprintf(stderr, "Unhandled exception in iModelBridgeFwk::Run.\n");
        }
#endif

#ifdef _WIN32
    ::ReleaseMutex(mutex);
    ::CloseHandle(mutex);
#endif

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
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::_QueryAllFilesAssignedToBridge(bvector<BeFileName>& fns, wchar_t const* bridgeRegSubKey)
    {
    return GetRegistry()._QueryAllFilesAssignedToBridge(fns, bridgeRegSubKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::_AssignFileToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey, BeGuidCP guid)
    {
    return GetRegistry()._AssignFileToBridge(fn, bridgeRegSubKey, guid);
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

    BeFileName registryDir = m_jobEnvArgs.m_registryDir;
    if (registryDir.empty())
        registryDir = m_jobEnvArgs.m_stagingDir;

    BeSQLite::DbResult res;
    m_registry = iModelBridgeRegistry::OpenForFwk(res, registryDir, m_briefcaseBasename);

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
* @bsimethod                                    Abeesh.Basheer                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::BeBriefcaseId iModelBridgeFwk::GetBriefcaseId()
    {
    if (!m_useIModelHub)
        return BeSQLite::BeBriefcaseId();

    if (m_iModelHubArgs->m_briefcaseId.IsValid())
        return m_iModelHubArgs->m_briefcaseId;

    uint32_t bcid;
    if (BE_SQLITE_ROW != m_stateDb.QueryProperty(&bcid, sizeof(bcid), s_briefcaseIdPropSpec))
        return BeSQLite::BeBriefcaseId();

    return BeSQLite::BeBriefcaseId(bcid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Wouter.Rombouts                 03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::TestFeatureFlag(CharCP ff, bool& flag) const
    {
    if (m_bridge != nullptr)
        {
        flag = m_bridge->TestFeatureFlag(ff);
        LOG.debugv("iModelBridgeFwk::TestFeatureFlag: bridge returned %d for feature '%s'.", flag ? 1 : 0, ff);
        return BSISUCCESS;
        }

    if (m_useIModelHub && m_iModelHubArgs)
        {
        if (SUCCESS != iModelBridgeLdClient::GetInstance(m_iModelHubArgs->m_environment).IsFeatureOn(flag, ff))
            {
            LOG.errorv("Testing feature flag %s failed for environment %d", ff, m_iModelHubArgs->m_environment);
            return ERROR;
            }
        LOG.debugv("iModelBridgeFwk::TestFeatureFlag: bridge returned %d for feature '%s'.", flag ? 1 : 0, ff);
        return BSISUCCESS;
        }
    LOG.error("iModelBridgeFwk::TestFeatureFlag: unable to determine how to test feature flag.");
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Majerle                 07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::GetFeatureValue(Utf8StringR value, CharCP featureName) const
    {
    if (m_bridge != nullptr)
        {
        value = m_bridge->GetFeatureValue(featureName);
        return BSISUCCESS;
        }

    if (m_useIModelHub && m_iModelHubArgs)
        {
        if (SUCCESS != iModelBridgeLdClient::GetInstance(m_iModelHubArgs->m_environment).GetFeatureValue(value, featureName))
            {
            LOG.errorv("get feature value %s failed for environment %d", featureName, m_iModelHubArgs->m_environment);
            return ERROR;
            }
        return BSISUCCESS;
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool            iModelBridgeFwk::EnableECProfileUpgrade() const
    {
    bool allowProfileUpgrade = false;
    TestFeatureFlag("allow-ec-schema-3-2", allowProfileUpgrade);
    return allowProfileUpgrade;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::WriteErrorDocument()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SetLastError(iModelBridgeError const& err)
    {
    if (m_lastError != nullptr)
        *m_lastError = err;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SetLastError(IBriefcaseManager::Response const& response, Utf8CP description, iModelBridgeErrorId* idToUse)
    {
    if (response.Result() == RepositoryStatus::Success)
        return;

    iModelBridgeError err;
    err.m_id = idToUse? *idToUse : static_cast<iModelBridgeErrorId>(response.Result());
    err.m_description = description;
    err.m_message = IRepositoryManager::RepositoryStatusToString(response.Result());
    response.ToJson(err.m_extendedData);

    SetLastError(err);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void getOutlierElementInfo(const bvector<BeInt64Id>& elementOutliers, DgnDbCP db, Utf8String& message)
    {
    EC::ECSqlStatement stmt;
    stmt.Prepare(*db, "SELECT e2.Identifier, json_extract(e1.JsonProperties, '$.v8ModelName') V8ModelName, \
        json_extract(e3.JsonProperties, '$.fileName') FileName FROM bis.ExternalSourceAspect e1 INNER JOIN bis.ExternalSourceAspect e2 ON e1.Element.Id = e2.Scope.Id \
        INNER JOIN bis.ExternalSourceAspect e3 ON e3.Element.Id = e1.Scope.Id WHERE e2.Element.Id =?");
    BeAssert(stmt.IsPrepared());

    for (auto eid : elementOutliers)
        {
        auto outlierElement = db->Elements().GetElement((DgnElementId)eid.GetValue());
        if (!outlierElement.IsValid())
            continue;

        Utf8String v8ElementId;
        Utf8String sourceModelName = "<unknown>";
        Utf8String sourceFileName = "<unknown>";

        stmt.Reset();
        stmt.ClearBindings();
        stmt.BindId(1, eid);
        if (BeSQLite::DbResult::BE_SQLITE_ROW != stmt.Step())
            continue;

        v8ElementId = stmt.GetValueText(0);
        sourceModelName = stmt.GetValueText(1);
        sourceFileName = stmt.GetValueText(2);

        Utf8PrintfString elementDetails("\nElement Name:%s, Id: %s, Model Name: %s, File Name: %s", outlierElement->GetDisplayLabel().c_str(), v8ElementId.c_str(), sourceModelName.c_str(), sourceFileName.c_str());
        message.append(elementDetails.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static  BentleyStatus        GetLatLongFromUserExtents(GeoPoint& low, GeoPoint& high, iModelBridgeFwk::FwkContext& context)
    {
    auto iModelInfo = context.m_iModelInfo;
    if (!iModelInfo.IsValid())
        return ERROR;

    auto extentValues = iModelInfo->GetExtent();
    if (extentValues.size() < 4)
        return ERROR;

    low.Init(extentValues[1], extentValues[0], 0);
    high.Init(extentValues[3], extentValues[2],0);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::GetUserProvidedExtents(AxisAlignedBox3d& extents, iModelBridgeFwk::FwkContext& context)
    {
    GeoPoint low, high;
    if (SUCCESS != GetLatLongFromUserExtents(low, high, context))
        return ERROR;

    LOG.tracev(L"iModel extent values from hub %f, %f x %f,  %f", low.latitude, low.longitude, high.latitude, high.longitude);


    DgnGCS* gcs = m_briefcaseDgnDb->GeoLocation().GetDgnGCS();
    if (NULL != gcs)//Use the GCS from the db if available.
        {
        m_briefcaseDgnDb->GeoLocation().XyzFromLatLongWGS84(extents.low, low);
        m_briefcaseDgnDb->GeoLocation().XyzFromLatLongWGS84(extents.high, high);
        return SUCCESS;
        }

    WString warningMsg;
    StatusInt warning;
    auto wsg84 = GeoCoordinates::BaseGCS::CreateGCS();        // WGS84 - used to convert Long/Latitude to ECEF.
    wsg84->InitFromEPSGCode(&warning, &warningMsg, 4326); // We do not care about warnings. This GCS exists in the dictionary

    DPoint3d points[2];
    wsg84->XYZFromLatLong(points[0], low);
    wsg84->XYZFromLatLong(points[1], high);

    extents.InitFrom(points[0], points[1]);
    DPoint3d delta;
    delta.Init(points[1].x - points[0].x, points[1].y - points[0].y, points[1].z - points[1].z);
    LOG.tracev(L"iModel extent rectangle from hub %f x %f", delta.x, delta.y);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetElementOutLiers(bvector<BeInt64Id>& elementOutliers, DgnDbR db, AxisAlignedBox3d const& range)
    {
    auto stmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId,Origin,Yaw,Pitch,Roll,BBoxLow,BBoxHigh FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d));

    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (stmt->IsValueNull(1)) // has no placement
            continue;

        double yaw = stmt->GetValueDouble(2);
        double pitch = stmt->GetValueDouble(3);
        double roll = stmt->GetValueDouble(4);

        DPoint3d low = stmt->GetValuePoint3d(5);
        DPoint3d high = stmt->GetValuePoint3d(6);

        Placement3d placement(stmt->GetValuePoint3d(1),
            YawPitchRollAngles(Angle::FromDegrees(yaw), Angle::FromDegrees(pitch), Angle::FromDegrees(roll)),
            ElementAlignedBox3d(low.x, low.y, low.z, high.x, high.y, high.z));

        AxisAlignedBox3d elementRange = placement.CalculateRange();
        if (!elementRange.IsContained(range))
            elementOutliers.push_back(stmt->GetValueId<DgnElementId>(0));
        }

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
int             iModelBridgeFwk::UpdateProjectExtents(iModelBridgeFwk::FwkContext& context)
    {
    if (ProjectExtentsSource::Calculated != m_briefcaseDgnDb->GeoLocation().GetProjectExtentsSource())
        return SUCCESS; // Don't allow bridges to adjust project extents established as correct by the user...

    //For Bridges the rule is on every update
    // 1. We calculate the project extents based on input data
    // 2. We set the x-y to the value provided by iModelHub. (Need to check whether computed value is better and clamp it down by imodelhub values in real world cases)
    // 3. We set the z value to be actual z as computed without outliers with a maximum clamp down value of Mt.Everest
    // 4. Do not increase project  extents if there is no change so that we do not have empty changesets.
    //We need a push-pull these changes and also do it under a schema lock
    AxisAlignedBox3d extents = m_briefcaseDgnDb->GeoLocation().GetProjectExtents();

    size_t      outlierCount;
    DRange3d    rangeWithOutliers;

    bvector<BeInt64Id> elementOutliers;
    AxisAlignedBox3d calculated = m_briefcaseDgnDb->GeoLocation().ComputeProjectExtents(&rangeWithOutliers, &elementOutliers);

    // Yes, this is evil, but until users are allowed to manually specify a project extent, some bridges and scenarios need to defeat the outlier computation above.
    AxisAlignedBox3d originalCalculatedRange = calculated;
    m_bridge->_AdjustProjectExtents(calculated, rangeWithOutliers, *m_briefcaseDgnDb);
    bool didBridgeAdjustRange = !calculated.IsEqual(originalCalculatedRange); // used later to suppress reporting

    AxisAlignedBox3d userProvided = calculated;
    bool useiModelHubExtents = false;
    TestFeatureFlag("allow-imodelhub-projectextents", useiModelHubExtents);

    bool modifiedCalculatedRange = false;
    if (useiModelHubExtents && SUCCESS == GetUserProvidedExtents(userProvided, context))
        {
        static const double MaxHeight = 8848.0392; //Clamp down z by Mt. everest height
        userProvided.low.z = MIN (calculated.low.z, MaxHeight);
        userProvided.high.z = MIN(calculated.high.z, MaxHeight);
        if (NULL == m_briefcaseDgnDb->GeoLocation().GetDgnGCS())
            {
            //If the iModel does not have a GCS assume the center of calculated range matches the center of user provided extents
			//Draw a picture.
            DPoint3d calcCenter = calculated.GetCenter();
            DPoint3d userProvidedCenter = userProvided.GetCenter();
            DPoint3d diff;
            diff.DifferenceOf(calcCenter, userProvidedCenter);
            userProvided.low.Add(diff);
            userProvided.high.Add(diff);
            }

        //Now use the user provided extents to clamp down the calculated extents
        AxisAlignedBox3d clampedDownRange;
        clampedDownRange.IntersectionOf(userProvided, calculated);
        if (clampedDownRange.IsNull())
            clampedDownRange = userProvided;

        if (!calculated.IsContained(clampedDownRange))
            modifiedCalculatedRange = true;
        calculated = clampedDownRange;
        }

    if (!extents.IsEqual(calculated))
        {
        m_briefcaseDgnDb->GeoLocation().SetProjectExtents(calculated);
        if (modifiedCalculatedRange) //Recompute element outliers since they might have changed as a result of the clamp down.
            {
            elementOutliers.clear();
            GetElementOutLiers(elementOutliers, *m_briefcaseDgnDb, calculated);
            }

        if (!didBridgeAdjustRange && elementOutliers.size() > 0)
            {
            Utf8String elementDetails;
            getOutlierElementInfo(elementOutliers, m_briefcaseDgnDb.get(), elementDetails);

            Utf8PrintfString message("Elements out of range : %s", elementDetails.c_str());
            ReportIssue(message);
            }
        }

    m_briefcaseDgnDb->GeoLocation().Save();//Save the geo location value incase a bridge or the fwk has modified the in memory values.

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::SetUpECEFLocation(FwkContext& context)
    {
    DgnGCS* gcs = m_briefcaseDgnDb->GeoLocation().GetDgnGCS();
    if (NULL != gcs)//Do nothing if the iModel is already set up correctly by the bridge.
        return SUCCESS;

    //!Use the center of the user provided extents as the origin.
    EcefLocation currentEcefLocation = m_briefcaseDgnDb->GeoLocation().GetEcefLocation();
    if (currentEcefLocation.m_isValid)
        return SUCCESS;//TODO: Store a syncinfo and update the map location if it was created by this function.

    GeoPoint low, high;
    if (SUCCESS != GetLatLongFromUserExtents(low, high, context))
        return ERROR;

    GeoPoint originLatLong;
    originLatLong.latitude = 0.5*(low.latitude + high.latitude);
    originLatLong.longitude = 0.5*(low.longitude + high.longitude);
    originLatLong.elevation = 0;

    WString warningMsg;
    StatusInt warning;
    auto wsg84 = GeoCoordinates::BaseGCS::CreateGCS();        // WGS84 - used to convert Long/Latitude to ECEF.
    wsg84->InitFromEPSGCode(&warning, &warningMsg, 4326); // We do not care about warnings. This GCS exists in the dictionary

    DPoint3d origin, originEcef;
    wsg84->CartesianFromLatLong(origin, originLatLong);
    wsg84->XYZFromLatLong(originEcef, originLatLong);

    DPoint3d yLocation = DPoint3d::FromSumOf(origin, DPoint3d::From(0.0, 10.0, 0.0));
    GeoPoint yGeoLocation;
    wsg84->LatLongFromCartesian(yGeoLocation, yLocation);
    DPoint3d ecefY;
    wsg84->XYZFromLatLong(ecefY, yGeoLocation);

    RotMatrix rMatrix = RotMatrix::FromIdentity();
    DVec3d zVector, yVector;
    zVector.Normalize((DVec3dCR)originEcef);
    yVector.NormalizedDifference(ecefY, originEcef);

    rMatrix.SetColumn(yVector, 1);
    rMatrix.SetColumn(zVector, 2);
    rMatrix.SquareAndNormalizeColumns(rMatrix, 1, 2);

    auto ecefTrans = Transform::From(rMatrix, originEcef);
    ecefTrans.TranslateInLocalCoordinates(ecefTrans, -origin.x, -origin.y, -origin.z);

    EcefLocation ecefLocation;
    ecefLocation.m_origin = ecefTrans.Origin();
    YawPitchRollAngles::TryFromRotMatrix(ecefLocation.m_angles, rMatrix);
    ecefLocation.m_isValid = true;

    if (currentEcefLocation.m_isValid && currentEcefLocation.m_origin.AlmostEqualXY(ecefLocation.m_origin))
        return SUCCESS;

    m_briefcaseDgnDb->GeoLocation().SetEcefLocation(ecefLocation);
    m_briefcaseDgnDb->GeoLocation().Save();
    return SUCCESS;
    }
POP_DISABLE_DEPRECATION_WARNINGS
