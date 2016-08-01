/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Util/ExtendedDataAdapterTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#if defined (USE_GTEST) // use of gmock

#include "ExtendedDataAdapterTests.h"

#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ExtendedDataAdapter.h>

#include "../CachingTestsHelper.h"
#include "MockExtendedDataAdapterDelegate.h"

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

SeedFile ExtendedDataAdapterTests::s_seedECDb("ecdbAdapterTest.ecdb",
[] (BeFileNameCR filePath)
    {
    ECDb db;
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(filePath));

    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">    
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />

            <ECClass typeName="TestClass" isDomainClass="True">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>
                            <Strategy>TablePerHierarchy</Strategy>
                        </MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>

            <ECClass typeName="TestClass2" isDomainClass="True">
                <BaseClass>TestClass</BaseClass>
            </ECClass>

            <ECClass typeName="TestEDClass" isDomainClass="True">
                <ECProperty propertyName="Content" typeName="string" />
            </ECClass>

            <ECRelationshipClass typeName="TestEDRelClass" isDomainClass="True" strength="embedding">
                <Source cardinality="(0,1)" polymorphic="True">
                    <Class class="TestClass" />
                </Source>
                <Target cardinality="(0,1)" polymorphic="True">
                    <Class class="TestEDClass" />
                </Target>
            </ECRelationshipClass>

        </ECSchema>)xml");

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    EXPECT_EQ(SUCCESS, db.Schemas().ImportECSchemas(*cache));
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db.SaveChanges());
    });

std::shared_ptr<ObservableECDb> ExtendedDataAdapterTests::GetTestECDb()
    {
    auto db = std::make_shared<ObservableECDb>();
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db->OpenBeSQLiteDb(s_seedECDb.GetTestFile(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    return db;
    }

TEST_F(ExtendedDataAdapterTests, UpdateData_DelegateReturnsNullClasses_Error)
    {
    auto db = GetTestECDb();
    ECDbAdapter dbAdapter(*db);
    NiceMock<MockExtendedDataAdapterDelegate> del;
    ExtendedDataAdapter adapter(*db, del);

    ECInstanceKey owner;
    ASSERT_EQ(SUCCESS, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass")).Insert(owner, Json::Value()));

    EXPECT_CALL(del, GetExtendedDataClass(_)).WillRepeatedly(Return(nullptr));
    EXPECT_CALL(del, GetExtendedDataRelationshipClass(_)).WillRepeatedly(Return(nullptr));

    auto data = adapter.GetData(owner);
    EXPECT_EQ(ERROR, adapter.UpdateData(data));
    }

TEST_F(ExtendedDataAdapterTests, UpdateData_InstanceExistsAndFirstTime_DataIsReadable)
    {
    auto db = GetTestECDb();
    ECDbAdapter dbAdapter(*db);
    NiceMock<MockExtendedDataAdapterDelegate> del;
    ExtendedDataAdapter adapter(*db, del);

    ECInstanceKey instance;
    ASSERT_EQ(SUCCESS, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass")).Insert(instance, Json::Value()));

    EXPECT_CALL(del, GetExtendedDataClass(_)).WillRepeatedly(Return(dbAdapter.GetECClass("TestSchema.TestEDClass")));
    EXPECT_CALL(del, GetExtendedDataRelationshipClass(_)).WillRepeatedly(Return(dbAdapter.GetECRelationshipClass("TestSchema.TestEDRelClass")));

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
    NiceMock<MockExtendedDataAdapterDelegate> del;
    ExtendedDataAdapter adapter(*db, del);

    ECInstanceKey instance;
    ASSERT_EQ(SUCCESS, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass")).Insert(instance, Json::Value()));

    EXPECT_CALL(del, GetExtendedDataClass(_)).WillRepeatedly(Return(dbAdapter.GetECClass("TestSchema.TestEDClass")));
    EXPECT_CALL(del, GetExtendedDataRelationshipClass(_)).WillRepeatedly(Return(dbAdapter.GetECRelationshipClass("TestSchema.TestEDRelClass")));

    ExtendedData data = adapter.GetData(instance);
    data.SetValue("Test", "Value");
    ASSERT_EQ(SUCCESS, adapter.UpdateData(data));

    ASSERT_EQ(SUCCESS, JsonDeleter(*db, *dbAdapter.GetECClass("TestSchema.TestClass")).Delete(instance.GetECInstanceId()));
    data = adapter.GetData(instance);
    EXPECT_TRUE(data.GetData().empty());
    EXPECT_TRUE(data.GetValue("Test").isNull());
    }

TEST_F(ExtendedDataAdapterTests, UpdateData_OwnerAndDelegatePointsToOtherHolder_DataReadable)
    {
    auto db = GetTestECDb();
    ECDbAdapter dbAdapter(*db);
    NiceMock<MockExtendedDataAdapterDelegate> del;
    ExtendedDataAdapter adapter(*db, del);

    ECInstanceKey owner, holder;
    ASSERT_EQ(SUCCESS, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass")).Insert(owner, Json::Value()));
    ASSERT_EQ(SUCCESS, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass2")).Insert(holder, Json::Value()));

    EXPECT_CALL(del, GetExtendedDataClass(_)).WillRepeatedly(Return(dbAdapter.GetECClass("TestSchema.TestEDClass")));
    EXPECT_CALL(del, GetExtendedDataRelationshipClass(_)).WillRepeatedly(Return(dbAdapter.GetECRelationshipClass("TestSchema.TestEDRelClass")));
    EXPECT_CALL(del, GetHolderKey(owner)).WillRepeatedly(Return(holder));

    ExtendedData data = adapter.GetData(owner);
    data.SetValue("Test", "Value");
    ASSERT_EQ(SUCCESS, adapter.UpdateData(data));

    data = adapter.GetData(owner);
    EXPECT_FALSE(data.GetData().empty());
    EXPECT_EQ(Json::Value("Value"), data.GetValue("Test"));
    }

TEST_F(ExtendedDataAdapterTests, GetData_HolderInstanceDeleted_DataReturnedIsEmpty)
    {
    auto db = GetTestECDb();
    ECDbAdapter dbAdapter(*db);
    NiceMock<MockExtendedDataAdapterDelegate> del;
    ExtendedDataAdapter adapter(*db, del);

    ECInstanceKey owner, holder;
    ASSERT_EQ(SUCCESS, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass")).Insert(owner, Json::Value()));
    ASSERT_EQ(SUCCESS, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass2")).Insert(holder, Json::Value()));

    EXPECT_CALL(del, GetExtendedDataClass(_)).WillRepeatedly(Return(dbAdapter.GetECClass("TestSchema.TestEDClass")));
    EXPECT_CALL(del, GetExtendedDataRelationshipClass(_)).WillRepeatedly(Return(dbAdapter.GetECRelationshipClass("TestSchema.TestEDRelClass")));
    EXPECT_CALL(del, GetHolderKey(owner)).WillRepeatedly(Return(holder));

    ExtendedData data = adapter.GetData(owner);
    data.SetValue("Test", "Value");
    ASSERT_EQ(SUCCESS, adapter.UpdateData(data));

    ASSERT_EQ(SUCCESS, JsonDeleter(*db, *dbAdapter.GetECClass("TestSchema.TestClass2")).Delete(holder.GetECInstanceId()));

    data = adapter.GetData(owner);
    EXPECT_TRUE(data.GetData().empty());
    EXPECT_EQ(Json::Value::null, data.GetValue("Test"));
    }

#endif
