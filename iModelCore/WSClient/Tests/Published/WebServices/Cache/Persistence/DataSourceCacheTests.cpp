/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Persistence/DataSourceCacheTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DataSourceCacheTests.h"

#include "../Util/MockECDbSchemaChangeListener.h"
#include <Bentley/BeDebugLog.h>

#ifdef USE_GTEST
using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

using std::shared_ptr;

Json::Value ReadInstance(IDataSourceCachePtr cache, ObjectIdCR objectId)
    {
    Json::Value instanceJson;
    cache->ReadInstance(objectId, instanceJson);
    return instanceJson;
    }

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

TEST_F(DataSourceCacheTests, UpdateSchemas_EmptyVectorPassed_DoesNothingAndSucceeds)
    {
    DataSourceCache cache;
    cache.Create(BeFileName(":memory:"), CacheEnvironment());

    BentleyStatus result = cache.UpdateSchemas(std::vector<BeFileName> {});

    EXPECT_EQ(SUCCESS, result);
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_PathPasssed_SuccessAndSchemaAccessable)
    {
    DataSourceCache cache;
    cache.Create(BeFileName(":memory:"), CacheEnvironment());

    BeFileName schemaPath = GetTestSchemaPath();
    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<BeFileName> {schemaPath}));

    EXPECT_TRUE(nullptr != cache.GetAdapter().GetECSchema("TestSchema"));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_SchemasPassed_SuccessAndSchemasAccessable)
    {
    DataSourceCache cache;
    cache.Create(BeFileName(":memory:"), CacheEnvironment());
    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema(), GetTestSchema2() }));

    EXPECT_TRUE(nullptr != cache.GetAdapter().GetECSchema("TestSchema"));
    EXPECT_TRUE(nullptr != cache.GetAdapter().GetECSchema("TestSchema2"));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_SchemasPassedToDataSourceCacheWithCachedStatements_SuccessAndSchemasAccessable)
    {
    DataSourceCache cache;
    cache.Create(BeFileName(":memory:"), CacheEnvironment());

    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema()}));
    ASSERT_TRUE(nullptr != cache.GetAdapter().GetECSchema("TestSchema"));

    ASSERT_EQ(SUCCESS, cache.LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "A"}));
    ASSERT_TRUE(cache.FindInstance({"TestSchema.TestClass", "A"}).IsValid());

    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema2()}));
    EXPECT_TRUE(nullptr != cache.GetAdapter().GetECSchema("TestSchema2"));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_SchemasWithDeletedPropertyPassedToDataSourceCacheWithCachedStatements_SuccessAndSchemasAccessable)
    {
    DataSourceCache cache;
    cache.Create(BeFileName(":memory:"), CacheEnvironment());

    auto schema1 = ParseSchema(
        R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="A" typeName="string" />
                <ECProperty propertyName="B" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    auto schema2 = ParseSchema(
        R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="A" typeName="string" />
            </ECClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<ECSchemaPtr> {schema1}));
    ASSERT_TRUE(nullptr != cache.GetAdapter().GetECSchema("TestSchema"));

    ASSERT_EQ(SUCCESS, cache.LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "Foo"}));
    ASSERT_TRUE(cache.FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());

    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<ECSchemaPtr> {schema2}));
    EXPECT_TRUE(nullptr != cache.GetAdapter().GetECSchema("TestSchema"));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_NullSchemaPassed_Error)
    {
    DataSourceCache cache;
    cache.Create(BeFileName(":memory:"), CacheEnvironment());

    BeTest::SetFailOnAssert(false);
    BentleyStatus result = cache.UpdateSchemas(std::vector<ECSchemaPtr> {nullptr});
    BeTest::SetFailOnAssert(true);

    EXPECT_EQ(ERROR, result);
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_SchemaChangeListenerRegisteredAndSchemaPassed_CallsListenerBeforeAndAfterSchemaUpdate)
    {
    MockECDbSchemaChangeListener listener;

    DataSourceCache cache;
    cache.Create(BeFileName(":memory:"), CacheEnvironment());
    cache.RegisterSchemaChangeListener(&listener);

    EXPECT_CALL(listener, OnSchemaChanged()).Times(2);
    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema()}));
    }

TEST_F(DataSourceCacheTests, UpdateSchemas_CalledOnOtherConnection_CallsListenerOnceTransactionIsStarted)
    {
    BeFileName path = StubFilePath();
    ECDb::CreateParams params;
    params.SetStartDefaultTxn(DefaultTxn::No);

    DataSourceCache cache1;
    DataSourceCache cache2;

    ASSERT_EQ(SUCCESS, cache1.Create(path, StubCacheEnvironemnt(), params));
    ASSERT_EQ(SUCCESS, cache2.Open(path, StubCacheEnvironemnt(), params));

    MockECDbSchemaChangeListener listener1;
    MockECDbSchemaChangeListener listener2;

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
    }

TEST_F(DataSourceCacheTests, GetInstance_NotCached_ReturnsDataNotCachedAndNullInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    Json::Value instance;
    CacheStatus status = cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instance);

    EXPECT_EQ(CacheStatus::DataNotCached, status);
    EXPECT_TRUE(instance.isNull());
    }

TEST_F(DataSourceCacheTests, GetInstance_LinkedToRoot_ReturnsOk)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});

    Json::Value instance;
    CacheStatus status = cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instance);

    EXPECT_EQ(CacheStatus::OK, status);
    }

void VerifyJsonRawFormat(JsonValueCR instance)
    {
    EXPECT_EQ("TestSchema.TestClass", instance[DataSourceCache_PROPERTY_ClassKey].asString());
    EXPECT_EQ("Foo", instance[DataSourceCache_PROPERTY_RemoteId].asString());

    EXPECT_TRUE(instance.isMember("$ECInstanceId"));
    EXPECT_TRUE(instance.isMember("$ECInstanceLabel"));

    EXPECT_TRUE(instance.isMember("TestProperty"));
    }

void VerifyJsonDisplayDataFormat(JsonValueCR instance)
    {
    EXPECT_EQ("TestSchema.TestClass", instance[DataSourceCache_PROPERTY_ClassKey].asString());
    EXPECT_EQ("Foo", instance[DataSourceCache_PROPERTY_RemoteId].asString());

    EXPECT_TRUE(instance.isMember("$ECInstanceId"));
    EXPECT_TRUE(instance.isMember("$ECInstanceLabel"));
    EXPECT_TRUE(instance.isMember("$ECClassLabel"));

    EXPECT_TRUE(instance.isMember("TestProperty"));
    }

TEST_F(DataSourceCacheTests, GetInstance_RawFormat_ReturnsPlaceholderInstanceWithExpectedFormat)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});

    Json::Value instance;
    cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instance, DataSourceCache::JsonFormat::Raw);

    VerifyJsonRawFormat(instance);
    }

TEST_F(DataSourceCacheTests, GetInstance_DisplayFormat_ReturnsDisplayInstanceWithExpectedFormat)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});

    Json::Value instance;
    cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instance, DataSourceCache::JsonFormat::Display);

    EXPECT_EQ("TestSchema.TestClass", instance[DataSourceCache_PROPERTY_ClassKey].asString());
    EXPECT_EQ("Foo", instance[DataSourceCache_PROPERTY_RemoteId].asString());

    EXPECT_TRUE(instance.isMember("$DisplayInfo"));
    EXPECT_TRUE(instance.isMember("$DisplayData"));
    EXPECT_TRUE(instance.isMember("$RawData"));

    EXPECT_TRUE(instance["$DisplayInfo"].isMember("Categories"));

    VerifyJsonDisplayDataFormat(instance["$DisplayData"]);
    VerifyJsonRawFormat(instance["$RawData"]);
    }

TEST_F(DataSourceCacheTests, UpdateInstance_InstanceNotInCache_ReturnsErrorAndInstanceNotCached)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    BeTest::SetFailOnAssert(false);
    BentleyStatus status = cache->UpdateInstance({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse());
    BeTest::SetFailOnAssert(true);

    EXPECT_EQ(ERROR, status);
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, UpdateInstance_InstanceInCache_SuccessfullyUpdates)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "TestValue"}});
    BentleyStatus status = cache->UpdateInstance({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse());

    EXPECT_EQ(SUCCESS, status);

    Json::Value updatedInstance;
    cache->ReadInstance({"TestSchema.TestClass", "Foo"}, updatedInstance);

    EXPECT_EQ(Utf8String("TestValue"), updatedInstance["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, GetCachedObjectInfo_InstanceNotCached_IsFullyCachedIsFalse)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, GetCachedObjectInfo_InstanceCached_IsFullyCachedIsTrue)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), "foo_root");

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, RemoveInstance_InstanceNotCached_ReturnsDataNotCached)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CacheStatus status = cache->RemoveInstance({"TestSchema.TestClass", "Foo"});

    EXPECT_EQ(CacheStatus::DataNotCached, status);
    }

TEST_F(DataSourceCacheTests, RemoveInstance_InstanceCached_Deletes)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), "foo_root");

    CacheStatus status = cache->RemoveInstance({"TestSchema.TestClass", "Foo"});

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, RemoveInstance_ChildQueryExists_DeletesQueryResults)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances parentInstance;
    parentInstance.Add({"TestSchema.TestClass", "parent"});
    cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "parent"}, parentInstance.ToWSObjectsResponse(), "foo_root");
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "parent"}), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "resultA"});
    instances.Add({"TestSchema.TestClass", "resultB"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "resultA"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "resultB"}).IsFullyCached());

    CacheStatus status = cache->RemoveInstance({"TestSchema.TestClass", "parent"});

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "parent"}).IsFullyCached());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "resultA"}).IsFullyCached());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "resultB"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, RemoveInstance_ChildQueryResultHasOtherParent_DoesNotDeleteResult)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "parentA"});
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "parentB"});

    CachedResponseKey responseKeyA(cache->FindInstance({"TestSchema.TestClass", "parentA"}), nullptr);
    CachedResponseKey responseKeyB(cache->FindInstance({"TestSchema.TestClass", "parentB"}), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "result"});
    cache->CacheResponse(responseKeyA, instances.ToWSObjectsResponse());
    cache->CacheResponse(responseKeyB, instances.ToWSObjectsResponse());

    CacheStatus status = cache->RemoveInstance({"TestSchema.TestClass", "parentA"});

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "result"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, RemoveInstance_InstanceIsInCachedQueryResults_QueryResultCacheTagIsRemoved)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse("TestTag"));
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
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});

    cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(StubFile()), FileCache::Persistent);
    BeFileName cachedFilePath = cache->ReadFilePath({"TestSchema.TestClass", "Foo"});

    EXPECT_TRUE(cachedFilePath.DoesPathExist());

    CacheStatus status = cache->RemoveInstance({"TestSchema.TestClass", "Foo"});

    EXPECT_FALSE(cachedFilePath.DoesPathExist());
    EXPECT_EQ(CacheStatus::OK, status);
    }

TEST_F(DataSourceCacheTests, RemoveInstance_ChildQueryResultInstanceIsInRoot_LeavesResultInstanceInCache)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    cache->LinkInstanceToRoot("parent_root", {"TestSchema.TestClass", "parent"});
    cache->LinkInstanceToRoot("child_root", {"TestSchema.TestClass", "resultInRoot"});

    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "parent"}), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "resultInRoot"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    CacheStatus status = cache->RemoveInstance({"TestSchema.TestClass", "parent"});

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "resultInRoot"}).IsFullyCached());
    }

//TEST_F (DataSourceCacheTests, RemoveInstance_UnderlyingTreeHasCyclicHoldingRelationships_StillRemovesAllTree)
//    {
//    shared_ptr<DataSourceCache> cache = GetTestCache ();
//    cache->LinkInstanceToRoot ("foo_root", {"TestSchema.TestClass", "parent"});
//
//    Json::Value children;
//    children["TestClass"][0][DataSourceCache_PROPERTY_RemoteId] = "childA";
//    ToRapidJson (childrenRJ1, children);
//    cache->SetChildren ({"TestSchema.TestClass", "parent"}, childrenRJ1, nullptr);
//
//    children.clear ();
//
//    children["TestClass"][0][DataSourceCache_PROPERTY_RemoteId] = "childB";
//    ToRapidJson (childrenRJ2, children);
//    cache->SetChildren ({"TestSchema.TestClass", "childA"}, childrenRJ2, nullptr);
//
//    children.clear ();
//
//    children["TestClass"][0][DataSourceCache_PROPERTY_RemoteId] = "childA";
//    ToRapidJson (childrenRJ3, children);
//    cache->SetChildren ({"TestSchema.TestClass", "childB"}, childrenRJ3, nullptr);
//
//    EXPECT_TRUE (cache->HasInstance ({"TestSchema.TestClass", "parent"}));
//    EXPECT_TRUE (cache->HasInstance ({"TestSchema.TestClass", "childB"}));
//    EXPECT_TRUE (cache->HasInstance ({"TestSchema.TestClass", "childA"}));
//
//    cache->RemoveInstance ({"TestSchema.TestClass", "parent"});
//
//    EXPECT_FALSE (cache->HasInstance ({"TestSchema.TestClass", "childB"}));
//    EXPECT_FALSE (cache->HasInstance ({"TestSchema.TestClass", "childA"}));
//    }

TEST_F(DataSourceCacheTests, RemoveInstance_ChildQueryResultInstanceIsInWeaklyLinkedToRoot_RemovesResultInstanceInCache)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "resultInRoot"});

    cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "foo_root", nullptr, true);
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "parent"});

    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "parent"}), nullptr);
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    CacheStatus status = cache->RemoveInstance({"TestSchema.TestClass", "parent"});

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "resultInRoot"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, FindInstance_NotCachedInstance_Null)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, FindInstance_PassedCachedRelationshipId_ReturnsInvalidAsFindRelationshipShouldBeUsed)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestRelationshipClass", "AB"}).IsValid());
    }

