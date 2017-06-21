/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/iModelBridgeFwk.cpp $
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
#include <Logging/bentleylogging.h>
#include <iModelBridge/iModelBridgeBimHost.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LOGGING

#define RETURN_STATUS_SUCCESS           0
#define RETURN_STATUS_USAGE_ERROR       1
#define RETURN_STATUS_CONVERTER_ERROR   2
#define RETURN_STATUS_SERVER_ERROR      3
#define RETURN_STATUS_LOCAL_ERROR       4

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridgeFwk"))

BEGIN_BENTLEY_DGN_NAMESPACE

static int s_maxWaitForMutex = 60000;

#if defined(__unix__)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
T_iModelBridge_getInstance iModelBridgeFwk::JobDefArgs::LoadBridge()
    {
    BeAssert(false && "TBD");
    return nullptr;
    }

#else

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
static void setDllSearchPathFor(BentleyApi::BeFileNameCR bridgeDllName)
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
T_iModelBridge_getInstance iModelBridgeFwk::JobDefArgs::LoadBridge()
    {
    setDllSearchPathFor(m_bridgeLibraryName);

    auto hinst = ::LoadLibraryW(m_bridgeLibraryName.c_str());
    if (!hinst)
        {
        fwprintf(stderr, L"%ls: not found or could not be loaded\n", m_bridgeLibraryName.c_str());
        return nullptr;
        }

    auto getInstance = (T_iModelBridge_getInstance) GetProcAddress (hinst, "iModelBridge_getInstance");
    if (!getInstance)
        {
        fwprintf(stderr, L"%ls: Does not export a function called 'iModelBridge_getInstance'\n", m_bridgeLibraryName.c_str());
        return nullptr;
        }

    return getInstance;
    }

