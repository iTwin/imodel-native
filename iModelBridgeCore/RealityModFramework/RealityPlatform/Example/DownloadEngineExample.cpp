/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/Example/DownloadEngineExample.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/BeFileName.h>
#include <Bentley/RefCounted.h>

#include <RealityPlatform/RealityDataDownload.h>

#include <stdio.h>
#include <conio.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM



static int callback_progress_func (int index,void *pClient, size_t ByteCurrent,size_t ByteTotal)
    {
    int ret = 0;

    RealityDataDownload::FileTransfer* pEntry = (RealityDataDownload::FileTransfer*)pClient;

    printf("* ProgressInfo: (%d) %ls -- %lu of %lu\n", index, pEntry->filename.c_str(), ByteCurrent, ByteTotal);

    return ret;   // # 0 --> will abort the transfer.
    }

static void callback_status_func (int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    RealityDataDownload::FileTransfer* pEntry = (RealityDataDownload::FileTransfer*)pClient;
    printf("****** Status: (%d) ErrCode: %d - (%s) <%ls>\n", index, ErrorCode, pMsg, pEntry->filename.c_str());

    if (ErrorCode == 0)
        {
        WString out(L"k:\\tmp\\data\\unzip\\");
        RealityDataDownload::UnZipFile(pEntry->filename, out);
        }
    }


int wmain(int pi_Argc, wchar_t *pi_ppArgv[])
{
    (void)pi_Argc;
    (void)pi_ppArgv;

    printf("DownloadEngine...\n");

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


    RealityDataDownload::UrlLink_UrlFile urlList;

    for (size_t i=0; i<urlUSGSLink.size(); ++i)
        {
        WString filename(L"k:\\tmp\\data\\");
        RealityDataDownload::ExtractFileName(filename, urlUSGSLink[i]);

        urlList.push_back(std::make_pair(urlUSGSLink[i], filename));
        }

    for (size_t i = 0; i < urlOSMLink.size(); ++i)
    {
        wchar_t filename[1024];
        swprintf (filename, 1024, L"k:\\tmp\\data\\OsmFile_%2lu.osm", i);

        urlList.push_back(std::make_pair(urlOSMLink[i], WString (filename)));
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







