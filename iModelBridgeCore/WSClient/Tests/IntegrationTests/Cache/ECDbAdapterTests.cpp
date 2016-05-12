/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Cache/ECDbAdapterTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECDbAdapterTests.h"

#include <WebServices/Cache/Util/ECDbAdapter.h>

// StrictMock is not portable
#if defined (BENTLEY_WIN32)

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

#define INSERT_INSTANCE(db, ecClassCP, keyOut) \
    ASSERT_EQ(SUCCESS, JsonInserter(db, *ecClassCP).Insert(keyOut, Json::Value()));

#define INSERT_RELATIONSHIP(db, ecRelClassCP, source, target, rel) \
    ASSERT_TRUE((rel = ECDbAdapter(db).RelateInstances(ecRelClassCP, source, target)).IsValid());

#define CREATE_MockECDbAdapterDeleteListener(listener) \
    StrictMock<MockECDbAdapterDeleteListener> listener; \
    ON_CALL(listener, OnBeforeDelete(_, _, _)).WillByDefault(Return(SUCCESS));

#define EXPECT_INSTANCE_EXISTS(db, key) \
    auto foundInstance = ECDbAdapter(*db).FindInstance( \
        db->Schemas().GetECClass(key.GetECClassId()), \
        Utf8PrintfString("ECInstanceId = %llu", key.GetECInstanceId().GetValue())); \
    EXPECT_TRUE(foundInstance.IsValid()); \

#if 1
#define EXPECT_CALL_OnBeforeDelete(listener, db, instanceKey) \
    EXPECT_CALL(listener, OnBeforeDelete(Ref(*ECDbAdapter(*db).GetECClass(instanceKey)), instanceKey.GetECInstanceId(), _)) \
    .WillOnce(Invoke([&](ECClassCR ecClass, ECInstanceId id, bset<ECInstanceKey>&) \
    { \
    /* Check if instance was not deleted yet */ \
    EXPECT_INSTANCE_EXISTS(db, instanceKey); \
    return SUCCESS; \
    }));
#else
void EXPECT_CALL_OnBeforeDelete(MockECDbAdapterDeleteListener& listener, std::shared_ptr<ObservableECDb> db, ECInstanceKey instanceKey)
    {
    EXPECT_CALL(listener, OnBeforeDelete(Ref(*ECDbAdapter(*db).GetECClass(instanceKey)), instanceKey.GetECInstanceId(), _))
        .WillOnce(Invoke([&] (ECClassCR ecClass, ECInstanceId id, bset<ECInstanceKey>&)
        {
        /* Check if instance was not deleted yet */
        auto foundInstance = ECDbAdapter(*db).FindInstance(&ecClass, Utf8PrintfString("ECInstanceId = %llu", id.GetValue()));
        EXPECT_TRUE(foundInstance.IsValid());
        return SUCCESS;
        }));
    }
#endif

std::shared_ptr<ObservableECDb> ECDbAdapterTests::CreateTestDb(ECSchemaPtr schema)
    {
    auto db = std::make_shared<ObservableECDb>();

    BeFileName path;
    path = StubFilePath("ecdbAdapterTest.ecdb");
    path = BeFileName(L":memory:");
    path.BeDeleteFile();

    EXPECT_EQ(BE_SQLITE_OK, db->CreateNewDb(path.GetNameUtf8().c_str()));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    EXPECT_EQ(SUCCESS, db->Schemas().ImportECSchemas(*cache));

    return db;
    }

