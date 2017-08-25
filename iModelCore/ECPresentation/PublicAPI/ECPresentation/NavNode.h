/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/NavNode.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/ExtendedData.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define NAVNODE_TYPE_ECInstanceNode             "ECInstanceNode"
#define NAVNODE_TYPE_ECClassGroupingNode        "ECClassGroupingNode"
#define NAVNODE_TYPE_ECRelationshipGroupingNode "ECRelationshipGroupingNode"
#define NAVNODE_TYPE_ECPropertyGroupingNode     "ECPropertyGroupingNode"
#define NAVNODE_TYPE_DisplayLabelGroupingNode   "DisplayLabelGroupingNode"

struct ECClassGroupingNodeKey;
struct ECPropertyGroupingNodeKey;
struct ECInstanceNodeKey;
struct DisplayLabelGroupingNodeKey;

//=======================================================================================
//! Base class for a @ref NavNode key which identifies similar nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE NavNodeKey : RefCountedBase
{
private:
    Utf8String m_type;

protected:
    NavNodeKey(Utf8String type) : m_type(type) {}
    virtual ~NavNodeKey() {}
    virtual int _Compare(NavNodeKey const& other) const {return m_type.compare(other.m_type);}
    virtual bool _IsSimilar(NavNodeKey const& other) const {return m_type.Equals(other.m_type);}
    virtual ECClassGroupingNodeKey const* _AsECClassGroupingNodeKey() const {return nullptr;}
    virtual ECPropertyGroupingNodeKey const* _AsECPropertyGroupingNodeKey() const {return nullptr;}
    virtual ECInstanceNodeKey const* _AsECInstanceNodeKey() const {return nullptr;}
    virtual DisplayLabelGroupingNodeKey const* _AsDisplayLabelGroupingNodeKey() const {return nullptr;}
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::MemoryPoolAllocator<>*) const;

public:
    //! Try to get this key as a @ref ECClassGroupingNodeKey.
    ECClassGroupingNodeKey const* AsECClassGroupingNodeKey() const {return _AsECClassGroupingNodeKey();}
    //! Try to get this key as a @ref ECPropertyGroupingNodeKey.
    ECPropertyGroupingNodeKey const* AsECPropertyGroupingNodeKey() const {return _AsECPropertyGroupingNodeKey();}
    //! Try to get this key as a @ref ECInstanceNodeKey.
    ECInstanceNodeKey const* AsECInstanceNodeKey() const {return _AsECInstanceNodeKey();}
    //! Try to get this key as a @ref DisplayLabelGroupingNodeKey.
    DisplayLabelGroupingNodeKey const* AsDisplayLabelGroupingNodeKey() const {return _AsDisplayLabelGroupingNodeKey();}
    
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

    //! Get the type of the @ref NavNode.
    Utf8StringCR GetType() const {return m_type;}
    
    //! Serialize this key to JSON.
    rapidjson::Document AsJson(rapidjson::MemoryPoolAllocator<>* allocator = nullptr) const {return _AsJson(allocator);}
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

typedef NavNodeKeyList NavNodeKeyPath;

//=======================================================================================
//! NavNodeKey for grouping nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                03/2017
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GroupingNodeKey : NavNodeKey
{
private:
    uint64_t m_nodeId;
protected:
    GroupingNodeKey(uint64_t nodeId, Utf8String type) 
        : NavNodeKey(type), m_nodeId(nodeId) 
        {}
protected:
    ECPRESENTATION_EXPORT virtual int _Compare(NavNodeKey const& other) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::MemoryPoolAllocator<>*) const override;
public:
    //! Get the node id.
    uint64_t GetNodeId() const {return m_nodeId;}
};

