#include "../RealityPlatformTools/SimpleGCSApi.cpp"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

bool GCSRequestManager::AlternateDownload
    (RealityDataDownload::DownloadReport* report, const RealityDataDownload::Link_File_wMirrors_wSisters& downloadOrder,
        BeFileName certificatePath, RealityDataDownload_ProxyCallBack proxyCallback)
    {
    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(downloadOrder);
    if (pDownload != NULL)
        {
        pDownload->SetProgressCallBack(GCS_progress_func, 0.1);
        pDownload->SetStatusCallBack(GCS_status_func);
        if (!certificatePath.empty())
            pDownload->SetCertificatePath(certificatePath);
        if (proxyCallback)
            pDownload->SetProxyCallBack(proxyCallback);
        report = pDownload->Perform();
        }
    else
        {
        ReportError("Failed to initialize download");
        return false;
        }

    return true;
    }