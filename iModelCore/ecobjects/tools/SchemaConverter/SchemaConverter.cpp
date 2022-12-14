/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/BeFileName.h>
#include <Bentley/bvector.h>
#include <Bentley/bpair.h>
#include <BeXml/BeXml.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeFileListIterator.h>
#include <Bentley/Logging.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

USING_NAMESPACE_BENTLEY_EC


BentleyApi::NativeLogging::ILogger* s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger("SchemaConverter");

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void ShowUsage(char* str)
    {
    fprintf(stderr, "\n%s -i <inputSchemaPath> -o <outputDirectory> [-x exmlVersion] [-r directories] [-c directory] [-a] [-s] [-u] [-v version]\n\n%s\n\n%s\n\n%s\t\t%s\n%s\t%s\n%s\t%s\n%s\t\t%s\n%s\t\t%s\n%s\t\t%s\n%s\t\t%s\n%s\t\t%s\n\n%s\t%s\n\t%s\n\n",
            str, "Tool to convert ECSchemas between different versions of ECXml", "options:",
            " -x --xml 2|3|3.[x]", "convert to the specified ecxmlversion.  3 defaults to 3.2, use 3.[x] to specify a specific version of EC 3 to target.  e.g. 3.1.",
            " -r --ref DIR0 [DIR1 ... DIRN]", "other directories for reference schemas",
            " -c --conversion DIR", "looks for the conversion schema file in this directory",
            " -u --include", "include the standard schemas in the converted schemas",
            " -a --all", "convert the entire schema graph",
            " -s --sup", "convert all the supplemental schemas",
            " -v --ver XX.XX.XX / XX.XX", "specify the schema version",
            " --removeUnusedReferences", "remove all schema references that aren't referenced by elements of a schema",
            "Notes:",
            "if the input path is a directory, all files matching '*.ecschema.xml' will be converted",
            "if output directory is the same as the input directory, the files will be overwritten");
    }

