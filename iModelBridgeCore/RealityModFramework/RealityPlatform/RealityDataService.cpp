/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityDataService.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <RealityPlatform/RealityDataService.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityConversionTools.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM


Utf8StringCR RealityDataUrl::GetServerName() const { return RealityDataService::GetServer(); }

Utf8StringCR RealityDataUrl::GetVersion() const { return RealityDataService::GetWSGProtocol(); }

//Utf8StringCR RealityDataUrl::GetPluginName() const { return ""; }

Utf8StringCR RealityDataUrl::GetSchema() const { return RealityDataService::GetSchemaName(); }

//Utf8StringCR RealityDataUrl::GetClassName() const { return ""; }

Utf8StringCR RealityDataUrl::GetRepoId() const { return RealityDataService::GetName(); }


void RealityDataByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServer();
    WSGURL::_PrepareHttpRequestStringAndPayload(); 
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetName());
    m_httpRequestString.append("/S3MX/RealityData/");
    m_httpRequestString.append(m_id);
    }

void RealityDataProjectRelationshipByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServer();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetName());
    m_httpRequestString.append("/S3MX/RealityDataProjectRelationship?$filter=ProjectId+eq+'");
    m_httpRequestString.append(m_id);
    m_httpRequestString.append("'");
    }

void RealityDataFolderByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServer();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetName());
    m_httpRequestString.append("/S3MX/Folder/");
    m_httpRequestString.append(m_id);
    }

void RealityDataDocumentByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServer();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetName());
    m_httpRequestString.append("/S3MX/Document/");
    m_httpRequestString.append(m_id);
    }

void RealityDataDocumentContentByIdRequest::ChangeInstanceId(Utf8String instanceId)
    {
    m_id = instanceId;
    m_validRequestString = false;
    }

Utf8String RealityDataDocumentContentByIdRequest::GetAzureRedirectionRequestUrl()
    {
    //https://s3mxcloudservice.cloudapp.net/v2.4/Repositories/S3MXECPlugin--Server/S3MX/Document/ab9c6aa6-91ad-424b-935c-28a3c396a041~2FGraz~2FScene~2FProduction_Graz_3MX.3mx/FileAccess.FileAccessKey?$filter=Permissions+eq+'Read'&api.singleurlperinstance=true 
    Utf8String url = "https://";
    //url.append(m_serverName);
    url.append(RealityDataService::GetServer());
    url.append("/");
    url.append(RealityDataService::GetWSGProtocol());
    url.append("/Repositories/");
    url.append(RealityDataService::GetName());
    url.append("/S3MX/Document/");
    url.append(m_id);
    url.append("/FileAccess.FileAccessKey?$filter=Permissions+eq+'Read'&api.singleurlperinstance=true ");
    m_allowAzureRedirection = false;

    WSGURL wsgurl = WSGURL(url);

    int status = 0;
    Utf8String jsonString = WSGRequest::GetInstance().PerformRequest(wsgurl, status);

    Utf8String AzureUrl = "";

    Json::Value instances(Json::objectValue);
    if ((status != CURLE_OK) || (!Json::Reader::Parse(jsonString, instances) || (!instances.isMember("errorMessage") && !instances.isMember("instances")) || instances.isMember("errorMessage")))
        return AzureUrl;

    for (auto instance : instances["instances"])
    {
        if (instance.isMember("properties") && instance["properties"].isMember("Url"))
            AzureUrl = instance["properties"]["Url"].asCString();
    }

    if(AzureUrl.length() > 0)
        m_allowAzureRedirection = true;

    return AzureUrl;
    }

void RealityDataDocumentContentByIdRequest::SetAzureRedirectionUrlToContainer(Utf8String azureContainerUrl)
    {
    m_AzureRedirectionURL = azureContainerUrl;
    m_AzureRedirected = true;
    }

bool RealityDataDocumentContentByIdRequest::IsAzureBlobRedirected() { return m_AzureRedirected; }

void RealityDataDocumentContentByIdRequest::SetAzureRedirectionPossible(bool possible) { m_allowAzureRedirection = possible; }

bool RealityDataDocumentContentByIdRequest::IsAzureRedirectionPossible() { return m_allowAzureRedirection; }

void RealityDataDocumentContentByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    if(m_allowAzureRedirection && m_AzureRedirected)
        {
        m_httpRequestString = m_AzureRedirectionURL;

        m_validRequestString = true;
        }
    else
        {
        WSGURL::_PrepareHttpRequestStringAndPayload();
        m_httpRequestString.append("/");
        m_httpRequestString.append(m_version);
        m_httpRequestString.append("/Repositories/");
        m_httpRequestString.append(m_repoId);
        m_httpRequestString.append("/");
        m_httpRequestString.append(m_schema);
        m_httpRequestString.append("/");
        m_httpRequestString.append(m_className);
        m_httpRequestString.append("/");
        m_httpRequestString.append(m_id);
        }
    }

