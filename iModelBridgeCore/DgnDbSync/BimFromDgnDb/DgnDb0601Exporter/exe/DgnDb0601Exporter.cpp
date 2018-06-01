/*--------------------------------------------------------------------------------------+
|
|     $Source: BimFromDgnDb/DgnDb0601Exporter/exe/DgnDb0601Exporter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#if defined (BENTLEY_WIN32)
#define UNICODE
#include <Windows.h>
#endif

#include <DgnDb06Api/Logging/bentleylogging.h>
#include <BimFromDgnDb/BimFromDgnDb.h>
#include <BimFromDgnDb/JsonFromDgnDb0601.h>
#include "DgnDb0601Exporter.h"

DGNDB06_USING_NAMESPACE_BENTLEY_LOGGING
DGNDB06_USING_NAMESPACE_BENTLEY

#define FORMAT_LOGGABLE_MSG(msg,fmt) {                          \
                                va_list args;                   \
                                va_start (args, fmt);           \
                                msg = WPrintfString(fmt, args); \
                                va_end (args);                  \
                                }

// This should come from bentleylogging.h, but it is unpublished, which means the VendorApi version doesn't have it.
#define CONFIG_OPTION_CONFIG_FILE           L"CONFIG_FILE"

static WCharCP s_configFileName = L"BimTeleporter.logging.config.xml";
#define JSON_EXT L"json"

#define LOG             BentleyApi::NativeLogging::LoggingManager::GetLogger("BimTeleporter")

BEGIN_BIM_EXPORTER_NAMESPACE

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
WString BimExporter0601::GetArgValueW(WCharCP arg)
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
void BimExporter0601::PrintError(WCharCP fmt, ...)
    {
    WString msg;
    FORMAT_LOGGABLE_MSG(msg, fmt)
    fwprintf(stderr, msg.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
void BimExporter0601::LogMessage(WCharCP fmt, ...)
    {
    WString msg;
    FORMAT_LOGGABLE_MSG(msg,fmt)
    LOG->info(msg.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
void BimExporter0601::PrintMessage(WCharCP fmt, ...)
    {
    WString msg;
    FORMAT_LOGGABLE_MSG(msg,fmt)
    fwprintf(stdout, msg.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BimExporter0601::GetLogConfigurationFilename(BeFileName& configFile, WCharCP argv0)
    {
    WString programBasename = BeFileName::GetFileNameWithoutExtension(argv0);

    if (SUCCESS == getEnv(configFile, L"BENTLEY_BIMTELEPORTER_LOGGING_CONFIG"))
        {
        if (configFile.DoesPathExist())
            {
            PrintMessage(L"%ls configuring logging with %s (Set by BENTLEY_BIMTELEPORTER_LOGGING_CONFIG environment variable.)\n", programBasename.c_str(), configFile.GetName());
            return SUCCESS;
            }
        }

    configFile = BeFileName(BeFileName::DevAndDir, argv0);
    configFile.AppendToPath(s_configFileName);
    configFile.BeGetFullPathName();

    if (BeFileName::DoesPathExist(configFile))
        {
        PrintMessage(L"%ls configuring logging using %ls. Override by setting BENTLEY_BIMTELEPORTER_LOGGING_CONFIG in environment.\n", programBasename.c_str(), configFile.GetName());
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BimExporter0601::InitLogging(WCharCP argv0)
    {
    BeFileName configFile;

    if (SUCCESS == GetLogConfigurationFilename(configFile, argv0))
        {
        NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, configFile);
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        }
    else
        {
        LogMessage(L"Logging.config.xml not found. Activating default logging using console provider.\n");
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
        NativeLogging::LoggingConfig::SetSeverity(L"Performance", NativeLogging::LOG_TRACE);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BimExporter0601::_ParseCommandLine(WStringR errmsg, int argc, WCharCP argv[])
    {
    if (argc < 3)
        return ERROR;

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
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BimExporter0601::_Initialize(int argc, WCharCP argv[])
    {
    m_inputFileName.BeGetFullPathName();
    m_outputPath.BeGetFullPathName();

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
        m_outputPath.AppendExtension(JSON_EXT);
        }
    else
        {
        m_outputPath.OverrideNameParts(L"." JSON_EXT);
        }
    InitLogging(argv[0]);

    return SUCCESS;

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
int BimExporter0601::_PrintUsage(WCharCP programName)
    {
    WString exeName = BeFileName::GetFileNameAndExtension(programName);

    fwprintf (stderr,
L"\n\
Takes a DgnDb and writes out BIS Json that can be used to import it as a later generation BIM file.\n\
\n\
Usage: %ls -i|--input= -o|--output= \
\n\
    --input=                (required)  The source DgnDb\n\
\n\
    --output=               (optional)  Output directory.\n\
\n", exeName.c_str());

    return 1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
int BimExporter0601::Run(int argc, WCharCP argv[])
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
        fwprintf(stderr, L"No output directory specified\n");
        return _PrintUsage(argv[0]);
        }

    if (m_inputFileName.empty())
        {
        fwprintf(stderr, L"No input BIM specified\n");
        return _PrintUsage(argv[0]);
        }

    if (!m_inputFileName.DoesPathExist())
        {
        PrintError(L"Input file '%ls' does not exist.", m_inputFileName.GetName());
        return _PrintUsage(argv[0]);
        }

    _Initialize(argc, argv);

    // use the standard Windows temporary directory
    wchar_t tempPathW[MAX_PATH];
    ::GetTempPathW(_countof(tempPathW), tempPathW);

    // the application directory is where the executable is located
    wchar_t moduleFileName[MAX_PATH];
    ::GetModuleFileNameW(NULL, moduleFileName, _countof(moduleFileName));
    BeFileName moduleDirectory(BeFileName::DevAndDir, moduleFileName);
    BeFileName executableDirectory = moduleDirectory;
    executableDirectory.AppendSeparator();

    BeFileName assetsDirectory = executableDirectory;
    assetsDirectory.AppendToPath(L"Assets");
    BentleyB0200::Dgn::BimTeleporter::DgnDb0601ToJson exporter(m_inputFileName.GetName(), tempPathW, assetsDirectory.GetName());

    LOG->infov(L"Successfully opened %ls\n", m_inputFileName.GetName());

    auto logFunc = [] (BimFromDgnDbLoggingSeverity severity, const char* message)
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("BimTeleporter")->message((SEVERITY) severity, message);
        };
    exporter.SetLogger(logFunc);

    auto perfLogFunc = [] (const char* message)
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("BimTeleporter.Performance")->info(message);
        };
    exporter.SetPerformanceLogger(perfLogFunc);

    BeFile file;
    BeFileName jsonPath(m_outputPath.GetDirectoryName());
    jsonPath.AppendString(m_inputFileName.GetFileNameWithoutExtension().c_str());
    jsonPath.AppendExtension(L"json");
    if (file.Create(jsonPath, true) != BeFileStatus::Success)
        {
        LOG->errorv(L"Failed to create JSON file %ls", jsonPath.GetName());
        return false;
        }

    exporter.SetQueueWrite([&file] (const char* jsonEntry)
        {
        file.Write(nullptr, jsonEntry, (uint32_t) strlen(jsonEntry));
        Utf8CP comma(",\n");
        file.Write(nullptr, comma, (uint32_t) strlen(comma));
        }
    );

    Utf8CP openBracket = "[\n";
    file.Write(nullptr, openBracket, (uint32_t) strlen(openBracket));
    exporter.ExportDgnDb();
    file.Flush();
    LOG->infov(L"Exported dgndb to '%ls'", jsonPath.GetName());
    Utf8CP closeBracket = "]\n";
    file.Write(nullptr, closeBracket, (uint32_t) strlen(closeBracket));

    PrintMessage(L"Successfully exported %ls to %ls\n", m_inputFileName.GetName(), m_outputPath.GetName());
    return 0;
    }
END_BIM_EXPORTER_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, wchar_t const* argv[])
    {
    BentleyG0601::Dgn::BimTeleporter::BimExporter0601 app;
    return app.Run(argc, argv);
    }
