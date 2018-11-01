/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/KeySet.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/KeySet.h>

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
KeySetPtr KeySet::Create(bvector<ECClassInstanceKey> const& instanceKeys)
    {
    InstanceKeyMap map;
    for (ECClassInstanceKeyCR key : instanceKeys)
        map[key.GetClass()].insert(key.GetId());
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
        ECInstanceId instanceId;
        ECInstanceId::FromString(instanceId, instance->GetInstanceId().c_str());
        map[&instance->GetClass()].insert(instanceId);
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
        map[ecClass].insert(ECInstanceId());
    return new KeySet(map, NavNodeKeySet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr KeySet::GetAllNavNodeKeys() const
    {
    if (m_nodeKeysContainer.IsNull())
        {
        NavNodeKeyList keys;
        std::copy(m_nodes.begin(), m_nodes.end(), std::back_inserter(keys));
        for (auto const& entry : m_instances)
            {
            ECClassCP ecClass = entry.first;
            for (ECInstanceId instanceId : entry.second)
                keys.push_back(ECInstanceNodeKey::Create(ECClassInstanceKey(ecClass, instanceId), bvector<Utf8String>()));
            }
        m_nodeKeysContainer = NavNodeKeyListContainer::Create(keys);
        }
    return m_nodeKeysContainer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool KeySet::Contains(ECClassCP cls, ECInstanceId instanceId) const
    {
    auto iter = m_instances.find(cls);
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
    for (auto const& entry : other.m_instances)
        {
        ECClassCP ecClass = entry.first;
        bset<ECInstanceId> const& instances = entry.second;
        for (ECInstanceId instanceId : instances)
            {
            if (Add(ecClass, instanceId))
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
bool KeySet::Remove(ECClassCP cls, ECInstanceId instanceId)
    {
    auto classIter = m_instances.find(cls);
    if (m_instances.end() == classIter)
        return false;

    bset<ECInstanceId>& instanceIds = classIter->second;
    if (0 == instanceIds.erase(instanceId))
        return false;

    InvalidateNodeKeysContainer();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t KeySet::Remove(KeySetCR toRemove)
    {
    uint64_t removed = 0;
    for (auto const& entry : toRemove.m_instances)
        {
        ECClassCP ecClass = entry.first;
        bset<ECInstanceId> const& instances = entry.second;
        for (ECInstanceId instanceId : instances)
            {
            if (Remove(ecClass, instanceId))
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
KeySetPtr KeySet::FromJson(IConnectionCR connection, JsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetKeySetFromJson(connection, json);
    }