TEST_F(ECDbAdapterTests, ECDbBrokenCardinality)
    {
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="A" />
            <ECClass typeName="B" />
            <ECClass typeName="C" />
            <ECRelationshipClass typeName="Rel" isDomainClass="True" strength="holding" >
                <Source cardinality="(0,N)" polymorphic="False">
                    <Class class="A" />
                </Source>
                <Target cardinality="(1,1)" polymorphic="False">
                    <Class class="B" />
                    <Class class="C" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml");

    // Target cardinality (x,N) makes this work fine, (x,1) causes problems

    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);

    ECInstanceKey a, b, rel;
    INSERT_INSTANCE(*db, adapter.GetECClass("TestSchema.A"), a);
    INSERT_INSTANCE(*db, adapter.GetECClass("TestSchema.B"), b);
    INSERT_RELATIONSHIP(*db, adapter.GetECRelationshipClass("TestSchema.Rel"), a, b, rel);
    db->SaveChanges();

    ECInstanceFinder finder(*db);
    ECInstanceFinder::FindOptions options(ECInstanceFinder::RelatedDirection_HeldChildren, UINT8_MAX);

    ECInstanceKeyMultiMap related, seed;
    seed.Insert(a.GetECClassId(), a.GetECInstanceId());

    EXPECT_EQ(SUCCESS, finder.FindInstances(related, seed, options));
    EXPECT_EQ(2, related.size());
    }

TEST_F(ECDbAdapterTests, ECDbDeleteParent_ShouldNotDeleteChildThatHasOtherParent)
    {
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" />
            <ECRelationshipClass typeName="HoldingRel" strength="holding">
                <Source cardinality="(0,N)"><Class class="TestClass" /></Source>
                <Target cardinality="(0,N)"><Class class="TestClass" /></Target>
            </ECRelationshipClass>
        </ECSchema>)xml");
    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, parent2, child;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, parent2);
    INSERT_INSTANCE(*db, ecClass, child);

    auto rel = ECDbAdapter(*db).RelateInstances(holdingRelClass, parent, child);
    ASSERT_TRUE(rel.IsValid());
    auto rel2 = ECDbAdapter(*db).RelateInstances(holdingRelClass, parent2, child);
    ASSERT_TRUE(rel.IsValid());

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*db, "DELETE FROM ONLY [TestSchema].[TestClass] WHERE ECInstanceId = ?"));

    statement.BindId(1, parent.GetECInstanceId());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    ASSERT_EQ(SUCCESS, db->Purge(ECDb::PurgeMode::All));

    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(2, notDeletedInstances.size());
    EXPECT_NCONTAIN(notDeletedInstances, parent.GetECInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, parent2.GetECInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, child.GetECInstanceId());
    notDeletedInstances = adapter.FindInstances(holdingRelClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, rel2.GetECInstanceId());
    }

TEST_F(ECDbAdapterTests, ECDbDeleteRelationshio_ShouldNotDeleteEnds1)
    {
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" />
            <ECRelationshipClass typeName="HoldingRel" strength="holding">
                <Source cardinality="(0,N)"><Class class="TestClass" /></Source>
                <Target cardinality="(0,N)"><Class class="TestClass" /></Target>
            </ECRelationshipClass>
        </ECSchema>)xml");
    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, parent2, child;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, parent2);
    INSERT_INSTANCE(*db, ecClass, child);

    auto rel = ECDbAdapter(*db).RelateInstances(holdingRelClass, parent, child);
    ASSERT_TRUE(rel.IsValid());
    auto rel2 = ECDbAdapter(*db).RelateInstances(holdingRelClass, parent2, child);
    ASSERT_TRUE(rel.IsValid());

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*db, "DELETE FROM ONLY [TestSchema].[HoldingRel] WHERE ECInstanceId = ?"));

    statement.BindId(1, rel.GetECInstanceId());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(3, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, parent.GetECInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, parent2.GetECInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, child.GetECInstanceId());
    notDeletedInstances = adapter.FindInstances(holdingRelClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    }

