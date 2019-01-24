/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformTools/GeoCoordinationService.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/BeFileListIterator.h>
#include <Bentley/bset.h>
#include <Bentley/DateTime.h>

#include <RealityPlatformTools/GeoCoordinationService.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatformTools/RealityConversionTools.h>

#include <iostream>
#include <Bentley/Base64Utilities.h>

#define MAX_NB_CONNECTIONS          10
USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//! converts bvector of geopoints to a Polygon string that can be passed to GCS
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
    m_httpRequestString.append(Utf8PrintfString("v%s/Repositories/%s/%s/SpatialEntityWithDetailsView?polygon={points:[", GeoCoordinationService::GetWSGProtocol().c_str(), GeoCoordinationService::GetRepoName().c_str(), GeoCoordinationService::GetSchemaName().c_str()));
    m_httpRequestString.append(GetPolygonAsString(m_projectArea, false));

    m_httpRequestString.append("],coordinate_system:'4326'}&");
    if (hasFilter || m_informationSourceFilter > 0)
        {
        m_httpRequestString.append("$filter=");
        if (hasFilter)
            {
            m_httpRequestString.append(m_filter);
            m_httpRequestString.append("&");
            }
        if (m_informationSourceFilter > 0)
            {
            m_httpRequestString.append("Classification+in+[");
            if (m_informationSourceFilter & RealityDataBase::Classification::IMAGERY)
                m_httpRequestString.append(Utf8PrintfString("'%s',", GeoCoordinationService::s_ImageryKey.c_str()));
            if (m_informationSourceFilter & RealityDataBase::Classification::TERRAIN)
                m_httpRequestString.append(Utf8PrintfString("'%s',", GeoCoordinationService::s_TerrainKey.c_str()));
            if (m_informationSourceFilter & RealityDataBase::Classification::MODEL)
                m_httpRequestString.append(Utf8PrintfString("'%s',", GeoCoordinationService::s_ModelKey.c_str()));
            if (m_informationSourceFilter & RealityDataBase::Classification::PINNED)
                m_httpRequestString.append(Utf8PrintfString("'%s',", GeoCoordinationService::s_PinnedKey.c_str()));
            m_httpRequestString = m_httpRequestString.substr(0, m_httpRequestString.size() - 1); //remove comma
            m_httpRequestString.append("]&");
            }
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
            case GeoCoordinationField::Name:
                order.append("Name");
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

void BingKeyRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append(Utf8PrintfString("v%s/Repositories/ContextKeyService--Server/ContextKeyServiceSchema/BingApiKey?$filter=productId+eq+%s", GeoCoordinationService::GetWSGProtocol().c_str(), m_id.c_str()));
    
    m_requestType = HttpRequestType::GET_Request;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void SpatialEntityWithDetailsByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append(Utf8PrintfString("v%s/Repositories/%s/%s/SpatialEntityWithDetailsView/", GeoCoordinationService::GetWSGProtocol().c_str(), GeoCoordinationService::GetRepoName().c_str(), GeoCoordinationService::GetSchemaName().c_str()));
    m_httpRequestString.append(m_encodedId);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void SpatialEntityByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append(Utf8PrintfString("v%s/Repositories/%s/%s/SpatialEntity/", GeoCoordinationService::GetWSGProtocol().c_str(), GeoCoordinationService::GetRepoName().c_str(), GeoCoordinationService::GetSchemaName().c_str()));
    m_httpRequestString.append(m_encodedId);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void SpatialEntityDataSourceByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append(Utf8PrintfString("v%s/Repositories/%s/%s/SpatialDataSource/", GeoCoordinationService::GetWSGProtocol().c_str(), GeoCoordinationService::GetRepoName().c_str(), GeoCoordinationService::GetSchemaName().c_str()));
    m_httpRequestString.append(m_encodedId);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void SpatialEntityServerByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append(Utf8PrintfString("v%s/Repositories/%s/%s/Server/", GeoCoordinationService::GetWSGProtocol().c_str(), GeoCoordinationService::GetRepoName().c_str(), GeoCoordinationService::GetSchemaName().c_str()));
    m_httpRequestString.append(m_encodedId);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void SpatialEntityMetadataByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append(Utf8PrintfString("v%s/Repositories/%s/%s/Metadata/", GeoCoordinationService::GetWSGProtocol().c_str(), GeoCoordinationService::GetRepoName().c_str(), GeoCoordinationService::GetSchemaName().c_str()));
    m_httpRequestString.append(m_encodedId);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
PackagePreparationRequest::PackagePreparationRequest(bvector<GeoPoint2d> projectArea, bvector<Utf8String> listOfSpatialEntities) :
    m_projectArea(projectArea), m_listOfSpatialEntities(listOfSpatialEntities)
    {
    m_validRequestString = false;
    m_requestType = HttpRequestType::POST_Request;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void PackagePreparationRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append(Utf8PrintfString("v%s/Repositories/%s/%s/PackageRequest/", GeoCoordinationService::GetWSGProtocol().c_str(), GeoCoordinationService::GetRepoName().c_str(), GeoCoordinationService::GetSchemaName().c_str()));

    m_requestPayload = R"({"instance":{"instanceId":null,"className":"PackageRequest","schemaName":"RealityModeling","properties":{"RequestedEntities":[)";
    for (Utf8String id : m_listOfSpatialEntities)
        {
        m_requestPayload.append(R"({ "Id":")");
        m_requestPayload.append(id);
        m_requestPayload.append(R"("},)");
        }
    
    // Remove comma from the last entities
    if(m_listOfSpatialEntities.size() > 0 )
        {
        m_requestPayload = m_requestPayload.substr(0, m_requestPayload.size() - 1);
        }

    //if (containOsmClass())
    /*listAsPostFields.append("],'CoordinateSystem':null,'OSM': true,'Polygon':'[");
    else*/
    m_requestPayload.append(R"(],"CoordinateSystem":null,"OSM": false,"Polygon":"[)");

    m_requestPayload.append(GetPolygonAsString(m_projectArea, false));
    m_requestPayload.append(R"(]"}}, "requestOptions":{"CustomOptions":{"Version":"2", "Requestor":"GeoCoordinationService", "RequestorVersion":"1.0" }}})");

    m_requestHeader.clear();
    m_requestHeader.push_back("Content-Type: application/json");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void PreparedPackageRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append(Utf8PrintfString("v%s/Repositories/%s/%s/PreparedPackage/", GeoCoordinationService::GetWSGProtocol().c_str(), GeoCoordinationService::GetRepoName().c_str(), GeoCoordinationService::GetSchemaName().c_str()));
    m_httpRequestString.append(m_encodedId);
    m_httpRequestString.append("/$file");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              09/2018
