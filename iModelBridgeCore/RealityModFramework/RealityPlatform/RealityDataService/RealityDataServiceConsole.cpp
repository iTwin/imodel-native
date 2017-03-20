/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityDataService/RealityDataServiceConsole.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityDataService.h>
#include <RealityPlatform/WSGServices.h>

#include <stdio.h>
#include <conio.h>
#include <iomanip>
#include <sstream>
#include <iostream>

#include "RealityDataServiceConsole.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

void RealityDataConsole::InterpretCommand()
    {
    std::string str;
    std::getline(std::cin, str);
    m_lastInput = Utf8String(str.c_str()).Trim();

    bvector<Utf8String> args;
    BeStringUtilities::ParseArguments(args, m_lastInput.c_str());
    if(args.size() < 1)
        {
        DisplayInfo("missing input. Please refer to \"Help\" \n", DisplayOption::Error);
        return;
        }
    if(args.size() > 2)
        {
        DisplayInfo("too many inputs to parse. Please refer to \"Help\" \n", DisplayOption::Error);
        return;
        }

    if(args[0].EqualsI("quit"))
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
    else if (args[0].EqualsI("ChangeProps"))
        m_lastCommand = Command::ChangeProps;
    else if (args[0].EqualsI("Delete"))
        m_lastCommand = Command::Delete;
    else if (args[0].EqualsI("Filter"))
        m_lastCommand = Command::Filter;
    else if (args[0].EqualsI("Relationships"))
        m_lastCommand = Command::Relationships;
    else
        {
        m_lastCommand = Command::Error;

        if (args[0].EqualsI("index"))
            m_lastCommand = Command::ChoiceIndex;
        else if (args[0].EqualsI("value"))
            m_lastCommand = Command::ChoiceValue;
        else if (args[0].EqualsI("cd"))
            m_lastCommand = Command::ChangeDir;
        else if (args[0].EqualsI("FileAccess"))
            m_lastCommand = Command::FileAccess;
        else if (args[0].EqualsI("AzureAdress"))
            m_lastCommand = Command::AzureAdress;
        else if (args[0].EqualsI("CreateRD"))
            m_lastCommand = Command::CreateRD;
        if(m_lastCommand != Command::Error)
            {
            if (args.size() > 1)
                m_lastInput = args[1];
            else
                {
                if(m_lastCommand == Command::ChangeDir && args[0].Contains(".."))
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
    m_server( WSGServer("", true)),
    m_serverNodes(bvector<NavNode>()),
    m_machineRepos(bvector<Utf8String>()),
    m_currentNode(nullptr),
    m_nameFilter(""),
    m_groupFilter(""),
    m_typeFilter(""),
    m_ownerFilter("")
    {
    m_functionMap.Insert(Command::Help,         &RealityDataConsole::Usage);
    m_functionMap.Insert(Command::SetServer,    &RealityDataConsole::ConfigureServer);
    m_functionMap.Insert(Command::List,         &RealityDataConsole::List);
    m_functionMap.Insert(Command::ListAll,      &RealityDataConsole::ListAll);
    m_functionMap.Insert(Command::ChangeDir,    &RealityDataConsole::ChangeDir);
    m_functionMap.Insert(Command::Stat,         &RealityDataConsole::EnterpriseStat);
    m_functionMap.Insert(Command::Details,      &RealityDataConsole::Details);
    m_functionMap.Insert(Command::Download,     &RealityDataConsole::Download);
    m_functionMap.Insert(Command::Upload,       &RealityDataConsole::Upload);
    m_functionMap.Insert(Command::FileAccess,   &RealityDataConsole::FileAccess);
    m_functionMap.Insert(Command::AzureAdress,  &RealityDataConsole::AzureAdress);
    m_functionMap.Insert(Command::ChangeProps,  &RealityDataConsole::ChangeProps);
    m_functionMap.Insert(Command::Delete,       &RealityDataConsole::Delete);
    m_functionMap.Insert(Command::Filter,       &RealityDataConsole::Filter);
    m_functionMap.Insert(Command::Relationships,&RealityDataConsole::Relationships);
    m_functionMap.Insert(Command::CreateRD,     &RealityDataConsole::CreateRD);

    //commands that should never occur, within Run()
    m_functionMap.Insert(Command::Quit,         &RealityDataConsole::DummyFunction);
    m_functionMap.Insert(Command::Retry,        &RealityDataConsole::DummyFunction);
    m_functionMap.Insert(Command::Error,        &RealityDataConsole::InputError);
    m_functionMap.Insert(Command::ChoiceIndex,  &RealityDataConsole::DummyFunction);
    m_functionMap.Insert(Command::ChoiceValue,  &RealityDataConsole::DummyFunction);

    m_realityDataProperties = bvector<Utf8String>();
    //m_realityDataProperties.push_back("Id");
    m_realityDataProperties.push_back("EnterpriseId");
    //m_realityDataProperties.push_back("ContainerName");
    m_realityDataProperties.push_back("Name");
    m_realityDataProperties.push_back("Dataset");
    m_realityDataProperties.push_back("Group");
    m_realityDataProperties.push_back("Description");
    m_realityDataProperties.push_back("RootDocument");
    //m_realityDataProperties.push_back("Size");
    m_realityDataProperties.push_back("Classification");
    m_realityDataProperties.push_back("Type");
    m_realityDataProperties.push_back("Footprint");
    m_realityDataProperties.push_back("ThumbnailDocument");
    m_realityDataProperties.push_back("MetadataURL");
    m_realityDataProperties.push_back("ResolutionInMeters");
    m_realityDataProperties.push_back("AccuracyInMeters");
    m_realityDataProperties.push_back("Visibility");
    m_realityDataProperties.push_back("Listable");
    //m_realityDataProperties.push_back("ModifiedTimestamp");
    //m_realityDataProperties.push_back("CreatedTimestamp");
    m_realityDataProperties.push_back("OwnedBy");
    m_realityDataProperties.push_back("-Finish-");

    m_filterProperties = bvector<Utf8String>();
    m_filterProperties.push_back("Name");
    m_filterProperties.push_back("Group");
    m_filterProperties.push_back("Type");
    m_filterProperties.push_back("OwnedBy");
    m_filterProperties.push_back("-Finish-");

    m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);       // see the methods Disp...()
    }

void RealityDataConsole::Choice(bvector<Utf8String> options, Utf8StringR input)
    {
    PrintResults(options);
    DisplayInfo ("an option can be selected by its Index or by its Value\n", DisplayOption::Question);
    DisplayInfo (" by using either \"Index #\" or \"Value NameOfValue\" \n  ?", DisplayOption::Question);
    
    uint64_t choice;
    InterpretCommand();
    switch(m_lastCommand)
        {
        case Command::Quit:
            break;
        case Command::ChoiceIndex:
            {
            if (BeStringUtilities::ParseUInt64(choice, m_lastInput.c_str()) == BentleyStatus::SUCCESS)
                {
                if(choice >= options.size())
                    {
                    DisplayInfo(Utf8PrintfString("Invalid Selection, selected index not between 0 and %lu \n", (options.size() - 1)), DisplayOption::Error);
                    m_lastCommand = Command::Retry;
                    }
                else
                    input = options[choice];
                }
            else
                {
                DisplayInfo("Could not extract integer from provided input...\n", DisplayOption::Error);
                m_lastCommand = Command::Retry;
                }
            break;
            }
        case Command::ChoiceValue:
            {
            input = m_lastInput;
            break;
            }
        case Command::Error:
            {
            DisplayInfo("input error, please enter your choice in one of the following formats\n", DisplayOption::Error);
            DisplayInfo(" Index #  or Value NameOfValue\n", DisplayOption::Error);
            m_lastCommand = Command::Retry;
            }
        }
    if (m_lastCommand == Command::Retry)
        return Choice(options, input);
    }

int main(int argc, char* argv[])
    {
    SetConsoleTitle("RealityDataService Navigator");

    RealityDataConsole console = RealityDataConsole();
    console.Run();
    }

void RealityDataConsole::Run()
    {
    ConfigureServer();
    while(m_lastCommand != Command::Quit)
        {
        if(m_currentNode != nullptr)
            DisplayInfo (Utf8PrintfString("%s", m_currentNode->node.GetInstanceId()), DisplayOption::Tip);
            DisplayInfo ("> ", DisplayOption::Tip);
        InterpretCommand();
        (this->*(m_functionMap[m_lastCommand]))();
        }
    }

void RealityDataConsole::Usage()
    {
    DisplayInfo ("  RealityDataConsole tool for RDS V1.0\n\n");
    DisplayInfo ("  Avalaible Commands (case insensitive):\n");
    DisplayInfo ("  Quit        Exit the application\n");
    DisplayInfo ("  Retry       (during a multi-step operation) Restart current operation\n");
    DisplayInfo ("  Help        Print current Display\n");
    DisplayInfo ("  SetServer   Change server settings (server url, repository and schema)\n");
    DisplayInfo ("  List        List all subfiles/folders for the given location on your server\n");
    DisplayInfo ("  Dir         same as List\n");
    DisplayInfo ("  Filter      filters RealityDatas returned from a List/Dir command\n");
    DisplayInfo ("  cd          Change current location. Must be called in one of the following ways\n");
    DisplayInfo ("  cd [number] navigates to node at the given index, as specified in the most recent List command\n");
    DisplayInfo ("  cd ..       go up one level\n");
    DisplayInfo ("  ListAll     List every file beneath the current location (paged)\n");
    DisplayInfo ("  Details     show the details for the location\n");
    DisplayInfo ("  Stat        show enterprise statistics\n");
    DisplayInfo ("  Download    Download files from the current location on the server\n");
    DisplayInfo ("  Upload      Upload files to the server\n");
    DisplayInfo ("  FileAccess  Prints the URL to use if you wish to request an azure file access\n");
    DisplayInfo ("  AzureAdress Prints the URL to use\n");
    DisplayInfo ("  ChangeProps Modify the properties of a RealityData\n");
    DisplayInfo ("  Relationships Show all projects attached to this RealityData\n");
    DisplayInfo ("  CreateRD    Create a new RealityData (must provide a name)\n");
    DisplayInfo ("  Delete      Delete a RealityData, Folder or single Document\n");
    }

void RealityDataConsole::PrintResults(bvector<Utf8String> results)
    {
    std::stringstream index;
    Utf8String fullOption;
    DisplayInfo("Index \t Value\n");
    std::string str;
    for (int i = 0; i < results.size(); ++i)
        {
        DisplayInfo(Utf8PrintfString("%5d \t %s\n", i, results[i]));
        }
    }

void RealityDataConsole::ConfigureServer()
    {
    DisplayInfo("Welcome to the RealityDataService Navigator. Please enter your server name\n", DisplayOption::Question);
    DisplayInfo("  Example format : dev-realitydataservices-eus.cloudapp.net\n  ?", DisplayOption::Question);
    Utf8String server;
    std::string input;
    std::getline(std::cin, input);
    server = Utf8String(input.c_str()).Trim();
    bool verifyCertificate = false;
    while(1)
        {
        DisplayInfo("Does this server have a recognized certificate? [ y / n ]  ?", DisplayOption::Question);
        Utf8String temp;
        std::getline(std::cin, input);
        temp = Utf8String(input.c_str()).Trim();
        if(temp.EqualsI("y"))
            {
            verifyCertificate = true;
            break;
            }
        else if(temp.EqualsI("n"))
            {
            verifyCertificate = false;
            break;
            }
        else
            DisplayInfo("invalid answer\n", DisplayOption::Error);
        }

    DisplayInfo("Retrieving version information. One moment...\n\n", DisplayOption::Tip);

    m_server = WSGServer(server, verifyCertificate);
    Utf8String version = m_server.GetVersion();

    if(version.length() == 0)
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
    
    bvector<Utf8String> repoNames = m_server.GetRepositories();
    m_lastCommand = Command::Error;
    if(repoNames.size() == 0)
        {
        DisplayInfo("There was an error contacting the server. No repositories found\n --> Is your ConnectClient configured correctly?\n", DisplayOption::Error);
        while(m_lastCommand != Command::Retry && m_lastCommand != Command::Quit)
            {
            DisplayInfo("\"Retry\" to try with a different server; \"Quit\" to exit\n", DisplayOption::Error);
            InterpretCommand();
            if(m_lastCommand == Command::Retry)
                return ConfigureServer();
            else if(m_lastCommand == Command::Quit)
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
        DisplayInfo("please select a repository from the following options\n", DisplayOption::Question);
        Choice(repoNames, repo);
        switch(m_lastCommand)
            {
        case Command::Quit:
            return;
        case Command::Retry:
            return ConfigureServer();
            }
        }

    if(repo.length() > 0)
        {
        DisplayInfo("\n");

        bvector<Utf8String> schemaNames = m_server.GetSchemaNames(repo);

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
            switch(m_lastCommand)
                {
                case Command::Quit:
                    return;
                case Command::Retry:
                    return ConfigureServer();
                }
            }

        if(schema.length() > 0)
            {
            RealityDataService::SetServerComponents(server, version, repo, schema);
            m_server = WSGServer(RealityDataService::GetServerName(), verifyCertificate);
            }
        }

    DisplayInfo("Server successfully configured, ready for use. Type \"help\" for list of commands\n", DisplayOption::Tip);
    }

void RealityDataConsole::List()
    {
    m_serverNodes.clear();
    Utf8String nodeString;
    bvector<Utf8String> nodeStrings;

    if(m_currentNode == nullptr)
        return ListRoots();
    else
        m_serverNodes = NodeNavigator::GetInstance().GetChildNodes(m_server, RealityDataService::GetRepoName(), m_currentNode->node);

    for (NavNode node : m_serverNodes)
        {
        nodeString = node.GetLabel();
        if(node.GetClassName() == "Folder")
            nodeString.append("/");
        nodeStrings.push_back(nodeString);
        }

    PrintResults(nodeStrings);
    }

void RealityDataConsole::ListRoots()
    {
    RealityDataListByEnterprisePagedRequest enterpriseReq = RealityDataListByEnterprisePagedRequest("", 0, 2500);

    bvector<Utf8String> properties = bvector<Utf8String>();
    if (m_nameFilter.length() > 0)
        properties.push_back(RealityDataFilterCreator::FilterByName(m_nameFilter));
    if (m_groupFilter.length() > 0)
        properties.push_back(RealityDataFilterCreator::FilterByGroup(m_groupFilter));
    if (m_typeFilter.length() > 0)
        properties.push_back(RealityDataFilterCreator::FilterByType(m_typeFilter));
    if (m_ownerFilter.length() > 0)
        properties.push_back(RealityDataFilterCreator::FilterByOwner(m_ownerFilter));
    if(properties.size() > 0)
        enterpriseReq.SetFilter(RealityDataFilterCreator::GroupFiltersAND(properties));

    RequestStatus status = RequestStatus::SUCCESS;
    bvector<RealityDataPtr> enterpriseVec = bvector<RealityDataPtr>();
    bvector<RealityDataPtr> partialVec;

    partialVec = RealityDataService::Request(enterpriseReq, status);
    while(partialVec.size() > 0)
        {
        enterpriseVec.insert(enterpriseVec.end(), partialVec.begin(), partialVec.end());
        partialVec = RealityDataService::Request(enterpriseReq, status);
        }
    bvector<Utf8String> nodes = bvector<Utf8String>();

    Utf8String schema = RealityDataService::GetSchemaName();
    for(RealityDataPtr rData : enterpriseVec)
        {
        nodes.push_back(Utf8PrintfString("%-30s (%s) %s", rData->GetName(), rData->IsListable() ? "Lst" : " - ", rData->GetIdentifier()));
        m_serverNodes.push_back(NavNode(schema, rData->GetIdentifier(), "ECObjects", "RealityData"));
        }

    PrintResults(nodes);
    }

void RealityDataConsole::ListAll()
    {
    if(m_currentNode == nullptr)
        {   
        DisplayInfo("Please navigate to a RealityData or Folder before using this command\n", DisplayOption::Tip);
        return;
        }

    AzureHandshake* handshake = new AzureHandshake(m_currentNode->node.GetInstanceId(), false);
    RealityDataService::RequestToJSON((RealityDataUrl*)handshake, handshake->GetJsonResponse());
    Utf8String azureServer;
    Utf8String azureToken;
    int64_t tokenTimer;
    BentleyStatus handshakeStatus = handshake->ParseResponse(azureServer, azureToken, tokenTimer);
    delete handshake;
    if (handshakeStatus != BentleyStatus::SUCCESS)
        {
        DisplayInfo("Failure retrieving Azure token\n", DisplayOption::Error);
        return;
        }

    AllRealityDataByRootId rdsRequest = AllRealityDataByRootId(m_currentNode->node.GetInstanceId());
    RequestStatus status;
    bvector<bpair<WString, uint64_t>> filesInRepo = RealityDataService::Request(rdsRequest, status);
    
    DisplayInfo (Utf8PrintfString(" %lu files in selection.\n", filesInRepo.size()), DisplayOption::Tip);
    DisplayInfo ("these will be displayed, 20 at a time. Input \"Cancel\" to quit at any time, otherwise press enter to proceed to the next page\n", DisplayOption::Tip);

    std::string str;
    size_t placeholder = 0;
    size_t step;
    size_t size = filesInRepo.size();
    while (m_lastCommand != Command::Cancel && placeholder < size)
        {
        std::getline(std::cin, str);
        if(Utf8String(str.c_str()).Trim().EqualsI("Cancel"))
            m_lastCommand = Command::Cancel;
        else
            {
            step = (size < (placeholder + 20)) ? size : placeholder + 20;
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
            DisplayInfo("Already at root\n", DisplayOption::Tip);
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
    if (BeStringUtilities::ParseUInt64(choice, m_lastInput.c_str()) != BentleyStatus::SUCCESS)
        {
        DisplayInfo("Could not extract integer from provided input...\n", DisplayOption::Error);
        return;
        }

    if(m_serverNodes.size() == 0)
        {   
        DisplayInfo("Need to use \"List\" or \"Dir\" before using this command\n", DisplayOption::Tip);
        DisplayInfo("If you have already done this, there may be no listable locations to navigate to\n", DisplayOption::Tip);
        }

    if(choice < (uint64_t)m_serverNodes.size())
        {
        NodeList* newNode = new NodeList();
        newNode->node = m_serverNodes[choice];
        newNode->parentNode = m_currentNode;
        if(m_currentNode != nullptr)
            m_currentNode->childNode = newNode;
        m_currentNode = newNode;
        m_serverNodes.clear();
        }
    else
        DisplayInfo(Utf8PrintfString("Invalid Selection, selected index not between 0 and %lu\n", (m_serverNodes.size() - 1)), DisplayOption::Error);
    }

void RealityDataConsole::EnterpriseStat()
    {
    RequestStatus status;
    RealityDataEnterpriseStatRequest* ptt = new RealityDataEnterpriseStatRequest("");
    uint64_t NbRealityData;
    uint64_t TotalSizeKB;
    RealityDataService::Request(*ptt, &NbRealityData, &TotalSizeKB, status);

    DisplayInfo ("Enterprise statistics: \n");
    DisplayInfo (Utf8PrintfString("   NbRealityData: %lu\n", NbRealityData));
    DisplayInfo (Utf8PrintfString("   TotalSize(KB): %lu\n\n", TotalSizeKB));
    }

static void downloadProgressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    char progressString[1024];
    sprintf(progressString, "percentage of files downloaded : %f\r", repoProgress * 100.0);
    std::cout << progressString;
	
//DMxx    RealityDataConsole::DisplayInfo (Utf8PrintfString("percentage of files downloaded : %f\r", repoProgress * 100.0)); //, RealityDataConsole::Tip);
    }

static void uploadProgressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    char progressString[1024];
    //sprintf(progressString, "%s upload percent : %f", filename.c_str(), progress * 100.0f);
    sprintf(progressString, "upload percent : %f\r", repoProgress * 100.0);
    std::cout << progressString ;
	
//DMxx    RealityDataConsole::DisplayInfo(Utf8PrintfString("upload percent : %f\r", repoProgress * 100.0)); //, RealityDataConsole::Tip);
    }

void RealityDataConsole::Download()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo ("Please navigate to a RealityData or Folder before using this command\n", DisplayOption::Tip);
        return;
        }

    DisplayInfo (Utf8PrintfString("Downloading from %s\n", m_currentNode->node.GetLabel()), DisplayOption::Tip);
    DisplayInfo ("if you wish to change this, use command \"Cancel\" to back out and use cd to change the directory\n\n", DisplayOption::Tip);
    DisplayInfo ("please enter the destination folder on the local machine (must be existing folder)\n ?", DisplayOption::Question);
    
    InterpretCommand();
    if(m_lastCommand == Command::Cancel)
        return;

    BeFileName fileName = BeFileName(m_lastInput);
    if(!fileName.DoesPathExist())
        {
        DisplayInfo ("could not validate specified path. Please verify that the folder exists and try again\n", DisplayOption::Error);
        return;
        }

    RealityDataServiceDownload download = RealityDataServiceDownload(fileName, m_currentNode->node.GetInstanceId());
    download.SetProgressCallBack(downloadProgressFunc);
    download.SetProgressStep(0.1);
    download.OnlyReportErrors(true);
    TransferReport* tReport = download.Perform();
    Utf8String report;
    tReport->ToXml(report);
    DisplayInfo ("if any files failed to download, they will be listed here: \n");
    DisplayInfo (Utf8PrintfString("%s\n", report));
    }

void RealityDataConsole::Upload()
    {
    DisplayInfo("please enter the source folder on the local machine (must be existing folder)\n  ?", DisplayOption::Question);
    
    InterpretCommand();
    if (m_lastCommand == Command::Cancel)
        return;

    BeFileName fileName = BeFileName(m_lastInput);
    if (!fileName.DoesPathExist())
        {
        DisplayInfo("Could not validate specified path. Please verify that the folder exists and try again\n", DisplayOption::Error);
        return;
        }

    DisplayInfo("please input GUID for upload\n  ?", DisplayOption::Error);
    std::string input;
    std::getline(std::cin, input);
    Utf8String guid = Utf8String(input.c_str()).Trim();

    bmap<RealityDataField, Utf8String> properties;
    DisplayInfo("please input value for Name\n  ?", DisplayOption::Error);
    std::getline(std::cin, input);
    properties.Insert(RealityDataField::Name, Utf8String(input.c_str()).Trim());

    DisplayInfo("please input value for Classification\n  ?", DisplayOption::Error);
    std::getline(std::cin, input);
    properties.Insert(RealityDataField::Classification, Utf8String(input.c_str()).Trim());

    DisplayInfo("please input value for Type\n  ?", DisplayOption::Error);
    std::getline(std::cin, input);
    properties.Insert(RealityDataField::Type, Utf8String(input.c_str()).Trim());

    Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

    RealityDataServiceUpload upload = RealityDataServiceUpload(fileName, guid, propertyString, true, true);
    upload.SetProgressCallBack(uploadProgressFunc);
    upload.SetProgressStep(0.1);
    upload.OnlyReportErrors(true);
    TransferReport* tReport = upload.Perform();
    Utf8String report;
    tReport->ToXml(report);
    DisplayInfo("if any files failed to upload, they will be listed here: \n", DisplayOption::Tip);
    DisplayInfo(report);
    }

void RealityDataConsole::Details()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo("please navigate to an item (with cd) before using this function\n", DisplayOption::Tip);
        return;
        }
    Utf8String className = m_currentNode->node.GetClassName();
    RequestStatus status;
    
    Utf8String instanceId = m_currentNode->node.GetInstanceId();
    instanceId.ReplaceAll("/", "~2F");

    if (className == "Document")
        {
        RealityDataDocumentByIdRequest documentReq = RealityDataDocumentByIdRequest(instanceId);
        RealityDataDocumentPtr document = RealityDataService::Request(documentReq, status);

        if(document == nullptr)
            {
            DisplayInfo("There was an error retrieving information for this item\n", DisplayOption::Error);
            return;
            }

        DisplayInfo (Utf8PrintfString(" Document       : %s\n", document->GetName()));
        DisplayInfo (Utf8PrintfString(" Container name : %s\n", document->GetContainerName()));
        DisplayInfo (Utf8PrintfString(" Id             : %s\n", document->GetId()));
        DisplayInfo (Utf8PrintfString(" Folder Id      : %s\n", document->GetFolderId()));
        DisplayInfo (Utf8PrintfString(" Access Url     : %s\n", document->GetAccessUrl()));
        DisplayInfo (Utf8PrintfString(" RealityData Id : %s\n", document->GetRealityDataId()));
        DisplayInfo (Utf8PrintfString(" ContentType    : %s\n", document->GetContentType()));
        DisplayInfo (Utf8PrintfString(" Size           : %lu\n", document->GetSize()));
        }
    else if (className == "Folder")
        {
        RealityDataFolderByIdRequest folderReq = RealityDataFolderByIdRequest(instanceId);
        RealityDataFolderPtr folder = RealityDataService::Request(folderReq, status);

        if (folder == nullptr)
            {
            DisplayInfo("There was an error retrieving information for this item\n", DisplayOption::Error);
            return;
            }

        DisplayInfo (Utf8PrintfString("Folder         : %s\n", folder->GetName()));
        DisplayInfo (Utf8PrintfString("Parent folder  : %s\n", folder->GetParentId()));
        DisplayInfo (Utf8PrintfString("RealityData Id : %s\n", folder->GetRealityDataId()));
        }
    else if (className == "RealityData")
        {
        RealityDataByIdRequest idReq = RealityDataByIdRequest(instanceId);
        RealityDataPtr entity = RealityDataService::Request(idReq, status);

        if (entity == nullptr)
            {
            DisplayInfo ("There was an error retrieving information for this item\n", DisplayOption::Error);
            return;
            }

        DisplayInfo (Utf8PrintfString(" Id                 : %s\n", entity->GetIdentifier()));
        DisplayInfo (Utf8PrintfString(" EnterpriseId       : %s\n", entity->GetEnterpriseId()));
        DisplayInfo (Utf8PrintfString(" Container name     : %s\n", entity->GetContainerName()));
        DisplayInfo (Utf8PrintfString(" RealityData name   : %s\n", entity->GetName()));
        DisplayInfo (Utf8PrintfString(" Dataset            : %s\n", entity->GetDataset()));
        DisplayInfo (Utf8PrintfString(" Group              : %s\n", entity->GetGroup()));
        DisplayInfo (Utf8PrintfString(" Description        : %s\n", entity->GetDescription()));
        DisplayInfo (Utf8PrintfString(" Root document      : %s\n", entity->GetRootDocument()));
        DisplayInfo (Utf8PrintfString(" Size (kb)          : %lu\n", entity->GetTotalSize()));
        DisplayInfo (Utf8PrintfString(" Classification     : %s\n", entity->GetClassificationTag()));
        DisplayInfo (Utf8PrintfString(" Type               : %s\n", entity->GetRealityDataType()));
        DisplayInfo (Utf8PrintfString(" Footprint          : %s\n", entity->GetFootprintString()));
        DisplayInfo (Utf8PrintfString(" ThumbnailDocument  : %s\n", entity->GetThumbnailDocument()));
        DisplayInfo (Utf8PrintfString(" MetadataURL        : %s\n", entity->GetMetadataURL()));
        DisplayInfo (Utf8PrintfString(" AccuracyInMeters   : %s\n", entity->GetAccuracy()));
        DisplayInfo (Utf8PrintfString(" ResolutionInMeters : %s\n", entity->GetResolution()));
        DisplayInfo (Utf8PrintfString(" Visibility         : %s\n", entity->GetVisibilityTag()));
        DisplayInfo (Utf8PrintfString(" Listable           : %s\n", entity->IsListable() ? "true" : "false"));
        DisplayInfo (Utf8PrintfString(" Modified timestamp : %s\n", entity->GetModifiedDateTime().ToString()));
        DisplayInfo (Utf8PrintfString(" Created timestamp  : %s\n", entity->GetCreationDateTime().ToString()));
        DisplayInfo (Utf8PrintfString(" OwnedBy            : %s\n", entity->GetOwner()));
        }
    }

