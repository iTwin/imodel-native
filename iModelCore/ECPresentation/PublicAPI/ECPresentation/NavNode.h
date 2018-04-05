/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/NavNode.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/ExtendedData.h>
#include <Bentley/md5.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

#define NAVNODE_TYPE_ECInstanceNode             "ECInstanceNode"
#define NAVNODE_TYPE_ECClassGroupingNode        "ECClassGroupingNode"
#define NAVNODE_TYPE_ECRelationshipGroupingNode "ECRelationshipGroupingNode"
#define NAVNODE_TYPE_ECPropertyGroupingNode     "ECPropertyGroupingNode"
#define NAVNODE_TYPE_DisplayLabelGroupingNode   "DisplayLabelGroupingNode"

struct ECInstanceNodeKey;
struct ECClassGroupingNodeKey;
struct ECPropertyGroupingNodeKey;
struct LabelGroupingNodeKey;
//=======================================================================================
//! Base class for a @ref NavNode key which identifies similar nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE NavNodeKey : RefCountedBase
{
private:
    Utf8String m_type;
    bvector<Utf8String> m_pathFromRoot;
    mutable Utf8String m_keyHash;

protected:
    NavNodeKey(Utf8String type, bvector<Utf8String> path) : m_type(type), m_pathFromRoot(path) {}
    virtual ~NavNodeKey() {}
    ECPRESENTATION_EXPORT virtual int _Compare(NavNodeKey const& other) const;
    virtual bool _IsSimilar(NavNodeKey const& other) const {return m_type.Equals(other.m_type);}
    virtual ECInstanceNodeKey const* _AsECInstanceNodeKey() const {return nullptr;}
    virtual ECClassGroupingNodeKey const* _AsECClassGroupingNodeKey() const {return nullptr;}
    virtual ECPropertyGroupingNodeKey const* _AsECPropertyGroupingNodeKey() const {return nullptr;}
    virtual LabelGroupingNodeKey const* _AsLabelGroupingNodeKey() const {return nullptr;}
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::Document::AllocatorType*) const;
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash() const;

public:
    ECInstanceNodeKey const* AsECInstanceNodeKey() const {return _AsECInstanceNodeKey();}
    ECClassGroupingNodeKey const* AsECClassGroupingNodeKey() const {return _AsECClassGroupingNodeKey();}
    ECPropertyGroupingNodeKey const* AsECPropertyGroupingNodeKey() const {return _AsECPropertyGroupingNodeKey();}
    LabelGroupingNodeKey const* AsLabelGroupingNodeKey() const {return _AsLabelGroupingNodeKey();}

    //! Get the node hash.
    ECPRESENTATION_EXPORT Utf8String GetNodeHash() const;
    //! Get the path from root to this node.
    bvector<Utf8String> const& GetPathFromRoot() const {return m_pathFromRoot;}
    
    //! Compare this key with the supplied one.
    int Compare(NavNodeKey const& other) const {return _Compare(other);}
    //! Compare this key with the supplied one. Used for storing @ref NavNodeKey objects in maps and sets.
    bool operator<(NavNodeKey const& other) const {return Compare(other) < 0;}
    //! Is this node equal to the supplied one.
    bool operator==(NavNodeKey const& other) const {return 0 == Compare(other);}
    //! Is this node similar to the supplied one. 
    //! @note Similar nodes can be unequal, e.g. label grouping node keys are similar if labels match, but they're not
    //! equal because they represent different nodes.
    bool IsSimilar(NavNodeKey const& other) const {return _IsSimilar(other);}
    //! Get hash string of this node key.
    ECPRESENTATION_EXPORT Utf8StringCR GetHash() const;

    //! Get the type of the @ref NavNode.
    Utf8StringCR GetType() const {return m_type;}
    
    //! Serialize this key to JSON.
    rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(allocator);}

    //! Create an @ref NavNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static NavNodeKeyPtr Create(JsonValueCR);
    //! Create an @ref NavNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static NavNodeKeyPtr Create(RapidJsonValueCR);
    //! Create a new key.
    static NavNodeKeyPtr Create(Utf8String type, bvector<Utf8String> path) {return new NavNodeKey(type, path);}
    //! Create a key from the supplied JSON.
    ECPRESENTATION_EXPORT static NavNodeKeyPtr FromJson(JsonValueCR);
    //! Create a key from the supplied JSON.
    ECPRESENTATION_EXPORT static NavNodeKeyPtr FromJson(RapidJsonValueCR);
};

