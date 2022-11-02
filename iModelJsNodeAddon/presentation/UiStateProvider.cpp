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
ECPresentationResult IModelJsECPresentationUiStateProvider::AddNodeKeys(Utf8StringCR rulesetId, NavNodeKeyListCR keys)
    {
    BeMutexHolder lock(m_mutex);
    auto iter = m_expandedNodesPerHierarchy.find(rulesetId);
    if (m_expandedNodesPerHierarchy.end() == iter)
        iter = m_expandedNodesPerHierarchy.Insert(rulesetId, NavNodeKeySet()).first;

    for (NavNodeKeyCPtr key : keys)
        iter->second.insert(key);

    return ECPresentationResult(rapidjson::Value(rapidjson::kNullType), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult IModelJsECPresentationUiStateProvider::RemoveNodeKeys(Utf8StringCR rulesetId, NavNodeKeyListCR keys)
    {
    BeMutexHolder lock(m_mutex);
    auto iter = m_expandedNodesPerHierarchy.find(rulesetId);
    if (m_expandedNodesPerHierarchy.end() == iter)
        return ECPresentationResult(rapidjson::Value(rapidjson::kNullType), false);

    for (NavNodeKeyCPtr key : keys)
        iter->second.erase(key);

    if (iter->second.empty())
        m_expandedNodesPerHierarchy.erase(rulesetId);

    return ECPresentationResult(rapidjson::Value(rapidjson::kNullType), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr IModelJsECPresentationUiStateProvider::_GetExpandedNodes(Utf8StringCR rulesetId) const
    {
    BeMutexHolder lock(m_mutex);
    auto iter = m_expandedNodesPerHierarchy.find(rulesetId);
    if (m_expandedNodesPerHierarchy.end() != iter)
        return NavNodeKeySetContainer::Create(iter->second);

    // if expanded nodes for this ruleset is not found assume that nothing is expanded
    return NavNodeKeyListContainer::Create();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult IModelJsECPresentationUiStateProvider::UpdateHierarchyState(ECPresentationManager& manager, ECDbCR db, Utf8StringCR rulesetId, Utf8StringCR change, Utf8StringCR serializedKeys)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (connection.IsNull())
        return ECPresentationResult(ECPresentationStatus::Error, "Uknown ECDb connection");

    NavNodeKeyList keys = IModelJsECPresentationSerializer::GetNavNodeKeysFromSerializedJson(*connection, serializedKeys.c_str());
    if (0 == strcmp("nodesExpanded", change.c_str()))
        return AddNodeKeys(rulesetId, keys);
    if (0 == strcmp("nodesCollapsed", change.c_str()))
        return RemoveNodeKeys(rulesetId, keys);

    return ECPresentationResult(ECPresentationStatus::Error, "Uknown hierarchy state change type");
    }