//=======================================================================================
//! NavNodeKey for ECClass grouping nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECClassGroupingNodeKey : GroupingNodeKey
{
private:
    ECN::ECClassId m_classId;
protected:
    ECClassGroupingNodeKey(uint64_t nodeId, ECN::ECClassId classId, Utf8String type) 
        : GroupingNodeKey(nodeId, type), m_classId(classId) 
        {}
protected:
    ECPRESENTATION_EXPORT virtual bool _IsSimilar(NavNodeKey const& other) const override;
    virtual ECClassGroupingNodeKey const* _AsECClassGroupingNodeKey() const override {return this;}
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::MemoryPoolAllocator<>*) const override;
public:
    //! Create an @ref ECClassGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECClassGroupingNodeKey> Create(JsonValueCR);
    //! Create an @ref ECClassGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECClassGroupingNodeKey> Create(RapidJsonValueCR);
    //! Create an @ref ECClassGroupingNodeKey using the supplied parameters.
    static RefCountedPtr<ECClassGroupingNodeKey> Create(uint64_t nodeId, ECN::ECClassId classId, Utf8String type = NAVNODE_TYPE_ECClassGroupingNode)
        {
        return new ECClassGroupingNodeKey(nodeId, classId, type);
        }
    //! Get ECClass ID.
    ECN::ECClassId GetECClassId() const {return m_classId;}
};

//=======================================================================================
//! NavNodeKey for ECProperty grouping nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECPropertyGroupingNodeKey : ECClassGroupingNodeKey
{
private:
    Utf8String m_propertyName;
    int m_rangeIndex;
    rapidjson::Document const* m_value;
private:
    ECPropertyGroupingNodeKey(uint64_t nodeId, ECN::ECClassId ecClassId, Utf8String propertyName, int rangeIndex, rapidjson::Value const* value)
        : ECClassGroupingNodeKey(nodeId, ecClassId, NAVNODE_TYPE_ECPropertyGroupingNode), m_propertyName(propertyName), m_rangeIndex(rangeIndex), m_value(nullptr)
        {
        if (nullptr != value)
            {
            rapidjson::Document* doc = new rapidjson::Document();
            doc->CopyFrom(*value, doc->GetAllocator());
            m_value = doc;
            }
        }
    ~ECPropertyGroupingNodeKey() {DELETE_AND_CLEAR(m_value);}
protected:
    ECPRESENTATION_EXPORT virtual bool _IsSimilar(NavNodeKey const& other) const override;
    virtual ECPropertyGroupingNodeKey const* _AsECPropertyGroupingNodeKey() const override {return this;}
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::MemoryPoolAllocator<>*) const override;
public:
    //! Create an @ref ECPropertyGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECPropertyGroupingNodeKey> Create(JsonValueCR);
    //! Create an @ref ECPropertyGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECPropertyGroupingNodeKey> Create(RapidJsonValueCR);
    //! Create an @ref ECPropertyGroupingNodeKey using supplied parameters.
    //! @param[in] nodeId ID of the node.
    //! @param[in] ecClassId The ID of the ECClass that the grouping property belongs to.
    //! @param[in] propertyName The name of the grouping property.
    //! @param[in] rangeIndex The range index if this is range grouping or -1 if this is value grouping.
    //! @param[in] value The grouping value if this is value grouping or nullptr if this is range grouping
    static RefCountedPtr<ECPropertyGroupingNodeKey> Create(uint64_t nodeId, ECN::ECClassId ecClassId, Utf8String propertyName, int rangeIndex, rapidjson::Value const* value)
        {
        return new ECPropertyGroupingNodeKey(nodeId, ecClassId, propertyName, rangeIndex, value);
        }
    //! Create an @ref ECPropertyGroupingNodeKey using supplied parameters.
    //! @param[in] nodeId ID of the node.
    //! @param[in] ecClass The ECClass that the grouping property belongs to.
    //! @param[in] ecProperty The grouping property.
    //! @param[in] rangeIndex The range index if this is range grouping or -1 if this is value grouping.
    //! @param[in] value The grouping value if this is value grouping or nullptr if this is range grouping
    static RefCountedPtr<ECPropertyGroupingNodeKey> Create(uint64_t nodeId, ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty, int rangeIndex, rapidjson::Value const* value)
        {
        return Create(nodeId, ecClass.GetId(), ecProperty.GetName(), rangeIndex, value);
        }
    //! Get the grouping property name.
    Utf8StringCR GetPropertyName() const {return m_propertyName;}
    //! Get grouping range index (or -1 if grouping by value)
    int GetGroupingRangeIndex() const {return m_rangeIndex;}
    //! Get the grouping value (or nullptr if grouping by range)
    rapidjson::Value const* GetGroupingValue() const {return m_value;}
};

