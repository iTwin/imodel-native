/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#if defined (BENTLEY_WIN32)
#define UNICODE
#include <Windows.h>
#endif

#include <Bentley/BeTimeUtilities.h>
#include <BimFromDgnDb/BimFromDgnDbAPI.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnIModel.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <BeSQLite/L10N.h>
#include "BimUpgrader.h"

USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY

#define FORMAT_LOGGABLE_MSG(msg,fmt) {                          \
                                va_list args;                   \
                                va_start (args, fmt);           \
                                msg = WPrintfString(fmt, args); \
                                va_end (args);                  \
                                }

static WCharCP s_configFileName = L"BimUpgrader.logging.config.xml";
#define BIM_EXT L"bim"

BEGIN_BIM_FROM_DGNDB_NAMESPACE


struct KnownDesktopLocationsAdmin : BentleyB0200::Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin
    {
    BeFileName m_tempDirectory;
    BeFileName m_executableDirectory;
    BeFileName m_assetsDirectory;

    virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override { return m_tempDirectory; }
    virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override { return m_assetsDirectory; }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   BentleySystems
    //---------------------------------------------------------------------------------------
    KnownDesktopLocationsAdmin()
        {
        // use the standard Windows temporary directory
        wchar_t tempPathW[MAX_PATH];
        ::GetTempPathW(_countof(tempPathW), tempPathW);
        m_tempDirectory.SetName(tempPathW);
        m_tempDirectory.AppendSeparator();

        // the application directory is where the executable is located
        wchar_t moduleFileName[MAX_PATH];
        ::GetModuleFileNameW(NULL, moduleFileName, _countof(moduleFileName));
        BeFileName moduleDirectory(BeFileName::DevAndDir, moduleFileName);
        m_executableDirectory = moduleDirectory;
        m_executableDirectory.AppendSeparator();

        m_assetsDirectory = m_executableDirectory;
        m_assetsDirectory.AppendToPath(L"Assets");
        }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
struct BimUpgraderHost : BentleyB0200::Dgn::DgnPlatformLib::Host
    {
    virtual void                        _SupplyProductName(Utf8StringR name) override { name.assign("BimUpgrader"); }
    virtual IKnownLocationsAdmin&       _SupplyIKnownLocationsAdmin() override { return *new KnownDesktopLocationsAdmin(); };

    virtual BentleyB0200::BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override
        {
        BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
        sqlangFile.AppendToPath(L"sqlang");
        sqlangFile.AppendToPath(L"BimFromJson_en-US.sqlang.db3");

        return BentleyB0200::BeSQLite::L10N::SqlangFiles(sqlangFile);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus getEnv(BeFileName& fn, WCharCP envname)
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
WString BimUpgrader::GetArgValueW(WCharCP arg)
    {
    WString argValue(arg);
    argValue = argValue.substr(argValue.find_first_of('=', 0) + 1);
    argValue.Trim(L"\"");
    argValue.Trim();
    return argValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void BimUpgrader::PrintError(WCharCP fmt, ...)
    {
    WString msg;
    FORMAT_LOGGABLE_MSG(msg, fmt)
    fwprintf(stderr, msg.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void BimUpgrader::_PrintMessage(WCharCP fmt, ...)
    {
    WString msg;
    FORMAT_LOGGABLE_MSG(msg,fmt)
    wprintf(msg.c_str());
    //GetLogger().info(msg.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BimUpgrader::GetLogConfigurationFilename(BeFileName& configFile, WCharCP argv0)
    {
    WString programBasename = BeFileName::GetFileNameWithoutExtension(argv0);

    if (SUCCESS == getEnv(configFile, L"BENTLEY_BIMUPGRADER_LOGGING_CONFIG"))
        {
        if (configFile.DoesPathExist())
            {
            _PrintMessage(L"%ls configuring logging with %s (Set by BENTLEY_BIMUPGRADER_LOGGING_CONFIG environment variable.)\n", programBasename.c_str(), configFile.GetName());
            return SUCCESS;
            }
        }

    configFile = BeFileName(BeFileName::DevAndDir, argv0);
    configFile.AppendToPath(s_configFileName);
    configFile.BeGetFullPathName();

    if (BeFileName::DoesPathExist(configFile))
        {
        _PrintMessage(L"%ls configuring logging using %ls. Override by setting BENTLEY_BIMUPGRADER_LOGGING_CONFIG in environment.\n", programBasename.c_str(), configFile.GetName());
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BimUpgrader::InitLogging(WCharCP argv0)
    {
    BeFileName configFile;

    if (SUCCESS == GetLogConfigurationFilename(configFile, argv0))
        {
        NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, configFile);
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        }
    else
        {
        _PrintMessage(L"Logging.config.xml not found. Activating default logging using console provider.\n");
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
        NativeLogging::LoggingConfig::SetSeverity(L"Performance", NativeLogging::LOG_TRACE);
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BimUpgrader::_ParseCommandLine(WStringR errmsg, int argc, WCharCP argv[])
    {
    if (argc < 2)
        return ERROR;

    m_compress = false;
    for (int iArg = 1; iArg < argc; ++iArg)
        {
        if (argv[iArg] == wcsstr(argv[iArg], L"--input=") || argv[iArg] == wcsstr(argv[iArg], L"-i="))
            {
            BeFileName::FixPathName(m_inputFileName, GetArgValueW(argv[iArg]).c_str());
            if (BeFileName::IsDirectory(m_inputFileName.c_str()))
                return ERROR;

            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--output=") || argv[iArg] == wcsstr(argv[iArg], L"-o="))
            {
            m_outputPath.SetName(GetArgValueW(argv[iArg]));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--compress"))
            {
            m_compress = true;
            continue;
            }
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BimUpgrader::_Initialize(int argc, WCharCP argv[])
    {
    m_inputFileName.BeGetFullPathName();
    m_outputPath.BeGetFullPathName();

    if (!m_inputFileName.DoesPathExist())
        {
        PrintError(L"Cannot find input file '%ls'\n", m_inputFileName.GetName());
        return ERROR;
        }
    if (!m_outputPath.DoesPathExist())
        {
        if (m_outputPath.GetExtension().empty())    // it's probably supposed to be a directory name
            m_outputPath.AppendSeparator();
        BeFileName outputDir = m_outputPath.GetDirectoryName();
        if (!outputDir.DoesPathExist() && (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(outputDir.c_str())))
            {
            PrintError(L"Cannot create output directory <%ls>\n", outputDir.c_str());
            return ERROR;
            }
        }

    if (m_outputPath.IsDirectory())
        {
        m_outputPath.AppendToPath(m_inputFileName.GetFileNameWithoutExtension().c_str());
        m_outputPath.AppendExtension(BIM_EXT);
        }
    else
        {
        m_outputPath.OverrideNameParts(L"." BIM_EXT);
        }
    InitLogging(argv[0]);

    m_host = new BimUpgraderHost();
    m_host->SetProgressMeter(new PrintfProgressMeter());
    DgnPlatformLib::Initialize(*m_host, false);
    return SUCCESS;

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
int BimUpgrader::_PrintUsage(WCharCP programName)
    {
    WString exeName = BeFileName::GetFileNameAndExtension(programName);

    fwprintf (stderr,
L"\n\
Takes a BIM of one generation and 'teleports' the data into a new BIM of the current generation.\n\
\n\
Usage: %ls -i|--input= -o|--output= \
\n\
    --input=                (required)  The source 0601 dgndb file\n\
\n\
    --output=               (optional)  The destination directory\n\
\n", exeName.c_str());

    return 1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
int BimUpgrader::Run(int argc, WCharCP argv[])
    {
    BentleyApi::WString errmsg;
    if (SUCCESS != _ParseCommandLine(errmsg, argc, argv))
        {
        if (!errmsg.empty())
            fwprintf(stderr, L"%ls\n", errmsg.c_str());
        _PrintUsage(argv[0]);
        return 1;
        }

    if (m_outputPath.empty())
        {
        fwprintf(stderr, L"No output path specified\n");
        return _PrintUsage(argv[0]);
        }

    if (m_inputFileName.empty())
        {
        fwprintf(stderr, L"No input Json file specified\n");
        return _PrintUsage(argv[0]);
        }

    if (SUCCESS != _Initialize(argc, argv))
        {
        return _PrintUsage(argv[0]);
        }

    bool converted = BentleyB0200::Dgn::BimFromDgnDb::BimFromDgnDb::Upgrade(m_inputFileName.GetName(), m_outputPath.GetName());
    Http::HttpClient::Uninitialize();

    if (!converted)
        return 1;

    if (!m_compress)
        return 0;

    CreateIModelParams createImodelParams;
    createImodelParams.SetOverwriteExisting(true);
    BeFileName bimName(m_outputPath);
    BeFileName imodelName(bimName);
    imodelName.OverrideNameParts(L".imodel");

    BeSQLite::DbResult rc = DgnIModel::Create(imodelName, bimName, createImodelParams);

    return 0;
    }
END_BIM_FROM_DGNDB_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, wchar_t const* argv[])
    {
    BentleyB0200::Dgn::BimFromDgnDb::BimUpgrader app;
    return app.Run(argc, argv);
    }