TEST_F(DataSourceCacheTests, FindCachedObject_InstanceLinkedToRoot_IsNotNull)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    BentleyStatus status = cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, LinkInstanceToRoot_NotCachedInstance_HasInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    BentleyStatus status = cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, UnlinkInstanceFromRoot_ExistingInstanceLinkedToRoot_InstanceRemoved)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("A", {"TestSchema.TestClass", "Foo"}));

    EXPECT_EQ(SUCCESS, cache->UnlinkInstanceFromRoot("A", {"TestSchema.TestClass", "Foo"}));
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, UnlinkInstanceFromRoot_ExistingInstanceButLinkedToOtherRoot_InstanceNotRemoved)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("A", {"TestSchema.TestClass", "IdA"}));
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("B", {"TestSchema.TestClass", "IdB"}));

    EXPECT_EQ(SUCCESS, cache->UnlinkInstanceFromRoot("A", {"TestSchema.TestClass", "IdB"}));
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "IdA"}).IsValid());
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "IdB"}).IsValid());
    }

TEST_F(DataSourceCacheTests, UnlinkInstanceFromRoot_ExistingInstanceLinkedAlsoToOtherRoot_InstanceNotRemoved)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("A", {"TestSchema.TestClass", "Foo"}));
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("B", {"TestSchema.TestClass", "Foo"}));

    EXPECT_EQ(SUCCESS, cache->UnlinkInstanceFromRoot("A", {"TestSchema.TestClass", "Foo"}));
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, UnlinkInstanceFromRoot_NotCachedInstance_Suceeds)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("A", {"TestSchema.TestClass", "Foo"}));

    EXPECT_EQ(SUCCESS, cache->UnlinkInstanceFromRoot("A", {"TestSchema.TestClass", "NotExistingInstance"}));
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, UnlinkInstanceFromRoot_NonExistingRoot_SuceedsAndKeepsInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    EXPECT_EQ(SUCCESS, cache->LinkInstanceToRoot("A", {"TestSchema.TestClass", "Foo"}));

    EXPECT_EQ(SUCCESS, cache->UnlinkInstanceFromRoot("NonExistingRoot", {"TestSchema.TestClass", "Foo"}));
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheInstanceAndLinkToRoot_ObjectIdDoesNotMachWSObjectsResponse_ErrorIsReturned)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "DifferentId"});

    BeTest::SetFailOnAssert(false);
    BentleyStatus status = cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "SomeOtherId"}, instances.ToWSObjectsResponse(), nullptr);
    BeTest::SetFailOnAssert(true);

    EXPECT_EQ(ERROR, status);
    }

TEST_F(DataSourceCacheTests, CacheInstanceAndLinkToRoot_WSObjectResultPassed_InstanceInRootAndFullyCached)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    BentleyStatus status = cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), "Root");

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    EXPECT_TRUE(cache->IsInstanceInRoot("Root", cache->FindInstance({"TestSchema.TestClass", "Foo"})));
    }

TEST_F(DataSourceCacheTests, CacheInstanceAndLinkToRoot_InstanceJsonPassed_InstanceInRootAndFullyCached)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    rapidjson::Value instanceJson(rapidjson::kObjectType);
    BentleyStatus status = cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, instanceJson, "CacheTag", "Root");

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    EXPECT_TRUE(cache->IsInstanceInRoot("Root", cache->FindInstance({"TestSchema.TestClass", "Foo"})));
    }

TEST_F(DataSourceCacheTests, CacheInstancesAndLinkToRoot_OneInstance_InstanceCached)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    BentleyStatus status = cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "Root");

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    EXPECT_TRUE(cache->IsInstanceInRoot("Root", cache->FindInstance({"TestSchema.TestClass", "Foo"})));
    }

TEST_F(DataSourceCacheTests, CacheInstancesAndLinkToRoot_OneInstanceWithWeakLink_InstanceCached)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    BentleyStatus status = cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "Root", nullptr, true);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheInstancesAndLinkToRoot_OneInstanceWithETag_ETagIsCached)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    ObjectIdCR objectId {"TestSchema.TestClass", "Id"};
    instances.Add(objectId, {}, "TestTag");

    cache->CacheInstanceAndLinkToRoot(objectId, instances.ToWSObjectsResponse(), nullptr);

    EXPECT_EQ("TestTag", cache->ReadInstanceCacheTag(objectId));
    }

TEST_F(DataSourceCacheTests, RemoveRoot_RootHasLinkedInstance_InstanceRemovedFromCache)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});

    BentleyStatus status = cache->RemoveRoot("foo_root");

    EXPECT_EQ(SUCCESS, status);
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, RemoveRoot_RootHasWeaklyLinkedInstance_InstanceRemovedFromCache)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "foo_root", nullptr, true);

    BentleyStatus status = cache->RemoveRoot("foo_root");

    EXPECT_EQ(SUCCESS, status);
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, RemoveRoot_InstanceLinkedToSeveralRoots_InstanceNotRemoved)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("root1", {"TestSchema.TestClass", "Foo"});
    cache->LinkInstanceToRoot("root2", {"TestSchema.TestClass", "Foo"});

    BentleyStatus status = cache->RemoveRoot("root1");

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, RemoveRoot_RootContainsCachedQueryWithCyclicRelationshipToItsParent_QueryResultsAndParentDeleted)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

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
    shared_ptr<DataSourceCache> cache = GetTestCache();
    BentleyStatus status = cache->RemoveRootsByPrefix("Foo");
    EXPECT_EQ(SUCCESS, status);
    }

TEST_F(DataSourceCacheTests, RemoveRootsByPrefix_RootsWithLinkedInstancesExist_DeletesOnlyPrefixedRootLinkedInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Test", {"TestSchema.TestClass", "A"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("TestFoo", {"TestSchema.TestClass", "B"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("BooTestFoo", {"TestSchema.TestClass", "C"}));
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("FooTest", {"TestSchema.TestClass", "D"}));

    BentleyStatus status = cache->RemoveRootsByPrefix("Test");

    EXPECT_EQ(SUCCESS, status);
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "C"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "D"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, IsInstanceConnectedToRoot_InstanceIsRootChild_True)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("root", {"TestSchema.TestClass", "Foo"});

    EXPECT_TRUE(cache->IsInstanceConnectedToRoot("root", cache->FindInstance({"TestSchema.TestClass", "Foo"})));
    }

TEST_F(DataSourceCacheTests, ReadInstancesLinkedToRoot_RootNotUsed_ReturnsSuccessAndNoInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    Json::Value instances;
    BentleyStatus status = cache->ReadInstancesLinkedToRoot("NotUsedRoot", instances);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, instances.size());
    }

TEST_F(DataSourceCacheTests, ReadInstancesLinkedToRoot_TwoDifferentClassInstancesLinkedToRoot_ReturnsInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "A"});
    cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass2", "B"});

    Json::Value instances;
    BentleyStatus status = cache->ReadInstancesLinkedToRoot("TestRoot", instances);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(2, instances.size());
    EXPECT_THAT(cache->ObjectIdFromJsonInstance(instances[0]), AnyOf(ObjectId("TestSchema.TestClass", "A"), ObjectId("TestSchema.TestClass2", "B")));
    EXPECT_THAT(cache->ObjectIdFromJsonInstance(instances[1]), AnyOf(ObjectId("TestSchema.TestClass", "A"), ObjectId("TestSchema.TestClass2", "B")));
    }

TEST_F(DataSourceCacheTests, DeleteFilesInTemporaryPersistence_RootPersistenceSetToFull_LeavesFile)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});
    cache->SetupRoot("foo_root", CacheRootPersistence::Full);
    cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(StubFile()), FileCache::Persistent);

    EXPECT_TRUE(cache->ReadFilePath({"TestSchema.TestClass", "Foo"}).DoesPathExist());

    BentleyStatus status = cache->RemoveFilesInTemporaryPersistence();

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->ReadFilePath({"TestSchema.TestClass", "Foo"}).DoesPathExist());
    }

TEST_F(DataSourceCacheTests, DeleteFilesInTemporaryPersistence_RootPersistenceSetToTemporaryAndFileCachedToPersistent_DeletesFile)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});
    cache->SetupRoot("foo_root", CacheRootPersistence::Temporary);
    cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(StubFile()), FileCache::Persistent);

    EXPECT_TRUE(cache->ReadFilePath({"TestSchema.TestClass", "Foo"}).DoesPathExist());

    BentleyStatus status = cache->RemoveFilesInTemporaryPersistence();

    EXPECT_EQ(SUCCESS, status);
    EXPECT_FALSE(cache->ReadFilePath({"TestSchema.TestClass", "Foo"}).DoesPathExist());
    }

TEST_F(DataSourceCacheTests, DeleteFilesInTemporaryPersistence_ModifiedFileExists_LeavesModifiedFile)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});
    cache->SetupRoot("foo_root", CacheRootPersistence::Temporary);
    auto instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});

    cache->GetChangeManager().ModifyFile(instance, StubFile(), false);
    EXPECT_TRUE(cache->ReadFilePath(instance).DoesPathExist());

    BentleyStatus status = cache->RemoveFilesInTemporaryPersistence();

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->ReadFilePath(instance).DoesPathExist());
    }

TEST_F(DataSourceCacheTests, Reset_InstanceLinkedToRoot_InstanceRemoved)
    {
    // Arrange
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});
    // Act
    ASSERT_EQ(SUCCESS, cache->Reset());
    // Assert
    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, Reset_NullRootRemovedWithNavigationBase_NavigationBaseIsNotInCache)
    {
    // Arrange
    shared_ptr<DataSourceCache> cache = GetTestCache();
    // Act
    ASSERT_EQ(SUCCESS, cache->Reset());
    // Assert
    EXPECT_FALSE(cache->FindInstance(ObjectId()).IsValid());
    }

TEST_F(DataSourceCacheTests, Reset_FileCachedBefore_CachesNewFileToSameLocation)
    {
    // Arrange
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});
    cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(StubFile()), FileCache::Temporary);
    BeFileName oldCachedPath = cache->ReadFilePath({"TestSchema.TestClass", "Foo"});
    // Act
    BentleyStatus status = cache->Reset();
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "other_foo"});
    cache->CacheFile({"TestSchema.TestClass", "other_foo"}, StubWSFileResponse(StubFile()), FileCache::Temporary);
    BeFileName newCachedPath = cache->ReadFilePath({"TestSchema.TestClass", "other_foo"});
    // Assert
    EXPECT_EQ(SUCCESS, status);
    EXPECT_FALSE(oldCachedPath.empty());
    EXPECT_FALSE(newCachedPath.empty());
    EXPECT_FALSE(oldCachedPath.DoesPathExist());
    EXPECT_EQ(Utf8String(BeFileName::GetDirectoryName(oldCachedPath)),
              Utf8String(BeFileName::GetDirectoryName(newCachedPath)));
    }

TEST_F(DataSourceCacheTests, IsObjectFullyPersisted_NonExistingObject_False)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    EXPECT_FALSE(cache->IsInstanceFullyPersisted({"TestSchema.TestClass", "NonExistant"}));
    }

TEST_F(DataSourceCacheTests, IsObjectFullyPersisted_ObjectInFullPersistenceRoot_True)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->SetupRoot("foo_root", CacheRootPersistence::Full);
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});

    EXPECT_TRUE(cache->IsInstanceFullyPersisted({"TestSchema.TestClass", "Foo"}));
    }

TEST_F(DataSourceCacheTests, IsObjectFullyPersisted_ObjectInTemporaryPersistenceRoot_False)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->SetupRoot("foo_root", CacheRootPersistence::Temporary);
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});

    EXPECT_FALSE(cache->IsInstanceFullyPersisted({"TestSchema.TestClass", "Foo"}));
    }

TEST_F(DataSourceCacheTests, IsObjectFullyPersisted_ObjectQueryParentInFullPersistenceRoot_True)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    cache->SetupRoot("foo_root", CacheRootPersistence::Full);
    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "parent"});
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "parent"}), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    EXPECT_TRUE(cache->IsInstanceFullyPersisted({"TestSchema.TestClass", "Foo"}));
    }

TEST_F(DataSourceCacheTests, IsObjectFullyPersisted_ObjectInTemporaryAndFullPersistenceRoots_True)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->SetupRoot("temp_root", CacheRootPersistence::Temporary);
    cache->SetupRoot("full_root", CacheRootPersistence::Full);
    cache->LinkInstanceToRoot("temp_root", {"TestSchema.TestClass", "Foo"});
    cache->LinkInstanceToRoot("full_root", {"TestSchema.TestClass", "Foo"});

    EXPECT_TRUE(cache->IsInstanceFullyPersisted({"TestSchema.TestClass", "Foo"}));
    }

TEST_F(DataSourceCacheTests, SetupRoot_NewRoot_Succeeds)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    EXPECT_EQ(SUCCESS, cache->SetupRoot("foo", CacheRootPersistence::Temporary));
    EXPECT_EQ(SUCCESS, cache->SetupRoot("boo", CacheRootPersistence::Full));
    }

TEST_F(DataSourceCacheTests, DoesRootExist_NonExistingRoot_FalseAndRootIsNotCreated)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    EXPECT_FALSE(cache->DoesRootExist("Foo"));
    EXPECT_FALSE(cache->DoesRootExist("Foo"));
    }

TEST_F(DataSourceCacheTests, DoesRootExist_ExistingRoot_True)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ASSERT_TRUE(cache->FindOrCreateRoot("Foo").IsValid());

    EXPECT_TRUE(cache->DoesRootExist("Foo"));
    }

TEST_F(DataSourceCacheTests, FindOrCreateRoot_NonExistingRoot_CreatesNewRoot)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    EXPECT_TRUE(cache->FindOrCreateRoot("Foo").IsValid());
    }