//=======================================================================================
//! NavNodeKey for display label grouping and custom nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DisplayLabelGroupingNodeKey : GroupingNodeKey
{
private:
    Utf8String m_label;
protected:
    DisplayLabelGroupingNodeKey(uint64_t nodeId, Utf8String label, Utf8String type) 
        : GroupingNodeKey(nodeId, type), m_label(label) 
        {}
protected:
    ECPRESENTATION_EXPORT virtual bool _IsSimilar(NavNodeKey const& other) const override;
    virtual DisplayLabelGroupingNodeKey const* _AsDisplayLabelGroupingNodeKey() const override {return this;}
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::MemoryPoolAllocator<>*) const override;
public:
    //! Create a @ref DisplayLabelGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<DisplayLabelGroupingNodeKey> Create(JsonValueCR);
    //! Create a @ref DisplayLabelGroupingNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<DisplayLabelGroupingNodeKey> Create(RapidJsonValueCR);
    //! Create a @ref DisplayLabelGroupingNodeKey using supplied parameters.
    //! @param[in] nodeId ID of the node.
    //! @param[in] label The display label.
    //! @param[in] type The node type. By default the display label node type is used, but in case
    //! of custom nodes, the custom type should be supplied.
    static RefCountedPtr<DisplayLabelGroupingNodeKey> Create(uint64_t nodeId, Utf8String label, Utf8String type = NAVNODE_TYPE_DisplayLabelGroupingNode)
        {
        return new DisplayLabelGroupingNodeKey(nodeId, label, type);
        }
    //! Set the display label.
    void SetLabel(Utf8String label) {m_label = label;}
    //! Get the display label.
    Utf8StringCR GetLabel() const {return m_label;}
};

typedef RefCountedPtr<ECInstanceNodeKey>  ECInstanceNodeKeyPtr;
//=======================================================================================
//! NavNodeKey for ECInstance nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECInstanceNodeKey : NavNodeKey
{
private:
    BeSQLite::EC::ECInstanceKey m_instanceKey;
private:
    ECInstanceNodeKey(ECN::ECClassId classId, BeSQLite::EC::ECInstanceId instanceId) 
        : NavNodeKey(NAVNODE_TYPE_ECInstanceNode), m_instanceKey(classId, instanceId)
        {}
protected:
    ECPRESENTATION_EXPORT virtual int _Compare(NavNodeKey const& other) const override;
    ECPRESENTATION_EXPORT virtual bool _IsSimilar(NavNodeKey const& other) const override;
    virtual ECInstanceNodeKey const* _AsECInstanceNodeKey() const override {return this;}
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::MemoryPoolAllocator<>*) const override;
public:
    //! Create an @ref ECInstanceNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECInstanceNodeKey> Create(JsonValueCR);
    //! Create an @ref ECInstanceNodeKey from a JSON object.
    ECPRESENTATION_EXPORT static RefCountedPtr<ECInstanceNodeKey> Create(RapidJsonValueCR);
    //! Create an @ref ECInstanceNodeKey using supplied parameters.
    //! @param[in] classId The ID of the ECInstance class.
    //! @param[in] instanceId The ECInstance ID.
    static RefCountedPtr<ECInstanceNodeKey> Create(ECN::ECClassId classId, BeSQLite::EC::ECInstanceId instanceId)
        {
        return new ECInstanceNodeKey(classId, instanceId);
        }
    //! Create an @ref ECInstanceNodeKey using supplied parameters.
    //! @param[in] key The ECInstance key.
    static RefCountedPtr<ECInstanceNodeKey> Create(BeSQLite::EC::ECInstanceKeyCR key)
        {
        return new ECInstanceNodeKey(key.GetClassId(), key.GetInstanceId());
        }
    //! Create an @ref ECInstanceNodeKey using supplied parameters.
    //! @param[in] instance The ECInstance.
    static RefCountedPtr<ECInstanceNodeKey> Create(ECN::IECInstanceCR instance)
        {
        BeSQLite::EC::ECInstanceId instanceId;
        BeSQLite::EC::ECInstanceId::FromString(instanceId, instance.GetInstanceId().c_str());
        return Create(instance.GetClass().GetId(), instanceId);
        }
    //! Get ECClass ID.
    ECN::ECClassId GetECClassId() const {return m_instanceKey.GetClassId();}
    //! Get the ECInstance ID.
    BeSQLite::EC::ECInstanceId GetInstanceId() const {return m_instanceKey.GetInstanceId();}
    //! Get the ECInstance key.
    BeSQLite::EC::ECInstanceKeyCR GetInstanceKey() const {return m_instanceKey;}
};

