/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/ConnectTokenProviderTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "ConnectTestsHelper.h"

class ConnectTokenProviderTests : public WSClientBaseTest
    {
    private:
        std::shared_ptr<StubBuddiClient> m_buddiClient;
        StubLocalState m_localState;

    public:
        virtual void SetUp () override;
    };
