/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WSChangeset.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/WSChangeset.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSChangeset::WSChangeset()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WSChangeset::GetInstanceCount() const
    {
    size_t count = m_instances.size();
    for (auto& instance : m_instances)
        {
        count += instance->CountRelatedInstances();
        }
    return count;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WSChangeset::GetRelationshipCount() const
    {
    return GetInstanceCount() - m_instances.size();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WSChangeset::CalculateSize() const
    {
    static const size_t sizeMinimum = Utf8String(R"({"instances":[]})").size();

    size_t size = sizeMinimum;

    for (auto& instance : m_instances)
        {
        size += instance->CalculateSize();
        }

    if (!m_instances.empty())
        {
        size += m_instances.size() - 1; // Seperating ","
        }

    return size;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSChangeset::ToString() const
    {
    Json::Value changesetJson;
    Json::Value& instancesJson = changesetJson["instances"];
    instancesJson = Json::arrayValue;

    for (Json::ArrayIndex i = 0; i < instancesJson.size(); i++)
        {
        instancesJson.append(Json::objectValue);
        }

    Json::ArrayIndex index = 0;
    for (auto& instance : m_instances)
        {
        instance->ToJson(instancesJson[index]);
        ++index;
        }

    return Json::FastWriter::ToString(changesetJson);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSChangeset::Instance& WSChangeset::AddInstance
(
ObjectId instanceId,
ChangeStatus status,
std::shared_ptr<Json::Value> properties
)
    {
    m_instances.push_back(std::make_shared<Instance>());
    auto& instance = m_instances.back();

    instance->m_id = instanceId;
    instance->m_status = status;
    instance->m_properties = properties;

    return *instance;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSChangeset::RemoveInstance(Instance& instanceToRemove)
    {
    for (auto& instance : m_instances)
        {
        if (instance.get() == &instanceToRemove)
            {
            m_instances.erase(&instance);
            return true;
            }
        if (instance->RemoveRelatedInstance(instanceToRemove))
            {
            return true;
            }
        }
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSChangeset::Instance& WSChangeset::Instance::AddRelatedInstance
(
ObjectId relId,
ChangeStatus relStatus,
ECRelatedInstanceDirection relDirection,
ObjectId instanceId,
ChangeStatus status,
std::shared_ptr<Json::Value> properties
)
    {
    m_relationships.push_back(std::make_shared<Relationship>());
    auto& relationship = m_relationships.back();

    relationship->m_id = relId;
    relationship->m_status = relStatus;
    relationship->m_direction = relDirection;
    relationship->m_instance.m_id = instanceId;
    relationship->m_instance.m_status = status;
    relationship->m_instance.m_properties = properties;

    return relationship->m_instance;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::Instance::ToJson(JsonValueR instanceJsonOut) const
    {
    FillBase(instanceJsonOut, m_id, m_status, m_properties);

    if (m_relationships.empty())
        {
        return;
        }

    JsonValueR relationshipsJson = instanceJsonOut["relationshipInstances"];
    for (Json::ArrayIndex i = 0; i < m_relationships.size(); i++)
        {
        relationshipsJson.append(Json::objectValue);
        }

    Json::ArrayIndex index = 0;
    for (auto& relationship : m_relationships)
        {
        JsonValueR relationshipJson = relationshipsJson[index];

        FillBase(relationshipJson, relationship->m_id, relationship->m_status, nullptr);
        relationshipJson["direction"] = GetDirectionStr(relationship->m_direction);

        relationship->m_instance.ToJson(relationshipJson["relatedInstance"]);

        ++index;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::Instance::FillBase
(
JsonValueR instanceJsonOut,
ObjectIdCR id,
ChangeStatus status,
std::shared_ptr<Json::Value> properties
)
    {
    instanceJsonOut["schemaName"] = id.schemaName;
    instanceJsonOut["className"] = id.className;
    if (ChangeStatus::Created != status)
        {
        instanceJsonOut["remoteId"] = id.remoteId;
        }

    Utf8CP statusStr = GetChangeStatusStr(status);
    if (nullptr != statusStr)
        {
        instanceJsonOut["changeStatus"] = statusStr;
        }

    if (nullptr != properties && (ChangeStatus::Created == status || ChangeStatus::Modified == status))
        {
        instanceJsonOut["properties"] = *properties;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WSChangeset::Instance::CalculateBaseSize
(
ObjectIdCR id,
ChangeStatus status,
std::shared_ptr<Json::Value> properties,
size_t& size
)
    {
    if (0 != size)
        {
        return size;
        }

    size += CalculateFieldSize("schemaName", id.schemaName.c_str());
    size += CalculateFieldSize("className", id.className.c_str());
    if (ChangeStatus::Created != status)
        {
        size += CalculateFieldSize("remoteId", id.remoteId.c_str());
        }

    Utf8CP statusStr = GetChangeStatusStr(status);
    if (nullptr != statusStr)
        {
        size += CalculateFieldSize("changeStatus", statusStr);
        }

    if (nullptr != properties && (ChangeStatus::Created == status || ChangeStatus::Modified == status))
        {
        static const size_t propertiesFieldMinimum = Utf8String(R"("properties":,)").size();
        size += propertiesFieldMinimum;
        size += Json::FastWriter::ToString(*properties).size();
        }

    size -= 1; // Last field seperating ","
    size += 2; // Opening and closing braces "{}"

    return size;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WSChangeset::Instance::CalculateSize() const
    {
    size_t size = CalculateBaseSize(m_id, m_status, m_properties, m_baseSize);

    if (m_relationships.empty())
        {
        return size;
        };

    static const size_t relationshipsFieldMinimum = Utf8String(R"("relationshipInstances":[],)").size();
    size += relationshipsFieldMinimum;

    for (auto& relationship : m_relationships)
        {
        size += CalculateBaseSize(relationship->m_id, relationship->m_status, nullptr, relationship->m_baseSize);
        size += CalculateDirectionFieldSize(relationship->m_direction);

        static const size_t relatedFieldMinimum = Utf8String(R"("relatedInstance":,)").size();
        size += relatedFieldMinimum;
        size += relationship->m_instance.CalculateSize();
        }

    size += m_relationships.size() - 1; // Seperating ","

    return size;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WSChangeset::Instance::CalculateFieldSize(Utf8CP fieldName, Utf8CP fieldValue)
    {
    static const size_t stringFieldMinimum = Utf8String(R"("":"",)").size();

    size_t size = stringFieldMinimum;

    size += std::strlen(fieldName);
    size += std::strlen(fieldValue);

    return size;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WSChangeset::Instance::CalculateDirectionFieldSize(ECRelatedInstanceDirection direction)
    {
    if (direction == ECRelatedInstanceDirection::Forward)
        {
        static const size_t size = CalculateFieldSize("direction", GetDirectionStr(direction));
        return size;
        }
    if (direction == ECRelatedInstanceDirection::Backward)
        {
        static const size_t size = CalculateFieldSize("direction", GetDirectionStr(direction));
        return size;
        }
    BeAssert(false);
    return 0;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WSChangeset::Instance::CountRelatedInstances() const
    {
    size_t count = m_relationships.size();
    for (auto& relationship : m_relationships)
        {
        count += relationship->m_instance.CountRelatedInstances();
        }
    return count;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSChangeset::Instance::RemoveRelatedInstance(Instance& instanceToRemove)
    {
    for (auto& relationship : m_relationships)
        {
        if (&relationship->m_instance == &instanceToRemove)
            {
            m_relationships.erase(&relationship);
            return true;
            }
        if (relationship->m_instance.RemoveRelatedInstance(instanceToRemove))
            {
            return true;
            }
        }
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP WSChangeset::Instance::GetChangeStatusStr(ChangeStatus status)
    {
    switch (status)
        {
        case ChangeStatus::Existing:
            return nullptr;
        case ChangeStatus::Created:
            return "created";
        case ChangeStatus::Modified:
            return "modified";
        case ChangeStatus::Deleted:
            return "deleted";
        default:
            BeAssert(false);
            break;
        }
    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP WSChangeset::Instance::GetDirectionStr(ECRelatedInstanceDirection direction)
    {
    switch (direction)
        {
        case ECRelatedInstanceDirection::Forward:
            return "forward";
        case ECRelatedInstanceDirection::Backward:
            return "backward";
        default:
            BeAssert(false);
            break;
        }
    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectIdCR WSChangeset::Instance::GetId() const
    {
    return m_id;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSChangeset::ChangeStatus WSChangeset::Instance::GetStatus() const
    {
    return m_status;
    }