//=====================================================================================
void LastPackageRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append(Utf8PrintfString("v%s/Repositories/%s/%s/PreparedPackage?last=true", GeoCoordinationService::GetWSGProtocol().c_str(), GeoCoordinationService::GetRepoName().c_str(), GeoCoordinationService::GetSchemaName().c_str()));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void PreparedPackagesRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append(Utf8PrintfString("v%s/Repositories/%s/%s/PreparedPackage", GeoCoordinationService::GetWSGProtocol().c_str(), GeoCoordinationService::GetRepoName().c_str(), GeoCoordinationService::GetSchemaName().c_str()));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
DownloadReportUploadRequest::DownloadReportUploadRequest(Utf8StringCR guid, Utf8StringCR identifier, BeFileName report)
    : m_downloadReport(report), m_guid(guid)
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
    m_httpRequestString.append(Utf8PrintfString("v%s/Repositories/%s/%s/DownloadReport/", GeoCoordinationService::GetWSGProtocol().c_str(), GeoCoordinationService::GetRepoName().c_str(), GeoCoordinationService::GetSchemaName().c_str()));
    m_httpRequestString.append(m_guid);
    m_httpRequestString.append("/$file");

    m_requestHeader.clear();
    m_requestHeader.push_back(Utf8PrintfString("Content-Disposition : attachment; filename=\"%s\"", m_id.c_str()));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String GeoCoordinationService::s_geoCoordinationServer = "";
Utf8String GeoCoordinationService::s_geoCoordinationWSGProtocol = "2.4";
Utf8String GeoCoordinationService::s_geoCoordinationRepoName = "IndexECPlugin--Server";
Utf8String GeoCoordinationService::s_geoCoordinationRepoNameWProjectId = "";
Utf8String GeoCoordinationService::s_geoCoordinationSchemaName = "RealityModeling";
Utf8String GeoCoordinationService::s_geoCoordinationProjectId = "";

bool GeoCoordinationService::s_verifyPeer = false;
Utf8String GeoCoordinationService::s_certificatePath = "";

const Utf8String GeoCoordinationService::s_ImageryKey = "Imagery";
const Utf8String GeoCoordinationService::s_TerrainKey = "Terrain";
const Utf8String GeoCoordinationService::s_ModelKey = "Model";
const Utf8String GeoCoordinationService::s_PinnedKey = "Pinned";

