/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/RealityDataServiceConsole.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#define NOMINMAX

#include <Bentley/Bentley.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityDataService.h>
#include <RealityPlatform/WSGServices.h>
#include <RealityAdmin/RealityDataServiceConsole.h>
#include <CCApi/CCPublic.h>

#include <stdio.h>
#include <conio.h>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>


USING_NAMESPACE_BENTLEY_REALITYPLATFORM

static std::istream* s_inputSource = nullptr;
static std::ostream* s_outputDestination = nullptr;

static void statusFunc(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    if (ErrorCode > 0)
        *s_outputDestination << Utf8PrintfString("Curl error code : %d \n %s", ErrorCode, pMsg) << std::endl;
    else if (ErrorCode < 0)
        *s_outputDestination << pMsg << std::endl;
    }

void RealityDataConsole::InterpretCommand()
    {
    m_lastCommand = Command::Dummy;
    std::string str;
    std::getline(*s_inputSource, str);
    m_lastInput = Utf8String(str.c_str()).Trim();

    bvector<Utf8String> args;
    BeStringUtilities::ParseArguments(args, m_lastInput.c_str());
    if (args.size() < 1)
        {
        DisplayInfo("missing input. Please refer to \"Help\" \n", DisplayOption::Error);
        return;
        }
    if (args.size() > 2)
        {
        DisplayInfo("too many inputs to parse. Please refer to \"Help\" \n", DisplayOption::Error);
        return;
        }

    if (args[0].EqualsI("quit"))
        m_lastCommand = Command::Quit;
    else if (args[0].EqualsI("retry"))
        m_lastCommand = Command::Retry;
    else if (args[0].EqualsI("ListAll"))
        m_lastCommand = Command::ListAll;
    else if (args[0].EqualsI("list") || args[0].EqualsI("dir"))
        m_lastCommand = Command::List;
    else if (args[0].EqualsI("help"))
        m_lastCommand = Command::Help;
    else if (args[0].EqualsI("stat"))
        m_lastCommand = Command::Stat;
    else if (args[0].EqualsI("cancel"))
        m_lastCommand = Command::Cancel;
    else if (args[0].EqualsI("details"))
        m_lastCommand = Command::Details;
    else if (args[0].EqualsI("Download"))
        m_lastCommand = Command::Download;
    else if (args[0].EqualsI("Upload"))
        m_lastCommand = Command::Upload;
    else if (args[0].EqualsI("SetServer"))
        m_lastCommand = Command::SetServer;
    else if (args[0].EqualsI("SetProjectId"))
        m_lastCommand = Command::SetProjectId;
    else if (args[0].EqualsI("ChangeProps"))
        m_lastCommand = Command::ChangeProps;
    else if (args[0].EqualsI("Delete"))
        m_lastCommand = Command::Delete;
    else if (args[0].EqualsI("Filter"))
        m_lastCommand = Command::Filter;
    else if (args[0].EqualsI("Relationships"))
        m_lastCommand = Command::Relationships;
    else if (args[0].EqualsI("Link"))
        m_lastCommand = Command::Link;
    else if (args[0].EqualsI("Unlink"))
        m_lastCommand = Command::Unlink;
    else if (args[0].EqualsI("CreateRD"))
        m_lastCommand = Command::CreateRD;
    else
        {
        m_lastCommand = Command::Error;

        if (args[0].EqualsI("cd") || args[0].EqualsI("cd.."))
            m_lastCommand = Command::ChangeDir;
        else if (args[0].EqualsI("FileAccess"))
            m_lastCommand = Command::FileAccess;
        else if (args[0].EqualsI("AzureAddress"))
            m_lastCommand = Command::AzureAddress;
        if (m_lastCommand != Command::Error)
            {
            if (args.size() > 1)
                m_lastInput = args[1];
            else
                {
                if (m_lastCommand == Command::ChangeDir && args[0].Contains(".."))
                    m_lastInput = ".."; //allow "cd.."
                else
                    {
                    DisplayInfo("must input a value, with this command\n", DisplayOption::Error);
                    m_lastCommand = Command::Error;
                    }
                }
            }
        }
    }

RealityDataConsole::RealityDataConsole() :
    m_server(WSGServer("", true)),
    m_serverNodes(bvector<NavNode>()),
    m_machineRepos(bvector<Utf8String>()),
    m_currentNode(nullptr),
    m_nameFilter(""),
    m_groupFilter(""),
    m_typeFilter(""),
    m_ownerFilter("")
    {
    m_functionMap.Insert(Command::Help, &RealityDataConsole::Usage);
    m_functionMap.Insert(Command::SetServer, &RealityDataConsole::ConfigureServer);
    m_functionMap.Insert(Command::SetProjectId, &RealityDataConsole::SetProjectId);
    m_functionMap.Insert(Command::List, &RealityDataConsole::List);
    m_functionMap.Insert(Command::ListAll, &RealityDataConsole::ListAll);
    m_functionMap.Insert(Command::ChangeDir, &RealityDataConsole::ChangeDir);
    m_functionMap.Insert(Command::Stat, &RealityDataConsole::EnterpriseStat);
    m_functionMap.Insert(Command::Details, &RealityDataConsole::Details);
    m_functionMap.Insert(Command::Download, &RealityDataConsole::Download);
    m_functionMap.Insert(Command::Upload, &RealityDataConsole::Upload);
    m_functionMap.Insert(Command::FileAccess, &RealityDataConsole::FileAccess);
    m_functionMap.Insert(Command::AzureAddress, &RealityDataConsole::AzureAddress);
    m_functionMap.Insert(Command::ChangeProps, &RealityDataConsole::ChangeProps);
    m_functionMap.Insert(Command::Delete, &RealityDataConsole::Delete);
    m_functionMap.Insert(Command::Filter, &RealityDataConsole::Filter);
    m_functionMap.Insert(Command::Relationships, &RealityDataConsole::Relationships);
    m_functionMap.Insert(Command::CreateRD, &RealityDataConsole::CreateRD);
    m_functionMap.Insert(Command::Link, &RealityDataConsole::Link);
    m_functionMap.Insert(Command::Unlink, &RealityDataConsole::Unlink);

    //commands that should never occur, within Run()
    m_functionMap.Insert(Command::Quit, &RealityDataConsole::DummyFunction);
    m_functionMap.Insert(Command::Retry, &RealityDataConsole::DummyFunction);
    m_functionMap.Insert(Command::Error, &RealityDataConsole::InputError);
    m_functionMap.Insert(Command::ChoiceIndex, &RealityDataConsole::DummyFunction);
    m_functionMap.Insert(Command::ChoiceValue, &RealityDataConsole::DummyFunction);
    m_functionMap.Insert(Command::Dummy, &RealityDataConsole::DummyFunction);
    m_functionMap.Insert(Command::Cancel, &RealityDataConsole::DummyFunction);

    m_realityDataProperties = bvector<Utf8String>();
    //m_realityDataProperties.push_back("Id");
    m_realityDataProperties.push_back("OrganizationId");
    //m_realityDataProperties.push_back("ContainerName");
    m_realityDataProperties.push_back("Name");
    m_realityDataProperties.push_back("Dataset");
    m_realityDataProperties.push_back("Group");
    m_realityDataProperties.push_back("Description");
    m_realityDataProperties.push_back("RootDocument");
    //m_realityDataProperties.push_back("Size");
    m_realityDataProperties.push_back("Classification");
    m_realityDataProperties.push_back("Type");
    m_realityDataProperties.push_back("Streamed");
    m_realityDataProperties.push_back("Footprint");
    m_realityDataProperties.push_back("ThumbnailDocument");
    m_realityDataProperties.push_back("MetadataUrl");
    m_realityDataProperties.push_back("Copyright");
    m_realityDataProperties.push_back("TermsOfUse");
    m_realityDataProperties.push_back("ResolutionInMeters");
    m_realityDataProperties.push_back("AccuracyInMeters");
    m_realityDataProperties.push_back("Visibility");
    m_realityDataProperties.push_back("Listable");
    //m_realityDataProperties.push_back("ModifiedTimestamp");
    //m_realityDataProperties.push_back("CreatedTimestamp");
    m_realityDataProperties.push_back("OwnedBy");
    m_realityDataProperties.push_back("-Finish-");

    m_visibilityOptions = bvector<Utf8String>();
    //m_visibilityOptions.push_back("PUBLIC");
    m_visibilityOptions.push_back("ENTERPRISE");
    m_visibilityOptions.push_back("PRIVATE");
    m_visibilityOptions.push_back("PERMISSION");

    m_classificationOptions = bvector<Utf8String>();
    m_classificationOptions.push_back("Model");
    m_classificationOptions.push_back("Imagery");
    m_classificationOptions.push_back("Pinned");
    m_classificationOptions.push_back("Terrain");

    m_typeOptions = bvector<Utf8String>();
    m_typeOptions.push_back("3MX");
    m_typeOptions.push_back("3SM");
    m_typeOptions.push_back("RealityMesh3DTiles");
    m_typeOptions.push_back("POD");
    m_typeOptions.push_back("PointCloud3DTiles");
    m_typeOptions.push_back("Orthophoto");
    m_typeOptions.push_back("RasterDSM");
    m_typeOptions.push_back("Custom");

    m_filterProperties = bvector<Utf8String>();
    m_filterProperties.push_back("Name");
    m_filterProperties.push_back("Group");
    m_filterProperties.push_back("Type");
    m_filterProperties.push_back("OwnedBy");
    m_filterProperties.push_back("Fuzzy Filter");
    m_filterProperties.push_back("ProjectId");
    m_filterProperties.push_back("-Finish-");

    m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);       // see the methods Disp...()
    }

