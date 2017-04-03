/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WSChangeset.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/WSChangeset.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSChangeset::WSChangeset(Format format) : m_format(format)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSChangeset::IsEmpty() const
    {
    return m_instances.empty();
    }

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
    static const size_t sizeMinimumSingle = Utf8String(R"({"instance":{}})").size();
    static const size_t sizeMinimumMulti = Utf8String(R"({"instances":[]})").size();

    size_t size = Format::SingeInstance == m_format ? sizeMinimumSingle : sizeMinimumMulti;

    for (auto& instance : m_instances)
        {
        size += instance->CalculateSize();
        }

    if (!m_instances.empty() && Format::MultipleInstances == m_format)
        {
        size += m_instances.size() - 1; // Seperating ","
        }

    if (!m_instances.empty() && Format::SingeInstance == m_format)
        {
        size -= 2; // size of "{}"
        }

    if (nullptr != m_options)
        {
        size += Utf8String(R"(",requestOptions":)").size();
        size += m_options->CalculateSize();
        }

    return size;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSChangeset::ToRequestString() const
    {
    Json::Value changesetJson;
    ToRequestJson(changesetJson);
    return Json::FastWriter::ToString(changesetJson);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::ToRequestJson(JsonValueR changesetJson) const
    {
    changesetJson = Json::objectValue;

    ToOptionsRequestJson(changesetJson);

    if (Format::SingeInstance == m_format)
        {
        ToSingleInstanceRequestJson(changesetJson);
        }
    else
        {
        ToMultipleInstancesRequestJson(changesetJson);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::ToSingleInstanceRequestJson(JsonValueR changesetJson) const
    {
    Json::Value& instanceJson = changesetJson["instance"];
    instanceJson = Json::objectValue;

    if (!m_instances.empty())
        {
        m_instances.front()->ToJson(instanceJson);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::ToMultipleInstancesRequestJson(JsonValueR changesetJson) const
    {
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
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::ToOptionsRequestJson(JsonValueR changesetJson) const
    {
    if (m_options == nullptr)
        return;

    Json::Value& optionsJson = changesetJson["requestOptions"];
    m_options->ToJson(optionsJson);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WSChangeset::ExtractNewIdsFromResponse(RapidJsonValueCR response, const IdHandler& handler) const
    {
    if (Format::SingeInstance == m_format)
        {
        return ExtractNewIdsFromSingleInstanceResponse(response, handler);
        }
    else
        {
        return ExtractNewIdsFromMultipleInstancesResponse(response, handler);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WSChangeset::ExtractNewIdsFromSingleInstanceResponse(RapidJsonValueCR response, const IdHandler& handler) const
    {
    auto& instanceJson = response["changedInstance"]["instanceAfterChange"];
    if (!instanceJson.IsObject() || m_instances.empty())
        {
        return ERROR;
        }

    if (SUCCESS != m_instances.front()->ExtractNewIdsFromInstanceAfterChange(instanceJson, handler))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WSChangeset::ExtractNewIdsFromMultipleInstancesResponse(RapidJsonValueCR response, const IdHandler& handler) const
    {
    auto& instancesJson = response["changedInstances"];
    if (!instancesJson.IsArray() || instancesJson.Size() != m_instances.size())
        {
        return ERROR;
        }

    rapidjson::SizeType index = 0;
    for (auto& instance : m_instances)
        {
        auto& instanceJson = instancesJson[index]["instanceAfterChange"];
        if (SUCCESS != instance->ExtractNewIdsFromInstanceAfterChange(instanceJson, handler))
            {
            return ERROR;
            }
        index++;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                  julius.cepukenas  08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
WSChangeset::Options& WSChangeset::GetRequestOptions()
    {
    if (nullptr == m_options)
        m_options = std::make_shared<Options>();

    return *m_options;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                  julius.cepukenas  08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::RemoveRequestOptions()
    {
    m_options = nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSChangeset::Instance& WSChangeset::AddInstance(ObjectId instanceId, ChangeState state, JsonValuePtr properties)
    {
    if (Format::SingeInstance == m_format && !m_instances.empty())
        {
        BeAssert(false);
        WSChangeset::Instance* nullChangeset = nullptr;
        return *nullChangeset;
        }

    m_instances.push_back(std::make_shared<Instance>());
    auto& instance = m_instances.back();

    instance->m_id = instanceId;
    instance->m_state = state;
    instance->m_properties = properties;

    return *instance;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSChangeset::Instance* WSChangeset::FindInstance(ObjectIdCR id) const
    {
    for (auto& instance : m_instances)
        {
        if (id == instance->m_id)
            {
            return instance.get();
            }
        auto related = instance->FindRelatedInstance(id);
        if (nullptr != related)
            {
            return related;
            }
        }
    return nullptr;
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
ChangeState relState,
ECRelatedInstanceDirection relDirection,
ObjectId instanceId,
ChangeState state,
JsonValuePtr properties
)
    {
    m_relationships.push_back(std::make_shared<Relationship>());
    auto& relationship = m_relationships.back();

    relationship->m_id = relId;
    relationship->m_state = relState;
    relationship->m_direction = relDirection;
    relationship->m_instance.m_id = instanceId;
    relationship->m_instance.m_state = state;
    relationship->m_instance.m_properties = properties;

    return relationship->m_instance;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSChangeset::Instance* WSChangeset::Instance::FindRelatedInstance(ObjectIdCR id) const
    {
    for (auto& relationship : m_relationships)
        {
        if (id == relationship->m_instance.m_id)
            {
            return &relationship->m_instance;
            }
        auto related = relationship->m_instance.FindRelatedInstance(id);
        if (nullptr != related)
            {
            return related;
            }
        }
    return nullptr;
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
BentleyStatus WSChangeset::Instance::ExtractNewIdsFromInstanceAfterChange(RapidJsonValueCR instanceAfterChange, const IdHandler& handler) const
    {
    if (ChangeState::Created == m_state)
        {
        if (SUCCESS != handler(m_id, GetObjectIdFromInstance(instanceAfterChange)))
            {
            return ERROR;
            }
        }

    if (m_relationships.empty())
        {
        return SUCCESS;
        }

    auto& relationshipsJson = instanceAfterChange["relationshipInstances"];
    if (relationshipsJson.Size() != m_relationships.size())
        {
        return ERROR;
        }

    rapidjson::SizeType index = 0;
    for (auto& relationship : m_relationships)
        {
        auto& relationshipJson = relationshipsJson[index];
        if (ChangeState::Created == relationship->m_state)
            {
            if (SUCCESS != handler(relationship->m_id, GetObjectIdFromInstance(relationshipJson)))
                {
                return ERROR;
                }
            }
        auto& relatedInstanceJson = relationshipJson["relatedInstance"];
        if (SUCCESS != relationship->m_instance.ExtractNewIdsFromInstanceAfterChange(relatedInstanceJson, handler))
            {
            return ERROR;
            }
        index++;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectId WSChangeset::Instance::GetObjectIdFromInstance(RapidJsonValueCR instance)
    {
    ObjectId id;
    id.schemaName = instance["schemaName"].GetString();
    id.className = instance["className"].GetString();
    id.remoteId = instance["instanceId"].GetString();
    return id;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::Instance::ToJson(JsonValueR instanceJsonOut) const
    {
    FillBase(instanceJsonOut, m_id, m_state, m_properties);

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

        FillBase(relationshipJson, relationship->m_id, relationship->m_state, nullptr);
        relationshipJson["direction"] = GetDirectionStr(relationship->m_direction);

        relationship->m_instance.ToJson(relationshipJson["relatedInstance"]);

        ++index;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::Instance::FillBase(JsonValueR instanceJsonOut, ObjectIdCR id, ChangeState state, JsonValuePtr properties)
    {
    instanceJsonOut["schemaName"] = id.schemaName;
    instanceJsonOut["className"] = id.className;
    if (ChangeState::Created != state)
        {
        instanceJsonOut["instanceId"] = id.remoteId;
        }

    Utf8CP stateStr = GetChangeStateStr(state);
    if (nullptr != stateStr)
        {
        instanceJsonOut["changeState"] = stateStr;
        }

    if (nullptr != properties && !properties->empty() && (ChangeState::Created == state || ChangeState::Modified == state))
        {
        instanceJsonOut["properties"] = *properties;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WSChangeset::Instance::CalculateBaseSize(ObjectIdCR id, ChangeState state, JsonValuePtr properties, size_t& size)
    {
    if (0 != size)
        {
        return size;
        }

    size += CalculateFieldSize("schemaName", id.schemaName.c_str());
    size += CalculateFieldSize("className", id.className.c_str());
    if (ChangeState::Created != state)
        {
        size += CalculateFieldSize("instanceId", id.remoteId.c_str());
        }

    Utf8CP stateStr = GetChangeStateStr(state);
    if (nullptr != stateStr)
        {
        size += CalculateFieldSize("changeState", stateStr);
        }

    if (nullptr != properties && (ChangeState::Created == state || ChangeState::Modified == state))
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
    size_t size = CalculateBaseSize(m_id, m_state, m_properties, m_baseSize);

    if (m_relationships.empty())
        {
        return size;
        };

    static const size_t relationshipsFieldMinimum = Utf8String(R"("relationshipInstances":[],)").size();
    size += relationshipsFieldMinimum;

    for (auto& relationship : m_relationships)
        {
        size += CalculateBaseSize(relationship->m_id, relationship->m_state, nullptr, relationship->m_baseSize);
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
Utf8CP WSChangeset::Instance::GetChangeStateStr(ChangeState state)
    {
    switch (state)
        {
        case ChangeState::Existing:
            return nullptr;
        case ChangeState::Created:
            return "new";
        case ChangeState::Modified:
            return "modified";
        case ChangeState::Deleted:
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
WSChangeset::ChangeState WSChangeset::Instance::GetState() const
    {
    return m_state;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::Options::SetResponseContent(ResponseContent content)
    {
    m_baseSize = 0;
    m_responseContent = content;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::Options::SetCustomOption(Utf8StringCR name, Utf8StringCR value)
    {
    m_baseSize = 0;
    m_customOptions[name] = value;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::Options::RemoveCustomOption(Utf8StringCR name)
    {
    m_baseSize = 0;
    m_customOptions.erase(name);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::Options::SetRefreshInstances(bool toRefreshInstance)
    {
    m_baseSize = 0;
    if (toRefreshInstance)
        {
        m_refreshInstances = OptRefreshInstances::Refresh;
        return;
        }

    m_refreshInstances = OptRefreshInstances::DontRefresh;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas    08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP WSChangeset::Options::GetResponseContentStr(ResponseContent response)
    {  
    switch (response)
        {
        case ResponseContent::FullInstance:
            return "FullInstance";
        case ResponseContent::Empty:
            return "Empty";
        case ResponseContent::InstanceId:
            return "InstanceId";
        default:
            BeAssert(false);
            break;
        }
    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void WSChangeset::Options::ToJson(JsonValueR jsonOut) const
    {
    if (ResponseContent::Null == m_responseContent && OptRefreshInstances::Null == m_refreshInstances && m_customOptions.empty())
        {
        jsonOut = Json::objectValue;
        return;
        }

    if (ResponseContent::Null != m_responseContent)
        jsonOut["ResponseContent"] = GetResponseContentStr(m_responseContent);

    if (OptRefreshInstances::Null != m_refreshInstances)
        {
        if (OptRefreshInstances::Refresh == m_refreshInstances)
            jsonOut["RefreshInstances"] = true;
        else
            jsonOut["RefreshInstances"] = false;
        }

    if (!m_customOptions.empty())
        {
        JsonValueR customOptions = jsonOut["CustomOptions"];
        for (auto pair : m_customOptions)
            customOptions[pair.first] = pair.second;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t WSChangeset::Options::CalculateSize() const
    {
    if (0 != m_baseSize)
        return m_baseSize;

    Json::Value requestOptions;
    ToJson(requestOptions);
    m_baseSize += Json::FastWriter::ToString(requestOptions).size();

    return m_baseSize;
    }