void RealityDataConsole::FileAccess()
    {
    if(m_currentNode == nullptr)
        {
        DisplayInfo("Must select a RealityData, first\n", DisplayOption::Error);
        return;
        }

    AzureHandshake* handshake;
    if(m_lastInput.ContainsI("w"))
        handshake = new AzureHandshake(m_currentNode->node.GetInstanceId(), true);
    else
        handshake = new AzureHandshake(m_currentNode->node.GetInstanceId(), false);
    DisplayInfo(Utf8PrintfString("%s\n", handshake->GetHttpRequestString()));
    delete handshake;
    }

void RealityDataConsole::AzureAdress()
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

    RequestStatus status;
    RealityDataByIdRequest idReq = RealityDataByIdRequest(m_currentNode->node.GetRootId());
    RealityDataPtr entity = RealityDataService::Request(idReq, status);

    if (entity == nullptr)
        {
        DisplayInfo("There was an error retrieving information for this item\n", DisplayOption::Error);
        return;
        }

    RealityDataService::RequestToJSON((RealityDataUrl*)handshake, handshake->GetJsonResponse());
    Utf8String azureServer;
    Utf8String azureToken;
    int64_t tokenTimer;
    BentleyStatus handshakeStatus = handshake->ParseResponse(azureServer, azureToken, tokenTimer);
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
    else if (m_currentNode->node.GetClassName() != "RealityData")
        {
        DisplayInfo("can only change properties of RealityData at root, use \"cd ..\" to navigate back\n", DisplayOption::Error);
        return;
        }

    Utf8String input; 
    DisplayInfo("set properties from the list, use the -Finish- option to send the update\n", DisplayOption::Tip);
    bmap<RealityDataField, Utf8String> props = bmap<RealityDataField, Utf8String>();
    std::string str;
    Utf8String propertyString = "";
    Utf8String value;
    while (input != "-Finish-")
        {
        Choice(m_realityDataProperties, input);
        if(input == "-Finish-")
            break;
        else
            {
            DisplayInfo(Utf8PrintfString("Input value for %s\n", input));

            std::getline(std::cin, str);
            if(propertyString.length() > 0)
                propertyString.append(",");

            value = Utf8String(str.c_str()).Trim();
            if(input == "Listable")
                {
                if(value.EqualsI("false")) // a little cumbersome but forces proper format of boolean values
                    propertyString.append("\"Listable\" : false");
                else if (value.EqualsI("true"))
                    propertyString.append("\"Listable\" : true");
                else
                    DisplayInfo("Listable is boolean. Value must be true or false\n", DisplayOption::Error);
                }
            else
                propertyString.append(Utf8PrintfString("\"%s\" : \"%s\"", input, value));
            }
        }

    RealityDataServiceChange changeReq = RealityDataServiceChange(m_currentNode->node.GetRootId(), propertyString);

    int status = RequestType::Body;
    WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());
    Utf8String jsonResponse = WSGRequest::GetInstance().PerformRequest(changeReq, status, RealityDataService::GetVerifyPeer());

    Json::Value instances(Json::objectValue);
    if ((status != CURLE_OK) || !Json::Reader::Parse(jsonResponse, instances) || instances.isMember("errorMessage"))
        DisplayInfo(instances["errorMessage"].asString(), DisplayOption::Error);
    else 
        Details();
    }

