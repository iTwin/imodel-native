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
    // NOTE: as of 2/2019 UrlProvider's DefaultUrls for this are old (only are used if call to buddi fails)
    return UrlProvider::Urls::UsageLoggingServicesLocation.Get();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BuddiProvider::UlasRealtimeLoggingBaseUrl()
    {
    // TODO: make this a generic buddi lookup method with buddiName (string) as input?
    // Ulas Posting Service url is not in UrlProvider by name
    return UrlProvider::UrlDescriptor("UsageLoggingServices.RealtimeLogging.Url", "", "", "", "", nullptr).Get();
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
    return UrlProvider::UrlDescriptor("UsageLoggingServices.AccessKey.Url", "", "", "", "", nullptr).Get();
    }
