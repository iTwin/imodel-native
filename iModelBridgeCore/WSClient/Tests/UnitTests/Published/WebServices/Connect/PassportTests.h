/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "ConnectTestsHelper.h"

class PassportTests : public BaseMockHttpHandlerTest
    {
    private:
        std::shared_ptr<StubBuddiClient> m_client;
        RuntimeJsonLocalState m_localState;

    public:
        virtual void SetUp () override;
        virtual void TearDown () override;
    };
