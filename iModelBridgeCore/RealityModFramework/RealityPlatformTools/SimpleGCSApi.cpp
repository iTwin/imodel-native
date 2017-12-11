#include <iostream>
#include <Bentley/Base64Utilities.h>
#include <Bentley/DateTime.h>

#include <RealityPlatformTools/SimpleGCSApi.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

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

void GCSRequestManager::SimplePackageDownload(bvector<GeoPoint2d> footprint, bvector<RealityDataBase::Classification> classes, SE_selectionFunction pi_func, BeFileName path)
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

    bvector<Utf8String> selectedSubset = pi_func(dataset);

    PackagePreparationRequest prepReq = PackagePreparationRequest(space, selectedSubset);
    RawServerResponse prepResponse = RawServerResponse();
    Utf8String prep = GeoCoordinationService::Request(prepReq, prepResponse);

    if (prepResponse.status != RequestStatus::OK)
        {
        ReportError(prepResponse.body);
        return;
        }

    PreparedPackageRequest packReq = PreparedPackageRequest(prep);
    RawServerResponse packResponse = RawServerResponse();
    GeoCoordinationService::Request(packReq, path, packResponse);

    if (!path.DoesPathExist())
        {
        ReportError("download failed");
        return;
        }
    
    Utf8String report = "<RealityDataDownload_DownloadReport PackageId=\"0\" Date=\"2017 - 04 - 04T20:07 : 17.807Z\"><File FileName = \"LC80160332016095LGN00_B2.TIF\" url = \"https://s3-us-west-2.amazonaws.com/landsat-pds/L8/016/033/LC80160332016095LGN00/LC80160332016095LGN00_B2.TIF\" filesize = \"62748461\" timeSpent = \"7\"><DownloadAttempt attemptNo = \"1\" CURLcode = \"0\" downloadProgress = \"1\" / >< / File>< / RealityDataDownload_DownloadReport>";

    BeFile reportPath = path.PopDir();
    reportPath.AppendToPath(L"report.xml");

    BeFile stream;
    stream.Create(baseFile.c_str(), true);
    stream.Open(baseFile.GetNameUtf8(), BeFileAccess::Write);
    uint32_t bytesWritten;
    uint32_t numBytes = (uint32_t)report.length();

    stream.Write(&bytesWritten, report.c_str(), numBytes);
    stream.Close();

    DateTime now = DateTime::GetCurrentTimeUtc();

    DownloadReportUploadRequest upReq = DownloadReportUploadRequest(prep, Utf8PrintfString("report-%s.xml", now.ToString()), reportPath);
    RawServerResponse upResponse = RawServerResponse();
    GeoCoordinationService::Request(upReq, upResponse);

    reportPath.BeDeleteFile();
    }