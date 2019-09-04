/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatformTools/RealityDataService.h>
#include <RealityPlatformTools/WSGServices.h>
#include <RealityPlatformTools/RealityConversionTools.h>


#include <stdio.h>
#include <conio.h>
#include <iomanip>
#include <sstream>  
#include <iostream>
#include <chrono>
#include <thread>

#include "RealityDataServicePerformanceTests.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM


static void silentErrorCallback(Utf8String basicMessage, const RawServerResponse& rawResponse)
    {
    }




static void statusFunc(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    if (ErrorCode > 0)
        std::cout << Utf8PrintfString("Curl error code : %d \n %s", ErrorCode, pMsg) << std::endl;
    else if (ErrorCode < 0)
        std::cout << pMsg << std::endl;
    }



static void uploadProgressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    }

static void downloadProgressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    }


//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
int main(int argc, char* argv[])
    {
    SetConsoleTitle("RealityDataService Performance Test");

    RealityDataServicePerformanceTests console = RealityDataServicePerformanceTests();

    if (argc == 2)
        console.Run(argv[1], 40);
    else
        console.Usage();

        //std::cout << "Press any key to continue" << std::endl;
    //getch();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
RealityDataServicePerformanceTests::RealityDataServicePerformanceTests() 
: m_server("", true)
    {
    double min_lon = 12.402;
    double min_lat = 23.502;
    double max_lon = 12.456;
    double max_lat = 23.513;

    m_newRealityData = RealityData::Create();
//    m_newRealityData->SetIdentifier("41d330bb-e1e6-45f3-a29b-02e145e31aa2");
    m_newRealityData->SetName("INTERNAL-ONLY TEST Reality Data");
    m_newRealityData->SetResolution("1.11x1.12");
    m_newRealityData->SetAccuracy("3.1");
    m_newRealityData->SetClassification(RealityDataBase::Classification::MODEL);
    m_newRealityData->SetVisibility(RealityDataBase::Visibility::PRIVATE);
    m_newRealityData->SetDataset("INTERNAL-ONLY TEST DATASET");
    m_newRealityData->SetRealityDataType("TEST_ONLY");
//    m_newRealityData->SetStreamed(false);
    m_newRealityData->SetThumbnailDocument("Thumnail.jpg");
    m_newRealityData->SetRootDocument("root.test");
    m_newRealityData->SetListable(true);
    m_newRealityData->SetMetadataUrl("metadata.xml");
//    m_newRealityData->SetCopyright("belongs to every one");
//    m_newRealityData->SetTermsOfUse("use wisely");
    m_newRealityData->SetGroup("TestGroup-185a9dbe-d9ed-4c87-9289-57beb3c94a1a");
    m_newRealityData->SetDescription("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aliquam vestibulum nunc quis malesuada varius. Donec at molestie enim, sit amet interdum mauris.\
                                    Sed dapibus ultricies orci, id dictum ligula consectetur vitae. Quisque eu ipsum in urna molestie ultricies. Nullam fringilla erat vitae placerat semper. Nulla consectetur justo lacinia, \
                                    lobortis mi et, feugiat arcu. Mauris ullamcorper sapien quis urna feugiat porta. Curabitur et velit quis dolor sodales commodo. \
                                    Interdum et malesuada fames ac ante ipsum primis in faucibus. Proin id eros felis. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas");

    bvector<GeoPoint2d> myFootprint;
    myFootprint.push_back(GeoPoint2d::From(min_lon, min_lat));
    myFootprint.push_back(GeoPoint2d::From(min_lon, max_lat));
    myFootprint.push_back(GeoPoint2d::From(max_lon, max_lat));
    myFootprint.push_back(GeoPoint2d::From(max_lon, min_lat));
    myFootprint.push_back(GeoPoint2d::From(min_lon, min_lat));

    m_newRealityData->SetFootprint(myFootprint);
    m_newRealityData->SetApproximateFootprint(false);


    // Create a temporary file
    char fileNameBuffer[1025];
    m_tempFileName = Utf8String(tmpnam(fileNameBuffer));

    // Fill the temporary file with data
    FILE* theFile = fopen(m_tempFileName.c_str(),"w");

    if (theFile == NULL)
        throw std::exception();

    char buffer[1024];
    for (int index = 1 ; index < 1000000; index++ /* Needed to advance past modulo 1000 */)
        
        {
        for (; (index % 1000) != 0; index++)
            buffer[index % 1000] = index % 255;
        
        fwrite(buffer, 1000, 1, theFile);
        }
    fclose(theFile);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
RealityDataServicePerformanceTests::~RealityDataServicePerformanceTests() 
    {
    // Destroy local file.
    remove(m_tempFileName.c_str());

    m_newRealityData = NULL;
    }
//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
void RealityDataServicePerformanceTests::Run(Utf8String serverName, int numberOfLoops)
    {
    timeStats dummyStats;

    // This first call has for purpose to wake up the service in case it has fallen asleep.
    ConfigureServerTest(serverName, dummyStats, true);
    
    RealityDataService::SetErrorCallback(silentErrorCallback);

    for (int i = 0 ; i < numberOfLoops ; i++)
        {
        bool relationshipCreated = false;
        timeStats currentTimeStats;
    
        if (SUCCESS == ConfigureServerTest(serverName, currentTimeStats))
            {
            if (SUCCESS == CreateRealityDataTest(currentTimeStats))
                {
                if (SUCCESS == CreateRelationshipToProject(currentTimeStats))
                    {
                    relationshipCreated = true;
                    }
    
                if (SUCCESS == UploadTest1(currentTimeStats))
                    {
                    DownloadTest(currentTimeStats);
    
                    GetDocumentTest(currentTimeStats);
    
                    GetRealityData(currentTimeStats);
    
                    GetRealityDataWithFilter(currentTimeStats);
    
                    GetRelationship(currentTimeStats);
    
                    UpdateTest(currentTimeStats);
                    }
    
                if (relationshipCreated)
                    {
                    DeleteRelationship(currentTimeStats);
                    }

                // This test must be called if creation was sucessful
                DeleteRealityDataTest(currentTimeStats);
    
                DataLocationStatTest(currentTimeStats);

                EnterpriseStatTest(currentTimeStats);

                ServiceStatTest(currentTimeStats);

                UserStatTest(currentTimeStats);

                }
            }

        m_listOfStats.push_back(currentTimeStats);
        }

    // Compute and print summary
    ComputeAndPrintStats();
    }
//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
void RealityDataServicePerformanceTests::ComputeAndPrintStats()
    {
        
        int64_t mean_handshake;
        int64_t mean_createRealityData;
        int64_t mean_createRelationship;
        int64_t mean_uploadTest;
        int64_t mean_downloadTest;
        int64_t mean_getDocument;
        int64_t mean_getRealityData;
        int64_t mean_listRealityData;
        int64_t mean_getRelationship;
        int64_t mean_modifyRealityData;
        int64_t mean_deleteRelationship;
        int64_t mean_deleteRealityData;
        int64_t mean_dataLocationStats;
        int64_t mean_enterpriseStats;
        int64_t mean_serviceStats;
        int64_t mean_userStats;

        int64_t total_handshake = 0;
        int64_t total_createRealityData = 0;
        int64_t total_createRelationship = 0;
        int64_t total_uploadTest = 0;
        int64_t total_downloadTest = 0;
        int64_t total_getDocument = 0;
        int64_t total_getRealityData = 0;
        int64_t total_listRealityData = 0;
        int64_t total_getRelationship = 0;
        int64_t total_modifyRealityData = 0;
        int64_t total_deleteRelationship = 0;
        int64_t total_deleteRealityData = 0;
        int64_t total_dataLocationStats = 0;
        int64_t total_enterpriseStats = 0;
        int64_t total_serviceStats = 0;
        int64_t total_userStats = 0;



        int64_t min_handshake               = m_listOfStats[0].m_handshake;
        int64_t min_createRealityData       = m_listOfStats[0].m_createRealityData;
        int64_t min_createRelationship      = m_listOfStats[0].m_createRelationship;
        int64_t min_uploadTest              = m_listOfStats[0].m_uploadTest;
        int64_t min_downloadTest            = m_listOfStats[0].m_downloadTest;
        int64_t min_getDocument             = m_listOfStats[0].m_getDocument;
        int64_t min_getRealityData          = m_listOfStats[0].m_getRealityData;
        int64_t min_listRealityData         = m_listOfStats[0].m_listRealityData;
        int64_t min_getRelationship         = m_listOfStats[0].m_getRelationship;
        int64_t min_modifyRealityData       = m_listOfStats[0].m_modifyRealityData;
        int64_t min_deleteRelationship      = m_listOfStats[0].m_deleteRelationship;
        int64_t min_deleteRealityData       = m_listOfStats[0].m_deleteRealityData;
        int64_t min_dataLocationStats       = m_listOfStats[0].m_dataLocationStats;
        int64_t min_enterpriseStats         = m_listOfStats[0].m_enterpriseStats;
        int64_t min_serviceStats            = m_listOfStats[0].m_serviceStats;
        int64_t min_userStats               = m_listOfStats[0].m_userStats;

        int64_t max_handshake               = m_listOfStats[0].m_handshake;
        int64_t max_createRealityData       = m_listOfStats[0].m_createRealityData;
        int64_t max_createRelationship      = m_listOfStats[0].m_createRelationship;
        int64_t max_uploadTest              = m_listOfStats[0].m_uploadTest;
        int64_t max_downloadTest            = m_listOfStats[0].m_downloadTest;
        int64_t max_getDocument             = m_listOfStats[0].m_getDocument;
        int64_t max_getRealityData          = m_listOfStats[0].m_getRealityData;
        int64_t max_listRealityData         = m_listOfStats[0].m_listRealityData;
        int64_t max_getRelationship         = m_listOfStats[0].m_getRelationship;
        int64_t max_modifyRealityData       = m_listOfStats[0].m_modifyRealityData;
        int64_t max_deleteRelationship      = m_listOfStats[0].m_deleteRelationship;
        int64_t max_deleteRealityData       = m_listOfStats[0].m_deleteRealityData;
        int64_t max_dataLocationStats       = m_listOfStats[0].m_dataLocationStats;
        int64_t max_enterpriseStats         = m_listOfStats[0].m_enterpriseStats;
        int64_t max_serviceStats            = m_listOfStats[0].m_serviceStats;
        int64_t max_userStats               = m_listOfStats[0].m_userStats;


        
        for (timeStats currentTimeStats: m_listOfStats)
            {

            min_handshake           = min(min_handshake            ,  currentTimeStats.m_handshake           );
            min_createRealityData   = min(min_createRealityData    ,  currentTimeStats.m_createRealityData   );
            min_createRelationship  = min(min_createRelationship   ,  currentTimeStats.m_createRelationship  );
            min_uploadTest          = min(min_uploadTest           ,  currentTimeStats.m_uploadTest          );
            min_downloadTest        = min(min_downloadTest         ,  currentTimeStats.m_downloadTest        );
            min_getDocument         = min(min_getDocument          ,  currentTimeStats.m_getDocument         );
            min_getRealityData      = min(min_getRealityData       ,  currentTimeStats.m_getRealityData      );
            min_listRealityData     = min(min_listRealityData      ,  currentTimeStats.m_listRealityData     );
            min_getRelationship     = min(min_getRelationship      ,  currentTimeStats.m_getRelationship     );
            min_modifyRealityData   = min(min_modifyRealityData    ,  currentTimeStats.m_modifyRealityData   );
            min_deleteRelationship  = min(min_deleteRelationship   ,  currentTimeStats.m_deleteRelationship  );
            min_deleteRealityData   = min(min_deleteRealityData    ,  currentTimeStats.m_deleteRealityData   );
            min_enterpriseStats     = min(min_enterpriseStats      ,  currentTimeStats.m_enterpriseStats     );
            min_serviceStats        = min(min_serviceStats         ,  currentTimeStats.m_serviceStats        );
            min_userStats           = min(min_userStats            ,  currentTimeStats.m_userStats           );



            max_handshake           = max(max_handshake            ,  currentTimeStats.m_handshake           );
            max_createRealityData   = max(max_createRealityData    ,  currentTimeStats.m_createRealityData   );
            max_createRelationship  = max(max_createRelationship   ,  currentTimeStats.m_createRelationship  );
            max_uploadTest          = max(max_uploadTest           ,  currentTimeStats.m_uploadTest          );
            max_downloadTest        = max(max_downloadTest         ,  currentTimeStats.m_downloadTest        );
            max_getDocument         = max(max_getDocument          ,  currentTimeStats.m_getDocument         );
            max_getRealityData      = max(max_getRealityData       ,  currentTimeStats.m_getRealityData      );
            max_listRealityData     = max(max_listRealityData      ,  currentTimeStats.m_listRealityData     );
            max_getRelationship     = max(max_getRelationship      ,  currentTimeStats.m_getRelationship     );
            max_modifyRealityData   = max(max_modifyRealityData    ,  currentTimeStats.m_modifyRealityData   );
            max_deleteRelationship  = max(max_deleteRelationship   ,  currentTimeStats.m_deleteRelationship  );
            max_deleteRealityData   = max(max_deleteRealityData    ,  currentTimeStats.m_deleteRealityData   );
            max_enterpriseStats     = max(max_enterpriseStats      ,  currentTimeStats.m_enterpriseStats     );
            max_serviceStats        = max(max_serviceStats         ,  currentTimeStats.m_serviceStats        );
            max_userStats           = max(max_userStats            ,  currentTimeStats.m_userStats           );


            total_handshake           += currentTimeStats.m_handshake         ;
            total_createRealityData   += currentTimeStats.m_createRealityData ;
            total_createRelationship  += currentTimeStats.m_createRelationship;
            total_uploadTest          += currentTimeStats.m_uploadTest        ;
            total_downloadTest        += currentTimeStats.m_downloadTest      ;
            total_getDocument         += currentTimeStats.m_getDocument       ;
            total_getRealityData      += currentTimeStats.m_getRealityData    ;
            total_listRealityData     += currentTimeStats.m_listRealityData   ;
            total_getRelationship     += currentTimeStats.m_getRelationship   ;
            total_modifyRealityData   += currentTimeStats.m_modifyRealityData ;
            total_deleteRelationship  += currentTimeStats.m_deleteRelationship;
            total_deleteRealityData   += currentTimeStats.m_deleteRealityData ;
            total_enterpriseStats     += currentTimeStats.m_enterpriseStats   ;
            total_serviceStats        += currentTimeStats.m_serviceStats      ;
            total_userStats           += currentTimeStats.m_userStats         ;


            }
        mean_handshake           = total_handshake           / m_listOfStats.size();       
        mean_createRealityData   = total_createRealityData   / m_listOfStats.size();
        mean_createRelationship  = total_createRelationship  / m_listOfStats.size();
        mean_uploadTest          = total_uploadTest          / m_listOfStats.size();
        mean_downloadTest        = total_downloadTest        / m_listOfStats.size();
        mean_getDocument         = total_getDocument         / m_listOfStats.size();
        mean_getRealityData      = total_getRealityData      / m_listOfStats.size();
        mean_listRealityData     = total_listRealityData     / m_listOfStats.size();
        mean_getRelationship     = total_getRelationship     / m_listOfStats.size();
        mean_modifyRealityData   = total_modifyRealityData   / m_listOfStats.size();
        mean_deleteRelationship  = total_deleteRelationship  / m_listOfStats.size();      
        mean_deleteRealityData   = total_deleteRealityData   / m_listOfStats.size();
        mean_dataLocationStats   = total_dataLocationStats   / m_listOfStats.size();
        mean_enterpriseStats     = total_enterpriseStats     / m_listOfStats.size();
        mean_serviceStats        = total_serviceStats        / m_listOfStats.size();
        mean_userStats           = total_userStats           / m_listOfStats.size();

        std::cout << "  TEST                                        mean              min             max" << std::endl;
        std::cout << "WSG Handshake                            : " << mean_handshake           << ",         " << min_handshake             << ",           " << max_handshake           << std::endl;
        std::cout << "Create RealityData                       : " << mean_createRealityData   << ",         " << min_createRealityData     << ",           " << max_createRealityData   << std::endl;
        std::cout << "Create RealityDataRelationship           : " << mean_createRelationship  << ",         " << min_createRelationship    << ",           " << max_createRelationship  << std::endl;
        std::cout << "Upload                                   : " << mean_uploadTest          << ",         " << min_uploadTest            << ",           " << max_uploadTest          << std::endl;
        std::cout << "Download                                 : " << mean_downloadTest        << ",         " << min_downloadTest          << ",           " << max_downloadTest        << std::endl;
        std::cout << "Get Document                             : " << mean_getDocument         << ",         " << min_getDocument           << ",           " << max_getDocument         << std::endl;
        std::cout << "Get RealityData                          : " << mean_getRealityData      << ",         " << min_getRealityData        << ",           " << max_getRealityData      << std::endl;
        std::cout << "List RealityData                         : " << mean_listRealityData     << ",         " << min_listRealityData       << ",           " << max_listRealityData     << std::endl;
        std::cout << "GetRelationship                          : " << mean_getRelationship     << ",         " << min_getRelationship       << ",           " << max_getRelationship     << std::endl;
        std::cout << "Modify RealityData                       : " << mean_modifyRealityData   << ",         " << min_modifyRealityData     << ",           " << max_modifyRealityData   << std::endl;
        std::cout << "Delete Relationship                      : " << mean_deleteRelationship  << ",         " << min_deleteRelationship    << ",           " << max_deleteRelationship  << std::endl;
        std::cout << "Delete Reality Data                      : " << mean_deleteRealityData   << ",         " << min_deleteRealityData     << ",           " << max_deleteRealityData   << std::endl;
        std::cout << "Data Locations                           : " << mean_dataLocationStats   << ",         " << min_dataLocationStats     << ",           " << max_dataLocationStats   << std::endl;
        std::cout << "Enterprise stats                         : " << mean_enterpriseStats     << ",         " << min_enterpriseStats       << ",           " << max_enterpriseStats     << std::endl;
        std::cout << "Service stats                            : " << mean_serviceStats        << ",         " << min_serviceStats          << ",           " << max_serviceStats        << std::endl;
        std::cout << "User stats                               : " << mean_userStats           << ",         " << min_userStats             << ",           " << max_userStats           << std::endl;

    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
void RealityDataServicePerformanceTests::Usage()
    {
    DisplayInfo ("  RealityDataServicePerformanceTests tool for RDS V1.0\n\n");
    DisplayInfo ("  Performs automated performance texting and reports performance for each operations.\n");
    DisplayInfo ("  Usage:\n");
    DisplayInfo ("  RealityDataServicePerformanceTests <server>.\n");
    DisplayInfo ("  Where server is the name of the server ex: dev-realitydataservices-eus.cloudapp.net\n .\n");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::ConfigureServerTest(Utf8String serverName, timeStats& theTimeStats, bool silent)
    {
    bool verifyCertificate = false;

    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);


    m_server = WSGServer(serverName, verifyCertificate);
    RawServerResponse response;

    Utf8String version = m_server.GetVersion(response);
    if (version.size() == 0)
        return ERROR;

    if (response != OK)
        return ERROR;

    Utf8String repo = "S3MXECPlugin--Server";
    Utf8String schema = "S3MX";
    
    bvector<Utf8String> repoNames = m_server.GetRepositories(response);
    if(repoNames.size() == 0)
        {
        DisplayInfo("There was an error contacting the server. No repositories found\n", DisplayOption::Error);
        return ERROR;
        }
    
    if (response != OK)
        return ERROR;

    // Make sure repository S3MX is part of the list
    bool foundS3MXPlugIn = false;

    for (int index = 0 ; !foundS3MXPlugIn && index < repoNames.size() ; index++)
        {
        foundS3MXPlugIn = (repoNames[index] == repo);
        }

    if (!foundS3MXPlugIn)
        {           
        DisplayInfo("Server does not contain S3MXECPlugin--Server repository\n", DisplayOption::Error);
        return ERROR;
        }


    bvector<Utf8String> schemaNames = m_server.GetSchemaNames(repo, response);

    if (response != OK)
        return ERROR;

    if (schemaNames.size() == 0)
        {
        DisplayInfo("No schemas were found for repository S3MXECPlugin--Server\n", DisplayOption::Error);
        return ERROR;
        }

    // Make sure schema S3MX is part of the list
    bool foundS3MX = false;
    for (int index = 0 ; !foundS3MX && index < schemaNames.size() ; index++)
        {
        foundS3MX = (schemaNames[index] == schema);
        }

    if (!foundS3MX)
        {           
        DisplayInfo("Server does not contain S3MX repository\n", DisplayOption::Error);
        return ERROR;
        }

    RealityDataService::SetServerComponents(serverName, version, repo, schema);
    RealityDataService::SetUserAgent("RealityData ServicePerformanceTest-  dummy user agent");
    m_server = WSGServer(RealityDataService::GetServerName(), verifyCertificate);
   
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (!silent)
        {
        if (SUCCESS != response.status)
            std::cout << "WSG Handshake Test: Failure no: " << response.status << std::endl;
        else
            std::cout << "WSG Handshake Test : " << (endTime - startTime) << std::endl;
        }

    theTimeStats.m_handshake = (endTime - startTime);

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::CreateRealityDataTest(timeStats& theTimeStats)
    {
    RawServerResponse response;
    bvector<RealityDataPtr> entities = bvector<RealityDataPtr>();

    // Clear id so we get a new one
    m_newRealityData->SetIdentifier("");

    RealityDataCreateRequest creationRequest(*m_newRealityData); 
    
    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    Utf8String realityDataBody = RealityDataService::Request(creationRequest, response);
  
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    Json::Value instance(Json::objectValue);
    Json::Reader::Parse(realityDataBody, instance);
    if (response.status == RequestStatus::OK && !instance["changedInstance"].isNull() && !instance["changedInstance"]["instanceAfterChange"].isNull() && !instance["changedInstance"]["instanceAfterChange"]["instanceId"].isNull())
        {
        m_newRealityData->SetIdentifier(instance["changedInstance"]["instanceAfterChange"]["instanceId"].asString().c_str());
        }

    // Report
    if (SUCCESS != response.status)
        std::cout << "Creation RealityData Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Creation RealityData " << m_newRealityData->GetIdentifier() << ": " << (endTime - startTime) << std::endl;
  
    theTimeStats.m_createRealityData = (endTime - startTime);
    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::CreateRelationshipToProject(timeStats& theTimeStats)
    {
    RawServerResponse response;

    int64_t startTime;
    int64_t endTime;

    RealityDataRelationshipPtr relationship = RealityDataRelationship::Create();

    RealityDataRelationshipCreateRequest creationRelationshipRequest(m_newRealityData->GetIdentifier(), "toto");

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    RealityDataService::Request(creationRelationshipRequest, response);
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (OK != response.status)
        std::cout << "Create RealityDataRelationship: Failure no: " << response.status << std::endl;
    else
        std::cout << "Create RealityDataRelationship Test: " << (endTime - startTime) << std::endl;
    
    theTimeStats.m_createRelationship = (endTime - startTime);

    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::UploadTest1(timeStats& theTimeStats)
    {
    RawServerResponse response;

    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Upload file
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    properties.Insert(RealityDataField::Name, m_newRealityData->GetName());
    properties.Insert(RealityDataField::Classification, m_newRealityData->GetClassificationTag());
    properties.Insert(RealityDataField::Type, m_newRealityData->GetRealityDataType());
    properties.Insert(RealityDataField::Visibility, m_newRealityData->GetVisibilityTag());
    properties.Insert(RealityDataField::RootDocument, m_newRealityData->GetRootDocument());

    Utf8String formatedProps = RealityDataServiceUpload::PackageProperties(properties);
    RealityDataServiceUpload upload = RealityDataServiceUpload(BeFileName(m_tempFileName), m_newRealityData->GetIdentifier(), formatedProps, true, true, statusFunc);
    upload.SetProgressCallBack(uploadProgressFunc);
    upload.SetProgressStep(0.1);
    upload.OnlyReportErrors(true);
    const TransferReport& tReport = upload.Perform();
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (tReport.results.size() != 0)
        std::cout << "Upload Files Test error: file failed to upload" << std::endl;
    else
        std::cout << "Upload Files Test: " << (endTime - startTime) << std::endl;

    theTimeStats.m_uploadTest = (endTime - startTime);

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::UploadTest2(timeStats& theTimeStats)
    {
    RawServerResponse response;

    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Upload file
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    properties.Insert(RealityDataField::Name, m_newRealityData->GetName());
    properties.Insert(RealityDataField::Classification, m_newRealityData->GetClassificationTag());
    properties.Insert(RealityDataField::Type, m_newRealityData->GetRealityDataType());
    properties.Insert(RealityDataField::Visibility, m_newRealityData->GetVisibilityTag());
    properties.Insert(RealityDataField::RootDocument, m_newRealityData->GetRootDocument());

    Utf8String formatedProps = RealityDataServiceUpload::PackageProperties(properties);
    RealityDataServiceUpload upload = RealityDataServiceUpload(BeFileName(m_tempFileName), m_newRealityData->GetIdentifier() + "\\DummyDirectory", formatedProps, true, true, statusFunc);
    upload.SetProgressCallBack(uploadProgressFunc);
    upload.SetProgressStep(0.1);
    upload.OnlyReportErrors(true);
    const TransferReport& tReport = upload.Perform();
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (tReport.results.size() != 0)
        std::cout << "Upload Test error: file failed to upload" << std::endl;
    else
        std::cout << "Upload Test: " << (endTime - startTime) << std::endl;

    // theTimeStats.m_upload2 = (endTime - startTime);

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::DownloadTest(timeStats& theTimeStats)
    {
    RawServerResponse response;
    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Create a temporary file
    char fileNameBuffer[1025];
    Utf8String downloadFileName = Utf8String(tmpnam(fileNameBuffer));

    BeFileName beFile(m_tempFileName.c_str());

    Utf8String namePart = Utf8String(beFile.GetFileNameAndExtension().c_str());

    Utf8String nodeId = m_newRealityData->GetIdentifier() + "/" + namePart;

    RealityDataServiceDownload download = RealityDataServiceDownload(BeFileName(downloadFileName), nodeId);
    download.SetProgressCallBack(downloadProgressFunc);
    download.SetProgressStep(0.1);
    download.OnlyReportErrors(true);
    const TransferReport& tReport = download.Perform();


    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (tReport.results.size() != 0)
        std::cout << "Download Files error: file failed to download" << std::endl;
    else
        std::cout << "Download Files Test: " << (endTime - startTime) << std::endl;

    theTimeStats.m_downloadTest = (endTime - startTime);

    return SUCCESS;
    }




//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::UpdateTest(timeStats& theTimeStats)
    {
    // Modify the reality data
    m_newRealityData->SetMetadataUrl("");
    m_newRealityData->SetGroup("TestGroup-1");
    m_newRealityData->SetDescription("No text anymore");

    // Update on server
    RawServerResponse response;

    RealityDataChangeRequest changeRequest(*m_newRealityData); 
    
    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    RealityDataService::Request(changeRequest, response);
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (SUCCESS != response.status)
        std::cout << "Modify RealityData Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Modify RealityData Test: " << (endTime - startTime) << std::endl;
  
    theTimeStats.m_modifyRealityData = (endTime - startTime);

    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::GetRelationship(timeStats& theTimeStats)
    {
    RawServerResponse response;

    int64_t startTime = 0;
    int64_t endTime = 0;
    
    RealityDataRelationshipByRealityDataIdRequest myRequest(m_newRealityData->GetIdentifier());

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    bvector<RealityDataRelationshipPtr> listOfRel = RealityDataService::Request(myRequest, response);
    
    if (listOfRel.size() == 0 && OK == response.status)
        {
        std::cout << "Get Relaitionships Test: No error returned but no relationship fetched " << std::endl;
        return ERROR;
        }

    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (OK != response.status)
        std::cout << "Get RealityDataRelationship: Failure no: " << response.status << std::endl;
    else
        std::cout << "Get RealityDataRelationship Test: " << (endTime - startTime) << std::endl;
    
    theTimeStats.m_getRelationship = (endTime - startTime);

    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::DeleteRelationship(timeStats& theTimeStats)
    {
    RawServerResponse response;

    int64_t startTime = 0;
    int64_t endTime = 0;
    
    RealityDataRelationshipDelete myRequest(m_newRealityData->GetIdentifier(), "toto");

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    RealityDataService::Request(myRequest, response);
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (OK != response.status)
        std::cout << "Delete RealityDataRelationship: Failure no: " << response.status << std::endl;
    else
        std::cout << "Delete RealityDataRelationship Test: " << (endTime - startTime) << std::endl;

    theTimeStats.m_deleteRelationship = (endTime - startTime);
    
    return (StatusInt)response.status;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::DeleteRealityDataTest(timeStats& theTimeStats, bool silent)
    {
    RawServerResponse response;
  
    RealityDataDeleteRequest deletionRequest(m_newRealityData->GetIdentifier()); 
    
    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    RealityDataService::Request(deletionRequest, response);
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (!silent)
        {
        if (OK != response.status)
            std::cout << "Delete RealityData: Failure no: " << response.status << std::endl;
        else
            std::cout << "Delete RealityData Test: " << (endTime - startTime) << std::endl;
        }

    theTimeStats.m_deleteRealityData = (endTime - startTime);

    return (StatusInt)response.status;    
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::GetRealityData(timeStats& theTimeStats)
    {

    RealityDataByIdRequest request = RealityDataByIdRequest(m_newRealityData->GetIdentifier());

    RawServerResponse response;


    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    RealityDataPtr otherRealityData = RealityDataService::Request(request, response);

    if (otherRealityData.IsNull() && OK == response.status)
        {
        std::cout << "GetRealityData Test: No error returned but no reality data fetched " << std::endl;
        return ERROR;
        }
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (SUCCESS != response.status)
        std::cout << "Get RealityData Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Get RealityData Test: " << (endTime - startTime) << std::endl;


    theTimeStats.m_getRealityData = (endTime - startTime);

    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::GetRealityDataWithFilter(timeStats& theTimeStats)
    {

    RealityDataListByUltimateIdPagedRequest organizationReq = RealityDataListByUltimateIdPagedRequest("", 0, 2500);
    organizationReq.SetQuery("ONLY");

    RawServerResponse organizationResponse = RawServerResponse();
    organizationResponse.status = RequestStatus::OK;
    bvector<RealityDataPtr> organizationVec = bvector<RealityDataPtr>();
    bvector<RealityDataPtr> partialVec;


    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    while(organizationResponse.status == RequestStatus::OK)
        {//When LASTPAGE has been added, loop will exit
        partialVec = RealityDataService::Request(organizationReq, organizationResponse);
        organizationVec.insert(organizationVec.end(), partialVec.begin(), partialVec.end());
        }

    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    std::cout << "List RealityData with filter Test: " << (endTime - startTime) << std::endl;

    theTimeStats.m_listRealityData = (endTime - startTime);

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::GetRealityDataWithPolygon(timeStats& theTimeStats)
    {

    double min_lon = 12.402;
    double min_lat = 23.502;
    double max_lon = 12.403;
    double max_lat = 23.503;


    bvector<GeoPoint2d> myFootprint;
    myFootprint.push_back(GeoPoint2d::From(min_lon, min_lat));
    myFootprint.push_back(GeoPoint2d::From(min_lon, max_lat));
    myFootprint.push_back(GeoPoint2d::From(max_lon, max_lat));
    myFootprint.push_back(GeoPoint2d::From(max_lon, min_lat));
    myFootprint.push_back(GeoPoint2d::From(min_lon, min_lat));




    RealityDataListByUltimateIdPagedRequest organizationReq = RealityDataListByUltimateIdPagedRequest("", 0, 2500);

    bvector<RDSFilter> properties = bvector<RDSFilter>();
    properties.push_back(RealityDataFilterCreator::FilterSpatial(myFootprint, 4326));

    organizationReq.SetFilter(RealityDataFilterCreator::GroupFiltersAND(properties));

    RawServerResponse organizationResponse = RawServerResponse();
    organizationResponse.status = RequestStatus::OK;
    bvector<RealityDataPtr> organizationVec = bvector<RealityDataPtr>();
    bvector<RealityDataPtr> partialVec;

    while(organizationResponse.status == RequestStatus::OK)
        {//When LASTPAGE has been added, loop will exit
        partialVec = RealityDataService::Request(organizationReq, organizationResponse);
        organizationVec.insert(organizationVec.end(), partialVec.begin(), partialVec.end());
        }

    return SUCCESS;
    }




//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::GetDocumentTest(timeStats& theTimeStats)
    {



    RawServerResponse response;

    BeFileName beFile(m_tempFileName.c_str());

    Utf8String namePart = Utf8String(beFile.GetFileNameAndExtension().c_str());

    Utf8String documentId = m_newRealityData->GetIdentifier() + "/" + namePart;
    RealityDataDocumentByIdRequest request = RealityDataDocumentByIdRequest(documentId);


    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    RealityDataDocumentPtr otherRealityDataDocument = RealityDataService::Request(request, response);

    if (otherRealityDataDocument.IsNull() && OK == response.status)
        {
        std::cout << "GetDocument Test: No error returned but no reality data fetched " << std::endl;
        return ERROR;
        }
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (SUCCESS != response.status)
        std::cout << "Get Document Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Get Document Test: " << (endTime - startTime) << std::endl;

    theTimeStats.m_getDocument = (endTime - startTime);

    return (StatusInt)response.status;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::GetFolderTest(timeStats& theTimeStats)
    {
    RawServerResponse response;

    BeFileName beFile(m_tempFileName.c_str());

    Utf8String namePart = Utf8String(beFile.GetFileNameAndExtension().c_str());

    Utf8String folderId = m_newRealityData->GetIdentifier() + "/" + namePart;
    RealityDataFolderByIdRequest request = RealityDataFolderByIdRequest(folderId);


    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    RealityDataFolderPtr otherRealityDataFolder = RealityDataService::Request(request, response);

    if (otherRealityDataFolder.IsNull() && OK == response.status)
        {
        std::cout << "GetFolder Test: No error returned but no folder fetched " << std::endl;
        return ERROR;
        }
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (SUCCESS != response.status)
        std::cout << "GetFolder Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "GetFolder Test: " << (endTime - startTime) << std::endl;


    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::DeleteDocumentTest(timeStats& theTimeStats)
    {
    RawServerResponse response;

    BeFileName beFile(m_tempFileName.c_str());

    Utf8String namePart = Utf8String(beFile.GetFileNameAndExtension().c_str());

    Utf8String documentId = m_newRealityData->GetIdentifier() + "/" + namePart;
    RealityDataDeleteDocument request = RealityDataDeleteDocument(documentId);


    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    RealityDataService::Request(request, response);
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (SUCCESS != response.status)
        std::cout << "DeleteDocument Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "DeleteDocument Test: " << (endTime - startTime) << std::endl;


    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::DataLocationStatTest(timeStats& theTimeStats)
    {
    RawServerResponse response;
    AllRealityDataLocationsRequest ptt;
    bvector<RealityDataLocation> locations;

    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    locations = RealityDataService::Request(ptt, response);
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (SUCCESS != response.status)
        std::cout << "Get Enterprise stats Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Get Enterprise stats Test: " << (endTime - startTime) << std::endl;

    theTimeStats.m_dataLocationStats = (endTime - startTime);


    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::EnterpriseStatTest(timeStats& theTimeStats)
    {
    RawServerResponse response;
    RealityDataEnterpriseStatRequest ptt("");
    RealityDataEnterpriseStat stat;

    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    RealityDataService::Request(ptt, stat, response);
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (SUCCESS != response.status)
        std::cout << "Get Enterprise stats Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Get Enterprise stats Test: " << (endTime - startTime) << std::endl;

    theTimeStats.m_enterpriseStats = (endTime - startTime);


    return (StatusInt)response.status;
    }
//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::ServiceStatTest(timeStats& theTimeStats)
    {
    RawServerResponse response;
    RealityDataServiceStatRequest ptt("");
    bvector<RealityDataServiceStat> stats;

    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    stats = RealityDataService::Request(ptt, response);
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (SUCCESS != response.status)
        std::cout << "Get Service stats Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Get Service stats Test: " << (endTime - startTime) << std::endl;

    theTimeStats.m_serviceStats = (endTime - startTime);


    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::UserStatTest(timeStats& theTimeStats)
    {
    RawServerResponse response;
    RealityDataUserStatRequest ptt("");
    bvector<RealityDataUserStat> stats;

    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    stats = RealityDataService::Request(ptt, response);
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (SUCCESS != response.status)
        std::cout << "Get User stats Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Get User stats Test: " << (endTime - startTime) << std::endl;

    theTimeStats.m_userStats = (endTime - startTime);


    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
void RealityDataServicePerformanceTests::DisplayInfo(Utf8StringCR msg, DisplayOption option)
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

    SetConsoleTextAttribute(m_hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
    }


