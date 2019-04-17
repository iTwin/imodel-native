/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <WebServicesTestsHelper.h>

#include "WSClientIntegrationTestsHost.h"

extern "C"
int main(int argc, char** argv)
    {
    ::testing::InitGoogleMock(&argc, argv);

    auto host = WSClientIntegrationTestsHost::Create(argv[0]);
    BeTest::Initialize(*host);

    BeTest::SetAssertionFailureHandler([](WCharCP msg) {FAIL() << msg;});

    auto status = RUN_ALL_TESTS();
    HttpClient::Uninitialize();
    return status;
    }
