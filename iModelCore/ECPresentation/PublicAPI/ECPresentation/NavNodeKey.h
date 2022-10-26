/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/Connection.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_EC

#define NAVNODE_TYPE_ECInstancesNode            "ECInstancesNode"
#define NAVNODE_TYPE_ECClassGroupingNode        "ECClassGroupingNode"
#define NAVNODE_TYPE_ECRelationshipGroupingNode "ECRelationshipGroupingNode"
#define NAVNODE_TYPE_ECPropertyGroupingNode     "ECPropertyGroupingNode"
#define NAVNODE_TYPE_DisplayLabelGroupingNode   "DisplayLabelGroupingNode"

struct ECInstancesNodeKey;
struct GroupingNodeKey;
struct ECClassGroupingNodeKey;
struct ECPropertyGroupingNodeKey;
struct LabelGroupingNodeKey;
//=======================================================================================
//! Base class for a @ref NavNode key which identifies similar nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE NavNodeKey : RefCountedBase
{
private:
    Utf8String m_type;
    bvector<Utf8String> m_hashPath;
    Utf8String m_specificationIdentifier;
    PresentationQueryBasePtr m_instanceKeysSelectQuery;

private:
    static bvector<Utf8String> CreateHashPath(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
        Utf8StringCR type, Utf8StringCR label, PresentationQueryCP instanceKeysSelectQuery);

protected:
    static Utf8String GetConnectionIdentifier(IConnectionCR connection) {return connection.IsOpen() ? connection.GetDb().GetDbGuid().ToString() : "";}

protected:
    ECPRESENTATION_EXPORT NavNodeKey(NavNodeKey const& other);
    ECPRESENTATION_EXPORT NavNodeKey(Utf8String type, Utf8String specificationIdentifier, bvector<Utf8String> hashPath);
    ECPRESENTATION_EXPORT ~NavNodeKey();
    ECPRESENTATION_EXPORT virtual int _Compare(NavNodeKey const& other) const;
    virtual bool _IsSimilar(NavNodeKey const& other) const {return m_type.Equals(other.m_type);}
    virtual ECInstancesNodeKey const* _AsECInstanceNodeKey() const {return nullptr;}
    virtual GroupingNodeKey const* _AsGroupingNodeKey() const {return nullptr;}
    virtual ECClassGroupingNodeKey const* _AsECClassGroupingNodeKey() const {return nullptr;}
    virtual ECPropertyGroupingNodeKey const* _AsECPropertyGroupingNodeKey() const {return nullptr;}
    virtual LabelGroupingNodeKey const* _AsLabelGroupingNodeKey() const {return nullptr;}
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType*) const;

public:
    ECInstancesNodeKey const* AsECInstanceNodeKey() const {return _AsECInstanceNodeKey();}
    GroupingNodeKey const* AsGroupingNodeKey() const {return _AsGroupingNodeKey();}
    ECClassGroupingNodeKey const* AsECClassGroupingNodeKey() const {return _AsECClassGroupingNodeKey();}
    ECPropertyGroupingNodeKey const* AsECPropertyGroupingNodeKey() const {return _AsECPropertyGroupingNodeKey();}
    LabelGroupingNodeKey const* AsLabelGroupingNodeKey() const {return _AsLabelGroupingNodeKey();}

    //! Get the path from root to this node.
    bvector<Utf8String> const& GetHashPath() const {return m_hashPath;}
    bvector<Utf8String>& GetHashPath() {return m_hashPath;}
    ECPRESENTATION_EXPORT Utf8StringCR GetHash() const;

    //! Compare this key with the supplied one.
    int Compare(NavNodeKey const& other) const {return _Compare(other);}
    //! Compare this key with the supplied one. Used for storing @ref NavNodeKey objects in maps and sets.
    bool operator<(NavNodeKey const& other) const {return Compare(other) < 0;}
    //! Is this node equal to the supplied one.
    bool operator==(NavNodeKey const& other) const {return 0 == Compare(other);}
    //! Is this node different from the supplied one.
    bool operator!=(NavNodeKey const& other) const {return 0 != Compare(other);}
    //! Is this node similar to the supplied one.
    //! @note Similar nodes can be unequal, e.g. label grouping node keys are similar if labels match, but they're not
    //! equal because they represent different nodes.
    bool IsSimilar(NavNodeKey const& other) const {return _IsSimilar(other);}

    //! Get the type of the @ref NavNode.
    Utf8StringCR GetType() const {return m_type;}
    //! Get identifier of specification that produced the node
    Utf8StringCR GetSpecificationIdentifier() const {return m_specificationIdentifier;}

    //! Serialize this key to JSON.
    rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, allocator);}
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;

    //! Create a new key.
    static NavNodeKeyPtr Create(Utf8String type, Utf8String specificationIdentifier, bvector<Utf8String> hashPath) {return new NavNodeKey(type, specificationIdentifier, hashPath);}
    static NavNodeKeyPtr Create(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
        Utf8String type, Utf8StringCR label, PresentationQueryCP instanceKeysSelectQuery)
        {
        auto hashPath = CreateHashPath(connectionIdentifier, specificationIdentifier, parentKey, type, label, instanceKeysSelectQuery);
        return new NavNodeKey(type, specificationIdentifier, hashPath);
        }
    static NavNodeKeyPtr Create(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
        Utf8String type, Utf8StringCR label, PresentationQueryCP instanceKeysSelectQuery)
        {
        return Create(GetConnectionIdentifier(connection), specificationIdentifier, parentKey, type, label, instanceKeysSelectQuery);
        }
    //! Create a key from the supplied JSON.
    ECPRESENTATION_EXPORT static NavNodeKeyPtr FromJson(IConnectionCR, BeJsConst);

    ECPRESENTATION_EXPORT PresentationQueryBasePtr GetInstanceKeysSelectQuery() const;
    ECPRESENTATION_EXPORT void SetInstanceKeysSelectQuery(PresentationQueryBasePtr);
};