#endif

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
        L"--fwk-bridge-library=       (required)  The full path to the bridge library.\n"
        L"--fwk-staging-dir=          (required)  The staging directory.\n"
        L"--fwk-input=                (required)  Input file name\n"
        L"--fwk-logging-config-file=  (optional)  The name of the logging configuration file.\n"
        L"--fwk-max-wait=milliseconds (optional)  The maximum amount of time to wait for other instances of this job to finish.\n"
        L"--fwk-revision-comment=     (optional)  The revision comment.\n"
        L"--fwk-input-gcs=gcsspec     (optional)  Specifies the GCS of the input DGN root model. Ignored if DGN root model already has a GCS.\n"
        L"--fwk-geoCalculation=       (optional)  If a new model is added with the --update option, sets the geographic coordinate calculation method. Possible Values are:\n"
        L"                                           Default   - Base the calculation method on the source GCS. GCS's calculated from Placemarks are transformed, others are reprojected\n"
        L"                                           Reproject - Do full geographic reprojection of each point\n"
        L"                                           Transform - use the source GCS and DgnDb GCS to construct and use a linear transform\n"
        L"                                           TransformScaled - Similar to Transform, except the transform is scaled to account for the GCS grid to ground scale (also known as K factor).\n"
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
    NativeLogging::LoggingConfig::SetSeverity(L"Performance", NativeLogging::LOG_TRACE);
    //NativeLogging::LoggingConfig::SetSeverity(L"DgnCore", NativeLogging::LOG_TRACE);
    //NativeLogging::LoggingConfig::SetSeverity(L"BeSQLite", NativeLogging::LOG_TRACE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridgeFwk::JobDefArgs::ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[])
    {
    auto fwkAssetsDirRaw = iModelBridgeSacAdapter::GetExecutablePath(argv[0]).GetDirectoryName();       // this will be:    blah/iModelBridgeFwk/lib/x64
    fwkAssetsDirRaw.AppendToPath(L"..");
    fwkAssetsDirRaw.AppendToPath(L"..");
    fwkAssetsDirRaw.AppendToPath(L"assets");                                                            // we want:         blah/iModelBridgeFwk/assets

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

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-bridge-library"))
            {
            m_bridgeLibraryName.SetName(getArgValueW(argv[iArg]));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-create-repository-if-necessary")) // undocumented
            {
            m_createRepositoryIfNecessary = true;
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-staging-dir="))
            {
            m_stagingDir.SetName(getArgValueW(argv[iArg]));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-input="))
            {
            BeFileName::FixPathName (m_inputFileName, getArgValueW(argv[iArg]).c_str());
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-max-wait="))
            {
            wscanf(getArgValueW(argv[iArg]).c_str(), L"%d", &s_maxWaitForMutex);
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-logging-config-file="))
            {
            m_loggingConfigFileName.SetName(getArgValueW(argv[iArg]));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-revision-comment="))
            {
            m_revisionComment = getArgValue(argv[iArg]);
            continue;
            }

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

        if (argv[iArg] == wcsstr(argv[iArg], L"--fwk-assetsDir="))
            {
            BeFileName assetsDir(getArgValueW(argv[iArg]));
            if (assetsDir.DoesPathExist())
                fwkAssetsDirRaw = assetsDir;
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
    if (m_bridgeLibraryName.empty())
        {
        fwprintf(stderr, L"The bridge library name is required.\n");
        return BSIERROR;
        }

    if (!m_bridgeLibraryName.DoesPathExist())
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
        m_loggingConfigFileName = iModelBridgeSacAdapter::GetExecutablePath(argv[0]).GetDirectoryName();
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
BeFileName iModelBridgeFwk::GetSyncStateFileName()
    {
    BeFileName fn(m_briefcaseName);
    fn.append(L".dgnv8mirror_SyncState");
    return fn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SetSyncState(SyncState st)
    {
    BeFile bf;
    if (BeFileStatus::Success != bf.Create(GetSyncStateFileName(), true))
        {
        BeAssert(false);
        return;
        }
    bf.Write(nullptr, &st, sizeof(st));
    bf.Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeFwk::SyncState iModelBridgeFwk::GetSyncState()
    {
    BeFile bf;
    if (BeFileStatus::Success != bf.Open(GetSyncStateFileName(), BeFileAccess::Read))
        return SyncState::Initial;

    uint32_t nread;
    SyncState st;
    if (BeFileStatus::Success != bf.Read(&st, &nread, sizeof(st)) || nread != sizeof(st))
        st = SyncState::Initial;

    return st;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName iModelBridgeFwk::GetModelsFileName()
    {
    BeFileName fn(m_briefcaseName);
    fn.append(L".dgnv8mirror_models");
    return fn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SaveNewModelIds()
    {
    if (m_modelsInserted.empty())
        return;

    FILE* fp = fopen(GetModelsFileName().GetNameUtf8().c_str(), "w+");
    if (nullptr == fp)
        {
        BeAssert(false);
        return;
        }
    for (auto mid : m_modelsInserted)
        {
        fprintf(fp, "%" PRIu64 "\n", mid.GetValue());
        }
    fclose(fp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::ReadNewModelIds()
    {
    BeAssert(m_modelsInserted.empty());

    FILE* fp = fopen(GetModelsFileName().GetNameUtf8().c_str(), "r");
    if (nullptr == fp)
        return;

    uint64_t mid;

    while (1 == fscanf(fp, "%" SCNu64 "\n", &mid))
        {
        m_modelsInserted.push_back(DgnModelId(mid));
        }
    fclose(fp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::DeleteNewModelIdsFile()
    {
    unlink(GetModelsFileName().GetNameUtf8().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName iModelBridgeFwk::GetStateFileName()
    {
    BeFileName fn(m_briefcaseName);
    fn.append(L".dgnv8mirror_state");
    return fn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SetState(BootstrappingState st)
    {
    BeFile bf;
    if (BeFileStatus::Success != bf.Create(GetStateFileName(), true))
        {
        BeAssert(false);
        return;
        }
    bf.Write(nullptr, &st, sizeof(st));
    bf.Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeFwk::BootstrappingState iModelBridgeFwk::GetState()
    {
    BeFile bf;
    if (BeFileStatus::Success != bf.Open(GetStateFileName(), BeFileAccess::Read))
        return BootstrappingState::Initial;

    uint32_t nread;
    BootstrappingState st;
    if (BeFileStatus::Success != bf.Read(&st, &nread, sizeof(st)) || nread != sizeof(st))
        st = BootstrappingState::Initial;

    return st;
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
        fwprintf(stderr, L"%ls - is a directory name. I need to create a file by this name.\n", m_briefcaseName.c_str());
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
    m_briefcaseDgnDb = m_bridge->DoCreateDgnDb(m_modelsInserted);
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

    DeleteNewModelIdsFile();

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
void iModelBridgeFwk::SetBridgeParams(iModelBridge::Params& params)
    {
    Briefcase_MakeBriefcaseName(); // => defines m_briefcaseName
    params.m_briefcaseName = m_briefcaseName;
    params.SetReportFileName();
    params.GetReportFileName().BeDeleteFile();
    params.m_assetsDir = m_jobEnvArgs.m_bridgeLibraryName.GetDirectoryName();
    params.m_assetsDir.AppendToPath(L"Assets");
    params.m_libraryDir = m_jobEnvArgs.m_bridgeLibraryName.GetDirectoryName();
    params.m_repoAdmin = &m_repoAdmin;
    params.m_inputFileName = m_jobEnvArgs.m_inputFileName;
    params.m_gcsCalculationMethod = m_jobEnvArgs.m_gcsCalculationMethod;
    params.m_inputGcs = m_jobEnvArgs.m_inputGcs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::LoadBridge()
    {
    auto getInstance = m_jobEnvArgs.LoadBridge();
    if (nullptr == getInstance)
        return BentleyStatus::ERROR;

    m_bridge = getInstance();
    if (nullptr == m_bridge)
        {
        fwprintf(stderr, L"%ls: iModelBridge_getInstance function returned a nullptr\n", m_jobEnvArgs.m_bridgeLibraryName.c_str());
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
int iModelBridgeFwk::Run0(int argc, WCharCP argv[])
    {
    iModelBridgeSacAdapter::InitCrt(false);

    // Put out this info message, so that we can relate all subsequent logging messages to this bridge.
    // The log on this machine may have messages from many bridge jobs.
    LOG.infov("Running bridge [%s] staging dir [%s]", m_jobEnvArgs.m_bridgeLibraryName.c_str(), Utf8String(m_jobEnvArgs.m_stagingDir).c_str());

    // First of all, load the bridge.
    // *** TRICKY: Do not call InitBridge until AFTER we acquire the briefcase (or decide to create a new DgnDb).
    if (BentleyStatus::SUCCESS != LoadBridge())
        return RETURN_STATUS_CONVERTER_ERROR;

    // Initialize the DgnViewLib Host.
    iModelBridge::Params params;
    SetBridgeParams(params);
    iModelBridgeKnownLocationsAdmin::SetAssetsDir(params.GetAssetsDir()); // Note that we set up the host with the assets directory for the *the bridge*
    Dgn::iModelBridgeBimHost host(params.GetRepositoryAdmin(), m_bridge->_SupplySqlangRelPath());
    DgnViewLib::Initialize(host, true);

    static PrintfProgressMeter s_meter;
    T_HOST.SetProgressMeter(&s_meter);

    // ***
	// ***
	// *** DO NOT CHANGE THE ORDER OF THE STEPS BELOW
	// *** Talk to Sam Wilson if you need to make a change.
	// ***
	// ***

    //  Sign into the iModelHub
    if (BSISUCCESS != Briefcase_Initialize(argc, argv))
        return RETURN_STATUS_SERVER_ERROR;

    //  Make sure we have a briefcase
    Briefcase_MakeBriefcaseName(); // => defines m_briefcaseName
    bool createdNewRepo = false;
    if (BSISUCCESS != BootstrapBriefcase(createdNewRepo))
        {
        m_briefcaseDgnDb = nullptr;
        return RETURN_STATUS_SERVER_ERROR;
        }

    if (createdNewRepo) // This is an optimization of a special case - we just created a new repo from scratch. There's no need to go through the update logic and pullmergepush
        return RETURN_STATUS_SUCCESS;

    // ***
    // *** TRICK: Do not call InitBridge until we have done PullMergePush
    // ***          That is because the bridge may register a new or upgraded schema, which will require DgnDb::OpenDgnDb to do an import or upgrade.
    // ***          At that time, we must ensure that the BIM does not contain any lingering local Txns and that it is as the tip (so that we can get the schema lock).
    // ***          So, we need to be able to open the BIM just in order to pull/merge/push, before we allow the bridge to add a schema import/upgrade into the mix.
    // ***
    //  By getting the BIM to the tip, this initial pull also helps ensure that we will be able to get locks for the other changes that that bridge will make later.
    BeSQLite::DbResult dbres;
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

    BeAssert(!m_briefcaseName.empty() && m_briefcaseName.DoesPathExist() && !m_briefcaseName.IsDirectory() && (SyncState::Initial == GetSyncState()));
    m_briefcaseDgnDb = nullptr;

    s_meter.Hide();

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
    freopen (m_stderrFileName.GetNameUtf8().c_str(), "w+", stderr);
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
        GetLogger().error(wstr.c_str());

    wstr.clear();
    BeFileName issuesFileName(m_briefcaseName);
    issuesFileName.append(L"-issues");
    readEntireFile(wstr, issuesFileName);
    if (!wstr.empty())
        GetLogger().error(wstr.c_str());

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
        res = Run0(argc, argv);
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
