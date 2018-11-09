/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/IImsClient.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <WebServices/Connect/SamlToken.h>
#include <BeHttp/Credentials.h>
#include <BeHttp/HttpError.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/AsyncResult.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef AsyncResult<SamlTokenPtr, HttpError> SamlTokenResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct IImsClient> IImsClientPtr;
struct IImsClient
    {
    public:
        virtual ~IImsClient() {};

        //! Request new security token using credentials
        //! @param creds - credentials to use for token
        //! @param rpUri - relying party URI the token will be used for. Defaults to generic RP.
        //! @param lifetime - request specific token lifetime in minutes. Zero defaults to lifetime defined by service
        virtual AsyncTaskPtr<SamlTokenResult> RequestToken(CredentialsCR creds, Utf8String rpUri = nullptr, uint64_t lifetime = 0) = 0;

        //! Request new security token using other token. Can be used to renew same token or get delegation token for different RP
        //! @param parent - parent token to use for authentication
        //! @param rpUri - relying party URI the token will be used for. Defaults to generic RP.
        //! @param lifetime - request specific token lifetime in minutes. Zero defaults to lifetime defined by service
        virtual AsyncTaskPtr<SamlTokenResult> RequestToken(SamlTokenCR parent, Utf8String rpUri = nullptr, uint64_t lifetime = 0) = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
