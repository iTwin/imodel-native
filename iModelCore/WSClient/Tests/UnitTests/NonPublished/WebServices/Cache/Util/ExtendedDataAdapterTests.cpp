/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#if defined (USE_GTEST) // use of gmock

#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ExtendedDataAdapter.h>

#include "../CachingTestsHelper.h"
#include "MockExtendedDataAdapterDelegate.h"

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

struct ExtendedDataAdapterTests : WSClientBaseTest
    {
    private:
        static SeedFile s_seedECDb;

    public:
        std::shared_ptr<ObservableECDb> GetTestECDb();
    };

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
                        <MapStrategy>TablePerHierarchy</MapStrategy>
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
    EXPECT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db.SaveChanges());
    });

std::shared_ptr<ObservableECDb> ExtendedDataAdapterTests::GetTestECDb()
    {
    auto db = std::make_shared<ObservableECDb>();
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db->OpenBeSQLiteDb(s_seedECDb.GetTestFile(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    return db;
    }
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedDataAdapterTests, UpdateData_DelegateReturnsNullClasses_Error)
    {
    auto db = GetTestECDb();
    ECDbAdapter dbAdapter(*db);
    NiceMock<MockExtendedDataAdapterDelegate> del;
    ExtendedDataAdapter adapter(*db, del);

    ECInstanceKey owner;
    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass"), nullptr).Insert(owner, Json::Value()));

    EXPECT_CALL(del, GetExtendedDataClass(_)).WillRepeatedly(Return(nullptr));
    EXPECT_CALL(del, GetExtendedDataRelationshipClass(_)).WillRepeatedly(Return(nullptr));

    auto data = adapter.GetData(owner);
    EXPECT_EQ(ERROR, adapter.UpdateData(data));
    }
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedDataAdapterTests, UpdateData_InstanceExistsAndFirstTime_DataIsReadable)
    {
    auto db = GetTestECDb();
    ECDbAdapter dbAdapter(*db);
    NiceMock<MockExtendedDataAdapterDelegate> del;
    ExtendedDataAdapter adapter(*db, del);

    ECInstanceKey instance;
    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass"), nullptr).Insert(instance, Json::Value()));

    EXPECT_CALL(del, GetExtendedDataClass(_)).WillRepeatedly(Return(dbAdapter.GetECClass("TestSchema.TestEDClass")));
    EXPECT_CALL(del, GetExtendedDataRelationshipClass(_)).WillRepeatedly(Return(dbAdapter.GetECRelationshipClass("TestSchema.TestEDRelClass")));

    ExtendedData data = adapter.GetData(instance);
    data.SetValue("Test", "Value");

    ASSERT_EQ(SUCCESS, adapter.UpdateData(data));
    data = adapter.GetData(instance);
    EXPECT_FALSE(data.GetData().empty());
    EXPECT_EQ(Json::Value("Value"), data.GetValue("Test"));
    }
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedDataAdapterTests, GetData_InstanceDeleted_DataReturnedIsEmpty)
    {
    auto db = GetTestECDb();
    ECDbAdapter dbAdapter(*db);
    NiceMock<MockExtendedDataAdapterDelegate> del;
    ExtendedDataAdapter adapter(*db, del);

    ECInstanceKey instance;
    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass"), nullptr).Insert(instance, Json::Value()));

    EXPECT_CALL(del, GetExtendedDataClass(_)).WillRepeatedly(Return(dbAdapter.GetECClass("TestSchema.TestEDClass")));
    EXPECT_CALL(del, GetExtendedDataRelationshipClass(_)).WillRepeatedly(Return(dbAdapter.GetECRelationshipClass("TestSchema.TestEDRelClass")));

    ExtendedData data = adapter.GetData(instance);
    data.SetValue("Test", "Value");
    ASSERT_EQ(SUCCESS, adapter.UpdateData(data));

    ECSqlStatement stmt;
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM TestSchema.TestClass WHERE ECInstanceId=%s", instance.GetInstanceId().ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*db, ecsql.c_str(), nullptr));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    data = adapter.GetData(instance);
    EXPECT_TRUE(data.GetData().empty());
    EXPECT_TRUE(data.GetValue("Test").isNull());
    }
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedDataAdapterTests, UpdateData_OwnerAndDelegatePointsToOtherHolder_DataReadable)
    {
    auto db = GetTestECDb();
    ECDbAdapter dbAdapter(*db);
    NiceMock<MockExtendedDataAdapterDelegate> del;
    ExtendedDataAdapter adapter(*db, del);

    ECInstanceKey owner, holder;
    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass"), nullptr).Insert(owner, Json::Value()));
    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass2"), nullptr).Insert(holder, Json::Value()));

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
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedDataAdapterTests, GetData_HolderInstanceDeleted_DataReturnedIsEmpty)
    {
    auto db = GetTestECDb();
    ECDbAdapter dbAdapter(*db);
    NiceMock<MockExtendedDataAdapterDelegate> del;
    ExtendedDataAdapter adapter(*db, del);

    ECInstanceKey owner, holder;
    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass"), nullptr).Insert(owner, Json::Value()));
    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *dbAdapter.GetECClass("TestSchema.TestClass2"), nullptr).Insert(holder, Json::Value()));

    EXPECT_CALL(del, GetExtendedDataClass(_)).WillRepeatedly(Return(dbAdapter.GetECClass("TestSchema.TestEDClass")));
    EXPECT_CALL(del, GetExtendedDataRelationshipClass(_)).WillRepeatedly(Return(dbAdapter.GetECRelationshipClass("TestSchema.TestEDRelClass")));
    EXPECT_CALL(del, GetHolderKey(owner)).WillRepeatedly(Return(holder));

    ExtendedData data = adapter.GetData(owner);
    data.SetValue("Test", "Value");
    ASSERT_EQ(SUCCESS, adapter.UpdateData(data));

    ECSqlStatement stmt;
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM TestSchema.TestClass2 WHERE ECInstanceId=%s", holder.GetInstanceId().ToString().c_str());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*db, ecsql.c_str(), nullptr));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    data = adapter.GetData(owner);
    EXPECT_TRUE(data.GetData().empty());
    EXPECT_EQ(Json::Value::GetNull(), data.GetValue("Test"));
    }

#endif
