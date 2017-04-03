/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/Example/GeoCoordinationServiceExample.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/GeoCoordinationService.h>

#include <stdio.h>
#include <conio.h>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

/*-----------------------------------------------------------------//
* Bentley GeoCoordinationServiceExample
* This application uses hard coded values and is not guaranteed to 
* function properly.
* The purpose of its existence is only to show how to structure
* and execute RealityDataService operations
//----------------------------------------------------------------*/
int main(int argc, char *argv[])
    {
    GeoCoordinationService::SetServerComponents("dev-contextservices-eus.cloudapp.net", "2.4", "IndexECPlugin--Server", "RealityModeling");

    std::cout << GeoCoordinationService::GetServerName() << std::endl;
    std::cout << GeoCoordinationService::GetWSGProtocol() << std::endl;
    std::cout << GeoCoordinationService::GetRepoName() << std::endl;
    std::cout << GeoCoordinationService::GetSchemaName() << std::endl << std::endl;

    bvector<GeoPoint2d> space = bvector<GeoPoint2d>();
    space.push_back(GeoPoint2d::From(115.73, 49.44));
    space.push_back(GeoPoint2d::From(115.73, 50.18));
    space.push_back(GeoPoint2d::From(116.53, 50.18));
    space.push_back(GeoPoint2d::From(116.53, 49.44));

    SpatialEntityWithDetailsSpatialRequest spatialReq = SpatialEntityWithDetailsSpatialRequest(space, Classification::Imagery);
    RawServerResponse spatialResponse = RawServerResponse();
    bvector<SpatialEntityPtr> spatialEntities = GeoCoordinationService::Request(spatialReq, spatialResponse);

    if (spatialResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "Entities found in provided space :" << std::endl;
        std::cout << spatialEntities.size() << std::endl << std::endl;
        }
    else
        std::cout << "error performing spatial request" << std::endl;

    SpatialEntityWithDetailsByIdRequest idReq = SpatialEntityWithDetailsByIdRequest("1");
    RawServerResponse idResponse = RawServerResponse();
    SpatialEntityPtr idEntity = GeoCoordinationService::Request(idReq, idResponse);

    if (idResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "Entity found for id '1' with provider :" << std::endl;
        std::cout << idEntity->GetProvider() << std::endl << std::endl;
        }
    else
        std::cout << "error SpatialEntityWithDetailsByIdRequest" << std::endl;


    SpatialEntityByIdRequest idBasicReq = SpatialEntityByIdRequest("1");
    RawServerResponse idBasicResponse = RawServerResponse();
    SpatialEntityPtr idBasicEntity = GeoCoordinationService::Request(idBasicReq, idBasicResponse);

    if (idBasicResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "Entity found for id '1' with provider :" << std::endl;
        std::cout << idBasicEntity->GetProvider() << std::endl << std::endl;
        }
    else
        std::cout << "error SpatialEntityByIdRequest" << std::endl;

    SpatialEntityDataSourceByIdRequest dsReq = SpatialEntityDataSourceByIdRequest("1");
    RawServerResponse dsResponse = RawServerResponse();
    SpatialEntityDataSourcePtr dSource = GeoCoordinationService::Request(dsReq, dsResponse);

    if (dsResponse.status != RequestStatus::BADREQ)
    {
        std::cout << "DataSource found for id '1' with Url :" << std::endl;
        std::cout << dSource->GetUrl() << std::endl << std::endl;
    }
    else
        std::cout << "error SpatialEntityDataSourceByIdRequest" << std::endl;

    SpatialEntityServerByIdRequest serverReq = SpatialEntityServerByIdRequest("1");
    RawServerResponse serverResponse = RawServerResponse();
    SpatialEntityServerPtr serverEntity = GeoCoordinationService::Request(serverReq, serverResponse);

    if (serverResponse.status != RequestStatus::BADREQ)
    {
        std::cout << "Server found for id '1' with name :" << std::endl;
        std::cout << serverEntity->GetName() << std::endl << std::endl;
    }
    else
        std::cout << "error SpatialEntityServerByIdRequest" << std::endl;

    SpatialEntityMetadataByIdRequest metaReq = SpatialEntityMetadataByIdRequest("1");
    RawServerResponse metaResponse = RawServerResponse();
    SpatialEntityMetadataPtr metaEntity = GeoCoordinationService::Request(metaReq, metaResponse);

    if (metaResponse.status != RequestStatus::BADREQ)
    {
        std::cout << "Metadata found for id '1' with provenance :" << std::endl;
        std::cout << metaEntity->GetProvenance() << std::endl << std::endl;
    }
    else
        std::cout << "error SpatialEntityMetadataByIdRequest" << std::endl;


    /*PackagePreparationRequest prepReq = PackagePreparationRequest(space, );
    RawServerResponse prepResponse = RawServerResponse();
    Utf8String prep = GeoCoordinationService::Request(prepReq, prepResponse);

    if (prepResponse.status != RequestStatus::BADREQ)
    {
        std::cout << "PackageId  :" << std::endl;
        std::cout << metaEntity->GetProvenance() << std::endl << std::endl;
    }
    else
        std::cout << "error SpatialEntityMetadataByIdRequest" << std::endl;*/

    }