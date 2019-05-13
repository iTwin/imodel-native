/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <RealityPlatformTools/SimpleWSGBase.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void ConnectedResponse::Clone(const RawServerResponse& raw)
    {
    header = raw.header;
    body = raw.body;
    responseCode = raw.responseCode;
    toolCode = raw.toolCode;
    status = raw.status;

    simpleSuccess = (raw.toolCode == 0) && (raw.responseCode < 400);
    simpleMessage = raw.body;
    }

WSG_FeedbackFunction    WSGRequestManager::s_callback = nullptr;
WSG_FeedbackFunction    WSGRequestManager::s_errorCallback = nullptr;

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void WSGRequestManager::Report(Utf8String message)
    {
    if (s_callback)
        s_callback(message.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void WSGRequestManager::ReportError(Utf8String message)
    {
    if (s_errorCallback)
        s_errorCallback(message.c_str());
    }