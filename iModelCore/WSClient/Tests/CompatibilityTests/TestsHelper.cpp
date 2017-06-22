/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/TestsHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeThread.h>
#include "TestsHelper.h"

ClientInfoPtr StubValidClientInfo()
    {
    auto productId = "2545"; // Navigator Desktop CONNECT Edition
    return std::shared_ptr<ClientInfo>(new ClientInfo("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId));
    }
