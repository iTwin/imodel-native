/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#ifdef INCLUDE_LONG_RUNNING_TESTS
#include "../BackDoorORDBridge/PublicApi/BackDoor/ORDBridge/BackDoor.h"

TEST_F(CiviliModelBridgesORDBridgeTests, CoffsConversionTest)
{

    ASSERT_TRUE(RunTestApp(WCharCP(L"Coffs\\Corridor\\Master Corridor1.dgn"), WCharCP(L"CoffsConversionTest.bim"), false, true));
    VerifyConvertedElementCount("CoffsConversionTest.bim", 8, 10);
}
#endif
