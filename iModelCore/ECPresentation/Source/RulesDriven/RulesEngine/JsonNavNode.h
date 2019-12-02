/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/NavNode.h>
#include <ECPresentation/Update.h>
#include "RulesEngineTypes.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

// Member names of the JsonNavNode JSON object
#define NAVNODE_NodeId              "NodeId"
#define NAVNODE_ParentNodeId        "ParentNodeId"
#define NAVNODE_Key                 "Key"
#define NAVNODE_InstanceId          "ECInstanceId"
#define NAVNODE_ExpandedImageId     "ExpandedImageId"
#define NAVNODE_CollapsedImageId    "CollapsedImageId"
#define NAVNODE_ForeColor           "ForeColor"
#define NAVNODE_BackColor           "BackColor"
#define NAVNODE_FontStyle           "FontStyle"
#define NAVNODE_Type                "Type"
#define NAVNODE_HasChildren         "HasChildren"
#define NAVNODE_IsSelectable        "IsSelectable"
#define NAVNODE_IsEditable          "IsEditable"
#define NAVNODE_IsChecked           "IsChecked"
#define NAVNODE_IsCheckboxVisible   "IsCheckboxVisible"
#define NAVNODE_IsCheckboxEnabled   "IsCheckboxEnabled"
#define NAVNODE_IsExpanded          "IsExpanded"
#define NAVNODE_Label               "Label"
#define NAVNODE_Description         "Description"
#define NAVNODE_InternalData        "InternalData"
#define NAVNODE_UsersExtendedData   "ExtendedData"

