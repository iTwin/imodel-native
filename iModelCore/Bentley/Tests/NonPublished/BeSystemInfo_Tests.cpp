/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/BeSystemInfo.h>
#include <Bentley/BeTest.h>

class BeSystemInfoTests : public ::testing::Test
    {
    void SetUp() override;
    };

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void BeSystemInfoTests::SetUp()
    {
#if defined(ANDROID)
    BeSystemInfo::CacheAndroidDeviceId("DeviceTests_TestId");
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
#if !defined(BENTLEYCONFIG_OS_LINUX) && !defined(BENTLEYCONFIG_OS_APPLE_MACOS) // informed Kyle/Caleb/Gintaras 9/28/18
TEST_F (BeSystemInfoTests, GetDeviceId_CalledMultipleTimes_ReturnsSameNonEmptyValue)
    {
    auto id1 = BeSystemInfo::GetDeviceId ();
    auto id2 = BeSystemInfo::GetDeviceId ();

    EXPECT_FALSE (id1.empty ());
    EXPECT_FALSE (id2.empty ());
    EXPECT_STREQ (id1.c_str (), id2.c_str ());
    }
#endif
