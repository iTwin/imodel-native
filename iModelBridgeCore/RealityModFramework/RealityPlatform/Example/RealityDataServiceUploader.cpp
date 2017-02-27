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

static void progressFunc(Utf8String filename, float progress)
    {
    char progressString[1024];
    sprintf(progressString, "%s upload percent : %f", filename.c_str(), progress * 100.0f);
    std::cout << progressString << std::endl;
    }

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
    properties.Insert(RealityDataField::Name, "Helsinki");
    properties.Insert(RealityDataField::Dataset, "Terrain");
    properties.Insert(RealityDataField::Group, "Exemple group");
    properties.Insert(RealityDataField::Description, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
    properties.Insert(RealityDataField::RootDocument, "Scene/Production_Helsinki_3MX_ok.3mx");
    properties.Insert(RealityDataField::Classification, "Exemple classification");
    properties.Insert(RealityDataField::Type, "3mx");
    properties.Insert(RealityDataField::Footprint, "{ \\\"points\\\" : [[24.7828757,59.9224887],[25.2544848,59.9224887],[25.2544848,60.2978389],[24.7828757,60.2978389],[24.7828757,59.9224887]], \\\"coordinate_system\\\" : \\\"4326\\\" }");
    properties.Insert(RealityDataField::ThumbnailDocument, "thumbnail.jpg");
    properties.Insert(RealityDataField::MetadataURL, "metadata.html");
    properties.Insert(RealityDataField::AccuracyInMeters, "16.14");
    properties.Insert(RealityDataField::ResolutionInMeters, "16.14x23.19");
    properties.Insert(RealityDataField::OwnedBy, "francis.boily@bentley.com");

    Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

    BeFileName Montgomery = BeFileName("D:/Helsinki");
    RealityDataServiceUpload* upload = new RealityDataServiceUpload(Montgomery, "43a4a51a-bfd3-4271-a9d9-21db56cdcf10", propertyString, true);
    if (upload->IsValidUpload())
        {
        upload->SetProgressCallBack(progressFunc);
        time_t time = std::time(nullptr);
        UploadReport* ur = upload->Perform();
        time_t time2 = std::time(nullptr);
        time2 -= time;
        Utf8String report;
        ur->ToXml(report);
        std::cout << report << std::endl;
        std::cout << time2 << std::endl;
        }

    return 0;
}