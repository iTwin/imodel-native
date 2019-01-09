/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Apps/DwgBridge.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <Dwg/DwgBridge.h>
#include    <Dwg/DwgHelper.h>
#include    <iModelBridge/iModelBridgeSacAdapter.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWG

// Use the same file names created by the makefile:
static WCharCP      s_configFileName = L"DwgImporter.logging.config.xml";
static WCharCP      s_sqlangFileName = L"sqlang/DwgImporter_en-US.sqlang.db3";
static DwgBridge*   s_dwgBridgeInstance = nullptr;

// Need different registry keys for OpenDWG and RealDWG bridge installers to avoid conflicts uninstalling the products.
#ifdef DWGTOOLKIT_OpenDwg
    static const wchar_t* s_dwgBridgeRegKey = L"OpenDwgBridge";
#elif DWGTOOLKIT_RealDwg
    static const wchar_t* s_dwgBridgeRegKey = L"RealDwgBridge";
#endif


BEGIN_DWG_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgBridge::_PrintUsage ()
    {
    fwprintf (stderr,
L"\
--unstableIds           (optional; publishing only) A flag in syncinfo that indicates that subsequent updates should assume that ElementIds are not a reliable way to cross-check elements in two versions of a file. The same piece of graphics could have been assigned a new ElementId in the new version of the file.\n\
--configuration=        (optional; publishing only) Path to the publisher configuration file (defaults to \"ConvertConfig.xml\" next to the EXE)\n\
--password=             Password needed for the importer to open protected input files.\n\
");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge::CmdLineArgStatus DwgBridge::_ParseCommandLineArg (int iArg, int argc, WCharCP argv[])
    {
    if (0 == wcscmp(argv[iArg], L"--unstableIds"))
        {
        GetImportOptions().SetStableIdPolicy(StableIdPolicy::ByHash);
        return CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--password="))
        {
        GetImportOptions().SetPassword(GetArgValue(argv[iArg]).c_str());
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

        GetImportOptions().SetConfigFile(configFile);
        return CmdLineArgStatus::Success;
        }

    // This is not a common command line argument => let the subclass handle it
    return CmdLineArgStatus::NotRecognized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBridge::GetLogConfigurationFilename(BeFileNameR configFile, WCharCP argv0)
    {
    WString programBasename = BeFileName::GetFileNameWithoutExtension(argv0);

    if (BentleyStatus::SUCCESS == GetEnv(configFile, L"BENTLEY_DWGIMPORTER_LOGGING_CONFIG"))
        {
        if (configFile.DoesPathExist())
            {
            fwprintf (stdout, L"%ls configuring logging with %s (Set by BENTLEY_DWGIMPORTER_LOGGING_CONFIG environment variable.)\n", programBasename.c_str(), configFile.GetName());
            return BentleyStatus::SUCCESS;
            }
        }

    configFile = BeFileName(BeFileName::DevAndDir, argv0);
    configFile.AppendToPath(s_configFileName);
    configFile.BeGetFullPathName();

    if (BeFileName::DoesPathExist(configFile))
        {
        fwprintf (stdout, L"%ls configuring logging using %ls. Override it by setting BENTLEY_DWGIMPORTER_LOGGING_CONFIG in environment.\n", programBasename.c_str(), configFile.GetName());
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgBridge::GetImportConfiguration (BeFileNameR instanceFilePath, BeFileNameCR configurationPath, WCharCP argv0)
    {
    if (!configurationPath.empty())
        {
        instanceFilePath.SetName(configurationPath);
        }
    else
        {
        WString programDir = BeFileName::GetDirectoryName (argv0);
        instanceFilePath.SetName(programDir.c_str());
        instanceFilePath.AppendToPath(L"Assets");
        instanceFilePath.AppendToPath(L"ConvertConfig.xml");
        }
    BeAssert (instanceFilePath.DoesPathExist());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBridge::GetEnv (BeFileName& fn, WCharCP envname)
    {
    wchar_t filepath[MAX_PATH];
    if ((0 == ::GetEnvironmentVariableW(envname, filepath, MAX_PATH)))
        return ERROR;

    fn.SetName(filepath);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
WString DwgBridge::_SupplySqlangRelPath ()
    {
    return s_sqlangFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBridge::_ConvertToBim (Dgn::SubjectCR jobSubject)
    {
    auto status = m_importer->Process ();

    if (m_importer->WasAborted())
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBridge::_MakeSchemaChanges () 
    {
    // create DwgAttributeDefinition shema
    if (BentleyStatus::SUCCESS != m_importer->MakeSchemaChanges() || m_importer->WasAborted())
        return  BentleyStatus::ERROR;
    return  BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBridge::_MakeDefinitionChanges (SubjectCR jobSubject)
    {
    if (BentleyStatus::SUCCESS != m_importer->MakeDefinitionChanges(jobSubject) || m_importer->WasAborted())
        return  BentleyStatus::ERROR;
    return  BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBridge::_OnOpenBim (DgnDbR bim)
    {
    // instantiate a new importer to begin a new job
    m_importer.reset (this->_CreateDwgImporter());
    // will save elements into target BIM
    m_importer->SetDgnDb (bim);
    // boostrap importer with required syncInfo file
    this->CreateSyncInfoIfAbsent ();
    // attach syncInfo to importer
    return  m_importer->AttachSyncInfo ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridge::_OnCloseBim (BentleyStatus status, iModelBridge::ClosePurpose purpose)
    {
    // remove the importer
    m_importer.reset (nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter*    DwgBridge::_CreateDwgImporter ()
    {
    // provide the default DWG importer
    return  new DwgImporter (m_options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridge::_Terminate (BentleyStatus convertStatus)
    {
    // terminate the toolkit after DwgDbDatabase is released
    DwgImporter::TerminateDwgHost ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBridge::_Initialize (int argc, WCharCP argv[])
    {
    if (_GetParams().GetBridgeRegSubKey().empty())
        _GetParams().SetBridgeRegSubKey(s_dwgBridgeRegKey);

    auto host = DgnPlatformLib::QueryHost ();
    if (nullptr == host)
        {
        BeAssert(false && "framework must register the host");
        return  BentleyStatus::ERROR;
        }

    // Initialize the importer, using default toolkit installation (same location as this app):
    DwgImporter::Initialize (nullptr);

    // Resolve import config file.
    BeFileName configFile;
    GetImportConfiguration (configFile, GetImportOptions().GetConfigFile(), argv[0]);
    GetImportOptions().SetConfigFile (configFile);

    // Set dir prefix to be dropped from filenames when coming up with unique but portable filenames in syncinfo
    GetImportOptions().SetInputRootDir (_GetParams().GetInputFileName().GetDirectoryName());

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SubjectCPtr DwgBridge::_InitializeJob ()
    {
    auto status = m_importer->InitializeJob ();

    if (DwgImporter::ImportJobCreateStatus::Success == status)
        return  &m_importer->GetImportJob().GetSubject ();

    BeAssert(false && "failed initializing the ImportJob for DWG bridge!");
    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SubjectCPtr DwgBridge::_FindJob ()
    {
    auto status = m_importer->FindJob ();
    if (DwgImporter::ImportJobLoadStatus::Success == status)
        return  &m_importer->GetImportJob().GetSubject ();

    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridge::CreateSyncInfoIfAbsent ()
    {
    //  If I am creating a new local file or if I just acquired a briefcase for an existing repository, then I will have to bootstrap syncinfo.
    if (!DwgSyncInfo::GetDbFileName(_GetParams().GetBriefcaseName()).DoesPathExist())
        {
        // Bootstrap the DWG importer by pairing an empty syncinfo file with the briefcase
        DwgSyncInfo::CreateEmptyFile (DwgSyncInfo::GetDbFileName(_GetParams().GetBriefcaseName()));
        }

    BeAssert (DwgSyncInfo::GetDbFileName(_GetParams().GetBriefcaseName()).DoesPathExist() && "syncInfo file not exists!");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridge::_DeleteSyncInfo ()
    {
    // force deleting the syncInfo
    DwgSyncInfo::GetDbFileName(_GetParams().GetBriefcaseName()).BeDeleteFile ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBridge::_OpenSource ()
    {
    // query DWG input file name
    auto inputFilename = _GetParams().GetInputFileName ();
    // set it as root file and try opening it
    auto status = m_importer->OpenDwgFile (inputFilename);

    if (BentleyStatus::SUCCESS != status)
        fwprintf (stderr, L"Error opening DWG file %ls!\n", inputFilename.GetName());

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgBridge::_CloseSource (BentleyStatus status, iModelBridge::ClosePurpose)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgBridge::_DetectDeletedDocuments()
    {
    m_importer->_DetectDeletedDocuments();
    return m_importer->WasAborted() ? BSIERROR : BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBridge::RunAsStandaloneExe (int argc, WCharCP argv[])
    {
    iModelBridgeSacAdapter::InitCrt (false);

    // Parse command line arguments into the bridge params
    iModelBridgeSacAdapter::Params  params;

    auto    status = iModelBridgeSacAdapter::ParseCommandLine (*this, params, argc, argv);
    if (status != BentleyStatus::SUCCESS)
        return  status;

    // Set a file for logging
    BeFileName  logFilename;
    if (params.GetLoggingConfigFile().empty() && 
        (status = this->GetLogConfigurationFilename(logFilename, argv[0])) == BentleyStatus::SUCCESS)
        params.SetLoggingConfigFile (logFilename);

    if (status != BentleyStatus::SUCCESS)
        return  status;

    // set a default job description for the job, as a be_Prop
    if (params.GetDescription().empty())
        params.SetDescription ("Job run as a standalone bridge");

    // set LastEditor to the standalone exe name, as a be_Prop
    Utf8String  editor(BeFileName::GetFileNameWithoutExtension(argv[0]).c_str());

    // Initialize the DgnHost
    iModelBridgeSacAdapter::InitializeHost (*this, editor.c_str());

    // Initialize the importer
    status = this->_Initialize (argc, argv);
    if (status != BentleyStatus::SUCCESS)
        {
        fwprintf (stderr, L"Failed initializing DWG bridge!\n");
        return status;
        }

    // tell the importer that it will be run from a standalone app:
    GetImportOptions().SetRunAsStandaloneApp (true);

    // Run the bridge
    status = iModelBridgeSacAdapter::Execute (*this, params);

    return  status;
    }

END_DWG_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge* iModelBridge_getInstance(wchar_t const* bridgeRegSubKey)
    {
    // Instiate and supply a generic DwgBridge, for either OpenDwgBridge or RealDwgBridge
    s_dwgBridgeInstance = new DwgBridge ();
    return  s_dwgBridgeInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridge_releaseInstance(iModelBridge* bridgeInstance)
    {
    if (dynamic_cast<DwgBridge*>(bridgeInstance) != nullptr)
        {
        delete s_dwgBridgeInstance;
        s_dwgBridgeInstance = nullptr;
        return  BentleyStatus::SUCCESS;
        }
    return  BentleyStatus::ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridge_getAffinity(WCharP buffer, const size_t bufferSize, iModelBridgeAffinityLevel& affinityLevel, WCharCP affinityLibPath, WCharCP dwgdxfName)
    {
    // default the affinity to None:
    affinityLevel = BentleyApi::Dgn::iModelBridge::Affinity::None;

    // A generic DwgBridge supports any valid DWG, DXF and DXB file type.
    BeFileName  filename(dwgdxfName);
    if (DwgHelper::SniffDwgFile(filename) || DwgHelper::SniffDxfFile(filename))
        {
        affinityLevel = BentleyApi::Dgn::iModelBridge::Affinity::Medium;
        BeStringUtilities::Wcsncpy(buffer, bufferSize, s_dwgBridgeRegKey);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/18
+---------------+---------------+---------------+---------------+---------------+------*/
EXPORT_ATTRIBUTE bool DwgBridge_getBridgeRegistryKey (WCharP regKey, const size_t size)
    {
    // DwgBridge is the module who knows which the registry key is in effective:
    regKey[0] = 0;
    return nullptr != BeStringUtilities::Wcsncpy(regKey, size, s_dwgBridgeRegKey);
    }
