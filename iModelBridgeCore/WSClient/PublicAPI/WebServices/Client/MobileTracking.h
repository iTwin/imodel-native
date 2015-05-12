/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Client/MobileTracking.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/DateTime.h>
#include <MobileDgn/Utils/Http/HttpClient.h>
#include <WebServices/Client/WebServicesClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct MobileTracking
    {
    private:
        Utf8String m_deviceID;
        Utf8String m_imsUserID;
        Utf8String m_productID;
        Utf8String m_projectID;
        Utf8String m_usageDate;
        Utf8String m_version;

    public:
        WSCLIENT_EXPORT MobileTracking();
        WSCLIENT_EXPORT MobileTracking(Utf8StringCR device, Utf8StringCR userId, Utf8StringCR productId, Utf8StringCR projectId, DateTimeCR usageDate, Utf8StringCR version);
        WSCLIENT_EXPORT bool IsEmpty();
        WSCLIENT_EXPORT Json::Value ToJson();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE