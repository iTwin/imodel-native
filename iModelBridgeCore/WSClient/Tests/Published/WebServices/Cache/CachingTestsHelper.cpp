/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/CachingTestsHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CachingTestsHelper.h"

#include <WebServices/Client/Response/WSObjectsReaderV2.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

bool operator <= (const DateTime& lhs, const DateTime& rhs)
    {
    DateTime::CompareResult result = DateTime::Compare(lhs, rhs);
    BeAssert(DateTime::CompareResult::Error != result);
    return result == DateTime::CompareResult::Equals || result == DateTime::CompareResult::EarlierThan;
    }

bool operator >= (const DateTime& lhs, const DateTime& rhs)
    {
    DateTime::CompareResult result = DateTime::Compare(lhs, rhs);
    BeAssert(DateTime::CompareResult::Error != result);
    return result == DateTime::CompareResult::Equals || result == DateTime::CompareResult::LaterThan;
    }

bool VerifyHasRelationship
(
std::shared_ptr<DataSourceCache> cache,
ECRelationshipClassCP relClass,
ObjectIdCR sourceId,
ObjectIdCR targetId
)
    {
    auto source = cache->FindInstance(sourceId);
    auto target = cache->FindInstance(targetId);
    return VerifyHasRelationship(cache, relClass, source, target);
    }

bool VerifyHasRelationship
(
std::shared_ptr<DataSourceCache> cache,
ECRelationshipClassCP relClass,
ECInstanceKeyCR source,
ECInstanceKeyCR target
)
    {
    return FindRelationship(cache, relClass, source, target).IsValid();
    }

ECInstanceKey FindRelationship
(
std::shared_ptr<DataSourceCache> cache,
ECRelationshipClassCP relClass,
ObjectIdCR sourceId,
ObjectIdCR targetId
)
    {
    auto source = cache->FindInstance(sourceId);
    auto target = cache->FindInstance(targetId);
    return FindRelationship(cache, relClass, source, target);
    }

ECInstanceKey FindRelationship
(
std::shared_ptr<DataSourceCache> cache,
ECRelationshipClassCP relClass,
ECInstanceKeyCR source,
ECInstanceKeyCR target
)
    {
    return cache->GetAdapter().FindRelationship(relClass, source, target);
    }

int CountClassInstances(IDataSourceCache& ds, Utf8StringCR classKey)
    {
    return ds.GetAdapter().CountClassInstances(ds.GetAdapter().GetECClass(classKey));
    }

ICachingDataSource::ObjectsResult StubObjectsResult(JsonValueCR jsonInstances, ICachingDataSource::DataOrigin origin)
    {
    auto jsonPtr = std::make_shared<Json::Value>(jsonInstances);
    return ICachingDataSource::ObjectsResult::Success(ICachingDataSource::ObjectsData(jsonPtr, origin));
    }

WSFileResponse StubWSFileResponse(BeFileNameCR filePath, Utf8StringCR eTag)
    {
    return WSFileResponse(filePath, HttpStatus::OK, eTag);
    }

WSFileResponse StubWSFileResponseNotModified()
    {
    return WSFileResponse(BeFileName(), HttpStatus::NotModified, "");
    }

WSFileResult StubWSFileResult(BeFileNameCR filePath, Utf8StringCR eTag)
    {
    return WSFileResult::Success(StubWSFileResponse(filePath, eTag));
    }

WSFileResult StubWSFileResultNotModified()
    {
    return WSFileResult::Success(StubWSFileResponseNotModified());
    }

WSObjectsResponse StubWSObjectsResponseNotModified()
    {
    auto body = HttpStringBody::Create();
    auto reader = WSObjectsReaderV2::Create();
    return WSObjectsResponse(reader, body, HttpStatus::NotModified, "");
    }

WSObjectsResponse StubWSObjectsResponseV2(Utf8StringCR jsonBody, Utf8StringCR eTag)
    {
    auto body = HttpStringBody::Create(jsonBody);
    auto reader = WSObjectsReaderV2::Create();
    return WSObjectsResponse(reader, body, HttpStatus::OK, eTag);
    }

WSObjectsResult StubWSObjectsResultNotModified()
    {
    return WSObjectsResult::Success(StubWSObjectsResponseNotModified());
    }

WSObjectsResult StubWSObjectsResultInvalidInstances()
    {
    struct StubReader : public WSObjectsReader
        {
        public:
            static std::shared_ptr<StubReader> Create()
                {
                return std::shared_ptr<StubReader>(new StubReader());
                }

            bool HasReadErrors() const override
                {
                return true;
                }

            rapidjson::SizeType GetInstanceCount() const
                {
                return 0;
                }

            Instances ReadInstances(std::shared_ptr<const rapidjson::Value> data) override
                {
                return WSObjectsReader::Instances(shared_from_this());
                }

            Instance GetInstance(rapidjson::SizeType index) const override
                {
                return WSObjectsReader::Instance(shared_from_this());
                }

            Instance GetRelatedInstance(const rapidjson::Value* relatedInstance) const override
                {
                return WSObjectsReader::Instance(shared_from_this());
                }

            RelationshipInstance GetRelationshipInstance(const rapidjson::Value* relationshipInstance) const override
                {
                return WSObjectsReader::RelationshipInstance(shared_from_this());
                }

            Utf8String GetInstanceETag(const rapidjson::Value* instance) const override
                {
                return nullptr;
                }
        };

    WSObjectsResponse response(StubReader::Create(), HttpStringBody::Create("{}"), HttpStatus::OK, nullptr);
    return WSObjectsResult::Success(response);
    }

WSInfoResult StubWSInfoResult(BeVersion webApiVersion)
    {
    return WSInfoResult::Success(WSInfo(StubWSInfoHttpResponseWebApi(webApiVersion)));
    }

