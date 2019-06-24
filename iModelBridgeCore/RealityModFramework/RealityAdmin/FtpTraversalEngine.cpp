/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeDirectoryIterator.h>
#include <BeXml/BeXml.h>

#include <RealityPlatformTools/RealityDataDownload.h>
#include <RealityAdmin/FtpTraversalEngine.h>
#include <RealityAdmin/RealityDataHandler.h>
#include <RealityAdmin/RealityPlatformUtil.h>


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

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpClientPtr FtpClient::ConnectTo(Utf8CP serverUrl, Utf8CP serverName, Utf8CP providerName, Utf8CP datasetName, Utf8CP filePattern, bool extractThumbnails, Utf8CP classification, SpatialEntityMetadataCR metadataSeed)
    {
    return new FtpClient(serverUrl, serverName, providerName, datasetName, filePattern, extractThumbnails, classification, metadataSeed);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityPtr FtpClient::ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath) const
    {
    return FtpDataHandler::ExtractDataFromPath(inputDirPath, outputDirPath, m_filePattern.c_str(), m_extractThumbnails, m_metadataSeed);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
FtpClient::FtpClient(Utf8CP serverUrl, Utf8CP serverName, Utf8CP providerName, Utf8CP datasetName, Utf8CP filePattern, bool extractThumbnails, Utf8CP classification, SpatialEntityMetadataCR metadataSeed) : SpatialEntityClient(serverUrl, serverName, providerName, datasetName, filePattern, extractThumbnails, classification, metadataSeed)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityHandlerStatus FtpClient::_GetFileList(Utf8CP url, bvector<Utf8String>& fileList) const
    {
    // Create request to get a list of all the files from the given path.
    FtpRequestPtr pRequest = FtpRequest::Create(url);
    pRequest->SetDirListOnly(true);

    // Perform file listing request.
    SpatialEntityResponsePtr pResponse = pRequest->Perform();
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

            _GetFileList(subPath.c_str(), fileList);
            }
        else
            {
            fileFullPath = url;
            fileFullPath.append(content);

            // &&AR ... Apply filePattern yet keep on adding ZIP files.
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
FtpRequestPtr FtpRequest::Create(Utf8CP url)
    {
    return new FtpRequest(url);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityResponsePtr FtpRequest::Perform()
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
FtpRequest::FtpRequest(Utf8CP url)
    : SpatialEntityRequest(url)
    {}


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
SpatialEntityPtr FtpDataHandler::ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath, Utf8CP filePattern, bool extractThumbnail, SpatialEntityMetadataCR metadataSeed)
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

        // Search in zip folder for the tif file to process.        
        BeFileName rootDir(outputDirPath);
        BeDirectoryIterator::WalkDirsAndMatch(fileList, rootDir, L"*.tif", true);
        if (fileList.empty())
            return NULL;
        }
    else if (inputName.GetExtension() == L"tif") //&&AR Use file pattern instead
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


    // Location. 
    //&&JFC TODO: Construct path from compound.
    WString locationW = fileList[0].GetFileNameAndExtension();
    Utf8String location(locationW);
    newDataSource->SetLocationInCompound(location.c_str());

    // Metadata.    
    BeFileName metadataFilename = FtpDataHandler::BuildMetadataFilename(fileList[0].GetDirectoryName().GetNameUtf8().c_str());
    SpatialEntityMetadataPtr pMetadata = SpatialEntityMetadata::CreateFromFile(metadataFilename.GetNameUtf8().c_str(), metadataSeed);
    if (pMetadata != NULL)
        pExtractedData->SetMetadata(pMetadata.get());

    // Resolution and geocoding.
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
                RasterFacility::CreateSisterFile(fileList[0].GetNameUtf8().c_str(), geocoding.c_str());
                resolution = pRasterData->ComputeResolutionInMeters();

                // Save geocoding
                newDataSource->SetGeoCS(geocoding.c_str());
                }
            }
        pExtractedData->SetResolution(resolution.c_str());
        }


    // Footprint.
    bvector<GeoPoint2d> shape = bvector<GeoPoint2d>();
    DRange2d extents = DRange2d();
    if (SUCCESS != pData->GetFootprint(&shape, &extents))
    {
        // File has no geocoding, try to parse metadata and create sister file.
        Utf8String geocoding = FtpDataHandler::RetrieveGeocodingFromMetadata(metadataFilename);
        if (!geocoding.empty())
        {
            // Make sure geocoding is well formatted.
            geocoding.ReplaceAll("::", ":");
            RasterFacility::CreateSisterFile(fileList[0].GetNameUtf8().c_str(), geocoding.c_str());
            pData->GetFootprint(&shape, &extents);
			
            // Save geocoding
            newDataSource->SetGeoCS(geocoding.c_str());
        }

    }
    pExtractedData->SetFootprint(shape);

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





#if (0)
    // Thumbnail.
    if (extractThumbnail)
        {
        SpatialEntityThumbnailPtr pThumbnail = SpatialEntityThumbnail::Create();
        pThumbnail->SetProvenance("Created by SpatialEntityHandler tool");
        pThumbnail->SetFormat("png");
        pThumbnail->SetGenerationDetails("Created by SpatialEntityHandler tool");
        bvector<Byte> data;
        uint32_t width = THUMBNAIL_WIDTH;
        uint32_t height = THUMBNAIL_HEIGHT;
        pData->GetThumbnail(data, width, height);
        pThumbnail->SetData(data);
        pThumbnail->SetWidth(width);
        pThumbnail->SetHeight(height);
        if (pThumbnail != NULL)
            pExtractedData->SetThumbnail(*pThumbnail);
        }
#endif
    return pExtractedData; 
    }
