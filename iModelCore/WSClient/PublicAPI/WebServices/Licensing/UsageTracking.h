/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Licensing/UsageTracking.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/HttpClient.h>
#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Licensing/UsageTrackingData.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct UsageTracking
    {
    enum class Status
        {
        Success = 0,
        Error,
        NoUsages
        };

    private:
        UsageTracking() {}
        static Utf8String VerifyClientMobile(Utf8StringCR ims, Utf8StringCR dev);
        static Utf8String GetServiceUrl();

        //__PUBLISH_SECTION_END__
    public:
        //Only used for testing. Shouldn't be needed in apps
        WSCLIENT_EXPORT static Json::Value GetUserUsages(Utf8StringCR userGuid, Utf8StringCR deviceId);
        WSCLIENT_EXPORT static Json::Value GetUserUsages(Utf8StringCR userGuid, Utf8StringCR deviceId, Utf8StringCR date);
        //__PUBLISH_SECTION_START__

    public:
        WSCLIENT_EXPORT static void Initialize(std::shared_ptr<IHttpHandler> customHttpHandler = nullptr);
        WSCLIENT_EXPORT static void Uninitialize();

        WSCLIENT_EXPORT static AsyncTaskPtr<Status> RegisterUserUsages
            (
            Utf8StringCR dev,
            Utf8StringCR userId,
            Utf8StringCR prodId,
            Utf8StringCR projId,
            DateTimeCR usageDate,
            Utf8StringCR prodVer
            );
        WSCLIENT_EXPORT static AsyncTaskPtr<Status> RegisterUserUsages(bvector<UsageTrackingData> usages);
        WSCLIENT_EXPORT static AsyncTaskPtr<Status> RegisterUserUsages(UsageTrackingData usage);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
