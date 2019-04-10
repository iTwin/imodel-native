/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/TestsHelper.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeThread.h>
#include "TestsHelper.h"

ClientInfoPtr StubValidClientInfo()
    {
    auto productId = "test"; // Fake for tests
    return std::shared_ptr<ClientInfo>(new ClientInfo("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId));
    }
