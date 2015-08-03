/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Util/ExtendedDataAdapterTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ExtendedDataAdapterTests.h"

#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ExtendedDataAdapter.h>

#include "../CachingTestsHelper.h"

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

std::shared_ptr<ObservableECDb> GetTestECDb()
    {
    auto db = std::make_shared<ObservableECDb>();
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db->CreateNewDb(":memory:"));

    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" />
        </ECSchema>)");

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    EXPECT_EQ(SUCCESS, db->Schemas().ImportECSchemas(*cache));
    EXPECT_EQ(SUCCESS, ExtendedDataAdapter(*db).ImportSchema());

    return db;
    }

TEST_F(ExtendedDataAdapterTests, UpdateData_InstanceExistsAndFirstTime_SavesData)
    {
    auto db = GetTestECDb();
    ECDbAdapter dbAdapter(*db);
    ExtendedDataAdapter adapter(*db);

    ECInstanceKey instance;
    ASSERT_EQ(SUCCESS, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass")).Insert(instance, Json::Value()));

    ExtendedData data = adapter.GetData(instance);
    data.SetValue("Test", "Value");

    ASSERT_EQ(SUCCESS, adapter.UpdateData(data));

    data = adapter.GetData(instance);
    EXPECT_FALSE(data.GetData().empty());
    EXPECT_EQ(Json::Value("Value"), data.GetValue("Test"));
    }

TEST_F(ExtendedDataAdapterTests, GetData_InstanceDeleted_DataReturnedIsEmpty)
    {
    auto db = GetTestECDb();
    ECDbAdapter dbAdapter(*db);
    ExtendedDataAdapter adapter(*db);

    ECInstanceKey instance;
    ASSERT_EQ(SUCCESS, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass")).Insert(instance, Json::Value()));

    ExtendedData data = adapter.GetData(instance);
    data.SetValue("Test", "Value");
    ASSERT_EQ(SUCCESS, adapter.UpdateData(data));

    ASSERT_EQ(SUCCESS, JsonDeleter(*db, *dbAdapter.GetECClass("TestSchema.TestClass")).Delete(instance.GetECInstanceId()));

    data = adapter.GetData(instance);
    EXPECT_TRUE(data.GetData().empty());
    EXPECT_TRUE(data.GetValue("Test").isNull());
    }