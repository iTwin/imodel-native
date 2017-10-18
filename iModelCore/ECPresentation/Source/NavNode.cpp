/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/NavNode.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/NavNode.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static int CompareRapidJsonPrimitiveValues(RapidJsonValueCR lhs, RapidJsonValueCR rhs)
    {
    if (lhs.GetType() != rhs.GetType())
        return lhs.GetType() < rhs.GetType() ? -1 : 1;

    if (lhs == rhs)
        return 0;

    BeAssert(lhs.IsBool() || lhs.IsNumber() || lhs.IsNull() || lhs.IsString() || lhs.IsArray());

    if (lhs.IsBool())
        return (false == lhs.GetBool()) ? -1 : 1;

    if (lhs.IsInt64())
        return (lhs.GetInt64() < rhs.GetInt64()) ? -1 : 1;
    
    if (lhs.IsUint64())
        return (lhs.GetUint64() < rhs.GetUint64()) ? -1 : 1;
    
    if (lhs.IsDouble())
        return (lhs.GetDouble() < rhs.GetDouble()) ? -1 : 1;

    if (lhs.IsString())
        return strcmp(lhs.GetString(), rhs.GetString());

    if (lhs.IsArray())
        {
        if (lhs.Size() < rhs.Size())
            return -1;
        if (lhs.Size() > rhs.Size())
            return 1;
        for (rapidjson::SizeType i = 0; i < lhs.Size(); ++i)
            {
            int cmp = CompareRapidJsonPrimitiveValues(lhs[i], rhs[i]);
            if (0 != cmp)
                return cmp;
            }
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodeKey::FromJson(RapidJsonValueCR json)
    {
    BeAssert(json.IsObject());
    Utf8CP type = json["Type"].GetString();
    if (0 == strcmp(NAVNODE_TYPE_ECInstanceNode, type))
        return ECInstanceNodeKey::Create(json);
    if (0 == strcmp(NAVNODE_TYPE_ECClassGroupingNode, type))
        return ECClassGroupingNodeKey::Create(json);
    if (0 == strcmp(NAVNODE_TYPE_ECPropertyGroupingNode, type))
        return ECPropertyGroupingNodeKey::Create(json);
    
    // custom nodes are just like display label nodes
    return DisplayLabelGroupingNodeKey::Create(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodeKey::FromJson(JsonValueCR json)
    {
    if (!json.isObject() || json.isNull())
        {
        BeAssert(false);
        return nullptr;
        }

    Utf8CP type = json["Type"].asCString();
    if (nullptr == type)
        {
        BeAssert(false);
        return nullptr;
        }

    if (0 == strcmp(NAVNODE_TYPE_ECInstanceNode, type))
        return ECInstanceNodeKey::Create(json);
    if (0 == strcmp(NAVNODE_TYPE_ECClassGroupingNode, type))
        return ECClassGroupingNodeKey::Create(json);
    if (0 == strcmp(NAVNODE_TYPE_ECPropertyGroupingNode, type))
        return ECPropertyGroupingNodeKey::Create(json);

    // custom nodes are just like display label nodes
    return DisplayLabelGroupingNodeKey::Create(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NavNodeKey::_AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Type", rapidjson::Value(m_type.c_str(), json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 NavNodeKey::_ComputeHash() const
    {
    MD5 h;
    h.Add(m_type.c_str(), m_type.SizeInBytes());
    return h;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR NavNodeKey::GetHash() const
    {
    if (m_hash.empty())
        m_hash = _ComputeHash().GetHashString();
    return m_hash;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
int GroupingNodeKey::_Compare(NavNodeKey const& other) const
    {
    int baseCmp = NavNodeKey::_Compare(other);
    if (0 != baseCmp)
        return baseCmp;
    
    BeAssert(nullptr != dynamic_cast<GroupingNodeKey const*>(&other));
    GroupingNodeKey const& otherKey = static_cast<GroupingNodeKey const&>(other);

    if (m_nodeId < otherKey.m_nodeId)
        return -1;
    if (m_nodeId > otherKey.m_nodeId)
        return 1;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document GroupingNodeKey::_AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json = NavNodeKey::_AsJson(allocator);
    json.AddMember("NodeId", m_nodeId, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 GroupingNodeKey::_ComputeHash() const
    {
    MD5 h = NavNodeKey::_ComputeHash();
    h.Add(&m_nodeId, sizeof(uint64_t));
    return h;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<uint64_t, uint64_t>::const_iterator GroupingNodeKey::_RemapNodeId(bmap<uint64_t, uint64_t> const& remapInfo)
    {
    auto iter = remapInfo.find(m_nodeId);
    if (remapInfo.end() != iter)
        m_nodeId = iter->second;
    return iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECClassGroupingNodeKey> ECClassGroupingNodeKey::Create(JsonValueCR json)
    {
    uint64_t nodeId = BeJsonUtilities::UInt64FromValue(json["NodeId"]);
    ECClassId classId(BeJsonUtilities::UInt64FromValue(json["ECClassId"]));
    return Create(nodeId, classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECClassGroupingNodeKey> ECClassGroupingNodeKey::Create(RapidJsonValueCR json)
    {
    uint64_t nodeId = BeRapidJsonUtilities::UInt64FromValue(json["NodeId"]);
    ECClassId classId(BeRapidJsonUtilities::UInt64FromValue(json["ECClassId"]));
    return Create(nodeId, classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClassGroupingNodeKey::_IsSimilar(NavNodeKey const& other) const
    {
    if (!GroupingNodeKey::_IsSimilar(other))
        return false;
    
    BeAssert(nullptr != dynamic_cast<ECClassGroupingNodeKey const*>(&other));
    ECClassGroupingNodeKey const& otherKey = static_cast<ECClassGroupingNodeKey const&>(other);
    return m_classId == otherKey.m_classId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECClassGroupingNodeKey::_AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json = GroupingNodeKey::_AsJson(allocator);
    json.AddMember("ECClassId", rapidjson::Value(m_classId.ToString().c_str(), json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECPropertyGroupingNodeKey> ECPropertyGroupingNodeKey::Create(JsonValueCR json)
    {
    uint64_t nodeId = BeJsonUtilities::UInt64FromValue(json["NodeId"]);
    ECClassId classId(BeJsonUtilities::UInt64FromValue(json["ECClassId"]));
    Utf8CP propertyName = json["PropertyName"].asCString();
    int rangeIndex = json["RangeIndex"].asInt();
    if (!json.isMember("GroupingValue"))
        return Create(nodeId, classId, propertyName, rangeIndex, nullptr);

    rapidjson::Document groupingValue;
    groupingValue.Parse(Json::FastWriter().write(json["GroupingValue"]).c_str());
    return Create(nodeId, classId, propertyName, rangeIndex, &groupingValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECPropertyGroupingNodeKey> ECPropertyGroupingNodeKey::Create(RapidJsonValueCR json)
    {
    uint64_t nodeId = BeRapidJsonUtilities::UInt64FromValue(json["NodeId"]);
    ECClassId classId(BeRapidJsonUtilities::UInt64FromValue(json["ECClassId"]));
    Utf8CP propertyName = json["PropertyName"].GetString();
    int rangeIndex = json["RangeIndex"].GetInt();
    rapidjson::Value const* groupingValue = nullptr;
    if (json.HasMember("GroupingValue"))
        groupingValue = &json["GroupingValue"];
    return Create(nodeId, classId, propertyName, rangeIndex, groupingValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECPropertyGroupingNodeKey::_IsSimilar(NavNodeKey const& other) const
    {
    if (!GroupingNodeKey::_IsSimilar(other))
        return false;
    
    BeAssert(nullptr != dynamic_cast<ECPropertyGroupingNodeKey const*>(&other));
    ECPropertyGroupingNodeKey const& otherKey = static_cast<ECPropertyGroupingNodeKey const&>(other);
    return m_rangeIndex == otherKey.m_rangeIndex
        && m_propertyName == otherKey.m_propertyName
        && (m_value == otherKey.m_value
            || nullptr != m_value && nullptr != otherKey.m_value && 0 == CompareRapidJsonPrimitiveValues(*m_value, *otherKey.m_value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECPropertyGroupingNodeKey::_AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json = ECClassGroupingNodeKey::_AsJson(allocator);
    json.AddMember("PropertyName", rapidjson::Value(m_propertyName.c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("RangeIndex", m_rangeIndex, json.GetAllocator());
    if (nullptr != m_value)
        json.AddMember("GroupingValue", rapidjson::Value(*m_value, json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECInstanceNodeKey> ECInstanceNodeKey::Create(JsonValueCR json)
    {
    ECClassId classId(BeJsonUtilities::UInt64FromValue(json["ECClassId"]));
    ECInstanceId instanceId(BeJsonUtilities::UInt64FromValue(json["ECInstanceId"]));
    return Create(classId, instanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECInstanceNodeKey> ECInstanceNodeKey::Create(RapidJsonValueCR json)
    {
    ECClassId classId(BeRapidJsonUtilities::UInt64FromValue(json["ECClassId"]));
    ECInstanceId instanceId(BeRapidJsonUtilities::UInt64FromValue(json["ECInstanceId"]));
    return Create(classId, instanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECInstanceNodeKey::_AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json = NavNodeKey::_AsJson(allocator);
    json.AddMember("ECClassId", rapidjson::Value(m_instanceKey.GetClassId().ToString().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("ECInstanceId", rapidjson::Value(m_instanceKey.GetInstanceId().ToString().c_str(), json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int ECInstanceNodeKey::_Compare(NavNodeKey const& other) const
    {
    int baseCmp = NavNodeKey::_Compare(other);
    if (0 != baseCmp)
        return baseCmp;
    
    BeAssert(nullptr != dynamic_cast<ECInstanceNodeKey const*>(&other));
    ECInstanceNodeKey const& otherKey = static_cast<ECInstanceNodeKey const&>(other);
        
    if (m_instanceKey < otherKey.m_instanceKey)
        return -1;
    if (m_instanceKey == otherKey.m_instanceKey)
        return 0;
    return 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECInstanceNodeKey::_IsSimilar(NavNodeKey const& other) const
    {
    if (!NavNodeKey::_IsSimilar(other))
        return false;

    BeAssert(nullptr != dynamic_cast<ECInstanceNodeKey const*>(&other));
    ECInstanceNodeKey const& otherKey = static_cast<ECInstanceNodeKey const&>(other);
    return m_instanceKey == otherKey.m_instanceKey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ECInstanceNodeKey::_ComputeHash() const
    {
    MD5 h = NavNodeKey::_ComputeHash();
    h.Add(&m_instanceKey, sizeof(ECInstanceKey));
    return h;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<DisplayLabelGroupingNodeKey> DisplayLabelGroupingNodeKey::Create(JsonValueCR json)
    {
    uint64_t nodeId = BeJsonUtilities::UInt64FromValue(json["NodeId"]);
    return Create(nodeId, json["DisplayLabel"].asCString(), json["Type"].asCString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<DisplayLabelGroupingNodeKey> DisplayLabelGroupingNodeKey::Create(RapidJsonValueCR json)
    {
    uint64_t nodeId = BeRapidJsonUtilities::UInt64FromValue(json["NodeId"]);
    return Create(nodeId, json["DisplayLabel"].GetString(), json["Type"].GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayLabelGroupingNodeKey::_IsSimilar(NavNodeKey const& other) const
    {
    if (!GroupingNodeKey::_IsSimilar(other))
        return false;
    
    BeAssert(nullptr != dynamic_cast<DisplayLabelGroupingNodeKey const*>(&other));
    DisplayLabelGroupingNodeKey const& otherKey = static_cast<DisplayLabelGroupingNodeKey const&>(other);
    return m_label.Equals(otherKey.m_label);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DisplayLabelGroupingNodeKey::_AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json = GroupingNodeKey::_AsJson(allocator);
    json.AddMember("DisplayLabel", rapidjson::Value(m_label.c_str(), json.GetAllocator()), json.GetAllocator());
    return json;
    }


static uint64_t s_nodeIdentifiers = 1;
uint64_t NavNode::CreateNodeId() {return s_nodeIdentifiers++;}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyCR NavNode::GetKey() const
    {
    if (m_key.IsNull())
        m_key = _CreateKey();
    return *m_key;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NodesPathElement::AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json(allocator);
    if (m_node.IsNull())
        return json;

    json.SetObject();
    json.AddMember("Node", m_node->AsJson(&json.GetAllocator()), json.GetAllocator());
    json.AddMember("Index", (uint64_t)m_index, json.GetAllocator());
    json.AddMember("IsMarked", m_isMarked, json.GetAllocator());

    rapidjson::Value childrenJson;
    childrenJson.SetArray();
    for (NodesPathElement const& child : m_children)
        childrenJson.PushBack(child.AsJson(&json.GetAllocator()), json.GetAllocator());
    json.AddMember("Children", childrenJson, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeySetContainer::Create() {return new NavNodeKeySetContainer(NavNodeKeySet());}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeySetContainer::Create(NavNodeKeySet set) {return new NavNodeKeySetContainer(set);}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeySetContainer::Create(NavNodeKeySet const* set) {return new NavNodeKeySetContainer(set);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeyListContainer::Create() {return new NavNodeKeyListContainer(NavNodeKeyList());}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeyListContainer::Create(NavNodeKeyList list) {return new NavNodeKeyListContainer(list);}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeyListContainer::Create(NavNodeKeyList const* list) {return new NavNodeKeyListContainer(list);}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR INavNodeKeysContainer::GetHash() const
    {
    if (m_hash.empty())
        {
        MD5 h;
        for (NavNodeKeyCPtr const& key : *this)
            h.Add(key->GetHash().c_str(), key->GetHash().SizeInBytes());
        m_hash = h.GetHashString();
        }
    return m_hash;
    }