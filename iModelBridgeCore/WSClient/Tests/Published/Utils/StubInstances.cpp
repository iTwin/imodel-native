/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Utils/StubInstances.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StubInstances.h"
#include <WebServices/Client/Response/WSObjectsReaderV2.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_WSCLIENT_UNITTESTS

void StubInstances::Clear()
    {
    m_instances.clear();
    }

StubInstances::StubRelationshipInstances StubInstances::Add(ObjectIdCR objectId, std::map<Utf8String, Json::Value> properties, Utf8StringCR eTag)
    {
    StubInstance instance;

    instance.objectId = objectId;
    instance.properties = properties;
    instance.eTag = eTag;
    m_instances.push_back(instance);

    return StubRelationshipInstances(m_instances.back().relationshipInstances);
    }

Json::Value StubInstances::ConvertStubInstanceToJson(const StubInstance& instance) const
    {
    Json::Value instanceJson;

    instanceJson["schemaName"] = instance.objectId.schemaName;
    instanceJson["className"] = instance.objectId.className;
    instanceJson["instanceId"] = instance.objectId.remoteId;
    instanceJson["eTag"] = instance.eTag;
    instanceJson["properties"] = Json::objectValue;

    for (auto& property : instance.properties)
        {
        instanceJson["properties"][property.first] = property.second;
        }

    for (auto& relationshipInstance : instance.relationshipInstances)
        {
        instanceJson["relationshipInstances"].append(ConvertStubRelationshipInstanceToJson(*relationshipInstance));
        }

    return instanceJson;
    }

Json::Value StubInstances::ConvertStubRelationshipInstanceToJson(const StubRelationshipInstance& instance) const
    {
    Json::Value instanceJson;

    instanceJson["schemaName"] = instance.objectId.schemaName;
    instanceJson["className"] = instance.objectId.className;
    instanceJson["instanceId"] = instance.objectId.remoteId;
    instanceJson["properties"] = Json::objectValue;

    for (auto& property : instance.properties)
        {
        instanceJson["properties"][property.first] = property.second;
        }

    instanceJson["relatedInstance"] = ConvertStubInstanceToJson(instance.relatedInstance);
    instanceJson["direction"] = BentleyApi::ECN::ECRelatedInstanceDirection::Forward == instance.direction ? "forward" : "backward";

    return instanceJson;
    }

WSObjectsResponse StubInstances::ToWSObjectsResponse(Utf8StringCR eTag) const
    {
    auto body = HttpStringBody::Create(ToJsonWebApiV2());
    auto reader = WSObjectsReaderV2::Create();

    return WSObjectsResponse(reader, body, HttpStatus::OK, eTag);
    }

WSObjectsResult StubInstances::ToWSObjectsResult(Utf8StringCR eTag) const
    {
    return WSObjectsResult::Success(ToWSObjectsResponse(eTag));
    }

WSChangesetResult StubInstances::ToWSChangesetResult() const
    {
    auto body = HttpStringBody::Create(ToChangesetResponseJson());
    return WSChangesetResult::Success(body);
    }

Utf8String StubInstances::ToJsonWebApiV1() const
    {
    Utf8String mainSchemaName;
    Json::Value dataJson(Json::objectValue);

    if (!m_instances.empty())
        {
        mainSchemaName = m_instances.front().objectId.schemaName;
        }

    for (auto& instance : m_instances)
        {
        BeAssert(instance.objectId.schemaName.Equals(mainSchemaName) && "Only one schema is supported in WebApi1");
        BeAssert(instance.relationshipInstances.empty() && "Relationship instances are not supported in WebApi1");

        Json::Value instanceJson;
        instanceJson["$id"] = instance.objectId.remoteId;
        for (auto& keyValue : instance.properties)
            {
            instanceJson[keyValue.first] = keyValue.second;
            }

        dataJson[instance.objectId.className].append(instanceJson);
        }

    return dataJson.toStyledString();
    }

Utf8String StubInstances::ToJsonWebApiV2() const
    {
    Json::Value dataJson;
    dataJson["instances"] = Json::arrayValue;

    for (auto& instance : m_instances)
        {
        dataJson["instances"].append(ConvertStubInstanceToJson(instance));
        }

    return dataJson.toStyledString();
    }

Utf8String StubInstances::ToChangesetResponseJson() const
    {
    Json::Value dataJson;
    dataJson["changedInstances"] = Json::arrayValue;

    for (auto& instance : m_instances)
        {
        auto& instanceJson = dataJson["changedInstances"].append(Json::objectValue);
        instanceJson["instanceAfterChange"] = ConvertStubInstanceToJson(instance);
        }

    return dataJson.toStyledString();
    }

StubInstances::StubRelationshipInstances::StubRelationshipInstances(bvector<std::shared_ptr<StubRelationshipInstance>>& relationships) :
m_relationshipInstances(relationships)
    {}

StubInstances::StubRelationshipInstances StubInstances::StubRelationshipInstances::AddRelated
(
ObjectIdCR relationshipObjectId,
ObjectIdCR relatedObjectId,
std::map<Utf8String, Json::Value> relatedProperties,
BentleyApi::ECN::ECRelatedInstanceDirection direction,
std::map<Utf8String, Json::Value> relationshipProperties
)
    {
    auto relationshipInstance = std::make_shared<StubRelationshipInstance>();

    relationshipInstance->objectId = relationshipObjectId;
    relationshipInstance->direction = direction;
    relationshipInstance->properties = relationshipProperties;
    relationshipInstance->relatedInstance.objectId = relatedObjectId;
    relationshipInstance->relatedInstance.properties = relatedProperties;

    m_relationshipInstances.push_back(relationshipInstance);

    return StubRelationshipInstances(m_relationshipInstances.back()->relatedInstance.relationshipInstances);
    }