//=======================================================================================
//! Comparer for refcounted @ref NavNodeKey objects.
// @bsiclass
//=======================================================================================
struct NavNodeKeyPtrComparer
    {
    bool operator()(NavNodeKeyCPtr const& lhs, NavNodeKeyCPtr const& rhs) const
        {
        if (lhs.get() == rhs.get())
            return false;
        if (lhs.IsNull())
            return true;
        if (rhs.IsNull())
            return false;
        return lhs->Compare(*rhs) < 0;
        }
    };

typedef bset<NavNodeKeyCPtr, NavNodeKeyPtrComparer> NavNodeKeySet;
typedef NavNodeKeySet* NavNodeKeySetP, &NavNodeKeySetR;
typedef NavNodeKeySet const* NavNodeKeySetCP;
typedef NavNodeKeySet const& NavNodeKeySetCR;

typedef bvector<NavNodeKeyCPtr> NavNodeKeyList;
typedef NavNodeKeyList* NavNodeKeyListP, &NavNodeKeyListR;
typedef NavNodeKeyList const* NavNodeKeyListCP;
typedef NavNodeKeyList const& NavNodeKeyListCR;

typedef RefCountedPtr<ECInstancesNodeKey>  ECInstancesNodeKeyPtr;
//=======================================================================================
//! NavNodeKey for nodes that represent multiple ECInstances.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECInstancesNodeKey : NavNodeKey
{
private:
    bvector<ECClassInstanceKey> m_instanceKeys;
private:
    ECInstancesNodeKey(bvector<ECClassInstanceKey> keys, Utf8String specificationIdentifier, bvector<Utf8String> path)
        : NavNodeKey(NAVNODE_TYPE_ECInstancesNode, specificationIdentifier, path), m_instanceKeys(keys)
        {}
    static bvector<Utf8String> CreateHashPath(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
        bvector<ECClassInstanceKey> const& instanceKeys);
protected:
    ECPRESENTATION_EXPORT bool _IsSimilar(NavNodeKey const& other) const override;
    ECInstancesNodeKey const* _AsECInstanceNodeKey() const override {return this;}
    ECPRESENTATION_EXPORT rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;
public:
    static ECInstancesNodeKeyPtr Create(bvector<ECClassInstanceKey> keys, Utf8String specificationIdentifier, bvector<Utf8String> path) {return new ECInstancesNodeKey(keys, specificationIdentifier, path);}
    static ECInstancesNodeKeyPtr Create(ECClassInstanceKey key, Utf8String specificationIdentifier, bvector<Utf8String> path) {return new ECInstancesNodeKey({ key }, specificationIdentifier, path);}
    static ECInstancesNodeKeyPtr Create(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, bvector<ECClassInstanceKey> const& instanceKeys)
        {
        auto hashPath = CreateHashPath(connectionIdentifier, specificationIdentifier, parentKey, instanceKeys);
        return new ECInstancesNodeKey(instanceKeys, specificationIdentifier, hashPath);
        }
    static ECInstancesNodeKeyPtr Create(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, bvector<ECClassInstanceKey> const& instanceKeys)
        {
        return Create(GetConnectionIdentifier(connection), specificationIdentifier, parentKey, instanceKeys);
        }
    static ECInstancesNodeKeyPtr Create(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, bvector<ECInstanceKey> const& instanceKeys)
        {
        bvector<ECClassInstanceKey> classInstanceKeys;
        std::transform(instanceKeys.begin(), instanceKeys.end(), std::back_inserter(classInstanceKeys),
            [&connection](auto const& key){return ECClassInstanceKey(connection.GetECDb().Schemas().GetClass(key.GetClassId()), key.GetInstanceId());});
        return Create(GetConnectionIdentifier(connection), specificationIdentifier, parentKey, classInstanceKeys);
        }
    bvector<ECClassInstanceKey> const& GetInstanceKeys() const {return m_instanceKeys;}
    bool HasInstanceKey(ECClassInstanceKeyCR key) const {return m_instanceKeys.end() != std::find(m_instanceKeys.begin(), m_instanceKeys.end(), key);}
    bool HasInstanceKey(ECInstanceKeyCR key) const {return m_instanceKeys.end() != std::find_if(m_instanceKeys.begin(), m_instanceKeys.end(), [&key](ECClassInstanceKeyCR k){return k.GetClass() && k.GetClass()->GetId() == key.GetClassId() && k.GetId() == key.GetInstanceId();});}
};

