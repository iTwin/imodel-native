/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../RealityPlatformTools/SimpleGCSApi.cpp"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

bool GCSRequestManager::AlternateDownload
    (RealityDataDownload::DownloadReport** report, const RealityDataDownload::Link_File_wMirrors_wSisters& downloadOrder, 
        BeFileName certificatePath, RealityDataDownload_ProxyCallBack proxyCallback)
    {
    ReportError("No way to download files, please set an RPT_DownloadFunction to use and try again");
    return false;
    }