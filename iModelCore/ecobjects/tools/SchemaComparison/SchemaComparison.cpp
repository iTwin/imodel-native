/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/BeFile.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECObjects/SchemaComparer.h>
#include <Windows.h>

#define SCHEMA_COMPARISON_EXIT_DIFFERENCES_DETECTED 1
#define SCHEMA_COMPARISON_EXIT_DIFFERENCES_CHECKSUMS_MATCH 2
#define SCHEMA_COMPARISON_EXIT_IDENTICAL_CHECKSUMS_NOT_MATCH -2
#define SCHEMA_COMPARISON_EXIT_FAILURE -1
#define NSCHEMAS 2
#define CSTR_HR "----------------------------------------"

BentleyApi::NativeLogging::ILogger* s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger("SchemaComparison");

struct ExactSearchPathSchemaFileLocater : ECN::SearchPathSchemaFileLocater
    {
    using ECN::SearchPathSchemaFileLocater::SearchPathSchemaFileLocater;
    protected:
        ECN::ECSchemaPtr _LocateSchema(ECN::SchemaKeyR key, ECN::SchemaMatchType matchType, ECN::ECSchemaReadContextR schemaContext) override;
    public:
        static ECN::SearchPathSchemaFileLocaterPtr CreateExactSearchPathSchemaFileLocater(bvector<WString> const& searchPaths, bool includeFilesWithNoVerExt = false);
    };

ECN::ECSchemaPtr ExactSearchPathSchemaFileLocater::_LocateSchema(ECN::SchemaKeyR key, ECN::SchemaMatchType matchType, ECN::ECSchemaReadContextR schemaContext)
    {
    return SearchPathSchemaFileLocater::_LocateSchema(key, ECN::SchemaMatchType::Exact, schemaContext);
    }

ECN::SearchPathSchemaFileLocaterPtr ExactSearchPathSchemaFileLocater::CreateExactSearchPathSchemaFileLocater(bvector<WString> const& searchPaths, bool includeFilesWithNoVerExt)
    {
    return new ExactSearchPathSchemaFileLocater(searchPaths, includeFilesWithNoVerExt);
    }

struct SchemaComparisonOptions
    {
    BeFileName InFileNames[NSCHEMAS];
    bvector<BeFileName> ReferenceDirectories[NSCHEMAS];
    bool NoStandardSchemas;
    bool UseStandardUnitsAndFormatsSchemas;
    };

static void ShowUsage(const char* progName)
    {
    std::fprintf(stderr,
                 "\n%s -i <inputSchemaPath> <inputSchemaPath> [-r directories]\n%s\n%s\n%s\n%s\n%s\n%s\n",
                 progName,
                 "Tool to compare and check for differences between two ECSchemas.\nReturns the number of differences between provided ECSchemas on success, -1 on error.\n",
                 "options:",
                 " -h --help                         show this help message and exit",
                 " -i        FILE0 [FILE1 ... FILEN] specify input schemas",
                 " -r --ref  DIR0  [DIR1  ... DIRN]  other directories for reference schemas",
                 " --NoStandardSchemas               do not initialize SchemaReadContext with standard schemas directory");
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
    size_t referenceSchemasIndex = 0;
    bool allInputFilesDefined = false;
    options.NoStandardSchemas = false;
    options.UseStandardUnitsAndFormatsSchemas = false;
    for (int i = 1; i < argc; ++i)
        {
        if (0 == std::strcmp(argv[i], "-h") || 0 == std::strcmp(argv[i], "--help"))
            return false;
        if (0 == std::strcmp(argv[i], "--NoStandardSchemas"))
            options.NoStandardSchemas = true;
        else if (0 == std::strcmp(argv[i], "--UseStandardUnitsAndFormatsSchemas"))
            options.UseStandardUnitsAndFormatsSchemas = true;
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
            if (referenceSchemasIndex >= NSCHEMAS)
                return false;
            while (false == NoParameterNext(argc, argv, i))
                {
                BeFileName refDir(argv[++i]);
                refDir.AppendSeparator();
                options.ReferenceDirectories[referenceSchemasIndex].push_back(refDir);
                }
            for (auto const& refDirectory : options.ReferenceDirectories[referenceSchemasIndex])
                {
                if (!refDirectory.Contains(L"\\"))
                    {
                    std::fputs("-r/--ref should be followed by one or more directories", stderr);
                    return false;
                    }
                }
            ++referenceSchemasIndex;
            }
        else
            {
            std::fprintf(stderr, "Unknown option %s\n", argv[i]);
            return false;
            }
        }

    while (referenceSchemasIndex > 0 && referenceSchemasIndex < NSCHEMAS)
        {
        options.ReferenceDirectories[referenceSchemasIndex] = options.ReferenceDirectories[referenceSchemasIndex - 1];
        ++referenceSchemasIndex;
        }

    return allInputFilesDefined;
    }

static void LogError(Utf8String message)
    {
    size_t offset = 0;
    Utf8String m;
    while ((offset = message.GetNextToken (m, "\n\r", offset)) != Utf8String::npos)
        {
        if(!Utf8String::IsNullOrEmpty(m.c_str()))
            s_logger->infov("%s", m.c_str());
        }
    }