TEST_F(ECDbAdapterTests, ECDbDeleteRelationshio_ShouldNotDeleteEnds2)
    {
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" />
            <ECRelationshipClass typeName="HoldingRel" strength="holding">
                <Source cardinality="(0,N)"><Class class="TestClass" /></Source>
                <Target cardinality="(0,2)"><Class class="TestClass" /></Target>
            </ECRelationshipClass>
        </ECSchema>)xml");
    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);

    auto rel = ECDbAdapter(*db).RelateInstances(holdingRelClass, parent, child);
    ASSERT_TRUE(rel.IsValid());

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*db, "DELETE FROM ONLY [TestSchema].[HoldingRel] WHERE ECInstanceId = ?"));

    statement.BindId(1, rel.GetECInstanceId());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(2, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, parent.GetECInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, child.GetECInstanceId());
    notDeletedInstances = adapter.FindInstances(holdingRelClass);
    EXPECT_EQ(0, notDeletedInstances.size());
    }

TEST_F(ECDbAdapterTests, ECDbDeleteRelationshio_ShouldNotDeleteEnds3)
    {
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="A" />
            <ECClass typeName="B" />
            <ECRelationshipClass typeName="HoldingRel" strength="holding">
                <Source cardinality="(0,N)"><Class class="A" /></Source>
                <Target cardinality="(0,N)"><Class class="B" /></Target>
            </ECRelationshipClass>
        </ECSchema>)xml");
    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);

    auto ecClassA = adapter.GetECClass("TestSchema.A");
    auto ecClassB = adapter.GetECClass("TestSchema.B");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child;
    INSERT_INSTANCE(*db, ecClassA, parent);
    INSERT_INSTANCE(*db, ecClassB, child);

    auto rel = ECDbAdapter(*db).RelateInstances(holdingRelClass, parent, child);
    ASSERT_TRUE(rel.IsValid());

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*db, "DELETE FROM ONLY [TestSchema].[HoldingRel] WHERE ECInstanceId = ?"));

    statement.BindId(1, rel.GetECInstanceId());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    auto notDeletedInstances = adapter.FindInstances(ecClassA);
    EXPECT_EQ(1, notDeletedInstances.size());
    notDeletedInstances = adapter.FindInstances(ecClassB);
    EXPECT_EQ(1, notDeletedInstances.size());
    notDeletedInstances = adapter.FindInstances(holdingRelClass);
    EXPECT_EQ(0, notDeletedInstances.size());
    }

TEST_F(ECDbAdapterTests, ECDb_UpdateNotExisting_Error)
    {
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass">
                <ECProperty propertyName="TestProperty" typeName="string" />
            </ECClass>
        </ECSchema>)xml");
    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");

    JsonUpdater updater(*db, *ecClass);

    Json::Value instance;
    EXPECT_EQ(ERROR, updater.Update(instance));

    instance.clear();
    instance[DataSourceCache_PROPERTY_LocalInstanceId] = "123";
    EXPECT_EQ(ERROR, updater.Update(instance));
    }

