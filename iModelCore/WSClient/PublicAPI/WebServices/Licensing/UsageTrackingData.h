/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/DateTime.h>
#include <BeHttp/HttpClient.h>
#include <WebServices/Client/WebServicesClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct UsageTrackingData
    {
    private:
        Utf8String m_deviceID;
        Utf8String m_imsUserID;
        Utf8String m_productID;
        Utf8String m_projectID;
        Utf8String m_usageDate;
        Utf8String m_version;

    public:
        WSCLIENT_EXPORT UsageTrackingData();
        WSCLIENT_EXPORT UsageTrackingData(Utf8StringCR device, Utf8StringCR userId, Utf8StringCR productId, Utf8StringCR projectId, DateTimeCR usageDate, Utf8StringCR version);
        WSCLIENT_EXPORT bool IsEmpty();
        WSCLIENT_EXPORT Json::Value ToJson();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