TEST_F(DataSourceCacheTests, FindOrCreateRoot_DifferentRoot_CreatesNewRoot)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto rootKeyA = cache->FindOrCreateRoot("A");
    auto rootKeyB = cache->FindOrCreateRoot("B");

    EXPECT_TRUE(rootKeyA.IsValid());
    EXPECT_TRUE(rootKeyB.IsValid());
    EXPECT_THAT(rootKeyA, Not(rootKeyB));
    }

TEST_F(DataSourceCacheTests, FindOrCreateRoot_ExistingRoot_ReturnsSameKey)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

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
    shared_ptr<DataSourceCache> cache = GetTestCache();

    BeTest::SetFailOnAssert(false);
    BentleyStatus status = cache->UpdateInstances(StubWSObjectsResponseNotModified());
    BeTest::SetFailOnAssert(true);

    EXPECT_EQ(ERROR, status);
    }

TEST_F(DataSourceCacheTests, UpdateInstances_NotExistingInstance_InstanceInsertedIntoRejected)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    bset<ObjectId> notFound;
    notFound.insert({"TestSchema.TestClass", "Test"});
    BentleyStatus status = cache->UpdateInstances(instances.ToWSObjectsResponse(), &notFound);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(2, notFound.size());
    EXPECT_CONTAINS(notFound, ObjectId("TestSchema.TestClass", "Foo"));
    EXPECT_CONTAINS(notFound, ObjectId("TestSchema.TestClass", "Test"));
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, UpdateInstances_ExistingInstance_Caches)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    cache->LinkInstanceToRoot("foo_root", {"TestSchema.TestClass", "Foo"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    bset<ObjectId> notFound;
    BentleyStatus status = cache->UpdateInstances(instances.ToWSObjectsResponse(), &notFound);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, notFound.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, UpdateInstances_ExistingInstance_InsertsIntoCachedInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    bset<ECInstanceKey> cachedInstances;
    cachedInstances.insert(StubECInstanceKey(0, 42));
    BentleyStatus status = cache->UpdateInstances(instances.ToWSObjectsResponse(), nullptr, &cachedInstances);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(2, cachedInstances.size());
    EXPECT_CONTAINS(cachedInstances, instanceKey);
    EXPECT_CONTAINS(cachedInstances, StubECInstanceKey(0, 42));
    }

TEST_F(DataSourceCacheTests, CacheInstancesAndLinkToRoot_NewInstnaces_ReturnsCachedInstanceKeys)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass2", "B"});

    ECInstanceKeyMultiMap instanceKeys;
    instanceKeys.insert({1, ECInstanceId(1234)});
    ASSERT_EQ(SUCCESS, cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "root", &instanceKeys));

    EXPECT_THAT(instanceKeys, SizeIs(3));
    EXPECT_THAT(instanceKeys, Contains(ECDbHelper::ToPair(ECInstanceKey(1, ECInstanceId(1234)))));
    EXPECT_THAT(instanceKeys, Contains(ECDbHelper::ToPair(cache->FindInstance({"TestSchema.TestClass", "A"}))));
    EXPECT_THAT(instanceKeys, Contains(ECDbHelper::ToPair(cache->FindInstance({"TestSchema.TestClass2", "B"}))));
    }

TEST_F(DataSourceCacheTests, ReadInstance_NavigationBaseObjectId_ErrorAndNullJson)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, ObjectId()));

    Json::Value instance(Json::objectValue);
    EXPECT_THAT(cache->ReadInstance(ObjectId(), instance), CacheStatus::Error);
    EXPECT_THAT(instance.isNull(), true);
    }

TEST_F(DataSourceCacheTests, ReadInstance_NotExistingObjectId_ErrorAndNullJson)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    Json::Value instance(Json::objectValue);
    EXPECT_THAT(cache->ReadInstance(ObjectId("TestSchema.TestClass", "NonExisting"), instance), CacheStatus::DataNotCached);
    EXPECT_THAT(instance.isNull(), true);
    }

TEST_F(DataSourceCacheTests, ReadInstance_ExistingObjectId_ReturnsJsonInstanceWithRemoteIdAndValues)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
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
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, ObjectId()));

    auto instance = cache->ReadInstance(ObjectId());
    EXPECT_THAT(instance.IsNull(), true);
    }

TEST_F(DataSourceCacheTests, ReadInstance_NotExistingObjectId_ReturnsNull)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto instance = cache->ReadInstance(ObjectId("TestSchema.TestClass", "NonExisting"));
    EXPECT_THAT(instance.IsNull(), true);
    }

TEST_F(DataSourceCacheTests, ReadInstance_ExistingObjectId_ReturnsECInstanceWithRemoteIdAndValues)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
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
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, ObjectId()));

    auto instanceKey = cache->FindInstance(ObjectId());
    ASSERT_TRUE(instanceKey.IsValid());

    auto instance = cache->ReadInstance(instanceKey);
    EXPECT_THAT(instance.IsNull(), true);
    }

TEST_F(DataSourceCacheTests, ReadInstance_ExistingInstanceKey_ReturnsECInstanceWithRemoteIdAndValues)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
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
    shared_ptr<DataSourceCache> cache = GetTestCache();

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
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "NonExisting"}), nullptr);
    EXPECT_NE(0, responseKey.GetParent().GetECClassId());

    auto status = cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse());
    EXPECT_EQ(ERROR, status);
    }

TEST_F(DataSourceCacheTests, CacheResponse_ParentInCache_ReturnsSuccess)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "parent"});
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "parent"}), nullptr);

    auto status = cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse());
    EXPECT_EQ(SUCCESS, status);
    }

TEST_F(DataSourceCacheTests, CacheResponse_TwoInstancesAsServerResult_CachesFullInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "TestValueA"}});
    instances.Add({"TestSchema.TestClass2", "B"}, {{"TestProperty", "TestValueB"}});

    BentleyStatus status = cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass2", "B"}).IsFullyCached());

    Json::Value instanceJson;
    cache->ReadInstance({"TestSchema.TestClass", "A"}, instanceJson);
    EXPECT_EQ("TestValueA", instanceJson["TestProperty"].asString());
    cache->ReadInstance({"TestSchema.TestClass2", "B"}, instanceJson);
    EXPECT_EQ("TestValueB", instanceJson["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QueryWithSameNameAndParentCachedPreviusly_ReplacesOldQueryResults)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    BentleyStatus status = cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    EXPECT_EQ(SUCCESS, status);
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, CacheResponse_QueryWithSameNameButDifferentParentsCachedPreviusly_DoesNotOverrideExistingQuery)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey1(cache->FindOrCreateRoot("root1"), "TestQuery");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("root2"), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse());

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    BentleyStatus status = cache->CacheResponse(responseKey2, instances.ToWSObjectsResponse());

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceRemovedInNewData_RemovesInstanceFromCache)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceRemovedInNewDataButIsInRoot_LeavesInstanceInCache)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);
    cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "Foo"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    instances.Clear();
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceRemovedInNewResultsButIsWeaklyLinkedToRoot_RemovesInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    cache->CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), "foo_root", nullptr, true);

    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    instances.Clear();
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    EXPECT_FALSE(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceRemovedInNewResults_RemovesInstanceChildQueryResults)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey baseResultsKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    cache->CacheResponse(baseResultsKey, instances.ToWSObjectsResponse());

    CachedResponseKey fooResultsKey(cache->FindInstance({"TestSchema.TestClass", "Foo"}), nullptr);
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "ChildFoo"});
    cache->CacheResponse(fooResultsKey, instances.ToWSObjectsResponse());

    instances.Clear();
    cache->CacheResponse(baseResultsKey, instances.ToWSObjectsResponse());

    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "ChildFoo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceWithCachedFileRemovedInNewResults_RemovesCachedFile)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(StubFile()), FileCache::Persistent);
    BeFileName cachedFilePath = cache->ReadFilePath({"TestSchema.TestClass", "Foo"});

    EXPECT_TRUE(cachedFilePath.DoesPathExist());

    instances.Clear();
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    EXPECT_FALSE(cachedFilePath.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, CacheResponse_InstanceAddedInNewResults_AddsInstanceToCache)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CacheResponse_RootAsParent_InstancesRelatedToRootInHoldingRelationships)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ECInstanceKey root = cache->FindOrCreateRoot(nullptr);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    cache->CacheResponse({root, ""}, instances.ToWSObjectsResponse());

    ECInstanceKeyMultiMap seedInstances;
    seedInstances.insert(ECDbHelper::ToPair(root));
    ECInstanceKeyMultiMap relatedInstances;
    ECInstanceFinder finder(cache->GetAdapter().GetECDb());
    finder.FindInstances(relatedInstances, seedInstances, ECInstanceFinder::FindOptions(ECInstanceFinder::RelatedDirection_HeldChildren, UINT8_MAX));

    ECInstanceKey instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(instance, relatedInstances));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultNotModified_LeavesPreviouslyCachedData)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    BentleyStatus status = cache->CacheResponse(responseKey, StubWSObjectsResponseNotModified());

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultInstanceWithRelatedInstances_CachesAllRelatedInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances instances;
    auto instancesRelatedToA = instances.Add({"TestSchema.TestClass", "A"});
    instancesRelatedToA.AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    instancesRelatedToA.AddRelated({"TestSchema.TestRelationshipClass", "AC"}, {"TestSchema.TestClass", "C"})
        .AddRelated({"TestSchema.TestRelationshipClass", "CD"}, {"TestSchema.TestClass", "D"});

    BentleyStatus status = cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "C"}).IsInCache());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "D"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultInstanceWithRelatedInstances_CachesRelatedInstancesWithDefinedRelationship)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultWithCyclicRelationship_CachesCyclicRelationship)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"})
        .AddRelated({"TestSchema.TestRelationshipClass", "BA"}, {"TestSchema.TestClass", "A"});

    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, {"TestSchema.TestClass", "B"}, {"TestSchema.TestClass", "A"}));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultWithCyclicRelationship_CachesInstanceOnce)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "CachedValue"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"})
        .AddRelated({"TestSchema.TestRelationshipClass", "BA"}, {"TestSchema.TestClass", "A"}, {{"TestProperty", "IgnoredValue"}});

    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    Json::Value instanceJson;
    cache->ReadInstance({"TestSchema.TestClass", "A"}, instanceJson);

    EXPECT_EQ("CachedValue", instanceJson["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, CacheResponse_NewResultWithChangedRelationship_RemovesOldRelationship)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"})
        .AddRelated({"TestSchema.TestRelationshipClass", "BC"}, {"TestSchema.TestClass", "C"});

    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AC"}, {"TestSchema.TestClass", "C"});
    instances.Add({"TestSchema.TestClass", "B"});

    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema", "TestRelationshipClass");

    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "C"}));
    EXPECT_FALSE(VerifyHasRelationship(cache, testRelClass, {"TestSchema.TestClass", "B"}, {"TestSchema.TestClass", "C"}));
    }

TEST_F(DataSourceCacheTests, CacheResponse_MultipleClassRelationshipsAndNewResultHasNewIntermixedRelationships_LeavesOldRelationshipsAndAddsNew_REGRESSION)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    auto testRelClass1 = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto testRelClass2 = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass2");
    ASSERT_LT(testRelClass1->GetId(), testRelClass2->GetId()); // Ensure that ids are as expected for regression

    {
    StubInstances instances;
    auto instance = instances.Add({"TestSchema.TestClass", "A"});
    instance.AddRelated({"TestSchema.TestRelationshipClass2", "AB"}, {"TestSchema.TestClass", "B"});
    instance.AddRelated({"TestSchema.TestRelationshipClass2", "AC"}, {"TestSchema.TestClass", "C"});

    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));
    }

    {
    StubInstances instances;
    auto instance = instances.Add({"TestSchema.TestClass", "A"});
    instance.AddRelated({"TestSchema.TestRelationshipClass2", "AB"}, {"TestSchema.TestClass", "B"});
    instance.AddRelated({"TestSchema.TestRelationshipClass", "A1"}, {"TestSchema.TestClass", "1"});
    instance.AddRelated({"TestSchema.TestRelationshipClass2", "AC"}, {"TestSchema.TestClass", "C"});
    instance.AddRelated({"TestSchema.TestRelationshipClass", "A2"}, {"TestSchema.TestClass", "2"});

    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));
    }

    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass2, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass2, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "C"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass1, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "1"}));
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass1, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "2"}));
    }

TEST_F(DataSourceCacheTests, CacheResponse_NewResultIsEmptyWhenCachedWithRelationships_RemovesOldInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    instances.Clear();
    auto status = cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    EXPECT_EQ(SUCCESS, status);
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    }

