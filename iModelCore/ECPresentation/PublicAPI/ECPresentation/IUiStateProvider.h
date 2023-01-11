/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/NavNode.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
//! Stores information about a hierarchy level state.
// @bsiclass
//=======================================================================================
struct HierarchyLevelState
{
private:
    bool m_isExpanded;
    bvector<std::shared_ptr<InstanceFilterDefinition const>> m_instanceFilters;
public:
    HierarchyLevelState() : m_isExpanded(false) {}
    bool IsExpanded() const {return m_isExpanded;}
    void SetIsExpanded(bool value) {m_isExpanded = value;}
    bvector<std::shared_ptr<InstanceFilterDefinition const>> const& GetInstanceFilters() const
        {
        static bvector<std::shared_ptr<InstanceFilterDefinition const>> const s_emptyInstanceFilter{ nullptr };
        return m_instanceFilters.empty() ? s_emptyInstanceFilter : m_instanceFilters;
        }
    void SetInstanceFilters(bvector<std::shared_ptr<InstanceFilterDefinition const>> value) {m_instanceFilters = value;}
};

//=======================================================================================
//! Stores information about UI state of a single hierarchy (that consists of multiple
//! hierarchy levels)
// @bsiclass
//=======================================================================================
struct UiState
{
private:
    bmap<NavNodeKeyCPtr, HierarchyLevelState, NavNodeKeyPtrComparer> m_hierarchyLevelStates;

public:
    HierarchyLevelState const& GetHierarchyLevelState(NavNodeKeyCP parentNodeKey) const
        {
        static HierarchyLevelState const s_default;
        auto iter = m_hierarchyLevelStates.find(parentNodeKey);
        return (iter != m_hierarchyLevelStates.end()) ? iter->second : s_default;
        }
    HierarchyLevelState const& GetHierarchyLevelState(NavNodeCP parentNode) const
        {
        return GetHierarchyLevelState(parentNode ? parentNode->GetKey().get() : nullptr);
        }

    HierarchyLevelState& GetHierarchyLevelState(NavNodeKeyCP parentNodeKey)
        {
        return m_hierarchyLevelStates[parentNodeKey];
        }
    HierarchyLevelState& GetHierarchyLevelState(NavNodeCP parentNode)
        {
        return GetHierarchyLevelState(parentNode ? parentNode->GetKey().get() : nullptr);
        }

    void IterateHierarchyLevels(std::function<bool(NavNodeKeyCP parentNodeKey, HierarchyLevelState const& state)> const& cb) const
        {
        for (auto const& entry : m_hierarchyLevelStates)
            {
            bool res = cb(entry.first.get(), entry.second);
            if (!res)
                break;
            }
        }
};

//=======================================================================================
//! Stores information about UI state associated with a connection.
// @bsiclass
//=======================================================================================
struct ConnectionUiState
{
private:
    IConnectionCP m_connection;
    std::shared_ptr<UiState> m_state;
public:
    ConnectionUiState(IConnectionCR connection, std::shared_ptr<UiState> state): m_connection(&connection), m_state(state) {}
    IConnectionCR GetConnection() const {return *m_connection;}
    UiState const& GetState() const {return *m_state;}
    std::shared_ptr<UiState> GetStatePtr() const {return m_state;}
};

//=======================================================================================
//! Stores information about UI state associated with a ruleset.
// @bsiclass
//=======================================================================================
struct RulesetUiState
{
private:
    Utf8StringCP m_rulesetId;
    std::shared_ptr<UiState> m_state;
public:
    RulesetUiState(Utf8StringCR rulesetId, std::shared_ptr<UiState> state): m_rulesetId(&rulesetId), m_state(state) {}
    Utf8StringCR GetRulesetId() const {return *m_rulesetId;}
    UiState const& GetState() const {return *m_state;}
    std::shared_ptr<UiState> GetStatePtr() const {return m_state;}
};

//=======================================================================================
//! An interface for a class that provides information about current UI state.
// @bsiclass
//=======================================================================================
struct IUiStateProvider
{
protected:
    virtual std::shared_ptr<UiState> _GetUiState(IConnectionCR, Utf8StringCR) const = 0;
    virtual bvector<RulesetUiState> _GetUiState(IConnectionCR) const = 0;
    virtual bvector<ConnectionUiState> _GetUiState(Utf8StringCR) const = 0;
public:
    virtual ~IUiStateProvider() {}
    std::shared_ptr<UiState> GetUiState(IConnectionCR connectionId, Utf8StringCR rulesetId) const {return _GetUiState(connectionId, rulesetId);}
    bvector<RulesetUiState> GetUiState(IConnectionCR connectionId) const {return _GetUiState(connectionId);}
    bvector<ConnectionUiState> GetUiState(Utf8StringCR rulesetId) const {return _GetUiState(rulesetId);}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