int CompareSchemas(SchemaComparisonOptions& options, BeFileName& assetsDir)
    {
    ECN::ECSchemaReadContextPtr contexts[NSCHEMAS];
    ECN::ECSchemaPtr schemas[NSCHEMAS];
    Utf8String checksums[NSCHEMAS];
    for (int i = 0; i < NSCHEMAS; ++i)
        {
        contexts[i] = ECN::ECSchemaReadContext::CreateContext(true, true);
        contexts[i]->SetPreserveElementOrder(false);
        contexts[i]->SetPreserveXmlComments(false);
        contexts[i]->SetSkipValidation(true);

        bvector<WString> searchPaths;
        for (auto const& refDir : options.ReferenceDirectories[i])
            searchPaths.push_back(WString(refDir.GetName()));
        
        ECN::SearchPathSchemaFileLocaterPtr exactSchemaLocater = ExactSearchPathSchemaFileLocater::CreateExactSearchPathSchemaFileLocater(searchPaths, true);
        contexts[i]->AddSchemaLocater(*exactSchemaLocater);

        if (options.UseStandardUnitsAndFormatsSchemas)
            {
            static ECN::SchemaKey units("Units", 1, 0, 0);
            static ECN::SchemaKey formats("Formats", 1, 0, 0);

            ECN::ECSchemaPtr unitsSchema = contexts[i]->LocateSchema(units, ECN::SchemaMatchType::LatestWriteCompatible);
            if (!unitsSchema.IsValid())
                {
                s_logger->errorv("Failed to load standard 'Units' schema.");
                return SCHEMA_COMPARISON_EXIT_FAILURE;
                }

            ECN::ECSchemaPtr formatsSchema = contexts[i]->LocateSchema(formats, ECN::SchemaMatchType::LatestWriteCompatible);
            if (!formatsSchema.IsValid())
                {
                s_logger->errorv("Failed to load standard 'Formats' schema.");
                return SCHEMA_COMPARISON_EXIT_FAILURE;
                }
            }

        ECN::SchemaReadStatus status = ECN::ECSchema::ReadFromXmlFile(schemas[i], options.InFileNames[i].c_str(), *contexts[i]);
        if (status != ECN::SchemaReadStatus::Success || !schemas[i].IsValid())
            {
            Utf8String err("Failed to read schema ");
            err += Utf8CP(options.InFileNames[i].GetFileNameWithoutExtension().c_str());
            s_logger->fatal(err.c_str());
            return SCHEMA_COMPARISON_EXIT_FAILURE;
            }
        s_logger->infov("Located schema: %s\n", schemas[i]->GetName().c_str());

        checksums[i] = schemas[i]->ComputeCheckSum();

        s_logger->infov("Checksum for %s = %s\n", schemas[i]->GetFullSchemaName().c_str(), checksums[i].c_str());
        }

    bvector<ECN::ECSchemaCP> cpSchemas[NSCHEMAS] = {{schemas[0].get()}, {schemas[1].get()}};
    ECN::SchemaComparer comparer;
    ECN::SchemaDiff diff;
    ECN::SchemaComparer::Options comparerOptions(ECN::SchemaComparer::DetailLevel::Full, ECN::SchemaComparer::DetailLevel::Full);
    if (ERROR == comparer.Compare(diff, cpSchemas[0], cpSchemas[1], comparerOptions))
        {
        s_logger->error("Failed to compare schemas");
        return SCHEMA_COMPARISON_EXIT_FAILURE;
        }
    if (diff.Changes().IsEmpty())
        {
        s_logger->info("The schemas are identical");

        if (Utf8String::IsNullOrEmpty(checksums[0].c_str()) || Utf8String::IsNullOrEmpty(checksums[1].c_str()))
            {
            s_logger->error("Checksum computation for one or both of the schemas failed because of a serialization error. See errors above for more info.");
            return SCHEMA_COMPARISON_EXIT_FAILURE;
            }

        if (checksums[0].EqualsIAscii(checksums[1]))
            return SUCCESS;
        else
            {
            s_logger->error("The checksums do not match");
            return SCHEMA_COMPARISON_EXIT_IDENTICAL_CHECKSUMS_NOT_MATCH;
            }
        }
    for (size_t i = 0; i < diff.Changes().Count(); ++i)
        {
        Utf8String s;
        diff.Changes()[i].WriteToString(s);
        LogError(s);
        }
    s_logger->info("The schemas are different (see log)");

    if (Utf8String::IsNullOrEmpty(checksums[0].c_str()) || Utf8String::IsNullOrEmpty(checksums[1].c_str()))
        {
        s_logger->error("Checksum computation for one or both of the schemas failed because of a serialization error. See errors above for more info.");
        return SCHEMA_COMPARISON_EXIT_FAILURE;
        }

    if (checksums[0].EqualsIAscii(checksums[1]))
        {
        s_logger->error("The checksums should not match");
        return SCHEMA_COMPARISON_EXIT_DIFFERENCES_CHECKSUMS_MATCH;
        }

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
        return SCHEMA_COMPARISON_EXIT_FAILURE;

    BeFileName exePath(exePathW);
    BeFileName workingDirectory(exePath.GetDirectoryName());

    BeFileName logFilePath(workingDirectory);
    logFilePath.AppendToPath(L"SchemaComparison.logging.config.xml");
    logFilePath.BeGetFullPathName();
    BentleyApi::NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, logFilePath);
    BentleyApi::NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
    workingDirectory.AppendToPath(L"Assets");

    if (progOptions.NoStandardSchemas)
        s_logger->infov(L"Skipping initialization of ECSchemaReadContext");
    else
        {
        ECN::ECSchemaReadContext::Initialize(workingDirectory);
        s_logger->infov(L"Initializing ECSchemaReadContext to '%ls'", workingDirectory.c_str());    
        }

    int comparisonResult = CompareSchemas(progOptions, workingDirectory);

    return comparisonResult;
    }
