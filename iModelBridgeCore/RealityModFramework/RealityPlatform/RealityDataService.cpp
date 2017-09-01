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

#include "RealityDataServiceInternal.h"

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
        virtual ~RealityDataFileTransfer(){}

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

        REALITYDATAPLATFORM_EXPORT RawServerResponse& GetResponse() { return m_response; }

        size_t                  nbRetry;
        size_t                  m_index;
    protected:

        REALITYDATAPLATFORM_EXPORT virtual void EncodeId() const override
            {
            m_encodedFileUrl = BeStringUtilities::UriEncode(m_fileUrl.c_str());
            }

        Utf8String              m_fileUrl;
        mutable Utf8String      m_encodedFileUrl;
        Utf8String              m_filename;

        BeFile                  m_fileStream;
        uint64_t                m_fileSize;

        uint64_t                m_transferProgress;

        Utf8String              m_azureServer;
        float                   m_progressStep;
        Utf8String              m_azureToken;
        mutable Utf8String      m_requestWithToken;

        time_t                  m_startTime;

        RawServerResponse       m_response;
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
        m_fileUrl = fileFromRoot;
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
    RealityDataFileDownload(BeFileName filename, Utf8String fileUrl, Utf8String azureServer, size_t index, uint64_t fileSize) :
        iAppend(0)
        {
        m_azureServer = azureServer;
        m_index = index;
        m_filename = filename.GetNameUtf8();
        m_fileSize = fileSize;
        m_transferProgress = 0;
        nbRetry = 0;
        m_validRequestString = false;
        m_fileUrl = fileUrl;

        m_requestType = HttpRequestType::GET_Request;
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

static size_t DownloadWriteCallback(void *buffer, size_t size, size_t nmemb, void *pClient)
    {
    if (NULL == pClient)
        return 0;

    RealityDataFileDownload *fileDown = (RealityDataFileDownload *)pClient;
    if (!(fileDown->GetFileStream().IsOpen()))
        {
        if (fileDown->GetFileStream().Open(fileDown->GetFilename().c_str(), BeFileAccess::Write) != BeFileStatus::Success)
            return 0;   // failure, can't open file to write
        }
    fileDown->iAppend += nmemb;
    uint32_t byteWritten;
    if (fileDown->GetFileStream().Write(&byteWritten, buffer, (uint32_t)(size*nmemb)) != BeFileStatus::Success)
        byteWritten = 0;

    return byteWritten;
    }

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
    ((Utf8String*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8StringCR RealityDataUrl::GetServerName() const { return RealityDataService::GetServerName(); }

Utf8StringCR RealityDataUrl::GetVersion() const { return RealityDataService::GetWSGProtocol(); }

Utf8StringCR RealityDataUrl::GetSchema() const { return RealityDataService::GetSchemaName(); }

Utf8StringCR RealityDataUrl::GetRepoId() const { return RealityDataService::GetRepoName(); }

void RealityDataUrl::EncodeId() const 
    {
    m_id.ReplaceAll("/", "~2F");
    m_encodedId = BeStringUtilities::UriEncode(m_id.c_str());
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void RealityDataUrl::_PrepareHttpRequestStringAndPayload() const
    {
    m_serverName = RealityDataService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append(Utf8PrintfString("v%s/Repositories/%s/%s", RealityDataService::GetWSGProtocol(), RealityDataService::GetRepoName(), RealityDataService::GetSchemaName()));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataEnterpriseStatRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/EnterpriseStat/");
    if(!m_encodedId.empty())
        {
        Utf8String date = Utf8PrintfString("%d-%d-%d",m_date.GetYear(), m_date.GetMonth(), m_date.GetDay());
        m_httpRequestString.append(Utf8PrintfString("%s~2F%s", date, m_encodedId));
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataAllEnterpriseStatsRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/EnterpriseStat?extended=true");
    Utf8String date = Utf8PrintfString("&$filter=Date+eq+\'%d-%d-%d\'",m_date.GetYear(), m_date.GetMonth(), m_date.GetDay());
    m_httpRequestString.append(date);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/RealityData/");
    m_httpRequestString.append(m_encodedId);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataDelete::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/RealityData/");
    m_httpRequestString.append(m_encodedId);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataProjectRelationshipByProjectIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append(Utf8PrintfString("/RealityDataRelationship?$filter=ProjectId+eq+'%s'", m_encodedId));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataProjectRelationshipByRealityDataIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append(Utf8PrintfString("/RealityDataRelationship?$filter=RealityDataId+eq+'%s'", m_encodedId));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataFolderByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/Folder/");
    m_httpRequestString.append(m_encodedId);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataDocumentByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/Document/");
    m_httpRequestString.append(m_encodedId);
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
RawServerResponse RealityDataDocumentContentByIdRequest::GetAzureRedirectionRequestUrl() const
    {
    RawServerResponse rawResponse = RawServerResponse();
    if(m_allowAzureRedirection)
        {
        if(m_handshakeRequest == nullptr)
            {
            bvector<Utf8String> lines;
            BeStringUtilities::Split(m_id.c_str(), "~", lines);
            Utf8String root = lines[0];

            m_handshakeRequest = new AzureHandshake(root, false);
            }
        rawResponse = RealityDataService::BasicRequest((RealityDataUrl*)m_handshakeRequest);

        if (rawResponse.status != RequestStatus::BADREQ && m_handshakeRequest->ParseResponse(rawResponse.body, m_azureServer, m_azureToken, m_azureTokenTimer) == BentleyStatus::SUCCESS)
            m_AzureRedirected = true;
        }

    return rawResponse;
    }

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
    if(m_AzureRedirected)
        {
        m_httpRequestString = m_azureServer;
        bvector<Utf8String> parts;
        m_id.ReplaceAll("~2F", "/");
        BeStringUtilities::Split(m_id.c_str(), "/", parts);
        Utf8String Guid = parts[0];
        m_id.ReplaceAll(Guid.c_str(), "");
        EncodeId();
        m_httpRequestString.append(m_encodedId);
        m_httpRequestString.append("\?");
        m_httpRequestString.append(m_azureToken);

        m_validRequestString = true;
        }
    else
        {
        EncodeId();
        RealityDataUrl::_PrepareHttpRequestStringAndPayload();
        m_httpRequestString.append("/Document/");
        m_httpRequestString.append(m_encodedId);
        m_httpRequestString.append("/$file");
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void RealityDataDocumentContentByIdRequest::EncodeId() const
    {
    if(m_AzureRedirected)
        m_encodedId = BeStringUtilities::UriEncode(m_id.c_str());
    else
        {
        m_id.ReplaceAll("/", "~2F");
        m_encodedId = BeStringUtilities::UriEncode(m_id.c_str());
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterByName(Utf8String name)
    {   
    return RDSFilter(Utf8PrintfString("Name+eq+'%s'", BeStringUtilities::UriEncode(name.c_str())));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterByClassification(RealityDataBase::Classification classification)
    {
    return RDSFilter(Utf8PrintfString("Classification+eq+'%s'", BeStringUtilities::UriEncode(RealityDataBase::GetTagFromClassification(classification).c_str())));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterBySize(uint64_t minSize, uint64_t maxSize)
    {
    return RDSFilter(Utf8PrintfString("Size+ge+%u+and+Size+le+%u", minSize, maxSize));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterSpatial(bvector<GeoPoint2d> area, uint64_t coordSys)
    {   
    return RDSFilter(Utf8PrintfString("polygon=%s", RealityDataBase::FootprintToRDSString(area, Utf8PrintfString("%lu", coordSys))));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterByOwner(Utf8String owner)
    {
    return RDSFilter(Utf8PrintfString("OwnedBy+eq+'%s'", BeStringUtilities::UriEncode(owner.c_str())));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterByCreationDate(DateTime minDate, DateTime maxDate)
    {
    return RDSFilter(Utf8PrintfString("CreatedTimestamp+ge+'%s'+and+CreatedTimestamp+le+'%s'", minDate.ToString(), maxDate.ToString()));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterByModificationDate(DateTime minDate, DateTime maxDate)
    {
    return RDSFilter(Utf8PrintfString("ModifiedTimestamp+ge+'%s'+and+ModifiedTimestamp+le+'%s'", minDate.ToString(), maxDate.ToString()));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterVisibility(RealityDataBase::Visibility visibility)
    {
    return RDSFilter(Utf8PrintfString("Visibility+eq+'%s'", RealityDataBase::GetTagFromVisibility(visibility)));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterByResolution(double resMin, double resMax, bool filterOutUnspecified)
    {
    return RDSFilter(Utf8PrintfString("ResolutionInMeters+ge+'%f'+and+ResolutionInMeters+le+'%f'", resMin, resMax));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterByAccuracy(double accuracyMin, double accuracyMax, bool filterOutUnspecified)
    {
    return RDSFilter(Utf8PrintfString("AccuracyInMeters+ge+'%f'+and+AccuracyInMeters+le+'%f'", accuracyMin, accuracyMax));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterByType(Utf8String types)
    {
    return RDSFilter(Utf8PrintfString("Type+eq+'%s'", BeStringUtilities::UriEncode(types.c_str())));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterByDataset(Utf8String dataset)
    {
    return RDSFilter(Utf8PrintfString("Dataset+eq+'%s'", BeStringUtilities::UriEncode(dataset.c_str())));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterByGroup(Utf8String group)
    {   
    return RDSFilter(Utf8PrintfString("Group+eq+'%s'", BeStringUtilities::UriEncode(group.c_str())));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterRelationshipByRealityDataId(Utf8String realityDataId)
    {
    return RDSFilter(Utf8PrintfString("RealityDataId+eq+'%s'", BeStringUtilities::UriEncode(realityDataId.c_str())));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::FilterRelationshipByProjectId(Utf8String projectId)
    {
    return RDSFilter(Utf8PrintfString("ProjectId+eq+'%s'", BeStringUtilities::UriEncode(projectId.c_str())));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              04/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::GroupFiltersAND(bvector<RDSFilter> filters)
{
    Utf8String filter = "";//"(";
    filter.append(filters[0].ToString());
    for (size_t i = 1; i < filters.size(); i++)
    {
        filter.append("+and+");
        filter.append(filters[i].ToString());
    }

    //filter.append(")");
    return RDSFilter(filter);
}

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              04/2017
//=====================================================================================
RDSFilter RealityDataFilterCreator::GroupFiltersOR(bvector<RDSFilter> filters)
{
    Utf8String filter = "";//"(";
    filter.append(filters[0].ToString());
    for (size_t i = 1; i < filters.size(); i++)
    {
        filter.append("+or+");
        filter.append(filters[i].ToString());
    }

    //filter.append(")");
    return RDSFilter(filter);
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
void RealityDataPagedRequest::_PrepareBaseRequestString() const
    {
    m_serverName = RealityDataService::GetServerName();
    WSGURL::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("v");
    m_httpRequestString.append(RealityDataService::GetWSGProtocol());
    m_httpRequestString.append("/Repositories/");
    m_httpRequestString.append(RealityDataService::GetRepoName());
    m_httpRequestString.append("/");
    m_httpRequestString.append(RealityDataService::GetSchemaName());
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataPagedRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataPagedRequest::_PrepareBaseRequestString();
    m_httpRequestString.append("/RealityData?");

    if (m_filter.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("$filter=%s&", m_filter));
    if (m_order.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("%s&", m_order));

    m_httpRequestString.append(Utf8PrintfString("$skip=%u&$top=%u", m_startIndex, m_pageSize));
    
    if (m_query.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("&query=%s", m_query));

    if (m_project.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("&project=%s", m_project));
    }

static bmap<RealityDataField, Utf8String> CreatePropertyMap()
    {
    bmap<RealityDataField, Utf8String> m = bmap<RealityDataField, Utf8String>();
    m.Insert(RealityDataField::Id, "Id");
    m.Insert(RealityDataField::OrganizationId, "OrganizationId");
    m.Insert(RealityDataField::UltimateId, "UltimateId");
    m.Insert(RealityDataField::UltimateSite, "UltimateSite");
    m.Insert(RealityDataField::ContainerName, "ContainerName");
    m.Insert(RealityDataField::DataLocationGuid, "DataLocationGuid");
    m.Insert(RealityDataField::Name, "Name");
    m.Insert(RealityDataField::Dataset, "Dataset");
    m.Insert(RealityDataField::Description, "Description");
    m.Insert(RealityDataField::RootDocument, "RootDocument");
    m.Insert(RealityDataField::Size, "Size");
    m.Insert(RealityDataField::Classification, "Classification");
    m.Insert(RealityDataField::Type, "Type");
    m.Insert(RealityDataField::Streamed, "Streamed");
    m.Insert(RealityDataField::Footprint, "Footprint");
    m.Insert(RealityDataField::ThumbnailDocument, "ThumbnailDocument");
    m.Insert(RealityDataField::MetadataUrl, "MetadataUrl");
    m.Insert(RealityDataField::Copyright, "Copyright");
    m.Insert(RealityDataField::TermsOfUse, "TermsOfUse");
    m.Insert(RealityDataField::ResolutionInMeters, "ResolutionInMeters");
    m.Insert(RealityDataField::AccuracyInMeters, "AccuracyInMeters");
    m.Insert(RealityDataField::Visibility, "Visibility");
    m.Insert(RealityDataField::Listable, "Listable");
    m.Insert(RealityDataField::CreatedTimestamp, "CreatedTimestamp");
    m.Insert(RealityDataField::ModifiedTimestamp, "ModifiedTimestamp");
    m.Insert(RealityDataField::OwnedBy, "OwnedBy");
    m.Insert(RealityDataField::Group, "Group");

    return m;
    }

static bmap<RealityDataField, Utf8String> s_propertyMap = CreatePropertyMap();

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataPagedRequest::SortBy(RealityDataField field, bool ascending)
    {
    auto searchField = s_propertyMap.find(field);
    if(searchField != s_propertyMap.end())
        {
        Utf8String order = "$orderby=";

        order.append(searchField->second);

        if(ascending)
            order.append("+asc");
        else
            order.append("+desc");

        m_order = order;
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataPagedRequest::SetFilter(const RDSFilter& filter) 
    { 
    m_filter = filter.ToString();
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void RealityDataPagedRequest::SetQuery(Utf8StringCR query) 
    { 
    m_query = BeStringUtilities::UriEncode(query.c_str());
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void RealityDataPagedRequest::SetProject(Utf8StringCR project) 
    { 
    m_project = BeStringUtilities::UriEncode(project.c_str());
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void RealityDataPagedRequest::EncodeId() const
    {
    m_id.ReplaceAll("/", "~2F");
    m_encodedId = BeStringUtilities::UriEncode(m_id.c_str());
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//! DEPRECATED
//=====================================================================================
void RealityDataListByOrganizationPagedRequest::_PrepareHttpRequestStringAndPayload() const
    {
    assert(0 && "This function is deprecated, please use RealityDataListByUltimateIdPagedRequest");
}

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              08/2017
//=====================================================================================
void RealityDataListByUltimateIdPagedRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataPagedRequest::_PrepareBaseRequestString();
    m_httpRequestString.append("/RealityData?$filter=UltimateId+eq+'");

    if (m_encodedId.length() == 0)
        {
        Utf8String token = CurlConstructor().GetToken();
        token.ReplaceAll("Authorization: Token ", "");
        Utf8String decodedToken = Base64Utilities::Decode(token);

        const char* charstring = decodedToken.c_str();
        Utf8String keyword = "ultimateid";
        const char* attributePosition = strstr(charstring, keyword.c_str());
        keyword = "<saml:AttributeValue>";
        const char* valuePosition = strstr(attributePosition, keyword.c_str());
        valuePosition += keyword.length();
        Utf8String idString = Utf8String(valuePosition);

        bvector<Utf8String> lines;
        BeStringUtilities::Split(idString.c_str(), "< ", lines);
        m_id = lines[0];
        EncodeId();
        }

    m_httpRequestString.append(Utf8PrintfString("%s'", m_encodedId));
    if (m_filter.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("+and+%s", m_filter)); // TODO: and/or?
    if (m_order.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("&%s", m_order));

    m_httpRequestString.append(Utf8PrintfString("&$skip=%u&$top=%u", m_startIndex, m_pageSize));

    if (m_query.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("&query=%s", m_query));

    if (m_project.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("&project=%s", m_project));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataProjectRelationshipByProjectIdPagedRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataPagedRequest::_PrepareBaseRequestString();
    m_httpRequestString.append(Utf8PrintfString("/RealityDataRelationship?$filter=RelationType+eq+'CONNECT-Project'+and+RelatedId+eq+'%s'", m_encodedId));

    if (m_filter.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("+and+%s", m_filter)); // TODO: and/or?
    if (m_order.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("&%s", m_order));

    m_httpRequestString.append(Utf8PrintfString("&$skip=%u&$top=%u", m_startIndex, m_pageSize));

    if (m_query.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("&query=%s", m_query));

    if (m_project.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("&project=%s", m_project));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataProjectRelationshipByRealityDataIdPagedRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataPagedRequest::_PrepareBaseRequestString();
    m_httpRequestString.append(Utf8PrintfString("/RealityDataRelationship?$filter=RelationType+eq+'CONNECT-Project'+and+RealityDataId+eq+'%s'", m_id));

    if (m_filter.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("+and+%s", m_filter)); // TODO: and/or?
    if (m_order.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("&%s", m_order));

    m_httpRequestString.append(Utf8PrintfString("&$skip=%u&$top=%u", m_startIndex, m_pageSize));

    if (m_query.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("&query=%s", m_query));

    if (m_project.length() > 0)
        m_httpRequestString.append(Utf8PrintfString("&project=%s", m_project));
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
AllRealityDataByRootId::AllRealityDataByRootId(Utf8StringCR rootId) : RealityDataDocumentContentByIdRequest(rootId), m_marker("")
    {
    m_validRequestString = false; 
    m_handshakeRequest = nullptr;

    Utf8String id = rootId;
    id.ReplaceAll("\\", "/");
    id.ReplaceAll("~2F", "/");
    bvector<Utf8String> parts;
    BeStringUtilities::Split(id.c_str(), "/", parts);

    m_id = parts[0];
    if(m_id == id)
        m_filter = "";
    else
        {
        m_filter = id;
        id = parts[0];
        id.append("/");
        m_filter.ReplaceAll(id.c_str(),"");
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void AllRealityDataByRootId::_PrepareHttpRequestStringAndPayload() const
    {
    m_httpRequestString = m_azureServer;
    m_httpRequestString.append("?");
    m_httpRequestString.append(m_azureToken);
    m_httpRequestString.append("&restype=container&comp=list");
    if(m_marker.length() > 0)
        {
        m_httpRequestString.append("&marker=");
        m_httpRequestString.append(m_marker);
        }
    if(m_filter.length() > 0)
        {
        Utf8String encodedFilter = BeStringUtilities::UriEncode(m_filter.c_str());
        m_httpRequestString.append("&prefix=");
        m_httpRequestString.append(encodedFilter);
        }

    m_validRequestString = true;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataCreateRequest::RealityDataCreateRequest(Utf8String realityDataId, Utf8String properties)
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
//! @bsimethod                                   Alain.Robert              03/2017
//=====================================================================================
RealityDataCreateRequest::RealityDataCreateRequest(RealityDataCR realityData)
    { 
    m_id = realityData.GetIdentifier(); 
    m_validRequestString = false;

    Utf8String formatedProps = RealityConversionTools::RealityDataToJson(realityData);

    m_requestType = HttpRequestType::POST_Request;
    m_requestPayload = "{\"instance\":{\"instanceId\":\"";
    m_requestPayload.append(m_id);
    m_requestPayload.append("\", \"className\": \"RealityData\",\"schemaName\":\"S3MX\", \"properties\": {");
    m_requestPayload.append(formatedProps);
    m_requestPayload.append("}}}");
    }


//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataCreateRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/RealityData");
    m_requestHeader.push_back("Content-Type: application/json");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataChangeRequest::RealityDataChangeRequest(Utf8String realityDataId, Utf8String properties)
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
//! @bsimethod                                   Alain.Robert              03/2017
//=====================================================================================
RealityDataChangeRequest::RealityDataChangeRequest(RealityDataCR realityData)
    {
    m_id = realityData.GetIdentifier(); 
    m_validRequestString = false;

    Utf8String formatedProps = RealityConversionTools::RealityDataToJson(realityData);

    m_requestType = HttpRequestType::POST_Request;
    m_requestPayload = "{\"instance\":{\"instanceId\":\"";
    m_requestPayload.append(m_id);
    m_requestPayload.append("\", \"className\": \"RealityData\",\"schemaName\":\"S3MX\", \"properties\": {");
    m_requestPayload.append(formatedProps);
    m_requestPayload.append("}}}");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataChangeRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/RealityData/");
    m_httpRequestString.append(m_encodedId);
    m_requestHeader.push_back("Content-Type: application/json");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataRelationshipCreateRequest::RealityDataRelationshipCreateRequest(Utf8String realityDataId, Utf8String projectId)
    {
    m_id = realityDataId;
    m_validRequestString = false;

    m_requestType = HttpRequestType::POST_Request;
    m_requestPayload = "{\"instance\":{\"className\": \"RealityDataRelationship\",\"schemaName\":\"S3MX\", \"properties\": { \"RelationType\" : \"CONNECT-Project\", \"RelatedId\" : \"";
    m_requestPayload.append(projectId);
    m_requestPayload.append("\", \"RealityDataId\": \"");
    m_requestPayload.append(m_id);
    m_requestPayload.append("\"}}}");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataRelationshipCreateRequest::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/RealityDataRelationship");
    m_requestHeader.push_back("Content-Type: application/json");
    }


//=====================================================================================
//! @bsimethod                                   Alain.Robert              03/2017
//=====================================================================================
RealityDataRelationshipDelete::RealityDataRelationshipDelete(Utf8String realityDataId, Utf8String projectId)
    :m_projectId(projectId)
    {
    m_id = realityDataId;
    m_validRequestString = false;

    m_requestType = HttpRequestType::DELETE_Request;
    }

//=====================================================================================
//! @bsimethod                                   Alain.Robert              03/2017
//=====================================================================================
void RealityDataRelationshipDelete::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/RealityDataRelationship/");
    m_httpRequestString.append(m_encodedId);
    m_httpRequestString.append("~2F");
    m_httpRequestString.append(m_projectId);
    m_requestHeader.push_back("Content-Type: application/json");
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataServiceTransfer::~RealityDataServiceTransfer()
    {
    delete m_handshakeRequest;
    for (RealityDataFileTransfer* fileTransfer : m_filesToTransfer)
        delete fileTransfer;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataFileUpload::_PrepareHttpRequestStringAndPayload() const
    {
    m_httpRequestString = m_azureServer;
    Utf8String addon = "";
    if(!m_fileUrl.StartsWith("/"))
        addon.append("/");

    EncodeId();
    addon.append(m_encodedFileUrl);
    addon.append("?");

    m_httpRequestString.append(addon);
    m_validRequestString = true;

    m_requestHeader.clear();
    if(m_fileUrl.EndsWith(".js"))
        m_requestHeader.push_back("Content-Type: application/javascript");
    else if (m_fileUrl.EndsWith(".html"))
        m_requestHeader.push_back("Content-Type: text/html");
    else if (m_fileUrl.EndsWith(".css"))
        m_requestHeader.push_back("Content-Type: text/css");
    
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
void RealityDataFileUpload::UpdateTransferedSize()
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
        m_validRequestString = false;
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
    EncodeId();
    m_httpRequestString = m_azureServer;
    m_httpRequestString.append(m_encodedFileUrl);
    m_httpRequestString.append("?");
    m_validRequestString = true;

    m_requestHeader.clear();
    }

AzureHandshake::AzureHandshake() : m_isWrite(false)
    {
    m_validRequestString = false;
    m_id = "";
    }

AzureHandshake::AzureHandshake(Utf8String sourcePath, bool isWrite) : m_isWrite(isWrite) 
    { 
    m_validRequestString = false; 

    sourcePath.ReplaceAll("~2F", "/");
    bvector<Utf8String> subStr;
    BeStringUtilities::Split(sourcePath.c_str(), "/", subStr);

    m_id = subStr[0];
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void AzureHandshake::_PrepareHttpRequestStringAndPayload() const
    {
    RealityDataUrl::_PrepareHttpRequestStringAndPayload();
    m_httpRequestString.append("/RealityData/");
    m_httpRequestString.append(m_encodedId);
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
void TransferReport::ToXml(Utf8StringR report) const
    {
    BeXmlWriterPtr writer = BeXmlWriter::Create();
    BeAssert(writer.IsValid());
    writer->SetIndentation(2);

    writer->WriteElementStart("RealityDataService_Report");
        {
        writer->WriteAttribute("Date", Utf8String(DateTime::GetCurrentTimeUtc().ToString()).c_str());

        for (size_t i = 0; i < results.size(); ++i)
            {
            writer->WriteElementStart("File");
                {
                TransferResult* tr = results[i];
                writer->WriteAttribute("FileName", tr->name.c_str());
                writer->WriteAttribute("timeSpent", Utf8PrintfString("%lu", tr->timeSpent).c_str());
                writer->WriteAttribute("CURLcode",  Utf8PrintfString("%u", tr->errorCode).c_str());
                writer->WriteAttribute("progress", Utf8PrintfString("%u", tr->progress).c_str());
                if(tr->errorCode != CURLE_OK && tr->response.header.length() > 0)
                    {
                    writer->WriteElementStart("Response");
                    writer->WriteAttribute("ResponseCode", Utf8PrintfString("%u", tr->response.responseCode).c_str());
                    writer->WriteAttribute("Header", Utf8PrintfString("%s", tr->response.header).c_str());
                    writer->WriteElementEnd();
                    }
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
    Utf8String propertyString = "";
    RealityDataField field;
    for(bmap<RealityDataField, Utf8String>::iterator it = properties.begin(); it != properties.end(); it.increment())
        {
        field = it.key();
        if(propertyString.length() > 0)
            propertyString.append(",");
        if(field == RealityDataField::Streamed || field == RealityDataField::Listable || field == RealityDataField::Footprint)
            propertyString.append(Utf8PrintfString("\"%s\" : %s", s_propertyMap[field], properties[field]));
        else
            propertyString.append(Utf8PrintfString("\"%s\" : \"%s\"", s_propertyMap[field], properties[field]));
        }
    
    return propertyString;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
BentleyStatus RealityDataServiceUpload::CreateUpload(Utf8String properties)
    {
    if(m_id.length() == 0)
        {
        RealityDataCreateRequest createRequest = RealityDataCreateRequest(m_id, properties);
        RawServerResponse createResponse = RawServerResponse();
        WSGRequest::GetInstance().PerformRequest(createRequest, createResponse, RealityDataService::GetVerifyPeer());
    
        Json::Value instance(Json::objectValue);
        Json::Reader::Parse(createResponse.body, instance);
        if (!instance["changedInstance"].isNull() && !instance["changedInstance"]["instanceAfterChange"].isNull() && !instance["changedInstance"]["instanceAfterChange"]["instanceId"].isNull())
            {
            m_id = instance["changedInstance"]["instanceAfterChange"]["instanceId"].asString();
            ReportStatus(0, nullptr, -1, Utf8PrintfString("New RealityData created with GUID %s\n", m_id).c_str());
            }
        else
            {
            ReportStatus(0, nullptr, -1, "RealityData creation failed\n");
            ReportStatus(0, nullptr, -1, Utf8PrintfString("with error %s\n", createResponse.body).c_str());
            ReportStatus(0, nullptr, -1, Utf8PrintfString("server code : %lu\n", createResponse.responseCode).c_str());
            return BentleyStatus::ERROR;
            }
        }
    else
        {
        RealityDataByIdRequest* getRequest = new RealityDataByIdRequest(m_id);
        RawServerResponse idResponse;
        if ((idResponse = RealityDataService::BasicRequest((RealityDataUrl*)getRequest)).status == RequestStatus::BADREQ) //file does not exist, need POST Create
            {
            RealityDataCreateRequest createRequest = RealityDataCreateRequest(m_id, properties);
            RawServerResponse createResponse = RawServerResponse();
            WSGRequest::GetInstance().PerformRequest(createRequest, createResponse, RealityDataService::GetVerifyPeer());
            if(createResponse.body.ContainsI("error"))
                {
                ReportStatus(0, nullptr, -1, Utf8PrintfString("Creation Error message : %s\n", createResponse.body).c_str());
                ReportStatus(0, nullptr, -1, Utf8PrintfString("server code : %lu\n", createResponse.responseCode).c_str());
                return BentleyStatus::ERROR;
                }
            if ((idResponse = RealityDataService::BasicRequest((RealityDataUrl*)getRequest)).status == RequestStatus::BADREQ)
                {
                ReportStatus(0, nullptr, -1, "Unable to create RealityData with specified parameters\n");
                ReportStatus(0, nullptr, -1, Utf8PrintfString("server code : %lu\n", idResponse.responseCode).c_str());
                return BentleyStatus::ERROR;
                }
            
            }
        else if (!m_overwrite)
            {
            ReportStatus(0, nullptr, -1, "RealityData with specified GUID already exists on server. Overwrite variable not specified, aborting operation");
            return BentleyStatus::ERROR;
            }
        delete getRequest;
        }

    return BentleyStatus::SUCCESS;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
BentleyStatus AzureHandshake::ParseResponse(Utf8StringCR jsonResponse, Utf8StringR azureServer, Utf8StringR azureToken, int64_t& tokenTimer)
    {
    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(jsonResponse, instances);

    Json::Value instance;
    
    if(instances.isMember("instances") && !instances["instances"][0].isNull() && instances["instances"][0].isMember("properties") && !instances["instances"][0]["properties"].isNull())
        instance = instances["instances"][0]["properties"];
    
    if(instance.isMember("Url") && !instance["Url"].isNull())
        {
        Utf8String url = instance["Url"].asString();
        bvector<Utf8String> parts;
        BeStringUtilities::Split(url.c_str(), "\?", parts);
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
const TransferReport& RealityDataServiceTransfer::Perform()
    {
    m_currentTransferedAmount = 0;
    m_progress = 0.0;

    m_report = TransferReport();

    if(!IsValidTransfer())
        {
        ReportStatus(0, nullptr, -1, "No files to transfer, please verify that the previous steps completed without failure");
        return m_report;
        }

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
            ReportStatus(-1, NULL, mc, "curl_multi_wait() failed\n");
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
                void *pClient;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &pClient);

                RealityDataUrl *request = reinterpret_cast<RealityDataUrl*>(pClient);
                RealityDataFileTransfer *fileTrans = dynamic_cast<RealityDataFileTransfer*>(request);
                RealityDataFileUpload *fileUp = dynamic_cast<RealityDataFileUpload*>(request);

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
                    curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &(fileTrans->GetResponse().responseCode));
                    fileTrans->GetResponse().curlCode = msg->data.result;
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
                            }  
                        }
                    else
                        {
                        ReportStatus((int)fileTrans->m_index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
                        ReportStatus(0, nullptr, -1, Utf8PrintfString("\nServer returned code %ld\n", fileTrans->GetResponse().responseCode).c_str());
                        }
                    }
                           
                    
                curl_multi_remove_handle(m_pCurlHandle, msg->easy_handle);
                curl_easy_cleanup(msg->easy_handle);
                }
            else
                {
                ReportStatus(-1, NULL, msg->msg, "CurlMsg failed\n");
                }

            if (m_curEntry < (int)m_filesToTransfer.size() && still_running < MAX_NB_CONNECTIONS)
                {
                if (SetupNextEntry())
                    still_running++;
                }
            }

        } while (still_running);

    return m_report;
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
void RealityDataServiceTransfer::SetupCurlforFile(RealityDataUrl* request, bool verifyPeer)
    {
    // If cancel requested, don't queue new files
    if (NULL != m_pHeartbeatFunc && m_pHeartbeatFunc() != 0)
        return;

    RealityDataFileTransfer* fileTransfer = dynamic_cast<RealityDataFileTransfer*>(request);
    if(fileTransfer != nullptr)
        {
        fileTransfer->SetAzureToken(GetAzureToken());
        fileTransfer->UpdateTransferedSize();
        }
    else
        return; //unexpected request

    RealityDataFileUpload* fileUpload = dynamic_cast<RealityDataFileUpload*>(request);
    RealityDataFileDownload* fileDownload = dynamic_cast<RealityDataFileDownload*>(request);

    RawServerResponse& rawResponse = fileTransfer->GetResponse();
    rawResponse.clear();
    rawResponse.curlCode = ServerType::Azure;

    CURL *pCurl = PrepareCurl(*request, rawResponse, verifyPeer, nullptr);
    if (pCurl)
        {
        curl_easy_setopt(pCurl, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 0L);
        if (!m_proxyUrl.empty())
            {
            curl_easy_setopt(pCurl, CURLOPT_PROXY, m_proxyUrl.c_str());
            curl_easy_setopt(pCurl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
            if (!m_proxyCreds.empty())
                {
                curl_easy_setopt(pCurl, CURLOPT_PROXYUSERPWD, m_proxyCreds.c_str());
                }
            }
        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 0L);
        
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(pCurl, CURLOPT_PRIVATE, request);

        curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(pCurl, CURLOPT_LOW_SPEED_LIMIT, 1L); // B/s
        curl_easy_setopt(pCurl, CURLOPT_LOW_SPEED_TIME, 60); //60s

        curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, WriteCallback);
        curl_easy_setopt(pCurl, CURLOPT_HEADERDATA, &(rawResponse.header));

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

            if (fileDownload->GetFileStream().Create(fileDownload->GetFilename().c_str(), true) != BeFileStatus::Success)
                {
                ReportStatus(0, nullptr, -1, Utf8PrintfString("\nFailed to create File %s on local machine\nAborting download of this file", fileDownload->GetFilename().c_str()).c_str());
                return;   // failure, can't open file to write
                }
            }

        curl_multi_add_handle((CURLM*)m_pCurlHandle, pCurl);
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataServiceTransfer::ReportStatus(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    RealityDataFileTransfer* pEntry = (RealityDataFileTransfer*)pClient;

    if(!m_onlyReportErrors || ErrorCode != static_cast<int>(CURLE_OK))
        {
        if (m_pStatusFunc)
            m_pStatusFunc(index, pClient, ErrorCode, pMsg);

        if(pEntry == nullptr)
            return;

        TransferResult* tr = new TransferResult();
        tr->errorCode = ErrorCode;
        uint64_t uploadSize = pEntry->GetFileSize();
        if(uploadSize > 0)
            tr->progress = (100 * pEntry->GetTransferedSize() / pEntry->GetFileSize());
        else
            tr->progress = 100;
        tr->timeSpent = std::time(nullptr) - pEntry->GetStartTime();
        tr->name = pEntry->GetFilename();
        tr->response = pEntry->GetResponse();
        m_report.results.push_back(tr);
        }

    if (pEntry == nullptr)
        return;

    m_filesToTransfer[pEntry->m_index] = nullptr;
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
        RawServerResponse rawResponse = RealityDataService::BasicRequest((RealityDataUrl*)m_handshakeRequest);
        if(m_handshakeRequest->ParseResponse(rawResponse.body, m_azureServer, m_azureToken, m_azureTokenTimer) != BentleyStatus::SUCCESS)
            ReportStatus(0, nullptr, -1, "Failure retrieving Azure token\n");
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
//! @bsimethod                                   Spencer.Mason              08/2017
//=====================================================================================
RealityDataServiceUpload::RealityDataServiceUpload
    (BeFileName uploadPath, Utf8String properties, bool overwrite, RealityDataServiceTransfer_StatusCallBack pi_func, 
    bvector<BeFileName> colorList, bool isBlackList) :
    RealityDataServiceUpload(uploadPath, "", properties, overwrite, true, pi_func, colorList, isBlackList)
    {}

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataServiceUpload::RealityDataServiceUpload(BeFileName uploadPath, Utf8String id, Utf8String properties, 
    bool overwrite, bool listable, RealityDataServiceTransfer_StatusCallBack pi_func, bvector<BeFileName> colorList, 
    bool isBlackList) :
    RealityDataServiceTransfer(), m_overwrite(overwrite)
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
    m_pStatusFunc = pi_func;

    if(!listable)
        {
        if(properties.length() > 0)
            properties.append(",");
        properties.append("\"Listable\" : false");
        }

    if (!RealityDataService::AreParametersSet())
        {
        ReportStatus(0, nullptr, -1, "Server, Version, Repository and Schema not set, please use RealityDataService::SetServerComponents before calling this function\n");
        return;
        }

    if(CreateUpload(properties) != BentleyStatus::SUCCESS)
        return;
    m_handshakeRequest = new AzureHandshake(m_id, true);
    GetAzureToken();

    RealityDataFileUpload* fileUp;

    if(uploadPath.DoesPathExist() && uploadPath.IsDirectory()) //path is directory, find all documents
        {
        uploadPath.AppendSeparator();

        BeFileName root(uploadPath);

        uploadPath.AppendToPath(L"*");
        BeFileListIterator fileIt = BeFileListIterator(uploadPath.GetName(), true);

        BeFileName fileName;
        //BeFileListIterator returns the same filenames twice for every subfolder containing it
        // i.e. folder/folder2/test.txt would appear 4 times
        // the bset is used to avoid adding multiple uploads for a single file
        bset<Utf8String> duplicateSet = bset<Utf8String>(); 
        size_t i = 0;
        bool whiteListed;
        bool noColorList = colorList.empty();
        while (fileIt.GetNextFileName(fileName) == BentleyStatus::SUCCESS) 
            {
            if(!uploadPath.IsDirectory() && duplicateSet.find(fileName.GetNameUtf8()) == duplicateSet.end())
                {
                duplicateSet.insert(fileName.GetNameUtf8());
                whiteListed = noColorList || isBlackList;
                for (BeFileName colorFile : colorList)
                    {
                    if(fileName.GetNameUtf8().ContainsI(colorFile.GetNameUtf8()))
                        whiteListed = !isBlackList;
                    }
            
                if(whiteListed)
                    {
                    fileUp = new RealityDataFileUpload(fileName, root, m_azureServer, i++);
                    m_filesToTransfer.push_back(fileUp);
                    m_fullTransferSize += fileUp->GetFileSize();
                    }
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
RealityDataServiceDownload::RealityDataServiceDownload(BeFileName targetLocation, Utf8String serverId, RealityDataServiceTransfer_StatusCallBack pi_func) :
    RealityDataServiceTransfer()
    {
    m_id = serverId;
    m_azureTokenTimer = 0;
    m_progress = 0.0;
    m_progressStep = 0.01;
    m_progressThreshold = 0.01;
    m_onlyReportErrors = false;
    m_currentTransferedAmount = 0;
    m_handshakeRequest = nullptr;
    m_pStatusFunc = pi_func;

    m_handshakeRequest = new AzureHandshake(m_id, false);
    GetAzureToken();

    AllRealityDataByRootId rdsRequest = AllRealityDataByRootId(m_id);
    RawServerResponse rawResponse = RawServerResponse();
    bvector<bpair<WString, uint64_t>> filesInRepo = RealityDataService::Request(rdsRequest, rawResponse);

    // If no files listed ... something went wrong
    if (filesInRepo.size() == 0)
        rawResponse.status = RequestStatus::BADREQ;

    if(rawResponse.status == RequestStatus::BADREQ)
        {
        ReportStatus(0, nullptr, -1, "Error performing SAS request on Azure server\n");
        return;
        }

    BeFileName downloadLocation;
    WString path;
    WString fileUrl;
    Utf8String utf8FileUrl;
    size_t parts;

    WString root;
    BeStringUtilities::Utf8ToWChar(root, m_id.c_str());
    bvector<WString> folders;
    BeStringUtilities::Split(root.c_str(),L"/", folders);
    WString guid = folders[0];
    guid.append(L"/");
    root.ReplaceAll(guid.c_str(), L""); //remove guid from root

    if ((folders.size() > 1) && !m_id.EndsWith("/")) //if path ends with "/" it is a folder; otherwise, it is a single document
        {
        path = filesInRepo[0].first;

        fileUrl = L"/";
        fileUrl.append(path);
        fileUrl.ReplaceAll(L"\\", L"/");
        BeStringUtilities::WCharToUtf8(utf8FileUrl, fileUrl.c_str());

        downloadLocation = targetLocation;
        downloadLocation.AppendToPath(folders[folders.size()-1].c_str());

        m_filesToTransfer.push_back(new RealityDataFileDownload(downloadLocation, utf8FileUrl, m_azureServer, 0, filesInRepo[0].second));
        }
    else
        {
        for( size_t i = 0; i < filesInRepo.size(); ++i)
            {
            path = filesInRepo[i].first;

            fileUrl = L"/";
            fileUrl.append(path);
            fileUrl.ReplaceAll(L"\\", L"/");
            BeStringUtilities::WCharToUtf8(utf8FileUrl, fileUrl.c_str());

            path.ReplaceAll(root.c_str(), L""); // if user downloader Folder1/Folder2/Data1, it should download to Data1, not Folder1/Folder2/Data1
            parts = path.ReplaceAll(L"/", L"/"); // only way I've found to count occurences in a string, replace if better exists
            if(parts > 0) //if file is in a directory
                {
                downloadLocation = targetLocation;
                downloadLocation.AppendToPath(path.c_str());
                downloadLocation.PopDir();

                if (!downloadLocation.DoesPathExist())
                    BeFileName::CreateNewDirectory(downloadLocation.c_str());
                }

            downloadLocation = targetLocation;
            downloadLocation.AppendToPath(path.c_str());
        
            m_filesToTransfer.push_back(new RealityDataFileDownload(downloadLocation, utf8FileUrl, m_azureServer, i, filesInRepo[i].second));
            }
        }

    m_fullTransferSize = m_filesToTransfer.size();

    m_pCurlHandle = curl_multi_init();
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataServiceDownload::RealityDataServiceDownload(Utf8String serverId, bvector<RealityDataFileTransfer*> downloadList, RealityDataServiceTransfer_StatusCallBack pi_func) :
    RealityDataServiceTransfer()
    {
    m_id = serverId;
    m_azureTokenTimer = 0;
    m_progress = 0.0;
    m_progressStep = 0.01;
    m_progressThreshold = 0.01;
    m_onlyReportErrors = false;
    m_currentTransferedAmount = 0;
    m_handshakeRequest = nullptr;
    m_pStatusFunc = pi_func;

    m_handshakeRequest = new AzureHandshake(m_id, false);
    GetAzureToken();

    m_filesToTransfer = downloadList;

    m_fullTransferSize = m_filesToTransfer.size();
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
static Utf8String s_realityDataServer = "dev-realitydataservices-eus.cloudapp.net";
static Utf8String s_realityDataWSGProtocol = "2.4";
static Utf8String s_realityDataRepoName = "S3MXECPlugin--Server";
static Utf8String s_realityDataRepoNameWProjectId = "";
static Utf8String s_realityDataSchemaName = "S3MX";
static Utf8String s_projectId = "";
static bool       s_initializedParams = false;

static bool       s_verifyPeer = false;
static Utf8String s_realityDataCertificatePath = "";

Utf8StringCR RealityDataService::GetServerName()      { return s_realityDataServer; }
Utf8StringCR RealityDataService::GetWSGProtocol()     { return s_realityDataWSGProtocol; }
Utf8StringCR RealityDataService::GetSchemaName()      { return s_realityDataSchemaName; }
const bool   RealityDataService::GetVerifyPeer()      { return s_verifyPeer; } //TODO: verify when possible...
Utf8StringCR RealityDataService::GetCertificatePath() { return s_realityDataCertificatePath; }
Utf8StringCR RealityDataService::GetProjectId()       { return s_projectId; }
const bool   RealityDataService::AreParametersSet()   { return s_initializedParams; }

Utf8StringCR RealityDataService::GetRepoName()        
    {
    if (s_projectId.empty())
        return s_realityDataRepoName; 
    else if (!s_realityDataRepoNameWProjectId.Contains(s_projectId))
        {
        s_realityDataRepoNameWProjectId = s_realityDataRepoName;
        s_realityDataRepoNameWProjectId.ReplaceAll("Server", s_projectId.c_str());
        }

    return s_realityDataRepoNameWProjectId;
    }

void RealityDataService::SetServerComponents(Utf8StringCR server, Utf8StringCR WSGProtocol, Utf8StringCR repoName, Utf8StringCR schemaName, Utf8StringCR certificatePath)
    {
    BeAssert(server.size() != 0);
    BeAssert(WSGProtocol.size() != 0);
    BeAssert(repoName.size() != 0);
    BeAssert(schemaName.size() != 0);

    s_realityDataServer = server;
    s_realityDataWSGProtocol = WSGProtocol;
    s_realityDataRepoName = repoName;
    s_realityDataSchemaName = schemaName;
    s_realityDataRepoNameWProjectId = "";

    if (certificatePath.size() == 0)
        s_verifyPeer = false;
    else
        s_verifyPeer = true;
    s_realityDataCertificatePath = certificatePath;
    s_realityDataRepoNameWProjectId = s_realityDataRepoName;
    s_initializedParams = true;
    }

void RealityDataService::SetServerComponents(Utf8StringCR server, Utf8StringCR WSGProtocol, Utf8StringCR repoName, Utf8StringCR schemaName, Utf8StringCR certificatePath, Utf8StringCR projectId)
    {
    SetServerComponents(server, WSGProtocol, repoName, schemaName, certificatePath);
    s_projectId = projectId;
    }

void RealityDataService::SetProjectId(Utf8StringCR projectId)
    {
    s_projectId = projectId;
    }

static void defaultErrorCallback(Utf8String basicMessage, const RawServerResponse& rawResponse)
    {
    std::cout << basicMessage << std::endl;
    std::cout << rawResponse.body << std::endl;
    }

static RealityDataService_ErrorCallBack s_errorCallback = defaultErrorCallback;

void RealityDataService::SetErrorCallback(RealityDataService_ErrorCallBack errorCallback)
    { 
    s_errorCallback = errorCallback; 
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bvector<RealityDataPtr> RealityDataService::Request(const RealityDataPagedRequest& request, RawServerResponse& rawResponse)
    {
    bvector<RealityDataPtr> entities = bvector<RealityDataPtr>();
    if(!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return entities;
        }

    rawResponse = PagedBasicRequest((&request));

    if (rawResponse.status != RequestStatus::OK)
        s_errorCallback("RealityDataPagedRequest failed with response", rawResponse);
    else
        {
        RealityConversionTools::JsonToRealityData(rawResponse.body.c_str(), &entities);
        if ((uint16_t)entities.size() < request.GetPageSize())
            rawResponse.status = RequestStatus::LASTPAGE;
        }

    return entities;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataService::Request(const RealityDataEnterpriseStatRequest& request, RealityDataEnterpriseStat& statObject, RawServerResponse& rawResponse)
    {
    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return;
        }

    rawResponse = BasicRequest(static_cast<const RealityDataUrl*>(&request));

    if (rawResponse.status != RequestStatus::OK)
        s_errorCallback("RealityDataEnterpriseStatRequest failed with response", rawResponse);

    RealityConversionTools::JsonToEnterpriseStat(rawResponse.body.c_str(), statObject);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bvector<RealityDataEnterpriseStat>  RealityDataService::Request(const RealityDataAllEnterpriseStatsRequest& request, RawServerResponse& rawResponse)
    {

    bvector<RealityDataEnterpriseStat> entities;
    if(!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return entities;
        }

    rawResponse = BasicRequest(static_cast<const RealityDataUrl*>(&request));

    if (rawResponse.status != RequestStatus::OK)
        s_errorCallback("RealityDataAllEnterpriseStatsRequest failed with response", rawResponse);
    else
        {
        RealityConversionTools::JsonToEnterpriseStats(rawResponse.body.c_str(), entities);
        }

    return entities;
    }
    
//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bvector<bpair<WString, uint64_t>> RealityDataService::Request(const AllRealityDataByRootId& request, RawServerResponse& rawResponse)
    {
    bvector<bpair<WString, uint64_t>> documents = bvector<bpair<WString, uint64_t>>();
    
    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return documents;
        }
    rawResponse = request.GetAzureRedirectionRequestUrl();
    if (rawResponse.status != RequestStatus::OK)
        return documents;
    int64_t timer = request.GetTokenTimer();

    bool nextMarker;
    WString value, fileName;
    uint64_t fileSize;
    do
        {
        WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());
        rawResponse = RawServerResponse();
        WSGRequest::GetInstance().PerformAzureRequest(request, rawResponse, RealityDataService::GetVerifyPeer());

        BeXmlStatus xmlStatus = BEXML_Success;
        BeXmlReaderPtr reader = BeXmlReader::CreateAndReadFromString(xmlStatus, rawResponse.body.c_str());
        BeAssert(reader.IsValid());

        value.clear();

        while (IBeXmlReader::ReadResult::READ_RESULT_Success == (reader->ReadTo(IBeXmlReader::NodeType::NODE_TYPE_Element, "Name", false, nullptr)))
            {
            reader->ReadTo(IBeXmlReader::NodeType::NODE_TYPE_Text, nullptr, false, &value);
            fileName = value;
            reader->ReadTo(IBeXmlReader::NodeType::NODE_TYPE_Element, "Content-Length", false, nullptr);
            reader->ReadTo(IBeXmlReader::NodeType::NODE_TYPE_Text, nullptr, false, &value);
            fileSize = BeStringUtilities::ParseUInt64(Utf8String(value.c_str()).c_str());

            documents.push_back(make_bpair(fileName, fileSize));
            }

        nextMarker = false;
        //the previous loop reaches the end of the file, so to find the "NextMarker" element, we need to restart from the top
        reader = BeXmlReader::CreateAndReadFromString(xmlStatus, rawResponse.body.c_str());
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
RealityDataPtr RealityDataService::Request(const RealityDataByIdRequest& request, RawServerResponse& rawResponse)
    {
    bvector<RealityDataPtr> entities = bvector<RealityDataPtr>();

    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return nullptr;
        }

    rawResponse = BasicRequest(static_cast<const RealityDataUrl*>(&request));
    
    if (rawResponse.status != RequestStatus::OK)
        {
        s_errorCallback("RealityDataByIdRequest failed with response", rawResponse);
        return nullptr;
        }
    RealityConversionTools::JsonToRealityData(rawResponse.body.c_str(), &entities);

    return entities[0];
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataService::Request(const RealityDataDelete& request, RawServerResponse& rawResponse)
    {

    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return;
        }

    rawResponse = BasicRequest(static_cast<const RealityDataUrl*>(&request), "changedInstance");
    
    if (rawResponse.status != RequestStatus::OK)
        {
        s_errorCallback("RealityDataDelete failed with response", rawResponse);
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataDocumentPtr RealityDataService::Request(const RealityDataDocumentByIdRequest& request, RawServerResponse& rawResponse)
    {
    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return nullptr;
        }

    rawResponse = BasicRequest(static_cast<const RealityDataUrl*>(&request));

    if (rawResponse.status != RequestStatus::OK)
        {
        s_errorCallback("RealityDataDocumentByIdRequest failed with response", rawResponse);
        return nullptr;
        }

    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(rawResponse.body, instances);

    return RealityDataDocument::Create(instances["instances"][0]);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataService::Request(RealityDataDocumentContentByIdRequest& request, BeFile* file, RawServerResponse& rawResponse)
    {
    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return;
        }

    WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());
    request.GetAzureRedirectionRequestUrl();

    if (request.IsAzureBlobRedirected())
        WSGRequest::GetInstance().PerformAzureRequest(request, rawResponse, RealityDataService::GetVerifyPeer(), file);
    else
        WSGRequest::GetInstance().PerformRequest(request, rawResponse, RealityDataService::GetVerifyPeer(), file);

    if(rawResponse.curlCode != CURLE_OK)
        {
        rawResponse.status = RequestStatus::BADREQ;
        s_errorCallback("RealityDataDocumentContentByIdRequest failed with response", rawResponse);
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataFolderPtr RealityDataService::Request(const RealityDataFolderByIdRequest& request, RawServerResponse& rawResponse)
    {
    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return nullptr;
        }

    rawResponse = BasicRequest(static_cast<const RealityDataUrl*>(&request));

    if(rawResponse.status != RequestStatus::OK)
        {
        s_errorCallback("RealityDataFolderByIdRequest failed with response", rawResponse);
        return nullptr;
        }

    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(rawResponse.body, instances);

    return RealityDataFolder::Create(instances["instances"][0]);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//! DEPRECATED
//=====================================================================================
bvector<RealityDataPtr> RealityDataService::Request(const RealityDataListByOrganizationPagedRequest& request, RawServerResponse& rawResponse)
    {
    assert(0 && "This function is deprecated, please use RealityDataListByUltimateIdPagedRequest");
    return bvector<RealityDataPtr>();
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//! DEPRECATED
//=====================================================================================
bvector<RealityDataPtr> RealityDataService::Request(const RealityDataListByUltimateIdPagedRequest& request, RawServerResponse& rawResponse)
    {
    bvector<RealityDataPtr> entities = bvector<RealityDataPtr>();
    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return entities;
        }

    rawResponse = PagedBasicRequest(static_cast<const RealityDataPagedRequest*>(&request));

    if (rawResponse.status != RequestStatus::OK)
        s_errorCallback("RealityDataListByUltimateIdPagedRequest failed with response", rawResponse);
    else
        {
        RealityConversionTools::JsonToRealityData(rawResponse.body.c_str(), &entities);
        if ((uint16_t)entities.size() < request.GetPageSize())
            rawResponse.status = RequestStatus::LASTPAGE;
        }

    return entities;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bvector<RealityDataProjectRelationshipPtr> RealityDataService::Request(const RealityDataProjectRelationshipByProjectIdRequest& request, RawServerResponse& rawResponse)
    {
    return _RequestRelationship(static_cast<const RealityDataUrl*>(&request), rawResponse);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bvector<RealityDataProjectRelationshipPtr> RealityDataService::Request(const RealityDataProjectRelationshipByRealityDataIdRequest& request, RawServerResponse& rawResponse)
    {
    return _RequestRelationship(static_cast<const RealityDataUrl*>(&request), rawResponse);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bvector<RealityDataProjectRelationshipPtr> RealityDataService::_RequestRelationship(const RealityDataUrl* request, RawServerResponse& rawResponse)
{
    bvector<RealityDataProjectRelationshipPtr> relations = bvector<RealityDataProjectRelationshipPtr>();

    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return relations;
        }

    rawResponse = BasicRequest(request);

    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(rawResponse.body, instances);
    if (rawResponse.status != RequestStatus::OK)
        s_errorCallback("RealityDataProjectRelationshipRequest failed with response", rawResponse);
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
bvector<RealityDataProjectRelationshipPtr> RealityDataService::Request(const RealityDataProjectRelationshipByProjectIdPagedRequest& request, RawServerResponse& rawResponse)
    {
    return _RequestPagedRelationships(static_cast<const RealityDataPagedRequest*>(&request), rawResponse);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bvector<RealityDataProjectRelationshipPtr> RealityDataService::Request(const RealityDataProjectRelationshipByRealityDataIdPagedRequest& request, RawServerResponse& rawResponse)
    {
    return _RequestPagedRelationships(static_cast<const RealityDataPagedRequest*>(&request), rawResponse);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
bvector<RealityDataProjectRelationshipPtr> RealityDataService::_RequestPagedRelationships(const RealityDataPagedRequest* request, RawServerResponse& rawResponse)
    {
    bvector<RealityDataProjectRelationshipPtr> relations = bvector<RealityDataProjectRelationshipPtr>();

    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return relations;
        }

    rawResponse = PagedBasicRequest(request);

    Json::Value instances(Json::objectValue);
    Json::Reader::Parse(rawResponse.body, instances);

    if (rawResponse.status != RequestStatus::OK)
        s_errorCallback("RealityDataProjectRelationshipPagedRequest failed with response", rawResponse);
    else
        {
        for (auto instance : instances["instances"])
            relations.push_back(RealityDataProjectRelationship::Create(instance));
        if ((uint8_t)relations.size() < request->GetPageSize())
            rawResponse.status = RequestStatus::LASTPAGE;
        }

    return relations;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String RealityDataService::Request(const RealityDataChangeRequest& request, RawServerResponse& rawResponse)
    {
    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return "";
        }

    rawResponse = BasicRequest(static_cast<const RealityDataUrl*>(&request), "changedInstance");

    if (rawResponse.status != RequestStatus::OK)
        {
        s_errorCallback("RealityDataChangeRequest failed with response", rawResponse);
        return "";
        }

    return rawResponse.body;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String RealityDataService::Request(const RealityDataCreateRequest& request, RawServerResponse& rawResponse)
{
    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return "";
        }

    rawResponse = BasicRequest(static_cast<const RealityDataUrl*>(&request), "changedInstance");

    if (rawResponse.status != RequestStatus::OK)
        {
        s_errorCallback("RealityDataCreateRequest failed with response", rawResponse);
        return "";
        }

    return rawResponse.body;
}

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String RealityDataService::Request(const RealityDataRelationshipCreateRequest& request, RawServerResponse& rawResponse)
    {
    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return "";
        }

    rawResponse = BasicRequest(static_cast<const RealityDataUrl*>(&request), "changedInstance");

    if (rawResponse.status != RequestStatus::OK)
        s_errorCallback("RealityDataRelationshipCreateRequest failed with response", rawResponse);

    return rawResponse.body;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
Utf8String RealityDataService::Request(const RealityDataRelationshipDelete& request, RawServerResponse& rawResponse)
    {
    rawResponse.clear();

    if (!RealityDataService::AreParametersSet())
        {
        rawResponse.status = RequestStatus::PARAMSNOTSET;
        return "";
        }

    rawResponse = BasicRequest(static_cast<const RealityDataUrl*>(&request), "changedInstance");

    if (rawResponse.status != RequestStatus::OK)
        s_errorCallback("RealityDataRelationshipDelete failed with response", rawResponse);

    return rawResponse.body;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RawServerResponse RealityDataService::PagedBasicRequest(const RealityDataPagedRequest* request, Utf8StringCR keyword)
    {
    RawServerResponse response = RawServerResponse();
    WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());
    WSGRequest::GetInstance().PerformRequest(*request, response, RealityDataService::GetVerifyPeer());

    Json::Value instances(Json::objectValue);
    if(response.ValidateJSONResponse(instances, keyword) == RequestStatus::OK)
        {
        request->AdvancePage();
        response.status = RequestStatus::OK;
        }

    return response;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RawServerResponse RealityDataService::BasicRequest(const RealityDataUrl* request, Utf8StringCR keyword)
    {
    RawServerResponse response = RawServerResponse();
    WSGRequest::GetInstance().SetCertificatePath(RealityDataService::GetCertificatePath());
    WSGRequest::GetInstance().PerformRequest(*request, response, RealityDataService::GetVerifyPeer());

    Json::Value instances(Json::objectValue);
    if (response.ValidateJSONResponse(instances, keyword) == RequestStatus::OK)
        response.status = RequestStatus::OK;
    
    return response;
    }

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau            06/2017
//=====================================================================================
void RealityDataServiceHelper::ResetServerComponents()
    {
    s_initializedParams = false;
    s_realityDataServer = "dev-realitydataservices-eus.cloudapp.net";
    s_realityDataWSGProtocol = "2.4";
    s_realityDataRepoName = "S3MXECPlugin--Server";
    s_realityDataRepoNameWProjectId = "";
    s_realityDataSchemaName = "S3MX";
    s_projectId = "";
    }