/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
#if !defined (BENTLEYCONFIG_OS_LINUX) // informed Kyle/Caleb/Gintaras 9/28/18
TEST_F (BeSystemInfoTests, GetDeviceId_CalledMultipleTimes_ReturnsSameNonEmptyValue)
    {
    auto id1 = BeSystemInfo::GetDeviceId ();
    auto id2 = BeSystemInfo::GetDeviceId ();

    EXPECT_FALSE (id1.empty ());
    EXPECT_FALSE (id2.empty ());
    EXPECT_STREQ (id1.c_str (), id2.c_str ());
    }
#endif
