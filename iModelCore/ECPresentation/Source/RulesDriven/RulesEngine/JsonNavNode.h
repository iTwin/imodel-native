/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
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
    rapidjson::MemoryPoolAllocator<> m_allocator;
    mutable rapidjson::Document m_json;
    NavNodeKeyCPtr m_nodeKey;

private:
    ECPRESENTATION_EXPORT void AddMember(Utf8CP name, rapidjson::Value& value);

protected:
    ECPRESENTATION_EXPORT uint64_t _GetInstanceId() const override;
    ECPRESENTATION_EXPORT uint64_t _GetNodeId() const override;
    ECPRESENTATION_EXPORT uint64_t _GetParentNodeId() const override;
    ECPRESENTATION_EXPORT Utf8String _GetLabel() const override;
    ECPRESENTATION_EXPORT Utf8String _GetDescription() const override;
    ECPRESENTATION_EXPORT NavNodeKeyCPtr _GetNodeKey() const override;
    ECPRESENTATION_EXPORT Utf8String _GetExpandedImageId() const override;
    ECPRESENTATION_EXPORT Utf8String _GetCollapsedImageId() const override;
    ECPRESENTATION_EXPORT Utf8String _GetForeColor() const override;
    ECPRESENTATION_EXPORT Utf8String _GetBackColor() const override;
    ECPRESENTATION_EXPORT Utf8String _GetFontStyle() const override;
    ECPRESENTATION_EXPORT Utf8String _GetType() const override;
    ECPRESENTATION_EXPORT bool _HasChildren() const override;
    ECPRESENTATION_EXPORT bool _IsSelectable() const override;
    ECPRESENTATION_EXPORT bool _IsEditable() const override;
    ECPRESENTATION_EXPORT bool _IsChecked() const override;
    ECPRESENTATION_EXPORT bool _IsCheckboxVisible() const override;
    ECPRESENTATION_EXPORT bool _IsCheckboxEnabled() const override;
    ECPRESENTATION_EXPORT bool _IsExpanded() const override;
    ECPRESENTATION_EXPORT rapidjson::Value const* _GetUsersExtendedData() const override;

    ECPRESENTATION_EXPORT void _SetInstanceId(uint64_t instanceId) override;
    ECPRESENTATION_EXPORT void _SetLabel(Utf8CP label) override;
    ECPRESENTATION_EXPORT void _SetType(Utf8CP type) override;
    ECPRESENTATION_EXPORT void _SetExpandedImageId(Utf8CP imageId) override;
    ECPRESENTATION_EXPORT void _SetCollapsedImageId(Utf8CP imageId) override;
    ECPRESENTATION_EXPORT void _SetDescription(Utf8CP description) override;
    ECPRESENTATION_EXPORT void _SetForeColor(Utf8CP color) override;
    ECPRESENTATION_EXPORT void _SetBackColor(Utf8CP color) override;
    ECPRESENTATION_EXPORT void _SetFontStyle(Utf8CP style) override;
    ECPRESENTATION_EXPORT void _SetHasChildren(bool value) override;
    ECPRESENTATION_EXPORT void _SetIsChecked(bool value) override;
    ECPRESENTATION_EXPORT void _SetIsCheckboxVisible(bool value) override;
    ECPRESENTATION_EXPORT void _SetIsCheckboxEnabled(bool value) override;
    ECPRESENTATION_EXPORT void _SetNodeId(uint64_t) override;
    ECPRESENTATION_EXPORT void _SetParentNodeId(uint64_t) override;
    ECPRESENTATION_EXPORT void _SetIsExpanded(bool value) override;
    ECPRESENTATION_EXPORT void _SetIsSelectable(bool value) override;
    ECPRESENTATION_EXPORT void _SetIsEditable(bool value) override;
    ECPRESENTATION_EXPORT void _SetNodeKey(NavNodeKeyCR nodeKey) override {m_nodeKey = &nodeKey;}

    ECPRESENTATION_EXPORT NavNodePtr _Clone() const override;

    ECPRESENTATION_EXPORT RapidJsonValueR _GetExtendedData() const override;
    ECPRESENTATION_EXPORT rapidjson::MemoryPoolAllocator<>& _GetExtendedDataAllocator() const override;