Utf8String RealityDataFilterCreator::FilterByClassification(int classification)
    { 
    //$filter=
    Utf8String filter = "Class+eq+";
    Utf8P buf = new Utf8Char();
    BeStringUtilities::FormatUInt64(buf, classification);
    filter.append(buf);
    return filter;
    }

Utf8String RealityDataFilterCreator::FilterBySize(double minSize, double maxSize)
    {
    //$filter=
    Utf8String filter = "Size+ge+";
    char buf[32];
    sprintf(buf, "%f", minSize);
    filter.append(buf);
    filter.append("and+Size+le+");
    sprintf(buf, "%f", maxSize);
    filter.append(buf);
    return filter;
    }

Utf8String RealityDataFilterCreator::FilterSpatial(bvector<GeoPoint2d> area, uint64_t coordSys)
    {   
    //S3MX/RealityData?polygon={"points":[[10.6787109375,46.6343507029],[30.3662109375,46.6343507029],[30.3662109375,60.5761747263]
    //,[10.6787109375,60.5761747263],[10.6787109375,46.6343507029]],"coordinate_system":"4326"} 
    Utf8String filter = "polygon={\"points\":[";
    char buf[32];
    for(int i = 0; i < area.size(); i++)
        {
        filter.append("[");
        sprintf(buf, "%f", area[i].longitude);
        filter.append(buf);
        filter.append(",");
        sprintf(buf, "%f", area[i].latitude);
        filter.append(buf);
        filter.append("]");
        filter.append(",");
        }   
    filter.append("["); 
    sprintf(buf, "%f", area[0].longitude);//close the box
    filter.append(buf);
    filter.append(",");
    sprintf(buf, "%f", area[0].latitude);
    filter.append(buf);
    filter.append("]], \"coordinate_system\":\"");
    BeStringUtilities::FormatUInt64(buf, coordSys);
    filter.append(buf);
    filter.append("\"}");

    return filter;
    }

Utf8String RealityDataFilterCreator::FilterByOwner(Utf8String owner)
    {
    Utf8String filter = "DocumentOwners-forward-User.*";
    filter.append(owner);
    return filter;
    }

Utf8String RealityDataFilterCreator::FilterByCreationDate(DateTime minDate, DateTime maxDate)
    {
    /*m_serverName = RealityDataService::GetServer();
    WSGUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetName());
    m_httpRequestString.append("/S3MX/RealityData?$filter=Enterprise+eq+'");
    m_httpRequestString.append(m_id);
    m_httpRequestString.append("'&$skip=");
    Utf8P buf = new Utf8Char();
    BeStringUtilities::FormatUInt64(buf, m_startIndex);
    m_httpRequestString.append(buf);
    m_httpRequestString.append("&$top=");
    BeStringUtilities::FormatUInt64(buf, m_pageSize);
    m_httpRequestString.append(buf);*/
    return "TODO";
    }

Utf8String RealityDataFilterCreator::FilterByModificationDate(DateTime minDate, DateTime maxDate)
    {
    return "TODO";
    }

Utf8String RealityDataFilterCreator::FilterPublic(bool isPublic)
    {
    Utf8String filter = "PublicAccess+eq+";
    if(isPublic)
        filter.append("true");
    else
        filter.append("false");
    return filter;
    }

Utf8String RealityDataFilterCreator::FilterByResolution(double resMin, double resMax, bool filterOutUnspecified)
    {
    return "TODO";
    }

Utf8String RealityDataFilterCreator::FilterByAccuracy(double accuracyMin, double accuracyMax, bool filterOutUnspecified)
    {
    return "TODO";
    }

Utf8String RealityDataFilterCreator::FilterByType(Utf8String types)
    {
    Utf8String filter = "Type+in+{";
    filter.append(types);
    filter.append("}");
    return filter;
    }

Utf8String RealityDataFilterCreator::FilterByDataset(Utf8String dataset)
    {
    return "TODO";
    }


Utf8String RealityDataFilterCreator::GroupFilters(bvector<Utf8String> filters)
    {
    Utf8String filter = "(";
    filter.append(filters[0]);
    for(int i = 1; i < filters.size(); i++)
        {
        filter.append("&");
        filter.append(filters[i]);
        }

    filter.append(")");
    return filter;
    }

Utf8String RealityDataFilterCreator::GroupAlternativeFilters(bvector<Utf8String> filters)
{
    Utf8String filter = "(";
    filter.append(filters[0]);
    for (int i = 1; i < filters.size(); i++)
    {
        filter.append("|");
        filter.append(filters[i]);
    }

    filter.append(")");
    return filter;
}