TEST_F(DataSourceCacheTests, CacheResponse_NewResultWithoutRelationshipButSameRelationshipCachedInOtherQuery_LeavesOldRelationship)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey1(cache->FindOrCreateRoot(nullptr), "TestQuery1");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot(nullptr), "TestQuery2");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse());
    cache->CacheResponse(responseKey2, instances.ToWSObjectsResponse());

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});

    auto status = cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse());

    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsRelationshipWithSameIds_CachesRelationships)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "SameId"}, {"TestSchema.TestClass", "B"})
        .AddRelated({"TestSchema.TestRelationshipClass", "SameId"}, {"TestSchema.TestClass", "C"});

    ASSERT_EQ(SUCCESS, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

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

TEST_F(DataSourceCacheTests, CachePartialResponse_InstanceNotCachedPreviously_CachesPartialInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "Partial"});

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, partialInstances.ToWSObjectsResponse(), rejected);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Partial"}).IsInCache());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Partial"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_InstancePreviouslyCachedAsFullInstance_DoesNotOverwriteFullInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "FullyCached"}, {{"TestProperty", "FullValue"}});
    cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "FullyCached"}, fullInstance.ToWSObjectsResponse(), "FullRoot");
    // Roots are by default Full persistence
    cache->SetupRoot("FullRoot", CacheRootPersistence::Default);

    // Act
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "FullyCached"});

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, partialInstances.ToWSObjectsResponse(), rejected);

    // Assert
    Json::Value instance;
    cache->ReadInstance({"TestSchema.TestClass", "FullyCached"}, instance);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ("FullValue", instance["TestProperty"].asString());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "FullyCached"}).IsFullyCached());
    EXPECT_CONTAINS(rejected, ObjectId("TestSchema.TestClass", "FullyCached"));
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_InstancePreviouslyCachedAsFullInstanceInTemporaryRoot_OverwritesItWithPartial)
    {
    // Arrange
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "FullValue"}});

    cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, fullInstance.ToWSObjectsResponse(), "TempRoot");
    cache->SetupRoot("TempRoot", CacheRootPersistence::Temporary);

    // Act
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "PartialValue"}});

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, partialInstances.ToWSObjectsResponse(), rejected);

    // Assert
    Json::Value instance;
    cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instance);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ("PartialValue", instance["TestProperty"].asString());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    EXPECT_NCONTAIN(rejected, ObjectId("TestSchema.TestClass", "Foo"));
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QueryHasEmptySelectToSelectAllProperties_CachesInstanceAsFull)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsAllProperties_CachesInstanceAsFull)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsAllPropertiesWithDifferentSchemaClass_CachesInstanceAsFullAndReturnsNoRejects)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema2", "TestSchema.TestClass");
    query.SetSelect("*");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsNotAllProperties_CachesInstanceAsPartial)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("TestProperty");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsInCache());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QueryPolymorphicallySelectsNotAllProperties_CachesInstanceAsPartial)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema", "TestClass!poly");
    query.SetSelect("TestProperty");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsInCache());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QueryDSelectsForwardRelatedInstanceForDifferentSchemaWithAllProperties_CachesInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema2", "TestSchema.TestClass");
    query.SetSelect("*,TestSchema.TestRelationshipClass-forward-TestSchema.TestClass.*");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsForwardRelatedInstanceWithAllProperties_CachesInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass.*");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsBackwardRelatedInstanceWithAllProperties_CachesInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Backward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-backward-TestClass.*");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsRelatedInstancePolimorphicallyWithAllProperties_CachesFullInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass!poly.*");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsRelatedInstancePolimorphicallyWithAllPropertiesAndResultHasDerived_CachesFullInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestDerivedClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass!poly.*");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestDerivedClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsRelationshipPolymorphically_CachesFullInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass!poly-forward-TestClass.*");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsRelationshipPolymorphicallyAndResponseHasDerivedRelationshipAndNotAllProperties_CachesRelatedAsPartial)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestDerivedRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass!poly-forward-TestClass.$id");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsRelatedFullyWithDefaultRelationship_RelatedIsCachedAsPartialAsDefaultRelationshipsAreNotSupported)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestClass.*");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsRelatedIdOnlyAndRelatedWasCachedAsFull_RejectsRelatedInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"}, {{"TestProperty", "OldValue"}});

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass.$id");

    bset<ObjectId> rejected;
    EXPECT_EQ(SUCCESS, cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query));
    EXPECT_EQ(1, rejected.size());
    EXPECT_CONTAINS(rejected, ObjectId("TestSchema.TestClass", "B"));
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "B"}, instanceJson));
    EXPECT_EQ("OldValue", instanceJson["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsRelationshipPolymorphicallyAndResponseHasDerivedRelationship_CachesFullInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestDerivedRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Forward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass!poly-forward-TestClass.*");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsRelatedInstanceWithAllPropertiesAndAliases_CachesInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, ECRelatedInstanceDirection::Backward);

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*," + query.GetAlias("TestRelationshipClass-backward-TestClass") + ".*");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsRelatedInstanceWithNotAllPropertiesForFullyCachedInstances_RejectsRelatedInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances fullInstances;
    fullInstances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "FullA"}});
    fullInstances.Add({"TestSchema.TestClass", "B"}, {{"TestProperty", "FullB"}});
    CachedResponseKey fullResponseKey(cache->FindOrCreateRoot("FullRoot"), nullptr);
    cache->SetupRoot("FullRoot", CacheRootPersistence::Full);
    cache->CacheResponse(fullResponseKey, fullInstances.ToWSObjectsResponse());

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "NewA"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass.TestProperty");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_THAT(ToStdSet(rejected), ContainerEq(std::set<ObjectId> { {"TestSchema.TestClass", "B"} }));
    EXPECT_EQ("NewA", ReadInstance(cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    EXPECT_EQ("FullB", ReadInstance(cache, {"TestSchema.TestClass", "B"})["TestProperty"].asString());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsNestedRelatedInstanceWithNotAllPropertiesForFullyCachedInstances_RejectsRelatedInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances fullInstances;
    fullInstances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "FullA"}});
    fullInstances.Add({"TestSchema.TestClass", "B"}, {{"TestProperty", "FullB"}});
    fullInstances.Add({"TestSchema.TestClass", "C"}, {{"TestProperty", "FullC"}});
    CachedResponseKey fullResponseKey(cache->FindOrCreateRoot("FullRoot"), nullptr);
    cache->SetupRoot("FullRoot", CacheRootPersistence::Full);
    cache->CacheResponse(fullResponseKey, fullInstances.ToWSObjectsResponse());

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "NewA"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"})
        .AddRelated({"TestSchema.TestRelationshipClass", "BC"}, {"TestSchema.TestClass", "C"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass/TestRelationshipClass-forward-TestClass.TestName");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);

    ObjectId b("TestSchema.TestClass", "B");
    ObjectId c("TestSchema.TestClass", "C");
    EXPECT_THAT(ToStdSet(rejected), ContainerEq(std::set<ObjectId> { b, c }));

    EXPECT_EQ("NewA", ReadInstance(cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    EXPECT_EQ("FullB", ReadInstance(cache, {"TestSchema.TestClass", "B"})["TestProperty"].asString());
    EXPECT_EQ("FullC", ReadInstance(cache, {"TestSchema.TestClass", "C"})["TestProperty"].asString());

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "C"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsNestedRelatedInstanceWithAllPropertiesForFullyCachedInstances_CachesInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances fullInstances;
    fullInstances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "FullA"}});
    fullInstances.Add({"TestSchema.TestClass", "B"}, {{"TestProperty", "FullB"}});
    fullInstances.Add({"TestSchema.TestClass", "C"}, {{"TestProperty", "FullC"}});
    CachedResponseKey fullResponseKey(cache->FindOrCreateRoot("FullRoot"), nullptr);
    cache->SetupRoot("FullRoot", CacheRootPersistence::Full);
    cache->CacheResponse(fullResponseKey, fullInstances.ToWSObjectsResponse());

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "NewA"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {{"TestProperty", "NewB"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "BC"}, {"TestSchema.TestClass", "C"}, {{"TestProperty", "NewC"}});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*,TestRelationshipClass-forward-TestClass.*,TestRelationshipClass-forward-TestClass/TestRelationshipClass-forward-TestClass.*");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ(0, rejected.size());

    EXPECT_EQ("NewA", ReadInstance(cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    EXPECT_EQ("NewB", ReadInstance(cache, {"TestSchema.TestClass", "B"})["TestProperty"].asString());
    EXPECT_EQ("NewC", ReadInstance(cache, {"TestSchema.TestClass", "C"})["TestProperty"].asString());

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "C"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsNotAllPropertiesForFullyCachedInstances_RejectsAllInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances fullInstances;
    fullInstances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "FullA"}});
    fullInstances.Add({"TestSchema.TestClass", "B"}, {{"TestProperty", "FullB"}});
    CachedResponseKey fullResponseKey(cache->FindOrCreateRoot("FullRoot"), nullptr);
    cache->SetupRoot("FullRoot", CacheRootPersistence::Full);
    cache->CacheResponse(fullResponseKey, fullInstances.ToWSObjectsResponse());

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("TestProperty,TestRelationshipClass-forward-TestClass.TestProperty");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);

    ObjectId a("TestSchema.TestClass", "A");
    ObjectId b("TestSchema.TestClass", "B");
    EXPECT_THAT(ToStdSet(rejected), ContainerEq(std::set<ObjectId> { a, b }));

    EXPECT_EQ("FullA", ReadInstance(cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    EXPECT_EQ("FullB", ReadInstance(cache, {"TestSchema.TestClass", "B"})["TestProperty"].asString());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_QuerySelectsNotAllPropertiesForFullyCachedInstance_RejectsInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "FullValue"}});
    cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, fullInstance.ToWSObjectsResponse(), "FullRoot");
    cache->SetupRoot("FullRoot", CacheRootPersistence::Full);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("TestProperty");

    bset<ObjectId> rejected;
    auto status = cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, &query);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_THAT(ToStdSet(rejected), ContainerEq(std::set<ObjectId> { {"TestSchema.TestClass", "Foo"} }));
    EXPECT_EQ("FullValue", ReadInstance(cache, {"TestSchema.TestClass", "Foo"})["TestProperty"].asString());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_KeyHasNoHolder_ParentHasHoldingRelationshipToResults)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey key(cache->FindOrCreateRoot("Parent"), "TestQuery", ECInstanceKey());

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    bset<ObjectId> rejected;
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key, instances.ToWSObjectsResponse(), rejected));
    EXPECT_THAT(rejected, IsEmpty());

    ECInstanceKey instanceKey = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    ECInstanceKeyMultiMap parentInstances;
    ASSERT_EQ(SUCCESS, cache->ReadInstancesConnectedToRootMap("Parent", parentInstances));

    EXPECT_THAT(ECDbHelper::IsInstanceInMultiMap(instanceKey, parentInstances), true);
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_KeyHasDifferentHolder_ParentDoesNotHaveHoldingRelationshipToResultsButHolderDoes)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey key(cache->FindOrCreateRoot("Parent"), "TestQuery", cache->FindOrCreateRoot("Holder"));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    bset<ObjectId> rejected;
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key, instances.ToWSObjectsResponse(), rejected));
    EXPECT_THAT(rejected, IsEmpty());

    ECInstanceKey instanceKey = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    ECInstanceKeyMultiMap parentInstances, holderInstances;
    ASSERT_EQ(SUCCESS, cache->ReadInstancesConnectedToRootMap("Parent", parentInstances));
    ASSERT_EQ(SUCCESS, cache->ReadInstancesConnectedToRootMap("Holder", holderInstances));

    EXPECT_THAT(ECDbHelper::IsInstanceInMultiMap(instanceKey, parentInstances), false);
    EXPECT_THAT(ECDbHelper::IsInstanceInMultiMap(instanceKey, holderInstances), true);
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_KeyHasDifferentHolderAndThenParentIsRemoved_RemovesResults)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey key(cache->FindOrCreateRoot("Parent"), "TestQuery", cache->FindOrCreateRoot("Holder"));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    bset<ObjectId> rejected;
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key, instances.ToWSObjectsResponse(), rejected));
    EXPECT_THAT(rejected, IsEmpty());

    ASSERT_EQ(SUCCESS, cache->RemoveRoot("Parent"));
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid(), false);
    EXPECT_THAT(cache->IsResponseCached(key), false);
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_MultipleNestedResponsesWithHolderAndHolderIsRemoved_RemovesResults)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    bset<ObjectId> rejected;

    StubInstances instances1;
    instances1.Add({"TestSchema.TestClass", "A"});
    CachedResponseKey key1(cache->FindOrCreateRoot("Parent"), "TestQuery", cache->FindOrCreateRoot("Holder"));
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key1, instances1.ToWSObjectsResponse(), rejected));
    EXPECT_THAT(rejected, IsEmpty());

    StubInstances instances2;
    instances2.Add({"TestSchema.TestClass", "B"});
    CachedResponseKey key2(cache->FindInstance({"TestSchema.TestClass", "A"}), "TestQuery", cache->FindOrCreateRoot("Holder"));
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key2, instances2.ToWSObjectsResponse(), rejected));
    EXPECT_THAT(rejected, IsEmpty());

    StubInstances instances3;
    instances3.Add({"TestSchema.TestClass", "C"});
    CachedResponseKey key3(cache->FindInstance({"TestSchema.TestClass", "B"}), "TestQuery", cache->FindOrCreateRoot("Holder"));
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key3, instances3.ToWSObjectsResponse(), rejected));
    EXPECT_THAT(rejected, IsEmpty());

    ASSERT_EQ(SUCCESS, cache->RemoveRoot("Parent"));
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), false);
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid(), false);
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "C"}).IsValid(), false);
    EXPECT_THAT(cache->IsResponseCached(key1), false);
    EXPECT_THAT(cache->IsResponseCached(key2), false);
    EXPECT_THAT(cache->IsResponseCached(key3), false);
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_RelationshipWithProperties_CachesRelationshipProperties)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipPropertiesClass", "AB"}, {"TestSchema.TestClass", "B"}, {}, 
            ECRelatedInstanceDirection::Forward, {{"TestProperty", "RelationshipValue"}});

    bset<ObjectId> rejected;
    EXPECT_EQ(SUCCESS, cache->CachePartialResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse(), rejected, nullptr));
    EXPECT_THAT(rejected, IsEmpty());

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipPropertiesClass");
    auto relationshipKey = cache->FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"});

    EXPECT_TRUE(relationshipKey.IsValid());
    EXPECT_THAT(cache->FindRelationship(relationshipKey), ObjectId("TestSchema.TestRelationshipPropertiesClass", "AB"));

    Json::Value relationshipJson;
    cache->GetAdapter().GetJsonInstance(relationshipJson, relationshipKey);
    EXPECT_NE(Json::Value::null, relationshipJson);
    EXPECT_EQ("RelationshipValue", relationshipJson["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_KeysHaveSameHolderAndNameAndParent_NewResponseOverridesOldOne)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey key(cache->FindOrCreateRoot("Parent"), "TestQuery", cache->FindOrCreateRoot("Holder"));

    StubInstances oldInstances;
    oldInstances.Add({"TestSchema.TestClass", "A"});

    bset<ObjectId> rejected;
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key, oldInstances.ToWSObjectsResponse(), rejected));
    EXPECT_THAT(rejected, IsEmpty());
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), true);

    StubInstances newInstances;
    newInstances.Add({"TestSchema.TestClass", "B"});

    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key, newInstances.ToWSObjectsResponse(), rejected));
    EXPECT_THAT(rejected, IsEmpty());
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), false);
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid(), true);
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_KeysHaveSameParentAndSameNameButDifferentHolders_NewResponseOverridesOldOne)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey key1(cache->FindOrCreateRoot("Parent"), "TestQuery", cache->FindOrCreateRoot("Holder1"));
    CachedResponseKey key2(cache->FindOrCreateRoot("Parent"), "TestQuery", cache->FindOrCreateRoot("Holder2"));

    StubInstances oldInstances;
    oldInstances.Add({"TestSchema.TestClass", "A"});

    bset<ObjectId> rejected;
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key1, oldInstances.ToWSObjectsResponse(), rejected));
    EXPECT_THAT(rejected, IsEmpty());
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), true);

    StubInstances newInstances;
    newInstances.Add({"TestSchema.TestClass", "B"});

    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key2, newInstances.ToWSObjectsResponse(), rejected));
    EXPECT_THAT(rejected, IsEmpty());
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), false);
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid(), true);
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_KeysHaveSameHolderAndNameButDifferentParents_DoesNotOverrideEachOther)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey key1(cache->FindOrCreateRoot("Parent1"), "TestQuery", cache->FindOrCreateRoot("Holder"));
    CachedResponseKey key2(cache->FindOrCreateRoot("Parent2"), "TestQuery", cache->FindOrCreateRoot("Holder"));

    StubInstances oldInstances;
    oldInstances.Add({"TestSchema.TestClass", "A"});

    bset<ObjectId> rejected;
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key1, oldInstances.ToWSObjectsResponse(), rejected));
    EXPECT_THAT(rejected, IsEmpty());
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), true);

    StubInstances newInstances;
    newInstances.Add({"TestSchema.TestClass", "B"});

    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key2, newInstances.ToWSObjectsResponse(), rejected));
    EXPECT_THAT(rejected, IsEmpty());
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "A"}).IsValid(), true);
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "B"}).IsValid(), true);
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_NotCancelledCancellatioTokenPassed_CachesAndReturnsSuccess)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);
    auto token = SimpleCancellationToken::Create(false);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    bset<ObjectId> rejected;
    EXPECT_EQ(SUCCESS, cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, nullptr, token));
    EXPECT_THAT(cache->IsResponseCached(responseKey), true);
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_CancelledCancellatioTokenPassed_ReturnsError)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);
    auto token = SimpleCancellationToken::Create(true);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    bset<ObjectId> rejected;
    EXPECT_EQ(ERROR, cache->CachePartialResponse(responseKey, instances.ToWSObjectsResponse(), rejected, nullptr, token));
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
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "Foo");

    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));
    EXPECT_TRUE(cache->IsResponseCached(responseKey));
    }

