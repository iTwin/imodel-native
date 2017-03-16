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



BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
static Utf8String GetPolygonAsString(bvector<GeoPoint2D> area, bool urlEncode) const
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
SpatialEntityWithDetailsSpatialRequest::SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2D> projectArea, int classification) :
    m_informationSourcefilter(false), m_projectArea(projectArea), m_informationSourceFilter(classification), m_filter("")
    {   
    m_validRequest = false;
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
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
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
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
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
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
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
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
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
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
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
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/RealityModeling/Metadata/");
    m_httpRequestString.append(m_id);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
PackagePreparationRequest::PackagePreparationRequest(bvector<GeoPoint2D> projectArea, bvector<Utf8String> listOfSpatialEntities):
    m_projectArea(projectArea), m_listOfSpatialEntities(listOfSpatialEntities)
    {
    m_validRequest = false;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void PackagePreparationRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
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
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/RealityModeling/PreparedPackage/");
    m_httpRequestString.append(m_id);
    m_httpRequestString.append("/$file");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void DownloadReportUploadRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = GeoCoordinationService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/v");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/RealityModeling/DownloadReport/");
    m_httpRequestString.append(m_id);
    m_httpRequestString.append("/$file");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String GeoCoordinationService::s_realityDataServer = "https://connect-contextservices.bentley.com/";
Utf8String GeoCoordinationService::s_realityDataWSGProtocol = "2.4";
Utf8String GeoCoordinationService::s_realityDataRepoName = "IndexECPlugin-Server";
Utf8String GeoCoordinationService::s_realityDataSchemaName = "RealityModeling";

int GeoCoordinationService::s_verifyPeer = 0;
Utf8String GeoCoordinationService::s_realityDataCertificatePath = "";

const Utf8String GeoCoordinationService::s_ImageryKey = "Imagery";
const Utf8String GeoCoordinationService::s_TerrainKey = "Terrain";
const Utf8String GeoCoordinationService::s_ModelKey = "Model";
const Utf8String GeoCoordinationService::s_PinnedKey = "Pinned";

Utf8StringCR GeoCoordinationService::GetServerName() { return s_realityDataServer; }
Utf8StringCR GeoCoordinationService::GetWSGProtocol() { return s_realityDataWSGProtocol; }
Utf8StringCR GeoCoordinationService::GetRepoName() { return s_realityDataRepoName; }
Utf8StringCR GeoCoordinationService::GetSchemaName() { return s_realityDataSchemaName; }
const int GeoCoordinationService::GetVerifyPeer() { return s_verifyPeer; } //TODO: verify when possible...
Utf8StringCR GeoCoordinationService::GetCertificatePath() { return s_realityDataCertificatePath; }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
bvector<SpatialEntityPtr> GeoCoordinationService::SpatialEntityRequestBase(const GeoCoordinationServiceRequest* request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = RequestToJSON((&request), jsonString);

    bvector<SpatialEntityPtr> entities = bvector<SpatialEntityPtr>();
    if (status != RequestStatus::SUCCESS)
        {
        std::cout << "SpatialEntityRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        return nullptr;
        }
    RealityConversionTools::JsonToSpatialEntity(jsonString.c_str(), &entities);
    return entities;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
SpatialEntityPtr GeoCoordinationService::Request(const SpatialEntityByIdRequest& request, RequestStatus& status)
    {
    return SpatialEntityRequestBase(static_cast<const GeoCoordinationServiceRequest*>(&request), status)[0];
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
SpatialEntityPtr GeoCoordinationService::Request(const SpatialEntityWithDetailsByIdRequest& request, RequestStatus& status)
    {
    return SpatialEntityRequestBase(static_cast<const GeoCoordinationServiceRequest*>(&request), status)[0];
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
SpatialEntityDataSourcePtr GeoCoordinationService::Request(const SpatialEntityDataSourceById& request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = RequestToJSON(static_cast<const GeoCoordinationServiceRequest*>(&request), jsonString);

    bvector<SpatialEntityDataSourcePtr> entities = bvector<SpatialEntityDataSourcePtr>();
    if (status != RequestStatus::SUCCESS)
        {
        std::cout << "SpatialEntityDataSourceByIdRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        return nullptr;
        }
    RealityConversionTools::JsonToSpatialEntityDataSource(jsonString.c_str(), &entities);

    return entities[0];
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
SpatialEntityServerPtr GeoCoordinationService::Request(const SpatialEntityServerByIdRequest& request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = RequestToJSON(static_cast<const GeoCoordinationServiceRequest*>(&request), jsonString);

    bvector<SpatialEntityServerPtr> entities = bvector<SpatialEntityServerPtr>();
    if (status != RequestStatus::SUCCESS)
        {
        std::cout << "SpatialEntityServerByIdRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        return nullptr;
        }
    RealityConversionTools::JsonToSpatialEntityServer(jsonString.c_str(), &entities);

    return entities[0];
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
SpatialEntityMetadataPtr GeoCoordinationService::Request(const SpatialEntityMetadataByIdRequest& request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = RequestToJSON(static_cast<const GeoCoordinationServiceRequest*>(&request), jsonString);

    bvector<SpatialEntityMetadataPtr> entities = bvector<SpatialEntityMetadataPtr>();
    if (status != RequestStatus::SUCCESS)
        {
        std::cout << "SpatialEntityServerByIdRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        return nullptr;
        }
    RealityConversionTools::JsonToSpatialEntityMetadata(jsonString.c_str(), &entities);

    return entities[0];
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
bvector<SpatialEntityPtr> GeoCoordinationService::Request(const SpatialEntityWithDetailsSpatialRequest& request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = PagedRequestToJSON((&request), jsonString);

    bvector<SpatialEntityPtr> entities = bvector<SpatialEntityPtr>();
    if (status == RequestStatus::ERROR)
    {
        std::cout << "SpatialEntityWithDetailsSpatialRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        return nullptr;
    }
    else if ((uint8_t)entities.size() < request.GetPageSize())
        status = RequestStatus::NOMOREPAGES;

    RealityConversionTools::JsonToSpatialEntity(jsonString.c_str(), &entities);
    return entities;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
Utf8String GeoCoordinationService::Request(const PackagePreparationRequest& request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = RequestToJSON(static_cast<const GeoCoordinationServiceRequest*>(&request), jsonString);

    Utf8String packageInfos;
    Utf8String packageId = "";
    Json::Value packageInfos(Json::objectValue);
    Json::Reader::Parse(jsonString, packageInfos);

    if (status == RequestStatus::ERROR || !packageInfos.isMember("changedInstance"))
        status =  RequestStatus::ERROR;
    else
        packageId = packageInfos["changedInstance"]["instanceAfterChange"]["instanceId"].asCString();

    return packageId;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void GeoCoordinationService::Request(const PreparedPackageRequest& request, BeFileName filename, RequestStatus& status)
    {    
    Utf8String jsonString;
    
    int stat = RequestType::Body;
    WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());
    char outfile[1024] = "";
    strcpy(outfile, fileName.GetNameUtf8().c_str());
    FILE* file = fopen(outfile, "wb");

    Utf8String resultString = WSGRequest::GetInstance().PerformRequest(request, stat, RealityDataService::GetVerifyPeer(), file);

    status = RequestStatus::SUCCESS;
    if (stat != CURLE_OK)
        {
        status = RequestStatus::ERROR;
        std::cout << "Package download failed with response" << std::endl;
        std::cout << resultString << std::endl;
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void Request(const DownloadReportUploadRequest& request, RequestStatus& status)
    {

    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
RequestStatus GeoCoordinationService::PagedRequestToJSON(RealityDataPagedRequest* const request, Utf8StringR jsonResponse)
{
    int status = RequestType::Body;
    WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());
    jsonResponse = WSGRequest::GetInstance().PerformRequest(*request, status, RealityDataService::GetVerifyPeer());

    Json::Value instances(Json::objectValue);
    if ((status != CURLE_OK) || !Json::Reader::Parse(jsonResponse, instances) || instances.isMember("errorMessage") || !instances.isMember("instances"))
        return RequestStatus::ERROR;

    request->AdvancePage();

    return RequestStatus::SUCCESS;
}

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
RequestStatus GeoCoordinationService::RequestToJSON(GeoCoordinationServiceRequest* const request, Utf8StringR jsonResponse)
{
    int status = RequestType::Body;
    WSGRequest::GetInstance().SetCertificatePath(GeoCoordinationService::GetCertificatePath());
    jsonResponse = WSGRequest::GetInstance().PerformRequest(*request, status, GeoCoordinationService::GetVerifyPeer());

    Json::Value instances(Json::objectValue);
    if ((status != CURLE_OK) || !Json::Reader::Parse(jsonResponse, instances) || instances.isMember("errorMessage") || !instances.isMember("instances"))
        return RequestStatus::ERROR;

    return RequestStatus::SUCCESS;
}