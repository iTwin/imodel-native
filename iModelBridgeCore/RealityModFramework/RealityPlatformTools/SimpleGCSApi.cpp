#include <iostream>
#include <Bentley/Base64Utilities.h>
#include <Bentley/DateTime.h>

#include <RealityPlatformTools/SimpleGCSApi.h>
#include <RealityPlatform/SpatioTemporalData.h>
#include <RealityPlatformTools/RealityDataDownload.h>
#include <RealityPlatformTools/RealityConversionTools.h>
#include <RealityPlatformTools/md5.h>

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

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Francis Boily                   09/2015
//+---------------+---------------+---------------+---------------+---------------+------*/
/*static WString createDirWithHash(Utf8StringCR uri, WStringCR tempPath, uint64_t filesize)
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
    }*/

void GCSRequestManager::Setup()
    {
    Utf8String serverName = MakeBuddiCall(L"ContextServices");
    WSGServer server = WSGServer(serverName, false);

    RawServerResponse versionResponse = RawServerResponse();
    Utf8String version = server.GetVersion(versionResponse);
    if (versionResponse.responseCode > 399)
        {
        ReportError("cannot reach server");
        return;
        }

    GeoCoordinationService::SetServerComponents(serverName, version, "IndexECPlugin--Server", "RealityModeling");
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

    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(downloadOrder);
    RealityDataDownload::DownloadReport* report = nullptr;
    if (pDownload != NULL)
        {
        pDownload->SetProgressCallBack(GCS_progress_func, 0.1);
        pDownload->SetStatusCallBack(GCS_status_func);
        if(!certificatePath.empty())
            pDownload->SetCertificatePath(certificatePath);
        if(proxyCallback)
            pDownload->SetProxyCallBack(proxyCallback);
        report = pDownload->Perform();
        }
    else
        {
        ReportError("Failed to initialize download");
        return;
        }

    Report("-----Download Complete-----");

    Utf8String reportString;
    report->ToXml(reportString);

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