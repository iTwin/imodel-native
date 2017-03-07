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
    else if (args[0].ContainsI("cdi"))
        return Command::ChangeDirIndex;
    else if (args[0].ContainsI("cd"))
        return Command::ChangeDir;
    else if (args[0].ContainsI("help"))
        return Command::Help;
    /*else if (args[0].ContainsI("error"))
        return Command::Error;
    else if (args[0].ContainsI("error"))
        return Command::Error;*/

    return Command::Error;
    }

RealityDataConsole::RealityDataConsole() : 
    m_server( WSGServer("", true)),
    m_serverNodes(bvector<NavNode>()),
    m_machineRepos(bvector<Utf8String>())
    {}

Command RealityDataConsole::Choice(bvector<Utf8String> options, Utf8StringR input)
    {
    PrintResults(options);
    std::cout << "an option can be selected by its Index or by its Value" << std::endl;
    std::cout << "by using either \"Index #\" or \"Value NameOfValue\"" << std::endl;
    
    uint64_t choice;
    Command userCommand;
    Utf8String str;
    //bvector<Utf8String> args = bvector<Utf8String>();
    userCommand = InterpretCommand(str, 2);
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
    uint64_t index;
    while(userCommand != Command::Quit)
        {
        std::cout << m_currentNode << "> ";
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
            case Command::ChangeDirIndex:
                {
                if(BeStringUtilities::ParseUInt64(index, input.c_str()) == BentleyStatus::SUCCESS)
                    userCommand = ChangeDir(index);
                else
                    std::cout << "Could not extract integer from provided input..." << std::endl;
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
    std::cout << "cd [text] \t attempt to reach the node specified in [text]" << std::endl;
    std::cout << "cdi [number] \t navigates to node at the given index, as specified in the most recent List command" << std::endl;
    std::cout << "cd .. \t go up one level" << std:: endl;
    std::cout << "ListAll \t List every file for the current Reality Data (paged)" << std::endl;

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
        if(temp.Contains("y") || temp.Contains("Y"))
            {
            verifyCertificate = true;
            break;
            }
        else if(temp.Contains("n") || temp.Contains("N"))
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

    if(m_currentNode.length() == 0)
        m_serverNodes = NodeNavigator::GetInstance().GetRootNodes(m_server, RealityDataService::GetRepoName());
    else
        m_serverNodes = NodeNavigator::GetInstance().GetChildNodes(m_server, RealityDataService::GetRepoName(), m_currentNode);

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
        /*Utf8String navString = m_currentNode.GetNavString();
        navString.ReplaceAll(m_currentNode.GetLabel(), "");*/
        bvector<Utf8String> segments;
        Utf8String currentNode = m_currentNode;
        currentNode.ReplaceAll("~2F", " ");
        BeStringUtilities::Split(m_currentNode.c_str(), " ", segments);

        size_t index = segments.size() - 1;
        if(segments[index].length() == 0) //if node ends with ~2F
            --index;

        m_currentNode.ReplaceAll(segments[index].c_str(), "");
        m_currentNode.ReplaceAll("~2F~2F", "");
        }

    return Command::AllGood;
    }

Command RealityDataConsole::ChangeDir(uint64_t choice)
    {
    if(choice < (uint64_t)m_serverNodes.size())
        m_currentNode = m_serverNodes[choice].GetNavString();
    else
        std::cout << "Invalid Selection, selected index not between 0 and " << (m_serverNodes.size() - 1) << std::endl;
       
    return Command::AllGood;
    }