/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Licensing/UsageTracking.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <MobileDgn/Utils/Http/HttpClient.h>
#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Licensing/MobileTracking.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct UsageTracking
    {
    enum UsageStatus
        {
        USAGE_SUCCESS = 0,
        USAGE_ERROR,
        USAGE_NO_USAGES
        };

    private:
        UsageTracking() {}
        static Utf8String VerifyClientMobile(Utf8StringCR ims, Utf8StringCR dev);

        //__PUBLISH_SECTION_END__
    public:
        //Only used for testing. Shouldn't be needed in apps
        WSCLIENT_EXPORT static Json::Value GetUserUsages(Utf8StringCR userGuid, Utf8StringCR deviceId);
        WSCLIENT_EXPORT static Json::Value GetUserUsages(Utf8StringCR userGuid, Utf8StringCR deviceId, Utf8StringCR date);
        //__PUBLISH_SECTION_START__

    public:
        WSCLIENT_EXPORT static StatusInt RegisterUserUsages(Utf8StringCR dev, Utf8StringCR userId, Utf8StringCR prodId, Utf8StringCR projId, DateTimeCR usageDate, Utf8StringCR prodVer);
        WSCLIENT_EXPORT static StatusInt RegisterUserUsages(bvector<MobileTracking> usages);
        WSCLIENT_EXPORT static StatusInt RegisterUserUsages(MobileTracking usage);

        WSCLIENT_EXPORT static Utf8String GetUsageTrackingUrl();
        WSCLIENT_EXPORT static void Initialize(std::shared_ptr<IHttpHandler> customHttpHandler = nullptr);
        WSCLIENT_EXPORT static void Uninintialize();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
