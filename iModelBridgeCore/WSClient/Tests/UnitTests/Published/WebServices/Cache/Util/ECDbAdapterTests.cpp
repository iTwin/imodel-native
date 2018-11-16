/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Util/ECDbAdapterTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECDbAdapterTests.h"

#include <WebServices/Cache/Util/ECDbAdapter.h>
#include "MockECDbAdapterDeleteListener.h"

// StrictMock is not portable
#if defined (BENTLEY_WIN32)

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

SeedFile ECDbAdapterTests::s_seedECDb("ecdbAdapterTest.ecdb",
                                      [] (BeFileNameCR filePath)
    {
    ECDb db;
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(filePath));

    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" alias="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="A" />
            <ECEntityClass typeName="B">
                <BaseClass>A</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="TestClass" />
            <ECEntityClass typeName="TestClass2" />
            <ECEntityClass typeName="TestClass3" />
            <ECRelationshipClass typeName="ReferencingRel" strength="referencing" modifier="Sealed">
                <Source multiplicity="(0..*)" polymorphic="false" roleLabel="CachedFileInfo"><Class class="TestClass" /></Source>
                <Target multiplicity="(0..*)" polymorphic="false" roleLabel="CachedFileInfo"><Class class="TestClass" /></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="HoldingRel" strength="holding" modifier="Sealed">
                <Source multiplicity="(0..*)" polymorphic="false" roleLabel="CachedFileInfo"><Class class="TestClass" /></Source>
                <Target multiplicity="(0..*)" polymorphic="false" roleLabel="CachedFileInfo"><Class class="TestClass" /></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="EmbeddingRel" strength="embedding" modifier="Sealed">
                <Source multiplicity="(0..1)" polymorphic="false" roleLabel="CachedFileInfo"><Class class="TestClass" /></Source>
                <Target multiplicity="(0..*)" polymorphic="false" roleLabel="CachedFileInfo"><Class class="TestClass" /></Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="TestChildClass">
                <ECNavigationProperty propertyName="ParentId" relationshipName="EmbeddingRequiredRel" direction="backward" />
            </ECEntityClass>
            <ECRelationshipClass typeName="EmbeddingRequiredRel" strength="embedding" modifier="Sealed">
                <Source multiplicity="(1..1)" polymorphic="false" roleLabel="TestClass"><Class class="TestClass" /></Source>
                <Target multiplicity="(0..1)" polymorphic="false" roleLabel="TestChildClass"><Class class="TestChildClass" /></Target>
            </ECRelationshipClass>
        </ECSchema>)xml");

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    EXPECT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db.SaveChanges());
    });

SeedFile ECDbAdapterTests::s_seedEmptyECDb("ecdbAdapterTest-empty.ecdb",
                                           [] (BeFileNameCR filePath)
    {
    ECDb db;
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(filePath));
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db.SaveChanges());
    });

