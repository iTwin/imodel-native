/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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

    if(s_requestCallback)
        s_requestCallback(request, rawResponse);
    }
