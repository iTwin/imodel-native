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


#if (0) 
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    properties.Insert(RealityDataField::Name, "Performance Test 1");
    properties.Insert(RealityDataField::Dataset, "Terrain");
    properties.Insert(RealityDataField::Group, "Performance group");
    properties.Insert(RealityDataField::Description, "Test_1");
    properties.Insert(RealityDataField::RootDocument, "Test_1/SaltLake/3MX/Scene/Production_utsalt_3mx.3mx");
    properties.Insert(RealityDataField::Classification, "Exemple Test 1");
    properties.Insert(RealityDataField::Type, "3mx");
    properties.Insert(RealityDataField::Footprint, "{ \\\"points\\\" : [[24.7828757,59.9224887],[25.2544848,59.9224887],[25.2544848,60.2978389],[24.7828757,60.2978389],[24.7828757,59.9224887]], \\\"coordinate_system\\\" : \\\"4326\\\" }");
    properties.Insert(RealityDataField::ThumbnailDocument, "thumbnail.jpg");
    properties.Insert(RealityDataField::MetadataURL, "metadata.html");
    properties.Insert(RealityDataField::AccuracyInMeters, "1.0");
    properties.Insert(RealityDataField::ResolutionInMeters, "1.0x1.0");
    properties.Insert(RealityDataField::OwnedBy, "Donald.Morissette@bentley.com");
    BeFileName fName = BeFileName("J:/_Data_Tests/_RDS_Performance/Test_1");
#endif
#if (1) 
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    properties.Insert(RealityDataField::Name, "Performance Test 2");
    properties.Insert(RealityDataField::Dataset, "Images");
    properties.Insert(RealityDataField::Group, "Performance group");
    properties.Insert(RealityDataField::Description, "Test_2");
    properties.Insert(RealityDataField::RootDocument, "");
    properties.Insert(RealityDataField::Classification, "Exemple Test 2");
    properties.Insert(RealityDataField::Type, "Images");
//    properties.Insert(RealityDataField::Footprint, "{ \\\"points\\\" : [[24.7828757,59.9224887],[25.2544848,59.9224887],[25.2544848,60.2978389],[24.7828757,60.2978389],[24.7828757,59.9224887]], \\\"coordinate_system\\\" : \\\"4326\\\" }");
//    properties.Insert(RealityDataField::ThumbnailDocument, "thumbnail.jpg");
//    properties.Insert(RealityDataField::MetadataURL, "metadata.html");
//    properties.Insert(RealityDataField::AccuracyInMeters, "1.0");
//    properties.Insert(RealityDataField::ResolutionInMeters, "1.0x1.0");
    properties.Insert(RealityDataField::OwnedBy, "Donald.Morissette@bentley.com");
    BeFileName fName = BeFileName("J:/_Data_Tests/_RDS_Performance/Test_2");
#endif

    Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

    RealityDataServiceUpload* upload = new RealityDataServiceUpload(fName, "43a4a51a-bfd3-4271-a9d9-21db56cdcf10", propertyString, true);
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