void RealityDataConsole::Choice(bvector<Utf8String> options, Utf8StringR input)
    {
    PrintResults(options);
    DisplayInfo("An option can be selected by its Index\n", DisplayOption::Question);
    DisplayInfo("Please input your choice\n? ", DisplayOption::Question);

    uint64_t choice;

    std::string str;
    std::getline(*s_inputSource, str);
    input = Utf8String(str.c_str()).Trim();
    if (input.EqualsI("Quit"))
        {
        m_lastCommand = Command::Quit;
        return;
        }
    else if (input.EqualsI("Retry"))
        return Choice(options, input);
    else
        {   
        if ((BeStringUtilities::ParseUInt64(choice, input.c_str()) == BentleyStatus::SUCCESS) && (choice > 0))
            { //sometimes ParseUInt64 will return SUCCESS, even if the input wasn't a number, and set the int to 0
            choice -= 1;
            if (choice >= options.size())
                {
                DisplayInfo(Utf8PrintfString("Invalid Selection; selected index not between 1 and %lu \n", options.size()), DisplayOption::Error);
                m_lastCommand = Command::Retry;
                }
            else
                input = options[choice];
            }
        else
            {
            DisplayInfo("Could not extract a number from provided input. Use input as was provided? [ y / n ]\n", DisplayOption::Question);
            std::getline(*s_inputSource, str);
            Utf8String use(str.c_str());
            if (use.EqualsI("Quit"))
                {
                m_lastCommand = Command::Quit;
                return;
                }
            else if (!use.EqualsI("y"))
                {
                DisplayInfo("Retrying\n", DisplayOption::Tip);
                return Choice(options, input);
                }
            }
        }
    }


void RealityDataConsole::Run(BeFileName infile, BeFileName outfile)
    {
    if (infile.DoesPathExist())
        {
        std::ifstream* ifs = new std::ifstream();
        ifs->open(infile.GetNameUtf8().c_str());
        s_inputSource = ifs;
        }
    
    BeFile outFileStream;
    if (outFileStream.Create(outfile.GetNameUtf8().c_str(), true) == BeFileStatus::Success)
        {
        outFileStream.Close();
        std::ofstream* ofs = new std::ofstream();
        ofs->open(outfile.GetNameUtf8().c_str());
        s_outputDestination = ofs;
        }

    if(s_inputSource == nullptr || s_outputDestination == nullptr)
        return;

    ConfigureServer();
    _Run();
    s_outputDestination->flush();
    (dynamic_cast<std::ifstream*>(s_inputSource))->close();
    (dynamic_cast<std::ofstream*>(s_outputDestination))->close();
    delete s_inputSource;
    delete s_outputDestination;
    }

void RealityDataConsole::Run()
    {
    s_inputSource = &std::cin;
    s_outputDestination = &std::cout;
    ConfigureServer();
    _Run();
    }

void RealityDataConsole::Run(Utf8String server, Utf8String projectId)
    {
    s_inputSource = &std::cin;
    s_outputDestination = &std::cout;
    m_server = WSGServer(server, false);
    RawServerResponse versionResp = RawServerResponse();
    RealityDataService::SetServerComponents(server, m_server.GetVersion(versionResp), "S3MXECPlugin--Server", "S3MX", "", projectId);
    DisplayInfo(Utf8PrintfString("Console started with server: %s and projectId: %s\n", server, projectId), DisplayOption::Tip);
    _Run();
    }

void RealityDataConsole::_Run()
    {
    while (m_lastCommand != Command::Quit)
        {
        if (m_currentNode != nullptr)
            DisplayInfo(Utf8PrintfString("%s", m_currentNode->node.GetInstanceId()), DisplayOption::Tip);
        DisplayInfo("> ", DisplayOption::Tip);
        InterpretCommand();
        (this->*(m_functionMap[m_lastCommand]))();
        }
    }

void RealityDataConsole::Usage()
    {
    DisplayInfo("  RealityDataConsole tool for RDS V1.0\n\n");
    DisplayInfo("  Available Commands (case insensitive):\n");
    DisplayInfo("  Quit                Exit the application\n");
    DisplayInfo("  Retry               (during a multi-step operation) Restart current operation\n");
    DisplayInfo("  Help                Print current Display\n");
    DisplayInfo("  SetServer           Change server settings (server url, repository and schema)\n");
    DisplayInfo("  SetProjectId        Sets the project id used for all RDS requests\n");
    DisplayInfo("  List                List all subfiles/folders for the given location on your server\n");
    DisplayInfo("  Dir                 same as List\n");
    DisplayInfo("  Filter              filters RealityDatas returned from a List/Dir command\n");
    DisplayInfo("  cd                  Change current location. Must be called in one of the following ways\n");
    DisplayInfo("  cd [number]         navigates to node at the given index, as specified in the most recent List command\n");
    DisplayInfo("  cd ..               go up one level\n");
    DisplayInfo("  ListAll             List every file beneath the current location (paged)\n");
    DisplayInfo("  Details             show the details for the location\n");
    DisplayInfo("  Stat                show enterprise statistics\n");
    DisplayInfo("  Download            Download files from the current location on the server\n");
    DisplayInfo("  Upload              Upload files to the server\n");
    DisplayInfo("  FileAccess <opt>    Prints the URL to use if you wish to request an azure file access (option \"w\" for write access or \"r\" for read access)\n");
    DisplayInfo("  AzureAddress <opt>  Prints the URL to use (option \"w\" for write access or \"r\" for read access)\n");
    DisplayInfo("  ChangeProps         Modify the properties of a RealityData\n");
    DisplayInfo("  Relationships       Show all projects attached to this RealityData\n");
    DisplayInfo("  Link                Create a relationship between a RealityData and a project\n");
    DisplayInfo("  Unlink              Remove a relationship between a RealityData and a project\n");
    DisplayInfo("  CreateRD            Create a new RealityData\n");
    DisplayInfo("  Delete              Delete a RealityData, Folder or single Document\n");
    DisplayInfo("\nNote: Special characters may not be displayed properly, and items containing such characters may have bugs with uploading and downloading.\n ", DisplayOption::Tip);
    DisplayInfo("The good news is: this is a console problem, not an SDK problem. If you need to interact with those entries, please use a different service\n", DisplayOption::Tip);
    }

void RealityDataConsole::PrintResults(bvector<Utf8String> results)
    {
    std::stringstream index;
    Utf8String fullOption;
    DisplayInfo("Index \t Value\n");
    std::string str;
    for (size_t i = 0; i < results.size(); ++i)
        {
        DisplayInfo(Utf8PrintfString("%5d \t %s\n", (i + 1), results[i]));
        }
    }

void RealityDataConsole::PrintResults(bmap<Utf8String, bvector<Utf8String>> results)
    {
    std::stringstream index;
    Utf8String fullOption;
    DisplayInfo("Index \t Value\n");
    std::string str;
    size_t j = 0;
    for(bmap<Utf8String, bvector<Utf8String>>::iterator iter = results.begin(); iter != results.end(); ++iter)
        {
        Utf8String key = iter->first;
        DisplayInfo(Utf8PrintfString("\t\t%s\n", key), DisplayOption::Question);
        for (size_t i = 0; i < results[key].size(); ++i)
            {
            DisplayInfo(Utf8PrintfString("%5d \t %s\n", (j + 1), results[key][i]));
            ++j;
            }
        }
    }