TEST_F(ECDbAdapterTests, DeleteInstances_DeletingLotsOfHoldingInstances_PerformanceIsAcceptable)
    {
    // Prepare seed file
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" />
            <ECRelationshipClass typeName="HoldingRel" strength="holding">
                <Source cardinality="(0,N)"><Class class="TestClass" /></Source>
                <Target cardinality="(0,N)"><Class class="TestClass" /></Target>
            </ECRelationshipClass>
        </ECSchema>)xml");

    auto seedPath = StubFilePath("seed.ecdb");
    if (seedPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(seedPath));

    ObservableECDb seed;
    seed.CreateNewDb(seedPath);

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, seed.Schemas().ImportECSchemas(*cache));

    auto ecClass = seed.Schemas().GetECClass("TestSchema", "TestClass");
    auto holdingRelClass = seed.Schemas().GetECClass("TestSchema", "HoldingRel")->GetRelationshipClassCP();

    // Generate data
    auto start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    bset<ECInstanceKey> allInstances;
    ECInstanceKey parent;
    INSERT_INSTANCE(seed, ecClass, parent);
    bset<ECInstanceKey> parents;
    parents.insert(parent);
    allInstances.insert(parent);
    for (int i = 0; i < 9; i++)
        {
        bset<ECInstanceKey> children;
        for (auto& parent : parents)
            {
            ECInstanceKey a, b, rela, relb;
            INSERT_INSTANCE(seed, ecClass, a);
            INSERT_INSTANCE(seed, ecClass, b);
            INSERT_RELATIONSHIP(seed, holdingRelClass, parent, a, rela);
            INSERT_RELATIONSHIP(seed, holdingRelClass, parent, b, relb);
            children.insert(a);
            children.insert(b);
            }
        allInstances.insert(children.begin(), children.end());
        parents = children;
        }
    auto end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    BeDebugLog(Utf8PrintfString("Adding %d instances took %f ms", allInstances.size(), end - start).c_str());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, seed.SaveChanges());

    // Test performance
    int count = 20;
    double totalTime = 0;

    for (int i = 0; i < count; i++)
        {
        auto path = StubFilePath("performance.ecdb");
        if (path.DoesPathExist())
            ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(path));
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seedPath, path));

        ObservableECDb db;
        db.OpenBeSQLiteDb(path, Db::OpenParams(Db::OpenMode::ReadWrite));
        ECDbAdapter adapter(db);

        ECInstanceKeyMultiMap instances;
        instances.Insert(parent.GetECClassId(), parent.GetECInstanceId());

        start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        EXPECT_EQ(SUCCESS, adapter.DeleteInstances(instances));
        end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

        double currentTime = end - start;
        totalTime += currentTime;
        BeDebugLog(Utf8PrintfString("DeleteInstances took %f ms", currentTime).c_str());

        auto notDeletedInstances = adapter.FindInstances(ecClass);
        EXPECT_EQ(0, notDeletedInstances.size());
        }
    BeDebugLog(Utf8PrintfString("DeleteInstances mean took %f ms", totalTime / count).c_str());
    }

TEST_F(ECDbAdapterTests, DeleteInstances_DeletingLotsOfInstances_PerformanceIsAcceptable)
    {
    // Prepare seed file
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" />
        </ECSchema>)xml");

    auto seedPath = StubFilePath("seed.ecdb");
    if (seedPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(seedPath));

    ObservableECDb seed;
    seed.CreateNewDb(seedPath);

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, seed.Schemas().ImportECSchemas(*cache));

    auto ecClass = seed.Schemas().GetECClass("TestSchema", "TestClass");

    // Generate data
    auto start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    ECInstanceKeyMultiMap instances;
    for (int i = 0; i < 1023; i++)
        {
        ECInstanceKey key;
        INSERT_INSTANCE(seed, ecClass, key);
        instances.insert({key.GetECClassId(), key.GetECInstanceId()});
        }
    auto end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    BeDebugLog(Utf8PrintfString("Adding %d instances took %f ms", instances.size(), end - start).c_str());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, seed.SaveChanges());

    // Test performance
    int count = 20;
    double totalTime = 0;

    for (int i = 0; i < count; i++)
        {
        auto path = StubFilePath("performance.ecdb");
        if (path.DoesPathExist())
            ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(path));
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seedPath, path));

        ObservableECDb db;
        db.OpenBeSQLiteDb(path, Db::OpenParams(Db::OpenMode::ReadWrite));
        ECDbAdapter adapter(db);

        start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        EXPECT_EQ(SUCCESS, adapter.DeleteInstances(instances));
        end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

        double currentTime = end - start;
        totalTime += currentTime;
        BeDebugLog(Utf8PrintfString("DeleteInstances took %f ms", currentTime).c_str());

        auto notDeletedInstances = adapter.FindInstances(ecClass);
        EXPECT_EQ(0, notDeletedInstances.size());
        }
    BeDebugLog(Utf8PrintfString("DeleteInstances mean took %f ms", totalTime / count).c_str());
    }

#endif
