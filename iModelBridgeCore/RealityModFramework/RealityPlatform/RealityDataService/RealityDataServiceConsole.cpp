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

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

Utf8String CheckCommand(Utf8String entry)
    {
    if(entry.Contains("quit"))
        return "quit";
    else if (entry.Contains("retry"))
        return "retry";
    return "";
    }

Utf8String Choice(bvector<Utf8String> options)
    {
    std::stringstream index;
    Utf8String fullOption;
    std::cout << "Index \t Value" << std::endl;
    std::string str;
    for(int i = 0; i < options.size(); ++i)
        {
        index.str(std::string());
        index << std::setw(5) << std::setfill(' ') << i;
        fullOption = index.str().c_str();
        fullOption.append(" \t ");
        fullOption.append(options[i]);
        str = fullOption.c_str();
        std::cout << str << std::endl;
        }
    std::cout << "an option can be selected by its Index or by its Value" << std::endl;
    std::cout << "by using either \"Index #\" or \"Value NameOfValue\"" << std::endl;
    
    uint64_t choice;
    bvector<Utf8String> args = bvector<Utf8String>();
    do
        {
        std::getline(std::cin, str);
        BeStringUtilities::ParseArguments(args, Utf8CP(str.c_str()));
        if(args.size() != 2)
            {
            std::cout << "input error, please enter your choice in one of the following formats" << std::endl; 
            std::cout << "\"Index #\" or \"Value NameOfValue\"" << std::endl;
            }
        } while (args.size() != 2);

    Utf8String result = "";
    if(args[0].ContainsI("Index"))
        {
        Utf8String j;
        for(int i = 0; i < args.size(); ++i)
            j = args[i];
        BeStringUtilities::ParseUInt64(choice, args[1].c_str());
        result = options[choice];
        }
    else
        result = args[1];
        
    return result;
    }

int main(int argc, char* argv[])
    {
    SetConsoleTitle("RealityDataService Navigator");

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

    std::cout << "Retrieving version information. One moment..." << std::endl;

    WSGServer wsgServer = WSGServer(server, verifyCertificate);
    Utf8String version = wsgServer.GetVersion();
    Utf8String repo;
    Utf8String schema;
    
    bvector<Utf8String> repoNames = wsgServer.GetRepositories();
    if(repoNames.size() == 0)
        {
        std::cout << "There was an error contacting the server" << std::endl;
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
        repo = Choice(repoNames);
        }

    if(repo.length() > 0)
        {
        bvector<Utf8String> schemaNames = wsgServer.GetSchemaNames(repo);

        if (schemaNames.size() == 0)
            {
            std::cout << "No schemas were found for the given server and repo" << std::endl;
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
            schema = Choice(schemaNames);
            }

        if(schema.length() > 0)
            RealityDataService::SetServerComponents(server, version, repo, schema);
        }

    getch();
    }