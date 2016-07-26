/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/IConnectAuthenticationPersistence.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <BeHttp/Credentials.h>
#include <WebServices/Connect/ITokenStore.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct IConnectAuthenticationPersistence> IConnectAuthenticationPersistencePtr;
struct EXPORT_VTABLE_ATTRIBUTE IConnectAuthenticationPersistence : public ITokenStore
    {
    public:
        virtual ~IConnectAuthenticationPersistence()
            {};

        //! Store credentials
        virtual void SetCredentials(CredentialsCR credentials) = 0;
        //! Returns existing credentials or empty if none found
        virtual Credentials GetCredentials() const = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
