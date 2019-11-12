/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/IECPresentationManager.h>
#include "RulesEngineTypes.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* Describes a combined level of hierarchy which may consist of multiple hierarchy levels
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct CombinedHierarchyLevelInfo
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
    CombinedHierarchyLevelInfo() : m_physicalParentNodeId(nullptr) {}
    CombinedHierarchyLevelInfo(CombinedHierarchyLevelInfo&& other) 
        : m_connectionId(std::move(other.m_connectionId)), m_rulesetId(std::move(other.m_rulesetId)), m_locale(std::move(other.m_locale))
        {
        m_physicalParentNodeId = other.m_physicalParentNodeId;
        other.m_physicalParentNodeId = nullptr;
        }
    CombinedHierarchyLevelInfo(CombinedHierarchyLevelInfo const& other)
        : m_connectionId(other.m_connectionId), m_rulesetId(other.m_rulesetId), m_locale(other.m_locale)
        {
        SetParentNodeId(m_physicalParentNodeId, other.m_physicalParentNodeId, false);
        }
    CombinedHierarchyLevelInfo(Utf8String connectionId, Utf8String rulesetId, Utf8String locale, uint64_t physicalParentNodeId)
        : m_connectionId(connectionId), m_rulesetId(rulesetId), m_locale(locale)
        {
        SetParentNodeId(m_physicalParentNodeId, &physicalParentNodeId, false);
        }
    virtual ~CombinedHierarchyLevelInfo()
        {
        DELETE_AND_CLEAR(m_physicalParentNodeId);
        }
    CombinedHierarchyLevelInfo& operator=(CombinedHierarchyLevelInfo&& other)
        {
        m_connectionId = std::move(other.m_connectionId);
        m_rulesetId = std::move(other.m_rulesetId);
        m_locale = std::move(other.m_locale);
        DELETE_AND_CLEAR(m_physicalParentNodeId);
        m_physicalParentNodeId = other.m_physicalParentNodeId;
        other.m_physicalParentNodeId = nullptr;
        return *this;
        }
    CombinedHierarchyLevelInfo& operator=(CombinedHierarchyLevelInfo const& other)
        {
        m_connectionId = other.m_connectionId;
        m_rulesetId = other.m_rulesetId;
        m_locale = other.m_locale;
        SetParentNodeId(m_physicalParentNodeId, other.m_physicalParentNodeId);
        return *this;
        }
    bool operator<(CombinedHierarchyLevelInfo const& other) const
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
    bool operator==(CombinedHierarchyLevelInfo const& other) const
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
* Describes one level of hierarchy which may consist of multiple data sources
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct HierarchyLevelInfo : CombinedHierarchyLevelInfo
{
private:
    uint64_t m_id;
    uint64_t const* m_virtualParentNodeId;
public:
    HierarchyLevelInfo() : CombinedHierarchyLevelInfo(), m_id(0), m_virtualParentNodeId(nullptr) {}
    HierarchyLevelInfo(HierarchyLevelInfo&& other) 
        : CombinedHierarchyLevelInfo(other), m_id(other.m_id)
        {
        m_virtualParentNodeId = other.m_virtualParentNodeId;
        other.m_virtualParentNodeId = nullptr;
        }
    HierarchyLevelInfo(HierarchyLevelInfo const& other)
        : CombinedHierarchyLevelInfo(other), m_id(other.m_id)
        {
        SetParentNodeId(m_virtualParentNodeId, other.m_virtualParentNodeId, false);
        }
    HierarchyLevelInfo(Utf8String connectionId, Utf8String rulesetId, Utf8String locale, uint64_t physicalParentNodeId, uint64_t virtualParentNodeId)
        : CombinedHierarchyLevelInfo(connectionId, rulesetId, locale, physicalParentNodeId), m_id(0)
        {
        SetParentNodeId(m_virtualParentNodeId, &virtualParentNodeId, false);
        }
    HierarchyLevelInfo(uint64_t id, Utf8String connectionId, Utf8String rulesetId, Utf8String locale, uint64_t physicalParentNodeId, uint64_t virtualParentNodeId)
        : HierarchyLevelInfo(connectionId, rulesetId, locale, physicalParentNodeId, virtualParentNodeId)
        {
        m_id = id;
        }
    ~HierarchyLevelInfo()
        {
        DELETE_AND_CLEAR(m_virtualParentNodeId);
        }
    HierarchyLevelInfo& operator=(HierarchyLevelInfo&& other)
        {
        CombinedHierarchyLevelInfo::operator=(other);
        m_id = other.m_id;
        DELETE_AND_CLEAR(m_virtualParentNodeId);
        m_virtualParentNodeId = other.m_virtualParentNodeId;
        other.m_virtualParentNodeId = nullptr;
        return *this;
        }
    HierarchyLevelInfo& operator=(HierarchyLevelInfo const& other)
        {
        CombinedHierarchyLevelInfo::operator=(other);
        m_id = other.m_id;
        SetParentNodeId(m_virtualParentNodeId, other.m_virtualParentNodeId);
        return *this;
        }
    bool IsValid() const {return 0 != m_id;}
    bool operator<(HierarchyLevelInfo const& other) const
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
        return CombinedHierarchyLevelInfo::operator<(other);
        }
    bool operator==(HierarchyLevelInfo const& other) const
        {
        return m_id == other.m_id
            && 0 == CompareIds(m_virtualParentNodeId, other.m_virtualParentNodeId)
            && CombinedHierarchyLevelInfo::operator==(other);
        }
    void Invalidate() {m_id = 0;}
    uint64_t GetId() const {return m_id;}
    void SetId(uint64_t id) {m_id = id;}
    uint64_t const* GetVirtualParentNodeId() const {return m_virtualParentNodeId;}
    void SetVirtualParentNodeId(uint64_t id) {SetParentNodeId(m_virtualParentNodeId, &id);}
};
    
/*=================================================================================**//**
* Describes a single data source
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct DataSourceInfo
{
private:
    uint64_t m_id;
    uint64_t m_hierarchyLevelId;
    bvector<uint64_t> m_index;
public:
    DataSourceInfo() : m_id(0), m_hierarchyLevelId(0) {}
    DataSourceInfo(DataSourceInfo const& other)
        : m_id(other.m_id), m_hierarchyLevelId(other.m_hierarchyLevelId), m_index(other.m_index)
        {}
    DataSourceInfo(DataSourceInfo&& other)
        : m_id(other.m_id), m_hierarchyLevelId(other.m_hierarchyLevelId), m_index(std::move(other.m_index))
        {}
    DataSourceInfo(uint64_t hierarchyLevelId, bvector<uint64_t> index)
        : m_id(0), m_hierarchyLevelId(hierarchyLevelId), m_index(index)
        {}
    DataSourceInfo(uint64_t id, uint64_t hierarchyLevelId, bvector<uint64_t> index)
        : m_id(id), m_hierarchyLevelId(hierarchyLevelId), m_index(index)
        {}
    bool IsValid() const {return 0 != m_id;}
    bool operator<(DataSourceInfo const& other) const
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
    DataSourceInfo& operator=(DataSourceInfo const& other)
        {
        m_id = other.m_id;
        m_hierarchyLevelId = other.m_hierarchyLevelId;
        m_index = other.m_index;
        return *this;
        }
    DataSourceInfo& operator=(DataSourceInfo&& other)
        {
        m_id = other.m_id;
        m_hierarchyLevelId = other.m_hierarchyLevelId;
        m_index = std::move(other.m_index);
        return *this;
        }
    bool operator==(DataSourceInfo const& other) const
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

END_BENTLEY_ECPRESENTATION_NAMESPACE
