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

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpClientPtr FtpClient::ConnectTo(Utf8CP serverUrl, Utf8CP serverName)
    {
    return new FtpClient(serverUrl, serverName);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
WebResourceDataPtr FtpClient::ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath) const
    {
    return FtpDataHandler::ExtractDataFromPath(inputDirPath, outputDirPath);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpClient::FtpClient(Utf8CP serverUrl, Utf8CP serverName) : WebResourceClient(serverUrl, serverName)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
WebResourceStatus FtpClient::GetFileList(Utf8CP url, bvector<Utf8String>& fileList) const
    {
    // Create request to get a list of all the files from the given path.
    FtpRequestPtr pRequest = FtpRequest::Create(url);
    pRequest->SetDirListOnly(true);

    // Perform file listing request.
    WebResourceResponsePtr pResponse = pRequest->Perform();
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

    return WebResourceStatus::Success;
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
WebResourceResponsePtr FtpRequest::Perform()
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
FtpRequest::FtpRequest(Utf8CP url)
    : WebResourceRequest(url)
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