typedef RefCountedPtr<GroupingNodeKey> GroupingNodeKeyPtr;
//=======================================================================================
//! NavNodeKey for ECClass grouping nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GroupingNodeKey : NavNodeKey
{
private:
    uint64_t m_groupedInstancesCount;
    std::unique_ptr<bvector<ECInstanceKey>> m_groupedInstanceKeys;
protected:
    GroupingNodeKey(Utf8String type, Utf8String specificationIdentifier, bvector<Utf8String> path, uint64_t groupedInstancesCount, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys = nullptr)
        : NavNodeKey(type, specificationIdentifier, path), m_groupedInstancesCount(groupedInstancesCount), m_groupedInstanceKeys(std::move(groupedInstanceKeys))
        {}
    GroupingNodeKey const* _AsGroupingNodeKey() const override {return this;}
public:
    //! Get the number of ECInstances grouped by the node.
    uint64_t GetGroupedInstancesCount() const {return m_groupedInstancesCount;}
    //! Get a list of grouped instance keys, if known
    bvector<ECInstanceKey> const* GetGroupedInstanceKeys() const {return m_groupedInstanceKeys.get();}
};

typedef RefCountedPtr<ECClassGroupingNodeKey>  ECClassGroupingNodeKeyPtr;
//=======================================================================================
//! NavNodeKey for ECClass grouping nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECClassGroupingNodeKey : GroupingNodeKey
{
private:
    ECClassCP m_class;
    bool m_isPolymorphic;
private:
    ECClassGroupingNodeKey(Utf8String specificationIdentifier, bvector<Utf8String> path, ECClassCR ecClass, bool isPolymorphic, uint64_t groupedInstancesCount, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys)
        : GroupingNodeKey(NAVNODE_TYPE_ECClassGroupingNode, specificationIdentifier, path, groupedInstancesCount, std::move(groupedInstanceKeys)), m_class(&ecClass), m_isPolymorphic(isPolymorphic)
        {}
    static bvector<Utf8String> CreateHashPath(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
        ECClassCR groupingClass, bool isPolymorphic, PresentationQueryCP instanceKeysSelectQuery);
protected:
    ECClassGroupingNodeKey const* _AsECClassGroupingNodeKey() const override {return this;}
    ECPRESENTATION_EXPORT rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;
    ECPRESENTATION_EXPORT bool _IsSimilar(NavNodeKey const& other) const override;
public:
    //! Create an @ref ECClassGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECClassGroupingNodeKey> FromJson(IConnectionCR, BeJsConst);
    //! Create an @ref ECClassGroupingNodeKey using supplied parameters.
    //! @param[in] ecClass The grouping ECClass
    //! @param[in] isPolymorphic Is the class grouping polymorphically (instances of the class and its subclasses)
    //! @param[in] path The hashes which describe path from root node to this node.
    //! @param[in] groupedInstancesCount Count of ECInstances the node is grouping.
    static RefCountedPtr<ECClassGroupingNodeKey> Create(ECClassCR ecClass, bool isPolymorphic, Utf8String specificationIdentifier,
        bvector<Utf8String> path, uint64_t groupedInstancesCount, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys = nullptr)
        {
        return new ECClassGroupingNodeKey(specificationIdentifier, path, ecClass, isPolymorphic, groupedInstancesCount, std::move(groupedInstanceKeys));
        }
    static RefCountedPtr<ECClassGroupingNodeKey> Create(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
        ECClassCR groupingClass, bool isPolymorphic, uint64_t groupedInstancesCount,
        PresentationQueryCP instanceKeysSelectQuery, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys = nullptr)
        {
        auto hashPath = CreateHashPath(connectionIdentifier, specificationIdentifier, parentKey, groupingClass, isPolymorphic, instanceKeysSelectQuery);
        return new ECClassGroupingNodeKey(specificationIdentifier, hashPath, groupingClass, isPolymorphic, groupedInstancesCount, std::move(groupedInstanceKeys));
        }
    static RefCountedPtr<ECClassGroupingNodeKey> Create(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
        ECClassCR groupingClass, bool isPolymorphic, uint64_t groupedInstancesCount,
        PresentationQueryCP instanceKeysSelectQuery, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys = nullptr)
        {
        return Create(GetConnectionIdentifier(connection), specificationIdentifier, parentKey, groupingClass, isPolymorphic,
            groupedInstancesCount, instanceKeysSelectQuery, std::move(groupedInstanceKeys));
        }
    //! Get the ECClass
    ECClassCR GetECClass() const {return *m_class;}
    //! Get the ECClass ID
    ECClassId GetECClassId() const {return m_class->GetId();}
    //! Is the class grouping polymorphically
    bool IsPolymorphic() const {return m_isPolymorphic;}
};

