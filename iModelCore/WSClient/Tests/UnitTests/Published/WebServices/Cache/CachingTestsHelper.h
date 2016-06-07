/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/CachingTestsHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/CachingDataSource.h>
#include <WebServices/Cache/Persistence/DataSourceCache.h>
#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../../Utils/WebServicesTestsHelper.h"

// Returns true if relationship between instances exist

bool VerifyHasRelationship(
    std::shared_ptr<DataSourceCache> cache,
    Utf8StringCR relClassKey,
    ObjectIdCR sourceId,
    ObjectIdCR targetId);

bool VerifyHasRelationship(
    std::shared_ptr<DataSourceCache> cache,
    ECRelationshipClassCP relClass,
    ObjectIdCR sourceId,
    ObjectIdCR targetId);

bool VerifyHasRelationship(
    std::shared_ptr<DataSourceCache> cache,
    ECRelationshipClassCP relClass,
    ECInstanceKeyCR source,
    ECInstanceKeyCR target);

ECInstanceKey FindRelationship(
    std::shared_ptr<DataSourceCache> cache,
    ECRelationshipClassCP relClass,
    ObjectIdCR sourceId,
    ObjectIdCR targetId);

ECInstanceKey FindRelationship(
    std::shared_ptr<DataSourceCache> cache,
    ECRelationshipClassCP relClass,
    ECInstanceKeyCR source,
    ECInstanceKeyCR target);

int CountClassInstances(IDataSourceCache& ds, Utf8StringCR classKey);
bool DoesInstanceExist(IDataSourceCache& ds, ECInstanceKeyCR key);

Json::Value ReadInstance(IDataSourceCache& ds, ECInstanceKeyCR key);
Json::Value ReadModifiedProperties(IDataSourceCache& ds, ECInstanceKeyCR key);

ICachingDataSource::ObjectsResult StubObjectsResult(
    JsonValueCR jsonInstances = Json::arrayValue,
    ICachingDataSource::DataOrigin origin = ICachingDataSource::DataOrigin::CachedData);

    // Create result with IsModified == false
WSObjectsResponse StubWSObjectsResponseNotModified(Utf8StringCR skipToken = "");
WSObjectsResponse StubWSObjectsResponseV2(Utf8StringCR jsonBody, Utf8StringCR eTag = "");
WSObjectsResult StubWSObjectsResultNotModified(Utf8StringCR skipToken = "");
WSObjectsResult StubWSObjectsResultInvalidInstances();

WSFileResponse StubWSFileResponse(BeFileNameCR filePath, Utf8StringCR eTag = "");
WSFileResponse StubWSFileResponseNotModified();

WSFileResult StubWSFileResult(BeFileNameCR filePath, Utf8StringCR eTag = "");
WSFileResult StubWSFileResultNotModified();

WSInfoResult StubWSInfoResult(BeVersion webApiVersion = BeVersion(2, 0));

WSObjectsResult StubWSObjectsResult();
WSObjectsResult StubWSObjectsResult(ObjectIdCR objectId);
WSCreateObjectResult StubWSCreateObjectResult();
WSCreateObjectResult StubWSCreateObjectResult(ObjectIdCR objectId);
WSCreateObjectResult StubWSCreateObjectResult(ObjectIdCR sourceId, ObjectIdCR relationshipId, ObjectIdCR targetId);

CacheEnvironment StubCacheEnvironemnt();

Utf8String StubSchemaXml(Utf8StringCR schemaName = "TestSchema", Utf8String optionalSchemaPrefix = "");

ECSchemaPtr StubSchema(Utf8StringCR schemaName = "TestSchema", Utf8String optionalSchemaPrefix = "");

ECSchemaPtr StubRelationshipSchema(
    Utf8StringCR schemaName = "TestSchema",
    Utf8StringCR classA = "A",
    Utf8StringCR classB = "B",
    Utf8StringCR relAB = "AB");

IECInstancePtr StubInstance(ECSchemaPtr ecSchema);

IECInstancePtr StubInstance(ECClassCP ecClass);

ECInstanceKey StubECInstanceKey(int64_t classId = 1, int64_t instanceId = 1);

bpair<ECClassId, ECInstanceId> StubECInstanceKeyPair(int64_t classId = 1, int64_t instanceId = 1);

ECInstanceKey StubInstanceInCache(
    IDataSourceCache& cache,
    ObjectIdCR objectId = ObjectId("TestSchema.TestClass", "Foo"),
    std::map<Utf8String, Json::Value> properties = {});

ECInstanceKey StubInstanceInCacheJson(IDataSourceCache& cache, ObjectIdCR objectId, JsonValueCR properties);

ECInstanceKey StubNonExistingInstanceKey(IDataSourceCache& cache, Utf8StringCR classKey = "TestSchema.TestClass", uint64_t instanceId = 1);

ECInstanceKeyMultiMap StubECInstanceKeyMultiMap(const std::vector<ECInstanceKey>& keys = {});

CachedResponseKey StubInstancesInCache(
    IDataSourceCache& cache,
    StubInstances& instances,
    Utf8StringCR root = BeGuid().ToString(),
    Utf8String responseName = BeGuid().ToString());

ECInstanceKey StubRelationshipInCache(
    IDataSourceCache& cache,
    ObjectIdCR relId = {"TestSchema.TestRelationshipClass", "AB"},
    ObjectIdCR source = {"TestSchema.TestClass", "A"},
    ObjectIdCR target = {"TestSchema.TestClass", "B"});

ECInstanceKey StubCreatedRelationshipInCache(
    IDataSourceCache& cache,
    Utf8StringCR relClassKey = "TestSchema.TestRelationshipClass",
    ObjectIdCR source = {"TestSchema.TestClass", "A"},
    ObjectIdCR target = {"TestSchema.TestClass", "B"});

ECInstanceKey StubCreatedRelationshipInCache(
    IDataSourceCache& cache,
    IChangeManager::SyncStatus status,
    Utf8StringCR relClassKey = "TestSchema.TestRelationshipClass",
    ObjectIdCR source = {"TestSchema.TestClass", "A"},
    ObjectIdCR target = {"TestSchema.TestClass", "B"});

ECInstanceKey StubCreatedObjectInCache(IDataSourceCache& cache, Utf8StringCR classKey = "TestSchema.TestClass");

ECInstanceKey StubCreatedObjectInCache(IDataSourceCache& cache, IChangeManager::SyncStatus status, Utf8StringCR classKey = "TestSchema.TestClass");

ECInstanceKey StubCreatedFileInCache(IDataSourceCache& cache, Utf8StringCR classKey = "TestSchema.TestClass", BeFileName filePath = StubFile());

CachedResponseKey StubCachedResponseKey(IDataSourceCache& cache, Utf8StringCR name = "TestQuery");

ObjectId StubFileInCache(
    IDataSourceCache& cache,
    FileCache location = FileCache::Temporary,
    ObjectIdCR objectId = ObjectId("TestSchema.TestClass", "Foo"),
    BeFileNameCR path = StubFile());

ObjectId StubFileInCache(IDataSourceCache& cache, BeFileNameCR path);