void RealityDataConsole::Delete()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo("please navigate to an item (with cd) before using this function\n", DisplayOption::Tip);
        return;
        }
    Utf8String className = m_currentNode->node.GetClassName();

    Utf8String instanceId = m_currentNode->node.GetInstanceId();
    instanceId.ReplaceAll("/", "~2F");
    std::string str;
    Utf8String jsonResponse = "";

    if (className == "Document")
        {
        DisplayInfo(Utf8PrintfString("Deleting Document %s.\nConfirm? [ y / n ]", m_currentNode->node.GetInstanceId()), DisplayOption::Tip);
        std::getline(std::cin, str);
        if(str != "y")
            return;

        RealityDataDeleteDocument documentReq = RealityDataDeleteDocument(instanceId);
        RealityDataService::RequestToJSON(&documentReq, jsonResponse);
        }
    else if (className == "Folder")
        {
        DisplayInfo(Utf8PrintfString("Deleting Folder %s. All documents contained within will also be deleted.\nConfirm? [ y / n ]", m_currentNode->node.GetInstanceId()), DisplayOption::Tip);
        std::getline(std::cin, str);
        if (str != "y")
            return;

        RealityDataDeleteFolder folderReq = RealityDataDeleteFolder(instanceId);
        RealityDataService::RequestToJSON(&folderReq, jsonResponse);
        }
    else if (className == "RealityData")
        {
        DisplayInfo(Utf8PrintfString("Deleting RealityData %s. All folders and documents contained within will also be deleted.\n", m_currentNode->node.GetInstanceId()), DisplayOption::Tip);
        DisplayInfo("All project relationships attached to this RealityData will also be removed.\nConfirm ? [y / n]", DisplayOption::Tip);
        std::getline(std::cin, str);
        if (str != "y")
            return;

        RealityDataDelete realityDataReq = RealityDataDelete(instanceId);
        RealityDataService::RequestToJSON(&realityDataReq, jsonResponse);
        }

        if(jsonResponse.Contains("errorMessage"))
            {
            Json::Value instances(Json::objectValue);
            if (Json::Reader::Parse(jsonResponse, instances) && instances.isMember("errorMessage"))
                jsonResponse = instances["errorMessage"].asString();
            DisplayInfo(Utf8PrintfString("There was an error removing this item\n%s", jsonResponse), DisplayOption::Error);
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
        DisplayInfo("\nCurrent Filters:\n");
        DisplayInfo(Utf8PrintfString("Name : %s\n", m_nameFilter));
        DisplayInfo(Utf8PrintfString("Group : %s\n", m_groupFilter));
        DisplayInfo(Utf8PrintfString("Type : %s\n", m_typeFilter));
        DisplayInfo(Utf8PrintfString("OwnedBy : %s\n\n", m_ownerFilter));
        DisplayInfo("set filters from the list, use the -Finish- option to return\n", DisplayOption::Tip);

        Choice(m_filterProperties, filter);
        if (filter == "-Finish-")
            break;

        DisplayInfo(Utf8PrintfString("Set filter for %s (Enter blank field to remove filter). Careful, filters are case sensitive\n", filter), DisplayOption::Tip);

        std::getline(std::cin, str);
        value = Utf8String(str.c_str());
        if (filter.Equals("Name"))
            m_nameFilter = value;
        else if (filter.Equals("Group"))
            m_groupFilter = value;
        else if (filter.Equals("Type"))
            m_typeFilter = value;
        else if (filter.Equals("OwnedBy"))
            m_ownerFilter = value;
        }
    }

