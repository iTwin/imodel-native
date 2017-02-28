/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityDataService.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/BeFileListIterator.h>
#include <Bentley/bset.h>

//#include <iostream>
#include <RealityPlatform/RealityDataService.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityConversionTools.h>

#include <stdio.h>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <Bentley/Base64Utilities.h>

#define MAX_NB_CONNECTIONS          10
USING_NAMESPACE_BENTLEY_REALITYPLATFORM


static size_t CurlReadDataCallback(void* buffer, size_t size, size_t count, RealityDataFileUpload* request)
    {
    return request->OnReadData(buffer, size * count);
    }

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
    AzureWriteHandshake* handshake = (AzureWriteHandshake*)userp;
    if (handshake != nullptr)
        handshake->GetJsonResponse().append((char*)contents, size * nmemb);
    else
        ((Utf8String*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
    }

Utf8StringCR RealityDataUrl::GetServerName() const { return RealityDataService::GetServerName(); }

Utf8StringCR RealityDataUrl::GetVersion() const { return RealityDataService::GetWSGProtocol(); }

Utf8StringCR RealityDataUrl::GetSchema() const { return RealityDataService::GetSchemaName(); }

Utf8StringCR RealityDataUrl::GetRepoId() const { return RealityDataService::GetRepoName(); }


void RealityDataByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload(); 
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetRepoName());
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetSchemaName());
    m_httpRequestString.append("/RealityData/");
    m_httpRequestString.append(m_id);
    }

void RealityDataProjectRelationshipByProjectIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetRepoName());
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetSchemaName());
    m_httpRequestString.append("/RealityDataProjectRelationship?$filter=ProjectId+eq+'");
    m_httpRequestString.append(m_id);
    m_httpRequestString.append("'");
    }

void RealityDataFolderByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetRepoName());
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetSchemaName());
    m_httpRequestString.append("/Folder/");
    m_httpRequestString.append(m_id);
    }

void RealityDataDocumentByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetRepoName());
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetSchemaName());
    m_httpRequestString.append("/Document/");
    m_httpRequestString.append(m_id);
    }

void RealityDataDocumentContentByIdRequest::ChangeInstanceId(Utf8String instanceId)
    {
    m_id = instanceId;
    m_validRequestString = false;
    }

void RealityDataDocumentContentByIdRequest::GetAzureRedirectionRequestUrl()
    {
    //https://s3mxcloudservice.cloudapp.net/v2.4/Repositories/S3MXECPlugin--Server/S3MX/Document/ab9c6aa6-91ad-424b-935c-28a3c396a041~2FGraz~2FScene~2FProduction_Graz_3MX.3mx/FileAccess.FileAccessKey?$filter=Permissions+eq+'Read'&api.singleurlperinstance=true 
    Utf8String url = "https://";
    url.append(RealityDataService::GetServerName());
    url.append("/");
    url.append(RealityDataService::GetWSGProtocol());
    url.append("/Repositories/");
    url.append(RealityDataService::GetRepoName());
    url.append("/");
    url.append(RealityDataService::GetSchemaName());
    url.append("/RealityData/");
    
    bvector<Utf8String> lines;
    BeStringUtilities::Split(m_id.c_str(), "~", lines);
    Utf8String root = lines[0];

    url.append(root);
    url.append("/FileAccess.FileAccessKey?$filter=Permissions+eq+'Read'&api.singleurlperinstance=true ");
    m_allowAzureRedirection = false;

    WSGURL wsgurl = WSGURL(url);

    int status = RequestType::Body;
    WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());
    Utf8String jsonString = WSGRequest::GetInstance().PerformRequest(wsgurl, status, RealityDataService::GetVerifyPeer());

    Json::Value instances(Json::objectValue);
    if ((status != CURLE_OK) || (!Json::Reader::Parse(jsonString, instances) || (!instances.isMember("errorMessage") && !instances.isMember("instances")) || instances.isMember("errorMessage")))
        return;

    for (auto instance : instances["instances"])
        {
        if (instance.isMember("properties") && instance["properties"].isMember("Url") && !instance["properties"]["Url"].isNull())
            {
            bvector<Utf8String> parts;
            Utf8String AzureUrl = instance["properties"]["Url"].asCString();
            BeStringUtilities::Split(AzureUrl.c_str(), "\?", parts);
            //https://realityblobdeveussa01.blob.core.windows.net/cc5421e5-a80e-469f-a459-8c76da351fe5?sv=2015-04-05&sr=c&sig=6vtz14nV4FsCidf9XCWm%2FAS48%2BJozxk3zpd1FKwUmnI%3D&se=2017-02-10T15%3A36%3A43Z&sp=r
            m_azureServer = parts[0];
            m_azureToken = parts[1];
            m_allowAzureRedirection = true;
            }
        }
    }

/*void RealityDataDocumentContentByIdRequest::SetAzureRedirectionUrlToContainer(Utf8String azureContainerUrl)
    {
    m_AzureRedirectionURL = azureContainerUrl;
    m_AzureRedirected = true;
    }*/

bool RealityDataDocumentContentByIdRequest::IsAzureBlobRedirected() { return m_AzureRedirected; }

