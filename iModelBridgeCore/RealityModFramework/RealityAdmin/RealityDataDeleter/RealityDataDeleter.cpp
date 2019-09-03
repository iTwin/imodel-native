/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatformTools/RealityDataService.h>
#include <RealityPlatformTools/WSGServices.h>
#include <CCApi/CCPublic.h>
#include <curl/curl.h>

#include <stdio.h>
#include <conio.h>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>


USING_NAMESPACE_BENTLEY_REALITYPLATFORM

struct NodeList
    {
    NodeList() : parentNode(nullptr), childNode(nullptr)
    {}

    NodeList *parentNode;
    NavNode node;
    NodeList *childNode;
    };

enum class DisplayOption
    {
    Info,
    Question,
    Tip,
    Error
    };

HANDLE        s_hConsole;

static std::istream*        s_inputSource =  &std::cin;
static std::ostream*        s_outputDestination = &std::cout;
static bvector<NavNode>     s_serverNodes;
static WSGServer            s_server("", true);
;

void DisplayInfo(Utf8StringCR msg, DisplayOption option)
    {
    switch (option)
        {
    case DisplayOption::Question:
        SetConsoleTextAttribute(s_hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
        break;

    case DisplayOption::Tip:
        SetConsoleTextAttribute(s_hConsole, FOREGROUND_GREEN);
        break;

    case DisplayOption::Error:
        SetConsoleTextAttribute(s_hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        break;

    case DisplayOption::Info:
    default:
        SetConsoleTextAttribute(s_hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
        break;
        }

    if (!msg.empty())
        *s_outputDestination << msg;

    // commande
    SetConsoleTextAttribute(s_hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
Utf8String MakeBuddiCall(int region)
    {
    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled(api, &installed);
    if (!installed)
        {
        DisplayInfo("Connection client does not seem to be installed\n", DisplayOption::Error);
        CCApi_FreeApi(api);
        return "";
        }
    bool running = false;
    status = CCApi_IsRunning(api, &running);
    if (status != APIERR_SUCCESS || !running)
        {
        DisplayInfo("Connection client does not seem to be running\n", DisplayOption::Error);
        CCApi_FreeApi(api);
        return "";
        }
    bool loggedIn = false;
    status = CCApi_IsLoggedIn(api, &loggedIn);
    if (status != APIERR_SUCCESS || !loggedIn)
        {
        DisplayInfo("Connection client does not seem to be logged in\n", DisplayOption::Error);
        CCApi_FreeApi(api);
        return "";
        }
    bool acceptedEula = false;
    status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
    if (status != APIERR_SUCCESS || !acceptedEula)
        {
        DisplayInfo("Connection client user does not seem to have accepted EULA\n", DisplayOption::Error);
        CCApi_FreeApi(api);
        return "";
        }
    bool sessionActive = false;
    status = CCApi_IsUserSessionActive(api, &sessionActive);
    if (status != APIERR_SUCCESS || !sessionActive)
        {
        DisplayInfo("Connection client does not seem to have an active session\n", DisplayOption::Error);
        CCApi_FreeApi(api);
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

	Utf8String returnedString(charServer);
	delete [] charServer;
    return returnedString;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
void PrintResults(bvector<pair<Utf8String, Utf8String>> results)
{
    std::stringstream index;
    Utf8String fullOption;
    DisplayInfo("Index \t Value\n", DisplayOption::Info);
    std::string str;
    Utf8String currentSection = "";

    for (size_t i = 0; i < results.size(); ++i)
    {
        if (currentSection != results[i].first)
            {
            currentSection = results[i].first;
            DisplayInfo(Utf8PrintfString("\t\t%s\n", currentSection.c_str()), DisplayOption::Question);
            }
        DisplayInfo(Utf8PrintfString("%5d \t %s\n", (i + 1), results[i].second.c_str()), DisplayOption::Info);
    }
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
void ListRoots()
    {
    RealityDataListByUltimateIdPagedRequest ultimateReq = RealityDataListByUltimateIdPagedRequest("", 0, 2500);

    bvector<RDSFilter> properties = bvector<RDSFilter>();

    ultimateReq.SortBy(RealityDataField::OwnedBy, true);

    RawServerResponse ultimateResponse = RawServerResponse();
    ultimateResponse.status = RequestStatus::OK;
    bvector<RealityDataPtr> ultimateVec = bvector<RealityDataPtr>();
    bvector<RealityDataPtr> partialVec;

    while (ultimateResponse.status == RequestStatus::OK)
        {//When LASTPAGE has been added, loop will exit
        partialVec = RealityDataService::Request(ultimateReq, ultimateResponse);
        ultimateVec.insert(ultimateVec.end(), partialVec.begin(), partialVec.end());
        }
    bvector<pair<Utf8String, Utf8String>> nodes;

    bvector<Utf8String> subvec = bvector<Utf8String>();
    Utf8String owner;
    if(ultimateVec.size() > 0)
        {
        owner = ultimateVec[0]->GetOwner();
        owner.ToLower();
        }

    Utf8String schema = RealityDataService::GetSchemaName();
    Utf8String rdOwner;
    for (RealityDataPtr rData : ultimateVec)
        {
        rdOwner = rData->GetOwner();
        rdOwner.ToLower();

        nodes.push_back(pair<Utf8String, Utf8String>(rdOwner, Utf8PrintfString("%-30s  %-22s (%s / %s) %s  %ld", rData->GetName().c_str(), rData->GetRealityDataType().c_str(), rData->IsListable() ? "Lst" : " - ", ShortenVisibility(rData->GetVisibilityTag()).c_str(), rData->GetIdentifier().c_str(), rData->GetTotalSize())));

        s_serverNodes.push_back(NavNode(schema, rData->GetIdentifier(), "ECObjects", "RealityData"));
        }

    PrintResults(nodes);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
void ForceMassUnlink()
    {
    DisplayInfo(Utf8PrintfString("Attempting to unlink all %d entries from all their relationships. Please be extra patient...\n", s_serverNodes.size()), DisplayOption::Tip);
    
    RawServerResponse projectResponse = RawServerResponse();

    RealityDataRelationshipByRealityDataIdRequest idReq = RealityDataRelationshipByRealityDataIdRequest("");
    bvector<RealityDataRelationshipPtr> entities;

    Utf8String id;
    RealityDataRelationshipDelete relReq = RealityDataRelationshipDelete("", "");//dummy
    RawServerResponse relationResponse = RawServerResponse();
    WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());

    for (size_t i = 0; i < s_serverNodes.size(); ++i)
        {
        idReq = RealityDataRelationshipByRealityDataIdRequest(s_serverNodes[i].GetInstanceId());
        entities = RealityDataService::Request(idReq, projectResponse);

        if (entities.size() == 0)
            DisplayInfo(Utf8PrintfString("Reality Data %d (%s) has no relationship\n", i, s_serverNodes[i].GetInstanceId().c_str()), DisplayOption::Error);
        else
            DisplayInfo(Utf8PrintfString("Reality Data %d (%s) removing %d relationships\n", i, s_serverNodes[i].GetInstanceId().c_str(), entities.size()), DisplayOption::Error);

        for(RealityDataRelationshipPtr entity : entities)
            {
            relReq = RealityDataRelationshipDelete(s_serverNodes[i].GetInstanceId(), entity->GetRelatedId());
 //           WSGRequest::GetInstance().PerformRequest(relReq, relationResponse, RealityDataService::GetVerifyPeer());
            if (relationResponse.body.ContainsI("errorMessage"))
                DisplayInfo(Utf8PrintfString("unlink RD %s from project %s failed with error:\n%s\n", s_serverNodes[i].GetInstanceId().c_str(), entity->GetRelatedId().c_str(), relationResponse.body.c_str()), DisplayOption::Error);
            }
        }
    
    DisplayInfo("Mass Unlink Complete\n", DisplayOption::Tip);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
void MassDelete()
    {
    std::string str, str2;

    DisplayInfo(Utf8PrintfString("Using this command will delete ALL %d entries listed above.\n", s_serverNodes.size()), DisplayOption::Question);
    DisplayInfo("Are you SURE? [ y / n ]", DisplayOption::Question);
    std::getline(*s_inputSource, str);
    if (str != "y" && str != "Y")
        return;

    str2 = Utf8PrintfString("%d", s_serverNodes.size()).c_str();

    DisplayInfo("To authenticate that you REALLY want to do this, please enter the number of entries to delete, as displayed above\n", DisplayOption::Question);
    std::getline(*s_inputSource, str);

    if (str != str2)
        return;

    ForceMassUnlink();

    DisplayInfo(Utf8PrintfString("Deleting all %d entries. Please be patient...\n", s_serverNodes.size()), DisplayOption::Tip);

    RawServerResponse rawResponse = RawServerResponse();
    RealityDataDeleteRequest realityDataReq = RealityDataDeleteRequest(""); //dummy

    bvector<Utf8String> errors = bvector<Utf8String>();

    for(size_t i = 0; i < s_serverNodes.size(); ++i)
        {
        realityDataReq = RealityDataDeleteRequest(s_serverNodes[i].GetInstanceId());
//        rawResponse = RealityDataService::BasicRequest(&realityDataReq);
        if (rawResponse.body.Contains("errorMessage"))
            errors.push_back(Utf8PrintfString("%s failed to delete with error:\n%s\n",s_serverNodes[i].GetInstanceId().c_str(), rawResponse.body.c_str()));
        else
            DisplayInfo(Utf8PrintfString("Deleted entry %d\n", i), DisplayOption::Tip);

        rawResponse.clear();
        }
    
    if(!errors.empty())
        {
        DisplayInfo("There was an error removing the following items:\n", DisplayOption::Error);
        for(size_t i = 0 ; i < errors.size() ; ++ i ) 
            DisplayInfo(errors[i], DisplayOption::Error);
        }

    DisplayInfo("Mass Delete Completed\n", DisplayOption::Tip);

    DisplayInfo("Listing all reality data remaining (added during the delete process or not deleted because of errors or insufficient rights):\n", DisplayOption::Info);

    ListRoots();

    }



int main(int argc, char* argv[])
    {
    SetConsoleTitle("RealityData Deleter");

    DisplayInfo("Welcome to the RealityData Deleter.\n", DisplayOption::Tip);
    DisplayInfo("This application will delete ALL Reality Data the currently logged CONNECT User has delete access to.\n", DisplayOption::Tip);
    DisplayInfo("the user must also have rights to add or delete relationships between reality data and CONNECT projects.\n", DisplayOption::Tip);
    DisplayInfo("Normally only an administrator should use this command that will delete all data belonging to his or her enterprise.\n", DisplayOption::Tip);
    DisplayInfo("IMPORTANT: The current user must NOT be an external contractor that contributed to another enterprise\nas an external consultant, otherwise data he or she is the owner/creator/has delete rights of data\nbelonging to this or these other enterprises may also be deleted.\n", DisplayOption::Tip);

    DisplayInfo("Do you want to continue (y/n)(Data to be deleted will be listed prior to final confirmation from user)?", DisplayOption::Question);
    Utf8String server;
    Utf8String answer;
    std::string input;
    std::getline(*s_inputSource, input);
    answer = Utf8String(input.c_str()).Trim();
    
    if (answer != "y" && answer != "Y")
        exit(0);

    server = MakeBuddiCall(0);

    if(server.empty())
        {
        DisplayInfo("ConnectionClient required for Deleter functionality. Cannot Proceed\n", DisplayOption::Error);
        exit(1);
        }
    else
        DisplayInfo(Utf8PrintfString("Connecting to %s\n", server.c_str()), DisplayOption::Tip);

    DisplayInfo("Retrieving version information. One moment...\n\n", DisplayOption::Tip);

    s_server = WSGServer(server, false);
    RawServerResponse versionResponse = RawServerResponse();
    Utf8String version = s_server.GetVersion(versionResponse);

    if (version.length() == 0)
        {
        DisplayInfo("There was an error contacting the server\n Please check that the URL was entered correctly and that the Connection Client is running and properly configured\n", DisplayOption::Error);
        exit(1);
        }

    Utf8String repo;
    Utf8String schema;

    RawServerResponse repoResponse = RawServerResponse();
    bvector<Utf8String> repoNames = s_server.GetRepositories(repoResponse);
    if (repoNames.size() == 0)
        {
        DisplayInfo("There was an error contacting the server. No repositories found\n --> Is your ConnectClient configured correctly?\n", DisplayOption::Error);
        exit(1);
        }
    else if (repoNames.size() == 1)
        {
        repo = repoNames[0];
        }
    else
        {
        DisplayInfo("There should only be one repository\n", DisplayOption::Error);
        exit(1);
        }

    if (repo.length() > 0)
        {
        DisplayInfo("\n", DisplayOption::Info);

        RawServerResponse schemaResponse = RawServerResponse();
        bvector<Utf8String> schemaNames = s_server.GetSchemaNames(repo, schemaResponse);

        if (schemaNames.size() == 0)
            {
            DisplayInfo("No schemas were found for the given server and repo\n", DisplayOption::Error);
            exit(1);
            }
        else
            {
            bool schemaFound = false;
            for (auto currentSchema : schemaNames)
                {
                if (currentSchema == "S3MX")
                    {
                    schemaFound = true;
            
                    break;
                    }
                }

            if (!schemaFound)
                {
                DisplayInfo("Appropriate schema not found error\n", DisplayOption::Error);
                exit(1);
                }
            }
        }
    else
        {
        DisplayInfo("Server configuration failed, invalid parameters passed\n", DisplayOption::Error);
        exit(1);
        }

    RealityDataService::SetServerComponents(server, version, repo, "S3MX");

    DisplayInfo("Server successfully configured, ready for use.\n", DisplayOption::Tip);
    DisplayInfo("Listing all reality data to be deleted sorted by owner (this may take some time):\n", DisplayOption::Info);
    DisplayInfo("This list contains all data accessible to current user but only data user has delete rights upon will be effectively deleted.:\n", DisplayOption::Info);

    ListRoots();

    MassDelete();

    return 0;
    }
