/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/IConnectTokenProvider.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/SamlToken.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct IConnectTokenProvider> IConnectTokenProviderPtr;
struct IConnectTokenProvider
    {
    public:
        virtual ~IConnectTokenProvider()
            {};

        //! Retrieves new token, caches and returns it.
        //! Returns null if token cannot be retrieved.
        virtual SamlTokenPtr UpdateToken() = 0;

        //! Returns cached token.
        //! Returns null if token is not cached - calling UpdateToken() would be next step.
        virtual SamlTokenPtr GetToken() = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
