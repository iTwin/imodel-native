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

#include "RealityDataServicePerformanceTests.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

static void statusFunc(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    if (ErrorCode > 0)
        std::cout << Utf8PrintfString("Curl error code : %d \n %s", ErrorCode, pMsg) << std::endl;
    else if (ErrorCode < 0)
        std::cout << pMsg << std::endl;
    }



static void uploadProgressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    char progressString[1024];
    sprintf(progressString, "upload percent : %f\r", repoProgress * 100.0);
    std::cout << progressString ;
	
    }

static void downloadProgressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    char progressString[1024];
    sprintf(progressString, "percentage of files downloaded : %f\r", repoProgress * 100.0);
    std::cout << progressString;
	
//DMxx    RealityDataConsole::DisplayInfo (Utf8PrintfString("percentage of files downloaded : %f\r", repoProgress * 100.0)); //, RealityDataConsole::Tip);
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
    if (SUCCESS == ConfigureServerTest(serverName))
        {
        if (SUCCESS == CreateRealityDataTest())
            {
            if (SUCCESS == CreateRelationshipToProject())
                {
                relationshipCreated = true;
                }

            if (SUCCESS == UploadTest())
                {
                DownloadTest();

                NavigationTest();

                InformationExtractionTest();

                ListTest();

                InformationManagementTest();
                }

            if (relationshipCreated)
                {
                DeleteRelationship();
                }

            // This test must be called if creation was sucessful
            DeleteRealityDataTest();
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
StatusInt RealityDataServicePerformanceTests::ConfigureServerTest(Utf8String serverName)
    {
    bool verifyCertificate = false;

    DisplayInfo("WSG Request handshake test: ", DisplayOption::Info);

    m_server = WSGServer(serverName, verifyCertificate);
    RawServerResponse response;

    Utf8String version = m_server.GetVersion(response);
    if (version.size() == 0)
        return ERROR;

    Utf8String repo = "S3MXECPlugin--Server";
    Utf8String schema = "S3MX";
    
    bvector<Utf8String> repoNames = m_server.GetRepositories(response);
    if(repoNames.size() == 0)
        {
        DisplayInfo("There was an error contacting the server. No repositories found\n", DisplayOption::Error);
        return ERROR;
        }
    
    
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
StatusInt RealityDataServicePerformanceTests::UploadTest()
    {
    RawServerResponse response;

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
    TransferReport* tReport = upload.Perform();
    Utf8String report;
    tReport->ToXml(report);
    DisplayInfo("if any files failed to upload, they will be listed here: \n", DisplayOption::Tip);
    DisplayInfo(report);
     

    return SUCCESS;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::DownloadTest()
    {
    RawServerResponse response;

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
    TransferReport* tReport = download.Perform();
    Utf8String report;
    tReport->ToXml(report);
    DisplayInfo ("If any files failed to download, they will be listed here: \n");
    DisplayInfo (Utf8PrintfString("%s\n", report));



    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// This will traverse the navigation node interface a few times and report the 
// time taken.
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::NavigationTest()
    {
    RawServerResponse response;

    // To BE DONE!
    
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
        std::cout << "Information Extraction Test: Failure no: " << response.status;
    else
        std::cout << "Information Extraction Test: " << (endTime - startTime) / 1000.0;
    
    return (StatusInt)response.status;
    }




//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::InformationManagementTest()
    {
    // TO BE DONE
    return SUCCESS;
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
#if (0)
    RealityDataService::Request(myRequest, response);
#endif
    
    // End time
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);

    // Report
    if (OK != response.status)
        std::cout << "Relationship deletion Test: Failure no: " << response.status;
    else
        std::cout << "Relationship deletion Test: " << (endTime - startTime) / 1000.0;
    
    return (StatusInt)response.status;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::DeleteRealityDataTest()
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
    if (OK != response.status)
        std::cout << "Deletion Test: Failure no: " << response.status;
    else
        std::cout << "Deletion Test: " << (endTime - startTime) / 1000.0;
    
    return (StatusInt)response.status;    
    }


//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataServicePerformanceTests::ListTest()
    {
    RawServerResponse response;

    Utf8String nodeString;
    bvector<Utf8String> nodeStrings;
#if (0)
    m_serverNodes = NodeNavigator::GetInstance().GetRootNodes(m_server, RealityDataService::GetRepoName());

    for (NavNode node : m_serverNodes)
        {
        nodeString = node.GetLabel();
        if(node.GetClassName() == "Folder")
            nodeString.append("/");
        nodeStrings.push_back(nodeString);
        }

    PrintResults(nodeStrings);
#endif
    return response.status;
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
    RealityDataService::Request(ptt, &NbRealityData, &TotalSizeKB, response);

    return (StatusInt)response.status;
    }


#if (0)
//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
void RealityDataServicePerformanceTests::Details()
    {
    if (m_currentNode == nullptr)
        {
        DisplayInfo("please navigate to an item (with cd) before using this function\n", DisplayOption::Tip);
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
        RealityDataFolderByIdRequest folderReq = RealityDataFolderByIdRequest(m_currentNode->node.GetInstanceId());
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
        RealityDataByIdRequest idReq = RealityDataByIdRequest(m_currentNode->node.GetInstanceId());
        RealityDataPtr entity = RealityDataService::Request(idReq, status);

        if (entity == nullptr)
            {
            DisplayInfo ("There was an error retrieving information for this item\n", DisplayOption::Error);
            return;
            }

        DisplayInfo (Utf8PrintfString(" RealityData name   : %s\n", entity->GetName()));
        DisplayInfo (Utf8PrintfString(" Id                 : %s\n", entity->GetIdentifier()));
        DisplayInfo (Utf8PrintfString(" Container name     : %s\n", entity->GetContainerName()));
        DisplayInfo (Utf8PrintfString(" Dataset            : %s\n", entity->GetDataset()));
        DisplayInfo (Utf8PrintfString(" Description        : %s\n", entity->GetDescription()));
        DisplayInfo (Utf8PrintfString(" Root document      : %s\n", entity->GetRootDocument()));
        DisplayInfo (Utf8PrintfString(" Size (kb)          : %lu", entity->GetIdentifier()));
        DisplayInfo (Utf8PrintfString(" Classification     : %s\n", entity->GetClassificationTag()));
        DisplayInfo (Utf8PrintfString(" Type               : %s\n", entity->GetRealityDataType()));
        DisplayInfo (Utf8PrintfString(" Accuracy (m)       : %f", entity->GetAccuracyValue()));
        DisplayInfo (Utf8PrintfString(" Modified timestamp : %s\n", entity->GetModifiedDateTime().ToString()));
        DisplayInfo (Utf8PrintfString(" Created timestamp  : %s\n", entity->GetCreationDateTime().ToString()));
        }
    }

#endif

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


