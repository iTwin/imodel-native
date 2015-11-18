/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/CachingTestsHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/CachingDataSource.h>
#include <WebServices/Cache/Persistence/DataSourceCache.h>
#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../../Utils/WebServicesTestsHelper.h"

// Operator for comparisons
bool operator <= (const DateTime& lhs, const DateTime& rhs);
bool operator >= (const DateTime& lhs, const DateTime& rhs);

// Returns true if relationship between instances exist

bool VerifyHasRelationship
(
std::shared_ptr<DataSourceCache> cache,
ECRelationshipClassCP relClass,
ObjectIdCR sourceId,
ObjectIdCR targetId
);

bool VerifyHasRelationship
(
std::shared_ptr<DataSourceCache> cache,
ECRelationshipClassCP relClass,
ECInstanceKeyCR source,
ECInstanceKeyCR target
);

ECInstanceKey FindRelationship
(
std::shared_ptr<DataSourceCache> cache,
ECRelationshipClassCP relClass,
ObjectIdCR sourceId,
ObjectIdCR targetId
);

ECInstanceKey FindRelationship
(
std::shared_ptr<DataSourceCache> cache,
ECRelationshipClassCP relClass,
ECInstanceKeyCR source,
ECInstanceKeyCR target
);

int CountClassInstances(IDataSourceCache& ds, Utf8StringCR classKey);

ICachingDataSource::ObjectsResult StubObjectsResult
(
JsonValueCR jsonInstances = Json::arrayValue,
ICachingDataSource::DataOrigin origin = ICachingDataSource::DataOrigin::CachedData
);

// Create result with IsModified == false
WSObjectsResponse StubWSObjectsResponseNotModified();
WSObjectsResponse StubWSObjectsResponseV2(Utf8StringCR jsonBody, Utf8StringCR eTag = "");
WSObjectsResult StubWSObjectsResultNotModified();
WSObjectsResult StubWSObjectsResultInvalidInstances();

WSFileResponse StubWSFileResponse(BeFileNameCR filePath, Utf8StringCR eTag = "");
WSFileResponse StubWSFileResponseNotModified();

WSFileResult StubWSFileResult(BeFileNameCR filePath, Utf8StringCR eTag = "");
WSFileResult StubWSFileResultNotModified();

WSInfoResult StubWSInfoResult(BeVersion webApiVersion = BeVersion(2, 0));

WSObjectsResult StubWSObjectsResult(ObjectIdCR objectId);
WSCreateObjectResult StubWSCreateObjectResult();
WSCreateObjectResult StubWSCreateObjectResult(ObjectIdCR objectId);
WSCreateObjectResult StubWSCreateObjectResult(ObjectIdCR sourceId, ObjectIdCR relationshipId, ObjectIdCR targetId);

CacheEnvironment StubCacheEnvironemnt();

Utf8String StubSchemaXml(Utf8StringCR schemaName = "TestSchema", Utf8String optionalSchemaPrefix = "");
ECSchemaPtr StubSchema(Utf8StringCR schemaName = "TestSchema", Utf8String optionalSchemaPrefix = "");
ECSchemaPtr StubRelationshipSchema(Utf8StringCR schemaName = "TestSchema", Utf8StringCR classA = "A", Utf8StringCR classB = "B", Utf8StringCR relAB = "AB");

IECInstancePtr StubInstance(ECSchemaPtr ecSchema);
IECInstancePtr StubInstance(ECClassCP ecClass);

ECInstanceKey StubECInstanceKey(int64_t classId = 1, int64_t instanceId = 1);
bpair<ECClassId, ECInstanceId> StubECInstanceKeyPair(int64_t classId = 1, int64_t instanceId = 1);
ECInstanceKey StubInstanceInCache(IDataSourceCache& cache, ObjectIdCR objectId = ObjectId("TestSchema.TestClass", "Foo"), std::map<Utf8String, Json::Value> properties = {});
ECInstanceKey StubNonExistingInstanceKey(IDataSourceCache& cache, Utf8StringCR classKey = "TestSchema.TestClass", uint64_t instanceId = 1);
ECInstanceKeyMultiMap StubECInstanceKeyMultiMap(const std::vector<ECInstanceKey>& keys = {});

CachedResponseKey StubCachedResponseKey(IDataSourceCache& cache, Utf8StringCR name = "TestQuery");