typedef RefCountedPtr<ECPropertyGroupingNodeKey>  ECPropertyGroupingNodeKeyPtr;
//=======================================================================================
//! NavNodeKey for ECProperty grouping nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECPropertyGroupingNodeKey : GroupingNodeKey
{
private:
    ECClassCP m_class;
    Utf8String m_propertyName;
    rapidjson::Document::AllocatorType m_groupingValuesArrayAllocator;
    rapidjson::Document m_groupingValuesArray;
private:
    ECPropertyGroupingNodeKey(Utf8String specificationIdentifier, bvector<Utf8String> path, ECClassCR ecClass, Utf8String propertyName, RapidJsonValueCR groupingValuesArray, uint64_t groupedInstancesCount, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys)
        : GroupingNodeKey(NAVNODE_TYPE_ECPropertyGroupingNode, specificationIdentifier, path, groupedInstancesCount, std::move(groupedInstanceKeys)), m_class(&ecClass), m_propertyName(propertyName),
        m_groupingValuesArrayAllocator(64), m_groupingValuesArray(&m_groupingValuesArrayAllocator)
        {
        m_groupingValuesArray.CopyFrom(groupingValuesArray, m_groupingValuesArrayAllocator);
        }
    static bvector<Utf8String> CreateHashPath(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
        ECClassCR propertyClass, Utf8StringCR propertyName, RapidJsonValueCR groupedValuesJson, PresentationQueryCP instanceKeysSelectQuery);
protected:
    ECPropertyGroupingNodeKey const* _AsECPropertyGroupingNodeKey() const override {return this;}
    ECPRESENTATION_EXPORT rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;
    ECPRESENTATION_EXPORT bool _IsSimilar(NavNodeKey const& other) const override;
public:
    //! Create an @ref ECPropertyGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECPropertyGroupingNodeKey> FromJson(IConnectionCR, BeJsConst);
    //! Create an @ref ECPropertyGroupingNodeKey using supplied parameters.
    //! @param[in] ecClass ECClass of the grouping property
    //! @param[in] propertyName Name of the grouping property
    //! @param[in] groupingValuesArray A JSON array of grouping values.
    //! @param[in] path The hashes which describe path from root node to this node.
    //! @param[in] groupedInstancesCount Count of ECInstances the node is grouping.
    static RefCountedPtr<ECPropertyGroupingNodeKey> Create(ECClassCR ecClass, Utf8String propertyName, RapidJsonValueCR groupingValuesArray, Utf8String specificationIdentifier,
        bvector<Utf8String> path, uint64_t groupedInstancesCount, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys = nullptr)
        {
        return new ECPropertyGroupingNodeKey(specificationIdentifier, path, ecClass, propertyName, groupingValuesArray, groupedInstancesCount, std::move(groupedInstanceKeys));
        }
    static RefCountedPtr<ECPropertyGroupingNodeKey> Create(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
        ECClassCR propertyClass, Utf8StringCR propertyName, RapidJsonValueCR groupedValuesJson, uint64_t groupedInstancesCount,
        PresentationQueryCP instanceKeysSelectQuery, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys = nullptr)
        {
        auto hashPath = CreateHashPath(connectionIdentifier, specificationIdentifier, parentKey, propertyClass, propertyName, groupedValuesJson, instanceKeysSelectQuery);
        return new ECPropertyGroupingNodeKey(specificationIdentifier, hashPath, propertyClass, propertyName, groupedValuesJson, groupedInstancesCount, std::move(groupedInstanceKeys));
        }
    static RefCountedPtr<ECPropertyGroupingNodeKey> Create(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
        ECClassCR propertyClass, Utf8StringCR propertyName, RapidJsonValueCR groupedValuesJson, uint64_t groupedInstancesCount,
        PresentationQueryCP instanceKeysSelectQuery, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys = nullptr)
        {
        return Create(GetConnectionIdentifier(connection), specificationIdentifier, parentKey, propertyClass, propertyName, groupedValuesJson,
            groupedInstancesCount, instanceKeysSelectQuery, std::move(groupedInstanceKeys));
        }
    ECClassCR GetECClass() const {return *m_class;}
    ECClassId GetECClassId() const {return m_class->GetId();}
    Utf8StringCR GetPropertyName() const {return m_propertyName;}
    RapidJsonValueCR GetGroupingValuesArray() const {return m_groupingValuesArray;}
};