void RealityDataDocumentContentByIdRequest::SetAzureRedirectionPossible(bool possible) { m_allowAzureRedirection = possible; }

bool RealityDataDocumentContentByIdRequest::IsAzureRedirectionPossible() { return m_allowAzureRedirection; }

void RealityDataDocumentContentByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    if(m_allowAzureRedirection)
        {
        m_httpRequestString = m_azureServer;
        bvector<Utf8String> parts;
        BeStringUtilities::Split(m_id.c_str(), "~", parts);
        Utf8String Guid = parts[0];
        Guid.append("~2F");
        m_id.ReplaceAll(Guid.c_str(), "");
        m_httpRequestString.append("/");
        m_httpRequestString.append(m_id);
        m_httpRequestString.append("\?");
        m_httpRequestString.append(m_azureToken);
        m_httpRequestString.ReplaceAll("~2F", "/");

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
    delete buf;
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
    Utf8String filter = "OwnedBy+eq+'";
    filter.append(owner);
    filter.append("'");
    return filter;
    }

Utf8String RealityDataFilterCreator::FilterByCreationDate(DateTime minDate, DateTime maxDate)
    {
    Utf8String filter = "CreatedTimestamp+ge+'";
    filter.append(minDate.ToString());
    filter.append("'+and+CreatedTimestamp+le+'");
    filter.append(maxDate.ToString());
    filter.append("'");

    return filter;
    }

Utf8String RealityDataFilterCreator::FilterByModificationDate(DateTime minDate, DateTime maxDate)
    {
    Utf8String filter = "ModifiedTimestamp+ge+'";
    filter.append(minDate.ToString());
    filter.append("'+and+ModifiedTimestamp+le+'");
    filter.append(maxDate.ToString());
    filter.append("'");

    return filter;
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
    Utf8String filter = "ResolutionInMeters+ge+'";
    char buf[32];
    sprintf(buf, "%f", resMin);
    filter.append(buf);
    filter.append("'+and+ResolutionInMeters+le+'");
    sprintf(buf, "%f", resMax);
    filter.append(buf);
    filter.append("'");

    return filter;
    }

Utf8String RealityDataFilterCreator::FilterByAccuracy(double accuracyMin, double accuracyMax, bool filterOutUnspecified)
    {
    Utf8String filter = "AccuracyInMeters+ge+'";
    char buf[32];
    sprintf(buf, "%f", accuracyMin);
    filter.append(buf);
    filter.append("'+and+AccuracyInMeters+le+'");
    sprintf(buf, "%f", accuracyMax);
    filter.append(buf);
    filter.append("'");

    return filter;
    }

Utf8String RealityDataFilterCreator::FilterByType(Utf8String types)
    {
    Utf8String filter = "Type+eq+'";
    filter.append(types);
    filter.append("'");
    return filter;
    }

Utf8String RealityDataFilterCreator::FilterByDataset(Utf8String dataset)
    {
    Utf8String filter = "Dataset+eq+'";
    filter.append(dataset);
    filter.append("'");

    return filter;
    }


Utf8String RealityDataFilterCreator::GroupFiltersAND(bvector<Utf8String> filters)
    {
    Utf8String filter = "";//"(";
    filter.append(filters[0]);
    for(int i = 1; i < filters.size(); i++)
        {
        filter.append("+and+");
        filter.append(filters[i]);
        }

    //filter.append(")");
    return filter;
    }

Utf8String RealityDataFilterCreator::GroupFiltersOR(bvector<Utf8String> filters)
    {
    Utf8String filter = "";//"(";
    filter.append(filters[0]);
    for (int i = 1; i < filters.size(); i++)
        {
        filter.append("+or+");
        filter.append(filters[i]);
        }

    //filter.append(")");
    return filter;
    }


Utf8StringCR RealityDataPagedRequest::GetServerName() const { return RealityDataService::GetServerName(); }
Utf8StringCR RealityDataPagedRequest::GetVersion() const { return RealityDataService::GetWSGProtocol(); }
Utf8StringCR RealityDataPagedRequest::GetSchema() const { return RealityDataService::GetSchemaName(); }
Utf8StringCR RealityDataPagedRequest::GetRepoId() const { return RealityDataService::GetRepoName(); }

void RealityDataPagedRequest::_PrepareHttpRequestStringAndPayload() const
    {
    bool hasFilter = m_filter.length() > 0;
    bool hasOrder = m_order.length() > 0;

    m_serverName = RealityDataService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetRepoName());
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetSchemaName());
    m_httpRequestString.append("/RealityData?");
    if (hasFilter)
        {
        m_httpRequestString.append("$filter=");
        m_httpRequestString.append(m_filter);
        m_httpRequestString.append("&");
        }
    if(hasOrder)
        {
        m_httpRequestString.append(m_order);
        m_httpRequestString.append("&");
        }
    if(hasFilter || hasOrder)
    m_httpRequestString.append("$skip=");
    Utf8P buf = new Utf8Char();
    BeStringUtilities::FormatUInt64(buf, m_startIndex);
    m_httpRequestString.append(buf);
    m_httpRequestString.append("&$top=");
    BeStringUtilities::FormatUInt64(buf, m_pageSize);
    m_httpRequestString.append(buf);
    delete buf;
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
        order.append("+asc");
    else
        order.append("+desc");

    m_order = order;
    }

