/*--------------------------------------------------------------------------------------+
|
|     $Source: tools/SchemaConverter/SchemaConverter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/BeFileName.h>
#include <Bentley/stdcxx/bvector.h>
#include <Bentley/stdcxx/rw/bpair.h>
#include <BeXml/BeXml.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeFIleListIterator.h>
#include <Logging/bentleylogging.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

USING_NAMESPACE_BENTLEY_EC


BentleyApi::NativeLogging::ILogger* s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger("SchemaConverter");

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static void ShowUsage(char* str)
    {
    fprintf(stderr, "\n%s -i <inputSchemaPath> -o <outputDirectory> [-x exmlVersion] [-d directories] [-c directory] [-a] [-s] [-v version]\n\n%s\n\n%s\n\n%s\t\t%s\n%s\t\t%s\n%s\t%s\n%s\t\t%s\n%s\t\t%s\n%s\t\t%s\n\n",
        str, "Tool to convert ECSchemas between different versions of ECXml", "options:",
        " -x --xml 2|3", "convert to the specified exmlversion",
        " -r --ref DIR0 [DIR1 ... DIRN]", "other directories for reference schemas",
        " -c --conversion DIR", "looks for the conversion schema file in this directory",
        " -a --all", "convert the entire schema graph",
        " -s --sup", "convert all the supplemental schemas",
        " -v --ver XX.XX", "specify the schema version");
    }

struct ConversionOptions
    {
    BeFileName          InputFile;
    bvector<BeFileName> ReferenceDirectories;

    BeFileName          OutputDirectory;

    bool                HasConversionDirectory = false;
    BeFileName          ConversionDirectory;

    int                 TargetECXmlVersion = 3;

    bool                IncludeSupplementals = false;
    bool                IncludeAll = false;

    bool                ChangeVersion = false;
    uint32_t            MajorVersion;
    uint32_t            MinorVersion;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
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
        schema.SetVersionMajor(options.MajorVersion);
        schema.SetVersionMinor(options.MinorVersion);
        }
    schemaName.AssignUtf8(schema.GetFullSchemaName().c_str());
    schemaName += L".ecschema.xml";
    BeFileName file(nullptr, options.OutputDirectory.GetName(), schemaName.c_str(), nullptr);
    outputFile = file;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      01/2016
//--------------------------------------------------------------------------------------
static bool TryWriteSchema(ECSchemaR schema, ConversionOptions options)
    {
    BeFileName outputFile;
    GetOutputFile(outputFile, schema, options);
    // Check for overwriting the file already in the directory
    if (BeStringUtilities::Wcsicmp(options.InputFile.GetName(), outputFile.GetName()) == 0)
        {
        s_logger->errorv(L"Warning: Can't overwrite the file '%ls'.", options.InputFile.c_str());
        s_logger->errorv(L"Process terminated!!!");
        return false;
        }

    s_logger->infov(L"Saving converted version schema '%ls' in directory '%ls'", outputFile.GetFileNameAndExtension(), options.OutputDirectory.GetName());
    SchemaWriteStatus status = schema.WriteToXmlFile(outputFile.GetName(), options.TargetECXmlVersion, 0);
    if (status != SchemaWriteStatus::Success)
        {
        s_logger->errorv("Failed to save '%s' as ECXml version '%d'.0", schema.GetName(), options.TargetECXmlVersion);
        return false;
        }
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      01/2016
//--------------------------------------------------------------------------------------
static int ConvertLoadedSchema(ECSchemaReadContextPtr context, ECSchemaR schema, ConversionOptions options)
    {
    if (!TryWriteSchema(schema, options))
        return -1;

    if (options.IncludeSupplementals || options.IncludeAll)
        {
        bvector<ECSchemaP> supplementalSchemas;
        context->GetCache().GetSupplementalSchemasFor(schema.GetName().c_str(), supplementalSchemas);
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
            if (refSchema.second->IsStandardSchema())
                continue;
            s_logger->infov(L"Saving the reference schema '%ls' in '%ls'", refSchema.second->GetFullSchemaName(), options.OutputDirectory.GetName());
            if (0 != ConvertLoadedSchema(context, *refSchema.second, options))
                return -1;
            }
        }
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static int ConvertSchema
(
ConversionOptions options
)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->AddSchemaPath(options.InputFile.GetDirectoryName().GetName());
    for (auto const& refDir: options.ReferenceDirectories)
        context->AddSchemaPath(refDir.GetName());

    if (options.HasConversionDirectory)
        context->AddConversionSchemaPath(options.ConversionDirectory.GetName());
    
    Utf8String schemaName;
    uint32_t versionMajor;
    uint32_t versionMinor;
    WString schemaFullName = options.InputFile.GetFileNameAndExtension();
    schemaFullName.ReplaceI(L".ecschema.xml", L"");
    ECObjectsStatus status = ECSchema::ParseSchemaFullName(schemaName, versionMajor, versionMinor, Utf8String(schemaFullName));
    if (ECObjectsStatus::Success != status)
        {
        s_logger->errorv("Could not parse schema name and version from input file name '%s'", schemaFullName);
        return -1;
        }

    s_logger->infov("Reading schema '%s'", options.InputFile.GetName());
    SchemaKey key(schemaName.c_str(), versionMajor, versionMinor);
    ECSchemaPtr schema = context->LocateSchema(key, SchemaMatchType::SCHEMAMATCHTYPE_Exact);
    if (!schema.IsValid())
        {
        s_logger->errorv("Failed to read schema '%s'", schemaFullName);
        return -1;
        }

    return ConvertLoadedSchema(context, *schema, options);
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
            outputDefined = true;
            options.OutputDirectory.AssignUtf8(argv[i]);
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
            double ver = atof(argv[i]);
            options.ChangeVersion = true;
            options.MajorVersion = ((int)(ver * 100)) / 100;
            options.MinorVersion = ((int)(ver * 100)) % 100;
            }
        else if (0 == strcmp(argv[i], "-a") || 0 == strcmp(argv[i], "--all"))
            options.IncludeAll = true;
        else if (0 == strcmp(argv[i], "-s") || 0 == strcmp(argv[i], "--sup"))
            options.IncludeSupplementals = true;
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
            options.TargetECXmlVersion = atoi(argv[i]);
            if (2 != options.TargetECXmlVersion && 3 != options.TargetECXmlVersion)
                {
                fprintf(stderr, "-x/--xml should be followed by '2' or '3'");
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
    logFilePath.AppendToPath(L"SchemaConverter.logging.config.xml");
    logFilePath.BeGetFullPathName();
    BentleyApi::NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, logFilePath);
    BentleyApi::NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
    
    ECSchemaReadContext::Initialize(workingDirectory);
    s_logger->infov(L"Initializing ECSchemaReadContext to '%ls'", workingDirectory);

    s_logger->infov(L"Loading schema '%ls' for conversion to ECXml version '%d'", options.InputFile.GetName(), options.TargetECXmlVersion);
    return ConvertSchema(options);
    }



