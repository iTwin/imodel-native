/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/KeySet.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr KeySet::Create(NavNodeKeyList const& nodeKeys)
    {
    NavNodeKeySet set;
    for (NavNodeKeyCPtr const& Key : nodeKeys)
        set.insert(Key);
    return new KeySet(InstanceKeyMap(), set);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr KeySet::Create(bvector<ECClassInstanceKey> const& instanceKeys)
    {
    InstanceKeyMap map;
    for (ECClassInstanceKeyCR key : instanceKeys)
        map[key.GetClass()].insert(key.GetId());
    return new KeySet(map, NavNodeKeySet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr KeySet::Create(bvector<ECClassCP> const& classes)
    {
    InstanceKeyMap map;
    for (ECClassCP ecClass : classes)
        map[ecClass].insert(ECInstanceId());
    return new KeySet(map, NavNodeKeySet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr KeySet::GetAllNavNodeKeys() const
    {
    BeMutexHolder lock(m_mutex);
    if (m_nodeKeysContainer.IsNull())
        {
        NavNodeKeyList keys;
        std::copy(m_nodes.begin(), m_nodes.end(), std::back_inserter(keys));
        for (auto const& entry : m_instances)
            {
            ECClassCP ecClass = entry.first;
            for (ECInstanceId instanceId : entry.second)
                keys.push_back(ECInstancesNodeKey::Create(ECClassInstanceKey(ecClass, instanceId), "", { Utf8String(ecClass->GetId().ToString()).append(":").append(instanceId.ToString()) }));
            }
        m_nodeKeysContainer = NavNodeKeyListContainer::Create(keys);
        }
    return m_nodeKeysContainer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool KeySet::Contains(ECClassCP cls, ECInstanceId instanceId) const
    {
    BeMutexHolder lock(m_mutex);
    auto iter = m_instances.find(cls);
    return m_instances.end() != iter && iter->second.end() != iter->second.find(instanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t KeySet::MergeWith(KeySetCR other)
    {
    BeMutexHolder lock(m_mutex);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool KeySet::Remove(ECClassCP cls, ECInstanceId instanceId)
    {
    BeMutexHolder lock(m_mutex);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool KeySet::Remove(NavNodeKeyCR nodeKey)
    {
    BeMutexHolder lock(m_mutex);

    if (0 == m_nodes.erase(&nodeKey))
        return false;

    InvalidateNodeKeysContainer();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t KeySet::Remove(KeySetCR toRemove)
    {
    BeMutexHolder lock(m_mutex);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document KeySet::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    BeMutexHolder lock(m_mutex);
    return ECPresentationManager::GetSerializer().AsJson(ctx , *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document KeySet::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr KeySet::FromJson(IConnectionCR connection, BeJsConst json)
    {
    return ECPresentationManager::GetSerializer().GetKeySetFromJson(connection, json);
    }
