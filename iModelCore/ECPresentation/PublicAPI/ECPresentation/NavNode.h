/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/ExtendedData.h>
#include <ECPresentation/LabelDefinition.h>
#include <ECPresentation/Connection.h>
#include <ECPresentation/NavNodeKey.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

struct NavNodesHelper;
struct NavNodesFactory;

struct PresentationQuery;

//=======================================================================================
//! An abstract navigation node object. @ref NavNode objects are used to create a hierarchy
//! for presentation-driven trees.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass
//=======================================================================================
struct NavNode : IRapidJsonExtendedDataHolder, RefCountedBase
{
    friend struct NavNodesHelper;
    friend struct NavNodesFactory;

private:
    mutable rapidjson::MemoryPoolAllocator<> m_allocator;
    mutable rapidjson::Document m_internalExtendedData;
    std::unique_ptr<rapidjson::Document> m_usersExtendedData;
    LabelDefinitionCPtr m_labelDefinition;
    BeGuid m_nodeId;
    NavNodeKeyPtr m_nodeKey;
    Utf8String m_description;
    Utf8String m_imageId;
    Utf8String m_foreColor;
    Utf8String m_backColor;
    Utf8String m_fontStyle;
    Utf8String m_type;
    bool m_determinedChildren;
    bool m_hasChildren;
    bool m_isChecked;
    bool m_isCheckboxVisible;
    bool m_isCheckboxEnabled;
    bool m_shouldAutoExpand;
    std::unique_ptr<PresentationQuery const> m_instanceKeysSelectQuery;

private:
    void InitUsersExtendedData(rapidjson::Value const* source = nullptr);

protected:
    RapidJsonValueR _GetExtendedData() const override {return m_internalExtendedData;}
    rapidjson::MemoryPoolAllocator<>& _GetExtendedDataAllocator() const override {return m_allocator;}

public:
    ECPRESENTATION_EXPORT NavNode();
    ECPRESENTATION_EXPORT NavNode(NavNodeCR);
    static NavNodePtr Create() {return new NavNode();}
    NavNodePtr Clone() const {return new NavNode(*this);}

    //! Set the @ref NavNodeKey for this node.
    void SetNodeKey(NavNodeKeyR nodeKey) {m_nodeKey = &nodeKey;}
    //! Get the @ref NavNodeKey for this node.
    NavNodeKeyCPtr GetKey() const {return m_nodeKey;}
    NavNodeKeyPtr GetKey() {return m_nodeKey;}

    //! Is this node equal to the supplied one.
    bool Equals(NavNodeCR other) const {return 0 == GetKey()->Compare(*other.GetKey());}

    //! Set guid for this node.
    void SetNodeId(BeGuid id) {m_nodeId = id;}
    //! Get guid of this node.
    BeGuidCR GetNodeId() const {return m_nodeId;}

    //! Set description.
    void SetDescription(Utf8String description) {m_description = description;}
    //! Get the description.
    Utf8StringCR GetDescription() const {return m_description;}

    //! Set image ID for this node.
    void SetImageId(Utf8String imageId) {m_imageId = imageId;}
    //! Get image ID for when this node.
    Utf8StringCR GetImageId() const {return m_imageId;}

    //! Set the color of this node's text.
    void SetForeColor(Utf8String color) {m_foreColor = color;}
    //! Get the color of this node's text.
    Utf8StringCR GetForeColor() const {return m_foreColor;}
    //! Set the background color of this node.
    void SetBackColor(Utf8String color) {m_backColor = color;}
    //! Get the background color of this node.
    Utf8StringCR GetBackColor() const {return m_backColor;}
    //! Set the font style of this node's text.
    void SetFontStyle(Utf8String style) {m_fontStyle = style;}
    //! Get the font style of this node's text.
    Utf8StringCR GetFontStyle() const {return m_fontStyle;}

    //! Set the type of this node.
    void SetType(Utf8String type) {m_type = type;}
    //! Get the type of this node.
    Utf8StringCR GetType() const {return m_type;}

