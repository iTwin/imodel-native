/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ExtendedData.h>
#include <ECPresentation/NavNode.h>
#include <ECPresentation/Content.h>
#include "../RulesEngineTypes.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define ITEM_EXTENDEDDATA_ConnectionId                          "ConnectionId"
#define ITEM_EXTENDEDDATA_RulesetId                             "RulesetId"
#define ITEM_EXTENDEDDATA_IsCustomized                          "IsNodeCustomized"
#define ITEM_EXTENDEDDATA_CheckboxBoundPropertyName             "CheckboxBoundPropertyName"
#define ITEM_EXTENDEDDATA_IsCheckboxBoundPropertyInversed       "IsCheckboxBoundPropertyInversed"
#define ITEM_EXTENDEDDATA_RelatedInstanceKeys                   "RelatedInstanceKeys"
#define ITEM_EXTENDEDDATA_RelatedInstanceKey_ECClassId          "ECClassId"
#define ITEM_EXTENDEDDATA_RelatedInstanceKey_ECInstanceId       "ECInstanceId"
#define ITEM_EXTENDEDDATA_RelatedInstanceKeys_Alias             "Alias"
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ItemExtendedData : RapidJsonAccessor
{
    struct RelatedInstanceKey
        {
        private:
            BeSQLite::EC::ECInstanceKey m_instanceKey;
            Utf8String m_alias;
        public:
            RelatedInstanceKey() {}
            RelatedInstanceKey(BeSQLite::EC::ECInstanceKey instanceKey, Utf8String alias) : m_instanceKey(instanceKey), m_alias(alias) {}
            static RelatedInstanceKey FromJson(RapidJsonValueCR);
            static bvector<RelatedInstanceKey> FromJsonArray(RapidJsonValueCR);
            BeSQLite::EC::ECInstanceKey const& GetInstanceKey() const { return m_instanceKey; }
            Utf8CP GetAlias() const { return m_alias.c_str(); }
            rapidjson::Document ToJson(rapidjson::Document::AllocatorType* = nullptr) const;
        };

    ItemExtendedData() : RapidJsonAccessor() {}
    ItemExtendedData(RapidJsonValueCR data) : RapidJsonAccessor(data) {}
    ItemExtendedData(RapidJsonValueR data, rapidjson::MemoryPoolAllocator<>& allocator) : RapidJsonAccessor(data, allocator) {}
    ItemExtendedData(ItemExtendedData const& other) : RapidJsonAccessor(other) {}
    ItemExtendedData(IRapidJsonExtendedDataHolder& item) : RapidJsonAccessor(item) {}
    ItemExtendedData(IRapidJsonExtendedDataHolder const& item) : RapidJsonAccessor(item) {}

    bool HasConnectionId() const {return GetJson().HasMember(ITEM_EXTENDEDDATA_ConnectionId);}
    Utf8CP GetConnectionId() const {return GetJson()[ITEM_EXTENDEDDATA_ConnectionId].GetString();}
    void SetConnectionId(Utf8StringCR connectionId) {AddMember(ITEM_EXTENDEDDATA_ConnectionId, rapidjson::Value(connectionId.c_str(), GetAllocator()));}

    bool HasRulesetId() const {return GetJson().HasMember(ITEM_EXTENDEDDATA_RulesetId);}
    Utf8CP GetRulesetId() const {return GetJson()[ITEM_EXTENDEDDATA_RulesetId].GetString();}
    void SetRulesetId(Utf8CP rulesetId) {AddMember(ITEM_EXTENDEDDATA_RulesetId, rapidjson::Value(rulesetId, GetAllocator()));}

    bool IsCustomized() const {return GetJson().HasMember(ITEM_EXTENDEDDATA_IsCustomized) ? GetJson()[ITEM_EXTENDEDDATA_IsCustomized].GetBool() : false;}
    void SetIsCustomized(bool value) {AddMember(ITEM_EXTENDEDDATA_IsCustomized, rapidjson::Value(value));}

    bool HasCheckboxBoundPropertyName() const {return GetJson().HasMember(ITEM_EXTENDEDDATA_CheckboxBoundPropertyName);}
    Utf8CP GetCheckboxBoundPropertyName() const {return GetJson()[ITEM_EXTENDEDDATA_CheckboxBoundPropertyName].GetString();}
    void SetCheckboxBoundPropertyName(Utf8CP value) {AddMember(ITEM_EXTENDEDDATA_CheckboxBoundPropertyName, rapidjson::Value(value, GetAllocator()));}

    bool IsCheckboxBoundPropertyInversed() const {return GetJson().HasMember(ITEM_EXTENDEDDATA_IsCheckboxBoundPropertyInversed) ? GetJson()[ITEM_EXTENDEDDATA_IsCheckboxBoundPropertyInversed].GetBool() : false;}
    void SetCheckboxBoundPropertyInversed(bool value) {AddMember(ITEM_EXTENDEDDATA_IsCheckboxBoundPropertyInversed, rapidjson::Value(value));}

    // Related instance keys list contains definitions (key + alias) of instances that are related
    // to the primary instance. The relationship is set up in the ruleset by using RelatedInstance
    // sub-specification in navigation specifications.
    ECPRESENTATION_EXPORT static bvector<RelatedInstanceKey> ParseRelatedInstanceKeys(RapidJsonValueCR);
    ECPRESENTATION_EXPORT static bvector<RelatedInstanceKey> ParseRelatedInstanceKeys(Utf8CP);
    ECPRESENTATION_EXPORT bvector<RelatedInstanceKey> GetRelatedInstanceKeys() const;
    ECPRESENTATION_EXPORT void SetRelatedInstanceKeys(rapidjson::Value&& json);
    ECPRESENTATION_EXPORT void SetRelatedInstanceKeys(Utf8CP serializedJson);
    ECPRESENTATION_EXPORT void SetRelatedInstanceKeys(bvector<RelatedInstanceKey> const& keys);
    ECPRESENTATION_EXPORT void SetRelatedInstanceKey(RelatedInstanceKey const& key);
    ECPRESENTATION_EXPORT void AddRelatedInstanceKeys(bvector<RelatedInstanceKey> const& keys);
};

