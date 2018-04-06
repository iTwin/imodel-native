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
    return IECPresentationManager::GetSerializer().GetNavNodeKeyFromJsonPoly(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodeKey::FromJson(JsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetNavNodeKeyFromJsonPoly(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodeKey::Create(JsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetNavNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodeKey::Create(RapidJsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetNavNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NavNodeKey::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
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
    return IECPresentationManager::GetSerializer().GetECInstanceNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECInstanceNodeKey> ECInstanceNodeKey::Create(RapidJsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetECInstanceNodeKeyFromJson(json);
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
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECInstanceNodeKey::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
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
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECClassGroupingNodeKey> ECClassGroupingNodeKey::Create(JsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetECClassGroupingNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECClassGroupingNodeKey> ECClassGroupingNodeKey::Create(RapidJsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetECClassGroupingNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECClassGroupingNodeKey::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ECClassGroupingNodeKey::_ComputeHash() const
    {
    MD5 h = NavNodeKey::_ComputeHash();
    h.Add(&m_classId, sizeof(ECClassId));
    return h;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECPropertyGroupingNodeKey> ECPropertyGroupingNodeKey::Create(JsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetECPropertyGroupingNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECPropertyGroupingNodeKey> ECPropertyGroupingNodeKey::Create(RapidJsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetECPropertyGroupingNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECPropertyGroupingNodeKey::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ECPropertyGroupingNodeKey::_ComputeHash() const
    {
    MD5 h = NavNodeKey::_ComputeHash();
    h.Add(&m_classId, sizeof(ECClassId));
    h.Add(m_propertyName.c_str(), m_propertyName.SizeInBytes());
    // wip: do we also absolutely need grouping value?
    return h;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<LabelGroupingNodeKey> LabelGroupingNodeKey::Create(JsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetLabelGroupingNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<LabelGroupingNodeKey> LabelGroupingNodeKey::Create(RapidJsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetLabelGroupingNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document LabelGroupingNodeKey::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 LabelGroupingNodeKey::_ComputeHash() const
    {
    MD5 h = NavNodeKey::_ComputeHash();
    h.Add(m_label.c_str(), m_label.SizeInBytes());
    return h;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr KeySet::Create(NavNodeKeyList const& nodeKeys)
    {
    NavNodeKeySet set;
    for (NavNodeKeyCPtr const& Key : nodeKeys)
        set.insert(Key);
    return new KeySet(InstanceKeyMap(), set);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr KeySet::Create(bvector<ECInstanceKey> const& instanceKeys)
    {
    InstanceKeyMap map;
    for (ECInstanceKey const& key : instanceKeys)
        map[key.GetClassId()].insert(key.GetInstanceId());
    return new KeySet(map, NavNodeKeySet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr KeySet::Create(bvector<IECInstancePtr> const& instances)
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
KeySetPtr KeySet::Create(bvector<ECClassCP> const& classes)
    {
    InstanceKeyMap map;
    for (ECClassCP ecClass : classes)
        {
        ECClassId classId = ecClass->GetId();
        map[classId].insert(ECInstanceId());
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
                keys.push_back(ECInstanceNodeKey::Create(classId, instanceId, bvector<Utf8String>()));
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
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr KeySet::FromJson(JsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetKeySetFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNode::FromJson(RapidJsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetNavNodeFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NavNode::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NodesPathElement::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
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
