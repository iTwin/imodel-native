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

Command RealityDataConsole::InterpretCommand(Utf8StringR entry, int argc)
    {
    std::string str;
    std::getline(std::cin, str);
    entry = Utf8String(str.c_str());

    bvector<Utf8String> args;
    BeStringUtilities::ParseArguments(args, entry.c_str());
    if(args.size() > argc)
        return Command::Error;

    if(args.size() > 1)
        entry = args[1];

    if(args[0].ContainsI("quit"))
        return Command::Quit;
    else if (args[0].ContainsI("retry"))
        return Command::Retry;
    else if (args[0].ContainsI("error"))
        return Command::Error;
    else if (args[0].ContainsI("index"))
        return Command::ChoiceIndex;
    else if (args[0].ContainsI("value"))
        return Command::ChoiceValue;
    else if (args[0].ContainsI("list") || args[0].ContainsI("dir"))
        return Command::List;
    else if (args[0].ContainsI("cd"))
        return Command::ChangeDir;
    else if (args[0].ContainsI("help"))
        return Command::Help;
    else if (args[0].ContainsI("stat"))
        return Command::Stat;
    else if (args[0].ContainsI("cancel"))
        return Command::Cancel;
    else if (args[0].ContainsI("details"))
        return Command::Details;

    return Command::Error;
    }

RealityDataConsole::RealityDataConsole() : 
    m_server( WSGServer("", true)),
    m_serverNodes(bvector<NavNode>()),
    m_machineRepos(bvector<Utf8String>()),
    m_currentNode(nullptr)
    {}

Command RealityDataConsole::Choice(bvector<Utf8String> options, Utf8StringR input)
    {
    PrintResults(options);
    std::cout << "an option can be selected by its Index or by its Value" << std::endl;
    std::cout << "by using either \"Index #\" or \"Value NameOfValue\"" << std::endl;
    
    uint64_t choice;
    Utf8String str;

    Command userCommand = InterpretCommand(str, 2);
    switch(userCommand)
        {
        case Command::Quit:
            break;
        case Command::ChoiceIndex:
            {
            if (BeStringUtilities::ParseUInt64(choice, str.c_str()) == BentleyStatus::SUCCESS)
                {
                if(choice > options.size())
                    {
                    std::cout << "Invalid Selection, selected index not between 0 and " << (options.size() - 1) << std::endl;
                    userCommand = Command::Retry;
                    }
                else
                    input = options[choice];
                }
            else
                {
                std::cout << "Could not extract integer from provided input..." << std::endl;
                userCommand = Command::Retry;
                }
            break;
            }
        case Command::ChoiceValue:
            {
            input = str;
            break;
            }
        case Command::Error:
            {
            std::cout << "input error, please enter your choice in one of the following formats" << std::endl;
            std::cout << "Index #" << std::endl << "or" << std::endl << "Value NameOfValue" << std::endl;
            userCommand = Command::Retry;
            }
        }
    if (userCommand == Command::Retry)
        return Choice(options, input);

    return userCommand;
    }

int main(int argc, char* argv[])
    {
    SetConsoleTitle("RealityDataService Navigator");

    RealityDataConsole console = RealityDataConsole();
    console.Run();
    }

