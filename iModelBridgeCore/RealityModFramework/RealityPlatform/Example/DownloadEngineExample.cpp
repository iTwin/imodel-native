/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/Example/DownloadEngineExample.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/BeFileName.h>
#include <Bentley/RefCounted.h>

#include <RealityPlatform/RealityDataDownload.h>
#include <RealityPackage/RealityDataPackage.h>
#include <RealityPlatform/md5.h>

#include <stdio.h>
#include <conio.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM
USING_NAMESPACE_BENTLEY_REALITYPACKAGE

WString sOutputFolder = L"k:\\tmp\\data\\";     // Could be override by parameter at the execution.

static int callback_progress_func (int index,void *pClient, size_t ByteCurrent,size_t ByteTotal)
    {
    int ret = 0;

    RealityDataDownload::FileTransfer* pEntry = (RealityDataDownload::FileTransfer*)pClient;

    printf("* ProgressInfo: (%d) %ls -- %llu of %llu\n", index, pEntry->filename.c_str(), ByteCurrent, ByteTotal);

    return ret;   // # 0 --> will abort the transfer.
    }

static void callback_status_func (int index, void *pClient, int ErrorCode, const char* pMsg)
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
        for (size_t i = 0; i < pathComponents.size()-1; ++i)
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


///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Francis Boily                   09/2015
//+---------------+---------------+---------------+---------------+---------------+------*/
static WString createDirWithHash(Utf8StringCR uri, WStringCR tempPath, uint64_t filesize)
{
    // Extract filename form URL, the last part of the URL until a '/'or '\' or '='
    WString urlW(uri.c_str(), BentleyCharEncoding::Utf8);
    urlW.ReplaceAll(WCSALT_DIR_SEPARATOR, WCSDIR_SEPARATOR);
    WString delim = WCSDIR_SEPARATOR;
    delim += L"=";
    bvector<WString> pathComponents;
    bvector<WString> filenameComponents;
    BeStringUtilities::Split(urlW.c_str(), delim.c_str(), NULL, pathComponents);
    BeStringUtilities::Split(pathComponents[pathComponents.size() - 1].c_str(), L".", filenameComponents);

    // Creating one directory per file because we'll need to unzip it into the directory
    BeFileName separatedDirectoryForZip(tempPath);
    WString filenameTemp = filenameComponents[0];

    // Creating the MD5 hash
    MD5Context md5c;
    MD5Init(&md5c);

    //Adding the filesize to the hash
    Utf8String hashWithFilesize = uri;
    hashWithFilesize.append(Utf8PrintfString("%d", filesize));

    // Append that data to the MD5 buffer 
    MD5Update(&md5c, (const unsigned char*)hashWithFilesize.c_str(), (int)strlen(uri.c_str()));
    // Calculate the hash of the current fragment
    unsigned char signature[16];
    MD5Final(signature, &md5c);

    WString finalHashValue;
    char tempHashFragment[3];
    // Write the resulting hashed strings in the result vector
    for (int j = 0; j < sizeof signature; ++j)
    {
        // Bytes are written one by one (one byte equals 2 hex characters)
        sprintf_s(tempHashFragment, sizeof(tempHashFragment), "%02X", signature[j]);
        finalHashValue.AppendA(tempHashFragment);
    }

    // Appending the hash to the current directory
    filenameTemp.AppendUtf8("_");
    filenameTemp.append(finalHashValue.c_str());
    separatedDirectoryForZip.AppendToPath(filenameTemp.c_str());
    separatedDirectoryForZip.AppendSeparator();

    if (!separatedDirectoryForZip.DoesPathExist())
        BeFileName::CreateNewDirectory(separatedDirectoryForZip);

    return separatedDirectoryForZip.GetWCharCP();
}


