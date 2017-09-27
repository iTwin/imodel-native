/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/StubImsClient.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Connect/IImsClient.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include "ConnectTestsHelper.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubImsClient : public IImsClient
    {
    public:
        SamlTokenPtr stubToken = StubSamlToken();

    public:
        std::shared_ptr<StubImsClient> Create()
            {
            return std::make_shared<StubImsClient>();
            }

        AsyncTaskPtr<SamlTokenResult> RequestToken(CredentialsCR creds, Utf8String rpUri = nullptr, uint64_t lifetime = 0) override
            {
            SamlTokenPtr stubToken = StubSamlTokenWithUser(creds.GetUsername());
            return CreateCompletedAsyncTask(SamlTokenResult::Success(stubToken));
            };

        AsyncTaskPtr<SamlTokenResult> RequestToken(SamlTokenCR parent, Utf8String rpUri = nullptr, uint64_t lifetime = 0) override
            {
            Utf8String name = ConnectSignInManager::GetUserInfo(parent).username;
            SamlTokenPtr stubToken = StubSamlTokenWithUser(name);
            return CreateCompletedAsyncTask(SamlTokenResult::Success(stubToken));
            };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
