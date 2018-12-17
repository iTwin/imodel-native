/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/HttpTraversalEngine.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeDirectoryIterator.h>
#include <BeXml/BeXml.h>
#include <regex>

#include <RealityAdmin/HttpTraversalEngine.h>
#include <RealityPlatformTools/RealityDataDownload.h>
#include <RealityAdmin/RealityDataHandler.h>
#include <RealityAdmin/RealityPlatformUtil.h>

#define THUMBNAIL_WIDTH     512
#define THUMBNAIL_HEIGHT    512

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// Curl callback that receive data.
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
static size_t WriteData(void* buffer, size_t size, size_t nmemb, void* stream)
    {
    ((Utf8StringP)stream)->append((Utf8CP)buffer, size * nmemb);
    return size * nmemb;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
HttpClientPtr HttpClient::ConnectTo(Utf8CP serverUrl, Utf8CP serverName, Utf8CP providerName, Utf8CP datasetName, Utf8CP filePattern, bool extractThumbnails, Utf8CP classification, SpatialEntityMetadataCR metadataSeed)
    {
    return new HttpClient(serverUrl, serverName, providerName, datasetName, filePattern, extractThumbnails, classification, metadataSeed);
    }

SpatialEntityPtr HttpClient::ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath) const
    {
    return HttpDataHandler::ExtractDataFromPath(inputDirPath, outputDirPath, m_filePattern.c_str(), m_extractThumbnails, m_metadataSeed);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
HttpClient::HttpClient(Utf8CP serverUrl, Utf8CP serverName, Utf8CP providerName, Utf8CP datasetName, Utf8CP filePattern, bool extractThumbnails, Utf8CP classification, SpatialEntityMetadataCR metadataSeed) : SpatialEntityClient(serverUrl, serverName, providerName, datasetName, filePattern, extractThumbnails, classification, metadataSeed)
    {
    m_certificatePath = BeFileName();

    // Set certificate path.
    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);

    BeFileName caBundlePath(exeDir);
    caBundlePath.AppendToPath(L"Assets").AppendToPath(L"http").AppendToPath(L"cabundle.pem");

    // Make sure directory exist.
    if (caBundlePath.DoesPathExist())
        m_certificatePath = caBundlePath;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityHandlerStatus HttpClient::_GetFileList(Utf8CP url, bvector<Utf8String>& fileList) const
    {
    // Create request to get page content.
    HttpRequestPtr pRequest = HttpRequest::Create(url);
    pRequest->SetDirListOnly(true);

    if (!m_certificatePath.IsEmpty())
        pRequest->SetCertificatePath(m_certificatePath.GetNameUtf8().c_str());

    // Perform request.
    SpatialEntityResponsePtr pResponse = pRequest->Perform();
    if (!pResponse->IsSuccess())
        return pResponse->GetStatus();

    // Find all regex matches to retrieve links from page content.  
    bvector<Utf8String> linkList;
    regex linkRegex("<\\s*a\\s+"                        // The opening of the <a> tag.
                    "[^<]*href\\s*=\\s*"                // The href element.
                    "\"([^<\"]+)\""                     // The actual link to parse.
                    "[^<]*>", regex_constants::icase);  // The closing '>' of the <a> tag.    
    cregex_iterator matchBegin(pResponse->GetContent().begin(), pResponse->GetContent().end(), linkRegex);
    cregex_iterator matchEnd;
    for (auto i = matchBegin; i != matchEnd; ++i)
        {
        cmatch match = *i;

        // Verify if this is a parent directory.
        Utf8String fullUrl(url);
        Utf8String linkStr = match[1].str().c_str();
        if (fullUrl.Contains(linkStr))
            continue;

        linkList.push_back(linkStr);
        }
    
    // Construct file list.
    Utf8String subPath;
    Utf8String fileFullPath;
    for (Utf8StringCR link : linkList)
        {
        if (IsDirectory(link.c_str()))
            {
            subPath = url;
            subPath.append(link);

            _GetFileList(subPath.c_str(), fileList);
            }            
        else
            {  
            fileFullPath = url;
            fileFullPath.append(link);

            // Process listed data.
            if (GetObserver() != NULL)
                GetObserver()->OnFileListed(fileList, fileFullPath.c_str());
            else
                fileList.push_back(fileFullPath);
            }
        }

    return SpatialEntityHandlerStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
HttpRequestPtr HttpRequest::Create(Utf8CP url)
    {
    return new HttpRequest(url);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityResponsePtr HttpRequest::Perform()
    {
    Utf8String response;
    CurlHolder curl;
    CURLcode res;

    // Specify url to get.
    curl_easy_setopt(curl.Get(), CURLOPT_URL, m_url.c_str());

    // Set options.
    //curl_easy_setopt(curl.Get(), CURLOPT_CUSTOMREQUEST, m_method); // Custom string for request method.
    curl_easy_setopt(curl.Get(), CURLOPT_SSL_VERIFYPEER, 1); // Verify the SSL certificate.
    curl_easy_setopt(curl.Get(), CURLOPT_SSL_VERIFYHOST, 1);  

    if (!m_caPath.empty())
        curl_easy_setopt(curl.Get(), CURLOPT_CAINFO, m_caPath.c_str());

    //curl_easy_setopt(curl.Get(), CURLOPT_HTTPGET, 1); // Do a HTTP GET request.
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
HttpRequest::HttpRequest(Utf8CP url)
    : SpatialEntityRequest (url), m_caPath()
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityPtr HttpDataHandler::ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath, Utf8CP filePattern, bool extractThumbnail, SpatialEntityMetadataCR metadataSeed)
    { 
    BeFileName inputName(inputDirPath);
    bvector<BeFileName> fileList;

    // Look up for data type.    
    if (inputName.GetExtension() == L"zip")
        {
        // Unzip file.
        WString filenameW(inputDirPath, BentleyCharEncoding::Utf8);
        WString outputDirPathW(outputDirPath, BentleyCharEncoding::Utf8);
        RealityDataDownload::UnZipFile(filenameW, outputDirPathW);

        // Search in zip folder for the tif or hgt file to process.        
        BeFileName rootDir(outputDirPath);
        BeDirectoryIterator::WalkDirsAndMatch(fileList, rootDir, L"*.tif", true);
        BeDirectoryIterator::WalkDirsAndMatch(fileList, rootDir, L"*.hgt", true);

        // Consider files that are equal or less than 1kb garbage.
        for (size_t i = 0; i < fileList.size(); ++i)
            {
            uint64_t size;
            fileList[i].GetFileSize(size);
            size /= 1024; // bytes to kylobites.
            if (size <= 1)
                {
                fileList.erase(fileList.begin() + i);
                i -= 1;
                }                
            }        

        if (fileList.empty())
            return NULL;
        }
    else if (inputName.GetExtension() == L"tif" || inputName.GetExtension() == L"hgt") //&&AR Use file pattern instead
        {
        fileList.push_back(inputName);
        if (fileList.empty())
            return NULL;
        }
    else
        // Format not supported.
        return NULL;

    // Create empty data.
    SpatialEntityPtr pExtractedData = SpatialEntity::Create();    

    // Data extraction.
    // &&AR Not all traversed files are raster ... we must try out other types or introduce a generic creater.
    RealityDataExtractPtr pData = RasterData::Create(fileList[0].GetNameUtf8().c_str());

    // Name.
    Utf8String name = fileList[0].GetNameUtf8();
    name.erase(0, fileList[0].GetNameUtf8().find_last_of("\\") + 1);
    pExtractedData->SetName(name.erase(name.find_last_of('.')).c_str());

    // Url.
    SpatialEntityDataSourcePtr newDataSource = SpatialEntityDataSource::Create();
    UriPtr uri = Uri::Create(fileList[0].GetNameUtf8().c_str());
    newDataSource->SetUri(*uri);

    // Compound type.

    BeFileName compoundFilePath(inputDirPath);
    Utf8String compoundType(compoundFilePath.GetExtension().c_str());
    newDataSource->SetCompoundType(compoundType.c_str());

    // Size.
    uint64_t size;
    compoundFilePath.GetFileSize(size);
    size /= 1024; // GetFileSize returns a size in bytes. Convert to kilobytes.
    newDataSource->SetSize(size);

    // Type.
    Utf8String fileType(fileList[0].GetExtension().c_str());
    newDataSource->SetDataType(fileType.c_str());

    // &&AR ... simplified ... this has to be patched up.
    if (fileType == "hgt")
        newDataSource->SetNoDataValue("-32768");

    // Location. 
    //&&JFC TODO: Construct path from compound.
    WString locationW = fileList[0].GetFileNameAndExtension();
    Utf8String location(locationW);
    newDataSource->SetLocationInCompound(location.c_str());

    // Server.
    SpatialEntityServerPtr pServer = SpatialEntityServer::Create();
    if (pServer != NULL)
        newDataSource->SetServer(pServer.get());

    pExtractedData->AddDataSource(*newDataSource);

    pExtractedData->SetDataType(newDataSource->GetDataType().c_str());

    // Classification
    // &&AR Since we currently only process rasters the file is bound to be imagery
    pExtractedData->SetClassification(SpatialEntity::Classification::IMAGERY);



    // Date.
    time_t lastModifiedTime;
    fileList[0].GetFileTime(NULL, NULL, &lastModifiedTime);
    DateTime date = DateTime();
    if (NULL != lastModifiedTime)
        DateTime::FromUnixMilliseconds(date, lastModifiedTime*1000);

    pExtractedData->SetDate(date);



    // Resolution and geocoding.
    RasterDataPtr pRasterData = dynamic_cast<RasterDataP>(pData.get());
    if (pRasterData != NULL)
        {
        Utf8String resolution = pRasterData->ComputeResolutionInMeters();
        pExtractedData->SetResolution(resolution.c_str());
        }
  
    // Footprint.
    bvector<GeoPoint2d> shape = bvector<GeoPoint2d>();
    DRange2d extents = DRange2d();
    if (SUCCESS != pData->GetFootprint(&shape, &extents))
    {
    }
    pExtractedData->SetFootprint(shape);

    SpatialEntityMetadataPtr newMetadata = SpatialEntityMetadata::CreateFromMetadata(metadataSeed);
    pExtractedData->SetMetadata(newMetadata.get());





    return pExtractedData; 
    }
