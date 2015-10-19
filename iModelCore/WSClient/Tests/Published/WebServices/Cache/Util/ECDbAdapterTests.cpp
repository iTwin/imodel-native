/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Util/ECDbAdapterTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECDbAdapterTests.h"

#include <Bentley/BeDebugLog.h>

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

using std::shared_ptr;

TEST_F(ECDbAdapterTests, GetECClass_ValidClassKey_ReturnsClass)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ECClassCP ecClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    EXPECT_EQ("TestClass", ecClass->GetName());
    }

TEST_F(ECDbAdapterTests, GetECClass_InValidClassKey_ReturnsNullptr)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ECClassCP ecClass = cache->GetAdapter().GetECClass("NotClassKey");
    EXPECT_EQ(nullptr, ecClass);
    }

TEST_F(ECDbAdapterTests, GetECClass_ValidClassKeyWithNotExistingClass_ReturnsNullptr)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ECClassCP ecClass = cache->GetAdapter().GetECClass("TestSchema.NotExistingClass");
    EXPECT_EQ(nullptr, ecClass);
    }

TEST_F(ECDbAdapterTests, GetECClass_ValidClassId_ReturnsClass)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ECClassId ecClassId = cache->GetAdapter().GetECClass("TestSchema", "TestClass")->GetId();

    ECClassCP ecClass = cache->GetAdapter().GetECClass(ecClassId);

    EXPECT_EQ("TestClass", ecClass->GetName());
    }

TEST_F(ECDbAdapterTests, GetECClass_InValidClassId_ReturnsNull)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ECClassCP ecClass = cache->GetAdapter().GetECClass(9999);
    EXPECT_EQ(nullptr, ecClass);
    }

TEST_F(ECDbAdapterTests, GetECClasses_EmptyMap_EmptyResult)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    EXPECT_TRUE(cache->GetAdapter().GetECClasses(ECInstanceKeyMultiMap()).empty());
    }

TEST_F(ECDbAdapterTests, GetECClasses_MapWithTwoSameClassInstances_ReturnsOneClass)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ECClassCP ecClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");

    ECInstanceKeyMultiMap map;
    map.insert({ecClass->GetId(), ECInstanceId(1)});
    map.insert({ecClass->GetId(), ECInstanceId(2)});

    auto classes = cache->GetAdapter().GetECClasses(map);

    ASSERT_EQ(1, classes.size());
    EXPECT_EQ(ecClass, classes[0]);
    }

TEST_F(ECDbAdapterTests, FindRelationshipClasses_ShcemaHasSuchRelationshipClass_RelationshipClassReturned)
    {
    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto schema = StubRelationshipSchema("TestSchema", "A", "B", "AB");
    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(*cache));

    auto sourceClass = db.Schemas().GetECClass("TestSchema", "A");
    auto targetClass = db.Schemas().GetECClass("TestSchema", "B");
    auto relationshipClass = db.Schemas().GetECClass("TestSchema", "AB")->GetRelationshipClassCP();

    ECDbAdapter adapter(db);
    bvector<ECRelationshipClassCP> relationshipClasses = adapter.FindRelationshipClasses(sourceClass->GetId(), targetClass->GetId());

    EXPECT_EQ(1, relationshipClasses.size());
    EXPECT_CONTAINS(relationshipClasses, relationshipClass);
    }

TEST_F(ECDbAdapterTests, FindRelationshipClassWithSource_SchemaHasTwoMatchingRelationsipClasses_NullReturned)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="A" />
            <ECClass typeName="B" />
            <ECRelationshipClass typeName="AB1">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB2">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(*cache));

    auto sourceClass = db.Schemas().GetECClass("TestSchema", "A");
    auto targetClass = db.Schemas().GetECClass("TestSchema", "B");

    ECDbAdapter adapter(db);
    ECRelationshipClassCP relationshipClass = adapter.FindRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_EQ(nullptr, relationshipClass);
    }

TEST_F(ECDbAdapterTests, FindRelationshipClassWithSource_SchemaHasOneMatchingRelationsipClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Base" />
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AB1">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB2">
                <Source polymorphic="True"><Class class="Base"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(*cache));

    auto sourceClass = db.Schemas().GetECClass("TestSchema","A");
    auto targetClass = db.Schemas().GetECClass("TestSchema", "B");

    ECDbAdapter adapter(db);
    ECRelationshipClassCP relationshipClass = adapter.FindRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(db.Schemas().GetECClass("TestSchema", "AB1")->GetRelationshipClassCP(), relationshipClass);
    }