#define NAVNODE_JSON_CHUNK_SIZE     256

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE JsonNavNode : NavNode
{
    friend struct NavNodesHelper;
    friend struct JsonNavNodesFactory;

private:
    mutable rapidjson::MemoryPoolAllocator<> m_allocator;
    mutable rapidjson::Document m_internalExtendedData;
    rapidjson::Document* m_usersExtendedData;
    uint64_t m_nodeId;
    uint64_t m_parentNodeId;
    NavNodeKeyCPtr m_nodeKey;
    Utf8String m_label;
    Utf8String m_description;
    Utf8String m_imageId;
    Utf8String m_foreColor;
    Utf8String m_backColor;
    Utf8String m_fontStyle;
    Utf8String m_type;
    bool m_determinedChildren;
    bool m_hasChildren;
    bool m_isSelectable;
    bool m_isEditable;
    bool m_isChecked;
    bool m_isCheckboxVisible;
    bool m_isCheckboxEnabled;
    bool m_isExpanded;

private:
    void InitUsersExtendedData(rapidjson::Value const* source = nullptr);
    
protected:
    ECPRESENTATION_EXPORT JsonNavNode();
    ECPRESENTATION_EXPORT JsonNavNode(JsonNavNode const&);

    rapidjson::MemoryPoolAllocator<>& _GetExtendedDataAllocator() const override {return m_allocator;}
    RapidJsonValueR _GetExtendedData() const override {return m_internalExtendedData;}
    rapidjson::Value const* _GetUsersExtendedData() const override {return m_usersExtendedData;}

    NavNodePtr _Clone() const override {return new JsonNavNode(*this);}

    uint64_t _GetNodeId() const override {return m_nodeId;}
    uint64_t _GetParentNodeId() const override {return m_parentNodeId;}
    NavNodeKeyCPtr _GetNodeKey() const override {return m_nodeKey;}
    Utf8String _GetLabel() const override {return m_label;}
    Utf8String _GetDescription() const override {return m_description;}
    Utf8String _GetExpandedImageId() const override {return m_imageId;}
    Utf8String _GetCollapsedImageId() const override {return m_imageId;}
    Utf8String _GetForeColor() const override {return m_foreColor;}
    Utf8String _GetBackColor() const override {return m_backColor;}
    Utf8String _GetFontStyle() const override {return m_fontStyle;}
    Utf8String _GetType() const override {return m_type;}
    bool _DeterminedChildren() const override {return m_determinedChildren;}
    bool _HasChildren() const override {BeAssert(m_determinedChildren); return m_hasChildren;}
    bool _IsSelectable() const override {return m_isSelectable;}
    bool _IsEditable() const override {return m_isEditable;}
    bool _IsChecked() const override {return m_isChecked;}
    bool _IsCheckboxVisible() const override {return m_isCheckboxVisible;}
    bool _IsCheckboxEnabled() const override {return m_isCheckboxEnabled;}
    bool _IsExpanded() const override {return m_isExpanded;}

    void _SetLabel(Utf8CP label) override {m_label = label;}
    void _SetType(Utf8CP type) override {m_type = type;}
    void _SetExpandedImageId(Utf8CP imageId) override {m_imageId = imageId;}
    void _SetCollapsedImageId(Utf8CP imageId) override {m_imageId = imageId;}
    void _SetDescription(Utf8CP description) override {m_description = description;}
    void _SetForeColor(Utf8CP color) override {m_foreColor = color;}
    void _SetBackColor(Utf8CP color) override {m_backColor = color;}
    void _SetFontStyle(Utf8CP style) override {m_fontStyle = style;}
    void _SetHasChildren(bool value) override {m_hasChildren = value; m_determinedChildren = true;}
    void _SetIsChecked(bool value) override {m_isChecked = value;}
    void _SetIsCheckboxVisible(bool value) override {m_isCheckboxVisible = value;}
    void _SetIsCheckboxEnabled(bool value) override {m_isCheckboxEnabled = value;}
    void _SetNodeId(uint64_t id) override {m_nodeId = id;}
    void _SetParentNodeId(uint64_t id) override {m_parentNodeId = id;}
    void _SetIsExpanded(bool value) override {m_isExpanded = value;}
    void _SetIsSelectable(bool value) override {m_isSelectable = value;}
    void _SetIsEditable(bool value) override {m_isEditable = value;}
    void _SetNodeKey(NavNodeKeyCR nodeKey) override {m_nodeKey = &nodeKey;}

public:
    ECPRESENTATION_EXPORT ~JsonNavNode();
    static JsonNavNodePtr Create() {return new JsonNavNode();}
    JsonNavNodePtr Clone() {return new JsonNavNode(*this);}

    ECPRESENTATION_EXPORT rapidjson::Document GetJson() const;
    ECPRESENTATION_EXPORT void InitFromJson(RapidJsonValueCR);

    void ResetHasChildren() {m_determinedChildren = false;}
    void SetImageId(Utf8CP imageId) {SetExpandedImageId(imageId);}
    void AddUsersExtendedData(Utf8CP key, ECValueCR value);
    void SetParentNode(NavNodeCR node) {SetParentNodeId(node.GetNodeId());}
    bool HasKey() const {return m_nodeKey.IsValid();}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE JsonNavNodesFactory : RefCounted<INavNodesFactory>
{
protected:
    virtual NavNodePtr _CreateECInstanceNode(Utf8StringCR connectionId, Utf8StringCR locale, bvector<ECInstanceKey> const& instanceKeys, Utf8CP label) const override {return CreateECInstanceNode(connectionId, locale, instanceKeys, label);}
    virtual NavNodePtr _CreateECClassGroupingNode(Utf8StringCR connectionId, Utf8StringCR locale, ECClassCR ecClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const override {return CreateECClassGroupingNode(connectionId, locale, ecClass, label, groupedInstanceKeys);}
    virtual NavNodePtr _CreateECRelationshipGroupingNode(Utf8StringCR connectionId, Utf8StringCR locale, ECRelationshipClassCR relationshipClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const override {return CreateECRelationshipGroupingNode(connectionId, locale, relationshipClass, label, groupedInstanceKeys);}
    virtual NavNodePtr _CreateECPropertyGroupingNode(Utf8StringCR connectionId, Utf8StringCR locale, ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR groupedInstanceKeys) const override {return CreateECPropertyGroupingNode(connectionId, locale, ecClass, ecProperty, label, imageId, groupingValue, isRangeGrouping, groupedInstanceKeys);}
    virtual NavNodePtr _CreateDisplayLabelGroupingNode(Utf8StringCR connectionId, Utf8StringCR locale, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const override {return CreateDisplayLabelGroupingNode(connectionId, locale, label, groupedInstanceKeys);}
    virtual NavNodePtr _CreateCustomNode(Utf8StringCR connectionId, Utf8StringCR locale, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const override {return CreateCustomNode(connectionId, locale, label, description, imageId, type);}

public:
    ECPRESENTATION_EXPORT void InitECInstanceNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, bvector<ECInstanceKey> const&, Utf8CP label) const;
    ECPRESENTATION_EXPORT void InitECInstanceNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, ECClassId, ECInstanceId, Utf8CP label) const;
    ECPRESENTATION_EXPORT void InitECInstanceNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, IECInstanceCR, Utf8CP label) const;
    ECPRESENTATION_EXPORT void InitECClassGroupingNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, ECClassCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT void InitECRelationshipGroupingNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, ECRelationshipClassCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT void InitECPropertyGroupingNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, ECClassCR, ECPropertyCR, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT void InitDisplayLabelGroupingNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT void InitCustomNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const;
    ECPRESENTATION_EXPORT void InitFromJson(JsonNavNodeR, IConnectionCR, RapidJsonValueCR) const;

    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECInstanceNode(Utf8StringCR, Utf8StringCR, bvector<ECClassInstanceKey> const&, Utf8CP label) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECInstanceNode(Utf8StringCR, Utf8StringCR, bvector<ECInstanceKey> const&, Utf8CP label) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECInstanceNode(Utf8StringCR, Utf8StringCR, ECClassId, ECInstanceId, Utf8CP label) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECInstanceNode(Utf8StringCR, Utf8StringCR, IECInstanceCR, Utf8CP label) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECClassGroupingNode(Utf8StringCR, Utf8StringCR, ECClassCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECRelationshipGroupingNode(Utf8StringCR, Utf8StringCR, ECRelationshipClassCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECPropertyGroupingNode(Utf8StringCR, Utf8StringCR, ECClassCR, ECPropertyCR, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateDisplayLabelGroupingNode(Utf8StringCR, Utf8StringCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateCustomNode(Utf8StringCR, Utf8StringCR, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateFromJson(IConnectionCR, RapidJsonValueCR) const;
};
typedef JsonNavNodesFactory const& JsonNavNodesFactoryCR;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE NavNodesHelper
{
private:
    NavNodesHelper() {}
public:
    static void AddRelatedInstanceInfo(NavNodeR node, Utf8CP serializedJson);
    static void SetSkippedInstanceKeys(NavNodeR node, Utf8CP serializedJson);
    ECPRESENTATION_EXPORT static bvector<JsonChange> GetChanges(JsonNavNode const& oldNode, JsonNavNode const& newNode);
    static bool IsGroupingNode(NavNodeCR);
    static bool IsCustomNode(NavNodeCR node);
    ECPRESENTATION_EXPORT static NavNodeKeyPtr CreateNodeKey(IConnectionCR, JsonNavNodeCR node, bvector<Utf8String> const& path, bool isFake);
    ECPRESENTATION_EXPORT static NavNodeKeyPtr CreateNodeKey(IConnectionCR, JsonNavNodeCR node, NavNodeKeyCP parentNodeKey);
    ECPRESENTATION_EXPORT static NavNodeKeyPtr CreateFakeNodeKey(IConnectionCR, JsonNavNodeCR node);
    static Utf8String NodeKeyHashPathToString(NavNodeKeyCR key);
    static bvector<Utf8String> NodeKeyHashPathFromString(Utf8CP str);
};


END_BENTLEY_ECPRESENTATION_NAMESPACE
