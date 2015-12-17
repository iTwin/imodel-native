/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/TestUtils/TestMain.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    return RUN_ALL_TESTS();
    }
