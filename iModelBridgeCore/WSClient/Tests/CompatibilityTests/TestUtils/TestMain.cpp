/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/TestUtils/TestMain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestsHost.h"

extern "C"
int main(int argc, char** argv)
    {
    ::testing::InitGoogleMock(&argc, argv);

    auto host = TestsHost::Create(argv[0]);
    BeTest::Initialize(*host);

    return RUN_ALL_TESTS();
    }