Utf8String RealityDataConsole::MakeBuddiCall(int region)
    {
    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled(api, &installed);
    if (!installed)
        {
        DisplayInfo("Connection client does not seem to be installed\n", DisplayOption::Error);
        return "";
        }
    bool running = false;
    status = CCApi_IsRunning(api, &running);
    if (status != APIERR_SUCCESS || !running)
        {
        DisplayInfo("Connection client does not seem to be running\n", DisplayOption::Error);
        return "";
        }
    bool loggedIn = false;
    status = CCApi_IsLoggedIn(api, &loggedIn);
    if (status != APIERR_SUCCESS || !loggedIn)
        {
        DisplayInfo("Connection client does not seem to be logged in\n", DisplayOption::Error);
        return "";
        }
    bool acceptedEula = false;
    status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
    if (status != APIERR_SUCCESS || !acceptedEula)
        {
        DisplayInfo("Connection client user does not seem to have accepted EULA\n", DisplayOption::Error);
        return "";
        }
    bool sessionActive = false;
    status = CCApi_IsUserSessionActive(api, &sessionActive);
    if (status != APIERR_SUCCESS || !sessionActive)
        {
        DisplayInfo("Connection client does not seem to have an active session\n", DisplayOption::Error);
        return "";
        }

    wchar_t* buddiUrl;
    UINT32 strlen = 0;

    if(region > 100)
        {
        CCApi_GetBuddiRegionUrl(api, L"RealityDataServices", region, NULL, &strlen);
        strlen += 1;
        buddiUrl = (wchar_t*)malloc((strlen) * sizeof(wchar_t));
        CCApi_GetBuddiRegionUrl(api, L"RealityDataServices", region, buddiUrl, &strlen);
        }
    else
        {
        CCApi_GetBuddiUrl(api, L"RealityDataServices", NULL, &strlen);
        strlen += 1;
        buddiUrl = (wchar_t*)malloc((strlen) * sizeof(wchar_t));
        CCApi_GetBuddiUrl(api, L"RealityDataServices", buddiUrl, &strlen);
        }

    char* charServer = new char[strlen];
    wcstombs(charServer, buddiUrl, strlen);
    
    CCApi_FreeApi(api);

    return Utf8String(charServer);
    }

void RealityDataConsole::ConfigureServer()
    {
    DisplayInfo("Welcome to the RealityDataService Navigator.\n", DisplayOption::Tip);
    DisplayInfo("If you want to specifically contact dev or qa, enter dev or qa. For a custom server url, type \"custom\".\n"
        "Otherwise, enter blank and we will connect you to the proper server for you ConnectionClient configuration\n", DisplayOption::Question);
    Utf8String server, serverChoice;
    std::string input;
    std::getline(*s_inputSource, input);
    serverChoice = Utf8String(input.c_str()).Trim();
    
    if (serverChoice.EqualsI("dev"))
        server = MakeBuddiCall(103);
    else if (serverChoice.EqualsI("qa"))
        server = MakeBuddiCall(102);
    else if (serverChoice.EqualsI("custom"))
        {
        DisplayInfo("Enter server url\n", DisplayOption::Question);
        std::getline(*s_inputSource, input);
        server = Utf8String(input.c_str()).Trim();
        }
    else
        server = MakeBuddiCall();

    if(server.empty())
        {
        DisplayInfo("ConnectionClient required for console functionality. Cannot Proceed\n", DisplayOption::Error);
        return;
        }
    else
        DisplayInfo(Utf8PrintfString("Connecting to %s\n", server), DisplayOption::Tip);

    bool verifyCertificate = false;
    Utf8String certificatePath = "";

    DisplayInfo("Does this server have a recognized certificate? [ y / n ]  ?", DisplayOption::Question);
    Utf8String temp;
    std::getline(*s_inputSource, input);
    temp = Utf8String(input.c_str()).Trim();
    if (temp.EqualsI("y"))
        {
        verifyCertificate = true;
        DisplayInfo("If you need to use a custom certificate file, enter the path to it now. Otherwise input blank\n ?", DisplayOption::Question);
        std::getline(*s_inputSource, input);
        certificatePath = Utf8String(input.c_str()).Trim();
        BeFileName fileName = BeFileName(certificatePath);
        if (!certificatePath.empty() && !BeFileName::DoesPathExist(fileName))
            {
            DisplayInfo("Could not validate specified path. Please verify that it is correct and try again\n", DisplayOption::Error);
            return;
            }
        WSGRequest::GetInstance().SetCertificatePath(certificatePath.c_str());
        }
    else if (temp.EqualsI("quit"))
        {
        m_lastCommand = Command::Quit;
        return;
        }

    DisplayInfo("Retrieving version information. One moment...\n\n", DisplayOption::Tip);

    m_server = WSGServer(server, verifyCertificate);
    RawServerResponse versionResponse = RawServerResponse();
    Utf8String version = m_server.GetVersion(versionResponse);

    if (version.length() == 0)
        {
        DisplayInfo("There was an error contacting the server\n Please check that the URL was entered correctly and that the Connection Client is running and properly configured\n", DisplayOption::Error);
        while (m_lastCommand != Command::Retry && m_lastCommand != Command::Quit)
            {
            DisplayInfo("\"Retry\" to try with a different server; \"Quit\" to exit\n", DisplayOption::Error);
            InterpretCommand();
            if (m_lastCommand == Command::Retry)
                return ConfigureServer();
            else if (m_lastCommand == Command::Quit)
                {
                DisplayInfo("Quitting...\n", DisplayOption::Error);
                return;
                }
            }
        }

    Utf8String repo;
    Utf8String schema;

    RawServerResponse repoResponse = RawServerResponse();
    bvector<Utf8String> repoNames = m_server.GetRepositories(repoResponse);
    m_lastCommand = Command::Error;
    if (repoNames.size() == 0)
        {
        DisplayInfo("There was an error contacting the server. No repositories found\n --> Is your ConnectClient configured correctly?\n", DisplayOption::Error);
        while (m_lastCommand != Command::Retry && m_lastCommand != Command::Quit)
            {
            DisplayInfo("\"Retry\" to try with a different server; \"Quit\" to exit\n", DisplayOption::Error);
            InterpretCommand();
            if (m_lastCommand == Command::Retry)
                return ConfigureServer();
            else if (m_lastCommand == Command::Quit)
                {
                DisplayInfo("Quitting...\n", DisplayOption::Error);

                return;
                }
            }
        }
    else if (repoNames.size() == 1)
        {
        DisplayInfo("Only one repository found\n");
        repo = repoNames[0];
        DisplayInfo(Utf8PrintfString("Defaulting to %s\n", repo));
        }
    else
        {
        DisplayInfo("Please select a repository from the following options\n", DisplayOption::Question);
        Choice(repoNames, repo);
        switch (m_lastCommand)
            {
        case Command::Quit:
            return;
        case Command::Retry:
            return ConfigureServer();
            }
        }

    if (repo.length() > 0)
        {
        DisplayInfo("\n");

        RawServerResponse schemaResponse = RawServerResponse();
        bvector<Utf8String> schemaNames = m_server.GetSchemaNames(repo, schemaResponse);

        if (schemaNames.size() == 0)
            {
            DisplayInfo("No schemas were found for the given server and repo\n", DisplayOption::Error);
            while (m_lastCommand != Command::Retry && m_lastCommand != Command::Quit)
                {
                DisplayInfo("\"Retry\" to try with a different server or repo; \"Quit\" to exit\n", DisplayOption::Error);
                InterpretCommand();
                if (m_lastCommand == Command::Retry)
                    return ConfigureServer();
                else if (m_lastCommand == Command::Quit)
                    {
                    DisplayInfo("Quitting...\n", DisplayOption::Error);
                    return;
                    }
                }
            }
        else if (schemaNames.size() == 1)
            {
            DisplayInfo("Only one schema found\n");
            schema = schemaNames[0];
            DisplayInfo(Utf8PrintfString("Defaulting to %s \n", schema));
            }
        else
            {
            DisplayInfo("please select a repository from the following options\n", DisplayOption::Question);
            Choice(schemaNames, schema);
            switch (m_lastCommand)
                {
            case Command::Quit:
                return;
            case Command::Retry:
                return ConfigureServer();
                }
            }

        if (schema.empty())
            {
            DisplayInfo("Server configuration failed, invalid parameters passed\n", DisplayOption::Error);
            return;
            }
        }
    else
        {
        DisplayInfo("Server configuration failed, invalid parameters passed\n", DisplayOption::Error);
        return;
        }

    DisplayInfo("ProjectId required for multiple operations. Set ProjectId now?\n", DisplayOption::Question);
    std::string str;
    std::getline(*s_inputSource, str);
    if (strstr(str.c_str(), "quit"))
        {
        m_lastCommand = Command::Quit;
        return;
        }
    else if (str == "y")
        {
        DisplayInfo("Please set project id\n ?", DisplayOption::Question);
        std::string input;
        std::getline(*s_inputSource, input);
        RealityDataService::SetServerComponents(server, version, repo, schema, certificatePath, Utf8String(input.c_str()).Trim());
        }
    else if (str.length() == 36) //they input the project id right away
        {
        RealityDataService::SetServerComponents(server, version, repo, schema, certificatePath, Utf8String(str.c_str()).Trim());
        }
    else 
        {
        DisplayInfo("ProjectId not set\n", DisplayOption::Tip);
        if (verifyCertificate)
            RealityDataService::SetServerComponents(server, version, repo, schema, certificatePath);
        else
            RealityDataService::SetServerComponents(server, version, repo, schema);
        }

    DisplayInfo("Server successfully configured, ready for use. Type \"help\" for list of commands\n", DisplayOption::Tip);
    }
    
