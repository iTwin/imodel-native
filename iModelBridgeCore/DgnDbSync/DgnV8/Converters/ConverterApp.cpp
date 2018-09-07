/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Converters/ConverterApp.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)
    #define UNICODE
    #include <Windows.h>
#endif

#include <DgnDbSync/DgnV8/ConverterApp.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeDirectoryIterator.h>
#include <DgnPlatform/DgnIModel.h>
#include <DgnPlatform/WebMercator.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger(L"DgnV8Converter"))

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
WString ConverterApp::GetCommonCommandLineOptions()
    {
    return L"\
--unstableIds                   (optional) A flag in syncinfo that indicates that subsequent updates should assume that ElementIds are not a reliable way to cross-check elements in two versions of a file. The same piece of graphics could have been assigned a new ElementId in the new version of the file.\n\
--configuration=                (optional) Path to the publisher configuration file (defaults to \"ImportConfig.xml\" next to the EXE)\n\
--password=                     (optional) Password needed for the converter to open protected input files.\n\
--name-prefix=prefix            (optional) When converting V8 files to be inserted into an existing DgnDb, you should specify the prefix to be used for the names of all content written to shared models, such as the dictionary.\n\
--embed-directory=              (optional) Directory of documents to embed\n\
--embed-files=                  (optional) List of files to be embedded (semi-colon delimited)\n\
--pwExtensionDll=               (optional) Pathname of the ProjectWise extension DLL to find linked documents\n\
--pwDataSource=                 (optional) Name of the ProjectWise data source to find linked documents\n\
--pwUser=                       (optional) Name of the ProjectWise user\n\
--pwPassword=                   (optional) Password of the ProjectWise user\n\
--pwWorkDir=                    (optional) Working directory to be used by ProjectWise\n\
";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge::CmdLineArgStatus ConverterApp::_ParseCommandLineArg(int iArg, int argc, WCharCP argv[])
    {
    if (0 == wcscmp(argv[iArg], L"--unstableIds"))
        {
        _GetConverterParams().SetStableIdPolicy(StableIdPolicy::ByHash);
        return CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--password="))
        {
        _GetConverterParams().SetPassword(GetArgValue(argv[iArg]).c_str());
        return CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--configuration="))
        {
        WString argValue = argv[iArg];
        argValue = argValue.substr(argValue.find_first_of(L'=', 0) + 1);

        BeFileName configFile;
        BeFileName::FixPathName(configFile, argValue.c_str());
        if (BeFileName::IsDirectory(configFile.c_str()))
            return CmdLineArgStatus::Error;

        _GetConverterParams().SetConfigFile(configFile);
        return CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--embed-directory="))
        {
        WString argValue = argv[iArg];
        argValue = argValue.substr(argValue.find_first_of(L'=', 0) + 1);

        BeFileName embedDir;
        BeFileName::FixPathName(embedDir, argValue.c_str());

        _GetConverterParams().SetEmbedDir(embedDir);
        return CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--embed-files="))
        {
        WString argValue = argv[iArg];
        argValue = argValue.substr(argValue.find_first_of(L'=', 0) + 1);

        bvector<WString> tokens;
        BeStringUtilities::Split(argValue.c_str(), L";", tokens);

        bvector<BeFileName> embedFiles;
        for (WStringCR token : tokens)
            {
            BeFileName fileName;
            BeFileName::FixPathName(fileName, token.c_str());
            embedFiles.push_back(fileName);
            }

        _GetConverterParams().SetEmbedFiles(embedFiles);
        return CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--pwExtensionDll="))
        {
        WString argValue = argv[iArg];
        argValue = argValue.substr(argValue.find_first_of(L'=', 0) + 1);

        BeFileName pwExtensionDll;
        BeFileName::FixPathName(pwExtensionDll, argValue.c_str());
        if (!pwExtensionDll.DoesPathExist())
            return CmdLineArgStatus::Error;

        _GetConverterParams().SetProjectWiseExtensionDll(pwExtensionDll);
        return CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--pwWorkDir="))
        {
        WString argValue = argv[iArg];
        argValue = argValue.substr(argValue.find_first_of(L'=', 0) + 1);

        BeFileName pwWorkDir;
        BeFileName::FixPathName(pwWorkDir, argValue.c_str());
        if (!BeFileName::IsDirectory(pwWorkDir.c_str()))
            return CmdLineArgStatus::Error;

        _GetConverterParams().SetProjectWiseWorkDir(pwWorkDir);
        return CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--pwUser="))
        {
        _GetConverterParams().SetProjectWiseUser(GetArgValue(argv[iArg]).c_str());
        return CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--pwPassword="))
        {
        _GetConverterParams().SetProjectWisePassword(GetArgValue(argv[iArg]).c_str());
        return CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--pwDataSource="))
        {
        _GetConverterParams().SetProjectWiseDataSource(GetArgValue(argv[iArg]).c_str());
        return CmdLineArgStatus::Success;
        }

    // This is not a common command line argument => let the subclass handle it
    return CmdLineArgStatus::NotRecognized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ConverterApp::_GetLoggingConfigurationFilename(WCharCP argv0)
    {
    static WCharCP s_configFileName = L"DgnV8Converter.logging.config.xml";

    BeFileName configFile;
    WString programBasename = BeFileName::GetFileNameWithoutExtension(argv0);

    if (SUCCESS == GetEnv(configFile, L"BENTLEY_DGNV8CONVERTER_LOGGING_CONFIG"))
        {
        if (configFile.DoesPathExist())
            {
            fwprintf(stdout, L"%ls configuring logging with %s (Set by BENTLEY_DGNV8CONVERTER_LOGGING_CONFIG environment variable.)\n", programBasename.c_str(), configFile.GetName());
            return configFile;
            }
        }

    configFile = Desktop::FileSystem::GetExecutableDir();
    configFile.AppendToPath(s_configFileName);
    configFile.BeGetFullPathName();

    if (BeFileName::DoesPathExist(configFile))
        {
        fwprintf(stdout, L"%ls configuring logging using %ls. Override by setting BENTLEY_DGNV8CONVERTER_LOGGING_CONFIG in environment.\n", programBasename.c_str(), configFile.GetName());
        return configFile;
        }

    return BeFileName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ConverterApp::GetImportConfiguration(BeFileNameR instanceFilePath, BeFileNameCR configurationPath)
    {
    BeFileName programDir(_GetParams().GetLibraryDir());

    if (!configurationPath.empty())
        {
        instanceFilePath.SetName(configurationPath);
        }
    else
        {
        instanceFilePath.SetName(programDir.c_str());
        instanceFilePath.AppendToPath(L"assets");
        instanceFilePath.AppendToPath(L"ImportConfig.xml");
        }

    fwprintf(stdout, L"Configuration file <%ls>\n", instanceFilePath.GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConverterApp::GetEnv(BeFileName& fn, WCharCP envname)
    {
    wchar_t filepath[MAX_PATH];
    if ((0 == GetEnvironmentVariableW(envname, filepath, MAX_PATH)))
        return ERROR;

    fn.SetName(filepath);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool GetV8DgnDllsRelativeDir(BentleyApi::BeFileNameR v8DllsRelativeDir, bool& isPowerplatformBased, int argc, WCharCP argv[])
    {
    WChar tmpString[8*MAXFILELENGTH];
    int argsNeeded = 0;
    for ( ; argc > 0 && argsNeeded <=2; argc--, argv++)
        {
        WCharCP  thisArg = *argv;
        if ( (*thisArg != '-') || (*(thisArg+1) != '-') )
            continue;

        thisArg = thisArg+2;

        BeStringUtilities::Wcsncpy (tmpString, _countof (tmpString), thisArg);
        BeStringUtilities::Wcsupr (tmpString);

        if (wcsncmp (tmpString, L"V8DLLSDIR=", 10) == 0)
            {
            v8DllsRelativeDir.assign (thisArg + 10);
            v8DllsRelativeDir.DropQuotes();
            v8DllsRelativeDir.AppendSeparator(v8DllsRelativeDir);
            ++argsNeeded;
            continue;
            }
        if (wcsncmp(tmpString, L"PPBASEDCONVERTER", 16) == 0)
            {
            isPowerplatformBased = true;
            ++argsNeeded;
            continue;
            }
        }

    return !v8DllsRelativeDir.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct     ResourceUpdateCaller : public DgnV8Api::IEnumerateAvailableHandlers
    {
    iModelBridge::IDocumentPropertiesAccessor& m_accessor;

    ResourceUpdateCaller(iModelBridge::IDocumentPropertiesAccessor& accessor)
        :m_accessor(accessor)
        {}

    virtual StatusInt _ProcessHandler(DgnV8Api::Handler& handler)
        {
        ConvertToDgnDbElementExtension* extension = ConvertToDgnDbElementExtension::Cast(handler);
        if (NULL == extension)
            return SUCCESS;
        extension->_UpdateResourceDefinitions(m_accessor);
        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConverterApp::_Initialize(int argc, WCharCP argv[])
    {
    BeAssert((nullptr != DgnPlatformLib::QueryHost()) && "framework must register the host");

    if (_GetParams().GetBridgeRegSubKey().empty())
        _GetParams().SetBridgeRegSubKey(RootModelConverter::GetRegistrySubKey());

    BentleyApi::BeFileName v8DllsRelativeDir;
    bool isPowerplatformBased = false;
    if (GetV8DgnDllsRelativeDir(v8DllsRelativeDir, isPowerplatformBased, argc, argv))
        {
        _GetConverterParams().SetV8SdkRelativeDir(v8DllsRelativeDir, isPowerplatformBased);
        }

    Converter::Initialize(_GetParams().GetLibraryDir(), _GetParams().GetAssetsDir(), _GetConverterParams().GetV8SdkRelativeDir(), nullptr, isPowerplatformBased, argc, argv);

    // Resolve import config file.
    BeFileName configFile;
    GetImportConfiguration(configFile, _GetConverterParams().GetConfigFile());
    _GetConverterParams().SetConfigFile(configFile);

    // Set dir prefix to be dropped from filenames when coming up with unique but portable filenames in syncinfo
    _GetConverterParams().SetInputRootDir(_GetParams().GetInputFileName().GetDirectoryName());

    iModelBridge::IDocumentPropertiesAccessor* docAccessor = _GetParams().GetDocumentPropertiesAccessor();
    if (nullptr != docAccessor)
        {
        ResourceUpdateCaller rCaller(*docAccessor);
        DgnV8Api::ElementHandlerManager::EnumerateAvailableHandlers(rCaller);
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootModelConverterApp::_Initialize(int argc, WCharCP argv[])
    {
    DetectDrawingsDirs();    // populate m_params.m_drawingAndSheetFiles
    return T_Super::_Initialize(argc, argv);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverterApp::DetectDrawingsDirs()
    {
    if (m_params.GetDrawingsDirs().empty())
        return;

    size_t offset = 0;
    Utf8String dir;
    Utf8String dirs(m_params.GetDrawingsDirs());
    while ((offset = dirs.GetNextToken (dir, ";", offset)) != Utf8String::npos)
        {
        if (!dir.empty())
            {
            bvector<BeFileName> files;
            BeDirectoryIterator::WalkDirsAndMatch(files, BeFileName(dir), L"*", true);
            for (auto const& fn : files)
                m_params.AddDrawingOrSheetFile(fn);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverterApp::_PrintUsage()
    {
    fwprintf (stderr, L"\
--root-model={<modelname>|.active|.default}    (optional)  Identifies the root model in the input file\n\
                                                            If '.active' then the root comes from the first open view in the active view group. This is the default.\n\
                                                            If '.default' then the default model is used.\n\
--2d-Models-Spatial     (optional)    If specified, then 2d models (other than Drawing and Sheet models) are considered to be spatial models\n\
--DGN-WorkSpace=        (optional)    Specifies the WorkSpace name for a MicroStation CONNECT configuration\n\
--DGN-WorkSet=          (optional)    Specifies the WorkSet name for a MicroStation CONNECT configuration\n\
--DGN-User=             (optional)    Specifies the User name for a MicroStation V8i configuration\n\
--DGN-Project=          (optional)    Specifies the User name for a MicroStation V8i configuration\n\
--DGN-Install=          (optional)    Specifies the MicroStation V8i or MicroStation CONNECT installation directory (containing ustation.exe or microstation.exe)\n\
--CfgVar=assignment     (optional)    Assigns the configuration variable in assignment statement of the form VARNAME=value\n\
--V8i                   (optional)    If included, the configuration files are from V8i.\n\
--DebugCfg              (optional)    If included, prints out configuration debugging information.\n\
--V8DllsDir=            (optional)    Specifies the relative directory containing the Dgn V8 Dlls. Default is DgnV8\n\
--CustomGcsDir=         (optional)    Specifies the directory containing custom GCS definition files\n\
%s\n", GetCommonCommandLineOptions().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge::CmdLineArgStatus RootModelConverterApp::_ParseCommandLineArg(int iArg, int argc, WCharCP argv[])
    {
    if (argv[iArg] == wcsstr(argv[iArg], L"--name-prefix="))
        {
        m_params.SetNamePrefix(GetArgValue(argv[iArg]).c_str());
        return CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--root-model="))
        {
        RootModelConverter::RootModelChoice rmc;

        Utf8String value = GetArgValue(argv[iArg]);
        if (value.EqualsI(".default"))
            rmc.SetUseDefaultModel();
        else if (value.EqualsI(".active"))
            rmc.SetUseActiveViewGroup();
        else
            rmc = RootModelConverter::RootModelChoice(value);

        m_params.SetRootModelChoice(rmc);
        return CmdLineArgStatus::Success;
        }

    if ( (0 == wcsncmp (argv[iArg], L"--DGN", 5)) || (0 == wcsncmp (argv[iArg], L"--dgn", 5)) ||
            (0 == wcsncmp (argv[iArg], L"--v8i", 5)) || (0 == wcsncmp (argv[iArg], L"--V8I", 5)) ||
            (0 == wcsncmp (argv[iArg], L"--V8DllsDir", 11)) ||
            (0 == wcsncmp (argv[iArg],  L"--DebugCfg", 10)))
        {
        // these are handled by the configuration variable reading code.
        return CmdLineArgStatus::Success;
        }

    if (0 == wcscmp(argv[iArg], L"--2d-Models-Spatial"))
        {
        m_params.SetConsiderNormal2dModelsSpatial(true);
        return CmdLineArgStatus::Success;
        }

    return T_Super::_ParseCommandLineArg(iArg, argc, argv);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProgressMeter& ConverterApp::GetProgressMeter() const
    {
    static NopProgressMeter s_nopMeter;
    auto meter = T_HOST.GetProgressMeter();
    return meter? *meter: s_nopMeter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
WString ConverterApp::_SupplySqlangRelPath()
    {
    // *** Keep this consistent with DgnV8Converter.mke
    return L"sqlang/DgnV8Converter_en-US.sqlang.db3";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConverterApp::CreateSyncInfoIfNecessary()
    {
    //  If I am creating a new local file or if I just acquired a briefcase for an existing repository, then I will have to bootstrap syncinfo.
    if (!SyncInfo::GetDbFileName(_GetParams().GetBriefcaseName()).DoesPathExist())
        {
        if (BSISUCCESS != SyncInfo::CreateEmptyFile(SyncInfo::GetDbFileName(_GetParams().GetBriefcaseName()))) // Bootstrap the V8 converter by pairing an empty syncinfo file with the briefcase
            return BSIERROR;
        }

    BeAssert(SyncInfo::GetDbFileName(_GetParams().GetBriefcaseName()).DoesPathExist());

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterApp::_DeleteSyncInfo()
    {
    SyncInfo::GetDbFileName(_GetParams().GetBriefcaseName()).BeDeleteFile();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootModelConverterApp::_MakeSchemaChanges()
    {
    auto status = m_converter->MakeSchemaChanges();
    return ((BSISUCCESS != status) || m_converter->WasAborted())? BSIERROR: BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootModelConverterApp::_OnOpenBim(DgnDbR db)
    {
    bool skipECData = false;
    if (m_converter.IsValid())
        skipECData = m_converter->m_skipECContent;
    m_converter.reset(new RootModelConverter(m_params));
    m_converter->m_skipECContent = skipECData;
    m_converter->SetDgnDb(db);
    CreateSyncInfoIfNecessary();
    return m_converter->AttachSyncInfo();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverterApp::_OnCloseBim(BentleyStatus)
    {
    m_converter.reset(nullptr); // this also has the side effect of closing the source files
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootModelConverterApp::_OpenSource()
    {
    auto initStat = m_converter->InitRootModel();
    if (DgnV8Api::DGNFILE_STATUS_Success != initStat)
        {
        switch (initStat)
            {
            case DgnV8Api::DGNOPEN_STATUS_FileNotFound:
                LOG.fatalv(L"%ls - file not found", _GetParams().GetInputFileName().GetName());
                fwprintf(stderr, L"%ls - file not found\n", _GetParams().GetInputFileName().GetName());
                break;

            default:
                m_converter->ReportDgnV8FileOpenError(initStat, _GetParams().GetInputFileName().c_str());
                LOG.fatalv(L"%ls - cannot find or load root model. See %ls-issues for more information.", _GetParams().GetInputFileName().GetName(), _GetParams().GetBriefcaseName().GetName());
            }

        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverterApp::_CloseSource(BentleyStatus)
    {
    // _OnCloseBim will close the source files
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr RootModelConverterApp::_FindJob()
    {
    auto status = m_converter->FindJob();
    return (SpatialConverterBase::ImportJobLoadStatus::Success == status)? &m_converter->GetImportJob().GetSubject(): nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr RootModelConverterApp::_InitializeJob()
    {
    BeAssert(m_converter->IsFileAssignedToBridge(*m_converter->GetRootV8File()) && "The bridge assigned to the root file/model must be the bridge that creates the root subject");

    auto status = m_converter->InitializeJob();

    //  Make sure that we really should be converting this model as our root
    if (RootModelConverter::ImportJobCreateStatus::Success != status)
        {
        if (RootModelConverter::ImportJobCreateStatus::FailedExistingNonRootModel == status)
            {
            // This model was converted by some other job and not as its root.
            // This is probably a user error. If we were to use this as a root, we could end up creating duplicates of it and its references, possibly
            //  using different transforms. That would probably only cause confusion.
            LOG.fatalv(L"%ls - error - the selected root model [%ls] was previously converted, not as a root but as a reference attachment.", _GetParams().GetBriefcaseName().GetName(), m_converter->GetRootModelP()->GetModelName());
            return nullptr;
            }

        BeAssert(RootModelConverter::ImportJobCreateStatus::FailedExistingRoot != status); // If the root was previously converted, then we should be doing an update!
        }

    return (RootModelConverter::ImportJobCreateStatus::Success == status)? &m_converter->GetImportJob().GetSubject(): nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootModelConverterApp::_ConvertToBim(Dgn::SubjectCR jobSubject)
    {
    m_converter->Process();
    return m_converter->WasAborted()? BSIERROR: BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootModelConverterApp::_DetectDeletedDocuments()
    {
    m_converter->_DetectDeletedDocuments();
    return m_converter->WasAborted()? BSIERROR: BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ConverterApp::Run(int argc, WCharCP argv[])
    {
    iModelBridgeSacAdapter::InitCrt(false);

    _GetConverterParams().SetWantProvenanceInBim(true); // this is set ONLY for stand-alone converters

    iModelBridgeSacAdapter::Params saparams;
    if (BentleyStatus::SUCCESS != iModelBridgeSacAdapter::ParseCommandLine(*this, saparams, argc, argv))
        return BentleyStatus::ERROR;

    if (saparams.GetLoggingConfigFile().empty())
        saparams.SetLoggingConfigFile(_GetLoggingConfigurationFilename(argv[0]));

    iModelBridgeSacAdapter::InitializeHost(*this, "DgnV8Converter");

    if (BSISUCCESS != _Initialize(argc, argv))
        {
        fprintf(stderr, "_Initialize failed\n");
        return BentleyStatus::ERROR;
        }

    return iModelBridgeSacAdapter::Execute(*this, saparams);
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