const Utf8String GeoCoordinationService::s_USGSInformationSourceKey = "usgsapi";
const Utf8String GeoCoordinationService::s_PublicIndexInformationSourceKey = "index";
const Utf8String GeoCoordinationService::s_AllInformationSourceKey = "all";

GeoCoordinationService_RequestCallback GeoCoordinationService::s_requestCallback = nullptr;

Utf8StringCR GeoCoordinationService::GetServerName() { return s_geoCoordinationServer; }
Utf8StringCR GeoCoordinationService::GetWSGProtocol() { return s_geoCoordinationWSGProtocol; }
Utf8StringCR GeoCoordinationService::GetRepoName() 
    { 
    //if (s_geoCoordinationProjectId.empty())
        return s_geoCoordinationRepoName;
    /*else if (!RealityPlatformTopazUtilities::ContainsI(s_geoCoordinationRepoNameWProjectId, s_geoCoordinationProjectId.c_str()))
        {
        s_geoCoordinationRepoNameWProjectId = s_geoCoordinationRepoName;
        s_geoCoordinationRepoNameWProjectId.ReplaceAll("Server", s_geoCoordinationProjectId.c_str());
        }

    return s_geoCoordinationRepoNameWProjectId;*/
    }
Utf8StringCR GeoCoordinationService::GetSchemaName() { return s_geoCoordinationSchemaName; }
const bool GeoCoordinationService::GetVerifyPeer() { return s_verifyPeer; } //TODO: verify when possible...
Utf8StringCR GeoCoordinationService::GetCertificatePath() { return s_certificatePath; }
Utf8StringCR GeoCoordinationService::GetProjectId() { return s_geoCoordinationProjectId; }
void GeoCoordinationService::SetProjectId(Utf8StringCR projectId) { s_geoCoordinationProjectId = projectId; }

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
    auto entities = SpatialEntityRequestBase(static_cast<const GeoCoordinationServiceRequest*>(&request), rawResponse);
    if (entities.size() > 0)
        return entities[0];
    else
        return nullptr;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
