/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/RulesetVariables.h>
#include "../RulesEngineTypes.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* Identifies a combined level of hierarchy which may consist of multiple hierarchy levels.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CombinedHierarchyLevelIdentifier
{
private:
    Utf8String m_connectionId;
    Utf8String m_rulesetId;
    BeGuid m_physicalParentNodeId;
    BeGuid m_removalId;

public:
    CombinedHierarchyLevelIdentifier() {}
    CombinedHierarchyLevelIdentifier(Utf8String connectionId, Utf8String rulesetId, BeGuid physicalParentNodeId)
        : m_connectionId(connectionId), m_rulesetId(rulesetId), m_physicalParentNodeId(physicalParentNodeId)
        {}
    bool operator<(CombinedHierarchyLevelIdentifier const& other) const
        {
        NUMERIC_LESS_COMPARE(m_physicalParentNodeId, other.m_physicalParentNodeId);
        NUMERIC_LESS_COMPARE(m_removalId, other.m_removalId);
        STR_LESS_COMPARE(m_rulesetId.c_str(), other.m_rulesetId.c_str());
        STR_LESS_COMPARE(m_connectionId.c_str(), other.m_connectionId.c_str());
        return false;
        }
    bool operator==(CombinedHierarchyLevelIdentifier const& other) const
        {
        return m_connectionId == other.m_connectionId
            && m_rulesetId == other.m_rulesetId
            && m_physicalParentNodeId == other.m_physicalParentNodeId
            && m_removalId == other.m_removalId;
        }
    Utf8StringCR GetConnectionId() const {return m_connectionId;}
    Utf8StringCR GetRulesetId() const {return m_rulesetId;}
    BeGuidCR GetPhysicalParentNodeId() const {return m_physicalParentNodeId;}
    void SetPhysicalParentNodeId(BeGuid id) {m_physicalParentNodeId = id;}
    BeGuidCR GetRemovalId() const {return m_removalId;}
    void SetRemovalId(BeGuid id) {m_removalId = id;}
};