//=======================================================================================
//! Comparer for refcounted @ref NavNodeKey objects.
// @bsiclass                                    Grigas.Petraitis                08/2016
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

typedef RefCountedPtr<ECInstanceNodeKey>  ECInstanceNodeKeyPtr;
//=======================================================================================
//! NavNodeKey for ECInstance nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECInstanceNodeKey : NavNodeKey
{
private:
    ECInstanceKey m_instanceKey;
private:
    ECInstanceNodeKey(bvector<Utf8String> path, ECClassId classId, ECInstanceId instanceId) 
        : NavNodeKey(NAVNODE_TYPE_ECInstanceNode, path), m_instanceKey(classId, instanceId)
        {}
protected:
    ECPRESENTATION_EXPORT bool _IsSimilar(NavNodeKey const& other) const override;
    ECInstanceNodeKey const* _AsECInstanceNodeKey() const override {return this;}
    ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
public:
    //! Create an @ref ECInstanceNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECInstanceNodeKey> Create(JsonValueCR);
    //! Create an @ref ECInstanceNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECInstanceNodeKey> Create(RapidJsonValueCR);
    //! Create an @ref ECInstanceNodeKey using supplied parameters.
    //! @param[in] classId The ID of the ECInstance class.
    //! @param[in] instanceId The ECInstance ID.
    //! @param[in] path The hashes which describe path from root node to this node.
    static RefCountedPtr<ECInstanceNodeKey> Create(ECClassId classId, ECInstanceId instanceId, bvector<Utf8String> path)
        {
        return new ECInstanceNodeKey(path, classId, instanceId);
        }
    //! Create an @ref ECInstanceNodeKey using supplied parameters.
    //! @param[in] key The ECInstance key.
    //! @param[in] path The hashes which describe path from root node to this node.
    static RefCountedPtr<ECInstanceNodeKey> Create(ECInstanceKeyCR key, bvector<Utf8String> path)
        {
        return new ECInstanceNodeKey(path, key.GetClassId(), key.GetInstanceId());
        }
    //! Create an @ref ECInstanceNodeKey using supplied parameters.
    //! @param[in] instance The ECInstance.
    //! @param[in] path The hashes which describe path from root node to this node.
    static RefCountedPtr<ECInstanceNodeKey> Create(IECInstanceCR instance, bvector<Utf8String> path)
        {
        ECInstanceId instanceId;
        ECInstanceId::FromString(instanceId, instance.GetInstanceId().c_str());
        return Create(instance.GetClass().GetId(), instanceId, path);
        }
    //! Get the ECClass ID.
    ECClassId GetECClassId() const {return m_instanceKey.GetClassId();}
    //! Get the ECInstance ID.
    ECInstanceId GetInstanceId() const {return m_instanceKey.GetInstanceId();}
    //! Get the ECInstance key.
    ECInstanceKeyCR GetInstanceKey() const {return m_instanceKey;}
};

typedef RefCountedPtr<ECClassGroupingNodeKey>  ECClassGroupingNodeKeyPtr;
//=======================================================================================
//! NavNodeKey for ECClass grouping nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECClassGroupingNodeKey : NavNodeKey
{
private:
    ECClassId m_classId;
private:
    ECClassGroupingNodeKey(bvector<Utf8String> path, ECClassId classId) 
        : NavNodeKey(NAVNODE_TYPE_ECClassGroupingNode, path), m_classId(classId)
        {}
protected:
    //ECPRESENTATION_EXPORT int _Compare(NavNodeKey const& other) const override;
    ECClassGroupingNodeKey const* _AsECClassGroupingNodeKey() const override {return this;}
    ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
public:
    //! Create an @ref ECClassGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECClassGroupingNodeKey> Create(JsonValueCR);
    //! Create an @ref ECClassGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECClassGroupingNodeKey> Create(RapidJsonValueCR);
    //! Create an @ref ECClassGroupingNodeKey using supplied parameters.
    //! @param[in] classId The ID of the class.
    //! @param[in] path The hashes which describe path from root node to this node.
    static RefCountedPtr<ECClassGroupingNodeKey> Create(ECClassId classId, bvector<Utf8String> path = bvector<Utf8String>())
        {
        return new ECClassGroupingNodeKey(path, classId);
        }
    //! Get the ECClass ID.
    ECClassId GetECClassId() const {return m_classId;}
};

