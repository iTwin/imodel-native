/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ExtendedData.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ExtendedData.h>
#include <ECPresentation/NavNode.h>
#include <ECPresentation/Content.h>
#include "JsonNavNode.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define ITEM_EXTENDEDDATA_ConnectionId                          "ConnectionId"
#define ITEM_EXTENDEDDATA_RulesetId                             "RulesetId"
#define ITEM_EXTENDEDDATA_IsCustomized                          "IsNodeCustomized"
#define ITEM_EXTENDEDDATA_CheckboxBoundPropertyName             "CheckboxBoundPropertyName"
#define ITEM_EXTENDEDDATA_IsCheckboxBoundPropertyInversed       "IsCheckboxBoundPropertyInversed"
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2015
+===============+===============+===============+===============+===============+======*/
struct ItemExtendedData : RapidJsonAccessor
{
    ItemExtendedData() : RapidJsonAccessor() {}
    ItemExtendedData(RapidJsonValueCR data) : RapidJsonAccessor(data) {}
    ItemExtendedData(RapidJsonValueR data, rapidjson::MemoryPoolAllocator<>& allocator) : RapidJsonAccessor(data, allocator) {}
    ItemExtendedData(ItemExtendedData const& other) : RapidJsonAccessor(other) {}
    ItemExtendedData(IRapidJsonExtendedDataHolder& item) : RapidJsonAccessor(item) {}
    ItemExtendedData(IRapidJsonExtendedDataHolder const& item) : RapidJsonAccessor(item) {}

    bool HasConnectionId() const {return GetJson().HasMember(ITEM_EXTENDEDDATA_ConnectionId);}
    BeSQLite::BeGuid GetConnectionId() const {BeSQLite::BeGuid guid; guid.FromString(GetJson()[ITEM_EXTENDEDDATA_ConnectionId].GetString()); return guid;}
    void SetConnectionId(BeSQLite::BeGuidCR connectionId) {AddMember(ITEM_EXTENDEDDATA_ConnectionId, rapidjson::Value(connectionId.ToString().c_str(), GetAllocator()));}

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
};

#define CONTENTSETITEM_EXTENDEDDATA_ContractId                  "ContractId"
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2015
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

#define NAVNODE_EXTENDEDDATA_SpecificationId                    "SpecificationId"
#define NAVNODE_EXTENDEDDATA_RequestedSpecification             "RequestedSpecification"
#define NAVNODE_EXTENDEDDATA_VirtualParentId                    "VirtualParentId"
#define NAVNODE_EXTENDEDDATA_ParentECClassId                    "ParentECClassId"
#define NAVNODE_EXTENDEDDATA_RelationshipDirection              "RelationshipDirection"
#define NAVNODE_EXTENDEDDATA_AlwaysReturnsChildren              "AlwaysReturnsChildren"
#define NAVNODE_EXTENDEDDATA_HideIfOnlyOneChild                 "HideIfOnlyOneChild"
#define NAVNODE_EXTENDEDDATA_HideIfNoChildren                   "HideIfNoChildren"
#define NAVNODE_EXTENDEDDATA_HideIfGroupingValueNotSpecified    "HideIfGroupingValueNotSpecified"
#define NAVNODE_EXTENDEDDATA_HideNodesInHierarchy               "HideNodesInHierarchy"
#define NAVNODE_EXTENDEDDATA_ECClassId                          "ECClassId"
#define NAVNODE_EXTENDEDDATA_GroupingType                       "GroupingType"
#define NAVNODE_EXTENDEDDATA_PropertyName                       "PropertyName"
#define NAVNODE_EXTENDEDDATA_PropertyValue                      "PropertyValue"
#define NAVNODE_EXTENDEDDATA_PropertyValueRangeIndex            "PropertyValueRangeIndex"
#define NAVNODE_EXTENDEDDATA_GroupedInstanceKeys                "GroupedInstanceKeys"
#define NAVNODE_EXTENDEDDATA_RelatedInstanceKeys                "RelatedInstanceKeys"
#define NAVNODE_EXTENDEDDATA_InstanceKey_ECClassId              "ECClassId"
#define NAVNODE_EXTENDEDDATA_InstanceKey_ECInstanceId           "ECInstanceId"
#define NAVNODE_EXTENDEDDATA_RelatedInstanceKeys_Alias          "Alias"
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2015
+===============+===============+===============+===============+===============+======*/
struct NavNodeExtendedData : ItemExtendedData
{
    struct RelatedInstanceKey
    {
    private:
        BeSQLite::EC::ECInstanceKey m_instanceKey;
        Utf8CP m_alias;
    public:
        RelatedInstanceKey() {}
        RelatedInstanceKey(BeSQLite::EC::ECInstanceKey instanceKey, Utf8CP alias) : m_instanceKey(instanceKey), m_alias(alias) {}
        BeSQLite::EC::ECInstanceKey const& GetInstanceKey() const {return m_instanceKey;}
        Utf8CP GetAlias() const {return m_alias;}
    };