void RealityDataConsole::SetProjectId()
    {
    DisplayInfo("Please set project id\n ?", DisplayOption::Question);
    std::string input;
    std::getline(*s_inputSource, input);
    RealityDataService::SetProjectId(Utf8String(input.c_str()).Trim());
    }

void RealityDataConsole::List()
    {
    if (m_currentNode != nullptr && m_currentNode->node.GetECClassName() == "Document")
        {
        DisplayInfo("You are currently on a document, there are no files beneath this point\n", DisplayOption::Error);
        return;
        }

    m_serverNodes.clear();
    Utf8String nodeString;
    bvector<Utf8String> nodeStrings;

    RawServerResponse nodeResponse = RawServerResponse();

    if (m_currentNode == nullptr)
        return ListRoots();
    else
        m_serverNodes = NodeNavigator::GetInstance().GetChildNodes(m_server, RealityDataService::GetRepoName(), m_currentNode->node, nodeResponse);

    if(nodeResponse.body.ContainsI("not listable"))
        DisplayInfo("This entity is not listable\n", DisplayOption::Error);
    else
        {
        for (NavNode node : m_serverNodes)
            {
            nodeString = node.GetLabel();
            if (node.GetECClassName() == "Folder")
                nodeString.append("/");
            nodeStrings.push_back(nodeString);
            }   

        PrintResults(nodeStrings);
        }
    }

Utf8String ShortenVisibility(Utf8String visibility)
    {
    if(visibility.EqualsI("PUBLIC"))
        return "PUB";
    else if (visibility.EqualsI("ENTERPRISE"))
        return "ENT";
    else if (visibility.EqualsI("PRIVATE"))
        return "PRV";
    else if (visibility.EqualsI("PERMISSION"))
        return "PRM";
    else
        return "---";
    }

void RealityDataConsole::ListRoots()
    {
    RealityDataListByOrganizationPagedRequest organizationReq = RealityDataListByOrganizationPagedRequest("", 0, 2500);

    bvector<RDSFilter> properties = bvector<RDSFilter>();
    if (m_nameFilter.length() > 0)
        properties.push_back(RealityDataFilterCreator::FilterByName(m_nameFilter));
    if (m_groupFilter.length() > 0)
        properties.push_back(RealityDataFilterCreator::FilterByGroup(m_groupFilter));
    if (m_typeFilter.length() > 0)
        properties.push_back(RealityDataFilterCreator::FilterByType(m_typeFilter));
    if (m_ownerFilter.length() > 0)
        properties.push_back(RealityDataFilterCreator::FilterByOwner(m_ownerFilter));
    if (properties.size() > 0)
        organizationReq.SetFilter(RealityDataFilterCreator::GroupFiltersAND(properties));

    if (m_queryFilter.length() > 0)
        organizationReq.SetQuery(m_queryFilter);

    if (m_projectFilter.length() > 0)
        organizationReq.SetProject(m_projectFilter);

    organizationReq.SortBy(RealityDataField::OwnedBy, true);

    RawServerResponse organizationResponse = RawServerResponse();
    organizationResponse.status = RequestStatus::OK;
    bvector<RealityDataPtr> organizationVec = bvector<RealityDataPtr>();
    bvector<RealityDataPtr> partialVec;

    while (organizationResponse.status == RequestStatus::OK)
        {//When LASTPAGE has been added, loop will exit
        partialVec = RealityDataService::Request(organizationReq, organizationResponse);
        organizationVec.insert(organizationVec.end(), partialVec.begin(), partialVec.end());
        }
    bmap<Utf8String, bvector<Utf8String>> nodes = bmap<Utf8String, bvector<Utf8String>>();
    bvector<Utf8String> subvec = bvector<Utf8String>();
    Utf8String owner;
    if(organizationVec.size() > 0)
        owner = organizationVec[0]->GetOwner();

    Utf8String schema = RealityDataService::GetSchemaName();
    int position = 0;
    for (RealityDataPtr rData : organizationVec)
        {
        if(owner != rData->GetOwner())
            {
            nodes.Insert(owner, subvec);
            subvec.clear();
            owner = rData->GetOwner();
            }

        subvec.push_back(Utf8PrintfString("%-30s  %-22s (%s / %s) %s  %ld", rData->GetName(), rData->GetRealityDataType(), rData->IsListable() ? "Lst" : " - ", ShortenVisibility(rData->GetVisibilityTag()), rData->GetIdentifier(), rData->GetTotalSize()));

        m_serverNodes.push_back(NavNode(schema, rData->GetIdentifier(), "ECObjects", "RealityData"));

        position++;
        if(position == organizationVec.size())
            nodes.Insert(owner, subvec);
        }

    PrintResults(nodes);
    }

void RealityDataConsole::ListAll()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo("Please navigate to a RealityData or Folder before using this command\n", DisplayOption::Tip);
        return;
        }

    AzureHandshake* handshake = new AzureHandshake(m_currentNode->node.GetInstanceId(), false);
    RawServerResponse handshakeResponse = RealityDataService::BasicRequest((RealityDataUrl*)handshake);
    Utf8String azureServer;
    Utf8String azureToken;
    int64_t tokenTimer;
    BentleyStatus handshakeStatus = handshake->ParseResponse(handshakeResponse.body, azureServer, azureToken, tokenTimer);
    delete handshake;
    if (handshakeStatus != BentleyStatus::SUCCESS)
        {
        DisplayInfo("Failure retrieving Azure token\n", DisplayOption::Error);
        return;
        }

    AllRealityDataByRootId rdsRequest = AllRealityDataByRootId(m_currentNode->node.GetInstanceId());

    RawServerResponse sasResponse = RawServerResponse();
    bvector<bpair<WString, uint64_t>> filesInRepo = RealityDataService::Request(rdsRequest, sasResponse);

    DisplayInfo(Utf8PrintfString(" %lu files in selection.\n", filesInRepo.size()), DisplayOption::Tip);
    DisplayInfo("these will be displayed, 20 at a time. Input \"Cancel\" to quit at any time, otherwise press enter to proceed to the next page\n", DisplayOption::Tip);

    std::string str;
    size_t placeholder = 0;
    size_t step;
    size_t size = filesInRepo.size();
    while (m_lastCommand != Command::Cancel && m_lastCommand != Command::Quit && placeholder < size)
        {
        std::getline(*s_inputSource, str);
        if (Utf8String(str.c_str()).Trim().EqualsI("Cancel"))
            m_lastCommand = Command::Cancel;
        else if (Utf8String(str.c_str()).Trim().EqualsI("Quit"))
            m_lastCommand = Command::Quit;
        else
            {
            step = (size < (placeholder + 20)) ? size : placeholder + 20;
            DisplayInfo("Input \"Cancel\" to quit at any time, otherwise press enter to proceed to the next page\n", DisplayOption::Tip);
            for (; placeholder < step; ++placeholder)
                {
                DisplayInfo(Utf8String(WPrintfString(L"%s %lu bytes \n", filesInRepo[placeholder].first.c_str(), filesInRepo[placeholder].second)));
                }
            }
        }
    DisplayInfo("----------------\nexiting listing\n----------------\n", DisplayOption::Tip);
    }

void RealityDataConsole::ChangeDir()
    {
    if (m_lastInput == "..")
        {
        if (m_currentNode == nullptr)
            {
            DisplayInfo("Already at root\n", DisplayOption::Tip);
            return;
            }
        if (m_currentNode->parentNode != nullptr)
            {
            m_currentNode = m_currentNode->parentNode;
            delete m_currentNode->childNode;
            }
        else
            {
            delete m_currentNode;
            m_currentNode = nullptr;
            }
        List();
        return;
        }

    uint64_t choice;
    choice = _atoi64(m_lastInput.c_str());
    if (choice == 0)
        {
        DisplayInfo("Could not extract integer from provided input...\n", DisplayOption::Error);
        return;
        }

    if (m_serverNodes.size() == 0)
        {
        DisplayInfo("Need to use \"List\" or \"Dir\" before using this command\n", DisplayOption::Error);
        DisplayInfo("If you have already done this, there may be no listable locations to navigate to\n", DisplayOption::Error);
        return;
        }

    choice -= 1; //Adjusted for how navnodes are displayed

    if (choice < (uint64_t)m_serverNodes.size())
        {
        NodeList* newNode = new NodeList();
        newNode->node = m_serverNodes[choice];
        newNode->parentNode = m_currentNode;
        if (m_currentNode != nullptr)
            m_currentNode->childNode = newNode;
        m_currentNode = newNode;
        m_serverNodes.clear();
        }
    else
        DisplayInfo(Utf8PrintfString("Invalid Selection, selected index not between 1 and %lu\n", m_serverNodes.size()), DisplayOption::Error);
    }

