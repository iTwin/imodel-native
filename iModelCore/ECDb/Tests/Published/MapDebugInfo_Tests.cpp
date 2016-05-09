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
struct MapDebugInfo_Tests : public ECDbTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MapDebugInfo_Tests, SchemaMapDebugInfo)
    {
    SetupECDb("schemamapdebuginfo.ecdb", BeFileName("ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    Utf8String schemaInfo;
    ECDbMapDebugInfo::GetMapInfoForSchema(schemaInfo, GetECDb(), "ECSqlTest", false);
    //printf("Schema Map Info: \n%s", schemaInfo.c_str());
    ASSERT_FALSE(schemaInfo.empty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MapDebugInfo_Tests, SchemaMapDebugInfo_SkipUnmappedClasses)
    {
    SetupECDb("schemamapdebuginfo.ecdb", BeFileName("ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    Utf8String schemaInfo;
    ECDbMapDebugInfo::GetMapInfoForSchema(schemaInfo, GetECDb(), "ECSqlTest", true);
    //printf("Schema Map Info: \n%s", schemaInfo.c_str());
    ASSERT_FALSE(schemaInfo.empty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MapDebugInfo_Tests, ClassMapDebugInfo)
    {
    SetupECDb("classmapdebuginfo.ecdb", BeFileName("ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    Utf8String classInfo;
    ECDbMapDebugInfo::GetMapInfoForClass(classInfo, GetECDb(), "ecsql.PA");
    //printf("Class Map Info: \n%s", classInfo.c_str());
    ASSERT_FALSE(classInfo.empty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MapDebugInfo_Tests, ClassesMapDebugInfo)
    {
    SetupECDb("allclassesmapdebuginfo.ecdb", BeFileName("ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    Utf8String classesInfo;
    ECDbMapDebugInfo::GetMapInfoForAllClasses(classesInfo, GetECDb(), false);
    //printf("All Classes Map Info: \n%s", classesInfo.c_str());
    ASSERT_FALSE(classesInfo.empty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MapDebugInfo_Tests, ClassesMapDebugInfo_SkipUnmappedClasses)
    {
    SetupECDb("allclassesmapdebuginfo.ecdb", BeFileName("ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    Utf8String classesInfo;
    ECDbMapDebugInfo::GetMapInfoForAllClasses(classesInfo, GetECDb(), true);
    //printf("All Classes Map Info: \n%s", classesInfo.c_str());
    ASSERT_FALSE(classesInfo.empty());
    }

END_ECDBUNITTESTS_NAMESPACE