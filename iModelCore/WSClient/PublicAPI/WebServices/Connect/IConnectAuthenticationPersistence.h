/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/IConnectAuthenticationPersistence.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <MobileDgn/Utils/Http/Credentials.h>
#include <WebServices/Connect/SamlToken.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE IConnectAuthenticationPersistence
    {
    public:
        virtual ~IConnectAuthenticationPersistence ()
            {
            };

        //! Persist credentials
        virtual void SetCredentials (MobileDgn::Utils::CredentialsCR credentials) = 0;
        //! Returns existing credentials or empty if none found
        virtual MobileDgn::Utils::Credentials GetCredentials () const = 0;

        //! Persist token
        virtual void SetToken (SamlTokenPtr token) = 0;
        //! Return existing token or null if not token is persisted
        virtual SamlTokenPtr GetToken ()  const = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
