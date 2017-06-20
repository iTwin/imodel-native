/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Apps/DwgImportApp.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportApp.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DGNDBSYNC_DWG

static bool             s_quietAssertions;
static const wchar_t    s_spinner[] = L" /-\\|";
static const size_t     s_spinnerSize = _countof(s_spinner)-1;
static bool             s_alwaysShowTopLevelTasks = false; // WARNING: it's usually too slow to force display all top-level tasks
static WCharCP          s_configFileName = L"DwgImporter.logging.config.xml";

#define LOG             (*NativeLogging::LoggingManager::GetLogger(L"DwgImporter"))


BEGIN_DGNDBSYNC_DWG_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DwgImportApp::GetCommonCommandLineOptions ()
    {
    return L"\
    --description=          (optional; publishing only) A string saved as the 'description' property in the DgnDb.\n\
    --unstableIds           (optional; publishing only) A flag in syncinfo that indicates that subsequent updates should assume that ElementIds are not a reliable way to cross-check elements in two versions of a file. The same piece of graphics could have been assigned a new ElementId in the new version of the file.\n\
    --configuration=        (optional; publishing only) Path to the publisher configuration file (defaults to \"ConvertConfig.xml\" next to the EXE)\n\
    --no-assert-dialogs     Prevents modal assert dialogs\n\
    --compress              Additionally compresses the output into an .imodel\n\
    --password=             Password needed for the converter to open protected input files.\n\
    --update[=description]  (optional; updating only) This option is applied to cause the converter to update the output file, rather than re-create it. An optional description of the changes may be supplied.\n\
    ";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImportApp::ParseStatus       DwgImportApp::ParseCommonCommandLineArgument (WCharP argv[], int& iArg)
    {
    if (0 == wcscmp(argv[iArg], L"--no-assert-dialogs"))
        {
        m_preventAssertDialogs = true;
        return ParseStatus::Success;
        }

    if (0 == wcscmp(argv[iArg], L"--unstableIds"))
        {
        m_options.SetStableIdPolicy(StableIdPolicy::ByHash);
        return ParseStatus::Success;
        }

    if (0 == wcscmp(argv[iArg], L"--compress"))
        {
        m_shouldCompress = true;
        return ParseStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--description="))
        {
        m_options.SetDescription(GetArgValue(argv[iArg]).c_str());
        return ParseStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--password="))
        {
        m_options.SetPassword(GetArgValue(argv[iArg]).c_str());
        return ParseStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--compress-chunk-size"))
        {
        WCharCP compressSizeSpec = wcschr(argv[iArg], L'=');
        if (nullptr != compressSizeSpec)
            m_compressChunkSize = BeStringUtilities::Wtoi(compressSizeSpec+1);
        return ParseStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--output=") || argv[iArg] == wcsstr(argv[iArg], L"-o="))
        {
        m_outputName.SetName(GetArgValueW(argv[iArg]));
        return ParseStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--configuration="))
        {
        WString argValue = argv[iArg];
        argValue = argValue.substr(argValue.find_first_of(L'=', 0) + 1);

        BeFileName configFile;
        BeFileName::FixPathName(configFile, argValue.c_str());
        if (BeFileName::IsDirectory(configFile.c_str()))
            return ParseStatus::Error;

        m_options.SetConfigFile(configFile);
        return ParseStatus::Success;
        }

    if (0 == wcscmp(argv[iArg], L"--update") || argv[iArg] == wcsstr(argv[iArg], L"--update="))
        {
        m_tryUpdate = true;
        if (argv[iArg] == wcsstr(argv[iArg], L"--update="))
            m_options.SetDescription(GetArgValue(argv[iArg]).c_str());

        return ParseStatus::Success;
        }

    // This is not a common command line argument => let the subclass handle it
    return ParseStatus::NotRecognized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportApp::PrintMessage (WCharCP fmt, ...) 
    {
    va_list args; 
    va_start (args, fmt); 
    WPrintfString str (fmt, args);
    va_end (args);

    wprintf(str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportApp::PrintError (WCharCP fmt, ...)
    {
    va_list args; 
    va_start (args, fmt); 
    WPrintfString str (fmt, args);
    va_end(args);

    fwprintf(stderr, str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
int             DwgImportApp::PrintUsage (WCharCP programName)
    {
    WString exeName = BeFileName::GetFileNameAndExtension(programName);

    fwprintf (stderr, L"\n\
Import a DWG file and its references to a DgnDb.\n\
\n\
Usage: %ls -i|--input= -o|--output= [OPTIONS...]\n\
\n\
    --input=                (required)  A DWG/DXF file \n\
\n\
    --output=               (required)  Output directory\n\
\n\
OPTIONS:\n\
%ls\n",
    exeName.c_str(), GetCommonCommandLineOptions().c_str());

    return 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int             DwgImportApp::ParseCommandLine (int argc, WCharP argv[])
    {
    if (argc < 2)
        return PrintUsage(argv[0]);

    for (int iArg = 1; iArg < argc; ++iArg)
        {
        if (argv[iArg] == wcsstr(argv[iArg], L"--input=") || argv[iArg] == wcsstr(argv[iArg], L"-i="))
            {
            BeFileName::FixPathName (m_inputFileName, GetArgValueW(argv[iArg]).c_str());
            if(BeFileName::IsDirectory(m_inputFileName.c_str()))
                return PrintUsage(argv[0]); 

            continue;
            }

        ParseStatus status = ParseCommonCommandLineArgument(argv, iArg);
        if (ParseStatus::Success == status)
            {
            continue;
            }
        else if (ParseStatus::Error == status)
            {
            return PrintUsage(argv[0]);
            }

        fwprintf(stderr, L"Unrecognized command line option: %ls\n", argv[iArg]);
        return PrintUsage(argv[0]);
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportApp::GetImportConfiguration (BeFileNameR instanceFilePath, BeFileNameCR configurationPath, WCharCP argv0)
    {
    WString programDir = BeFileName::GetDirectoryName (argv0);

    if (!configurationPath.empty())
        {
        instanceFilePath.SetName(configurationPath);
        }
    else
        {
        instanceFilePath.SetName(programDir.c_str());
        instanceFilePath.AppendToPath(L"ConvertConfig.xml");
        }

    WString programBasename = BeFileName::GetFileNameWithoutExtension (argv0);

    PrintMessage (L"%ls creating DgnDb using configuration file <%ls>\n", programBasename.c_str(), instanceFilePath.GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImportApp::GetEnv(BeFileName& fn, WCharCP envname)
    {
    wchar_t filepath[MAX_PATH];
    if ((0 == GetEnvironmentVariableW(envname, filepath, MAX_PATH)))
        return ERROR;

    fn.SetName(filepath);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImportApp::MustBeFile(BeFileName& fn)
    {
    if (fn.DoesPathExist() && !fn.IsDirectory())
        return BSISUCCESS;

    PrintError(L"Input file <%ls> does not exist or is not a file\n", fn.c_str());
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImportApp::GetLogConfigurationFilename(BeFileName& configFile, WCharCP argv0)
    {
    WString programBasename = BeFileName::GetFileNameWithoutExtension(argv0);

    if (SUCCESS == GetEnv(configFile, L"BENTLEY_DWGIMPORTER_LOGGING_CONFIG"))
        {
        if (configFile.DoesPathExist())
            {
            PrintMessage(L"%ls configuring logging with %s (Set by BENTLEY_DWGIMPORTER_LOGGING_CONFIG environment variable.)\n", programBasename.c_str(), configFile.GetName());
            return SUCCESS;
            }
        }

    configFile = BeFileName(BeFileName::DevAndDir, argv0);
    configFile.AppendToPath(s_configFileName);
    configFile.BeGetFullPathName();

    if (BeFileName::DoesPathExist(configFile))
        {
        PrintMessage(L"%ls configuring logging using %ls. Override it by setting BENTLEY_DWGIMPORTER_LOGGING_CONFIG in environment.\n", programBasename.c_str(), configFile.GetName());
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportApp::InitLogging(WCharCP argv0)
    {
    BeFileName configFile;

    if (SUCCESS == GetLogConfigurationFilename(configFile, argv0))
        {
        NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, configFile);
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        }
    else
        {
        PrintMessage(L"Logging.config.xml not found. Activating default logging using console provider.\n");
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
        NativeLogging::LoggingConfig::SetSeverity(L"Performance", NativeLogging::LOG_TRACE);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
WString DwgImportApp::GetArgValueW (WCharCP arg)
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
Utf8String DwgImportApp::GetArgValue(WCharCP arg)
    {
    return Utf8String(GetArgValueW(arg));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportApp::PostProcess ()
    {
    if (m_tryUpdate)
        {
        // Update
        if (!m_wasUpdateEmpty)
            CreateIModel ();
        return;
        }
    
    //  Create
    CreateIModel ();
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct CompressProgressMeter : ICompressProgressTracker
{
private:
    uint32_t m_lastReported;
    uint32_t m_fileSize;
    BeMutex m_criticalSection;
    Utf8String  m_operation;

public:
    void Reset(uint64_t fileSize)
        {
        m_lastReported = 0;
        m_fileSize = (uint32_t)(fileSize / (1024 * 1024));
        }

    CompressProgressMeter(uint64_t fileSize, Utf8CP operation)
        {
        Reset(fileSize);
        m_operation.assign(operation);
        }

    StatusInt _Progress(uint64_t inputProcessed, int64_t outputProcessed) override
        {
        BeMutexHolder holder(m_criticalSection);

        uint32_t oneMeg = 1024 * 1024;
        uint32_t current = (uint32_t)(inputProcessed / oneMeg);
        if (current <= m_lastReported)
            return BSISUCCESS;

        m_lastReported = current;
        printf("++++ PROGRESS> %s processed %d meg of %d\r", m_operation.c_str(), current, m_fileSize);
        return BSISUCCESS;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportApp::CreateIModel()
    {
    if (!m_shouldCompress)
        return;

    BeFileName imodel(m_outputName);
    imodel.OverrideNameParts(L".imodel");

    CreateIModelParams params;
    params.SetOverwriteExisting(true);
    if (m_compressChunkSize)
        params.SetChunkSize(m_compressChunkSize);

    m_meter.SetCurrentStepName ("Creating IModel");
    auto status = DgnIModel::Create (imodel, m_outputName, params);

    if (BSISUCCESS != status)
        {
        PrintError(L"*** IModel Creation failed.****\n");
        imodel.BeDeleteFile();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImportApp::ExtractFromIModel (BeFileName& outFile, BeFileNameCR imodelFile)
    {
    uint64_t fileSize;
    imodelFile.GetFileSize(fileSize);
    CompressProgressMeter progress(fileSize, "Extract");

    DbResult dbResult;
    auto status = DgnIModel::ExtractUsingDefaults (dbResult, outFile, imodelFile, true, &progress);

    return (status != DgnDbStatus::Success) ? BSIERROR : BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImportApp::Process ()
    {
    // start the importer either by creating a new or updating existing DgnDb:
    DwgImporter*    importer = m_tryUpdate ? new DwgUpdater(m_options) : new DwgImporter(m_options);
    if (nullptr == importer)
        {
        PrintError (L"Out of memory prior to processing file %ls.\n", m_inputFileName.GetName());
        return  BSIERROR;
        }

    // try open the inpurt DWG file:
    auto status = importer->OpenDwgFile (m_inputFileName);
    if (BSISUCCESS != status)
        {
        PrintError (L"Error opening DWG file %ls.\n", m_inputFileName.GetName());
        return BSIERROR;
        }

    // create a new or update existing output DgnDb:
    status = m_tryUpdate ? importer->OpenExistingDgnDb(m_outputName) : importer->CreateNewDgnDb(m_outputName);
    if (BSISUCCESS != status)
        {
        PrintError(L"Error creating output file %ls.\n", m_outputName.GetName());
        return BSIERROR;
        }

    // ready to import DWG to DgnDb:
    status = importer->Process ();

    delete importer;

    // terminate the toolkit after DwgDbDatabase is released(i.e. above ~DwgImporter call):
    DwgImporter::TerminateDwgHost ();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportApp::OnFailedConversion()
    {
    BeFileName::BeDeleteFile (m_outputName.c_str());
    PrintError (L"\n*** %s - conversion failed.***\n", m_outputName.GetName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2012
//---------------------------------------------------------------------------------------
void            DwgImportApp::AssertHandler(WCharCP message, WCharCP file, uint32_t line, BeAssertFunctions::AssertType atype)
    {
    if (s_quietAssertions || atype == BeAssertFunctions::AssertType::Data)
        {
        WPrintfString str(L"ASSERT: (%ls) @ %ls:%u\n", message, file, line);
        LOG.warning(str.c_str());
        ::OutputDebugStringW (str.c_str());
        }
    else
        {
        BeAssertFunctions::DefaultAssertionFailureHandler(message, file, line);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImportApp::Initialize(int argc, WCharP argv[])
    {
    m_inputFileName.BeGetFullPathName();
    
    m_outputName.BeGetFullPathName();
    if (!m_outputName.DoesPathExist())
        {
        if (m_outputName.GetExtension().empty())    // it's probably supposed to be a directory name
            m_outputName.AppendSeparator();
        BeFileName outputDir = m_outputName.GetDirectoryName();
        if (!outputDir.DoesPathExist() && (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(outputDir.c_str())))
            {
            PrintError (L"Cannot create output directory <%ls>\n", outputDir.c_str());
            return BSIERROR;
            }
        }

    if (m_outputName.IsDirectory())
        {
        m_outputName.AppendToPath(m_inputFileName.GetFileNameWithoutExtension().c_str());
        m_outputName.AppendExtension(UNCOMPRESSED_DGNDB_EXT);
        }
    else
        {
        m_outputName.OverrideNameParts(L"." UNCOMPRESSED_DGNDB_EXT);
        }

    InitLogging (argv[0]);

    // FOR THE CONSOLE PUBLISHER ONLY! "Gui" publishers won't have any console output and won't need this.
    // C++ programs start-up with the "C" locale in effect by default, and the "C" locale does not support conversions of any characters outside
    // the "basic character set". ... The call to setlocale() says "I want to use the user's default narrow string encoding". This encoding is
    // based on the Posix-locale for Posix environments. In Windows, this encoding is the ACP, which is based on the system-locale.
    // However, the success of this code is dependent on two things:
    //      1) The narrow encoding must support the wide character being converted.
    //      2) The font/gui must support the rendering of that character.
    // In Windows, #2 is often solved by setting cmd.exe's font to Lucida Console."
    // (http://cboard.cprogramming.com/cplusplus-programming/145590-non-english-characters-cout-2.html)
    setlocale(LC_CTYPE, "");

    // instantiate DgnPlatform host
    DwgImporter::InitializeDgnHost (*this);

    m_options.SetProgressMeter (&m_meter);

    // We normally want modal message boxes for asserts to alert developers, but we need a way to bypass this when run during the build.
    s_quietAssertions = m_preventAssertDialogs;
    BeAssertFunctions::SetBeAssertHandler (AssertHandler); 
    if (!s_quietAssertions)
        _set_error_mode (_OUT_TO_MSGBOX);

    // Resolve import config file.
    BeFileName configFile;
    GetImportConfiguration(configFile, m_options.GetConfigFile(), argv[0]);
    m_options.SetConfigFile(configFile);

    // Set dir prefix to be dropped from filenames when coming up with unique but portable filenames in syncinfo
    m_options.SetInputRootDir(m_inputFileName.GetDirectoryName());

    BeFileName reportFileName(m_outputName);
    reportFileName.append(L"-issues");
    reportFileName.BeDeleteFile();
    m_options.SetReportFile(reportFileName);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
L10N::SqlangFiles DwgImportApp::_SupplySqlangFiles() 
    {
    BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang/DwgImporter_en-US.sqlang.db3");
    BeAssert(sqlangFile.DoesPathExist());

    return L10N::SqlangFiles(sqlangFile);
    }

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
    if (m_fileCount || m_stepsRemaining || m_tasksRemaining)
        tbd = Utf8PrintfString(":%d/%d/%d", m_fileCount, m_stepsRemaining, m_tasksRemaining);

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
    auto now = BeTimeUtilities::QuerySecondsCounter ();

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
    if (newName && m_taskName == newName)
        return;

    m_taskName = newName? newName: "";
    m_spinCount=0;
    ForceNextUpdateToDisplay();
    UpdateDisplay();
    T_Super::_SetCurrentTaskName(newName); // decrements task count
    }

/*---------------------------------------------------------------------------------**//**
* Sets the current task name, within the current step. Also indicates that the previous task was complete.
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void PrintfProgressMeter::_SetCurrentStepName(Utf8CP stepName)
    {
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
    T_Super::_SetCurrentStepName(stepName); // decrements step count
    }

END_DGNDBSYNC_DWG_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, WCharP argv[])
    {
    DwgImportApp    importApp;
    if (BSISUCCESS != importApp.ParseCommandLine(argc, argv))
        return 1;

#if defined (DEBUG_COMMAND_LINE)
    printf ("argc = %d\n", argc);
    for (int iArg=0; iArg<argc; iArg++)
        printf ("argv[%d] = %ls\n", iArg, argv[iArg]);
#endif

    if (importApp.m_outputName.empty())
        {
        fwprintf (stderr, L"No output directory specified\n");
        return importApp.PrintUsage(argv[0]);
        }

    if (BSISUCCESS != importApp.Initialize(argc, argv))
        return 2;

    BentleyStatus   status = importApp.Process ();
    if (BSISUCCESS == status)
        importApp.PostProcess ();

    return (int)status;
    }
