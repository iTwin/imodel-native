/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Licensing/FeatureTracking.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnClientFx/Utils/Http/HttpClient.h>
#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Licensing/FeatureTrackingData.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct FeatureTracking
    {
    enum FeatureTrackingStatus
        {
        FEATURE_TRACKING_SUCCESS = 0,
        FEATURE_TRACKING_ERROR,
        FEATURE_TRACKING_NO_USAGES
        };

    private:
        FeatureTracking() {}
        static Utf8String VerifyClientMobile(Utf8StringCR ims, Utf8StringCR dev);
        static Utf8String GetServiceUrl();

        //__PUBLISH_SECTION_END__
    public:
        //Only used for testing. Shouldn't be needed in apps
        WSCLIENT_EXPORT static Json::Value GetUserFeatureUsages(Utf8StringCR userGuid, Utf8StringCR deviceId);
        WSCLIENT_EXPORT static Json::Value GetUserFeatureUsages(Utf8StringCR userGuid, Utf8StringCR deviceId, Utf8StringCR date);
        //__PUBLISH_SECTION_START__

    public:
        WSCLIENT_EXPORT static StatusInt RegisterFeatureUsage(Utf8StringCR dev, Utf8StringCR userId, Utf8StringCR prodId, Utf8StringCR prodVer, Utf8StringCR projId, Utf8StringCR featureName, DateTimeCR usageStartDate, DateTimeCR usageEndDate);
        WSCLIENT_EXPORT static StatusInt RegisterFeatureUsage(bvector<FeatureTrackingData> usages);
        WSCLIENT_EXPORT static StatusInt RegisterFeatureUsage(FeatureTrackingData usage);

        WSCLIENT_EXPORT static void Initialize(std::shared_ptr<IHttpHandler> customHttpHandler = nullptr);
        WSCLIENT_EXPORT static void Uninitialize();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
