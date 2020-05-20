/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/IECPresentationManager.h>
#include "RulesEngineTypes.h"
#include "RulesetVariables.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* Identifies a combined level of hierarchy which may consist of multiple hierarchy levels
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct CombinedHierarchyLevelIdentifier
{
private:
    Utf8String m_connectionId;
    Utf8String m_rulesetId;
    Utf8String m_locale;
    uint64_t const* m_physicalParentNodeId;
protected:
    static void SetParentNodeId(uint64_t const*& id, uint64_t const* value, bool cleanupOldValue = true)
        {
        if (cleanupOldValue && nullptr != id)
            delete id;
        if (nullptr != value && 0 != *value)
            id = new uint64_t(*value);
        else
            id = nullptr;
        }
    static int CompareIds(uint64_t const* lhs, uint64_t const* rhs)
        {
        if (nullptr == lhs && nullptr != rhs)
            return -1;
        if (nullptr == rhs && nullptr != lhs)
            return 1;
        if (nullptr != lhs && *lhs < *rhs)
            return 1;
        if (nullptr != lhs && *lhs > *rhs)
            return -1;
        return 0;
        }
public:
    CombinedHierarchyLevelIdentifier() : m_physicalParentNodeId(nullptr) {}
    CombinedHierarchyLevelIdentifier(CombinedHierarchyLevelIdentifier&& other)
        : m_connectionId(std::move(other.m_connectionId)), m_rulesetId(std::move(other.m_rulesetId)), m_locale(std::move(other.m_locale))
        {
        m_physicalParentNodeId = other.m_physicalParentNodeId;
        other.m_physicalParentNodeId = nullptr;
        }
    CombinedHierarchyLevelIdentifier(CombinedHierarchyLevelIdentifier const& other)
        : m_connectionId(other.m_connectionId), m_rulesetId(other.m_rulesetId), m_locale(other.m_locale)
        {
        SetParentNodeId(m_physicalParentNodeId, other.m_physicalParentNodeId, false);
        }
    CombinedHierarchyLevelIdentifier(Utf8String connectionId, Utf8String rulesetId, Utf8String locale, uint64_t physicalParentNodeId)
        : m_connectionId(connectionId), m_rulesetId(rulesetId), m_locale(locale)
        {
        SetParentNodeId(m_physicalParentNodeId, &physicalParentNodeId, false);
        }
    virtual ~CombinedHierarchyLevelIdentifier()
        {
        DELETE_AND_CLEAR(m_physicalParentNodeId);
        }
    CombinedHierarchyLevelIdentifier& operator=(CombinedHierarchyLevelIdentifier&& other)
        {
        m_connectionId = std::move(other.m_connectionId);
        m_rulesetId = std::move(other.m_rulesetId);
        m_locale = std::move(other.m_locale);
        DELETE_AND_CLEAR(m_physicalParentNodeId);
        m_physicalParentNodeId = other.m_physicalParentNodeId;
        other.m_physicalParentNodeId = nullptr;
        return *this;
        }
    CombinedHierarchyLevelIdentifier& operator=(CombinedHierarchyLevelIdentifier const& other)
        {
        m_connectionId = other.m_connectionId;
        m_rulesetId = other.m_rulesetId;
        m_locale = other.m_locale;
        SetParentNodeId(m_physicalParentNodeId, other.m_physicalParentNodeId);
        return *this;
        }
    bool operator<(CombinedHierarchyLevelIdentifier const& other) const
        {
        int physicalParentNodeIdCmp = CompareIds(m_physicalParentNodeId, other.m_physicalParentNodeId);
        if (physicalParentNodeIdCmp < 0)
            return true;
        if (physicalParentNodeIdCmp > 0)
            return false;
        int rulesetIdCmp = m_rulesetId.CompareTo(other.m_rulesetId);
        if (rulesetIdCmp < 0)
            return true;
        if (rulesetIdCmp > 0)
            return false;
        int localeCmp = m_locale.CompareTo(other.m_locale);
        if (localeCmp < 0)
            return true;
        if (localeCmp > 0)
            return false;
        return (m_connectionId < other.m_connectionId);
        }
    bool operator==(CombinedHierarchyLevelIdentifier const& other) const
        {
        return m_connectionId == other.m_connectionId
            && m_rulesetId.Equals(other.m_rulesetId)
            && m_locale.Equals(other.m_locale)
            && 0 == CompareIds(m_physicalParentNodeId, other.m_physicalParentNodeId);
        }
    Utf8StringCR GetConnectionId() const {return m_connectionId;}
    Utf8StringCR GetRulesetId() const {return m_rulesetId;}
    Utf8StringCR GetLocale() const {return m_locale;}
    uint64_t const* GetPhysicalParentNodeId() const {return m_physicalParentNodeId;}
    void SetPhysicalParentNodeId(uint64_t id) {SetParentNodeId(m_physicalParentNodeId, &id);}
};