int wmain(int pi_Argc, wchar_t *pi_ppArgv[])
{
    printf("DownloadEngine...\n");

    //printf("argc = %d\n", pi_Argc);
    //for (int iArg = 0; iArg < pi_Argc; iArg++)
    //    printf("argv[%d] = %ls\n", iArg, pi_ppArgv[iArg]);

    RealityDataDownload::UrlLink_UrlFile urlList;

    if (pi_Argc != 3 && pi_Argc != 1)
        { 
        printf("Usage: DownloadEngineExample <xrdpFile outputFolder> \n if no file specify, some hardcoded urls will be used\n   Example:DownloadEngineExample test1.xrdp c:\\temp\\ ");
        return 1;
        }
    else if ((3 == pi_Argc) && BeFileName::DoesPathExist(pi_ppArgv[1]))
        { 
        sOutputFolder = pi_ppArgv[2];

        //Preparing to parse the XML of the *.xrdp
        WString errorMsg;
        BeFileNameCR xrdpFile = BeFileName(pi_ppArgv[1]);
        RealityPackageStatus readStatus = RealityPackageStatus::UnknownError;
        RealityDataPackagePtr pDataPackage = RealityDataPackage::CreateFromFile(readStatus, xrdpFile, &errorMsg);

        // Only the raster and terrain for the moment. 
        //    for the other, need to rework the code to determine the filename.
        bvector<RealityPackage::RealityDataPtr> DownloadList (pDataPackage->GetImageryGroup().begin(), pDataPackage->GetImageryGroup().end());
        DownloadList.insert (DownloadList.end(), pDataPackage->GetTerrainGroup().begin(), pDataPackage->GetTerrainGroup().end());
        //DownloadList.insert(DownloadList.end(), pDataPackage->GetModelGroup().begin(), pDataPackage->GetModelGroup().end());

        if (DownloadList.size() == 0)
            {
            printf("Input(.xrdp) file is empty: %ls", pi_ppArgv[1]);
            return 1;
            }

        for (auto& realityData : DownloadList)
            {
            RealityDataSourceCP pSource = dynamic_cast<RealityDataSourceCP>(&realityData->GetSource(0));
            if (NULL == pSource)
                continue;

            WString filename = createDirWithHash(pSource->GetUri(), sOutputFolder, realityData->GetSource(0).GetFilesize());
            RealityDataDownload::ExtractFileName(filename, pSource->GetUri());
            urlList.push_back(std::make_pair(pSource->GetUri(), filename));
            }
        }
    else 
        // Manually set some urls
        {
        //bvector<AString> urlUSGSLink =
        //    {"http://tdds.cr.usgs.gov/browse/ortho/17T/LE/17TLE480475_200909_0x3000m_CL_1.jpg", 
        //     "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/19/IMG/ned19_n40x75_w112x00_ut_saltlakecity_2006_thumb.jpg",
        //     "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NAIP/ut_2014/40111/m_4011126_ne_12_1_20140701_20141030.jp2", 
        //    };


        bvector<AString> urlUSGSLink =
        { 
    //      "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/Elevation/13/IMG/USGS_NED_13_n41w076_IMG.zip",
    //    "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/Elevation/1/IMG/USGS_NED_1_n41w076_IMG.zip",
    //    "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/19/IMG/ned19_n40x25_w075x75_pa_northeast_2010.zip",
        "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/19/IMG/ned19_n40x25_w075x75_pa_east_2006.zip"
        };

        bvector<AString> urlOSMLink =
            {"http://api.openstreetmap.org/api/0.6/map?bbox=-112.132,40.5292,-111.52,40.8019",
             "http://overpass-api.de/api/map?bbox=-112.1320,40.5292,-111.5200,40.8019",
            };


        for (size_t i=0; i<urlUSGSLink.size(); ++i)
            {
            WString filename(sOutputFolder);
            RealityDataDownload::ExtractFileName(filename, urlUSGSLink[i]);

            urlList.push_back(std::make_pair(urlUSGSLink[i], filename));
            }

        for (size_t i = 0; i < urlOSMLink.size(); ++i)
            {
            wchar_t filename[1024];
            swprintf (filename, 1024, L"%lsOsmFile_%2llu.osm", sOutputFolder.c_str(), i);

            urlList.push_back(std::make_pair(urlOSMLink[i], WString (filename)));
            }
        }


    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(urlList);
    if (pDownload != NULL)
        {
        pDownload->SetProgressCallBack(callback_progress_func, 0.1);
        pDownload->SetStatusCallBack(callback_status_func);
        pDownload->Perform();
        }
    else
        pDownload->SetProgressCallBack(callback_progress_func);     

    printf("-----Done-----");
    _getch();
    return 0;
}







