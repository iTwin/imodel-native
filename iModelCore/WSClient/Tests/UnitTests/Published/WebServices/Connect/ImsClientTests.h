/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/ImsClientTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "ConnectTestsHelper.h"

class ImsClientTests : public BaseMockHttpHandlerTest
    {
    public:
        std::shared_ptr<StubBuddiClient> m_buddiClient;
        StubLocalState m_localState;

    public:
        virtual void SetUp () override;
    };