typedef RefCountedPtr<LabelGroupingNodeKey>  LabelGroupingNodeKeyPtr;
//=======================================================================================
//! NavNodeKey for display label grouping nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LabelGroupingNodeKey : GroupingNodeKey
{
private:
    Utf8String m_label;
private:
    LabelGroupingNodeKey(Utf8String specificationIdentifier, bvector<Utf8String> path, Utf8String label, uint64_t groupedInstancesCount, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys)
        : GroupingNodeKey(NAVNODE_TYPE_DisplayLabelGroupingNode, specificationIdentifier, path, groupedInstancesCount, std::move(groupedInstanceKeys)), m_label(label)
        {}
    static bvector<Utf8String> CreateHashPath(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
        Utf8StringCR label, PresentationQueryCP instanceKeysSelectQuery);
protected:
    LabelGroupingNodeKey const* _AsLabelGroupingNodeKey() const override {return this;}
    ECPRESENTATION_EXPORT rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;
    ECPRESENTATION_EXPORT bool _IsSimilar(NavNodeKey const& other) const override;
public:
    //! Create an @ref LabelGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<LabelGroupingNodeKey> FromJson(BeJsConst);
    //! Create an @ref LabelGroupingNodeKey using supplied parameters.
    //! @param[in] label Label of grouped nodes.
    //! @param[in] path The hashes which describe path from root node to this node.
    //! @param[in] groupedInstancesCount Count of ECInstances the node is grouping.
    static RefCountedPtr<LabelGroupingNodeKey> Create(Utf8String label, Utf8String specificationIdentifier, bvector<Utf8String> path, uint64_t groupedInstancesCount, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys = nullptr)
        {
        return new LabelGroupingNodeKey(specificationIdentifier, path, label, groupedInstancesCount, std::move(groupedInstanceKeys));
        }
    static RefCountedPtr<LabelGroupingNodeKey> Create(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, Utf8String label,
        uint64_t groupedInstancesCount, PresentationQueryCP instanceKeysSelectQuery, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys = nullptr)
        {
        return new LabelGroupingNodeKey(specificationIdentifier, CreateHashPath(connectionIdentifier, specificationIdentifier, parentKey, label, instanceKeysSelectQuery),
            label, groupedInstancesCount, std::move(groupedInstanceKeys));
        }
    static RefCountedPtr<LabelGroupingNodeKey> Create(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, Utf8String label,
        uint64_t groupedInstancesCount, PresentationQueryCP instanceKeysSelectQuery, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys = nullptr)
        {
        return Create(GetConnectionIdentifier(connection), specificationIdentifier, parentKey, label, groupedInstancesCount, instanceKeysSelectQuery, std::move(groupedInstanceKeys));
        }
    Utf8StringCR GetLabel() const {return m_label;}
};