TEST_F(DataSourceCacheTests, IsResponseCached_ResponseWithHoldererCached_True)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "Foo", cache->FindOrCreateRoot("Holder"));

    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));
    EXPECT_TRUE(cache->IsResponseCached(responseKey));
    }

TEST_F(DataSourceCacheTests, MarkTemporaryInstancesAsPartial_PartiallyCachedQueryContainsFullyCachedInstanceInTemporaryRoot_SetsInstanceStateToPartial)
    {
    // Arrange
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot("Root"), "TestQuery");

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "Foo"});

    bset<ObjectId> rejected;
    cache->CachePartialResponse(responseKey, partialInstances.ToWSObjectsResponse(), rejected);

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "FullValue"}});

    cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, fullInstance.ToWSObjectsResponse(), "Root");
    cache->SetupRoot("Root", CacheRootPersistence::Temporary);

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());

    // Act
    auto status = cache->MarkTemporaryInstancesAsPartial({responseKey});

    // Assert
    EXPECT_EQ(SUCCESS, status);
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, MarkTemporaryInstancesAsPartial_PartiallyCachedQueryContainsFullyCachedParentInstanceInTemporaryRoot_SetsInstanceStateToPartial)
    {
    // Arrange
    shared_ptr<DataSourceCache> cache = GetTestCache();

    // Same instance is parent and result - will force different weak result relationship in cached query
    cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Foo"});
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "Foo"}), "TestQuery");

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "Foo"});

    bset<ObjectId> rejected;
    cache->CachePartialResponse(responseKey, partialInstances.ToWSObjectsResponse(), rejected);

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "FullValue"}});

    cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, fullInstance.ToWSObjectsResponse(), "TestRoot");
    cache->SetupRoot("TestRoot", CacheRootPersistence::Temporary);

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());

    // Act
    auto status = cache->MarkTemporaryInstancesAsPartial({responseKey});

    // Assert
    EXPECT_EQ(SUCCESS, status);
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, MarkTemporaryInstancesAsPartial_PartiallyCachedQueryContainsFullyCachedInstanceInFullRoot_LeavesInstanceStateFullyCached)
    {
    // Arrange
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "FullyCached"}, {{"TestProperty", "FullValue"}});

    cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "FullyCached"}, fullInstance.ToWSObjectsResponse(), "FullPersistenceRoot");
    cache->SetupRoot("FullPersistenceRoot", CacheRootPersistence::Full);
    cache->SetupRoot(nullptr, CacheRootPersistence::Temporary);

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "FullyCached"});

    bset<ObjectId> rejected;
    cache->CachePartialResponse(responseKey, partialInstances.ToWSObjectsResponse(), rejected);

    // Act
    auto status = cache->MarkTemporaryInstancesAsPartial({responseKey});

    // Assert
    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "FullyCached"}).IsFullyCached());
    }

TEST_F(DataSourceCacheTests, ReadResponse_NonExistingQuery_ReturnsDataNotCachedAndEmptyArray)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ECInstanceKey root = cache->FindOrCreateRoot(nullptr);

    Json::Value results;
    auto status = cache->ReadResponse({root, "NonExisting"}, results);

    EXPECT_EQ(CacheStatus::DataNotCached, status);
    EXPECT_TRUE(results.empty());
    }

TEST_F(DataSourceCacheTests, ReadResponse_NonExistingParent_ReturnsDataNotCachedAndEmptyArray)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ECInstanceKey parent = cache->FindInstance({"TestSchema.TestClass", "NonExisting"});

    Json::Value results;
    auto status = cache->ReadResponse({parent, nullptr}, results);

    EXPECT_EQ(CacheStatus::DataNotCached, status);
    EXPECT_TRUE(results.empty());
    }

TEST_F(DataSourceCacheTests, ReadResponse_ZeroResultsCached_ReturnsOkAndEmptyArray)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse());

    Json::Value results;
    auto status = cache->ReadResponse(responseKey, results);

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_TRUE(results.empty());
    }

TEST_F(DataSourceCacheTests, ReadResponse_PartialInstanceRejectedWhileCaching_StillReturnsInstanceAsQueryResult)
    {
    // Arrange
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances fullInstance;
    fullInstance.Add({"TestSchema.TestClass", "FullyCached"}, {{"TestProperty", "FullValue"}});

    cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "FullyCached"}, fullInstance.ToWSObjectsResponse(), "FullRoot");
    cache->SetupRoot("FullRoot", CacheRootPersistence::Full);

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances partialInstances;
    partialInstances.Add({"TestSchema.TestClass", "FullyCached"});

    bset<ObjectId> rejected;
    cache->CachePartialResponse(responseKey, partialInstances.ToWSObjectsResponse(), rejected);

    // Act
    Json::Value queryResults;
    auto status = cache->ReadResponse(responseKey, queryResults);

    // Assert
    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_EQ(1, queryResults.size());
    EXPECT_EQ("FullyCached", queryResults[0][DataSourceCache_PROPERTY_RemoteId].asString());
    }

TEST_F(DataSourceCacheTests, ReadResponse_CachedResultsWithTwoInstance_ReturnsBothInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    Json::Value results;
    auto status = cache->ReadResponse(responseKey, results);

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_EQ(2, results.size());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), cache->ObjectIdFromJsonInstance(results[0]));
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "B"), cache->ObjectIdFromJsonInstance(results[1]));
    }

TEST_F(DataSourceCacheTests, ReadResponse_CachedResultsIncludeInstancesRelatedToOtherInstance_ReturnsBothInstances)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    Json::Value results;
    auto status = cache->ReadResponse(responseKey, results);

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_EQ(2, results.size());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), cache->ObjectIdFromJsonInstance(results[0]));
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "B"), cache->ObjectIdFromJsonInstance(results[1]));
    }

TEST_F(DataSourceCacheTests, ReadResponse_CachedResultsWithInstance_ReturnsInstanceProperties)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "42"}});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    Json::Value results;
    auto status = cache->ReadResponse(responseKey, results);

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_EQ(1, results.size());
    EXPECT_EQ("42", results[0]["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, ReadResponse_CachedWithKeyWithSeperateHolderButPassingOnlyParent_ReturnsCachedResults)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey1(cache->FindOrCreateRoot("Parent"), nullptr, cache->FindOrCreateRoot("Holder"));
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("Parent"), nullptr);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "42"}});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse()));

    Json::Value results;
    auto status = cache->ReadResponse(responseKey2, results);

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_EQ(1, results.size());
    EXPECT_EQ("42", results[0]["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, ReadResponse_CachedResultsIncludeParent_ReturnsParentAlso)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "A"});
    auto instanceA = cache->FindInstance({"TestSchema.TestClass", "A"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    cache->CacheResponse({instanceA, "TestQuery"}, instances.ToWSObjectsResponse());

    Json::Value results;
    auto status = cache->ReadResponse({instanceA, "TestQuery"}, results);

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_EQ(1, results.size());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), cache->ObjectIdFromJsonInstance(results[0]));
    }

TEST_F(DataSourceCacheTests, ReadResponseInstanceKeys_NonExistingQuery_ReturnsDataNotCachedAndEmptyMap)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey({cache->FindOrCreateRoot(nullptr), "NonExisting"});

    ECInstanceKeyMultiMap instances;
    auto status = cache->ReadResponseInstanceKeys(responseKey, instances);

    EXPECT_EQ(CacheStatus::DataNotCached, status);
    EXPECT_EQ(0, instances.size());
    }

TEST_F(DataSourceCacheTests, ReadResponseInstanceKeys_TwoInstancesCachedAsResult_ReturnsThemInMap)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey({cache->FindOrCreateRoot(nullptr), "TestQuery"});

    StubInstances stubInstances;
    stubInstances.Add({"TestSchema.TestClass", "A"});
    stubInstances.Add({"TestSchema.TestClass", "B"});
    cache->CacheResponse(responseKey, stubInstances.ToWSObjectsResponse());

    ECInstanceKeyMultiMap instances;
    auto status = cache->ReadResponseInstanceKeys(responseKey, instances);

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_EQ(2, instances.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "A"}), instances));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "B"}), instances));
    }

TEST_F(DataSourceCacheTests, ReadResponseInstanceKeys_ResultsContainParent_ReturnsParent)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "A"});
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "A"}), "TestQuery");

    StubInstances stubInstances;
    stubInstances.Add({"TestSchema.TestClass", "A"});
    cache->CacheResponse(responseKey, stubInstances.ToWSObjectsResponse());

    ECInstanceKeyMultiMap instances;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey, instances));

    EXPECT_EQ(1, instances.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "A"}), instances));
    }

TEST_F(DataSourceCacheTests, ReadResponseInstanceKeys_CachedInstanceWithRelationshipToItself_ReturnsInstanceKeysAsDuplicate)
    {
    // Does not work due to fact that CachedResponseInfo relateds results instances and does not relate duplicates
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto parentInstance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Parent"});
    CachedResponseKey responseKey(parentInstance, nullptr);

    StubInstances stubInstances;
    stubInstances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AA"}, {"TestSchema.TestClass", "A"});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, stubInstances.ToWSObjectsResponse()));

    ECInstanceKeyMultiMap instances;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey, instances));

    ECInstanceKeyMultiMap expectedInstances;
    expectedInstances.insert(ECDbHelper::ToPair(cache->FindInstance({"TestSchema.TestClass", "A"})));
    expectedInstances.insert(ECDbHelper::ToPair(cache->FindInstance({"TestSchema.TestClass", "A"})));
    EXPECT_THAT(instances, ContainerEq(expectedInstances));
    }

TEST_F(DataSourceCacheTests, ReadResponseInstanceKeys_CachedParentInstanceWithRelationshipToItself_ReturnsInstanceKeysAsDuplicate)
    {
    // Does not work due to fact that CachedResponseInfo relateds results instances and does not relate duplicates
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto parentInstance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    CachedResponseKey responseKey(parentInstance, nullptr);

    StubInstances stubInstances;
    stubInstances
        .Add({"TestSchema.TestClass", "A"})
        .AddRelated({"TestSchema.TestRelationshipClass", "AA"}, {"TestSchema.TestClass", "A"});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, stubInstances.ToWSObjectsResponse()));

    ECInstanceKeyMultiMap instances;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey, instances));

    ECInstanceKeyMultiMap expectedInstances;
    expectedInstances.insert(ECDbHelper::ToPair(parentInstance));
    expectedInstances.insert(ECDbHelper::ToPair(parentInstance));
    EXPECT_THAT(instances, ContainerEq(expectedInstances));
    }

