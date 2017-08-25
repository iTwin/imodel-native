/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/JsonNavNode.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/NavNode.h>
#include <ECPresentation/RulesDriven/Update.h>
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
#define NAVNODE_ExtendedData        "ExtendedData"

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
    BeSQLite::EC::ECDb const* m_ecdb;
    mutable RefCountedPtr<ECN::IECInstance const> m_instance;

private:
    void LoadECInstance() const;
    ECPRESENTATION_EXPORT void AddMember(Utf8CP name, rapidjson::Value& value);

protected:
    void SetECDb(BeSQLite::EC::ECDbCR ecdb) {m_ecdb = &ecdb;}
    void SetInstance(ECN::IECInstanceCR instance) {m_instance = &instance;}

protected:
    ECPRESENTATION_EXPORT uint64_t _GetNodeId() const override;
    ECPRESENTATION_EXPORT uint64_t _GetParentNodeId() const override;
    ECPRESENTATION_EXPORT Utf8String _GetLabel() const override;
    ECPRESENTATION_EXPORT Utf8String _GetDescription() const override;
    ECPRESENTATION_EXPORT virtual NavNodeKeyCPtr _CreateKey() const override;
    ECPRESENTATION_EXPORT RefCountedPtr<ECN::IECInstance const> _GetInstance() const override;
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
    ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const override;
    ECPRESENTATION_EXPORT RapidJsonValueR _GetExtendedData() const override;
    ECPRESENTATION_EXPORT rapidjson::MemoryPoolAllocator<>& _GetExtendedDataAllocator() const override;

public:
    ECPRESENTATION_EXPORT JsonNavNode();
    static JsonNavNodePtr Create() {return new JsonNavNode();}

    ECPRESENTATION_EXPORT RapidJsonValueCR GetJson() const;
    bool DeterminedChildren() const {return m_json.HasMember(NAVNODE_HasChildren);}
    
    ECPRESENTATION_EXPORT void SetLabel(Utf8CP label);
    ECPRESENTATION_EXPORT void SetImageId(Utf8CP imageId);
    ECPRESENTATION_EXPORT void SetType(Utf8CP type);
    ECPRESENTATION_EXPORT void SetExpandedImageId(Utf8CP imageId);
    ECPRESENTATION_EXPORT void SetCollapsedImageId(Utf8CP imageId);
    ECPRESENTATION_EXPORT void SetDescription(Utf8CP description);
    ECPRESENTATION_EXPORT void SetForeColor(Utf8CP color);
    ECPRESENTATION_EXPORT void SetBackColor(Utf8CP color);
    ECPRESENTATION_EXPORT void SetFontStyle(Utf8CP style);
    ECPRESENTATION_EXPORT void SetHasChildren(bool value);
    ECPRESENTATION_EXPORT void SetIsChecked(bool value);
    ECPRESENTATION_EXPORT void SetIsCheckboxVisible(bool value);
    ECPRESENTATION_EXPORT void SetIsCheckboxEnabled(bool value);
    ECPRESENTATION_EXPORT void SetInstanceId(uint64_t instanceId);
    ECPRESENTATION_EXPORT void SetNodeId(uint64_t);
    ECPRESENTATION_EXPORT void SetParentNodeId(uint64_t);
    ECPRESENTATION_EXPORT void SetIsExpanded(bool value);
    void SetParentNode(NavNodeCR node) {SetParentNodeId(node.GetNodeId());}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE JsonNavNodesFactory : INavNodesFactory
{
protected:
    NavNodePtr _CreateECInstanceNode(BeSQLite::EC::ECDbCR db, ECN::ECClassId classId, BeSQLite::EC::ECInstanceId instanceId, Utf8CP label) const {return CreateECInstanceNode(db, classId, instanceId, label);}
    NavNodePtr _CreateECInstanceNode(BeSQLite::BeGuidCR connectionId, ECN::IECInstanceCR instance, Utf8CP label) const {return CreateECInstanceNode(connectionId, instance, label);}
    NavNodePtr _CreateECClassGroupingNode(BeSQLite::BeGuidCR connectionId, ECN::ECClassCR ecClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const {return CreateECClassGroupingNode(connectionId, ecClass, label, groupedInstanceKeys);}
    NavNodePtr _CreateECRelationshipGroupingNode(BeSQLite::BeGuidCR connectionId, ECN::ECRelationshipClassCR relationshipClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const {return CreateECRelationshipGroupingNode(connectionId, relationshipClass, label, groupedInstanceKeys);}
    NavNodePtr _CreateECPropertyGroupingNode(BeSQLite::BeGuidCR connectionId, ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR groupedInstanceKeys) const {return CreateECPropertyGroupingNode(connectionId, ecClass, ecProperty, label, imageId, groupingValue, isRangeGrouping, groupedInstanceKeys);}
    NavNodePtr _CreateDisplayLabelGroupingNode(BeSQLite::BeGuidCR connectionId, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const {return CreateDisplayLabelGroupingNode(connectionId, label, groupedInstanceKeys);}
    NavNodePtr _CreateCustomNode(BeSQLite::BeGuidCR connectionId, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const {return CreateCustomNode(connectionId, label, description, imageId, type);}

public:
    ECPRESENTATION_EXPORT void InitECInstanceNode(JsonNavNodeR, BeSQLite::EC::ECDbCR, ECN::ECClassId, BeSQLite::EC::ECInstanceId, Utf8CP label) const;
    ECPRESENTATION_EXPORT void InitECInstanceNode(JsonNavNodeR, BeSQLite::BeGuidCR, ECN::IECInstanceCR, Utf8CP label) const;
    ECPRESENTATION_EXPORT void InitECClassGroupingNode(JsonNavNodeR, BeSQLite::BeGuidCR, ECN::ECClassCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT void InitECRelationshipGroupingNode(JsonNavNodeR, BeSQLite::BeGuidCR, ECN::ECRelationshipClassCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT void InitECPropertyGroupingNode(JsonNavNodeR, BeSQLite::BeGuidCR, ECN::ECClassCR, ECN::ECPropertyCR, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT void InitDisplayLabelGroupingNode(JsonNavNodeR, BeSQLite::BeGuidCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT void InitCustomNode(JsonNavNodeR, BeSQLite::BeGuidCR, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const;
    ECPRESENTATION_EXPORT void InitFromJson(JsonNavNodeR, BeSQLite::EC::ECDbCR, rapidjson::Document&&) const;

    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECInstanceNode(BeSQLite::EC::ECDbCR, ECN::ECClassId, BeSQLite::EC::ECInstanceId, Utf8CP label) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECInstanceNode(BeSQLite::BeGuidCR, ECN::IECInstanceCR, Utf8CP label) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECClassGroupingNode(BeSQLite::BeGuidCR, ECN::ECClassCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECRelationshipGroupingNode(BeSQLite::BeGuidCR, ECN::ECRelationshipClassCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateECPropertyGroupingNode(BeSQLite::BeGuidCR, ECN::ECClassCR, ECN::ECPropertyCR, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateDisplayLabelGroupingNode(BeSQLite::BeGuidCR, Utf8CP label, GroupedInstanceKeysListCR) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateCustomNode(BeSQLite::BeGuidCR, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const;
    ECPRESENTATION_EXPORT JsonNavNodePtr CreateFromJson(BeSQLite::EC::ECDbCR, rapidjson::Document&&) const;
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
};


END_BENTLEY_ECPRESENTATION_NAMESPACE
