/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Licensing/MobileTrackingTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "MobileTrackingTests.h"
#include <WebServices/Licensing/MobileTracking.h>
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F (MobileTrackingTests, ToJson_With_ProjectID)
    {
    DateTime dt (DateTime::GetCurrentTimeUtc ());
    MobileTracking mt ("device", "userId", "productId", "projectId", dt, "version");
    Json::Value usage = mt.ToJson ();
    EXPECT_EQ ("device", usage["DeviceID"].asString ());
    EXPECT_EQ ("userid", usage["ImsUserID"].asString ());
    EXPECT_EQ ("productId", usage["ProductID"].asString ());
    EXPECT_EQ (Utf8PrintfString ("%d-%d-%d", dt.GetYear (), dt.GetMonth (), dt.GetDay ()), usage["UsageDate"].asString ());
    EXPECT_EQ ("version", usage["Version"].asString ());
    EXPECT_EQ ("projectId", usage["ProjectID"].asString ());
    }

TEST_F (MobileTrackingTests, ToJson_Without_ProjectID)
    {
    DateTime dt (DateTime::GetCurrentTimeUtc ());
    MobileTracking mt ("device", "userId", "productId", "", dt, "version");
    Json::Value usage = mt.ToJson ();
    EXPECT_EQ ("device", usage["DeviceID"].asString ());
    EXPECT_EQ ("userid", usage["ImsUserID"].asString ());
    EXPECT_EQ ("productId", usage["ProductID"].asString ());
    EXPECT_EQ (Utf8PrintfString ("%d-%d-%d", dt.GetYear (), dt.GetMonth (), dt.GetDay ()), usage["UsageDate"].asString ());
    EXPECT_EQ ("version", usage["Version"].asString ());
    EXPECT_TRUE (usage["ProjectID"].asString ().empty ());
    }

TEST_F (MobileTrackingTests, ToJson_EmptyUsage)
    {
    MobileTracking mt;
    Json::Value usage = mt.ToJson ();
    EXPECT_TRUE (usage.isNull ());
    }

TEST_F (MobileTrackingTests, IsEmpty_Empty)
    {
    MobileTracking mt;
    EXPECT_TRUE (mt.IsEmpty ());
    }

TEST_F (MobileTrackingTests, IsEmpty_NotEmpty)
    {
    DateTime dt (DateTime::GetCurrentTimeUtc ());
    MobileTracking mt ("device", "userId", "productId", "", dt, "version");
    EXPECT_FALSE (mt.IsEmpty ());
    }