//=======================================================================================
//! An abstract container of @ref NavNodeKey objects.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass
//=======================================================================================
struct INavNodeKeysContainer : RefCountedBase
{
    //===================================================================================
    //! An interator for the abstract @ref INavNodeKeysContainer.
    // @bsiclass
    //===================================================================================
    struct Iterator
    {
    private:
        INavNodeKeysContainer const& m_container;
        void* m_impl;
    public:
        Iterator(INavNodeKeysContainer const& container, void* impl) : m_container(container), m_impl(impl) {}
        Iterator(Iterator const& other) : m_container(other.m_container), m_impl(m_container._Copy(other.m_impl)) {}
        Iterator(Iterator&& other) : m_container(other.m_container), m_impl(std::move(other.m_impl)) {}
        ~Iterator() {m_container._Destroy(m_impl);}
        NavNodeKeyCPtr operator*() const {return m_container._Dereference(m_impl);}
        Iterator& operator=(Iterator const& rhs) {m_container._Assign(m_impl, rhs.m_impl); return *this;}
        Iterator& operator++() {m_container._Inc(m_impl); return *this;}
        bool operator==(Iterator const& rhs) const {return m_container._Equals(m_impl, rhs.m_impl);}
        bool operator!=(Iterator const& rhs) const {return !m_container._Equals(m_impl, rhs.m_impl);}
    };

private:
    mutable Utf8String m_hash;

protected:
    virtual void* _CreateBegin() const = 0;
    virtual void* _CreateEnd() const = 0;
    virtual void* _Find(NavNodeKeyCPtr) const = 0;
    virtual void* _Copy(void const*) const = 0;
    virtual void _Destroy(void*) const = 0;
    virtual NavNodeKeyCPtr _Dereference(void const*) const = 0;
    virtual bool _Equals(void const*, void const*) const = 0;
    virtual void _Assign(void*, void const*) const = 0;
    virtual void _Inc(void*) const = 0;
    virtual size_t _GetSize() const = 0;

public:
    //! Get the iterator at the beginning of this container.
    Iterator begin() const {return Iterator(*this, _CreateBegin());}
    //! Get the iterator at the end of this container.
    Iterator end() const {return Iterator(*this, _CreateEnd());}
    //! Get the iterator at the found element (if any)
    Iterator find(NavNodeKeyCPtr key) const {return Iterator(*this, _Find(key));}
    //! Get the size of this container.
    size_t size() const {return _GetSize();}
    //! Is this container empty.
    bool empty() const {return 0 == size();}
    //! Get hash string of all the keys in this container.
    ECPRESENTATION_EXPORT Utf8StringCR GetHash() const;
};
typedef INavNodeKeysContainer const& INavNodeKeysContainerCR;
typedef INavNodeKeysContainer const* INavNodeKeysContainerCP;
typedef RefCountedPtr<INavNodeKeysContainer const> INavNodeKeysContainerCPtr;

//=======================================================================================
//! A set-driven container of @ref NavNodeKey objects.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass
//=======================================================================================
struct NavNodeKeySetContainer : INavNodeKeysContainer
{
private:
    NavNodeKeySet m_set;
    NavNodeKeySet const* m_ptr;
private:
    NavNodeKeySetContainer(NavNodeKeySet set) : m_set(set), m_ptr(&m_set) {}
    NavNodeKeySetContainer(NavNodeKeySet const* set) : m_ptr(set) {}
protected:
    void* _CreateBegin() const override {return new NavNodeKeySet::const_iterator(m_ptr->begin());}
    void* _CreateEnd() const override {return new NavNodeKeySet::const_iterator(m_ptr->end());}
    void* _Find(NavNodeKeyCPtr key) const override {return new NavNodeKeySet::const_iterator(m_ptr->find(key));}
    void* _Copy(void const* other) const override {return new NavNodeKeySet::const_iterator(*static_cast<NavNodeKeySet::const_iterator const*>(other));}
    void _Destroy(void* iter) const override {delete static_cast<NavNodeKeySet::const_iterator*>(iter);}
    NavNodeKeyCPtr _Dereference(void const* iter) const override {return **static_cast<NavNodeKeySet::const_iterator const*>(iter);}
    bool _Equals(void const* lhs, void const* rhs) const override {return *static_cast<NavNodeKeySet::const_iterator const*>(lhs) == *static_cast<NavNodeKeySet::const_iterator const*>(rhs);}
    void _Assign(void* lhs, void const* rhs) const override {*static_cast<NavNodeKeySet::const_iterator*>(lhs) = *static_cast<NavNodeKeySet::const_iterator const*>(rhs);}
    void _Inc(void* iter) const override {++(*static_cast<NavNodeKeySet::const_iterator*>(iter));}
    size_t _GetSize() const override {return m_ptr->size();}
public:
    //! Creates an empty set-driven @ref NavNodeKey container.
    ECPRESENTATION_EXPORT static INavNodeKeysContainerCPtr Create();

