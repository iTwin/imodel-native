/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/Example/ConversionToolsTester.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/BeFileName.h>
#include <Bentley/RefCounted.h>

#include <RealityPlatformTools/RealityDataDownload.h>
#include <RealityPlatform/RealityDataPackage.h>
#include <RealityPlatformTools/RealityConversionTools.h>

#include <stdio.h>
#include <conio.h>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM
USING_NAMESPACE_BENTLEY_REALITYPLATFORM

WString sOutputFolder = L"d:\\tmp\\data\\";     // Could be override by parameter at the execution.

static int callback_progress_func(int index, void *pClient, size_t ByteCurrent, size_t ByteTotal)
{
    int ret = 0;

    RealityDataDownload::FileTransfer* pEntry = (RealityDataDownload::FileTransfer*)pClient;

    printf("* ProgressInfo: (%d) %ls -- %llu of %llu\n", index, pEntry->filename.c_str(), ByteCurrent, ByteTotal);

    return ret;   // # 0 --> will abort the transfer.
}

static void callback_status_func(int index, void *pClient, int ErrorCode, const char* pMsg)
{
    RealityDataDownload::FileTransfer* pEntry = (RealityDataDownload::FileTransfer*)pClient;
    printf("****** Status: (%d) ErrCode: %d - fromCache(%d) - (%s) <%ls>\n", index, ErrorCode, pEntry->fromCache, pMsg, pEntry->filename.c_str());

    if (ErrorCode == 0)
    {
        WString out;

        // Extract path only to unzip there
        WString urlW(pEntry->filename.c_str());
        urlW.ReplaceAll(WCSALT_DIR_SEPARATOR, WCSDIR_SEPARATOR);
        WString delim = WCSDIR_SEPARATOR;
        bvector<WString> pathComponents;
        bvector<WString> filenameComponents;
        BeStringUtilities::Split(urlW.c_str(), delim.c_str(), NULL, pathComponents);
        for (size_t i = 0; i < pathComponents.size() - 1; ++i)
        {
            out += pathComponents[i];
            out += WCSDIR_SEPARATOR;
        }

        if (RealityDataDownload::UnZipFile(pEntry->filename, out))
            printf("******     Unzip status Success\n");
        else
            printf("******     Unzip status Failed\n");
    }
    else
    {
        // An error occured ... we will try one of the alternate source
    }
}

void ShowUsage()
{
    std::cout << "Usage: ConversionToolsTester.exe -f:[file] -cs:[connectionString] [options]" << std::endl << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -f, --file              The AWS file to parse (Required)" << std::endl;
    std::cout << "  -h, --help              Show this help message and exit" << std::endl;
    
    std::cout << std::endl << "Press any key to exit." << std::endl;
    getch();
}


int main(int argc, char *argv[])
{
    RealityDataDownload::Link_File_wMirrors_wSisters urlList;

    bool hasFile = false;
    std::string fileName;

    for (int i = 0; i < argc; ++i)
        {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            {
            ShowUsage();
            return 0;
            }
        else if (strstr(argv[i], "-f:") || strstr(argv[i], "--file:"))
            {
            char* substringPosition = strstr(argv[i], ":");
            substringPosition++;
            fileName = std::string(substringPosition);
            hasFile = true;
            }
        }

    if(!hasFile)
        {
        ShowUsage();
        return 0;
        }

    BeFileName filename = BeFileName(fileName.c_str());
    urlList = RealityConversionTools::PackageFileToDownloadOrder(filename);


    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(urlList);
    if (pDownload != NULL)
    {
        pDownload->SetProgressCallBack(callback_progress_func, 0.1f);
        pDownload->SetStatusCallBack(callback_status_func);
        pDownload->Perform();
    }
    else
        pDownload->SetProgressCallBack(callback_progress_func);

    printf("-----Done-----");
    _getch();
    return 0;
}
