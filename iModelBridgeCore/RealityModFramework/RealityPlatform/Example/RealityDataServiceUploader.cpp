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

static void progressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    std::cout << Utf8PrintfString("Upload percent : %3.0f%%\r", repoProgress * 100.0);
    }

/*static void statusFunc(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    if(ErrorCode > 0)
        std::cout << Utf8PrintfString("Curl error code : %d /n %s", ErrorCode, pMsg);
    else
        std::cout << pMsg;
    }*/

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
    RealityDataService::SetServerComponents("dev-realitydataservices-eus.cloudapp.net", "2.4", "S3MXECPlugin--Server", "S3MX");


#if (1) 
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    properties.Insert(RealityDataField::Name, "Barmettes");
    properties.Insert(RealityDataField::Dataset, "Images");
    properties.Insert(RealityDataField::Group, "SPAR Demo");
    properties.Insert(RealityDataField::Description, "SPAR Demo Viewer Cesium");
    properties.Insert(RealityDataField::RootDocument, "Barmettes.json");
    properties.Insert(RealityDataField::Classification, "Exemple Test 2");
    properties.Insert(RealityDataField::Type, "3DTiles");
//    properties.Insert(RealityDataField::Footprint, "{ \\\"points\\\" : [[24.7828757,59.9224887],[25.2544848,59.9224887],[25.2544848,60.2978389],[24.7828757,60.2978389],[24.7828757,59.9224887]], \\\"coordinate_system\\\" : \\\"4326\\\" }");
//    properties.Insert(RealityDataField::ThumbnailDocument, "thumbnail.jpg");
//    properties.Insert(RealityDataField::MetadataURL, "metadata.html");
//    properties.Insert(RealityDataField::AccuracyInMeters, "1.0");
//    properties.Insert(RealityDataField::ResolutionInMeters, "1.0x1.0");
    properties.Insert(RealityDataField::OwnedBy, "Donald.Morissette@bentley.com");
    BeFileName fName = BeFileName("J:/_Data_Tests/_SPAR_Demo/Barmettes");
#endif

    Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

    RealityDataServiceUpload* upload = new RealityDataServiceUpload(fName, "15d33ed9-24d9-4b95-8e05-90de075b939b", propertyString, true);
    if (upload->IsValidUpload())
        {
        upload->SetProgressCallBack(progressFunc);
        upload->SetProgressStep(0.05);
        upload->OnlyReportErrors(true);
        time_t time = std::time(nullptr);
        TransferReport* ur = upload->Perform();
        time_t time2 = std::time(nullptr);
        time2 -= time;
        Utf8String report;
        ur->ToXml(report);
        std::cout << report << std::endl;
        std::cout << time2 << std::endl;
        }

    return 0;
}