TEST_F(DataSourceCacheTests, ReadResponseInstanceKeys_CachedWithKeyWithSeperateHolderButPassingOnlyParent_ReturnsCachedResults)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey1(cache->FindOrCreateRoot("Parent"), nullptr, cache->FindOrCreateRoot("Holder"));
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("Parent"), nullptr);

    StubInstances stubInstances;
    stubInstances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey1, stubInstances.ToWSObjectsResponse()));

    ECInstanceKeyMultiMap instances;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey2, instances));

    EXPECT_EQ(1, instances.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "A"}), instances));
    }

TEST_F(DataSourceCacheTests, ReadResponseObjectIds_NonExistingQuery_ReturnsDataNotCached)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey({cache->FindOrCreateRoot(nullptr), "NonExisting"});

    bset<ObjectId> objectIds;
    auto status = cache->ReadResponseObjectIds(responseKey, objectIds);

    EXPECT_EQ(CacheStatus::DataNotCached, status);
    }

TEST_F(DataSourceCacheTests, ReadResponseObjectIds_NonExistingParent_ReturnsDataNotCachedAndNoIds)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ECInstanceKey parent = cache->FindInstance({"TestSchema.TestClass", "NonExisting"});
    CachedResponseKey responseKey(parent, nullptr);

    bset<ObjectId> objectIds;
    auto status = cache->ReadResponseObjectIds(responseKey, objectIds);

    EXPECT_EQ(CacheStatus::DataNotCached, status);
    EXPECT_TRUE(objectIds.empty());
    }

TEST_F(DataSourceCacheTests, ReadResponseObjectIds_TwoInstancesCachedAsResult_ReturnsTheirObjectIds)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey({cache->FindOrCreateRoot(nullptr), "TestQuery"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    bset<ObjectId> objectIds;
    auto status = cache->ReadResponseObjectIds(responseKey, objectIds);

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_EQ(2, objectIds.size());
    EXPECT_CONTAINS(objectIds, ObjectId("TestSchema.TestClass", "A"));
    EXPECT_CONTAINS(objectIds, ObjectId("TestSchema.TestClass", "B"));
    }

TEST_F(DataSourceCacheTests, ReadResponseObjectIds_ResultsContainParent_ReturnsParentObjectId)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "A"});
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "A"}), "TestQuery");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    cache->CacheResponse(responseKey, instances.ToWSObjectsResponse());

    bset<ObjectId> objectIds;
    auto status = cache->ReadResponseObjectIds(responseKey, objectIds);

    EXPECT_EQ(CacheStatus::OK, status);
    EXPECT_EQ(1, objectIds.size());
    EXPECT_CONTAINS(objectIds, ObjectId("TestSchema.TestClass", "A"));
    }

TEST_F(DataSourceCacheTests, ReadResponseObjectIds_CachedWithKeyWithSeperateHolderButPassingOnlyParent_ReturnsCachedResults)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey1(cache->FindOrCreateRoot("Parent"), nullptr, cache->FindOrCreateRoot("Holder"));
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("Parent"), nullptr);

    StubInstances stubInstances;
    stubInstances.Add({"TestSchema.TestClass", "A"});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey1, stubInstances.ToWSObjectsResponse()));

    bset<ObjectId> objectIds;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseObjectIds(responseKey2, objectIds));

    EXPECT_EQ(1, objectIds.size());
    EXPECT_CONTAINS(objectIds, ObjectId("TestSchema.TestClass", "A"));
    }

TEST_F(DataSourceCacheTests, RemoveResponses_NonExistingQuery_ReturnsSuccess)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ECInstanceKey root = cache->FindOrCreateRoot(nullptr);

    BentleyStatus status = cache->RemoveResponse({root, "NonExisting"});

    EXPECT_EQ(SUCCESS, status);
    }

TEST_F(DataSourceCacheTests, RemoveResponses_QuerySharesRelationshipWithOtherQuery_LeavesRelationship)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey1(cache->FindOrCreateRoot(nullptr), "TestQuery1");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot(nullptr), "TestQuery2");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse());
    cache->CacheResponse(responseKey2, instances.ToWSObjectsResponse());

    cache->RemoveResponse(responseKey1);

    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto instanceA = cache->FindInstance({"TestSchema.TestClass", "A"});
    auto instanceB = cache->FindInstance({"TestSchema.TestClass", "B"});

    EXPECT_TRUE(instanceA.IsValid());
    EXPECT_TRUE(instanceB.IsValid());
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, instanceA, instanceB));
    }

TEST_F(DataSourceCacheTests, RemoveCachedQueryResults_QueryWithRelationshipWhenOtherQueryHoldsOnlyInstances_DeletesRelationship)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    CachedResponseKey responseKey1(cache->FindOrCreateRoot(nullptr), "TestQuery1");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot(nullptr), "TestQuery2");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});

    cache->CacheResponse(responseKey1, instances.ToWSObjectsResponse());

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    cache->CacheResponse(responseKey2, instances.ToWSObjectsResponse());

    cache->RemoveResponse(responseKey2);

    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto instanceA = cache->FindInstance({"TestSchema.TestClass", "A"});
    auto instanceB = cache->FindInstance({"TestSchema.TestClass", "B"});

    EXPECT_TRUE(instanceA.IsValid());
    EXPECT_TRUE(instanceB.IsValid());
    EXPECT_FALSE(VerifyHasRelationship(cache, testRelClass, instanceA, instanceB));
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheTag_NonExistingQueryResults_ReturnsEmptyString)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ECInstanceKey root = cache->FindOrCreateRoot(nullptr);

    EXPECT_EQ("", cache->ReadResponseCacheTag({root, "NonExisting"}));
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheTag_PreviouslyCachedWithTag_ReturnsSameTag)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse("FooTag"));

    EXPECT_EQ("FooTag", cache->ReadResponseCacheTag(responseKey));
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheTag_PreviouslyCachedWithTagAndHolder_ReturnsSameTag)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery", cache->FindOrCreateRoot("Holder"));

    cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse("FooTag"));

    EXPECT_EQ("FooTag", cache->ReadResponseCacheTag(responseKey));
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheDate_NonExistingQueryResults_ReturnsInvalidDate)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ECInstanceKey root = cache->FindOrCreateRoot(nullptr);

    EXPECT_EQ(DateTime(), cache->ReadResponseCachedDate({root, "NonExisting"}));
    EXPECT_FALSE(cache->ReadResponseCachedDate({root, "NonExisting"}).IsValid());
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheDate_PreviouslyCached_ReturnsCorrectCachedDate)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery");

    auto before = DateTime::GetCurrentTimeUtc();
    BeThreadUtilities::BeSleep(1); // DateTime persistence introduces rounding error in nano seconds
    cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse());
    BeThreadUtilities::BeSleep(1);
    auto after = DateTime::GetCurrentTimeUtc();

    EXPECT_BETWEEN(before, cache->ReadResponseCachedDate(responseKey), after);
    }

TEST_F(DataSourceCacheTests, ReadResponseCacheDate_PreviouslyCachedWithHolder_ReturnsCorrectCachedDate)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot(nullptr), "TestQuery", cache->FindOrCreateRoot("Holder"));

    auto before = DateTime::GetCurrentTimeUtc();
    BeThreadUtilities::BeSleep(1); // DateTime persistence introduces rounding error in nano seconds
    cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse());
    BeThreadUtilities::BeSleep(1);
    auto after = DateTime::GetCurrentTimeUtc();

    EXPECT_BETWEEN(before, cache->ReadResponseCachedDate(responseKey), after);
    }

TEST_F(DataSourceCacheTests, ReadInstanceLabel_EmptyObjectId_ReturnsEmpty)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    EXPECT_EQ("", cache->ReadInstanceLabel({}));
    }

TEST_F(DataSourceCacheTests, ReadInstanceLabel_ClassWithLabelProperty_ReturnsCachedLabel)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->CacheInstanceAndLinkToRoot({"TestSchema.TestLabeledClass", "Foo"}, *ToRapidJson(R"({"Name" : "TestLabel"})"), "", "");
    EXPECT_EQ("TestLabel", cache->ReadInstanceLabel({"TestSchema.TestLabeledClass", "Foo"}));
    }

TEST_F(DataSourceCacheTests, ReadInstanceLabel_DreivedClassWithBaseLabelProperty_ReturnsCachedLabel)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->CacheInstanceAndLinkToRoot({"TestSchema.TestDerivedLabeledClass", "Foo"}, *ToRapidJson(R"({"Name" : "TestLabel"})"), "", "");
    EXPECT_EQ("TestLabel", cache->ReadInstanceLabel({"TestSchema.TestDerivedLabeledClass", "Foo"}));
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_NonExistingInstance_Error)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    Utf8String fileName;
    uint64_t fileSize;
    EXPECT_EQ(ERROR, cache->ReadFileProperties(StubNonExistingInstanceKey(*cache), fileName, fileSize));
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_NonFileInstance_SuccessAndEmptyValues)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"}, {{"TestProperty", "TestValue"}});

    Utf8String fileName = "NoValue";
    uint64_t fileSize = 99;

    ASSERT_EQ(SUCCESS, cache->ReadFileProperties(instanceKey, fileName, fileSize));

    EXPECT_EQ("", fileName);
    EXPECT_EQ(0, fileSize);
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_LabeledInstance_SuccessAndReturnsLabel)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestLabeledClass", "Foo"}, {{"Name", "TestName"}});

    Utf8String fileName = "NoValue";
    uint64_t fileSize = 99;

    ASSERT_EQ(SUCCESS, cache->ReadFileProperties(instanceKey, fileName, fileSize));

    EXPECT_EQ("TestName", fileName);
    EXPECT_EQ(0, fileSize);
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_InstanceOfClassClassWithFileDependentProperties_SuccessAndReturnsLabelAndSize)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestFileClass", "Foo"}, {{"TestName", "TestName"}, {"TestSize", "42"}});

    Utf8String fileName = "NoValue";
    uint64_t fileSize = 99;

    ASSERT_EQ(SUCCESS, cache->ReadFileProperties(instanceKey, fileName, fileSize));

    EXPECT_EQ("TestName", fileName);
    EXPECT_EQ(42, fileSize);
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_InstanceOfClassClassWithOnlyFileNameProperty_SuccessAndReturnsLabel)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestFileClass2", "Foo"}, {{"TestName", "TestName"}, {"TestSize", "42"}});

    Utf8String fileName = "NoValue";
    uint64_t fileSize = 99;

    ASSERT_EQ(SUCCESS, cache->ReadFileProperties(instanceKey, fileName, fileSize));

    EXPECT_EQ("TestName", fileName);
    EXPECT_EQ(0, fileSize);
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_InstanceOfClassClassWithOnlyFileSizeProperty_SuccessAndReturnsFileSize)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestFileClass3", "Foo"}, {{"TestName", "TestName"}, {"TestSize", "42"}});

    Utf8String fileName = "NoValue";
    uint64_t fileSize = 99;

    ASSERT_EQ(SUCCESS, cache->ReadFileProperties(instanceKey, fileName, fileSize));

    EXPECT_EQ("", fileName);
    EXPECT_EQ(42, fileSize);
    }

TEST_F(DataSourceCacheTests, ReadFileProperties_InstanceOfClassClassWithFileDependentPropertiesButNoNameOrSize_SuccessAndReturnsLabel)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestFileClass4", "Foo"}, {{"Name", "TestName"}});

    Utf8String fileName = "NoValue";
    uint64_t fileSize = 99;

    ASSERT_EQ(SUCCESS, cache->ReadFileProperties(instanceKey, fileName, fileSize));

    EXPECT_EQ("TestName", fileName);
    EXPECT_EQ(0, fileSize);
    }

TEST_F(DataSourceCacheTests, CacheFile_ObjectWithSuchIdNotCached_ReturnsError)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    auto status = cache->CacheFile({"TestSchema.TestClass", "NotExisting"}, WSFileResponse(), FileCache::Persistent);
    EXPECT_EQ(ERROR, status);
    }

TEST_F(DataSourceCacheTests, CacheFile_WSFileResponsePassed_MovesFileToCacheLocation)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot("Root", fileId);

    BeFileName fileToCachePath = StubFile();
    EXPECT_TRUE(fileToCachePath.DoesPathExist());

    auto status = cache->CacheFile(fileId, WSFileResponse(fileToCachePath, HttpStatus::OK, "TestTag"), FileCache::Persistent);
    BeFileName cachedFilePath = cache->ReadFilePath(fileId);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_NE(fileToCachePath, cachedFilePath);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    EXPECT_FALSE(fileToCachePath.DoesPathExist());
    EXPECT_EQ("TestTag", cache->ReadFileCacheTag(fileId));
    }

TEST_F(DataSourceCacheTests, CacheFile_CachingPersistentLocation_CachedFilePathBeginsWithEnvronmentPath)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot("Root", fileId);

    cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, nullptr), FileCache::Persistent);

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    BeFileName environmentPath = StubCacheEnvironemnt().persistentFileCacheDir;

    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    EXPECT_THAT(cachedFilePath.GetNameUtf8().c_str(), StartsWith(environmentPath.GetNameUtf8().c_str()));
    }

TEST_F(DataSourceCacheTests, CacheFile_CachingTemporaryLocation_CachedFilePathBeginsWithEnvronmentPath)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot("Root", fileId);

    cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, nullptr), FileCache::Temporary);

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    BeFileName environmentPath = StubCacheEnvironemnt().temporaryFileCacheDir;

    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    EXPECT_THAT(cachedFilePath.GetNameUtf8().c_str(), StartsWith(environmentPath.GetNameUtf8().c_str()));
    }

