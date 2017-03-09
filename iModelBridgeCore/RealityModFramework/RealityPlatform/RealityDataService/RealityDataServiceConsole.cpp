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
    m_lastInput = Utf8String(str.c_str());

    bvector<Utf8String> args;
    BeStringUtilities::ParseArguments(args, m_lastInput.c_str());
    if(args.size() > 2)
        {
        std::cout << "too many inputs to parse. Please refer to \"Help\"" << std::endl;
        return;
        }

    if(args[0].ContainsI("quit"))
        m_lastCommand = Command::Quit;
    else if (args[0].ContainsI("retry"))
        m_lastCommand = Command::Retry;
    else if (args[0].ContainsI("ListAll"))
        m_lastCommand = Command::ListAll;
    else if (args[0].ContainsI("list") || args[0].ContainsI("dir"))
        m_lastCommand = Command::List;
    else if (args[0].ContainsI("help"))
        m_lastCommand = Command::Help;
    else if (args[0].ContainsI("stat"))
        m_lastCommand = Command::Stat;
    else if (args[0].ContainsI("cancel"))
        m_lastCommand = Command::Cancel;
    else if (args[0].ContainsI("details"))
        m_lastCommand = Command::Details;
    else if (args[0].ContainsI("Download"))
        m_lastCommand = Command::Download;
    else if (args[0].ContainsI("Upload"))
        m_lastCommand = Command::Upload;
    else
        {
        m_lastCommand = Command::Error;

        if (args[0].ContainsI("index"))
            m_lastCommand = Command::ChoiceIndex;
        else if (args[0].ContainsI("value"))
            m_lastCommand = Command::ChoiceValue;
        else if (args[0].ContainsI("cd"))
            m_lastCommand = Command::ChangeDir;
        
        if(m_lastCommand != Command::Error)
            {
            if (args.size() > 1)
                m_lastInput = args[1];
            else
                {
                std::cout << "must input a value, with this command" << std::endl;
                m_lastCommand = Command::Error;
                }
            }
        }
    }

RealityDataConsole::RealityDataConsole() : 
    m_server( WSGServer("", true)),
    m_serverNodes(bvector<NavNode>()),
    m_machineRepos(bvector<Utf8String>()),
    m_currentNode(nullptr)
    {
    m_functionMap.Insert(Command::Help, &RealityDataConsole::Usage);
    m_functionMap.Insert(Command::SetServer, &RealityDataConsole::ConfigureServer);
    m_functionMap.Insert(Command::List, &RealityDataConsole::List);
    m_functionMap.Insert(Command::ListAll, &RealityDataConsole::ListAll);
    m_functionMap.Insert(Command::ChangeDir, &RealityDataConsole::ChangeDir);
    m_functionMap.Insert(Command::Stat, &RealityDataConsole::EnterpriseStat);
    m_functionMap.Insert(Command::Details, &RealityDataConsole::Details);
    m_functionMap.Insert(Command::Download, &RealityDataConsole::Download);
    m_functionMap.Insert(Command::Upload, &RealityDataConsole::Upload);

    //commands that should never occur, within Run()
    m_functionMap.Insert(Command::Quit, &RealityDataConsole::DummyFunction);
    m_functionMap.Insert(Command::Retry, &RealityDataConsole::DummyFunction);
    m_functionMap.Insert(Command::Error, &RealityDataConsole::InputError);
    m_functionMap.Insert(Command::ChoiceIndex, &RealityDataConsole::DummyFunction);
    m_functionMap.Insert(Command::ChoiceValue, &RealityDataConsole::DummyFunction);
    }

void RealityDataConsole::Choice(bvector<Utf8String> options, Utf8StringR input)
    {
    PrintResults(options);
    std::cout << "an option can be selected by its Index or by its Value" << std::endl;
    std::cout << "by using either \"Index #\" or \"Value NameOfValue\"" << std::endl;
    
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
                if(choice > options.size())
                    {
                    std::cout << "Invalid Selection, selected index not between 0 and " << (options.size() - 1) << std::endl;
                    m_lastCommand = Command::Retry;
                    }
                else
                    input = options[choice];
                }
            else
                {
                std::cout << "Could not extract integer from provided input..." << std::endl;
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
            std::cout << "input error, please enter your choice in one of the following formats" << std::endl;
            std::cout << "Index #" << std::endl << "or" << std::endl << "Value NameOfValue" << std::endl;
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
            std::cout << m_currentNode->node.GetInstanceId();
        std::cout << "> ";
        InterpretCommand();
        (this->*(m_functionMap[m_lastCommand]))();
        }
    }