#define CONTENTSETITEM_EXTENDEDDATA_ContractId                  "ContractId"
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentSetItemExtendedData : ItemExtendedData
{
    ContentSetItemExtendedData() : ItemExtendedData() {}
    ContentSetItemExtendedData(RapidJsonValueCR data) : ItemExtendedData(data) {}
    ContentSetItemExtendedData(RapidJsonValueR data, rapidjson::MemoryPoolAllocator<>& allocator) : ItemExtendedData(data, allocator) {}
    ContentSetItemExtendedData(ContentSetItemExtendedData const& other) : ItemExtendedData(other) {}
    ContentSetItemExtendedData(ContentSetItemR item) : ItemExtendedData(item) {}
    ContentSetItemExtendedData(ContentSetItemCR item) : ItemExtendedData(item) {}

    bool HasContractId() const {return GetJson().HasMember(CONTENTSETITEM_EXTENDEDDATA_ContractId);}
    uint64_t GetContractId() const {return GetJson()[CONTENTSETITEM_EXTENDEDDATA_ContractId].GetUint64();}
    void SetContractId(uint64_t id) {AddMember(CONTENTSETITEM_EXTENDEDDATA_ContractId, rapidjson::Value(id));}
};

#define NAVNODE_EXTENDEDDATA_RequestedSpecification             "RequestedSpecification"
#define NAVNODE_EXTENDEDDATA_VirtualParentIds                   "VirtualParentIds"
#define NAVNODE_EXTENDEDDATA_RelationshipDirection              "RelationshipDirection"
#define NAVNODE_EXTENDEDDATA_HasChildrenHint                    "HasChildrenHint"
#define NAVNODE_EXTENDEDDATA_HideIfOnlyOneChild                 "HideIfOnlyOneChild"
#define NAVNODE_EXTENDEDDATA_HideIfNoChildren                   "HideIfNoChildren"
#define NAVNODE_EXTENDEDDATA_HideExpression                     "HideExpression"
#define NAVNODE_EXTENDEDDATA_HideIfGroupingValueNotSpecified    "HideIfGroupingValueNotSpecified"
#define NAVNODE_EXTENDEDDATA_HideNodesInHierarchy               "HideNodesInHierarchy"
#define NAVNODE_EXTENDEDDATA_ECClassId                          "ECClassId"
#define NAVNODE_EXTENDEDDATA_IsECClassPolymorphic               "IsECClassPolymorphic"
#define NAVNODE_EXTENDEDDATA_ECInstanceKeys                     "ECInstanceKeys"
#define NAVNODE_EXTENDEDDATA_PropertyName                       "PropertyName"
#define NAVNODE_EXTENDEDDATA_PropertyValues                     "PropertyValues"
#define NAVNODE_EXTENDEDDATA_PropertyValueRangeIndexes          "PropertyValueRangeIndexes"
#define NAVNODE_EXTENDEDDATA_ChildrenArtifacts                  "ChildrenArtifacts"
#define NAVNODE_EXTENDEDDATA_InstanceKey_ECClassId              "c"
#define NAVNODE_EXTENDEDDATA_InstanceKey_ECInstanceId           "i"
#define NAVNODE_EXTENDEDDATA_IsLabelCustomized                  "IsLabelCustomized"
#define NAVNODE_EXTENDEDDATA_AllowedSimilarAncestors            "AllowedSimilarAncestors"
#define NAVNODE_EXTENDEDDATA_MergedNodeIds                      "MergedNodeIds"
#define NAVNODE_EXTENDEDDATA_IsNodeInitialized                  "IsNodeInitialized"
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavNodeExtendedData : ItemExtendedData
{
    NavNodeExtendedData() : ItemExtendedData() {}
    NavNodeExtendedData(RapidJsonValueCR data) : ItemExtendedData(data) {}
    NavNodeExtendedData(RapidJsonValueR data, rapidjson::MemoryPoolAllocator<>& allocator) : ItemExtendedData(data, allocator) {}
    NavNodeExtendedData(NavNodeExtendedData const& other) : ItemExtendedData(other) {}
    NavNodeExtendedData(NavNodeR node) : ItemExtendedData(node) {}
    NavNodeExtendedData(NavNodeCR node) : ItemExtendedData(node) {}

    bool IsLabelCustomized() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_IsLabelCustomized) ? GetJson()[NAVNODE_EXTENDEDDATA_IsLabelCustomized].GetBool() : false;}
    void SetIsLabelCustomized(bool value) {AddMember(NAVNODE_EXTENDEDDATA_IsLabelCustomized, rapidjson::Value(value));}

    bool GetRequestedSpecification() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_RequestedSpecification) ? GetJson()[NAVNODE_EXTENDEDDATA_RequestedSpecification].GetBool() : false;}
    void SetRequestedSpecification(bool value) {AddMember(NAVNODE_EXTENDEDDATA_RequestedSpecification, rapidjson::Value(value));}

    ECPRESENTATION_EXPORT bvector<BeGuid> GetVirtualParentIds() const;
    ECPRESENTATION_EXPORT void AddVirtualParentId(BeGuidCR id);
    ECPRESENTATION_EXPORT void SetVirtualParentIds(bvector<BeGuid> const& ids);

    ECPRESENTATION_EXPORT bvector<BeGuid> GetMergedNodeIds() const;
    ECPRESENTATION_EXPORT void AddMergedNodeId(BeGuidCR id);
    ECPRESENTATION_EXPORT void SetMergedNodeIds(bvector<BeGuid> const& ids);

    bool IsNodeInitialized() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_IsNodeInitialized) ? GetJson()[NAVNODE_EXTENDEDDATA_IsNodeInitialized].GetBool() : false;}
    void SetNodeInitialized(bool value) {AddMember(NAVNODE_EXTENDEDDATA_IsNodeInitialized, rapidjson::Value(value));}

    bool HasRelationshipDirection() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_RelationshipDirection);}
    ECRelatedInstanceDirection GetRelationshipDirection() const {return (ECRelatedInstanceDirection)GetJson()[NAVNODE_EXTENDEDDATA_RelationshipDirection].GetInt();}
    void SetRelationshipDirection(ECRelatedInstanceDirection direction) {AddMember(NAVNODE_EXTENDEDDATA_RelationshipDirection, rapidjson::Value((int)direction));}

    ChildrenHint GetChildrenHint() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_HasChildrenHint) ? (ChildrenHint)GetJson()[NAVNODE_EXTENDEDDATA_HasChildrenHint].GetInt() : ChildrenHint::Unknown;}
    void SetChildrenHint(ChildrenHint hint) {AddMember(NAVNODE_EXTENDEDDATA_HasChildrenHint, rapidjson::Value((int)hint));}

    bool HideIfOnlyOneChild() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_HideIfOnlyOneChild) ? GetJson()[NAVNODE_EXTENDEDDATA_HideIfOnlyOneChild].GetBool() : false;}
    void SetHideIfOnlyOneChild(bool value) {AddMember(NAVNODE_EXTENDEDDATA_HideIfOnlyOneChild, rapidjson::Value(value));}

    bool HideIfNoChildren() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_HideIfNoChildren) ? GetJson()[NAVNODE_EXTENDEDDATA_HideIfNoChildren].GetBool() : false;}
    void SetHideIfNoChildren(bool value) {AddMember(NAVNODE_EXTENDEDDATA_HideIfNoChildren, rapidjson::Value(value));}

    bool HasHideExpression() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_HideExpression);}
    Utf8CP GetHideExpression() const {return GetJson()[NAVNODE_EXTENDEDDATA_HideExpression].GetString();}
    void SetHideExpression(Utf8StringCR expr) {AddMember(NAVNODE_EXTENDEDDATA_HideExpression, rapidjson::Value(expr.c_str(), GetAllocator()));}

    bool HideIfGroupingValueNotSpecified() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_HideIfGroupingValueNotSpecified) ? GetJson()[NAVNODE_EXTENDEDDATA_HideIfGroupingValueNotSpecified].GetBool() : false;}
    void SetHideIfGroupingValueNotSpecified(bool value) {AddMember(NAVNODE_EXTENDEDDATA_HideIfGroupingValueNotSpecified, rapidjson::Value(value));}

    bool HideNodesInHierarchy() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_HideNodesInHierarchy) ? GetJson()[NAVNODE_EXTENDEDDATA_HideNodesInHierarchy].GetBool() : false;}
    void SetHideNodesInHierarchy(bool value) {AddMember(NAVNODE_EXTENDEDDATA_HideNodesInHierarchy, rapidjson::Value(value));}

    bool HasECClassId() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_ECClassId);}
    ECClassId GetECClassId() const {return ECClassId(GetJson()[NAVNODE_EXTENDEDDATA_ECClassId].GetUint64());}
    void SetECClassId(ECClassId const& id) {AddMember(NAVNODE_EXTENDEDDATA_ECClassId, rapidjson::Value(id.GetValue()));}

    bool IsECClassPolymorphic() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_IsECClassPolymorphic) ? GetJson()[NAVNODE_EXTENDEDDATA_IsECClassPolymorphic].GetBool() : false;}
    void SetIsECClassPolymorphic(bool value) {AddMember(NAVNODE_EXTENDEDDATA_IsECClassPolymorphic, rapidjson::Value(value));}

    bool HasPropertyName() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_PropertyName);}
    Utf8CP GetPropertyName() const {return GetJson()[NAVNODE_EXTENDEDDATA_PropertyName].GetString();}
    void SetPropertyName(Utf8CP name) {AddMember(NAVNODE_EXTENDEDDATA_PropertyName, rapidjson::Value(name, GetAllocator()));}

    bool HasPropertyValues() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_PropertyValues);}
    ECPRESENTATION_EXPORT RapidJsonValueCR GetPropertyValues() const;
    ECPRESENTATION_EXPORT void SetPropertyValues(RapidJsonValueCR value);

    bool HasPropertyValueRangeIndexes() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_PropertyValueRangeIndexes);}
    ECPRESENTATION_EXPORT bvector<int> GetPropertyValueRangeIndexes() const;
    ECPRESENTATION_EXPORT RapidJsonValueCR GetPropertyValueRangeIndexesJson() const;
    ECPRESENTATION_EXPORT void SetPropertyValueRangeIndexes(bvector<int> const& indexes);
    ECPRESENTATION_EXPORT void SetPropertyValueRangeIndexes(RapidJsonValueCR value);

    int GetAllowedSimilarAncestors() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_AllowedSimilarAncestors) ? GetJson()[NAVNODE_EXTENDEDDATA_AllowedSimilarAncestors].GetInt() : 0;}
    void SetAllowedSimilarAncestors(int value) {AddMember(NAVNODE_EXTENDEDDATA_AllowedSimilarAncestors, rapidjson::Value(value));}

    ECPRESENTATION_EXPORT bvector<NodeArtifacts> GetChildrenArtifacts() const;
    ECPRESENTATION_EXPORT bool SetChildrenArtifacts(bvector<NodeArtifacts> artifacts);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
