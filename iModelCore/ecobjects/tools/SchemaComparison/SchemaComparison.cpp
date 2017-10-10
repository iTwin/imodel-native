/*--------------------------------------------------------------------------------------+
|
|     $Source: tools/SchemaComparison/SchemaComparison.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/BeFile.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECObjects/SchemaComparer.h>
#include <Windows.h>

#define SCHEMA_COMPARISON_EXIT_DIFFERENCES_DETECTED 1
#define SCHEMA_COMPARISON_EXIT_FAILURE -1
#define NSCHEMAS 2
#define CSTR_HR "----------------------------------------"

BentleyApi::NativeLogging::ILogger* s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger("SchemaComparison");

struct SchemaComparisonOptions
    {
    BeFileName InFileNames[NSCHEMAS];
    bvector<BeFileName> ReferenceDirectories;
    };

static void ShowUsage(const char* progName)
    {
    std::fprintf(stderr,
                 "\n%s -i <inputSchemaPath> <inputSchemaPath> [-r directories]\n%s\n%s\n%s\n%s\n%s\n",
                 progName,
                 "Tool to compare and check for differences between two ECSchemas.\nReturns the number of differences between provided ECSchemas on success, -1 on error.\n",
                 "options:",
                 " -h --help                         show this help message and exit",
                 " -i        FILE0 [FILE1 ... FILEN] specify input schemas",
                 " -r --ref  DIR0  [DIR1  ... DIRN]  other directories for reference schemas");
    }

static bool NoParameterNext(int argc, char** argv, int index)
    {
    int next = index + 1;
    if (next >= argc || argv[next][0] == '-')
        return true;
    return false;
    }

static bool TryParseInput(int argc, char** argv, SchemaComparisonOptions& options)
    {
    size_t inFileIndex = 0;
    bool allInputFilesDefined = false;
    for (int i = 1; i < argc; ++i)
        {
        if (0 == std::strcmp(argv[i], "-h") || 0 == std::strcmp(argv[i], "--help"))
            {
            return false;
            }
        else if (0 == std::strcmp(argv[i], "-i"))
            {
            if (NoParameterNext(argc, argv, i))
                return false;
            while (false == NoParameterNext(argc, argv, i))
                {
                if (allInputFilesDefined)
                    return false;
                options.InFileNames[inFileIndex].AssignUtf8(argv[++i]);
                if (options.InFileNames[inFileIndex].IsDirectory())
                    return false;
                ++inFileIndex;
                allInputFilesDefined = inFileIndex == NSCHEMAS;
                }
            }
        else if (0 == std::strcmp(argv[i], "-r") || 0 == std::strcmp(argv[i], "--ref"))
            {
            while (false == NoParameterNext(argc, argv, i))
                options.ReferenceDirectories.push_back(BeFileName(argv[++i]));
            for (auto const& refDirectory : options.ReferenceDirectories)
                {
                if (!refDirectory.Contains(L"\\"))
                    {
                    std::fputs("-r/--ref should be followed by one or more directories", stderr);
                    return false;
                    }
                }
            }
        else
            {
            std::fprintf(stderr, "Unknown option %s\n", argv[i]);
            return false;
            }
        }

    return allInputFilesDefined;
    }

int CompareSchemas(SchemaComparisonOptions& options)
    {
    ECN::ECSchemaReadContextPtr contexts[NSCHEMAS];
    ECN::ECSchemaPtr schemas[NSCHEMAS];
    for (int i = 0; i < NSCHEMAS; ++i)
        {
        contexts[i] = ECN::ECSchemaReadContext::CreateContext(true, true);
        contexts[i]->SetPreserveElementOrder(true);
        contexts[i]->SetPreserveXmlComments(false);

        for (auto const& refDir : options.ReferenceDirectories)
            {
            contexts[i]->AddSchemaPath(refDir.GetName());
            }
        contexts[i]->AddSchemaPath(options.InFileNames[i].GetDirectoryName().GetName());

        Utf8String fullname(options.InFileNames[i].GetFileNameAndExtension());
        ECN::SchemaKey key;
        ECN::SchemaKey::ParseSchemaFullName(key, fullname.c_str());

        schemas[i] = contexts[i]->LocateSchema(key, ECN::SchemaMatchType::Latest);
        if (!schemas[i].IsValid())
            {
            Utf8String err("Failed to read schema ");
            err += key.GetName();
            s_logger->fatal(err.c_str());
            return SCHEMA_COMPARISON_EXIT_FAILURE;
            }
        s_logger->infov("Located schema: %s\n", key.GetName().c_str());
        }

    bvector<ECN::ECSchemaCP> cpSchemas[NSCHEMAS] = {{schemas[0].get()}, {schemas[1].get()}};
    ECN::SchemaComparer comparer;
    ECN::SchemaChanges changes;
    ECN::SchemaComparer::Options comparerOptions(ECN::SchemaComparer::AppendDetailLevel::Partial, ECN::SchemaComparer::AppendDetailLevel::Partial);
    if (ERROR == comparer.Compare(changes, cpSchemas[0], cpSchemas[1], comparerOptions))
        {
        s_logger->error("Failed to compare schemas");
        return SCHEMA_COMPARISON_EXIT_FAILURE;
        }
    if (changes.IsEmpty())
        {
        s_logger->info("The schemas are identical");
        return SUCCESS;
        }
    for (size_t i = 0; i < changes.Count(); ++i)
        {
        Utf8String s;
        changes.WriteToString(s);
        s_logger->error(s.c_str());
        }
    std::printf("The schemas are different (see log)");
    return SCHEMA_COMPARISON_EXIT_DIFFERENCES_DETECTED;
    }

int main(int argc, char** argv)
    {
    SchemaComparisonOptions progOptions;

    if (!TryParseInput(argc, argv, progOptions))
        {
        ShowUsage(argv[0]);
        return SCHEMA_COMPARISON_EXIT_FAILURE;
        }

    WChar exePathW[MAX_PATH];
    if (0 == ::GetModuleFileNameW(nullptr, exePathW, MAX_PATH))
        {
        return SCHEMA_COMPARISON_EXIT_FAILURE;
        }
    BeFileName exePath(exePathW);
    BeFileName workingDirectory(exePath.GetDirectoryName());

    BeFileName logFilePath(workingDirectory);
    logFilePath.AppendToPath(L"SchemaComparison.logging.config.xml");
    logFilePath.BeGetFullPathName();
    BentleyApi::NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, logFilePath);
    BentleyApi::NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
    workingDirectory.AppendToPath(L"Assets");

    ECN::ECSchemaReadContext::Initialize(workingDirectory);
    s_logger->infov(L"Initializing ECSchemaReadContext to '%ls'", workingDirectory);

    return CompareSchemas(progOptions);
    }