WSObjectsResult StubWSObjectsResult(ObjectIdCR objectId)
    {
    StubInstances instances;
    instances.Add(objectId);
    return WSObjectsResult::Success(instances.ToWSObjectsResponse());
    }

WSCreateObjectResult StubWSCreateObjectResult()
    {
    return WSCreateObjectResult::Error(StubHttpResponse(ConnectionStatus::CouldNotConnect));
    }

WSCreateObjectResult StubWSCreateObjectResult(ObjectIdCR objectId)
    {
    StubInstances instances;
    instances.Add(objectId);
    return instances.ToWSCreateObjectResult();
    }

WSCreateObjectResult StubWSCreateObjectResult(ObjectIdCR sourceId, ObjectIdCR relationshipId, ObjectIdCR targetId)
    {
    StubInstances instances;
    instances.Add(sourceId).AddRelated(relationshipId, targetId);
    return instances.ToWSCreateObjectResult();
    }

CacheEnvironment StubCacheEnvironemnt()
    {
    CacheEnvironment environment;
    environment.temporaryFileCacheDir = GetTestsTempDir().AppendToPath(L"test_files_temporary/");
    environment.persistentFileCacheDir = GetTestsTempDir().AppendToPath(L"test_files_persistent/");
    return environment;
    }

Utf8String StubSchemaXml(Utf8StringCR schemaName, Utf8String schemaPrefix)
    {
    if (schemaPrefix.empty())
        {
        schemaPrefix = schemaName;
        }

    Utf8PrintfString schemaXml(
        R"(<ECSchema schemaName="%s" nameSpacePrefix="%s" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">            
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="TestProperty" typeName="string" />
            </ECClass>
        </ECSchema>)",
        schemaName.c_str(),
        schemaPrefix.c_str());

    return schemaXml;
    }

ECSchemaPtr StubSchema(Utf8StringCR schemaName, Utf8String optionalSchemaPrefix)
    {
    Utf8String schemaXml = StubSchemaXml(schemaName, optionalSchemaPrefix);
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *ECSchemaReadContext::CreateContext());
    return schema;
    }

ECSchemaPtr StubRelationshipSchema(Utf8StringCR schemaName, Utf8StringCR classA, Utf8StringCR classB, Utf8StringCR relAB)
    {
    return ParseSchema(Utf8PrintfString(R"xml(
        <ECSchema schemaName="%s" nameSpacePrefix="%s" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="%s" />
            <ECClass typeName="%s" />
            <ECRelationshipClass typeName="%s">
                <Source polymorphic="False"><Class class="%s"/></Source>
                <Target polymorphic="False"><Class class="%s"/></Target>
            </ECRelationshipClass>
        </ECSchema>)xml",
        schemaName.c_str(),
        schemaName.c_str(),
        classA.c_str(),
        classB.c_str(),
        relAB.c_str(),
        classA.c_str(),
        classB.c_str()));
    }

IECInstancePtr StubInstance(ECSchemaPtr ecSchema)
    {
    IECInstancePtr instance;

    for (ECClassCP ecClass : ecSchema->GetClasses())
        {
        if (ECClassModifier::Abstract != ecClass->GetClassModifier() && ecClass->IsEntityClass())
            {
            instance = StubInstance(ecClass);
            }
        }

    EXPECT_TRUE(instance.IsValid());
    return instance;
    }

IECInstancePtr StubInstance(ECClassCP ecClass)
    {
    return ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    }

ECInstanceKey StubECInstanceKey(int64_t classId, int64_t instanceId)
    {
    return ECInstanceKey(classId, ECInstanceId(instanceId));
    }

bpair<ECClassId, ECInstanceId> StubECInstanceKeyPair(int64_t classId, int64_t instanceId)
    {
    return bpair <ECClassId, ECInstanceId>(classId, ECInstanceId(instanceId));
    }

ECInstanceKeyMultiMap StubECInstanceKeyMultiMap(const std::vector<ECInstanceKey>& keys)
    {
    ECInstanceKeyMultiMap multiMap;
    for (auto& key : keys)
        {
        multiMap.insert(ECDbHelper::ToPair(key));
        }
    return multiMap;
    }

ECInstanceKey StubInstanceInCache(IDataSourceCache& cache, ObjectIdCR objectId, std::map<Utf8String, Json::Value> properties)
    {
    if (properties.empty())
        {
        EXPECT_EQ(SUCCESS, cache.LinkInstanceToRoot(nullptr, objectId));
        }
    else
        {
        StubInstances instances;
        instances.Add(objectId, properties);
        EXPECT_EQ(SUCCESS, cache.CacheInstanceAndLinkToRoot(objectId, instances.ToWSObjectsResponse(), nullptr));
        }

    auto instance = cache.FindInstance(objectId);
    EXPECT_TRUE(instance.IsValid());
    return instance;
    }

ECInstanceKey StubNonExistingInstanceKey(IDataSourceCache& cache, Utf8StringCR classKey, uint64_t instanceId)
    {
    auto ecClass = cache.GetAdapter().GetECClass(classKey);
    auto nonExistingInstance = ECInstanceKey(ecClass->GetId(), ECInstanceId(instanceId));
    EXPECT_TRUE(nonExistingInstance.IsValid());
    EXPECT_FALSE(cache.GetAdapter().FindInstance(ecClass, Utf8PrintfString("ECInstanceId = %llu", instanceId)).IsValid());
    return nonExistingInstance;
    }

CachedResponseKey StubCachedResponseKey(IDataSourceCache& cache, Utf8StringCR name)
    {
    auto key = CachedResponseKey(cache.FindOrCreateRoot(nullptr), name);
    EXPECT_TRUE(key.IsValid());
    return key;
    }