TEST_F(ECDbAdapterTests, FindRelationshipClassWithTarget_SchemaHasTwoMatchingRelationsipClasses_NullReturned)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="A" />
            <ECClass typeName="B" />
            <ECRelationshipClass typeName="AB1">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB2">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(*cache));

    auto sourceClass = db.GetClassLocater().LocateClass("TestSchema", "A");
    auto targetClass = db.GetClassLocater().LocateClass("TestSchema", "B");

    ECDbAdapter adapter(db);
    ECRelationshipClassCP relationshipClass = adapter.FindRelationshipClassWithTarget(sourceClass->GetId(), targetClass->GetId());

    EXPECT_EQ(nullptr, relationshipClass);
    }

TEST_F(ECDbAdapterTests, FindRelationshipClassWithTarget_SchemaHasOneMatchingRelationsipClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Base" />
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AB1">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB2">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="Base"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(*cache));

    auto sourceClass = db.GetClassLocater().LocateClass("TestSchema", "A");
    auto targetClass = db.GetClassLocater().LocateClass("TestSchema", "B");

    ECDbAdapter adapter(db);
    ECRelationshipClassCP relationshipClass = adapter.FindRelationshipClassWithTarget(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(db.GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP(), relationshipClass);
    }

TEST_F(ECDbAdapterTests, FindClosestRelationshipClassWithSource_SchemaHasTwoMatchingRelClassesSecondIsCloser_ReturnsSecondRelClass)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Base" />
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AB1">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="Base"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB2">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.GetEC().GetSchemaManager().ImportECSchemas(*cache));

    ECDbAdapter adapter(db);
    auto sourceClass = adapter.GetECClass("TestSchema.A");
    auto targetClass = adapter.GetECClass("TestSchema.B");

    ECRelationshipClassCP relationshipClass = adapter.FindClosestRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(adapter.GetECRelationshipClass("TestSchema.AB2"), relationshipClass);
    }

TEST_F(ECDbAdapterTests, FindClosestRelationshipClassWithSource_SchemaHasTwoMatchingRelClassesFirstIsCloser_ReturnsFirstRelClass)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Base" />
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AB1">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB2">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="Base"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.GetEC().GetSchemaManager().ImportECSchemas(*cache));
    
    ECDbAdapter adapter(db);
    auto sourceClass = adapter.GetECClass("TestSchema.A");
    auto targetClass = adapter.GetECClass("TestSchema.B");

    ECRelationshipClassCP relationshipClass = adapter.FindClosestRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(adapter.GetECRelationshipClass("TestSchema.AB1"), relationshipClass);
    }

TEST_F(ECDbAdapterTests, FindClosestRelationshipClassWithSource_SchemaHasNoMatchingRelationsipClasses_NullReturned)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Base" />
            <ECClass typeName="A"/>
            <ECClass typeName="B"/>
            <ECRelationshipClass typeName="AB1">
                <Source polymorphic="True"><Class class="Base"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB2">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="Base"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.GetEC().GetSchemaManager().ImportECSchemas(*cache));

    ECDbAdapter adapter(db);
    auto sourceClass = adapter.GetECClass("TestSchema.A");
    auto targetClass = adapter.GetECClass("TestSchema.B");

    ECRelationshipClassCP relationshipClass = adapter.FindClosestRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_EQ(nullptr, relationshipClass);
    }

TEST_F(ECDbAdapterTests, FindRelationshipClassesWithSource_SchemaHasTwoMatchingRelationsipClasses_BothReturned)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="A" />
            <ECClass typeName="B" />
            <ECRelationshipClass typeName="AB1">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB2">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(*cache));

    auto sourceClass = db.GetClassLocater().LocateClass("TestSchema", "A");

    ECDbAdapter adapter(db);
    bvector<ECRelationshipClassCP> relationshipClasses = adapter.FindRelationshipClassesWithSource(sourceClass->GetId(), "TestSchema");

    EXPECT_EQ(2, relationshipClasses.size());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB2")->GetRelationshipClassCP());
    }

