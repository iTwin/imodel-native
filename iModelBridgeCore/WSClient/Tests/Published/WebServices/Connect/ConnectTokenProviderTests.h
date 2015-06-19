/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Connect/ConnectTokenProviderTests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "ConnectTestsHelper.h"

class ConnectTokenProviderTests : public BaseMockHttpHandlerTest
    {
    private:
        std::shared_ptr<StubBuddiClient> m_client;
        StubLocalState m_localState;

    public:
        virtual void SetUp () override;
        virtual void TearDown () override;
    };