typedef RefCountedPtr<ECPropertyGroupingNodeKey>  ECPropertyGroupingNodeKeyPtr;
//=======================================================================================
//! NavNodeKey for ECProperty grouping nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECPropertyGroupingNodeKey : NavNodeKey
{
private:
    ECClassId m_classId;
    Utf8String m_propertyName;
    rapidjson::Document const* m_groupingValue;
private:
    ECPropertyGroupingNodeKey(bvector<Utf8String> path, ECClassId classId, Utf8String propertyName, rapidjson::Value const* groupingValue) 
        : NavNodeKey(NAVNODE_TYPE_ECPropertyGroupingNode, path), m_classId(classId), m_propertyName(propertyName), m_groupingValue(nullptr)
        {
        if (nullptr != groupingValue)
            {
            rapidjson::Document* doc = new rapidjson::Document();
            doc->CopyFrom(*groupingValue, doc->GetAllocator());
            m_groupingValue = doc;
            }
        }
    ~ECPropertyGroupingNodeKey() {DELETE_AND_CLEAR(m_groupingValue);}
protected:
    ECPropertyGroupingNodeKey const* _AsECPropertyGroupingNodeKey() const override {return this;}
    ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
public:
    //! Create an @ref ECPropertyGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECPropertyGroupingNodeKey> Create(JsonValueCR);
    //! Create an @ref ECPropertyGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECPropertyGroupingNodeKey> Create(RapidJsonValueCR);
    //! Create an @ref ECPropertyGroupingNodeKey using supplied parameters.
    //! @param[in] classId The ID of the class.
    //! @param[in] propertyName Name of the grouping property
    //! @param[in] groupingValue The grouping value.
    //! @param[in] path The hashes which describe path from root node to this node.
    static RefCountedPtr<ECPropertyGroupingNodeKey> Create(ECClassId classId, Utf8String propertyName, rapidjson::Value const* groupingValue, bvector<Utf8String> path = bvector<Utf8String>())
        {
        return new ECPropertyGroupingNodeKey(path, classId, propertyName, groupingValue);
        }
    ECClassId GetECClassId() const {return m_classId;}
    Utf8StringCR GetPropertyName() const {return m_propertyName;}
    rapidjson::Value const* GetGroupingValue() const {return m_groupingValue;}
};

typedef RefCountedPtr<LabelGroupingNodeKey>  LabelGroupingNodeKeyPtr;
//=======================================================================================
//! NavNodeKey for display label grouping nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LabelGroupingNodeKey : NavNodeKey
{
private:
    Utf8String m_label;
private:
    LabelGroupingNodeKey(bvector<Utf8String> path, Utf8String label) 
        : NavNodeKey(NAVNODE_TYPE_DisplayLabelGroupingNode, path), m_label(label)
        {}
protected:
    LabelGroupingNodeKey const* _AsLabelGroupingNodeKey() const override {return this;}
    ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
public:
    //! Create an @ref LabelGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<LabelGroupingNodeKey> Create(JsonValueCR);
    //! Create an @ref LabelGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<LabelGroupingNodeKey> Create(RapidJsonValueCR);
    //! Create an @ref LabelGroupingNodeKey using supplied parameters.
    //! @param[in] label Label of grouped nodes.
    //! @param[in] path The hashes which describe path from root node to this node.
    static RefCountedPtr<LabelGroupingNodeKey> Create(Utf8String label, bvector<Utf8String> path = bvector<Utf8String>())
        {
        return new LabelGroupingNodeKey(path, label);
        }
    Utf8StringCR GetLabel() const {return m_label;}
};

