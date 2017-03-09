/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/Example/RealityDataServiceDownloader.cpp $
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
    char progressString[1024];
    sprintf(progressString, "percentage of files downloaded : %f", repoProgress * 100.0);
    std::cout << progressString << std::endl;
    }

void PrintUsage()
    {
    std::cout << "RealityDataServiceDownloader for RDS V1.0" << std::endl << std::endl;
    std::cout << "RealityDataServiceDownloader [server] [schema] [repository] [downloadSource] [downloadDestination]" << std::endl;
    std::cout << "   [server]                   : ex: dev-realitydataservices-eus.cloudapp.net" << std::endl;
    std::cout << "   [schema]                   : ex: S3MXECPlugin--Server" << std::endl;
    std::cout << "   [repository]               : ex: S3MX" << std::endl;
    std::cout << "   [downloadSource]           : ex: 5ffc6e51-edc3-4fb3-8b4f-a4becbc045dd" << std::endl;
    std::cout << "   [downloadDestination]      : ex: D:/RealityModDownload (folder must exist)" << std::endl;
    }

int main(int argc, char *argv[])
    {
    if(argc < 6)
        {
        PrintUsage();
        getch();
        return -1;
        }

    WSGServer wsgServer = WSGServer(argv[1], false);
    Utf8String version = wsgServer.GetVersion();
    RealityDataService::SetServerComponents(argv[1], version, argv[2], argv[3]);

    Utf8String sourceOnServer = Utf8String(argv[4]);

    BeFileName fileName = BeFileName(argv[5]);
    if (!fileName.DoesPathExist())
        {
        std::cout << "could not validate specified path. Please verify that the folder exists and try again" << std::endl;
        return -1;
        }

    RealityDataServiceDownload download = RealityDataServiceDownload(fileName, sourceOnServer);
    download.SetProgressCallBack(progressFunc);
    download.SetProgressStep(0.05);
    download.OnlyReportErrors(true);
    TransferReport* tReport = download.Perform();
    Utf8String report;
    tReport->ToXml(report);
    std::cout << "if any files failed to download, they will be listed here: " << std::endl;
    std::cout << report << std::endl;
    
    return 0;
    }