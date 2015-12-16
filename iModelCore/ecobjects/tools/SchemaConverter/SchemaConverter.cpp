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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <wchar.h>

USING_NAMESPACE_BENTLEY_EC

namespace {
	BentleyApi::NativeLogging::ILogger* s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger("SchemaConverter");
	//---------------------------------------------------------------------------------------
	// @bsimethod                                                   BentleySystems
	//---------------------------------------------------------------------------------------
	static void ShowUsage(char* str)
		{
		fprintf(stderr, "\n%s -i <inputSchemaPath> -o <outputDirectory> [-V VERSION] [-f]\n\n%s\n\n%s\n\n%s\t%s\n%s\t\t%s\n\n",
			str, "Tool to convert between different versions of ECSchema(s)", "options:",
			" -V --version 2|3", "the schema will be converted to the specified version",
			" -f --overwrite", "overwrites the xml file after conversion");
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                                   BentleySystems
	//---------------------------------------------------------------------------------------
	static int ConvertSchema(BeFileNameCR ecSchemaFile, BeFileNameCR outputFile, int version)
		{		
		ECSchemaReadContextPtr contextPtr = ECSchemaReadContext::CreateContext();
		contextPtr->AddSchemaPath(ecSchemaFile.GetDirectoryName().GetName());

		ECSchemaPtr schema;
		SchemaReadStatus readSchemaStatus = ECSchema::ReadFromXmlFile(schema, ecSchemaFile.GetName(), *contextPtr);
		if (SchemaReadStatus::Success != readSchemaStatus)
			return (int)readSchemaStatus;

		s_logger->infov(L"Saving ECv3 version of the schema in directory '%ls'", outputFile.GetDirectoryName());
		return (int)schema->WriteToXmlFile(outputFile.GetName(), version, 0);
		}
}
//BentleyApi::NativeLogging::ILogger* s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger ("SchemaConverter");
//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
int main(int argc, char** argv)
	{
	char* input;
	char* output;
	int version = 3;
	int flag = 0;
	int check = 0;
	if (argc < 5)
		{
		ShowUsage(argv[0]);
		return -1;
		}
	for (int i = 1; i < argc; i++)
		{
		if (strcmp(argv[i], "-i") == 0)
			{
			if ((argv[i + 1])[0] == '-')
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
			if ((argv[i + 1])[0] == '-')
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
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			{
			ShowUsage(argv[0]);
			return -1;
			}
		else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0)
			{
			if ((argv[i + 1])[0] == '-' || !(atof(argv[i + 1]) - 2 == 0 || atof(argv[i + 1]) - 3 == 0))
				{
				fprintf(stderr, " -V should be follwed by either the version 2.0(2) or 3.0(3)");
				return -1;
				}
			else
				version = atoi(argv[++i]);
			}
		else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--overwrite") == 0)
			check = 1;
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

	BeFileName inputFileName;
	inputFileName.AssignUtf8(input);
	BeFileName outputDirectory;
	outputDirectory.AssignUtf8(output);
	BeFileName outputFile(nullptr, outputDirectory.GetName(), inputFileName.GetFileNameAndExtension().c_str(), nullptr);
	if (wcscmp(inputFileName.GetDirectoryName(), outputFile.GetDirectoryName()) == 0 && check == 0)
		{
		fprintf(stderr, "Warning: Can't overwrite the file '%ls'.",
				inputFileName.GetFileNameAndExtension().c_str());
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
   
    s_logger->infov(L"Loading schema '%ls' for conversion to ECv3", inputFileName);
    return ConvertSchema(inputFileName, outputFile, version);
    }

