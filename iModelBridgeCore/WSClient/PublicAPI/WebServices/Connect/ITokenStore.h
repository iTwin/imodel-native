/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ITokenStore.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <WebServices/Connect/SamlToken.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ITokenStore> ITokenStorePtr;
struct EXPORT_VTABLE_ATTRIBUTE ITokenStore
    {
    public:
        virtual ~ITokenStore()
            {};

        //! Store token
        virtual void SetToken(SamlTokenPtr token) = 0;
        //! Return existing token or null if no token was stored
        virtual SamlTokenPtr GetToken()  const = 0;
        //! Return time when token was set or invalid if no token was stored
        virtual DateTime GetTokenSetTime() const = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
