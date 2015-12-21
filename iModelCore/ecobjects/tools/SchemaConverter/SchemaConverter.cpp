/*--------------------------------------------------------------------------------------+
|
|     $Source: tools/SchemaConverter/SchemaConverter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/BeFileName.h>
#include <Bentley/stdcxx/bvector.h>
#include <Bentley/stdcxx/rw/bpair.h>
#include <BeXml/BeXml.h>
#include <Bentley/BeStringUtilities.h>
#include <Logging/bentleylogging.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

USING_NAMESPACE_BENTLEY_EC

namespace {
	BentleyApi::NativeLogging::ILogger* s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger("SchemaConverter");
	//---------------------------------------------------------------------------------------
	// @bsimethod                                                   BentleySystems
	//---------------------------------------------------------------------------------------
	static void ShowUsage(char* str)
		{
		fprintf(stderr, "\n%s -i <inputSchemaPath> -o <outputDirectory> [-V VERSION] [-d DIRECTORIES] [-a] [-r VERSION]\n\n%s\n\n%s\n\n%s\t%s\n%s\t%s\n%s\t%s\n%s\t%s\n\n",
			str, "Tool to convert between different versions of ECSchema(s)", "options:",
			" -V --ver 2|3", "the schema will be converted to the specified version",
			" -d --dir DIR", "looks into the following directories for reference schemas",
			" -a --all", "convert the entire schema graph",
			" -r --ref 2|3", "convert all the reference schemas to this version");
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                                   BentleySystems
	//---------------------------------------------------------------------------------------
	static bpair<uint32_t, uint32_t> GetVersionChanges(BeFileNameCR schemaFile, int version)
		{
		Utf8CP ECXML_URI = "http://www.bentley.com/schemas/Bentley.ECXML";
		BeXmlStatus status;
		BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile(status, schemaFile.GetName());
		BeXmlNodeP rootNode = static_cast<BeXmlNodeP>(xmlDocGetRootElement(&(xmlDom->GetDocument())));
		Utf8String schemaNamespace(rootNode->GetNamespace());
		Utf8String schemaVersion = schemaNamespace.substr(strlen(ECXML_URI) + 1);
		int ecXmlMajorVersion, ecXmlMinorVersion, major=0, minor=0;
		sscanf(schemaVersion.c_str(), "%d.%d", &ecXmlMajorVersion, &ecXmlMinorVersion);
		
		if (version > ecXmlMajorVersion)
			{
			major++;
			minor=0;
			}
		if (version == ecXmlMajorVersion)
			minor++;

		return bpair<uint32_t, uint32_t>(major, minor);
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                                   BentleySystems
	//---------------------------------------------------------------------------------------
	static int ConvertSchema(BeFileNameCR ecSchemaFile, BeFileNameCR outputDirectory, bvector<BeFileName> referenceDirectories, int version, bool all, int refVersion)
		{		
		ECSchemaReadContextPtr contextPtr = ECSchemaReadContext::CreateContext();
		contextPtr->AddSchemaPath(ecSchemaFile.GetDirectoryName().GetName());
		for (auto dir: referenceDirectories)
			contextPtr->AddSchemaPath(dir.GetName());
		ECSchemaPtr schema;
		SchemaReadStatus readSchemaStatus = ECSchema::ReadFromXmlFile(schema, ecSchemaFile.GetName(), *contextPtr);
		if (SchemaReadStatus::Success != readSchemaStatus)
			return (int)readSchemaStatus;
		
		bpair<uint32_t, uint32_t> versions = GetVersionChanges(ecSchemaFile, version);
		schema->SetVersionMajor(schema->GetVersionMajor() + versions.first);
		schema->SetVersionMinor(schema->GetVersionMinor() + versions.second);
		WString schemaName;
		schemaName.AssignUtf8(schema->GetFullSchemaName().c_str());
		schemaName += L".ecschema.xml";
		BeFileName outputSchemaFile(nullptr, outputDirectory.GetName(), schemaName.c_str(), nullptr);
		if (BeStringUtilities::Wcsicmp(ecSchemaFile.GetName(), outputSchemaFile.GetName()) == 0)
			{
			s_logger->infov(L"Warning: Can't overwrite the file '%ls'.", ecSchemaFile.GetName());
			s_logger->infov(L"Process terminated!!!");
			return -1;
			}
		
		if (all)
			refVersion=version;
		if (refVersion != 0)
			{
			for (auto ref : schema->GetReferencedSchemas())
				{
				ECSchemaPtr refSchema = ref.second;
				refSchema->SetVersionMajor(refSchema->GetVersionMajor() + versions.first);
				refSchema->SetVersionMinor(refSchema->GetVersionMinor() + versions.second);
				WString s;
				s.AssignUtf8(refSchema->GetFullSchemaName().c_str());
				s += L".ecschema.xml";
				BeFileName referenceSchemaOutputFile(nullptr, outputSchemaFile.GetDirectoryName().GetName(), s.c_str(), nullptr);
				if(0 != (int)refSchema->WriteToXmlFile(referenceSchemaOutputFile.GetName(), refVersion, 0))
					return -1;
				}
			}

		s_logger->infov(L"Saving ECv3 version of the schema in directory '%ls'", outputDirectory.GetName());
		return (int)schema->WriteToXmlFile(outputSchemaFile.GetName(), version, 0);
		}
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
int main(int argc, char** argv)
	{
	char* input;
	char* output;
	bvector<char*> directories;
	bool all = false;
	int version = 3;
	int flag = 0;
	int refversion = 0;
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
		else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0)
			all = true;
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			{
			ShowUsage(argv[0]);
			return -1;
			}
		else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--ver") == 0)
			{
			if ((argv[i + 1])[0] == '-' || !(atof(argv[i + 1]) - 2 == 0 || atof(argv[i + 1]) - 3 == 0))
				{
				fprintf(stderr, " -V should be follwed by either the version 2.0(2) or 3.0(3)");
				return -1;
				}
			else
				version = atoi(argv[++i]);
			}
		else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--ref") == 0)
			{
			if ((argv[i + 1])[0] == '-' || !(atof(argv[i + 1]) - 2 == 0 || atof(argv[i + 1]) - 3 == 0))
				{
				fprintf(stderr, " -r should be follwed by either the version 2.0(2) or 3.0(3)");
				return -1;
				}
			else
				refversion = atoi(argv[++i]);
			}
		else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--ref") == 0)
			{
			while (i + 1 != argc)
				{
				if ((argv[i + 1])[0] != '-')
					{
					bool check = false;
					for (size_t j = 0; j<strlen(argv[i+1]); j++)
						{
						if ((argv[i+1])[j] == '\\')
							check = true;
						}
					if (check)
						directories.push_back(argv[++i]);
					else
						{
							ShowUsage(argv[0]);
							return -1;
						}
					}
				else
					break;
				}				
			if (directories.size() == 0)
				{
				ShowUsage(argv[0]);
				return -1;
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
	s_logger->infov(L"Loading schema '%ls' for conversion to ECv3", inputFileName.GetName());
	return ConvertSchema(inputFileName, outputDirectory, refDirectories, version, all, refversion);
    }

