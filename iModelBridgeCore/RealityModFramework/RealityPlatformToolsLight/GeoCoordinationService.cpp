/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformToolsLight/GeoCoordinationService.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../RealityPlatformTools/GeoCoordinationService.cpp"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

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

    if(s_requestCallback)
        s_requestCallback(request, rawResponse);
    }
