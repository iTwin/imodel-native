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
    properties.Insert(RealityDataField::Name, "exampleUpload");
    properties.Insert(RealityDataField::Classification, "Terrain");
    properties.Insert(RealityDataField::Type, "3mx");
    properties.Insert(RealityDataField::Footprint, "{ \\\"points\\\" : [[-112.101512,40.700246],[-111.7394581,40.700246],[-111.7394581,40.8529699],[-112.101512,40.8529699],[-112.101512,40.700246]], \\\"coordinate_system\\\" : \\\"4326\\\" }");
    properties.Insert(RealityDataField::OwnedBy, "francis.boily@bentley.com");

    Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

    BeFileName Montgomery = BeFileName("D:/RealityModFrameworkFolder");
    RealityDataServiceUpload* upload = new RealityDataServiceUpload(Montgomery, "604f9be9-e74f-4614-a23e-b02e2dc129f5", propertyString, true);
    if (upload->IsValidUpload())
        upload->Perform();

    return 0;
}