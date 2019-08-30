/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    <Dwg/DwgBridge.h>
#include    <Dwg/DwgHelper.h>
#include    <iModelBridge/iModelBridgeSacAdapter.h>
#include    <WebServices/Client/ClientInfo.h>
#include    "prg.h" // generated header with build version number

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
    BeFileName programDir(_GetParams().GetLibraryDir());

    if (!configurationPath.empty())
        {
        instanceFilePath.SetName(configurationPath);
        }
    else
        {
        if (!programDir.DoesPathExist())
            programDir.assign (BeFileName::GetDirectoryName(argv0));
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
    if (m_importer->WasAborted() || BentleyStatus::SUCCESS != m_importer->MakeSchemaChanges())
        return  BentleyStatus::ERROR;
    return  BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBridge::_MakeDefinitionChanges (SubjectCR jobSubject)
    {
    if (m_importer->WasAborted() || BentleyStatus::SUCCESS != m_importer->MakeDefinitionChanges(jobSubject))
        return  BentleyStatus::ERROR;
    return  BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBridge::_OnOpenBim (DgnDbR bim)
    {
    if (!bim.IsDbOpen() || bim.IsReadonly())
        {
        BeAssert(false && "DbDb is not open or open for read only!!");
        return  BentleyStatus::BSIERROR;
        }
    // instantiate a new importer to begin a new job
    if (m_importer == nullptr)
        {
        m_importer.reset (this->_CreateDwgImporter());
        if (m_importer == nullptr)
            return  BentleyStatus::BSIERROR;
        }
    // will save elements into target BIM
    m_importer->SetDgnDb (bim);
    return  BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridge::_OnCloseBim (BentleyStatus status, iModelBridge::ClosePurpose purpose)
    {
    // remove the importer if done, or release pointer help by the importer
    if (purpose == iModelBridge::ClosePurpose::SchemaUpgrade)
        m_importer->_ReleaseDgnDb ();
    else
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
    // remove the importer and terminate the toolkit
    m_importer.reset (nullptr);
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
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBridge::_OpenSource ()
    {
    // query DWG input file name
    auto inputFilename = _GetParams().GetInputFileName ();
    auto existingFilename = m_importer->GetRootDwgFileName();

    BentleyStatus   status = inputFilename.DoesPathExist() ? BSISUCCESS : BSIERROR;

    // set it as root file and try opening it, if not already open
    if (!existingFilename.EqualsI(inputFilename))
        status = m_importer->OpenDwgFile (inputFilename);

    if (BentleyStatus::SUCCESS != status)
        fwprintf (stderr, L"Error opening DWG file %ls!\n", inputFilename.GetName());

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgBridge::_CloseSource (BentleyStatus status, iModelBridge::ClosePurpose)
    {
    // we don't want to close DWG file mainly for the sake of performance
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFontCP   DwgBridge::_TryResolveFont (DgnFontCP font)
    {
    return m_importer->ResolveFont(font);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridge::_SetClientInfo ()
    {
    static constexpr char s_bridgeName[] = "iModelBridgeService-Dwg";
    static constexpr char s_bridgeGuid[] = "977E6087-1F14-420E-B51C-C1BC0F128BC4";
    static constexpr char s_bridgePrgId[] = "2706";

    BeVersion bridgeVersion(REL_V "." MAJ_V "." MIN_V "." SUBMIN_V);
    auto& params = this->_GetParams();
    auto clientInfo = WebServices::ClientInfo::Create(s_bridgeName, bridgeVersion, s_bridgeGuid, s_bridgePrgId, params.GetDefaultHeaderProvider());

    params.SetClientInfo (clientInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgBridge::DwgBridge ()
    {
    this->_SetClientInfo ();
    }

END_DWG_NAMESPACE


#ifdef DWGTOOLKIT_OpenDwg

#include    <Dwg/DwgDb/DwgDbHost.h>
USING_NAMESPACE_DWGDB

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          08/19
+===============+===============+===============+===============+===============+======*/
struct SniffHost : IDwgDbHost
{
SniffHost()
    {
    IDwgDbHost::InitializeToolkit (*this);
    }
~SniffHost()
    {
    IDwgDbHost::TerminateToolkit ();
    }

bool _IsValid() const override { return true; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus _FindFile (WStringR fullpathOut, WCharCP filenameIn, DwgDbDatabaseP dwg = nullptr, AcadFileType hint = AcadFileType::Default) override
    {
    LPCWSTR fname = static_cast<LPCWSTR> (filenameIn);
    LPCWSTR extname = nullptr;
    if (hint == AcadFileType::ARXApplication)
        extname = L".tx";

    WCHAR found[2000] = { 0 };
    DWORD maxChars = _countof (found);
    LPWSTR filepart = nullptr;
    LPCWSTR searchPath = nullptr;

    auto foundChars = ::SearchPathW (searchPath, fname, extname, maxChars, found, &filepart);
    if (foundChars > 0 && foundChars < maxChars)
        {
        fullpathOut.assign (reinterpret_cast<WCharCP>(found), foundChars);
        return  DwgDbStatus::Success;
        }
    return DwgDbStatus::FileNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void _FatalError (WCharCP format, ...) override
    {
    va_list     varArgs;
    va_start (varArgs, format);

    WString     err = WPrintfString (format, varArgs);
    va_end (varArgs);

    ::fwprintf (stderr, L"Toolkit fatal error: %ls\n", err.c_str());
    }

}; // SniffHost

static SniffHost*   s_sniffHost = nullptr;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
static bool HasAecPropertySetDefs (BeFileNameCR filename)
    {
    if (s_sniffHost == nullptr)
        s_sniffHost = new SniffHost ();

    bool hasAecPsetDefs = false;
    try
        {
        auto dwg = s_sniffHost->ReadFile(filename, false, false, FileShareMode::DenyNo);
        if (dwg.IsValid())
            {
            DwgDbObjectId   aecpsetdefsId;
            DwgDbDictionaryPtr  mainDictionary(dwg->GetNamedObjectsDictionaryId(), DwgDbOpenMode::ForRead);
            if (mainDictionary.OpenStatus() == DwgDbStatus::Success && mainDictionary->GetIdAt(aecpsetdefsId, L"AEC_PROPERTY_SET_DEFS") == DwgDbStatus::Success)
                {
                DwgDbDictionaryPtr  aecpsetDefs(aecpsetdefsId, DwgDbOpenMode::ForRead);
                if (aecpsetDefs.OpenStatus() == DwgDbStatus::Success)
                    {
                    // find at least 1 entry under AEC_PROPERTY_SET_DEFS
                    auto iter = aecpsetDefs->GetIterator();
                    if (iter.IsValid() && iter->IsValid())
                        hasAecPsetDefs = !iter->Done();
                    }
                }
            }
        }
    catch (...)
        {
        ::fwprintf (stderr, L"An exception thrown opening file %ls\n", filename.c_str());
        }

    return  hasAecPsetDefs;
    }
#endif

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
#ifdef DWGTOOLKIT_RealDwg
        affinityLevel = BentleyApi::Dgn::iModelBridge::Affinity::Medium;
        BeStringUtilities::Wcsncpy(buffer, bufferSize, s_dwgBridgeRegKey);

#elif DWGTOOLKIT_OpenDwg
        if (HasAecPropertySetDefs(filename))
            {
            affinityLevel = BentleyApi::Dgn::iModelBridge::Affinity::High;
            BeStringUtilities::Wcsncpy(buffer, bufferSize, s_dwgBridgeRegKey);
            }
#endif
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
