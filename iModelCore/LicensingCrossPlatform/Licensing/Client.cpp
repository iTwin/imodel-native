/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/Client.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Client.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <Licensing/Utils/InMemoryJsonLocalState.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Client::Client()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                              
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Client::TestMethod()
    {
    // Just a test code.
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, localState);

    auto url = UrlProvider::Urls::UsageLoggingServicesLocation.Get();

    return SUCCESS;
    }