TEST_F(DataSourceCacheTests, CacheFile_FileCachedPreviously_CachesNewFileAndRemovesOld)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot("Root", fileId);

    cache->CacheFile(fileId, WSFileResponse(StubFile("abc", "Foo.txt"), HttpStatus::OK, nullptr), FileCache::Persistent);
    BeFileName cachedFileA = cache->ReadFilePath(fileId);
    EXPECT_TRUE(cachedFileA.DoesPathExist());

    cache->CacheFile(fileId, WSFileResponse(StubFile("def", "Foo.txt"), HttpStatus::OK, nullptr), FileCache::Persistent);
    BeFileName cachedFileB = cache->ReadFilePath(fileId);

    EXPECT_TRUE(cachedFileB.DoesPathExist());
    EXPECT_FALSE(cachedFileA.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, CacheFile_FileCachedPreviouslyAndCachingToDifferentLocation_CachesNewFileAndRemovesOld)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot("Root", fileId);

    cache->CacheFile(fileId, WSFileResponse(StubFile("abc", "Foo.txt"), HttpStatus::OK, nullptr), FileCache::Temporary);
    BeFileName cachedFileA = cache->ReadFilePath(fileId);
    EXPECT_TRUE(cachedFileA.DoesPathExist());

    cache->CacheFile(fileId, WSFileResponse(StubFile("def", "Foo.txt"), HttpStatus::OK, nullptr), FileCache::Persistent);
    BeFileName cachedFileB = cache->ReadFilePath(fileId);

    EXPECT_TRUE(cachedFileB.DoesPathExist());
    EXPECT_FALSE(cachedFileA.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, CacheFile_WSFileResponseNotModifiedPassed_UpdatesCachedDate)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot("Root", fileId);
    cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, ""), FileCache::Persistent);

    auto before = DateTime::GetCurrentTimeUtc();
    BeThreadUtilities::BeSleep(1);
    auto status = cache->CacheFile(fileId, WSFileResponse(BeFileName(), HttpStatus::NotModified, ""), FileCache::Persistent);
    BeThreadUtilities::BeSleep(1);
    auto after = DateTime::GetCurrentTimeUtc();

    DateTime cachedDate = cache->ReadFileCachedDate(fileId);

    ASSERT_EQ(SUCCESS, status);
    EXPECT_BETWEEN(before, cachedDate, after);
    }

TEST_F(DataSourceCacheTests, ReadFileCachedDate_FileNotCached_ReturnsInvalid)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", fileId));

    ASSERT_THAT(cache->ReadFileCachedDate(fileId).IsValid(), false);
    }

TEST_F(DataSourceCacheTests, ReadFileCachedDate_FileNotOnDiskAnyMore_ReturnsInvalid)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Root", fileId));
    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, ""), FileCache::Persistent));
    ASSERT_EQ(BeFileNameStatus::Success, cache->ReadFilePath(fileId).BeDeleteFile());

    ASSERT_THAT(cache->ReadFileCachedDate(fileId).IsValid(), false);
    }

TEST_F(DataSourceCacheTests, ReadInstanceCachedDate_InstanceCached_ReturnsDateWhenCached)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

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
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ASSERT_THAT(cache->ReadInstanceCachedDate({"TestSchema.TestClass", "NonExisting"}).IsValid(), false);
    }

TEST_F(DataSourceCacheTests, CacheFile_CachingPersistentLocation_ExternalFileInfoIsAssociatedWithInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot("Root", fileId);

    auto status = cache->CacheFile(fileId, WSFileResponse(StubFile("abc", "Test.txt"), HttpStatus::OK, nullptr), FileCache::Persistent);
    ASSERT_EQ(SUCCESS, status);

    ECRelationshipClassCP instanceHasFileInfoClass = cache->GetAdapter().GetECRelationshipClass("ECDb_FileInfo.InstanceHasFileInfo");
    ECClassCP fileInfoClass = cache->GetAdapter().GetECClass("ECDb_FileInfo.ExternalFileInfo");
    auto instanceKey = cache->FindInstance({"TestSchema.TestClass", "Foo"});

    Json::Value externalFileInfos;
    cache->GetAdapter().GetJsonRelatedTargets(externalFileInfos, instanceHasFileInfoClass, fileInfoClass, instanceKey);

    JsonValueCR externalFileInfo = externalFileInfos[0];

    EXPECT_EQ(CacheEnvironment::GetPersistentRootFolderId(), externalFileInfo["RootFolder"].asInt());
    EXPECT_NE("", externalFileInfo["RelativePath"].asString());
    EXPECT_EQ("Test.txt", externalFileInfo["Name"].asString());
    }

TEST_F(DataSourceCacheTests, CacheFile_CachingTemporaryLocation_ExternalFileInfoIsAssociatedWithInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot("Root", fileId);

    auto status = cache->CacheFile(fileId, WSFileResponse(StubFile("abc", "Test.txt"), HttpStatus::OK, nullptr), FileCache::Temporary);
    ASSERT_EQ(SUCCESS, status);

    ECRelationshipClassCP instanceHasFileInfoClass = cache->GetAdapter().GetECRelationshipClass("ECDb_FileInfo.InstanceHasFileInfo");
    ECClassCP fileInfoClass = cache->GetAdapter().GetECClass("ECDb_FileInfo.ExternalFileInfo");
    auto instanceKey = cache->FindInstance({"TestSchema.TestClass", "Foo"});

    Json::Value externalFileInfos;
    cache->GetAdapter().GetJsonRelatedTargets(externalFileInfos, instanceHasFileInfoClass, fileInfoClass, instanceKey);

    JsonValueCR externalFileInfo = externalFileInfos[0];

    EXPECT_EQ(CacheEnvironment::GetTemporaryRootFolderId(), externalFileInfo["RootFolder"].asInt());
    EXPECT_NE("", externalFileInfo["RelativePath"].asString());
    EXPECT_EQ("Test.txt", externalFileInfo["Name"].asString());
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_MovingCachedFileToTemporary_MovesFileToTemporaryEnvironmentLocation)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot("Root", fileId);

    cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, nullptr), FileCache::Persistent);

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::Temporary));

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    BeFileName environmentPath = StubCacheEnvironemnt().temporaryFileCacheDir;

    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    EXPECT_THAT(cachedFilePath.GetNameUtf8().c_str(), StartsWith(environmentPath.GetNameUtf8().c_str()));
    }

TEST_F(DataSourceCacheTests, SetFileCacheLocation_MovingCachedFileToPersistent_MovesFileToPersistentEnvironmentLocation)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot("Root", fileId);

    cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, nullptr), FileCache::Temporary);

    ASSERT_EQ(SUCCESS, cache->SetFileCacheLocation(fileId, FileCache::Persistent));

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    BeFileName environmentPath = StubCacheEnvironemnt().persistentFileCacheDir;

    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    EXPECT_THAT(cachedFilePath.GetNameUtf8().c_str(), StartsWith(environmentPath.GetNameUtf8().c_str()));
    }

TEST_F(DataSourceCacheTests, GetFileCacheLocation_NotExsitingInstance_ReturnsTemporary)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "NonExisting"};

    FileCache cacheLocation = cache->GetFileCacheLocation(fileId);
    EXPECT_EQ(FileCache::Temporary, cacheLocation);
    }

TEST_F(DataSourceCacheTests, GetFileCacheLocation_NotCachedFile_ReturnsTemporary)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot("Root", fileId);

    EXPECT_EQ(FileCache::Temporary, cache->GetFileCacheLocation(fileId));
    }

TEST_F(DataSourceCacheTests, GetFileCacheLocation_CachedToTemporary_ReturnsTemporary)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot("Root", fileId);

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, nullptr), FileCache::Temporary));
    EXPECT_EQ(FileCache::Temporary, cache->GetFileCacheLocation(fileId));
    }

TEST_F(DataSourceCacheTests, GetFileCacheLocation_CachedToPersistent_ReturnsPersistent)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot("Root", fileId);

    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, nullptr), FileCache::Persistent));
    EXPECT_EQ(FileCache::Persistent, cache->GetFileCacheLocation(fileId));
    }

TEST_F(DataSourceCacheTests, ReadFilePath_CachedFileAndObjectIdPassed_ReturnsPath)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot(nullptr, fileId);

    auto status = cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, "TestTag"), FileCache::Persistent);
    BeFileName cachedFilePath = cache->ReadFilePath(fileId);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, ReadFilePath_CachedFileAndECInstanceKeyPassed_ReturnsPath)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    cache->LinkInstanceToRoot(nullptr, fileId);

    auto status = cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, "TestTag"), FileCache::Persistent);

    ECInstanceKey fileKey = cache->FindInstance(fileId);
    BeFileName cachedFilePath = cache->ReadFilePath(fileKey);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cachedFilePath.DoesPathExist());
    }

TEST_F(DataSourceCacheTests, ReadFilePath_CachedFileButIsDeletedFromDisk_ReturnsEmpty)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, fileId));
    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, "TestTag"), FileCache::Persistent));

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    ASSERT_EQ(BeFileNameStatus::Success, cachedFilePath.BeDeleteFile());

    EXPECT_THAT(cache->ReadFilePath(fileId), IsEmpty());
    }

TEST_F(DataSourceCacheTests, ReadFileCacheTag_NotCachedFile_ReturnsEmpty)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, fileId));

    EXPECT_THAT(cache->ReadFileCacheTag(fileId), IsEmpty());
    }

TEST_F(DataSourceCacheTests, ReadFileCacheTag_CachedFile_ReturnsTag)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, fileId));
    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, "TestTag"), FileCache::Persistent));

    EXPECT_THAT(cache->ReadFileCacheTag(fileId), Eq("TestTag"));
    }

TEST_F(DataSourceCacheTests, ReadFileCacheTag_CachedFileButIsDeletedFromDisk_ReturnsEmpty)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, fileId));
    ASSERT_EQ(SUCCESS, cache->CacheFile(fileId, WSFileResponse(StubFile(), HttpStatus::OK, "TestTag"), FileCache::Persistent));

    BeFileName cachedFilePath = cache->ReadFilePath(fileId);
    ASSERT_EQ(BeFileNameStatus::Success, cachedFilePath.BeDeleteFile());

    EXPECT_THAT(cache->ReadFileCacheTag(fileId), IsEmpty());
    }

TEST_F(DataSourceCacheTests, FindInstance_ObjectIdAndNotCached_InvalidKey)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ECInstanceKey instanceKey = cache->FindInstance({"TestSchema.TestClass", "NonExisting"});

    EXPECT_FALSE(instanceKey.IsValid());
    }

TEST_F(DataSourceCacheTests, FindInstance_EmptyObjectId_ReturnsInvalid)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ECInstanceKey instanceKey = cache->FindInstance(ObjectId());
    EXPECT_FALSE(instanceKey.IsValid());
    }

TEST_F(DataSourceCacheTests, FindInstance_EmptyObjectIdAndNavigationBaseLinkedToRoot_ReturnsKeyToNavigationBase)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, ObjectId()));

    ECInstanceKey instanceKey = cache->FindInstance(ObjectId());
    EXPECT_TRUE(instanceKey.IsValid());
    }

TEST_F(DataSourceCacheTests, FindInstance_KeyToNavigationBase_ReturnsEmptyObjectId)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot(nullptr, ObjectId()));
    ECInstanceKey baseKey = cache->FindInstance(ObjectId());
    ASSERT_TRUE(baseKey.IsValid());

    EXPECT_EQ(ObjectId(), cache->FindInstance(baseKey));
    }

TEST_F(DataSourceCacheTests, FindInstance_ECInstanceKeyAndNotCached_InvalidObjectId)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ECClassCP ecClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");

    ObjectId objectId = cache->FindInstance(ECInstanceKey(ecClass->GetId(), ECInstanceId(1)));

    EXPECT_FALSE(objectId.IsValid());
    }

TEST_F(DataSourceCacheTests, FindInstance_ObjectIdAndInCache_ValidKey)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "Foo"});
    Json::Value instance;
    cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instance);
    ECInstanceKey instanceKey = cache->GetAdapter().GetInstanceKeyFromJsonInstance(instance);

    ECInstanceKey foundInstanceKey = cache->FindInstance({"TestSchema.TestClass", "Foo"});

    EXPECT_TRUE(foundInstanceKey.IsValid());
    EXPECT_EQ(instanceKey, foundInstanceKey);
    }

TEST_F(DataSourceCacheTests, FindInstance_ECInstanceKeyAndInCache_ValidObjectId)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot(nullptr, {"TestSchema.TestClass", "Foo"});
    Json::Value instance;
    cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instance);
    ECInstanceKey instanceKey = cache->GetAdapter().GetInstanceKeyFromJsonInstance(instance);

    ObjectId objectId = cache->FindInstance(instanceKey);

    EXPECT_TRUE(objectId.IsValid());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "Foo"), objectId);
    }

TEST_F(DataSourceCacheTests, FindInstance_ObjectIdAndChangeDeleted_ValidKey)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instanceKey));

    ECInstanceKey foundInstanceKey = cache->FindInstance({"TestSchema.TestClass", "Foo"});

    EXPECT_TRUE(foundInstanceKey.IsValid());
    EXPECT_EQ(instanceKey, foundInstanceKey);
    }

TEST_F(DataSourceCacheTests, FindInstance_ECInstanceKeyAndChangeDeleted_ValidObjectId)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    auto instanceKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instanceKey));

    ObjectId objectId = cache->FindInstance(instanceKey);

    EXPECT_TRUE(objectId.IsValid());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "Foo"), objectId);
    }

TEST_F(DataSourceCacheTests, FindRelationship_NoRelationshipWithSuchEnds_ReturnsInvalidKey)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto instanceA = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto instanceB = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    EXPECT_FALSE(cache->FindRelationship(*relClass, instanceA, instanceB).IsValid());
    EXPECT_FALSE(cache->FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}).IsValid());
    }

TEST_F(DataSourceCacheTests, FindRelationship_RelationshipWithSuchEndsExists_ReturnsRelationshipKey)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    ASSERT_EQ(SUCCESS, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

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
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto instanceA = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto instanceB = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache->GetAdapter().RelateInstances(relClass, instanceA, instanceB);

    EXPECT_EQ(ObjectId(), cache->FindRelationship(relationship));
    }