void RealityDataPagedRequest::SetFilter(Utf8StringCR filter) { m_filter = filter; }

void RealityDataListByEnterprisePagedRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetRepoName());
    m_httpRequestString.append("/"); 
    m_httpRequestString.append(RealityDataService::GetSchemaName());
    m_httpRequestString.append("/RealityData?$filter=Enterprise+eq+'");

    if(m_id.length() == 0)
        {
        Utf8String token = CurlConstructor().GetToken();
        token.ReplaceAll("Authorization: Token ","");
        Utf8String decodedToken = Base64Utilities::Decode(token);

        const char* charstring = decodedToken.c_str();
        Utf8String keyword = "organizationid";
        const char* attributePosition = strstr(charstring, keyword.c_str());
        keyword = "<saml:AttributeValue>";
        const char* valuePosition = strstr(attributePosition, keyword.c_str()); 
        valuePosition += keyword.length();
        Utf8String idString = Utf8String(valuePosition);

        bvector<Utf8String> lines;
        BeStringUtilities::Split(idString.c_str(), "< ", lines);
        m_id = lines[0];
        }

    m_httpRequestString.append(m_id);
    m_httpRequestString.append("'&$skip=");
    Utf8P buf = new Utf8Char();
    BeStringUtilities::FormatUInt64(buf, m_startIndex);
    m_httpRequestString.append(buf);
    m_httpRequestString.append("&$top=");
    BeStringUtilities::FormatUInt64(buf, m_pageSize);
    m_httpRequestString.append(buf);
    delete buf;
    }

void RealityDataProjectRelationshipByProjectIdPagedRequest::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetRepoName());
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetSchemaName());
    m_httpRequestString.append("/RealityDataProjectRelationship?$filter=ProjectId+eq+'");
    m_httpRequestString.append(m_id);
    m_httpRequestString.append("'&$skip=");
    Utf8P buf = new Utf8Char();
    BeStringUtilities::FormatUInt64(buf, m_startIndex);
    m_httpRequestString.append(buf);
    m_httpRequestString.append("&$top=");
    BeStringUtilities::FormatUInt64(buf, m_pageSize);
    m_httpRequestString.append(buf);
    delete buf;
    }

RealityDataServiceCreate::RealityDataServiceCreate(Utf8String realityDataId, Utf8String properties)
    { 
    m_id = realityDataId; 
    m_validRequestString = false;

    m_requestType = HttpRequestType::POST_Request;
    m_requestPayload = "{\"instance\":{\"instanceId\":\"";
    m_requestPayload.append(m_id);
    m_requestPayload.append("\", \"className\": \"RealityData\",\"schemaName\":\"S3MX\", \"properties\": {");
    m_requestPayload.append(properties);
    m_requestPayload.append("}}}");
    }

void RealityDataServiceCreate::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetRepoName());
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetSchemaName());
    m_httpRequestString.append("/RealityData/");
    m_httpRequestString.append(m_id);
    m_requestHeader.push_back("Content-Type: application/json");
    }

void RealityDataFileUpload::_PrepareHttpRequestStringAndPayload() const
    {
    m_httpRequestString = m_azureServer;
    Utf8String addon = "/";
    addon.append(m_fileUrl);
    addon.append("?");
    addon.ReplaceAll("//","/"); //this covers whether the user input the directory as 
                                // C:/Directory or C:/Directory/
    m_httpRequestString.append(addon);
    m_validRequestString = true;

    m_requestHeader.clear();
    if(m_moreToSend)
        m_requestHeader.push_back("x-ms-blob-type: BlockBlob");
    }

bool RealityDataFileUpload::FinishedSending()
    { 
    if(!m_singleChunk)
        return !m_moreToSend; 
    else
        return (m_uploadProgress >= m_fileSize);
    }

void RealityDataFileUpload::UpdateUploadedSize()//uint32_t amount, void* ptr)
    {
    if(m_uploadProgress < m_fileSize - 1)
        {
        uint64_t uploadStep = m_uploadProgress + m_chunkSize;
        if(m_fileSize > uploadStep)
            {
        m_chunkStop = uploadStep;
            }
        else
            {
            m_chunkStop = m_fileSize;
            m_chunkSize = m_fileSize - m_uploadProgress;
            }

        if(!m_singleChunk)
            {
            m_blockList.append("<Latest>");
            std::stringstream blockIdStream;
            blockIdStream << std::setw(5) << std::setfill('0') << m_chunkNumber;
            std::string blockId = blockIdStream.str();
            m_chunkNumberString = Base64Utilities::Encode(blockId.c_str()).c_str();
            m_blockList.append(m_chunkNumberString);
            m_blockList.append("</Latest>");
            }
        m_uploadProgress = m_chunkStop;
        ++m_chunkNumber;
        }
    else if (!m_singleChunk)
        {
        m_moreToSend = false;
        m_blockList.append("</BlockList>");
        m_chunkSize = m_blockList.length();
        }
    }