Utf8StringCR RealityDataPagedRequest::GetServerName() const { return RealityDataService::GetServer(); }
Utf8StringCR RealityDataPagedRequest::GetVersion() const { return RealityDataService::GetWSGProtocol(); }
//Utf8StringCR RealityDataPagedRequest::GetPluginName() const { return ""; }
Utf8StringCR RealityDataPagedRequest::GetSchema() const { return RealityDataService::GetSchemaName(); }
//Utf8StringCR RealityDataPagedRequest::GetClassName() const { return ""; }
Utf8StringCR RealityDataPagedRequest::GetRepoId() const { return RealityDataService::GetName(); }

void RealityDataPagedRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServer();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetName());
    m_httpRequestString.append("/S3MX/RealityData?");
    m_httpRequestString.append(m_filter);
    m_httpRequestString.append(m_order);
    m_httpRequestString.append("'&$skip=");
    Utf8P buf = new Utf8Char();
    BeStringUtilities::FormatUInt64(buf, m_startIndex);
    m_httpRequestString.append(buf);
    m_httpRequestString.append("&$top=");
    BeStringUtilities::FormatUInt64(buf, m_pageSize);
    m_httpRequestString.append(buf);
    }

void RealityDataPagedRequest::SortBy(RealityDataField field, bool ascending)
    {
    Utf8String order = "$orderby=";
    switch(field)
        {
    case RealityDataField::Id:
        order.append("Id");
        break;
    case RealityDataField::Enterprise:
        order.append("Enterprise");
        break;
    case RealityDataField::ContainerName:
        order.append("ContainerName");
        break;
    case RealityDataField::Name:
        order.append("Name");
        break;
    case RealityDataField::Dataset:
        order.append("Dataset");
        break;
    case RealityDataField::Description:
        order.append("Description");
        break;
    case RealityDataField::RootDocument:
        order.append("RootDocument");
        break;
    case RealityDataField::Size:
        order.append("Size");
        break;
    case RealityDataField::Classification:
        order.append("Classification");
        break;
    case RealityDataField::Type:
        order.append("Type");
        break;
    case RealityDataField::Footprint:
        order.append("Footprint");
        break;
    case RealityDataField::ThumbnailDocument:
        order.append("ThumbnailDocument");
        break;
    case RealityDataField::MetadataURL:
        order.append("MetadataURL");
        break;
    case RealityDataField::ResolutionInMeters:
        order.append("ResolutionInMeters");
        break;
    case RealityDataField::AccuracyInMeters:
        order.append("AccuracyInMeters");
        break;
    case RealityDataField::PublicAccess:
        order.append("PublicAccess");
        break;
    case RealityDataField::Listable:
        order.append("Listable");
        break;
    case RealityDataField::ModifiedTimestamp:
        order.append("ModifiedTimestamp");
        break;
    case RealityDataField::CreatedTimestamp:
        order.append("CreatedTimestamp");
        break;
    case RealityDataField::OwnedBy:
        order.append("OwnedBy");
        break;
        }

    if(ascending)
        order.append(" asc");
    else
        order.append(" desc");

    m_order = order;
    }

void RealityDataPagedRequest::SetFilter(Utf8StringCR filter) { m_filter = filter; }

void RealityDataListByEnterprisePagedRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServer();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetName());
    m_httpRequestString.append("/S3MX/RealityData?$filter=Enterprise+eq+'");
    m_httpRequestString.append(m_id);
    m_httpRequestString.append("'&$skip=");
    Utf8P buf = new Utf8Char();
    BeStringUtilities::FormatUInt64(buf, m_startIndex);
    m_httpRequestString.append(buf);
    m_httpRequestString.append("&$top=");
    BeStringUtilities::FormatUInt64(buf, m_pageSize);
    m_httpRequestString.append(buf);
    }

void RealityDataProjectRelationByProjectIdPagedRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServer();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetName());
    m_httpRequestString.append("/S3MX/RealityDataProjectRelationship?$filter=ProjectId+eq+'");
    m_httpRequestString.append(m_id);
    m_httpRequestString.append("'&$skip=");
    Utf8P buf = new Utf8Char();
    BeStringUtilities::FormatUInt64(buf, m_startIndex);
    m_httpRequestString.append(buf);
    m_httpRequestString.append("&$top=");
    BeStringUtilities::FormatUInt64(buf, m_pageSize);
    m_httpRequestString.append(buf);
    }

void RealityDataServiceUpload::SetSourcePath(Utf8String sourcePath, Utf8String rootDocument, Utf8String thumbnailDocument)
    {
    //TODO
    }