void RealityDataConsole::Usage()
    {
    std::cout << "RealityDataConsole tool for RDS V1.0" << std::endl << std::endl;
    std::cout << "Avalaible Commands (case insensitive):" << std::endl;
    std::cout << "Quit \t Exit the application" << std::endl;
    std::cout << "Retry \t (during a multi-step operation) Restart current operation" << std::endl;
    std::cout << "Help \t Print current Display" << std::endl;
    std::cout << "SetServer \t Change server settings (server url, repository and schema)" << std::endl;
    std::cout << "List \t List all subfiles/folders for the given location on your server" << std::endl;
    std::cout << "Dir \t same as List" << std::endl;
    std::cout << "cd \t Change current location. Must be called in one of the following ways" << std::endl;
    std::cout << "cd [number] \t navigates to node at the given index, as specified in the most recent List command" << std::endl;
    std::cout << "cd .. \t go up one level" << std:: endl;
    std::cout << "ListAll \t List every file beneath the current location (paged)" << std::endl;
    std::cout << "Details \t show the details for the location" << std::endl;
    std::cout << "Stat \t show enterprise statistics" << std::endl;
    std::cout << "Download\t Download files from the current location on the server" << std::endl;
    std::cout << "Upload \t Upload files to the server" << std::endl;
    }

void RealityDataConsole::PrintResults(bvector<Utf8String> results)
    {
    std::stringstream index;
    Utf8String fullOption;
    std::cout << "Index \t Value" << std::endl;
    std::string str;
    for (int i = 0; i < results.size(); ++i)
        {
        index.str(std::string());
        index << std::setw(5) << std::setfill(' ') << i;
        fullOption = index.str().c_str();
        fullOption.append(" \t ");
        fullOption.append(results[i]);
        str = fullOption.c_str();
        std::cout << str << std::endl;
        }
    }

