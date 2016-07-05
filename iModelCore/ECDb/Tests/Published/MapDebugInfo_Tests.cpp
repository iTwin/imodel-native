/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/MapDebugInfo_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

/*.............................................................
......Tests in this Fixture are just for Coverage Purpose......
...............................................................*/

//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ClassMappingInfoHelperTests : public ECDbTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassMappingInfoHelperTests, GetInfosBySchema)
    {
    SetupECDb("schemamapdebuginfo.ecdb", BeFileName("ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    Json::Value json;
    ASSERT_EQ(SUCCESS, ClassMappingInfoHelper::GetInfos(json, GetECDb(), "ECSqlTest", false));
    ASSERT_TRUE(json.isObject());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassMappingInfoHelperTests, GetInfosBySchema_SkipUnmappedClasses)
    {
    SetupECDb("schemamapdebuginfo.ecdb", BeFileName("ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    Json::Value json;
    ASSERT_EQ(SUCCESS, ClassMappingInfoHelper::GetInfos(json, GetECDb(), "ECSqlTest", true));
    ASSERT_TRUE(json.isObject());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassMappingInfoHelperTests, GetInfo)
    {
    SetupECDb("classmapdebuginfo.ecdb", BeFileName("ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    Json::Value json;
    ASSERT_EQ(SUCCESS, ClassMappingInfoHelper::GetInfos(json, GetECDb(), "ECSqlTest", "PA"));
    ASSERT_TRUE(json.isObject());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassMappingInfoHelperTests, GetInfos)
    {
    SetupECDb("allclassesmapdebuginfo.ecdb", BeFileName("ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    Json::Value json;
    ASSERT_EQ(SUCCESS, ClassMappingInfoHelper::GetInfos(json, GetECDb(), false));
    ASSERT_TRUE(json.isObject());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassMappingInfoHelperTests, GetInfos_SkipUnmappedClasses)
    {
    SetupECDb("allclassesmapdebuginfo.ecdb", BeFileName("ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    Json::Value json;
    ASSERT_EQ(SUCCESS, ClassMappingInfoHelper::GetInfos(json, GetECDb(), true));
    ASSERT_TRUE(json.isObject());
    }

END_ECDBUNITTESTS_NAMESPACE