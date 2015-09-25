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
    RealityDataDownload::FileTransfer* pEntry = (RealityDataDownload::FileTransfer*)pClient;

    printf("* ProgressInfo: (%d) %ls -- %zu of %zu\n", index, pEntry->filename.c_str(), ByteCurrent, ByteTotal);

    return 0;   // # 0 --> will abort the transfer.
    }

static void callback_status_func (int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    RealityDataDownload::FileTransfer* pEntry = (RealityDataDownload::FileTransfer*)pClient;
    printf("****** Status: (%d) ErrCode: %d - (%s) <%ls>\n", index, ErrorCode, pMsg, pEntry->filename.c_str());
    }


int wmain(int pi_Argc, wchar_t *pi_ppArgv[])
{
    (void)pi_Argc;
    (void)pi_ppArgv;

    printf("DownloadEngine...\n");

    std::pair<AString, WString> urlList2[] = 
        {   {"http://tdds.cr.usgs.gov/browse/ortho/17T/LE/17TLE480475_200909_0x3000m_CL_1.jpg", L"k:\\tmp\\Data\\17TLE480475_200909_0x3000m_CL_1.jpg"},
            {"http://api.openstreetmap.org/api/0.6/map?bbox=-112.132,40.5292,-111.52,40.8019", L"k:\\tmp\\Data\\SaltLake_1.osm" },
            {"http://overpass-api.de/api/map?bbox=-112.1320,40.5292,-111.5200,40.8019", L"k:\\tmp\\Data\\SaltLake_2.osm" },
            {"http://XXgisdata.usgs.gov/tdds/downloadfile.php?TYPE\u003dortho\u0026ORIG\u003dSBDDG\u0026FNAME\u003d12TVL275015_201203_0x1250m_4B_1.zip", L"k:\\tmp\\Data\\u003d12TVL275015_201203_0x1250m_4B_1.zip" },
            {"ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/19/IMG/ned19_n40x75_w112x00_ut_saltlakecity_2006_thumb.jpg", L"k:\\tmp\\Data\\ned19_n40x75_w112x00_ut_saltlakecity_2006_thumb.jpg" },
            {"ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NAIP/ut_2014/40111/m_4011126_ne_12_1_20140701_20141030.jp2", L"k:\\tmp\\Data\\m_4011126_ne_12_1_20140701_20141030.jp2" }
        };

    {
    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(urlList2, 6);
    if (pDownload != NULL)
        {
        pDownload->SetProgressCallBack(callback_progress_func);
        pDownload->SetStatusCallBack(callback_status_func);
        pDownload->Perform();
        }
    }


    printf("-----Done-----");
    _getch();
    return 0;
}







