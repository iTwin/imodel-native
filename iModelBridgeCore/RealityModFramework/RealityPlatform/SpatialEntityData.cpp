/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/SpatialEntityData.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeDirectoryIterator.h>
#include <BeXml/BeXml.h>

#include <RealityPlatform/SpatialEntityData.h>
#include <RealityPlatform/RealityDataDownload.h>


#define THUMBNAIL_WIDTH     512
#define THUMBNAIL_HEIGHT    512

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// Curl callback that receive data.
//
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
static size_t WriteData(void* buffer, size_t size, size_t nmemb, void* stream)
    {
    ((Utf8StringP)stream)->append((Utf8CP)buffer, size * nmemb);
    return size * nmemb;
    }

// Static FtpClient members initialization.
ISpatialEntityTraversalObserver* SpatialEntityClient::m_pObserver = NULL;
SpatialEntityClient::RepositoryMapping SpatialEntityClient::m_dataRepositories = SpatialEntityClient::RepositoryMapping();
int SpatialEntityClient::m_retryCount = 0;

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityStatus SpatialEntityClient::DownloadContent(Utf8CP outputDirPath) const
    {
    SpatialEntityStatus status = SpatialEntityStatus::UnknownError;

    // Set working directory.
    Utf8String workingDir = outputDirPath;
    if (workingDir.empty())
        {
        // Find temp directory.
        BeFileName tempDirPath;
        BeFileName::BeGetTempPath(tempDirPath);
        if (!tempDirPath.IsEmpty())
            {
            tempDirPath.AppendToPath(L"Bentley\\ConceptStationApp\\.RealityData\\SpatialEntityData\\");
            BeFileName::CreateNewDirectory(tempDirPath.GetName());
            workingDir = tempDirPath.GetNameUtf8().c_str();
            }
        }

    // Perform file listing request.
    bvector<Utf8String> fileList;
    status = GetFileList(fileList);
    if (SpatialEntityStatus::Success != status)
        return status;

    if (fileList.empty())
        return SpatialEntityStatus::Success; // There is no file to download. This is not an error because all files may already be in the cache and there is no need to redownload them.

                                           // Construct data mapping (FileFullPathAndName, FileNameOnly) for files to download.
    RealityDataDownload::UrlLink_UrlFile urlList;
    for (size_t i = 0; i < fileList.size(); ++i)
        {
        // The local filename is created by appending the working dir, the SpatialEntity main url and the filename.        
        WString SpatialEntityUrl(fileList[i].c_str(), BentleyCharEncoding::Utf8);
        size_t pos = SpatialEntityUrl.find(L"//") + 2;
        size_t len = SpatialEntityUrl.find(L'/', pos) - pos;
        SpatialEntityUrl = SpatialEntityUrl.substr(pos, len);
        WString shortUrl;
        for (wchar_t& car : SpatialEntityUrl)
            {
            if (L'.' != car)
                shortUrl.push_back(car);
            }

        WString filename;
        RealityDataDownload::ExtractFileName(filename, fileList[i]);

        WString localFilename(workingDir.c_str(), BentleyCharEncoding::Utf8);
        localFilename.append(shortUrl + L'_' + filename);

        urlList.push_back(std::make_pair(fileList[i], localFilename));
        }

    // Download files.
    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(urlList);
    if (pDownload == NULL)
        return SpatialEntityStatus::DownloadError;

    if (!m_certificatePath.IsEmpty())
       pDownload->SetCertificatePath(m_certificatePath.GetName());

    pDownload->SetStatusCallBack(ConstructRepositoryMapping);
    if (!pDownload->Perform())
        return SpatialEntityStatus::DownloadError;

    return status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityStatus SpatialEntityClient::GetFileList(bvector<Utf8String>& fileList) const
    {
    return _GetFileList(m_pServer->GetUrl().c_str(), fileList);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityStatus SpatialEntityClient::GetData() const
    {
    SpatialEntityStatus status = SpatialEntityStatus::UnknownError;

    // Download files from root. Store them in our temp directory.
    status = DownloadContent();
    if (SpatialEntityStatus::Success != status)
        return status;

    // Data extraction.
    SpatialEntityDataPtr pExtractedData = SpatialEntityData::Create();
    for (size_t i = 0; i < m_dataRepositories.size(); ++i)
        {
        //&&JFC TODO: Can do better ?
        // Construct output path.
        Utf8String outputPath = m_dataRepositories[i].second;
        outputPath.erase(outputPath.find_last_of('.'));
        outputPath.append("\\");
        WString outputPathW(outputPath.c_str(), BentleyCharEncoding::Utf8);
        BeFileName::CreateNewDirectory(outputPathW.c_str());

        // Extract data.
        pExtractedData = ExtractDataFromPath(m_dataRepositories[i].second.c_str(), outputPath.c_str());
        if (pExtractedData == NULL)
            {
            // Could not extract data, ignore and continue.
            BeFileName::EmptyAndRemoveDirectory(outputPathW.c_str());
            continue;
            }

        // Override source url so that it points to the SpatialEntity repository and not the local one.
        pExtractedData->SetUrl(m_dataRepositories[i].first.c_str());

        // Set server.
        pExtractedData->SetServer(*m_pServer);

        // Set provider.
        pExtractedData->SetProvider(GetServerName().c_str());

        // Set dataset
        pExtractedData->SetDataset(GetDataset().c_str());

        // Set classification (overide if specified)
        if (GetClassification().size() > 0)
            pExtractedData->SetClassification(GetClassification().c_str());

        // Process created data.
        if (m_pObserver != NULL && pExtractedData != NULL)
            m_pObserver->OnDataExtracted(*pExtractedData);

        // Delete working dir and its content.
        BeFileName::EmptyAndRemoveDirectory(outputPathW.c_str());
        }

    return status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityClient::GetServerUrl() const
    {
    return m_pServer->GetUrl();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    7/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityClient::GetServerName() const
    {
    return m_pServer->GetName();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    10/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityClient::GetDataset() const
    {
    return m_datasetName;
    }
	
//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    10/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityClient::GetFilePattern() const
    {
    return m_filePattern;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    10/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityClient::GetClassification() const
    {
    return m_classification;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
const SpatialEntityClient::RepositoryMapping& SpatialEntityClient::GetRepositoryMapping() const
    {
    return m_dataRepositories;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void SpatialEntityClient::SetObserver(ISpatialEntityTraversalObserver* pObserver)
    {
    m_pObserver = pObserver;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityClient::SpatialEntityClient(Utf8CP serverUrl, Utf8CP serverName, Utf8CP datasetName, Utf8CP filePattern, bool extractThumbnails, Utf8CP classification)
    {
    m_certificatePath = BeFileName();
    m_pServer = SpatialEntityServer::Create(serverUrl, serverName);
    m_pObserver = NULL;
    m_dataRepositories = RepositoryMapping();
    m_datasetName = Utf8String(datasetName);
    m_filePattern = Utf8String(filePattern);
    m_extractThumbnails = extractThumbnails;
    m_classification = Utf8String(classification);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityClient::~SpatialEntityClient()
    {
    if (0 != m_pObserver)
        {
        delete m_pObserver;
        m_pObserver = 0;
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void SpatialEntityClient::ConstructRepositoryMapping(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    RealityDataDownload::FileTransfer* pEntry = (RealityDataDownload::FileTransfer*)pClient;
    if (ErrorCode == 0)
        {
        // Construct repo mapping (remote location, local location) for downloaded file.
        if (pEntry->mirrors.empty())
            return;
        Utf8String url(pEntry->mirrors.front().url);
        Utf8String filename(pEntry->mirrors.front().filename);

        m_dataRepositories.push_back(make_bpair(url.c_str(), filename.c_str()));

        m_retryCount = 0;

        // Process downloaded data.
        if (m_pObserver != NULL)
            m_pObserver->OnFileDownloaded(url.c_str());
        }
    else
        {
        // Download failed. RealityDataDownload will retry 25 times. 
        // Add a longer sleep between each tentative so that the server have a better chance to respond. For example:
        // Retry count: 0,  Sleep time: 0.002 second.
        // Retry count: 4,  Sleep time: 1 second.
        // Retry count: 24, Sleep time: 5 seconds.
        Sleep(++m_retryCount * 200);
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
bool SpatialEntityClient::IsDirectory(Utf8CP content) const
    {
    //&&JFC TODO: More robust check.
    Utf8String contentStr(content);
    return (BeStringUtilities::NPOS == contentStr.find("."));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                           Spencer.Mason                          10/2016
//-------------------------------------------------------------------------------------
ISpatialEntityTraversalObserver* SpatialEntityClient::GetObserver()
    {
    return m_pObserver;
    }
    
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityRequest::SpatialEntityRequest(Utf8CP url)
    : m_url(url), m_method("NLST"), m_dirListOnly(false), m_verbose(false)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityResponsePtr SpatialEntityRequest::Perform()
    {
    Utf8String response;
    CurlHolder curl;
    CURLcode res;

    // Specify url to get.
    curl_easy_setopt(curl.Get(), CURLOPT_URL, m_url);

    // Set options.
    curl_easy_setopt(curl.Get(), CURLOPT_CUSTOMREQUEST, m_method); // Custom string for request method.
    curl_easy_setopt(curl.Get(), CURLOPT_DIRLISTONLY, m_dirListOnly); // Ask for names only in a directory listing.
    curl_easy_setopt(curl.Get(), CURLOPT_VERBOSE, m_verbose); // Switch on full protocol/debug output while testing.

                                                              // Send all data to this function.
    curl_easy_setopt(curl.Get(), CURLOPT_WRITEFUNCTION, WriteData);

    // We pass our struct to the callback function.
    curl_easy_setopt(curl.Get(), CURLOPT_WRITEDATA, &response);

    // Perform the request, res will get the return code.
    res = curl_easy_perform(curl.Get());

    // Check for errors.
    SpatialEntityStatus status = SpatialEntityStatus::Success;
    if (CURLE_OK != res || response.empty())
        status = SpatialEntityStatus::CurlError;

    return SpatialEntityResponse::Create(m_url.c_str(), response.c_str(), status);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityResponsePtr SpatialEntityResponse::Create()
    {
    return new SpatialEntityResponse("", "", SpatialEntityStatus::UnknownError);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityResponsePtr SpatialEntityResponse::Create(Utf8CP effectiveUrl, Utf8CP m_content, SpatialEntityStatus traversalStatus)
    {
    return new SpatialEntityResponse(effectiveUrl, m_content, traversalStatus);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityResponse::GetUrl() const
    {
    return m_effectiveUrl;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityResponse::GetContent() const
    {
    return m_content;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityStatus SpatialEntityResponse::GetStatus() const
    {
    return m_status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
bool SpatialEntityResponse::IsSuccess() const
    {
    return (!m_effectiveUrl.empty() &&
        !m_content.empty() &&
        (SpatialEntityStatus::Success == m_status));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityResponse::SpatialEntityResponse(Utf8CP effectiveUrl, Utf8CP content, SpatialEntityStatus status)
    : m_effectiveUrl(effectiveUrl), m_content(content), m_status(status)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataPtr SpatialEntityData::Create()
    {
    return new SpatialEntityData();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityData::GetName() const { return m_name; }
void SpatialEntityData::SetName(Utf8CP name) { m_name = name; }

Utf8StringCR SpatialEntityData::GetUrl() const { return m_url; }
void SpatialEntityData::SetUrl(Utf8CP url) { m_url = url; }

Utf8StringCR SpatialEntityData::GetGeoCS() const { return m_geoCS; }
void SpatialEntityData::SetGeoCS(Utf8CP geoCS) { m_geoCS = geoCS; }

Utf8StringCR SpatialEntityData::GetCompoundType() const { return m_compoundType; }
void SpatialEntityData::SetCompoundType(Utf8CP type) { m_compoundType = type; }

uint64_t SpatialEntityData::GetSize() const { return m_size; }
void SpatialEntityData::SetSize(uint64_t size) { m_size = size; }

Utf8StringCR SpatialEntityData::GetResolution() const { return m_resolution; }
void SpatialEntityData::SetResolution(Utf8CP res) { m_resolution = res; }

Utf8StringCR SpatialEntityData::GetProvider() const { return m_provider; }
void SpatialEntityData::SetProvider(Utf8CP provider) { m_provider = provider; }

Utf8StringCR SpatialEntityData::GetDataset() const { return m_dataset; }
void SpatialEntityData::SetDataset(Utf8CP dataset) { m_dataset = dataset; }

Utf8StringCR SpatialEntityData::GetClassification() const { return m_classification; }
void SpatialEntityData::SetClassification(Utf8CP classification) { m_classification = classification; }

Utf8StringCR SpatialEntityData::GetDataType() const { return m_dataType; }
void SpatialEntityData::SetDataType(Utf8CP type) { m_dataType = type; }

Utf8StringCR SpatialEntityData::GetLocationInCompound() const { return m_locationInCompound; }
void SpatialEntityData::SetLocationInCompound(Utf8CP location) { m_locationInCompound = location; }

DateTimeCR SpatialEntityData::GetDate() const { return m_date; }
void SpatialEntityData::SetDate(DateTimeCR date) { m_date = date; }

const bvector<DPoint2d>& SpatialEntityData::GetFootprint() const { return m_footprint; }
void SpatialEntityData::SetFootprint(bvector<DPoint2d>& footprint) { m_footprint = footprint; }

DRange2dCR SpatialEntityData::GetFootprintExtents() const { return m_footprintExtents; }
void SpatialEntityData::SetFootprintExtents(DRange2dCR footprintExtents) { m_footprintExtents = footprintExtents; }

SpatialEntityThumbnailCR SpatialEntityData::GetThumbnail() const { return *m_pThumbnail; }
void SpatialEntityData::SetThumbnail(SpatialEntityThumbnailR thumbnail) { m_pThumbnail = &thumbnail; }

SpatialEntityMetadataCR SpatialEntityData::GetMetadata() const { return *m_pMetadata; }
void SpatialEntityData::SetMetadata(SpatialEntityMetadataR metadata) { m_pMetadata = &metadata; }

SpatialEntityServerCR SpatialEntityData::GetServer() const { return *m_pServer; }
void SpatialEntityData::SetServer(SpatialEntityServerR server) { m_pServer = &server; }

bool SpatialEntityData::GetIsMultiband() const { return m_isMultiband; }
void SpatialEntityData::SetIsMultiband(bool isMultiband) { m_isMultiband = isMultiband; }

Utf8String SpatialEntityData::GetMultibandUrl() const { return m_multibandDownloadUrl; }
void SpatialEntityData::SetMultibandUrl(Utf8String url) { m_multibandDownloadUrl = url; }

float SpatialEntityData::GetCloudCover() const { return m_cloudCover; }
void SpatialEntityData::SetCloudCover(float cover) { m_cloudCover = cover; }

float SpatialEntityData::GetRedBandSize() const { return m_redSize; }
void SpatialEntityData::SetRedBandSize(float size) { m_redSize = size; }

float SpatialEntityData::GetBlueBandSize() const { return m_blueSize; }
void SpatialEntityData::SetBlueBandSize(float size) { m_blueSize = size; }

float SpatialEntityData::GetGreenBandSize() const { return m_greenSize; }
void SpatialEntityData::SetGreenBandSize(float size) { m_greenSize = size; }

float SpatialEntityData::GetPanchromaticBandSize() const { return m_panchromaticSize; }
void SpatialEntityData::SetPanchromaticBandSize(float size) { m_panchromaticSize = size; }

SQLINTEGER SpatialEntityData::GetMultibandServerId() const { return m_multibandServerId; }
void SpatialEntityData::SetMultibandServerId(SQLINTEGER id) { m_multibandServerId = id; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityData::SpatialEntityData()
    {
    m_size = 0;
    m_date = DateTime();
    m_footprint = bvector<DPoint2d>();
    m_footprintExtents = DRange2d();
    m_pThumbnail = SpatialEntityThumbnail::Create();
    m_pMetadata = SpatialEntityMetadata::Create();
    m_pServer = SpatialEntityServer::Create();
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityThumbnailPtr SpatialEntityThumbnail::Create()
    {
    return new SpatialEntityThumbnail();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityThumbnail::GetProvenance() const { return m_provenance; }
void SpatialEntityThumbnail::SetProvenance(Utf8CP provenance) { m_provenance = provenance; }

Utf8StringCR SpatialEntityThumbnail::GetFormat() const { return m_format; }
void SpatialEntityThumbnail::SetFormat(Utf8CP format) { m_format = format; }

uint32_t SpatialEntityThumbnail::GetWidth() const { return m_width; }
void SpatialEntityThumbnail::SetWidth(uint32_t width) { m_width = width; }

uint32_t SpatialEntityThumbnail::GetHeight() const { return m_height; }
void SpatialEntityThumbnail::SetHeight(uint32_t height) { m_height = height; }

DateTimeCR SpatialEntityThumbnail::GetStamp() const { return m_stamp; }
void SpatialEntityThumbnail::SetStamp(DateTimeCR date) { m_stamp = date; }

const bvector<Byte>& SpatialEntityThumbnail::GetData() const { return m_data; }
void SpatialEntityThumbnail::SetData(const bvector<Byte>& data) { m_data = data; }

Utf8StringCR SpatialEntityThumbnail::GetGenerationDetails() const { return m_generationDetails; }
void SpatialEntityThumbnail::SetGenerationDetails(Utf8CP details) { m_generationDetails = details; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityThumbnail::SpatialEntityThumbnail()
    {
    m_width = THUMBNAIL_WIDTH;
    m_height = THUMBNAIL_HEIGHT;
    m_stamp = DateTime::GetCurrentTimeUtc();
    m_data = bvector<Byte>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityMetadataPtr SpatialEntityMetadata::Create()
    {
    return new SpatialEntityMetadata();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityMetadataPtr SpatialEntityMetadata::CreateFromFile(Utf8CP filePath)
    {
    return new SpatialEntityMetadata(filePath);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityMetadata::GetProvenance() const { return m_provenance; }
void SpatialEntityMetadata::SetProvenance(Utf8CP provenance) { m_provenance = provenance; }

Utf8StringCR SpatialEntityMetadata::GetDescription() const { return m_description; }
void SpatialEntityMetadata::SetDescription(Utf8CP description) { m_description = description; }

Utf8StringCR SpatialEntityMetadata::GetContactInfo() const { return m_contactInfo; }
void SpatialEntityMetadata::SetContactInfo(Utf8CP info) { m_contactInfo = info; }

Utf8StringCR SpatialEntityMetadata::GetLegal() const { return m_legal; }
void SpatialEntityMetadata::SetLegal(Utf8CP legal) { m_legal = legal; }

Utf8StringCR SpatialEntityMetadata::GetFormat() const { return m_format; }
void SpatialEntityMetadata::SetFormat(Utf8CP format) { m_format = format; }

Utf8StringCR SpatialEntityMetadata::GetData() const { return m_data; }
void SpatialEntityMetadata::SetData(Utf8CP data) { m_data = data; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityMetadata::SpatialEntityMetadata()
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityMetadata::SpatialEntityMetadata(Utf8CP filePath)
    {
    BeFileName metadataFile(filePath);
    Utf8String provenance(metadataFile.GetFileNameAndExtension());
    m_provenance = provenance;
    Utf8String format(metadataFile.GetExtension());
    m_format = format;

    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, filePath, NULL);
    if (BEXML_Success == xmlStatus)
        {
        BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
        if (NULL != pRootNode)
            {
            // Convert to string.
            pRootNode->GetXmlString(m_data);
            }
        }
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityServerPtr SpatialEntityServer::Create()
    {
    return new SpatialEntityServer();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityServerPtr SpatialEntityServer::Create(Utf8CP url, Utf8CP name)
    {
    return new SpatialEntityServer(url, name);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityServer::GetProtocol() const { return m_protocol; }
void SpatialEntityServer::SetProtocol(Utf8CP protocol) { m_protocol = protocol; }

Utf8StringCR SpatialEntityServer::GetName() const { return m_name; }
void SpatialEntityServer::SetName(Utf8CP name) { m_name = name; }

Utf8StringCR SpatialEntityServer::GetUrl() const { return m_url; }
void SpatialEntityServer::SetUrl(Utf8CP url) { m_url = url; }

Utf8StringCR SpatialEntityServer::GetContactInfo() const { return m_contactInfo; }
void SpatialEntityServer::SetContactInfo(Utf8CP info) { m_contactInfo = info; }

Utf8StringCR SpatialEntityServer::GetLegal() const { return m_legal; }
void SpatialEntityServer::SetLegal(Utf8CP legal) { m_legal = legal; }

bool SpatialEntityServer::IsOnline() const { return m_online; }
void SpatialEntityServer::SetOnline(bool online) { m_online = online; }

DateTimeCR SpatialEntityServer::GetLastCheck() const { return m_lastCheck; }
void SpatialEntityServer::SetLastCheck(DateTimeCR data) { m_lastCheck = data; }

DateTimeCR SpatialEntityServer::GetLastTimeOnline() const { return m_lastTimeOnline; }
void SpatialEntityServer::SetLastTimeOnline(DateTimeCR data) { m_lastTimeOnline = data; }

double SpatialEntityServer::GetLatency() const { return m_latency; }
void SpatialEntityServer::SetLatency(double latency) { m_latency = latency; }

Utf8StringCR SpatialEntityServer::GetState() const { return m_state; }
void SpatialEntityServer::SetState(Utf8CP state) { m_state = state; }

Utf8StringCR SpatialEntityServer::GetType() const { return m_type; }
void SpatialEntityServer::SetType(Utf8CP type) { m_type = type; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityServer::SpatialEntityServer()
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityServer::SpatialEntityServer(Utf8CP url, Utf8CP name)
    : m_url(url), m_name(name)
    {
    // Default values.
    m_online = true;
    m_lastCheck = DateTime::GetCurrentTimeUtc();
    m_lastTimeOnline = DateTime::GetCurrentTimeUtc();
    m_latency = 0.0;

    if (!m_url.empty())
        {
        // Extract protocol and type from url.
        Utf8String protocol(url);
        m_protocol = protocol.substr(0, protocol.find_first_of(":"));
        m_type = m_protocol;

        if (m_name.empty())
            {
            // No server name was provided, try to extract it from url.
            Utf8String name(url);
            size_t beginPos = name.find_first_of("://") + 3;
            size_t pos = name.find_last_of(".");
            size_t endPos = name.find("/", pos);
            m_name = name.substr(beginPos, endPos - beginPos);
            }
        }
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityStatus SpatialEntityDataHandler::UnzipFiles(Utf8CP inputDirPath, Utf8CP outputDirPath)
    {
    // Get a list of zip files to process.
    bvector<BeFileName> fileFoundList;
    BeFileName rootDir(inputDirPath);
    BeDirectoryIterator::WalkDirsAndMatch(fileFoundList, rootDir, L"*.zip", true);

    // Unzip files.    
    for (size_t i = 0; i < fileFoundList.size(); ++i)
        {
        WString outputDirPathW(outputDirPath, BentleyCharEncoding::Utf8);
        AString outputDirPathA(outputDirPath);

        // Construct output path.
        WString outputFolderName;
        RealityDataDownload::ExtractFileName(outputFolderName, fileFoundList[i].GetNameUtf8());
        outputFolderName.erase(outputFolderName.find_last_of('.'));
        outputDirPathW.append(outputFolderName);
        BeFileName::CreateNewDirectory(outputDirPathW.c_str());

        WString filenameW(fileFoundList[i].GetName());
        RealityDataDownload::UnZipFile(filenameW, outputDirPathW);
        }

    return SpatialEntityStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
BeFileName SpatialEntityDataHandler::BuildMetadataFilename(Utf8CP dirPath)
    {
    bvector<BeFileName> fileFoundList;
    BeFileName rootDir(dirPath);
    BeDirectoryIterator::WalkDirsAndMatch(fileFoundList, rootDir, L"*.xml", false);

    if (fileFoundList.empty())
        return BeFileName();

    // Find the xml file corresponding to the metadata.
    for (BeFileNameCR file : fileFoundList)
        {
        // Create xmlDom from file.
        BeXmlStatus xmlStatus = BEXML_Success;
        BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, file.GetNameUtf8().c_str());
        if (BEXML_Success != xmlStatus)
            {
            return BeFileName();
            }

        // Make sure the root node is <gmd:MD_Metadata>.
        BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
        if (NULL == pRootNode)
            return BeFileName();

        if (pRootNode->IsIName("MD_Metadata"))
            return file;
        }

    // No metadata file found.
    return BeFileName();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
Utf8String SpatialEntityDataHandler::RetrieveGeocodingFromMetadata(BeFileNameCR filename)
    {
    Utf8String geocoding;

    // Create xmlDom from metadata file.
    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, filename.GetNameUtf8().c_str());
    if (BEXML_Success != xmlStatus)
        {
        return NULL;
        }

    pXmlDom->RegisterNamespace("gmd", "http://www.isotc211.org/2005/gmd");

    // Get root node.
    BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
    if (NULL == pRootNode)
        return NULL;

    // Get reference system info node.
    BeXmlNodeP pRefSysNode = pRootNode->SelectSingleNode("gmd:referenceSystemInfo");
    if (NULL == pRefSysNode)
        return NULL;

    // Get md reference system node.
    BeXmlNodeP pMdRefNode = pRefSysNode->SelectSingleNode("gmd:MD_ReferenceSystem");
    if (NULL == pMdRefNode)
        return NULL;

    // Get reference system identifier node.
    BeXmlNodeP pRefSysIdNode = pMdRefNode->SelectSingleNode("gmd:referenceSystemIdentifier");
    if (NULL == pRefSysIdNode)
        return NULL;

    // Get rs identifier node.
    BeXmlNodeP pRsIdNode = pRefSysIdNode->SelectSingleNode("gmd:RS_Identifier");
    if (NULL == pRsIdNode)
        return NULL;

    // Get code.
    BeXmlNodeP pCodeNode = pRsIdNode->SelectSingleNode("gmd:code");
    if (NULL == pCodeNode)
        return NULL;

    xmlStatus = pCodeNode->GetContent(geocoding);
    if (BEXML_Success != xmlStatus)
        return NULL;

    return geocoding.Trim();
    }

