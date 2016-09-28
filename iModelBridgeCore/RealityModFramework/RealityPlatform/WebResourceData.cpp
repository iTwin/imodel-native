/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/WebResourceData.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeDirectoryIterator.h>
#include <BeXml/BeXml.h>

#include <RealityPlatform/WebResourceData.h>
#include <RealityPlatform/RealityDataHandler.h>
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
IWebResourceTraversalObserver* WebResourceClient::m_pObserver = NULL;
WebResourceClient::RepositoryMapping WebResourceClient::m_dataRepositories = WebResourceClient::RepositoryMapping();
int WebResourceClient::m_retryCount = 0;

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
WebResourceStatus WebResourceClient::DownloadContent(Utf8CP outputDirPath) const
{
    WebResourceStatus status = WebResourceStatus::UnknownError;

    // Set working directory.
    Utf8String workingDir = outputDirPath;
    if (workingDir.empty())
    {
        // Find temp directory.
        BeFileName tempDirPath;
        BeFileName::BeGetTempPath(tempDirPath);
        if (!tempDirPath.IsEmpty())
        {
            tempDirPath.AppendToPath(L"Bentley\\ConceptStationApp\\.RealityData\\WebResourcedata\\");
            BeFileName::CreateNewDirectory(tempDirPath.GetName());
            workingDir = tempDirPath.GetNameUtf8().c_str();
        }
    }

    // Perform file listing request.
    bvector<Utf8String> fileList;
    status = GetFileList(fileList);
    if (WebResourceStatus::Success != status)
        return status;

    if (fileList.empty())
        return WebResourceStatus::Success; // There is no file to download. This is not an error because all files may already be in the cache and there is no need to redownload them.

                                           // Construct data mapping (FileFullPathAndName, FileNameOnly) for files to download.
    RealityDataDownload::UrlLink_UrlFile urlList;
    for (size_t i = 0; i < fileList.size(); ++i)
    {
        // The local filename is created by appending the working dir, the WebResource main url and the filename.        
        WString WebResourceUrl(fileList[i].c_str(), BentleyCharEncoding::Utf8);
        size_t pos = WebResourceUrl.find(L"//") + 2;
        size_t len = WebResourceUrl.find(L'/', pos) - pos;
        WebResourceUrl = WebResourceUrl.substr(pos, len);
        WString shortUrl;
        for (wchar_t& car : WebResourceUrl)
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
        return WebResourceStatus::DownloadError;

    if (!m_certificatePath.IsEmpty())
       pDownload->SetCertificatePath(m_certificatePath.GetName());

    pDownload->SetStatusCallBack(ConstructRepositoryMapping);
    if (!pDownload->Perform())
        return WebResourceStatus::DownloadError;

    return status;
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
WebResourceStatus WebResourceClient::GetFileList(bvector<Utf8String>& fileList) const
{
    return _GetFileList(m_pServer->GetUrl().c_str(), fileList);
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceStatus WebResourceClient::GetData() const
{
    WebResourceStatus status = WebResourceStatus::UnknownError;

    // Download files from root. Store them in our temp directory.
    status = DownloadContent();
    if (WebResourceStatus::Success != status)
        return status;

    // Data extraction.
    WebResourceDataPtr pExtractedData = WebResourceData::Create();
    for (size_t i = 0; i < m_dataRepositories.size(); ++i)
    {
        //&&JFC TODO: Can do better ?
        // Construct output path.
        Utf8String outputPath = m_dataRepositories[i].second;
        outputPath.erase(outputPath.find_last_of('.'));
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

        // Override source url so that it points to the WebResource repository and not the local one.
        pExtractedData->SetUrl(m_dataRepositories[i].first.c_str());

        // Set server.
        pExtractedData->SetServer(*m_pServer);

        // Set provider.
        pExtractedData->SetProvider(GetServerName().c_str());

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
Utf8StringCR WebResourceClient::GetServerUrl() const
{
    return m_pServer->GetUrl();
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    7/2016
//-------------------------------------------------------------------------------------
Utf8StringCR WebResourceClient::GetServerName() const
{
    return m_pServer->GetName();
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
const WebResourceClient::RepositoryMapping& WebResourceClient::GetRepositoryMapping() const
{
    return m_dataRepositories;
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void WebResourceClient::SetObserver(IWebResourceTraversalObserver* pObserver)
{
    m_pObserver = pObserver;
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
WebResourceClient::WebResourceClient(Utf8CP serverUrl, Utf8CP serverName)
{
    m_certificatePath = BeFileName();
    m_pServer = WebResourceServer::Create(serverUrl, serverName);
    m_pObserver = NULL;
    m_dataRepositories = RepositoryMapping();
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
WebResourceClient::~WebResourceClient()
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
void WebResourceClient::ConstructRepositoryMapping(int index, void *pClient, int ErrorCode, const char* pMsg)
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
bool WebResourceClient::IsDirectory(Utf8CP content) const
{
    //&&JFC TODO: More robust check.
    Utf8String contentStr(content);
    return (BeStringUtilities::NPOS == contentStr.find("."));
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
WebResourceRequest::WebResourceRequest(Utf8CP url)
    : m_url(url), m_method("NLST"), m_dirListOnly(false), m_verbose(false)
{}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
WebResourceResponsePtr WebResourceRequest::Perform()
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
    WebResourceStatus status = WebResourceStatus::Success;
    if (CURLE_OK != res || response.empty())
        status = WebResourceStatus::CurlError;

    return WebResourceResponse::Create(m_url.c_str(), response.c_str(), status);
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
WebResourceResponsePtr WebResourceResponse::Create()
{
    return new WebResourceResponse("", "", WebResourceStatus::UnknownError);
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
WebResourceResponsePtr WebResourceResponse::Create(Utf8CP effectiveUrl, Utf8CP m_content, WebResourceStatus traversalStatus)
{
    return new WebResourceResponse(effectiveUrl, m_content, traversalStatus);
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
Utf8StringCR WebResourceResponse::GetUrl() const
{
    return m_effectiveUrl;
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
Utf8StringCR WebResourceResponse::GetContent() const
{
    return m_content;
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
WebResourceStatus WebResourceResponse::GetStatus() const
{
    return m_status;
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
bool WebResourceResponse::IsSuccess() const
{
    return (!m_effectiveUrl.empty() &&
        !m_content.empty() &&
        (WebResourceStatus::Success == m_status));
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
WebResourceResponse::WebResourceResponse(Utf8CP effectiveUrl, Utf8CP content, WebResourceStatus status)
    : m_effectiveUrl(effectiveUrl), m_content(content), m_status(status)
{}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceDataPtr WebResourceData::Create()
{
    return new WebResourceData();
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
Utf8StringCR WebResourceData::GetName() const { return m_name; }
void WebResourceData::SetName(Utf8CP name) { m_name = name; }

Utf8StringCR WebResourceData::GetUrl() const { return m_url; }
void WebResourceData::SetUrl(Utf8CP url) { m_url = url; }

Utf8StringCR WebResourceData::GetCompoundType() const { return m_compoundType; }
void WebResourceData::SetCompoundType(Utf8CP type) { m_compoundType = type; }

uint64_t WebResourceData::GetSize() const { return m_size; }
void WebResourceData::SetSize(uint64_t size) { m_size = size; }

Utf8StringCR WebResourceData::GetResolution() const { return m_resolution; }
void WebResourceData::SetResolution(Utf8CP res) { m_resolution = res; }

Utf8StringCR WebResourceData::GetProvider() const { return m_provider; }
void WebResourceData::SetProvider(Utf8CP provider) { m_provider = provider; }

Utf8StringCR WebResourceData::GetDataType() const { return m_dataType; }
void WebResourceData::SetDataType(Utf8CP type) { m_dataType = type; }

Utf8StringCR WebResourceData::GetLocationInCompound() const { return m_locationInCompound; }
void WebResourceData::SetLocationInCompound(Utf8CP location) { m_locationInCompound = location; }

DateTimeCR WebResourceData::GetDate() const { return m_date; }
void WebResourceData::SetDate(DateTimeCR date) { m_date = date; }

DRange2dCR WebResourceData::GetFootprint() const { return m_footprint; }
void WebResourceData::SetFootprint(DRange2dCR footprint) { m_footprint = footprint; }

WebResourceThumbnailCR WebResourceData::GetThumbnail() const { return *m_pThumbnail; }
void WebResourceData::SetThumbnail(WebResourceThumbnailR thumbnail) { m_pThumbnail = &thumbnail; }

WebResourceMetadataCR WebResourceData::GetMetadata() const { return *m_pMetadata; }
void WebResourceData::SetMetadata(WebResourceMetadataR metadata) { m_pMetadata = &metadata; }

WebResourceServerCR WebResourceData::GetServer() const { return *m_pServer; }
void WebResourceData::SetServer(WebResourceServerR server) { m_pServer = &server; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceData::WebResourceData()
{
    m_size = 0;
    m_date = DateTime();
    m_footprint = DRange2d();
    m_pThumbnail = WebResourceThumbnail::Create();
    m_pMetadata = WebResourceMetadata::Create();
    m_pServer = WebResourceServer::Create();
}


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceThumbnailPtr WebResourceThumbnail::Create()
{
    return new WebResourceThumbnail();
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceThumbnailPtr WebResourceThumbnail::Create(RealityDataCR rasterData)
{
    return new WebResourceThumbnail(rasterData);
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR WebResourceThumbnail::GetProvenance() const { return m_provenance; }
void WebResourceThumbnail::SetProvenance(Utf8CP provenance) { m_provenance = provenance; }

Utf8StringCR WebResourceThumbnail::GetFormat() const { return m_format; }
void WebResourceThumbnail::SetFormat(Utf8CP format) { m_format = format; }

uint32_t WebResourceThumbnail::GetWidth() const { return m_width; }
void WebResourceThumbnail::SetWidth(uint32_t width) { m_width = width; }

uint32_t WebResourceThumbnail::GetHeight() const { return m_height; }
void WebResourceThumbnail::SetHeight(uint32_t height) { m_height = height; }

DateTimeCR WebResourceThumbnail::GetStamp() const { return m_stamp; }
void WebResourceThumbnail::SetStamp(DateTimeCR date) { m_stamp = date; }

const bvector<Byte>& WebResourceThumbnail::GetData() const { return m_data; }
void WebResourceThumbnail::SetData(const bvector<Byte>& data) { m_data = data; }

Utf8StringCR WebResourceThumbnail::GetGenerationDetails() const { return m_generationDetails; }
void WebResourceThumbnail::SetGenerationDetails(Utf8CP details) { m_generationDetails = details; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceThumbnail::WebResourceThumbnail()
{
    m_width = THUMBNAIL_WIDTH;
    m_height = THUMBNAIL_HEIGHT;
    m_stamp = DateTime::GetCurrentTimeUtc();
    m_data = bvector<Byte>();
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceThumbnail::WebResourceThumbnail(RealityDataCR rasterData)
{
    m_provenance = "Created by WebResourceDataHandler tool.";
    m_format = "png";
    m_width = THUMBNAIL_WIDTH;
    m_height = THUMBNAIL_HEIGHT;
    m_stamp = DateTime::GetCurrentTimeUtc();
    m_generationDetails = "Created by WebResourceDataHandler tool.";

    rasterData.GetThumbnail(m_data, m_width, m_height);
}


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceMetadataPtr WebResourceMetadata::Create()
{
    return new WebResourceMetadata();
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceMetadataPtr WebResourceMetadata::CreateFromFile(Utf8CP filePath)
{
    return new WebResourceMetadata(filePath);
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR WebResourceMetadata::GetProvenance() const { return m_provenance; }
void WebResourceMetadata::SetProvenance(Utf8CP provenance) { m_provenance = provenance; }

Utf8StringCR WebResourceMetadata::GetDescription() const { return m_description; }
void WebResourceMetadata::SetDescription(Utf8CP description) { m_description = description; }

Utf8StringCR WebResourceMetadata::GetContactInfo() const { return m_contactInfo; }
void WebResourceMetadata::SetContactInfo(Utf8CP info) { m_contactInfo = info; }

Utf8StringCR WebResourceMetadata::GetLegal() const { return m_legal; }
void WebResourceMetadata::SetLegal(Utf8CP legal) { m_legal = legal; }

Utf8StringCR WebResourceMetadata::GetFormat() const { return m_format; }
void WebResourceMetadata::SetFormat(Utf8CP format) { m_format = format; }

Utf8StringCR WebResourceMetadata::GetData() const { return m_data; }
void WebResourceMetadata::SetData(Utf8CP data) { m_data = data; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceMetadata::WebResourceMetadata()
{}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceMetadata::WebResourceMetadata(Utf8CP filePath)
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
WebResourceServerPtr WebResourceServer::Create()
{
    return new WebResourceServer();
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceServerPtr WebResourceServer::Create(Utf8CP url, Utf8CP name)
{
    return new WebResourceServer(url, name);
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR WebResourceServer::GetProtocol() const { return m_protocol; }
void WebResourceServer::SetProtocol(Utf8CP protocol) { m_protocol = protocol; }

Utf8StringCR WebResourceServer::GetName() const { return m_name; }
void WebResourceServer::SetName(Utf8CP name) { m_name = name; }

Utf8StringCR WebResourceServer::GetUrl() const { return m_url; }
void WebResourceServer::SetUrl(Utf8CP url) { m_url = url; }

Utf8StringCR WebResourceServer::GetContactInfo() const { return m_contactInfo; }
void WebResourceServer::SetContactInfo(Utf8CP info) { m_contactInfo = info; }

Utf8StringCR WebResourceServer::GetLegal() const { return m_legal; }
void WebResourceServer::SetLegal(Utf8CP legal) { m_legal = legal; }

bool WebResourceServer::IsOnline() const { return m_online; }
void WebResourceServer::SetOnline(bool online) { m_online = online; }

DateTimeCR WebResourceServer::GetLastCheck() const { return m_lastCheck; }
void WebResourceServer::SetLastCheck(DateTimeCR data) { m_lastCheck = data; }

DateTimeCR WebResourceServer::GetLastTimeOnline() const { return m_lastTimeOnline; }
void WebResourceServer::SetLastTimeOnline(DateTimeCR data) { m_lastTimeOnline = data; }

double WebResourceServer::GetLatency() const { return m_latency; }
void WebResourceServer::SetLatency(double latency) { m_latency = latency; }

Utf8StringCR WebResourceServer::GetState() const { return m_state; }
void WebResourceServer::SetState(Utf8CP state) { m_state = state; }

Utf8StringCR WebResourceServer::GetType() const { return m_type; }
void WebResourceServer::SetType(Utf8CP type) { m_type = type; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceServer::WebResourceServer()
{}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceServer::WebResourceServer(Utf8CP url, Utf8CP name)
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
WebResourceStatus WebResourceDataHandler::UnzipFiles(Utf8CP inputDirPath, Utf8CP outputDirPath)
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

    return WebResourceStatus::Success;
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
BeFileName WebResourceDataHandler::BuildMetadataFilename(Utf8CP dirPath)
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
Utf8String WebResourceDataHandler::RetrieveGeocodingFromMetadata(BeFileNameCR filename)
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