void RealityDataConsole::EnterpriseStat()
    {
    RawServerResponse rawResponse = RawServerResponse();
    RealityDataEnterpriseStatRequest* ptt = new RealityDataEnterpriseStatRequest("");
    RealityDataEnterpriseStat stat;
    RealityDataService::Request(*ptt, stat, rawResponse);

    DisplayInfo("Enterprise statistics: \n");
    DisplayInfo(Utf8PrintfString("   NbRealityData : %lu\n", stat.GetNbRealityData()));
    DisplayInfo(Utf8PrintfString("   TotalSize(KB) : %lu\n", stat.GetTotalSizeKB()));
    DisplayInfo(Utf8PrintfString("   OrganizationId: %s\n", stat.GetOrganizationId().c_str()));
    DisplayInfo(Utf8PrintfString("   UltimateId    : %s\n", stat.GetUltimateId().c_str()));
    DisplayInfo(Utf8PrintfString("   UltimateSite  : %s\n\n", stat.GetUltimateSite().c_str()));
    }

static void downloadProgressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    char progressString[1024];
    sprintf(progressString, "percentage of files downloaded : %.1f\r", repoProgress * 100.0);
    *s_outputDestination << progressString;
    }

static void uploadProgressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    char progressString[1024];
    //sprintf(progressString, "%s upload percent : %f", filename.c_str(), progress * 100.0f);
    sprintf(progressString, "upload percent : %.1f\r", repoProgress * 100.0);
    *s_outputDestination << progressString;
    }

void RealityDataConsole::Download()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo("Please navigate to a RealityData or Folder before using this command\n", DisplayOption::Tip);
        return;
        }

    DisplayInfo(Utf8PrintfString("Downloading from %s\n", m_currentNode->node.GetLabel()), DisplayOption::Tip);
    DisplayInfo("If you wish to change this, use command \"Cancel\" to back out and use cd to change the directory\n\n", DisplayOption::Tip);
    DisplayInfo("Please enter the destination folder on the local machine (must be existing folder)\n ?", DisplayOption::Question);

    InterpretCommand();
    if (m_lastCommand == Command::Cancel)
        return;

    BeFileName fileName = BeFileName(m_lastInput);
    if (!fileName.DoesPathExist())
        {
        DisplayInfo("Could not validate specified path. Please verify that the folder exists and try again\n", DisplayOption::Error);
        return;
        }

    RealityDataServiceDownload download = RealityDataServiceDownload(fileName, m_currentNode->node.GetInstanceId(), statusFunc);
    if (download.IsValidTransfer())
        {
        download.SetProgressCallBack(downloadProgressFunc);
        download.SetProgressStep(0.1);
        download.OnlyReportErrors(true);
        const TransferReport& tReport = download.Perform();
        Utf8String report;
        tReport.ToXml(report);
        DisplayInfo("If any files failed to download, they will be listed here: \n");
        DisplayInfo(Utf8PrintfString("%s\n", report));
        }
    else
        DisplayInfo("Download could not be completed. Please verify you have access to this RealityData and that it has files to download\n", DisplayOption::Error);
    }

void RealityDataConsole::Upload()
    {
    DisplayInfo("Please enter the source folder on the local machine (must be existing folder)\n  ?", DisplayOption::Question);

    InterpretCommand();
    if (m_lastCommand == Command::Cancel)
        return;

    BeFileName fileName = BeFileName(m_lastInput);
    if (!fileName.DoesPathExist())
        {
        DisplayInfo("Could not validate specified path. Please verify that the folder exists and try again\n", DisplayOption::Question);
        return;
        }

    Utf8String guid = "";
    Utf8String propertyString = "";
    Utf8String option;
    if (m_currentNode == nullptr)
        {
        std::string input;
        bmap<RealityDataField, Utf8String> properties;
        DisplayInfo("Please input value for Name\n  ?", DisplayOption::Question);
        std::getline(*s_inputSource, input);
        Utf8String command(input.c_str());
        if (command.EqualsI("Quit"))
            {
            m_lastCommand = Command::Quit;
            return;
            }
        properties.Insert(RealityDataField::Name, Utf8String(input.c_str()).Trim());

        DisplayInfo("Please choose a Classification\n  ?", DisplayOption::Question);
        Choice(m_classificationOptions, option);
        properties.Insert(RealityDataField::Classification, option);

        DisplayInfo("Please choose a Type\n  ?", DisplayOption::Question);
        Choice(m_typeOptions, option);
        if(option == "Custom")
            {
            DisplayInfo("You have selected custom input. Please input value for Type\n  ?", DisplayOption::Question);
            std::getline(*s_inputSource, input);
            command = Utf8String(input.c_str());
            if (command.EqualsI("Quit"))
                {
                m_lastCommand = Command::Quit;
                return;
                }
            option = Utf8String(input.c_str()).Trim();
            }
        properties.Insert(RealityDataField::Type, option);

        DisplayInfo("Please choose a Visibility\n  ?", DisplayOption::Question);
        Choice(m_visibilityOptions, option);
        properties.Insert(RealityDataField::Visibility, option);

        DisplayInfo("Please input value for RootDocument\n  ?", DisplayOption::Question);
        std::getline(*s_inputSource, input);
        command = Utf8String(input.c_str());
        if (command.EqualsI("Quit"))
            {
            m_lastCommand = Command::Quit;
            return;
            }
        properties.Insert(RealityDataField::RootDocument, Utf8String(input.c_str()).Trim());

        propertyString = RealityDataServiceUpload::PackageProperties(properties);
        }
    else
        {
        guid = m_currentNode->node.GetInstanceId();
        }

    RealityDataServiceUpload upload = RealityDataServiceUpload(fileName, guid, propertyString, true, true, statusFunc);
    if (upload.IsValidTransfer())
        {
        upload.SetProgressCallBack(uploadProgressFunc);
        upload.SetProgressStep(std::min(1.0, (double)(100000000.0 / upload.GetFullTransferSize()) * 0.1)); 
        //anything under 100mb will update progress at 10% intervals. Largerfiles will have more frequent updates
        upload.OnlyReportErrors(true);
        const TransferReport& tReport = upload.Perform();
        Utf8String report;
        tReport.ToXml(report);
        DisplayInfo("if any files failed to upload, they will be listed here: \n", DisplayOption::Tip);
        DisplayInfo(report);
        }
    }

