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
        str, "Tool to convert between different versions of ECSchema(s)", "options:",
        " -x --xml 2|3", "convert to the specified exmlversion",
        " -d --dir DIR", "other directories for reference schemas",
        " -c --conversion DIR", "looks for the conversionschema file in this directory",
        " -a --all", "convert the entire schema graph",
        " -s --sup", "convert all the supplemental schemas",
        " -v --ver XX.XX", "specify the schema version");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static void GetOutputFile
(
BeFileName &outputFile,
ECSchemaPtr schema, 
bpair<uint32_t, uint32_t> versions, 
bool versionProvided,
BeFileNameCR outputDirectory
)
    {
    WString schemaName;
    if (versionProvided)
        {
        schema->SetVersionMajor(versions.first);
        schema->SetVersionMinor(versions.second);
        }
    schemaName.AssignUtf8(schema->GetFullSchemaName().c_str());
    schemaName += L".ecschema.xml";
    BeFileName file(nullptr, outputDirectory.GetName(), schemaName.c_str(), nullptr);
    outputFile = file;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static int ConvertSupplementalSchemas
(
Utf8StringCR schemaName,
WStringCR path,
ECSchemaReadContextPtr context,
BeFileNameCR outputDirectory, 
int exmlversion
)
    {
    bvector<ECSchemaP> supplementalSchemas;
    BeFileName schemaPath(path.c_str());
    WString filter;
    filter.AssignUtf8(schemaName.c_str());
    filter += L"_Supplemental_*.*.*.ecschema.xml";
    schemaPath.AppendToPath(filter.c_str());
    BeFileListIterator fileList(schemaPath.GetName(), false);
    BeFileName filePath;
    while (SUCCESS == fileList.GetNextFileName(filePath))
        {
        WCharCP     fileName = filePath.GetName();
        ECSchemaPtr schema = NULL;

        if (SchemaReadStatus::Success != ECSchema::ReadFromXmlFile(schema, fileName, *context))
            continue;
        supplementalSchemas.push_back(schema.get());
        }
    for (auto schema : supplementalSchemas)
        {
        BeFileName outputFile;
        GetOutputFile(outputFile, schema, bpair<uint32_t, uint32_t>(0, 0), false, outputDirectory);
        s_logger->infov(L"Saving the supplemental schema '%ls' in '%ls'", outputFile.GetFileNameAndExtension(), outputDirectory.GetName());
        if (0 != (int)schema->WriteToXmlFile(outputFile.GetName(), exmlversion, 0))
            return -1;
        }
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static int ConvertReferenceSchemas
(
ECSchemaPtr schema, 
int refVersion,  
BeFileName outputDirectory
)
    {
    for (auto ref : schema->GetReferencedSchemas())
        {
        BeFileName outputFile;
        GetOutputFile(outputFile, ref.second, bpair<uint32_t, uint32_t>(0, 0), false, outputDirectory);
        s_logger->infov(L"Saving the reference schema '%ls' in '%ls'", outputFile.GetFileNameAndExtension(), outputDirectory.GetName());
        if (0 != (int)ref.second->WriteToXmlFile(outputFile.GetName(), refVersion, 0))
            return -1;
        }
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static int ConvertSchema
(
BeFileNameCR ecSchemaFile, 
BeFileNameCR outputDirectory, 
bvector<BeFileName> referenceDirectories, 
BeFileNameCR otherDirectory, 
int exmlversion,
bpair<uint32_t, uint32_t> versions,  
bool all,  
bool supplemental, 
bool other,
bool versionProvided
)
    {        
    ECSchemaReadContextPtr contextPtr = ECSchemaReadContext::CreateContext();
    contextPtr->AddSchemaPath(ecSchemaFile.GetDirectoryName().GetName());
    for (auto dir: referenceDirectories)        
        contextPtr->AddSchemaPath(dir.GetName());
    // For the bad schemas
    if (other)
        contextPtr->AddConversionSchemaPath(otherDirectory.GetName());
    ECSchemaPtr schema;
    SchemaReadStatus readSchemaStatus = ECSchema::ReadFromXmlFile(schema, ecSchemaFile.GetName(), *contextPtr);
    if (SchemaReadStatus::Success != readSchemaStatus)
        return (int)readSchemaStatus;
        
    BeFileName outputFile;
    GetOutputFile(outputFile, schema, versions, versionProvided, outputDirectory);
    // Check for overwriting the file already in the directory
    if (BeStringUtilities::Wcsicmp(ecSchemaFile.GetName(), outputFile.GetName()) == 0)
        {
        s_logger->infov(L"Warning: Can't overwrite the file '%ls'.", ecSchemaFile.GetName());
        s_logger->infov(L"Process terminated!!!");
        return -1;
        }
        
    //Convert the supplemental schemas
    if (supplemental || all)
        {
        bvector<BeFileName> paths;
        paths.push_back(ecSchemaFile.GetDirectoryName());
        for (auto directory: referenceDirectories)
            paths.push_back(directory);
        for (auto path: paths)
            {
            if(0 != ConvertSupplementalSchemas(schema->GetName(), path.GetName(), contextPtr, outputDirectory, exmlversion))
                return -1;
            }
        }
        
    //Convert the reference schema according to the version specified
    if (all)
        {
        if (0 != ConvertReferenceSchemas(schema, exmlversion, outputDirectory))
            return -1;
        }

    s_logger->infov(L"Saving converted version schema '%ls' in directory '%ls'", outputFile.GetFileNameAndExtension(), outputDirectory.GetName());
    return (int)schema->WriteToXmlFile(outputFile.GetName(), exmlversion, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
int main(int argc, char** argv)
    {
    char* input;
    char* output;
    char* other;
    bvector<char*> directories;
    int version = 3;
    int flag = 0;
    bpair<uint32_t, uint32_t> versions(0,0);
    bool supplementalSchemas = false;
    bool otherDir = false;
    bool all = false;
    bool versionProvided = false;
    // automatic switching for command-line parsing
    if (argc < 5)
        {
        ShowUsage(argv[0]);
        return -1;
        }
    for (int i = 1; i < argc; i++)
        {
        if (strcmp(argv[i], "-i") == 0)
            {
            if ((argv[i + 1])[0] == '-' || i+1 == argc)
                {
                ShowUsage(argv[0]);
                return -1;
                }
            else
                {
                input = argv[++i];
                flag++;
                }
            }
        else if (strcmp(argv[i], "-o") == 0)
            {
            if ((argv[i + 1])[0] == '-' || i+1 == argc)
                {
                ShowUsage(argv[0]);
                return -1;
                }
            else
                {
                output = argv[++i];
                flag++;
                }
            }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--ver") == 0)
            {
            if (i + 1 == argc)
                {
                ShowUsage(argv[0]);
                return -1;
                }
            else if ((argv[i + 1])[0] == '-')
                {
                fprintf(stderr, "-v/--ver should be followed by the version in format XX.XX\n");
                return -1;
                }
            else
                {
                bool check = false;
                for (size_t j = 0; j<strlen(argv[i + 1]); j++)
                    {
                    if ((argv[i + 1])[j] == '.')
                        check = true;
                    }
                if (!check)
                    {
                    fprintf(stderr, "-v/--ver should be followed by the version in format XX.XX\n");
                    return -1;
                    }
                double ver = atof(argv[++i]);
                versionProvided = true;
                versions = bpair<uint32_t, uint32_t>(((int)(ver * 100)) / 100, ((int)(ver * 100)) % 100);
                }
            }
        else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0)
            all = true;
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--sup") == 0)
            supplementalSchemas = true;
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            {
            ShowUsage(argv[0]);
            return -1;
            }
        else if (strcmp(argv[i], "--conversion") == 0 || strcmp(argv[i], "-c") == 0)
            {
            bool check = false;
            otherDir = true;
            //check whether the argument follows a path format
            if (i + 1 == argc)
                {
                ShowUsage(argv[0]);
                return -1;
                }
            for (size_t j = 0; j<strlen(argv[i + 1]); j++)
                {
                if ((argv[i + 1])[j] == '\\')
                    check = true;
                }
            if (check)
                other = argv[++i];
            else
                {
                fprintf(stderr, "--look/-l should be followed by a directory path\n");
                return -1;
                }
            }
        else if (strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--xml") == 0)
            {
            //Allows the user to enter both real nos. and integers
            if (i + 1 == argc)
                {
                ShowUsage(argv[0]);
                return -1;
                }
            else if ((argv[i + 1])[0] == '-' || !(atof(argv[i + 1]) - 2 == 0 || atof(argv[i + 1]) - 3 == 0))
                {
                fprintf(stderr, " -x/--xml should be follwed by either the version 2.0(2) or 3.0(3)\n");
                return -1;
                }
            else
                version = atoi(argv[++i]);
            }
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0)
            {
            if (i + 1 == argc)
                {
                fprintf(stderr, "--dir/-d should be followed by directory path(s)");
                return -1;
                }
            while (i + 1 != argc)
                {
                if ((argv[i + 1])[0] != '-')
                    {
                    bool check = false;
                    //check whether the argument follows a path format
                    for (size_t j = 0; j<strlen(argv[i+1]); j++)
                        {
                        if ((argv[i+1])[j] == '\\')
                            check = true;
                        }
                    if (check)
                        directories.push_back(argv[++i]);
                    else
                        {
                        fprintf(stderr, "--dir/-d should be followed by directory paths\n");
                        return -1;
                        }
                    }
                else
                    break;
                }
            }
        else
            {
            ShowUsage(argv[0]);
            return -1;
            }
        }
    if (flag < 2)
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

    //storing input output and reference directories
    BeFileName inputFileName;
    inputFileName.AssignUtf8(input);
    BeFileName outputDirectory;
    outputDirectory.AssignUtf8(output);
    bvector<BeFileName> refDirectories;
    for (size_t i = 0; i < directories.size(); i++)
        {
        BeFileName temp;
        temp.AssignUtf8(directories[i]);
        refDirectories.push_back(temp);
        temp.Clear();
        }
    BeFileName otherDirectory;
    if (otherDir)
        otherDirectory.AppendUtf8(other);
    s_logger->infov(L"Loading schema '%ls' for conversion to teh specified version", inputFileName.GetName());
    return ConvertSchema(inputFileName, outputDirectory, refDirectories, otherDirectory, version, versions, all, supplementalSchemas, otherDir, versionProvided);
    }

