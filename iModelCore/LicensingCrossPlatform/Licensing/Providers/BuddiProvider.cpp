/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/


#include <WebServices/Configuration/UrlProvider.h>
#include "BuddiProvider.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BuddiProvider::UlasLocationBaseUrl()
    {
    return UrlProvider::Urls::UsageLoggingServicesLocation.Get();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BuddiProvider::UlasRealtimeLoggingBaseUrl()
    {
    return UrlProvider::UrlDescriptor
        (
        "UsageLoggingServices.RealtimeLogging.Url",
        "https://dev-connect-ulastm.bentley.com/Bentley.ULAS.PostingService/PostingSvcWebApi",
        "https://qa-connect-ulastm.bentley.com/Bentley.ULAS.PostingService/PostingSvcWebApi",
        "https://connect-ulastm.bentley.com/Bentley.ULAS.PostingService/PostingSvcWebApi",
        "https://qa-connect-ulastm.bentley.com/Bentley.ULAS.PostingService/PostingSvcWebApi",
        nullptr
        ).Get();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BuddiProvider::UlasRealtimeFeatureUrl()
    {
    return UlasRealtimeLoggingBaseUrl() + "/featureLog";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BuddiProvider::EntitlementPolicyBaseUrl()
    {
    return UrlProvider::Urls::EntitlementPolicyService.Get();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BuddiProvider::UlasAccessKeyBaseUrl()
    {
    // Ulas AccessKey Service url is not in UrlProvider by name
    return UrlProvider::UrlDescriptor
        (
        "UsageLoggingServices.AccessKey.Url",
        "https://dev-connect-ulastm.bentley.com/Bentley.ULAS.AccessKeyService/AccessKeySvcWebApi",
        "https://qa-connect-ulastm.bentley.com/Bentley.ULAS.AccessKeyService/AccessKeySvcWebApi",
        "https://connect-ulastm.bentley.com/Bentley.ULAS.AccessKeyService/AccessKeySvcWebApi",
        "https://qa-connect-ulastm.bentley.com/Bentley.ULAS.AccessKeyService/AccessKeySvcWebApi",
        nullptr
        ).Get();
    }
