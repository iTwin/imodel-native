/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/ConnectSignInManagerTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "ConnectTestsHelper.h"
#include <WebServices/Configuration/UrlProvider.h>
#include "StubSecureStore.h"
#include "StubImsClient.h"

class ConnectSignInManagerTests : public BaseMockHttpHandlerTest
    {
    public:
        std::shared_ptr<StubImsClient> m_imsClient;
        std::shared_ptr<StubBuddiClient> m_buddiClient;
        StubLocalState m_localState;
        std::shared_ptr<StubSecureStore> m_secureStore;

    public:
        void StubUrlProviderEnvironment(UrlProvider::Environment env);
        virtual void SetUp () override;
    };
