/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/IConnectTokenProvider.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/SamlToken.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct IConnectTokenProvider
    {
    public:
        virtual ~IConnectTokenProvider()
            {};

        //! Update cached token and return it. Return null if token cannot be retrieved
        virtual SamlTokenPtr UpdateToken() = 0;
        //! Return cached token. Return null if token is not cached
        virtual SamlTokenPtr GetToken() = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
