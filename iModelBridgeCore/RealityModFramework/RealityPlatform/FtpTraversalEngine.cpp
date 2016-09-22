/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/FtpTraversalEngine.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeDirectoryIterator.h>
#include <BeXml/BeXml.h>

#include <RealityPlatform/FtpTraversalEngine.h>
#include <RealityPlatform/RealityDataDownload.h>
#include <RealityPlatform/RealityDataHandler.h>
#include <RealityPlatform/RealityPlatformUtil.h>


USING_NAMESPACE_BENTLEY_REALITYPLATFORM


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct CurlHolder
    {
    public:
        CurlHolder() : m_curl(curl_easy_init()) {}
        ~CurlHolder() { if (NULL != m_curl) curl_easy_cleanup(m_curl); }
        CURL* Get() const { return m_curl; }

    private:
        CURL* m_curl;
    };


//-------------------------------------------------------------------------------------
// Curl callback that receive data.
//
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
static size_t WriteData(void* buffer, size_t size, size_t nmemb, void* stream)
    {
    ((Utf8StringP) stream)->append((Utf8CP) buffer, size * nmemb);
    return size * nmemb;
    }


// Static FtpClient members initialization.
IFtpTraversalObserver* FtpClient::m_pObserver = NULL;
FtpClient::RepositoryMapping FtpClient::m_dataRepositories = FtpClient::RepositoryMapping();
int FtpClient::m_retryCount = 0;

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpClientPtr FtpClient::ConnectTo(Utf8CP serverUrl, Utf8CP serverName)
    {
    return new FtpClient(serverUrl, serverName);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpStatus FtpClient::DownloadContent(Utf8CP outputDirPath) const
    {
    FtpStatus status = FtpStatus::UnknownError;

    // Set working directory.
    Utf8String workingDir = outputDirPath;
    if (workingDir.empty())
        {
        // Find temp directory.
        BeFileName tempDirPath;
        BeFileName::BeGetTempPath(tempDirPath);
        if (!tempDirPath.IsEmpty())
            {
            tempDirPath.AppendToPath(L"Bentley\\ConceptStationApp\\.RealityData\\ftpdata\\");
            BeFileName::CreateNewDirectory(tempDirPath.GetName());
            workingDir = tempDirPath.GetNameUtf8().c_str();
            }
        }

    // Perform file listing request.
    bvector<Utf8String> fileList;
    status = GetFileList(fileList);
    if (FtpStatus::Success != status)
        return status;

    if (fileList.empty())
        return FtpStatus::Success; // There is no file to download. This is not an error because all files may already be in the cache and there is no need to redownload them.

    // Construct data mapping (FileFullPathAndName, FileNameOnly) for files to download.
    RealityDataDownload::UrlLink_UrlFile urlList;
    for (size_t i = 0; i < fileList.size(); ++i)
        {
        // The local filename is created by appending the working dir, the ftp main url and the filename.        
        WString ftpUrl(fileList[i].c_str(), BentleyCharEncoding::Utf8);
        size_t pos = ftpUrl.find(L"//") + 2;
        size_t len = ftpUrl.find(L'/', pos) - pos;
        ftpUrl = ftpUrl.substr(pos, len);
        WString shortUrl;
        for (wchar_t& car : ftpUrl)
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
        return FtpStatus::DownloadError;

    pDownload->SetStatusCallBack(ConstructRepositoryMapping);
    if (!pDownload->Perform())
        return FtpStatus::DownloadError;

    return status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpStatus FtpClient::GetFileList(bvector<Utf8String>& fileList) const
    {
    return GetFileList(m_pServer->GetUrl().c_str(), fileList);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpStatus FtpClient::GetData() const
    {
    FtpStatus status = FtpStatus::UnknownError;

    // Download files from root. Store them in our temp directory.
    status = DownloadContent();
    if (FtpStatus::Success != status)
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
        pExtractedData = FtpDataHandler::ExtractDataFromPath(m_dataRepositories[i].second.c_str(), outputPath.c_str());
        if (pExtractedData == NULL)
            {
            // Could not extract data, ignore and continue.
            BeFileName::EmptyAndRemoveDirectory(outputPathW.c_str());
            continue;
            }
    
        // Override source url so that it points to the ftp repository and not the local one.
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
Utf8StringCR FtpClient::GetServerUrl() const
    {
    return m_pServer->GetUrl();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    7/2016
//-------------------------------------------------------------------------------------
Utf8StringCR FtpClient::GetServerName() const
    {
    return m_pServer->GetName();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
const FtpClient::RepositoryMapping& FtpClient::GetRepositoryMapping() const
    {
    return m_dataRepositories;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void FtpClient::SetObserver(IFtpTraversalObserver* pObserver)
    {
    m_pObserver = pObserver;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpClient::FtpClient(Utf8CP serverUrl, Utf8CP serverName)
    {
    m_pServer = WebResourceServer::Create(serverUrl, serverName);
    m_pObserver = NULL;    
    m_dataRepositories = RepositoryMapping();    
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpClient::~FtpClient()
    {
    if (0 != m_pObserver)
        {
        delete m_pObserver;
        m_pObserver = 0;
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpStatus FtpClient::GetFileList(Utf8CP url, bvector<Utf8String>& fileList) const
    {
    // Create request to get a list of all the files from the given path.
    FtpRequestPtr pRequest = FtpRequest::Create(url);
    pRequest->SetDirListOnly(true);

    // Perform file listing request.
    FtpResponsePtr pResponse = pRequest->Perform();
    if (!pResponse->IsSuccess())
        return pResponse->GetStatus();

    // Parse response.
    bvector<Utf8String> contentList;
    BeStringUtilities::Split(pResponse->GetContent().c_str(), "\n", contentList);

    // Construct file list.
    Utf8String subPath;
    Utf8String fileFullPath;
    for (Utf8StringCR content : contentList)
        {
        if (IsDirectory(content.c_str()))
            {
            subPath = url;
            subPath.append(content);
            subPath.append("/");

            GetFileList(subPath.c_str(), fileList);
            }
        else
            {
            fileFullPath = url;
            fileFullPath.append(content);

            // Process listed data.
            if (m_pObserver != NULL)
                m_pObserver->OnFileListed(fileList, fileFullPath.c_str());
            else
                fileList.push_back(fileFullPath);
            }
        }

    return FtpStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void FtpClient::ConstructRepositoryMapping(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    RealityDataDownload::FileTransfer* pEntry = (RealityDataDownload::FileTransfer*)pClient;
    if (ErrorCode == 0)
        {
        // Construct repo mapping (remote location, local location) for downloaded file.
        if(pEntry->mirrors.empty())
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
bool FtpClient::IsDirectory(Utf8CP content) const
    {
    //&&JFC TODO: More robust check.
    Utf8String contentStr(content);
    return (BeStringUtilities::NPOS == contentStr.find("."));
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpRequestPtr FtpRequest::Create(Utf8CP url)
    {
    return new FtpRequest(url);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpResponsePtr FtpRequest::Perform()
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
    FtpStatus status = FtpStatus::Success;
    if (CURLE_OK != res || response.empty())
        status = FtpStatus::CurlError;

    return FtpResponse::Create(m_url.c_str(), response.c_str(), status);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpRequest::FtpRequest(Utf8CP url)
    : m_url(url), m_method("NLST"), m_dirListOnly(false), m_verbose(false)
    {}


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpResponsePtr FtpResponse::Create()
    {
    return new FtpResponse("", "", FtpStatus::UnknownError);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpResponsePtr FtpResponse::Create(Utf8CP effectiveUrl, Utf8CP m_content, FtpStatus traversalStatus)
    {
    return new FtpResponse(effectiveUrl, m_content, traversalStatus);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
Utf8StringCR FtpResponse::GetUrl() const
    {
    return m_effectiveUrl;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
Utf8StringCR FtpResponse::GetContent() const
    {
    return m_content;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpStatus FtpResponse::GetStatus() const
    {
    return m_status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
bool FtpResponse::IsSuccess() const
    {
    return (!m_effectiveUrl.empty() && 
            !m_content.empty() && 
            (FtpStatus::Success == m_status));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpResponse::FtpResponse(Utf8CP effectiveUrl, Utf8CP content, FtpStatus status)
    : m_effectiveUrl(effectiveUrl), m_content(content), m_status(status)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
WebResourceDataPtr FtpDataHandler::ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath)
    { 
    BeFileName inputName(inputDirPath);
    bvector<BeFileName> tifFileList;

    // Look up for data type.    
    if (inputName.GetExtension() == L"zip")
        {
        // Unzip file.
        WString filenameW(inputDirPath, BentleyCharEncoding::Utf8);
        WString outputDirPathW(outputDirPath, BentleyCharEncoding::Utf8);
        RealityDataDownload::UnZipFile(filenameW, outputDirPathW);

        // Search in zip folder for the tif file to process.        
        BeFileName rootDir(outputDirPath);
        BeDirectoryIterator::WalkDirsAndMatch(tifFileList, rootDir, L"*.tif", true);
        if (tifFileList.empty())
            return NULL;
        }
    else if (inputName.GetExtension() == L"tif")
        {
        tifFileList.push_back(inputName);
        if (tifFileList.empty())
            return NULL;
        }
    else
        // Format not supported.
        return NULL;

    // Create empty data.
    WebResourceDataPtr pExtractedData = WebResourceData::Create();    

    // Data extraction.
    RealityDataPtr pData = RasterData::Create(tifFileList[0].GetNameUtf8().c_str());

    // Name.
    Utf8String name = tifFileList[0].GetNameUtf8();
    name.erase(0, tifFileList[0].GetNameUtf8().find_last_of("\\") + 1);
    pExtractedData->SetName(name.erase(name.find_last_of('.')).c_str());

    // Url.
    pExtractedData->SetUrl(tifFileList[0].GetNameUtf8().c_str());

    // Compound type.
    BeFileName compoundFilePath(inputDirPath);
    Utf8String compoundType(compoundFilePath.GetExtension().c_str());
    pExtractedData->SetCompoundType(compoundType.c_str());

    // Size.
    uint64_t size;
    compoundFilePath.GetFileSize(size);
    size /= 1024; // GetFileSize returns a size in bytes. Convert to kilobytes.
    pExtractedData->SetSize(size);

    // Type.
    Utf8String fileType(tifFileList[0].GetExtension().c_str());
    pExtractedData->SetDataType(fileType.c_str());

    // Location. 
    //&&JFC TODO: Construct path from compound.
    WString locationW = tifFileList[0].GetFileNameAndExtension();
    Utf8String location(locationW);
    pExtractedData->SetLocationInCompound(location.c_str());

    // Date.
    time_t lastModifiedTime;
    tifFileList[0].GetFileTime(NULL, NULL, &lastModifiedTime);
    DateTime date = DateTime();
    if (NULL != lastModifiedTime)
        DateTime::FromUnixMilliseconds(date, lastModifiedTime*1000);

    pExtractedData->SetDate(date);

    // Metadata.    
    BeFileName metadataFilename = FtpDataHandler::BuildMetadataFilename(tifFileList[0].GetDirectoryName().GetNameUtf8().c_str());
    WebResourceMetadataPtr pMetadata = WebResourceMetadata::CreateFromFile(metadataFilename.GetNameUtf8().c_str());
    if (pMetadata != NULL)
        pExtractedData->SetMetadata(*pMetadata);

    // Resolution.
    RasterDataPtr pRasterData = dynamic_cast<RasterDataP>(pData.get());
    if (pRasterData != NULL)
        {
        Utf8String resolution = pRasterData->ComputeResolutionInMeters();
        if (resolution.empty())
            {
            // File has no geocoding, try to parse metadata and create sister file.
            Utf8String geocoding = FtpDataHandler::RetrieveGeocodingFromMetadata(metadataFilename);
            if (!geocoding.empty())
                {
                // Make sure geocoding is well formatted.
                geocoding.ReplaceAll("::", ":");
                RasterFacility::CreateSisterFile(tifFileList[0].GetNameUtf8().c_str(), geocoding.c_str());
                resolution = pRasterData->ComputeResolutionInMeters();
                }
            }
        pExtractedData->SetResolution(resolution.c_str());
        }

    // Footprint.
    DRange2d shape = DRange2d::NullRange();
    if (SUCCESS != pData->GetFootprint(&shape))
        {
        // File has no geocoding, try to parse metadata and create sister file.
        Utf8String geocoding = FtpDataHandler::RetrieveGeocodingFromMetadata(metadataFilename);
        if (!geocoding.empty())
            {
            // Make sure geocoding is well formatted.
            geocoding.ReplaceAll("::", ":");
            RasterFacility::CreateSisterFile(tifFileList[0].GetNameUtf8().c_str(), geocoding.c_str());
            pData->GetFootprint(&shape);
            }
    
        }
    pExtractedData->SetFootprint(shape);

    // Thumbnail.
    WebResourceThumbnailPtr pThumbnail = WebResourceThumbnail::Create(*pData);
    if (pThumbnail != NULL)
        pExtractedData->SetThumbnail(*pThumbnail);

    // Server.
    WebResourceServerPtr pServer = WebResourceServer::Create();
    if (pServer != NULL)
        pExtractedData->SetServer(*pServer);

    return pExtractedData; 
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpStatus FtpDataHandler::UnzipFiles(Utf8CP inputDirPath, Utf8CP outputDirPath)
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

    return FtpStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
BeFileName FtpDataHandler::BuildMetadataFilename(Utf8CP dirPath)
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
Utf8String FtpDataHandler::RetrieveGeocodingFromMetadata(BeFileNameCR filename)
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

