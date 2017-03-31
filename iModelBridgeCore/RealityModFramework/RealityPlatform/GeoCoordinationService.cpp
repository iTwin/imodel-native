/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/GeoCoordinationService.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/BeFileListIterator.h>
#include <Bentley/bset.h>
#include <Bentley/DateTime.h>

//#include <iostream>
#include <RealityPlatform/GeoCoordinationService.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityConversionTools.h>

#include <stdio.h>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <Bentley/Base64Utilities.h>

#define MAX_NB_CONNECTIONS          10
USING_NAMESPACE_BENTLEY_REALITYPLATFORM


//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
static Utf8String GetPolygonAsString(bvector<GeoPoint2d> area, bool urlEncode)
    {
    Utf8String polygon;
    bool first = true;
    for (auto& point : area)
        {
        if (first)
            first = false;
        else
            polygon.append(urlEncode ? "%2C" : ",");
        polygon.append(urlEncode ? "%5B" : "[");
        polygon.append(Utf8PrintfString("%f%s%f", point.longitude, (urlEncode ? "%2C" : ","), point.latitude));
        polygon.append(urlEncode ? "%5D" : "]");
        }
    return polygon;
    }

static size_t CurlReadDataCallback(void* buffer, size_t size, size_t count, BeFile* fileStream)
    {
    uint32_t bytesRead = 0;
    
    BeFileStatus status = fileStream->Read(buffer, &bytesRead, (uint32_t)(size * count));
    if (status != BeFileStatus::Success)
        return 0;

    return bytesRead;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
Utf8StringCR GeoCoordinationServiceRequest::GetServerName() const { return GeoCoordinationService::GetServerName(); }

Utf8StringCR GeoCoordinationServiceRequest::GetVersion() const { return GeoCoordinationService::GetWSGProtocol(); }

Utf8StringCR GeoCoordinationServiceRequest::GetSchema() const { return GeoCoordinationService::GetSchemaName(); }

Utf8StringCR GeoCoordinationServiceRequest::GetRepoId() const { return GeoCoordinationService::GetRepoName(); }



//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
Utf8StringCR GeoCoordinationServicePagedRequest::GetServerName() const { return GeoCoordinationService::GetServerName(); }
Utf8StringCR GeoCoordinationServicePagedRequest::GetVersion() const { return GeoCoordinationService::GetWSGProtocol(); }
Utf8StringCR GeoCoordinationServicePagedRequest::GetSchema() const { return GeoCoordinationService::GetSchemaName(); }
Utf8StringCR GeoCoordinationServicePagedRequest::GetRepoId() const { return GeoCoordinationService::GetRepoName(); }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
SpatialEntityWithDetailsSpatialRequest::SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d> projectArea, int classification) :
     m_projectArea(projectArea)
    {   
    m_informationSourceFilter = classification;
    m_filter = "";
    m_validRequestString = false;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void SpatialEntityWithDetailsSpatialRequest::_PrepareHttpRequestStringAndPayload() const
    {
    bool hasFilter = m_filter.length() > 0;
    bool hasOrder = m_order.length() > 0;

    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(GeoCoordinationService::GetWSGProtocol());
    m_httpRequestString.append("/RealityModeling/SpatialEntityWithDetailsView?polygon={points:[");
    m_httpRequestString.append(GetPolygonAsString(m_projectArea, false));

    m_httpRequestString.append("],coordinate_system:'4326'}&");
    if (hasFilter)
        {
        m_httpRequestString.append("$filter=");
        m_httpRequestString.append(m_filter);
        m_httpRequestString.append("&");
        }
    if (hasOrder)
        {
        m_httpRequestString.append(m_order);
        m_httpRequestString.append("&");
        }
    m_httpRequestString.append("$skip=");
    m_httpRequestString += Utf8PrintfString("%u", m_startIndex);
    m_httpRequestString.append("&$top=");
    m_httpRequestString += Utf8PrintfString("%u", m_pageSize);

    m_requestType = HttpRequestType::GET_Request;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void GeoCoordinationServicePagedRequest::SortBy(GeoCoordinationField field, bool ascending)
    {
    Utf8String order = "$orderby=";
    switch (field)
        {
    case GeoCoordinationField::Id:
        order.append("Id");
        break;
    case GeoCoordinationField::Footprint:
        order.append("Footprint");
        break;
    case GeoCoordinationField::Description:
        order.append("Description");
        break;
    case GeoCoordinationField::ContactInformation:
        order.append("ContactInformation");
        break;
    case GeoCoordinationField::Keywords:
        order.append("Keywords");
        break;
    case GeoCoordinationField::Legal:
        order.append("Legal");
        break;
    case GeoCoordinationField::TermsOfUse:
        order.append("TermsOfUse");
        break;
    case GeoCoordinationField::DataSourceType:
        order.append("DataSourceType");
        break;
    case GeoCoordinationField::AccuracyInMeters:
        order.append("AccuracyInMeters");
        break;
    case GeoCoordinationField::Date:
        order.append("Date");
        break;
    case GeoCoordinationField::Classification:
        order.append("Classification");
        break;
    case GeoCoordinationField::FileSize:
        order.append("FileSize");
        break;
    case GeoCoordinationField::Streamed:
        order.append("Streamed");
        break;
    case GeoCoordinationField::SpatialDataSourceId:
        order.append("SpatialDataSourceId");
        break;
    case GeoCoordinationField::ResolutionInMeters:
        order.append("ResolutionInMeters");
        break;
    case GeoCoordinationField::ThumbnailURL:
        order.append("ThumbnailURL");
        break;
    case GeoCoordinationField::DataProvider:
        order.append("DataProvider");
        break;
    case GeoCoordinationField::DataProviderName:
        order.append("DataProviderName");
        break;
    case GeoCoordinationField::Dataset:
        order.append("Dataset");
        break;
    case GeoCoordinationField::Occlusion:
        order.append("Occlusion");
        break;
    case GeoCoordinationField::MetadataURL:
        order.append("MetadataURL");
        break;
    case GeoCoordinationField::RawMetadataURL:
        order.append("RawMetadataURL");
        break;
    case GeoCoordinationField::RawMetadataFormat:
        order.append("RawMetadataFormat");
        break;
    case GeoCoordinationField::SubAPI:
        order.append("SubAPI");
        break;
        }

    if (ascending)
        order.append("+asc");
    else
        order.append("+desc");

    m_order = order;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void SpatialEntityWithDetailsByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(GeoCoordinationService::GetWSGProtocol());
    m_httpRequestString.append("/RealityModeling/SpatialEntityWithDetailsView/");
    m_httpRequestString.append(m_id);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void SpatialEntityByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(GeoCoordinationService::GetWSGProtocol());
    m_httpRequestString.append("/RealityModeling/SpatialEntity/");
    m_httpRequestString.append(m_id);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void SpatialEntityDataSourceByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(GeoCoordinationService::GetWSGProtocol());
    m_httpRequestString.append("/RealityModeling/SpatialDataSource/");
    m_httpRequestString.append(m_id);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void SpatialEntityServerByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(GeoCoordinationService::GetWSGProtocol());
    m_httpRequestString.append("/RealityModeling/Server/");
    m_httpRequestString.append(m_id);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void SpatialEntityMetadataByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(GeoCoordinationService::GetWSGProtocol());
    m_httpRequestString.append("/RealityModeling/Metadata/");
    m_httpRequestString.append(m_id);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
PackagePreparationRequest::PackagePreparationRequest(bvector<GeoPoint2d> projectArea, bvector<Utf8String> listOfSpatialEntities):
    m_projectArea(projectArea), m_listOfSpatialEntities(listOfSpatialEntities)
    {
    m_validRequestString = false;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void PackagePreparationRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(GeoCoordinationService::GetWSGProtocol());
    m_httpRequestString.append("/RealityModeling/PackageRequest");

    m_requestPayload = "{'instance':{'instanceId':null,'className':'PackageRequest','schemaName':'RealityModeling','properties':{'RequestedEntities':[";
    for (Utf8String id : m_listOfSpatialEntities)
        {
        m_requestPayload.append("{ 'Id':'");
        m_requestPayload.append(id);
        m_requestPayload.append("'},");
        }

    //if (containOsmClass())
    /*listAsPostFields.append("],'CoordinateSystem':null,'OSM': true,'Polygon':'[");
    else*/
    m_requestPayload.append("],'CoordinateSystem':null,'OSM': false,'Polygon':'[");

    m_requestPayload.append(GetPolygonAsString(m_projectArea, false));
    m_requestPayload.append("]'}}, 'requestOptions':{'CustomOptions':{'Version':'2', 'Requestor':'GeoCoordinationService', 'RequestorVersion':'1.0' }}}");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void PreparedPackageRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(GeoCoordinationService::GetWSGProtocol());
    m_httpRequestString.append("/RealityModeling/PreparedPackage/");
    m_httpRequestString.append(m_id);
    m_httpRequestString.append("/$file");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
DownloadReportUploadRequest::DownloadReportUploadRequest(Utf8StringCR identifier, BeFileName report)
    : m_downloadReport(report)
    {
    m_validRequestString = false; 
    m_id = identifier; 
    m_requestType = HttpRequestType::PUT_Request;
    
    Utf8String fileFromRoot = report.GetNameUtf8();

    report.GetFileSize(m_fileSize);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void DownloadReportUploadRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(GeoCoordinationService::GetWSGProtocol());
    m_httpRequestString.append("/RealityModeling/DownloadReport/");
    m_httpRequestString.append(m_id);
    m_httpRequestString.append("/$file");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String GeoCoordinationService::s_geoCoordinationServer = "";
Utf8String GeoCoordinationService::s_geoCoordinationWSGProtocol = "2.4";
Utf8String GeoCoordinationService::s_geoCoordinationRepoName = "IndexECPlugin-Server";
Utf8String GeoCoordinationService::s_geoCoordinationSchemaName = "RealityModeling";

bool GeoCoordinationService::s_verifyPeer = false;
Utf8String GeoCoordinationService::s_certificatePath = "";

const Utf8String GeoCoordinationService::s_ImageryKey = "Imagery";
const Utf8String GeoCoordinationService::s_TerrainKey = "Terrain";
const Utf8String GeoCoordinationService::s_ModelKey = "Model";
const Utf8String GeoCoordinationService::s_PinnedKey = "Pinned";

const Utf8String GeoCoordinationService::s_USGSInformationSourceKey = "usgsapi";
const Utf8String GeoCoordinationService::s_PublicIndexInformationSourceKey = "index";
const Utf8String GeoCoordinationService::s_AllInformationSourceKey = "all";

Utf8StringCR GeoCoordinationService::GetServerName() { return s_geoCoordinationServer; }
Utf8StringCR GeoCoordinationService::GetWSGProtocol() { return s_geoCoordinationWSGProtocol; }
Utf8StringCR GeoCoordinationService::GetRepoName() { return s_geoCoordinationRepoName; }
Utf8StringCR GeoCoordinationService::GetSchemaName() { return s_geoCoordinationSchemaName; }
const bool GeoCoordinationService::GetVerifyPeer() { return s_verifyPeer; } //TODO: verify when possible...
Utf8StringCR GeoCoordinationService::GetCertificatePath() { return s_certificatePath; }

static void defaultErrorCallback(Utf8String basicMessage, const RawServerResponse& rawResponse)
    {
    std::cout << basicMessage << std::endl;
    std::cout << rawResponse.body << std::endl;
    }

GeoCoordinationService_ErrorCallBack GeoCoordinationService::s_errorCallback = defaultErrorCallback;


//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
bvector<SpatialEntityPtr> GeoCoordinationService::SpatialEntityRequestBase(const GeoCoordinationServiceRequest* request, RawServerResponse& rawResponse)
    {
    rawResponse = BasicRequest(request);

    bvector<SpatialEntityPtr> entities = bvector<SpatialEntityPtr>();
    if (rawResponse.status != RequestStatus::OK)
        s_errorCallback("SpatialEntityRequest failed with response", rawResponse); 
    else
        RealityConversionTools::JsonToSpatialEntity(rawResponse.body.c_str(), &entities);

    return entities;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
SpatialEntityPtr GeoCoordinationService::Request(const SpatialEntityByIdRequest& request, RawServerResponse& rawResponse)
    {
    return SpatialEntityRequestBase(static_cast<const GeoCoordinationServiceRequest*>(&request), rawResponse)[0];
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
SpatialEntityPtr GeoCoordinationService::Request(const SpatialEntityWithDetailsByIdRequest& request, RawServerResponse& rawResponse)
    {
    return SpatialEntityRequestBase(static_cast<const GeoCoordinationServiceRequest*>(&request), rawResponse)[0];
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
SpatialEntityDataSourcePtr GeoCoordinationService::Request(const SpatialEntityDataSourceByIdRequest& request, RawServerResponse& rawResponse)
    {
    rawResponse = BasicRequest(static_cast<const GeoCoordinationServiceRequest*>(&request));

    bvector<SpatialEntityDataSourcePtr> entities = bvector<SpatialEntityDataSourcePtr>();
    if (rawResponse.status != RequestStatus::OK)
        {
        s_errorCallback("SpatialEntityDataSourceByIdRequest failed with response", rawResponse);
        return nullptr;
        }
    RealityConversionTools::JsonToSpatialEntityDataSource(rawResponse.body.c_str(), &entities);

    return entities[0];
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
SpatialEntityServerPtr GeoCoordinationService::Request(const SpatialEntityServerByIdRequest& request, RawServerResponse& rawResponse)
    {
    rawResponse = BasicRequest(static_cast<const GeoCoordinationServiceRequest*>(&request));

    bvector<SpatialEntityServerPtr> entities = bvector<SpatialEntityServerPtr>();
    if (rawResponse.status != RequestStatus::OK)
        {
        s_errorCallback("SpatialEntityServerByIdRequest failed with response", rawResponse);
        return nullptr;
        }
    RealityConversionTools::JsonToSpatialEntityServer(rawResponse.body.c_str(), &entities);

    return entities[0];
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
SpatialEntityMetadataPtr GeoCoordinationService::Request(const SpatialEntityMetadataByIdRequest& request, RawServerResponse& rawResponse)
    {
    rawResponse = BasicRequest(static_cast<const GeoCoordinationServiceRequest*>(&request));

    bvector<SpatialEntityMetadataPtr> entities = bvector<SpatialEntityMetadataPtr>();
    if (rawResponse.status != RequestStatus::OK)
        {
        s_errorCallback("SpatialEntityServerByIdRequest failed with response", rawResponse);
        return nullptr;
        }
    RealityConversionTools::JsonToSpatialEntityMetadata(rawResponse.body.c_str(), &entities);

    return entities[0];
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
bvector<SpatialEntityPtr> GeoCoordinationService::Request(const SpatialEntityWithDetailsSpatialRequest& request, RawServerResponse& rawResponse)
    {
    rawResponse = BasicPagedRequest(static_cast<const GeoCoordinationServicePagedRequest*>(&request));

    bvector<SpatialEntityPtr> entities = bvector<SpatialEntityPtr>();
    if (rawResponse.status == RequestStatus::BADREQ)
        {
        s_errorCallback("SpatialEntityWithDetailsSpatialRequest failed with response", rawResponse);
        return entities;
        }
    else if ((uint8_t)entities.size() < request.GetPageSize())
        rawResponse.status = RequestStatus::LASTPAGE;

    RealityConversionTools::JsonToSpatialEntity(rawResponse.body.c_str(), &entities);
    return entities;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
Utf8String GeoCoordinationService::Request(const PackagePreparationRequest& request, RawServerResponse& rawResponse)
    {
    rawResponse = BasicRequest(static_cast<const GeoCoordinationServiceRequest*>(&request));

    Utf8String packageId = "";
    Json::Value packageInfos(Json::objectValue);
    Json::Reader::Parse(rawResponse.body, packageInfos);

    if (rawResponse.status == RequestStatus::BADREQ || !packageInfos.isMember("changedInstance"))
        rawResponse.status =  RequestStatus::BADREQ;
    else
        packageId = packageInfos["changedInstance"]["instanceAfterChange"]["instanceId"].asCString();

    return packageId;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void GeoCoordinationService::Request(const PreparedPackageRequest& request, BeFileName filename, RawServerResponse& rawResponse)
    {    
    WSGRequest::GetInstance().SetCertificatePath(GeoCoordinationService::GetCertificatePath());
    BeFile file;
    if(file.Create(filename.GetNameUtf8().c_str(), true) != BeFileStatus::Success)
        {
        s_errorCallback("PreparedPackageRequest failed to create file at provided location", rawResponse);
        return;
        }

    WSGRequest::GetInstance().PerformRequest(request, rawResponse, GeoCoordinationService::GetVerifyPeer(), &file);

    rawResponse.status = RequestStatus::OK;
    if (rawResponse.curlCode != CURLE_OK)
        {
        rawResponse.status = RequestStatus::BADREQ;
        s_errorCallback("Package download failed with response", rawResponse);
        }
    else
        rawResponse.status = RequestStatus::OK;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void GeoCoordinationService::Request(const DownloadReportUploadRequest& request, RawServerResponse& rawResponse)
    {
    BeFile fileStream;
    BeFileStatus status = fileStream.Open(request.GetFileName(), BeFileAccess::Read);
    if(status != BeFileStatus::Success)
        {
        s_errorCallback("DownloadReport File not found", rawResponse);
        return;
        }

    auto curl = WSGRequest::GetInstance().PrepareRequest(request, rawResponse, GeoCoordinationService::GetVerifyPeer(), &fileStream);

    if (rawResponse.curlCode == CURLcode::CURLE_FAILED_INIT)
        {
        s_errorCallback("Curl init failed for DownloadReportUploadRequest", rawResponse);
        return;
        }

    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, CurlReadDataCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &fileStream);

    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, request.GetMessageSize());

    rawResponse.curlCode = (int)curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(rawResponse.responseCode));
    curl_easy_cleanup(curl);

    if (rawResponse.curlCode == CURLE_OK)
        rawResponse.status = RequestStatus::OK;
    else
        {
        rawResponse.status = RequestStatus::BADREQ;
        s_errorCallback("Error Uploading DownloadReport", rawResponse);
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
RawServerResponse GeoCoordinationService::BasicPagedRequest(const GeoCoordinationServicePagedRequest* request)
    {
    RawServerResponse rawResponse = RawServerResponse();
    WSGRequest::GetInstance().SetCertificatePath(GeoCoordinationService::GetCertificatePath());
    WSGRequest::GetInstance().PerformRequest(*request, rawResponse, GeoCoordinationService::GetVerifyPeer());

    Json::Value instances(Json::objectValue);
    if ((rawResponse.status != CURLE_OK) || !Json::Reader::Parse(rawResponse.body, instances) || instances.isMember("errorMessage") || !instances.isMember("instances"))
        rawResponse.status =  RequestStatus::BADREQ;
    else
        {
        request->AdvancePage();
        rawResponse.status = RequestStatus::OK;
        }
    return rawResponse;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
RawServerResponse GeoCoordinationService::BasicRequest(const GeoCoordinationServiceRequest* request)
    {
    RawServerResponse rawResponse = RawServerResponse();
    WSGRequest::GetInstance().SetCertificatePath(GeoCoordinationService::GetCertificatePath());
    WSGRequest::GetInstance().PerformRequest(*request, rawResponse, GeoCoordinationService::GetVerifyPeer());

    Json::Value instances(Json::objectValue);
    if ((rawResponse.status != CURLE_OK) || !Json::Reader::Parse(rawResponse.body, instances) || instances.isMember("errorMessage") || !instances.isMember("instances"))
        rawResponse.status = RequestStatus::BADREQ;
    else
        rawResponse.status = RequestStatus::OK;

    return rawResponse;
    }