TEST_F(DataSourceCacheTests, FindRelationship_CachedRelationshipExists_ReturnsRelationshipObjectId)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    ASSERT_EQ(SUCCESS, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache->FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"});

    EXPECT_EQ(ObjectId({"TestSchema.TestRelationshipClass", "AB"}), cache->FindRelationship(relationship));
    }

TEST_F(DataSourceCacheTests, ReadInstancesConnectedToRootMap_DifferentClassInstancesLinked_ReturnsOnlyCachedInstanceIds)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("Foo", {"TestSchema.TestClass", "A"});
    cache->LinkInstanceToRoot("Foo", {"TestSchema.TestClass2", "B"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "C"});
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "A"}), "Foo");
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    ECInstanceKeyMultiMap map;
    cache->ReadInstancesConnectedToRootMap("Foo", map);

    EXPECT_EQ(3, map.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "A"}), map));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass2", "B"}), map));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "C"}), map));
    }

TEST_F(DataSourceCacheTests, ReadInstancesLinkedToRoot_DifferentClassInstancesLinked_ReturnsIds)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    cache->LinkInstanceToRoot("Foo", {"TestSchema.TestClass", "A"});
    cache->LinkInstanceToRoot("Foo", {"TestSchema.TestClass2", "B"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "C"});
    CachedResponseKey responseKey(cache->FindInstance({"TestSchema.TestClass", "A"}), "Foo");
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse()));

    ECInstanceKeyMultiMap map;
    cache->ReadInstancesLinkedToRoot("Foo", map);

    EXPECT_EQ(2, map.size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass", "A"}), map));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache->FindInstance({"TestSchema.TestClass2", "B"}), map));
    }

TEST_F(DataSourceCacheTests, ReadRootSyncDate_NoRoot_ReturnsInvalid)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    EXPECT_FALSE(cache->ReadRootSyncDate("NonExistingRoot").IsValid());
    }

TEST_F(DataSourceCacheTests, ReadRootSyncDate_ExistingRootButDateNotSet_ReturnsInvalid)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Foo", {"TestSchema.TestClass", "A"}));
    EXPECT_FALSE(cache->ReadRootSyncDate("Foo").IsValid());
    }

TEST_F(DataSourceCacheTests, ReadRootSyncDate_DateSetAutomatically_ReturnsSameDate)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

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
    shared_ptr<DataSourceCache> cache = GetTestCache();

    DateTime syncDate(DateTime::Kind::Utc, 1989, 02, 14, 0, 0, 0);
    ASSERT_EQ(SUCCESS, cache->SetRootSyncDate("Foo", syncDate));
    EXPECT_EQ(syncDate, cache->ReadRootSyncDate("Foo"));
    }

TEST_F(DataSourceCacheTests, ReadRootSyncDate_RootRemovedAfterSyncDateWasSet_ReturnsInvalid)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("Foo", {"TestSchema.TestClass", "A"}));
    ASSERT_EQ(SUCCESS, cache->SetRootSyncDate("Foo"));
    ASSERT_EQ(SUCCESS, cache->RemoveRoot("Foo"));

    EXPECT_FALSE(cache->ReadRootSyncDate("Foo").IsValid());
    }

TEST_F(DataSourceCacheTests, ReadResponseAccessDate_NoResponse_ReturnsInvalid)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache, "NonExisting");

    EXPECT_THAT(cache->ReadResponseAccessDate(responseKey).IsValid(), false);
    }

TEST_F(DataSourceCacheTests, ReadResponseAccessDate_CachedResponse_ReturnsInvalid)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));

    EXPECT_THAT(cache->ReadResponseAccessDate(responseKey).IsValid(), false);
    }

TEST_F(DataSourceCacheTests, ReadResponseAccessDate_AccessDateSetAutomatically_ReturnsSameDate)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));

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
    shared_ptr<DataSourceCache> cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));

    DateTime date(DateTime::Kind::Utc, 1989, 02, 14, 0, 0, 0);
    ASSERT_EQ(SUCCESS, cache->SetResponseAccessDate(responseKey, date));
    EXPECT_EQ(date, cache->ReadResponseAccessDate(responseKey));
    }

TEST_F(DataSourceCacheTests, SetResponseAccessDate_ResponseNotCached_ReturnsErrorAndDateNotSet)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    EXPECT_EQ(ERROR, cache->SetResponseAccessDate(responseKey));
    EXPECT_THAT(cache->ReadResponseAccessDate(responseKey).IsValid(), false);
    }

TEST_F(DataSourceCacheTests, RemoveTemporaryResponses_NoResponsesWithName_ReturnsSuccess)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey(cache->FindOrCreateRoot("Foo"), "Test");

    ASSERT_EQ(SUCCESS, cache->SetupRoot("Foo", CacheRootPersistence::Temporary));

    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, StubInstances().ToWSObjectsResponse()));

    EXPECT_THAT(cache->RemoveTemporaryResponses("Other", DateTime::GetCurrentTimeUtc()), SUCCESS);
    EXPECT_THAT(cache->IsResponseCached(responseKey), true);
    }

TEST_F(DataSourceCacheTests, RemoveTemporaryResponses_ResponsesWithSameNameButPersistent_LeavesResponses)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey1(cache->FindOrCreateRoot("A"), "Test");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("B"), "Test");
    CachedResponseKey responseKey3(cache->FindOrCreateRoot("C"), "Test");
    CachedResponseKey responseKey4(cache->FindOrCreateRoot("D"), "Test");

    ASSERT_EQ(SUCCESS, cache->SetupRoot("A", CacheRootPersistence::Temporary));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("B", CacheRootPersistence::Temporary));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("C", CacheRootPersistence::Full));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("D", CacheRootPersistence::Full));

    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey1, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey2, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey3, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey4, StubInstances().ToWSObjectsResponse()));

    EXPECT_THAT(cache->RemoveTemporaryResponses("Test", DateTime::GetCurrentTimeUtc()), SUCCESS);

    EXPECT_THAT(cache->IsResponseCached(responseKey1), false);
    EXPECT_THAT(cache->IsResponseCached(responseKey2), false);
    EXPECT_THAT(cache->IsResponseCached(responseKey3), true);
    EXPECT_THAT(cache->IsResponseCached(responseKey4), true);
    }

TEST_F(DataSourceCacheTests, RemoveTemporaryResponses_ResponseWithAccessDateLaterThanUsed_LeavesResponse)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey1(cache->FindOrCreateRoot("A"), "Test");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("B"), "Test");

    ASSERT_EQ(SUCCESS, cache->SetupRoot("A", CacheRootPersistence::Temporary));
    ASSERT_EQ(SUCCESS, cache->SetupRoot("A", CacheRootPersistence::Temporary));

    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey1, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey2, StubInstances().ToWSObjectsResponse()));

    ASSERT_EQ(SUCCESS, cache->SetResponseAccessDate(responseKey1, DateTime(DateTime::Kind::Utc, 1989, 02, 14, 0, 0, 0)));
    ASSERT_EQ(SUCCESS, cache->SetResponseAccessDate(responseKey2, DateTime(DateTime::Kind::Utc, 2010, 01, 01, 0, 0, 0)));

    EXPECT_THAT(cache->RemoveTemporaryResponses("Test", DateTime(DateTime::Kind::Utc, 2000, 01, 01, 0, 0, 0)), SUCCESS);
    EXPECT_THAT(cache->IsResponseCached(responseKey1), false);
    EXPECT_THAT(cache->IsResponseCached(responseKey2), true);
    }

TEST_F(DataSourceCacheTests, RemoveResponses_ResponsesWithSameName_RemovesResponses)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    CachedResponseKey responseKey1(cache->FindOrCreateRoot("A"), "Test");
    CachedResponseKey responseKey2(cache->FindOrCreateRoot("B"), "Test");
    CachedResponseKey responseKey3(cache->FindOrCreateRoot("C"), "Other");

    ASSERT_EQ(SUCCESS, cache->SetupRoot("A", CacheRootPersistence::Temporary));

    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey1, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey2, StubInstances().ToWSObjectsResponse()));
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey3, StubInstances().ToWSObjectsResponse()));

    ASSERT_EQ(SUCCESS, cache->RemoveResponses("Test"));

    EXPECT_THAT(cache->IsResponseCached(responseKey1), false);
    EXPECT_THAT(cache->IsResponseCached(responseKey2), false);
    EXPECT_THAT(cache->IsResponseCached(responseKey3), true);
    }

TEST_F(DataSourceCacheTests, GetExtendedData_UpdatedData_WorksFine)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache);

    auto extendedData = cache->GetExtendedDataAdapter().GetData(instance);
    extendedData.SetValue("A", "B");
    ASSERT_EQ(SUCCESS, cache->GetExtendedDataAdapter().UpdateData(extendedData));

    auto extendedData2 = cache->GetExtendedDataAdapter().GetData(instance);
    EXPECT_THAT(extendedData2.GetValue("A"), Eq("B"));
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsInstanceThatWasLocallyModified_UpdatesPropertiesThatWereNotChanged)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}, {"TestProperty2", "B"}});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(key, instances.ToWSObjectsResponse()));
    auto instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});

    auto properties = ToJson(R"({"TestProperty" : "A", "TestProperty2" : "ModifiedValueB"})");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "NewValueA"}, {"TestProperty2", "NewValueB"}});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instanceJson));

    EXPECT_EQ("NewValueA", instanceJson["TestProperty"].asString());
    EXPECT_EQ("ModifiedValueB", instanceJson["TestProperty2"].asString());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_FullResultContainsInstanceThatWasLocallyModified_UpdatesPropertiesThatWereNotChanged)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*");

    bset<ObjectId> rejected;
    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}, {"TestProperty2", "B"}});
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key, instances.ToWSObjectsResponse(), rejected, &query));
    auto instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    ASSERT_TRUE(rejected.empty());

    auto properties = ToJson(R"({"TestProperty" : "A", "TestProperty2" : "ModifiedValueB"})");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    rejected.clear();
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "NewValueA"}, {"TestProperty2", "NewValueB"}});
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key, instances.ToWSObjectsResponse(), rejected, &query));
    ASSERT_TRUE(rejected.empty());

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instanceJson));

    EXPECT_EQ("NewValueA", instanceJson["TestProperty"].asString());
    EXPECT_EQ("ModifiedValueB", instanceJson["TestProperty2"].asString());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_PartialResultsContainInstanceThatWasLocallyModified_RejectsAndDoesNotUpdateInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    bset<ObjectId> rejected;
    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}, {"TestProperty2", "B"}});
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key, instances.ToWSObjectsResponse(), rejected, nullptr));
    auto instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    ASSERT_TRUE(rejected.empty());

    auto properties = ToJson(R"({"TestProperty" : "A", "TestProperty2" : "ModifiedValueB"})");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    rejected.clear();
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "NewValueA"}, {"TestProperty2", "NewValueB"}});
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key, instances.ToWSObjectsResponse(), rejected, nullptr));
    EXPECT_EQ(1, rejected.size());
    EXPECT_CONTAINS(rejected, ObjectId("TestSchema.TestClass", "Foo"));

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instanceJson));

    EXPECT_EQ("A", instanceJson["TestProperty"].asString());
    EXPECT_EQ("ModifiedValueB", instanceJson["TestProperty2"].asString());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_FullResultContainsInstanceThatWasLocallyDeleted_IgnoresInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));

    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("*");
    bset<ObjectId> rejected;
    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key, instances.ToWSObjectsResponse(), rejected, &query));
    ASSERT_TRUE(rejected.empty());

    Json::Value instanceJson;
    EXPECT_EQ(CacheStatus::DataNotCached, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instanceJson));
    EXPECT_EQ(Json::Value::null, instanceJson);
    }

TEST_F(DataSourceCacheTests, CacheResponse_ResultContainsInstanceThatWasLocallyDeletedAndRelatedInstance_IgnoresDeletedInstanceAndUpdatesRelatedOne)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "OldA"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {{"TestProperty", "OldB"}});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(cache->FindInstance({"TestSchema.TestClass", "A"})));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "NewA"}})
        .AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"}, {{"TestProperty", "NewB"}});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(key, instances.ToWSObjectsResponse()));

    Json::Value instanceJson;
    EXPECT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "B"}, instanceJson));
    EXPECT_EQ("NewB", instanceJson["TestProperty"].asString());
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_PartialResultsContainsInstanceThatWasLocallyDeleted_IgnoresInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));

    bset<ObjectId> rejected;
    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key, instances.ToWSObjectsResponse(), rejected, nullptr));
    ASSERT_TRUE(rejected.empty());

    Json::Value instanceJson;
    EXPECT_EQ(CacheStatus::DataNotCached, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instanceJson));
    EXPECT_EQ(Json::Value::null, instanceJson);
    }

TEST_F(DataSourceCacheTests, CachePartialResponse_PartialResultsContainsInstanceThatWasCachedAsPartial_UpdatesInstance)
    {
    shared_ptr<DataSourceCache> cache = GetTestCache();

    bset<ObjectId> rejected;
    auto key = StubCachedResponseKey(*cache);
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}});
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key, instances.ToWSObjectsResponse(), rejected, nullptr));
    auto instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    ASSERT_TRUE(rejected.empty());

    rejected.clear();
    instances.Clear();
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "NewValueA"}});
    ASSERT_EQ(SUCCESS, cache->CachePartialResponse(key, instances.ToWSObjectsResponse(), rejected, nullptr));
    EXPECT_EQ(0, rejected.size());

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass", "Foo"}, instanceJson));

    EXPECT_EQ("NewValueA", instanceJson["TestProperty"].asString());
    }

#endif