std::shared_ptr<ObservableECDb> ECDbAdapterTests::GetTestDb()
    {
    auto db = std::make_shared<ObservableECDb>();
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db->OpenBeSQLiteDb(s_seedECDb.GetTestFile(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    return db;
    }

std::shared_ptr<ObservableECDb> ECDbAdapterTests::GetEmptyTestDb()
    {
    auto db = std::make_shared<ObservableECDb>();
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db->OpenBeSQLiteDb(s_seedEmptyECDb.GetTestFile(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    return db;
    }

std::shared_ptr<ObservableECDb> ECDbAdapterTests::CreateTestDb(ECSchemaPtr schema)
    {
    auto db = GetEmptyTestDb();
    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    EXPECT_EQ(SUCCESS, db->Schemas().ImportSchemas(cache->GetSchemas()));
    return db;
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, GetECClass_ValidClassKey_ReturnsClass)
    {
    auto db = GetTestDb();
    ECClassCP ecClass = ECDbAdapter(*db).GetECClass("TestSchema.TestClass");
    EXPECT_EQ("TestClass", ecClass->GetName());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, GetECClass_InValidClassKey_ReturnsNullptr)
    {
    auto db = GetTestDb();
    ECClassCP ecClass = ECDbAdapter(*db).GetECClass("NotClassKey");
    EXPECT_EQ(nullptr, ecClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, GetECClass_ValidClassKeyWithNotExistingClass_ReturnsNullptr)
    {
    auto db = GetTestDb();
    ECClassCP ecClass = ECDbAdapter(*db).GetECClass("TestSchema.NotExistingClass");
    EXPECT_EQ(nullptr, ecClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, GetECClass_ValidClassId_ReturnsClass)
    {
    auto db = GetTestDb();
    ECClassId ecClassId = ECDbAdapter(*db).GetECClass("TestSchema", "TestClass")->GetId();

    ECClassCP ecClass = ECDbAdapter(*db).GetECClass(ecClassId);

    EXPECT_EQ("TestClass", ecClass->GetName());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, GetECClass_InValidClassId_ReturnsNull)
    {
    auto db = GetTestDb();
    ECClassCP ecClass = ECDbAdapter(*db).GetECClass(ECClassId(UINT64_C(9999)));
    EXPECT_EQ(nullptr, ecClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, GetECClasses_EmptyMap_EmptyResult)
    {
    auto db = GetTestDb();
    EXPECT_TRUE(ECDbAdapter(*db).GetECClasses(ECInstanceKeyMultiMap()).empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, GetECClasses_MapWithTwoSameClassInstances_ReturnsOneClass)
    {
    auto db = GetTestDb();

    ECClassCP ecClass = ECDbAdapter(*db).GetECClass("TestSchema.TestClass");

    ECInstanceKeyMultiMap map;
    map.insert({ecClass->GetId(), ECInstanceId(UINT64_C(1))});
    map.insert({ecClass->GetId(), ECInstanceId(UINT64_C(2))});

    auto classes = ECDbAdapter(*db).GetECClasses(map);

    ASSERT_EQ(1, classes.size());
    EXPECT_EQ(ecClass, classes[0]);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, CountClassInstances_NullClass_Zero)
    {
    auto db = GetTestDb();
    EXPECT_EQ(0, ECDbAdapter(*db).CountClassInstances(nullptr));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindInstance_NullClass_Invalid)
    {
    auto db = GetTestDb();
    Json::Value json;
    EXPECT_EQ(false, ECDbAdapter(*db).FindInstance(nullptr).IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindInstances_NullClass_Empty)
    {
    auto db = GetTestDb();
    Json::Value json;
    EXPECT_EQ(0, ECDbAdapter(*db).FindInstances(nullptr).size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, GetJsonInstance_NullClassAndValidWhere_Error)
    {
    auto db = GetTestDb();
    Json::Value json;
    EXPECT_EQ(ERROR, ECDbAdapter(*db).GetJsonInstance(json, nullptr, "true"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, GetJsonInstances_NullClassAndValidWhere_Error)
    {
    auto db = GetTestDb();
    Json::Value json;
    EXPECT_EQ(ERROR, ECDbAdapter(*db).GetJsonInstances(json, nullptr, "true"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, GetJsonInstances_NullClassAndStatement_Error)
    {
    auto db = GetTestDb();
    Json::Value json;
    ECSqlStatement statement;
    EXPECT_EQ(ERROR, ECDbAdapter(*db).GetJsonInstances(json, nullptr, statement));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindRelationshipClasses_ShcemaHasSuchRelationshipClass_RelationshipClassReturned)
    {
    auto schema = StubRelationshipSchema("TestSchema", "A", "B", "AB");
    auto db = CreateTestDb(schema);

    auto sourceClass = db->Schemas().GetClass("TestSchema", "A");
    auto targetClass = db->Schemas().GetClass("TestSchema", "B");
    auto relationshipClass = db->Schemas().GetClass("TestSchema", "AB")->GetRelationshipClassCP();

    ECDbAdapter adapter(*db);
    bvector<ECRelationshipClassCP> relationshipClasses = adapter.FindRelationshipClasses(sourceClass->GetId(), targetClass->GetId());

    EXPECT_EQ(1, relationshipClasses.size());
    EXPECT_CONTAINS(relationshipClasses, relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
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

    auto db = CreateTestDb(schema);

    auto sourceClass = db->Schemas().GetClass("TestSchema", "A");
    auto targetClass = db->Schemas().GetClass("TestSchema", "B");

    ECDbAdapter adapter(*db);
    ECRelationshipClassCP relationshipClass = adapter.FindRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_EQ(nullptr, relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindRelationshipClassWithSource_SchemaHasOneMatchingRelationsipClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                           <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
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

    auto db = CreateTestDb(schema);

    auto sourceClass = db->Schemas().GetClass("TestSchema", "A");
    auto targetClass = db->Schemas().GetClass("TestSchema", "B");

    ECDbAdapter adapter(*db);
    ECRelationshipClassCP relationshipClass = adapter.FindRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(db->Schemas().GetClass("TestSchema", "AB1")->GetRelationshipClassCP(), relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Yashwanth.Chintana                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindRelationshipClassWithSource_SchemaHasOneMatchingBackwardRelationsipClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>
                            <Strategy>TablePerHierarchy</Strategy>
                        </MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AB1" strength="referencing" strengthDirection="backward">
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
    ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));

    auto sourceClass = db.GetClassLocater().LocateClass("TestSchema", "B");
    auto targetClass = db.GetClassLocater().LocateClass("TestSchema", "A");

    ECDbAdapter adapter(db);
    ECRelationshipClassCP relationshipClass = adapter.FindRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(db.GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP(), relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
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

    auto db = CreateTestDb(schema);

    auto sourceClass = db->GetClassLocater().LocateClass("TestSchema", "A");
    auto targetClass = db->GetClassLocater().LocateClass("TestSchema", "B");

    ECDbAdapter adapter(*db);
    ECRelationshipClassCP relationshipClass = adapter.FindRelationshipClassWithTarget(sourceClass->GetId(), targetClass->GetId());

    EXPECT_EQ(nullptr, relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Yashwanth.Chintana                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindRelationshipClassWithTarget_SchemaHasOneMatchingRelationsipClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                           <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
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

    auto db = CreateTestDb(schema);

    auto sourceClass = db->GetClassLocater().LocateClass("TestSchema", "A");
    auto targetClass = db->GetClassLocater().LocateClass("TestSchema", "B");

    ECDbAdapter adapter(*db);
    ECRelationshipClassCP relationshipClass = adapter.FindRelationshipClassWithTarget(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(db->GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP(), relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Yashwanth.Chintana                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindRelationshipClassWithTarget_SchemaHasOneMatchingBackwardRelationsipClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap"/>
            <ECClass typeName="Base" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AB1" strength="referencing" strengthDirection="backward">
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
    ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));

    auto sourceClass = db.GetClassLocater().LocateClass("TestSchema", "B");
    auto targetClass = db.GetClassLocater().LocateClass("TestSchema", "A");

    ECDbAdapter adapter(db);
    ECRelationshipClassCP relationshipClass = adapter.FindRelationshipClassWithTarget(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(db.GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP(), relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                   Mark.Uvari                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindClosestRelationshipClassWithSource_SchemaHasTwoMatchingRelClassesSecondIsCloser_ReturnsSecondRelClass)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                       <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
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

    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);

    auto sourceClass = adapter.GetECClass("TestSchema.A");
    auto targetClass = adapter.GetECClass("TestSchema.B");

    ECRelationshipClassCP relationshipClass = adapter.FindClosestRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(adapter.GetECRelationshipClass("TestSchema.AB2"), relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                   Mark.Uvari                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindClosestRelationshipClassWithSource_SchemaHasTwoMatchingRelClassesFirstIsCloser_ReturnsFirstRelClass)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                     <ClassMap xmlns="ECDbMap.02.00">
                       <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
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

    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);

    auto sourceClass = adapter.GetECClass("TestSchema.A");
    auto targetClass = adapter.GetECClass("TestSchema.B");

    ECRelationshipClassCP relationshipClass = adapter.FindClosestRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(adapter.GetECRelationshipClass("TestSchema.AB1"), relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                   Mark.Uvari                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindClosestRelationshipClassWithSource_SchemaHasNoMatchingRelationsipClasses_NullReturned)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
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

    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);

    auto sourceClass = adapter.GetECClass("TestSchema.A");
    auto targetClass = adapter.GetECClass("TestSchema.B");

    ECRelationshipClassCP relationshipClass = adapter.FindClosestRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_EQ(nullptr, relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Yashwanth.Chintana                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindClosestRelationshipClassWithSource_SchemaHasOneMatchingBackwardRelClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>
                            <Strategy>TablePerHierarchy</Strategy>
                        </MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="C">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AC1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="C"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="BA1" strength="referencing" strengthDirection="backward">
                <Source polymorphic="True"><Class class="B"/></Source>
                <Target polymorphic="True"><Class class="A"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="BC1" >
                <Source polymorphic="True"><Class class="B"/></Source>
                <Target polymorphic="True"><Class class="C"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));

    ECDbAdapter adapter(db);
    auto sourceClass = adapter.GetECClass("TestSchema.A");
    auto targetClass = adapter.GetECClass("TestSchema.B");

    ECRelationshipClassCP relationshipClass = adapter.FindClosestRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(adapter.GetECRelationshipClass("TestSchema.BA1"), relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Yashwanth.Chintana                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindClosestRelationshipClassWithSource_SchemaHasParentChildRelationshipsWithNoDirectMatching_ReturnsParent)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>
                            <Strategy>TablePerHierarchy</Strategy>
                        </MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="C">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="AA">
                <BaseClass>A</BaseClass>
            </ECClass>
            <ECClass typeName="BB">
                <BaseClass>B</BaseClass>
            </ECClass>
            <ECClass typeName="CC">
                <BaseClass>C</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AB1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            
            <ECRelationshipClass typeName="AACC1">
                <Source polymorphic="True"><Class class="AA"/></Source>
                <Target polymorphic="True"><Class class="CC"/></Target>
            </ECRelationshipClass>           
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));

    ECDbAdapter adapter(db);
    auto sourceClass = adapter.GetECClass("TestSchema.AA");
    auto targetClass = adapter.GetECClass("TestSchema.BB");

    ECRelationshipClassCP relationshipClass = adapter.FindClosestRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(adapter.GetECRelationshipClass("TestSchema.AB1"), relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Yashwanth.Chintana                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindClosestRelationshipClassWithSource_SchemaHasParentChildRelationshipsWithMatchingRelationship_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>
                            <Strategy>TablePerHierarchy</Strategy>
                        </MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="AA">
                <BaseClass>A</BaseClass>
            </ECClass>
            <ECClass typeName="BB">
                <BaseClass>B</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AB1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            
            <ECRelationshipClass typeName="AABB1" strength="referencing" strengthDirection="backward" >
                <Source polymorphic="True"><Class class="AA"/></Source>
                <Target polymorphic="True"><Class class="BB"/></Target>
            </ECRelationshipClass>           
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));

    ECDbAdapter adapter(db);
    auto sourceClass = adapter.GetECClass("TestSchema.BB");
    auto targetClass = adapter.GetECClass("TestSchema.AA");

    ECRelationshipClassCP relationshipClass = adapter.FindClosestRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(adapter.GetECRelationshipClass("TestSchema.AABB1"), relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Yashwanth.Chintana                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindClosestRelationshipClassWithSource_SchemaHasTwoMatchingRelClassesOneBackwardRelationshipClosest_ReturnsClosestMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>
                            <Strategy>TablePerHierarchy</Strategy>
                        </MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="C">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AC1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="C"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="BA1" strength="referencing" strengthDirection="backward">
                <Source polymorphic="True"><Class class="B"/></Source>
                <Target polymorphic="True"><Class class="A"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="Base"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));

    ECDbAdapter adapter(db);
    auto sourceClass = adapter.GetECClass("TestSchema.A");
    auto targetClass = adapter.GetECClass("TestSchema.B");

    ECRelationshipClassCP relationshipClass = adapter.FindClosestRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(adapter.GetECRelationshipClass("TestSchema.BA1"), relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Yashwanth.Chintana                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindClosestRelationshipClassWithSource_SchemaHasTwoMatchingRelClassesOneOppositeRelationshipClosest_ReturnsClosestMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>
                            <Strategy>TablePerHierarchy</Strategy>
                        </MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="C">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AC1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="C"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="BA1">
                <Source polymorphic="True"><Class class="B"/></Source>
                <Target polymorphic="True"><Class class="A"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="Base"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));

    ECDbAdapter adapter(db);
    auto sourceClass = adapter.GetECClass("TestSchema.A");
    auto targetClass = adapter.GetECClass("TestSchema.B");

    ECRelationshipClassCP relationshipClass = adapter.FindClosestRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(adapter.GetECRelationshipClass("TestSchema.BA1"), relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Yashwanth.Chintana                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindClosestRelationshipClassWithSource_SchemaHasTwoMatchingRelClassesOneBackwardOppositeRelationshipClosest_ReturnsMatchingFirstClass)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>
                            <Strategy>TablePerHierarchy</Strategy>
                        </MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="C">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AC1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="C"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="BA1">
                <Source polymorphic="True"><Class class="B"/></Source>
                <Target polymorphic="True"><Class class="A"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="Base"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));

    ECDbAdapter adapter(db);
    auto sourceClass = adapter.GetECClass("TestSchema.A");
    auto targetClass = adapter.GetECClass("TestSchema.B");

    ECRelationshipClassCP relationshipClass = adapter.FindClosestRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(adapter.GetECRelationshipClass("TestSchema.BA1"), relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Yashwanth.Chintana                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindClosestRelationshipClassWithSource_SchemaHasTwoMatchingRelClassesOneBackwardOppositeRelationshipClosest_ReturnsMatchingSecondClass)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>
                            <Strategy>TablePerHierarchy</Strategy>
                        </MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="C">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AC1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="C"/></Target>
            </ECRelationshipClass>         
            <ECRelationshipClass typeName="AB1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="Base"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="BA1">
                <Source polymorphic="True"><Class class="B"/></Source>
                <Target polymorphic="True"><Class class="A"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));

    ECDbAdapter adapter(db);
    auto sourceClass = adapter.GetECClass("TestSchema.A");
    auto targetClass = adapter.GetECClass("TestSchema.B");

    ECRelationshipClassCP relationshipClass = adapter.FindClosestRelationshipClassWithSource(sourceClass->GetId(), targetClass->GetId());

    EXPECT_NE(nullptr, relationshipClass);
    EXPECT_EQ(adapter.GetECRelationshipClass("TestSchema.BA1"), relationshipClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
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

    auto db = CreateTestDb(schema);
    auto sourceClass = db->GetClassLocater().LocateClass("TestSchema", "A");

    ECDbAdapter adapter(*db);
    bvector<ECRelationshipClassCP> relationshipClasses = adapter.FindRelationshipClassesWithSource(sourceClass->GetId(), "TestSchema");

    EXPECT_EQ(2, relationshipClasses.size());
    EXPECT_CONTAINS(relationshipClasses, db->GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db->GetClassLocater().LocateClass("TestSchema", "AB2")->GetRelationshipClassCP());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Yashwanth.Chintana                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindRelationshipClassesWithSource_SchemaHasThreeMatchingRelationsipClassesIncludingBackwardStrengthDirection_AllThreeReturned)
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
            <ECRelationshipClass typeName="BA1" strength="referencing" strengthDirection="backward">
                <Source polymorphic="True"><Class class="B"/></Source>
                <Target polymorphic="True"><Class class="A"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));

    auto sourceClass = db.GetClassLocater().LocateClass("TestSchema", "A");

    ECDbAdapter adapter(db);
    bvector<ECRelationshipClassCP> relationshipClasses = adapter.FindRelationshipClassesWithSource(sourceClass->GetId(), "TestSchema");

    EXPECT_EQ(3, relationshipClasses.size());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB2")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "BA1")->GetRelationshipClassCP());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindRelationshipClassesWithSource_SchemaHasOneMatchingRelationsipClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />

            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>

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

    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);
    auto sourceClass = db->GetClassLocater().LocateClass("TestSchema", "A");

    bvector<ECRelationshipClassCP> relationshipClasses = adapter.FindRelationshipClassesWithSource(sourceClass->GetId(), "TestSchema");

    EXPECT_EQ(2, relationshipClasses.size());
    EXPECT_CONTAINS(relationshipClasses, db->GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db->GetClassLocater().LocateClass("TestSchema", "AB2")->GetRelationshipClassCP());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindRelationshipClassesInSchema_SchemaHasOneMatchingRelationsipClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />

            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>

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

    auto db = CreateTestDb(schema);

    auto sourceClass = db->GetClassLocater().LocateClass("TestSchema", "A");
    auto targetClass = db->GetClassLocater().LocateClass("TestSchema", "B");

    ECDbAdapter adapter(*db);
    bvector<ECRelationshipClassCP> relationshipClasses =
        adapter.FindRelationshipClassesInSchema(sourceClass->GetId(), targetClass->GetId(), "TestSchema");

    EXPECT_EQ(2, relationshipClasses.size());
    EXPECT_CONTAINS(relationshipClasses, db->GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db->GetClassLocater().LocateClass("TestSchema", "AB2")->GetRelationshipClassCP());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindRelationshipClassesInSchema_SchemaHasTwoMatchingRelClassIncludingBackwardRelClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>
                            <Strategy>TablePerHierarchy</Strategy>
                        </MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="C">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AC1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="C"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="BA1" strength="referencing" strengthDirection="backward">
                <Source polymorphic="True"><Class class="B"/></Source>
                <Target polymorphic="True"><Class class="A"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB2" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="Base"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));

    auto sourceClass = db.GetClassLocater().LocateClass("TestSchema", "A");
    auto targetClass = db.GetClassLocater().LocateClass("TestSchema", "B");

    ECDbAdapter adapter(db);
    bvector<ECRelationshipClassCP> relationshipClasses = adapter.FindRelationshipClassesInSchema(sourceClass->GetId(), targetClass->GetId(), "TestSchema");

    EXPECT_EQ(3, relationshipClasses.size());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "BA1")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB2")->GetRelationshipClassCP());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindRelationshipClasses_SchemaHasOneMatchingRelationsipClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />

            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>

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

    auto db = CreateTestDb(schema);

    auto sourceClass = db->GetClassLocater().LocateClass("TestSchema", "A");
    auto targetClass = db->GetClassLocater().LocateClass("TestSchema", "B");

    ECDbAdapter adapter(*db);
    bvector<ECRelationshipClassCP> relationshipClasses =
        adapter.FindRelationshipClasses(sourceClass->GetId(), targetClass->GetId());

    EXPECT_EQ(2, relationshipClasses.size());
    EXPECT_CONTAINS(relationshipClasses, db->GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db->GetClassLocater().LocateClass("TestSchema", "AB2")->GetRelationshipClassCP());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Yashwanth.Chintana                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, FindRelationshipClasses_SchemaHasOneMatchingRelClassOneBackwardRelClassAndOneWithBaseClasses_ReturnsMatching)
    {
    auto schema = ParseSchema(R"(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
            <ECClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>
                            <Strategy>TablePerHierarchy</Strategy>
                        </MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="A">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="B">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECClass typeName="C">
                <BaseClass>Base</BaseClass>
            </ECClass>
            <ECRelationshipClass typeName="AC1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="C"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="BA1" strength="referencing" strengthDirection="backward">
                <Source polymorphic="True"><Class class="B"/></Source>
                <Target polymorphic="True"><Class class="A"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB1" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="B"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="AB2" >
                <Source polymorphic="True"><Class class="A"/></Source>
                <Target polymorphic="True"><Class class="Base"/></Target>
            </ECRelationshipClass>
        </ECSchema>)");

    ObservableECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);
    ASSERT_EQ(SUCCESS, db.Schemas().ImportSchemas(cache->GetSchemas()));

    auto sourceClass = db.GetClassLocater().LocateClass("TestSchema", "A");
    auto targetClass = db.GetClassLocater().LocateClass("TestSchema", "B");

    ECDbAdapter adapter(db);
    bvector<ECRelationshipClassCP> relationshipClasses = adapter.FindRelationshipClasses(sourceClass->GetId(), targetClass->GetId());

    EXPECT_EQ(3, relationshipClasses.size());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "BA1")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB1")->GetRelationshipClassCP());
    EXPECT_CONTAINS(relationshipClasses, db.GetClassLocater().LocateClass("TestSchema", "AB2")->GetRelationshipClassCP());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
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

    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);

    auto relClass = adapter.GetECRelationshipClass("TestSchema.AB");
    ECInstanceKey source, target;

    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *adapter.GetECClass("TestSchema.A"), nullptr).Insert(source, Json::Value()));
    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *adapter.GetECClass("TestSchema.B"), nullptr).Insert(target, Json::Value()));

    auto relationshipKey = adapter.RelateInstances(relClass, source, target);
    EXPECT_TRUE(relationshipKey.IsValid());
    EXPECT_EQ(relationshipKey, adapter.FindRelationship(relClass, source, target));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
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

    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);

    auto relClass = adapter.GetECRelationshipClass("TestSchema.AB");
    ECInstanceKey source, target;

    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *adapter.GetECClass("TestSchema.A"), nullptr).Insert(source, Json::Value()));
    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *adapter.GetECClass("TestSchema.B"), nullptr).Insert(target, Json::Value()));

    auto relationshipKey1 = adapter.RelateInstances(relClass, source, target);
    auto relationshipKey2 = adapter.RelateInstances(relClass, source, target);

    EXPECT_TRUE(relationshipKey1.IsValid());
    EXPECT_TRUE(relationshipKey2.IsValid());
    EXPECT_EQ(relationshipKey2, relationshipKey1);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, CountClassInstances_ValidAndInvalidECClasses_Counts)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto relClass = adapter.GetECRelationshipClass("TestSchema.ReferencingRel");

    ECInstanceKey a, b, c;
    INSERT_INSTANCE(*db, ecClass, a);
    INSERT_INSTANCE(*db, ecClass, b);
    INSERT_RELATIONSHIP(*db, relClass, a, b, c);

    EXPECT_EQ(0, adapter.CountClassInstances(nullptr));
    EXPECT_EQ(2, adapter.CountClassInstances(ecClass));
    EXPECT_EQ(1, adapter.CountClassInstances(relClass));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_InvalidKey_Error)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    CREATE_MockECDbAdapterDeleteListener(listener);
    adapter.RegisterDeleteListener(&listener);

    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(ERROR, adapter.DeleteInstances(StubECInstanceKeyMultiMap({ECInstanceKey()})));
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_NotExistingInstance_NotifiesBeforeDeletionAndDoesNothing)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);
    auto ecClass = adapter.GetECClass("TestSchema.TestClass");

    ECInstanceKey instance = StubECInstanceKey(ecClass->GetId().GetValue(), 123456);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL(listener, OnBeforeDelete(Ref(*ecClass), instance.GetInstanceId(), _)).WillOnce(Return(SUCCESS));
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({instance})));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_ExistingInstances_NotifiesBeforeDeletionAndDeletes)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);
    auto ecClass = adapter.GetECClass("TestSchema.TestClass");

    ECInstanceKey a, b;
    INSERT_INSTANCE(*db, ecClass, a);
    INSERT_INSTANCE(*db, ecClass, b);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, a);
    EXPECT_CALL_OnBeforeDelete(listener, db, b);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({a, b})));
    EXPECT_EQ(0, adapter.CountClassInstances(adapter.GetECClass("TestSchema.TestClass")));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_ExistingInstanceAndRemovedListener_ListenerNotNotifiedAndDeletes)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    ECInstanceKey instance;
    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *adapter.GetECClass("TestSchema.TestClass"), nullptr).Insert(instance, Json::Value()));

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL(listener, OnBeforeDelete(_, _, _)).Times(0);
    adapter.RegisterDeleteListener(&listener);
    adapter.UnRegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({instance})));
    EXPECT_EQ(0, adapter.CountClassInstances(adapter.GetECClass("TestSchema.TestClass")));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_ExistingInstanceAndListenerReturnsError_ReturnsErrorWithoutDeletion)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    ECInstanceKey instance;
    ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *adapter.GetECClass("TestSchema.TestClass"), nullptr).Insert(instance, Json::Value()));

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL(listener, OnBeforeDelete(_, _, _)).WillOnce(Return(ERROR));
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(ERROR, adapter.DeleteInstances(StubECInstanceKeyMultiMap({instance})));
    EXPECT_EQ(1, adapter.CountClassInstances(adapter.GetECClass("TestSchema.TestClass")));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingParentThatHoldsChild_DeletesParentAndChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child, rel;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, child, rel);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, parent);
    EXPECT_CALL_OnBeforeDelete(listener, db, child);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({parent})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(0, notDeletedInstances.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstance_DeletingParentThatHoldsChild_DeletesParentAndChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child, rel;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, child, rel);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, parent);
    EXPECT_CALL_OnBeforeDelete(listener, db, child);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstance(parent));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(0, notDeletedInstances.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstnace_DeletingNotExistingHoldingRelationship_NotifiesBeforeDeleteAndAndSuccess)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child, rel;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, child, rel);
    ASSERT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({child})));

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL(listener, OnBeforeDelete(Ref(*holdingRelClass), rel.GetInstanceId(), _)).WillOnce(Return(SUCCESS));
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({rel})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, parent.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingHoldingRelationship_DeletesRelationshipAndChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child, rel;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, child, rel);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, child);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({rel})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, parent.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingParentThatEmbedsChild_DeletesParentAndChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto embeddingRelClass = adapter.GetECRelationshipClass("TestSchema.EmbeddingRel");

    ECInstanceKey parent, child, rel;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_RELATIONSHIP(*db, embeddingRelClass, parent, child, rel);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, parent);
    EXPECT_CALL_OnBeforeDelete(listener, db, child);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({parent})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(0, notDeletedInstances.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingEmbeddingRelationship_DeletesRelationshipAndChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto embeddingRelClass = adapter.GetECRelationshipClass("TestSchema.EmbeddingRel");

    ECInstanceKey parent, child, rel;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_RELATIONSHIP(*db, embeddingRelClass, parent, child, rel);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, child);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({rel})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, parent.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingParentThatHoldsChildThatHasOtherHoldingParent_DoesNotDeleteChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child, otherParent, rel1, rel2;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_INSTANCE(*db, ecClass, otherParent);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, child, rel1);
    INSERT_RELATIONSHIP(*db, holdingRelClass, otherParent, child, rel2);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, parent);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({parent})));

    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(2, notDeletedInstances.size());
    EXPECT_NCONTAIN(notDeletedInstances, parent.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, otherParent.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, child.GetInstanceId());

    notDeletedInstances = adapter.FindInstances(holdingRelClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, rel2.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingHoldingRelationshipWhenChildHasOtherHoldingParent_DeletesRelationshipWithoutChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child, otherParent, rel1, rel2;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_INSTANCE(*db, ecClass, otherParent);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, child, rel1);
    INSERT_RELATIONSHIP(*db, holdingRelClass, otherParent, child, rel2);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({rel1})));

    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(3, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, parent.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, otherParent.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, child.GetInstanceId());

    notDeletedInstances = adapter.FindInstances(holdingRelClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, rel2.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingParentThatEmbedsChildThatHasOtherHoldingParent_DeletesParentAndChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto embeddingRelClass = adapter.GetECRelationshipClass("TestSchema.EmbeddingRel");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child, otherParent, rel1, rel2;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_INSTANCE(*db, ecClass, otherParent);
    INSERT_RELATIONSHIP(*db, embeddingRelClass, parent, child, rel1);
    INSERT_RELATIONSHIP(*db, holdingRelClass, otherParent, child, rel2);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, parent);
    EXPECT_CALL_OnBeforeDelete(listener, db, child);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel2);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({parent})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, otherParent.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingEmbeddingRelationshipWhenChilHasOtherHoldingParent_DeletesRelationshipAndChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto embeddingRelClass = adapter.GetECRelationshipClass("TestSchema.EmbeddingRel");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child, otherParent, rel1, rel2;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_INSTANCE(*db, ecClass, otherParent);
    INSERT_RELATIONSHIP(*db, embeddingRelClass, parent, child, rel1);
    INSERT_RELATIONSHIP(*db, holdingRelClass, otherParent, child, rel2);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, child);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel2);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({rel1})));

    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(2, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, parent.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, otherParent.GetInstanceId());

    EXPECT_EQ(0, adapter.FindInstances(embeddingRelClass).size());
    EXPECT_EQ(0, adapter.FindInstances(holdingRelClass).size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingRelationshipThatHoldsHierarchy_HierarchyDeleted)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child, subChild, rel1, rel2;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_INSTANCE(*db, ecClass, subChild);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, child, rel1);
    INSERT_RELATIONSHIP(*db, holdingRelClass, child, subChild, rel2);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, child);
    EXPECT_CALL_OnBeforeDelete(listener, db, subChild);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel2);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({rel1})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, parent.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingParentThatHoldsChildThatHoldsSubChildThatHasOtherHoldingParent_DoesNotDeleteSubChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child, subChild, otherParent, rel1, rel2, rel3;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_INSTANCE(*db, ecClass, subChild);
    INSERT_INSTANCE(*db, ecClass, otherParent);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, child, rel1);
    INSERT_RELATIONSHIP(*db, holdingRelClass, child, subChild, rel2);
    INSERT_RELATIONSHIP(*db, holdingRelClass, otherParent, subChild, rel3);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, parent);
    EXPECT_CALL_OnBeforeDelete(listener, db, child);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel2);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({parent})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(2, notDeletedInstances.size());
    EXPECT_NCONTAIN(notDeletedInstances, parent.GetInstanceId());
    EXPECT_NCONTAIN(notDeletedInstances, child.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, subChild.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, otherParent.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingParentThatHoldsChildThatHasOtherEmbeddingParent_ParentAndChildDeleted)
    {
    // Embedding relationship is ignored from perspecfive of holding relationship in ECDb.

    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto embeddingRelClass = adapter.GetECRelationshipClass("TestSchema.EmbeddingRel");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child, otherParent, rel1, rel2;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_INSTANCE(*db, ecClass, otherParent);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, child, rel1);
    INSERT_RELATIONSHIP(*db, embeddingRelClass, otherParent, child, rel2);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, parent);
    EXPECT_CALL_OnBeforeDelete(listener, db, child);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel2);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({parent})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    EXPECT_NCONTAIN(notDeletedInstances, parent.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, otherParent.GetInstanceId());
    EXPECT_NCONTAIN(notDeletedInstances, child.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingParentThatEmbedsChildThatHasOtherHoldingParent_ParentAndChildDeleted)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto embeddingRelClass = adapter.GetECRelationshipClass("TestSchema.EmbeddingRel");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child, otherParent, rel1, rel2;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_INSTANCE(*db, ecClass, otherParent);
    INSERT_RELATIONSHIP(*db, embeddingRelClass, parent, child, rel1);
    INSERT_RELATIONSHIP(*db, holdingRelClass, otherParent, child, rel2);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, parent);
    EXPECT_CALL_OnBeforeDelete(listener, db, child);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel2);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({parent})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    EXPECT_NCONTAIN(notDeletedInstances, parent.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, otherParent.GetInstanceId());
    EXPECT_NCONTAIN(notDeletedInstances, child.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingParentThatHoldsChildThatHasReferencingRelationship_DeletesParentAndChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");
    auto referencingRelClass = adapter.GetECRelationshipClass("TestSchema.ReferencingRel");

    ECInstanceKey parent, child, other, rel1, rel2;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_INSTANCE(*db, ecClass, other);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, child, rel1);
    INSERT_RELATIONSHIP(*db, referencingRelClass, other, child, rel2);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, parent);
    EXPECT_CALL_OnBeforeDelete(listener, db, child);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel2);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({parent})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    EXPECT_NCONTAIN(notDeletedInstances, parent.GetInstanceId());
    EXPECT_NCONTAIN(notDeletedInstances, child.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, other.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingParentThatHoldsTwoChildrenThatHoldSubChild_DeletesParentAndChildrenAndSubChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child1, child2, subChild, rel1, rel2, rel3, rel4;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child1);
    INSERT_INSTANCE(*db, ecClass, child2);
    INSERT_INSTANCE(*db, ecClass, subChild);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, child1, rel1);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, child2, rel2);
    INSERT_RELATIONSHIP(*db, holdingRelClass, child1, subChild, rel3);
    INSERT_RELATIONSHIP(*db, holdingRelClass, child2, subChild, rel4);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, parent);
    EXPECT_CALL_OnBeforeDelete(listener, db, child1);
    EXPECT_CALL_OnBeforeDelete(listener, db, child2);
    EXPECT_CALL_OnBeforeDelete(listener, db, subChild);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel2);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel3);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel4);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({parent})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(0, notDeletedInstances.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletingParentWithChildWithHoldingCircularRelationships_DeletesParentOnlyAsCircularDeletionNotSupported)
    {
    // TODO: This is flaw in ECDb - it does not handle circular relationships.

    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, a, b, c, rel1, rel2, rel3, rel4;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, a);
    INSERT_INSTANCE(*db, ecClass, b);
    INSERT_INSTANCE(*db, ecClass, c);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, a, rel1);
    INSERT_RELATIONSHIP(*db, holdingRelClass, a, b, rel2);
    INSERT_RELATIONSHIP(*db, holdingRelClass, b, c, rel3);
    INSERT_RELATIONSHIP(*db, holdingRelClass, c, a, rel4);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, parent);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({parent})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    //EXPECT_EQ(0, notDeletedInstances.size());
    EXPECT_EQ(3, notDeletedInstances.size());
    EXPECT_NCONTAIN(notDeletedInstances, parent.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, a.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, b.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, c.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_DeletedInstanceIsInHoldingCircularRelationships_DeletesChildren)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey a, b, c, rel1, rel2, rel3;
    INSERT_INSTANCE(*db, ecClass, a);
    INSERT_INSTANCE(*db, ecClass, b);
    INSERT_INSTANCE(*db, ecClass, c);
    INSERT_RELATIONSHIP(*db, holdingRelClass, a, b, rel1);
    INSERT_RELATIONSHIP(*db, holdingRelClass, b, c, rel2);
    INSERT_RELATIONSHIP(*db, holdingRelClass, c, a, rel3);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, a);
    EXPECT_CALL_OnBeforeDelete(listener, db, b);
    EXPECT_CALL_OnBeforeDelete(listener, db, c);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel2);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel3);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({a})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(0, notDeletedInstances.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_RelatedInstances_NotifiesEachChildInstanceDeletionAndDeletesThem)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto referencingRelClass = adapter.GetECRelationshipClass("TestSchema.ReferencingRel");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");
    auto embeddingRelClass = adapter.GetECRelationshipClass("TestSchema.EmbeddingRel");

    ECInstanceKey parent, a, b, c, d, e, f, rel1, rel2, rel3, rel4, rel5, rel6;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, a);
    INSERT_INSTANCE(*db, ecClass, b);
    INSERT_INSTANCE(*db, ecClass, c);
    INSERT_INSTANCE(*db, ecClass, d);
    INSERT_INSTANCE(*db, ecClass, e);
    INSERT_INSTANCE(*db, ecClass, f);
    INSERT_RELATIONSHIP(*db, referencingRelClass, parent, a, rel1);
    INSERT_RELATIONSHIP(*db, referencingRelClass, d, parent, rel2);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, b, rel3);
    INSERT_RELATIONSHIP(*db, holdingRelClass, e, parent, rel4);
    INSERT_RELATIONSHIP(*db, embeddingRelClass, parent, c, rel5);
    INSERT_RELATIONSHIP(*db, embeddingRelClass, f, parent, rel6);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, parent);
    EXPECT_CALL_OnBeforeDelete(listener, db, b);
    EXPECT_CALL_OnBeforeDelete(listener, db, c);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel2);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel3);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel4);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel5);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel6);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({parent})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(4, notDeletedInstances.size());
    EXPECT_NCONTAIN(notDeletedInstances, parent.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, a.GetInstanceId());
    EXPECT_NCONTAIN(notDeletedInstances, b.GetInstanceId());
    EXPECT_NCONTAIN(notDeletedInstances, c.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, d.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, e.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, f.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_RelatedChildChildrenInstances_NotifiesEachChildInstanceDeletionAndDeletesThem)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto referencingRelClass = adapter.GetECRelationshipClass("TestSchema.ReferencingRel");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");
    auto embeddingRelClass = adapter.GetECRelationshipClass("TestSchema.EmbeddingRel");

    ECInstanceKey parent, a, aa, b, bb, c, cc, rel1, rel2, rel3, rel4, rel5, rel6;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, a);
    INSERT_INSTANCE(*db, ecClass, b);
    INSERT_INSTANCE(*db, ecClass, c);
    INSERT_INSTANCE(*db, ecClass, aa);
    INSERT_INSTANCE(*db, ecClass, bb);
    INSERT_INSTANCE(*db, ecClass, cc);
    INSERT_RELATIONSHIP(*db, referencingRelClass, parent, a, rel1);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, b, rel2);
    INSERT_RELATIONSHIP(*db, embeddingRelClass, parent, c, rel3);
    INSERT_RELATIONSHIP(*db, referencingRelClass, parent, aa, rel4);
    INSERT_RELATIONSHIP(*db, holdingRelClass, parent, bb, rel5);
    INSERT_RELATIONSHIP(*db, embeddingRelClass, parent, cc, rel6);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, parent);
    EXPECT_CALL_OnBeforeDelete(listener, db, b);
    EXPECT_CALL_OnBeforeDelete(listener, db, c);
    EXPECT_CALL_OnBeforeDelete(listener, db, bb);
    EXPECT_CALL_OnBeforeDelete(listener, db, cc);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel2);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel3);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel4);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel5);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel6);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({parent})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(2, notDeletedInstances.size());
    EXPECT_NCONTAIN(notDeletedInstances, parent.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, a.GetInstanceId());
    EXPECT_NCONTAIN(notDeletedInstances, b.GetInstanceId());
    EXPECT_NCONTAIN(notDeletedInstances, c.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, aa.GetInstanceId());
    EXPECT_NCONTAIN(notDeletedInstances, bb.GetInstanceId());
    EXPECT_NCONTAIN(notDeletedInstances, cc.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_OnBeforeDeleteReturnsInstancesAndDeletedInstance_SkipsAdditonalDeletedAndSucceeds)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");

    ECInstanceKey a, b, c;
    INSERT_INSTANCE(*db, ecClass, a);
    INSERT_INSTANCE(*db, ecClass, b);
    INSERT_INSTANCE(*db, ecClass, c);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL(listener, OnBeforeDelete(Ref(*ecClass), a.GetInstanceId(), _))
        .WillRepeatedly(Invoke([&] (ECClassCR ecClass, ECInstanceId id, bset<ECInstanceKey>& additionalToDelete)
        {
        EXPECT_INSTANCE_EXISTS(db, a);
        additionalToDelete.insert(a);
        additionalToDelete.insert(b);
        return SUCCESS;
        }));
    EXPECT_CALL_OnBeforeDelete(listener, db, b);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({a})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, c.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_OnBeforeDeleteReturnsAdditionalToDelete_DeletesAdditionalInstances)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");

    ECInstanceKey a, b, c;
    INSERT_INSTANCE(*db, ecClass, a);
    INSERT_INSTANCE(*db, ecClass, b);
    INSERT_INSTANCE(*db, ecClass, c);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL(listener, OnBeforeDelete(Ref(*ecClass), a.GetInstanceId(), _))
        .WillOnce(Invoke([&] (ECClassCR ecClass, ECInstanceId id, bset<ECInstanceKey>& additionalToDelete)
        {
        EXPECT_INSTANCE_EXISTS(db, a);
        additionalToDelete.insert(b);
        additionalToDelete.insert(c);
        return SUCCESS;
        }));
    EXPECT_CALL_OnBeforeDelete(listener, db, b);
    EXPECT_CALL_OnBeforeDelete(listener, db, c);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({a})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(0, notDeletedInstances.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_OnBeforeDeleteReturnsAdditionalToDelete_DeletesAdditionalInstancesWithTheirChildren)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey a, b, c, bchild, cchild, rel1, rel2;
    INSERT_INSTANCE(*db, ecClass, a);
    INSERT_INSTANCE(*db, ecClass, b);
    INSERT_INSTANCE(*db, ecClass, c);
    INSERT_INSTANCE(*db, ecClass, bchild);
    INSERT_INSTANCE(*db, ecClass, cchild);
    INSERT_RELATIONSHIP(*db, holdingRelClass, b, bchild, rel1);
    INSERT_RELATIONSHIP(*db, holdingRelClass, c, cchild, rel2);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL(listener, OnBeforeDelete(Ref(*ecClass), a.GetInstanceId(), _))
        .WillOnce(Invoke([&] (ECClassCR ecClass, ECInstanceId id, bset<ECInstanceKey>& additionalToDelete)
        {
        EXPECT_INSTANCE_EXISTS(db, a);
        additionalToDelete.insert(b);
        return SUCCESS;
        }));
    EXPECT_CALL(listener, OnBeforeDelete(Ref(*ecClass), b.GetInstanceId(), _))
        .WillOnce(Invoke([&] (ECClassCR ecClass, ECInstanceId id, bset<ECInstanceKey>& additionalToDelete)
        {
        EXPECT_INSTANCE_EXISTS(db, b);
        additionalToDelete.insert(c);
        return SUCCESS;
        }));
    EXPECT_CALL_OnBeforeDelete(listener, db, bchild);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    EXPECT_CALL_OnBeforeDelete(listener, db, c);
    EXPECT_CALL_OnBeforeDelete(listener, db, cchild);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel2);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(StubECInstanceKeyMultiMap({a})));
    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(0, notDeletedInstances.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteRelationship_NotExistingRelationship_DoesNothingAndSuccess)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto relClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey a, b;
    INSERT_INSTANCE(*db, ecClass, a);
    INSERT_INSTANCE(*db, ecClass, b);

    CREATE_MockECDbAdapterDeleteListener(listener);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteRelationship(relClass, a, b));

    EXPECT_EQ(2, adapter.FindInstances(ecClass).size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteRelationship_ReferencingRelationship_DeletesRelationshipLeavingEndInstances)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto relClass = adapter.GetECRelationshipClass("TestSchema.ReferencingRel");

    ECInstanceKey a, b, rel;
    INSERT_INSTANCE(*db, ecClass, a);
    INSERT_INSTANCE(*db, ecClass, b);
    INSERT_RELATIONSHIP(*db, relClass, a, b, rel);
    EXPECT_EQ(1, adapter.FindInstances(relClass).size());

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteRelationship(relClass, a, b));

    EXPECT_EQ(2, adapter.FindInstances(ecClass).size());
    EXPECT_EQ(0, adapter.FindInstances(relClass).size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteRelationship_HoldingRelationship_DeletesRelationshipAndChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto relClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey a, b, rel;
    INSERT_INSTANCE(*db, ecClass, a);
    INSERT_INSTANCE(*db, ecClass, b);
    INSERT_RELATIONSHIP(*db, relClass, a, b, rel);
    EXPECT_EQ(1, adapter.FindInstances(relClass).size());

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, b);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteRelationship(relClass, a, b));

    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, a.GetInstanceId());
    EXPECT_EQ(0, adapter.FindInstances(relClass).size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteRelationship_EmbeddingRelationship_DeletesRelationshipAndChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto relClass = adapter.GetECRelationshipClass("TestSchema.EmbeddingRel");

    ECInstanceKey a, b, rel;
    INSERT_INSTANCE(*db, ecClass, a);

    for (int i = 0; i < 2; i++) // Test statement caching for same operations
        {
        INSERT_INSTANCE(*db, ecClass, b);
        INSERT_RELATIONSHIP(*db, relClass, a, b, rel);
        EXPECT_EQ(1, adapter.FindInstances(relClass).size());

        CREATE_MockECDbAdapterDeleteListener(listener);
        EXPECT_CALL_OnBeforeDelete(listener, db, b);
        EXPECT_CALL_OnBeforeDelete(listener, db, rel);
        adapter.RegisterDeleteListener(&listener);

        EXPECT_EQ(SUCCESS, adapter.DeleteRelationship(relClass, a, b));

        auto notDeletedInstances = adapter.FindInstances(ecClass);
        EXPECT_EQ(1, notDeletedInstances.size());
        EXPECT_CONTAINS(notDeletedInstances, a.GetInstanceId());
        EXPECT_EQ(0, adapter.FindInstances(relClass).size());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteRelationship_EmbeddingRelationshipWithRequiredParent_DeletesRelationshipAndChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto parentClass = adapter.GetECClass("TestSchema.TestClass");
    auto childClass = adapter.GetECClass("TestSchema.TestChildClass");
    auto relClass = adapter.GetECRelationshipClass("TestSchema.EmbeddingRequiredRel");

    ECInstanceKey a, b, rel;
    INSERT_INSTANCE(*db, parentClass, a);

    for (int i = 0; i < 2; i++) // Test statement caching for same operations
        {
        ASSERT_FALSE(childClass == nullptr);
        Json::Value instance;
        instance["ParentId"][ECJsonUtilities::json_navId()] = a.GetInstanceId().ToHexStr();
        ASSERT_EQ(BE_SQLITE_OK, JsonInserter(*db, *childClass, nullptr).Insert(b, instance));
        rel = ECInstanceKey(relClass->GetId(), b.GetInstanceId());

        EXPECT_EQ(1, adapter.FindInstances(relClass).size());

        CREATE_MockECDbAdapterDeleteListener(listener);
        EXPECT_CALL_OnBeforeDelete(listener, db, b);
        EXPECT_CALL_OnBeforeDelete(listener, db, rel);
        adapter.RegisterDeleteListener(&listener);

        EXPECT_EQ(SUCCESS, adapter.DeleteRelationship(relClass, a, b));

        auto notDeletedInstances = adapter.FindInstances(parentClass);
        EXPECT_EQ(1, notDeletedInstances.size());
        EXPECT_CONTAINS(notDeletedInstances, a.GetInstanceId());
        EXPECT_EQ(0, adapter.FindInstances(childClass).size());
        EXPECT_EQ(0, adapter.FindInstances(relClass).size());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteRelationship_HoldingRelationshipWithChildWithMultipleParents_DeletesRelationshipLeavingChild)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto relClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey parent, child, otherParent, rel1, rel2;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_INSTANCE(*db, ecClass, otherParent);
    INSERT_RELATIONSHIP(*db, relClass, parent, child, rel1);
    INSERT_RELATIONSHIP(*db, relClass, otherParent, child, rel2);
    EXPECT_EQ(2, adapter.FindInstances(relClass).size());

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel1);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteRelationship(relClass, parent, child));

    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(3, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, parent.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, child.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, otherParent.GetInstanceId());
    EXPECT_EQ(1, adapter.FindInstances(relClass).size());
    EXPECT_FALSE(adapter.HasRelationship(relClass, parent, child));
    EXPECT_TRUE(adapter.HasRelationship(relClass, otherParent, child));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, RelateInstances_EmbeddingRelationshipAndAdditonalParent_Error)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto relClass = adapter.GetECRelationshipClass("TestSchema.EmbeddingRel");

    ECInstanceKey parent, child, otherParent;
    INSERT_INSTANCE(*db, ecClass, parent);
    INSERT_INSTANCE(*db, ecClass, child);
    INSERT_INSTANCE(*db, ecClass, otherParent);

    ASSERT_TRUE(adapter.RelateInstances(relClass, parent, child).IsValid());
    ASSERT_FALSE(adapter.RelateInstances(relClass, otherParent, child).IsValid());

    EXPECT_EQ(1, adapter.FindInstances(relClass).size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteRelationship_OnBeforeDeleteReturnsAdditionalToDelete_DeletesAdditionalInstancesWithTheirChildren)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass = adapter.GetECClass("TestSchema.TestClass");
    auto refRelClass = adapter.GetECRelationshipClass("TestSchema.ReferencingRel");
    auto holdingRelClass = adapter.GetECRelationshipClass("TestSchema.HoldingRel");

    ECInstanceKey a, b, c, d, rel1, rel2;
    INSERT_INSTANCE(*db, ecClass, a);
    INSERT_INSTANCE(*db, ecClass, b);
    INSERT_INSTANCE(*db, ecClass, c);
    INSERT_INSTANCE(*db, ecClass, d);
    INSERT_RELATIONSHIP(*db, refRelClass, a, b, rel1);
    INSERT_RELATIONSHIP(*db, holdingRelClass, c, d, rel2);

    CREATE_MockECDbAdapterDeleteListener(listener);
    EXPECT_CALL(listener, OnBeforeDelete(Ref(*refRelClass), rel1.GetInstanceId(), _))
        .WillOnce(Invoke([&] (ECClassCR ecClass, ECInstanceId id, bset<ECInstanceKey>& additionalToDelete)
        {
        EXPECT_INSTANCE_EXISTS(db, rel1);
        additionalToDelete.insert(c);
        return SUCCESS;
        }));
    EXPECT_CALL_OnBeforeDelete(listener, db, c);
    EXPECT_CALL_OnBeforeDelete(listener, db, d);
    EXPECT_CALL_OnBeforeDelete(listener, db, rel2);
    adapter.RegisterDeleteListener(&listener);

    EXPECT_EQ(SUCCESS, adapter.DeleteRelationship(refRelClass, a, b));

    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(2, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, a.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, b.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_TwoMultipleClassInstances_DeletesInstances)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass1 = adapter.GetECClass("TestSchema.TestClass");
    auto ecClass2 = adapter.GetECClass("TestSchema.TestClass2");

    ECInstanceKey instance;
    ECInstanceKeyMultiMap instances;

    INSERT_INSTANCE(*db, ecClass1, instance);
    instances.insert(ECDbHelper::ToPair(instance));
    INSERT_INSTANCE(*db, ecClass2, instance);
    instances.insert(ECDbHelper::ToPair(instance));

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(instances));
    EXPECT_EQ(0, adapter.FindInstances(ecClass1).size());
    EXPECT_EQ(0, adapter.FindInstances(ecClass2).size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, DeleteInstances_MultipleClassInstances_DeletesInstances)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    auto ecClass1 = adapter.GetECClass("TestSchema.TestClass");
    auto ecClass2 = adapter.GetECClass("TestSchema.TestClass2");
    auto ecClass3 = adapter.GetECClass("TestSchema.TestClass3");

    ECInstanceKey instance;
    ECInstanceKeyMultiMap instances;

    INSERT_INSTANCE(*db, ecClass1, instance);
    instances.insert(ECDbHelper::ToPair(instance));
    INSERT_INSTANCE(*db, ecClass2, instance);
    instances.insert(ECDbHelper::ToPair(instance));
    INSERT_INSTANCE(*db, ecClass3, instance);
    instances.insert(ECDbHelper::ToPair(instance));
    INSERT_INSTANCE(*db, ecClass1, instance);
    instances.insert(ECDbHelper::ToPair(instance));
    INSERT_INSTANCE(*db, ecClass2, instance);
    instances.insert(ECDbHelper::ToPair(instance));
    INSERT_INSTANCE(*db, ecClass3, instance);
    instances.insert(ECDbHelper::ToPair(instance));

    EXPECT_EQ(SUCCESS, adapter.DeleteInstances(instances));
    EXPECT_EQ(0, adapter.FindInstances(ecClass1).size());
    EXPECT_EQ(0, adapter.FindInstances(ecClass2).size());
    EXPECT_EQ(0, adapter.FindInstances(ecClass3).size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, ExtractECInstanceKeys_InvalidStatement_ReturnsErrorAndEmptyList)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    ECSqlStatement statement;
    ECInstanceKeyMultiMap keys;

    BeTest::SetFailOnAssert(false);
    ASSERT_EQ(ERROR, adapter.PrepareStatement(statement, "Invalid statement"));
    BeTest::SetFailOnAssert(true);
    ASSERT_EQ(ERROR, adapter.ExtractECInstanceKeys(statement, keys));
    EXPECT_EQ(0, keys.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, ExtractECInstanceKeys_StatementWithIdsPassed_ReturnsKeys)
    {
    auto db = GetTestDb();
    ECDbAdapter adapter(*db);

    ECInstanceKey a, b;
    ASSERT_EQ(SUCCESS, JsonInserter(*db, *adapter.GetECClass("TestSchema.A"), nullptr).Insert(a, Json::Value()));
    ASSERT_EQ(SUCCESS, JsonInserter(*db, *adapter.GetECClass("TestSchema.B"), nullptr).Insert(b, Json::Value()));

    ECSqlStatement statement;
    ECInstanceKeyMultiMap keys;

    ASSERT_EQ(SUCCESS, adapter.PrepareStatement(statement, "SELECT ECClassId, ECInstanceId FROM TestSchema.A"));
    ASSERT_EQ(SUCCESS, adapter.ExtractECInstanceKeys(statement, keys));
    EXPECT_CONTAINS(keys, ECDbHelper::ToPair(a));
    EXPECT_CONTAINS(keys, ECDbHelper::ToPair(b));
    EXPECT_EQ(2, keys.size());

    statement.Finalize();
    keys.clear();

    ASSERT_EQ(SUCCESS, adapter.PrepareStatement(statement, "SELECT NULL, ECInstanceId, ECClassId FROM TestSchema.A"));
    ASSERT_EQ(SUCCESS, adapter.ExtractECInstanceKeys(statement, keys, nullptr, 2, 1));
    EXPECT_CONTAINS(keys, ECDbHelper::ToPair(a));
    EXPECT_CONTAINS(keys, ECDbHelper::ToPair(b));
    EXPECT_EQ(2, keys.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, ECDb_DeleteRelationship_ShouldNotDeleteEnds1)
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

    statement.BindId(1, rel.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(3, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, parent.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, parent2.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, child.GetInstanceId());
    notDeletedInstances = adapter.FindInstances(holdingRelClass);
    EXPECT_EQ(1, notDeletedInstances.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, ECDb_DeleteRelationship_ShouldNotDeleteEnds2)
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

    statement.BindId(1, rel.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    auto notDeletedInstances = adapter.FindInstances(ecClass);
    EXPECT_EQ(2, notDeletedInstances.size());
    EXPECT_CONTAINS(notDeletedInstances, parent.GetInstanceId());
    EXPECT_CONTAINS(notDeletedInstances, child.GetInstanceId());
    notDeletedInstances = adapter.FindInstances(holdingRelClass);
    EXPECT_EQ(0, notDeletedInstances.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, ECDb_DeleteRelationship_ShouldNotDeleteEnds3)
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

    statement.BindId(1, rel.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    auto notDeletedInstances = adapter.FindInstances(ecClassA);
    EXPECT_EQ(1, notDeletedInstances.size());
    notDeletedInstances = adapter.FindInstances(ecClassB);
    EXPECT_EQ(1, notDeletedInstances.size());
    notDeletedInstances = adapter.FindInstances(holdingRelClass);
    EXPECT_EQ(0, notDeletedInstances.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Petras.Sukys                     11/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, GetInstanceKeyFromJsonInstance_ValidJsonValue_ReturnCorrectECInstanceKey)
    {
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="A" />
        </ECSchema>)xml");
    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);

    auto ecClassA = adapter.GetECClass("TestSchema.A");
    ECInstanceKey instanceKey;
    INSERT_INSTANCE(*db, ecClassA, instanceKey);

    Json::Value jsonValue;
    jsonValue[ECJsonUtilities::json_className()] = "TestSchema.A";
    Utf8String idString;
    idString.Sprintf("%" PRIu64, instanceKey.GetInstanceId().GetValue());
    jsonValue[ECJsonUtilities::json_id()] = idString;

    auto result = adapter.GetInstanceKeyFromJsonInstance(jsonValue);
    EXPECT_EQ(instanceKey, result);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Petras.Sukys                     11/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbAdapterTests, GetInstanceKeyFromJsonInstance_ValidRapidJson_ReturnCorrectECInstanceKey)
    {
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="A" />
        </ECSchema>)xml");
    auto db = CreateTestDb(schema);
    ECDbAdapter adapter(*db);

    auto ecClassA = adapter.GetECClass("TestSchema.A");
    ECInstanceKey instanceKey;
    INSERT_INSTANCE(*db, ecClassA, instanceKey);

    rapidjson::Document document(rapidjson::kObjectType);
    document.AddMember(rapidjson::Value(ECJsonUtilities::json_className().c_str(), document.GetAllocator()).Move(), "TestSchema.A", document.GetAllocator());
    Utf8String idString;
    idString.Sprintf("%" PRIu64, instanceKey.GetInstanceId().GetValue());

    rapidjson::Value name(ECJsonUtilities::json_id().c_str(), document.GetAllocator());
    rapidjson::Value value(idString.c_str(), document.GetAllocator());
    document.AddMember(name, value, document.GetAllocator());

    auto result = adapter.GetInstanceKeyFromJsonInstance(document);
    EXPECT_EQ(instanceKey, result);
    }

#endif