//=======================================================================================
//! An abstract navigation node object. @ref NavNode objects are used to create a hierarchy
//! for presentation-driven trees.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE NavNode : IRapidJsonExtendedDataHolder, RefCountedBase
{
protected:
    virtual uint64_t _GetInstanceId() const = 0;
    virtual uint64_t _GetNodeId() const = 0;
    virtual uint64_t _GetParentNodeId() const = 0;
    virtual NavNodeKeyCPtr _GetNodeKey() const = 0;
    virtual Utf8String _GetLabel() const = 0;
    virtual Utf8String _GetDescription() const = 0;
    virtual Utf8String _GetExpandedImageId() const = 0;
    virtual Utf8String _GetCollapsedImageId() const = 0;
    virtual Utf8String _GetForeColor() const = 0;
    virtual Utf8String _GetBackColor() const = 0;
    virtual Utf8String _GetFontStyle() const = 0;
    virtual Utf8String _GetType() const = 0;
    virtual bool _HasChildren() const = 0;
    virtual bool _IsSelectable() const = 0;
    virtual bool _IsEditable() const = 0;
    virtual bool _IsChecked() const = 0;
    virtual bool _IsCheckboxVisible() const = 0;
    virtual bool _IsCheckboxEnabled() const = 0;  
    virtual bool _IsExpanded() const = 0;

    virtual void _SetInstanceId(uint64_t instanceId) = 0;
    virtual void _SetNodeId(uint64_t nodeId) = 0;
    virtual void _SetParentNodeId(uint64_t parentNodeId) = 0;
    virtual void _SetNodeKey(NavNodeKeyCR nodeKey) = 0;
    virtual void _SetLabel(Utf8CP label) = 0;
    virtual void _SetDescription(Utf8CP description) = 0;
    virtual void _SetExpandedImageId(Utf8CP imageId) = 0;
    virtual void _SetCollapsedImageId(Utf8CP imageId) = 0;
    virtual void _SetForeColor(Utf8CP color) = 0;
    virtual void _SetBackColor(Utf8CP color) = 0;
    virtual void _SetFontStyle(Utf8CP style) = 0;
    virtual void _SetType(Utf8CP type) = 0;
    virtual void _SetHasChildren(bool value) = 0;
    virtual void _SetIsSelectable(bool value) = 0;
    virtual void _SetIsEditable(bool value) = 0;
    virtual void _SetIsChecked(bool value) = 0;
    virtual void _SetIsCheckboxVisible(bool value) = 0;
    virtual void _SetIsCheckboxEnabled(bool value) = 0;
    virtual void _SetIsExpanded(bool value) = 0;

public:
    //! Set the @ref NavNodeKey for this node.
    void SetNodeKey(NavNodeKeyCR nodeKey) {_SetNodeKey(nodeKey);}
    //! Get the @ref NavNodeKey for this node.
    NavNodeKeyCPtr GetKey() const {return _GetNodeKey();}

    //! Is this node equal to the supplied one.
    bool Equals(NavNodeCR other) const {return 0 == GetKey()->Compare(*other.GetKey());}

    //! Set the instance Id.
    void SetInstanceId(uint64_t instanceId) {_SetInstanceId(instanceId);}
    //! Get the instance id.
    uint64_t GetInstanceId() const {return _GetInstanceId();}

    //! Set unique ID of this node.
    void SetNodeId(uint64_t nodeId) {_SetNodeId(nodeId);}
    //! Get the unique ID of this node.
    uint64_t GetNodeId() const {return _GetNodeId();}

    //! Set unique parent node ID
    void SetParentNodeId(uint64_t parentNodeId) {_SetParentNodeId(parentNodeId);}
    //! Get unique parent node ID or 0 if this is a root node.
    uint64_t GetParentNodeId() const {return _GetParentNodeId();}

    //! Set label.
    void SetLabel(Utf8CP label) {_SetLabel(label);}
    //! Get the label.
    Utf8String GetLabel() const {return _GetLabel();}
    //! Set description.
    void SetDescription(Utf8CP description) {_SetDescription(description);}
    //! Get the description.
    Utf8String GetDescription() const {return _GetDescription();}

    //! Set image ID for this node when it is expanded.
    void SetExpandedImageId(Utf8CP imageId) {_SetExpandedImageId(imageId);}
    //! Get image ID for when this node is expanded.
    Utf8String GetExpandedImageId() const {return _GetExpandedImageId();}
    //! Set image ID for this node when it is collapsed.
    void SetCollapsedImageId(Utf8CP imageId) {_SetCollapsedImageId(imageId);}
    //! Get image ID for when this node is collapsed.
    Utf8String GetCollapsedImageId() const {return _GetCollapsedImageId();}

    //! Set the color of this node's text.
    void SetForeColor(Utf8CP color) {_SetForeColor(color);}
    //! Get the color of this node's text.
    Utf8String GetForeColor() const {return _GetForeColor();}
    //! Set the background color of this node.
    void SetBackColor(Utf8CP color) {_SetBackColor(color);}
    //! Get the background color of this node.
    Utf8String GetBackColor() const {return _GetBackColor();}
    //! Set the font style of this node's text.
    void SetFontStyle(Utf8CP style) {_SetFontStyle(style);}
    //! Get the font style of this node's text.
    Utf8String GetFontStyle() const {return _GetFontStyle();}

    //! Set the type of this node.
    void SetType(Utf8CP type) {_SetType(type);}
    //! Get the type of this node.
    Utf8String GetType() const {return _GetType();}

    //! Set if this node has a children.
    void SetHasChildren(bool value) {_SetHasChildren(value);}
    //! Does this node have children.
    bool HasChildren() const {return _HasChildren();}

    //! Set if this node is selectable.
    void SetIsSelectable(bool value) {_SetIsSelectable(value);}
    //! Is this node selectable.
    bool IsSelectable() const {return _IsSelectable();}
    //! Set if this node is editable.
    void SetIsEditable(bool value) {_SetIsEditable(value);}
    //! Is this node editable.
    bool IsEditable() const {return _IsEditable();}
    //! Set if this node is checked.
    void SetIsChecked(bool value) {_SetIsChecked(value);}
    //! Is this node checked.
    bool IsChecked() const {return _IsChecked();}
    //! Set if the checkbox is visible for this node.
    void SetIsCheckboxVisible(bool value) {_SetIsCheckboxVisible(value);}
    //! Is the checkbox visible for this node.
    bool IsCheckboxVisible() const {return _IsCheckboxVisible();}
    //! Set if the checkbox is enabled for this node.
    void SetIsCheckboxEnabled(bool value) {_SetIsCheckboxEnabled(value);}
    //! Is the checkbox enabled for this node.
    bool IsCheckboxEnabled() const {return _IsCheckboxEnabled();}
    //! Set if this node is expanded.
    void SetIsExpanded(bool value) {_SetIsExpanded(value);}
    //! Is this node expanded.
    bool IsExpanded() const {return _IsExpanded();}

    //! Serialize the node to JSON.
    ECPRESENTATION_EXPORT static NavNodePtr FromJson(RapidJsonValueCR json);
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
};

