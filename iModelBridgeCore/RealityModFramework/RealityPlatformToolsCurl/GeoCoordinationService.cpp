/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <curl/curl.h>
#include "../RealityPlatformTools/GeoCoordinationService.cpp"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//! writes the body of a server response to a file
//=====================================================================================
static size_t CurlReadDataCallback(void* buffer, size_t size, size_t count, BeFile* fileStream)
    {
    uint32_t bytesRead = 0;

    BeFileStatus status = fileStream->Read(buffer, &bytesRead, (uint32_t) (size * count));
    if (status != BeFileStatus::Success)
        return 0;

    return bytesRead;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//=====================================================================================
void GeoCoordinationService::Request(const DownloadReportUploadRequest& request, RawServerResponse& rawResponse)
    {
    BeFile fileStream;
    BeFileStatus status = fileStream.Open(request.GetFileName(), BeFileAccess::Read);
    if (status != BeFileStatus::Success)
        {
        s_errorCallback("DownloadReport File not found", rawResponse);
        return;
        }

    auto curl = WSGRequest::GetInstance().PrepareRequest(request, rawResponse, GeoCoordinationService::GetVerifyPeer(), &fileStream);

    if (rawResponse.toolCode == CURLcode::CURLE_FAILED_INIT)
        {
        s_errorCallback("Curl init failed for DownloadReportUploadRequest", rawResponse);
        return;
        }

    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, CurlReadDataCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &fileStream);

    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, request.GetMessageSize());

    rawResponse.toolCode = (int) curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(rawResponse.responseCode));
    curl_easy_cleanup(curl);

    if (rawResponse.toolCode == CURLE_OK)
        rawResponse.status = RequestStatus::OK;
    else
        {
        rawResponse.status = RequestStatus::BADREQ;
        s_errorCallback("Error Uploading DownloadReport", rawResponse);
        }
    }