//=======================================================================================
//! An abstract navigation node object. @ref NavNode objects are used to create a hierarchy
//! for presentation-driven trees.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE NavNode : IRapidJsonExtendedDataHolder, RefCountedBase
{
private:
    mutable NavNodeKeyCPtr m_key;

protected:
    virtual uint64_t _GetNodeId() const = 0;
    virtual uint64_t _GetParentNodeId() const = 0;
    virtual NavNodeKeyCPtr _CreateKey() const = 0;
    virtual RefCountedPtr<ECN::IECInstance const> _GetInstance() const = 0;
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
    virtual rapidjson::Document _AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const = 0;

protected:
    void InvalidateKey() {m_key = nullptr;}

public:
    //! Get the @ref NavNodeKey for this node.
    ECPRESENTATION_EXPORT NavNodeKeyCR GetKey() const;

    //! Is this node equal to the supplied one.
    bool Equals(NavNodeCR other) const {return 0 == GetKey().Compare(other.GetKey());}

    //! Create a unique node ID.
    ECPRESENTATION_EXPORT static uint64_t CreateNodeId();
    //! Get the unique ID of this node.
    uint64_t GetNodeId() const {return _GetNodeId();}

    //! Get unique parent node ID or 0 if this is a root node.
    uint64_t GetParentNodeId() const {return _GetParentNodeId();}

    //! Get the ECInstance that's represented by this node or nullptr if this is not an ECInstance node.
    //! @note Calling this function executes a select query against the database to create the ECInstance
    //! if it hasn't been cached yet.
    RefCountedPtr<ECN::IECInstance const> GetInstance() const {return _GetInstance();}

    //! Get the label.
    Utf8String GetLabel() const {return _GetLabel();}
    //! Get the description.
    Utf8String GetDescription() const {return _GetDescription();}

    //! Get image ID for when this node is expanded.
    Utf8String GetExpandedImageId() const {return _GetExpandedImageId();}
    //! Get image ID for when this node is collapsed.
    Utf8String GetCollapsedImageId() const {return _GetCollapsedImageId();}

    //! Get the color of this node's text.
    Utf8String GetForeColor() const {return _GetForeColor();}
    //! Get the background color of this node.
    Utf8String GetBackColor() const {return _GetBackColor();}
    //! Get the font style of this node's text.
    Utf8String GetFontStyle() const {return _GetFontStyle();}

    //! Get the type of this node.
    Utf8String GetType() const {return _GetType();}

    //! Does this node have children.
    bool HasChildren() const {return _HasChildren();}

    //! Is this node selectable.
    bool IsSelectable() const {return _IsSelectable();}
    //! Is this node editable.
    bool IsEditable() const {return _IsEditable();}
    //! Is this node checked.
    bool IsChecked() const {return _IsChecked();}
    //! Is the checkbox visible for this node.
    bool IsCheckboxVisible() const {return _IsCheckboxVisible();}
    //! Is the checkbox enabled for this node.
    bool IsCheckboxEnabled() const {return _IsCheckboxEnabled();}
    //! Is this node expanded.
    bool IsExpanded() const {return _IsExpanded();}

    //! Serialize the node to JSON.
    rapidjson::Document AsJson(rapidjson::MemoryPoolAllocator<>* allocator = nullptr) const {return _AsJson(allocator);}
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
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::MemoryPoolAllocator<>* allocator = nullptr) const;
};