void RealityDataConsole::Details()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo("please navigate to an item (with cd) before using this function\n", DisplayOption::Tip);
        return;
        }
    RawServerResponse rawResponse = RawServerResponse();
    Utf8String className = m_currentNode->node.GetECClassName();

    Utf8String instanceId = m_currentNode->node.GetInstanceId();
    instanceId.ReplaceAll("/", "~2F");

    if (className == "Document")
        {
        RealityDataDocumentByIdRequest documentReq = RealityDataDocumentByIdRequest(instanceId);
        RealityDataDocumentPtr document = RealityDataService::Request(documentReq, rawResponse);

        if (document == nullptr)
            {
            DisplayInfo("There was an error retrieving information for this item\n", DisplayOption::Error);
            return;
            }

        DisplayInfo(Utf8PrintfString(" Document       : %s\n", document->GetName()));
        DisplayInfo(Utf8PrintfString(" Container name : %s\n", document->GetContainerName()));
        DisplayInfo(Utf8PrintfString(" Id             : %s\n", document->GetId()));
        DisplayInfo(Utf8PrintfString(" Folder Id      : %s\n", document->GetFolderId()));
        DisplayInfo(Utf8PrintfString(" Access Url     : %s\n", document->GetAccessUrl()));
        DisplayInfo(Utf8PrintfString(" RealityData Id : %s\n", document->GetRealityDataId()));
        DisplayInfo(Utf8PrintfString(" ContentType    : %s\n", document->GetContentType()));
        DisplayInfo(Utf8PrintfString(" Size           : %lu\n", document->GetSize()));
        }
    else if (className == "Folder")
        {
        RealityDataFolderByIdRequest folderReq = RealityDataFolderByIdRequest(instanceId);
        RealityDataFolderPtr folder = RealityDataService::Request(folderReq, rawResponse);

        if (folder == nullptr)
            {
            DisplayInfo("There was an error retrieving information for this item\n", DisplayOption::Error);
            return;
            }

        DisplayInfo(Utf8PrintfString("Folder         : %s\n", folder->GetName()));
        DisplayInfo(Utf8PrintfString("Parent folder  : %s\n", folder->GetParentId()));
        DisplayInfo(Utf8PrintfString("RealityData Id : %s\n", folder->GetRealityDataId()));
        }
    else if (className == "RealityData")
        {
        RealityDataByIdRequest idReq = RealityDataByIdRequest(instanceId);
        RealityDataPtr entity = RealityDataService::Request(idReq, rawResponse);

        if (entity == nullptr)
            {
            DisplayInfo("There was an error retrieving information for this item\n", DisplayOption::Error);
            return;
            }

        DisplayInfo(Utf8PrintfString(" Id                 : %s\n", entity->GetIdentifier()));
        DisplayInfo(Utf8PrintfString(" OrganizationId     : %s\n", entity->GetOrganizationId()));
        DisplayInfo(Utf8PrintfString(" Container name     : %s\n", entity->GetContainerName()));
        DisplayInfo(Utf8PrintfString(" RealityData name   : %s\n", entity->GetName()));
        DisplayInfo(Utf8PrintfString(" Dataset            : %s\n", entity->GetDataset()));
        DisplayInfo(Utf8PrintfString(" Group              : %s\n", entity->GetGroup()));
        DisplayInfo(Utf8PrintfString(" Description        : %s\n", entity->GetDescription()));
        DisplayInfo(Utf8PrintfString(" Root document      : %s\n", entity->GetRootDocument()));
        DisplayInfo(Utf8PrintfString(" Size (kb)          : %lu\n", entity->GetTotalSize()));
        DisplayInfo(Utf8PrintfString(" Classification     : %s\n", entity->GetClassificationTag()));
        DisplayInfo(Utf8PrintfString(" Type               : %s\n", entity->GetRealityDataType()));
        DisplayInfo(Utf8PrintfString(" Streamed           : %s\n", entity->IsStreamed() ? "true" : "false"));
        DisplayInfo(Utf8PrintfString(" Footprint          : %s\n", entity->GetFootprintString()));
        DisplayInfo(Utf8PrintfString(" ThumbnailDocument  : %s\n", entity->GetThumbnailDocument()));
        DisplayInfo(Utf8PrintfString(" MetadataUrl        : %s\n", entity->GetMetadataUrl()));
        DisplayInfo(Utf8PrintfString(" UltimateId         : %s\n", entity->GetUltimateId()));
        DisplayInfo(Utf8PrintfString(" UltimateSite       : %s\n", entity->GetUltimateSite()));
        DisplayInfo(Utf8PrintfString(" Copyright          : %s\n", entity->GetCopyright()));
        DisplayInfo(Utf8PrintfString(" TermsOfUse         : %s\n", entity->GetTermsOfUse()));
        DisplayInfo(Utf8PrintfString(" AccuracyInMeters   : %s\n", entity->GetAccuracy()));
        DisplayInfo(Utf8PrintfString(" ResolutionInMeters : %s\n", entity->GetResolution()));
        DisplayInfo(Utf8PrintfString(" Visibility         : %s\n", entity->GetVisibilityTag()));
        DisplayInfo(Utf8PrintfString(" Listable           : %s\n", entity->IsListable() ? "true" : "false"));
        DisplayInfo(Utf8PrintfString(" Modified timestamp : %s\n", entity->GetModifiedDateTime().ToString()));
        DisplayInfo(Utf8PrintfString(" Created timestamp  : %s\n", entity->GetCreationDateTime().ToString()));
        DisplayInfo(Utf8PrintfString(" OwnedBy            : %s\n", entity->GetOwner()));
        }
    }

void RealityDataConsole::FileAccess()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo("Must select a RealityData, first\n", DisplayOption::Error);
        return;
        }

    AzureHandshake* handshake;
    if (m_lastInput.ContainsI("w"))
        handshake = new AzureHandshake(m_currentNode->node.GetInstanceId(), true);
    else
        handshake = new AzureHandshake(m_currentNode->node.GetInstanceId(), false);
    DisplayInfo(Utf8PrintfString("%s\n", handshake->GetHttpRequestString()));
    delete handshake;
    }

void RealityDataConsole::AzureAddress()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo("Must select a RealityData, first\n", DisplayOption::Error);
        return;
        }

    AzureHandshake* handshake;
    if (m_lastInput.ContainsI("w"))
        handshake = new AzureHandshake(m_currentNode->node.GetRootId(), true);
    else
        handshake = new AzureHandshake(m_currentNode->node.GetRootId(), false);

    RawServerResponse idResponse = RawServerResponse();
    RealityDataByIdRequest idReq = RealityDataByIdRequest(m_currentNode->node.GetRootId());
    RealityDataPtr entity = RealityDataService::Request(idReq, idResponse);

    if (entity == nullptr)
        {
        DisplayInfo("There was an error retrieving information for this item\n", DisplayOption::Error);
        return;
        }

    RawServerResponse handshakeResponse = RealityDataService::BasicRequest((RealityDataUrl*)handshake);
    Utf8String azureServer;
    Utf8String azureToken;
    int64_t tokenTimer;
    BentleyStatus handshakeStatus = handshake->ParseResponse(handshakeResponse.body, azureServer, azureToken, tokenTimer);
    delete handshake;
    Utf8String rootDocument = entity->GetRootDocument();

    if (handshakeStatus != BentleyStatus::SUCCESS)
        {
        DisplayInfo("Failure retrieving Azure adress\n", DisplayOption::Error);
        return;
        }
    else if (rootDocument.length() > 0)
        DisplayInfo(Utf8PrintfString("%s/%s?%s\n", azureServer, rootDocument, azureToken));
    else
        DisplayInfo(Utf8PrintfString("%s?%s\n", azureServer, azureToken));
    }

void RealityDataConsole::ChangeProps()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo("Must select a RealityData, first\n", DisplayOption::Error);
        return;
        }
    else if (m_currentNode->node.GetECClassName() != "RealityData")
        {
        DisplayInfo("can only change properties of RealityData at root, use \"cd ..\" to navigate back\n", DisplayOption::Error);
        return;
        }

    Utf8String input;
    DisplayInfo("set properties from the list, use the -Finish- option to send the update\n", DisplayOption::Tip);
    std::string str;
    Utf8String propertyString = "";
    Utf8String value;
    while (!input.Equals("-Finish-"))
        {
        Choice(m_realityDataProperties, input);
        if (input.Equals("-Finish-"))
            break;
        else
            {
            DisplayInfo(Utf8PrintfString("Input value for %s\n", input));

            std::getline(*s_inputSource, str);
            if (propertyString.length() > 0)
                propertyString.append(",");

            value = Utf8String(str.c_str()).Trim();
            if (input.Equals("Listable") || input.Equals("Streamed"))
                {
                if (value.EqualsI("false")) // a little cumbersome but forces proper format of boolean values
                    propertyString.append(Utf8PrintfString("\"%s\" : false", input));
                else if (value.EqualsI("true"))
                    propertyString.append(Utf8PrintfString("\"%s\" : true", input));
                else
                    DisplayInfo(Utf8PrintfString("%s is boolean. Value must be true or false\n", input), DisplayOption::Error);
                }
            else
                propertyString.append(Utf8PrintfString("\"%s\" : \"%s\"", input, value));
            }
        }

    if(propertyString.empty())
        return Details();

    RealityDataChangeRequest changeReq = RealityDataChangeRequest(m_currentNode->node.GetRootId(), propertyString);

    RawServerResponse changeResponse = RawServerResponse();
    Utf8String response = RealityDataService::Request(changeReq, changeResponse);

    Json::Value instances(Json::objectValue);
    if ((changeResponse.status != RequestStatus::OK) || !Json::Reader::Parse(changeResponse.body, instances) || instances.isMember("errorMessage"))
        DisplayInfo(instances["errorMessage"].asString(), DisplayOption::Error);
    else
        Details();
    }

