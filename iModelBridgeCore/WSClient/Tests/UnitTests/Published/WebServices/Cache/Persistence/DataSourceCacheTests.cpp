/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Persistence/DataSourceCacheTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DataSourceCacheTests.h"

#include "../Util/MockECDbSchemaChangeListener.h"
#include <Bentley/BeDebugLog.h>

#ifdef USE_GTEST
using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

template<typename T>
std::set<T> ToStdSet(const bset<T>& bset)
    {
    return std::set<T>(bset.begin(), bset.end());
    }

TEST_F(DataSourceCacheTests, Create_ValidParameters_Success)
    {
    BentleyStatus result = DataSourceCache().Create(BeFileName(":memory:"), CacheEnvironment());
    EXPECT_EQ(SUCCESS, result);
    }

TEST_F(DataSourceCacheTests, Open_NoFile_Error)
    {
    BeFileName path = StubFilePath();
    EXPECT_EQ(ERROR, DataSourceCache().Open(path, CacheEnvironment()));
    }

TEST_F(DataSourceCacheTests, Open_NonECDbFile_Error)
    {
    BeFileName path = StubFile("FooContent");

    EXPECT_EQ(ERROR, DataSourceCache().Open(path, CacheEnvironment()));
    }

TEST_F(DataSourceCacheTests, Open_SimpleECDb_Succeeds)
    {
    BeFileName path = StubFilePath();

    ECDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(path));
    db.CloseDb();

    EXPECT_EQ(SUCCESS, DataSourceCache().Open(path, CacheEnvironment()));
    }

TEST_F(DataSourceCacheTests, Open_ExistingDb_Success)
    {
    BeFileName path = StubFilePath();

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Create(path, StubCacheEnvironemnt()));
    ASSERT_EQ(BE_SQLITE_OK, cache.GetAdapter().GetECDb().AbandonChanges()); // Remove all uncommited changes from default txn
    cache.Close();

    EXPECT_EQ(SUCCESS, DataSourceCache().Open(path, CacheEnvironment()));
    }

TEST_F(DataSourceCacheTests, Open_ExistingDbWithNoDefaultTransaction_Success)
    {
    BeFileName path = StubFilePath();

    ECDb::CreateParams params;
    params.SetStartDefaultTxn(DefaultTxn::No);

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Create(path, StubCacheEnvironemnt(), params));
    ASSERT_EQ(0, cache.GetECDb().GetCurrentSavepointDepth()); // No default txn
    cache.Close();

    EXPECT_EQ(SUCCESS, DataSourceCache().Open(path, CacheEnvironment(), params));
    }

TEST_F(DataSourceCacheTests, GetEnvironment_CreatedWithEmptyEnvironment_Empty)
    {
    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Create(BeFileName(":memory:"), CacheEnvironment()));

    CacheEnvironment cacheEnv = cache.GetEnvironment();
    EXPECT_EQ(L"", cacheEnv.persistentFileCacheDir);
    EXPECT_EQ(L"", cacheEnv.temporaryFileCacheDir);
    EXPECT_EQ(L"", cacheEnv.externalFileCacheDir);
    }

TEST_F(DataSourceCacheTests, GetEnvironment_CreatedWithFullBaseEnvironment_SubFoldersToSameBaseEnvironment)
    {
    CacheEnvironment baseEnv;
    baseEnv.persistentFileCacheDir = GetTestsTempDir().AppendToPath(L"Pers");
    baseEnv.temporaryFileCacheDir = GetTestsTempDir().AppendToPath(L"Temp");
    baseEnv.externalFileCacheDir = GetTestsTempDir().AppendToPath(L"Ext");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Create(BeFileName(":memory:"), baseEnv));

    CacheEnvironment cacheEnv = cache.GetEnvironment();
    EXPECT_THAT(cacheEnv.persistentFileCacheDir.c_str(), HasSubstr(baseEnv.persistentFileCacheDir.c_str()));
    EXPECT_THAT(cacheEnv.temporaryFileCacheDir.c_str(), HasSubstr(baseEnv.temporaryFileCacheDir.c_str()));
    EXPECT_THAT(cacheEnv.externalFileCacheDir.c_str(), HasSubstr(baseEnv.externalFileCacheDir.c_str()));
    }

TEST_F(DataSourceCacheTests, GetEnvironment_CreatedWithoutExternalEnvironmentDir_ExternalIsSubfolderToPersistentDir)
    {
    CacheEnvironment baseEnv;
    baseEnv.persistentFileCacheDir = GetTestsTempDir().AppendToPath(L"Pers");
    baseEnv.temporaryFileCacheDir = GetTestsTempDir().AppendToPath(L"Temp");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Create(BeFileName(":memory:"), baseEnv));

    CacheEnvironment cacheEnv = cache.GetEnvironment();
    EXPECT_THAT(cacheEnv.externalFileCacheDir.c_str(), HasSubstr(baseEnv.persistentFileCacheDir.c_str()));
    EXPECT_THAT(cacheEnv.externalFileCacheDir.c_str(), HasSubstr(cacheEnv.persistentFileCacheDir.c_str()));
    EXPECT_THAT(cacheEnv.externalFileCacheDir.c_str(), AnyOf(EndsWith(L"\\ext\\"), EndsWith(L"//ext//")));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_EmptyVectorPassed_DoesNothingAndSucceeds)
    {
    auto cache = GetTestCache();
    EXPECT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<BeFileName> {}));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_PathPasssed_SuccessAndSchemaAccessable)
    {
    auto cache = GetTestCache();

    BeFileName schemaPath = GetTestSchemaPath();
    ASSERT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<BeFileName> {schemaPath}));

    EXPECT_TRUE(nullptr != cache->GetAdapter().GetECSchema("TestSchema"));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_SchemasPassed_SuccessAndSchemasAccessable)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema(), GetTestSchema2() }));

    EXPECT_TRUE(nullptr != cache->GetAdapter().GetECSchema("TestSchema"));
    EXPECT_TRUE(nullptr != cache->GetAdapter().GetECSchema("TestSchema2"));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_RootInstanceCreated_ShouldNotDeleteRootInstance)
    {
    auto cache = GetTestCache();

    auto root1 = cache->FindOrCreateRoot("Foo");
    ASSERT_TRUE(root1.IsValid());

    cache->GetECDb().SaveChanges();

    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="UpgradeTestSchema" nameSpacePrefix="UTS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass">
                <ECProperty propertyName="TestProperty" typeName="string" />
            </ECClass>
        </ECSchema>)xml");
    ASSERT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<ECSchemaPtr> {schema}));

    cache->GetECDb().SaveChanges();

    EXPECT_TRUE(nullptr != cache->GetAdapter().GetECSchema("TestSchema"));

    auto root2 = cache->FindOrCreateRoot("Foo");
    ASSERT_TRUE(root2.IsValid());
    EXPECT_EQ(root1, root2);
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_SchemasPassedToDataSourceCacheWithCachedStatements_SuccessAndSchemasAccessable)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "A"}));
    ASSERT_TRUE(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid());

    ASSERT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema2()}));
    EXPECT_TRUE(nullptr != cache->GetAdapter().GetECSchema("TestSchema2"));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_SchemaWithoutMajorVersionChangeWithDeletedPropertyPassed_Error)
    {
    auto cache = GetTestCache();

    auto schema1 = ParseSchema(
        R"xml(<ECSchema schemaName="UpdateSchema" nameSpacePrefix="US" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="A" typeName="string" />
                <ECProperty propertyName="B" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    auto schema2 = ParseSchema(
        R"xml(<ECSchema schemaName="UpdateSchema" nameSpacePrefix="US" version="1.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="A" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<ECSchemaPtr> {schema1}));
    ASSERT_TRUE(nullptr != cache->GetAdapter().GetECSchema("UpdateSchema"));
    EXPECT_EQ(ERROR, cache->UpdateSchemas(std::vector<ECSchemaPtr> {schema2}));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_SchemaWithMajorVersionChangeButNoSharedColumnsWithDeletedPropertyPassed_Error)
    {
    auto cache = GetTestCache();

    auto schema1 = ParseSchema(
        R"xml(<ECSchema schemaName="UpdateSchema" nameSpacePrefix="US" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="A" typeName="string" />
                <ECProperty propertyName="B" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    auto schema2 = ParseSchema(
        R"xml(<ECSchema schemaName="UpdateSchema" nameSpacePrefix="US" version="2.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="A" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<ECSchemaPtr> {schema1}));
    ASSERT_TRUE(nullptr != cache->GetAdapter().GetECSchema("UpdateSchema"));
    EXPECT_EQ(ERROR, cache->UpdateSchemas(std::vector<ECSchemaPtr> {schema2}));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_SchemaWithMajorVersionChangeAndRequiredCAsWithDeletedPropertyPassed_SuccessAndPropertyDeleted)
    {
    auto cache = GetTestCache();

    // ShareColumns & TablePerHierarchy is needed for BIM0200 property deletion to work
    auto schema1 = ParseSchema(
        R"xml(<ECSchema schemaName="UpdateSchema" nameSpacePrefix="US" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap"/>
            <ECClass typeName="TestClass" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00" />
                </ECCustomAttributes>
                <ECProperty propertyName="A" typeName="string" />
                <ECProperty propertyName="B" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    auto schema2 = ParseSchema(
        R"xml(<ECSchema schemaName="UpdateSchema" nameSpacePrefix="US" version="2.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap"/>
            <ECClass typeName="TestClass" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00" />
                </ECCustomAttributes>
                <ECProperty propertyName="A" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<ECSchemaPtr> {schema1}));
    ASSERT_TRUE(nullptr != cache->GetAdapter().GetECSchema("UpdateSchema"));

    EXPECT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<ECSchemaPtr> {schema2}));
    ASSERT_TRUE(nullptr != cache->GetAdapter().GetECSchema("UpdateSchema"));
    auto ecSchema = cache->GetAdapter().GetECSchema("UpdateSchema");
    EXPECT_EQ(2, ecSchema->GetVersionRead());
    EXPECT_EQ(0, ecSchema->GetVersionWrite());
    EXPECT_EQ(0, ecSchema->GetVersionMinor());
    auto ecClass = ecSchema->GetClassCP("TestClass");
    EXPECT_TRUE(nullptr != ecClass->GetPropertyP("A"));
    EXPECT_TRUE(nullptr == ecClass->GetPropertyP("B"));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_SchemasWithDeletedPropertyPassedToDataSourceCacheWithCachedStatements_SuccessAndSchemasAccessable)
    {
    auto cache = GetTestCache();

    // ShareColumns & TablePerHierarchy is needed for BIM0200 property deletion to work
    auto schema1 = ParseSchema(
        R"xml(<ECSchema schemaName="UpdateSchema" nameSpacePrefix="US" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap"/>
            <ECClass typeName="TestClass" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00" />
                </ECCustomAttributes>
                <ECProperty propertyName="A" typeName="string" />
                <ECProperty propertyName="B" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    auto schema2 = ParseSchema(
        R"xml(<ECSchema schemaName="UpdateSchema" nameSpacePrefix="US" version="2.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap"/>
            <ECClass typeName="TestClass" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00" />
                </ECCustomAttributes>
                <ECProperty propertyName="A" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<ECSchemaPtr> {schema1}));
    ASSERT_TRUE(nullptr != cache->GetAdapter().GetECSchema("UpdateSchema"));

    // Do some operations to cache statements
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, {"UpdateSchema.TestClass", "Foo"}));
    ASSERT_TRUE(cache->FindInstance({"UpdateSchema.TestClass", "Foo"}).IsValid());

    ASSERT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<ECSchemaPtr> {schema2}));
    ASSERT_TRUE(nullptr != cache->GetAdapter().GetECSchema("UpdateSchema"));
    auto ecSchema = cache->GetAdapter().GetECSchema("UpdateSchema");
    EXPECT_EQ(2, ecSchema->GetVersionRead());
    EXPECT_EQ(0, ecSchema->GetVersionWrite());
    EXPECT_EQ(0, ecSchema->GetVersionMinor());
    auto ecClass = ecSchema->GetClassCP("TestClass");
    EXPECT_TRUE(nullptr != ecClass->GetPropertyP("A"));
    EXPECT_TRUE(nullptr == ecClass->GetPropertyP("B"));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_SchemaWithOneToOneRelationship_ChangesRelationshipToZeroToOneAndAllowsCaching)
    {
    auto cache = GetTestCache();

    // Such schema is not supported by ECDb when caching data with WSCache. UpdateSchemas will adjust it.
    auto schema = ParseSchema(
        R"xml(<ECSchema schemaName="UpdateSchema" nameSpacePrefix="US" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="A" >  
                <ECProperty propertyName="Name" typeName="string" />
            </ECClass>            
            <ECClass typeName="B" >  
                <ECProperty propertyName="Name" typeName="string" />
            </ECClass>
            <ECRelationshipClass typeName="AB" isDomainClass="True" strength="referencing" strengthDirection="forward">
                <Source cardinality="(1,1)" polymorphic="True">
                    <Class class="A" />
                </Source>
                <Target cardinality="(1,1)" polymorphic="True">
                    <Class class="B" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<ECSchemaPtr> {schema}));
    auto cachedSchema = cache->GetAdapter().GetECSchema("UpdateSchema");
    ASSERT_TRUE(nullptr != cachedSchema);
    auto cachedRelClass = cachedSchema->GetClassCP("AB")->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != cachedRelClass);


    // Test caching
    StubInstances instances;
    instances.Add({"UpdateSchema.A", "AA"}).AddRelated({"UpdateSchema.AB", "AABB"}, {"UpdateSchema.B", "BB"});
    auto key = StubCachedResponseKey(*cache);
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    EXPECT_TRUE(VerifyHasRelationship(cache, "UpdateSchema.AB", {"UpdateSchema.A", "AA"}, {"UpdateSchema.B", "BB"}));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_NullSchemaPassed_Error)
    {
    auto cache = GetTestCache();

    BeTest::SetFailOnAssert(false);
    BentleyStatus result = cache->UpdateSchemas(std::vector<ECSchemaPtr> {nullptr});
    BeTest::SetFailOnAssert(true);

    EXPECT_EQ(ERROR, result);
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_SchemaChangeListenerRegisteredAndSchemaPassed_CallsListenerBeforeAndAfterSchemaUpdate)
    {
    MockECDbSchemaChangeListener listener;
    auto cache = GetTestCache();

    cache->RegisterSchemaChangeListener(&listener);

    EXPECT_CALL(listener, OnSchemaChanged()).Times(2);
    ASSERT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema()}));

    cache->UnRegisterSchemaChangeListener(&listener);
    }

TEST_F(DataSourceCacheTests, Close_SchemaChangeListenerRegistered_CallsListener)
    {
    MockECDbSchemaChangeListener listener;
    auto cache = GetTestCache();

    cache->RegisterSchemaChangeListener(&listener);

    EXPECT_CALL(listener, OnSchemaChanged()).Times(1);

    cache->Close();
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_CalledOnOtherConnection_CallsListenerOnceTransactionIsStarted)
    {
    BeFileName path = StubFilePath();
    ECDb::CreateParams params;
    params.SetStartDefaultTxn(DefaultTxn::No);

    MockECDbSchemaChangeListener listener1;
    MockECDbSchemaChangeListener listener2;

    DataSourceCache cache1;
    DataSourceCache cache2;

    ASSERT_EQ(SUCCESS, cache1.Create(path, StubCacheEnvironemnt(), params));
    ASSERT_EQ(SUCCESS, cache2.Open(path, StubCacheEnvironemnt(), params));

    cache1.RegisterSchemaChangeListener(&listener1);
    cache2.RegisterSchemaChangeListener(&listener2);

    // Update schemas on one connection
    Savepoint sp1(cache1.GetECDb(), "Foo", true);

    EXPECT_CALL(listener1, OnSchemaChanged()).Times(2);
    ASSERT_EQ(SUCCESS, cache1.UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema()}));
    Mock::VerifyAndClear(&listener1);

    ASSERT_EQ(BE_SQLITE_OK, sp1.Commit());

    // Start transaction on other connection
    EXPECT_CALL(listener2, OnSchemaChanged()).Times(1);
    Savepoint sp2(cache2.GetECDb(), "Foo", true);
    Mock::VerifyAndClear(&listener2);

    ASSERT_EQ(BE_SQLITE_OK, sp2.Commit());

    cache1.UnRegisterSchemaChangeListener(&listener1);
    cache2.UnRegisterSchemaChangeListener(&listener2);
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_DefaultUsedSchemasPassed_Success)
    {
    DataSourceCache cache;
    cache.Create(BeFileName(":memory:"), CacheEnvironment());

    auto path = GetTestsAssetsDir().AppendToPath(L"ECSchemas/WSClient/Cache/WSCacheMetaSchema.03.00.ecschema.xml");

    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<BeFileName> {path}));

    EXPECT_TRUE(nullptr != cache.GetAdapter().GetECSchema("WSCacheMetaSchema"));
    }

TEST_F(DataSourceCacheTests, GetInstance_NotCached_ReturnsDataNotCachedAndNullInstance)
    {
    auto cache = GetTestCache();

    Json::Value instance;
    EXPECT_EQ(CacheStatus::DataNotCached, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instance));
    EXPECT_TRUE(instance.isNull());
    }

TEST_F(DataSourceCacheTests, GetInstance_LinkedToRoot_ReturnsOk)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));

    Json::Value instance;
    EXPECT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instance));
    }

void VerifyJsonRawFormat(JsonValueCR instance)
    {
    EXPECT_EQ("TestSchema.TestClass", instance[DataSourceCache_PROPERTY_ClassKey].asString());
    EXPECT_EQ("Foo", instance[DataSourceCache_PROPERTY_RemoteId].asString());

    EXPECT_TRUE(instance.isMember(ECJsonUtilities::json_id()));
    }

TEST_F(DataSourceCacheTests, GetInstance_NewInstance_ReturnsPlaceholderInstanceWithExpectedFormat)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));

    Json::Value instance;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instance));

    VerifyJsonRawFormat(instance);
    }

TEST_F(DataSourceCacheTests, UpdateInstance_InstanceNotInCache_ReturnsErrorAndInstanceNotCached)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(ERROR, cache->UpdateInstance({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse()));
    BeTest::SetFailOnAssert(true);

    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, UpdateInstance_InstanceInCache_SuccessfullyUpdates)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "TestValue"}});
    EXPECT_EQ(SUCCESS, cache->UpdateInstance({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse()));

    Json::Value updatedInstance;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, updatedInstance));

    EXPECT_EQ(Utf8String("TestValue"), updatedInstance["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, GetCachedObjectInfo_InstanceNotCached_IsFullyCachedIsFalse)
    {
    auto cache = GetTestCache();

    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, GetCachedObjectInfo_InstanceCached_IsFullyCachedIsTrue)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), "foo_root"));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, RemoveInstance_InstanceNotCached_ReturnsDataNotCached)
    {
    auto cache = GetTestCache();

    EXPECT_EQ(CacheStatus::DataNotCached, cache->RemoveInstance({"TestSchema.TestClass", "Foo"}));
    }

TEST_F(DataSourceCacheTests, RemoveInstance_InstanceCached_DeletesCachedInstanceFromECDb)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), "foo_root"));

    auto instanceKey = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(DoesInstanceExist(*cache, instanceKey));

    EXPECT_EQ(CacheStatus::OK, cache->RemoveInstance({"TestSchema.TestClass", "Foo"}));
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    EXPECT_FALSE(DoesInstanceExist(*cache, instanceKey));
    }

TEST_F(DataSourceCacheTests, RemoveInstance_NavigationBaseObject_DeletesCachedInstanceFromECDb)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Foo", ObjectId()));
    ASSERT_TRUE(cache->FindInstance(ObjectId()).IsValid());

    EXPECT_EQ(CacheStatus::OK, cache->RemoveInstance(ObjectId()));
    EXPECT_FALSE(cache->FindInstance(ObjectId()).IsValid());
    }

TEST_F(DataSourceCacheTests, RemoveInstance_NotExistingNavigationBaseObject_ReturnsDataNotCached)
    {
    auto cache = GetTestCache();

    ASSERT_FALSE(cache->FindInstance(ObjectId()).IsValid());
    EXPECT_EQ(CacheStatus::DataNotCached, cache->RemoveInstance(ObjectId()));
    }

TEST_F(DataSourceCacheTests, RemoveInstance_ChildQueryExists_DeletesQueryResults)
    {
    auto cache = GetTestCache();

    StubInstances parentInstance;
    parentInstance.Add({"TestSchema.TestClass", "parent"});
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "parent"}, parentInstance.ToWSObjectsResponse(), "foo_root"));
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "parent"}), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "resultA"});
    instances.Add({"TestSchema.TestClass", "resultB"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "resultA"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "resultB"}).IsFullyCached());

    EXPECT_EQ(CacheStatus::OK, cache->RemoveInstance({"TestSchema.TestClass", "parent"}));

    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "parent"}).IsFullyCached());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "resultA"}).IsFullyCached());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "resultB"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, RemoveInstance_ChildQueryResultHasOtherParent_DoesNotDeleteResult)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "parentA"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "parentB"}));

    CachedResponseKey responseKeyA(cache->FindInstance({"TestSchema.TestClass", "parentA"}), nullptr);
    CachedResponseKey responseKeyB(cache->FindInstance({"TestSchema.TestClass", "parentB"}), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "result"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKeyA, instances.ToWSObjectsResponse()));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKeyB, instances.ToWSObjectsResponse()));

    EXPECT_EQ(CacheStatus::OK, cache->RemoveInstance({"TestSchema.TestClass", "parentA"}));
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "result"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, RemoveInstance_InstanceIsInCachedQueryResults_QueryResultCacheTagIsRemoved)
    {
    auto cache = GetTestCache();
    CachedResponseKey responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse("TestTag")));
    ASSERT_EQ("TestTag", cache->ReadResponseCacheTag(responseKey));

    ASSERT_EQ(CacheStatus::OK, cache->RemoveInstance({"TestSchema.TestClass", "A"}));

    EXPECT_EQ("", cache->ReadResponseCacheTag(responseKey));
    bset<ObjectId> responseIds;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseObjectIds(responseKey, responseIds));
    EXPECT_THAT(responseIds, SizeIs(1));
    EXPECT_THAT(responseIds, Contains(ObjectId("TestSchema.TestClass", "B")));
    }

TEST_F(DataSourceCacheTests, RemoveInstance_HasCachedFile_FileIsDeleted)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));

    ASSERT_EQ(SUCCESS, cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(), FileCache::Persistent));

    BeFileName cachedFilePath = cache->ReadFilePath({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(cachedFilePath.DoesPathExist());

    EXPECT_EQ(CacheStatus::OK, cache->RemoveInstance({"TestSchema.TestClass", "Foo"}));
    EXPECT_FALSE(cachedFilePath.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, RemoveInstance_ChildQueryResultInstanceIsInRoot_LeavesResultInstanceInCache)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("parent_root", {"TestSchema.TestClass", "parent"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("child_root", {"TestSchema.TestClass", "resultInRoot"}));

    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "parent"}), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "resultInRoot"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_EQ(CacheStatus::OK, cache->RemoveInstance({"TestSchema.TestClass", "parent"}));
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "resultInRoot"}).IsFullyCached());
    }

//TEST_F (DataSourceCacheTests, RemoveInstance_UnderlyingTreeHasCyclicHoldingRelationships_StillRemovesAllTree)
//    {
//    auto cache = GetTestCache ();
//    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot ("foo_root", {"TestSchema.TestClass", "parent"});
//
//    Json::Value children;
//    children["TestClass"][0][DataSourceCache_PROPERTY_RemoteId] = "childA";
//    ToRapidJson (childrenRJ1, children);
//    ASSERT_EQ(SUCCESS, cache->SetChildren ({"TestSchema.TestClass", "parent"}, childrenRJ1, nullptr);
//
//    children.clear ();
//
//    children["TestClass"][0][DataSourceCache_PROPERTY_RemoteId] = "childB";
//    ToRapidJson (childrenRJ2, children);
//    ASSERT_EQ(SUCCESS, cache->SetChildren ({"TestSchema.TestClass", "childA"}, childrenRJ2, nullptr);
//
//    children.clear ();
//
//    children["TestClass"][0][DataSourceCache_PROPERTY_RemoteId] = "childA";
//    ToRapidJson (childrenRJ3, children);
//    ASSERT_EQ(SUCCESS, cache->SetChildren ({"TestSchema.TestClass", "childB"}, childrenRJ3, nullptr);
//
//    EXPECT_TRUE (cache->HasInstance ({"TestSchema.TestClass", "parent"}));
//    EXPECT_TRUE (cache->HasInstance ({"TestSchema.TestClass", "childB"}));
//    EXPECT_TRUE (cache->HasInstance ({"TestSchema.TestClass", "childA"}));
//
//    ASSERT_EQ(SUCCESS, cache->RemoveInstance ({"TestSchema.TestClass", "parent"});
//
//    EXPECT_FALSE (cache->HasInstance ({"TestSchema.TestClass", "childB"}));
//    EXPECT_FALSE (cache->HasInstance ({"TestSchema.TestClass", "childA"}));
//    }

TEST_F(DataSourceCacheTests, RemoveInstance_ChildQueryResultInstanceIsInWeaklyLinkedToRoot_RemovesResultInstanceInCache)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "resultInRoot"});

    ASSERT_EQ(SUCCESS, cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "foo_root", nullptr, true));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "parent"}));

    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "parent"}), nullptr);
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_EQ(CacheStatus::OK, cache->RemoveInstance({"TestSchema.TestClass", "parent"}));
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "resultInRoot"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, FindInstance_NotCachedInstance_Null)
    {
    auto cache = GetTestCache();

    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, FindInstance_PassedCachedRelationshipId_ReturnsInvalidAsFindRelationshipShouldBeUsed)
    {
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestRelationshipClass", "AB"}).IsValid());
    }

