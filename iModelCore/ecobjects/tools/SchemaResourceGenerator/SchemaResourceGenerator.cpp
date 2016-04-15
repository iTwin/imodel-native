/*--------------------------------------------------------------------------------------+
|
|     $Source: tools/SchemaResourceGenerator/SchemaResourceGenerator.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECObjects/ECObjectsAPI.h>
#include <Logging/bentleylogging.h>
#if defined (BENTLEY_WIN32)
// WIP_BEFILENAME_PORTABILITY - need better way of setting temp directory
#include <windows.h>
#endif

USING_NAMESPACE_BENTLEY_EC

BentleyApi::NativeLogging::ILogger* s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger("SchemaConverter");

struct Options
    {
    bvector<BeFileName> InputFiles;
    Utf8String CultureName;
    BeFileName OutDirectory;
    bvector<BeFileName> ReferencePaths;
    Utf8String OutputFileName;
    int Precendence = 9900;
    bool NoSupplementation = false;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static void ShowUsage()
    {
    const char *usage =
#include "usage.txt"
        ;

    printf(usage);
    }

enum class InputParserState
    {
    InputFiles,
    Precedence,
    CultureName,
    ReferencePaths,
    OutDirectory,
    OutFile,
    NoSupplementation,
    Help,
    ExpectingSwitch
    };

static bmap<Utf8String, InputParserState> CreateSwitchMap()
    {
    bmap<Utf8String, InputParserState> m;
    m["/?"] = InputParserState::Help;
    m["-p"] = InputParserState::Precedence;
    m["-c"] = InputParserState::CultureName;
    m["-r"] = InputParserState::ReferencePaths;
    m["-o"] = InputParserState::OutDirectory;
    m["-of"] = InputParserState::OutFile;
    m["-ns"] = InputParserState::NoSupplementation;
    return m;
    };

static bmap<Utf8String, InputParserState> const ConsoleSwitches = CreateSwitchMap();

static bool TryParseInput(int argc, char** argv, Options& options)
    {
    if (argc < 2)
        {
        ShowUsage();
        return false;
        }

    InputParserState currentState = InputParserState::InputFiles;
    bset<InputParserState> usedSwitches;
    for (int i = 1; i < argc; i++)
        {
        Utf8CP arg = argv[i];
        InputParserState currentSwitch = InputParserState::InputFiles;
        auto const csw = ConsoleSwitches.find(arg);
        bool isSwitch = csw != ConsoleSwitches.end();

        if (isSwitch)
            {
            currentSwitch = csw->second;
            if (usedSwitches.find(currentSwitch) != usedSwitches.end())
                {
                s_logger->errorv("Switch %s is specified multiple times.", arg);
                return false;
                }

            usedSwitches.insert(currentSwitch);
            }

        switch (currentState)
            {
                case InputParserState::InputFiles: //still reading input files
                    if (!isSwitch)
                        {
                        s_logger->infov("Input File #%d: %s", i, arg);
                        BeFileName file;
                        file.AssignUtf8(arg);
                        options.InputFiles.push_back(file);
                        continue;
                        }

                    if (options.InputFiles.size() <= 0)
                        {
                        s_logger->error("No input files specified.");
                        return false;
                        }
                    break;

                case InputParserState::CultureName:
                    options.CultureName = arg;
                    s_logger->infov("Culture: %s", arg);
                    currentState = InputParserState::ExpectingSwitch;
                    continue;

                case InputParserState::OutDirectory:
                    options.OutDirectory = BeFileName(arg, true);
                    s_logger->infov("Output Directory: %s", arg);
                    currentState = InputParserState::ExpectingSwitch;
                    continue;

                case InputParserState::ReferencePaths:
                    if (!isSwitch)
                        {
                        s_logger->infov("Reference Path #%d: %s", options.ReferencePaths.size(), arg);
                        BeFileName file;
                        file.AssignUtf8(arg);
                        options.ReferencePaths.push_back(file);
                        continue;
                        }

                    if (options.ReferencePaths.size() <= 0)
                        {
                        s_logger->error("No reference paths specified.");
                        return false;
                        }
                    break;

                case InputParserState::OutFile:
                    options.OutputFileName = arg;
                    s_logger->infov("Output File: %s", arg);
                    currentState = InputParserState::ExpectingSwitch;
                    continue;

                case InputParserState::Precedence:
                    Utf8P end = nullptr;
                    uint32_t p = strtoul(arg, &end, 10);
                    if (end == arg)
                        {
                        s_logger->errorv("Invalid Precedence. '%s' is not a number.", arg);
                        return false;
                        }

                    if (p < 9900 || p > 9999)
                        {
                        s_logger->errorv("Invalid Precedence: %d. Value must be between 9900 and 9999.", p);
                        return false;
                        }

                    options.Precendence = p;
                    s_logger->infov("Precedence: %d", p);
                    currentState = InputParserState::ExpectingSwitch;
                    continue;
            }

        if (!isSwitch)
            {
            s_logger->errorv("Unrecognized parameter: %s", arg);
            return false;
            }

        if (currentSwitch == InputParserState::NoSupplementation)
            {
            options.NoSupplementation = true;
            s_logger->info("No supplementation.");
            currentState = InputParserState::ExpectingSwitch;
            continue;
            }

        if (currentSwitch == InputParserState::Help)
            {
            ShowUsage();
            return false;
            }

        currentState = currentSwitch;
        }

    if (currentState == InputParserState::InputFiles || currentState == InputParserState::ExpectingSwitch)
        return true;

    if (currentState == InputParserState::ReferencePaths && options.ReferencePaths.size() > 0)
        {
        return true;
        }

    s_logger->error("Invalid call. The last console switch requires a following parameter.");
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
int main(int argc, char** argv)
    {
#if defined(BENTLEY_WIN32)
    WChar exePathW[MAX_PATH];
    if (0 == ::GetModuleFileNameW(nullptr, exePathW, MAX_PATH))
        {
        fprintf(stderr, "Could not load logging config file");
        return -1;
        }

    BeFileName exePath(exePathW);
    BeFileName exeDirectory(exePath.GetDirectoryName());
    BeFileName logFilePath(exeDirectory);
    logFilePath.AppendToPath(L"SchemaConverter.logging.config.xml");
    logFilePath.BeGetFullPathName();
    BentleyApi::NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, logFilePath);
    BentleyApi::NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
#endif

    Options options;
    if (!TryParseInput(argc, argv, options))
        {
        return -1;
        }

    for (auto& inputFile : options.InputFiles)
        {
        s_logger->info(inputFile.GetNameUtf8().c_str());
        }
    //ECSchemaReadContext::Initialize(workingDirectory);


    return 0;
    }