void RealityDataConsole::MassUnlink()
    {
    Utf8StringCR projectId = RealityDataService::GetProjectId();
    DisplayInfo(Utf8PrintfString("Unlinking all %d entries. Please be patient...\n", m_serverNodes.size()), DisplayOption::Tip);
    if(!projectId.empty())
        {
        RealityDataRelationshipDelete relReq = RealityDataRelationshipDelete("", "");//dummy

        RawServerResponse relationResponse = RawServerResponse();
        WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());

        for (int i = 0; i < m_serverNodes.size(); ++i)
            {
            relReq = RealityDataRelationshipDelete(m_serverNodes[i].GetInstanceId(), projectId);
            WSGRequest::GetInstance().PerformRequest(relReq, relationResponse, RealityDataService::GetVerifyPeer());
            if (relationResponse.body.ContainsI("errorMessage"))
                DisplayInfo(Utf8PrintfString("unlink RD %s from project %s failed with error:\n%s\n", m_serverNodes[i].GetInstanceId(), projectId, relationResponse.body), DisplayOption::Error);
            }
        }
    else
        DisplayInfo("No ProjectId set; or ProjectId set incorrectly. Operation may not work correctly.\n", DisplayOption::Error);
     
    DisplayInfo("Mass Unlink Complete\n", DisplayOption::Tip);
    }

void RealityDataConsole::ForceMassUnlink()
    {
    DisplayInfo(Utf8PrintfString("Attempting to unlink all %d entries from all their relationships. Please be extra patient...\n", m_serverNodes.size()), DisplayOption::Tip);
    
    RawServerResponse projectResponse = RawServerResponse();

    RealityDataProjectRelationshipByRealityDataIdRequest idReq = RealityDataProjectRelationshipByRealityDataIdRequest("");
    bvector<RealityDataProjectRelationshipPtr> entities;

    Utf8String id;
    RealityDataRelationshipDelete relReq = RealityDataRelationshipDelete("", "");//dummy
    RawServerResponse relationResponse = RawServerResponse();
    WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());

    for (int i = 0; i < m_serverNodes.size(); ++i)
        {
        idReq = RealityDataProjectRelationshipByRealityDataIdRequest(m_serverNodes[i].GetInstanceId());
        entities = RealityDataService::Request(idReq, projectResponse);

        for(RealityDataProjectRelationshipPtr entity : entities)
            {
            relReq = RealityDataRelationshipDelete(m_serverNodes[i].GetInstanceId(), entity->GetRelatedId());
            WSGRequest::GetInstance().PerformRequest(relReq, relationResponse, RealityDataService::GetVerifyPeer());
            if (relationResponse.body.ContainsI("errorMessage"))
                DisplayInfo(Utf8PrintfString("unlink RD %s from project %s failed with error:\n%s\n", m_serverNodes[i].GetInstanceId(), entity->GetRelatedId(), relationResponse.body), DisplayOption::Error);
            }
        }
    
    DisplayInfo("Mass Unlink Complete\n", DisplayOption::Tip);
    }

void RealityDataConsole::MassDelete()
    {
    std::string str, str2;

    DisplayInfo(Utf8PrintfString("Using this command like this will delete ALL %d entries, from your most recent List/Dir command.\n"
        "(NOTE: If the RealityData is linked to any project other than your own, it will not be deleted but it will be unlinked from your project)\n", m_serverNodes.size()), DisplayOption::Question);
    DisplayInfo("Are you SURE? [ y / n ]", DisplayOption::Question);
    std::getline(*s_inputSource, str);
    if (strstr(str.c_str(), "quit"))
        {
        m_lastCommand = Command::Quit;
        return;
        }
    else if (str != "y")
        return;

    str2 = Utf8PrintfString("%d", m_serverNodes.size()).c_str();

    DisplayInfo("To authenticate that you REALLY want to do this, please enter the number of entries to delete, as displayed above\n", DisplayOption::Question);
    std::getline(*s_inputSource, str);
    if (strstr(str.c_str(), "quit"))
        {
        m_lastCommand = Command::Quit;
        return;
        }
    else if (str != str2 && str != "force")
        return;
    else if (str == "force")
        ForceMassUnlink();
    else
        MassUnlink();

    DisplayInfo(Utf8PrintfString("Deleting all %d entries. Please be patient...\n", m_serverNodes.size()), DisplayOption::Tip);

    RawServerResponse rawResponse = RawServerResponse();
    RealityDataDelete realityDataReq = RealityDataDelete(""); //dummy

    bvector<Utf8String> errors = bvector<Utf8String>();

    for(int i = 0; i < m_serverNodes.size(); ++i)
        {
        realityDataReq = RealityDataDelete(m_serverNodes[i].GetInstanceId());
        rawResponse = RealityDataService::BasicRequest(&realityDataReq);
        if (rawResponse.body.Contains("errorMessage"))
            errors.push_back(Utf8PrintfString("%s failed to delete with error:\n%s\n",m_serverNodes[i].GetInstanceId(), rawResponse.body));
        rawResponse.clear();
        }
    
    if(!errors.empty())
        {
        DisplayInfo("There was an error removing the following items:\n", DisplayOption::Error);
        for(int i = 0 ; i < errors.size() ; ++ i ) 
            DisplayInfo(errors[i], DisplayOption::Error);
        }

    DisplayInfo("Mass Delete Complete\n", DisplayOption::Tip);

    List();

    }

void RealityDataConsole::Delete()
    {
    if (m_currentNode == nullptr)
        return MassDelete();

    Utf8String className = m_currentNode->node.GetECClassName();

    Utf8String instanceId = m_currentNode->node.GetInstanceId();
    instanceId.ReplaceAll("/", "~2F");
    std::string str;
    RawServerResponse rawResponse = RawServerResponse();

    if (className == "Document")
        {
        DisplayInfo(Utf8PrintfString("Deleting Document %s.\nConfirm? [ y / n ]", m_currentNode->node.GetInstanceId()), DisplayOption::Question);
        std::getline(*s_inputSource, str);
        if(strstr(str.c_str(), "quit"))
            {      
            m_lastCommand = Command::Quit;
            return;
            }
        else if (str != "y")
            return;

        RealityDataDeleteDocument documentReq = RealityDataDeleteDocument(instanceId);
        rawResponse = RealityDataService::BasicRequest(&documentReq);
        }
    else if (className == "Folder")
        {
        DisplayInfo(Utf8PrintfString("Deleting Folder %s. All documents contained within will also be deleted.\nConfirm? [ y / n ]", m_currentNode->node.GetInstanceId()), DisplayOption::Question);
        std::getline(*s_inputSource, str);
        if (strstr(str.c_str(), "quit"))
            {
            m_lastCommand = Command::Quit;
            return;
            }
        else if (str != "y")
            return;

        RealityDataDeleteFolder folderReq = RealityDataDeleteFolder(instanceId);
        rawResponse = RealityDataService::BasicRequest(&folderReq);
        }
    else if (className == "RealityData")
        {
        DisplayInfo(Utf8PrintfString("Deleting RealityData %s. All folders and documents contained within will also be deleted.\n", m_currentNode->node.GetInstanceId()), DisplayOption::Question);
        DisplayInfo("A RealityData can only be deleted if there are no project relationships attached to it.\nConfirm ? [y / n]", DisplayOption::Question);
        std::getline(*s_inputSource, str);
        if (strstr(str.c_str(), "quit"))
            {
            m_lastCommand = Command::Quit;
            return;
            }
        else if (str != "y")
            return;

        RealityDataDelete realityDataReq = RealityDataDelete(instanceId);
        rawResponse = RealityDataService::BasicRequest(&realityDataReq);
        }

    if (rawResponse.body.Contains("errorMessage"))
        {
        Json::Value instances(Json::objectValue);
        if (Json::Reader::Parse(rawResponse.body, instances) && instances.isMember("errorMessage"))
            rawResponse.body = instances["errorMessage"].asString();
        DisplayInfo(Utf8PrintfString("There was an error removing this item\n%s", rawResponse.body), DisplayOption::Error);
        }
    else
        {
        DisplayInfo("item deleted\n", DisplayOption::Tip);
        m_lastInput = "..";
        ChangeDir();
        }
    }

