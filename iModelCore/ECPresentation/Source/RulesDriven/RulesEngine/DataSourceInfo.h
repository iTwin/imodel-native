/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/DataSourceInfo.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/IECPresentationManager.h>
#include "RulesEngineTypes.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* Describes one level of hierarchy which may consist of multiple data sources
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct HierarchyLevelInfo
{
private:
    Utf8String m_connectionId;
    Utf8String m_rulesetId;
    Utf8String m_locale;
    uint64_t const* m_physicalParentNodeId;
private:
    void SetParentNodeId(uint64_t const* id)
        {
        if (nullptr != id && 0 != *id)
            m_physicalParentNodeId = new uint64_t(*id);
        else
            m_physicalParentNodeId = nullptr;
        }
public:
    HierarchyLevelInfo() : m_physicalParentNodeId(nullptr) {}
    HierarchyLevelInfo(HierarchyLevelInfo&& other) 
        : m_connectionId(std::move(other.m_connectionId)), m_rulesetId(std::move(other.m_rulesetId)), m_locale(std::move(other.m_locale))
        {
        m_physicalParentNodeId = other.m_physicalParentNodeId;
        other.m_physicalParentNodeId = nullptr;
        }
    HierarchyLevelInfo(HierarchyLevelInfo const& other)
        : m_connectionId(other.m_connectionId), m_rulesetId(other.m_rulesetId), m_locale(other.m_locale)
        {
        SetParentNodeId(other.m_physicalParentNodeId);
        }
    HierarchyLevelInfo(Utf8String connectionId, Utf8String rulesetId, Utf8String locale, uint64_t const* physicalParentNodeId)
        : m_connectionId(connectionId), m_rulesetId(rulesetId), m_locale(locale)
        {
        SetParentNodeId(physicalParentNodeId);
        }
    HierarchyLevelInfo(Utf8String connectionId, Utf8String rulesetId, Utf8String locale, uint64_t physicalParentNodeId)
        : m_connectionId(connectionId), m_rulesetId(rulesetId), m_locale(locale)
        {
        SetParentNodeId(&physicalParentNodeId);
        }
    ~HierarchyLevelInfo() {DELETE_AND_CLEAR(m_physicalParentNodeId);}
    HierarchyLevelInfo& operator=(HierarchyLevelInfo&& other)
        {
        m_connectionId = std::move(other.m_connectionId);
        m_rulesetId = std::move(other.m_rulesetId);
        m_locale = std::move(other.m_locale);
        m_physicalParentNodeId = other.m_physicalParentNodeId;
        other.m_physicalParentNodeId = nullptr;
        return *this;
        }
    HierarchyLevelInfo& operator=(HierarchyLevelInfo const& other)
        {
        m_connectionId = other.m_connectionId;
        m_rulesetId = other.m_rulesetId;
        m_locale = other.m_locale;
        SetParentNodeId(other.m_physicalParentNodeId);
        return *this;
        }
    bool IsValid() const {return !m_connectionId.empty();}
    bool operator<(HierarchyLevelInfo const& other) const
        {
        if (nullptr == m_physicalParentNodeId && nullptr != other.m_physicalParentNodeId)
            return true;
        if (nullptr == other.m_physicalParentNodeId && nullptr != m_physicalParentNodeId)
            return false;
        if (nullptr != m_physicalParentNodeId && *m_physicalParentNodeId < *other.m_physicalParentNodeId)
            return true;
        if (nullptr != m_physicalParentNodeId && *m_physicalParentNodeId > *other.m_physicalParentNodeId)
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
    bool operator==(HierarchyLevelInfo const& other) const
        {
        return m_connectionId == other.m_connectionId
            && m_rulesetId.Equals(other.m_rulesetId)
            && m_locale.Equals(other.m_locale)
            && (nullptr == m_physicalParentNodeId && nullptr == other.m_physicalParentNodeId
                || nullptr != m_physicalParentNodeId && nullptr != other.m_physicalParentNodeId && *m_physicalParentNodeId == *other.m_physicalParentNodeId);
        }
    Utf8StringCR GetConnectionId() const {return m_connectionId;}
    Utf8StringCR GetRulesetId() const {return m_rulesetId;}
    Utf8StringCR GetLocale() const {return m_locale;}
    uint64_t const* GetPhysicalParentNodeId() const {return m_physicalParentNodeId;}
    void SetPhysicalParentNodeId(uint64_t id) {DELETE_AND_CLEAR(m_physicalParentNodeId); SetParentNodeId(&id);}
};
    
/*=================================================================================**//**
* Describes a single data source
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct DataSourceInfo : HierarchyLevelInfo
{
private:
    uint64_t m_datasourceId;
    uint64_t const* m_virtualParentNodeId;
public:
    DataSourceInfo() : m_datasourceId(0), m_virtualParentNodeId(nullptr) {}
    DataSourceInfo(DataSourceInfo const& other)
        : HierarchyLevelInfo(other), m_datasourceId(other.m_datasourceId), m_virtualParentNodeId(nullptr)
        {
        if (nullptr != other.m_virtualParentNodeId)
            m_virtualParentNodeId = new uint64_t(*other.m_virtualParentNodeId);
        }
    DataSourceInfo(DataSourceInfo&& other)
        : HierarchyLevelInfo(other), m_datasourceId(other.m_datasourceId), m_virtualParentNodeId(other.m_virtualParentNodeId)
        {
        other.m_virtualParentNodeId = nullptr;
        }
    DataSourceInfo(Utf8String connectionId, Utf8String rulesetId, Utf8String locale, uint64_t const* physicalParentNodeId, uint64_t const* virtualParentNodeId)
        : HierarchyLevelInfo(connectionId, rulesetId, locale, physicalParentNodeId), m_datasourceId(0), m_virtualParentNodeId(nullptr)
        {
        if (nullptr != virtualParentNodeId)
            m_virtualParentNodeId = new uint64_t(*virtualParentNodeId);
        }
    DataSourceInfo(uint64_t datasourceId, Utf8String connectionId, Utf8String rulesetId, Utf8String locale, uint64_t const* physicalParentNodeId, uint64_t const* virtualParentNodeId)
        : HierarchyLevelInfo(connectionId, rulesetId, locale, physicalParentNodeId), m_datasourceId(datasourceId), m_virtualParentNodeId(nullptr)
        {
        if (nullptr != virtualParentNodeId)
            m_virtualParentNodeId = new uint64_t(*virtualParentNodeId);
        }
    ~DataSourceInfo() {DELETE_AND_CLEAR(m_virtualParentNodeId);}
    bool IsValid() const {return 0 != m_datasourceId && HierarchyLevelInfo::IsValid();}
    DataSourceInfo& operator=(DataSourceInfo const& other)
        {
        HierarchyLevelInfo::operator=(other);
        m_datasourceId = other.m_datasourceId;
        m_virtualParentNodeId = (nullptr != other.m_virtualParentNodeId) ? new uint64_t(*other.m_virtualParentNodeId) : nullptr;
        return *this;
        }
    DataSourceInfo& operator=(DataSourceInfo&& other)
        {
        HierarchyLevelInfo::operator=(other);
        m_datasourceId = other.m_datasourceId;
        m_virtualParentNodeId = other.m_virtualParentNodeId;
        other.m_virtualParentNodeId = nullptr;
        return *this;
        }
    bool operator==(DataSourceInfo const& other) const
        {
        return HierarchyLevelInfo::operator==(other)
            && m_datasourceId == other.m_datasourceId
            && (nullptr == m_virtualParentNodeId && nullptr == other.m_virtualParentNodeId
                || nullptr != m_virtualParentNodeId && nullptr != other.m_virtualParentNodeId && *m_virtualParentNodeId == *other.m_virtualParentNodeId);
        }
    uint64_t GetDataSourceId() const {return m_datasourceId;}
    void SetDataSourceId(uint64_t id) {m_datasourceId = id;}
    uint64_t const* GetVirtualParentNodeId() const {return m_virtualParentNodeId;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
