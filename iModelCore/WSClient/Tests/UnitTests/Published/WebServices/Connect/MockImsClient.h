/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/MockImsClient.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Connect/IImsClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

#if defined (USE_GTEST)
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockImsClient : public IImsClient
    {
    public:
        MockImsClient()
            {
            using namespace ::testing;
            ON_CALL(*this, RequestToken(An<CredentialsCR>(), _, _)).WillByDefault(Return(CreateCompletedAsyncTask(SamlTokenResult::Error(HttpError()))));
            ON_CALL(*this, RequestToken(An<SamlTokenCR>(), _, _)).WillByDefault(Return(CreateCompletedAsyncTask(SamlTokenResult::Error(HttpError()))));
            }

        MOCK_METHOD3(RequestToken, AsyncTaskPtr<SamlTokenResult>(CredentialsCR, Utf8String, uint64_t));
        MOCK_METHOD3(RequestToken, AsyncTaskPtr<SamlTokenResult>(SamlTokenCR, Utf8String, uint64_t));
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