TEST_F(DataSourceCacheTests, FindCachedObject_InstanceLinkedToRoot_IsNotNull)
    {
    auto cache = GetTestCache();

    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, LinkInstanceToRoot_NotCachedInstance_HasInstance)
    {
    auto cache = GetTestCache();

    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, UnlinkInstanceFromRoot_ExistingInstanceLinkedToRoot_InstanceRemoved)
    {
    auto cache = GetTestCache();
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("A", {"TestSchema.TestClass", "Foo"}));

    EXPECT_EQ(SUCCESS, cache->UnlinkInstanceFromRoot("A", {"TestSchema.TestClass", "Foo"}));
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, UnlinkInstanceFromRoot_ExistingInstanceButLinkedToOtherRoot_InstanceNotRemoved)
    {
    auto cache = GetTestCache();
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("A", {"TestSchema.TestClass", "IdA"}));
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("B", {"TestSchema.TestClass", "IdB"}));

    EXPECT_EQ(SUCCESS, cache->UnlinkInstanceFromRoot("A", {"TestSchema.TestClass", "IdB"}));
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "IdA"}).IsValid());
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "IdB"}).IsValid());
    }

TEST_F(DataSourceCacheTests, UnlinkInstanceFromRoot_ExistingInstanceLinkedAlsoToOtherRoot_InstanceNotRemoved)
    {
    auto cache = GetTestCache();
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("A", {"TestSchema.TestClass", "Foo"}));
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("B", {"TestSchema.TestClass", "Foo"}));

    EXPECT_EQ(SUCCESS, cache->UnlinkInstanceFromRoot("A", {"TestSchema.TestClass", "Foo"}));
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, UnlinkInstanceFromRoot_NotCachedInstance_Suceeds)
    {
    auto cache = GetTestCache();
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("A", {"TestSchema.TestClass", "Foo"}));

    EXPECT_EQ(SUCCESS, cache->UnlinkInstanceFromRoot("A", {"TestSchema.TestClass", "NotExistingInstance"}));
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, UnlinkInstanceFromRoot_NonExistingRoot_SuceedsAndKeepsInstance)
    {
    auto cache = GetTestCache();
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("A", {"TestSchema.TestClass", "Foo"}));

    EXPECT_EQ(SUCCESS, cache->UnlinkInstanceFromRoot("NonExistingRoot", {"TestSchema.TestClass", "Foo"}));
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheInstanceAndLinkToRoot_ObjectIdDoesNotMachWSObjectsResponse_ErrorIsReturned)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "DifferentId"});

    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(ERROR, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "SomeOtherId"}, instances.ToWSObjectsResponse(), nullptr));
    BeTest::SetFailOnAssert(true);
    }

TEST_F(DataSourceCacheTests, CacheInstanceAndLinkToRoot_WSObjectResultPassed_InstanceInRootAndFullyCached)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    EXPECT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), "Root"));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    EXPECT_TRUE(cache->IsInstanceInRoot("Root", cache->FindInstance({"TestSchema.TestClass", "Foo"})));
    }

TEST_F(DataSourceCacheTests, CacheInstanceAndLinkToRoot_InstanceJsonPassed_InstanceInRootAndFullyCached)
    {
    auto cache = GetTestCache();

    rapidjson::Value instanceJson(rapidjson::kObjectType);
    EXPECT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, instanceJson, "CacheTag", "Root"));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    EXPECT_TRUE(cache->IsInstanceInRoot("Root", cache->FindInstance({"TestSchema.TestClass", "Foo"})));
    }

TEST_F(DataSourceCacheTests, CacheInstancesAndLinkToRoot_OneInstance_InstanceCached)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    EXPECT_EQ(SUCCESS, cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "Root"));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    EXPECT_TRUE(cache->IsInstanceInRoot("Root", cache->FindInstance({"TestSchema.TestClass", "Foo"})));
    }

TEST_F(DataSourceCacheTests, CacheInstancesAndLinkToRoot_OneInstanceWithWeakLink_InstanceCached)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    EXPECT_EQ(SUCCESS, cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "Root", nullptr, true));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheInstancesAndLinkToRoot_OneInstanceWithETag_ETagIsCached)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    ObjectIdCR objectId {"TestSchema.TestClass", "Id"};
    instances.Add(objectId, {}, "TestTag");

    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot(objectId, instances.ToWSObjectsResponse(), nullptr));

    EXPECT_EQ("TestTag", cache->ReadInstanceCacheTag(objectId));
    }

TEST_F(DataSourceCacheTests, CacheInstancesAndLinkToRoot_RelatedInstancesWithWeakLink_InstancesCached)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    EXPECT_EQ(SUCCESS, cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "Root", nullptr, true));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheInstancesAndLinkToRoot_RelatedInstancesWitStrongLink_InstancesCached)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    EXPECT_EQ(SUCCESS, cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "Root", nullptr, true));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, RemoveRoot_RootHasLinkedInstance_InstanceRemovedFromCache)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));

    EXPECT_EQ(SUCCESS, cache->RemoveRoot("foo_root"));

    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, RemoveRoot_RootHasWeaklyLinkedInstance_InstanceRemovedFromCache)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "foo_root", nullptr, true));

    EXPECT_EQ(SUCCESS, cache->RemoveRoot("foo_root"));

    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, RemoveRoot_InstanceLinkedToSeveralRoots_InstanceNotRemoved)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("root1", {"TestSchema.TestClass", "Foo"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("root2", {"TestSchema.TestClass", "Foo"}));

    EXPECT_EQ(SUCCESS, cache->RemoveRoot("root1"));

    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, RemoveRoot_RootContainsCachedQueryWithCyclicRelationshipToItsParent_QueryResultsAndParentDeleted)
    {
    auto cache = GetTestCache();

    cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "CyclicParent"});
    auto instanceA = cache->FindInstance({"TestSchema.TestClass", "CyclicParent"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "CyclicParent"});
    cache->CacheResponse({instanceA, "TestQuery"}, instances.ToWSObjectsResponse());

    cache->RemoveRoot(nullptr);

    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "CyclicParent"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, RemoveRootsByPrefix_NoSuchRoots_ReturnsSuccess)
    {
    auto cache = GetTestCache();
    EXPECT_EQ(SUCCESS, cache->RemoveRootsByPrefix("Foo"));
    }

TEST_F(DataSourceCacheTests, RemoveRootsByPrefix_RootsWithLinkedInstancesExist_DeletesOnlyPrefixedRootLinkedInstances)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Test", {"TestSchema.TestClass", "A"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("TestFoo", {"TestSchema.TestClass", "B"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("BooTestFoo", {"TestSchema.TestClass", "C"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("FooTest", {"TestSchema.TestClass", "D"}));

    EXPECT_EQ(SUCCESS, cache->RemoveRootsByPrefix("Test"));

    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "C"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "D"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, IsInstanceConnectedToRoot_InstanceIsRootChild_True)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("root", {"TestSchema.TestClass", "Foo"}));

    EXPECT_TRUE(cache->IsInstanceConnectedToRoot("root", cache->FindInstance({"TestSchema.TestClass", "Foo"})));
    }

TEST_F(DataSourceCacheTests, ReadInstancesLinkedToRoot_RootNotUsed_ReturnsSuccessAndNoInstances)
    {
    auto cache = GetTestCache();

    Json::Value instances;
    EXPECT_EQ(SUCCESS, cache->ReadInstancesLinkedToRoot("NotUsedRoot", instances));

    EXPECT_EQ(0, instances.size());
    }

TEST_F(DataSourceCacheTests, ReadInstancesLinkedToRoot_TwoDifferentClassInstancesLinkedToRoot_ReturnsInstances)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "A"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass2", "B"}));

    Json::Value instances;
    EXPECT_EQ(SUCCESS, cache->ReadInstancesLinkedToRoot("TestRoot", instances));

    EXPECT_EQ(2, instances.size());
    EXPECT_THAT(cache->ObjectIdFromJsonInstance(instances[0]), AnyOf(ObjectId("TestSchema.TestClass", "A"), ObjectId("TestSchema.TestClass2", "B")));
    EXPECT_THAT(cache->ObjectIdFromJsonInstance(instances[1]), AnyOf(ObjectId("TestSchema.TestClass", "A"), ObjectId("TestSchema.TestClass2", "B")));
    }

TEST_F(DataSourceCacheTests, RemoveFile_NotExistingInstance_Success)
    {
    auto cache = GetTestCache();
    EXPECT_EQ(SUCCESS, cache->RemoveFile({"TestSchema.TestClass", "NotExisting"}));
    }

TEST_F(DataSourceCacheTests, RemoveFile_NotExistingFile_Success)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache);
    EXPECT_EQ(SUCCESS, cache->RemoveFile(cache->FindInstance(instance)));
    }

TEST_F(DataSourceCacheTests, RemoveFile_FileCachedToTemporary_DeletesFileWithContainingFolder)
    {
    auto cache = GetTestCache();
    auto fileId = StubFileInCache(*cache, FileCache::Temporary);
    auto path = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path.DoesPathExist());

    EXPECT_EQ(SUCCESS, cache->RemoveFile(fileId));

    EXPECT_EQ(L"", cache->ReadFilePath(fileId));
    EXPECT_FALSE(path.DoesPathExist());
    EXPECT_FALSE(path.GetDirectoryName().DoesPathExist());
    EXPECT_TRUE(GetTestCacheEnvironment().temporaryFileCacheDir.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, RemoveFile_FileCachedToPersistent_DeletesFileWithContainingFolder)
    {
    auto cache = GetTestCache();
    auto fileId = StubFileInCache(*cache, FileCache::Persistent);
    auto path = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path.DoesPathExist());

    EXPECT_EQ(SUCCESS, cache->RemoveFile(fileId));

    EXPECT_EQ(L"", cache->ReadFilePath(fileId));
    EXPECT_FALSE(path.DoesPathExist());
    EXPECT_FALSE(path.GetDirectoryName().DoesPathExist());
    EXPECT_TRUE(GetTestCacheEnvironment().persistentFileCacheDir.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, RemoveFile_FileCachedToExternal_DeletesFileWithSubFolderButLeavesExternalDir)
    {
    auto cache = GetTestCache();
    auto fileId = StubFileInCache(*cache, FileCache::Persistent);
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External, BeFileName("SubFolder")));
    auto path = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path.DoesPathExist());

    EXPECT_EQ(SUCCESS, cache->RemoveFile(fileId));

    EXPECT_EQ(L"", cache->ReadFilePath(fileId));
    EXPECT_FALSE(path.DoesPathExist());

    EXPECT_TRUE(BeFileName(cache->GetEnvironment().externalFileCacheDir).DoesPathExist());
    EXPECT_THAT(GetFolderContent(cache->GetEnvironment().externalFileCacheDir), IsEmpty());
    }

TEST_F(DataSourceCacheTests, RemoveFile_TwoFilesInExternalFolderAndDeletingOneOfThem_MovesFileButLeavesExternalSubfFolder)
    {
    auto cache = GetTestCache();
    ObjectId fileIdA = StubFileInCache(*cache, FileCache::Temporary, {"TestSchema.TestClass", "A"});
    ObjectId fileIdB = StubFileInCache(*cache, FileCache::Temporary, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileIdA, FileCache::External, BeFileName(L"SubFolder")));
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileIdB, FileCache::External, BeFileName(L"SubFolder")));
    BeFileName pathA = cache->ReadFilePath(fileIdA);
    BeFileName pathB = cache->ReadFilePath(fileIdB);
    EXPECT_TRUE(pathA.DoesPathExist());
    EXPECT_TRUE(pathB.DoesPathExist());

    ASSERT_EQ(SUCCESS, cache->RemoveFile(fileIdA));

    EXPECT_EQ(L"", cache->ReadFilePath(fileIdA));
    EXPECT_FALSE(pathA.DoesPathExist());

    EXPECT_EQ(pathB, cache->ReadFilePath(fileIdB));
    EXPECT_TRUE(pathB.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, RemoveFile_AdditionalFileUpInExternalDirectory_RemovesEmptySubDirectoriesOnly)
    {
    auto cache = GetTestCache();
    ObjectId fileIdA = StubFileInCache(*cache, FileCache::Temporary, {"TestSchema.TestClass", "A"});
    ObjectId fileIdB = StubFileInCache(*cache, FileCache::Temporary, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileIdA, FileCache::External, BeFileName(L"A/B/C")));
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileIdB, FileCache::External, BeFileName(L"A/B")));
    BeFileName pathA = cache->ReadFilePath(fileIdA);
    BeFileName pathB = cache->ReadFilePath(fileIdB);
    EXPECT_TRUE(pathA.DoesPathExist());
    EXPECT_TRUE(pathB.DoesPathExist());

    ASSERT_EQ(SUCCESS, cache->RemoveFile(fileIdA));

    EXPECT_EQ(L"", cache->ReadFilePath(fileIdA));
    EXPECT_FALSE(pathA.DoesPathExist());

    EXPECT_EQ(pathB, cache->ReadFilePath(fileIdB));
    EXPECT_TRUE(pathB.DoesPathExist());
    EXPECT_TRUE(BeFileName(cache->GetEnvironment().externalFileCacheDir).AppendToPath(L"A/B").DoesPathExist());
    EXPECT_FALSE(BeFileName(cache->GetEnvironment().externalFileCacheDir).AppendToPath(L"A/B/C").DoesPathExist());
    }

TEST_F(DataSourceCacheTests, RemoveFilesInTemporaryPersistence_RootPersistenceSetToFull_LeavesFile)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("foo_root", CacheRootPersistence::Full));
    ASSERT_EQ(SUCCESS, cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(), FileCache::Persistent));
    auto path = cache->ReadFilePath({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(path.DoesPathExist());

    EXPECT_EQ(SUCCESS, cache->RemoveFilesInTemporaryPersistence());

    EXPECT_EQ(path, cache->ReadFilePath({"TestSchema.TestClass", "Foo"}));
    EXPECT_TRUE(path.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, RemoveFilesInTemporaryPersistence_RootPersistenceSetToTemporaryAndFileCachedToTemporary_DeletesFileWithContainingFolder)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("foo_root", CacheRootPersistence::Temporary));
    ASSERT_EQ(SUCCESS, cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(), FileCache::Temporary));
    auto path = cache->ReadFilePath({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(path.DoesPathExist());

    EXPECT_EQ(SUCCESS, cache->RemoveFilesInTemporaryPersistence());

    EXPECT_EQ(L"", cache->ReadFilePath({"TestSchema.TestClass", "Foo"}));
    EXPECT_FALSE(path.DoesPathExist());
    EXPECT_FALSE(path.GetDirectoryName().DoesPathExist());
    EXPECT_TRUE(GetTestCacheEnvironment().temporaryFileCacheDir.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, RemoveFilesInTemporaryPersistence_RootPersistenceSetToTemporaryAndFileCachedToPersistent_DeletesFileWithContainingFolder)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("foo_root", CacheRootPersistence::Temporary));
    ASSERT_EQ(SUCCESS, cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(), FileCache::Persistent));
    auto path = cache->ReadFilePath({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(path.DoesPathExist());

    EXPECT_EQ(SUCCESS, cache->RemoveFilesInTemporaryPersistence());

    EXPECT_EQ(L"", cache->ReadFilePath({"TestSchema.TestClass", "Foo"}));
    EXPECT_FALSE(path.DoesPathExist());
    EXPECT_FALSE(path.GetDirectoryName().DoesPathExist());
    EXPECT_TRUE(GetTestCacheEnvironment().persistentFileCacheDir.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, RemoveFilesInTemporaryPersistence_RootPersistenceSetToTemporaryAndFileCachedToExternal_DeletesFileWithSubFolderButLeavesExternalDir)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("foo_root", CacheRootPersistence::Temporary));
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation({"TestSchema.TestClass", "Foo"}, FileCache::External, BeFileName("SubFolder")));
    ASSERT_EQ(SUCCESS, cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(), FileCache::External));
    auto path = cache->ReadFilePath({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(path.DoesPathExist());

    EXPECT_EQ(SUCCESS, cache->RemoveFilesInTemporaryPersistence());

    EXPECT_EQ(L"", cache->ReadFilePath({"TestSchema.TestClass", "Foo"}));
    EXPECT_FALSE(path.DoesPathExist());

    EXPECT_TRUE(BeFileName(cache->GetEnvironment().externalFileCacheDir).DoesPathExist());
    EXPECT_THAT(GetFolderContent(cache->GetEnvironment().externalFileCacheDir), IsEmpty());
    }

TEST_F(DataSourceCacheTests, RemoveFilesInTemporaryPersistence_RootPersistenceSetToTemporaryAndOldFileCachedToPersistent_DeletesFile)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("foo_root", CacheRootPersistence::Temporary));
    ASSERT_EQ(SUCCESS, cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(), FileCache::Persistent));
    BeFileName path = cache->ReadFilePath({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(path.DoesPathExist());

    int64_t unixMs;
    EXPECT_EQ(SUCCESS, DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(unixMs));
    unixMs -= 3600 * 1000;
    time_t unixTime = static_cast<time_t>(unixMs / 1000 - 1);
    EXPECT_EQ(BeFileNameStatus::Success, path.SetFileTime(&unixTime, nullptr));

    DateTime maxAccessDateTime;
    EXPECT_EQ(SUCCESS, DateTime::FromUnixMilliseconds(maxAccessDateTime, unixMs));
    EXPECT_EQ(SUCCESS, cache->RemoveFilesInTemporaryPersistence(&maxAccessDateTime));

    EXPECT_EQ(L"", cache->ReadFilePath({"TestSchema.TestClass", "Foo"}));
    EXPECT_FALSE(path.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, RemoveFilesInTemporaryPersistence_RootPersistenceSetToTemporaryAndNewFileCachedToPersistent_LeavesFile)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("foo_root", CacheRootPersistence::Temporary));
    ASSERT_EQ(SUCCESS, cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(), FileCache::Persistent));
    BeFileName path = cache->ReadFilePath({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(path.DoesPathExist());

    int64_t unixMs;
    EXPECT_EQ(SUCCESS, DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(unixMs));
    unixMs -= 3600 * 1000;
    time_t unixTime = static_cast<time_t>(unixMs / 1000 + 1);
    EXPECT_EQ(BeFileNameStatus::Success, path.SetFileTime(&unixTime, nullptr));

    DateTime maxAccessDateTime;
    EXPECT_EQ(SUCCESS, DateTime::FromUnixMilliseconds(maxAccessDateTime, unixMs));
    EXPECT_EQ(SUCCESS, cache->RemoveFilesInTemporaryPersistence(&maxAccessDateTime));

    EXPECT_EQ(path, cache->ReadFilePath({"TestSchema.TestClass", "Foo"}));
    EXPECT_TRUE(path.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, RemoveFilesInTemporaryPersistence_ModifiedFileExists_LeavesModifiedFile)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("foo_root", CacheRootPersistence::Temporary));
    auto instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});

    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile(), false));
    EXPECT_TRUE(cache->ReadFilePath(instance).DoesPathExist());

    EXPECT_EQ(SUCCESS, cache->RemoveFilesInTemporaryPersistence());

    EXPECT_TRUE(cache->ReadFilePath(instance).DoesPathExist());
    }

TEST_F(DataSourceCacheTests, Reset_InstanceLinkedToRoot_InstanceRemoved)
    {
    // Arrange
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));
    // Act
    ASSERT_EQ(SUCCESS, cache->Reset());
    // Assert
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, Reset_NullRootRemovedWithNavigationBase_NavigationBaseIsNotInCache)
    {
    // Arrange
    auto cache = GetTestCache();
    // Act
    ASSERT_EQ(SUCCESS, cache->Reset());
    // Assert
    EXPECT_FALSE(cache->FindInstance(ObjectId()).IsValid());
    }

TEST_F(DataSourceCacheTests, Reset_FileCachedBefore_CachesNewFileToSameLocation)
    {
    // Arrange
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));
    ASSERT_EQ(SUCCESS, cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(), FileCache::Temporary));
    BeFileName oldCachedPath = cache->ReadFilePath({"TestSchema.TestClass", "Foo"});
    // Act
    EXPECT_EQ(SUCCESS, cache->Reset());
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "other_foo"}));
    ASSERT_EQ(SUCCESS, cache->CacheFile({"TestSchema.TestClass", "other_foo"}, StubWSFileResponse(), FileCache::Temporary));
    BeFileName newCachedPath = cache->ReadFilePath({"TestSchema.TestClass", "other_foo"});
    // Assert
    EXPECT_FALSE(oldCachedPath.empty());
    EXPECT_FALSE(newCachedPath.empty());
    EXPECT_FALSE(oldCachedPath.DoesPathExist());
    EXPECT_EQ(Utf8String(BeFileName::GetDirectoryName(oldCachedPath)),
              Utf8String(BeFileName::GetDirectoryName(newCachedPath)));
    }

TEST_F(DataSourceCacheTests, IsObjectFullyPersisted_NonExistingObject_False)
    {
    auto cache = GetTestCache();

    EXPECT_FALSE(cache->IsInstanceFullyPersisted({"TestSchema.TestClass", "NonExistant"}));
    }

TEST_F(DataSourceCacheTests, IsObjectFullyPersisted_ObjectInFullPersistenceRoot_True)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->SetupRoot("foo_root", CacheRootPersistence::Full));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));

    EXPECT_TRUE(cache->IsInstanceFullyPersisted({"TestSchema.TestClass", "Foo"}));
    }

TEST_F(DataSourceCacheTests, IsObjectFullyPersisted_ObjectInTemporaryPersistenceRoot_False)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->SetupRoot("foo_root", CacheRootPersistence::Temporary));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));

    EXPECT_FALSE(cache->IsInstanceFullyPersisted({"TestSchema.TestClass", "Foo"}));
    }

TEST_F(DataSourceCacheTests, IsObjectFullyPersisted_ObjectQueryParentInFullPersistenceRoot_True)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->SetupRoot("foo_root", CacheRootPersistence::Full));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "parent"}));
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "parent"}), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_TRUE(cache->IsInstanceFullyPersisted({"TestSchema.TestClass", "Foo"}));
    }

TEST_F(DataSourceCacheTests, IsObjectFullyPersisted_ObjectInTemporaryAndFullPersistenceRoots_True)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->SetupRoot("temp_root", CacheRootPersistence::Temporary));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("full_root", CacheRootPersistence::Full));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("temp_root", {"TestSchema.TestClass", "Foo"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("full_root", {"TestSchema.TestClass", "Foo"}));

    EXPECT_TRUE(cache->IsInstanceFullyPersisted({"TestSchema.TestClass", "Foo"}));
    }

TEST_F(DataSourceCacheTests, SetupRoot_NewRoot_Succeeds)
    {
    auto cache = GetTestCache();

    EXPECT_EQ(SUCCESS, cache->SetupRoot("foo", CacheRootPersistence::Temporary));
    EXPECT_EQ(SUCCESS, cache->SetupRoot("boo", CacheRootPersistence::Full));
    }

TEST_F(DataSourceCacheTests, DoesRootExist_NonExistingRoot_FalseAndRootIsNotCreated)
    {
    auto cache = GetTestCache();

    EXPECT_FALSE(cache->DoesRootExist("Foo"));
    EXPECT_FALSE(cache->DoesRootExist("Foo"));
    }

TEST_F(DataSourceCacheTests, DoesRootExist_ExistingRoot_True)
    {
    auto cache = GetTestCache();
    ASSERT_TRUE(cache->FindOrCreateRoot("Foo").IsValid());

    EXPECT_TRUE(cache->DoesRootExist("Foo"));
    }

TEST_F(DataSourceCacheTests, FindOrCreateRoot_NonExistingRoot_CreatesNewRoot)
    {
    auto cache = GetTestCache();

    EXPECT_TRUE(cache->FindOrCreateRoot("Foo").IsValid());
    }

TEST_F(DataSourceCacheTests, FindOrCreateRoot_DifferentRoot_CreatesNewRoot)
    {
    auto cache = GetTestCache();

    auto rootKeyA = cache->FindOrCreateRoot("A");
    auto rootKeyB = cache->FindOrCreateRoot("B");

    EXPECT_TRUE(rootKeyA.IsValid());
    EXPECT_TRUE(rootKeyB.IsValid());
    EXPECT_THAT(rootKeyA, Not(rootKeyB));
    }

TEST_F(DataSourceCacheTests, FindOrCreateRoot_ExistingRoot_ReturnsSameKey)
    {
    auto cache = GetTestCache();

    auto rootKey = cache->FindOrCreateRoot("Foo");
    EXPECT_TRUE(rootKey.IsValid());

    EXPECT_THAT(cache->FindOrCreateRoot("Foo"), rootKey);
    }

TEST_F(DataSourceCacheTests, RenameRoot_ExistingRoot_SuccessAndRenamesRoot)
    {
    auto cache = GetTestCache();

    ASSERT_THAT(cache->FindOrCreateRoot("Root").IsValid(), true);
    EXPECT_THAT(cache->DoesRootExist("Other"), false);

    ASSERT_THAT(cache->RenameRoot("Root", "Other"), SUCCESS);
    EXPECT_THAT(cache->DoesRootExist("Root"), false);
    EXPECT_THAT(cache->DoesRootExist("Other"), true);
    }

TEST_F(DataSourceCacheTests, RenameRoot_NotExistingRoot_SuccessAndCreatesRoot)
    {
    auto cache = GetTestCache();

    EXPECT_THAT(cache->DoesRootExist("Root"), false);
    EXPECT_THAT(cache->DoesRootExist("Other"), false);

    ASSERT_THAT(cache->RenameRoot("Root", "Other"), SUCCESS);
    EXPECT_THAT(cache->DoesRootExist("Root"), false);
    EXPECT_THAT(cache->DoesRootExist("Other"), true);
    }

