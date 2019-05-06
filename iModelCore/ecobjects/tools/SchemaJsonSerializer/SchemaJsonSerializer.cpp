/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeFileName.h>
#include <Bentley/stdcxx/bvector.h>
#include <Bentley/stdcxx/rw/bpair.h>
#include <BeXml/BeXml.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeFileListIterator.h>
#include <Logging/bentleylogging.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

USING_NAMESPACE_BENTLEY_EC


BentleyApi::NativeLogging::ILogger* s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger("SchemaJsonSerializer");

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static void ShowUsage(char* str)
    {
    fprintf(stderr, "\n%s -i <inputSchemaPath> -o <outputSchemaPath> [-r directories] [-c directory] [-a]\n\n%s\n\n%s\n\n%s\t%s\n%s\t%s\n%s\t%s\n%s\t%s\n\n%s\t%s\n\n",
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

    BeFileName          OutputDirectory;

    bool                HasConversionDirectory = false;
    BeFileName          ConversionDirectory;

    bool                IncludeSupplementals = false;
    bool                IncludeAll = false;
    bool                IncludeStandards = false;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                              Prasanna.Prakash                       01/2016
//---------------------------------------------------------------------------------------
static void GetOutputFile(BeFile &outputFile, ECSchemaR schema, ConversionOptions options)
    {
    WString schemaName;
    schemaName.AssignUtf8(schema.GetName().c_str());
    schemaName += L".ecschema.json";
    BeFileName file(nullptr, options.OutputDirectory.GetName(), schemaName.c_str(), nullptr);

    outputFile.Create(file.c_str());

    s_logger->infov(L"Saving converted version schema '%ls' in directory '%ls'", file.GetFileNameAndExtension().c_str(), options.OutputDirectory.GetName());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      01/2016
//--------------------------------------------------------------------------------------
static bool TryWriteSchema(ECSchemaR schema, ConversionOptions options)
    {
    BeFile outputFile;
    GetOutputFile(outputFile, schema, options);

    Utf8String jsonString;
    bool status = schema.WriteToJsonString(jsonString);
    if (status != true)
        {
        s_logger->errorv("Failed to save '%s' as ECJson.", schema.GetName().c_str());
        return false;
        }

    uint32_t bytesWritten;
    outputFile.Write(&bytesWritten, jsonString.c_str(), (uint32_t)jsonString.size());

    outputFile.Close();

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      01/2016
//--------------------------------------------------------------------------------------
static int WriteLoadedSchema(ECSchemaReadContextR context, ECSchemaR schema, ConversionOptions options)
    {
    if (!TryWriteSchema(schema, options))
        return -1;

    if (options.IncludeAll)
        {
        for (auto const& refSchema : schema.GetReferencedSchemas())
            {
            if (refSchema.second->IsStandardSchema() && !options.IncludeStandards)
                continue;
            s_logger->infov(L"Saving the reference schema '%ls' in '%ls'", refSchema.second->GetFullSchemaName().c_str(), options.OutputDirectory.GetName());
            if (0 != WriteLoadedSchema(context, *refSchema.second, options))
                return -1;
            }
        }

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
static int WriteSchema(ConversionOptions& options, BeFileName& inputFile, ECSchemaReadContextR context)
    {
    s_logger->infov(L"Reading schema '%ls'", inputFile.GetName());
    ECSchemaPtr schema;

    ECSchema::ReadFromXmlFile(schema, inputFile.c_str(), context);
    if (!schema.IsValid())
        {
        s_logger->errorv(L"Failed to read schema '%ls'", inputFile.GetName());
        return -1;
        }

    return WriteLoadedSchema(context, *schema, options);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static int WriteSchema(ConversionOptions options)
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
            WriteSchema(options, filePath, *context);
            }
        }
    else
        {
        context->AddSchemaPath(options.InputFile.GetDirectoryName().GetName());
        return WriteSchema(options, options.InputFile, *context);
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

        else if (0 == strcmp(argv[i], "-u") || 0 == strcmp(argv[i], "--include"))
            options.IncludeStandards = true;
        }

    return inputDefined && outputDefined;
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
    logFilePath.AppendToPath(L"SchemaJsonSerializer.logging.config.xml");
    logFilePath.BeGetFullPathName();
    BentleyApi::NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, logFilePath);
    BentleyApi::NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
    workingDirectory.AppendToPath(L"Assets");

    ECSchemaReadContext::Initialize(workingDirectory);
    s_logger->infov(L"Initializing ECSchemaReadContext to '%ls'", workingDirectory.c_str());

    s_logger->infov(L"Loading schema '%ls' for  validation", options.InputFile.GetName());
    int validationResult = WriteSchema(options);

    return validationResult;
    }