TEST_F(ECDbAdapterTests, FindRelationshipClassesWithSource_SchemaHasOneMatchingRelationsipClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Base" />
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AB1">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB2">
                <Source polymorphic="True"><Class class="Base"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(*cache));

    auto sourceClass = db.GetClassLocater().LocateClass("TestSchema", "A");

    ECDbAdapter adapter(db);
    bvector<ECRelationshipClassCP> relationshipClasses = adapter.FindRelationshipClassesWithSource(sourceClass->GetId(), "TestSchema");

    EXPECT_EQ(2, relationshipClasses.size());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB2")->GetRelationshipClassCP());
    }

TEST_F(ECDbAdapterTests, FindRelationshipClassesInSchema_SchemaHasOneMatchingRelationsipClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Base" />
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="C">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AC1">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="C"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB1">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB2">
                <Source polymorphic="True"><Class class="Base"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(*cache));

    auto sourceClass = db.GetClassLocater().LocateClass("TestSchema", "A");
    auto targetClass = db.GetClassLocater().LocateClass("TestSchema", "B");

    ECDbAdapter adapter(db);
    bvector<ECRelationshipClassCP> relationshipClasses = adapter.FindRelationshipClassesInSchema(sourceClass->GetId(), targetClass->GetId(), "TestSchema");

    EXPECT_EQ(2, relationshipClasses.size());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB2")->GetRelationshipClassCP());
    }

TEST_F(ECDbAdapterTests, FindRelationshipClasses_SchemaHasOneMatchingRelationsipClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Base" />
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="C">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AC1">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="C"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB1">
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB2">
                <Source polymorphic="True"><Class class="Base"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(*cache));

    auto sourceClass = db.GetClassLocater().LocateClass("TestSchema", "A");
    auto targetClass = db.GetClassLocater().LocateClass("TestSchema", "B");

    ECDbAdapter adapter(db);
    bvector<ECRelationshipClassCP> relationshipClasses = adapter.FindRelationshipClasses(sourceClass->GetId(), targetClass->GetId());

    EXPECT_EQ(2, relationshipClasses.size());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB2")->GetRelationshipClassCP());
    }

TEST_F(ECDbAdapterTests, RelateInstances_InstancesExist_RelationshipIsCreated)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="A" />
            <ECClass typeName="B" />
            <ECRelationshipClass typeName="AB">
                <Source><Class class="A"/></Source>
                <Target><Class class="B"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(*cache));

    ECDbAdapter adapter(db);

    auto relClass = adapter.GetECRelationshipClass("TestSchema.AB");
    ECInstanceKey source, target;

    ASSERT_EQ(SUCCESS, JsonInserter(db, *adapter.GetECClass("TestSchema.A")).Insert(source, Json::Value()));
    ASSERT_EQ(SUCCESS, JsonInserter(db, *adapter.GetECClass("TestSchema.B")).Insert(target, Json::Value()));

    auto relationshipKey = adapter.RelateInstances(relClass, source, target);
    EXPECT_TRUE(relationshipKey.IsValid());
    EXPECT_EQ(relationshipKey, adapter.FindRelationship(relClass, source, target));
    }

TEST_F(ECDbAdapterTests, RelateInstances_RelationshipAlreadyExists_ReturnsSameRelationshipKey)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="A" />
            <ECClass typeName="B" />
            <ECRelationshipClass typeName="AB">
                <Source><Class class="A"/></Source>
                <Target><Class class="B"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(*cache));

    ECDbAdapter adapter(db);

    auto relClass = adapter.GetECRelationshipClass("TestSchema.AB");
    ECInstanceKey source, target;

    ASSERT_EQ(SUCCESS, JsonInserter(db, *adapter.GetECClass("TestSchema.A")).Insert(source, Json::Value()));
    ASSERT_EQ(SUCCESS, JsonInserter(db, *adapter.GetECClass("TestSchema.B")).Insert(target, Json::Value()));

    auto relationshipKey1 = adapter.RelateInstances(relClass, source, target);
    auto relationshipKey2 = adapter.RelateInstances(relClass, source, target);

    EXPECT_TRUE(relationshipKey1.IsValid());
    EXPECT_TRUE(relationshipKey2.IsValid());
    EXPECT_EQ(relationshipKey2, relationshipKey1);
    }