struct ConversionOptions
    {
    BeFileName          InputFile;
    bvector<BeFileName> ReferenceDirectories;

    BeFileName          OutputDirectory;

    bool                HasConversionDirectory = false;
    BeFileName          ConversionDirectory;

    int                 TargetECXmlVersionMajor = 3;
    int                 TargetECXmlVersionMinor = 2;

    bool                IncludeSupplementals = false;
    bool                IncludeAll = false;
    bool                IncludeStandards = false;
    bool                RemoveUnusedSchemaReferences = false;

    bool                ChangeVersion = false;
    uint32_t            MajorVersion = 0;
    uint32_t            WriteVersion = 0;
    uint32_t            MinorVersion = 0;
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void GetOutputFile
(
    BeFileName          &outputFile,
    ECSchemaR          schema,
    ConversionOptions   options
)
    {
    WString schemaName;
    if (options.ChangeVersion)
        {
        schema.SetVersionRead(options.MajorVersion);
        schema.SetVersionWrite(options.WriteVersion);
        schema.SetVersionMinor(options.MinorVersion);
        }
    if (options.TargetECXmlVersionMajor == 2)
        schemaName.AssignUtf8(schema.GetLegacyFullSchemaName().c_str());
    else if (options.TargetECXmlVersionMajor == 3)
        schemaName.AssignUtf8(schema.GetFullSchemaName().c_str());
    schemaName += L".ecschema.xml";
    BeFileName file(nullptr, options.OutputDirectory.GetName(), schemaName.c_str(), nullptr);
    outputFile = file;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
static bool TryWriteSchema(ECSchemaR schema, ConversionOptions options)
    {
    BeFileName outputFile;
    GetOutputFile(outputFile, schema, options);

    s_logger->infov(L"Saving converted version schema '%ls' in directory '%ls'", outputFile.GetFileNameAndExtension().c_str(), options.OutputDirectory.GetName());
    ECVersion version;
    if (ECObjectsStatus::Success != ECSchema::CreateECVersion(version, options.TargetECXmlVersionMajor, options.TargetECXmlVersionMinor))
        return false;

    SchemaWriteStatus status = schema.WriteToXmlFile(outputFile.GetName(), version);
    if (status != SchemaWriteStatus::Success)
        {
        s_logger->errorv("Failed to save '%s' as ECXml version '%d'.'%d'", schema.GetName().c_str(), options.TargetECXmlVersionMajor, options.TargetECXmlVersionMinor);
        return false;
        }
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
static int ConvertLoadedSchema(ECSchemaReadContextR context, ECSchemaR schema, ConversionOptions options)
    {
    if (!ECSchemaConverter::Convert(schema, &context))
        return -1;

    if (options.RemoveUnusedSchemaReferences)
        schema.RemoveUnusedSchemaReferences();

    if (!TryWriteSchema(schema, options))
        return -1;

    if (options.IncludeSupplementals || options.IncludeAll)
        {
        bvector<ECSchemaP> supplementalSchemas;
        context.GetCache().GetSupplementalSchemasFor(schema.GetName().c_str(), supplementalSchemas);
        for (auto const& supSchema : supplementalSchemas)
            {
            if (!TryWriteSchema(*supSchema, options))
                return -1;
            }
        }

    if (options.IncludeAll)
        {
        for (auto const& refSchema : schema.GetReferencedSchemas())
            {
            if (refSchema.second->IsStandardSchema() && !options.IncludeStandards)
                continue;
            s_logger->infov(L"Saving the reference schema '%ls' in '%ls'", refSchema.second->GetFullSchemaName().c_str(), options.OutputDirectory.GetName());
            if (0 != ConvertLoadedSchema(context, *refSchema.second, options))
                return -1;
            }
        }

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
static int ConvertSchema(ConversionOptions& options, BeFileName& inputFile, ECSchemaReadContextR context)
    {
    s_logger->infov(L"Reading schema '%ls'", inputFile.GetName());
    ECSchemaPtr schema;

    Utf8String fullName(inputFile.GetFileNameAndExtension());
    SchemaKey key;
    SchemaKey::ParseSchemaFullName(key, fullName.c_str());
    schema = context.LocateSchema(key, SchemaMatchType::Exact);

    if (!schema.IsValid())
        {
        s_logger->errorv(L"Failed to read schema '%ls'", inputFile.GetName());
        return -1;
        }

    return ConvertLoadedSchema(context, *schema, options);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static int ConvertSchema
(
    ConversionOptions options
)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext(true, true);

    if (options.TargetECXmlVersionMajor == 3)
        context->SetPreserveElementOrder(true);

    for (auto const& refDir : options.ReferenceDirectories)
        context->AddSchemaPath(refDir.GetName());

    if (options.HasConversionDirectory)
        context->AddConversionSchemaPath(options.ConversionDirectory.GetName());

    if (options.InputFile.IsDirectory())
        {
        context->AddSchemaPath(options.InputFile.GetName());
        BeFileName schemaPath(options.InputFile);
        schemaPath.AppendToPath(L"*.ecschema.xml");
        BeFileListIterator fileList(schemaPath.GetName(), false);
        BeFileName filePath;
        while (SUCCESS == fileList.GetNextFileName(filePath))
            {
            ConvertSchema(options, filePath, *context);
            }
        }
    else
        {
        context->AddSchemaPath(options.InputFile.GetDirectoryName().GetName());
        return ConvertSchema(options, options.InputFile, *context);
        }
    return 0;
    }

static bool NoParameterNext(int argc, char** argv, int index)
    {
    int next = index + 1;
    if (next >= argc || argv[next][0] == '-')
        return true;
    return false;
    }

static bool TryParseInput(int argc, char** argv, ConversionOptions& options)
    {
    if (argc < 5)
        return false;

    bool inputDefined = false;
    bool outputDefined = false;
    for (int i = 1; i < argc; ++i)
        {
        if (0 == strcmp(argv[i], "-i"))
            {
            if (NoParameterNext(argc, argv, i))
                return false;

            ++i;
            inputDefined = true;
            options.InputFile.AssignUtf8(argv[i]);
            }
        else if (0 == strcmp(argv[i], "-o"))
            {
            if (NoParameterNext(argc, argv, i))
                return false;

            ++i;

            if (CreateDirectory(argv[i], NULL) ||
                ERROR_ALREADY_EXISTS == GetLastError())
                {
                outputDefined = true;
                options.OutputDirectory.AssignUtf8(argv[i]);
                }

            if (!options.OutputDirectory.Contains(L"\\"))
                {
                fprintf(stderr, "-o should be followed by a directory");
                return false;
                }
            }
        else if (0 == strcmp(argv[i], "-v") || 0 == strcmp(argv[i], "--ver"))
            {
            if (NoParameterNext(argc, argv, i))
                return false;

            ++i;
            bvector<Utf8String> tokens;
            BeStringUtilities::Split((Utf8CP)argv[i], ".", tokens);
            options.ChangeVersion = true;

            options.MajorVersion = atoi(tokens[0].c_str());
            if (tokens.size() == 3)
                {
                options.WriteVersion = atoi(tokens[1].c_str());
                options.MinorVersion = atoi(tokens[2].c_str());
                }
            else if (tokens.size() == 2)
                options.MinorVersion = atoi(tokens[1].c_str());
            else
                {
                fprintf(stderr, "Incorrect format for the schema version!!");
                return false;
                }
            }
        else if (0 == strcmp(argv[i], "-a") || 0 == strcmp(argv[i], "--all"))
            options.IncludeAll = true;
        else if (0 == strcmp(argv[i], "-s") || 0 == strcmp(argv[i], "--sup"))
            options.IncludeSupplementals = true;
        else if (0 == strcmp(argv[i], "-u") || 0 == strcmp(argv[i], "--include"))
            options.IncludeStandards = true;
        else if (0 == strcmp(argv[i], "-c") || 0 == strcmp(argv[i], "--conversion"))
            {
            if (NoParameterNext(argc, argv, i))
                return false;

            ++i;
            options.HasConversionDirectory = true;
            options.ConversionDirectory.AssignUtf8(argv[i]);
            if (!options.ConversionDirectory.Contains(L"\\"))
                {
                fprintf(stderr, "-c/--conversion should be followed by a directory");
                return false;
                }
            }
        else if (0 == strcmp(argv[i], "-x") || 0 == strcmp(argv[i], "--xml"))
            {
            if (NoParameterNext(argc, argv, i))
                return false;

            ++i;
            bvector<Utf8String> tokens;
            BeStringUtilities::Split((Utf8CP)argv[i], ".", tokens);
            options.TargetECXmlVersionMajor = atoi(tokens[0].c_str());
            if (2 != options.TargetECXmlVersionMajor && 3 != options.TargetECXmlVersionMajor)
                {
                fprintf(stderr, "-x/--xml should be followed by '2', '3' (defaults to 3.2) or '3.x' where x is the minor xml version");
                return false;
                }

            if (tokens.size() >= 2)
                {
                options.TargetECXmlVersionMinor = atoi(tokens[1].c_str());
                }
            else if (2 == options.TargetECXmlVersionMajor)
                options.TargetECXmlVersionMinor = 0;
            else if (3 == options.TargetECXmlVersionMajor)
                options.TargetECXmlVersionMinor = 2;
            }
        else if (0 == strcmp(argv[i], "-r") || 0 == strcmp(argv[i], "--ref"))
            {
            while (false == NoParameterNext(argc, argv, i))
                {
                ++i;
                options.ReferenceDirectories.push_back(BeFileName(argv[i]));
                }
            for (auto const& refDirectory : options.ReferenceDirectories)
                {
                if (!refDirectory.Contains(L"\\"))
                    {
                    fprintf(stderr, "-r/--ref should be followed by one or more directories");
                    return false;
                    }
                }
            }
        else if (0 == _strcmpi(argv[i], "--removeUnusedReferences"))
            {
            options.RemoveUnusedSchemaReferences = true;
            }
        }

    return inputDefined && outputDefined;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int main(int argc, char** argv)
    {
    ConversionOptions options;

    if (!TryParseInput(argc, argv, options))
        {
        ShowUsage(argv[0]);
        return -1;
        }

    WChar exePathW[MAX_PATH];
    if (0 == ::GetModuleFileNameW(nullptr, exePathW, MAX_PATH))
        {
        fprintf(stderr, "Could not load logging config file");
        return -1;
        }

    BeFileName exePath(exePathW);
    BeFileName workingDirectory(exePath.GetDirectoryName());
    BeFileName logFilePath(workingDirectory);
    logFilePath.AppendToPath(L"SchemaConverter.logging.config.xml");
    logFilePath.BeGetFullPathName();
    BentleyApi::NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, logFilePath);
    BentleyApi::NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
    workingDirectory.AppendToPath(L"Assets");

    ECSchemaReadContext::Initialize(workingDirectory);
    s_logger->infov(L"Initializing ECSchemaReadContext to '%ls'", workingDirectory.c_str());

    s_logger->infov(L"Loading schema '%ls' for conversion to ECXml version '%d'.'%d'", options.InputFile.GetName(), options.TargetECXmlVersionMajor, options.TargetECXmlVersionMinor);
    int convertResult = ConvertSchema(options);

    return convertResult;
    }