void RealityDataConsole::ConfigureServer()
    {
    std::cout << "Welcome to the RealityDataService Navigator. Please enter your server name" << std::endl;
    std::cout << "Example format : dev-realitydataservices-eus.cloudapp.net" << std::endl;
    Utf8String server;
    std::string input;
    std::getline(std::cin, input);
    server = Utf8String(input.c_str());
    bool verifyCertificate = false;
    while(1)
        {
        std::cout << "Does this server have a recognized certificate? [ y / n ]" << std::endl;
        Utf8String temp;
        std::getline(std::cin, input);
        temp = Utf8String(input.c_str());
        if(temp.ContainsI("y"))
            {
            verifyCertificate = true;
            break;
            }
        else if(temp.ContainsI("n"))
            {
            verifyCertificate = false;
            break;
            }
        else
            std::cout << "invalid answer" << std::endl;
        }

    std::cout << "Retrieving version information. One moment..." << std::endl << std::endl;

    m_server = WSGServer(server, verifyCertificate);
    Utf8String version = m_server.GetVersion();
    Utf8String repo;
    Utf8String schema;
    
    bvector<Utf8String> repoNames = m_server.GetRepositories();
    m_lastCommand = Command::Error;
    if(repoNames.size() == 0)
        {
        std::cout << "There was an error contacting the server. No repositories found" << std::endl;
        while(m_lastCommand != Command::Retry && m_lastCommand != Command::Quit)
            {
            std::cout << "\"Retry\" to try with a different server; \"Quit\" to exit" << std::endl;
            InterpretCommand();
            if(m_lastCommand == Command::Retry)
                return ConfigureServer();
            else if(m_lastCommand == Command::Quit)
                {
                std::cout << "Quitting..." << std::endl;
                return;
                }
            }
        }
    else if (repoNames.size() == 1)
        {
        std::cout << "Only one repository found" << std::endl;
        repo = repoNames[0];
        std::cout << "Defaulting to " << repo << std::endl;
        }
    else
        {
        std::cout << "please select a repository from the following options" << std::endl;
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
        std::cout << std::endl;

        bvector<Utf8String> schemaNames = m_server.GetSchemaNames(repo);

        if (schemaNames.size() == 0)
            {
            std::cout << "No schemas were found for the given server and repo" << std::endl;
            while (m_lastCommand != Command::Retry && m_lastCommand != Command::Quit)
                {
                std::cout << "\"Retry\" to try with a different server or repo; \"Quit\" to exit" << std::endl;
                InterpretCommand();
                if (m_lastCommand == Command::Retry)
                    return ConfigureServer();
                else if (m_lastCommand == Command::Quit)
                    {
                    std::cout << "Quitting..." << std::endl;
                    return;
                    }
                }
            }
        else if (schemaNames.size() == 1)
            {
            std::cout << "Only one schema found" << std::endl;
            schema = schemaNames[0];
            std::cout << "Defaulting to " << schema << std::endl;
            }
        else
            {
            std::cout << "please select a repository from the following options" << std::endl;
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

    std::cout << "Server successfully configured, ready for use. Type \"help\" for list of commands" << std::endl;
    }

void RealityDataConsole::List()
    {   
    Utf8String nodeString;
    bvector<Utf8String> nodeStrings;

    if(m_currentNode == nullptr)
        m_serverNodes = NodeNavigator::GetInstance().GetRootNodes(m_server, RealityDataService::GetRepoName());
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

void RealityDataConsole::ListAll()
    {
    if(m_currentNode == nullptr)
        {   
        std::cout << "Please navigate to a RealityData or Folder before using this command" << std::endl;
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
        std::cout << "Failure retrieving Azure token" << std::endl;
        return;
        }

    AllRealityDataByRootId rdsRequest = AllRealityDataByRootId(m_currentNode->node.GetInstanceId());
    RequestStatus status;
    bvector<bpair<WString, uint64_t>> filesInRepo = RealityDataService::Request(rdsRequest, status);
    
    std::cout << filesInRepo.size() << " files in selection." << std::endl;
    std::cout << "these will be displayed, 20 at a time. Input \"Cancel\" to quit at any time, otherwise press enter to proceed to the next page" << std::endl;

    std::string str;
    while (m_lastCommand != Command::Cancel)
        {
        for (bpair<WString, uint64_t> file : filesInRepo)
            {
            std::cout << file.first.c_str() << ", size: " << file.second << std::endl;
            }
        std::getline(std::cin, str);
        if(Utf8String(str.c_str()).ContainsI("Cancel"))
            m_lastCommand = Command::Cancel;
        }
    }

void RealityDataConsole::ChangeDir()
    {
    if (m_lastInput == "..")
        {
        if (m_currentNode == nullptr)
            std::cout << "Already at root" << std::endl;
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
        return;
        }

    uint64_t choice;
    if (BeStringUtilities::ParseUInt64(choice, m_lastInput.c_str()) != BentleyStatus::SUCCESS)
        {
        std::cout << "Could not extract integer from provided input..." << std::endl;
        return;
        }

    if(choice < (uint64_t)m_serverNodes.size())
        {
        NodeList* newNode = new NodeList();
        newNode->node = m_serverNodes[choice];
        newNode->parentNode = m_currentNode;
        if(m_currentNode != nullptr)
            m_currentNode->childNode = newNode;
        m_currentNode = newNode;
        }
    else
        std::cout << "Invalid Selection, selected index not between 0 and " << (m_serverNodes.size() - 1) << std::endl;
    }

void RealityDataConsole::EnterpriseStat()
    {
    RequestStatus status;
    RealityDataEnterpriseStatRequest* ptt = new RealityDataEnterpriseStatRequest("");
    uint64_t NbRealityData;
    uint64_t TotalSizeKB;
    RealityDataService::Request(*ptt, &NbRealityData, &TotalSizeKB, status);

    std::cout << "Enterprise statistics: " << std::endl;
    std::cout << "   NbRealityData: " << NbRealityData << std::endl;
    std::cout << "   TotalSize(KB): " << TotalSizeKB << std::endl;
    }

static void downloadProgressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    char progressString[1024];
    sprintf(progressString, "percentage of files downloaded : %f", repoProgress * 100.0);
    std::cout << progressString << std::endl;
    }

static void uploadProgressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    char progressString[1024];
    //sprintf(progressString, "%s upload percent : %f", filename.c_str(), progress * 100.0f);
    sprintf(progressString, "upload percent : %f", repoProgress * 100.0);
    std::cout << progressString << std::endl;
    }

void RealityDataConsole::Download()
    {
    if (m_currentNode == nullptr)
        {
        std::cout << "Please navigate to a RealityData or Folder before using this command" << std::endl;
        return;
        }

    std::cout << "Downloading from " << m_currentNode->node.GetLabel() << std::endl;
    std::cout << "if you wish to change this, use command \"Cancel\" to back out and use cd to change the directory" << std::endl << std::endl;
    std::cout << "please enter the destination folder on the local machine (must be existing folder)" << std::endl;
    
    InterpretCommand();
    if(m_lastCommand == Command::Cancel)
        return;

    BeFileName fileName = BeFileName(m_lastInput);
    if(!fileName.DoesPathExist())
        {
        std::cout << "could not validate specified path. Please verify that the folder exists and try again" << std::endl;
        return;
        }

    RealityDataServiceDownload download = RealityDataServiceDownload(fileName, m_currentNode->node.GetInstanceId());
    download.SetProgressCallBack(downloadProgressFunc);
    download.SetProgressStep(0.1);
    download.OnlyReportErrors(true);
    TransferReport* tReport = download.Perform();
    Utf8String report;
    tReport->ToXml(report);
    std::cout << "if any files failed to download, they will be listed here: " << std::endl;
    std::cout << report << std::endl;
    }

void RealityDataConsole::Upload()
    {
    std::cout << "please enter the source folder on the local machine (must be existing folder)" << std::endl;
    
    BeFileName fileName = BeFileName(m_lastInput);
    if (!fileName.DoesPathExist())
        {
        std::cout << "could not validate specified path. Please verify that the folder exists and try again" << std::endl;
        return;
        }

    std::cout << "please input GUID for upload" << std::endl;
    std::string input;
    std::getline(std::cin, input);
    Utf8String guid = Utf8String(input.c_str());

    bmap<RealityDataField, Utf8String> properties;
    std::cout << "please input value for Name" << std::endl;
    std::getline(std::cin, input);
    properties.Insert(RealityDataField::Name, Utf8String(input.c_str()));

    std::cout << "please input value for Classification" << std::endl;
    std::getline(std::cin, input);
    properties.Insert(RealityDataField::Classification, Utf8String(input.c_str()));

    std::cout << "please input value for Type" << std::endl;
    std::getline(std::cin, input);
    properties.Insert(RealityDataField::Type, Utf8String(input.c_str()));

    Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

    RealityDataServiceUpload upload = RealityDataServiceUpload(fileName, guid, propertyString, true);
    upload.SetProgressCallBack(uploadProgressFunc);
    upload.SetProgressStep(0.1);
    upload.OnlyReportErrors(true);
    TransferReport* tReport = upload.Perform();
    Utf8String report;
    tReport->ToXml(report);
    std::cout << "if any files failed to upload, they will be listed here: " << std::endl;
    std::cout << report << std::endl;
    }

void RealityDataConsole::Details()
    {
    if (m_currentNode == nullptr)
        {
        std::cout << "please navigate to an item (with cd) before using this function" << std::endl;
        return;
        }
    Utf8String className = m_currentNode->node.GetClassName();
    RequestStatus status;
    if (className == "Document")
        {
        RealityDataDocumentByIdRequest documentReq = RealityDataDocumentByIdRequest(m_currentNode->node.GetInstanceId());
        RealityDataDocumentPtr document = RealityDataService::Request(documentReq, status);

        if(document == nullptr)
            {
            std::cout << "there was an error retrieving information for this item" << std::endl;
            return;
            }

        std::cout << "Document : " << document->GetName() << std::endl;
        std::cout << "Container name : " << document->GetContainerName() << std::endl;
        std::cout << "Id : " << document->GetId() << std::endl;
        std::cout << "Folder Id : " << document->GetFolderId() << std::endl;
        std::cout << "Access Url : " << document->GetAccessUrl() << std::endl;
        std::cout << "RealityData Id : " << document->GetRealityDataId() << std::endl;
        std::cout << "ContentType : " << document->GetContentType() << std::endl;
        std::cout << "Size : " << document->GetSize() << std::endl;
        }
    else if (className == "Folder")
        {
        RealityDataFolderByIdRequest folderReq = RealityDataFolderByIdRequest(m_currentNode->node.GetInstanceId());
        RealityDataFolderPtr folder = RealityDataService::Request(folderReq, status);

        if (folder == nullptr)
            {
            std::cout << "there was an error retrieving information for this item" << std::endl;
            return;
            }

        std::cout << "Folder : " << folder->GetName() << std::endl;
        std::cout << "Parent folder : " << folder->GetParentId() << std::endl;
        std::cout << "RealityData Id : " << folder->GetRealityDataId() << std::endl;
        }
    else if (className == "RealityData")
        {
        RealityDataByIdRequest idReq = RealityDataByIdRequest(m_currentNode->node.GetInstanceId());
        RealityDataPtr entity = RealityDataService::Request(idReq, status);

        if (entity == nullptr)
            {
            std::cout << "there was an error retrieving information for this item" << std::endl;
            return;
            }

        std::cout << "RealityData name : " << entity->GetName() << std::endl;
        std::cout << "Id : " << entity->GetIdentifier() << std::endl;
        std::cout << "Container name : " << entity->GetContainerName() << std::endl;
        std::cout << "Dataset : " << entity->GetDataset() << std::endl;
        std::cout << "Description : " << entity->GetDescription() << std::endl;
        std::cout << "Root document : " << entity->GetRootDocument() << std::endl;
        std::cout << "Size (kb) : " << entity->GetIdentifier() << std::endl;
        std::cout << "Classification : " << entity->GetClassificationTag() << std::endl;
        std::cout << "Type : " << entity->GetRealityDataType() << std::endl;
        //std::cout << "Footprint : " << entity->GetClassification() << std::endl;
        std::cout << "Accuracy (m) : " << entity->GetAccuracyValue() << std::endl;
        std::cout << "Modified timestamp : " << entity->GetModifiedDateTime().ToString() << std::endl;
        std::cout << "Created timestamp : " << entity->GetCreationDateTime().ToString() << std::endl;
        }
    }

void RealityDataConsole::InputError()
    {
    std::cout << "unrecognized Command. Type \"help\" for usage" << std::endl;
    }