/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <WebServicesTestsHelper.h>
#include <WebServices/Cache/Util/ECDbAdapter.h>

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

#define INSERT_INSTANCE(db, ecClassCP, keyOut) \
    ASSERT_FALSE(ecClassCP == nullptr);\
    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(db, *ecClassCP, nullptr).Insert(keyOut, Json::Value()));

#define INSERT_RELATIONSHIP(db, ecRelClassCP, source, target, rel) \
    ASSERT_FALSE(ecRelClassCP == nullptr); \
    ASSERT_TRUE((rel = ECDbAdapter(db).RelateInstances(ecRelClassCP, source, target)).IsValid());

#define CREATE_MockECDbAdapterDeleteListener(listener) \
    StrictMock<MockECDbAdapterDeleteListener> listener; \
    ON_CALL(listener, OnBeforeDelete(_, _, _)).WillByDefault(Return(SUCCESS));

#define EXPECT_INSTANCE_EXISTS(db, key) \
    auto foundInstance = ECDbAdapter(*db).FindInstance(\
    db->Schemas().GetClass(key.GetClassId()), \
    Utf8PrintfString("ECInstanceId = %llu", key.GetInstanceId().GetValue()).c_str()); \
    EXPECT_TRUE(foundInstance.IsValid()); \

#if 1
#define EXPECT_CALL_OnBeforeDelete(listener, db, instanceKey) \
    EXPECT_CALL(listener, OnBeforeDelete(Ref(*ECDbAdapter(*db).GetECClass(instanceKey)), instanceKey.GetInstanceId(), _)) \
    .WillOnce(Invoke([&](ECClassCR ecClass, ECInstanceId id, bset<ECInstanceKey>&) \
    { \
    /* Check if instance was not deleted yet */ \
    EXPECT_INSTANCE_EXISTS(db, instanceKey); \
    return SUCCESS; \
    }));
#else
    void EXPECT_CALL_OnBeforeDelete(MockECDbAdapterDeleteListener& listener, std::shared_ptr<ObservableECDb> db, ECInstanceKey instanceKey)
    {
    EXPECT_CALL(listener, OnBeforeDelete(Ref(*ECDbAdapter(*db).GetECClass(instanceKey)), instanceKey.GetInstanceId(), _))
        .WillOnce(Invoke([&] (ECClassCR ecClass, ECInstanceId id, bset<ECInstanceKey>&)
        {
        /* Check if instance was not deleted yet */
        auto foundInstance = ECDbAdapter(*db).FindInstance(&ecClass, Utf8PrintfString("ECInstanceId = %llu", id.GetValue()).c_str());
        EXPECT_TRUE(foundInstance.IsValid());
        return SUCCESS;
        }));
    }
#endif

struct ECDbAdapterTests : WSClientBaseTest
    {
    static std::shared_ptr<ObservableECDb> CreateTestDb(ECSchemaPtr schema);
    };

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
    EXPECT_EQ(SUCCESS, db->Schemas().ImportSchemas(cache->GetSchemas()));

    return db;
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     01/16
+---------------+---------------+---------------+---------------+---------------+------*/
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
    ASSERT_EQ(SUCCESS, seed.Schemas().ImportSchemas(cache->GetSchemas()));

    auto ecClass = seed.Schemas().GetClass("TestSchema", "TestClass");
    auto holdingRelClass = seed.Schemas().GetClass("TestSchema", "HoldingRel")->GetRelationshipClassCP();

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

    TESTLOG.infov("Adding %d instances took %f ms", allInstances.size(), end - start);
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
        instances.Insert(parent.GetClassId(), parent.GetInstanceId());

        start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        EXPECT_EQ(SUCCESS, adapter.DeleteInstances(instances));
        end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

        double currentTime = end - start;
        totalTime += currentTime;
        TESTLOG.infov("DeleteInstances took %f ms", currentTime);

        auto notDeletedInstances = adapter.FindInstances(ecClass);
        EXPECT_EQ(0, notDeletedInstances.size());
        }
    TESTLOG.infov("DeleteInstances mean took %f ms", totalTime / count);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     01/16
+---------------+---------------+---------------+---------------+---------------+------*/
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
    ASSERT_EQ(SUCCESS, seed.Schemas().ImportSchemas(cache->GetSchemas()));

    auto ecClass = seed.Schemas().GetClass("TestSchema", "TestClass");

    // Generate data
    auto start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    ECInstanceKeyMultiMap instances;
    for (int i = 0; i < 1023; i++)
        {
        ECInstanceKey key;
        INSERT_INSTANCE(seed, ecClass, key);
        instances.insert({ key.GetClassId(), key.GetInstanceId() });
        }
    auto end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    TESTLOG.infov("Adding %d instances took %f ms", instances.size(), end - start);
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
        TESTLOG.infov("DeleteInstances took %f ms", currentTime);

        auto notDeletedInstances = adapter.FindInstances(ecClass);
        EXPECT_EQ(0, notDeletedInstances.size());
        }
    TESTLOG.infov("DeleteInstances mean took %f ms", totalTime / count);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, ImportSchemas_WithManyProperties_Success)
    {
    // Testing SQLite/ECDb column count limits.

    Utf8String schemaXml = R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">)xml" "\n";

    schemaXml += R"xml(
    <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
    <ECClass typeName="BaseClass">
        <ECProperty propertyName="TestPropertyBase" typeName="string" />
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECClass>
    <ECClass typeName="SomeClass">
        <ECProperty propertyName="TestPropertySome" typeName="string" />
    </ECClass>
    <ECRelationshipClass typeName="TestRelationship" strength="holding" modifier="Sealed">
        <Source multiplicity="(0..1)" polymorphic="false">
            <Class class="SomeClass"/>
        </Source>
        <Target multiplicity="(0..*)" polymorphic="true">
            <Class class="BaseClass"/>
        </Target>
    </ECRelationshipClass>)xml";

    size_t pTotal = 0;
    for (size_t c = 0; c < 200; c++)
        {
        Utf8PrintfString classXml(R"xml(<ECClass typeName="TestClass_%d">)xml", c);
        schemaXml += classXml + "\n";

        for (size_t p = 0; p < 100; p++)
            {
            //pTotal++;
            Utf8PrintfString propertyXml(R"xml(<ECProperty propertyName="TestProperty_%d_%d" typeName="string" />)xml", p, pTotal);
            schemaXml += propertyXml + "\n";
            }
        schemaXml += R"xml(<BaseClass>BaseClass</BaseClass>)xml"  "\n";
        schemaXml += R"xml(</ECClass>)xml" "\n";
        }

    schemaXml += R"xml(</ECSchema>)xml" "\n";

    auto start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    auto schema = ParseSchema(schemaXml);
    auto end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    TESTLOG.infov("Parsing schema took %f ms", end - start);

    auto seedPath = StubFilePath("seed.ecdb");
    if (seedPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(seedPath));

    ObservableECDb seed;
    seed.CreateNewDb(seedPath);

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);

    start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    ASSERT_EQ(SUCCESS, seed.Schemas().ImportSchemas(cache->GetSchemas()));
    end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    TESTLOG.infov("Importing schema took %f ms", end - start);

    auto ecClass = seed.Schemas().GetClass("TestSchema", "TestClass");
    }
