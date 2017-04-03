/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/PerformanceTests/RealityDataServicePerformanceTests.cpp $
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
        console.Run(argv[1]);
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
    m_newRealityData->SetIdentifier("41d330bb-e1e6-45f3-a29b-02e145e31aa2");
    m_newRealityData->SetName("INTERNAL-ONLY TEST Reality Data");
    m_newRealityData->SetResolution("1.11x1.12");
    m_newRealityData->SetAccuracy("3.1x4.2");
    m_newRealityData->SetClassification(RealityDataBase::Classification::MODEL);
    m_newRealityData->SetVisibility(RealityDataBase::Visibility::PRIVATE);
    m_newRealityData->SetDataset("INTERNAL-ONLY TEST DATASET");
    m_newRealityData->SetRealityDataType("TEST_ONLY");
    m_newRealityData->SetThumbnailDocument("Thumnail.jpg");
    m_newRealityData->SetRootDocument("root.test");
    m_newRealityData->SetListable(true);
    m_newRealityData->SetMetadataURL("metadata.xml");
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
void RealityDataServicePerformanceTests::Run(Utf8String serverName)
    {
    bool relationshipCreated = false;

    RealityDataService::SetErrorCallback(silentErrorCallback);

    // This first call has for purpose to wake up the service in case it has fallen asleep.
    ConfigureServerTest(serverName, true);

    //// Clean existing entry in case program previously failed.
    //DeleteRealityDataTest(true);

    //// This sleep is require in case the deleting did occur ... the Azure container becomes unusable for a moment then
    //// sleep(5000);
    //std::this_thread::sleep_for(std::chrono::milliseconds(25000));



    if (SUCCESS == ConfigureServerTest(serverName))
        {
        if (SUCCESS == CreateRealityDataTest())
            {
            if (SUCCESS == CreateRelationshipToProject())
                {
                relationshipCreated = true;
                }

            if (SUCCESS == UploadTest1())
                {
                DownloadTest();

                InformationExtractionTest();

                GetRealityData();

                GetRealityDataWithFilter();

                GetRelationship();

                UpdateTest();
                }

            if (UploadTest2())
                {
                GetFolderTest();
                }

            if (relationshipCreated)
                {
                DeleteRelationship();
                }

            // This test must be called if creation was sucessful
            DeleteRealityDataTest();

            EnterpriseStatTest();

            }
        }
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
StatusInt RealityDataServicePerformanceTests::ConfigureServerTest(Utf8String serverName, bool silent)
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
    m_server = WSGServer(RealityDataService::GetServerName(), verifyCertificate);

   
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (!silent)
        {
        if (SUCCESS != response.status)
            std::cout << "Handshake Test: Failure no: " << response.status << std::endl;
        else
            std::cout << "Handshake Test : " << (endTime - startTime) / 1000.0 << std::endl;
        }

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::CreateRealityDataTest()
    {
    RawServerResponse response;

    RealityDataCreateRequest creationRequest(*m_newRealityData); 
    
    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    RealityDataService::Request(creationRequest, response);
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (SUCCESS != response.status)
        std::cout << "Creation Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Creation Test: " << (endTime - startTime) / 1000.0 << std::endl;
  
    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::CreateRelationshipToProject()
    {
    RawServerResponse response;

    int64_t startTime;
    int64_t endTime;

    RealityDataProjectRelationshipPtr relationship = RealityDataProjectRelationship::Create();

    RealityDataRelationshipCreateRequest creationRelationshipRequest(m_newRealityData->GetIdentifier(), "toto");

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    RealityDataService::Request(creationRelationshipRequest, response);
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (OK != response.status)
        std::cout << "Relationship creation Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Relationship creation Test: " << (endTime - startTime) / 1000.0 << std::endl;
    
    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::UploadTest1()
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
        std::cout << "Upload Test error: file failed to upload" << std::endl;
    else
        std::cout << "Upload Test: " << ((endTime - startTime) / 1000.0) << std::endl;

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::UploadTest2()
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
        std::cout << "Upload Test error: file failed to upload" << std::endl;
    else
        std::cout << "Upload Test: " << ((endTime - startTime) / 1000.0) << std::endl;

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::DownloadTest()
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
        std::cout << "Download Test error: file failed to download" << std::endl;
    else
        std::cout << "Download Test: " << (endTime - startTime) / 1000.0 << std::endl;

    return SUCCESS;
    }



//-------------------------------------------------------------------------------------
// This test will extract information about the reality data, folders and documents
// and measure the time taken to perform the operation.
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::InformationExtractionTest()
    {
    RawServerResponse response;

    int64_t startTime;
    int64_t endTime;
    
    // Obtain information about the reality data 
    RealityDataByIdRequest myRealityDataRequest(m_newRealityData->GetIdentifier());

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    RealityDataPtr myRealityData = RealityDataService::Request(myRealityDataRequest, response);
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Basic check
    if (myRealityData->GetIdentifier() != m_newRealityData->GetIdentifier())
        return ERROR;

    // Report
    if (OK != response.status)
        std::cout << "Information Extraction Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Information Extraction Test: " << (endTime - startTime) / 1000.0 << std::endl;
    
    return (StatusInt)response.status;
    }




//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::UpdateTest()
    {
    // Modify the reality data
    m_newRealityData->SetMetadataURL("");
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
        std::cout << "Change Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Change Test: " << (endTime - startTime) / 1000.0 << std::endl;
  
    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::GetRelationship()
    {
    RawServerResponse response;

    int64_t startTime = 0;
    int64_t endTime = 0;
    
    RealityDataProjectRelationshipByRealityDataIdRequest myRequest(m_newRealityData->GetIdentifier());

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    bvector<RealityDataProjectRelationshipPtr> listOfRel = RealityDataService::Request(myRequest, response);
    
    if (listOfRel.size() == 0 && OK == response.status)
        {
        std::cout << "Get Relaitionships Test: No error returned but no relationship fetched " << std::endl;
        return ERROR;
        }

    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (OK != response.status)
        std::cout << "Relationships Get Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Relationships Get Test: " << (endTime - startTime) / 1000.0 << std::endl;
    
    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::DeleteRelationship()
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
        std::cout << "Relationship deletion Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Relationship deletion Test: " << (endTime - startTime) / 1000.0 << std::endl;
    
    return (StatusInt)response.status;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::DeleteRealityDataTest(bool silent)
    {
    RawServerResponse response;
  
    RealityDataDelete deletionRequest(m_newRealityData->GetIdentifier()); 
    
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
            std::cout << "Deletion Test: Failure no: " << response.status << std::endl;
        else
            std::cout << "Deletion Test: " << (endTime - startTime) / 1000.0 << std::endl;
        }

    return (StatusInt)response.status;    
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::GetRealityData()
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
        std::cout << "GetRealityData Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "GetRealityData Test: " << (endTime - startTime) / 1000.0 << std::endl;


    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::GetRealityDataWithFilter()
    {

    RealityDataListByEnterprisePagedRequest enterpriseReq = RealityDataListByEnterprisePagedRequest("", 0, 2500);

    bvector<Utf8String> properties = bvector<Utf8String>();
    properties.push_back(RealityDataFilterCreator::FilterByName("ONLY"));

    enterpriseReq.SetFilter(RealityDataFilterCreator::GroupFiltersAND(properties));

    RawServerResponse enterpriseResponse = RawServerResponse();
    enterpriseResponse.status = RequestStatus::OK;
    bvector<RealityDataPtr> enterpriseVec = bvector<RealityDataPtr>();
    bvector<RealityDataPtr> partialVec;

    while(enterpriseResponse.status == RequestStatus::OK)
        {//When LASTPAGE has been added, loop will exit
        partialVec = RealityDataService::Request(enterpriseReq, enterpriseResponse);
        enterpriseVec.insert(enterpriseVec.end(), partialVec.begin(), partialVec.end());
        }

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::GetRealityDataWithPolygon()
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




    RealityDataListByEnterprisePagedRequest enterpriseReq = RealityDataListByEnterprisePagedRequest("", 0, 2500);

    bvector<Utf8String> properties = bvector<Utf8String>();
    properties.push_back(RealityDataFilterCreator::FilterSpatial(myFootprint, 4326));

    enterpriseReq.SetFilter(RealityDataFilterCreator::GroupFiltersAND(properties));

    RawServerResponse enterpriseResponse = RawServerResponse();
    enterpriseResponse.status = RequestStatus::OK;
    bvector<RealityDataPtr> enterpriseVec = bvector<RealityDataPtr>();
    bvector<RealityDataPtr> partialVec;

    while(enterpriseResponse.status == RequestStatus::OK)
        {//When LASTPAGE has been added, loop will exit
        partialVec = RealityDataService::Request(enterpriseReq, enterpriseResponse);
        enterpriseVec.insert(enterpriseVec.end(), partialVec.begin(), partialVec.end());
        }

    return SUCCESS;
    }




//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::GetDocumentTest()
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
        std::cout << "GetDocument Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "GetDocument Test: " << (endTime - startTime) / 1000.0 << std::endl;


    return (StatusInt)response.status;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::DeleteDocumentTest()
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
        std::cout << "DeleteDocument Test: " << (endTime - startTime) / 1000.0 << std::endl;


    return (StatusInt)response.status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::EnterpriseStatTest()
    {
    RawServerResponse response;
    RealityDataEnterpriseStatRequest ptt("");
    uint64_t NbRealityData;
    uint64_t TotalSizeKB;

    int64_t startTime;
    int64_t endTime;

    // Start time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(startTime);

    // Perform operation
    RealityDataService::Request(ptt, &NbRealityData, &TotalSizeKB, response);
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (SUCCESS != response.status)
        std::cout << "Enterprise stats Test: Failure no: " << response.status << std::endl;
    else
        std::cout << "Enterprise stats Test: " << (endTime - startTime) / 1000.0 << std::endl;


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