TEST_F(DataSourceCacheTests, RenameRoot_TargetRootExists_Error)
    {
    auto cache = GetTestCache();

    ASSERT_THAT(cache->FindOrCreateRoot("Root").IsValid(), true);
    ASSERT_THAT(cache->FindOrCreateRoot("Other").IsValid(), true);

    ASSERT_THAT(cache->RenameRoot("Root", "Other"), ERROR);
    EXPECT_THAT(cache->DoesRootExist("Root"), true);
    EXPECT_THAT(cache->DoesRootExist("Other"), true);
    }

TEST_F(DataSourceCacheTests, RenameRoot_ExistingRootWithInstanceLinked_RenamedRootHasInstanceLinked)
    {
    auto cache = GetTestCache();

    ASSERT_THAT(cache->LinkInstanceToRoot("Root", {"TestSchema.TestClass", "Foo"}), SUCCESS);

    ASSERT_THAT(cache->RenameRoot("Root", "Other"), SUCCESS);
    EXPECT_THAT(cache->IsInstanceInRoot("Other", cache->FindInstance({"TestSchema.TestClass", "Foo"})), true);
    EXPECT_THAT(cache->IsInstanceInRoot("Root", cache->FindInstance({"TestSchema.TestClass", "Foo"})), false);
    }

TEST_F(DataSourceCacheTests, UpdateInstances_WSResultNotModified_ReturnsError)
    {
    auto cache = GetTestCache();

    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(ERROR, cache->UpdateInstances(StubWSObjectsResponseNotModified()));
    BeTest::SetFailOnAssert(true);
    }

TEST_F(DataSourceCacheTests, UpdateInstances_NotExistingInstance_InstanceInsertedIntoRejected)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    bset<ObjectId> notFound;
    notFound.insert({"TestSchema.TestClass", "Test"});
    EXPECT_EQ(SUCCESS, cache->UpdateInstances(instances.ToWSObjectsResponse(), &notFound));

    EXPECT_EQ(2, notFound.size());
    EXPECT_CONTAINS(notFound, ObjectId("TestSchema.TestClass", "Foo"));
    EXPECT_CONTAINS(notFound, ObjectId("TestSchema.TestClass", "Test"));
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, UpdateInstances_ExistingInstance_Caches)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"}));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    bset<ObjectId> notFound;
    EXPECT_EQ(SUCCESS, cache->UpdateInstances(instances.ToWSObjectsResponse(), &notFound));

    EXPECT_EQ(0, notFound.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, UpdateInstances_ExistingInstance_InsertsIntoCachedInstances)
    {
    auto cache = GetTestCache();

    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    bset<ECInstanceKey> cachedInstances;
    cachedInstances.insert(StubECInstanceKey(0, 42));
    EXPECT_EQ(SUCCESS, cache->UpdateInstances(instances.ToWSObjectsResponse(), nullptr, &cachedInstances));

    EXPECT_EQ(2, cachedInstances.size());
    EXPECT_CONTAINS(cachedInstances, instanceKey);
    EXPECT_CONTAINS(cachedInstances, StubECInstanceKey(0, 42));
    }

TEST_F(DataSourceCacheTests, CacheInstancesAndLinkToRoot_NewInstnaces_ReturnsCachedInstanceKeys)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass2", "B"});

    ECInstanceKeyMultiMap instanceKeys;
    instanceKeys.insert({ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(1234))});
    ASSERT_EQ(SUCCESS, cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "root", &instanceKeys));

    EXPECT_THAT(instanceKeys, SizeIs(3));
    EXPECT_THAT(instanceKeys, Contains(ECDbHelper::ToPair(ECInstanceKey(ECClassId(UINT64_C(1)), ECInstanceId(UINT64_C(1234))))));
    EXPECT_THAT(instanceKeys, Contains(ECDbHelper::ToPair(cache->FindInstance({"TestSchema.TestClass", "A"}))));
    EXPECT_THAT(instanceKeys, Contains(ECDbHelper::ToPair(cache->FindInstance({"TestSchema.TestClass2", "B"}))));
    }

TEST_F(DataSourceCacheTests, ReadInstance_NavigationBaseObjectId_ErrorAndNullJson)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, ObjectId()));

    Json::Value instance(Json::objectValue);
    EXPECT_THAT(cache->ReadInstance(ObjectId(), instance), CacheStatus::Error);
    EXPECT_THAT(instance.isNull(), true);
    }

TEST_F(DataSourceCacheTests, ReadInstance_NotExistingObjectId_ErrorAndNullJson)
    {
    auto cache = GetTestCache();

    Json::Value instance(Json::objectValue);
    EXPECT_THAT(cache->ReadInstance(ObjectId("TestSchema.TestClass", "NonExisting"), instance), CacheStatus::DataNotCached);
    EXPECT_THAT(instance.isNull(), true);
    }

TEST_F(DataSourceCacheTests, ReadInstance_ExistingObjectId_ReturnsJsonInstanceWithRemoteIdAndValues)
    {
    auto cache = GetTestCache();
    ObjectId id("TestSchema.TestClass", "TestId");

    StubInstances instances;
    instances.Add(id, {{"TestProperty", "TestValue"}});
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot(id, instances.ToWSObjectsResponse(), nullptr));

    Json::Value instance(Json::objectValue);
    EXPECT_THAT(cache->ReadInstance(id, instance), CacheStatus::OK);
    EXPECT_THAT(instance.isNull(), false);
    EXPECT_THAT(instance[DataSourceCache_PROPERTY_RemoteId].asString(), Eq("TestId"));
    EXPECT_THAT(instance["TestProperty"].asString(), Eq("TestValue"));
    }

TEST_F(DataSourceCacheTests, ReadInstance_NavigationBaseObjectId_ReturnsNull)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, ObjectId()));

    auto instance = cache->ReadInstance(ObjectId());
    EXPECT_THAT(instance.IsNull(), true);
    }

TEST_F(DataSourceCacheTests, ReadInstance_NotExistingObjectId_ReturnsNull)
    {
    auto cache = GetTestCache();

    auto instance = cache->ReadInstance(ObjectId("TestSchema.TestClass", "NonExisting"));
    EXPECT_THAT(instance.IsNull(), true);
    }

TEST_F(DataSourceCacheTests, ReadInstance_ExistingObjectId_ReturnsECInstanceWithRemoteIdAndValues)
    {
    auto cache = GetTestCache();
    ObjectId id("TestSchema.TestClass", "TestId");

    StubInstances instances;
    instances.Add(id, {{"TestProperty", "TestValue"}});
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot(id, instances.ToWSObjectsResponse(), nullptr));

    auto instance = cache->ReadInstance(id);
    EXPECT_THAT(instance.IsNull(), false);
    EXPECT_THAT(instance->GetInstanceId(), Eq("TestId"));
    ECValue value;
    instance->GetValue(value, "TestProperty");
    EXPECT_THAT(value.GetUtf8CP(), StrEq("TestValue"));
    }

TEST_F(DataSourceCacheTests, ReadInstance_NavigationBaseInstanceKey_ReturnsNull)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, ObjectId()));

    auto instanceKey = cache->FindInstance(ObjectId());
    ASSERT_TRUE(instanceKey.IsValid());

    auto instance = cache->ReadInstance(instanceKey);
    EXPECT_THAT(instance.IsNull(), true);
    }

TEST_F(DataSourceCacheTests, ReadInstance_ExistingInstanceKey_ReturnsECInstanceWithRemoteIdAndValues)
    {
    auto cache = GetTestCache();
    ObjectId id("TestSchema.TestClass", "TestId");

    StubInstances instances;
    instances.Add(id, {{"TestProperty", "TestValue"}});
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot(id, instances.ToWSObjectsResponse(), nullptr));

    auto instanceKey = cache->FindInstance(id);
    ASSERT_TRUE(instanceKey.IsValid());

    auto instance = cache->ReadInstance(instanceKey);
    EXPECT_THAT(instance.IsNull(), false);
    EXPECT_THAT(instance->GetInstanceId(), Eq("TestId"));
    ECValue value;
    instance->GetValue(value, "TestProperty");
    EXPECT_THAT(value.GetUtf8CP(), StrEq("TestValue"));
    }

TEST_F(DataSourceCacheTests, ReadInstances_ExistingObjectIds_ReturnsInstances)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    instances.Add({"TestSchema.TestClass2", "Boo"});
    ASSERT_EQ(SUCCESS, cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "root"));

    bset<ObjectId> objectIds;
    objectIds.insert({"TestSchema.TestClass", "Foo"});
    objectIds.insert({"TestSchema.TestClass2", "Boo"});

    Json::Value instancesArray;
    ASSERT_EQ(SUCCESS, cache->ReadInstances(objectIds, instancesArray));

    EXPECT_EQ(2, instancesArray.size());
    }

TEST_F(DataSourceCacheTests, CacheResponse_NonExistingParent_ReturnsError)
    {
    auto cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "NonExisting"}), nullptr);
    EXPECT_TRUE(responseKey.GetParent().GetClassId().IsValid());

    EXPECT_EQ(CacheStatus::DataNotCached, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));
    }

TEST_F(DataSourceCacheTests, CacheResponse_CachedInstanceAsParentInCache_Succeeds)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache);
    CachedResponseKey responseKey(instance, nullptr);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));
    EXPECT_TRUE(cache->IsResponseCached(responseKey));
    }

TEST_F(DataSourceCacheTests, CacheResponse_RootAsParentInCache_Succeeds)
    {
    auto cache = GetTestCache();
    auto root = cache->FindOrCreateRoot("Foo");
    CachedResponseKey responseKey(root, nullptr);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));
    EXPECT_TRUE(cache->IsResponseCached(responseKey));
    }

TEST_F(DataSourceCacheTests, CacheResponse_NavigationBaseAsParentInCache_Succeeds)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, ObjectId()));
    auto navigationBase = cache->FindInstance(ObjectId());
    CachedResponseKey responseKey(navigationBase, nullptr);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));
    EXPECT_TRUE(cache->IsResponseCached(responseKey));
    }

TEST_F(DataSourceCacheTests, CacheResponse_MultipleResponsesForSameParentInstance_Succeeds)
    {
    auto cache = GetTestCache();
    ECInstanceKey parentKey = StubInstanceInCache(*cache);
    CachedResponseKey key1(parentKey, "A");
    CachedResponseKey key2(parentKey, "B");

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key2, StubInstances().ToWSObjectsResponse()));
    EXPECT_TRUE(cache->IsResponseCached(key1));
    EXPECT_TRUE(cache->IsResponseCached(key2));
    }

TEST_F(DataSourceCacheTests, CacheResponse_MultipleResponsesForSameParentRoot_Succeeds)
    {
    auto cache = GetTestCache();
    ECInstanceKey parentKey = cache->FindOrCreateRoot("Foo");
    CachedResponseKey key1(parentKey, "A");
    CachedResponseKey key2(parentKey, "B");

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key2, StubInstances().ToWSObjectsResponse()));
    EXPECT_TRUE(cache->IsResponseCached(key1));
    EXPECT_TRUE(cache->IsResponseCached(key2));
    }

TEST_F(DataSourceCacheTests, CacheResponse_MultipleResponsesForSameHolderInstance_Succeeds)
    {
    auto cache = GetTestCache();
    ECInstanceKey parentKey = cache->FindOrCreateRoot("Foo");
    ECInstanceKey holderKey = StubInstanceInCache(*cache);
    CachedResponseKey key1(parentKey, "A", holderKey);
    CachedResponseKey key2(parentKey, "B", holderKey);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key2, StubInstances().ToWSObjectsResponse()));
    EXPECT_TRUE(cache->IsResponseCached(key1));
    EXPECT_TRUE(cache->IsResponseCached(key2));
    }

TEST_F(DataSourceCacheTests, CacheResponse_MultipleResponsesForSameHolderRoot_Succeeds)
    {
    auto cache = GetTestCache();
    ECInstanceKey parentKey = cache->FindOrCreateRoot("Foo");
    ECInstanceKey holderKey = cache->FindOrCreateRoot("Boo");
    CachedResponseKey key1(parentKey, "A", holderKey);
    CachedResponseKey key2(parentKey, "B", holderKey);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key2, StubInstances().ToWSObjectsResponse()));
    EXPECT_TRUE(cache->IsResponseCached(key1));
    EXPECT_TRUE(cache->IsResponseCached(key2));
    }

TEST_F(DataSourceCacheTests, CacheResponse_TwoInstancesAsServerResult_CachesFullInstances)
    {
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "TestValueA"}});
    instances.Add({"TestSchema.TestClass2", "B"}, {{"TestProperty", "TestValueB"}});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass2", "B"}).IsFullyCached());

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "A"}, instanceJson));
    EXPECT_EQ("TestValueA", instanceJson["TestProperty"].asString());
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass2", "B"}, instanceJson));
    EXPECT_EQ("TestValueB", instanceJson["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QueryWithSameNameAndParentCachedPreviusly_RemovesOldQueryResults)
    {
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QueryWithSameNameButDifferentParentsCachedPreviusly_DoesNotOverrideExistingQuery)
    {
    auto cache = GetTestCache();

    CachedResponseKey responseKey1(cache->FindOrCreateRoot("root1"), "TestQuery");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("root2"), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse()));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey2, instances.ToWSObjectsResponse()));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceRemovedInNewData_RemovesInstanceFromCache)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceRemovedInNewDataButIsInRoot_LeavesInstanceInCache)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "Foo"}));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    instances.Clear();
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceRemovedInNewResultsButIsWeaklyLinkedToRoot_RemovesInstance)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "foo_root", nullptr, true));

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    instances.Clear();
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceRemovedInNewResults_RemovesInstanceChildQueryResults)
    {
    auto cache = GetTestCache();
    CachedResponseKey baseResultsKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(baseResultsKey, instances.ToWSObjectsResponse()));

    CachedResponseKey fooResultsKey(cache->FindInstance({"TestSchema.TestClass", "Foo"}), nullptr);
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "ChildFoo"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(fooResultsKey, instances.ToWSObjectsResponse()));

    instances.Clear();
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(baseResultsKey, instances.ToWSObjectsResponse()));

    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "ChildFoo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceWithCachedFileRemovedInNewResults_RemovesCachedFile)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    ASSERT_EQ(SUCCESS, cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(), FileCache::Persistent));
    BeFileName cachedFilePath = cache->ReadFilePath({"TestSchema.TestClass", "Foo"});

    EXPECT_TRUE(cachedFilePath.DoesPathExist());

    instances.Clear();
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    ASSERT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    EXPECT_FALSE(cachedFilePath.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, CacheResponse_NewResponseDoesNotContainHoldingRelationshipChild_RemovesChildInstance)
    {
    auto cache = GetTestCache();
    auto parent = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    CachedResponseKey responseKey(parent, "Foo");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestHoldingRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_NewResponseNoLongerContainsParent_RemovesCachedInstancesExceptParent)
    {
    auto cache = GetTestCache();
    auto parent = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    CachedResponseKey responseKey(parent, "Foo");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestHoldingRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    instances.Clear();
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_NewResponseIsEmpty_RemovesCachedInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestHoldingRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    instances.Clear();
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceAddedInNewResults_AddsInstanceToCache)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_RootAsParent_InstancesNotRelatedToRootInRelationships)
    {
    auto cache = GetTestCache();
    ECInstanceKey root = cache->FindOrCreateRoot(nullptr);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse({root, ""}, instances.ToWSObjectsResponse()));

    ECInstanceKeyMultiMap seedInstances;
    seedInstances.insert(ECDbHelper::ToPair(root));
    ECInstanceKeyMultiMap relatedInstances;
    ECInstanceFinder finder(cache->GetAdapter().GetECDb());
    finder.FindInstances(relatedInstances, seedInstances, ECInstanceFinder::FindOptions(ECInstanceFinder::RelatedDirection_All, UINT8_MAX));

    ECInstanceKey instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    EXPECT_FALSE(ECDbHelper::IsInstanceInMultiMap(instance, relatedInstances));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultNotModified_LeavesPreviouslyCachedData)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubWSObjectsResponseNotModified()));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultInstanceWithRelatedInstances_CachesAllRelatedInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    auto instancesRelatedToA = instances.Add({"TestSchema.TestClass", "A"});
    instancesRelatedToA.AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    instancesRelatedToA.AddRelated({"TestSchema.TestRelationshipClass", "AC"}, {"TestSchema.TestClass", "C"})
        .AddRelated({"TestSchema.TestRelationshipClass", "CD"}, {"TestSchema.TestClass", "D"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "C"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "D"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultInstanceWithRelatedInstances_CachesRelatedInstancesWithDefinedRelationship)
    {
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultWithCyclicRelationship_CachesCyclicRelationship)
    {
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"})
        .AddRelated({"TestSchema.TestRelationshipClass", "BA"}, {"TestSchema.TestClass", "A"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, {"TestSchema.TestClass", "B"}, {"TestSchema.TestClass", "A"}));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultWithCyclicRelationship_CachesInstanceOnce)
    {
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "CachedValue"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"})
        .AddRelated({"TestSchema.TestRelationshipClass", "BA"}, {"TestSchema.TestClass", "A"}, {{"TestProperty", "IgnoredValue"}});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "A"}, instanceJson));

    EXPECT_EQ("CachedValue", instanceJson["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, CacheResponse_NewResultWithChangedRelationship_RemovesOldRelationship)
    {
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"})
        .AddRelated({"TestSchema.TestRelationshipClass", "BC"}, {"TestSchema.TestClass", "C"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AC"}, {"TestSchema.TestClass", "C"});
    instances.Add({"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema", "TestRelationshipClass");

    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "C"}));
    EXPECT_FALSE(VerifyHasRelationship(cache, testRelClass, {"TestSchema.TestClass", "B"}, {"TestSchema.TestClass", "C"}));
    }

TEST_F(DataSourceCacheTests, CacheResponse_MultipleClassRelationshipsAndNewResultHasNewIntermixedRelationships_LeavesOldRelationshipsAndAddsNew_REGRESSION)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    auto testRelClass1 = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto testRelClass2 = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass2");
    ASSERT_LT(testRelClass1->GetId(), testRelClass2->GetId()); // Ensure that ids are as expected for regression

    StubInstances instances;
    auto instance1 = instances.Add({"TestSchema.TestClass", "A"});
    instance1.AddRelated({"TestSchema.TestRelationshipClass2", "AB"}, {"TestSchema.TestClass", "B"});
    instance1.AddRelated({"TestSchema.TestRelationshipClass2", "AC"}, {"TestSchema.TestClass", "C"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    instances.Clear();
    auto instance2 = instances.Add({"TestSchema.TestClass", "A"});
    instance2.AddRelated({"TestSchema.TestRelationshipClass2", "AB"}, {"TestSchema.TestClass", "B"});
    instance2.AddRelated({"TestSchema.TestRelationshipClass", "A1"}, {"TestSchema.TestClass", "1"});
    instance2.AddRelated({"TestSchema.TestRelationshipClass2", "AC"}, {"TestSchema.TestClass", "C"});
    instance2.AddRelated({"TestSchema.TestRelationshipClass", "A2"}, {"TestSchema.TestClass", "2"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass2, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass2, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "C"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass1, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "1"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass1, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "2"}));
    }

TEST_F(DataSourceCacheTests, CacheResponse_NewResultIsEmptyWhenCachedWithRelationships_RemovesOldInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    instances.Clear();
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, CacheResponse_NewResultWithoutRelationshipButSameRelationshipCachedInOtherQuery_LeavesOldRelationship)
    {
    auto cache = GetTestCache();

    CachedResponseKey responseKey1(cache->FindOrCreateRoot(nullptr), "TestQuery1");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot(nullptr), "TestQuery2");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse()));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey2, instances.ToWSObjectsResponse()));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse()));

    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsRelationshipWithSameIds_CachesRelationships)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "SameId"}, {"TestSchema.TestClass", "B"})
        .AddRelated({"TestSchema.TestRelationshipClass", "SameId"}, {"TestSchema.TestClass", "C"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    auto instanceA = cache->FindInstance({"TestSchema.TestClass", "A"});
    auto instanceB = cache->FindInstance({"TestSchema.TestClass", "B"});
    auto instanceC = cache->FindInstance({"TestSchema.TestClass", "C"});
    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationshipAB = cache->FindRelationship(*relClass, instanceA, instanceB);
    auto relationshipBC = cache->FindRelationship(*relClass, instanceB, instanceC);

    EXPECT_NE(relationshipAB, relationshipBC);
    EXPECT_EQ(ObjectId("TestSchema.TestRelationshipClass", "SameId"), cache->FindRelationship(relationshipAB));
    EXPECT_EQ(ObjectId("TestSchema.TestRelationshipClass", "SameId"), cache->FindRelationship(relationshipBC));
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceNotCachedPreviouslyAndQueryIsNull_CachesFullInstance)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "Partial"});

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, partialInstances.ToWSObjectsResponse(), &rejected, nullptr));

    EXPECT_TRUE(rejected.empty());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Partial"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Partial"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstancePreviouslyCachedAsFullInstanceAndQueryIsNull_OverwritesFullInstance)
    {
    auto cache = GetTestCache();

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "FullyCached"}, {{"TestProperty", "OldValue"}});
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "FullyCached"}, fullInstance.ToWSObjectsResponse(), "FullRoot"));
    // Roots are by default Full persistence
    ASSERT_EQ(SUCCESS, cache->SetupRoot("FullRoot", CacheRootPersistence::Default));

    // Act
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "FullyCached"}, {{"TestProperty", "NewValue"}});

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, partialInstances.ToWSObjectsResponse(), &rejected, nullptr));

    // Assert
    Json::Value instance;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "FullyCached"}, instance));

    EXPECT_EQ("NewValue", instance["TestProperty"].asString());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "FullyCached"}).IsFullyCached());
    EXPECT_TRUE(rejected.empty());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstancePreviouslyCachedAsFullInstanceInTemporaryRoot_OverwritesItWithNew)
    {
    // Arrange
    auto cache = GetTestCache();

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "FullValue"}});

    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, fullInstance.ToWSObjectsResponse(), "TempRoot"));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("TempRoot", CacheRootPersistence::Temporary));

    // Act
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "PartialValue"}});

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, partialInstances.ToWSObjectsResponse(), &rejected, nullptr));

    // Assert
    Json::Value instance;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instance));

    EXPECT_EQ("PartialValue", instance["TestProperty"].asString());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    EXPECT_TRUE(rejected.empty());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QueryPassedButNotRejectedIsNull_Error)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema", "TestClass");

    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(CacheStatus::Error, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse(), nullptr, &query));
    BeTest::SetFailOnAssert(true);
    }

