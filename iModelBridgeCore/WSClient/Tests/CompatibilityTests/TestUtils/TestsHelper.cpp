/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/TestUtils/TestsHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestsHelper.h"

ClientInfoPtr StubValidClientInfo()
    {
    auto productId = "2545"; // Navigator Desktop CONNECT Edition
    return std::shared_ptr<ClientInfo>(new ClientInfo("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId));
    }
