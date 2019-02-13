/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformTools/Example/RealityDataServiceUploader.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/Desktop/FileSystem.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatformTools/RealityDataService.h>
#include <BeSQLite/BeSQLite.h>

#include <stdio.h>
#include <conio.h>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

static void progressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    std::cout << Utf8PrintfString("Upload percent : %3.0f%%\r", repoProgress * 100.0);
    }

static void statusFunc(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    if(ErrorCode > 0)
        std::cout << Utf8PrintfString("Curl error code : %d \n %s", ErrorCode, pMsg);
    else if(ErrorCode < 0)
        std::cout << pMsg;
    }

int main(int argc, char *argv[])
{
    RealityDataService::SetServerComponents("dev-realitydataservices-eus.cloudapp.net", "2.4", "S3MXECPlugin--Server", "S3MX");
    RealityDataService::SetProjectId("72524420-7d48-4f4e-8b0f-144e5fa0aa22");

#if (0) 
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    properties.Insert(RealityDataField::Name, "Donald Test2");
    properties.Insert(RealityDataField::Dataset, "Model");
    properties.Insert(RealityDataField::Group, "SPAR Demo");
    properties.Insert(RealityDataField::Description, "SPAR Demo Scenario 0");
    properties.Insert(RealityDataField::RootDocument, "Production_8.json");
    properties.Insert(RealityDataField::Classification, "Model");
    properties.Insert(RealityDataField::Type, "3DTiles");
//    properties.Insert(RealityDataField::Streamed, "true");
//    properties.Insert(RealityDataField::Footprint, "{ \\\"points\\\" : [[24.7828757,59.9224887],[25.2544848,59.9224887],[25.2544848,60.2978389],[24.7828757,60.2978389],[24.7828757,59.9224887]], \\\"coordinate_system\\\" : \\\"4326\\\" }");
//    properties.Insert(RealityDataField::ThumbnailDocument, "thumbnail.jpg");
//    properties.Insert(RealityDataField::MetadataURL, "metadata.html");
//    properties.Insert(RealityDataField::Copyright, "data owned by a human");
//    properties.Insert(RealityDataField::TermsOfUse, "Do not use unless born in March or July");
//    properties.Insert(RealityDataField::AccuracyInMeters, "1.0");
//    properties.Insert(RealityDataField::ResolutionInMeters, "1.0x1.0");
    properties.Insert(RealityDataField::OwnedBy, "Jean-Philippe.Pons@bentley.com");

    properties.Insert(RealityDataField::Visibility, "ENTERPRISE");

    BeFileName fName = BeFileName("D:/RealityModFrameworkFolder/prod");
#endif
#if (1) 
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    properties.Insert(RealityDataField::Name, "Speed Test");
    properties.Insert(RealityDataField::Dataset, "ContextCapture");
    properties.Insert(RealityDataField::Group, "SPAR Demo");
    properties.Insert(RealityDataField::Description, "SPAR Demo Scenario 0");
    properties.Insert(RealityDataField::RootDocument, "prod.3sm");
    properties.Insert(RealityDataField::Classification, "Model");
    properties.Insert(RealityDataField::Type, "3SM");
    properties.Insert(RealityDataField::OwnedBy, "Cyril.Novel@bentley.com");

    properties.Insert(RealityDataField::Visibility, "ENTERPRISE");
    BeFileName fName = BeFileName("C:/Helsinki");
#endif


    Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

    BeFileName TempPath;
    Desktop::FileSystem::BeGetTempPath(TempPath);

    RealityDataServiceUpload* upload = new RealityDataServiceUpload(fName, ""/*Id.ToLower()*/, propertyString, false, true, statusFunc);
    if (upload->IsValidTransfer())
        {
        std::cout << Utf8PrintfString("Upload file : %s \n", fName.GetNameUtf8().c_str());

        upload->SetProgressCallBack(progressFunc);
        upload->SetProgressStep(0.5);
        upload->OnlyReportErrors(true);
        time_t time = std::time(nullptr);
        const TransferReport& ur = upload->Perform();
        time_t time2 = std::time(nullptr);
        time2 -= time;
        Utf8String report;
        ur.ToXml(report);
        std::cout << report << std::endl;
        std::cout << time2 << " Sec" << std::endl;
        }

    std::cout << "Press a key to continue...";
    getch();
    return 0;
}