TEST_F(DataSourceCacheTests, CacheResponse_QueryHasEmptySelectToSelectAllProperties_CachesInstanceAsFull)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema", "TestClass");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsAllProperties_CachesInstanceAsFull)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsAllPropertiesWithDifferentSchemaClass_CachesInstanceAsFullAndReturnsNoRejects)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema2", "TestSchema.TestClass");
    query.SetSelect("*");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsNotAllProperties_CachesInstanceAsPartial)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("TestProperty");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsInCache());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QueryPolymorphicallySelectsNotAllProperties_CachesInstanceAsPartial)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema", "TestClass!poly");
    query.SetSelect("TestProperty");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsInCache());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsJustIdButFromEmptySchemaAndClass_CachesInstanceAspartial)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    // Used to force partial caching
    WSQuery query("", "");
    query.SetSelect("$id");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsInCache());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsIdAndSomeProperiesForFullyCachedInstance_RejectsInstance)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "FullValue"}});
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, fullInstance.ToWSObjectsResponse(), "FullRoot"));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("FullRoot", CacheRootPersistence::Full));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("$id,TestProperty");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_THAT(ToStdSet(rejected), ContainerEq(std::set<ObjectId> { {"TestSchema.TestClass", "Foo"} }));
    EXPECT_EQ("FullValue", ReadInstance(*cache, {"TestSchema.TestClass", "Foo"})["TestProperty"].asString());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsSomeProperiesAndIdForFullyCachedInstance_RejectsInstance)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "FullValue"}});
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, fullInstance.ToWSObjectsResponse(), "FullRoot"));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("FullRoot", CacheRootPersistence::Full));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("TestProperty,$id");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_THAT(ToStdSet(rejected), ContainerEq(std::set<ObjectId> { {"TestSchema.TestClass", "Foo"} }));
    EXPECT_EQ("FullValue", ReadInstance(*cache, {"TestSchema.TestClass", "Foo"})["TestProperty"].asString());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsOnlyIdForFullyCachedInstance_TreatsInstanceAsReferenceAndDoesNotRejectIt)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "FullValue"}});
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, fullInstance.ToWSObjectsResponse(), "FullRoot"));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("FullRoot", CacheRootPersistence::Full));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("$id");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_EQ("FullValue", ReadInstance(*cache, {"TestSchema.TestClass", "Foo"})["TestProperty"].asString());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QueryDSelectsForwardRelatedInstanceForDifferentSchemaWithAllProperties_CachesInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema2", "TestSchema.TestClass");
    query.SetSelect("*,TestSchema.TestRelationshipClass-forward-TestSchema.TestClass.*");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsForwardRelatedInstanceWithAllProperties_CachesInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass.*");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsBackwardRelatedInstanceWithAllProperties_CachesInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Backward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-backward-TestClass.*");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsRelatedInstancePolimorphicallyWithAllProperties_CachesFullInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass!poly.*");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsRelatedInstancePolimorphicallyWithAllPropertiesAndResultHasDerived_CachesFullInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestDerivedClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass!poly.*");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestDerivedClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsRelationshipPolymorphically_CachesFullInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass!poly-forward-TestClass.*");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsRelationshipPolymorphicallyAndResponseHasDerivedRelationshipAndNotAllProperties_CachesRelatedAsPartial)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestDerivedRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass!poly-forward-TestClass.$id");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsRelatedFullyWithDefaultRelationship_RelatedIsCachedAsPartialAsDefaultRelationshipsAreNotSupported)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestClass.*");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsRelatedPropertyOnlyAndRelatedWasCachedAsFull_RejectsRelatedInstance)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"}, {{"TestProperty", "OldValue"}});

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass.TestProperty,TestRelationshipClass-forward-TestClass.$id");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));
    EXPECT_EQ(1, rejected.size());
    EXPECT_CONTAINS(rejected, ObjectId("TestSchema.TestClass", "B"));
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "B"}, instanceJson));
    EXPECT_EQ("OldValue", instanceJson["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsIdOnlyWithRelated_SkipsIdOnlyInstanceAndCachesRelatedInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"}, {{"TestProperty", "OldValue"}});

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("$id,TestRelationshipClass-forward-TestClass.*");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());

    EXPECT_EQ("OldValue", ReadInstance(*cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsRelatedIdOnlyAndRelatedWasCachedAsFull_TreatsIdOnlyAsReferenceAndDoesNotRejectInstance)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"}, {{"TestProperty", "OldValue"}});

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass.$id");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "B"}, instanceJson));
    EXPECT_EQ("OldValue", instanceJson["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsRelationshipPolymorphicallyAndResponseHasDerivedRelationship_CachesFullInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestDerivedRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestAbstractRelationshipClass!poly-forward-TestClass.*");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsRelatedInstanceWithAllPropertiesAndAliases_CachesInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Backward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*," + query.GetAlias("TestRelationshipClass-backward-TestClass") + ".*");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsRelatedInstanceWithNotAllPropertiesForFullyCachedInstances_RejectsRelatedInstance)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances fullInstances;
    fullInstances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "FullA"}});
    fullInstances.Add({"TestSchema.TestClass", "B"}, {{"TestProperty", "FullB"}});
    CachedResponseKey fullResponseKey(cache->FindOrCreateRoot("FullRoot"), nullptr);
    ASSERT_EQ(SUCCESS, cache->SetupRoot("FullRoot", CacheRootPersistence::Full));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(fullResponseKey, fullInstances.ToWSObjectsResponse()));

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "NewA"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass.TestProperty");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_THAT(ToStdSet(rejected), ContainerEq(std::set<ObjectId> { {"TestSchema.TestClass", "B"} }));
    EXPECT_EQ("NewA", ReadInstance(*cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    EXPECT_EQ("FullB", ReadInstance(*cache, {"TestSchema.TestClass", "B"})["TestProperty"].asString());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsNestedRelatedInstanceWithNotAllPropertiesForFullyCachedInstances_RejectsRelatedInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances fullInstances;
    fullInstances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "FullA"}});
    fullInstances.Add({"TestSchema.TestClass", "B"}, {{"TestProperty", "FullB"}});
    fullInstances.Add({"TestSchema.TestClass", "C"}, {{"TestProperty", "FullC"}});
    CachedResponseKey fullResponseKey(cache->FindOrCreateRoot("FullRoot"), nullptr);
    ASSERT_EQ(SUCCESS, cache->SetupRoot("FullRoot", CacheRootPersistence::Full));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(fullResponseKey, fullInstances.ToWSObjectsResponse()));

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "NewA"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"})
        .AddRelated({"TestSchema.TestRelationshipClass", "BC"}, {"TestSchema.TestClass", "C"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass/TestRelationshipClass-forward-TestClass.TestName");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    ObjectId b("TestSchema.TestClass", "B");
    ObjectId c("TestSchema.TestClass", "C");
    EXPECT_THAT(ToStdSet(rejected), ContainerEq(std::set<ObjectId> { b, c }));

    EXPECT_EQ("NewA", ReadInstance(*cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    EXPECT_EQ("FullB", ReadInstance(*cache, {"TestSchema.TestClass", "B"})["TestProperty"].asString());
    EXPECT_EQ("FullC", ReadInstance(*cache, {"TestSchema.TestClass", "C"})["TestProperty"].asString());

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "C"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsNestedRelatedInstanceWithAllPropertiesForFullyCachedInstances_CachesInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances fullInstances;
    fullInstances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "FullA"}});
    fullInstances.Add({"TestSchema.TestClass", "B"}, {{"TestProperty", "FullB"}});
    fullInstances.Add({"TestSchema.TestClass", "C"}, {{"TestProperty", "FullC"}});
    CachedResponseKey fullResponseKey(cache->FindOrCreateRoot("FullRoot"), nullptr);
    ASSERT_EQ(SUCCESS, cache->SetupRoot("FullRoot", CacheRootPersistence::Full));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(fullResponseKey, fullInstances.ToWSObjectsResponse()));

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "NewA"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {{"TestProperty", "NewB"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "BC"}, {"TestSchema.TestClass", "C"}, {{"TestProperty", "NewC"}});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass.*,TestRelationshipClass-forward-TestClass/TestRelationshipClass-forward-TestClass.*");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    EXPECT_EQ(0, rejected.size());

    EXPECT_EQ("NewA", ReadInstance(*cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    EXPECT_EQ("NewB", ReadInstance(*cache, {"TestSchema.TestClass", "B"})["TestProperty"].asString());
    EXPECT_EQ("NewC", ReadInstance(*cache, {"TestSchema.TestClass", "C"})["TestProperty"].asString());

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "C"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QuerySelectsNotAllPropertiesForFullyCachedInstances_RejectsAllInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances fullInstances;
    fullInstances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "FullA"}});
    fullInstances.Add({"TestSchema.TestClass", "B"}, {{"TestProperty", "FullB"}});
    CachedResponseKey fullResponseKey(cache->FindOrCreateRoot("FullRoot"), nullptr);
    ASSERT_EQ(SUCCESS, cache->SetupRoot("FullRoot", CacheRootPersistence::Full));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(fullResponseKey, fullInstances.ToWSObjectsResponse()));

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("TestProperty,TestRelationshipClass-forward-TestClass.TestProperty");

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), &rejected, &query));

    ObjectId a("TestSchema.TestClass", "A");
    ObjectId b("TestSchema.TestClass", "B");
    EXPECT_THAT(ToStdSet(rejected), ContainerEq(std::set<ObjectId> { a, b }));

    EXPECT_EQ("FullA", ReadInstance(*cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    EXPECT_EQ("FullB", ReadInstance(*cache, {"TestSchema.TestClass", "B"})["TestProperty"].asString());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_CacheTemporaryResponsesWithFullAndPartialInstance_InvalidatesFullResponsesWhenOverridenWithPartialData)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->SetupRoot(nullptr, CacheRootPersistence::Temporary));

    // Arrange
    auto partialResponseKey = StubCachedResponseKey(*cache, "Partial");
    auto fullResponseKey = StubCachedResponseKey(*cache, "Full");

    WSQuery partialQuery("TestSchema", "TestClass");
    partialQuery.SetSelect("TestProperty");

    WSQuery fullQuery("TestSchema", "TestClass");
    fullQuery.SetSelect("*");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Full"}});

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(partialResponseKey, instances.ToWSObjectsResponse("TagA"), &rejected, &partialQuery));
    ASSERT_THAT(rejected, IsEmpty());
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(fullResponseKey, instances.ToWSObjectsResponse("TagB"), &rejected, &fullQuery));
    ASSERT_THAT(rejected, IsEmpty());

    ASSERT_TRUE(cache->IsResponseCached(fullResponseKey));
    ASSERT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());

    // Act
    auto newResponseKey = StubCachedResponseKey(*cache, "New");
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Partial"}});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(newResponseKey, instances.ToWSObjectsResponse("TagC"), &rejected, &partialQuery));
    ASSERT_THAT(rejected, IsEmpty());
    ASSERT_EQ("Partial", ReadInstance(*cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    ASSERT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());

    // Assert
    EXPECT_TRUE(cache->IsResponseCached(newResponseKey));
    EXPECT_EQ("TagC", cache->ReadResponseCacheTag(newResponseKey));

    EXPECT_TRUE(cache->IsResponseCached(partialResponseKey));
    EXPECT_EQ("TagA", cache->ReadResponseCacheTag(partialResponseKey));

    EXPECT_TRUE(cache->IsResponseCached(fullResponseKey));
    EXPECT_EQ("", cache->ReadResponseCacheTag(fullResponseKey));
    }

TEST_F(DataSourceCacheTests, CacheResponse_CacheTemporaryResponsesWithFullAndPartialInstanceAsParent_InvalidatesFullResponsesWhenOverridenWithPartialData)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->SetupRoot(nullptr, CacheRootPersistence::Temporary));
    auto parent = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});

    // Arrange
    auto partialResponseKey = CachedResponseKey(parent, "Partial");
    auto fullResponseKey = CachedResponseKey(parent, "Full");

    WSQuery partialQuery("TestSchema", "TestClass");
    partialQuery.SetSelect("TestProperty");

    WSQuery fullQuery("TestSchema", "TestClass");
    fullQuery.SetSelect("*");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Full"}});

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(partialResponseKey, instances.ToWSObjectsResponse("TagA"), &rejected, &partialQuery));
    ASSERT_THAT(rejected, IsEmpty());
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(fullResponseKey, instances.ToWSObjectsResponse("TagB"), &rejected, &fullQuery));
    ASSERT_THAT(rejected, IsEmpty());

    ASSERT_EQ("TagA", cache->ReadResponseCacheTag(partialResponseKey));
    ASSERT_EQ("TagB", cache->ReadResponseCacheTag(fullResponseKey));
    ASSERT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());

    // Act
    auto newResponseKey = StubCachedResponseKey(*cache, "New");
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Partial"}});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(newResponseKey, instances.ToWSObjectsResponse("TagC"), &rejected, &partialQuery));
    ASSERT_THAT(rejected, IsEmpty());
    ASSERT_EQ("Partial", ReadInstance(*cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    ASSERT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());

    // Assert
    EXPECT_TRUE(cache->IsResponseCached(newResponseKey));
    EXPECT_EQ("TagC", cache->ReadResponseCacheTag(newResponseKey));

    EXPECT_TRUE(cache->IsResponseCached(partialResponseKey));
    EXPECT_EQ("TagA", cache->ReadResponseCacheTag(partialResponseKey));

    EXPECT_TRUE(cache->IsResponseCached(fullResponseKey));
    EXPECT_EQ("", cache->ReadResponseCacheTag(fullResponseKey));
    }

TEST_F(DataSourceCacheTests, CacheResponse_CacheTemporaryResponseWithFullAndThenPartial_DoesNotInvalidateButOverridesItselfWithPartialData)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->SetupRoot(nullptr, CacheRootPersistence::Temporary));
    auto responseKey = StubCachedResponseKey(*cache);

    // Arrange
    WSQuery fullQuery("TestSchema", "TestClass");
    fullQuery.SetSelect("*");
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Full"}});

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse("TagA"), &rejected, &fullQuery));
    ASSERT_THAT(rejected, IsEmpty());

    // Act
    WSQuery partialQuery("TestSchema", "TestClass");
    partialQuery.SetSelect("TestProperty");
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Partial"}});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse("TagB"), &rejected, &partialQuery));
    ASSERT_THAT(rejected, IsEmpty());

    ASSERT_TRUE(cache->IsResponseCached(responseKey));
    ASSERT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    ASSERT_EQ("Partial", ReadInstance(*cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    EXPECT_EQ("TagB", cache->ReadResponseCacheTag(responseKey));
    }

TEST_F(DataSourceCacheTests, CacheResponse_KeyHasNoHolder_ParentHasHoldingRelationshipToResults)
    {
    auto cache = GetTestCache();
    CachedResponseKey key(cache->FindOrCreateRoot("Parent"), "TestQuery", ECInstanceKey());

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    ECInstanceKey instanceKey = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    ECInstanceKeyMultiMap parentInstances;
    ASSERT_EQ(SUCCESS, cache->ReadInstancesConnectedToRootMap("Parent", parentInstances));

    EXPECT_THAT(ECDbHelper::IsInstanceInMultiMap(instanceKey, parentInstances), true);
    }

TEST_F(DataSourceCacheTests, CacheResponse_KeyHasDifferentHolder_ParentDoesNotHaveHoldingRelationshipToResultsButHolderDoes)
    {
    auto cache = GetTestCache();
    CachedResponseKey key(cache->FindOrCreateRoot("Parent"), "TestQuery", cache->FindOrCreateRoot("Holder"));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    ECInstanceKey instanceKey = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    ECInstanceKeyMultiMap parentInstances, holderInstances;
    ASSERT_EQ(SUCCESS, cache->ReadInstancesConnectedToRootMap("Parent", parentInstances));
    ASSERT_EQ(SUCCESS, cache->ReadInstancesConnectedToRootMap("Holder", holderInstances));

    EXPECT_THAT(ECDbHelper::IsInstanceInMultiMap(instanceKey, parentInstances), false);
    EXPECT_THAT(ECDbHelper::IsInstanceInMultiMap(instanceKey, holderInstances), true);
    }

TEST_F(DataSourceCacheTests, CacheResponse_KeyHasDifferentHolderAndThenParentIsRemoved_RemovesResults)
    {
    auto cache = GetTestCache();
    CachedResponseKey key(cache->FindOrCreateRoot("Parent"), "TestQuery", cache->FindOrCreateRoot("Holder"));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    ASSERT_EQ(SUCCESS, cache->RemoveRoot("Parent"));
    EXPECT_FALSE(cache->IsResponseCached(key));
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResponseContainsItsParentInstanceAndParentRootRemoved_QueryResultsAndParentDeleted)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", {"TestSchema.TestClass", "Parent"}));
    auto parent = cache->FindInstance({"TestSchema.TestClass", "Parent"});

    CachedResponseKey key(parent, "Foo");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Parent"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    ASSERT_EQ(SUCCESS, cache->RemoveRoot("Root"));
    EXPECT_FALSE(cache->IsResponseCached(key));
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Parent"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, CacheResponse_ParentIsRemoved_NewResponseIsNotCached)
    {
    auto cache = GetTestCache();
    auto parent = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    CachedResponseKey responseKey(parent, "Foo");

    ASSERT_EQ(CacheStatus::OK, cache->RemoveInstance({"TestSchema.TestClass", "A"}));
    ASSERT_EQ(CacheStatus::DataNotCached, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResponseContainsItsHolderInstanceAndHolderRootIsRemoved_QueryResultsAndHolderDeleted)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("RootParent", {"TestSchema.TestClass", "Parent"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("RootHolder", {"TestSchema.TestClass", "Holder"}));
    auto parent = cache->FindInstance({"TestSchema.TestClass", "Parent"});
    auto holder = cache->FindInstance({"TestSchema.TestClass", "Holder"});

    CachedResponseKey key(parent, "Foo", holder);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Holder"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    ASSERT_EQ(SUCCESS, cache->RemoveRoot("RootHolder"));
    EXPECT_FALSE(cache->IsResponseCached(key));
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Holder"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, CacheResponse_MultipleNestedResponsesWithHolderAndHolderIsRemoved_RemovesResults)
    {
    auto cache = GetTestCache();

    StubInstances instances1;
    instances1.Add({"TestSchema.TestClass", "A"});
    CachedResponseKey key1(cache->FindOrCreateRoot("Parent"), "TestQuery", cache->FindOrCreateRoot("Holder"));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, instances1.ToWSObjectsResponse()));

    StubInstances instances2;
    instances2.Add({"TestSchema.TestClass", "B"});
    CachedResponseKey key2(cache->FindInstance({"TestSchema.TestClass", "A"}), "TestQuery", cache->FindOrCreateRoot("Holder"));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key2, instances2.ToWSObjectsResponse()));

    StubInstances instances3;
    instances3.Add({"TestSchema.TestClass", "C"});
    CachedResponseKey key3(cache->FindInstance({"TestSchema.TestClass", "B"}), "TestQuery", cache->FindOrCreateRoot("Holder"));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key3, instances3.ToWSObjectsResponse()));

    ASSERT_EQ(SUCCESS, cache->RemoveRoot("Parent"));
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), false);
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid(), false);
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "C"}).IsValid(), false);
    EXPECT_THAT(cache->IsResponseCached(key1), false);
    EXPECT_THAT(cache->IsResponseCached(key2), false);
    EXPECT_THAT(cache->IsResponseCached(key3), false);
    }

TEST_F(DataSourceCacheTests, CacheResponse_RelationshipWithProperties_CachesRelationshipProperties)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipPropertiesClass", "AB"}, {"TestSchema.TestClass", "B"}, {},
        ECRelatedInstanceDirection::Forward, {{"TestProperty", "RelationshipValue"}});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipPropertiesClass");
    auto relationshipKey = cache->FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"});

    EXPECT_TRUE(relationshipKey.IsValid());
    EXPECT_THAT(cache->FindRelationship(relationshipKey), ObjectId("TestSchema.TestRelationshipPropertiesClass", "AB"));

    Json::Value relationshipJson;
    ASSERT_EQ(SUCCESS, cache->GetAdapter().GetJsonInstance(relationshipJson, relationshipKey));
    EXPECT_NE(Json::Value::GetNull(), relationshipJson);
    EXPECT_EQ("RelationshipValue", relationshipJson["TestProperty"].asString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsOneToOneRelationshipsViolatingSourceCardinality_Error)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    auto instance = instances.Add({"TestSchema.TestClassA", "A"});
    instance.AddRelated({"TestSchema.TestOneToOneRelationshipClass", ""}, {"TestSchema.TestClassB", "B"});
    instance.AddRelated({"TestSchema.TestOneToOneRelationshipClass", ""}, {"TestSchema.TestClassB", "C"}); // Second related instance should not be allowed

    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(CacheStatus::Error, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));
    BeTest::SetFailOnAssert(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                julius.cepukenas                     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsOneToOneRelationshipsViolatingTargetCardinality_Error)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    auto instance = instances.Add({"TestSchema.TestClassA", "A"});
    instance.AddRelated({"TestSchema.TestOneToOneRelationshipClass", ""}, {"TestSchema.TestClassB", "B"});
    instances.Add({"TestSchema.TestClassA", "C"})
        .AddRelated({"TestSchema.TestOneToOneRelationshipClass", ""}, {"TestSchema.TestClassB", "B"});

    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(CacheStatus::Error, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));
    BeTest::SetFailOnAssert(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsChangedOneToOneRelationship_ChangesRelationshipWithoutErrors)
    {
    // Arrange
    auto cache = GetTestCache();
    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestOneToOneRelationshipClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClassA", "A"})
        .AddRelated({"TestSchema.TestOneToOneRelationshipClass", ""}, {"TestSchema.TestClassB", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"}));
    // Act
    instances.Clear();
    instances.Add({"TestSchema.TestClassA", "A"})
        .AddRelated({"TestSchema.TestOneToOneRelationshipClass", ""}, {"TestSchema.TestClassB", "C"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));
    // Assert
    EXPECT_FALSE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "C"}));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                julius.cepukenas                     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsChangedOneToOneRelationshipWithUnusedInstances_RemovesUnusedInstancesFromCashe)
    {
    auto cache = GetTestCache();
    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestOneToOneRelationshipClass");
    //Prepare cache with A -> B and C -> D instances and relathionships
    StubInstances instances;
    instances.Add({"TestSchema.TestClassA", "A"})
        .AddRelated({"TestSchema.TestOneToOneRelationshipClass", ""}, {"TestSchema.TestClassB", "B"});
    instances.Add({"TestSchema.TestClassB", "C"})
        .AddRelated({"TestSchema.TestOneToOneRelationshipClass", ""}, {"TestSchema.TestClassA", "D"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));
    ASSERT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"}));
    ASSERT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassB", "C"}, {"TestSchema.TestClassA", "D"}));

    //Update cache with A -> D relathionship thus removing B C and their relathionships from server
    instances.Clear();
    instances.Add({"TestSchema.TestClassA", "A"})
        .AddRelated({"TestSchema.TestOneToOneRelationshipClass", ""}, {"TestSchema.TestClassA", "D"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    EXPECT_FALSE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"}));
    EXPECT_FALSE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassB", "C"}, {"TestSchema.TestClassA", "D"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassA", "D"}));
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClassB", "C"}).IsInCache());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClassB", "B"}).IsInCache());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                julius.cepukenas                     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsChangedOneToOneRelationship_DoesNotLooseChildrenData)
    {
    auto cache = GetTestCache();
    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestOneToOneRelationshipClass");
    //Prepare cache with A -> B and C -> D instances and relathionships
    StubInstances instances;
    instances.Add({"TestSchema.TestClassA", "A"})
        .AddRelated({"TestSchema.TestOneToOneRelationshipClass", ""}, {"TestSchema.TestClassB", "B"});
    instances.Add({"TestSchema.TestClassB", "C"})
        .AddRelated({"TestSchema.TestOneToOneRelationshipClass", ""}, {"TestSchema.TestClassA", "D"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));
    ASSERT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"}));
    ASSERT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassB", "C"}, {"TestSchema.TestClassA", "D"}));
    //Change the children relathionships with each other, and dont loose any data
    instances.Clear();
    instances.Add({"TestSchema.TestClassA", "A"})
        .AddRelated({"TestSchema.TestOneToOneRelationshipClass", ""}, {"TestSchema.TestClassA", "D"});
    instances.Add({"TestSchema.TestClassB", "C"})
        .AddRelated({"TestSchema.TestOneToOneRelationshipClass", ""}, {"TestSchema.TestClassB", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    EXPECT_FALSE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"}));
    EXPECT_FALSE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassB", "C"}, {"TestSchema.TestClassA", "D"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassA", "D"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassB", "C"}, {"TestSchema.TestClassB", "B"}));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                julius.cepukenas                     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsChangedOneToOneHoldingRelationship_DoesNotLooseChildrenData)
    {
    auto cache = GetTestCache();
    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestOneToOneHoldingRelationshipClass");
    //Prepare the cache with A -> B and C instances and holding relathionships
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestOneToOneHoldingRelationshipClass", ""}, {"TestSchema.TestClass", "B"});
    instances.Add({"TestSchema.TestClass", "C"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));
    ASSERT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    //Change parent instance of the relathionship, do not lose the data ( A and C -> B )
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "C"})
        .AddRelated({"TestSchema.TestOneToOneHoldingRelationshipClass", ""}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    EXPECT_FALSE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClass", "C"}, {"TestSchema.TestClass", "B"}));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                julius.cepukenas                     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheTests, CacheResponse_MultipleResultContainsChangedOneToOneHoldingRelationship_DoesNotLooseChildrenData3)
    {
    auto cache = GetTestCache();
    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestOneToOneHoldingRelationshipClass");
    auto parent = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    CachedResponseKey responseKey(parent, "Foo");
    auto child = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    CachedResponseKey responseKeyChild(parent, "Foo2");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestOneToOneHoldingRelationshipClass", ""}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));
    ASSERT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    //Swap relathionship of not a root response. Do not lose any data
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"})
        .AddRelated({"TestSchema.TestOneToOneHoldingRelationshipClass", ""}, {"TestSchema.TestClass", "C"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKeyChild, instances.ToWSObjectsResponse()));

    instances.Add({"TestSchema.TestClass", "D"})
        .AddRelated({"TestSchema.TestOneToOneHoldingRelationshipClass", ""}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    EXPECT_FALSE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClass", "D"}, {"TestSchema.TestClass", "B"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClass", "B"}, {"TestSchema.TestClass", "C"}));
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "C"}).IsFullyCached());
    }

