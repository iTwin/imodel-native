/*--------------------------------------------------------------------------------------+
|
|     $Source: tools/SchemaConverter/SchemaConverter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/BeFileName.h>
#include <Logging/bentleylogging.h>
#include <windows.h>

USING_NAMESPACE_BENTLEY_EC

namespace {

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static int ConvertSchema (BeFileNameCR ecSchemaFile, BeFileNameCR outputDirectory)
    {
    ECSchemaReadContextPtr contextPtr = ECSchemaReadContext::CreateContext();
    contextPtr->AddSchemaPath (ecSchemaFile.GetDirectoryName().GetName());

    ECSchemaPtr schema;
    SchemaReadStatus readSchemaStatus = ECSchema::ReadFromXmlFile (schema, ecSchemaFile.GetName(), *contextPtr);
    if (SchemaReadStatus::Success != readSchemaStatus)
        return (int)readSchemaStatus;
    
    BeFileName outputFile(nullptr, outputDirectory.GetName(), ecSchemaFile.GetFileNameAndExtension().c_str(), nullptr);
    return (int)schema->WriteToXmlFile(outputFile.GetName(), 3, 0);
    }

}
BentleyApi::NativeLogging::ILogger* s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger ("SchemaConverter");
//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
int main (int argc, char** argv)
    {
    if (argc != 3)
        {
        fprintf (stderr, "syntax: <inputSchemaPath> <outputDirectory>\n");
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
    
    BeFileName inputFileName;
    inputFileName.AssignUtf8(argv[1]);
    
    BeFileName outputDirectory;
    outputDirectory.AssignUtf8(argv[2]);
    
    s_logger->infov(L"Loading schema '%ls' for conversion to ECv3", inputFileName);
    s_logger->infov(L"Saving ECv3 version of the schema in directory '%ls'", outputDirectory);
    
    return ConvertSchema(inputFileName, outputDirectory);
    }