public:
    ECPRESENTATION_EXPORT JsonNavNode();
    static JsonNavNodePtr Create() {return new JsonNavNode();}

    ECPRESENTATION_EXPORT RapidJsonValueCR GetJson() const;
    bool DeterminedChildren() const {return m_json.HasMember(NAVNODE_HasChildren);}
    ECPRESENTATION_EXPORT void SetImageId(Utf8CP imageId);
    void AddUsersExtendedData(Utf8CP key, ECValueCR value);
    void SetParentNode(NavNodeCR node) {SetParentNodeId(node.GetNodeId());}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE JsonNavNodesFactory : RefCounted<INavNodesFactory>
{
protected:
    virtual NavNodePtr _CreateECInstanceNode(IConnectionCR db, Utf8StringCR locale, ECClassId classId, ECInstanceId instanceId, Utf8CP label) const override {return CreateECInstanceNode(db, locale, classId, instanceId, label);}
    virtual NavNodePtr _CreateECInstanceNode(Utf8StringCR connectionId, Utf8StringCR locale, IECInstanceCR instance, Utf8CP label) const override {return CreateECInstanceNode(connectionId, locale, instance, label);}
    virtual NavNodePtr _CreateECClassGroupingNode(Utf8StringCR connectionId, Utf8StringCR locale, ECClassCR ecClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const override {return CreateECClassGroupingNode(connectionId, locale, ecClass, label, groupedInstanceKeys);}
    virtual NavNodePtr _CreateECRelationshipGroupingNode(Utf8StringCR connectionId, Utf8StringCR locale, ECRelationshipClassCR relationshipClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const override {return CreateECRelationshipGroupingNode(connectionId, locale, relationshipClass, label, groupedInstanceKeys);}
    virtual NavNodePtr _CreateECPropertyGroupingNode(Utf8StringCR connectionId, Utf8StringCR locale, ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR groupedInstanceKeys) const override {return CreateECPropertyGroupingNode(connectionId, locale, ecClass, ecProperty, label, imageId, groupingValue, isRangeGrouping, groupedInstanceKeys);}
    virtual NavNodePtr _CreateDisplayLabelGroupingNode(Utf8StringCR connectionId, Utf8StringCR locale, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const override {return CreateDisplayLabelGroupingNode(connectionId, locale, label, groupedInstanceKeys);}
    virtual NavNodePtr _CreateCustomNode(Utf8StringCR connectionId, Utf8StringCR locale, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const override {return CreateCustomNode(connectionId, locale, label, description, imageId, type);}

public:
    ECPRESENTATION_EXPORT void InitECInstanceNode(JsonNavNodeR, IConnectionCR, Utf8StringCR, ECClassId, ECInstanceId, Utf8CP label) const;
    ECPRESENTATION_EXPORT void InitECInstanceNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, IECInstanceCR, Utf8CP label) const;
    ECPRESENTATION_EXPORT void InitECClassGroupingNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, ECClassCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT void InitECRelationshipGroupingNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, ECRelationshipClassCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT void InitECPropertyGroupingNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, ECClassCR, ECPropertyCR, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT void InitDisplayLabelGroupingNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT void InitCustomNode(JsonNavNodeR, Utf8StringCR, Utf8StringCR, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const;
    ECPRESENTATION_EXPORT void InitFromJson(JsonNavNodeR, IConnectionCR, rapidjson::Document&&) const;

    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECInstanceNode(IConnectionCR, Utf8StringCR, ECClassId, ECInstanceId, Utf8CP label) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECInstanceNode(Utf8StringCR, Utf8StringCR, IECInstanceCR, Utf8CP label) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECClassGroupingNode(Utf8StringCR, Utf8StringCR, ECClassCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECRelationshipGroupingNode(Utf8StringCR, Utf8StringCR, ECRelationshipClassCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECPropertyGroupingNode(Utf8StringCR, Utf8StringCR, ECClassCR, ECPropertyCR, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateDisplayLabelGroupingNode(Utf8StringCR, Utf8StringCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateCustomNode(Utf8StringCR, Utf8StringCR, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateFromJson(IConnectionCR, rapidjson::Document&&) const;
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
    ECPRESENTATION_EXPORT static void SwapData(JsonNavNode& lhs, JsonNavNode& rhs);
    static bool IsGroupingNode(NavNodeCR);
    static bool IsCustomNode(NavNodeCR node);
    ECPRESENTATION_EXPORT static NavNodeKeyPtr CreateNodeKey(IConnectionCR, JsonNavNodeCR, bvector<Utf8String> const& path);
    ECPRESENTATION_EXPORT static NavNodeKeyPtr CreateNodeKey(IConnectionCR, JsonNavNodeCR, Utf8CP pathFromRootJsonString);
};


END_BENTLEY_ECPRESENTATION_NAMESPACE
