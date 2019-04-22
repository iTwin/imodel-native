/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/Desktop/FileSystem.h>
#include <BeXml/BeXml.h>

#include <RealityPlatform/SpatialEntity.h>
#include <RealityPlatformTools/RealityDataDownload.h>
#include <RealityAdmin/SpatialEntityClient.h>
#include <RealityAdmin/ISpatialEntityTraversalObserver.h>

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
SpatialEntityHandlerStatus SpatialEntityClient::DownloadContent(Utf8CP outputDirPath) const
{
    SpatialEntityHandlerStatus status = SpatialEntityHandlerStatus::UnknownError;

    // Set working directory.
    Utf8String workingDir = outputDirPath;
    if (workingDir.empty())
    {
        // Find temp directory.
        BeFileName tempDirPath;
        Desktop::FileSystem::BeGetTempPath(tempDirPath);
        if (!tempDirPath.IsEmpty())
        {
            tempDirPath.AppendToPath(L"Bentley\\ConceptStationApp\\.RealityData\\SpatialEntity\\");
            BeFileName::CreateNewDirectory(tempDirPath.GetName());
            workingDir = tempDirPath.GetNameUtf8().c_str();
        }
    }

    // Perform file listing request.
    bvector<Utf8String> fileList;
    status = GetFileList(fileList);
    if (SpatialEntityHandlerStatus::Success != status)
        return status;

    if (fileList.empty())
        return SpatialEntityHandlerStatus::Success; // There is no file to download. This is not an error because all files may already be in the cache and there is no need to redownload them.

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
        localFilename.append(shortUrl + L' ' + WString(GetDataset().c_str(), true) + L'_' + filename);

        urlList.push_back(RealityDataDownload::url_file_pair(fileList[i], localFilename));
    }

    // Download files.
    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(urlList);
    if (pDownload == NULL)
        return SpatialEntityHandlerStatus::DownloadError;

    if (!m_certificatePath.IsEmpty())
        pDownload->SetCertificatePath(m_certificatePath.GetName());

    pDownload->SetStatusCallBack(ConstructRepositoryMapping);
    RealityDataDownload::DownloadReport* dlReport = pDownload->Perform();
    if (dlReport == nullptr) // This means that no new file were downloaded (all from cache)
        return SpatialEntityHandlerStatus::Success;

    /*Utf8String report;
    dlReport->ToXml(report);*/

    return status;
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityHandlerStatus SpatialEntityClient::GetFileList(bvector<Utf8String>& fileList) const
{
    return _GetFileList(m_pServer->GetUrl().c_str(), fileList);
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityHandlerStatus SpatialEntityClient::GetData() const
{
    SpatialEntityHandlerStatus status = SpatialEntityHandlerStatus::UnknownError;

    // Download files from root. Store them in our temp directory.
    status = DownloadContent();
    if (SpatialEntityHandlerStatus::Success != status)
        return status;

    // Data extraction.
    SpatialEntityPtr pExtractedData = SpatialEntity::Create();
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
        UriPtr uri = Uri::Create(m_dataRepositories[i].first.c_str());
        pExtractedData->GetDataSource(0).SetUri(*uri);
        pExtractedData->GetDataSource(0).SetServer(m_pServer.get());

        // Set provider.
        pExtractedData->SetProvider(GetServerName().c_str());

        // Set provider name
        pExtractedData->SetProviderName(GetProviderName().c_str());

        // Set dataset
        pExtractedData->SetDataset(GetDataset().c_str());

        // Set classification (overide if specified)
        if (GetClassification() != SpatialEntity::Classification::UNDEFINED_CLASSIF)
            pExtractedData->SetClassification(GetClassification());

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
Utf8StringCR SpatialEntityClient::GetProviderName() const
{
    return m_providerName;
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
SpatialEntity::Classification SpatialEntityClient::GetClassification() const
{
    return m_classification;
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    01/2017
//-------------------------------------------------------------------------------------
SpatialEntityMetadataCR SpatialEntityClient::GetMetadataSeed() const
{
    return m_metadataSeed;
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
SpatialEntityClient::SpatialEntityClient(Utf8CP serverUrl, Utf8CP serverName, Utf8CP providerName, Utf8CP datasetName, Utf8CP filePattern, bool extractThumbnails, Utf8CP classification, SpatialEntityMetadataCR metadataSeed)
    : m_metadataSeed(metadataSeed)
{
    m_certificatePath = BeFileName();
    m_pServer = SpatialEntityServer::Create(serverUrl, serverName);
    m_providerName = providerName,
    m_pObserver = NULL;
    m_dataRepositories = RepositoryMapping();
    m_datasetName = Utf8String(datasetName);
    m_filePattern = Utf8String(filePattern);
    m_extractThumbnails = extractThumbnails;
    SpatialEntity::GetClassificationFromTag(m_classification, classification);
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
    curl_easy_setopt(curl.Get(), CURLOPT_URL, m_url.c_str());

    // Set options.
    curl_easy_setopt(curl.Get(), CURLOPT_CUSTOMREQUEST, m_method.c_str()); // Custom string for request method.
    curl_easy_setopt(curl.Get(), CURLOPT_DIRLISTONLY, m_dirListOnly); // Ask for names only in a directory listing.
    curl_easy_setopt(curl.Get(), CURLOPT_VERBOSE, m_verbose); // Switch on full protocol/debug output while testing.

                                                              // Send all data to this function.
    curl_easy_setopt(curl.Get(), CURLOPT_WRITEFUNCTION, WriteData);

    // We pass our struct to the callback function.
    curl_easy_setopt(curl.Get(), CURLOPT_WRITEDATA, &response);

    // Perform the request, res will get the return code.
    res = curl_easy_perform(curl.Get());

    // Check for errors.
    SpatialEntityHandlerStatus status = SpatialEntityHandlerStatus::Success;
    if (CURLE_OK != res || response.empty())
        status = SpatialEntityHandlerStatus::CurlError;

    return SpatialEntityResponse::Create(m_url.c_str(), response.c_str(), status);
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityResponsePtr SpatialEntityResponse::Create()
{
    return new SpatialEntityResponse("", "", SpatialEntityHandlerStatus::UnknownError);
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityResponsePtr SpatialEntityResponse::Create(Utf8CP effectiveUrl, Utf8CP m_content, SpatialEntityHandlerStatus traversalStatus)
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
SpatialEntityHandlerStatus SpatialEntityResponse::GetStatus() const
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
        (SpatialEntityHandlerStatus::Success == m_status));
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityResponse::SpatialEntityResponse(Utf8CP effectiveUrl, Utf8CP content, SpatialEntityHandlerStatus status)
    : m_effectiveUrl(effectiveUrl), m_content(content), m_status(status)
{}
