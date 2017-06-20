/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/src/BimTeleporter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#if defined (BENTLEY_WIN32)
#define UNICODE
#include <Windows.h>
#endif

#include <Bentley/BeTimeUtilities.h>
#include <BimTeleporter/BisJson1Importer.h>
#include <BimTeleporter/BisJson1Exporter0601.h>
#include <BimTeleporter/BimTeleporter.h>
#include "BimTeleporterInternal.h"

#include <folly/ProducerConsumerQueue.h>
#include <Bentley/BeThread.h>

USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY

#define FORMAT_LOGGABLE_MSG(msg,fmt) {                          \
                                va_list args;                   \
                                va_start (args, fmt);           \
                                msg = WPrintfString(fmt, args); \
                                va_end (args);                  \
                                }

static WCharCP s_configFileName = L"BimTeleporter.logging.config.xml";
#define BIM_EXT L"bim"

using namespace BentleyB0200::Dgn::BimTeleporter;

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
WString BimTeleporter::GetArgValueW(WCharCP arg)
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
void BimTeleporter::PrintError(WCharCP fmt, ...)
    {
    WString msg;
    FORMAT_LOGGABLE_MSG(msg, fmt)
    fwprintf(stderr, msg.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void BimTeleporter::_PrintMessage(WCharCP fmt, ...)
    {
    WString msg;
    FORMAT_LOGGABLE_MSG(msg,fmt)
    wprintf(msg.c_str());
    //GetLogger().info(msg.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BimTeleporter::GetLogConfigurationFilename(BeFileName& configFile, WCharCP argv0)
    {
    WString programBasename = BeFileName::GetFileNameWithoutExtension(argv0);

    if (SUCCESS == getEnv(configFile, L"BENTLEY_BIMTELEPORTER_LOGGING_CONFIG"))
        {
        if (configFile.DoesPathExist())
            {
            _PrintMessage(L"%ls configuring logging with %s (Set by BENTLEY_BIMTELEPORTER_LOGGING_CONFIG environment variable.)\n", programBasename.c_str(), configFile.GetName());
            return SUCCESS;
            }
        }

    configFile = BeFileName(BeFileName::DevAndDir, argv0);
    configFile.AppendToPath(s_configFileName);
    configFile.BeGetFullPathName();

    if (BeFileName::DoesPathExist(configFile))
        {
        _PrintMessage(L"%ls configuring logging using %ls. Override by setting BENTLEY_BIMTELEPORTER_LOGGING_CONFIG in environment.\n", programBasename.c_str(), configFile.GetName());
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BimTeleporter::InitLogging(WCharCP argv0)
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
BentleyStatus BimTeleporter::_ParseCommandLine(WStringR errmsg, int argc, WCharCP argv[])
    {
    if (argc < 2)
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
BentleyStatus BimTeleporter::_Initialize(int argc, WCharCP argv[])
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

    return SUCCESS;

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
int BimTeleporter::_PrintUsage(WCharCP programName)
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
int BimTeleporter::Run(int argc, WCharCP argv[])
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

    BisJson1Exporter0601 exporter(m_inputFileName.GetName());
    auto logFunc = [] (TeleporterLoggingSeverity severity, const char* message)
        {
        GetLogger().message((SEVERITY) severity, message);
        };
    exporter.SetLogger(logFunc);

    auto perfLogFunc = [] (const char* message)
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("BimTeleporter.Performance")->info(message);
        };
    exporter.SetPerformanceLogger(perfLogFunc);

    BisJson1Importer importer(m_outputPath.GetName());
    exporter.SetQueueWrite([&importer] (const char* jsonEntry)
        {
        importer.AddToQueue(jsonEntry);
        });

    StopWatch totalTimer(true);
    std::thread consumer([&importer] { importer.CreateBim(); });
    std::thread producer([&exporter] { exporter.ExportDgnDb(); });
    producer.join();
    importer.SetDone();
    consumer.join();
    totalTimer.Stop();
    Utf8PrintfString message("Total teleportation|%.0f millisecs", totalTimer.GetElapsedSeconds() * 1000.0);
    BentleyApi::NativeLogging::LoggingManager::GetLogger("BimTeleporter.Performance")->info(message.c_str());

    //if (SUCCESS == importer.ImportDatabase(jsonInput))
    //    fwprintf(stdout, L"Successfully teleported %ls into %ls\n", m_inputFileName.GetName(), m_outputPath.GetName());
    //else
    //    fwprintf(stdout, L"Failed to teleport %ls into %ls\n", m_inputFileName.GetName(), m_outputPath.GetName());

    //dgndb->Schemas().CreateClassViewsInDb(); // Failing to create the views should not cause errors for the rest of the conversion

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, wchar_t const* argv[])
    {
    BimTeleporter app;
    return app.Run(argc, argv);
    }