//=======================================================================================
//! An interface for @ref NavNode objects locater which can find nodes by their keys.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct  INavNodeLocater
{
protected:
    virtual NavNodeCPtr _LocateNode(NavNodeKeyCR key) const = 0;
public:
    virtual ~INavNodeLocater() {}
    NavNodeCPtr LocateNode(NavNodeKeyCR key) const {return _LocateNode(key);}
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
    NavNodeKeySetCP m_set;
    bool m_ownsSet;
private:
    NavNodeKeySetContainer(NavNodeKeySetCR set, bool makeCopy) : m_set(makeCopy ? new NavNodeKeySet(set) : &set), m_ownsSet(makeCopy) {}
    NavNodeKeySetContainer(NavNodeKeySet&& set) : m_ownsSet(true) {NavNodeKeySetP thisSet = new NavNodeKeySet(); thisSet->swap(set); m_set = thisSet;}
    ~NavNodeKeySetContainer() {if (m_ownsSet) delete m_set;}
protected:
    void* _CreateBegin() const override {return new NavNodeKeySet::const_iterator(m_set->begin());}
    void* _CreateEnd() const override {return new NavNodeKeySet::const_iterator(m_set->end());}
    void* _Find(NavNodeKeyCPtr key) const override {return new NavNodeKeySet::const_iterator(m_set->find(key));}
    void* _Copy(void const* other) const override {return new NavNodeKeySet::const_iterator(*static_cast<NavNodeKeySet::const_iterator const*>(other));}
    void _Destroy(void* iter) const override {delete static_cast<NavNodeKeySet::const_iterator*>(iter);}
    NavNodeKeyCPtr _Dereference(void const* iter) const override {return **static_cast<NavNodeKeySet::const_iterator const*>(iter);}
    bool _Equals(void const* lhs, void const* rhs) const override {return *static_cast<NavNodeKeySet::const_iterator const*>(lhs) == *static_cast<NavNodeKeySet::const_iterator const*>(rhs);}
    void _Assign(void* lhs, void const* rhs) const override {*static_cast<NavNodeKeySet::const_iterator*>(lhs) = *static_cast<NavNodeKeySet::const_iterator const*>(rhs);}
    void _Inc(void* iter) const override {++(*static_cast<NavNodeKeySet::const_iterator*>(iter));}
    size_t _GetSize() const override {return m_set->size();}
//__PUBLISH_SECTION_START__
public:
    //! Creates an empty set-driven @ref NavNodeKey container.
    ECPRESENTATION_EXPORT static INavNodeKeysContainerCPtr Create();
    
    //! Creates @ref NavNodeKey container using the supplied @ref NavNodeKey set.
    //! @param[in] set The set to create the container from.
    //! @param[in] makeCopy Should a copy of the supplied set be made. If not, the caller has to make 
    //! sure the supplied set remains valid for the lifetime of this container.
    ECPRESENTATION_EXPORT static INavNodeKeysContainerCPtr Create(NavNodeKeySetCR set, bool makeCopy = false);

    //! Creates @ref NavNodeKey container using the supplied @ref NavNodeKey set.
    ECPRESENTATION_EXPORT static INavNodeKeysContainerCPtr Create(NavNodeKeySet&& set);
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
    NavNodeKeyListCP m_list;
    bool m_ownsList;
private:
    NavNodeKeyListContainer(NavNodeKeyListCR list, bool makeCopy) : m_list(makeCopy ? new NavNodeKeyList(list) : &list), m_ownsList(makeCopy) {}
    NavNodeKeyListContainer(NavNodeKeyList&& list) : m_ownsList(true) {NavNodeKeyListP thisList = new NavNodeKeyList(); thisList->swap(list); m_list = thisList;}
    ~NavNodeKeyListContainer() {if (m_ownsList) delete m_list;}
protected:
    void* _CreateBegin() const override {return new NavNodeKeyList::const_iterator(m_list->begin());}
    void* _CreateEnd() const override {return new NavNodeKeyList::const_iterator(m_list->end());}
    void* _Find(NavNodeKeyCPtr key) const override {return new NavNodeKeyList::const_iterator(std::find(m_list->begin(), m_list->end(), key));}
    void* _Copy(void const* other) const override {return new NavNodeKeyList::const_iterator(*static_cast<NavNodeKeyList::const_iterator const*>(other));}
    void _Destroy(void* iter) const override {delete static_cast<NavNodeKeyList::const_iterator*>(iter);}
    NavNodeKeyCPtr _Dereference(void const* iter) const override {return **static_cast<NavNodeKeyList::const_iterator const*>(iter);}
    bool _Equals(void const* lhs, void const* rhs) const override {return *static_cast<NavNodeKeyList::const_iterator const*>(lhs) == *static_cast<NavNodeKeyList::const_iterator const*>(rhs);}
    void _Assign(void* lhs, void const* rhs) const override {*static_cast<NavNodeKeyList::const_iterator*>(lhs) = *static_cast<NavNodeKeyList::const_iterator const*>(rhs);}
    void _Inc(void* iter) const override {++(*static_cast<NavNodeKeyList::const_iterator*>(iter));}
    size_t _GetSize() const override {return m_list->size();}
//__PUBLISH_SECTION_START__
public:
    //! Creates an empty vector-driven @ref NavNodeKey container.
    ECPRESENTATION_EXPORT static INavNodeKeysContainerCPtr Create();
    
    //! Creates @ref NavNodeKey container using the supplied @ref NavNodeKey vector.
    //! @param[in] list The vector to create the container from.
    //! @param[in] makeCopy Should a copy of the supplied vector be made. If not, the caller has to make 
    //! sure the supplied vector remains valid for the lifetime of this container.
    ECPRESENTATION_EXPORT static INavNodeKeysContainerCPtr Create(NavNodeKeyListCR list, bool makeCopy = false);
    
    //! Creates @ref NavNodeKey container using the supplied @ref NavNodeKey vector.
    ECPRESENTATION_EXPORT static INavNodeKeysContainerCPtr Create(NavNodeKeyList&& list);
};

