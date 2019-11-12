/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#if defined (BENTLEY_WIN32)
#define UNICODE
#include <Windows.h>
#endif

#include <DgnView/DgnViewAPI.h>
#include <DgnView/DgnViewLib.h>

#include <BimFromDgnDb/BimFromDgnDb.h>
#include <BimFromDgnDb/BimFromJson.h>
#include <Logging/bentleylogging.h>
#include "BimImporter.h"

#include <folly/futures/Future.h>
#include <folly/ProducerConsumerQueue.h>
#include <folly/BeFolly.h>

USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_EC

#define FORMAT_LOGGABLE_MSG(msg,fmt) {                          \
                                va_list args;                   \
                                va_start (args, fmt);           \
                                msg = WPrintfString(fmt, args); \
                                va_end (args);                  \
                                }

static WCharCP s_configFileName = L"BimUpgrader.logging.config.xml";
#define BIM_EXT L"bim"

BEGIN_BIM_FROM_DGNDB_NAMESPACE

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
WString BimImporter::GetArgValueW(WCharCP arg)
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
void BimImporter::PrintError(WCharCP fmt, ...)
    {
    WString msg;
    FORMAT_LOGGABLE_MSG(msg, fmt)
    fwprintf(stderr, msg.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void BimImporter::_PrintMessage(WCharCP fmt, ...)
    {
    WString msg;
    FORMAT_LOGGABLE_MSG(msg,fmt)
    wprintf(msg.c_str());
    //GetLogger().info(msg.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BimImporter::GetLogConfigurationFilename(BeFileName& configFile, WCharCP argv0)
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
void BimImporter::InitLogging(WCharCP argv0)
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
BentleyStatus BimImporter::_ParseCommandLine(WStringR errmsg, int argc, WCharCP argv[])
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
BentleyStatus BimImporter::_Initialize(int argc, WCharCP argv[])
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

    //m_host->SetProgressMeter(new PrintfProgressMeter());
    DgnPlatformLib::Initialize(*this, false);

    return SUCCESS;

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
int BimImporter::_PrintUsage(WCharCP programName)
    {
    WString exeName = BeFileName::GetFileNameAndExtension(programName);

    fwprintf (stderr,
L"\n\
Takes a BIM of one generation and 'teleports' the data into a new BIM of the current generation.\n\
\n\
Usage: %ls -i|--input= -o|--output= \
\n\
    --input=                (required)  The source json file\n\
\n\
    --output=               (optional)  The destination directory\n\
\n", exeName.c_str());

    return 1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Muhammad.Zaighum                  05/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ReadJsonInputFromFile(Json::Value& jsonInput, BeFileName jsonFilePath)
    {
    Utf8String fileContent;

    BeFile file;
    if (BeFileStatus::Success != file.Open(jsonFilePath, BeFileAccess::Read))
        return ERROR;

    uint64_t rawSize;
    if (BeFileStatus::Success != file.GetSize(rawSize) || rawSize > UINT32_MAX)
        return ERROR;

    uint32_t sizeToRead = (uint32_t) rawSize;

    uint32_t sizeRead;
    ScopedArray<Byte> scopedBuffer(sizeToRead);
    Byte* buffer = scopedBuffer.GetData();
    if (BeFileStatus::Success != file.Read(buffer, &sizeRead, sizeToRead) || sizeRead != sizeToRead)
        return ERROR;

    for (uint32_t ii = 0; ii < sizeRead; ii++)
        {
        if (buffer[ii] == '\n' || buffer[ii] == '\r')
            continue;
        fileContent.append(1, buffer[ii]);
        }

    file.Close();

    return Json::Reader::Parse(fileContent, jsonInput) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
L10N::SqlangFiles BimImporter::_SupplySqlangFiles()
    {
    BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang");
    sqlangFile.AppendToPath(L"BimFromJson_en-US.sqlang.db3");

    return L10N::SqlangFiles(sqlangFile);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
folly::Future<bool> ReadJsonFile(BeFileName inputFileName, BimFromJson* importer)
    {
    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=] ()
        {
        Json::Value jsonInput;
        ReadJsonInputFromFile(jsonInput, inputFileName);

        for (Json::Value::iterator iter = jsonInput.begin(); iter != jsonInput.end(); iter++)
            {
            Json::Value& entry = *iter;
            if (entry.isNull())
                continue;
            importer->AddToQueue(entry.ToString().c_str());
            }

        return true;
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
int BimImporter::Run(int argc, WCharCP argv[])
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

    BimFromJson importer(m_outputPath.GetName());
    if (!importer.CreateBim())
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("BimImporter")->fatal("Failed to create bim.  Aborting");
        return -1;
        }

    auto future = ReadJsonFile(m_inputFileName, &importer);
    importer.ImportJson(future);

    return 0;
    }
END_BIM_FROM_DGNDB_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, wchar_t const* argv[])
    {
    BentleyApi::Dgn::BimFromDgnDb::BimImporter app;
    return app.Run(argc, argv);
    }
