/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/NavNode.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

typedef bmap<ECClassCP, bset<ECInstanceId>, ECClassNameComparer> InstanceKeyMap;

//=======================================================================================
//! Struct that describes ECInstanceKeys and NavNodeKeys set
// @bsiclass
//=======================================================================================
struct KeySet : RefCountedBase
{
private:
    InstanceKeyMap m_instances;
    NavNodeKeySet m_nodes;
    mutable INavNodeKeysContainerCPtr m_nodeKeysContainer;
    mutable BeMutex m_mutex;

private:
    void InvalidateNodeKeysContainer() {m_nodeKeysContainer = nullptr;}

protected:
    KeySet(InstanceKeyMap instances, NavNodeKeySet nodes) : m_instances(instances), m_nodes(nodes) {}
    KeySet(KeySetCR other) : m_instances(other.m_instances), m_nodes(other.m_nodes) {}

public:
    //! Creates empty key set.
    static KeySetPtr Create() {return new KeySet(InstanceKeyMap(), NavNodeKeySet());}
    //! Creates key set from supplied instance keys and node keys.
    static KeySetPtr Create(InstanceKeyMap instances, NavNodeKeySet nodes) {return new KeySet(instances, nodes);}
    //! Creates key set from supplied instance key map.
    static KeySetPtr Create(InstanceKeyMap instances) {return new KeySet(instances, NavNodeKeySet());}
    //! Creates key set from supplied nodes key set.
    static KeySetPtr Create(NavNodeKeySet nodes) {return new KeySet(InstanceKeyMap(), nodes);}
    //! Creates key set from supplied nodes key list.
    ECPRESENTATION_EXPORT static KeySetPtr Create(NavNodeKeyList const& nodeKey);
    //! Creates key set from supplied instance keys.
    ECPRESENTATION_EXPORT static KeySetPtr Create(bvector<ECClassInstanceKey> const& instanceKeys);
    //! Creates key set from supplied instances
    ECPRESENTATION_EXPORT static KeySetPtr Create(bvector<IECInstancePtr> const& instaces);
    //! Creates key set from supplied instance.
    static KeySetPtr Create(IECInstanceR instance) {return KeySet::Create({&instance});}
    //! Creates key set from supplied node key
    static KeySetPtr Create(NavNodeKeyCR nodeKey) {return KeySet::Create({&nodeKey});}
    //! Creates a copy of provided key set.
    static KeySetPtr Create(KeySetCR other) {return new KeySet(other);}
    //! Created key set from supplied classes.
    ECPRESENTATION_EXPORT static KeySetPtr Create(bvector<ECClassCP> const& classes);

    //! Returns whether this key set is equal to the supplied one.
    bool Equals(KeySetCR other) const {BeMutexHolder lock(m_mutex); return GetHash().Equals(other.GetHash());}
    //! Equals operator override
    bool operator==(KeySetCR other) const {return Equals(other);}
    //! Get hash of this key set.
    Utf8StringCR GetHash() const {return GetAllNavNodeKeys()->GetHash();}
    //! Get NavNode keys contained in this key set.
    NavNodeKeySetCR GetNavNodeKeys() const {return m_nodes;}
    //! Get instance keys map.
    InstanceKeyMap const& GetInstanceKeys() const {return m_instances;}
    //! Get NavNode keys contained in this key set. (Creates node keys for instance keys too)
    ECPRESENTATION_EXPORT INavNodeKeysContainerCPtr GetAllNavNodeKeys() const;
    //! Get size of nav node keys.
    size_t size() const {return GetAllNavNodeKeys()->size();}
    //! Add instance key to this key set. (Returns true if key was added and false if key was in the set already)
    bool Add(ECClassCP ecClass, ECInstanceId instanceId) {BeMutexHolder lock(m_mutex); InvalidateNodeKeysContainer(); return m_instances[ecClass].insert(instanceId).second;}
    //! Add instance ket to this key set. (Returns true if key was added and false if key was in the set already)
    bool Add(ECClassInstanceKeyCR instanceKey) {return Add(instanceKey.GetClass(), instanceKey.GetId());}
    //! Add NavNode key to this key set. (Returns true if key was added and false if key was in the set already)
    bool Add(NavNodeKeyCR nodeKey) {BeMutexHolder lock(m_mutex); InvalidateNodeKeysContainer(); return m_nodes.insert(&nodeKey).second;}
    //! Returns whether this key set contains supplied instance key.
    ECPRESENTATION_EXPORT bool Contains(ECClassCP, ECInstanceId) const;
    //! Returns whether this key set contains supplied instance key.
    bool Contains(ECClassInstanceKeyCR instanceKey) const {return Contains(instanceKey.GetClass(), instanceKey.GetId());}
    //! Return whether this key set contains supplied NavNode key.
    bool Contains(NavNodeKeyCR nodeKey) const {BeMutexHolder lock(m_mutex); return m_nodes.end() != m_nodes.find(&nodeKey);}
    //! Clears this key set.
    void Clear() {BeMutexHolder lock(m_mutex); InvalidateNodeKeysContainer(); m_instances.clear(); m_nodes.clear();}
    //! Returns whether this key set is empty.
    bool empty() const {BeMutexHolder lock(m_mutex); return m_instances.empty() && m_nodes.empty();}
    //! Merge supplied key set which this key set. Returns number of new keys added.
    ECPRESENTATION_EXPORT uint64_t MergeWith(KeySetCR other);
    //! Remove supplied keys from this key set. Return number of keys removed.
    ECPRESENTATION_EXPORT uint64_t Remove(KeySetCR toRemove);
    //! Remove supplied nav node key from this key set.
    ECPRESENTATION_EXPORT bool Remove(NavNodeKeyCR nodeKey);
    //! Remove supplied instance key from this key set.
    ECPRESENTATION_EXPORT bool Remove(ECClassCP, ECInstanceId);
    //! Remove supplied instance ky from this key set.
    bool Remove(ECClassInstanceKeyCR instanceKey) {return Remove(instanceKey.GetClass(), instanceKey.GetId());}

    ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* = nullptr) const;
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* = nullptr) const;
    ECPRESENTATION_EXPORT static KeySetPtr FromJson(IConnectionCR, JsonValueCR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