SpatialEntityPtr GeoCoordinationService::Request(const SpatialEntityWithDetailsByIdRequest& request, RawServerResponse& rawResponse)
    {
    auto entities = SpatialEntityRequestBase(static_cast<const GeoCoordinationServiceRequest*>(&request), rawResponse);
    if (entities.size() > 0)
        return entities[0];
    else
        return nullptr;
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
        s_errorCallback("SpatialEntityMetadataByIdRequest failed with response", rawResponse);
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
    RealityConversionTools::JsonToSpatialEntity(rawResponse.body.c_str(), &entities);

    if (rawResponse.status == RequestStatus::BADREQ)
        {
        s_errorCallback("SpatialEntityWithDetailsSpatialRequest failed with response", rawResponse);
        return entities;
        }
    else if ((uint8_t) entities.size() < request.GetPageSize())
        rawResponse.status = RequestStatus::LASTPAGE;


    return entities;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
Utf8String GeoCoordinationService::Request(const PackagePreparationRequest& request, RawServerResponse& rawResponse)
    {
    rawResponse = BasicRequest(static_cast<const GeoCoordinationServiceRequest*>(&request), "changedInstance");

    Utf8String packageId = "";
    Json::Value packageInfos(Json::objectValue);
    Json::Reader::Parse(rawResponse.body, packageInfos);

    if (rawResponse.status == RequestStatus::BADREQ || !packageInfos.isMember("changedInstance"))
        rawResponse.status = RequestStatus::BADREQ;
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
    if (file.Create(filename.GetNameUtf8().c_str(), true) != BeFileStatus::Success)
        {
        s_errorCallback("PreparedPackageRequest failed to create file at provided location", rawResponse);
        return;
        }

    WSGRequest::GetInstance().PerformRequest(request, rawResponse, GeoCoordinationService::GetVerifyPeer(), &file);

    rawResponse.status = RequestStatus::OK;
    if (rawResponse.toolCode != 0)
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
Utf8String GeoCoordinationService::Request(const LastPackageRequest& request, RawServerResponse& rawResponse)
    {
    rawResponse = BasicRequest(static_cast<const GeoCoordinationServiceRequest*>(&request));

    Utf8String lastName = "";
    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(rawResponse.body, instances);

    if (rawResponse.status == RequestStatus::BADREQ || !instances["instances"][0].isMember("properties") || !instances["instances"][0]["properties"].isMember("Name"))
        rawResponse.status = RequestStatus::BADREQ;
    else
        lastName = instances["instances"][0]["properties"]["Name"].asCString();

    return lastName;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void GeoCoordinationService::Request(const PreparedPackagesRequest& request, RawServerResponse& rawResponse, bvector<Utf8String>& packageIds)
    {
    rawResponse = BasicRequest(static_cast<const GeoCoordinationServiceRequest*>(&request));
    
    Utf8String lastName = "";
    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(rawResponse.body, instances);

    if (rawResponse.status == RequestStatus::BADREQ || !instances["instances"][0].isMember("properties") || !instances["instances"][0]["properties"].isMember("Name") || !instances["instances"][0]["properties"].isMember("CreationTime"))
        rawResponse.status = RequestStatus::BADREQ;
    else
        {
        rawResponse.status = RequestStatus::OK;
        unsigned int size = instances["instances"].size();
        for(unsigned int count = 0; count < size; count++)
            {
            packageIds.push_back(instances["instances"]["properties"]["Name"].asCString());
            }
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void GeoCoordinationService::Request(const BingKeyRequest& request, RawServerResponse& rawResponse, Utf8StringR key, Utf8StringR expirationDate)
    {
    rawResponse = BasicRequest(static_cast<const GeoCoordinationServiceRequest*>(&request));

    Utf8String packageId = "";
    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(rawResponse.body, instances);

    if (rawResponse.status == RequestStatus::BADREQ || !instances["instances"][0].isMember("properties") || !instances["instances"][0]["properties"].isMember("key") || !instances["instances"][0]["properties"].isMember("expirationDate"))
        rawResponse.status = RequestStatus::BADREQ;
    else
        {
        key = instances["instances"][0]["properties"]["key"].asCString();
        expirationDate = instances["instances"][0]["properties"]["expirationDate"].asCString();
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
RawServerResponse GeoCoordinationService::BasicPagedRequest(const GeoCoordinationServicePagedRequest* request, Utf8StringCR keyword)
    {
    RawServerResponse rawResponse = RawServerResponse();
    WSGRequest::GetInstance().SetCertificatePath(GeoCoordinationService::GetCertificatePath());
    WSGRequest::GetInstance().PerformRequest(*request, rawResponse, GeoCoordinationService::GetVerifyPeer());

    Json::Value instances(Json::objectValue);
    if (rawResponse.status != 0)
        {
        s_errorCallback("Tool error", rawResponse);
        rawResponse.status = RequestStatus::BADREQ;
        }
    else if(!Json::Reader::Parse(rawResponse.body, instances))
        {
        s_errorCallback("Can't parse the JSON", rawResponse);
        rawResponse.status = RequestStatus::BADREQ;
        }
    else if (instances.isMember("errorMessage"))
        {
        s_errorCallback("Error message in the JSON", rawResponse);
        rawResponse.status = RequestStatus::BADREQ;
        }
    else if (!instances.isMember(keyword))
        {
        s_errorCallback("Keyword not found in the JSON", rawResponse);
        rawResponse.status = RequestStatus::BADREQ;
        }
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
RawServerResponse GeoCoordinationService::BasicRequest(const GeoCoordinationServiceRequest* request, Utf8StringCR keyword)
    {
    RawServerResponse rawResponse = RawServerResponse();
    WSGRequest::GetInstance().SetCertificatePath(GeoCoordinationService::GetCertificatePath());
    WSGRequest::GetInstance().PerformRequest(*request, rawResponse, GeoCoordinationService::GetVerifyPeer());

    Json::Value instances(Json::objectValue);
    if (rawResponse.status != 0)
        {
        s_errorCallback("Tool error", rawResponse);
        rawResponse.status = RequestStatus::BADREQ;
        }
    else if(!Json::Reader::Parse(rawResponse.body, instances))
        {
        s_errorCallback("Can't parse the JSON", rawResponse);
        rawResponse.status = RequestStatus::BADREQ;
        }
    else if (instances.isMember("errorMessage"))
        {
        s_errorCallback("Error message in the JSON", rawResponse);
        rawResponse.status = RequestStatus::BADREQ;
        }
    else if (!instances.isMember(keyword))
        {
        s_errorCallback("Keyword not found in the JSON", rawResponse);
        rawResponse.status = RequestStatus::BADREQ;
        }
    else
        rawResponse.status = RequestStatus::OK;

    return rawResponse;
    }