/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformTools/Example/RealityDataServiceDownloader.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatformTools/RealityDataService.h>

#include <stdio.h>
#include <conio.h>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

static void progressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    std::cout << Utf8PrintfString("percentage of files downloaded : %3.0f%%\r", repoProgress * 100.0);
    }

void PrintUsage()
    {
    std::cout << "RealityDataServiceDownloader for RDS V1.0" << std::endl << std::endl;
    std::cout << "RealityDataServiceDownloader [server] [schema] [repository] [downloadSource] [downloadDestination] [projectId]" << std::endl;
    std::cout << "   [server]                   : ex: dev-realitydataservices-eus.cloudapp.net" << std::endl;
    std::cout << "   [schema]                   : ex: S3MXECPlugin--Server" << std::endl;
    std::cout << "   [repository]               : ex: S3MX" << std::endl;
    std::cout << "   [downloadSource]           : ex: 5ffc6e51-edc3-4fb3-8b4f-a4becbc045dd" << std::endl;
    std::cout << "   [downloadDestination]      : ex: D:/RealityModDownload (folder must exist)" << std::endl;
    std::cout << "   [projectId]                : ex: 72524420-7d48-4f4e-8b0f-144e5fa0aa22" << std::endl;
    }

int main(int argc, char *argv[])
    {
    if(argc < 7)
        {
        PrintUsage();
        getch();
        return -1;
        }

    RawServerResponse rawResponse = RawServerResponse();

    WSGServer wsgServer = WSGServer(argv[1], false);
    Utf8String version = wsgServer.GetVersion(rawResponse);
    RealityDataService::SetServerComponents(argv[1], version, argv[2], argv[3]);
    RealityDataService::SetProjectId(argv[6]);

    Utf8String sourceOnServer = Utf8String(argv[4]);

    BeFileName fileName = BeFileName(argv[5]);
    std::cout << "OutputPath : " << fileName.GetNameUtf8() << std::endl;
    if (fileName.DoesPathExist())
        {
        std::cout << "The specified path exist, Ok to overwrite it ? (y/n) ";
        std::string str;
        std::getline(std::cin, str);
        Utf8String input(str.c_str());
        if (!input.EqualsI("y"))
            {
            std::cout << "Press a key to continue..." << std::endl;
            getch();
            return -1;
            }
        }
    else
        {
        if (BeFileName::CreateNewDirectory(fileName.GetName()) != BeFileNameStatus::Success)
            {
            std::cout << "Not able to create the path file. Please verify that the folder exists and try again" << std::endl;

            std::cout << "Press a key to continue..." << std::endl;
            getch();
            return -1;
            }
        }

    RealityDataServiceDownload download = RealityDataServiceDownload(fileName, sourceOnServer);
    if(download.IsValidTransfer())
        {
        download.SetProgressCallBack(progressFunc);
        download.SetProgressStep(0.5);
        download.OnlyReportErrors(true);
        const TransferReport& tReport = download.Perform();
        Utf8String report;
        tReport.ToXml(report);
        std::cout << std::endl << "if any files failed to download, they will be listed here: " << std::endl;
        std::cout << report << std::endl;
        }
    
    std::cout << "Press a key to continue..." << std::endl;
    getch();
    return 0;
    }