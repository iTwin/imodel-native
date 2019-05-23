/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../../Utils/WebServicesTestsHelper.h"

struct UrlProviderTests : BaseMockHttpHandlerTest
    {
    static WorkerThreadPtr s_thread;
    void SetUp();
    void TearDown();
    };

struct UrlDescriptorTests : ::testing::Test {};
