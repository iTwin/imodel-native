/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/BeSystemInfo_Tests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "BeSystemInfo_Tests.h"

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                07/15
//---------------------------------------------------------------------------------------
void BeSystemInfoTests::SetUp()
    {
#if defined(ANDROID)
    BeSystemInfo::CacheAndroidDeviceId("DeviceTests_TestId");
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                07/15
//---------------------------------------------------------------------------------------
TEST_F (BeSystemInfoTests, GetDeviceId_CalledMultipleTimes_ReturnsSameNonEmptyValue)
    {
    auto id1 = BeSystemInfo::GetDeviceId ();
    auto id2 = BeSystemInfo::GetDeviceId ();

    EXPECT_FALSE (id1.empty ());
    EXPECT_FALSE (id2.empty ());
    EXPECT_STREQ (id1.c_str (), id2.c_str ());
    }
