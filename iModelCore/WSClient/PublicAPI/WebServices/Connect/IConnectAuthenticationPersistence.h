/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <WebServices/Connect/ITokenStore.h>
#include <BeHttp/Credentials.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP

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