void RealityDataConsole::Filter()
    {
    Utf8String filter = "";
    Utf8String value;
    std::string str;
    while (filter != "-Finish-")
        {
        DisplayInfo("\n---", DisplayOption::Error); DisplayInfo("---", DisplayOption::Tip); DisplayInfo("---", DisplayOption::Question); DisplayInfo("---", DisplayOption::Tip); DisplayInfo("---\n", DisplayOption::Error);
        DisplayInfo("Current Filters:\n", DisplayOption::Tip);
        DisplayInfo(Utf8PrintfString("Name : %s\n", m_nameFilter));
        DisplayInfo(Utf8PrintfString("Group : %s\n", m_groupFilter));
        DisplayInfo(Utf8PrintfString("Type : %s\n", m_typeFilter));
        DisplayInfo(Utf8PrintfString("OwnedBy : %s\n", m_ownerFilter));
        DisplayInfo(Utf8PrintfString("ProjectId : %s\n", m_projectFilter));
        DisplayInfo(Utf8PrintfString("Fuzzy Filter : %s\n", m_queryFilter));
        DisplayInfo("---", DisplayOption::Error); DisplayInfo("---", DisplayOption::Tip); DisplayInfo("---", DisplayOption::Question); DisplayInfo("---", DisplayOption::Tip); DisplayInfo("---\n\n", DisplayOption::Error);
        DisplayInfo("set filters from the list, use the -Finish- option to return\n", DisplayOption::Tip);

        Choice(m_filterProperties, filter);
        if (filter == "-Finish-")
            break;

        if (filter.Equals("Fuzzy Filter"))
            DisplayInfo("\nSet Fuzzy Filter (Enter blank field to remove filter)\nThis Filter searches every property of the RealityData for the specified value (case insensitive)\n", DisplayOption::Tip);
        else
            DisplayInfo(Utf8PrintfString("Set filter for %s (Enter blank field to remove filter). Careful, filters are case sensitive\n", filter), DisplayOption::Tip);

        std::getline(*s_inputSource, str);
        value = Utf8String(str.c_str());
        if (value.EqualsI("Quit"))
            {
            m_lastCommand = Command::Quit;
            return;
            }
        else if (filter.Equals("Name"))
            m_nameFilter = value;
        else if (filter.Equals("Group"))
            m_groupFilter = value;
        else if (filter.Equals("Type"))
            m_typeFilter = value;
        else if (filter.Equals("OwnedBy"))
            m_ownerFilter = value;
        else if (filter.Equals("Fuzzy Filter"))
            m_queryFilter = value;
        else if (filter.Equals("ProjectId"))
            m_projectFilter = value;
        }
    }

void RealityDataConsole::Relationships()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo("Please navigate to an item (with cd) before using this function\n", DisplayOption::Tip);
        return;
        }
    RawServerResponse projectResponse = RawServerResponse();

    Utf8String instanceId = m_currentNode->node.GetInstanceId();
    instanceId.ReplaceAll("/", "~2F");

    RealityDataProjectRelationshipByRealityDataIdRequest idReq = RealityDataProjectRelationshipByRealityDataIdRequest(instanceId);
    bvector<RealityDataProjectRelationshipPtr> entities = RealityDataService::Request(idReq, projectResponse);

    if (projectResponse.status == RequestStatus::BADREQ)
        {
        DisplayInfo("There was an error retrieving this information\n", DisplayOption::Error);
        return;
        }
    else if (entities.size() == 0)
        {
        DisplayInfo("There seems to be no projects attached to this RealityData\n", DisplayOption::Error);
        return;
        }

    DisplayInfo("Projects attached to this RealityData\n\n");
    for (RealityDataProjectRelationshipPtr entity : entities)
        DisplayInfo(Utf8PrintfString(" ProjectId          : %s\n", entity->GetRelatedId()));
    }

void RealityDataConsole::CreateRD()
    {
    std::string input;
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    DisplayInfo("Please input value for Name\n  ?", DisplayOption::Question);
    std::getline(*s_inputSource, input);
    Utf8String name = Utf8String(input.c_str()).Trim();
    if(name.EqualsI("Quit"))
        {
        m_lastCommand = Command::Quit;
        return;
        }
    properties.Insert(RealityDataField::Name, name);

    Utf8String option;
    DisplayInfo("Please input value for Classification\n  ?", DisplayOption::Question);
    Choice(m_classificationOptions, option);
    properties.Insert(RealityDataField::Classification, option);

    DisplayInfo("Please input value for Type\n  ?", DisplayOption::Question);
    std::getline(*s_inputSource, input);
    Utf8String type = Utf8String(input.c_str()).Trim();
    if (type.EqualsI("Quit"))
        {
        m_lastCommand = Command::Quit;
        return;
        }
    properties.Insert(RealityDataField::Type, type);

    DisplayInfo("Please input value for Visibility\n  ?", DisplayOption::Question);
    Choice(m_visibilityOptions, option);
    properties.Insert(RealityDataField::Visibility, option);

    DisplayInfo("Please input value for RootDocument (relative path)\n  ?", DisplayOption::Question);
    std::getline(*s_inputSource, input);
    Utf8String rootDoc = Utf8String(input.c_str()).Trim();
    if (rootDoc.EqualsI("Quit"))
        {
        m_lastCommand = Command::Quit;
        return;
        }
    rootDoc.ReplaceAll("\\","/");
    properties.Insert(RealityDataField::RootDocument, rootDoc);

    RealityDataCreateRequest createRequest = RealityDataCreateRequest("", RealityDataServiceUpload::PackageProperties(properties));

    RawServerResponse createResponse = RawServerResponse();
    Utf8String response = RealityDataService::Request(createRequest, createResponse);

    Json::Value instance(Json::objectValue);
    Json::Reader::Parse(createResponse.body, instance);
    if (createResponse.status == RequestStatus::OK && !instance["changedInstance"].isNull() && !instance["changedInstance"]["instanceAfterChange"].isNull() && !instance["changedInstance"]["instanceAfterChange"]["instanceId"].isNull())
        {
        DisplayInfo(Utf8PrintfString("New RealityData \"%s\" created with GUID %s", name, instance["changedInstance"]["instanceAfterChange"]["instanceId"].asString()), DisplayOption::Info);
        }
    else
        {
        DisplayInfo("There was an error creating a new RealityData.", DisplayOption::Error);
        DisplayInfo(Utf8PrintfString("And message %s\n", createResponse.body), DisplayOption::Error);
        }
    }

void RealityDataConsole::Link()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo("Please navigate to a RealityData before using this command\n", DisplayOption::Tip);
        return;
        }

    DisplayInfo(Utf8PrintfString("Creating a relationship for %s\n", m_currentNode->node.GetInstanceId()), DisplayOption::Tip);
    DisplayInfo("If you wish to change this, use command \"Cancel\" to back out and use cd to change the directory\n\n", DisplayOption::Tip);
    DisplayInfo("Please enter the id of the project you would like to link this to\n ?", DisplayOption::Question);

    InterpretCommand();
    if (m_lastCommand == Command::Cancel)
        return;

    RealityDataRelationshipCreateRequest relReq = RealityDataRelationshipCreateRequest(m_currentNode->node.GetInstanceId(), m_lastInput);

    RawServerResponse relationResponse = RawServerResponse();
    Utf8String response = RealityDataService::Request(relReq, relationResponse);

    Json::Value instances(Json::objectValue);
    if ((relationResponse.status != RequestStatus::OK) || !Json::Reader::Parse(relationResponse.body, instances) || instances.isMember("errorMessage"))
        DisplayInfo(instances["errorMessage"].asString(), DisplayOption::Error);
    else
        Relationships();
    }

void RealityDataConsole::Unlink()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo("Please navigate to a RealityData before using this command\n", DisplayOption::Tip);
        return;
        }

    DisplayInfo(Utf8PrintfString("Removing a relationship for %s\n", m_currentNode->node.GetInstanceId()), DisplayOption::Tip);
    DisplayInfo("If you wish to change this, use command \"Cancel\" to back out and use cd to change the directory\n\n", DisplayOption::Tip);
    DisplayInfo("Please enter the id of the project you would like to unlink this to\n ?", DisplayOption::Question);

    InterpretCommand();
    if (m_lastCommand == Command::Cancel)
        return;

    RealityDataRelationshipDelete relReq = RealityDataRelationshipDelete(m_currentNode->node.GetInstanceId(), m_lastInput);

    RawServerResponse relationResponse = RawServerResponse();
    WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());
    WSGRequest::GetInstance().PerformRequest(relReq, relationResponse, RealityDataService::GetVerifyPeer());

    Json::Value instances(Json::objectValue);
    if ((relationResponse.status != CURLE_OK) || !Json::Reader::Parse(relationResponse.body, instances) || instances.isMember("errorMessage"))
        DisplayInfo(instances["errorMessage"].asString(), DisplayOption::Error);
    else
        Relationships();
    }

void RealityDataConsole::InputError()
    {
    DisplayInfo("Unrecognized Command. Type \"help\" for usage\n", DisplayOption::Error);
    }

void RealityDataConsole::DisplayInfo(Utf8StringCR msg, DisplayOption option)
    {
    switch (option)
        {
    case DisplayOption::Question:
        SetConsoleTextAttribute(m_hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
        break;

    case DisplayOption::Tip:
        SetConsoleTextAttribute(m_hConsole, FOREGROUND_GREEN);
        break;

    case DisplayOption::Error:
        SetConsoleTextAttribute(m_hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        break;

    case DisplayOption::Info:
    default:
        SetConsoleTextAttribute(m_hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
        break;
        }

    if (!msg.empty())
        *s_outputDestination << msg;

    // commande
    SetConsoleTextAttribute(m_hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
    }
