/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/Example/RealityDataServiceExample.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityDataService.h>

#include <stdio.h>
#include <conio.h>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

int main(int argc, char *argv[])
    {
    Utf8String id = "05610e4c-79d4-43ef-a9e5-e02e6328d843";
    Utf8String projectId = "1";
    Utf8String folderId = "ab9c6aa6-91ad-424b-935c-28a3c396a041~2FGraz~2F";
    Utf8String documentId = "ab9c6aa6-91ad-424b-935c-28a3c396a041~2FGraz~2FScene~2FProduction_Graz_3MX.3mx";
    Utf8String enterpriseId = "5e41126f-6875-400f-9f75-4492c99ee544";
    RealityDataService::SetServerComponents("dev-realitydataservices-eus.cloudapp.net", "v2.4", "S3MXECPlugin--Server", "S3MX");
    //RealityDataService::SetServerComponents("s3mxcloudservice.cloudapp.net", "v2.4", "S3MXECPlugin--Server", "S3MX");

    std::cout << RealityDataService::GetServerName() << std::endl;
    std::cout << RealityDataService::GetWSGProtocol() << std::endl;
    std::cout << RealityDataService::GetRepoName() << std::endl;
    std::cout << RealityDataService::GetSchemaName() << std::endl << std::endl;

    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField,Utf8String>();
    properties.Insert(RealityDataField::Name, "exampleUpload");
    properties.Insert(RealityDataField::Classification, "Terrain");
    properties.Insert(RealityDataField::Type, "3mx");
    properties.Insert(RealityDataField::Footprint, "{ \\\"points\\\" : [[-112.101512,40.700246],[-111.7394581,40.700246],[-111.7394581,40.8529699],[-112.101512,40.8529699],[-112.101512,40.700246]], \\\"coordinate_system\\\" : \\\"4326\\\" }");
    properties.Insert(RealityDataField::OwnedBy, "francis.boily@bentley.com");

    Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

    BeFileName Montgomery = BeFileName("D:/RealityModFrameworkFolder");
    RealityDataServiceUpload* upload = new RealityDataServiceUpload(Montgomery, "604f9be9-e74f-4614-a23e-b02e2dc129f5", propertyString, true);
    
    if(upload->IsValidUpload())
        {
        UploadReport* ur = upload->Perform();
        Utf8String report;
        ur->ToXml(report);
        std::cout << report << std::endl;
        }

    RealityDataService::SetServerComponents("s3mxcloudservice.cloudapp.net", "v2.4", "S3MXECPlugin--Server", "S3MX");

    RequestStatus status;

    RealityDataByIdRequest* idReq = new RealityDataByIdRequest(id);
    SpatialEntityPtr entity = RealityDataService::Request(*idReq, status);

    std::cout << "Entity provenance for Id " << id << ":" << std::endl;
    std::cout << entity->GetName() << std::endl << std::endl;


    RealityDataProjectRelationshipByProjectIdRequest* relationReq = new RealityDataProjectRelationshipByProjectIdRequest(projectId);
    bvector<RealityDataProjectRelationshipPtr> relationships = RealityDataService::Request(*relationReq, status);

    std::cout << "number of relationships found for projectId " << projectId << " :" << std::endl;
    std::cout << relationships.size() << std::endl;


    RealityDataFolderByIdRequest* folderReq = new RealityDataFolderByIdRequest(folderId);
    RealityDataFolderPtr folder = RealityDataService::Request(*folderReq, status);

    std::cout << "folder found for Id " << folderId << " :" << std::endl;
    std::cout << folder->GetName() << std::endl;


    RealityDataDocumentByIdRequest* documentReq = new RealityDataDocumentByIdRequest(documentId);
    RealityDataDocumentPtr document = RealityDataService::Request(*documentReq, status);

    std::cout << "document with Id " << documentId << " :" << std::endl;
    std::cout << document->GetName() << std::endl;

    RealityDataDocumentContentByIdRequest* contentRequest = new RealityDataDocumentContentByIdRequest(documentId);
    
    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);
    BeFileName fileName = BeFileName(exeDir);
    fileName.AppendToPath(BeFileName("testFile"));
    char outfile[1024] = "";
    strcpy(outfile, fileName.GetNameUtf8().c_str());
    FILE* file = fopen(outfile, "wb");

    RealityDataService::Request(*contentRequest, file, status);


    bvector<Utf8String> filter1 = bvector<Utf8String>();
    bvector<Utf8String> filter2 = bvector<Utf8String>();
    filter1.push_back(RealityDataFilterCreator::FilterByOwner("francis.boily@bentley.com"));
    filter1.push_back(RealityDataFilterCreator::FilterByCreationDate(DateTime(2016,12,01), DateTime(2017,01,05)));
    filter2.push_back(RealityDataFilterCreator::GroupFiltersAND(filter1));
    filter2.push_back(RealityDataFilterCreator::FilterByType("3mx"));
    filter2.push_back(RealityDataFilterCreator::FilterPublic(true));
    
    // important note: parentheses are not currently supported, which means that all filters (AND/OR) are evaluated together
    // results may differ from their intended goal
    Utf8String filters = RealityDataFilterCreator::GroupFiltersOR(filter2);

    RealityDataPagedRequest* filteredRequest = new RealityDataPagedRequest();

    filteredRequest->SetFilter(filters);
    filteredRequest->SortBy(RealityDataField::ModifiedTimestamp, true);

    bvector<SpatialEntityPtr> filteredSpatialEntities = RealityDataService::Request(*filteredRequest, status);

    std::cout << "Number of spatial entities found for filter : " << std::endl;
    std::cout << filteredSpatialEntities.size() << std::endl;


    RealityDataListByEnterprisePagedRequest* enterpriseReq = new RealityDataListByEnterprisePagedRequest(enterpriseId);
    bvector<SpatialEntityPtr> enterpriseVec = RealityDataService::Request(*enterpriseReq, status);

    std::cout << "Number of spatial entities found for enterprise" << enterpriseId << " :" << std::endl;
    std::cout << enterpriseVec.size() << std::endl;


    RealityDataProjectRelationshipByProjectIdPagedRequest* relationByIdReq = new RealityDataProjectRelationshipByProjectIdPagedRequest(projectId);
    bvector<RealityDataProjectRelationshipPtr> relationVec = RealityDataService::Request(*relationByIdReq, status);

    std::cout << "Number of relationships found for project " << projectId << " :" << std::endl;
    std::cout << relationVec.size() << std::endl;




    getch();

    return 0;
    }