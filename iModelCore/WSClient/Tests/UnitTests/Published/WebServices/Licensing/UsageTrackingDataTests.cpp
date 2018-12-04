/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Licensing/UsageTrackingDataTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UsageTrackingDataTests.h"
#include <WebServices/Licensing/UsageTrackingData.h>
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UsageTrackingDataTests, ToJson_With_ProjectID)
    {
    DateTime dt (DateTime::GetCurrentTimeUtc ());
    UsageTrackingData utd ("device", "userId", "productId", "projectId", dt, "version");
    Json::Value usage = utd.ToJson ();
    EXPECT_EQ ("device", usage["DeviceID"].asString ());
    EXPECT_EQ ("userid", usage["ImsUserID"].asString ());
    EXPECT_EQ ("productId", usage["ProductID"].asString ());
    EXPECT_EQ (Utf8PrintfString ("%d-%d-%d", dt.GetYear (), dt.GetMonth (), dt.GetDay ()), usage["UsageDate"].asString ());
    EXPECT_EQ ("version", usage["Version"].asString ());
    EXPECT_EQ ("projectId", usage["ProjectID"].asString ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UsageTrackingDataTests, ToJson_Without_ProjectID)
    {
    DateTime dt (DateTime::GetCurrentTimeUtc ());
    UsageTrackingData utd ("device", "userId", "productId", "", dt, "version");
    Json::Value usage = utd.ToJson ();
    EXPECT_EQ ("device", usage["DeviceID"].asString ());
    EXPECT_EQ ("userid", usage["ImsUserID"].asString ());
    EXPECT_EQ ("productId", usage["ProductID"].asString ());
    EXPECT_EQ (Utf8PrintfString ("%d-%d-%d", dt.GetYear (), dt.GetMonth (), dt.GetDay ()), usage["UsageDate"].asString ());
    EXPECT_EQ ("version", usage["Version"].asString ());
    EXPECT_TRUE (usage["ProjectID"].asString ().empty ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UsageTrackingDataTests, ToJson_EmptyUsage)
    {
    UsageTrackingData utd;
    Json::Value usage = utd.ToJson ();
    EXPECT_TRUE (usage.isNull ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UsageTrackingDataTests, IsEmpty_Empty)
    {
    UsageTrackingData utd;
    EXPECT_TRUE (utd.IsEmpty ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UsageTrackingDataTests, IsEmpty_NotEmpty)
    {
    DateTime dt (DateTime::GetCurrentTimeUtc ());
    UsageTrackingData utd ("device", "userId", "productId", "", dt, "version");
    EXPECT_FALSE (utd.IsEmpty ());
    }


