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

#define THUMBNAIL_WIDTH     512
#define THUMBNAIL_HEIGHT    512

USING_NAMESPACE_BENTLEY_REALITYPLATFORM


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct FtpFile
    {
    const char* filename; // Name to store the file as if successful.
    FILE* stream;
    };

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
//static size_t WriteData(void* buffer, size_t size, size_t nmemb, void* stream)
//    {
//    FtpFile* out = (FtpFile*)stream;
//    if (out && !out->stream)
//        {
//        // Open file for writing.
//        out->stream = fopen(out->filename, "wb");
//        if (!out->stream)
//            return -1; // Failure, can't open file to write.
//        }
//    return fwrite(buffer, size, nmemb, out->stream);
//    }

//static size_t GetFilesList_response(void* ptr, size_t size, size_t nmemb, void* data)
//    {
//    FILE* writehere = (FILE*) data;
//    return fwrite(ptr, size, nmemb, writehere);
//    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
static size_t WriteData(void* buffer, size_t size, size_t nmemb, void* stream)
    {
    ((Utf8StringP) stream)->append((Utf8CP) buffer, size * nmemb);
    return size * nmemb;
    }


FtpClient::RepositoryMapping FtpClient::m_dataRepositories = FtpClient::RepositoryMapping();

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpClientPtr FtpClient::ConnectTo(Utf8CP url)
    {
    return new FtpClient(url);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpStatus FtpClient::DownloadContent(Utf8CP outputDirPath) const
    {
    FtpStatus status = FtpStatus::UnknownError;

    // Make sure client is init.
    if (NULL == m_curlHolder.Get())
        return FtpStatus::ClientInitError;

    // Set output directory.
    Utf8String workingDir = outputDirPath;
    if (NULL == outputDirPath)
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

    // Construct data mapping (FileFullPathAndName, FileNameOnly) for files to download.
    RealityDataDownload::UrlLink_UrlFile urlList;
    for (size_t i = 0; i < fileList.size(); ++i)
        {
        WString filename(workingDir.c_str(), BentleyCharEncoding::Utf8);
        RealityDataDownload::ExtractFileName(filename, fileList[i]);

        // RealityDataDownload mapping.
        urlList.push_back(std::make_pair(fileList[i], filename));
        }

    // Download files.
    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(urlList);
    if (pDownload == NULL)
        return FtpStatus::DownloadInitError;

    pDownload->SetStatusCallBack(ConstructRepositoryMapping);
    if (!pDownload->Perform())
        return FtpStatus::DownloadRequestError;

    

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
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpStatus FtpClient::GetFileList(bvector<Utf8String>& fileList) const
    {
    return GetFileListFromPath(m_pServer->GetUrl().c_str(), fileList);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpStatus FtpClient::GetData() const
    {
    FtpStatus status = FtpStatus::UnknownError;
    //FtpDataHandlerPtr pDataHandler = FtpDataHandler::Create();

    // Download files from root. Store them in our temp directory.
    status = DownloadContent();
    if (FtpStatus::Success != status)
        return status;

    // Data extraction.
    FtpDataPtr pExtractedData;
    for (size_t i = 0; i < m_dataRepositories.size(); ++i)
        {
        // Construct output path.
        Utf8String outputPath = m_dataRepositories[i].second;
        outputPath.erase(outputPath.find_last_of('.'));

        //BeFileName outputDir(m_dataRepositories[i].second);
        WString outputPathW(outputPath.c_str(), BentleyCharEncoding::Utf8);
        BeFileName::CreateNewDirectory(outputPathW.c_str());

        pExtractedData = FtpDataHandler::ExtractDataFromPath(m_dataRepositories[i].second.c_str(), outputPath.c_str());

        // Override source url so that it points to the ftp repository and not the local one.
        pExtractedData->SetUrl(m_dataRepositories[i].first.c_str());

        // Set server.
        pExtractedData->SetServer(*m_pServer);

        if (m_pObserver != NULL && pExtractedData != NULL)
            m_pObserver->OnDataExtracted(*pExtractedData);
        }

    return status;
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
FtpClient::FtpClient(Utf8CP url)
    {
    //curl_global_init(CURL_GLOBAL_ALL);
    curl_global_init(CURL_GLOBAL_DEFAULT);

    //WChar exePath[MAX_PATH];
    //GetModuleFileNameW(NULL, exePath, MAX_PATH);

    // Find temp directory and construct the working directory.
    //BeFileName tempDirPath;
    //BeFileName::BeGetTempPath(tempDirPath);
    //if (!tempDirPath.IsEmpty())
    //    {
    //    tempDirPath.AppendToPath(L"Bentley\\ConceptStationApp\\.RealityData\\ftpdata");
    //    BeFileName::CreateNewDirectory(tempDirPath.GetName());
    //    m_workingDir = tempDirPath.GetNameUtf8();
    //    }

    m_pObserver = NULL;
    
    //m_dataRepositories = bvector<DataRepositoryPair>();

    // Create FtpServer.
    m_pServer = FtpServer::Create(url);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpClient::~FtpClient()
    {
    curl_global_cleanup();

    if (0 != m_pObserver)
        {
        delete m_pObserver;
        m_pObserver = 0;
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
//FtpStatus FtpClient::GetContent(bvector<Utf8String>& contentList) const
//    {
//    // Create request to get a list of all the content from root.
//    FtpRequestPtr pRequest = FtpRequest::Create(m_pServer->GetUrl().c_str());
//    pRequest->SetDirListOnly(true);
//
//    // Perform content request.
//    FtpResponsePtr pResponse = pRequest->Perform();
//    if (!pResponse->IsSuccess())
//        return pResponse->GetStatus();
//
//    // Parse response.
//    bvector<Utf8String> parsedList;
//    BeStringUtilities::Split(pResponse->GetContent().c_str(), "\n", contentList);
//
//    return FtpStatus::Success;
//    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
//Utf8StringCR FtpClient::GetUrl() const
//    {
//    return m_pServer->GetUrl();
//    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpStatus FtpClient::GetFileListFromPath(Utf8CP url, bvector<Utf8String>& fileList) const
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

            GetFileListFromPath(subPath.c_str(), fileList);
            }
        else
            {
            fileFullPath = url;
            fileFullPath.append(content);

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
    if (pEntry->filename.EndsWith(L"zip") && ErrorCode == 0)
        {
        // Construct data mapping (FileFullPathAndName, FileNameOnly) for downloaded file.
        Utf8String url(pEntry->url);
        Utf8String filename(pEntry->filename);

        m_dataRepositories.push_back(make_bpair(url.c_str(), filename.c_str()));
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
//FtpDataPtr FtpClient::CreateDataFromFile(BeFileNameCR file) const
//    {
//    return FtpData::Create(file);
//    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
bool FtpClient::IsDirectory(Utf8CP content) const
    {
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
FtpRequestPtr FtpRequest::Create(const CurlHolder& curlHolder, Utf8CP url)
    {
    return new FtpRequest(curlHolder, url);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpResponsePtr FtpRequest::Perform()
    {
    Utf8String response = "";
    FtpStatus status = FtpStatus::UnknownError;
    CURLcode res;
    CurlHolder test;

    // Specify URL to get.
    curl_easy_setopt(test.Get(), CURLOPT_URL, m_url);

    // Set options.
    curl_easy_setopt(test.Get(), CURLOPT_CUSTOMREQUEST, m_method); // Custom string for request method.
    curl_easy_setopt(test.Get(), CURLOPT_DIRLISTONLY, m_dirListOnly); // Ask for names only in a directory listing.
    curl_easy_setopt(test.Get(), CURLOPT_VERBOSE, m_verbose); // Switch on full protocol/debug output while testing.

    // Send all data to this function.
    curl_easy_setopt(test.Get(), CURLOPT_WRITEFUNCTION, WriteData);

    // We pass our struct to the callback function.
    curl_easy_setopt(test.Get(), CURLOPT_WRITEDATA, &response);

    // Perform the request, res will get the return code.
    res = curl_easy_perform(test.Get());

    // Check for errors.
    if (CURLE_OK != res)
        {
        switch (res)
            {
            case CURLE_COULDNT_RESOLVE_HOST:
                status = FtpStatus::CouldntResolveHost;
                break;
            case CURLE_COULDNT_CONNECT:
                status = FtpStatus::CouldntConnect;
                break;
            default:
                status = FtpStatus::UnknownError;
                break;
            }
        }
    else if (response.empty())
        status = FtpStatus::ResponseError;
    else
        status = FtpStatus::Success;

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
FtpRequest::FtpRequest(const CurlHolder& curlHolder, Utf8CP url)
    : m_curlHolder(curlHolder), m_url(url), m_method("NLST"), m_dirListOnly(false), m_verbose(false)
    {}


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
    return (!m_effectiveUrl.empty() && !m_content.empty() && (FtpStatus::Success == m_status));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpResponse::FtpResponse(Utf8CP effectiveUrl, Utf8CP content, FtpStatus status)
    : m_effectiveUrl(effectiveUrl), m_content(content), m_status(status)
    {

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpDataPtr FtpData::Create()
    {
    return new FtpData();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpDataPtr FtpData::Create(BeFileNameCR file)
    {
    return new FtpData(file);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
Utf8StringCR FtpData::GetName() const { return m_name; }
void FtpData::SetName(Utf8CP name) { m_name = name; }

Utf8StringCR FtpData::GetUrl() const { return m_url; }
void FtpData::SetUrl(Utf8CP url) { m_url = url; }

Utf8StringCR FtpData::GetCompoundType() const { return m_compoundType; }
void FtpData::SetCompoundType(Utf8CP type) { m_compoundType = type; }

uint64_t FtpData::GetSize() const { return m_size; }
void FtpData::SetSize(uint64_t size) { m_size = size; }

Utf8StringCR FtpData::GetDataType() const { return m_dataType; }
void FtpData::SetDataType(Utf8CP type) { m_dataType = type; }

Utf8StringCR FtpData::GetLocationInCompound() const { return m_locationInCompound; }
void FtpData::SetLocationInCompound(Utf8CP location) { m_locationInCompound = location; }

DateTimeCR FtpData::GetDate() const { return m_date; }
void FtpData::SetDate(DateTimeCR date) { m_date = date; }

DRange2dCR FtpData::GetFootprint() const { return m_footprint; }
void FtpData::SetFootprint(DRange2dCR footprint) { m_footprint = footprint; }

FtpThumbnailCR FtpData::GetThumbnail() const { return *m_pThumbnail; }
void FtpData::SetThumbnail(FtpThumbnailR thumbnail) { m_pThumbnail = &thumbnail; }

FtpMetadataCR FtpData::GetMetadata() const { return *m_pMetadata; }
void FtpData::SetMetadata(FtpMetadataR metadata) { m_pMetadata = &metadata; }

FtpServerCR FtpData::GetServer() const { return *m_pServer; }
void FtpData::SetServer(FtpServerR server) { m_pServer = &server; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpData::FtpData()
    {
    m_name = "";
    m_url = "";
    m_compoundType = "";
    m_size = 0;
    m_dataType = "";
    m_locationInCompound = "";
    m_date = DateTime();
    m_footprint = DRange2d();
    m_pThumbnail = FtpThumbnail::Create();
    m_pMetadata = FtpMetadata::Create();
    m_pServer = FtpServer::Create("");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpData::FtpData(BeFileNameCR file)
    {
    //// Unzip files downloaded from the ftp server so we can have access to the tif files.
    //UnzipFiles(file);
    //
    //// Get a list of tif files to process.
    //bvector<BeFileName> fileFoundList;
    //BeDirectoryIterator::WalkDirsAndMatch(fileFoundList, m_workingDir, L"*.tif", true);

    // Data extraction.
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpStatus FtpData::UnzipFiles(BeFileNameCR file) const
    {
    // Get a list of zip files to process.
    //bvector<BeFileName> fileFoundList;
    //BeDirectoryIterator::WalkDirsAndMatch(fileFoundList, m_workingDir, L"*.zip", true);

    // Unzip files.    
    //for (size_t i = 0; i < fileFoundList.size(); ++i)
    //    {
   //     WString outputDirPathW(m_workingDir.GetName());
   //
   //     // Construct output path.
   //     WString outputFolderName;
   //     RealityDataDownload::ExtractFileName(outputFolderName, fileFoundList[i].GetNameUtf8());
   //     outputFolderName.erase(outputFolderName.find_last_of('.'));
   //     outputDirPathW.append(outputFolderName);
   //     BeFileName::CreateNewDirectory(outputDirPathW.c_str());
   //
   //     WString filenameW(fileFoundList[i].GetName());
   //     RealityDataDownload::UnZipFile(filenameW, outputDirPathW);
   //     }
    
    //WString outputDirPathW(file.GetName());
    //
    //// Construct output path.
    //WString outputFolderName;
    //RealityDataDownload::ExtractFileName(outputFolderName, file.GetNameUtf8());
    //outputFolderName.erase(outputFolderName.find_last_of('.'));
    //outputDirPathW.append(outputFolderName);
    //BeFileName::CreateNewDirectory(outputDirPathW.c_str());
    //
    //WString filenameW(fileFoundList[i].GetName());
    //RealityDataDownload::UnZipFile(filenameW, outputDirPathW);

    return FtpStatus::Success;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpThumbnailPtr FtpThumbnail::Create()
    {
    return new FtpThumbnail();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpThumbnailPtr FtpThumbnail::Create(Utf8CP rasterFilePath)
    {
    return new FtpThumbnail(rasterFilePath);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR FtpThumbnail::GetProvenance() const { return m_provenance; }
void FtpThumbnail::SetProvenance(Utf8CP provenance) { m_provenance = provenance; }

Utf8StringCR FtpThumbnail::GetFormat() const { return m_format; }
void FtpThumbnail::SetFormat(Utf8CP format) { m_format = format; }

uint32_t FtpThumbnail::GetWidth() const { return m_width; }
void FtpThumbnail::SetWidth(uint32_t width) { m_width = width; }

uint32_t FtpThumbnail::GetHeight() const { return m_height; }
void FtpThumbnail::SetHeight(uint32_t height) { m_height = height; }

DateTimeCR FtpThumbnail::GetStamp() const { return m_stamp; }
void FtpThumbnail::SetStamp(DateTimeCR date) { m_stamp = date; }

const bvector<Byte>& FtpThumbnail::GetData() const { return m_data; }
void FtpThumbnail::SetData(const bvector<Byte>& data) { m_data = data; }

Utf8StringCR FtpThumbnail::GetGenerationDetails() const { return m_generationDetails; }
void FtpThumbnail::SetGenerationDetails(Utf8CP details) { m_generationDetails = details; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpThumbnail::FtpThumbnail()
    {
    m_provenance = "";
    m_format = "";
    m_generationDetails = "";
    m_width = THUMBNAIL_WIDTH;
    m_height = THUMBNAIL_HEIGHT;
    m_stamp = DateTime::GetCurrentTimeUtc();
    m_data = bvector<Byte>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpThumbnail::FtpThumbnail(Utf8CP rasterFilePath)
    {
    m_provenance = "Created by RealityDataHandler tool.";
    m_format = "png";
    m_width = THUMBNAIL_WIDTH;
    m_height = THUMBNAIL_HEIGHT;  
    m_stamp = DateTime::GetCurrentTimeUtc();
    m_generationDetails = "Created by RealityDataHandler tool.";        

    RealityDataPtr pRasterData = RasterData::Create(rasterFilePath);
    pRasterData->GetThumbnail(m_data, m_width, m_height);
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpMetadataPtr FtpMetadata::Create()
    {
    return new FtpMetadata();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpMetadataPtr FtpMetadata::Create(Utf8CP filePath)
    {
    return new FtpMetadata(filePath);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR FtpMetadata::GetProvenance() const { return m_provenance; }
void FtpMetadata::SetProvenance(Utf8CP provenance) { m_provenance = provenance; }

Utf8StringCR FtpMetadata::GetDescription() const { return m_description; }
void FtpMetadata::SetDescription(Utf8CP description) { m_description = description; }

Utf8StringCR FtpMetadata::GetContactInfo() const { return m_contactInfo; }
void FtpMetadata::SetContactInfo(Utf8CP info) { m_contactInfo = info; }

Utf8StringCR FtpMetadata::GetLegal() const { return m_legal; }
void FtpMetadata::SetLegal(Utf8CP legal) { m_legal = legal; }

Utf8StringCR FtpMetadata::GetFormat() const { return m_format; }
void FtpMetadata::SetFormat(Utf8CP format) { m_format = format; }

Utf8StringCR FtpMetadata::GetData() const { return m_data; }
void FtpMetadata::SetData(Utf8CP data) { m_data = data; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpMetadata::FtpMetadata()
    {
    m_provenance = "";
    m_description = "";
    m_contactInfo = "";
    m_legal = "";
    m_format = "";
    m_data = "";
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpMetadata::FtpMetadata(Utf8CP filePath)
    {
    BeFileName metadataFile(filePath);
    Utf8String provenance(metadataFile.GetFileNameAndExtension());
    m_provenance = provenance;
    m_description = "";
    m_contactInfo = "";
    m_legal = "";
    Utf8String format(metadataFile.GetExtension());
    m_format = format;
    m_data = "";

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
FtpServerPtr FtpServer::Create(Utf8CP url)
    {
    return new FtpServer(url);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR FtpServer::GetProtocol() const { return m_protocol; }
void FtpServer::SetProtocol(Utf8CP protocol) { m_protocol = protocol; }

Utf8StringCR FtpServer::GetName() const { return m_name; }
void FtpServer::SetName(Utf8CP name) { m_name = name; }

Utf8StringCR FtpServer::GetUrl() const { return m_url; }
void FtpServer::SetUrl(Utf8CP url) { m_url = url; }

Utf8StringCR FtpServer::GetContactInfo() const { return m_contactInfo; }
void FtpServer::SetContactInfo(Utf8CP info) { m_contactInfo = info; }

Utf8StringCR FtpServer::GetLegal() const { return m_legal; }
void FtpServer::SetLegal(Utf8CP legal) { m_legal = legal; }

bool FtpServer::IsOnline() const { return m_online; }
void FtpServer::SetOnline(bool online) { m_online = online; }

DateTimeCR FtpServer::GetLastCheck() const { return m_lastCheck; }
void FtpServer::SetLastCheck(DateTimeCR data) { m_lastCheck = data; }

DateTimeCR FtpServer::GetLastTimeOnline() const { return m_lastTimeOnline; }
void FtpServer::SetLastTimeOnline(DateTimeCR data) { m_lastTimeOnline = data; }

double FtpServer::GetLatency() const { return m_latency; }
void FtpServer::SetLatency(double latency) { m_latency = latency; }

Utf8StringCR FtpServer::GetState() const { return m_state; }
void FtpServer::SetState(Utf8CP state) { m_state = state; }

Utf8StringCR FtpServer::GetType() const { return m_type; }
void FtpServer::SetType(Utf8CP type) { m_type = type; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
FtpServer::FtpServer(Utf8CP url)
    : m_url(url)
    {
    // Default values.
    m_protocol = "";
    m_name = "";
    m_contactInfo = "";
    m_legal = "";
    m_state = "";
    m_type = "";
    m_online = true;
    m_lastCheck = DateTime::GetCurrentTimeUtc();
    m_lastTimeOnline = DateTime::GetCurrentTimeUtc();
    m_latency = 0.0;

    // Extract protocol and name from url.
    if (!m_url.empty())
        {
        Utf8String protocol(url);
        m_protocol = protocol.substr(0, protocol.find_first_of(":"));

        Utf8String name(url);
        size_t beginPos = name.find_first_of("://") + 3;
        size_t pos = name.find_last_of(".");
        size_t endPos = name.find("/", pos);
        m_name = name.substr(beginPos, endPos - beginPos);
        }
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpDataPtr FtpDataHandler::ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath)
    { 
    // Create empty data.
    FtpDataPtr pExtractedData = FtpData::Create();

    // Unzip file.
    WString filenameW(inputDirPath, BentleyCharEncoding::Utf8);
    WString outputDirPathW(outputDirPath, BentleyCharEncoding::Utf8);
    RealityDataDownload::UnZipFile(filenameW, outputDirPathW);

    // Search in zip folder for the tif file to process.
    bvector<BeFileName> tifFileList;
    BeFileName rootDir(outputDirPath);
    BeDirectoryIterator::WalkDirsAndMatch(tifFileList, rootDir, L"*.tif", true);
    if (tifFileList.empty())
        return pExtractedData;

    // Data extraction.
    RealityDataPtr pRasterData = RasterData::Create(tifFileList[0].GetNameUtf8().c_str());

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

    // Footprint.
    DRange2d shape;    
    pRasterData->GetFootprint(&shape);
    pExtractedData->SetFootprint(shape);

    // Thumbnail.
    bvector<Byte> data;
    pRasterData->GetThumbnail(data, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
    FtpThumbnailPtr pThumbnail = FtpThumbnail::Create();
    pThumbnail->SetProvenance("Created by RealityDataHandler tool.");
    pThumbnail->SetFormat("pgn");
    pThumbnail->SetWidth(THUMBNAIL_WIDTH);
    pThumbnail->SetHeight(THUMBNAIL_HEIGHT);
    pThumbnail->SetData(data);
    pThumbnail->SetGenerationDetails("Created by RealityDataHandler tool.");
    if (pThumbnail != NULL)
        pExtractedData->SetThumbnail(*pThumbnail);

    // Metadata.    
    BeFileName metadataFilename = FtpDataHandler::BuildMetadataFilename(tifFileList[0].GetDirectoryName().GetNameUtf8().c_str());
    FtpMetadataPtr pMetadata = FtpMetadata::Create(metadataFilename.GetNameUtf8().c_str());
    if (pMetadata != NULL)
        pExtractedData->SetMetadata(*pMetadata);

    // Server.
    FtpServerPtr pServer = FtpServer::Create("");
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

    if (!fileFoundList.empty())
        return fileFoundList[0];

    return BeFileName();
    }