//=======================================================================================
//! A struct that defines a single step in the nodes path.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                12/2016
//=======================================================================================
struct NodesPathElement
{
private:
    NavNodeCPtr m_node;
    size_t m_index;
    bool m_isMarked;
    bvector<NodesPathElement> m_children;

public:
    //! Constructor. Creates an invalid object.
    NodesPathElement() : m_index(0), m_isMarked(false) {}

    //! Constructor.
    NodesPathElement(NavNodeCR node, size_t index) : m_node(&node), m_index(index), m_isMarked(false) {}

    //! Get the node.
    NavNodeCPtr GetNode() const {return m_node;}

    //! Get the node index.
    size_t GetIndex() const {return m_index;}

    //! Get writable list of children
    bvector<NodesPathElement>& GetChildren() {return m_children;}

    //! Get readonly list of children.
    bvector<NodesPathElement> const& GetChildren() const {return m_children;}

    //! Mark this path element.
    void SetIsMarked(bool marked) {m_isMarked = marked;}

    //! Is this path element marked.
    bool IsMarked() const {return m_isMarked;}

    //! Serialize this object to JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
};

//=======================================================================================
//! An abstract container of @ref NavNodeKey objects.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct INavNodeKeysContainer : RefCountedBase
{
    //===================================================================================
    //! An interator for the abstract @ref INavNodeKeysContainer.
    // @bsiclass                                     Grigas.Petraitis            08/2016
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
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct NavNodeKeySetContainer : INavNodeKeysContainer
{
//__PUBLISH_SECTION_END__
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
//__PUBLISH_SECTION_START__
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
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct NavNodeKeyListContainer : INavNodeKeysContainer
{
//__PUBLISH_SECTION_END__
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
//__PUBLISH_SECTION_START__
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
typedef GroupedInstanceKeysList const&       GroupedInstanceKeysListCR;

typedef bmap<ECClassId, bset<ECInstanceId>> InstanceKeyMap;
//=======================================================================================
//! Struct that describes ECInstanceKeys and NavNodeKeys set
// @bsiclass                                    Saulius.Skliutas                01/2018
//=======================================================================================
struct KeySet : RefCountedBase
{
private:
    InstanceKeyMap m_instances;
    NavNodeKeySet m_nodes;
    mutable INavNodeKeysContainerCPtr m_nodesContainer;

private:
    void InvalidateNodesContainer() {m_nodesContainer = nullptr;}

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
    ECPRESENTATION_EXPORT static KeySetPtr Create(bvector<ECInstanceKey> const& instanceKeys);
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
    bool Equals(KeySetCR other) const {return GetHash().Equals(other.GetHash());}
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
    bool Add(ECClassId classId, ECInstanceId instanceId) {InvalidateNodesContainer(); return m_instances[classId].insert(instanceId).second;}
    //! Add instance ket to this key set. (Returns true if key was added and false if key was in the set already)
    bool Add(ECInstanceKey instanceKey) {return Add(instanceKey.GetClassId(), instanceKey.GetInstanceId());}
    //! Add NavNode key to this key set. (Returns true if key was added and false if key was in the set already)
    bool Add(NavNodeKeyCR nodeKey) {InvalidateNodesContainer(); return m_nodes.insert(&nodeKey).second;}
    //! Returns whether this key set contains supplied instance key.
    ECPRESENTATION_EXPORT bool Contains(ECClassId classId, ECInstanceId instanceId) const;
    //! Returns whether this key set contains supplied instance key.
    bool Contains(ECInstanceKey instanceKey) const {return Contains(instanceKey.GetClassId(), instanceKey.GetInstanceId());}
    //! Return whether this key set contains supplied NavNode key.
    bool Contains(NavNodeKeyCPtr nodeKey) const {return m_nodes.end() != m_nodes.find(nodeKey);}
    //! Clears this key set.
    void Clear() {InvalidateNodesContainer(); m_instances.clear(); m_nodes.clear();}
    //! Returns whether this key set is empty.
    bool empty() const {return m_instances.empty() && m_nodes.empty();}
    //! Merge supplied key set which this key set. Returns number of new keys added.
    ECPRESENTATION_EXPORT uint64_t MergeWith(KeySetCR other);
    //! Remove supplied keys from this key set. Return number of keys removed.
    ECPRESENTATION_EXPORT uint64_t Remove(KeySetCR toRemove);
    //! Remove supplied nav node key from this key set.
    bool Remove(NavNodeKeyCR nodeKey) {return 0 != m_nodes.erase(&nodeKey);}
    //! Remove supplied instance key from this key set.
    ECPRESENTATION_EXPORT bool Remove(ECClassId classId, ECInstanceId instanceId);
    //! Remove supplied instance ky from this key set.
    bool Remove(ECInstanceKey instanceKey) {return Remove(instanceKey.GetClassId(), instanceKey.GetInstanceId());}

    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPRESENTATION_EXPORT static KeySetPtr FromJson(JsonValueCR json);
};

//=======================================================================================
//! An interface for a @ref NavNode factory which can create different kinds of nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                09/2016
//=======================================================================================
struct INavNodesFactory
{
protected:
    virtual NavNodePtr _CreateECInstanceNode(IConnectionCR, ECClassId, ECInstanceId, Utf8CP label) const = 0;
    virtual NavNodePtr _CreateECInstanceNode(Utf8StringCR, IECInstanceCR, Utf8CP label) const = 0;
    virtual NavNodePtr _CreateECClassGroupingNode(Utf8StringCR, ECClassCR, Utf8CP label, GroupedInstanceKeysListCR) const = 0;
    virtual NavNodePtr _CreateECRelationshipGroupingNode(Utf8StringCR, ECRelationshipClassCR, Utf8CP label, GroupedInstanceKeysListCR) const = 0;
    virtual NavNodePtr _CreateECPropertyGroupingNode(Utf8StringCR, ECClassCR, ECPropertyCR, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR) const = 0;
    virtual NavNodePtr _CreateDisplayLabelGroupingNode(Utf8StringCR, Utf8CP label, GroupedInstanceKeysListCR) const = 0;
    virtual NavNodePtr _CreateCustomNode(Utf8StringCR, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const = 0;

public:
    //! Virtual destructor.
    virtual ~INavNodesFactory() {}

    //! Creates an ECInstance node.
    //! @param[in] connection The connection that the instance belongs to.
    //! @param[in] classId The ID of the ECInstance class.
    //! @param[in] instanceId The ID of the ECInstance.
    //! @param[in] label The label of the node.
    NavNodePtr CreateECInstanceNode(IConnectionCR connection, ECClassId classId, ECInstanceId instanceId, Utf8CP label) const
        {
        return _CreateECInstanceNode(connection, classId, instanceId, label);
        }
    
    //! Creates an ECInstance node.
    //! @param[in] connectionId Guid of the connection that the instance belongs to.
    //! @param[in] instance The instance to create the node for.
    //! @param[in] label The label of the node.
    NavNodePtr CreateECInstanceNode(Utf8StringCR connectionId, IECInstanceCR instance, Utf8CP label) const
        {
        return _CreateECInstanceNode(connectionId, instance, label);
        }

    //! Creates an ECClass grouping node.
    //! @param[in] connectionId Guid of the connection that this ECClass is persisted in.
    //! @param[in] ecClass The ECClass to create the node for.
    //! @param[in] label The label of the node.
    //! @param[in] groupedInstanceKeys A list of instance keys grouped by the grouping node.
    NavNodePtr CreateECClassGroupingNode(Utf8StringCR connectionId, ECClassCR ecClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
        {
        return _CreateECClassGroupingNode(connectionId, ecClass, label, groupedInstanceKeys);
        }

    //! Creates an ECRelationship grouping node.
    //! @param[in] connectionId Guid of the connection that this ECRelationship is persisted in.
    //! @param[in] relationshipClass The ECRelationship to create the node for.
    //! @param[in] label The label of the node.
    //! @param[in] groupedInstanceKeys A list of instance keys grouped by the grouping node.
    NavNodePtr CreateECRelationshipGroupingNode(Utf8StringCR connectionId, ECRelationshipClassCR relationshipClass, 
        Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
        {
        return _CreateECRelationshipGroupingNode(connectionId, relationshipClass, label, groupedInstanceKeys);
        }

    //! Creates an ECProperty grouping node.
    //! @param[in] connectionId Guid of the connection that this ECProperty is persisted in.
    //! @param[in] ecClass The ECClass of the ECProperty. If the property belongs to a base class, but the grouped instances 
    //! are of derived class, this should be the derived class.
    //! @param[in] ecProperty The ECProperty to create the node for.
    //! @param[in] label The label of the node.
    //! @param[in] imageId The image ID to use for this node.
    //! @param[in] groupingValue The grouping value or range index if creating a range grouping node.
    //! @param[in] isRangeGrouping Should a range grouping node be created.
    //! @param[in] groupedInstanceKeys A list of instance keys grouped by the grouping node.
    NavNodePtr CreateECPropertyGroupingNode(Utf8StringCR connectionId, ECClassCR ecClass, ECPropertyCR ecProperty, 
        Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR groupedInstanceKeys) const
        {
        return _CreateECPropertyGroupingNode(connectionId, ecClass, ecProperty, label, imageId, groupingValue, isRangeGrouping, groupedInstanceKeys);
        }

    //! Creates a display label grouping node.
    //! @param[in] connectionId Guid of the connection whose instances this node groups.
    //! @param[in] label The label of the node.
    //! @param[in] groupedInstanceKeys A list of instance keys grouped by the grouping node.
    NavNodePtr CreateDisplayLabelGroupingNode(Utf8StringCR connectionId, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
        {
        return _CreateDisplayLabelGroupingNode(connectionId, label, groupedInstanceKeys);
        }

    //! Creates a custom node.
    //! @param[in] connectionId Guid of the connection the node is based on.
    //! @param[in] label The label of the node.
    //! @param[in] description The description of the node.
    //! @param[in] imageId The image ID to use for this node.
    //! @param[in] type The type identifier for this node.
    NavNodePtr CreateCustomNode(Utf8StringCR connectionId, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const
        {
        return _CreateCustomNode(connectionId, label, description, imageId, type);
        }
};

//! @}

END_BENTLEY_ECPRESENTATION_NAMESPACE