typedef bvector<BeSQLite::EC::ECInstanceKey> GroupedInstanceKeysList;
typedef GroupedInstanceKeysList const&       GroupedInstanceKeysListCR;

//=======================================================================================
//! An interface for a @ref NavNode factory which can create different kinds of nodes.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                09/2016
//=======================================================================================
struct INavNodesFactory
{
protected:
    virtual NavNodePtr _CreateECInstanceNode(BeSQLite::EC::ECDbCR, ECN::ECClassId, BeSQLite::EC::ECInstanceId, Utf8CP label) const = 0;
    virtual NavNodePtr _CreateECInstanceNode(BeSQLite::BeGuidCR, ECN::IECInstanceCR, Utf8CP label) const = 0;
    virtual NavNodePtr _CreateECClassGroupingNode(BeSQLite::BeGuidCR, ECN::ECClassCR, Utf8CP label, GroupedInstanceKeysListCR) const = 0;
    virtual NavNodePtr _CreateECRelationshipGroupingNode(BeSQLite::BeGuidCR, ECN::ECRelationshipClassCR, Utf8CP label, GroupedInstanceKeysListCR) const = 0;
    virtual NavNodePtr _CreateECPropertyGroupingNode(BeSQLite::BeGuidCR, ECN::ECClassCR, ECN::ECPropertyCR, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR) const = 0;
    virtual NavNodePtr _CreateDisplayLabelGroupingNode(BeSQLite::BeGuidCR, Utf8CP label, GroupedInstanceKeysListCR) const = 0;
    virtual NavNodePtr _CreateCustomNode(BeSQLite::BeGuidCR, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const = 0;

public:
    //! Virtual destructor.
    virtual ~INavNodesFactory() {}

    //! Creates an ECInstance node.
    //! @param[in] db The DB that the instance belongs to.
    //! @param[in] classId The ID of the ECInstance class.
    //! @param[in] instanceId The ID of the ECInstance.
    //! @param[in] label The label of the node.
    NavNodePtr CreateECInstanceNode(BeSQLite::EC::ECDbCR db, ECN::ECClassId classId, BeSQLite::EC::ECInstanceId instanceId, Utf8CP label) const
        {
        return _CreateECInstanceNode(db, classId, instanceId, label);
        }
    
    //! Creates an ECInstance node.
    //! @param[in] connectionId Guid of the connection that the instance belongs to.
    //! @param[in] instance The instance to create the node for.
    //! @param[in] label The label of the node.
    NavNodePtr CreateECInstanceNode(BeSQLite::BeGuidCR connectionId, ECN::IECInstanceCR instance, Utf8CP label) const
        {
        return _CreateECInstanceNode(connectionId, instance, label);
        }

    //! Creates an ECClass grouping node.
    //! @param[in] connectionId Guid of the connection that this ECClass is persisted in.
    //! @param[in] ecClass The ECClass to create the node for.
    //! @param[in] label The label of the node.
    //! @param[in] groupedInstanceKeys A list of instance keys grouped by the grouping node.
    NavNodePtr CreateECClassGroupingNode(BeSQLite::BeGuidCR connectionId, ECN::ECClassCR ecClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
        {
        return _CreateECClassGroupingNode(connectionId, ecClass, label, groupedInstanceKeys);
        }

    //! Creates an ECRelationship grouping node.
    //! @param[in] connectionId Guid of the connection that this ECRelationship is persisted in.
    //! @param[in] relationshipClass The ECRelationship to create the node for.
    //! @param[in] label The label of the node.
    //! @param[in] groupedInstanceKeys A list of instance keys grouped by the grouping node.
    NavNodePtr CreateECRelationshipGroupingNode(BeSQLite::BeGuidCR connectionId, ECN::ECRelationshipClassCR relationshipClass, 
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
    NavNodePtr CreateECPropertyGroupingNode(BeSQLite::BeGuidCR connectionId, ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty, 
        Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR groupedInstanceKeys) const
        {
        return _CreateECPropertyGroupingNode(connectionId, ecClass, ecProperty, label, imageId, groupingValue, isRangeGrouping, groupedInstanceKeys);
        }

    //! Creates a display label grouping node.
    //! @param[in] connectionId Guid of the connection whose instances this node groups.
    //! @param[in] label The label of the node.
    //! @param[in] groupedInstanceKeys A list of instance keys grouped by the grouping node.
    NavNodePtr CreateDisplayLabelGroupingNode(BeSQLite::BeGuidCR connectionId, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
        {
        return _CreateDisplayLabelGroupingNode(connectionId, label, groupedInstanceKeys);
        }

    //! Creates a custom node.
    //! @param[in] connectionId Guid of the connection the node is based on.
    //! @param[in] label The label of the node.
    //! @param[in] description The description of the node.
    //! @param[in] imageId The image ID to use for this node.
    //! @param[in] type The type identifier for this node.
    NavNodePtr CreateCustomNode(BeSQLite::BeGuidCR connectionId, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const
        {
        return _CreateCustomNode(connectionId, label, description, imageId, type);
        }
};

//! @}

END_BENTLEY_ECPRESENTATION_NAMESPACE