void RealityDataConsole::Relationships()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo("please navigate to an item (with cd) before using this function\n", DisplayOption::Tip);
        return;
        }
    RequestStatus status;

    Utf8String instanceId = m_currentNode->node.GetInstanceId();
    instanceId.ReplaceAll("/", "~2F");

    RealityDataProjectRelationshipByRealityDataIdRequest idReq = RealityDataProjectRelationshipByRealityDataIdRequest(instanceId);
    bvector<RealityDataProjectRelationshipPtr> entities = RealityDataService::Request(idReq, status);

    if (status == RequestStatus::ERROR)
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
    for(RealityDataProjectRelationshipPtr entity : entities)
        DisplayInfo(Utf8PrintfString(" ProjectId          : %s\n", entity->GetProjectId()));
    }

void RealityDataConsole::CreateRD()
    {
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    properties.Insert(RealityDataField::Name, m_lastInput);

    RealityDataServiceCreate createRequest = RealityDataServiceCreate("", RealityDataServiceUpload::PackageProperties(properties));
    int status;
    Utf8String response = WSGRequest::GetInstance().PerformRequest(createRequest, status, RealityDataService::GetVerifyPeer());
    if (status != CURLE_OK)
        {
        DisplayInfo(Utf8PrintfString("There was an error creating a new RealityData. Curl failed with error code %d\n", status), DisplayOption::Error);
        DisplayInfo(Utf8PrintfString("And message %s\n", response), DisplayOption::Error);
        }
    else
        {
        Json::Value instances(Json::objectValue);
        Json::Reader::Parse(response, instances);
        if (!instances["instances"].isNull() && !instances["instances"][0]["instanceId"].isNull())
            DisplayInfo(Utf8PrintfString("New RealityData created with GUID %s", instances["instances"][0]["instanceId"].asString()), DisplayOption::Info);
        }
    }

void RealityDataConsole::InputError()
    {
    DisplayInfo("unrecognized Command. Type \"help\" for usage\n", DisplayOption::Error);
    }

void RealityDataConsole::DisplayInfo(Utf8StringCR msg, DisplayOption option)
    {
    switch(option)
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
        std::cout << msg;

    // commande
    SetConsoleTextAttribute(m_hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}


