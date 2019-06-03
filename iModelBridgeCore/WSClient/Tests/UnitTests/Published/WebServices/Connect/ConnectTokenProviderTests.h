/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "ConnectTestsHelper.h"

class ConnectTokenProviderTests : public WSClientBaseTest
    {
    private:
        std::shared_ptr<StubBuddiClient> m_buddiClient;
        RuntimeJsonLocalState m_localState;

    public:
        virtual void SetUp () override;
    };
