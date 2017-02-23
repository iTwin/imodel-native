/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/Example/RealityDataServiceUploader.cpp $
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
    /*Utf8String server, version, repo, shema;
    std::cout << "enter server name" << std::endl;
    std::cin >> server;
    std::cout << "enter protocol version" << std::endl;
    std::cin >> version;
    std::cout << "enter repo name" << std::endl;
    std::cin >> repo;
    std::cout << "enter schema name" << std::endl;
    std::cin >> schema;
    
    RealityDataService::SetServerComponents(server, version, repo, schema);*/
    RealityDataService::SetServerComponents("dev-realitydataservices-eus.cloudapp.net", "v2.4", "S3MXECPlugin--Server", "S3MX");


    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    properties.Insert(RealityDataField::Name, "Graz");
    properties.Insert(RealityDataField::Dataset, "Terrain");
    properties.Insert(RealityDataField::Group, "Exemple group");
    properties.Insert(RealityDataField::Description, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
    properties.Insert(RealityDataField::RootDocument, "Graz/Scene/Production_Graz_3MX.3mx");
    properties.Insert(RealityDataField::Classification, "Exemple classification");
    properties.Insert(RealityDataField::Type, "3mx");
    properties.Insert(RealityDataField::Footprint, "{ \\\"points\\\" : [[-112.101512,40.700246],[-111.7394581,40.700246],[-111.7394581,40.8529699],[-112.101512,40.8529699],[-112.101512,40.700246]], \\\"coordinate_system\\\" : \\\"4326\\\" }");
    properties.Insert(RealityDataField::ThumbnailDocument, "Exemple thumbnaildocument");
    properties.Insert(RealityDataField::MetadataURL, "Exemple metadataurl");
    properties.Insert(RealityDataField::AccuracyInMeters, "16.14");
    properties.Insert(RealityDataField::ResolutionInMeters, "16.14x23.19");
    properties.Insert(RealityDataField::OwnedBy, "francis.boily@bentley.com");

    Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

    BeFileName Montgomery = BeFileName("D:/RealityModFrameworkFolder/Graz");
    RealityDataServiceUpload* upload = new RealityDataServiceUpload(Montgomery, "db892d23-9fdd-449b-bb0c-962b6ee15711", propertyString, true);
    if (upload->IsValidUpload())
        upload->Perform();

    return 0;
}