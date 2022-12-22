/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "UiStateProvider.h"
#include "ECPresentationSerializer.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IModelJsECPresentationUiStateProvider::~IModelJsECPresentationUiStateProvider()
    {
    if (m_manager)
        m_manager->GetConnections().DropListener(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationUiStateProvider::_OnConnectionEvent(ConnectionEvent const& evt)
    {
    if (evt.GetEventType() != ConnectionEventType::Closed)
        return;

    BeMutexHolder lock(m_mutex);
    auto iter = m_uiState.begin();
    while (iter != m_uiState.end())
        {
        if (iter->first.first == &evt.GetConnection())
            iter = m_uiState.erase(iter);
        else
            ++iter;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Warning: this must be protected with `m_mutex`, which must be held locked while the
* result is being used.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UiState& IModelJsECPresentationUiStateProvider::GetUiState(IConnectionCR connection, Utf8StringCR rulesetId)
    {
    auto key = bpair<IConnectionCP, Utf8String>(&connection, rulesetId);
    auto iter = m_uiState.find(key);
    if (m_uiState.end() == iter)
        iter = m_uiState.insert(std::make_pair(key, std::make_unique<UiState>())).first;
    return *iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<UiState> IModelJsECPresentationUiStateProvider::_GetUiState(IConnectionCR connection, Utf8StringCR rulesetId) const
    {
    BeMutexHolder lock(m_mutex);
    auto iter = m_uiState.find(make_bpair(&connection, rulesetId));
    if (m_uiState.end() != iter)
        return std::make_shared<UiState>(*iter->second);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RulesetUiState> IModelJsECPresentationUiStateProvider::_GetUiState(IConnectionCR connection) const
    {
    BeMutexHolder lock(m_mutex);
    bvector<RulesetUiState> result;
    for (auto const& entry : m_uiState)
        {
        if (entry.first.first == &connection)
            result.push_back(RulesetUiState(entry.first.second, std::make_shared<UiState>(*entry.second)));
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ConnectionUiState> IModelJsECPresentationUiStateProvider::_GetUiState(Utf8StringCR rulesetId) const
    {
    BeMutexHolder lock(m_mutex);
    bvector<ConnectionUiState> result;
    for (auto const& entry : m_uiState)
        {
        if (entry.first.second == rulesetId)
            result.push_back(ConnectionUiState(*entry.first.first, std::make_shared<UiState>(*entry.second)));
        }
    return result;
    }

#define PRESENTATION_JSON_ATTRIBUTE_StateChanges_NodeKey            "nodeKey"
#define PRESENTATION_JSON_ATTRIBUTE_StateChanges_IsExpanded         "isExpanded"
#define PRESENTATION_JSON_ATTRIBUTE_StateChanges_InstanceFilters    "instanceFilters"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult IModelJsECPresentationUiStateProvider::UpdateHierarchyState(ECPresentationManager& manager, ECDbCR db, Utf8StringCR rulesetId, BeJsConst stateChanges)
    {
    if (!m_manager)
        {
        m_manager = &manager;
        m_manager->GetConnections().AddListener(*this);
        }

    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "Uknown iModel connection");

    if (!stateChanges.isArray())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "Expected `stateChanges` argument to be an array");

    BeMutexHolder lock(m_mutex);
    UiState& uiState = GetUiState(*connection, rulesetId);

    for (BeJsConst::ArrayIndex i = 0; i < stateChanges.size(); ++i)
        {
        auto const& item = stateChanges[i];
        if (!item.isObject())
            return ECPresentationResult(ECPresentationStatus::InvalidArgument, "Expected items in `stateChanges` array to be of object type");
        if (!item.isMember(PRESENTATION_JSON_ATTRIBUTE_StateChanges_NodeKey))
            return ECPresentationResult(ECPresentationStatus::InvalidArgument, "Expected objects in `stateChanges` array to have a `" PRESENTATION_JSON_ATTRIBUTE_StateChanges_NodeKey "` member");

        NavNodeKeyCPtr nodeKey;
        if (!item[PRESENTATION_JSON_ATTRIBUTE_StateChanges_NodeKey].isNull())
            nodeKey = manager.GetSerializer().GetNavNodeKeyFromJson(*connection, item[PRESENTATION_JSON_ATTRIBUTE_StateChanges_NodeKey]);
        auto& hierarchyLevelState = uiState.GetHierarchyLevelState(nodeKey.get());

        if (item.isMember(PRESENTATION_JSON_ATTRIBUTE_StateChanges_IsExpanded) && !item[PRESENTATION_JSON_ATTRIBUTE_StateChanges_IsExpanded].isBool())
            return ECPresentationResult(ECPresentationStatus::InvalidArgument, "Expected `" PRESENTATION_JSON_ATTRIBUTE_StateChanges_IsExpanded "` member in `stateChanges` items to be a boolean");
        bool isExpanded = item[PRESENTATION_JSON_ATTRIBUTE_StateChanges_IsExpanded].GetBoolean(false);
        hierarchyLevelState.SetIsExpanded(isExpanded);

        if (item.isMember(PRESENTATION_JSON_ATTRIBUTE_StateChanges_InstanceFilters) && !item[PRESENTATION_JSON_ATTRIBUTE_StateChanges_InstanceFilters].isArray())
            return ECPresentationResult(ECPresentationStatus::InvalidArgument, "Expected `" PRESENTATION_JSON_ATTRIBUTE_StateChanges_InstanceFilters "` member in `stateChanges` items to be an array of strings");

        bvector<std::shared_ptr<InstanceFilterDefinition const>> instanceFilters;
        item[PRESENTATION_JSON_ATTRIBUTE_StateChanges_InstanceFilters].ForEachArrayMember([&](BeJsConst::ArrayIndex filterIndex, BeJsConst instanceFilterJson)
            {
            auto instanceFilter = manager.GetSerializer().GetInstanceFilterFromJson(*connection, instanceFilterJson);
            if (instanceFilter != nullptr)
                instanceFilters.push_back(std::move(instanceFilter));
            return false;
            });
        hierarchyLevelState.SetInstanceFilters(instanceFilters);
        }

    return ECPresentationResult(rapidjson::Value(rapidjson::kNullType), false);
    }
