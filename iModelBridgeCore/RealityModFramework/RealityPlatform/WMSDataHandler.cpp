/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/WMSDataHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"

#include <RealityPlatform/RealityDataHandler.h>

#ifndef BENTLEY_WINRT
#include <curl/curl.h>
#endif

#define THUMBNAIL_WIDTH     "256"
#define THUMBNAIL_HEIGHT    "256"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               5/2015
//=====================================================================================
struct CurlHolder
    {
    private:
        CURL* m_curl;

    public:
        CurlHolder() : m_curl(curl_easy_init()) {}
        ~CurlHolder() { curl_easy_cleanup(m_curl); }
        CURL* Get() const { return m_curl; }
    };

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
static size_t WriteData(void* ptr, size_t size, size_t nmemb, void* userp)
    {
    size_t totalSize = size * nmemb;
    auto buffer = (bvector<Byte>*) userp;
    buffer->insert(buffer->end(), (Byte*) ptr, (Byte*) ptr + totalSize);
    return totalSize;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 4/2015
//-------------------------------------------------------------------------------------
WmsData::WmsData(Utf8CP url)
    : m_url(url)
    {
    // Initialize baseGCS.
    Initialize();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 4/2015
//-------------------------------------------------------------------------------------
WmsData::~WmsData()
    {
    Terminate();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 4/2015
//-------------------------------------------------------------------------------------
bool WmsData::Initialize()
    {
    return SessionManager::InitBaseGCS();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 4/2015
//-------------------------------------------------------------------------------------
void WmsData::Terminate()
    {

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 4/2015
//-------------------------------------------------------------------------------------
RealityDataPtr WmsData::Create(Utf8CP url)
    {
    return new WmsData(url);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt WmsData::_GetThumbnail(bvector<Byte>& buffer, uint32_t width, uint32_t height) const
    {
    return ExtractThumbnail(buffer, width, height);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt WmsData::ExtractThumbnail(bvector<Byte>& buffer, uint32_t width, uint32_t height) const
    {
    // Get WMS GetMap Url and modified the width and height attributes to get a thumbnail.
    Utf8String thumbnailUrl(m_url);

    // URL tags.
    Utf8String widthTag = "WIDTH=";
    Utf8String heightTag = "HEIGHT=";
    Utf8String endChar = "&";

    size_t beginPos = 0;
    size_t endPos = 0;

    // Replace width.
    Utf8String widthStr;
    widthStr.reserve(21);
    BeStringUtilities::FormatUInt64((Utf8P) widthStr.c_str(), width);

    beginPos = thumbnailUrl.find(widthTag) + widthTag.length();
    endPos = thumbnailUrl.find(endChar, beginPos);
    thumbnailUrl.replace(beginPos, endPos - beginPos, widthStr.c_str());

    // Replace height.
    Utf8String heightStr;
    heightStr.reserve(21);
    BeStringUtilities::FormatUInt64((Utf8P) heightStr.c_str(), height);

    beginPos = thumbnailUrl.find(heightTag) + heightTag.length();
    endPos = thumbnailUrl.find(endChar, beginPos);
    thumbnailUrl.replace(beginPos, endPos - beginPos, heightStr.c_str());

    // Get response from WMS GetMap request.
    return GetFromServer(buffer, thumbnailUrl);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 4/2015
//-------------------------------------------------------------------------------------
StatusInt WmsData::_GetFootprint(DRange2dP pFootprint) const
    {
    return ExtractFootprint(pFootprint);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 4/2015
//-------------------------------------------------------------------------------------
StatusInt WmsData::ExtractFootprint(DRange2dP pFootprint) const
    {
    WString url;
    BeStringUtilities::Utf8ToWChar(url, m_url.c_str());

    // URL tags.
    WString srsTag = L"SRS=";
    WString bboxTag = L"BBOX=";
    WString endChar = L"&";
    WString delimiter = L",";

    size_t beginPos = 0;
    size_t endPos = 0;

    // Extract SRS.
    beginPos = url.find(srsTag) + srsTag.length();
    endPos = url.find(endChar, beginPos);
    WString srs = url.substr(beginPos, endPos - beginPos);

    // Extract BBox.
    beginPos = url.find(bboxTag) + bboxTag.length();
    endPos = url.find(endChar, beginPos);
    WString bbox = url.substr(beginPos, endPos - beginPos);

    // Extract each coord of the bounding box.
    bvector<WString> bboxCoord;
    BeStringUtilities::Split(bbox.c_str(), delimiter.c_str(), bboxCoord);

    if (4 != bboxCoord.size())
        return ERROR; // Invalid BoundingBox.

    double minX = BeStringUtilities::Wcstod(bboxCoord[0].c_str(), NULL);
    double minY = BeStringUtilities::Wcstod(bboxCoord[1].c_str(), NULL);
    double maxX = BeStringUtilities::Wcstod(bboxCoord[2].c_str(), NULL);
    double maxY = BeStringUtilities::Wcstod(bboxCoord[3].c_str(), NULL);

    // Create SrcGCS.
    GeoCoordinates::BaseGCSPtr pSrcGcs = GeoCoordinates::BaseGCS::CreateGCS(srs.c_str());
    if (!pSrcGcs.IsValid())
        return ERROR;

    // Create DestGCS.
    GeoCoordinates::BaseGCSPtr pDestGcs = GeoCoordinates::BaseGCS::CreateGCS(L"LL84");
    if (!pDestGcs->IsValid())
        return ERROR;

    // Reproject.
    double lowerX = 0.; double lowerY = 0.; 
    double upperX = 0.; double upperY = 0.;

    baseGeoCoord_reproject(&lowerX, &lowerY, minX, minY, &*pSrcGcs, &*pDestGcs);
    baseGeoCoord_reproject(&upperX, &upperY, maxX, maxY, &*pSrcGcs, &*pDestGcs);

    pFootprint->InitFrom(lowerX, lowerY, upperX, upperY);

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 4/2015
//-------------------------------------------------------------------------------------
StatusInt WmsData::GetFromServer(bvector<Byte>& buffer, Utf8StringCR url) const
    {
    curl_global_init(CURL_GLOBAL_ALL);

    CurlHolder curl;

    // Specify URL to get.
    curl_easy_setopt(curl.Get(), CURLOPT_URL, url);

    // Switch on full protocol/debug output while testing.
    //curl_easy_setopt(curl.Get(), CURLOPT_VERBOSE, 1L);

    // Send all data to this function.
    curl_easy_setopt(curl.Get(), CURLOPT_WRITEFUNCTION, WriteData);

    // We pass our struct to the callback function.
    curl_easy_setopt(curl.Get(), CURLOPT_WRITEDATA, &buffer);

    // Perform the request, res will get the return code.
    CURLcode res = curl_easy_perform(curl.Get());

    // Check for errors.
    if (CURLE_OK != res)
        {
        switch (res)
            {
            case CURLE_COULDNT_RESOLVE_HOST:
                //&&JFC TODO Status::Error_CouldNotResolveHost
                break;
            case CURLE_COULDNT_CONNECT:
                //&&JFC TODO Status::Error_NoConnection
                break;
            default:
                //&&JFC TODO Status::Error_Unknown
                break;
            }
        return ERROR;
        }

    // Check if data exists.
    if (buffer.empty())
        return ERROR;
    
    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt WmsData::_SaveFootprint(const DRange2dR data, const BeFileName outFilename) const
    {
    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt WmsData::_SaveThumbnail(const bvector<Byte>& buffer, const BeFileName outFilename) const
    {
    if (buffer.empty())
        return ERROR;

    BeFile file;
    uint32_t bytesWritten = 0;
    uint32_t byteCountToCopy = static_cast<uint32_t>(buffer.size());

    if (BeFileStatus::Success != file.Create(outFilename))
        return ERROR;

    if (BeFileStatus::Success != file.Open(outFilename, BeFileAccess::Write))
        return ERROR;

    if ((BeFileStatus::Success != file.Write(&bytesWritten, buffer.data(), byteCountToCopy)) || (bytesWritten != byteCountToCopy))
        return ERROR;

    if (BeFileStatus::Success != file.Close())
        return ERROR;

    return SUCCESS;
    }
