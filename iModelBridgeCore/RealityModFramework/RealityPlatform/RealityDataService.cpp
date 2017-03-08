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
#include <Bentley/DateTime.h>

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



BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! RealityDataFileTransfer
//! Control class used internally by the upload and download process.
//=====================================================================================
struct RealityDataFileTransfer : public RealityDataUrl
    {
    public:
        REALITYDATAPLATFORM_EXPORT virtual void ReadyFile() { m_transferProgress = 0; }

        REALITYDATAPLATFORM_EXPORT void CloseFile()
            {
            if (m_fileStream.IsOpen())
                m_fileStream.Close();
            }

        REALITYDATAPLATFORM_EXPORT virtual void Retry() = 0;

        REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR GetHttpRequestString() const override
        {
            if (!m_validRequestString)
                _PrepareHttpRequestStringAndPayload();

            BeAssert(m_validRequestString);
            BeAssert(m_httpRequestString.size() != 0);

            m_requestWithToken = m_httpRequestString;
            m_requestWithToken.append(m_azureToken);

            return m_requestWithToken;
        };

        REALITYDATAPLATFORM_EXPORT void SetAzureToken(Utf8String token) { m_azureToken = token; }

        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFilename() const { return m_filename; }

        REALITYDATAPLATFORM_EXPORT uint64_t GetFileSize() const { return m_fileSize; }

        REALITYDATAPLATFORM_EXPORT uint64_t GetTransferedSize() const { return m_transferProgress; }

        REALITYDATAPLATFORM_EXPORT BeFile& GetFileStream() { return m_fileStream; }

        REALITYDATAPLATFORM_EXPORT void StartTimer() { m_startTime = std::time(nullptr); }

        REALITYDATAPLATFORM_EXPORT time_t GetStartTime() const { return m_startTime; }

        REALITYDATAPLATFORM_EXPORT virtual void UpdateTransferedSize() {}

        size_t                  nbRetry;
        size_t                  m_index;
    protected:

        Utf8String              m_fileUrl;
        Utf8String              m_filename;

        BeFile                  m_fileStream;
        uint64_t                m_fileSize;

        uint64_t                m_transferProgress;

        Utf8String              m_azureServer;
        float                   m_progressStep;
        Utf8String              m_azureToken;
        mutable Utf8String      m_requestWithToken;

        time_t                  m_startTime;
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              02/2017
//! RealityDataFileUpload
//! A specialisation of the RealityDataFileTransfer class that complements with
//! upload process specific functionality.
//=====================================================================================
struct RealityDataFileUpload : public RealityDataFileTransfer
    {
public:
    RealityDataFileUpload(BeFileName filename, BeFileName root, Utf8String azureServer, size_t index) : 
        m_chunkSize(CHUNK_SIZE), m_chunkStop(0), m_chunkNumber(0), m_moreToSend(true) 
        {
        m_azureServer = azureServer;
        m_index = index;
        m_filename = filename.GetNameUtf8();
        m_transferProgress = 0;
        nbRetry = 0;
        m_validRequestString = false;
        Utf8String fileFromRoot = filename.GetNameUtf8();
        fileFromRoot.ReplaceAll(root.GetNameUtf8().c_str(), "");
        m_fileUrl = "/";
        m_fileUrl.append(fileFromRoot);
        m_fileUrl.ReplaceAll("\\","/");

        m_requestType = HttpRequestType::PUT_Request;

        filename.GetFileSize(m_fileSize);
        }

    REALITYDATAPLATFORM_EXPORT void ReadyFile() override
        {
        BeFileStatus status = m_fileStream.Open(m_filename, BeFileAccess::Read);
        BeAssert(status == BeFileStatus::Success);

        m_transferProgress = 0;

        m_chunkSize = CHUNK_SIZE;
        m_singleChunk = m_fileSize < m_chunkSize;

        if(!m_singleChunk)
            m_blockList = "<?xml version=\"1.0\" encoding=\"utf-8\"?><BlockList>";
        }

    REALITYDATAPLATFORM_EXPORT void Retry() override;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetHttpRequestString() const override
        {
        
        m_requestWithToken = RealityDataFileTransfer::GetHttpRequestString();

        if(!m_singleChunk)
            {
            if(m_moreToSend)
                {
                m_requestWithToken.append("&comp=block&blockid=");
                m_requestWithToken.append(m_chunkNumberString);
                }
            else
                {
                m_requestWithToken.append("&comp=blocklist");
                }
            }

        return m_requestWithToken;
        };

    REALITYDATAPLATFORM_EXPORT void SetChunkSize(uint64_t chunkSize) { m_chunkSize = chunkSize; }

    REALITYDATAPLATFORM_EXPORT bool FinishedSending(); 

    REALITYDATAPLATFORM_EXPORT uint64_t GetMessageSize() { return m_chunkSize; }

    REALITYDATAPLATFORM_EXPORT Utf8String GetBlockList() { return m_blockList; }
    
    REALITYDATAPLATFORM_EXPORT bool IsSingleChunk() { return m_singleChunk; }
    
    REALITYDATAPLATFORM_EXPORT size_t OnReadData(void* buffer, size_t size);

    REALITYDATAPLATFORM_EXPORT void UpdateTransferedSize() override;

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    mutable bool            m_moreToSend;
    mutable bool            m_singleChunk;

    uint64_t                m_chunkSize;
    uint64_t                m_chunkStop;
    uint32_t                m_chunkNumber;
    Utf8String              m_chunkNumberString;

    Utf8String              m_blockList;
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason              02/2017
//! RealityDataFileDownload
//! A specialisation of the RealityDataFileTransfer class that complements with
//! download process specific functionality.
//=====================================================================================
struct RealityDataFileDownload : public RealityDataFileTransfer
{
public:
    RealityDataFileDownload(BeFileName filename, BeFileName root, Utf8String azureServer, size_t index) :
        iAppend(0)
        {
        m_azureServer = azureServer;
        m_index = index;
        m_filename = filename.GetNameUtf8();
        m_transferProgress = 0;
        nbRetry = 0;
        m_validRequestString = false;
        Utf8String fileFromRoot = filename.GetNameUtf8();
        fileFromRoot.ReplaceAll(root.GetNameUtf8().c_str(), "");
        m_fileUrl = "/";
        m_fileUrl.append(fileFromRoot);
        m_fileUrl.ReplaceAll("\\", "/");

        m_requestType = HttpRequestType::GET_Request;

        filename.GetFileSize(m_fileSize);
        }

    REALITYDATAPLATFORM_EXPORT void Retry() override;

    size_t                  iAppend;

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };
END_BENTLEY_REALITYPLATFORM_NAMESPACE



//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//! Callback methods used by the upload/download process.
//=====================================================================================
static size_t CurlReadDataCallback(void* buffer, size_t size, size_t count, RealityDataFileUpload* request)
    {
    return request->OnReadData(buffer, size * count);
    }

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
    AzureHandshake* handshake = (AzureHandshake*)userp;
    if (handshake != nullptr)
        handshake->GetJsonResponse().append((char*)contents, size * nmemb);
    else
        ((Utf8String*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
    }

static size_t DownloadWriteCallback(void *buffer, size_t size, size_t nmemb, void *pClient)
    {
    if (NULL == pClient)
        return 0;

    RealityDataFileDownload *fileDown = (RealityDataFileDownload *)pClient;
    if (!(fileDown->GetFileStream().IsOpen()))
        {
        if (fileDown->iAppend)
            {
            if (fileDown->GetFileStream().Open(fileDown->GetFilename().c_str(), BeFileAccess::Write) != BeFileStatus::Success)
                return 0;   // failure, can't open file to write
            }
        else
            {
            if (fileDown->GetFileStream().Create(fileDown->GetFilename().c_str(), true) != BeFileStatus::Success)
                return 0;   // failure, can't open file to write
            }
        }
    fileDown->iAppend += nmemb;
    uint32_t byteWritten;
    if (fileDown->GetFileStream().Write(&byteWritten, buffer, (uint32_t)(size*nmemb)) != BeFileStatus::Success)
        byteWritten = 0;

    return byteWritten;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8StringCR RealityDataUrl::GetServerName() const { return RealityDataService::GetServerName(); }

Utf8StringCR RealityDataUrl::GetVersion() const { return RealityDataService::GetWSGProtocol(); }

Utf8StringCR RealityDataUrl::GetSchema() const { return RealityDataService::GetSchemaName(); }

Utf8StringCR RealityDataUrl::GetRepoId() const { return RealityDataService::GetRepoName(); }


//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataEnterpriseStatRequest::_PrepareHttpRequestStringAndPayload() const
{
    m_serverName = RealityDataService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetRepoName());
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetSchemaName());
    m_httpRequestString.append("/EnterpriseStat/");
    m_httpRequestString.append(m_id);
}

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataDocumentContentByIdRequest::ChangeInstanceId(Utf8String instanceId)
    {
    m_id = instanceId;
    m_validRequestString = false;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataDocumentContentByIdRequest::GetAzureRedirectionRequestUrl() const
    {
    if(m_handshakeRequest == nullptr)
        {
        bvector<Utf8String> lines;
        BeStringUtilities::Split(m_id.c_str(), "~", lines);
        Utf8String root = lines[0];

        m_handshakeRequest = new AzureHandshake(root, false);
        }

    RealityDataService::RequestToJSON((RealityDataUrl*)m_handshakeRequest, m_handshakeRequest->GetJsonResponse());
    if (m_handshakeRequest->ParseResponse(m_azureServer, m_azureToken, m_azureTokenTimer) == BentleyStatus::SUCCESS)
        m_allowAzureRedirection = true;
           
    }

/*void RealityDataDocumentContentByIdRequest::SetAzureRedirectionUrlToContainer(Utf8String azureContainerUrl)
    {
    m_AzureRedirectionURL = azureContainerUrl;
    m_AzureRedirected = true;
    }*/
//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bool RealityDataDocumentContentByIdRequest::IsAzureBlobRedirected() { return m_AzureRedirected; }

void RealityDataDocumentContentByIdRequest::SetAzureRedirectionPossible(bool possible) { m_allowAzureRedirection = possible; }

bool RealityDataDocumentContentByIdRequest::IsAzureRedirectionPossible() { return m_allowAzureRedirection; }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String RealityDataFilterCreator::FilterByClassification(int classification)
    { 
    //$filter=
    Utf8String filter = "Class+eq+";
    filter += Utf8PrintfString("%lu", classification);
    return filter;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String RealityDataFilterCreator::FilterBySize(double minSize, double maxSize)
    {
    //$filter=
    Utf8String filter = "Size+ge+";
    filter += Utf8PrintfString("%f", minSize);
    filter.append("and+Size+le+");
    filter += Utf8PrintfString("%f", maxSize);
    return filter;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

    filter += Utf8PrintfString("%lu", coordSys);
    filter.append("\"}");

    return filter;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String RealityDataFilterCreator::FilterByOwner(Utf8String owner)
    {
    Utf8String filter = "OwnedBy+eq+'";
    filter.append(owner);
    filter.append("'");
    return filter;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String RealityDataFilterCreator::FilterByCreationDate(DateTime minDate, DateTime maxDate)
    {
    Utf8String filter = "CreatedTimestamp+ge+'";
    filter.append(Utf8String(minDate.ToString()));
    filter.append("'+and+CreatedTimestamp+le+'");
    filter.append(Utf8String(maxDate.ToString()));
    filter.append("'");

    return filter;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String RealityDataFilterCreator::FilterByModificationDate(DateTime minDate, DateTime maxDate)
    {
    Utf8String filter = "ModifiedTimestamp+ge+'";
    filter.append(Utf8String(minDate.ToString()));
    filter.append("'+and+ModifiedTimestamp+le+'");
    filter.append(Utf8String(maxDate.ToString()));
    filter.append("'");

    return filter;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String RealityDataFilterCreator::FilterVisibility(RealityDataBase::Visibility visibility)
    {
    Utf8String filter = "Visibility+eq+'";

    Utf8String value = RealityDataBase::GetTagFromVisibility(visibility);

    filter.append(value);
    filter.append("'");

    return filter;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String RealityDataFilterCreator::FilterByType(Utf8String types)
    {
    Utf8String filter = "Type+eq+'";
    filter.append(types);
    filter.append("'");
    return filter;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String RealityDataFilterCreator::FilterByDataset(Utf8String dataset)
    {
    Utf8String filter = "Dataset+eq+'";
    filter.append(dataset);
    filter.append("'");

    return filter;
    }


//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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


//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8StringCR RealityDataPagedRequest::GetServerName() const { return RealityDataService::GetServerName(); }
Utf8StringCR RealityDataPagedRequest::GetVersion() const { return RealityDataService::GetWSGProtocol(); }
Utf8StringCR RealityDataPagedRequest::GetSchema() const { return RealityDataService::GetSchemaName(); }
Utf8StringCR RealityDataPagedRequest::GetRepoId() const { return RealityDataService::GetRepoName(); }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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
    m_httpRequestString.append("$skip=");
    m_httpRequestString += Utf8PrintfString("%u", m_startIndex);
    m_httpRequestString.append("&$top=");
    m_httpRequestString += Utf8PrintfString("%u", m_pageSize);
}

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataPagedRequest::SortBy(RealityDataField field, bool ascending)
    {
    Utf8String order = "$orderby=";
    switch(field)
        {
    case RealityDataField::Id:
        order.append("Id");
        break;
    case RealityDataField::EnterpriseId:
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
    case RealityDataField::Visibility:
        order.append("Visibility");
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataPagedRequest::SetFilter(Utf8StringCR filter) { m_filter = filter; }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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
    m_httpRequestString += Utf8PrintfString("%u", m_startIndex);
    m_httpRequestString.append("&$top=");
    m_httpRequestString += Utf8PrintfString("%u", m_pageSize);
}

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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
    m_httpRequestString += Utf8PrintfString("%u", m_startIndex);
    m_httpRequestString.append("&$top=");
    m_httpRequestString += Utf8PrintfString("%u", m_pageSize);

    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
AllRealityDataByRootId::AllRealityDataByRootId(Utf8StringCR rootId) : m_marker("")
    {
    m_validRequestString = false; 
    m_handshakeRequest = nullptr;

    Utf8String id = rootId;
    id.ReplaceAll("\\", "/");
    id.ReplaceAll("~2F", "/");
    bvector<Utf8String> parts;
    BeStringUtilities::Split(id.c_str(), "/", parts);

    m_id = parts[0];
    m_filter = id;
    id = parts[0];
    id.append("/");
    m_filter.ReplaceAll(id.c_str(),"");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void AllRealityDataByRootId::_PrepareHttpRequestStringAndPayload() const
    {
    //GetAzureRedirectionRequestUrl();
    m_httpRequestString = m_azureServer;
    m_httpRequestString.append("?");
    m_httpRequestString.append(m_azureToken);
    m_httpRequestString.append("&restype=container&comp=list");
    if(m_marker.length() > 0)
        {
        m_httpRequestString.append("&marker=");
        m_httpRequestString.append(m_marker);
        }

    m_validRequestString = true;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataServiceTransfer::~RealityDataServiceTransfer()
    {
    for (int i = 0; i < m_filesToTransfer.size(); i++)
        delete m_filesToTransfer[i];

    delete m_handshakeRequest;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bool RealityDataFileUpload::FinishedSending()
    { 
    if(!m_singleChunk)
        return !m_moreToSend; 
    else
        return (m_transferProgress >= m_fileSize);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataFileUpload::UpdateTransferedSize()//uint32_t amount, void* ptr)
    {
    if(m_transferProgress < m_fileSize - 1)
        {
        uint64_t uploadStep = m_transferProgress + m_chunkSize;
        if(m_fileSize > uploadStep)
            {
        m_chunkStop = uploadStep;
            }
        else
            {
            m_chunkStop = m_fileSize;
            m_chunkSize = m_fileSize - m_transferProgress;
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
        m_transferProgress = m_chunkStop;
        ++m_chunkNumber;
        }
    else if (!m_singleChunk)
        {
        m_moreToSend = false;
        m_blockList.append("</BlockList>");
        m_chunkSize = m_blockList.length();
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataFileUpload::Retry()
    {
    CloseFile();
    m_chunkSize = CHUNK_SIZE;
    ReadyFile();
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataFileDownload::Retry()
    {
    iAppend = 0;
    CloseFile();
    ReadyFile();
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataFileDownload::_PrepareHttpRequestStringAndPayload() const
    {
    m_httpRequestString = m_azureServer;
    m_httpRequestString.append(m_fileUrl);
    m_httpRequestString.append("?");
    m_httpRequestString.ReplaceAll("//", "/"); 
    m_validRequestString = true;

    m_requestHeader.clear();
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void AzureHandshake::_PrepareHttpRequestStringAndPayload() const
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
    m_httpRequestString.append("/FileAccess.FileAccessKey?$filter=Permissions+eq+");
    if(m_isWrite)
        m_httpRequestString.append("'Write'");
    else
        m_httpRequestString.append("'Read'");
    m_httpRequestString.append("&api.singleurlperinstance=true");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void TransferReport::ToXml(Utf8StringR report)
    {
    BeXmlWriterPtr writer = BeXmlWriter::Create();
    BeAssert(writer.IsValid());
    writer->SetIndentation(2);

    writer->WriteElementStart("RealityDataService_Report");
        {
        writer->WriteAttribute("Date", Utf8String(DateTime::GetCurrentTimeUtc().ToString()).c_str());

        for (int i = 0; i < results.size(); ++i)
            {
            writer->WriteElementStart("File");
                {
                TransferResult* tr = results[i];
                writer->WriteAttribute("FileName", tr->name.c_str());
                writer->WriteAttribute("timeSpent", Utf8PrintfString("%d", (long)tr->timeSpent).c_str());
                writer->WriteAttribute("CURLcode",  Utf8PrintfString("%u", tr->errorCode).c_str());
                writer->WriteAttribute("progress", Utf8PrintfString("%u", tr->progress).c_str());
                }
            writer->WriteElementEnd();
            }
        }
    writer->WriteElementEnd();
    writer->ToString(report);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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
        case RealityDataField::EnterpriseId:
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
        case RealityDataField::Visibility:
            propertyString.append("\"Visibility\" : \"");
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
BentleyStatus AzureHandshake::ParseResponse(Utf8StringR azureServer, Utf8StringR azureToken, int64_t& tokenTimer)
    {
    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(m_jsonResponse, instances);

    Json::Value instance;
    
    if(instances.isMember("instances") && !instances["instances"][0].isNull() && instances["instances"][0].isMember("properties") && !instances["instances"][0]["properties"].isNull())
        instance = instances["instances"][0]["properties"];
    
    if(instance.isMember("Url") && !instance["Url"].isNull())
        {
        Utf8String url = instance["Url"].asString();
        bvector<Utf8String> parts;
        BeStringUtilities::Split(url.c_str(), "\?", parts);
        //https://realityblobdeveussa01.blob.core.windows.net/cc5421e5-a80e-469f-a459-8c76da351fe5?sv=2015-04-05&sr=c&sig=6vtz14nV4FsCidf9XCWm%2FAS48%2BJozxk3zpd1FKwUmnI%3D&se=2017-02-10T15%3A36%3A43Z&sp=r
        azureServer = parts[0];
        azureToken = parts[1];

        DateTime tokenExpiry = DateTime::GetCurrentTimeUtc();
        parts.clear();
        BeStringUtilities::Split(azureToken.c_str(), "&", parts);
        for(Utf8String arg : parts)
            {
            if(arg.StartsWith("se=")) // se=2017-03-01T16%3A21%3A06Z
                {
                arg.ReplaceAll("se=",""); // 2017-03-01T16%3A21%3A06Z
                arg.ReplaceAll("%3A", ":"); // 2017-03-01T16:21:06Z
                DateTime::FromString(tokenExpiry, arg.c_str());
                break;
                }
            }

        tokenExpiry.ToUnixMilliseconds(tokenTimer);
        
        tokenTimer -= (10 * 60 * 1000); // renew token 10 minutes before it expires

        return BentleyStatus::SUCCESS;
        }
    else
        return BentleyStatus::ERROR;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
TransferReport* RealityDataServiceTransfer::Perform()
    {
    m_currentTransferedAmount = 0;
    m_progress = 0.0;

    m_report = TransferReport();
    // we can optionally limit the total amount of connections this multi handle uses 
    curl_multi_setopt(m_pCurlHandle, CURLMOPT_MAXCONNECTS, MAX_NB_CONNECTIONS);

    m_curEntry = 0;

    for (int i = 0; i < min(MAX_NB_CONNECTIONS, (int)m_filesToTransfer.size()); ++i)
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

                RealityDataFileTransfer *fileTrans = (RealityDataFileTransfer *)pClient;
                RealityDataFileUpload *fileUp = (RealityDataFileUpload *)pClient;
                
                // Retry on error
                if (msg->data.result == 56 || msg->data.result == 28)     // Recv failure, try again
                    {
                    if (fileTrans != nullptr && fileTrans->nbRetry < 5)
                        {
                        ++fileTrans->nbRetry;
                        fileTrans->Retry();
                        if(fileUp != nullptr)
                            UpdateTransferAmount((int64_t)fileTrans->GetTransferedSize() * -1);
                        SetupCurlforFile((RealityDataUrl*)(fileTrans), 0);
                        still_running++;
                        }
                    else
                        {
                        // Maximun retry done, return error.
                        ReportStatus((int)fileTrans->m_index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
                        }
                    }
                else if(fileTrans != nullptr)
                    {
                    if(msg->data.result == CURLE_OK)
                        {
                        if(fileUp != nullptr && !fileUp->FinishedSending())
                            {
                            if(m_pProgressFunc && UpdateTransferAmount((int64_t)fileUp->GetMessageSize()))
                                m_pProgressFunc(fileUp->GetFilename(), ((double)fileUp->GetTransferedSize()) / fileUp->GetFileSize(), m_progress);
                            SetupCurlforFile(fileUp, 0);
                            still_running++;
                            }
                        else
                            {
                            if (m_pProgressFunc) 
                                {
                                if(fileUp!= nullptr)
                                    UpdateTransferAmount((int64_t)fileUp->GetMessageSize());
                                else
                                    UpdateTransferAmount(1);
                                m_pProgressFunc(fileTrans->GetFilename(), 1.0, m_progress);
                                }
                            ReportStatus((int)fileTrans->m_index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
                            fileUp->CloseFile();
                            }  
                        }
                    else
                        ReportStatus((int)fileTrans->m_index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
                    }
                           
                    
                curl_multi_remove_handle(m_pCurlHandle, msg->easy_handle);
                curl_easy_cleanup(msg->easy_handle);
                }
            else
                {
                ReportStatus(-1, NULL, msg->msg, "CurlMsg failed");
                }

            if (m_curEntry < (int)m_filesToTransfer.size() && still_running < MAX_NB_CONNECTIONS)
                {
                if (SetupNextEntry())
                    still_running++;
                }
            }

        } while (still_running);

    return &m_report;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bool RealityDataServiceTransfer::SetupNextEntry()
    {
    if (NULL != m_pHeartbeatFunc && m_pHeartbeatFunc() != 0)
        return false;

    if (m_curEntry < (int)m_filesToTransfer.size())
        {
        RealityDataFileTransfer* fTrans = (RealityDataFileTransfer*)m_filesToTransfer[m_curEntry];
        fTrans->ReadyFile();
        SetupCurlforFile((RealityDataUrl*)(fTrans), 0);
        ++m_curEntry;
        }
    else
        return false;

    return true;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataServiceTransfer::SetupCurlforFile(RealityDataUrl* request, int verifyPeer)
    {
    // If cancel requested, don't queue new files
    if (NULL != m_pHeartbeatFunc && m_pHeartbeatFunc() != 0)
        return;

    int code = 2; //BodyNoToken
    AzureHandshake* handshake = dynamic_cast<AzureHandshake*>(request);
    RealityDataFileTransfer* fileTransfer = dynamic_cast<RealityDataFileTransfer*>(request);
    if (handshake != nullptr)
        code = 0;
    else if(fileTransfer != nullptr)
        {
        fileTransfer->SetAzureToken(GetAzureToken());
        fileTransfer->UpdateTransferedSize();
        }
    else
        return; //unexpected request

    RealityDataFileUpload* fileUpload = dynamic_cast<RealityDataFileUpload*>(request);
    RealityDataFileDownload* fileDownload = dynamic_cast<RealityDataFileDownload*>(request);

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
        else if (fileDownload != nullptr)
            {
            /* Define our callback to get called when there's data to be written */
            curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, DownloadWriteCallback);
            /* Set a pointer to our struct to pass to the callback */
            curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fileDownload);
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataServiceTransfer::ReportStatus(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    if(m_onlyReportErrors && ErrorCode == static_cast<int>(CURLE_OK))
        return;

    if (m_pStatusFunc)
        m_pStatusFunc(index, pClient, ErrorCode, pMsg);

    RealityDataFileTransfer* pEntry = (RealityDataFileTransfer*)pClient;

    if(pEntry == nullptr)
        return;

    TransferResult* tr = new TransferResult();
    tr->errorCode = ErrorCode;
    tr->progress = (100 * pEntry->GetTransferedSize() / pEntry->GetFileSize());
    tr->timeSpent = std::time(nullptr) - pEntry->GetStartTime();
    tr->name = pEntry->GetFilename();
    m_report.results.push_back(tr);

    delete pEntry;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String RealityDataServiceTransfer::GetAzureToken()
    {
    int64_t currentTime; 
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);
    if ((m_azureTokenTimer - currentTime) < (0))
        {
        RealityDataService::RequestToJSON((RealityDataUrl*)m_handshakeRequest, m_handshakeRequest->GetJsonResponse());
        if(m_handshakeRequest->ParseResponse(m_azureServer, m_azureToken, m_azureTokenTimer) != BentleyStatus::SUCCESS)
            ReportStatus(0, nullptr, -1, "Failure retrieving Azure token");
        }
    return m_azureToken;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bool RealityDataServiceTransfer::UpdateTransferAmount(int64_t transferedAmount)
    {
    m_currentTransferedAmount += transferedAmount;
    m_progress = ((double)m_currentTransferedAmount) / m_fullTransferSize;
    bool sendProgressCallback = false;
    if(m_progress > m_progressThreshold)
        {
        sendProgressCallback = true;
        m_progressThreshold += m_progressStep;
        }
    return sendProgressCallback;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataServiceUpload::RealityDataServiceUpload(BeFileName uploadPath, Utf8String id, Utf8String properties, bool overwrite) : 
    m_overwrite(overwrite)
    { 
    m_id = id;
    m_azureTokenTimer = 0;
    m_progress = 0.0; 
    m_progressStep = 0.01; 
    m_progressThreshold = 0.01;
    m_onlyReportErrors = false; 
    m_currentTransferedAmount = 0;
    m_fullTransferSize = 0;
    m_handshakeRequest = nullptr;

    if(CreateUpload(properties) != BentleyStatus::SUCCESS)
        return;
    m_handshakeRequest = new AzureHandshake(m_id, true);
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
                m_filesToTransfer.push_back(fileUp);
                m_fullTransferSize += fileUp->GetFileSize();
                }
            }
        }
    else if (uploadPath.DoesPathExist())
        {
        fileUp = new RealityDataFileUpload(uploadPath, uploadPath.GetDirectoryName(), m_azureServer, 0);
        m_filesToTransfer.push_back(fileUp);
        m_fullTransferSize = fileUp->GetFileSize();
        }

    m_pCurlHandle = curl_multi_init();
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataServiceDownload::RealityDataServiceDownload(BeFileName targetLocation, Utf8String serverId)
    {
    m_id = serverId;
    m_azureTokenTimer = 0;
    m_progress = 0.0;
    m_progressStep = 0.01;
    m_progressThreshold = 0.01;
    m_onlyReportErrors = false;
    m_currentTransferedAmount = 0;
    m_handshakeRequest = nullptr;

    m_handshakeRequest = new AzureHandshake(m_id, true);
    GetAzureToken();

    AllRealityDataByRootId rdsRequest = AllRealityDataByRootId(m_id);
    RequestStatus status;
    bvector<Utf8String> filesInRepo = RealityDataService::Request(rdsRequest, status);
    BeFileName downloadLocation;
    WString wPath;

    for( int i = 0; i < filesInRepo.size(); ++i)
        {
        downloadLocation = targetLocation;
        BeStringUtilities::Utf8ToWChar(wPath, filesInRepo[i].c_str());
        downloadLocation.AppendToPath(wPath.c_str());

        m_filesToTransfer.push_back(new RealityDataFileDownload(downloadLocation, targetLocation, m_azureServer, i));
        }

    m_fullTransferSize = m_filesToTransfer.size();
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataServiceDownload::RealityDataServiceDownload(Utf8String serverId, bvector<RealityDataFileTransfer*> downloadList)
    {
    m_id = serverId;
    m_azureTokenTimer = 0;
    m_progress = 0.0;
    m_progressStep = 0.01;
    m_progressThreshold = 0.01;
    m_onlyReportErrors = false;
    m_currentTransferedAmount = 0;
    m_handshakeRequest = nullptr;

    m_handshakeRequest = new AzureHandshake(m_id, true);
    GetAzureToken();

    m_filesToTransfer = downloadList;

    m_fullTransferSize = m_filesToTransfer.size();
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bvector<RealityDataPtr> RealityDataService::Request(const RealityDataPagedRequest& request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = PagedRequestToJSON((RealityDataPagedRequest*)(&request), jsonString);

    bvector<RealityDataPtr> entities = bvector<RealityDataPtr>();
    if (status != RequestStatus::SUCCESS)
        { 
        std::cout << "RealityDataPagedRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        }
    else
        {
        RealityConversionTools::JsonToRealityData(jsonString.c_str(), &entities);
        if ((uint8_t)entities.size() < request.GetPageSize())
            status = RequestStatus::NOMOREPAGES;
        }

    return entities;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataService::Request(const RealityDataEnterpriseStatRequest& request, uint64_t* pNbRealityData, uint64_t* pTotalSizeKB, RequestStatus& status)
    {
    Utf8String jsonString;
    status = RequestToJSON((RealityDataUrl*)(&request), jsonString);

    if (status != RequestStatus::SUCCESS)
        {
        std::cout << "RealityDataEnterpriseStatRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        status = RequestStatus::ERROR;
        }

    RealityConversionTools::JsonToEnterpriseStat(jsonString.c_str(), pNbRealityData, pTotalSizeKB);
    }
    
//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bvector<Utf8String> RealityDataService::Request(const AllRealityDataByRootId& request, RequestStatus& status)
    {
    bvector<Utf8String> documents = bvector<Utf8String>();
    request.GetAzureRedirectionRequestUrl();
    int64_t timer = request.GetTokenTimer();

    bool nextMarker;
    int stat = RequestType::BodyNoToken;
    WString value;
    Utf8String utf8Value;
    Utf8String xmlResponse;
    Utf8String filter = request.GetFilter();

    do
        {
        WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());
        xmlResponse = WSGRequest::GetInstance().PerformAzureRequest(request, stat, RealityDataService::GetVerifyPeer());

        BeXmlStatus xmlStatus = BEXML_Success;
        BeXmlReaderPtr reader = BeXmlReader::CreateAndReadFromString(xmlStatus, xmlResponse.c_str());
        BeAssert(reader.IsValid());

        value.clear();

        while (IBeXmlReader::ReadResult::READ_RESULT_Success == (reader->ReadTo(IBeXmlReader::NodeType::NODE_TYPE_Element, "Name", false, nullptr)))
            {
            reader->ReadTo(IBeXmlReader::NodeType::NODE_TYPE_Text, nullptr, false, &value);
            utf8Value = Utf8String(value.c_str());
            if(filter.length() > 0 && utf8Value.Contains(filter))
                documents.push_back(utf8Value);
            }

        nextMarker = false;
        //the previous loop reaches the end of the file, so to find the "NextMarker" element, we need to restart from the top
        reader = BeXmlReader::CreateAndReadFromString(xmlStatus, xmlResponse.c_str()); 
        if((IBeXmlReader::ReadResult::READ_RESULT_Success == (reader->ReadTo(IBeXmlReader::NodeType::NODE_TYPE_Element, "NextMarker", false, nullptr))))
            {
            reader->ReadTo(IBeXmlReader::NodeType::NODE_TYPE_Text, nullptr, false, &value);
            if(value.length() > 0)
                {
                request.SetMarker(Utf8String(value.c_str()));
                nextMarker = true;
                }
            }

        int64_t currentTime;
        DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);
        if(timer < currentTime)
            {
            request.GetAzureRedirectionRequestUrl();
            timer = request.GetTokenTimer();
            }

        }while(nextMarker);

    return documents;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataPtr RealityDataService::Request(const RealityDataByIdRequest& request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = RequestToJSON((RealityDataUrl*)(&request), jsonString);
    
    bvector<RealityDataPtr> entities = bvector<RealityDataPtr>();
    if (status != RequestStatus::SUCCESS)
        {
        std::cout << "RealityDataByIdRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        return nullptr;
        }
    RealityConversionTools::JsonToRealityData(jsonString.c_str(), &entities);

    return entities[0];
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bvector<RealityDataPtr> RealityDataService::Request(const RealityDataListByEnterprisePagedRequest& request, RequestStatus& status)
    {
    Utf8String jsonString;
    status = PagedRequestToJSON((RealityDataPagedRequest*)(&request), jsonString);

    bvector<RealityDataPtr> entities = bvector<RealityDataPtr>();
    if (status != RequestStatus::SUCCESS)
        {
        std::cout << "RealityDataListByEnterprisePagedRequest failed with response" << std::endl;
        std::cout << jsonString << std::endl;
        }
    else
        {
        RealityConversionTools::JsonToRealityData(jsonString.c_str(), &entities);
        if ((uint8_t)entities.size() < request.GetPageSize())
            status = RequestStatus::NOMOREPAGES;
        }

    return entities;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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
        if((uint8_t)relations.size() < request.GetPageSize())
            status = RequestStatus::NOMOREPAGES;
        }

    return relations;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
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