/*---------------------------------------------------------------------------------**//**
+* @bsimethod                                julius.cepukenas                     09/17
++---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsChangedOneToManyRelationship_ChangesRelationshipWithoutErrors)
    {
    auto cache = GetTestCache();
    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestOneToManyRelationshipClass");

    // Prepare cache with A -> B, D -> C and D instnaces
    StubInstances instances;
    instances.Add({"TestSchema.TestClassA", "A"})
        .AddRelated({"TestSchema.TestOneToManyRelationshipClass", ""}, {"TestSchema.TestClassB", "B"});
    instances.Add({"TestSchema.TestClassA", "D"})
        .AddRelated({"TestSchema.TestOneToManyRelationshipClass", ""}, {"TestSchema.TestClassB", "C"});
    instances.Add({"TestSchema.TestClassA", "D"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "D"}, {"TestSchema.TestClassB", "C"}));

    // Update relathionship of A -> B to D -> B. Relathionship can have many sources
    instances.Clear();
    instances.Add({"TestSchema.TestClassA", "D"})
        .AddRelated({"TestSchema.TestOneToManyRelationshipClass", ""}, {"TestSchema.TestClassB", "B"});
    instances.Add({"TestSchema.TestClassA", "D"})
        .AddRelated({"TestSchema.TestOneToManyRelationshipClass", ""}, {"TestSchema.TestClassB", "C"});
    instances.Add({"TestSchema.TestClassA", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    EXPECT_FALSE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "D"}, {"TestSchema.TestClassB", "B"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "D"}, {"TestSchema.TestClassB", "C"}));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                julius.cepukenas                     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsChangedManyToOneRelationship_ChangesRelationshipWithoutErrors)
    {
    auto cache = GetTestCache();
    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestManyToOneRelationshipClass");

    //Prepare cache with instnaces and relathionships
    StubInstances instances;
    instances.Add({"TestSchema.TestClassA", "A"})
        .AddRelated({"TestSchema.TestManyToOneRelationshipClass", ""}, {"TestSchema.TestClassB", "B"});
    instances.Add({"TestSchema.TestClassA", "C"})
        .AddRelated({"TestSchema.TestManyToOneRelationshipClass", ""}, {"TestSchema.TestClassB", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "C"}, {"TestSchema.TestClassB", "B"}));

    //Update relathionships. Relathionship can have one source many targets
    instances.Clear();
    instances.Add({"TestSchema.TestClassA", "A"})
        .AddRelated({"TestSchema.TestManyToOneRelationshipClass", ""}, {"TestSchema.TestClassB", "D"});
    instances.Add({"TestSchema.TestClassA", "C"})
        .AddRelated({"TestSchema.TestManyToOneRelationshipClass", ""}, {"TestSchema.TestClassB", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    EXPECT_FALSE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "D"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, {"TestSchema.TestClassA", "C"}, {"TestSchema.TestClassB", "B"}));
    }

TEST_F(DataSourceCacheTests, CacheResponse_KeysHaveSameHolderAndNameAndParent_NewResponseOverridesOldOne)
    {
    auto cache = GetTestCache();
    CachedResponseKey key(cache->FindOrCreateRoot("Parent"), "TestQuery", cache->FindOrCreateRoot("Holder"));

    StubInstances oldInstances;
    oldInstances.Add({"TestSchema.TestClass", "A"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, oldInstances.ToWSObjectsResponse()));
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), true);

    StubInstances newInstances;
    newInstances.Add({"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, newInstances.ToWSObjectsResponse()));
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), false);
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid(), true);
    }

TEST_F(DataSourceCacheTests, CacheResponse_KeysHaveSameParentAndSameNameButDifferentHolders_NewResponseOverridesOldOne)
    {
    auto cache = GetTestCache();
    CachedResponseKey key1(cache->FindOrCreateRoot("Parent"), "TestQuery", cache->FindOrCreateRoot("Holder1"));
    CachedResponseKey key2(cache->FindOrCreateRoot("Parent"), "TestQuery", cache->FindOrCreateRoot("Holder2"));

    StubInstances oldInstances;
    oldInstances.Add({"TestSchema.TestClass", "A"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, oldInstances.ToWSObjectsResponse()));
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), true);

    StubInstances newInstances;
    newInstances.Add({"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key2, newInstances.ToWSObjectsResponse()));
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), false);
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid(), true);
    }

TEST_F(DataSourceCacheTests, CacheResponse_KeysHaveSameHolderAndNameButDifferentParents_DoesNotOverrideEachOther)
    {
    auto cache = GetTestCache();
    CachedResponseKey key1(cache->FindOrCreateRoot("Parent1"), "TestQuery", cache->FindOrCreateRoot("Holder"));
    CachedResponseKey key2(cache->FindOrCreateRoot("Parent2"), "TestQuery", cache->FindOrCreateRoot("Holder"));

    StubInstances oldInstances;
    oldInstances.Add({"TestSchema.TestClass", "A"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, oldInstances.ToWSObjectsResponse()));
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), true);

    StubInstances newInstances;
    newInstances.Add({"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key2, newInstances.ToWSObjectsResponse()));
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), true);
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid(), true);
    }

TEST_F(DataSourceCacheTests, CacheResponse_NotCancelledCancellatioTokenPassed_CachesAndReturnsSuccess)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);
    auto token = SimpleCancellationToken::Create(false);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), nullptr, nullptr, 0, token));
    EXPECT_THAT(cache->IsResponseCached(responseKey), true);
    }

TEST_F(DataSourceCacheTests, CacheResponse_CancelledCancellatioTokenPassed_ReturnsError)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);
    auto token = SimpleCancellationToken::Create(true);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    EXPECT_EQ(CacheStatus::Error, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse(), nullptr, nullptr, 0, token));
    }

TEST_F(DataSourceCacheTests, CacheResponse_DifferentPages_Cached)
    {
    // Arrange
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    // Act
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 1));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "C"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 42));

    // Assert
    ECInstanceKeyMultiMap instanceKeys;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(key, instanceKeys));

    EXPECT_EQ(3, instanceKeys.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "A"}), instanceKeys));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "B"}), instanceKeys));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "C"}), instanceKeys));

    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "C"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_SamePage_Overrides)
    {
    // Arrange
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 42));

    // Act
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 42));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "C"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 123456));

    // Assert
    ECInstanceKeyMultiMap instanceKeys;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(key, instanceKeys));

    EXPECT_EQ(2, instanceKeys.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "B"}), instanceKeys));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "C"}), instanceKeys));

    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "C"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_CachedPagesAndFinalResponseOnFirstPage_OverridesAllPreviousPages)
    {
    // Arrange
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 42));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));

    // Act
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "C"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    // Assert
    ECInstanceKeyMultiMap instanceKeys;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(key, instanceKeys));

    EXPECT_EQ(1, instanceKeys.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "C"}), instanceKeys));

    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "C"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_FinalNotModifiedResponseAndDefaultPage_KeepsOnlyFirstPageAndCompletesResponse)
    {
    // Arrange
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 1));

    // Act
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubWSObjectsResponseNotModified()));

    // Assert
    EXPECT_TRUE(cache->IsResponseCached(key));

    ECInstanceKeyMultiMap instanceKeys;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(key, instanceKeys));

    EXPECT_EQ(1, instanceKeys.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "A"}), instanceKeys));

    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_FinalNotModifiedResponseAndOnLastPage_SetsAsCached)
    {
    // Arrange
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    EXPECT_FALSE(cache->IsResponseCached(key));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", ""), nullptr, nullptr, 1));
    EXPECT_TRUE(cache->IsResponseCached(key));

    // Act
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "C"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    EXPECT_FALSE(cache->IsResponseCached(key));

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubWSObjectsResponseNotModified(), nullptr, nullptr, 1));
    EXPECT_TRUE(cache->IsResponseCached(key));

    // Assert
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "C"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_NotModifiedPageAndThenModifiedPageResponses_ModifiedResponseClearsIsResponseCachedFlag)
    {
    // Arrange
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 1));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", ""), nullptr, nullptr, 42));
    EXPECT_TRUE(cache->IsResponseCached(key));

    // Act & Assert
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubWSObjectsResponseNotModified("NotFinal"), nullptr, nullptr, 0));
    EXPECT_TRUE(cache->IsResponseCached(key));

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 1));
    EXPECT_FALSE(cache->IsResponseCached(key));
    }

TEST_F(DataSourceCacheTests, CacheResponse_FinalNotModifiedPage_SetsAsIsResponseCached)
    {
    // Arrange
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 1));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", ""), nullptr, nullptr, 42));
    EXPECT_TRUE(cache->IsResponseCached(key));

    // Act & Assert
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    EXPECT_FALSE(cache->IsResponseCached(key));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 1));
    EXPECT_FALSE(cache->IsResponseCached(key));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubWSObjectsResponseNotModified(), nullptr, nullptr, 42));
    EXPECT_TRUE(cache->IsResponseCached(key));
    }

TEST_F(DataSourceCacheTests, CacheResponse_NotFinalAndNotModifiedPageAfterModifiedPage_DoesNotSetIsResponseCached)
    {
    // Arrange
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 1));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", ""), nullptr, nullptr, 42));
    EXPECT_TRUE(cache->IsResponseCached(key));

    // Act & Assert
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    EXPECT_FALSE(cache->IsResponseCached(key));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubWSObjectsResponseNotModified("NotFinal"), nullptr, nullptr, 1));
    EXPECT_FALSE(cache->IsResponseCached(key));
    }

TEST_F(DataSourceCacheTests, CacheResponse_FinalResponseAndPageIndexZero_RemovesAllPreviousPages)
    {
    // Arrange
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), nullptr, nullptr, 0));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), nullptr, nullptr, 1));

    // Act
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "C"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), nullptr, nullptr, 0));

    // Assert
    ECInstanceKeyMultiMap instanceKeys;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(key, instanceKeys));

    EXPECT_EQ(1, instanceKeys.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "C"}), instanceKeys));

    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "C"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_FinalResponseAndMultiplePages_RemovesPagesOutsideOfFinalPageIndex)
    {
    // Arrange
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 1));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "C"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 42));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "D"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 3));

    // Act
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "E"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("Final", ""), nullptr, nullptr, 1));

    // Assert
    ECInstanceKeyMultiMap instanceKeys;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(key, instanceKeys));

    EXPECT_EQ(2, instanceKeys.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "A"}), instanceKeys));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "E"}), instanceKeys));

    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "C"}).IsValid());
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "D"}).IsValid());
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "E"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_FinalResponseAndMultiplePagesWihtSpacedOutIndexes_RemovesPagesOutsideOfPageIndex)
    {
    // Arrange
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 42));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 4));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "C"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 2));

    // Act
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "E"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("Final", ""), nullptr, nullptr, 4));

    // Assert
    ECInstanceKeyMultiMap instanceKeys;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(key, instanceKeys));

    EXPECT_EQ(2, instanceKeys.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "C"}), instanceKeys));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "E"}), instanceKeys));

    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "E"}).IsValid());
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "C"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_ExistingInstanceWithReadOnlyProperty_UpdatesPropertyValueForSameInstanceInCache)
    {
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass3", "Foo"}, {{"TestReadOnlyProperty", "OldValue"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));
    ECInstanceKey instance1 = cache->FindInstance({"TestSchema.TestClass3", "Foo"});

    instances.Clear();
    instances.Add({"TestSchema.TestClass3", "Foo"}, {{"TestReadOnlyProperty", "NewValue"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));
    ECInstanceKey instance2 = cache->FindInstance({"TestSchema.TestClass3", "Foo"});

    EXPECT_EQ(instance1, instance2);
    Json::Value properties = ReadInstance(*cache, instance2);
    EXPECT_EQ("NewValue", properties["TestReadOnlyProperty"]);
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceWithCalculatedProperty_CachesAndUpdatesCalculatedPropertyValue)
    {
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass4", "Foo"}, {{"TestProperty", "OldValue"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    Json::Value properties = ReadInstance(*cache, cache->FindInstance({"TestSchema.TestClass4", "Foo"}));
    EXPECT_EQ("OldValue", properties["TestCalculatedProperty"]);

    instances.Clear();
    instances.Add({"TestSchema.TestClass4", "Foo"}, {{"TestProperty", "NewValue"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    properties = ReadInstance(*cache, cache->FindInstance({"TestSchema.TestClass4", "Foo"}));
    EXPECT_EQ("NewValue", properties["TestCalculatedProperty"]);
    }

TEST_F(DataSourceCacheTests, IsResponseCached_ParentDoesNotExist_False)
    {
    auto cache = GetTestCache();
    ECInstanceKey parentKey(cache->GetAdapter().GetECClass("TestSchema.TestClass")->GetId(), ECInstanceId());
    CachedResponseKey responseKey(parentKey, "DoesNotExist");
    EXPECT_FALSE(cache->IsResponseCached(responseKey));
    }

TEST_F(DataSourceCacheTests, IsResponseCached_ResponseNotCachedButParentExists_False)
    {
    auto cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "DoesNotExist");
    EXPECT_FALSE(cache->IsResponseCached(responseKey));
    }

TEST_F(DataSourceCacheTests, IsResponseCached_ResponseCached_True)
    {
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse()));
    EXPECT_TRUE(cache->IsResponseCached(key));
    }

TEST_F(DataSourceCacheTests, IsResponseCached_ResponseWithHoldererCached_True)
    {
    auto cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "Foo", cache->FindOrCreateRoot("Holder"));

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));
    EXPECT_TRUE(cache->IsResponseCached(responseKey));
    }

TEST_F(DataSourceCacheTests, IsResponseCached_NoFinalPageCached_False)
    {
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 1));
    EXPECT_FALSE(cache->IsResponseCached(key));
    }

TEST_F(DataSourceCacheTests, IsResponseCached_FinalPageCached_True)
    {
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", ""), nullptr, nullptr, 1));

    EXPECT_TRUE(cache->IsResponseCached(key));
    }

TEST_F(DataSourceCacheTests, IsResponseCached_NotFinalPageCachedIntoFinalizedResponse_False)
    {
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", ""), nullptr, nullptr, 1));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));

    EXPECT_FALSE(cache->IsResponseCached(key));
    }

TEST_F(DataSourceCacheTests, IsResponseCached_AllPagesCachedSecondTime_True)
    {
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", ""), nullptr, nullptr, 1));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("", ""), nullptr, nullptr, 1));

    EXPECT_TRUE(cache->IsResponseCached(key));
    }

TEST_F(DataSourceCacheTests, MarkTemporaryInstancesAsPartial_PartiallyCachedQueryContainsFullyCachedInstanceInTemporaryRoot_SetsInstanceStateToPartial)
    {
    // Arrange
    auto cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot("Root"), "TestQuery");

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "Foo"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, partialInstances.ToWSObjectsResponse()));

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "FullValue"}});

    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, fullInstance.ToWSObjectsResponse(), "Root"));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("Root", CacheRootPersistence::Temporary));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());

    // Act
    EXPECT_EQ(SUCCESS, cache->MarkTemporaryInstancesAsPartial({responseKey}));

    // Assert
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, MarkTemporaryInstancesAsPartial_PartiallyCachedQueryContainsFullyCachedParentInstanceInTemporaryRoot_SetsInstanceStateToPartial)
    {
    // Arrange
    auto cache = GetTestCache();

    // Same instance is parent and result - will force different weak result relationship in cached query
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Foo"}));
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "Foo"}), "TestQuery");

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "Foo"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, partialInstances.ToWSObjectsResponse()));

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "FullValue"}});

    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, fullInstance.ToWSObjectsResponse(), "TestRoot"));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("TestRoot", CacheRootPersistence::Temporary));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());

    // Act
    EXPECT_EQ(SUCCESS, cache->MarkTemporaryInstancesAsPartial({responseKey}));

    // Assert

    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, MarkTemporaryInstancesAsPartial_PartiallyCachedQueryContainsFullyCachedInstanceInFullRoot_LeavesInstanceStateFullyCached)
    {
    // Arrange
    auto cache = GetTestCache();

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "FullyCached"}, {{"TestProperty", "FullValue"}});

    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "FullyCached"}, fullInstance.ToWSObjectsResponse(), "FullPersistenceRoot"));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("FullPersistenceRoot", CacheRootPersistence::Full));
    ASSERT_EQ(SUCCESS, cache->SetupRoot(nullptr, CacheRootPersistence::Temporary));

    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "FullyCached"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, partialInstances.ToWSObjectsResponse()));

    // Act
    EXPECT_EQ(SUCCESS, cache->MarkTemporaryInstancesAsPartial({responseKey}));

    // Assert

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "FullyCached"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, ReadResponse_NonExistingQuery_ReturnsDataNotCachedAndEmptyArray)
    {
    auto cache = GetTestCache();
    ECInstanceKey root = cache->FindOrCreateRoot(nullptr);

    Json::Value results;
    EXPECT_EQ(CacheStatus::DataNotCached, cache->ReadResponse({root, "NonExisting"}, results));
    EXPECT_TRUE(results.empty());
    }

TEST_F(DataSourceCacheTests, ReadResponse_NonExistingParent_ReturnsDataNotCachedAndEmptyArray)
    {
    auto cache = GetTestCache();
    ECInstanceKey parent = cache->FindInstance({"TestSchema.TestClass", "NonExisting"});

    Json::Value results;
    EXPECT_EQ(CacheStatus::DataNotCached, cache->ReadResponse({parent, nullptr}, results));
    EXPECT_TRUE(results.empty());
    }

TEST_F(DataSourceCacheTests, ReadResponse_ZeroResultsCached_ReturnsOkAndEmptyArray)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));

    Json::Value results;
    EXPECT_EQ(CacheStatus::OK, cache->ReadResponse(responseKey, results));
    EXPECT_TRUE(results.empty());
    }

TEST_F(DataSourceCacheTests, ReadResponse_PartialInstanceRejectedWhileCaching_StillReturnsInstanceAsQueryResult)
    {
    // Arrange
    auto cache = GetTestCache();

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "FullyCached"}, {{"TestProperty", "FullValue"}});

    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "FullyCached"}, fullInstance.ToWSObjectsResponse(), "FullRoot"));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("FullRoot", CacheRootPersistence::Full));

    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "FullyCached"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, partialInstances.ToWSObjectsResponse()));

    // Act
    Json::Value queryResults;
    EXPECT_EQ(CacheStatus::OK, cache->ReadResponse(responseKey, queryResults));

    // Assert
    EXPECT_EQ(1, queryResults.size());
    EXPECT_EQ("FullyCached", queryResults[0][DataSourceCache_PROPERTY_RemoteId].asString());
    }

TEST_F(DataSourceCacheTests, ReadResponse_CachedResultsWithTwoInstance_ReturnsBothInstances)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    Json::Value results;
    EXPECT_EQ(CacheStatus::OK, cache->ReadResponse(responseKey, results));

    EXPECT_EQ(2, results.size());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), cache->ObjectIdFromJsonInstance(results[0]));
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "B"), cache->ObjectIdFromJsonInstance(results[1]));
    }

TEST_F(DataSourceCacheTests, ReadResponse_CachedResultsIncludeInstancesRelatedToOtherInstance_ReturnsBothInstances)
    {
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    Json::Value results;
    EXPECT_EQ(CacheStatus::OK, cache->ReadResponse(responseKey, results));

    EXPECT_EQ(2, results.size());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), cache->ObjectIdFromJsonInstance(results[0]));
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "B"), cache->ObjectIdFromJsonInstance(results[1]));
    }

TEST_F(DataSourceCacheTests, ReadResponse_CachedResultsWithInstance_ReturnsInstanceProperties)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "42"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    Json::Value results;
    EXPECT_EQ(CacheStatus::OK, cache->ReadResponse(responseKey, results));

    EXPECT_EQ(1, results.size());
    EXPECT_EQ("42", results[0]["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, ReadResponse_CachedWithKeyWithSeperateHolderButPassingOnlyParent_ReturnsCachedResults)
    {
    auto cache = GetTestCache();
    CachedResponseKey responseKey1(cache->FindOrCreateRoot("Parent"), nullptr, cache->FindOrCreateRoot("Holder"));
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("Parent"), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "42"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse()));

    Json::Value results;
    EXPECT_EQ(CacheStatus::OK, cache->ReadResponse(responseKey2, results));

    EXPECT_EQ(1, results.size());
    EXPECT_EQ("42", results[0]["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, ReadResponse_CachedResultsIncludeParent_ReturnsParentAlso)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "A"}));
    auto instanceA = cache->FindInstance({"TestSchema.TestClass", "A"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse({instanceA, "TestQuery"}, instances.ToWSObjectsResponse()));

    Json::Value results;
    EXPECT_EQ(CacheStatus::OK, cache->ReadResponse({instanceA, "TestQuery"}, results));

    EXPECT_EQ(1, results.size());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), cache->ObjectIdFromJsonInstance(results[0]));
    }

TEST_F(DataSourceCacheTests, ReadResponseInstanceKeys_NonExistingQuery_ReturnsDataNotCachedAndEmptyMap)
    {
    auto cache = GetTestCache();
    CachedResponseKey responseKey({cache->FindOrCreateRoot(nullptr), "NonExisting"});

    ECInstanceKeyMultiMap instances;
    EXPECT_EQ(CacheStatus::DataNotCached, cache->ReadResponseInstanceKeys(responseKey, instances));
    EXPECT_EQ(0, instances.size());
    }

TEST_F(DataSourceCacheTests, ReadResponseInstanceKeys_TwoInstancesCachedAsResult_ReturnsThemInMap)
    {
    auto cache = GetTestCache();

    CachedResponseKey responseKey({cache->FindOrCreateRoot(nullptr), "TestQuery"});

    StubInstances stubInstances;
    stubInstances.Add({"TestSchema.TestClass", "A"});
    stubInstances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, stubInstances.ToWSObjectsResponse()));

    ECInstanceKeyMultiMap instances;
    EXPECT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey, instances));

    EXPECT_EQ(2, instances.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "A"}), instances));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "B"}), instances));
    }

TEST_F(DataSourceCacheTests, ReadResponseInstanceKeys_ResultsContainParent_ReturnsParent)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "A"}));
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "A"}), "TestQuery");

    StubInstances stubInstances;
    stubInstances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, stubInstances.ToWSObjectsResponse()));

    ECInstanceKeyMultiMap instances;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey, instances));

    EXPECT_EQ(1, instances.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "A"}), instances));
    }

TEST_F(DataSourceCacheTests, ReadResponseInstanceKeys_CachedWithKeyWithSeperateHolderButPassingOnlyParent_ReturnsCachedResults)
    {
    auto cache = GetTestCache();
    CachedResponseKey responseKey1(cache->FindOrCreateRoot("Parent"), nullptr, cache->FindOrCreateRoot("Holder"));
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("Parent"), nullptr);

    StubInstances stubInstances;
    stubInstances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey1, stubInstances.ToWSObjectsResponse()));

    ECInstanceKeyMultiMap instances;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey2, instances));

    EXPECT_EQ(1, instances.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "A"}), instances));
    }

TEST_F(DataSourceCacheTests, ReadResponseObjectIds_NonExistingQuery_ReturnsDataNotCached)
    {
    auto cache = GetTestCache();
    CachedResponseKey responseKey({cache->FindOrCreateRoot(nullptr), "NonExisting"});

    bset<ObjectId> objectIds;
    EXPECT_EQ(CacheStatus::DataNotCached, cache->ReadResponseObjectIds(responseKey, objectIds));
    }

TEST_F(DataSourceCacheTests, ReadResponseObjectIds_NonExistingParent_ReturnsDataNotCachedAndNoIds)
    {
    auto cache = GetTestCache();
    ECInstanceKey parent = cache->FindInstance({"TestSchema.TestClass", "NonExisting"});
    CachedResponseKey responseKey(parent, nullptr);

    bset<ObjectId> objectIds;
    EXPECT_EQ(CacheStatus::DataNotCached, cache->ReadResponseObjectIds(responseKey, objectIds));
    EXPECT_TRUE(objectIds.empty());
    }

TEST_F(DataSourceCacheTests, ReadResponseObjectIds_TwoInstancesCachedAsResult_ReturnsTheirObjectIds)
    {
    auto cache = GetTestCache();

    CachedResponseKey responseKey({cache->FindOrCreateRoot(nullptr), "TestQuery"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    bset<ObjectId> objectIds;
    EXPECT_EQ(CacheStatus::OK, cache->ReadResponseObjectIds(responseKey, objectIds));

    EXPECT_EQ(2, objectIds.size());
    EXPECT_CONTAINS(objectIds, ObjectId("TestSchema.TestClass", "A"));
    EXPECT_CONTAINS(objectIds, ObjectId("TestSchema.TestClass", "B"));
    }

TEST_F(DataSourceCacheTests, ReadResponseObjectIds_MultiplePages_ReturnsTheirObjectIds)
    {
    // Arrange
    auto cache = GetTestCache();

    CachedResponseKey key({cache->FindOrCreateRoot(nullptr), "TestQuery"});

    StubInstances instances;
    auto response = instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 1));

    // Act
    bset<ObjectId> objectIds;
    EXPECT_EQ(CacheStatus::OK, cache->ReadResponseObjectIds(key, objectIds));

    // Assert
    EXPECT_EQ(2, objectIds.size());
    EXPECT_CONTAINS(objectIds, ObjectId("TestSchema.TestClass", "A"));
    EXPECT_CONTAINS(objectIds, ObjectId("TestSchema.TestClass", "B"));
    }

TEST_F(DataSourceCacheTests, ReadResponseObjectIds_ResultsContainParent_ReturnsParentObjectId)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "A"}));
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "A"}), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    bset<ObjectId> objectIds;
    EXPECT_EQ(CacheStatus::OK, cache->ReadResponseObjectIds(responseKey, objectIds));

    EXPECT_EQ(1, objectIds.size());
    EXPECT_CONTAINS(objectIds, ObjectId("TestSchema.TestClass", "A"));
    }

TEST_F(DataSourceCacheTests, ReadResponseObjectIds_CachedWithKeyWithSeperateHolderButPassingOnlyParent_ReturnsCachedResults)
    {
    auto cache = GetTestCache();
    CachedResponseKey responseKey1(cache->FindOrCreateRoot("Parent"), nullptr, cache->FindOrCreateRoot("Holder"));
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("Parent"), nullptr);

    StubInstances stubInstances;
    stubInstances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey1, stubInstances.ToWSObjectsResponse()));

    bset<ObjectId> objectIds;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseObjectIds(responseKey2, objectIds));

    EXPECT_EQ(1, objectIds.size());
    EXPECT_CONTAINS(objectIds, ObjectId("TestSchema.TestClass", "A"));
    }

TEST_F(DataSourceCacheTests, RemoveResponse_NonExistingQuery_ReturnsSuccess)
    {
    auto cache = GetTestCache();
    ECInstanceKey root = cache->FindOrCreateRoot(nullptr);

    EXPECT_EQ(SUCCESS, cache->RemoveResponse({root, "NonExisting"}));
    }

TEST_F(DataSourceCacheTests, RemoveResponse_ResponseSharesRelationshipWithOtherResponse_LeavesRelationship)
    {
    auto cache = GetTestCache();

    CachedResponseKey responseKey1(cache->FindOrCreateRoot(nullptr), "TestResponse1");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot(nullptr), "TestResponse2");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse()));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey2, instances.ToWSObjectsResponse()));

    EXPECT_EQ(SUCCESS, cache->RemoveResponse(responseKey1));

    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto instanceA = cache->FindInstance({"TestSchema.TestClass", "A"});
    auto instanceB = cache->FindInstance({"TestSchema.TestClass", "B"});

    EXPECT_TRUE(instanceA.IsValid());
    EXPECT_TRUE(instanceB.IsValid());
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, instanceA, instanceB));
    }

TEST_F(DataSourceCacheTests, RemoveResponse_ResponseWithRelationshipWhenOtherResponseHoldsOnlyInstances_DeletesRelationship)
    {
    auto cache = GetTestCache();

    CachedResponseKey responseKey1(cache->FindOrCreateRoot(nullptr), "TestResponse1");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot(nullptr), "TestResponse2");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse()));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey2, instances.ToWSObjectsResponse()));

    EXPECT_EQ(SUCCESS, cache->RemoveResponse(responseKey2));

    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto instanceA = cache->FindInstance({"TestSchema.TestClass", "A"});
    auto instanceB = cache->FindInstance({"TestSchema.TestClass", "B"});

    EXPECT_TRUE(instanceA.IsValid());
    EXPECT_TRUE(instanceB.IsValid());
    EXPECT_FALSE(VerifyHasRelationship(cache, testRelClass, instanceA, instanceB));
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheTag_NonExistingQueryResults_ReturnsEmptyString)
    {
    auto cache = GetTestCache();
    ECInstanceKey root = cache->FindOrCreateRoot(nullptr);

    EXPECT_EQ("", cache->ReadResponseCacheTag({root, "NonExisting"}));
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheTag_PreviouslyCachedWithTag_ReturnsSameTag)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse("FooTag")));

    EXPECT_EQ("FooTag", cache->ReadResponseCacheTag(responseKey));
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheTag_PreviouslyCachedMultipleTimesWithTags_ReturnsLastTag)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse("A")));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse("B")));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse("C")));

    EXPECT_EQ("C", cache->ReadResponseCacheTag(responseKey));
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheTag_PreviouslyCachedMultipleResponsesWithSameParentsWithTags_ReturnsSameTags)
    {
    auto cache = GetTestCache();
    auto key1 = CachedResponseKey(cache->FindOrCreateRoot(nullptr), "A");
    auto key2 = CachedResponseKey(cache->FindOrCreateRoot(nullptr), "B");

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, StubInstances().ToWSObjectsResponse("A")));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key2, StubInstances().ToWSObjectsResponse("B")));

    EXPECT_EQ("A", cache->ReadResponseCacheTag(key1));
    EXPECT_EQ("B", cache->ReadResponseCacheTag(key2));
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheTag_PreviouslyCachedWithTagAndHolder_ReturnsSameTag)
    {
    auto cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery", cache->FindOrCreateRoot("Holder"));

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse("FooTag")));

    EXPECT_EQ("FooTag", cache->ReadResponseCacheTag(responseKey));
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheTag_NotExistingPage_ReturnsEmpty)
    {
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("Foo"), nullptr, nullptr, 1));

    EXPECT_EQ("", cache->ReadResponseCacheTag(key, 2));
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheTag_DifferentPages_ReturnsTags)
    {
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("A", "NotFinal"), nullptr, nullptr, 0));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("B", "NotFinal"), nullptr, nullptr, 1));

    EXPECT_EQ("A", cache->ReadResponseCacheTag(key, 0));
    EXPECT_EQ("B", cache->ReadResponseCacheTag(key, 1));
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheTag_PagesOvewritten_NoTagForRemovedPages)
    {
    // Arrange
    auto cache = GetTestCache();
    auto key = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("A", "NotFinal"), nullptr, nullptr, 0));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("B", "NotFinal"), nullptr, nullptr, 1));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, StubInstances().ToWSObjectsResponse("C"), nullptr, nullptr, 0));

    // Assert
    EXPECT_EQ("C", cache->ReadResponseCacheTag(key, 0));
    EXPECT_EQ("", cache->ReadResponseCacheTag(key, 1));
    }

TEST_F(DataSourceCacheTests, ReadResponseCachedDate_NonExistingQueryResults_ReturnsInvalidDate)
    {
    auto cache = GetTestCache();
    ECInstanceKey root = cache->FindOrCreateRoot(nullptr);

    EXPECT_EQ(DateTime(), cache->ReadResponseCachedDate({root, "NonExisting"}));
    EXPECT_FALSE(cache->ReadResponseCachedDate({root, "NonExisting"}).IsValid());
    }

TEST_F(DataSourceCacheTests, ReadResponseCachedDate_Cached_ReturnsCorrectCachedDate)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    auto before = DateTime::GetCurrentTimeUtc();
    BeThreadUtilities::BeSleep(1); // DateTime persistence introduces rounding error in nano seconds
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));
    BeThreadUtilities::BeSleep(1);
    auto after = DateTime::GetCurrentTimeUtc();

    EXPECT_BETWEEN(before, cache->ReadResponseCachedDate(responseKey), after);
    }

TEST_F(DataSourceCacheTests, ReadResponseCachedDate_CachedSecondTime_ReturnsLatestCachedDate)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));
    auto dateTime1 = cache->ReadResponseCachedDate(responseKey);

    BeThreadUtilities::BeSleep(1);
    auto before = DateTime::GetCurrentTimeUtc();
    BeThreadUtilities::BeSleep(1); // DateTime persistence introduces rounding error in nano seconds
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));
    BeThreadUtilities::BeSleep(1);
    auto after = DateTime::GetCurrentTimeUtc();

    auto dateTime2 = cache->ReadResponseCachedDate(responseKey);
    EXPECT_NE(dateTime1, dateTime2);
    EXPECT_BETWEEN(before, dateTime2, after);
    }

TEST_F(DataSourceCacheTests, ReadResponseCachedDate_CachedSecondTimeAsNonModified_ReturnsLatestCachedDate)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));
    auto dateTime1 = cache->ReadResponseCachedDate(responseKey);

    BeThreadUtilities::BeSleep(1);
    auto before = DateTime::GetCurrentTimeUtc();
    BeThreadUtilities::BeSleep(1); // DateTime persistence introduces rounding error in nano seconds
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubWSObjectsResponseNotModified()));
    BeThreadUtilities::BeSleep(1);
    auto after = DateTime::GetCurrentTimeUtc();

    auto dateTime2 = cache->ReadResponseCachedDate(responseKey);
    EXPECT_NE(dateTime1, dateTime2);
    EXPECT_BETWEEN(before, dateTime2, after);
    }

TEST_F(DataSourceCacheTests, ReadResponseCachedDate_PreviouslyCachedWithHolder_ReturnsCorrectCachedDate)
    {
    auto cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery", cache->FindOrCreateRoot("Holder"));

    auto before = DateTime::GetCurrentTimeUtc();
    BeThreadUtilities::BeSleep(1); // DateTime persistence introduces rounding error in nano seconds
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));
    BeThreadUtilities::BeSleep(1);
    auto after = DateTime::GetCurrentTimeUtc();

    EXPECT_BETWEEN(before, cache->ReadResponseCachedDate(responseKey), after);
    }

TEST_F(DataSourceCacheTests, ReadResponseCachedDate_PageNotCached_ReturnsInvalid)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse(), nullptr, nullptr, 0));
    EXPECT_TRUE(cache->ReadResponseCachedDate(responseKey, 0).IsValid());
    EXPECT_FALSE(cache->ReadResponseCachedDate(responseKey, 1).IsValid());
    }

TEST_F(DataSourceCacheTests, ReadResponseCachedDate_DifferentPages_DifferentValues)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    auto before = DateTime::GetCurrentTimeUtc();
    BeThreadUtilities::BeSleep(1); // DateTime persistence introduces rounding error in nano seconds
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    BeThreadUtilities::BeSleep(1);
    auto after = DateTime::GetCurrentTimeUtc();

    auto dateTime1 = cache->ReadResponseCachedDate(responseKey, 0);
    EXPECT_BETWEEN(before, dateTime1, after);

    before = DateTime::GetCurrentTimeUtc();
    BeThreadUtilities::BeSleep(1); // DateTime persistence introduces rounding error in nano seconds
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 1));
    BeThreadUtilities::BeSleep(1);
    after = DateTime::GetCurrentTimeUtc();

    auto dateTime2 = cache->ReadResponseCachedDate(responseKey, 1);
    EXPECT_BETWEEN(before, dateTime2, after);
    EXPECT_NE(dateTime1, dateTime2);
    }

TEST_F(DataSourceCacheTests, ReadInstanceLabel_EmptyObjectId_ReturnsEmpty)
    {
    auto cache = GetTestCache();
    EXPECT_EQ("", cache->ReadInstanceLabel({}));
    }

TEST_F(DataSourceCacheTests, ReadInstanceLabel_ClassWithLabelProperty_ReturnsCachedLabel)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestLabeledClass", "Foo"}, *ToRapidJson(R"({"Name" : "TestLabel"})"), "", ""));
    EXPECT_EQ("TestLabel", cache->ReadInstanceLabel({"TestSchema.TestLabeledClass", "Foo"}));
    }

TEST_F(DataSourceCacheTests, ReadInstanceLabel_DreivedClassWithBaseLabelProperty_ReturnsCachedLabel)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestDerivedLabeledClass", "Foo"}, *ToRapidJson(R"({"Name" : "TestLabel"})"), "", ""));
    EXPECT_EQ("TestLabel", cache->ReadInstanceLabel({"TestSchema.TestDerivedLabeledClass", "Foo"}));
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_NonExistingInstance_Error)
    {
    auto cache = GetTestCache();
    Utf8String fileName;
    uint64_t fileSize;
    EXPECT_EQ(ERROR, cache->ReadFileProperties(StubNonExistingInstanceKey(*cache), &fileName, &fileSize));
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_NonFileInstance_SuccessAndEmptyValues)
    {
    auto cache = GetTestCache();

    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"}, {{"TestProperty", "TestValue"}});

    Utf8String fileName = "NoValue";
    uint64_t fileSize = 99;

    ASSERT_EQ(SUCCESS, cache->ReadFileProperties(instanceKey, &fileName, &fileSize));

    EXPECT_EQ("", fileName);
    EXPECT_EQ(0, fileSize);
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_LabeledInstance_SuccessAndReturnsEmptyNameAsLabelMightBeNotSuitable)
    {
    auto cache = GetTestCache();

    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestLabeledClass", "Foo"}, {{"Name", "TestName"}});

    Utf8String fileName = "NoValue";
    uint64_t fileSize = 99;

    ASSERT_EQ(SUCCESS, cache->ReadFileProperties(instanceKey, &fileName, &fileSize));

    EXPECT_EQ("", fileName);
    EXPECT_EQ(0, fileSize);
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_InstanceOfClassClassWithFileDependentProperties_SuccessAndReturnsLabelAndSize)
    {
    auto cache = GetTestCache();

    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestFileClass", "Foo"}, {{"TestName", "TestName"}, {"TestSize", "42"}});

    Utf8String fileName = "NoValue";
    uint64_t fileSize = 99;

    ASSERT_EQ(SUCCESS, cache->ReadFileProperties(instanceKey, &fileName, &fileSize));

    EXPECT_EQ("TestName", fileName);
    EXPECT_EQ(42, fileSize);
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_InstanceOfClassClassWithOnlyFileNameProperty_SuccessAndReturnsLabel)
    {
    auto cache = GetTestCache();
    
    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestFileClass2", "Foo"}, {{"TestName", "TestName"}, {"TestSize", "42"}});
    
    Utf8String fileName = "NoValue";
    uint64_t fileSize = 99;
    
    ASSERT_EQ(SUCCESS, cache->ReadFileProperties(instanceKey, &fileName, &fileSize));
    
    EXPECT_EQ("TestName", fileName);
    EXPECT_EQ(0, fileSize);
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_InstanceOfClassClassWithOnlyFileSizeProperty_SuccessAndReturnsFileSize)
    {
    auto cache = GetTestCache();
    
    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestFileClass3", "Foo"}, {{"TestName", "TestName"}, {"TestSize", "42"}});
    
    Utf8String fileName = "NoValue";
    uint64_t fileSize = 99;
    
    ASSERT_EQ(SUCCESS, cache->ReadFileProperties(instanceKey, &fileName, &fileSize));
    
    EXPECT_EQ("", fileName);
    EXPECT_EQ(42, fileSize);
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_InstanceOfClassClassWithFileDependentPropertiesButNoNameOrSize_SuccessAndReturnsEmptyNameAsLabelMightBeNotSuitable)
    {
    auto cache = GetTestCache();
    
    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestFileClass4", "Foo"}, {{"Name", "TestName"}});
    
    Utf8String fileName = "NoValue";
    uint64_t fileSize = 99;

    ASSERT_EQ(SUCCESS, cache->ReadFileProperties(instanceKey, &fileName, &fileSize));
    
    EXPECT_EQ("", fileName);
    EXPECT_EQ(0, fileSize);
    }

TEST_F(DataSourceCacheTests, CacheFile_NotExistingObject_ReturnsError)
    {
    auto cache = GetTestCache();
    EXPECT_EQ(ERROR, cache->CacheFile({"TestSchema.TestClass", "NotExisting"}, StubWSFileResponse(), FileCache::Persistent));
    }

TEST_F(DataSourceCacheTests, CacheFile_FileResponsePassed_MovesFileToCacheLocation)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));

    BeFileName fileToCachePath = StubFile();
    EXPECT_TRUE(fileToCachePath.DoesPathExist());

    EXPECT_EQ(SUCCESS, cache->CacheFile(fileId, WSFileResponse(fileToCachePath, HttpStatus::OK, "TestTag"), FileCache::Persistent));
    BeFileName cachedFilePath = cache->ReadFilePath(fileId);

    EXPECT_NE(fileToCachePath, cachedFilePath);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    EXPECT_FALSE(fileToCachePath.DoesPathExist());
    EXPECT_EQ("TestTag", cache->ReadFileCacheTag(fileId));
    }

TEST_F(DataSourceCacheTests, CacheFile_ToDefaultLocation_CachesToTemporaryLocation)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse()));

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    EXPECT_THAT(cachedFilePath.c_str(), StartsWith(GetTestCacheEnvironment().temporaryFileCacheDir.c_str()));
    EXPECT_EQ(FileCache::Temporary, cache->GetFileCacheLocation(fileId));
    }

TEST_F(DataSourceCacheTests, CacheFile_AutoLocation_CachesToTemporaryLocation)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::Auto));

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    EXPECT_THAT(cachedFilePath.c_str(), StartsWith(GetTestCacheEnvironment().temporaryFileCacheDir.c_str()));
    EXPECT_EQ(FileCache::Temporary, cache->GetFileCacheLocation(fileId));
    }

TEST_F(DataSourceCacheTests, CacheFile_AutoLocationForFileAlreadyCachedToExternal_CachesToExternalLocation)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));
    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::External));
    ASSERT_EQ(FileCache::External, cache->GetFileCacheLocation(fileId));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::Auto));

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    EXPECT_THAT(cachedFilePath.c_str(), StartsWith(GetTestCacheEnvironment().externalFileCacheDir.c_str()));
    EXPECT_EQ(FileCache::External, cache->GetFileCacheLocation(fileId));
    }

TEST_F(DataSourceCacheTests, CacheFile_AutoLocationForFileAlreadyConfiguredLocationToExternal_CachesToExternalLocation)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External));
    ASSERT_EQ(FileCache::External, cache->GetFileCacheLocation(fileId));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::Auto));

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    EXPECT_THAT(cachedFilePath.c_str(), StartsWith(GetTestCacheEnvironment().externalFileCacheDir.c_str()));
    EXPECT_EQ(FileCache::External, cache->GetFileCacheLocation(fileId));
    }

TEST_F(DataSourceCacheTests, CacheFile_ToPersistentLocation_CachedFilePathBeginsWithEnvironmentPath)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::Persistent));

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    EXPECT_THAT(cachedFilePath.c_str(), StartsWith(GetTestCacheEnvironment().persistentFileCacheDir.c_str()));
    }

TEST_F(DataSourceCacheTests, CacheFile_ToTemporaryLocation_CachedFilePathBeginsWithEnvironmentPath)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::Temporary));

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    EXPECT_THAT(cachedFilePath.c_str(), StartsWith(GetTestCacheEnvironment().temporaryFileCacheDir.c_str()));
    }

TEST_F(DataSourceCacheTests, CacheFile_ToExternalLocation_CachesFileToExternalFolderRoot)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::Temporary));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::External));

    BeFileName path = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path.DoesPathExist());
    EXPECT_EQ(GetTestCacheEnvironment().externalFileCacheDir, path.GetDirectoryName());
    }

TEST_F(DataSourceCacheTests, CacheFile_ToExternalButCacheEnvironmentExternalLocaitonWasEmpty_CachesFileToDefaultExternalLocationInPersistenceFolder)
    {
    auto env = GetTestCacheEnvironment();
    env.externalFileCacheDir = BeFileName();

    auto cache = GetTestCache(env);
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::External));

    BeFileName path = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path.DoesPathExist());
    EXPECT_EQ(env.persistentFileCacheDir, path.GetDirectoryName().PopDir().PopDir().AppendSeparator());
    }

TEST_F(DataSourceCacheTests, CacheFile_FileLocationSetToExternalAndCachingToExternal_CachesFileToExternalFolderRoot)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::External));

    BeFileName path = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path.DoesPathExist());
    EXPECT_EQ(GetTestCacheEnvironment().externalFileCacheDir, path.GetDirectoryName());
    }

TEST_F(DataSourceCacheTests, CacheFile_FileLocationSetToExternalSubFolderWithoutSlashAndCachingToExternal_CachesFileToExternalSubFolder)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));
    BeFileName relativePath(L"Foo");
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External, relativePath));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::External));

    BeFileName path = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path.DoesPathExist());

    EXPECT_EQ(BeFileName(GetTestCacheEnvironment().externalFileCacheDir).AppendToPath(relativePath).AppendSeparator(), path.GetDirectoryName());
    }

TEST_F(DataSourceCacheTests, CacheFile_FileLocationSetToExternalSubFoldersAndCachingToExternal_CachesFileToExternalSubFolder)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));
    BeFileName relativePath(L"Foo/Boo/");
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External, relativePath));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::External));

    BeFileName path = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path.DoesPathExist());
    EXPECT_EQ(BeFileName(GetTestCacheEnvironment().externalFileCacheDir).AppendToPath(relativePath), path.GetDirectoryName());
    }

TEST_F(DataSourceCacheTests, CacheFile_FileCachedInTemporaryAndCachingToExternalLocation_RemovesOldFileAndCachesNewToExternalRoot)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile("ContentA", "Foo.txt")), FileCache::Temporary));
    BeFileName path1 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path1.DoesPathExist());

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile("ContentB", "Foo.txt")), FileCache::External));
    BeFileName path2 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path2.DoesPathExist());

    EXPECT_FALSE(path1.DoesPathExist());
    EXPECT_FALSE(path1.GetDirectoryName().DoesPathExist());
    EXPECT_EQ("ContentB", SimpleReadFile(path2));
    EXPECT_EQ(GetTestCacheEnvironment().externalFileCacheDir, path2.GetDirectoryName());
    }

TEST_F(DataSourceCacheTests, CacheFile_FileWithSameNameCachedPreviously_ReplacesFileInSamePath)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile("ContentA", "Foo.txt")), FileCache::Persistent));
    BeFileName path1 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path1.DoesPathExist());

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile("ContentB", "Foo.txt")), FileCache::Persistent));
    BeFileName path2 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path2.DoesPathExist());

    EXPECT_EQ(path1, path2);
    EXPECT_EQ("ContentB", SimpleReadFile(path2));
    }

TEST_F(DataSourceCacheTests, CacheFile_FileWithDifferentNameCachedPreviously_RemovesOldFileAndAddsNewToSameCacheFolder)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile("ContentA", "A.txt")), FileCache::Persistent));
    BeFileName path1 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path1.DoesPathExist());

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile("ContentB", "B.txt")), FileCache::Persistent));
    BeFileName path2 = cache->ReadFilePath(fileId);

    EXPECT_TRUE(path2.DoesPathExist());
    EXPECT_FALSE(path1.DoesPathExist());
    EXPECT_EQ(path1.GetDirectoryName(), path2.GetDirectoryName());
    }

TEST_F(DataSourceCacheTests, CacheFile_FileWithSameNameCachedPreviouslyAndExternalLocation_ReplacesFileInSamePath)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile("ContentA", "Foo.txt")), FileCache::External));
    BeFileName path1 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path1.DoesPathExist());

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile("ContentB", "Foo.txt")), FileCache::External));
    BeFileName path2 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path2.DoesPathExist());

    EXPECT_EQ(path1, path2);
    EXPECT_EQ("ContentB", SimpleReadFile(path2));
    }

TEST_F(DataSourceCacheTests, CacheFile_FileWithDifferentNameCachedPreviouslyAndExternalLocation_RemovesOldFileAndAddsNewToSameFolder)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile("ContentA", "A.txt")), FileCache::External));
    BeFileName path1 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path1.DoesPathExist());

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile("ContentB", "B.txt")), FileCache::External));
    BeFileName path2 = cache->ReadFilePath(fileId);

    EXPECT_TRUE(path2.DoesPathExist());
    EXPECT_FALSE(path1.DoesPathExist());
    EXPECT_EQ(path1.GetDirectoryName(), path2.GetDirectoryName());
    }

TEST_F(DataSourceCacheTests, CacheFile_FileWithDifferentNameCachedPreviouslyAndExternalSubFolderLocation_RemovesOldFileAndAddsNewToSameSubFolder)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External, BeFileName(L"SubFolder")));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile("ContentA", "A.txt")), FileCache::External));
    BeFileName path1 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path1.DoesPathExist());

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile("ContentB", "B.txt")), FileCache::External));
    BeFileName path2 = cache->ReadFilePath(fileId);

    EXPECT_TRUE(path2.DoesPathExist());
    EXPECT_FALSE(path1.DoesPathExist());
    EXPECT_EQ(path1.GetDirectoryName(), path2.GetDirectoryName());
    }

TEST_F(DataSourceCacheTests, CacheFile_FileCachedPreviouslyAndCachingToDifferentLocation_CachesNewFileAndRemovesOldFileFolder)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::Temporary));
    BeFileName path1 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path1.DoesPathExist());

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::Persistent));
    BeFileName path2 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path2.DoesPathExist());

    EXPECT_FALSE(path1.DoesPathExist());
    EXPECT_FALSE(path1.GetDirectoryName().DoesPathExist());
    }

TEST_F(DataSourceCacheTests, CacheFile_FileCachedToExternalSubFolderAndCachingToDifferentLocation_CachesNewFileToFileStorageAndRemovesExternalSubFolder)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External, BeFileName(L"SubFolder")));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::External));
    BeFileName path1 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path1.DoesPathExist());

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::Persistent));
    BeFileName path2 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path2.DoesPathExist());

    EXPECT_FALSE(path1.DoesPathExist());
    EXPECT_THAT(path2.c_str(), Not(HasSubstr(L"SubFolder")));
    EXPECT_FALSE(path1.GetDirectoryName().DoesPathExist());
    }

TEST_F(DataSourceCacheTests, CacheFile_FileResponseNotModifiedPassed_UpdatesCachedDate)
    {
    auto cache = GetTestCache();
    ObjectId fileId = cache->FindInstance(StubInstanceInCache(*cache));
    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, ""), FileCache::Persistent));

    auto before = DateTime::GetCurrentTimeUtc();
    BeThreadUtilities::BeSleep(1);
    EXPECT_EQ(SUCCESS, cache->CacheFile(fileId, WSFileResponse(BeFileName(), HttpStatus::NotModified, ""), FileCache::Persistent));
    BeThreadUtilities::BeSleep(1);
    auto after = DateTime::GetCurrentTimeUtc();

    DateTime cachedDate = cache->ReadFileCachedDate(fileId);
    EXPECT_BETWEEN(before, cachedDate, after);
    }

TEST_F(DataSourceCacheTests, ReadFileCachedDate_FileNotCached_ReturnsInvalid)
    {
    auto cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", fileId));

    ASSERT_THAT(cache->ReadFileCachedDate(fileId).IsValid(), false);
    }

TEST_F(DataSourceCacheTests, ReadFileCachedDate_FileNotOnDiskAnyMore_ReturnsInvalid)
    {
    auto cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", fileId));
    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::Persistent));
    ASSERT_EQ(BeFileNameStatus::Success, cache->ReadFilePath(fileId).BeDeleteFile());

    ASSERT_THAT(cache->ReadFileCachedDate(fileId).IsValid(), false);
    }

TEST_F(DataSourceCacheTests, ReadInstanceCachedDate_InstanceCached_ReturnsDateWhenCached)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    ObjectIdCR objectId {"TestSchema.TestClass", "Id"};
    instances.Add(objectId, {}, "TestTag");

    auto before = DateTime::GetCurrentTimeUtc();
    BeThreadUtilities::BeSleep(1);
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot(objectId, instances.ToWSObjectsResponse(), nullptr));
    BeThreadUtilities::BeSleep(1);
    auto after = DateTime::GetCurrentTimeUtc();

    DateTime cachedDate = cache->ReadInstanceCachedDate(objectId);
    EXPECT_BETWEEN(before, cachedDate, after);
    }

TEST_F(DataSourceCacheTests, ReadInstanceCachedDate_InstanceNotCached_ReturnsInvalid)
    {
    auto cache = GetTestCache();

    ASSERT_THAT(cache->ReadInstanceCachedDate({"TestSchema.TestClass", "NonExisting"}).IsValid(), false);
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_NotExistingInstance_Error)
    {
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(ERROR, cache->SetFileCacheLocation(fileId, FileCache::Temporary));
    ASSERT_EQ(ERROR, cache->SetFileCacheLocation(fileId, FileCache::Persistent));
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_NotExistingFile_ChangesLocationAndFilePathIsEmpty)
    {
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", fileId));

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::Persistent));
    EXPECT_EQ(FileCache::Persistent, cache->GetFileCacheLocation(fileId));
    EXPECT_EQ(L"", cache->ReadFilePath(fileId));

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::Temporary));
    EXPECT_EQ(FileCache::Temporary, cache->GetFileCacheLocation(fileId));
    EXPECT_EQ(L"", cache->ReadFilePath(fileId));
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_PassingAutoForNotExistingFile_KeepsLocationTemporary)
    {
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", fileId));

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::Auto));
    EXPECT_EQ(FileCache::Temporary, cache->GetFileCacheLocation(fileId));
    EXPECT_EQ(L"", cache->ReadFilePath(fileId));
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_PassingAutoToAlreadyConfiguredLocationPersistent_KeepsLocationPersistent)
    {
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", fileId));

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::Persistent));
    ASSERT_EQ(FileCache::Persistent, cache->GetFileCacheLocation(fileId));
    ASSERT_EQ(L"", cache->ReadFilePath(fileId));

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::Auto));
    EXPECT_EQ(FileCache::Persistent, cache->GetFileCacheLocation(fileId));
    EXPECT_EQ(L"", cache->ReadFilePath(fileId));
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_NotExistingFile_SuccessAndLocationCanBeChangedWhenCaching)
    {
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", fileId));
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::Persistent));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::Temporary));

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    EXPECT_THAT(cachedFilePath.c_str(), StartsWith(GetTestCacheEnvironment().temporaryFileCacheDir.c_str()));
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_MovingCachedFileToTemporary_MovesFileToTemporaryEnvironmentLocation)
    {
    auto cache = GetTestCache();
    ObjectId fileId = StubFileInCache(*cache, FileCache::Persistent);
    BeFileName path1 = cache->ReadFilePath(fileId);

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::Temporary));

    EXPECT_FALSE(path1.DoesPathExist());
    EXPECT_FALSE(path1.GetDirectoryName().DoesPathExist());
    BeFileName path2 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path2.DoesPathExist());
    EXPECT_THAT(path2.c_str(), StartsWith(GetTestCacheEnvironment().temporaryFileCacheDir.c_str()));
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_MovingCachedFileToPersistent_MovesFileToPersistentEnvironmentLocation)
    {
    auto cache = GetTestCache();
    ObjectId fileId = StubFileInCache(*cache, FileCache::Temporary);
    BeFileName path1 = cache->ReadFilePath(fileId);

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::Persistent));

    EXPECT_FALSE(path1.DoesPathExist());
    EXPECT_FALSE(path1.GetDirectoryName().DoesPathExist());
    BeFileName path2 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path2.DoesPathExist());
    EXPECT_THAT(path2.c_str(), StartsWith(GetTestCacheEnvironment().persistentFileCacheDir.c_str()));
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_MovingFileFromTemporaryToPersistentWhenEnvironmentPathsAreTheSame_FileStaysInSamePath)
    {
    auto environment = GetTestCacheEnvironment();
    environment.temporaryFileCacheDir = environment.persistentFileCacheDir;

    auto cache = GetTestCache(environment);
    ObjectId fileId = StubFileInCache(*cache, FileCache::Temporary);
    BeFileName path1 = cache->ReadFilePath(fileId);

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::Persistent));
    BeFileName path2 = cache->ReadFilePath(fileId);

    EXPECT_TRUE(path1.DoesPathExist());
    EXPECT_EQ(path1, path2);
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_MovingCachedFileToExternalWithoutRelativePath_MovesFileToExternalRootLocation)
    {
    auto cache = GetTestCache();
    ObjectId fileId = StubFileInCache(*cache, FileCache::Temporary);
    BeFileName path1 = cache->ReadFilePath(fileId);

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External));

    BeFileName path2 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path2.DoesPathExist());
    EXPECT_EQ(GetTestCacheEnvironment().externalFileCacheDir, path2.GetDirectoryName());

    EXPECT_FALSE(path1.DoesPathExist());
    EXPECT_FALSE(path1.GetDirectoryName().DoesPathExist());
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_MovingCachedFileFromExternal_MovesFileToNewLocationAndRemovesExternalSubfolder)
    {
    auto cache = GetTestCache();
    ObjectId fileId = StubFileInCache(*cache, FileCache::Temporary);
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External, BeFileName(L"SubFolder")));
    BeFileName path1 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path1.DoesPathExist());

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::Persistent));

    BeFileName path2 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path2.DoesPathExist());
    EXPECT_THAT(path2.c_str(), StartsWith(GetTestCacheEnvironment().persistentFileCacheDir.c_str()));

    EXPECT_FALSE(path1.DoesPathExist());
    EXPECT_FALSE(path1.GetDirectoryName().DoesPathExist());
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_MovingToOtherExternalLocation_MovesFileAndRemovesExternalSubfolders)
    {
    auto cache = GetTestCache();
    ObjectId fileId = StubFileInCache(*cache, FileCache::Temporary);
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External, BeFileName(L"A/B/C")));
    BeFileName path1 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path1.DoesPathExist());

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External, BeFileName(L"C/D/E")));

    BeFileName path2 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path2.DoesPathExist());
    EXPECT_EQ(BeFileName(cache->GetEnvironment().externalFileCacheDir).AppendToPath(L"C/D/E"), path2.PopDir());

    EXPECT_FALSE(path1.DoesPathExist());
    EXPECT_FALSE(BeFileName(cache->GetEnvironment().externalFileCacheDir).AppendToPath(L"A").DoesPathExist());
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_TwoFilesInExternalFolderAndMovingOneOfThem_MovesFileButLeavesExternalSubfFolder)
    {
    auto cache = GetTestCache();
    ObjectId fileIdA = StubFileInCache(*cache, FileCache::Temporary, {"TestSchema.TestClass", "A"});
    ObjectId fileIdB = StubFileInCache(*cache, FileCache::Temporary, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileIdA, FileCache::External, BeFileName(L"SubFolder")));
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileIdB, FileCache::External, BeFileName(L"SubFolder")));
    BeFileName pathA = cache->ReadFilePath(fileIdA);
    BeFileName pathB = cache->ReadFilePath(fileIdB);
    EXPECT_TRUE(pathA.DoesPathExist());
    EXPECT_TRUE(pathB.DoesPathExist());

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileIdA, FileCache::External, BeFileName(L"Other")));

    BeFileName pathA2 = cache->ReadFilePath(fileIdA);
    EXPECT_TRUE(pathA2.DoesPathExist());
    EXPECT_EQ(BeFileName(cache->GetEnvironment().externalFileCacheDir).AppendToPath(L"Other"), pathA2.PopDir());

    EXPECT_TRUE(pathB.DoesPathExist());
    EXPECT_EQ(pathB, cache->ReadFilePath(fileIdB));
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_PassedRelativePathButNotExternalLocationForCachedFile_ErrorWithNoPathChange)
    {
    auto cache = GetTestCache();
    auto fileId = StubFileInCache(*cache);

    BeFileName path("Foo/Boo/");
    BeFileName path1 = cache->ReadFilePath(fileId);
    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(ERROR, cache->SetFileCacheLocation(fileId, FileCache::Persistent, path));
    EXPECT_EQ(ERROR, cache->SetFileCacheLocation(fileId, FileCache::Temporary, path));
    BeTest::SetFailOnAssert(true);
    BeFileName path2 = cache->ReadFilePath(fileId);
    EXPECT_EQ(path1, path2);
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_ExternalLocationForCachedFileWithoutRelativePath_MovesDirectlyToRootDirectory)
    {
    auto cache = GetTestCache();
    auto fileId = StubFileInCache(*cache);
    BeFileName path1 = cache->ReadFilePath(fileId);

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External));

    EXPECT_FALSE(path1.DoesPathExist());
    BeFileName path2 = cache->ReadFilePath(fileId);
    EXPECT_TRUE(path2.DoesPathExist());
    EXPECT_EQ(GetTestCacheEnvironment().externalFileCacheDir, path2.GetDirectoryName());
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_ExternalLocationForCachedFileWithRelativePath_MovesToRootDirectorySubfolder)
    {
    auto cache = GetTestCache();
    auto fileId = StubFileInCache(*cache);

    BeFileName path1 = cache->ReadFilePath(fileId);

    BeFileName relativePath(L"Foo/Boo/");
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External, relativePath));

    BeFileName path2 = cache->ReadFilePath(fileId);
    EXPECT_NE(path1, path2);
    EXPECT_FALSE(path1.DoesPathExist());
    EXPECT_TRUE(path2.DoesPathExist());
    EXPECT_EQ(BeFileName(GetTestCacheEnvironment().externalFileCacheDir).AppendToPath(relativePath), path2.GetDirectoryName());
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_ExternalLocationAndSameRelativePath_SuccessAndDoesNothingToFile)
    {
    auto cache = GetTestCache();
    auto fileId = StubFileInCache(*cache);
    BeFileName relativePath(L"Foo/Boo/");
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External, relativePath));

    BeFileName path1 = cache->ReadFilePath(fileId);
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External, relativePath));
    BeFileName path2 = cache->ReadFilePath(fileId);

    EXPECT_EQ(path1, path2);
    EXPECT_TRUE(path2.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_ExternalLocationForCachedFileWithRelativePathWithoutEndSlash_MovesToRootDirectorySubfolder)
    {
    auto cache = GetTestCache();
    auto fileId = StubFileInCache(*cache);

    BeFileName relativePath(L"Foo/Boo");
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External, relativePath));

    BeFileName path = cache->ReadFilePath(fileId);
    EXPECT_EQ(BeFileName(GetTestCacheEnvironment().externalFileCacheDir).AppendToPath(L"Foo/Boo/"), path.GetDirectoryName());
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_ExternalLocationForCachedFileWithRelativePathOnlyASlash_Error)
    {
    auto cache = GetTestCache();
    auto fileId = StubFileInCache(*cache);
    BeTest::SetFailOnAssert(false);
    ASSERT_EQ(ERROR, cache->SetFileCacheLocation(fileId, FileCache::External, BeFileName(L"/")));
    BeTest::SetFailOnAssert(true);
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_ExternalLocationAndFileAlreadyExistsThere_SuccessAndOverridesFile)
    {
    auto cache = GetTestCache();
    auto fileId = StubFileInCache(*cache, StubFile("A", "Test.txt"));
    BeFileName path1 = cache->ReadFilePath(fileId);

    BeFileName existingFilePath = GetTestCacheEnvironment().externalFileCacheDir;
    existingFilePath.AppendToPath(L"Foo/Boo/Test.txt");
    SimpleWriteToFile("B", existingFilePath);

    BeFileName relativePath(L"Foo/Boo/");
    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::External, relativePath));

    auto path2 = cache->ReadFilePath(fileId);
    EXPECT_NE(path1, path2);
    EXPECT_FALSE(path1.DoesPathExist());
    EXPECT_TRUE(path2.DoesPathExist());
    EXPECT_EQ("A", SimpleReadFile(existingFilePath));
    }

TEST_F(DataSourceCacheTests, GetFileCacheLocation_NotExistingInstance_ReturnsTemporary)
    {
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "NonExisting"};
    EXPECT_EQ(FileCache::Temporary, cache->GetFileCacheLocation(fileId));
    }

TEST_F(DataSourceCacheTests, GetFileCacheLocation_NotExistingInstanceButDefaultPassed_ReturnsDefault)
    {
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "NonExisting"};
    EXPECT_EQ(FileCache::Persistent, cache->GetFileCacheLocation(fileId, FileCache::Persistent));
    }

TEST_F(DataSourceCacheTests, GetFileCacheLocation_NotCachedFile_ReturnsTemporary)
    {
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", fileId));
    EXPECT_EQ(FileCache::Temporary, cache->GetFileCacheLocation(fileId));
    }

TEST_F(DataSourceCacheTests, GetFileCacheLocation_NotCachedFileButDefaultPassed_ReturnsSameDefault)
    {
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", fileId));
    EXPECT_EQ(FileCache::Persistent, cache->GetFileCacheLocation(fileId, FileCache::Persistent));
    }

TEST_F(DataSourceCacheTests, GetFileCacheLocation_NotCachedFileButDefaultAutoPassed_ReturnsTemporary)
    {
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", fileId));
    EXPECT_EQ(FileCache::Temporary, cache->GetFileCacheLocation(fileId, FileCache::Auto));
    }

TEST_F(DataSourceCacheTests, GetFileCacheLocation_CachedToTemporary_ReturnsTemporary)
    {
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", fileId));
    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::Temporary));

    EXPECT_EQ(FileCache::Temporary, cache->GetFileCacheLocation(fileId));
    }

TEST_F(DataSourceCacheTests, GetFileCacheLocation_CachedToPersistent_ReturnsPersistent)
    {
    auto cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", fileId));

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(), FileCache::Persistent));
    EXPECT_EQ(FileCache::Persistent, cache->GetFileCacheLocation(fileId));
    }

TEST_F(DataSourceCacheTests, ReadFilePath_CachedFileAndObjectIdPassed_ReturnsPath)
    {
    auto cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, fileId));

    EXPECT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile(), "TestTag"), FileCache::Persistent));
    BeFileName cachedFilePath = cache->ReadFilePath(fileId);

    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, ReadFilePath_CachedFileAndECInstanceKeyPassed_ReturnsPath)
    {
    auto cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, fileId));

    EXPECT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile(), "TestTag"), FileCache::Persistent));

    ECInstanceKey fileKey = cache->FindInstance(fileId);
    BeFileName cachedFilePath = cache->ReadFilePath(fileKey);

    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, ReadFilePath_CachedFileButIsDeletedFromDisk_ReturnsEmpty)
    {
    auto cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, fileId));
    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile(), "TestTag"), FileCache::Persistent));

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    ASSERT_EQ(BeFileNameStatus::Success, cachedFilePath.BeDeleteFile());

    EXPECT_THAT(cache->ReadFilePath(fileId), IsEmpty());
    }

TEST_F(DataSourceCacheTests, ReadFileCacheTag_NotCachedFile_ReturnsEmpty)
    {
    auto cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, fileId));

    EXPECT_THAT(cache->ReadFileCacheTag(fileId), IsEmpty());
    }

TEST_F(DataSourceCacheTests, ReadFileCacheTag_CachedFile_ReturnsTag)
    {
    auto cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, fileId));
    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile(), "TestTag"), FileCache::Persistent));

    EXPECT_THAT(cache->ReadFileCacheTag(fileId), Eq("TestTag"));
    }

TEST_F(DataSourceCacheTests, ReadFileCacheTag_CachedFileButIsDeletedFromDisk_ReturnsEmpty)
    {
    auto cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, fileId));
    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile(), "TestTag"), FileCache::Persistent));

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    ASSERT_EQ(BeFileNameStatus::Success, cachedFilePath.BeDeleteFile());

    EXPECT_THAT(cache->ReadFileCacheTag(fileId), IsEmpty());
    }

TEST_F(DataSourceCacheTests, FindInstance_ObjectIdAndNotCached_InvalidKey)
    {
    auto cache = GetTestCache();

    ECInstanceKey instanceKey = cache->FindInstance({"TestSchema.TestClass", "NonExisting"});

    EXPECT_FALSE(instanceKey.IsValid());
    }

TEST_F(DataSourceCacheTests, CacheFile_PersistentFileCached_CorrectECDbExternalFileInfoCreated)
    {
    auto cache = GetTestCache();
    auto fileId = StubFileInCache(*cache, FileCache::Persistent, StubObjectId(), StubFile("Foo", "Test.txt"));
    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());

    Json::Value externalFileInfos;
    ASSERT_EQ(SUCCESS, cache->GetAdapter().GetJsonInstances(externalFileInfos, cache->GetAdapter().GetECClass("ECDbFileInfo.ExternalFileInfo")));
    ASSERT_EQ(1, externalFileInfos.size());

    auto relativePath = externalFileInfos[0]["RelativePath"].asString();
    EXPECT_FALSE(relativePath.empty());
    EXPECT_TRUE(cachedFilePath.EndsWith(BeFileName(relativePath)));
    EXPECT_EQ(CacheEnvironment::GetRootFolderId(FileCache::Persistent), externalFileInfos[0]["RootFolder"].asInt());
    EXPECT_EQ("Test.txt", externalFileInfos[0]["Name"].asString());
    }

TEST_F(DataSourceCacheTests, CacheFile_TemporaryFileCached_CorrectECDbExternalFileInfoCreated)
    {
    auto cache = GetTestCache();
    auto fileId = StubFileInCache(*cache, FileCache::Temporary, StubObjectId(), StubFile("Foo", "Test.txt"));
    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());

    Json::Value externalFileInfos;
    ASSERT_EQ(SUCCESS, cache->GetAdapter().GetJsonInstances(externalFileInfos, cache->GetAdapter().GetECClass("ECDbFileInfo.ExternalFileInfo")));
    ASSERT_EQ(1, externalFileInfos.size());

    auto relativePath = externalFileInfos[0]["RelativePath"].asString();
    EXPECT_FALSE(relativePath.empty());
    EXPECT_TRUE(cachedFilePath.EndsWith(BeFileName(relativePath)));
    EXPECT_EQ(CacheEnvironment::GetRootFolderId(FileCache::Temporary), externalFileInfos[0]["RootFolder"].asInt());
    EXPECT_EQ("Test.txt", externalFileInfos[0]["Name"].asString());
    }

TEST_F(DataSourceCacheTests, CacheFile_ExternalFileCached_CorrectECDbExternalFileInfoCreated)
    {
    auto cache = GetTestCache();
    auto fileId = StubFileInCache(*cache, FileCache::External, StubObjectId(), StubFile("Foo", "Test.txt"));
    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());

    Json::Value externalFileInfos;
    ASSERT_EQ(SUCCESS, cache->GetAdapter().GetJsonInstances(externalFileInfos, cache->GetAdapter().GetECClass("ECDbFileInfo.ExternalFileInfo")));
    ASSERT_EQ(1, externalFileInfos.size());

    auto relativePath = externalFileInfos[0]["RelativePath"].asString();
    EXPECT_FALSE(relativePath.empty());
    EXPECT_TRUE(cachedFilePath.EndsWith(BeFileName(relativePath)));
    EXPECT_EQ(CacheEnvironment::GetRootFolderId(FileCache::External), externalFileInfos[0]["RootFolder"].asInt());
    EXPECT_EQ("Test.txt", externalFileInfos[0]["Name"].asString());
    }

TEST_F(DataSourceCacheTests, CacheFile_FileCached_CorrectECDbFileInfoStructureCreated)
    {
    auto cache = GetTestCache();

    auto fileId = ObjectId("TestSchema.TestClass", "Foo");
    auto fileKey = StubInstanceInCache(*cache, fileId);

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, StubWSFileResponse(StubFile()), FileCache::Persistent));
    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());

    Json::Value fileInfoOwnerships, externalFileInfos;
    ASSERT_EQ(SUCCESS, cache->GetAdapter().GetJsonInstances(fileInfoOwnerships, cache->GetAdapter().GetECClass("ECDbFileInfo.FileInfoOwnership")));
    ASSERT_EQ(SUCCESS, cache->GetAdapter().GetJsonInstances(externalFileInfos, cache->GetAdapter().GetECClass("ECDbFileInfo.ExternalFileInfo")));

    ASSERT_EQ(1, fileInfoOwnerships.size());
    ASSERT_EQ(1, externalFileInfos.size());

    auto externalFileInfoKey = cache->GetAdapter().GetInstanceKeyFromJsonInstance(externalFileInfos[0]);
    ASSERT_TRUE(externalFileInfoKey.IsValid());

    ECInstanceKey ownershipOwnerKey(
        ECClassId((uint64_t) BeJsonUtilities::Int64FromValue(fileInfoOwnerships[0]["OwnerECClassId"], -42)),
        ECInstanceId((uint64_t) BeJsonUtilities::Int64FromValue(fileInfoOwnerships[0]["OwnerId"], -42)));

    ECInstanceKey ownershipFileInfoKey(
        ECClassId((uint64_t) BeJsonUtilities::Int64FromValue(fileInfoOwnerships[0]["FileInfoECClassId"], -42)),
        ECInstanceId((uint64_t) BeJsonUtilities::Int64FromValue(fileInfoOwnerships[0]["FileInfoId"], -42)));

    EXPECT_EQ(fileKey, ownershipOwnerKey);
    EXPECT_EQ(externalFileInfoKey, ownershipFileInfoKey);
    }

TEST_F(DataSourceCacheTests, FindInstance_NotExistingObjectIdRemoteId_InvalidKeyButCorrectClass)
    {
    auto cache = GetTestCache();

    ECInstanceKey instanceKey = cache->FindInstance({"TestSchema.TestClass", "NonExisting"});

    EXPECT_FALSE(instanceKey.IsValid());
    EXPECT_EQ(cache->GetAdapter().GetECClass("TestSchema.TestClass")->GetId(), instanceKey.GetClassId());
    }

TEST_F(DataSourceCacheTests, FindInstance_NotExistingObjectIdClass_EmptyKey)
    {
    auto cache = GetTestCache();

    ECInstanceKey instanceKey = cache->FindInstance({"TestSchema.NonExistingClass", "Foo"});

    EXPECT_FALSE(instanceKey.IsValid());
    EXPECT_EQ(ECInstanceKey(), instanceKey);
    }

TEST_F(DataSourceCacheTests, FindInstance_EmptyObjectId_ReturnsInvalid)
    {
    auto cache = GetTestCache();

    ECInstanceKey instanceKey = cache->FindInstance(ObjectId());
    EXPECT_FALSE(instanceKey.IsValid());
    }

TEST_F(DataSourceCacheTests, FindInstance_EmptyObjectIdAndNavigationBaseLinkedToRoot_ReturnsKeyToNavigationBase)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, ObjectId()));

    ECInstanceKey instanceKey = cache->FindInstance(ObjectId());
    EXPECT_TRUE(instanceKey.IsValid());
    }

TEST_F(DataSourceCacheTests, FindInstance_KeyToNavigationBase_ReturnsEmptyObjectId)
    {
    auto cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, ObjectId()));
    ECInstanceKey baseKey = cache->FindInstance(ObjectId());
    ASSERT_TRUE(baseKey.IsValid());

    EXPECT_EQ(ObjectId(), cache->FindInstance(baseKey));
    }

TEST_F(DataSourceCacheTests, FindInstance_ECInstanceKeyAndNotCached_InvalidObjectId)
    {
    auto cache = GetTestCache();
    ECClassCP ecClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");

    ObjectId objectId = cache->FindInstance(ECInstanceKey(ecClass->GetId(), ECInstanceId(UINT64_C(1))));

    EXPECT_FALSE(objectId.IsValid());
    }

TEST_F(DataSourceCacheTests, FindInstance_ObjectIdAndInCache_ValidKey)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "Foo"}));
    Json::Value instance;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instance));
    ECInstanceKey instanceKey = cache->GetAdapter().GetInstanceKeyFromJsonInstance(instance);

    ECInstanceKey foundInstanceKey = cache->FindInstance({"TestSchema.TestClass", "Foo"});

    EXPECT_TRUE(foundInstanceKey.IsValid());
    EXPECT_EQ(instanceKey, foundInstanceKey);
    }

TEST_F(DataSourceCacheTests, FindInstance_ECInstanceKeyAndInCache_ValidObjectId)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "Foo"}));
    Json::Value instance;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instance));
    ECInstanceKey instanceKey = cache->GetAdapter().GetInstanceKeyFromJsonInstance(instance);

    ObjectId objectId = cache->FindInstance(instanceKey);

    EXPECT_TRUE(objectId.IsValid());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "Foo"), objectId);
    }

TEST_F(DataSourceCacheTests, FindInstance_ObjectIdAndChangeDeleted_ValidKey)
    {
    auto cache = GetTestCache();
    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instanceKey));

    ECInstanceKey foundInstanceKey = cache->FindInstance({"TestSchema.TestClass", "Foo"});

    EXPECT_TRUE(foundInstanceKey.IsValid());
    EXPECT_EQ(instanceKey, foundInstanceKey);
    }

TEST_F(DataSourceCacheTests, FindInstance_ECInstanceKeyAndChangeDeleted_ValidObjectId)
    {
    auto cache = GetTestCache();
    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instanceKey));

    ObjectId objectId = cache->FindInstance(instanceKey);

    EXPECT_TRUE(objectId.IsValid());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "Foo"), objectId);
    }

TEST_F(DataSourceCacheTests, FindRelationship_NoRelationshipWithSuchEnds_ReturnsInvalidKey)
    {
    auto cache = GetTestCache();

    auto instanceA = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto instanceB = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    EXPECT_FALSE(cache->FindRelationship(*relClass, instanceA, instanceB).IsValid());
    EXPECT_FALSE(cache->FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}).IsValid());
    }

TEST_F(DataSourceCacheTests, FindRelationship_RelationshipWithSuchEndsExists_ReturnsRelationshipKey)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    auto instanceA = cache->FindInstance({"TestSchema.TestClass", "A"});
    auto instanceB = cache->FindInstance({"TestSchema.TestClass", "B"});
    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto expectedRelationship = FindRelationship(cache, relClass, instanceA, instanceB);
    ASSERT_TRUE(expectedRelationship.IsValid());

    EXPECT_EQ(expectedRelationship, cache->FindRelationship(*relClass, instanceA, instanceB));
    EXPECT_EQ(expectedRelationship, cache->FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    }

TEST_F(DataSourceCacheTests, FindRelationship_NoCachedRelationshipWithSuchKey_ReturnsEmptyObjectId)
    {
    auto cache = GetTestCache();

    auto instanceA = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto instanceB = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache->GetAdapter().RelateInstances(relClass, instanceA, instanceB);

    EXPECT_EQ(ObjectId(), cache->FindRelationship(relationship));
    }

TEST_F(DataSourceCacheTests, FindRelationship_CachedRelationshipExists_ReturnsRelationshipObjectId)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache->FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"});

    EXPECT_EQ(ObjectId({"TestSchema.TestRelationshipClass", "AB"}), cache->FindRelationship(relationship));
    }

TEST_F(DataSourceCacheTests, ReadInstancesConnectedToRootMap_DifferentClassInstancesLinked_ReturnsOnlyCachedInstanceIds)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Foo", {"TestSchema.TestClass", "A"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Foo", {"TestSchema.TestClass2", "B"}));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "C"});
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "A"}), "Foo");
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    ECInstanceKeyMultiMap map;
    ASSERT_EQ(SUCCESS, cache->ReadInstancesConnectedToRootMap("Foo", map));

    EXPECT_EQ(3, map.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "A"}), map));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass2", "B"}), map));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "C"}), map));
    }

TEST_F(DataSourceCacheTests, ReadInstancesLinkedToRoot_DifferentClassInstancesLinked_ReturnsIds)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Foo", {"TestSchema.TestClass", "A"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Foo", {"TestSchema.TestClass2", "B"}));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "C"});
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "A"}), "Foo");
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    ECInstanceKeyMultiMap map;
    ASSERT_EQ(SUCCESS, cache->ReadInstancesLinkedToRoot("Foo", map));

    EXPECT_EQ(2, map.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "A"}), map));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass2", "B"}), map));
    }

TEST_F(DataSourceCacheTests, ReadInstanceHierarchy_InstanceIsInResponseButHasNoResponses_ReturnsNoInstances)
    {
    auto cache = GetTestCache();

    CachedResponseKey responseKey1(cache->FindOrCreateRoot("Root"), "Foo");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse()));

    CachedResponseKey responseKey2(cache->FindInstance({"TestSchema.TestClass", "A"}), "Foo");
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "C"});
    instances.Add({"TestSchema.TestClass", "D"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey2, instances.ToWSObjectsResponse()));

    auto instance = cache->FindInstance({"TestSchema.TestClass", "C"});

    ECInstanceKeyMultiMap map;
    ASSERT_EQ(SUCCESS, cache->ReadInstanceHierarchy(instance, map));

    EXPECT_EQ(0, map.size());
    }

TEST_F(DataSourceCacheTests, ReadInstanceHierarchy_InstanceHasResponsesCachedUnderIt_ReturnsInstancesUnderIt)
    {
    auto cache = GetTestCache();

    CachedResponseKey responseKey1(cache->FindOrCreateRoot("Root"), "Foo");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse()));

    CachedResponseKey responseKey2(cache->FindInstance({"TestSchema.TestClass", "A"}), "Foo");
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "C"});
    instances.Add({"TestSchema.TestClass", "D"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey2, instances.ToWSObjectsResponse()));

    CachedResponseKey responseKey3(cache->FindInstance({"TestSchema.TestClass", "C"}), "Foo");
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "E"});
    instances.Add({"TestSchema.TestClass", "F"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey3, instances.ToWSObjectsResponse()));

    auto instance = cache->FindInstance({"TestSchema.TestClass", "A"});

    ECInstanceKeyMultiMap map;
    ASSERT_EQ(SUCCESS, cache->ReadInstanceHierarchy(instance, map));

    EXPECT_EQ(4, map.size());
    EXPECT_CONTAINS(map, ECDbHelper::ToPair(cache->FindInstance({"TestSchema.TestClass", "C"})));
    EXPECT_CONTAINS(map, ECDbHelper::ToPair(cache->FindInstance({"TestSchema.TestClass", "D"})));
    EXPECT_CONTAINS(map, ECDbHelper::ToPair(cache->FindInstance({"TestSchema.TestClass", "E"})));
    EXPECT_CONTAINS(map, ECDbHelper::ToPair(cache->FindInstance({"TestSchema.TestClass", "F"})));
    }

TEST_F(DataSourceCacheTests, ReadRootSyncDate_NoRoot_ReturnsInvalid)
    {
    auto cache = GetTestCache();
    EXPECT_FALSE(cache->ReadRootSyncDate("NonExistingRoot").IsValid());
    }

TEST_F(DataSourceCacheTests, ReadRootSyncDate_ExistingRootButDateNotSet_ReturnsInvalid)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Foo", {"TestSchema.TestClass", "A"}));
    EXPECT_FALSE(cache->ReadRootSyncDate("Foo").IsValid());
    }

TEST_F(DataSourceCacheTests, ReadRootSyncDate_DateSetAutomatically_ReturnsSameDate)
    {
    auto cache = GetTestCache();

    auto before = DateTime::GetCurrentTimeUtc();
    BeThreadUtilities::BeSleep(1);
    ASSERT_EQ(SUCCESS, cache->SetRootSyncDate("Foo"));
    BeThreadUtilities::BeSleep(1);
    auto after = DateTime::GetCurrentTimeUtc();

    DateTime cachedDate = cache->ReadRootSyncDate("Foo");

    EXPECT_BETWEEN(before, cachedDate, after);
    }

TEST_F(DataSourceCacheTests, ReadRootSyncDate_SpecificDateSetOnNotExistingRoot_CreatesRootReturnsSameDate)
    {
    auto cache = GetTestCache();

    DateTime syncDate(DateTime::Kind::Utc, 1989, 02, 14, 0, 0, 0);
    ASSERT_EQ(SUCCESS, cache->SetRootSyncDate("Foo", syncDate));
    EXPECT_EQ(syncDate, cache->ReadRootSyncDate("Foo"));
    }

TEST_F(DataSourceCacheTests, ReadRootSyncDate_RootRemovedAfterSyncDateWasSet_ReturnsInvalid)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Foo", {"TestSchema.TestClass", "A"}));
    ASSERT_EQ(SUCCESS, cache->SetRootSyncDate("Foo"));
    ASSERT_EQ(SUCCESS, cache->RemoveRoot("Foo"));

    EXPECT_FALSE(cache->ReadRootSyncDate("Foo").IsValid());
    }

TEST_F(DataSourceCacheTests, ReadResponseAccessDate_NoResponse_ReturnsInvalid)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache, "NonExisting");

    EXPECT_THAT(cache->ReadResponseAccessDate(responseKey).IsValid(), false);
    }

TEST_F(DataSourceCacheTests, ReadResponseAccessDate_CachedResponse_ReturnsInvalid)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));

    EXPECT_THAT(cache->ReadResponseAccessDate(responseKey).IsValid(), false);
    }

TEST_F(DataSourceCacheTests, ReadResponseAccessDate_AccessDateSetAutomatically_ReturnsSameDate)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));

    auto before = DateTime::GetCurrentTimeUtc();
    BeThreadUtilities::BeSleep(1);
    ASSERT_EQ(SUCCESS, cache->SetResponseAccessDate(responseKey));
    BeThreadUtilities::BeSleep(1);
    auto after = DateTime::GetCurrentTimeUtc();

    DateTime cachedDate = cache->ReadResponseAccessDate(responseKey);
    EXPECT_BETWEEN(before, cachedDate, after);
    }

TEST_F(DataSourceCacheTests, ReadResponseAccessDate_AccessDateSet_ReturnsSameDate)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));

    DateTime date(DateTime::Kind::Utc, 1989, 02, 14, 0, 0, 0);
    ASSERT_EQ(SUCCESS, cache->SetResponseAccessDate(responseKey, date));
    EXPECT_EQ(date, cache->ReadResponseAccessDate(responseKey));
    }

TEST_F(DataSourceCacheTests, SetResponseAccessDate_ResponseNotCached_ReturnsErrorAndDateNotSet)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    EXPECT_EQ(ERROR, cache->SetResponseAccessDate(responseKey));
    EXPECT_THAT(cache->ReadResponseAccessDate(responseKey).IsValid(), false);
    }

TEST_F(DataSourceCacheTests, GetResponsesContainingInstance_InstanceNotCached_ReturnsEmpty)
    {
    auto cache = GetTestCache();

    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));
    auto instance = cache->FindInstance({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->RemoveInstance({"TestSchema.TestClass", "A"}));

    auto responces = cache->GetResponsesContainingInstance(instance);
    ASSERT_EQ(0, responces.size());
    }

TEST_F(DataSourceCacheTests, GetResponsesContainingInstance_ResponceWithOneInstance_ReturnsResponceKey)
    {
    auto cache = GetTestCache();

    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));
    auto instance = cache->FindInstance({"TestSchema.TestClass", "A"});

    auto responces = cache->GetResponsesContainingInstance(instance);
    ASSERT_EQ(1, responces.size());
    EXPECT_EQ(key, *responces.begin());
    }

TEST_F(DataSourceCacheTests, GetResponsesContainingInstance_TwoResponcesWithSameInstance_ReturnsBothResponceKeys)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    auto key1 = StubCachedResponseKey(*cache, "ResponceName");
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, instances.ToWSObjectsResponse()));

    auto key2 = StubCachedResponseKey(*cache, "ResponceName2");
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key2, instances.ToWSObjectsResponse()));

    auto instance = cache->FindInstance({"TestSchema.TestClass", "A"});

    auto responces = cache->GetResponsesContainingInstance(instance);
    ASSERT_EQ(2, responces.size());
    EXPECT_EQ(StubBSet({key1, key2}), responces);
    }

TEST_F(DataSourceCacheTests, GetResponsesContainingInstance_ExistingNamePassed_ReturnsResponceKeyFilteredByName)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    auto key1 = StubCachedResponseKey(*cache, "ResponceName");
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, instances.ToWSObjectsResponse()));

    auto key2 = StubCachedResponseKey(*cache, "ResponceName2");
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key2, instances.ToWSObjectsResponse()));

    auto instance = cache->FindInstance({"TestSchema.TestClass", "A"});

    auto responces = cache->GetResponsesContainingInstance(instance, "ResponceName");
    ASSERT_EQ(1, responces.size());
    EXPECT_EQ(key1, *responces.begin());
    }

TEST_F(DataSourceCacheTests, GetResponsesContainingInstance_NonExistingNamePassed_ReturnsEmpty)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    auto key1 = StubCachedResponseKey(*cache, "ResponceName");
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, instances.ToWSObjectsResponse()));

    auto instance = cache->FindInstance({"TestSchema.TestClass", "A"});

    auto responces = cache->GetResponsesContainingInstance(instance, "NonExistingName");
    EXPECT_EQ(0, responces.size());
    }

TEST_F(DataSourceCacheTests, GetResponsesContainingInstance_InstanceAddedToResponce_ReturnsResponceKey)
    {
    auto cache = GetTestCache();

    auto instance = StubCreatedObjectInCache(*cache);
    auto key1 = StubCachedResponseKey(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().AddCreatedInstanceToResponse(key1, instance));

    auto responces = cache->GetResponsesContainingInstance(instance);
    ASSERT_EQ(1, responces.size());
    EXPECT_EQ(key1, *responces.begin());
    }

TEST_F(DataSourceCacheTests, GetResponsesContainingInstance_HolderPassed_ReturnsResponceKey)
    {
    auto cache = GetTestCache();

    ECInstanceKey parentKey = cache->FindOrCreateRoot("Foo");
    ECInstanceKey holderKey = StubInstanceInCache(*cache);
    CachedResponseKey key1(parentKey, "A", holderKey);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, instances.ToWSObjectsResponse()));

    auto instance = cache->FindInstance({"TestSchema.TestClass", "A"});

    auto responces = cache->GetResponsesContainingInstance(instance);
    EXPECT_EQ(1, responces.size());
    EXPECT_EQ(key1, *responces.begin());
    }

TEST_F(DataSourceCacheTests, GetResponsesContainingInstance_HolderInResults_ReturnsResponceKey)
    {
    auto cache = GetTestCache();

    ECInstanceKey parentKey = cache->FindOrCreateRoot("Foo");
    ECInstanceKey holderKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    CachedResponseKey key1(parentKey, "A", holderKey);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, instances.ToWSObjectsResponse()));

    auto instance = cache->FindInstance({"TestSchema.TestClass", "A"});

    auto responces = cache->GetResponsesContainingInstance(instance);
    EXPECT_EQ(1, responces.size());
    EXPECT_EQ(key1, *responces.begin());
    }

TEST_F(DataSourceCacheTests, GetResponsesContainingInstance_ParentInResults_ReturnsResponceKey)
    {
    auto cache = GetTestCache();

    ECInstanceKey parentKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    CachedResponseKey key1(parentKey, "A");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key1, instances.ToWSObjectsResponse()));

    auto instance = cache->FindInstance({"TestSchema.TestClass", "A"});

    auto responces = cache->GetResponsesContainingInstance(instance);
    EXPECT_EQ(1, responces.size());
    EXPECT_EQ(key1, *responces.begin());
    }

TEST_F(DataSourceCacheTests, RemoveTemporaryResponses_NoResponsesWithName_ReturnsSuccess)
    {
    auto cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot("Foo"), "Test");

    ASSERT_EQ(SUCCESS, cache->SetupRoot("Foo", CacheRootPersistence::Temporary));

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));

    EXPECT_THAT(cache->RemoveTemporaryResponses("Other", DateTime::GetCurrentTimeUtc()), SUCCESS);
    EXPECT_THAT(cache->IsResponseCached(responseKey), true);
    }

TEST_F(DataSourceCacheTests, RemoveTemporaryResponses_ResponsesWithSameNameButPersistent_LeavesResponses)
    {
    auto cache = GetTestCache();
    CachedResponseKey responseKey1(cache->FindOrCreateRoot("A"), "Test");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("B"), "Test");
    CachedResponseKey responseKey3(cache->FindOrCreateRoot("C"), "Test");
    CachedResponseKey responseKey4(cache->FindOrCreateRoot("D"), "Test");

    ASSERT_EQ(SUCCESS, cache->SetupRoot("A", CacheRootPersistence::Temporary));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("B", CacheRootPersistence::Temporary));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("C", CacheRootPersistence::Full));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("D", CacheRootPersistence::Full));

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey1, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey2, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey3, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey4, StubInstances().ToWSObjectsResponse()));

    EXPECT_THAT(cache->RemoveTemporaryResponses("Test", DateTime::GetCurrentTimeUtc()), SUCCESS);

    EXPECT_THAT(cache->IsResponseCached(responseKey1), false);
    EXPECT_THAT(cache->IsResponseCached(responseKey2), false);
    EXPECT_THAT(cache->IsResponseCached(responseKey3), true);
    EXPECT_THAT(cache->IsResponseCached(responseKey4), true);
    }

TEST_F(DataSourceCacheTests, RemoveTemporaryResponses_ResponseWithAccessDateLaterThanUsed_LeavesResponse)
    {
    auto cache = GetTestCache();
    CachedResponseKey responseKey1(cache->FindOrCreateRoot("A"), "Test");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("B"), "Test");

    ASSERT_EQ(SUCCESS, cache->SetupRoot("A", CacheRootPersistence::Temporary));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("A", CacheRootPersistence::Temporary));

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey1, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey2, StubInstances().ToWSObjectsResponse()));

    ASSERT_EQ(SUCCESS, cache->SetResponseAccessDate(responseKey1, DateTime(DateTime::Kind::Utc, 1989, 02, 14, 0, 0, 0)));
    ASSERT_EQ(SUCCESS, cache->SetResponseAccessDate(responseKey2, DateTime(DateTime::Kind::Utc, 2010, 01, 01, 0, 0, 0)));

    EXPECT_THAT(cache->RemoveTemporaryResponses("Test", DateTime(DateTime::Kind::Utc, 2000, 01, 01, 0, 0, 0)), SUCCESS);
    EXPECT_THAT(cache->IsResponseCached(responseKey1), false);
    EXPECT_THAT(cache->IsResponseCached(responseKey2), true);
    }

TEST_F(DataSourceCacheTests, RemoveResponses_ResponsesWithSameName_RemovesResponses)
    {
    auto cache = GetTestCache();
    CachedResponseKey responseKey1(cache->FindOrCreateRoot("A"), "Test");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("B"), "Test");
    CachedResponseKey responseKey3(cache->FindOrCreateRoot("C"), "Other");

    ASSERT_EQ(SUCCESS, cache->SetupRoot("A", CacheRootPersistence::Temporary));

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey1, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey2, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(responseKey3, StubInstances().ToWSObjectsResponse()));

    ASSERT_EQ(SUCCESS, cache->RemoveResponses("Test"));

    EXPECT_THAT(cache->IsResponseCached(responseKey1), false);
    EXPECT_THAT(cache->IsResponseCached(responseKey2), false);
    EXPECT_THAT(cache->IsResponseCached(responseKey3), true);
    }

TEST_F(DataSourceCacheTests, GetExtendedData_UpdatedDataForObject_WorksFine)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache);

    auto extendedData = cache->GetExtendedDataAdapter().GetData(instance);
    extendedData.SetValue("A", "B");
    ASSERT_EQ(SUCCESS, cache->GetExtendedDataAdapter().UpdateData(extendedData));

    auto extendedData2 = cache->GetExtendedDataAdapter().GetData(instance);
    EXPECT_THAT(extendedData2.GetValue("A"), Eq("B"));
    }

TEST_F(DataSourceCacheTests, GetExtendedData_UpdatedDataForRelationship_WorksFine)
    {
    auto cache = GetTestCache();
    auto relationship = StubRelationshipInCache(*cache);

    auto extendedData = cache->GetExtendedDataAdapter().GetData(relationship);
    extendedData.SetValue("A", "B");
    ASSERT_EQ(SUCCESS, cache->GetExtendedDataAdapter().UpdateData(extendedData));

    auto extendedData2 = cache->GetExtendedDataAdapter().GetData(relationship);
    EXPECT_THAT(extendedData2.GetValue("A"), Eq("B"));
    }

TEST_F(DataSourceCacheTests, GetExtendedData_UpdatedDataForRoot_WorksFine)
    {
    auto cache = GetTestCache();
    auto root = cache->FindOrCreateRoot("Boo");

    auto extendedData = cache->GetExtendedDataAdapter().GetData(root);
    extendedData.SetValue("A", "B");
    ASSERT_EQ(SUCCESS, cache->GetExtendedDataAdapter().UpdateData(extendedData));

    auto extendedData2 = cache->GetExtendedDataAdapter().GetData(root);
    EXPECT_THAT(extendedData2.GetValue("A"), Eq("B"));
    }

TEST_F(DataSourceCacheTests, GetExtendedData_UpdatedDataForNavigationBase_WorksFine)
    {
    auto cache = GetTestCache();
    auto navigationBase = StubInstanceInCache(*cache, ObjectId());

    auto extendedData = cache->GetExtendedDataAdapter().GetData(navigationBase);
    extendedData.SetValue("A", "B");
    ASSERT_EQ(SUCCESS, cache->GetExtendedDataAdapter().UpdateData(extendedData));

    auto extendedData2 = cache->GetExtendedDataAdapter().GetData(navigationBase);
    EXPECT_THAT(extendedData2.GetValue("A"), Eq("B"));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsInstanceThatWasLocallyModified_UpdatesPropertiesThatWereNotChanged)
    {
    auto cache = GetTestCache();

    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}, {"TestProperty2", "B"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));
    auto instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});

    auto properties = ToJson(R"({"TestProperty" : "A", "TestProperty2" : "ModifiedValueB"})");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "NewValueA"}, {"TestProperty2", "NewValueB"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instanceJson));

    EXPECT_EQ("NewValueA", instanceJson["TestProperty"].asString());
    EXPECT_EQ("ModifiedValueB", instanceJson["TestProperty2"].asString());
    }

TEST_F(DataSourceCacheTests, CacheResponse_FullResultContainsInstanceThatWasLocallyModified_UpdatesPropertiesThatWereNotChanged)
    {
    auto cache = GetTestCache();

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*");

    bset<ObjectId> rejected;
    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}, {"TestProperty2", "B"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), &rejected, &query));
    auto instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    ASSERT_TRUE(rejected.empty());

    auto properties = ToJson(R"({"TestProperty" : "A", "TestProperty2" : "ModifiedValueB"})");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    rejected.clear();
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "NewValueA"}, {"TestProperty2", "NewValueB"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), &rejected, &query));
    ASSERT_TRUE(rejected.empty());

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instanceJson));

    EXPECT_EQ("NewValueA", instanceJson["TestProperty"].asString());
    EXPECT_EQ("ModifiedValueB", instanceJson["TestProperty2"].asString());
    }

TEST_F(DataSourceCacheTests, CacheResponse_PartialResultsContainInstanceThatWasLocallyModified_RejectsAndDoesNotUpdateInstance)
    {
    auto cache = GetTestCache();

    bset<ObjectId> rejected;
    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}, {"TestProperty2", "B"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), &rejected, nullptr));
    auto instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    ASSERT_TRUE(rejected.empty());

    auto properties = ToJson(R"({"TestProperty" : "A", "TestProperty2" : "ModifiedValueB"})");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("$id,TestProperty");

    rejected.clear();
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "NewValueA"}, {"TestProperty2", "NewValueB"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), &rejected, &query));
    EXPECT_EQ(1, rejected.size());
    EXPECT_CONTAINS(rejected, ObjectId("TestSchema.TestClass", "Foo"));

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instanceJson));

    EXPECT_EQ("A", instanceJson["TestProperty"].asString());
    EXPECT_EQ("ModifiedValueB", instanceJson["TestProperty2"].asString());
    }

TEST_F(DataSourceCacheTests, CacheResponse_PartialResultsContainInstanceWihtIdOnlyThatWasLocallyModified_DoesNotRejectOrUpdateInstance)
    {
    auto cache = GetTestCache();

    bset<ObjectId> rejected;
    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}, {"TestProperty2", "B"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), &rejected, nullptr));
    auto instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    ASSERT_TRUE(rejected.empty());

    auto properties = ToJson(R"({"TestProperty" : "A", "TestProperty2" : "ModifiedValueB"})");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("$id");

    rejected.clear();
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "NewValueA"}, {"TestProperty2", "NewValueB"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), &rejected, &query));
    EXPECT_EQ(0, rejected.size());

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instanceJson));

    EXPECT_EQ("A", instanceJson["TestProperty"].asString());
    EXPECT_EQ("ModifiedValueB", instanceJson["TestProperty2"].asString());
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultNoLongerContainsInstanceThatWasLocallyModified_RemovesModifiedInstance)
    {
    auto cache = GetTestCache();

    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    auto instance = cache->FindInstance({"TestSchema.TestClass", "A"});
    auto properties = ToJson(R"({"TestProperty" : "Modified"})");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    ASSERT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    ASSERT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    ASSERT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    ASSERT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, CacheResponse_FullResultContainsInstanceThatWasLocallyDeleted_IgnoresInstance)
    {
    auto cache = GetTestCache();

    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*");
    bset<ObjectId> rejected;
    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), &rejected, &query));
    ASSERT_TRUE(rejected.empty());

    Json::Value instanceJson;
    EXPECT_EQ(CacheStatus::DataNotCached, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instanceJson));
    EXPECT_EQ(Json::Value::GetNull(), instanceJson);
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsInstanceThatWasLocallyDeletedAndRelatedInstance_IgnoresDeletedInstanceAndUpdatesRelatedOne)
    {
    auto cache = GetTestCache();

    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "OldA"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {{"TestProperty", "OldB"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(cache->FindInstance({"TestSchema.TestClass", "A"})));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "NewA"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {{"TestProperty", "NewB"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    Json::Value instanceJson;
    EXPECT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "B"}, instanceJson));
    EXPECT_EQ("NewB", instanceJson["TestProperty"].asString());

    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChangeStatus(cache->FindInstance({"TestSchema.TestClass", "A"})));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsRelatedInstanceThatWasLocallyDeleted_IgnoresDeletedInstance)
    {
    auto cache = GetTestCache();

    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "OldA"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {{"TestProperty", "OldB"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(cache->FindInstance({"TestSchema.TestClass", "B"})));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "NewA"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {{"TestProperty", "NewB"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    Json::Value instanceJson;
    EXPECT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "A"}, instanceJson));
    EXPECT_EQ("NewA", instanceJson["TestProperty"].asString());

    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChangeStatus(cache->FindInstance({"TestSchema.TestClass", "B"})));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsMultipleRelationshipsToRelatedInstanceThatWasLocallyDeleted_IgnoresDeletedInstance)
    {
    auto cache = GetTestCache();

    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    instances.Add({"TestSchema.TestClass", "C"})
        .AddRelated({"TestSchema.TestRelationshipClass", "CB"}, {"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(cache->FindInstance({"TestSchema.TestClass", "B"})));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChangeStatus(cache->FindInstance({"TestSchema.TestClass", "B"})));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsRelatedInstanceThatWasLocallyDeleted_CachedResponseContainsDeletedInstanceAlso)
    {
    auto cache = GetTestCache();

    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(cache->FindInstance({"TestSchema.TestClass", "B"})));
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    bset<ObjectId> objectIds;
    EXPECT_EQ(CacheStatus::OK, cache->ReadResponseObjectIds(key, objectIds));
    EXPECT_EQ(2, objectIds.size());
    EXPECT_CONTAINS(objectIds, ObjectId("TestSchema.TestClass", "A"));
    EXPECT_CONTAINS(objectIds, ObjectId("TestSchema.TestClass", "B"));

    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChangeStatus(cache->FindInstance({"TestSchema.TestClass", "B"})));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultNoLongerContainsRelatedInstanceThatWasLocallyDeleted_RemovesDeletedInstanceFromCache)
    {
    auto cache = GetTestCache();

    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(cache->FindInstance({"TestSchema.TestClass", "B"})));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    bset<ObjectId> objectIds;
    EXPECT_EQ(CacheStatus::OK, cache->ReadResponseObjectIds(key, objectIds));
    EXPECT_EQ(1, objectIds.size());
    EXPECT_CONTAINS(objectIds, ObjectId("TestSchema.TestClass", "A"));
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_PartialResultsContainsInstanceThatWasLocallyDeleted_IgnoresInstance)
    {
    auto cache = GetTestCache();

    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));

    bset<ObjectId> rejected;
    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), &rejected, nullptr));
    ASSERT_TRUE(rejected.empty());

    Json::Value instanceJson;
    EXPECT_EQ(CacheStatus::DataNotCached, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instanceJson));
    EXPECT_EQ(Json::Value::GetNull(), instanceJson);

    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChangeStatus(cache->FindInstance({"TestSchema.TestClass", "Foo"})));
    }

TEST_F(DataSourceCacheTests, CacheResponse_PartialResultsContainsInstanceThatWasCachedAsPartial_UpdatesInstance)
    {
    auto cache = GetTestCache();

    bset<ObjectId> rejected;
    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), &rejected, nullptr));
    auto instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    ASSERT_TRUE(rejected.empty());

    rejected.clear();
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "NewValueA"}});
    ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), &rejected, nullptr));
    EXPECT_EQ(0, rejected.size());

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instanceJson));

    EXPECT_EQ("NewValueA", instanceJson["TestProperty"].asString());
    }

#endif