size_t RealityDataFileUpload::OnReadData(void* buffer, size_t size)
    {
    uint32_t bytesRead = 0;
    if(m_moreToSend)
        {
        BeFileStatus status = m_fileStream.Read(buffer, &bytesRead, (uint32_t)size);
        if(status != BeFileStatus::Success)
            return 0;
        }
    else if (!m_singleChunk)
        {
        memcpy(buffer, m_blockList.c_str(), m_blockList.length());
        bytesRead = (uint32_t)m_blockList.length();
        }
    return bytesRead;
    }

void RealityDataFileUpload::StartTimer()
    {
    m_startTime = std::time(nullptr);
    }

void RealityDataFileUpload::Retry()
    {
    CloseFile();
    m_chunkSize = 4*1024*1024;
    ReadyFile();
    }

void AzureWriteHandshake::_PrepareHttpRequestStringAndPayload() const
    {
    //https://dev-realitydataservices-eus.cloudapp.net/v2.4/Repositories/S3MXECPlugin--Server/S3MX/RealityData/cc5421e5-a80e-469f-a459-8c76da351fe5/FileAccess.FileAccessKey?$filter=Permissions+eq+'Read'&api.singleurlperinstance=true 
    m_serverName = RealityDataService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetRepoName());
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetSchemaName());
    m_httpRequestString.append("/RealityData/");
    m_httpRequestString.append(m_id);
    m_httpRequestString.append("/FileAccess.FileAccessKey?$filter=Permissions+eq+'Write'&api.singleurlperinstance=true");
    }

UploadReport::~UploadReport()
    {
    for (int i = 0; i < results.size(); ++i)
        delete (results[i]);
    }

void UploadReport::ToXml(Utf8StringR report)
    {
    BeXmlWriterPtr writer = BeXmlWriter::Create();
    BeAssert(writer.IsValid());
    writer->SetIndentation(2);

    writer->WriteElementStart("RealityDataService_UploadReport");
        {
        writer->WriteAttribute("Date", Utf8String(DateTime::GetCurrentTimeUtc().ToString()).c_str());

        for (int i = 0; i < results.size(); ++i)
            {
            writer->WriteElementStart("File");
                {
                UploadResult* ur = results[i];
                writer->WriteAttribute("FileName", ur->name.c_str());
                writer->WriteAttribute("timeSpent", (long)ur->timeSpent);
                writer->WriteAttribute("CURLcode", ur->errorCode);
                writer->WriteAttribute("uploadProgress", ur->uploadProgress);
                }
            writer->WriteElementEnd();
            }
        }
    writer->WriteElementEnd();
    writer->ToString(report);
    }