void RealityDataConsole::Run()
    {
    Command userCommand;
    userCommand = ConfigureServer();
    Utf8String input;
    while(userCommand != Command::Quit)
        {
        if(m_currentNode != nullptr)
            std::cout << m_currentNode->node.GetInstanceId();
        std::cout << "> ";
        userCommand = InterpretCommand(input, 2);
        switch(userCommand)
            {
            case Command::Quit:
                break;
            case Command::Error:
                {
                std::cout << "unrecognized Command. Type \"help\" for usage" << std::endl;
                break;
                }
            case Command::Help:
                {
                Usage();
                break;
                }
            case Command::SetServer:
                {
                userCommand = ConfigureServer();
                break;
                }
            case Command::List:
                {
                userCommand = List();
                break;
                }
            case Command::ChangeDir:
                {
                userCommand = ChangeDir(input);
                break;
                }   
            case Command::Stat:
                {
                userCommand = EnterpriseStat();
                break;
                }
            case Command::Details:
                {
                userCommand = Details();
                break;
                }
            }
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
    std::cout << "ListAll \t List every file for the current Reality Data (paged)" << std::endl;
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

Command RealityDataConsole::ConfigureServer()
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
    Command userCommand = Command::Error;
    if(repoNames.size() == 0)
        {
        std::cout << "There was an error contacting the server. No repositories found" << std::endl;
        while(userCommand != Command::Retry && userCommand != Command::Quit)
            {
            std::cout << "\"Retry\" to try with a different server; \"Quit\" to exit" << std::endl;
            userCommand = InterpretCommand(repo, 1);
            if(userCommand == Command::Retry)
                return ConfigureServer();
            else if(userCommand == Command::Quit)
                {
                std::cout << "Quitting..." << std::endl;
                return userCommand;
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
        userCommand = Choice(repoNames, repo);
        switch(userCommand)
            {
        case Command::Quit:
            return userCommand;
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
            while (userCommand != Command::Retry && userCommand != Command::Quit)
                {
                std::cout << "\"Retry\" to try with a different server or repo; \"Quit\" to exit" << std::endl;
                userCommand = InterpretCommand(repo, 1);
                if (userCommand == Command::Retry)
                    return ConfigureServer();
                else if (userCommand == Command::Quit)
                    {
                    std::cout << "Quitting..." << std::endl;
                    return userCommand;
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
            userCommand = Choice(schemaNames, schema);
            switch(userCommand)
                {
                case Command::Quit:
                    return userCommand;
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

    return Command::AllGood;
    }

Command RealityDataConsole::List()
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

    return Command::AllGood;
    }

Command RealityDataConsole::ChangeDir(Utf8String newNode)
    {
    if(newNode == "..")
        {
        if(m_currentNode == nullptr)
            {
            std::cout << "Already at root" << std::endl;
            return Command::Error;
            }
        if(m_currentNode->parentNode != nullptr)
            {
            m_currentNode = m_currentNode->parentNode;
            delete m_currentNode->childNode;
            }
        else
            delete m_currentNode;
        }
    uint64_t index;
    if (BeStringUtilities::ParseUInt64(index, newNode.c_str()) == BentleyStatus::SUCCESS)
        ChangeDir(index);
    else
        std::cout << "Could not extract integer from provided input..." << std::endl;

    return Command::AllGood;
    }

Command RealityDataConsole::ChangeDir(uint64_t choice)
    {
    if(choice < (uint64_t)m_serverNodes.size())
        {
        NodeList* newNode = new NodeList();
        newNode->node = m_serverNodes[choice];
        newNode->parentNode = m_currentNode;
        m_currentNode->childNode = newNode;
        m_currentNode = newNode;
        //m_currentNode = m_serverNodes[choice].GetNavString();
        }
    else
        std::cout << "Invalid Selection, selected index not between 0 and " << (m_serverNodes.size() - 1) << std::endl;
       
    return Command::AllGood;
    }

Command RealityDataConsole::EnterpriseStat()
    {
    RequestStatus status;
    RealityDataEnterpriseStatRequest* ptt = new RealityDataEnterpriseStatRequest("");
    uint64_t NbRealityData;
    uint64_t TotalSizeKB;
    RealityDataService::Request(*ptt, &NbRealityData, &TotalSizeKB, status);

    std::cout << "Enterprise statistics: " << std::endl;
    std::cout << "   NbRealityData: " << NbRealityData << std::endl;
    std::cout << "   TotalSize(KB): " << TotalSizeKB << std::endl;

    return Command::AllGood;
    }

static void downloadProgressFunc(Utf8String filename, double fileProgress, double repoProgress)
{
    char progressString[1024];
    sprintf(progressString, "percentage of files downloaded : %f", repoProgress * 100.0);
    std::cout << progressString << std::endl;
}

Command RealityDataConsole::Download()
    {
    std::cout << "using current source = " << m_currentNode->node.GetLabel() << std::endl;
    std::cout << "if you wish to change this, use command \"Cancel\" to back out and use cd to change the directory" << std::endl << std::endl;
    std::cout << "please enter the destination folder on the local machine (must be existing folder)" << std::endl;
    
    Utf8String str;
    Command userCommand = InterpretCommand(str, 1);
    if(userCommand == Command::Cancel)
        return userCommand;

    BeFileName fileName = BeFileName(str);
    if(!fileName.DoesPathExist())
        {
        std::cout << "could not validate specified path. Please verify that the folder exists and try again" << std::endl;
        return Command::Error;
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

    return Command::AllGood;
    }

Command RealityDataConsole::Details()
    {
    if (m_currentNode == nullptr)
        {
        std::cout << "please navigate to an item (with cd) before using this function" << std::endl;
        return Command::Error;
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
            return Command::Error;
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
            return Command::Error;
            }

        std::cout << "Folder : " << folder->GetName() << std::endl;
        std::cout << "Parent folder : " << folder->GetParentId() << std::endl;
        std::cout << "RealityData Id : " << folder->GetRealityDataId() << std::endl;
        }
    else if (className == "RealityData")
        {
        RealityDataByIdRequest idReq = RealityDataByIdRequest(m_currentNode->node.GetInstanceId());
        SpatialEntityPtr entity = RealityDataService::Request(idReq, status);

        if (entity == nullptr)
            {
            std::cout << "there was an error retrieving information for this item" << std::endl;
            return Command::Error;
            }

        std::cout << "RealityData name : " << entity->GetName() << std::endl;
        std::cout << "Id : " << entity->GetIdentifier() << std::endl;
        std::cout << "Container name : " << entity->GetContainerName() << std::endl;
        std::cout << "Dataset : " << entity->GetDataset() << std::endl;
        std::cout << "Description : " << entity->GetDescription() << std::endl;
        std::cout << "Root document : " << entity->GetRootDocument() << std::endl;
        std::cout << "Size (kb) : " << entity->GetIdentifier() << std::endl;
        std::cout << "Classification : " << entity->GetClassificationTag() << std::endl;
        std::cout << "Type : " << entity->GetDataType() << std::endl;
        //std::cout << "Footprint : " << entity->GetClassification() << std::endl;
        std::cout << "Accuracy (m) : " << entity->GetAccuracyValue() << std::endl;
        std::cout << "Modified timestamp : " << entity->GetModifiedTimestamp().ToString() << std::endl;
        std::cout << "Created timestamp : " << entity->GetDate().ToString() << std::endl;
        }

    return Command::AllGood;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
/*void ListSubItem(WSGServer& Server, Utf8String Repo, NavNode Root, Utf8String RootName, int MaxEntryDisplay = 25)
    {
    bvector<NavNode> subNodes = NodeNavigator::GetInstance().GetChildNodes(Server, Repo, Root);
    std::cout << RootName << std::endl;
    bool folderEmpty = true;
    int count = 0;
    for (NavNode subNode : subNodes)
        {
        if (subNode.GetClassName().Contains("Document"))
            {
            if (s_cmd & CmdListAllDetail)
                {
                Utf8String repoName(subNode.GetRootId() + "~2F" + subNode.GetInstanceId());
                repoName.ReplaceAll("/", "~2F");
                RequestStatus status;
                RealityDataDocumentPtr document = RealityDataService::Request(RealityDataDocumentByIdRequest(repoName), status);
                std::cout << "  " << subNode.GetInstanceId() <<
                    std::setw(12) << " Size(KB): " << document->GetSize() <<
                    std::setw(16) << " ContentType: " << document->GetContentType() << std::endl;
                }
            else
                std::cout << "  " << subNode.GetInstanceId() << std::endl;

            folderEmpty = false;
            }
        else
            {
            std::cout << "  " << subNode.GetInstanceId() << std::endl;
            folderEmpty = false;
            }

        ++count;
        if (count > MaxEntryDisplay)
            {
            std::cout << "     More than  " << MaxEntryDisplay << "entries..." << std::endl;
            break;
            }
        }
    if (folderEmpty)
        std::cout << "     Empty" << std::endl;

    std::cout << std::endl;

    for (NavNode subNode : subNodes)
        {
        if (subNode.GetClassName().Contains("Folder"))
            {
            ListSubItem(Server, Repo, subNode, subNode.GetInstanceId());
            }
        }
    }

bool FilterByProject(Utf8StringCR RealityId, bvector<RealityDataProjectRelationshipPtr>& ProjectRelation)
    {
    // Option not set
    if (!(s_option & OptFilterProject))
        return true;

    for (RealityDataProjectRelationshipPtr pData : ProjectRelation)
        {
        if (pData->GetRealityDataId() == RealityId)
            return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
/*void ListCmd()
    {
    if (s_cmd & (CmdList | CmdListDetail | CmdListAll | CmdListAllDetail))
        {
        RequestStatus status;

        RealityDataPagedRequest* enterpriseReq = new RealityDataPagedRequest();
        enterpriseReq->SetPageSize(25);

        // Filters ?
        if (s_option & (OptFilterOwner))
            {
            bvector<Utf8String> filter1 = bvector<Utf8String>();

            filter1.push_back(RealityDataFilterCreator::FilterByOwner(s_filterOwner));
            Utf8String filters = RealityDataFilterCreator::GroupFiltersOR(filter1);

            enterpriseReq->SetFilter(filters);
            }

        if (s_option & (OptSortModDate))
            enterpriseReq->SortBy(RealityDataField::ModifiedTimestamp, true);

        if (s_option & (OptSortGroup))
            enterpriseReq->SortBy(RealityDataField::Group, true);

        bvector<SpatialEntityPtr> enterpriseVec = RealityDataService::Request(*enterpriseReq, status);

        bvector<RealityDataProjectRelationshipPtr> relationships;
        if (s_option & (OptFilterProject))
            {
            RealityDataProjectRelationshipByProjectIdRequest* relationReq = new RealityDataProjectRelationshipByProjectIdRequest(s_filterProject);
            relationships = RealityDataService::Request(*relationReq, status);
            }


        size_t EnterpriseSizeKB(0);
        do 
            {
            if (RequestStatus::SUCCESS != status)
                exit(-1);

            for (SpatialEntityPtr pData : enterpriseVec)
                {
                EnterpriseSizeKB += pData->GetApproximateFileSize();

                if (FilterByProject(pData->GetIdentifier(), relationships))
                    std::cout << pData->GetIdentifier() << " -- " << pData->GetName() << std::endl;

                if (s_cmd & (CmdListDetail | CmdListAllDetail) && FilterByProject(pData->GetIdentifier(), relationships))
                    {
                    std::cout << "  " <<
                        " Dataset        : " << pData->GetDataset() << std::endl << "  " <<
                        " Group          : " << pData->GetGroup() << std::endl << "  " <<
                        " Classification : " << pData->GetClassificationTag() << std::endl << "  " <<
                        " Size(KB)       : " << pData->GetApproximateFileSize() << std::endl << "  " <<
                        " Owner          : " << pData->GetOwner() << std::endl << "  " <<
                        " Created        : " << pData->GetDate().ToString() << std::endl << "  " <<
                        " Modification   : " << pData->GetModifiedTimestamp().ToString() << std::endl << "  " <<
                        " GetRootDocument: " << pData->GetRootDocument() << std::endl << "  " <<
                        " Visibility     : " << pData->GetVisibilityTag() << std::endl << "  " <<
                        " Enterprise     : " << pData->GetEnterprise() << std::endl << "  " <<
                        " Description    : " << pData->GetDescription() << std::endl;
                    }
                }

            enterpriseVec.clear();
            } while ((enterpriseVec = RealityDataService::Request(*enterpriseReq, status)).size() > 0);
        std::cout << std::endl << "*** Size total : " << EnterpriseSizeKB << "KB" << std::endl;
        std::cout << std::endl;

        if (s_cmd & (CmdListAll | CmdListAllDetail))
            {
            WSGServer server = WSGServer("dev-realitydataservices-eus.cloudapp.net", false);
            bvector<NavNode> nodes = NodeNavigator::GetInstance().GetRootNodes(server, "S3MXECPlugin--Server");

            for (NavNode root : nodes)
                {
                RequestStatus status;
                SpatialEntityPtr pData = RealityDataService::Request(RealityDataByIdRequest(root.GetInstanceId()), status);

                ListSubItem(server, "S3MXECPlugin--Server", root, root.GetInstanceId() + " -- " + pData->GetName());
                }
            }
        }

    if (s_cmd == CmdListItem)
        {
        s_itemPath.ReplaceAll("/", "~2F");
        RequestStatus status;
        RealityDataDocumentPtr document = RealityDataService::Request(RealityDataDocumentByIdRequest(s_itemPath), status);

        std::cout << document->GetFolderId() << document->GetName() <<
            std::setw(12) << " Size(KB): " << document->GetSize() <<
            std::setw(16) << " ContentType: " << document->GetContentType() << std::endl;

        }

    }*/