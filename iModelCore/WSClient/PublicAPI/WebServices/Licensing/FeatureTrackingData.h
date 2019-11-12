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
struct FeatureTrackingData
    {
    private:
        Utf8String m_deviceID;
        Utf8String m_imsUserID;
        Utf8String m_productID;
        Utf8String m_version;
        Utf8String m_projectID;
        Utf8String m_featureID;
        Utf8String m_usageStartDate;
        Utf8String m_usageEndDate;

    public:
        WSCLIENT_EXPORT FeatureTrackingData();
        WSCLIENT_EXPORT FeatureTrackingData(Utf8StringCR device, Utf8StringCR userId, Utf8StringCR productId, Utf8StringCR version, Utf8StringCR projectId, Utf8StringCR featureId, DateTimeCR usageStartDate, DateTimeCR usageEndDate);
        WSCLIENT_EXPORT bool IsEmpty();
        WSCLIENT_EXPORT Json::Value ToJson();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
