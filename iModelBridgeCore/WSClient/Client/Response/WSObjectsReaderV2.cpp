/*--------------------------------------------------------------------------------------+
|
|  $Source: Client/Response/WSObjectsReaderV2.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/Response/WSObjectsReaderV2.h>

static rapidjson::Value s_emptyJsonObject(rapidjson::Type::kObjectType);
static rapidjson::Value s_emptyArrayJsonObject(rapidjson::Type::kArrayType);

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReaderV2::WSObjectsReaderV2(bool quoteInstanceETags) :
m_instanceCount(0),
m_quoteInstanceETags(quoteInstanceETags)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReaderV2::~WSObjectsReaderV2()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<WSObjectsReaderV2> WSObjectsReaderV2::Create(bool quoteInstanceETags)
    {
    return std::shared_ptr<WSObjectsReaderV2>(new WSObjectsReaderV2(quoteInstanceETags));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::SizeType WSObjectsReaderV2::GetInstanceCount() const
    {
    return m_instanceCount;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::Instances WSObjectsReaderV2::ReadInstances(std::shared_ptr<const rapidjson::Value> data)
    {
    m_data = data;

    if (nullptr == m_data ||
        !m_data->IsObject() ||
        !m_data->HasMember("instances") ||
        !(*m_data)["instances"].IsArray())
        {
        BeAssert(false && "Bad format");
        m_data = nullptr;
        m_instanceCount = 0;
        return Instances(shared_from_this());
        }

    m_instanceCount = (*m_data)["instances"].Size();
    return Instances(shared_from_this());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsReaderV2::HasReadErrors() const
    {
    return nullptr == m_data;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectId WSObjectsReaderV2::ReadObjectId(const rapidjson::Value& instanceData) const
    {
    if (!instanceData.IsObject() ||
        !instanceData["schemaName"].IsString() ||
        !instanceData["className"].IsString() ||
        !instanceData["instanceId"].IsString())
        {
        return ObjectId();
        }

    Utf8String schemaName = instanceData["schemaName"].GetString();
    Utf8String className = instanceData["className"].GetString();
    Utf8String remoteId = instanceData["instanceId"].GetString();

    return ObjectId(schemaName, className, remoteId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::Instance WSObjectsReaderV2::ReadInstance(const rapidjson::Value& instanceData) const
    {
    ObjectId objectId = ReadObjectId(instanceData);
    if (objectId.IsEmpty())
        {
        BeAssert(false && "Bad format");
        return Instance(shared_from_this());
        }

    const rapidjson::Value* instanceProperties = &instanceData["properties"];
    if (!instanceProperties->IsObject())
        {
        BeAssert(false && "Bad format");
        return Instance(shared_from_this());
        }

    const rapidjson::Value* relationshipInstancesData = nullptr;
    if (instanceData.HasMember("relationshipInstances"))
        {
        if (!instanceData["relationshipInstances"].IsArray())
            {
            BeAssert(false && "Bad format");
            return Instance(shared_from_this());
            }
        relationshipInstancesData = &instanceData["relationshipInstances"];
        }
    else
        {
        relationshipInstancesData = &s_emptyArrayJsonObject;
        }

    return Instance(shared_from_this(), objectId, &instanceData, instanceProperties, relationshipInstancesData);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::Instance WSObjectsReaderV2::GetInstance(rapidjson::SizeType index) const
    {
    if (index >= m_instanceCount)
        {
        BeAssert(false && "Index out of range");
        return Instance(nullptr, ObjectId(), nullptr, nullptr, nullptr);
        }

    const rapidjson::Value& instanceData = (*m_data)["instances"][index];
    return ReadInstance(instanceData);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::Instance WSObjectsReaderV2::GetRelatedInstance(const rapidjson::Value* relatedInstance) const
    {
    if (nullptr == relatedInstance)
        {
        return Instance(shared_from_this());
        }
    return ReadInstance(*relatedInstance);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::RelationshipInstance WSObjectsReaderV2::GetRelationshipInstance(const rapidjson::Value* relationshipInstance) const
    {
    if (nullptr == relationshipInstance)
        {
        return RelationshipInstance(shared_from_this());
        }
    ObjectId objectId = ReadObjectId(*relationshipInstance);
    if (objectId.IsEmpty())
        {
        BeAssert(false && "Bad format");
        return RelationshipInstance(shared_from_this());
        }

    const rapidjson::Value* instanceProperties = &(*relationshipInstance)["properties"];
    if (instanceProperties->IsNull())
        {
        instanceProperties = &s_emptyJsonObject;
        }
    else if (!instanceProperties->IsObject())
        {
        BeAssert(false && "Bad format");
        return RelationshipInstance(shared_from_this());
        }

    BentleyApi::ECN::ECRelatedInstanceDirection direction;
    RapidJsonValueCR directionData = (*relationshipInstance)["direction"];
    if (0 == strcmp("forward", directionData.GetString()))
        {
        direction = BentleyApi::ECN::ECRelatedInstanceDirection::Forward;
        }
    else if (0 == strcmp("backward", directionData.GetString()))
        {
        direction = BentleyApi::ECN::ECRelatedInstanceDirection::Backward;
        }
    else
        {
        BeAssert(false && "Bad format");
        return RelationshipInstance(shared_from_this());
        }

    const rapidjson::Value* relatedInstance = &(*relationshipInstance)["relatedInstance"];
    if (!relatedInstance->IsObject())
        {
        BeAssert(false && "Bad format");
        return RelationshipInstance(shared_from_this());
        }

    return RelationshipInstance(shared_from_this(), objectId, relationshipInstance, instanceProperties, relatedInstance, direction);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSObjectsReaderV2::GetInstanceETag(const rapidjson::Value* instance) const
    {
    Utf8String eTag;
    if (!(*instance)["eTag"].IsString())
        {
        return eTag;
        }

    Utf8CP eTagStr = (*instance)["eTag"].GetString();

    if (m_quoteInstanceETags)
        {
        eTag.Sprintf("\"%s\"", eTagStr);
        }
    else
        {
        eTag = eTagStr;
        }

    return eTag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::SizeType WSObjectsReaderV2::GetRelationshipInstanceCount(const rapidjson::Value* relationshipInstances) const
    {
    if (nullptr == relationshipInstances)
        return 0;

    if (!relationshipInstances->IsArray())
        return 0;

    return relationshipInstances->Size();
    }