/*=================================================================================**//**
* Identifies one level of hierarchy which may consist of multiple data sources
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchyLevelIdentifier
{
private:
    BeGuid m_id;
    CombinedHierarchyLevelIdentifier m_combined;
    BeGuid m_virtualParentNodeId;
public:
    HierarchyLevelIdentifier() {}
    HierarchyLevelIdentifier(CombinedHierarchyLevelIdentifier combined, BeGuid virtualParentNodeId)
        : m_combined(combined), m_virtualParentNodeId(virtualParentNodeId)
        {}
    HierarchyLevelIdentifier(Utf8String connectionId, Utf8String rulesetId, BeGuid physicalParentNodeId, BeGuid virtualParentNodeId)
        : m_combined(connectionId, rulesetId, physicalParentNodeId), m_virtualParentNodeId(virtualParentNodeId)
        {}
    HierarchyLevelIdentifier(BeGuid id, Utf8String connectionId, Utf8String rulesetId, BeGuid physicalParentNodeId, BeGuid virtualParentNodeId)
        : m_id(id), m_combined(connectionId, rulesetId, physicalParentNodeId), m_virtualParentNodeId(virtualParentNodeId)
        {}

    bool operator<(HierarchyLevelIdentifier const& other) const
        {
        NUMERIC_LESS_COMPARE(m_id, other.m_id);
        NUMERIC_LESS_COMPARE(m_virtualParentNodeId, other.m_virtualParentNodeId);
        NUMERIC_LESS_COMPARE(m_combined, other.m_combined);
        return false;
        }
    bool operator==(HierarchyLevelIdentifier const& other) const
        {
        return m_id == other.m_id
            && m_virtualParentNodeId == other.m_virtualParentNodeId
            && m_combined == other.m_combined;
        }

    bool IsValid() const {return m_id.IsValid();}
    void Invalidate() {m_id.Invalidate();}

    BeGuidCR GetId() const {return m_id;}
    void SetId(BeGuid id) {m_id = id;}
    BeGuidCR GetVirtualParentNodeId() const {return m_virtualParentNodeId;}
    void SetVirtualParentNodeId(BeGuid id) {m_virtualParentNodeId = id;}

    CombinedHierarchyLevelIdentifier& GetCombined() {return m_combined;}
    CombinedHierarchyLevelIdentifier const& GetCombined() const {return m_combined;}
    Utf8StringCR GetConnectionId() const {return m_combined.GetConnectionId();}
    Utf8StringCR GetRulesetId() const {return m_combined.GetRulesetId();}
    BeGuidCR GetRemovalId() const {return m_combined.GetRemovalId();}
    BeGuidCR GetPhysicalParentNodeId() const {return m_combined.GetPhysicalParentNodeId();}
};

/*=================================================================================**//**
* Identifies a single data source
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DataSourceIdentifier
{
private:
    BeGuid m_id;
    BeGuid m_hierarchyLevelId;
    Utf8String m_instanceFilter;
    bvector<uint64_t> m_index;
public:
    DataSourceIdentifier() : m_id(), m_hierarchyLevelId() {}
    DataSourceIdentifier(BeGuid hierarchyLevelId, bvector<uint64_t> index, Utf8String instanceFilter)
        : m_id(), m_hierarchyLevelId(hierarchyLevelId), m_index(index), m_instanceFilter(instanceFilter)
        {}
    DataSourceIdentifier(BeGuid id, BeGuid hierarchyLevelId, bvector<uint64_t> index, Utf8String instanceFilter)
        : m_id(id), m_hierarchyLevelId(hierarchyLevelId), m_index(index), m_instanceFilter(instanceFilter)
        {}
    bool operator<(DataSourceIdentifier const& other) const
        {
        NUMERIC_LESS_COMPARE(m_id, other.m_id);
        NUMERIC_LESS_COMPARE(m_hierarchyLevelId, other.m_hierarchyLevelId);
        VECTOR_LESS_COMPARE(m_index, other.m_index);
        STR_LESS_COMPARE(m_instanceFilter.c_str(), other.m_instanceFilter.c_str());
        return false;
        }
    bool operator==(DataSourceIdentifier const& other) const
        {
        return Equals(other);
        }
    bool operator!=(DataSourceIdentifier const& other) const
        {
        return !Equals(other);
        }
    bool Equals(DataSourceIdentifier const& other) const
        {
        return m_id == other.m_id
            && m_hierarchyLevelId == other.m_hierarchyLevelId
            && m_index == other.m_index
            && m_instanceFilter == other.m_instanceFilter;
        }
    bool IsValid() const { return m_id.IsValid(); }
    void Invalidate() {m_id.Invalidate();}
    BeGuidCR GetId() const {return m_id;}
    void SetId(BeGuid id) {m_id = id;}
    BeGuidCR GetHierarchyLevelId() const {return m_hierarchyLevelId;}
    bvector<uint64_t> const& GetIndex() const {return m_index;}
    Utf8StringCR GetInstanceFilter() const {return m_instanceFilter;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DataSourceFilter
{
    struct RelatedInstanceInfo
        {
        bvector<ECClassId> m_relationshipClassIds;
        bvector<ECInstanceKey> m_instanceKeys;
        RequiredRelationDirection m_direction;
        RelatedInstanceInfo() {}
        RelatedInstanceInfo(bvector<ECClassId> relationshipClassIds, RequiredRelationDirection direction, bvector<ECInstanceKey> instanceKeys)
            : m_relationshipClassIds(relationshipClassIds), m_direction(direction), m_instanceKeys(instanceKeys)
            {}
        RelatedInstanceInfo(bvector<ECClassId> relationshipClassIds, RequiredRelationDirection direction, bvector<ECClassInstanceKey> const& instanceKeys)
            : m_relationshipClassIds(relationshipClassIds), m_direction(direction)
            {
            ContainerHelpers::TransformContainer(m_instanceKeys, instanceKeys,
                [](auto const& k){return ECInstanceKey(k.GetClass()->GetId(), k.GetId());});
            }
        };

private:
    std::unique_ptr<RelatedInstanceInfo> m_relatedInstanceInfo;
    Utf8String m_instanceFilter;

private:
    void InitFromJson(RapidJsonValueCR json);

public:
    DataSourceFilter() {}
    ECPRESENTATION_EXPORT DataSourceFilter(DataSourceFilter const&);
    ECPRESENTATION_EXPORT DataSourceFilter(DataSourceFilter&&);
    ECPRESENTATION_EXPORT DataSourceFilter(std::unique_ptr<RelatedInstanceInfo>, Utf8String instanceFilter);
    DataSourceFilter(Utf8String instanceFilter) : m_instanceFilter(instanceFilter) {}
    DataSourceFilter(RapidJsonValueCR json) {InitFromJson(json);}
    ECPRESENTATION_EXPORT DataSourceFilter& operator=(DataSourceFilter const&);
    ECPRESENTATION_EXPORT DataSourceFilter& operator=(DataSourceFilter&&);
    rapidjson::Document AsJson() const;
    RelatedInstanceInfo const* GetRelatedInstanceInfo() const {return m_relatedInstanceInfo.get();}
};

/*=================================================================================**//**
* Identifies a single data source
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DataSourceInfo
{
    enum Parts
        {
        PART_Vars               = 1 << 0,
        PART_Filter             = 1 << 1,
        PART_RelatedClasses     = 1 << 2,
        PART_SpecificationHash  = 1 << 3,
        PART_NodeTypes          = 1 << 4,
        PART_ParentId           = 1 << 5,
        PART_TotalNodesCount    = 1 << 6,
        PART_HasNodes           = 1 << 7,
        PART_DirectNodesCount   = 1 << 8,
        PART_IsFinalized        = 1 << 9,
        PART_CustomJson         = 1 << 10,
        PART_HasPartialProviders= 1 << 11,
        PARTS_All = PART_Vars | PART_Filter | PART_RelatedClasses | PART_SpecificationHash | PART_NodeTypes | PART_ParentId
            | PART_TotalNodesCount | PART_HasNodes | PART_DirectNodesCount | PART_IsFinalized | PART_CustomJson | PART_HasPartialProviders
        };

private:
    DataSourceIdentifier m_identifier;
    RulesetVariables m_relatedVariables;
    DataSourceFilter m_filter;
    bmap<ECClassId, bool> m_relatedClasses;
    Utf8String m_specificationHash;
    Utf8String m_nodeTypes;
    BeGuid m_parentId;
    Nullable<bool> m_hasPartialProviders;
    bool m_isFinalized;
    Nullable<size_t> m_directNodesCount;
    Nullable<size_t> m_totalNodesCount;
    Nullable<bool> m_hasNodes;
    Json::Value m_customJson;
public:
    DataSourceInfo(): m_isFinalized(false) {}
    DataSourceInfo(DataSourceIdentifier identifier) : m_identifier(identifier), m_isFinalized(false) {}
    DataSourceInfo(DataSourceIdentifier identifier, RulesetVariables relatedVariables, DataSourceFilter filter, bmap<ECClassId, bool> relatedClasses,
        Utf8String specificationHash, Utf8String nodeTypes)
        : m_identifier(identifier), m_relatedVariables(relatedVariables), m_filter(filter), m_relatedClasses(relatedClasses),
        m_specificationHash(specificationHash), m_nodeTypes(nodeTypes), m_isFinalized(false)
        {}
    DataSourceIdentifier& GetIdentifier() {return m_identifier;}
    DataSourceIdentifier const& GetIdentifier() const {return m_identifier;}
    bool IsInitialized() const {return m_isFinalized;}
    void SetIsInitialized(bool value) {m_isFinalized = value;}
    RulesetVariables const& GetRelatedVariables() const {return m_relatedVariables;}
    void SetRelatedVariables(RulesetVariables vars) {m_relatedVariables = vars;}
    DataSourceFilter const& GetFilter() const {return m_filter;}
    void SetFilter(DataSourceFilter filter) {m_filter = filter;}
    bmap<ECClassId, bool> const& GetRelatedClasses() const {return m_relatedClasses;}
    void SetRelatedClasses(bmap<ECClassId, bool> map) {m_relatedClasses = map;}
    Utf8StringCR GetSpecificationHash() const {return m_specificationHash;}
    void SetSpecificationHash(Utf8String hash) {m_specificationHash = hash;}
    Utf8StringCR GetNodeTypes() const {return m_nodeTypes;}
    void SetNodeTypes(Utf8String value) {m_nodeTypes = value;}
    bool IsPartial() const {return m_parentId.IsValid();}
    BeGuidCR GetParentId() const {return m_parentId;}
    void SetParentId(BeGuid value) {m_parentId = value;}
    Nullable<bool> const& HasPartialProviders() const {return m_hasPartialProviders;}
    void SetHasPartialProviders(Nullable<bool> value) {m_hasPartialProviders = value;}
    Nullable<size_t> const& GetTotalNodesCount() const {return m_totalNodesCount;}
    void SetTotalNodesCount(Nullable<size_t> value) {m_totalNodesCount = value;}
    Nullable<bool> const& HasNodes() const {return m_hasNodes;}
    void SetHasNodes(Nullable<bool> value) {m_hasNodes = value;}
    Nullable<size_t> const& GetDirectNodesCount() const {return m_directNodesCount;}
    void SetDirectNodesCount(Nullable<size_t> value) {m_directNodesCount = value;}
    JsonValueCR GetCustomJson() const {return m_customJson;}
    JsonValueR GetCustomJson() {return m_customJson;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