void RealityDataServiceUpload::SetRealityDataId(Utf8String realityDataId) { m_id = realityDataId; }

//TODO:UploadReport* RealityDataServiceUpload::Perform();


Utf8String RealityDataService::s_realityDataServer = "https://connect-contextservices.bentley.com/";
Utf8String RealityDataService::s_realityDataWSGProtocol = "2.4";
Utf8String RealityDataService::s_realityDataName = "IndexECPlugin-Server";
Utf8String RealityDataService::s_realityDataSchemaName = "RealityModeling";

const Utf8String RealityDataService::s_ImageryKey = "Imagery";
const Utf8String RealityDataService::s_TerrainKey = "Terrain";
const Utf8String RealityDataService::s_ModelKey = "Model";
const Utf8String RealityDataService::s_PinnedKey = "Pinned";

Utf8StringCR RealityDataService::GetServer() { return s_realityDataServer; }
Utf8StringCR RealityDataService::GetWSGProtocol() { return s_realityDataWSGProtocol; }
Utf8StringCR RealityDataService::GetName() { return s_realityDataName; }
Utf8StringCR RealityDataService::GetSchemaName() { return s_realityDataSchemaName; }

bvector<SpatialEntityPtr> RealityDataService::Request(RealityDataPagedRequestCR request)
    {
    Utf8String jsonString;
    BentleyStatus status = RequestToJSON((RealityDataUrl*)(&request), jsonString);

    BeAssert(status == BentleyStatus::SUCCESS);

    bvector<SpatialEntityPtr> entities = bvector<SpatialEntityPtr>();
    RealityConversionTools::JsonToSpatialEntity(jsonString.c_str(), &entities);

    return entities;
    }

SpatialEntityPtr RealityDataService::Request(RealityDataByIdRequestCR request)
    {
    Utf8String jsonString;
    BentleyStatus status = RequestToJSON((RealityDataUrl*)(&request), jsonString);

    BeAssert(status == BentleyStatus::SUCCESS);

    bvector<SpatialEntityPtr> entities = bvector<SpatialEntityPtr>();
    RealityConversionTools::JsonToSpatialEntity(jsonString.c_str(), &entities);

    return entities[0];
    /*bvector<Utf8String> nodes = bvector<Utf8String>();
    Json::Value instances(Json::objectValue);
    for (auto instance : instances["instances"])
        {
        if (instance.isMember("instanceId") && instance.isMember("properties") && instance["properties"].isMember("Label"))
            nodes.push_back(instance["instanceId"].asCString());
        }

    return nodes[0];*/
    }

//TODORealityDataDocumentPtr RealityDataService::Request(RealityDataDocumentByIdRequestCR request);

bvector<Byte> RealityDataService::Request(RealityDataDocumentContentByIdRequestCR request)
    {
    return bvector<Byte>();//TODO
    }

//TODO RealityDataFolderPtr RealityDataService::Request(RealityDataFolderByIdRequestCR request);

bvector<SpatialEntityPtr> RealityDataService::Request(RealityDataListByEnterprisePagedRequestCR request)
    {
    Utf8String jsonString;
    BentleyStatus status = PagedRequestToJSON((RealityDataPagedRequest*)(&request), jsonString);

    BeAssert(status == BentleyStatus::SUCCESS);

    bvector<SpatialEntityPtr> entities = bvector<SpatialEntityPtr>();
    RealityConversionTools::JsonToSpatialEntity(jsonString.c_str(), &entities);

    return entities;
    }

//TODO bvector<RealityDataProjectRelationPtr> RealityDataService::Request(RealityDataProjectRelationByProjectIdPagedRequestCR request)

BentleyStatus RealityDataService::PagedRequestToJSON(RealityDataPagedRequestPtr request, Utf8StringR jsonResponse)
    {
    int status = 0;
    jsonResponse = WSGRequest::GetInstance().PerformRequest(*request, status);

    Json::Value instances(Json::objectValue);
    if((status != CURLE_OK) || !Json::Reader::Parse(jsonResponse, instances) || instances.isMember("errorMessage") || !instances.isMember("instances"))
        return BentleyStatus::ERROR;

    request->AdvancePage();

    return BentleyStatus::SUCCESS;
    }

BentleyStatus RealityDataService::RequestToJSON(RealityDataUrlPtr request, Utf8StringR jsonResponse)
    {
    int status = 0;
    jsonResponse = WSGRequest::GetInstance().PerformRequest(*request, status);

    Json::Value instances(Json::objectValue);
    if ((status != CURLE_OK) || !Json::Reader::Parse(jsonResponse, instances) || instances.isMember("errorMessage") || !instances.isMember("instances"))
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//TODO RealityDataServiceUploadPtr RealityDataService::CreateUpload();