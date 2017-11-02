/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/Example/DownloadEngineExample.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
USING_NAMESPACE_BENTLEY_REALITYPLATFORM

WString sOutputFolder = L"d:\\tmp\\data\\";     // Could be overriden by parameter at the execution.

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

    RealityDataDownload::Link_File_wMirrors_wSisters urlList;

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
        bvector<RealityPackage::PackageRealityDataPtr> DownloadList (pDataPackage->GetImageryGroup().begin(), pDataPackage->GetImageryGroup().end());
        DownloadList.insert (DownloadList.end(), pDataPackage->GetTerrainGroup().begin(), pDataPackage->GetTerrainGroup().end());
        //DownloadList.insert(DownloadList.end(), pDataPackage->GetModelGroup().begin(), pDataPackage->GetModelGroup().end());

        if (DownloadList.size() == 0)
            {
            printf("Input(.xrdp) file is empty: %ls", pi_ppArgv[1]);
            return 1;
            }

        bvector<RealityDataDownload::url_file_pair> wMirrors;
        bvector<bvector<RealityDataDownload::url_file_pair>> wSisters;
        for (auto& realityData : DownloadList)
            {
            RealityDataSourceCP pSource = dynamic_cast<RealityDataSourceCP>(&realityData->GetSource(0));
            if (NULL == pSource)
                continue;

            WString filename = createDirWithHash(pSource->GetUri().GetSource(), sOutputFolder, realityData->GetSource(0).GetSize());
            RealityDataDownload::ExtractFileName(filename, pSource->GetUri().GetSource());
            wMirrors = bvector<RealityDataDownload::url_file_pair>();
            wMirrors.push_back(RealityDataDownload::url_file_pair(pSource->GetUri().GetSource(), filename));
            wSisters = bvector<bvector<RealityDataDownload::url_file_pair>>();
            wSisters.push_back(wMirrors);
            urlList.push_back(wSisters); //insert each entry as completely independant from the others
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

        WString filename1(sOutputFolder);
        RealityDataDownload::ExtractFileName(filename1, L"can01.zip");
        WString filename2(sOutputFolder);
        RealityDataDownload::ExtractFileName(filename2, L"can02.zip");
        WString filename3(sOutputFolder);
        RealityDataDownload::ExtractFileName(filename3, L"can03.zip");
        WString filename4(sOutputFolder);
        RealityDataDownload::ExtractFileName(filename4, L"can04.zip");
        WString filename5(sOutputFolder);
        RealityDataDownload::ExtractFileName(filename5, L"can05.zip");
        WString filename6(sOutputFolder);
        RealityDataDownload::ExtractFileName(filename6, L"can06.zip");
        WString filename7(sOutputFolder);
        RealityDataDownload::ExtractFileName(filename7, L"can07.zip");
        WString filename8(sOutputFolder);
        RealityDataDownload::ExtractFileName(filename8, L"can08.zip");
        WString filenameBad(sOutputFolder);
        RealityDataDownload::ExtractFileName(filenameBad, L"badFile.zip");

        bvector<bvector<RealityDataDownload::url_file_pair>> sisterFileTest =
            {
                {
                RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a01_tif.zip", filename1),
                RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_0121_tif.zip", filenameBad)
                },
                {
                RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a05_tif.zip", filename5),
                RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a06_tif.zip", filename6),
                RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a07_tif.zip", filename7),
                RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a08_tif.zip", filename8)
                }
            };

        bvector<bvector<RealityDataDownload::url_file_pair>> cacheTest1 =
        {
            {
            RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a02_tif.zip", filename2),
            RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a06_tif.zip", filename6),
            RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a07_tif.zip", filename7)
            }
        };

        bvector<bvector<RealityDataDownload::url_file_pair>> cacheTest2 =
        {
            {
            RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a02_tif.zip", filename2),
            RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a06_tif.zip", filename3),
            RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a07_tif.zip", filename7)
            }
        };

        bvector<bvector<RealityDataDownload::url_file_pair>> cacheTest3 =
        {
            {
            RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a02_tif.zip", filename2),
            RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a06_tif.zip", filename6),
            RealityDataDownload::url_file_pair("ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/a/canimage_012a07_tif.zip", filename4)
            }
        };

        bvector<RealityDataDownload::url_file_pair> wMirrors;
        bvector<bvector<RealityDataDownload::url_file_pair>> wSisters;

        for (size_t i=0; i<urlUSGSLink.size(); ++i)
            {
            WString filename(sOutputFolder);
            RealityDataDownload::ExtractFileName(filename, urlUSGSLink[i]);

            wMirrors = bvector<RealityDataDownload::url_file_pair>();
            wMirrors.push_back(RealityDataDownload::url_file_pair(urlUSGSLink[i], filename));
            wSisters = bvector<bvector<RealityDataDownload::url_file_pair>>();
            wSisters.push_back(wMirrors);
            urlList.push_back(wSisters); //independant file test
            }

        for (size_t i = 0; i < urlOSMLink.size(); ++i)
            {
            wchar_t filename[1024];
            swprintf (filename, 1024, L"%lsOsmFile_%2llu.osm", sOutputFolder.c_str(), i);

            wMirrors = bvector<RealityDataDownload::url_file_pair>();
            wMirrors.push_back(RealityDataDownload::url_file_pair("http://api.openstreetmap.org/api/0.6/map?ddox=-112.132,40.5292,-111.52,40.8019", WString(filename))); //url with typo, to force use of mirror
            wMirrors.push_back(RealityDataDownload::url_file_pair(urlOSMLink[i], WString (filename)));
            wSisters = bvector<bvector<RealityDataDownload::url_file_pair>>();
            wSisters.push_back(wMirrors);
            urlList.push_back(wSisters); //mirror file test
            }

        urlList.push_back(sisterFileTest); //sister file test
        urlList.push_back(cacheTest1); //dl first file, other 2 exist
        urlList.push_back(cacheTest2); //dl second file, other 2 exist
        urlList.push_back(cacheTest3); //dl last file, other 2 exist
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







