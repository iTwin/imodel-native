/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/ChangesGraph.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ChangesGraph.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ChangesGraph::ChangesGraph(ChangeManager::ChangesCR changes) :
m_changes(changes)
    {
    for (auto& objChange : changes.GetObjectChanges())
        {
        auto& relationships = m_changedObjectsToRelationships[objChange.GetInstanceKey()];

        for (auto& relChange : changes.GetRelationshipChanges())
            {
            if (objChange.GetInstanceKey() == relChange.GetSourceKey() ||
                objChange.GetInstanceKey() == relChange.GetTargetKey())
                {
                relationships.push_back(&relChange);
                }
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<CacheChangeGroupPtr> ChangesGraph::BuildCacheChangeGroups()
    {
    bset<ECInstanceKey> handledChanges;
    bset<ECInstanceKey> handledFileChanges;

    bvector<CacheChangeGroupPtr> groups;

    for (auto& objectChange : m_changes.GetObjectChanges())
        {
        if (handledChanges.find(objectChange.GetInstanceKey()) != handledChanges.end())
            {
            continue;
            }
        handledChanges.insert(objectChange.GetInstanceKey());

        auto changeGroup = std::make_shared<CacheChangeGroup>();
        changeGroup->SetObjectChange(objectChange);

        if (objectChange.GetChangeStatus() == IChangeManager::ChangeStatus::Created)
            {
            AddRelationshipChangeToGroup(*changeGroup, objectChange.GetInstanceKey(), handledChanges);

            for (auto& fileChange : m_changes.GetFileChanges())
                {
                if (handledFileChanges.find(fileChange.GetInstanceKey()) != handledFileChanges.end())
                    {
                    continue;
                    }

                if (objectChange.GetInstanceKey() == fileChange.GetInstanceKey())
                    {
                    handledFileChanges.insert(fileChange.GetInstanceKey());
                    changeGroup->SetFileChange(fileChange);
                    }
                }
            }

        groups.push_back(changeGroup);
        }

    for (auto& relationshipChange : m_changes.GetRelationshipChanges())
        {
        if (handledChanges.find(relationshipChange.GetInstanceKey()) != handledChanges.end())
            {
            continue;
            }
        handledChanges.insert(relationshipChange.GetInstanceKey());

        auto changeGroup = std::make_shared<CacheChangeGroup>();
        changeGroup->SetRelationshipChange(relationshipChange);
        groups.push_back(changeGroup);
        }

    for (auto& fileChange : m_changes.GetFileChanges())
        {
        if (handledFileChanges.find(fileChange.GetInstanceKey()) != handledFileChanges.end())
            {
            continue;
            }
        handledFileChanges.insert(fileChange.GetInstanceKey());

        auto changeGroup = std::make_shared<CacheChangeGroup>();
        changeGroup->SetFileChange(fileChange);
        groups.push_back(changeGroup);
        }

    SetupDependencies(groups);

    return groups;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangesGraph::AddRelationshipChangeToGroup(CacheChangeGroup& changeGroup, ECInstanceKeyCR endInstanceKey, bset<ECInstanceKey>& handledChanges) const
    {
    const ChangeManager::RelationshipChange* relationshipChange = nullptr;

    // Find not handled relationship change that has smallest change number
    for (auto& candidateRelationshipChange : FindChangedRelationships(endInstanceKey))
        {
        if (handledChanges.find(candidateRelationshipChange->GetInstanceKey()) != handledChanges.end())
            {
            continue;
            }

        if (DoesObjectNeedHandling(candidateRelationshipChange->GetSourceKey(), handledChanges))
            {
            continue;
            }

        if (DoesObjectNeedHandling(candidateRelationshipChange->GetTargetKey(), handledChanges))
            {
            continue;
            }

        if (nullptr == relationshipChange ||
            candidateRelationshipChange->GetChangeNumber() < relationshipChange->GetChangeNumber())
            {
            relationshipChange = candidateRelationshipChange;
            }
        }

    if (nullptr != relationshipChange)
        {
        handledChanges.insert(relationshipChange->GetInstanceKey());
        changeGroup.SetRelationshipChange(*relationshipChange);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangesGraph::DoesObjectNeedHandling(ECInstanceKeyCR instanceKey, bset<ECInstanceKey>& handledChanges) const
    {
    auto* change = FindObjectChange(instanceKey);
    if (nullptr == change)
        {
        // Object not changed
        return false;
        }

    if (handledChanges.find(instanceKey) != handledChanges.end())
        {
        // Object handled
        return false;
        }

    return true;
    }
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<const ChangeManager::RelationshipChange*>& ChangesGraph::FindChangedRelationships(ECInstanceKeyCR instanceKey) const
    {
    auto it = m_changedObjectsToRelationships.find(instanceKey);
    if (it == m_changedObjectsToRelationships.end() || it->second.empty())
        {
        return m_constEmptyRelationshipChanges;
        }
    return it->second;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const ChangeManager::ObjectChange* ChangesGraph::FindObjectChange(ECInstanceKeyCR instanceKey) const
    {
    auto it = std::find_if(m_changes.GetObjectChanges().begin(), m_changes.GetObjectChanges().end(), [&] (ChangeManager::ObjectChangeCR change)
        {
        return change.GetInstanceKey() == instanceKey;
        });

    if (it == m_changes.GetObjectChanges().end())
        {
        return nullptr;
        }

    return &*it;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangesGraph::SetupDependencies(const bvector<CacheChangeGroupPtr>& changeGroups) const
    {
    for (auto changeGroup : changeGroups)
        {
        if (changeGroup->GetRelationshipChange().GetChangeStatus() == IChangeManager::ChangeStatus::Created)
            {
            SetupDependenciesForRelationship(changeGroups, changeGroup);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangesGraph::SetupDependenciesForRelationship
(
const bvector<CacheChangeGroupPtr>& changeGroups,
CacheChangeGroupPtr relationshipCacheChangeGroup
) const
    {
    for (auto changeGroup : changeGroups)
        {
        if (changeGroup == relationshipCacheChangeGroup)
            {
            continue;
            }
        if (changeGroup->GetObjectChange().GetChangeStatus() != IChangeManager::ChangeStatus::Created)
            {
            continue;
            }
        if (changeGroup->GetObjectChange().GetInstanceKey() == relationshipCacheChangeGroup->GetRelationshipChange().GetSourceKey() ||
            changeGroup->GetObjectChange().GetInstanceKey() == relationshipCacheChangeGroup->GetRelationshipChange().GetTargetKey())
            {
            relationshipCacheChangeGroup->AddDependency(changeGroup);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheChangeGroup::CacheChangeGroup() :
m_isSynced(false)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeManager::ObjectChangeCR CacheChangeGroup::GetObjectChange() const
    {
    return m_objectChange;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeManager::ObjectChangeR CacheChangeGroup::GetObjectChange()
    {
    return m_objectChange;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheChangeGroup::SetObjectChange(ChangeManager::ObjectChangeCR change)
    {
    m_objectChange = change;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeManager::RelationshipChangeCR CacheChangeGroup::GetRelationshipChange() const
    {
    return m_relationshipChange;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeManager::RelationshipChangeR CacheChangeGroup::GetRelationshipChange()
    {
    return m_relationshipChange;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheChangeGroup::SetRelationshipChange(ChangeManager::RelationshipChangeCR change)
    {
    m_relationshipChange = change;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeManager::FileChangeCR CacheChangeGroup::GetFileChange() const
    {
    return m_fileChange;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeManager::FileChangeR CacheChangeGroup::GetFileChange()
    {
    return m_fileChange;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheChangeGroup::SetFileChange(ChangeManager::FileChangeCR change)
    {
    m_fileChange = change;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CacheChangeGroup::DoesContain(ECInstanceKeyCR instanceKey) const
    {
    if (m_objectChange.GetInstanceKey() == instanceKey)
        return true;
    if (m_relationshipChange.GetInstanceKey() == instanceKey)
        return true;
    if (m_fileChange.GetInstanceKey() == instanceKey)
        return true;
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheChangeGroup::AddDependency(CacheChangeGroupPtr other)
    {
    m_dependsOn.insert(other);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CacheChangeGroup::DoesDependOn(CacheChangeGroupPtr other) const
    {
    if (nullptr == other)
        {
        return false;
        }
    auto it = std::find(m_dependsOn.begin(), m_dependsOn.end(), other);
    if (it == m_dependsOn.end())
        {
        return false;
        }
    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CacheChangeGroup::AreAllDependenciesSynced() const
    {
    for (auto& dependency : m_dependsOn)
        {
        if (!dependency->m_isSynced)
            {
            return false;
            }
        }
    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CacheChangeGroup::AreAllUnsyncedDependenciesInSet(const bset<ChangeGroup*>& set)
    {
    for (ChangeGroupPtr& dependency : m_dependsOn)
        {
        if (dependency->IsSynced())
            continue;
        if (!set.count(dependency.get()))
            return false;
        }
    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheChangeGroup::SetSynced(bool isSynced)
    {
    m_isSynced = isSynced;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool CacheChangeGroup::IsSynced() const
    {
    return m_isSynced;
    }
