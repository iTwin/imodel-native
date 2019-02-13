/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Utils/BuddiProvider.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#include <Licensing/Utils/BuddiProvider.h>
#include <WebServices/Configuration/UrlProvider.h>

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
    // Ulas Posting Service url is not in UrlProvider by name
    // TODO: make this a generic buddi lookup method with buddiName (string) as input?
    return UrlProvider::UrlDescriptor("UsageLoggingServices.RealtimeLogging.Url", "", "", "", "", nullptr).Get();
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BuddiProvider::EntitlementPolicyBaseUrl()
{
    return UrlProvider::Urls::EntitlementPolicyService.Get();
}
