/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/NavNode.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
* @bsimethod                                    Saulius.Skliutas                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String NavNodeKey::GetNodeHash() const
    {
    if (m_pathFromRoot.empty())
        return "";
    
    MD5 h;
    for (Utf8StringCR pathElement : m_pathFromRoot)
        h.Add(pathElement.c_str(), pathElement.SizeInBytes());

    return h.GetHashString();
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

    return NavNodeKey::Create(json);
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

    return NavNodeKey::Create(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodeKey::Create(JsonValueCR json)
    {
    bvector<Utf8String> path;
    for (JsonValueCR pathElement : json["PathFromRoot"])
        path.push_back(pathElement.asString());
    Utf8CP type = json["Type"].asCString();
    uint64_t classId = json["ECClassId"].asUInt64();
    return Create(type, path, ECClassId(classId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodeKey::Create(RapidJsonValueCR json)
    {
    bvector<Utf8String> path;
    for (RapidJsonValueCR pathElement : json["PathFromRoot"].GetArray())
        path.push_back(pathElement.GetString());
    Utf8CP type = json["Type"].GetString();
    uint64_t classId = json["ECClassId"].GetUint64();
    return Create(type, path, ECClassId(classId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NavNodeKey::_AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Type", rapidjson::Value(m_type.c_str(), json.GetAllocator()), json.GetAllocator());

    rapidjson::Value pathJson(rapidjson::kArrayType);
    for (Utf8StringCR pathElement : m_pathFromRoot)
        pathJson.PushBack(rapidjson::Value(pathElement.c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("PathFromRoot", pathJson, json.GetAllocator());
    json.AddMember("ECClassId", m_classId.GetValueUnchecked(), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 NavNodeKey::_ComputeHash() const
    {
    MD5 h;
    h.Add(m_type.c_str(), m_type.SizeInBytes());
    for (Utf8StringCR pathElement : m_pathFromRoot)
        h.Add(pathElement.c_str(), pathElement.SizeInBytes());
    return h;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR NavNodeKey::GetHash() const
    {
    if (m_keyHash.empty())
        m_keyHash = _ComputeHash().GetHashString();
    return m_keyHash;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int NavNodeKey::_Compare(NavNodeKey const& other) const
    {
    int typeCompareResult = m_type.compare(other.m_type);
    if (0 != typeCompareResult)
        return typeCompareResult;

    if (m_pathFromRoot < other.m_pathFromRoot)
        return -1;
    if (m_pathFromRoot > other.m_pathFromRoot)
        return 1;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECInstanceNodeKey> ECInstanceNodeKey::Create(JsonValueCR json)
    {
    ECClassId classId(BeJsonUtilities::UInt64FromValue(json["ECClassId"]));
    ECInstanceId instanceId(BeJsonUtilities::UInt64FromValue(json["ECInstanceId"]));
    bvector<Utf8String> path;
    for (JsonValueCR pathElement : json["PathFromRoot"])
        path.push_back(pathElement.asString());
    return Create(classId, instanceId, path);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECInstanceNodeKey> ECInstanceNodeKey::Create(RapidJsonValueCR json)
    {
    ECClassId classId(BeRapidJsonUtilities::UInt64FromValue(json["ECClassId"]));
    ECInstanceId instanceId(BeRapidJsonUtilities::UInt64FromValue(json["ECInstanceId"]));
    bvector<Utf8String> path;
    for (RapidJsonValueCR pathElement : json["PathFromRoot"].GetArray())
        path.push_back(pathElement.GetString());
    return Create(classId, instanceId, path);
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
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr KeySet::Create(NavNodeKeyList nodeKeys)
    {
    NavNodeKeySet set;
    for (NavNodeKeyCPtr const& Key : nodeKeys)
        set.insert(Key);
    return new KeySet(InstanceKeyMap(), set);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr KeySet::Create(bvector<ECInstanceKey> instanceKeys)
    {
    InstanceKeyMap map;
    for (ECInstanceKey const& key : instanceKeys)
        map[key.GetClassId()].insert(key.GetInstanceId());
    return new KeySet(map, NavNodeKeySet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr KeySet::Create(bvector<IECInstancePtr> instances)
    {
    InstanceKeyMap map;
    for (IECInstancePtr const& instance : instances)
        {
        ECClassId classId = instance->GetClass().GetId();
        ECInstanceId instanceId;
        ECInstanceId::FromString(instanceId, instance->GetInstanceId().c_str());
        map[classId].insert(instanceId);
        }
    return new KeySet(map, NavNodeKeySet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr KeySet::GetAllNavNodeKeys() const
    {
    if (m_nodesContainer.IsNull())
        {
        NavNodeKeyList keys;
        std::copy(m_nodes.begin(), m_nodes.end(), std::back_inserter(keys));
        for (auto& pair : m_instances)
            {
            ECClassId classId = pair.first;
            for (ECInstanceId const& instanceId : pair.second)
                keys.push_back(ECInstanceNodeKey::Create(classId, instanceId));
            }
        m_nodesContainer = NavNodeKeyListContainer::Create(keys);
        }
    return m_nodesContainer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool KeySet::Contains(ECClassId classId, ECInstanceId instanceId) const
    {
    auto iter = m_instances.find(classId);
    return m_instances.end() != iter && iter->second.end() != iter->second.find(instanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t KeySet::MergeWith(KeySetCR other)
    {
    if (Equals(other))
        return 0;

    uint64_t inserted = 0;
    for (auto& pair : other.m_instances)
        {
        ECClassId classId = pair.first;
        bset<ECInstanceId> const& instances = pair.second;
        for (ECInstanceId instanceId : instances)
            {
            if (Add(classId, instanceId))
                inserted++;
            }
        }

    for (NavNodeKeyCPtr const& nodeKey : other.m_nodes)
        {
        if (Add(*nodeKey))
            inserted++;
        }

    return inserted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool KeySet::Remove(ECClassId classId, ECInstanceId instanceId)
    {
    auto classIter = m_instances.find(classId);
    if (m_instances.end() == classIter)
        return false;

    bset<ECInstanceId>& instanceIds = classIter->second;
    if (0 == instanceIds.erase(instanceId))
        return false;

    InvalidateNodesContainer();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t KeySet::Remove(KeySetCR toRemove)
    {
    uint64_t removed = 0;
    for (auto& pair : toRemove.m_instances)
        {
        ECClassId classId = pair.first;
        bset<ECInstanceId> const& instances = pair.second;
        for (ECInstanceId instanceId : instances)
            {
            if (Remove(classId, instanceId))
                removed++;
            }
        }

    for (NavNodeKeyCPtr const& nodeKey : toRemove.m_nodes)
        {
        if (Remove(*nodeKey))
            removed++;
        }

    return removed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document KeySet::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();  
    rapidjson::Value instances(rapidjson::kObjectType);
    for (auto& pair : m_instances)
        {
        ECClassId classId = pair.first;
        rapidjson::Value instanceIds(rapidjson::kArrayType);
        for (ECInstanceId const& instanceId : pair.second)
            instanceIds.PushBack(instanceId.GetValueUnchecked(), json.GetAllocator());
        instances.AddMember(rapidjson::Value(classId.ToString().c_str(), json.GetAllocator()), instanceIds, json.GetAllocator());
        }

    rapidjson::Value nodeKeys(rapidjson::kArrayType);
    for (NavNodeKeyCPtr const& key : m_nodes)
        nodeKeys.PushBack(key->AsJson(&json.GetAllocator()), json.GetAllocator());

    json.AddMember("InstanceKeys", instances, json.GetAllocator());
    json.AddMember("NodeKeys", nodeKeys, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr KeySet::FromJson(JsonValueCR json)
    {
    InstanceKeyMap instanceKeys;
    JsonValueCR instanceKey = json["InstanceKeys"];
    bvector<Utf8String> classIds = instanceKey.getMemberNames();
    for (Utf8StringCR classIdString : classIds)
        {
        bset<ECInstanceId> instanceIdSet;
        for (JsonValueCR instanceIdJson : instanceKey[classIdString.c_str()])
            {
            uint64_t instanceId = BeJsonUtilities::UInt64FromValue(instanceIdJson);
            instanceIdSet.insert(ECInstanceId(instanceId));
            }
        ECClassId ecClassId;
        ECClassId::FromString(ecClassId, classIdString.c_str());
        instanceKeys[ecClassId] = instanceIdSet;
        }

    NavNodeKeySet nodeKeys;
    for (JsonValueCR nodeKeyJson : json["NodeKeys"])
        nodeKeys.insert(NavNodeKey::FromJson(nodeKeyJson));

    return KeySet::Create(instanceKeys, nodeKeys);
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
