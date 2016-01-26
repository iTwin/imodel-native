/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/TestUtils/TestMain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServicesTestsHelper.h>

#include "WSClientIntegrationTestsHost.h"

struct GtestFailureHandler : BeTest::IFailureHandler
    {
    virtual void _OnAssertionFailure(WCharCP msg) { FAIL() << msg; }
    virtual void _OnUnexpectedResult(WCharCP msg) { FAIL() << msg; }
    virtual void _OnFailureHandled() {}
    };

extern "C"
int main(int argc, char** argv)
    {
    ::testing::InitGoogleMock(&argc, argv);

    auto host = WSClientIntegrationTestsHost::Create(argv[0]);
    BeTest::Initialize(*host);

    GtestFailureHandler gtestFailureHandler;
    BeTest::SetIFailureHandler(gtestFailureHandler);

    return RUN_ALL_TESTS();
    }
