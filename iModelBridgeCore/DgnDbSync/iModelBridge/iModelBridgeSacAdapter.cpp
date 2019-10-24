/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux)
#include <unistd.h>
#endif

#include <stdio.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>
#include <iModelBridge/iModelBridgeBimHost.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeDirectoryIterator.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <DgnPlatform/DgnIModel.h>
#include "iModelBridgeHelpers.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING

#undef min
#undef max

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridge"))

//=======================================================================================
// @bsiclass
//=======================================================================================
struct CompressProgressMeter : BeSQLite::ICompressProgressTracker
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
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isBimExt(BeFileNameCR fn)
    {
    auto ext = fn.GetExtension();
    return ext.EqualsI(iModelBridge::str_BriefcaseExt()) || ext.EqualsI(iModelBridge::str_BriefcaseIExt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isImodelExt(BeFileNameCR fn)
    {
    auto ext = fn.GetExtension();
    return ext.EqualsI(iModelBridgeSacAdapter::std_CompressedDgnDbExt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSacAdapter::InitializeHost(iModelBridge& bridge, Utf8CP productName)
    {
    BeFileName fwkAssets = bridge._GetParams ().GetAssetsDir (); 
    if (fwkAssets.empty ())
        {
        fwkAssets = Desktop::FileSystem::GetExecutableDir ();
        fwkAssets.AppendToPath (L"Assets");
        }

    BeFileName fwkDb3 = fwkAssets;
    fwkDb3.AppendToPath(L"sqlang");
    fwkDb3.AppendToPath(L"iModelBridgeFwk_en-US.sqlang.db3");

    static iModelBridgeBimHost s_host(bridge._GetParams().GetRepositoryAdmin(), fwkAssets, fwkDb3, productName); // *** TBD supply bridge name as product name
    s_host.SetGeoCoordinateDataDir(bridge._GetParams().GetGeoCoordData());
    DgnPlatformLib::Initialize(s_host);

    static PrintfProgressMeter s_meter;
    T_HOST.SetProgressMeter(&s_meter);

    // Also initialize the bridge-specific L10N. This is not part of the host. We do 
    // it here anyway, as a convenience to the standalone converter.
    BeFileName bridgeSqlangPath(bridge._GetParams().GetAssetsDir());
    bridgeSqlangPath.AppendToPath(bridge._SupplySqlangRelPath().c_str());
    iModelBridge::L10N::Initialize(BeSQLite::L10N::SqlangFiles(bridgeSqlangPath));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSacAdapter::CreateIModel(BeFileNameCR imodelName, BeFileNameCR bimName, Params const& saparams)
    {
    CreateIModelParams createImodelParams;
    createImodelParams.SetOverwriteExisting(true);
    if (0 != saparams.GetCompressChunkSize())
        createImodelParams.SetChunkSize(saparams.GetCompressChunkSize());

    T_HOST.GetProgressMeter()->SetCurrentStepName ("Creating IModel");
    BeSQLite::DbResult rc = DgnIModel::Create (imodelName, bimName, createImodelParams);

    if (BeSQLite::BE_SQLITE_OK == rc)
        {
        fwprintf(stdout, L"Created %ls\n", imodelName.GetName());
        return BSISUCCESS;
        }

    fprintf(stderr, "*** IModel Creation failed.****\n");
    imodelName.BeDeleteFile();
    return BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridgeSacAdapter::ExtractFromIModel(BeFileName& outFile, BeFileNameCR imodelFile)
    {
    uint64_t fileSize;
    imodelFile.GetFileSize(fileSize);
    CompressProgressMeter progress(fileSize, "Extract");

    BeSQLite::DbResult dbResult;
    auto status = DgnIModel::ExtractUsingDefaults(dbResult, outFile, imodelFile, true, &progress);

    if (status != DgnDbStatus::Success)
        {
        fprintf(stderr, "*** IModel extraction failed.****\n");
        return BSIERROR;
        }

    fwprintf(stdout, L"Extracted %ls\n", outFile.GetName());
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSacAdapter::CreateOrUpdateBim(iModelBridge& bridge, Params const& saparams)
    {
    BeFileName outputFileName = bridge._GetParams().GetBriefcaseName();
    BeFileName inputFileName  = bridge._GetParams().GetInputFileName();     // may be empty (in edge case, where only deleted file detection is needed)

    DgnDbPtr db; 
    time_t mtime;

    if (!outputFileName.DoesPathExist())
        {
        bvector<DgnModelId> dont_care;
        db = bridge.DoCreateDgnDb(dont_care, saparams.GetDescription().c_str());
        if (!db.IsValid())
            {
            fwprintf(stderr, L"%ls - creation failed. See %ls for details.\n", inputFileName.GetName(),
                     bridge._GetParams().GetReportFileName().GetName());
            return BSIERROR;
            }
        }
    else
        {
        BeSQLite::DbResult dbres;
        bool _hadDomainSchemaChanges = false;
        BeFileName::GetFileTime(nullptr, nullptr, &mtime, outputFileName);
        db = bridge.OpenBimAndMergeSchemaChanges(dbres, _hadDomainSchemaChanges, outputFileName);
        if (!db.IsValid())
            {
            fwprintf(stderr, L"%ls - file not found or could not be opened (error %x)\n", inputFileName.GetName(), (int)dbres);
            return BSIERROR;
            }

        //  Tell the bridge that the briefcase is now open and ask it to open the source file(s).
        iModelBridgeCallOpenCloseFunctions callCloseOnReturn(bridge, *db);
        if (!callCloseOnReturn.IsReady())
            {
            LOG.fatalv("Bridge is not ready or could not open source file");
            return BentleyStatus::ERROR;
            }

        db->SaveChanges(); // If the _OnOpenBim or _OpenSource callbacks did things like attaching syncinfo, we need to commit that before going on.
                           // This also prevents a call to AbandonChanges in _MakeSchemaChanges from undoing what the open calls did.

        //  Let the bridge generate schema changes
        bool hasMoreChanges = false;
        do {
            bridge._MakeSchemaChanges(hasMoreChanges);
        } while (hasMoreChanges);

        bool madeDynamicSchemaChanges = db->Txns().HasChanges(); // see if _MakeSchemaChanges made any changes.

        if (madeDynamicSchemaChanges) // if _MakeSchemaChanges made any dynamic schema changes, we close and re-open in order to accommodate them.
            {
            callCloseOnReturn.CallCloseFunctions(iModelBridge::ClosePurpose::SchemaUpgrade);

            _hadDomainSchemaChanges = false;
            db = bridge.OpenBimAndMergeSchemaChanges(dbres, _hadDomainSchemaChanges, outputFileName);
            if (!db.IsValid())
                {
                fwprintf(stderr, L"%ls - open failed with error %x\n", inputFileName.GetName(), (int)dbres);
                return BentleyStatus::ERROR;
                }
            BeAssert(!_hadDomainSchemaChanges);
            
            callCloseOnReturn.CallOpenFunctions(*db);
            }

        db->BriefcaseManager().GetChannelPropsR().channelType = IBriefcaseManager::ChannelType::Shared;
        db->BriefcaseManager().GetChannelPropsR().channelParentId = db->Elements().GetRootSubjectId();

        SubjectCPtr jobsubj;
        BentleyStatus bstatus = bridge.DoMakeDefinitionChanges(jobsubj, *db);
        if (BSISUCCESS == bstatus)
            {
            db->BriefcaseManager().GetChannelPropsR().channelType = IBriefcaseManager::ChannelType::Normal;

            bstatus = bridge.DoConvertToExistingBim(*db, *jobsubj, saparams.GetDetectDeletedFiles());
            }
        
        if (BSISUCCESS != bstatus)
            {
            fwprintf(stderr, L"%ls - conversion failed. See %ls for details.\n", inputFileName.GetName(),
                     bridge._GetParams().GetReportFileName().GetName());
            return BSIERROR;
            }
        }

    if (!bridge.HadAnyChanges())
        outputFileName.SetFileTime(nullptr, &mtime);

    if (saparams.GetExpirationDate().IsValid())
        db->SaveExpirationDate(saparams.GetExpirationDate());

    auto rc = db->SaveChanges();//_GetParams().GetDescription().c_str());
    if (BeSQLite::BE_SQLITE_OK != rc)
        {
        //ReportIssueV(IssueSeverity::Fatal, IssueCategory::DiskIO(), Issue::SaveError(), nullptr, m_dgndb->GetLastError().c_str());
        LOG.fatalv("SaveChanges failed with %s", db->GetLastError().c_str());
        db->AbandonChanges();
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

struct ClearDocumentPropertiesAccessor
    {
    iModelBridge& m_bridge;
    ClearDocumentPropertiesAccessor(iModelBridge& b) : m_bridge(b) {}
    ~ClearDocumentPropertiesAccessor() {m_bridge._GetParams().ClearDocumentPropertiesAccessor();}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSacAdapter::Execute(iModelBridge& bridge, Params const& saparams)
    {
    saparams.Initialize();

    bridge._GetParams().SetDocumentPropertiesAccessor(const_cast<Params&>(saparams));
    ClearDocumentPropertiesAccessor clearDocumentPropertiesAccessorOnReturn(bridge);

    BeFileName outputFileName = bridge._GetParams().GetBriefcaseName();
    BeFileName inputFileName  = bridge._GetParams().GetInputFileName();

    iModelBridgeCallTerminate callTerminate(bridge);
    callTerminate.m_status = BSISUCCESS;
    bool isNewFile = false;
    if (true)
        {
        if (isBimExt(inputFileName) && isImodelExt(outputFileName))
            {
            CreateIModel(outputFileName, inputFileName, saparams);
            return BSISUCCESS;
            }
        if (isImodelExt(inputFileName) && isBimExt(outputFileName))
            {
            return ExtractFromIModel(outputFileName, inputFileName);
            }

        if (!saparams.ShouldTryUpdate())
            {
            BeFileName::BeDeleteFile(outputFileName);
            bridge._DeleteSyncInfo();
            }

        isNewFile = !outputFileName.DoesPathExist();
    
        iModelBridgeBimHost_SetBridge _registerBridgeOnHost(bridge);

        bridge._GetParams().SetInputFileName(inputFileName);
        if (BSISUCCESS != CreateOrUpdateBim(bridge, saparams))
            {
            if (isNewFile)
                {
                outputFileName.BeDeleteFile();
                bridge._DeleteSyncInfo();
                }
            callTerminate.m_status = BSIERROR;
            return BSIERROR;
            }
        }

    /* NEEDS WORK
    if (m_config.GetOptionValueBool("CompactDatabase", true))
        {
        SetStepName(ProgressMessage::STEP_COMPACTING());
        m_dgndb->CompactFile();
        }
        */

    if (isNewFile)
        fwprintf(stdout, L"Created %ls\n", outputFileName.c_str());

    // don't print a message regarding updates, as it is inaccurate or misleading in the case where the bim is not changetracked

    if (saparams.GetCreateStandalone())
        {
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, outputFileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        if (db.IsValid())
            {
            db->SetAsBriefcase(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
            db->SaveChanges();
            }
        }

    if (saparams.ShouldCompress())
        {
        BeFileName briefcaseName(outputFileName);
        BeFileName imodelName(briefcaseName);
        imodelName.OverrideNameParts(L".imodel");
        CreateIModel(imodelName, briefcaseName, saparams);
        }

    BentleyApi::Http::HttpClient::Uninitialize();

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSacAdapter::Params::Initialize() const
    {
    InitCrt(m_quietAsserts);

    // Init logging
    if (!m_loggingConfigFile.empty() && m_loggingConfigFile.DoesPathExist())
        {
        NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, m_loggingConfigFile);
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        return;
        }

    fprintf(stderr, "Logging.config.xml not specified. Activating default logging using console provider.\n");
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity(L"Performance", NativeLogging::LOG_TRACE);
    //NativeLogging::LoggingConfig::SetSeverity(L"DgnCore", NativeLogging::LOG_TRACE);
    //NativeLogging::LoggingConfig::SetSeverity(L"BeSQLite", NativeLogging::LOG_TRACE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSacAdapter::Params::PrintUsage()
    {
    fprintf(stderr,
"STAND-ALONE-SPECIFIC CONVERTER OPTIONS:\n"
"--no-assert-dialogs            (optional) Prevents modal assert dialogs\n"
"--update[=description]         (optional) Causes the converter to update the output file, rather than re-create it.\n"
"--detect-deleted-files         (optional) Detect deleted files -- specify only if the absence of a file implies that it was deleted.\n"
"--logging-config-file=         (optional) The name of the logging configuration file.\n"
"--standalone                   (optional) Create a standalone (user-editable) DgnDb rather than a master DgnDb\n"
"--compress                     (optional) Additionally compresses the output into an .imodel\n"
"--description=                 (optional) A string saved as the 'description' property in the DgnDb.\n"
"--expiration=                  (optional) The expiration date of the file.\n"
"--job-name=                    (optional) The code for the new job subject when creating a dgndb.\n"
"--input-guid=                  (optional) The document GUID of the input file.\n"
"--doc-attributes=              (optional) Document atttributes for the input file (in JSON format).\n"
"--transform=                   (optional) 3x4 transformation matrix in row-major form in JSON wire format. This is an additional transform to to be pre-multiplied to the normal GCS/units conversion matrix that the bridge computes and applies to all converted spatial data.\n"
"--merge-definitions            (optional) Merge definitions such as levels and materials by name from different root models into the shared dictionary model. The default is to keep definitions for each root model separate.\n"
"--bridge-assetsDir=            (optional) the full path to the assets directory for the bridge.\n"
    );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge::CmdLineArgStatus iModelBridgeSacAdapter::Params::ParseCommandLineArg(int iArg, int argc, WCharCP argv[])
    {
    if (0 == wcscmp(argv[iArg], L"--no-assert-dialogs"))
        {
        m_quietAsserts = true;
        return iModelBridge::CmdLineArgStatus::Success;
        }
    if (0 == wcscmp(argv[iArg], L"--update") || argv[iArg] == wcsstr(argv[iArg], L"--update="))
        {
        m_tryUpdate = true;
        if (argv[iArg] == wcsstr(argv[iArg], L"--update="))
            SetDescription(iModelBridge::GetArgValue(argv[iArg]).c_str());

        return iModelBridge::CmdLineArgStatus::Success;
        }
    if (argv[iArg] == wcsstr(argv[iArg], L"--logging-config-file"))
        {
        m_loggingConfigFile.SetName(iModelBridge::GetArgValueW(argv[iArg]).c_str());
        return iModelBridge::CmdLineArgStatus::Success;
        }
    if (0 == wcscmp(argv[iArg], L"--standalone"))
        {
        SetCreateStandalone(true);
        return iModelBridge::CmdLineArgStatus::Success;
        }

    if (0 == wcscmp(argv[iArg], L"--detect-deleted-files"))
        {
        SetDetectDeletedFiles(true);
        return iModelBridge::CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--doc-attributes="))
        {
        SetDocAttributesJson(iModelBridge::GetArgValue(argv[iArg]).c_str());
        return iModelBridge::CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--description="))
        {
        SetDescription(iModelBridge::GetArgValue(argv[iArg]).c_str());
        return iModelBridge::CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--expiration="))
        {
        DateTime expiry;
        if (BSISUCCESS != DateTime::FromString(expiry, iModelBridge::GetArgValue(argv[iArg]).c_str()))
            {
            fprintf(stderr, "%s - invalid date\n", iModelBridge::GetArgValue(argv[iArg]).c_str());
            return iModelBridge::CmdLineArgStatus::Error;
            }
        if (expiry.GetInfo().GetKind() == DateTime::Kind::Utc)
            SetExpirationDate(expiry);
        else
            {
            DateTime expiryUtc(DateTime::Kind::Local, expiry.GetYear(), expiry.GetMonth(), expiry.GetDay(), expiry.GetHour(), expiry.GetMinute(), expiry.GetSecond(), expiry.GetMillisecond());
            expiryUtc.ToUtc(expiryUtc);
            SetExpirationDate(expiryUtc);
            }
        return iModelBridge::CmdLineArgStatus::Success;
        }

    if (0 == wcscmp(argv[iArg], L"--compress"))
        {
        m_shouldCompress = true;
        return iModelBridge::CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--compress-chunk-size"))
        {
        WCharCP compressSizeSpec = wcschr(argv[iArg], L'=');
        if (nullptr != compressSizeSpec)
            m_compressChunkSize = BeStringUtilities::Wtoi(compressSizeSpec+1);
        return iModelBridge::CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--input-guid"))
        {
        BeSQLite::BeGuid docGuid;
        if (docGuid.FromString(iModelBridge::GetArgValue(argv[iArg]).c_str()) != BSISUCCESS)
            {
            fprintf(stderr, "%s - invalid GUID\n", iModelBridge::GetArgValue(argv[iArg]).c_str());
            return iModelBridge::CmdLineArgStatus::Error;
            }
        SetDocumentGuid(docGuid);
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--bridge-assetsDir"))
        {
        m_bridgeAssetsDir.SetName(iModelBridge::GetArgValueW(argv[iArg]).c_str());
        return iModelBridge::CmdLineArgStatus::Success;
        }

    return iModelBridge::CmdLineArgStatus::NotRecognized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSacAdapter::Params::GetDocumentProperties(iModelBridgeDocumentProperties& props)
    {
    if (m_docGuid.IsValid())
        props.m_docGuid = m_docGuid.ToString();
    if (!m_attributesJSON.empty())
        props.m_attributesJSON = m_attributesJSON;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool iModelBridgeSacAdapter::Params::_IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) 
    {
    return m_isFileAssignedToBridge;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSacAdapter::Params::_QueryAllFilesAssignedToBridge(bvector<BeFileName>& fns, wchar_t const* bridgeRegSubKey) 
    {
    fns.push_back(m_dupInputFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSacAdapter::Params::_GetDocumentProperties(iModelBridgeDocumentProperties& props, BeFileNameCR fn) 
    {
    if (!m_dupInputFileName.EqualsI(fn))
        return BSIERROR;
        
    GetDocumentProperties(props);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSacAdapter::Params::_GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFilePath, BeSQLite::BeGuid const& docGuid)
    {
    if (!docGuid.IsValid() && !m_docGuid.IsValid())
        return _GetDocumentProperties(props, localFilePath);
        
    if (docGuid != m_docGuid)
        return BSIERROR;
    
    GetDocumentProperties(props);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void unSupportedFwkArg(WCharCP arg)
    {
    LOG.warningv(L"%ls - this command-line option is not supported by the iModelFramework", arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge::CmdLineArgStatus iModelBridgeSacAdapter::ParseCommandLineArg(iModelBridge::Params& bparams, int iArg, int argc, WCharCP argv[])
    {
    if (argv[iArg] == wcsstr(argv[iArg], L"--output=") || argv[iArg] == wcsstr(argv[iArg], L"-o=") || argv[iArg] == wcsstr(argv[iArg], L"-o"))
        {
        bparams.m_briefcaseName.SetName(iModelBridge::GetArgValueW(argv[iArg]));
        return iModelBridge::CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--input=") || argv[iArg] == wcsstr(argv[iArg], L"-i=") || argv[iArg] == wcsstr(argv[iArg], L"-i"))
        {
        BeFileName::FixPathName (bparams.m_inputFileName, iModelBridge::GetArgValueW(argv[iArg]).c_str());
        return BeFileName::IsDirectory(bparams.m_inputFileName.c_str())? iModelBridge::CmdLineArgStatus::Error: iModelBridge::CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--input-gcs=") || argv[iArg] == wcsstr(argv[iArg], L"--output-gcs="))
        {
        unSupportedFwkArg(argv[iArg]);
        Json::Value json = Json::Value::From (iModelBridge::GetArgValue(argv[iArg]));
        if (json.isNull() || (BSISUCCESS != bparams.ParseJsonArgs(json, argv[iArg] == wcsstr(argv[iArg], L"--input-gcs="))))
            return iModelBridge::CmdLineArgStatus::Error;
        return iModelBridge::CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--drawings-dirs="))
        {
        bparams.m_drawingsDirs.SetName(iModelBridge::GetArgValueW(argv[iArg]).c_str());
        return iModelBridge::CmdLineArgStatus::Success;
        }

    if (0 == wcscmp(argv[iArg], L"--no-thumbnails"))
        {
        unSupportedFwkArg(argv[iArg]);
        bparams.m_wantThumbnails = false;
        return iModelBridge::CmdLineArgStatus::Success;
        }

    if (0 == wcscmp(argv[iArg], L"--merge-definitions"))
        {
        bparams.SetMergeDefinitions(true);
        return iModelBridge::CmdLineArgStatus::Success;
        }

    if (argv[iArg] == wcsstr(argv[iArg], L"--thumbnailTimeout"))
        {
        unSupportedFwkArg(argv[iArg]);
        WCharCP val = wcschr(argv[iArg], L'=');
        if (nullptr == val)
            {
            fwprintf(stderr, L"%ls - missing timeout value - specify the number of seconds to wait as an integer\n", val);
            return iModelBridge::CmdLineArgStatus::Error;
            }
        ++val; // step past the =
        auto ival = BeStringUtilities::Wtoi(val); // returns 0 in case of parsing error
        if (ival <= 0)
            {
            fwprintf(stderr, L"%ls - invalid timeout value - specify the number of seconds to wait as an integer\n", val);
            return iModelBridge::CmdLineArgStatus::Error;
            }
        bparams.m_thumbnailTimeout = BeDuration::Seconds(ival);
        return iModelBridge::CmdLineArgStatus::Success;
        }
    if (argv[iArg] == wcsstr(argv[iArg], L"--job-name="))
        {
        unSupportedFwkArg(argv[iArg]);
        bparams.SetBridgeJobName(iModelBridge::GetArgValue(argv[iArg]).c_str());
        return iModelBridge::CmdLineArgStatus::Success;
        }

    return iModelBridge::CmdLineArgStatus::NotRecognized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSacAdapter::Init(iModelBridge::Params& bparams, int argc, WCharCP argv[])
    {
    bparams.m_isCreatingNewDb = false;
    bparams.m_libraryDir = Desktop::FileSystem::GetExecutableDir();
    bparams.m_assetsDir = bparams.m_libraryDir;     // When running in a standalone converter, the bridge is just part of the program, and so the bridge's assets are merged into the program's assets directory.
    bparams.m_assetsDir.AppendToPath(L"Assets");
    bparams.m_repoAdmin = new DgnPlatformLib::Host::RepositoryAdmin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSacAdapter::InitForBeTest(iModelBridge::Params& bparams)
    {
    bparams.m_isCreatingNewDb = false;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(bparams.m_assetsDir);
    bparams.m_libraryDir = bparams.m_assetsDir;
    bparams.m_libraryDir.AppendToPath(L"..");
    bparams.m_libraryDir.BeGetFullPathName();
    bparams.m_repoAdmin = new DgnPlatformLib::Host::RepositoryAdmin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSacAdapter::ParseCommandLine(bvector<WString>& unrecognized, iModelBridge::Params& bparams, Params& saparams, int argc, WCharCP argv[])
    {
    // The most common need for a developer is to publish one DGN file to the current directory with default options... make that easy.
    if (2 == argc)
        {
        WString arg1 = iModelBridge::GetArgValueW(argv[1]);
        if (BeFileName::DoesPathExist(arg1.c_str()))
            {
            BeFileName::FixPathName(bparams.m_inputFileName, arg1.c_str());
            bparams.m_briefcaseName.SetName(L".");
            return BentleyStatus::SUCCESS;
            }
        }

    for (int iArg = 1; iArg < argc; ++iArg)
        {
        iModelBridge::CmdLineArgStatus res = ParseCommandLineArg(bparams, iArg, argc, argv);
        if (iModelBridge::CmdLineArgStatus::NotRecognized == res)
            {
            res = saparams.ParseCommandLineArg(iArg, argc, argv);
            if (iModelBridge::CmdLineArgStatus::NotRecognized == res)
                {
                unrecognized.push_back(argv[iArg]);
                continue;
                }
            }

        if (iModelBridge::CmdLineArgStatus::Success != res)    // handled but invalid
            return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2012
//---------------------------------------------------------------------------------------
static void justLogAssertionFailures(WCharCP message, WCharCP file, uint32_t line, BeAssertFunctions::AssertType atype)
    {
    WPrintfString str(L"ASSERT: (%ls) @ %ls:%u\n", message, file, line);
    LOG.error(str.c_str());
    //::OutputDebugStringW (str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSacAdapter::InitCrt(bool quietAsserts)
    {
#ifdef NDEBUG
    quietAsserts = true; // we never allow disruptive asserts in a production program
#endif

    if (quietAsserts)
        BeAssertFunctions::SetBeAssertHandler(justLogAssertionFailures);

#ifdef _WIN32
    if (quietAsserts)
        _set_error_mode(_OUT_TO_STDERR);
    else
        _set_error_mode(_OUT_TO_MSGBOX);

    #if defined (UNICODE_OUTPUT_FOR_TESTING)
        // turning this on makes it so we can show unicode characters, but screws up piped output for programs like python.
        _setmode(_fileno(stdout), _O_U16TEXT);  // so we can output any and all unicode to the console
        _setmode(_fileno(stderr), _O_U16TEXT);  // so we can output any and all unicode to the console
    #endif

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
#else
    // unix-specific CRT init
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2017
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridgeSacAdapter::FixInputFileName(iModelBridge::Params& bparams)
    {
    BeFileName fixedName = bparams.GetInputFileName();
    if ((BeFileNameStatus::Success != fixedName.BeGetFullPathName()) || !fixedName.DoesPathExist() && fixedName.IsDirectory())
        return ERROR;

    bparams.m_inputFileName = fixedName;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSacAdapter::FixBriefcaseName(iModelBridge::Params& bparams)
    {
    BeFileName fixedName = bparams.GetBriefcaseName();
    fixedName.BeGetFullPathName();
    if (!fixedName.DoesPathExist())
        {
        if (fixedName.GetExtension().empty())    // it's probably supposed to be a directory name
            fixedName.AppendSeparator();
        BeFileName outputDir = fixedName.GetDirectoryName();
        if (!outputDir.DoesPathExist() && (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(outputDir.c_str())))
            {
            LOG.fatalv(L"Cannot create output directory <%ls>", outputDir.c_str());
            return BSIERROR;
            }
        }

    if (fixedName.IsDirectory())
        {
        fixedName.AppendToPath(bparams.GetInputFileName().GetFileNameWithoutExtension().c_str());
        fixedName.AppendExtension(iModelBridge::str_BriefcaseExt());
        }
    else//If no extension override it
        {
        //fixedName.OverrideNameParts(L"." str_BriefcaseExt());
        }

    bparams.m_briefcaseName = fixedName;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSacAdapter::ParseCommandLine(iModelBridge& bridge, Params& saparams, int argc, WCharCP argv[])
    {
    Init(bridge._GetParams(), argc, argv);

    bvector<WString> unrecognizedArgs;

    if (BSISUCCESS != ParseCommandLine(unrecognizedArgs, bridge._GetParams(), saparams, argc, argv))
        {
        PrintCommandLineUsage(bridge, argc, argv);
        return BSIERROR;
        }

    if (BSISUCCESS != bridge._GetParams().Validate())
        {
        PrintCommandLineUsage(bridge, argc, argv);
        return BSIERROR;
        }

    if (!saparams.GetBridgeAssetsDir().empty())
        bridge._GetParams().SetAssetsDir(saparams.GetBridgeAssetsDir());

    FixInputFileName(bridge._GetParams());
    FixBriefcaseName(bridge._GetParams());

    bridge._GetParams().SetReportFileName();
    bridge._GetParams().GetReportFileName().BeDeleteFile();

    saparams.SetDupInputFileName(bridge._GetParams().GetInputFileName());

    if (unrecognizedArgs.empty())
        return BSISUCCESS;

    int unrecognizedArgCount = (int)unrecognizedArgs.size();
    bvector<WCharCP> unrecognizedArgPtrs;
    for (auto& a : unrecognizedArgs)
        unrecognizedArgPtrs.push_back(a.c_str());

    for (int i=0; i<unrecognizedArgCount; ++i)
        {
        auto status = bridge._ParseCommandLineArg(i, unrecognizedArgCount, unrecognizedArgPtrs.data());
        if (iModelBridge::CmdLineArgStatus::Success == status)
            continue;

        if (iModelBridge::CmdLineArgStatus::NotRecognized == status)
            fwprintf(stderr, L"unrecognized option: %s\n", unrecognizedArgPtrs[i]);
        else
            fwprintf(stderr, L"invalid option: %s\n", unrecognizedArgPtrs[i]);

        PrintCommandLineUsage(bridge, argc, argv);
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void buildArgv(bvector<WCharCP>& bargptrs, bvector<WString>& bargs, bvector<bpair<WString,WString>> const& args)
    {
    bargs.push_back(L"do not try to use program name");

    for (auto& pair : args)
        {
        WString arg(pair.first);
        arg.append(pair.second);
        bargs.push_back(arg);
        }

    for (auto& arg : bargs)
        bargptrs.push_back(arg.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSacAdapter::ParseCommandLineForBeTest(iModelBridge& bridge, bvector<bpair<WString,WString>> const& argPairs)
    {
    bvector<WCharCP> bargptrs;
    bvector<WString> bargs;
    buildArgv(bargptrs, bargs, argPairs);

    int argc = (int)bargptrs.size();
    WCharCP* argv = bargptrs.data();

    InitForBeTest(bridge._GetParams());

    bvector<WString> unrecognizedArgs;

    Params saparams;
    if (BSISUCCESS != ParseCommandLine(unrecognizedArgs, bridge._GetParams(), saparams, argc, argv))
        BeTest::Fail();

    if (BSISUCCESS != bridge._GetParams().Validate())
        BeTest::Fail();

    FixBriefcaseName(bridge._GetParams());

    if (unrecognizedArgs.empty())
        return;

    int unrecognizedArgCount = (int)unrecognizedArgs.size();
    bvector<WCharCP> unrecognizedArgPtrs;
    for (auto& a : unrecognizedArgs)
        unrecognizedArgPtrs.push_back(a.c_str());

    for (int i=0; i<unrecognizedArgCount; ++i)
        {
        auto status = bridge._ParseCommandLineArg(i, unrecognizedArgCount, unrecognizedArgPtrs.data());
        if (iModelBridge::CmdLineArgStatus::Success != status)
            BeTest::Fail();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSacAdapter::PrintCommandLineUsage(iModelBridge& bridge, int argc, WCharCP argv[])
    {
    fwprintf(stderr,
L"Usage: %ls -i|--input= -o|--output= [OPTIONS...]\n"
L"--input=                    (required)  Input root filename.\n"
L"--output=                   (required)  Output directory\n"
L"OPTIONS:\n"
L"--input-gcs=gcsspec         (optional)  Specifies the GCS of the input DGN root model. Ignored if DGN root model already has a GCS. gcsspec must be in JSON format.\n"
L"--output-gcs=gcsspec        (optional)  Specifies the GCS of the output DgnDb file. Ignored if the output DgnDb file already has a GCS (update mode). gcsspec must be in JSON format.\n"
L"                                        gcsspec is either the keyname of a GCS or the coordinates of the origin and the azimuthal angle for an AZMEA GCS.\n"
L"--drawings-dirs=            (optional)  A semicolon-separated list of directories to search recursively for drawings and sheets.\n"
L"--geoCalculation=           (optional)  If a new model is added with the --update option, sets the geographic coordinate calculation method. Possible Values are:\n"
L"                                        Default   - Base the calculation method on the source GCS. GCS's calculated from Placemarks are transformed, others are reprojected\n"
L"                                        Reproject - Do full geographic reprojection of each point\n"
L"                                        Transform - use the source GCS and DgnDb GCS to construct and use a linear transform\n"
L"                                        TransformScaled - Similar to Transform, except the transform is scaled to account for the GCS grid to ground scale (also known as K factor).\n"
L"--revision-comment=         (optional)  A description of the changes that are being pushed in the revision.\n"
L"--job-name=                 (optional)  A description of the job (becomes the job subject element's code). Used only in the initial conversion.\n"
L"--no-thumbnails             (optional)  Do not generate view thumbnails\n"
L"--thumbnailTimeout          (optional)  Maximum number of seconds wait for each thumbnail. The default is 30 seconds.\n"
    , argv[0]);

    Params::PrintUsage();

    bridge._PrintUsage();
    }