Utf8String RealityDataServiceUpload::PackageProperties(bmap<RealityDataField, Utf8String> properties)
    {
    Utf8String propertyString;
    bvector<Utf8String> propertyVector = bvector<Utf8String>();
    RealityDataField field;
    for(bmap<RealityDataField, Utf8String>::iterator it = properties.begin(); it != properties.end(); it.increment())
        {
        propertyString = "";
        field = it.key();
        switch (field)
            {
        case RealityDataField::Id:
            propertyString.append("\"Id\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::Enterprise:
            propertyString.append("\"Enterprise\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::ContainerName:
            propertyString.append("\"ContainerName\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::Name:
            propertyString.append("\"Name\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::Dataset:
            propertyString.append("\"Dataset\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::Description:
            propertyString.append("\"Description\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::RootDocument:
            propertyString.append("\"RootDocument\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::Size:
            propertyString.append("\"Size\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::Classification:
            propertyString.append("\"Classification\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::Type:
            propertyString.append("\"Type\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::Footprint:
            propertyString.append("\"Footprint\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::ThumbnailDocument:
            propertyString.append("\"ThumbnailDocument\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::MetadataURL:
            propertyString.append("\"MetadataURL\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::ResolutionInMeters:
            propertyString.append("\"ResolutionInMeters\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::AccuracyInMeters:
            propertyString.append("\"AccuracyInMeters\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::PublicAccess:
            propertyString.append("\"PublicAccess\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::Listable:
            propertyString.append("\"Listable\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::ModifiedTimestamp:
            propertyString.append("\"ModifiedTimestamp\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::CreatedTimestamp:
            propertyString.append("\"CreatedTimestamp\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::OwnedBy:
            propertyString.append("\"OwnedBy\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
        case RealityDataField::Group:
            propertyString.append("\"Group\" : \"");
            propertyString.append(properties[field]);
            propertyString.append("\"");
            break;
            }
        propertyVector.push_back(propertyString);
        }
    propertyString = propertyVector[0];
    for(int i = 1; i < propertyVector.size(); ++i)
        {
        propertyString.append(",");
        propertyString.append(propertyVector[i]);
        }
    return propertyString;
    }

BentleyStatus RealityDataServiceUpload::CreateUpload(Utf8String properties)
    {
    RealityDataByIdRequest* getRequest = new RealityDataByIdRequest(m_id);
    Utf8String response;
    if (RealityDataService::RequestToJSON((RealityDataUrl*)getRequest, response) == RequestStatus::ERROR) //file does not exist, need POST Create
        {
        RealityDataServiceCreate createRequest = RealityDataServiceCreate(m_id, properties);
        int status;
        response = WSGRequest::GetInstance().PerformRequest(createRequest, status, RealityDataService::GetVerifyPeer());
        if (RealityDataService::RequestToJSON((RealityDataUrl*)getRequest, response) == RequestStatus::ERROR)
            {
            ReportStatus(0, nullptr, -1, "Unable to create RealityData with specified parameters");
            return BentleyStatus::ERROR;
            }
        }
    else if (!m_overwrite)
        {
        ReportStatus(0, nullptr, -1, "RealityData with specified GUID already exists on server. Overwrite variable not specified, aborting operation");
        return BentleyStatus::ERROR;
        }

    delete getRequest;
    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(response, instances);
    if(!instances["instances"].isNull() && !instances["instances"][0]["properties"].isNull() &&!instances["instances"][0]["properties"]["ThumbnailDocument"].isNull())
        m_thumbnailDocument = instances["ThumbnailDocument"].asString();
    if(!instances["instances"].isNull() && !instances["instances"][0]["properties"].isNull() && !instances["instances"][0]["properties"]["RootDocument"].isNull())
        m_rootDocument = instances["RootDocument"].asString();
    return BentleyStatus::SUCCESS;
    }

BentleyStatus RealityDataServiceUpload::ParseHandshakeResponse(Utf8String json)
    {
    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(json, instances);

    Json::Value instance;
    
    if(instances.isMember("instances") && !instances["instances"][0].isNull() && instances["instances"][0].isMember("properties") && !instances["instances"][0]["properties"].isNull())
        instance = instances["instances"][0]["properties"];
    
    if(instance.isMember("Url") && !instance["Url"].isNull())
        {
        Utf8String url = instance["Url"].asString();
        bvector<Utf8String> parts;
        BeStringUtilities::Split(url.c_str(), "\?", parts);
        //https://realityblobdeveussa01.blob.core.windows.net/cc5421e5-a80e-469f-a459-8c76da351fe5?sv=2015-04-05&sr=c&sig=6vtz14nV4FsCidf9XCWm%2FAS48%2BJozxk3zpd1FKwUmnI%3D&se=2017-02-10T15%3A36%3A43Z&sp=r
        m_azureServer = parts[0];
        m_azureToken = parts[1];
        return BentleyStatus::SUCCESS;
        }
    else
        return BentleyStatus::ERROR;
    }

UploadReport* RealityDataServiceUpload::Perform()
    {
    m_currentUploadedAmount = 0;
    m_progress = 0.0;

    m_ulReport = UploadReport();
    // we can optionally limit the total amount of connections this multi handle uses 
    curl_multi_setopt(m_pCurlHandle, CURLMOPT_MAXCONNECTS, MAX_NB_CONNECTIONS);

    m_curEntry = 0;

    for (int i = 0; i < min(MAX_NB_CONNECTIONS, (int)m_filesToUpload.size()); ++i)
        {
        SetupNextEntry();
        }   

    int still_running; /* keep number of running handles */
    int repeats = 0;

    do
        {
        CURLMcode mc; /* curl_multi_wait() return code */
        int numfds;

        curl_multi_perform(m_pCurlHandle, &still_running);

        /* wait for activity, timeout or "nothing" */
        mc = curl_multi_wait(m_pCurlHandle, NULL, 0, 1000, &numfds);

        if (mc != CURLM_OK)
            {
            ReportStatus(-1, NULL, mc, "curl_multi_wait() failed");
            break;
            }

        /* 'numfds' being zero means either a timeout or no file descriptors to
        wait for. Try timeout on first occurrence, then assume no file
        descriptors and no file descriptors to wait for means wait for 100
        milliseconds. */

        if (!numfds)
            {
            repeats++; /* count number of repeated zero numfds */
            if (repeats > 1)
                Sleep(300);
            }
        else
            repeats = 0;

        CURLMsg *msg;
        int nbQueue;
        while ((msg = curl_multi_info_read(m_pCurlHandle, &nbQueue)))
            {
            if (msg->msg == CURLMSG_DONE)
                {
                char *pClient;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &pClient);

                RealityDataFileUpload *fileUp = (RealityDataFileUpload *)pClient;
                
                // Retry on error
                if (msg->data.result == 56 || msg->data.result == 28)     // Recv failure, try again
                    {
                    if (fileUp != nullptr && fileUp->nbRetry < 5)
                        {
                        ++fileUp->nbRetry;
                        //ReportStatus((int)fileUp->index, pClient, 0, "Trying again...");
                        fileUp->Retry();
                        SetupCurlforFile((RealityDataUrl*)(fileUp), 0);
                        still_running++;
                        }
                    else
                        {
                        // Maximun retry done, return error.
                        ReportStatus((int)fileUp->m_index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
                        }
                    }
                else if(fileUp != nullptr)
                    {
                    if(msg->data.result == CURLE_OK)
                        {
                        if(!fileUp->FinishedSending())
                            {
                            if(m_pProgressFunc && UpdateUploadedAmount(fileUp->GetMessageSize()))
                                m_pProgressFunc(fileUp->GetFilename(), ((double)fileUp->GetUploadedSize()) / fileUp->GetFileSize(), m_progress);
                            SetupCurlforFile(fileUp, 0);
                            still_running++;
                            }
                        else
                            {
                            if (m_pProgressFunc && UpdateUploadedAmount(fileUp->GetMessageSize()))
                                m_pProgressFunc(fileUp->GetFilename(), 1.0, m_progress);
                            ReportStatus((int)fileUp->m_index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
                            fileUp->CloseFile();
                            }  
                        }
                    else
                        ReportStatus((int)fileUp->m_index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
                    }
                           
                    
                curl_multi_remove_handle(m_pCurlHandle, msg->easy_handle);
                curl_easy_cleanup(msg->easy_handle);
                }
            else
                {
                ReportStatus(-1, NULL, msg->msg, "CurlMsg failed");
                }

            // Other URL to download ?
            if (m_curEntry < (int)m_filesToUpload.size() && still_running < MAX_NB_CONNECTIONS)
                {
                if (SetupNextEntry())
                    still_running++;
                }
            }

        } while (still_running);

    return &m_ulReport;
    }

bool RealityDataServiceUpload::SetupNextEntry()
    {
    if (NULL != m_pHeartbeatFunc && m_pHeartbeatFunc() != 0)
        return false;

    if (m_curEntry < (int)m_filesToUpload.size())
        {
        RealityDataFileUpload* fUp = m_filesToUpload[m_curEntry];
        fUp->ReadyFile();
        SetupCurlforFile((RealityDataUrl*)(fUp), 0);//SetupCurlandFile(&m_pEntries[m_curEntry]);
        ++m_curEntry;
        }
    else
        return false;

    return true;
    }

void RealityDataServiceUpload::SetupCurlforFile(RealityDataUrl* request, int verifyPeer)
    {
    // If cancel requested, don't queue new files
    /*if (NULL != m_pHeartbeatFunc && m_pHeartbeatFunc() != 0)
        return SetupCurlStatus::Success;*/

    int code = 2; //BodyNoToken
    AzureWriteHandshake* handshake = dynamic_cast<AzureWriteHandshake*>(request);
    RealityDataFileUpload* fileUpload = dynamic_cast<RealityDataFileUpload*>(request);
    if (handshake != nullptr)
        code = 0;
    else if(fileUpload != nullptr)
        {
        fileUpload->SetAzureToken(GetAzureToken());
        fileUpload->UpdateUploadedSize();
        }
    else
        return; //unexpected request

    CURL *pCurl = PrepareCurl(*request, code, verifyPeer, nullptr);
    if (pCurl)
        {
        curl_easy_setopt(pCurl, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 0L);
        /*if (!m_proxyUrl.empty())
            {
            curl_easy_setopt(pCurl, CURLOPT_PROXY, m_proxyUrl.c_str());
            curl_easy_setopt(pCurl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
            if (!m_proxyCreds.empty())
                {
                curl_easy_setopt(pCurl, CURLOPT_PROXYUSERPWD, m_proxyCreds.c_str());
                }
            }*/
        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 0L);
        
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(pCurl, CURLOPT_PRIVATE, request);

        curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(pCurl, CURLOPT_LOW_SPEED_LIMIT, 1L); // B/s
        curl_easy_setopt(pCurl, CURLOPT_LOW_SPEED_TIME, 60); //60s

        if(fileUpload != nullptr)
            {
            curl_easy_setopt(pCurl, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "PUT");

            curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, CurlReadDataCallback);
            curl_easy_setopt(pCurl, CURLOPT_READDATA, fileUpload);

            curl_easy_setopt(pCurl, CURLOPT_INFILESIZE_LARGE, fileUpload->GetMessageSize());
            fileUpload->StartTimer();
            }
        else if (handshake != nullptr)
            {
            /* Define our callback to get called when there's data to be written */
            curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
            /* Set a pointer to our struct to pass to the callback */
            curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, handshake);
            }

        curl_multi_add_handle((CURLM*)m_pCurlHandle, pCurl);
        }
    }


void RealityDataServiceUpload::ReportStatus(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    if (m_pStatusFunc)
        m_pStatusFunc(index, pClient, ErrorCode, pMsg);

    RealityDataFileUpload* pEntry = (RealityDataFileUpload*)pClient;

    if(pEntry == nullptr)
        return;

    UploadResult* ur = new UploadResult();
    ur->errorCode = ErrorCode;
    ur->uploadProgress = (100 * pEntry->GetUploadedSize() / pEntry->GetFileSize());
    ur->timeSpent = std::time(nullptr) - pEntry->GetStartTime();
    ur->name = pEntry->GetFilename();
    m_ulReport.results.push_back(ur);

    delete pEntry;
    }

Utf8String RealityDataServiceUpload::GetAzureToken()
    {
    if ((std::time(nullptr) - m_azureTokenTimer) > (50 * 60))
        {
        RealityDataService::RequestToJSON((RealityDataUrl*)m_handshakeRequest, m_handshakeRequest->GetJsonResponse());
        if(ParseHandshakeResponse(m_handshakeRequest->GetJsonResponse()) != BentleyStatus::SUCCESS)
            ReportStatus(0, nullptr, -1, "Failure retrieving Azure token");
        else
            m_azureTokenTimer = std::time(nullptr);
        }
    return m_azureToken;
    }

bool RealityDataServiceUpload::UpdateUploadedAmount(uint64_t uploadedAmount)
    {
    m_currentUploadedAmount += uploadedAmount;
    m_progress = ((double)m_currentUploadedAmount) / m_fullUploadSize;
    bool sendProgressCallback = false;
    if(m_progress > m_progressThreshold)
        {
        sendProgressCallback = true;
        m_progressThreshold += m_progressStep;
        }
    return sendProgressCallback;
    }

RealityDataServiceUpload::RealityDataServiceUpload(BeFileName uploadPath, Utf8String id, Utf8String properties, bool overwrite) : 
    m_id(id), m_overwrite(overwrite), m_azureTokenTimer(0), m_progress(0.0), m_progressStep(0.01), m_progressThreshold(0.01),
    m_currentUploadedAmount(0)
    { 
    if(CreateUpload(properties) != BentleyStatus::SUCCESS)
        return;
    m_handshakeRequest = new AzureWriteHandshake(m_id);
    GetAzureToken();

    RealityDataFileUpload* fileUp;

    if(uploadPath.DoesPathExist() && uploadPath.IsDirectory()) //path is directory, find all documents
        {
        uploadPath.AppendToPath(L"/");

        BeFileName root(uploadPath);

        uploadPath.AppendToPath(L"*");
        BeFileListIterator fileIt = BeFileListIterator(uploadPath.GetName(), true);

        BeFileName fileName;
        //BeFileListIterator returns the same filenames twice for every subfolder containing it
        // i.e. folder/folder2/test.txt would appear 4 times
        // the bset is used to avoid adding multiple uploads for a single file
        bset<Utf8String> duplicateSet = bset<Utf8String>(); 
        size_t i = 0;
        while (fileIt.GetNextFileName(fileName) == BentleyStatus::SUCCESS) 
            {
            if(!uploadPath.IsDirectory() && duplicateSet.find(fileName.GetNameUtf8()) == duplicateSet.end())
                {
                duplicateSet.insert(fileName.GetNameUtf8());
                fileUp = new RealityDataFileUpload(fileName, root, m_azureServer, i++);
                m_filesToUpload.push_back(fileUp);
                m_fullUploadSize += fileUp->GetFileSize();
                }
            }
        }
    else if (uploadPath.DoesPathExist())
        {
        fileUp = new RealityDataFileUpload(uploadPath, uploadPath.GetDirectoryName(), m_azureServer, 0);
        m_filesToUpload.push_back(fileUp);
        m_fullUploadSize = fileUp->GetFileSize();
        }

    m_pCurlHandle = curl_multi_init();
    }

Utf8String RealityDataService::s_realityDataServer = "https://connect-contextservices.bentley.com/";
Utf8String RealityDataService::s_realityDataWSGProtocol = "2.4";
Utf8String RealityDataService::s_realityDataRepoName = "IndexECPlugin-Server";
Utf8String RealityDataService::s_realityDataSchemaName = "RealityModeling";

int RealityDataService::s_verifyPeer = 0;
Utf8String RealityDataService::s_realityDataCertificatePath = "";

const Utf8String RealityDataService::s_ImageryKey = "Imagery";
const Utf8String RealityDataService::s_TerrainKey = "Terrain";
const Utf8String RealityDataService::s_ModelKey = "Model";
const Utf8String RealityDataService::s_PinnedKey = "Pinned";

Utf8StringCR RealityDataService::GetServerName() { return s_realityDataServer; }
Utf8StringCR RealityDataService::GetWSGProtocol() { return s_realityDataWSGProtocol; }
Utf8StringCR RealityDataService::GetRepoName() { return s_realityDataRepoName; }
Utf8StringCR RealityDataService::GetSchemaName() { return s_realityDataSchemaName; }
const int RealityDataService::GetVerifyPeer() { return s_verifyPeer; } //TODO: verify when possible...
Utf8StringCR RealityDataService::GetCertificatePath() { return s_realityDataCertificatePath; }

bvector<SpatialEntityPtr> RealityDataService::Request(const RealityDataPagedRequest& request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = PagedRequestToJSON((RealityDataPagedRequest*)(&request), jsonString);

    bvector<SpatialEntityPtr> entities = bvector<SpatialEntityPtr>();
    if (status != RequestStatus::SUCCESS)
        { 
        std::cout << "RealityDataPagedRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        }
    else
        {
        RealityConversionTools::JsonToSpatialEntity(jsonString.c_str(), &entities);
        if ((uint8_t)entities.size() > request.GetPageSize())
            status = RequestStatus::NOMOREPAGES;
        }

    return entities;
    }

SpatialEntityPtr RealityDataService::Request(const RealityDataByIdRequest& request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = RequestToJSON((RealityDataUrl*)(&request), jsonString);
    
    bvector<SpatialEntityPtr> entities = bvector<SpatialEntityPtr>();
    if (status != RequestStatus::SUCCESS)
        {
        std::cout << "RealityDataByIdRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        return nullptr;
        }
    RealityConversionTools::JsonToSpatialEntity(jsonString.c_str(), &entities);

    return entities[0];
    }

RealityDataDocumentPtr RealityDataService::Request(const RealityDataDocumentByIdRequest& request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = RequestToJSON((RealityDataUrl*)(&request), jsonString);

    if (status != RequestStatus::SUCCESS)
        {
        std::cout << "RealityDataDocumentByIdRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        return nullptr;
        }

    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(jsonString, instances);

    return RealityDataDocument::Create(instances["instances"][0]);
    }

void RealityDataService::Request(RealityDataDocumentContentByIdRequest& request, FILE* file, RequestStatus& status)
    {
    int stat = RequestType::Body;
    WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());
    request.GetAzureRedirectionRequestUrl();
    Utf8String resultString = "";
    if (request.IsAzureRedirectionPossible())
        {
        //request.SetAzureRedirectionUrlToContainer(azureUrl);
        resultString = WSGRequest::GetInstance().PerformAzureRequest(request, stat, RealityDataService::GetVerifyPeer(), file);
        }
    else
        resultString = WSGRequest::GetInstance().PerformRequest(request, stat, RealityDataService::GetVerifyPeer(), file);

    status = RequestStatus::SUCCESS;
    if(stat != CURLE_OK)
        {
        status = RequestStatus::ERROR;
        std::cout << "RealityDataDocumentContentByIdRequest failed with response" << std::endl;
        std::cout << resultString << std::endl;
        }
    }

RealityDataFolderPtr RealityDataService::Request(const RealityDataFolderByIdRequest& request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = RequestToJSON((RealityDataUrl*)(&request), jsonString);

    if(status != RequestStatus::SUCCESS)
        {
        std::cout << "RealityDataFolderByIdRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        return nullptr;
        }

    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(jsonString, instances);

    return RealityDataFolder::Create(instances["instances"][0]);
    }

bvector<SpatialEntityPtr> RealityDataService::Request(const RealityDataListByEnterprisePagedRequest& request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = PagedRequestToJSON((RealityDataPagedRequest*)(&request), jsonString);

    bvector<SpatialEntityPtr> entities = bvector<SpatialEntityPtr>();
    if (status != RequestStatus::SUCCESS)
        {
        std::cout << "RealityDataListByEnterprisePagedRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        }
    else
        {
        RealityConversionTools::JsonToSpatialEntity(jsonString.c_str(), &entities);
        if ((uint8_t)entities.size() > request.GetPageSize())
            status = RequestStatus::NOMOREPAGES;
        }

    return entities;
    }

bvector<RealityDataProjectRelationshipPtr> RealityDataService::Request(const RealityDataProjectRelationshipByProjectIdRequest& request, RequestStatus& status)
{
    Utf8String jsonString;
    status = RequestToJSON((RealityDataUrl*)(&request), jsonString);
    
    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(jsonString, instances);

    bvector<RealityDataProjectRelationshipPtr> relations = bvector<RealityDataProjectRelationshipPtr>();
    if (status != RequestStatus::SUCCESS)
        {
        std::cout << "RealityDataProjectRelationshipByProjectIdRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        }
    else
        {
        for (auto instance : instances["instances"])
            relations.push_back(RealityDataProjectRelationship::Create(instance));
        }

    return relations;
}

bvector<RealityDataProjectRelationshipPtr> RealityDataService::Request(const RealityDataProjectRelationshipByProjectIdPagedRequest& request, RequestStatus& status)
    {   
    Utf8String jsonString;
    status = PagedRequestToJSON((RealityDataPagedRequest*)(&request), jsonString);

    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(jsonString, instances);

    bvector<RealityDataProjectRelationshipPtr> relations = bvector<RealityDataProjectRelationshipPtr>();
    if (status != RequestStatus::SUCCESS)
        {
        std::cout << "RealityDataProjectRelationshipByProjectIdPagedRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        }
    else
        {
        for(auto instance : instances["instances"])
            relations.push_back(RealityDataProjectRelationship::Create(instance));
        if((uint8_t)relations.size() > request.GetPageSize())
            status = RequestStatus::NOMOREPAGES;
        }

    return relations;
    }

RequestStatus RealityDataService::PagedRequestToJSON(RealityDataPagedRequest* request, Utf8StringR jsonResponse)
    {
    int status = RequestType::Body;
    WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());
    jsonResponse = WSGRequest::GetInstance().PerformRequest(*request, status, RealityDataService::GetVerifyPeer());

    Json::Value instances(Json::objectValue);
    if((status != CURLE_OK) || !Json::Reader::Parse(jsonResponse, instances) || instances.isMember("errorMessage") || !instances.isMember("instances"))
        return RequestStatus::ERROR;

    request->AdvancePage();

    return RequestStatus::SUCCESS;
    }

RequestStatus RealityDataService::RequestToJSON(RealityDataUrl* request, Utf8StringR jsonResponse)
    {
    int status = RequestType::Body;
    WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());
    jsonResponse = WSGRequest::GetInstance().PerformRequest(*request, status, RealityDataService::GetVerifyPeer());

    Json::Value instances(Json::objectValue);
    if ((status != CURLE_OK) || !Json::Reader::Parse(jsonResponse, instances) || instances.isMember("errorMessage") || !instances.isMember("instances"))
        return RequestStatus::ERROR;

    return RequestStatus::SUCCESS;
    }