    NavNodeExtendedData() : ItemExtendedData() {}
    NavNodeExtendedData(RapidJsonValueCR data) : ItemExtendedData(data) {}
    NavNodeExtendedData(RapidJsonValueR data, rapidjson::MemoryPoolAllocator<>& allocator) : ItemExtendedData(data, allocator) {}
    NavNodeExtendedData(NavNodeExtendedData const& other) : ItemExtendedData(other) {}
    NavNodeExtendedData(NavNodeR node) : ItemExtendedData(node) {}
    NavNodeExtendedData(NavNodeCR node) : ItemExtendedData(node) {}
    
    bool HasSpecificationId() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_SpecificationId);}
    int GetSpecificationId() const {return GetJson()[NAVNODE_EXTENDEDDATA_SpecificationId].GetInt();}
    void SetSpecificationId(int specificationId) {AddMember(NAVNODE_EXTENDEDDATA_SpecificationId, rapidjson::Value(specificationId));}
    
    bool GetRequestedSpecification() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_RequestedSpecification) ? GetJson()[NAVNODE_EXTENDEDDATA_RequestedSpecification].GetBool() : false;}
    void SetRequestedSpecification(bool value) {AddMember(NAVNODE_EXTENDEDDATA_RequestedSpecification, rapidjson::Value(value));}
    
    bool HasVirtualParentId() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_VirtualParentId);}
    uint64_t GetVirtualParentId() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_VirtualParentId) ? GetJson()[NAVNODE_EXTENDEDDATA_VirtualParentId].GetUint64() : 0;}
    void SetVirtualParentId(uint64_t id) {AddMember(NAVNODE_EXTENDEDDATA_VirtualParentId, rapidjson::Value(id));}

    bool HasParentECClassId() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_ParentECClassId);}
    ECClassId GetParentECClassId() const {return ECClassId(GetJson()[NAVNODE_EXTENDEDDATA_ParentECClassId].GetUint64());}
    void SetParentECClassId(ECClassId const& id) {AddMember(NAVNODE_EXTENDEDDATA_ParentECClassId, rapidjson::Value(id.GetValue()));}
    
    bool HasRelationshipDirection() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_RelationshipDirection);}
    ECRelatedInstanceDirection GetRelationshipDirection() const {return (ECRelatedInstanceDirection)GetJson()[NAVNODE_EXTENDEDDATA_RelationshipDirection].GetInt();}
    void SetRelationshipDirection(ECRelatedInstanceDirection direction) {AddMember(NAVNODE_EXTENDEDDATA_RelationshipDirection, rapidjson::Value((int)direction));}
    
    bool GetAlwaysReturnsChildren() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_AlwaysReturnsChildren) ? GetJson()[NAVNODE_EXTENDEDDATA_AlwaysReturnsChildren].GetBool() : false;}
    void SetAlwaysReturnsChildren(bool value) {AddMember(NAVNODE_EXTENDEDDATA_AlwaysReturnsChildren, rapidjson::Value(value));}
    
    bool HideIfOnlyOneChild() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_HideIfOnlyOneChild) ? GetJson()[NAVNODE_EXTENDEDDATA_HideIfOnlyOneChild].GetBool() : false;}
    void SetHideIfOnlyOneChild(bool value) {AddMember(NAVNODE_EXTENDEDDATA_HideIfOnlyOneChild, rapidjson::Value(value));}
    
    bool HideIfNoChildren() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_HideIfNoChildren) ? GetJson()[NAVNODE_EXTENDEDDATA_HideIfNoChildren].GetBool() : false;}
    void SetHideIfNoChildren(bool value) {AddMember(NAVNODE_EXTENDEDDATA_HideIfNoChildren, rapidjson::Value(value));}
    
    bool HideIfGroupingValueNotSpecified() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_HideIfGroupingValueNotSpecified) ? GetJson()[NAVNODE_EXTENDEDDATA_HideIfGroupingValueNotSpecified].GetBool() : false;}
    void SetHideIfGroupingValueNotSpecified(bool value) {AddMember(NAVNODE_EXTENDEDDATA_HideIfGroupingValueNotSpecified, rapidjson::Value(value));}
        
    bool HideNodesInHierarchy() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_HideNodesInHierarchy) ? GetJson()[NAVNODE_EXTENDEDDATA_HideNodesInHierarchy].GetBool() : false;}
    void SetHideNodesInHierarchy(bool value) {AddMember(NAVNODE_EXTENDEDDATA_HideNodesInHierarchy, rapidjson::Value(value));}
    
    bool HasECClassId() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_ECClassId);}
    ECClassId GetECClassId() const {return ECClassId(GetJson()[NAVNODE_EXTENDEDDATA_ECClassId].GetUint64());}
    void SetECClassId(ECClassId const& id) {AddMember(NAVNODE_EXTENDEDDATA_ECClassId, rapidjson::Value(id.GetValue()));}

    bool HasGroupingType() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_GroupingType);}
    int GetGroupingType() const {return HasGroupingType() ? GetJson()[NAVNODE_EXTENDEDDATA_GroupingType].GetInt() : -1;}
    void SetGroupingType(int groupingType) {AddMember(NAVNODE_EXTENDEDDATA_GroupingType, rapidjson::Value(groupingType));}

    bool HasPropertyName() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_PropertyName);}
    Utf8CP GetPropertyName() const {return GetJson()[NAVNODE_EXTENDEDDATA_PropertyName].GetString();}
    void SetPropertyName(Utf8CP name) {AddMember(NAVNODE_EXTENDEDDATA_PropertyName, rapidjson::Value(name, GetAllocator()));}
    
    bool HasPropertyValue() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_PropertyValue);}
    rapidjson::Value const* GetPropertyValue() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_PropertyValue) ? &GetJson()[NAVNODE_EXTENDEDDATA_PropertyValue] : nullptr;}
    void SetPropertyValue(RapidJsonValueCR value) {AddMember(NAVNODE_EXTENDEDDATA_PropertyValue, rapidjson::Value(value, GetAllocator()));}
    
    bool HasPropertyValueRangeIndex() const {return GetJson().HasMember(NAVNODE_EXTENDEDDATA_PropertyValueRangeIndex);}
    int GetPropertyValueRangeIndex() const {return HasPropertyValueRangeIndex() ? GetJson()[NAVNODE_EXTENDEDDATA_PropertyValueRangeIndex].GetInt() : -1;}
    void SetPropertyValueRangeIndex(int index) {AddMember(NAVNODE_EXTENDEDDATA_PropertyValueRangeIndex, rapidjson::Value(index));}
    
    // Grouped instance keys contains ids of instances which are grouped by the node. For instance nodes there's
    // only one grouped instance key (of the instance itself); for grouping nodes the list contains keys of all
    // grouped instances; for custom nodes the list is empty.
    ECPRESENTATION_EXPORT BeSQLite::IdSet<BeSQLite::EC::ECInstanceId> GetGroupedInstanceIds() const;
    ECPRESENTATION_EXPORT bvector<BeSQLite::EC::ECInstanceKey> GetGroupedInstanceKeys() const;
    ECPRESENTATION_EXPORT void SetGroupedInstanceKeys(bvector<BeSQLite::EC::ECInstanceKey> const& keys);
    ECPRESENTATION_EXPORT void SetGroupedInstanceKey(BeSQLite::EC::ECInstanceKey const& key);
    
    // Related instance keys list contains definitions (key + alias) of instances that are related
    // to the primary instance. The relationship is set up in the ruleset by using RelatedInstance 
    // sub-specification in navigation specifications.
    ECPRESENTATION_EXPORT bvector<RelatedInstanceKey> GetRelatedInstanceKeys() const;
    ECPRESENTATION_EXPORT void SetRelatedInstanceKeys(rapidjson::Value&& json);
    ECPRESENTATION_EXPORT void SetRelatedInstanceKeys(bvector<RelatedInstanceKey> const& keys);
    ECPRESENTATION_EXPORT void SetRelatedInstanceKey(RelatedInstanceKey const& key);

    // Skipped instance keys contains keys of ECInstances which were skipped using SkipRelatedLevel
    // specification attribute.
    ECPRESENTATION_EXPORT bvector<BeSQLite::EC::ECInstanceKey> GetSkippedInstanceKeys() const;
    ECPRESENTATION_EXPORT void SetSkippedInstanceKeys(rapidjson::Value&& json);
    ECPRESENTATION_EXPORT void SetSkippedInstanceKeys(bvector<BeSQLite::EC::ECInstanceKey> const& keys);
    ECPRESENTATION_EXPORT void SetSkippedInstanceKey(BeSQLite::EC::ECInstanceKeyCR key);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