    //! Get display label definition of this node.
    LabelDefinitionCR GetLabelDefinition() const {return *m_labelDefinition;}
    //! Set display label definition of this node.
    void SetLabelDefinition(LabelDefinitionCR value) {m_labelDefinition = &value;}

    //! Set if this node has a children.
    void SetHasChildren(bool value) {m_hasChildren = value; m_determinedChildren = true;}
    //! Does this node have children.
    ECPRESENTATION_EXPORT bool HasChildren() const;
    //! Does this node have determined whether it has children or not.
    bool DeterminedChildren() const {return m_determinedChildren;}
    void ResetHasChildren() {m_determinedChildren = false;}

    //! Set if this node is checked.
    void SetIsChecked(bool value) {m_isChecked = value;}
    //! Is this node checked.
    bool IsChecked() const {return m_isChecked;}
    //! Set if the checkbox is visible for this node.
    void SetIsCheckboxVisible(bool value) {m_isCheckboxVisible = value;}
    //! Is the checkbox visible for this node.
    bool IsCheckboxVisible() const {return m_isCheckboxVisible;}
    //! Set if the checkbox is enabled for this node.
    void SetIsCheckboxEnabled(bool value) {m_isCheckboxEnabled = value;}
    //! Is the checkbox enabled for this node.
    bool IsCheckboxEnabled() const {return m_isCheckboxEnabled;}
    //! Set if this node should be auto-expanded.
    void SetShouldAutoExpand(bool value) {m_shouldAutoExpand = value;}
    //! Should this node be auto-expanded.
    bool ShouldAutoExpand() const {return m_shouldAutoExpand;}

    ECPRESENTATION_EXPORT PresentationQuery const* GetInstanceKeysSelectQuery() const;
    ECPRESENTATION_EXPORT void SetInstanceKeysSelectQuery(std::unique_ptr<PresentationQuery const>);

    //! Get extended data injected into this node by API user
    ECPRESENTATION_EXPORT RapidJsonAccessor GetUsersExtendedData() const;
    void AddUsersExtendedData(Utf8CP key, ECValueCR value);

    //! Serialize the node to JSON
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
};

//=======================================================================================
//! A struct that defines a single step in the nodes path.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass
//=======================================================================================
struct NodesPathElement
{
    //=======================================================================================
    //! A simple struct to store data related to NodesPathElement filtering
    //! @ingroup GROUP_Presentation_Navigation
    // @bsiclass
    //=======================================================================================
    struct FilteringData
    {
    private:
        uint16_t m_occurances = 0;
        uint64_t m_childrenOccurances = 0;

    public:
        //! Set how many times filterWord occured in node label
        void SetOccurances(uint16_t occurances) { m_occurances = occurances;}

        //! Get how many times filterWord occured in node label
        uint16_t GetOccurances() const {return m_occurances;}

        //! Set how many times filterWord occured in all of node's children
        void SetChildrenOccurances(uint64_t occurances) { m_childrenOccurances = occurances; }

        //! Get how many times filterWord occured in all of node's children
        uint64_t GetChildrenOccurances() const { return m_childrenOccurances; }
    };

private:
    NavNodeCPtr m_node;
    size_t m_index;
    bool m_isMarked;
    bvector<NodesPathElement> m_children;
    FilteringData m_filteringData;

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

    //! Get data related to node hierarchy filtering
    FilteringData& GetFilteringData() {return m_filteringData;}
    FilteringData const& GetFilteringData() const {return m_filteringData;}

    //! Serialize this object to JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct INodeLabelCalculator
{
protected:
    virtual LabelDefinitionPtr _GetNodeLabel(ECClassInstanceKeyCR key, bvector<ECInstanceKey> const& prevLabelRequestsStack) const = 0;
public:
    virtual ~INodeLabelCalculator() {}
    LabelDefinitionPtr GetNodeLabel(ECClassInstanceKeyCR key, bvector<ECInstanceKey> const& prevLabelRequestsStack = {}) const {return _GetNodeLabel(key, prevLabelRequestsStack);}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
