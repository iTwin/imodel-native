/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatformTools/GeoCoordinationService.h>
#include <SpatioTemporalSelector/SpatioTemporalSelector.h>
#include <RealityPlatform/SpatioTemporalData.h>
#include <curl/curl.h>

#include <stdio.h>
#include <conio.h>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

static bool FilterFunc(SpatialEntityPtr entity) 
    {
    Json::Value provider = entity->GetProvider(); 
    return provider.isString() ? provider.asString().EqualsI("Amazon Landsat 8") : false;
    }

/*-----------------------------------------------------------------//
* Bentley GeoCoordinationServiceExample
* This application uses hard coded values and is not guaranteed to 
* function properly.
* The purpose of its existence is only to show how to structure
* and execute RealityDataService operations
//----------------------------------------------------------------*/
int main(int argc, char *argv[])
    {
    GeoCoordinationService::SetServerComponents("dev-contextservices-eus.cloudapp.net", "2.4", "IndexECPlugin--Server", "RealityModeling");
    GeoCoordinationService::SetUserAgent("GeoCoordinateServiceExample dummy UserAgent");

    std::cout << GeoCoordinationService::GetServerName() << std::endl;
    std::cout << GeoCoordinationService::GetWSGProtocol() << std::endl;
    std::cout << GeoCoordinationService::GetRepoName() << std::endl;
    std::cout << GeoCoordinationService::GetSchemaName() << std::endl << std::endl;
    std::cout << GeoCoordinationService::GetUserAgent() << std::endl << std::endl;

    BingKeyRequest bkRequest = BingKeyRequest("1000");
    RawServerResponse bkResponse = RawServerResponse();
    Utf8String bingKey, expirationDate;
    GeoCoordinationService::Request(bkRequest, bkResponse, bingKey, expirationDate);

    std::cout << bingKey.c_str() << std::endl;
    std::cout << expirationDate.c_str() << std::endl;

    bvector<GeoPoint2d> space = bvector<GeoPoint2d>();
    space.push_back(GeoPoint2d::From(115.73, 49.44));
    space.push_back(GeoPoint2d::From(115.73, 50.18));
    space.push_back(GeoPoint2d::From(116.53, 50.18));
    space.push_back(GeoPoint2d::From(116.53, 49.44));
    space.push_back(GeoPoint2d::From(115.73, 49.44));

    SpatialEntityWithDetailsSpatialRequest spatialReq = SpatialEntityWithDetailsSpatialRequest(space, RealityDataBase::Classification::IMAGERY | RealityDataBase::Classification::TERRAIN);
    RawServerResponse spatialResponse = RawServerResponse();
    bvector<SpatialEntityPtr> spatialEntities = GeoCoordinationService::Request(spatialReq, spatialResponse);

    if (spatialResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "Entities found in provided space :" << std::endl;
        std::cout << spatialEntities.size() << std::endl << std::endl;
        }
    else
        std::cout << "error performing spatial request" << std::endl;

    SpatialEntityWithDetailsByIdRequest idReq = SpatialEntityWithDetailsByIdRequest("100");
    RawServerResponse idResponse = RawServerResponse();
    SpatialEntityPtr idEntity = GeoCoordinationService::Request(idReq, idResponse);

    if (idResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "Entity found for id '100' with provider :" << std::endl;
        std::cout << idEntity->GetProvider() << std::endl << std::endl;
        }
    else
        std::cout << "error SpatialEntityWithDetailsByIdRequest" << std::endl;


    SpatialEntityByIdRequest idBasicReq = SpatialEntityByIdRequest("100");
    RawServerResponse idBasicResponse = RawServerResponse();
    SpatialEntityPtr idBasicEntity = GeoCoordinationService::Request(idBasicReq, idBasicResponse);

    if (idBasicResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "Entity found for id '100' with provider :" << std::endl;
        std::cout << idBasicEntity->GetProvider() << std::endl << std::endl;
        }
    else
        std::cout << "error SpatialEntityByIdRequest" << std::endl;

    SpatialEntityDataSourceByIdRequest dsReq = SpatialEntityDataSourceByIdRequest("100");
    RawServerResponse dsResponse = RawServerResponse();
    SpatialEntityDataSourcePtr dSource = GeoCoordinationService::Request(dsReq, dsResponse);

    if (dsResponse.status != RequestStatus::BADREQ)
    {
        std::cout << "DataSource found for id '100' with Url :" << std::endl;
        std::cout << dSource->GetUri().ToString() << std::endl << std::endl;
    }
    else
        std::cout << "error SpatialEntityDataSourceByIdRequest" << std::endl;

    SpatialEntityServerByIdRequest serverReq = SpatialEntityServerByIdRequest("1");
    RawServerResponse serverResponse = RawServerResponse();
    SpatialEntityServerPtr serverEntity = GeoCoordinationService::Request(serverReq, serverResponse);

    if (serverResponse.status != RequestStatus::BADREQ)
    {
        std::cout << "Server found for id '1' with name :" << std::endl;
        std::cout << serverEntity->GetName() << std::endl << std::endl;
    }
    else
        std::cout << "error SpatialEntityServerByIdRequest" << std::endl;

    SpatialEntityMetadataByIdRequest metaReq = SpatialEntityMetadataByIdRequest("1");
    RawServerResponse metaResponse = RawServerResponse();
    SpatialEntityMetadataPtr metaEntity = GeoCoordinationService::Request(metaReq, metaResponse);

    if (metaResponse.status != RequestStatus::BADREQ)
    {
        std::cout << "Metadata found for id '1' with provenance :" << std::endl;
        std::cout << metaEntity->GetProvenance() << std::endl << std::endl;
    }
    else
        std::cout << "error SpatialEntityMetadataByIdRequest" << std::endl;


    SpatialEntityDatasetPtr dataset = SpatialEntityDataset::CreateFromJson(spatialResponse.body.c_str());
    if (dataset.IsNull())
        return -1;

    auto imageryIt(dataset->GetImageryGroupR().begin());
    while (imageryIt != dataset->GetImageryGroupR().end())
        {
        double occlusion = (*imageryIt)->GetOcclusion();
        if ((occlusion  > 50.0) || FilterFunc(*imageryIt))
            {
            imageryIt = dataset->GetImageryGroupR().erase(imageryIt);
            }
        else
            {
            imageryIt++;
            }
        }

    RealityPlatform::SpatioTemporalSelector::ResolutionMap selectedIds = SpatioTemporalSelector::GetIDsByRes(*dataset, space);
    
    PackagePreparationRequest prepReq = PackagePreparationRequest(space, selectedIds[RealityPlatform::ResolutionCriteria::Low]);
    RawServerResponse prepResponse = RawServerResponse();
    Utf8String prep = GeoCoordinationService::Request(prepReq, prepResponse);

    if (prepResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "PackageId  :" << std::endl;
        std::cout << prep << std::endl << std::endl;
        }
    else
        std::cout << "error PackagePreparationRequest" << std::endl;

    PreparedPackageRequest packReq = PreparedPackageRequest(prep);
    RawServerResponse packResponse = RawServerResponse();

    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);

    BeFileName baseFile = BeFileName(exeDir.c_str());
    BeFileName packageName = baseFile;
    packageName.AppendToPath(L"GCSExamplePackage.xrdp");

    GeoCoordinationService::Request(packReq, packageName, packResponse);

    if (packageName.DoesPathExist())
        std::cout << "Package downloaded" << std::endl;
    else
        std::cout << "error with PreparedPackageRequest" << std::endl;

    Utf8String report = "<RealityDataDownload_DownloadReport PackageId=\"0\" Date=\"2017 - 04 - 04T20:07 : 17.807Z\"><File FileName = \"LC80160332016095LGN00_B2.TIF\" url = \"https://s3-us-west-2.amazonaws.com/landsat-pds/L8/016/033/LC80160332016095LGN00/LC80160332016095LGN00_B2.TIF\" filesize = \"62748461\" timeSpent = \"7\"><DownloadAttempt attemptNo = \"1\" CURLcode = \"0\" downloadProgress = \"1\" / >< / File>< / RealityDataDownload_DownloadReport>";

    packageName.BeDeleteFile();

    BeFileName reportName = baseFile;
    baseFile.AppendToPath(L"report.xml");

    BeFile stream;
    stream.Create(baseFile.c_str(), true);
    stream.Open(baseFile.GetNameUtf8(), BeFileAccess::Write);
    uint32_t bytesWritten;
    uint32_t numBytes = (uint32_t)report.length();

    stream.Write(&bytesWritten, report.c_str(), numBytes);
    stream.Close();

    DownloadReportUploadRequest upReq = DownloadReportUploadRequest(prep, "report.xml", baseFile);
    RawServerResponse upResponse = RawServerResponse();
    GeoCoordinationService::Request(upReq, upResponse);

    if (upResponse.status != RequestStatus::BADREQ)
        {
        std::cout << "Download Report Uploaded" << std::endl;
        }
    else
        std::cout << "error DownloadReportUpload" << std::endl;

    baseFile.BeDeleteFile();

    getch();
    }