/*=================================================================================**//**
* Identifies one level of hierarchy which may consist of multiple data sources
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct HierarchyLevelIdentifier : CombinedHierarchyLevelIdentifier
{
private:
    uint64_t m_id;
    uint64_t const* m_virtualParentNodeId;
public:
    HierarchyLevelIdentifier() : CombinedHierarchyLevelIdentifier(), m_id(0), m_virtualParentNodeId(nullptr) {}
    HierarchyLevelIdentifier(HierarchyLevelIdentifier&& other)
        : CombinedHierarchyLevelIdentifier(other), m_id(other.m_id)
        {
        m_virtualParentNodeId = other.m_virtualParentNodeId;
        other.m_virtualParentNodeId = nullptr;
        }
    HierarchyLevelIdentifier(HierarchyLevelIdentifier const& other)
        : CombinedHierarchyLevelIdentifier(other), m_id(other.m_id)
        {
        SetParentNodeId(m_virtualParentNodeId, other.m_virtualParentNodeId, false);
        }
    HierarchyLevelIdentifier(Utf8String connectionId, Utf8String rulesetId, Utf8String locale, uint64_t physicalParentNodeId, uint64_t virtualParentNodeId)
        : CombinedHierarchyLevelIdentifier(connectionId, rulesetId, locale, physicalParentNodeId), m_id(0)
        {
        SetParentNodeId(m_virtualParentNodeId, &virtualParentNodeId, false);
        }
    HierarchyLevelIdentifier(uint64_t id, Utf8String connectionId, Utf8String rulesetId, Utf8String locale, uint64_t physicalParentNodeId, uint64_t virtualParentNodeId)
        : HierarchyLevelIdentifier(connectionId, rulesetId, locale, physicalParentNodeId, virtualParentNodeId)
        {
        m_id = id;
        }
    ~HierarchyLevelIdentifier()
        {
        DELETE_AND_CLEAR(m_virtualParentNodeId);
        }
    HierarchyLevelIdentifier& operator=(HierarchyLevelIdentifier&& other)
        {
        CombinedHierarchyLevelIdentifier::operator=(other);
        m_id = other.m_id;
        DELETE_AND_CLEAR(m_virtualParentNodeId);
        m_virtualParentNodeId = other.m_virtualParentNodeId;
        other.m_virtualParentNodeId = nullptr;
        return *this;
        }
    HierarchyLevelIdentifier& operator=(HierarchyLevelIdentifier const& other)
        {
        CombinedHierarchyLevelIdentifier::operator=(other);
        m_id = other.m_id;
        SetParentNodeId(m_virtualParentNodeId, other.m_virtualParentNodeId);
        return *this;
        }
    bool IsValid() const {return 0 != m_id;}
    bool operator<(HierarchyLevelIdentifier const& other) const
        {
        if (m_id < other.m_id)
            return true;
        if (m_id > other.m_id)
            return false;
        int virtualParentNodeIdCmp = CompareIds(m_virtualParentNodeId, other.m_virtualParentNodeId);
        if (virtualParentNodeIdCmp < 0)
            return true;
        if (virtualParentNodeIdCmp > 0)
            return false;
        return CombinedHierarchyLevelIdentifier::operator<(other);
        }
    bool operator==(HierarchyLevelIdentifier const& other) const
        {
        return m_id == other.m_id
            && 0 == CompareIds(m_virtualParentNodeId, other.m_virtualParentNodeId)
            && CombinedHierarchyLevelIdentifier::operator==(other);
        }
    void Invalidate() {m_id = 0;}
    uint64_t GetId() const {return m_id;}
    void SetId(uint64_t id) {m_id = id;}
    uint64_t const* GetVirtualParentNodeId() const {return m_virtualParentNodeId;}
    void SetVirtualParentNodeId(uint64_t id) {SetParentNodeId(m_virtualParentNodeId, &id);}
};

/*=================================================================================**//**
* Identifies a single data source
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct DataSourceIdentifier
{
private:
    uint64_t m_id;
    uint64_t m_hierarchyLevelId;
    bvector<uint64_t> m_index;
public:
    DataSourceIdentifier() : m_id(0), m_hierarchyLevelId(0) {}
    DataSourceIdentifier(DataSourceIdentifier const& other)
        : m_id(other.m_id), m_hierarchyLevelId(other.m_hierarchyLevelId), m_index(other.m_index)
        {}
    DataSourceIdentifier(DataSourceIdentifier&& other)
        : m_id(other.m_id), m_hierarchyLevelId(other.m_hierarchyLevelId), m_index(std::move(other.m_index))
        {}
    DataSourceIdentifier(uint64_t hierarchyLevelId, bvector<uint64_t> index)
        : m_id(0), m_hierarchyLevelId(hierarchyLevelId), m_index(index)
        {}
    DataSourceIdentifier(uint64_t id, uint64_t hierarchyLevelId, bvector<uint64_t> index)
        : m_id(id), m_hierarchyLevelId(hierarchyLevelId), m_index(index)
        {}
    bool IsValid() const {return 0 != m_id;}
    bool operator<(DataSourceIdentifier const& other) const
        {
        if (m_id < other.m_id)
            return true;
        if (m_id > other.m_id)
            return false;
        if (m_hierarchyLevelId < other.m_hierarchyLevelId)
            return true;
        if (m_hierarchyLevelId > other.m_hierarchyLevelId)
            return false;
        if (m_index.size() < other.m_index.size())
            return true;
        if (m_index.size() > other.m_index.size())
            return false;
        for (size_t i = 0; i < m_index.size(); ++i)
            {
            if (m_index[i] < other.m_index[i])
                return true;
            if (m_index[i] > other.m_index[i])
                return false;
            }
        return false;
        }
    DataSourceIdentifier& operator=(DataSourceIdentifier const& other)
        {
        m_id = other.m_id;
        m_hierarchyLevelId = other.m_hierarchyLevelId;
        m_index = other.m_index;
        return *this;
        }
    DataSourceIdentifier& operator=(DataSourceIdentifier&& other)
        {
        m_id = other.m_id;
        m_hierarchyLevelId = other.m_hierarchyLevelId;
        m_index = std::move(other.m_index);
        return *this;
        }
    bool operator==(DataSourceIdentifier const& other) const
        {
        return m_id == other.m_id
            && m_hierarchyLevelId == other.m_hierarchyLevelId
            && m_index == other.m_index;
        }
    void Invalidate() {m_id = 0;}
    uint64_t GetId() const {return m_id;}
    void SetId(uint64_t id) {m_id = id;}
    uint64_t GetHierarchyLevelId() const {return m_hierarchyLevelId;}
    bvector<uint64_t> const& GetIndex() const {return m_index;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct DataSourceFilter
{
    struct RelatedInstanceInfo
        {
        bset<ECClassId> m_relationshipClassIds;
        bvector<ECInstanceKey> m_instanceKeys;
        RequiredRelationDirection m_direction;
        RelatedInstanceInfo() {}
        RelatedInstanceInfo(bset<ECClassId> relationshipClassIds, RequiredRelationDirection direction, bvector<ECInstanceKey> instanceKeys)
            : m_relationshipClassIds(relationshipClassIds), m_direction(direction), m_instanceKeys(instanceKeys)
            {}
        RelatedInstanceInfo(bset<ECClassId> relationshipClassIds, RequiredRelationDirection direction, bvector<ECClassInstanceKey> const& instanceKeys)
            : m_relationshipClassIds(relationshipClassIds), m_direction(direction)
            {
            std::transform(instanceKeys.begin(), instanceKeys.end(), std::back_inserter(m_instanceKeys),
                [](ECClassInstanceKeyCR k){return ECInstanceKey(k.GetClass()->GetId(), k.GetId());});
            }
        bool IsValid() const {return !m_relationshipClassIds.empty() && !m_instanceKeys.empty();}
        };

private:
    RelatedInstanceInfo const* m_relatedInstanceInfo;
    Utf8String m_instanceFilter;

private:
    void InitFromJson(RapidJsonValueCR json);

public:
    DataSourceFilter() : m_relatedInstanceInfo(nullptr) {}
    ECPRESENTATION_EXPORT DataSourceFilter(DataSourceFilter const&);
    ECPRESENTATION_EXPORT DataSourceFilter(DataSourceFilter&&);
    DataSourceFilter(RapidJsonValueCR json) : m_relatedInstanceInfo(nullptr) {InitFromJson(json);}
    DataSourceFilter(Utf8CP instanceFilter) : m_instanceFilter(instanceFilter) {}
    ECPRESENTATION_EXPORT DataSourceFilter(RelatedInstanceInfo const&, Utf8CP instanceFilter);
    ~DataSourceFilter() {DELETE_AND_CLEAR(m_relatedInstanceInfo);}
    ECPRESENTATION_EXPORT DataSourceFilter& operator=(DataSourceFilter const&);
    ECPRESENTATION_EXPORT DataSourceFilter& operator=(DataSourceFilter&&);
    rapidjson::Document AsJson() const;
    RelatedInstanceInfo const* GetRelatedInstanceInfo() const {return m_relatedInstanceInfo;}
};

/*=================================================================================**//**
* Identifies a single data source
* @bsiclass                                     Grigas.Petraitis                05/2020
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
        PART_IsPartial          = 1 << 5,
        PARTS_All = PART_Vars | PART_Filter | PART_RelatedClasses | PART_SpecificationHash | PART_NodeTypes | PART_IsPartial
        };

private:
    DataSourceIdentifier m_identifier;
    RulesetVariables m_relatedVariables;
    DataSourceFilter m_filter;
    bmap<ECClassId, bool> m_relatedClasses;
    Utf8String m_specificationHash;
    Utf8String m_nodeTypes;
    bool m_isPartial;
public:
    DataSourceInfo() {}
    DataSourceInfo(DataSourceIdentifier identifier, RulesetVariables relatedVariables, DataSourceFilter filter, bmap<ECClassId, bool> relatedClasses, Utf8String specificationHash, Utf8String nodeTypes, bool isPartial = false)
        : m_identifier(identifier), m_relatedVariables(relatedVariables), m_filter(filter), m_relatedClasses(relatedClasses), m_specificationHash(specificationHash), m_nodeTypes(nodeTypes), m_isPartial(isPartial)
        {}
    DataSourceIdentifier& GetIdentifier() {return m_identifier;}
    DataSourceIdentifier const& GetIdentifier() const {return m_identifier;}
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
    bool IsPartial() const {return m_isPartial;}
    void SetIsPartial(bool value) {m_isPartial = value;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
