#include <iostream>
#include <Bentley/Base64Utilities.h>
#include <Bentley/DateTime.h>

#include <RealityPlatformTools/SimpleGCSApi.h>
#include <RealityPlatform/SpatioTemporalData.h>
#include <RealityPlatformTools/RealityDataDownload.h>
#include <RealityPlatformTools/RealityConversionTools.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

int GCSRequestManager::GCS_progress_func(int index, void *pClient, size_t ByteCurrent, size_t ByteTotal)
    {
    int ret = 0;

    RealityDataDownload::FileTransfer* pEntry = (RealityDataDownload::FileTransfer*)pClient;

    GCSRequestManager::Report(Utf8PrintfString("* ProgressInfo: (%d) %ls -- %llu of %llu\n", index, pEntry->filename.c_str(), ByteCurrent, ByteTotal));

    return ret;   // # 0 --> will abort the transfer.
    }

void GCSRequestManager::GCS_status_func(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    RealityDataDownload::FileTransfer* pEntry = (RealityDataDownload::FileTransfer*)pClient;
    GCSRequestManager::Report(Utf8PrintfString("****** Status: (%d) ErrCode: %d - fromCache(%d) - (%s) <%ls>\n", index, ErrorCode, pEntry->fromCache, pMsg, pEntry->filename.c_str()));

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
            GCSRequestManager::Report(Utf8PrintfString("******     Unzip status Success\n"));
        else
            GCSRequestManager::ReportError(Utf8PrintfString("******     Unzip status Failed\n"));
        }
    else
        {
        // An error occured ... we will try one of the alternate source
        }
    }

RPT_DownloadFunction GCSRequestManager::s_downloadFunction = nullptr;

void GCSRequestManager::Setup(Utf8String serverUrl, RPT_DownloadFunction downloadCallback)
    {
    s_downloadFunction = downloadCallback;

    Utf8String serverName = serverUrl;
    if(serverName.empty())
        serverName = MakeBuddiCall(L"ContextServices");
    WSGServer server = WSGServer(serverName, false);

    RawServerResponse versionResponse = RawServerResponse();
    Utf8String version = server.GetVersion(versionResponse);
    if (versionResponse.responseCode > 399 || version.empty())
        {
        ReportError("cannot reach server");
        return;
        }

    GeoCoordinationService::SetServerComponents(serverName, version, "IndexECPlugin--Server", "RealityModeling");
    }

void GCSRequestManager::SetDownloadFunction(RPT_DownloadFunction downloadCallback)
    {
    s_downloadFunction = downloadCallback;
    }

void GCSRequestManager::SimplePackageDownload(bvector<GeoPoint2d> footprint, bvector<RealityDataBase::Classification> classes, SE_selectionFunction pi_func, BeFileName path,
    BeFileName certificatePath, RealityDataDownload_ProxyCallBack proxyCallback)
    {
    int classMask = 0;
    for(int i = 0; i < classes.size(); ++i)
        classMask |= classes[i];

    SpatialEntityWithDetailsSpatialRequest spatialReq = SpatialEntityWithDetailsSpatialRequest(footprint, classMask);
    RawServerResponse spatialResponse = RawServerResponse();
    bvector<SpatialEntityPtr> spatialEntities = GeoCoordinationService::Request(spatialReq, spatialResponse);

    if (spatialResponse.status != RequestStatus::OK)
        {
        ReportError(spatialResponse.body);
        return;
        }

    SpatialEntityDatasetPtr dataset = SpatialEntityDataset::CreateFromJson(spatialResponse.body.c_str());
    if (dataset.IsNull())
        {
        ReportError("no spatial entities found for given footprint");
        return;
        }

    bvector<Utf8String> selectedSubset = pi_func(footprint, dataset);

    PackagePreparationRequest prepReq = PackagePreparationRequest(footprint, selectedSubset);
    RawServerResponse prepResponse = RawServerResponse();
    Utf8String prep = GeoCoordinationService::Request(prepReq, prepResponse);

    if (prepResponse.status != RequestStatus::OK)
        {
        ReportError(prepResponse.body);
        return;
        }

    PreparedPackageRequest packReq = PreparedPackageRequest(prep);
    RawServerResponse packResponse = RawServerResponse();

    BeFileName packagePath = BeFileName(path.c_str());
    packagePath.AppendToPath(L"GCSPackage.xrdp");

    GeoCoordinationService::Request(packReq, packagePath, packResponse);

    uint64_t size = 0;
    packagePath.GetFileSize(size);
    if(size <= 0)
        {
        ReportError("package download failed");
        return;
        }

    WString parseError = L"";
    RealityDataDownload::Link_File_wMirrors_wSisters downloadOrder = RealityConversionTools::PackageFileToDownloadOrder(packagePath, &parseError, path);

    if(!parseError.empty())
        {
        ReportError(Utf8String(parseError.c_str()));
        return;
        }
    
    bool submitReport = true;
    RealityDataDownload::DownloadReport* report = nullptr;
    if(s_downloadFunction)
        {
        report = s_downloadFunction(downloadOrder, certificatePath, proxyCallback);
        }
    else
        {
        submitReport = AlternateDownload(&report, downloadOrder, certificatePath, proxyCallback);
        }

    if (submitReport)
        {
        Report("-----Download Complete-----");

        Utf8String reportString;
        report->ToXml(reportString);

        delete report;

        BeFileName reportPath = path.PopDir();
        reportPath.AppendToPath(L"report.xml");

        BeFile stream;
        stream.Create(reportPath.c_str(), true);
        stream.Open(reportPath.GetNameUtf8(), BeFileAccess::Write);
        uint32_t bytesWritten;
        uint32_t numBytes = (uint32_t)reportString.length();

        stream.Write(&bytesWritten, reportString.c_str(), numBytes);
        stream.Close();

        DateTime now = DateTime::GetCurrentTimeUtc();

        Utf8String distinctReportName = Utf8PrintfString("report-%s.xml", now.ToString());
        distinctReportName.ReplaceAll(":", ".");

        DownloadReportUploadRequest upReq = DownloadReportUploadRequest(prep, distinctReportName, reportPath);
        RawServerResponse upResponse = RawServerResponse();
        GeoCoordinationService::Request(upReq, upResponse);

        reportPath.BeDeleteFile();
        }
    }

ConnectedResponse GCSRequestManager::SimpleBingKeyRequest(Utf8StringCR productId, Utf8StringR key, Utf8StringR expirationDate)
    {
    ConnectedResponse response = ConnectedResponse();

    BingKeyRequest bkRequest = BingKeyRequest(productId);
    RawServerResponse rawResponse = RawServerResponse();
    GeoCoordinationService::Request(bkRequest, rawResponse, key, expirationDate);
   
    response.Clone(rawResponse);

    return response;
    }