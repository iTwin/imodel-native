/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/BeFileName.h>
#include <Bentley/bvector.h>
#include <Bentley/bpair.h>
#include <BeXml/BeXml.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeFileListIterator.h>
#include <Logging/bentleylogging.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

USING_NAMESPACE_BENTLEY_EC


BentleyApi::NativeLogging::ILogger* s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger("SchemaValidator");

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static void ShowUsage(char* str)
    {
    fprintf(stderr, "\n%s -i <inputSchemaPath> [-r directories] [-c directory] [-a]\n\n%s\n\n%s\n\n%s\t%s\n%s\t%s\n%s\t%s\n%s\t%s\n\n%s\t%s\n\n",
            str, "Tool to validate ECSchemas", "options:",
            " -r --ref DIR0 [DIR1 ... DIRN]", "other directories for reference schemas",
            " -c --conversion DIR", "looks for the conversion schema file in this directory",
            " -u --include", "include the standard schemas in the converted schemas",
            " -a --all", "validate the entire schema graph",
            "Notes:",
            "if the input path is a directory, all files matching '*.ecschema.xml' will be validated");
    }

struct ConversionOptions
    {
    BeFileName          InputFile;
    bvector<BeFileName> ReferenceDirectories;

    bool                HasConversionDirectory = false;
    BeFileName          ConversionDirectory;

    bool                IncludeSupplementals = false;
    bool                IncludeAll = false;
    bool                IncludeStandards = false;
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      01/2016
//--------------------------------------------------------------------------------------
static int ValidateLoadedSchema(ECSchemaReadContextR context, ECSchemaR schema, ConversionOptions options)
    {
    ECSchemaValidator validator;
    if (!validator.Validate(schema))
        return -1;

    if (options.IncludeAll)
        {
        for (auto const& refSchema : schema.GetReferencedSchemas())
            {
            if (refSchema.second->IsStandardSchema() && !options.IncludeStandards)
                continue;

            if (0 != ValidateLoadedSchema(context, *refSchema.second, options))
                return -1;
            }
        }

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
static int ValidateSchema(ConversionOptions& options, BeFileName& inputFile, ECSchemaReadContextR context)
    {
    s_logger->infov(L"Reading schema '%ls'", inputFile.GetName());

    ECSchemaPtr schema = ECSchema::LocateSchema(inputFile.c_str(), context);
    if (!schema.IsValid())
        {
        s_logger->errorv(L"Failed to read schema '%ls'", inputFile.GetName());
        return -1;
        }

    return ValidateLoadedSchema(context, *schema, options);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static int ValidateSchema(ConversionOptions options)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext(true, true);

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
            ValidateSchema(options, filePath, *context);
            }
        }
    else
        {
        context->AddSchemaPath(options.InputFile.GetDirectoryName().GetName());
        return ValidateSchema(options, options.InputFile, *context);
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
    if (argc < 3)
        return false;

    bool inputDefined = false;
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
        else if (0 == strcmp(argv[i], "-a") || 0 == strcmp(argv[i], "--all"))
            options.IncludeAll = true;
        }

    return inputDefined;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
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
    logFilePath.AppendToPath(L"SchemaValidator.logging.config.xml");
    logFilePath.BeGetFullPathName();
    BentleyApi::NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, logFilePath);
    BentleyApi::NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
    workingDirectory.AppendToPath(L"Assets");

    ECSchemaReadContext::Initialize(workingDirectory);
    s_logger->infov(L"Initializing ECSchemaReadContext to '%ls'", workingDirectory.c_str());

    s_logger->infov(L"Loading schema '%ls' for  validation", options.InputFile.GetName());
    int validationResult = ValidateSchema(options);

    return validationResult;
    }
