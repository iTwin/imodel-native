/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Windows.h>
#include <Bentley/Bentley.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatformTools/RealityDataService.h>

#include <stdio.h>
#include <conio.h>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

/*-----------------------------------------------------------------//
* Bentley RealityDataServiceExample
* This application uses hard coded values and is not guaranteed to 
* function properly.
* The purpose of its existence is only to show how to structure
* and execute RealityDataService operations
//----------------------------------------------------------------*/
int main(int argc, char *argv[])
    {
    // 
    Utf8String id = "43a4a51a-bfd3-4271-a9d9-21db56cdcf10";
    Utf8String projectId = "1";
    Utf8String folderId = "43a4a51a-bfd3-4271-a9d9-21db56cdcf10~2FJ~3A~2F";
    Utf8String documentId = "43a4a51a-bfd3-4271-a9d9-21db56cdcf10~2FJ~3A~2F_Data_Tests~2F_RDS_Performance~2FTest_2~2FMosaic~2F916_16.itiff";
    Utf8String organizationId = "5e41126f-6875-400f-9f75-4492c99ee544";
    RealityDataService::SetServerComponents("dev-realitydataservices-eus.cloudapp.net", "2.4", "S3MXECPlugin--Server", "S3MX");
    RealityDataService::SetProjectId("72524420-7d48-4f4e-8b0f-144e5fa0aa22");

    std::cout << RealityDataService::GetServerName() << std::endl;
    std::cout << RealityDataService::GetWSGProtocol() << std::endl;
    std::cout << RealityDataService::GetRepoName() << std::endl;
    std::cout << RealityDataService::GetSchemaName() << std::endl << std::endl;

    //--------------------------DOWNLOAD--------------------------//
    /*BeFileName fName = BeFileName("D:\\RealityModFrameworkFolder");
    RealityDataServiceDownload download = RealityDataServiceDownload(fName, "604f9be9-e74f-4614-a23e-b02e2dc129f5/duplicates/Newf");
    download.Perform();*/

    //---------------------------UPLOAD---------------------------//
    // System specific File Path used. If you wish to test uploading, change the path passed to RealityDataServiceUpload and rebuild
    /*bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField,Utf8String>();
    properties.Insert(RealityDataField::Name, "exampleUpload");
    properties.Insert(RealityDataField::Classification, "Terrain");
    properties.Insert(RealityDataField::Type, "3mx");
    properties.Insert(RealityDataField::Streamed, "false");
    properties.Insert(RealityDataField::Footprint, "{ \\\"points\\\" : [[-112.101512,40.700246],[-111.7394581,40.700246],[-111.7394581,40.8529699],[-112.101512,40.8529699],[-112.101512,40.700246]], \\\"coordinate_system\\\" : \\\"4326\\\" }");
    properties.Insert(RealityDataField::OwnedBy, "francis.boily@bentley.com");

    Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

    BeFileName Montgomery = BeFileName("D:/RealityModFrameworkFolder");
    RealityDataServiceUpload* upload = new RealityDataServiceUpload(Montgomery, "604f9be9-e74f-4614-a23e-b02e2dc129f5", propertyString, true);
    
    if(upload->IsValidUpload())
        {
        TransferReport* ur = upload->Perform();
        Utf8String report;
        ur->ToXml(report);
        std::cout << report << std::endl;
        }*/

    RawServerResponse idResponse = RawServerResponse();

    RealityDataByIdRequest idReq(id);
    RealityDataPtr entity = RealityDataService::Request(idReq, idResponse);

    if (idResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "Entity provenance for Id " << id << ":" << std::endl;
        std::cout << entity->GetName() << std::endl << std::endl;
        }
    else
        std::cout << "error retrieving provenance for id" << std::endl;

    RealityDataRelationshipByProjectIdRequest relationReq(projectId);
    RawServerResponse relationResponse = RawServerResponse();
    bvector<RealityDataRelationshipPtr> relationships = RealityDataService::Request(relationReq, relationResponse);

    if (relationResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "number of relationships found for projectId " << projectId << " :" << std::endl;
        std::cout << relationships.size() << std::endl;
        }
    else
        std::cout << "error retrieving relationships for id" << std::endl;

    RealityDataFolderByIdRequest folderReq(folderId);
    RawServerResponse folderResponse = RawServerResponse();
    RealityDataFolderPtr folder = RealityDataService::Request(folderReq, folderResponse);

    if (folderResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "folder found for Id " << folderId << " :" << std::endl;
        std::cout << folder->GetName() << std::endl;
        }
    else
        std::cout << "error retrieving folder for id" << std::endl;

    RealityDataDocumentByIdRequest documentReq(documentId);
    RawServerResponse documentResponse = RawServerResponse();
    RealityDataDocumentPtr document = RealityDataService::Request(documentReq, documentResponse);

    if (documentResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "document with Id " << documentId << " :" << std::endl;
        std::cout << document->GetName() << std::endl;
        }
    else
        std::cout << "error retrieving document for id" << std::endl;

    RealityDataDocumentContentByIdRequest contentRequest(documentId);
    
    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);
    BeFileName fileName = BeFileName(exeDir.c_str());
    fileName.AppendToPath(BeFileName("testFile"));
    BeFile file;
    file.Create(fileName.GetName(), true);

    RawServerResponse contentResponse = RawServerResponse();
    RealityDataService::Request(contentRequest, &file, contentResponse);
    file.Close();
    
    bvector<RDSFilter> filter1 = bvector<RDSFilter>();
    bvector<RDSFilter> filter2 = bvector<RDSFilter>();
    filter1.push_back(RealityDataFilterCreator::FilterByOwner("francis.boily@bentley.com"));
    filter1.push_back(RealityDataFilterCreator::FilterByCreationDate(DateTime(2016,12,01), DateTime(2017,01,05))); 
    filter1.push_back(RealityDataFilterCreator::FilterByClassification(RealityDataBase::Classification::MODEL));
    filter2.push_back(RealityDataFilterCreator::GroupFiltersAND(filter1));
    filter2.push_back(RealityDataFilterCreator::FilterByType("3mx"));
    filter2.push_back(RealityDataFilterCreator::FilterVisibility(RealityDataBase::Visibility::ENTERPRISE));
    
    // important note: parentheses are not currently supported, which means that all filters (AND/OR) are evaluated together
    // results may differ from their intended goal
    RDSFilter filters = RealityDataFilterCreator::GroupFiltersOR(filter2);

    RealityDataPagedRequest filteredRequest;

    filteredRequest.SetFilter(filters);
    filteredRequest.SortBy(RealityDataField::ModifiedTimestamp, true);

    RawServerResponse filteredResponse;
    bvector<RealityDataPtr> filteredSpatialEntities = RealityDataService::Request(filteredRequest, filteredResponse);

    if (filteredResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "Number of spatial entities found for filter : " << std::endl;
        std::cout << filteredSpatialEntities.size() << std::endl;
        }
    else
        std::cout << "error retrieving spatial entities with filter" << std::endl;

    RealityDataListByUltimateIdPagedRequest organizationReq(organizationId);
    RawServerResponse organizationResponse = RawServerResponse();
    bvector<RealityDataPtr> organizationVec = RealityDataService::Request(organizationReq, organizationResponse);

    if (organizationResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "Number of spatial entities found for organization" << organizationId << " :" << std::endl;
        std::cout << organizationVec.size() << std::endl;
        }
    else
        std::cout << "error retrieving spatial entities with organization id" << std::endl;


    RealityDataRelationshipByProjectIdPagedRequest relationByIdReq(projectId);
    RawServerResponse relationResponse2 = RawServerResponse();
    bvector<RealityDataRelationshipPtr> relationVec = RealityDataService::Request(relationByIdReq, relationResponse2);

    if (relationResponse2.status != RequestStatus::BADREQ)
        {
        std::cout << "Number of relationships found for project " << projectId << " :" << std::endl;
        std::cout << relationVec.size() << std::endl;
        }
    else
        std::cout << "error retrieving relationships for project id" << std::endl;

    std::cout << "Execution finished. Press any key to exit" << std::endl;
    getch();

    return 0;
    }