    //! Creates @ref NavNodeKey container using the supplied @ref NavNodeKey set pointer.
    //! @param[in] set The set to create the container from.
    //! @note The container does not take ownership of the provided set - it has to remain valid
    //! for the container's lifetime and destroyed afterwards by the caller.
    ECPRESENTATION_EXPORT static INavNodeKeysContainerCPtr Create(NavNodeKeySet const* set);

    //! Creates @ref NavNodeKey container using the supplied @ref NavNodeKey set.
    //! @param[in] set The set to create the container from.
    ECPRESENTATION_EXPORT static INavNodeKeysContainerCPtr Create(NavNodeKeySet set);
};

//=======================================================================================
//! A vector-driven container of @ref NavNodeKey objects.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass
//=======================================================================================
struct NavNodeKeyListContainer : INavNodeKeysContainer
{
private:
    NavNodeKeyList m_list;
    NavNodeKeyList const* m_ptr;
private:
    NavNodeKeyListContainer(NavNodeKeyList list) : m_list(list), m_ptr(&m_list) {}
    NavNodeKeyListContainer(NavNodeKeyList const* list) : m_ptr(list) {}
protected:
    void* _CreateBegin() const override {return new NavNodeKeyList::const_iterator(m_ptr->begin());}
    void* _CreateEnd() const override {return new NavNodeKeyList::const_iterator(m_ptr->end());}
    void* _Find(NavNodeKeyCPtr key) const override {return new NavNodeKeyList::const_iterator(std::find(m_ptr->begin(), m_ptr->end(), key));}
    void* _Copy(void const* other) const override {return new NavNodeKeyList::const_iterator(*static_cast<NavNodeKeyList::const_iterator const*>(other));}
    void _Destroy(void* iter) const override {delete static_cast<NavNodeKeyList::const_iterator*>(iter);}
    NavNodeKeyCPtr _Dereference(void const* iter) const override {return **static_cast<NavNodeKeyList::const_iterator const*>(iter);}
    bool _Equals(void const* lhs, void const* rhs) const override {return *static_cast<NavNodeKeyList::const_iterator const*>(lhs) == *static_cast<NavNodeKeyList::const_iterator const*>(rhs);}
    void _Assign(void* lhs, void const* rhs) const override {*static_cast<NavNodeKeyList::const_iterator*>(lhs) = *static_cast<NavNodeKeyList::const_iterator const*>(rhs);}
    void _Inc(void* iter) const override {++(*static_cast<NavNodeKeyList::const_iterator*>(iter));}
    size_t _GetSize() const override {return m_ptr->size();}
public:
    //! Creates an empty vector-driven @ref NavNodeKey container.
    ECPRESENTATION_EXPORT static INavNodeKeysContainerCPtr Create();

    //! Creates @ref NavNodeKey container using the supplied @ref NavNodeKey vector.
    //! @param[in] list The vector to create the container from.
    //! @note The container does not take ownership of the provided list - it has to remain valid
    //! for the container's lifetime and destroyed afterwards by the caller.
    ECPRESENTATION_EXPORT static INavNodeKeysContainerCPtr Create(NavNodeKeyList const* list);

    //! Creates @ref NavNodeKey container using the supplied @ref NavNodeKey vector.
    //! @param[in] list The vector to create the container from.
    ECPRESENTATION_EXPORT static INavNodeKeysContainerCPtr Create(NavNodeKeyList list);
};

typedef bvector<ECInstanceKey> GroupedInstanceKeysList;
typedef GroupedInstanceKeysList const& GroupedInstanceKeysListCR;

END_BENTLEY_ECPRESENTATION_NAMESPACE
