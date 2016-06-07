/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/ChangesGraph.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Persistence/ChangeManager.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef std::shared_ptr<struct ChangeGroup> ChangeGroupPtr;
typedef struct ChangeGroup& ChangeGroupR;
typedef const struct ChangeGroup& ChangeGroupCR;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChangesGraph
    {
    private:
        const bvector<const ChangeManager::RelationshipChange*> m_constEmptyRelationshipChanges;

        ChangeManager::ChangesCR m_changes;
        bmap<ECInstanceKey, bvector<const ChangeManager::RelationshipChange*>> m_changedObjectsToRelationships;

    private:
        const ChangeManager::ObjectChange* FindObjectChange(ECInstanceKeyCR instanceKey) const;
        const bvector<const ChangeManager::RelationshipChange*>& FindChangedRelationships(ECInstanceKeyCR instanceKey) const;

        void AddRelationshipChangeToGroup(ChangeGroup& changeGroup, ECInstanceKeyCR endInstanceKey, bset<ECInstanceKey>& handledChanges) const;
        bool DoesObjectNeedHandling(ECInstanceKeyCR instanceKey, bset<ECInstanceKey>& handledChanges) const;

        void SetupDependencies(const bvector<ChangeGroupPtr>& changeGroupsInOut) const;
        void SetupDependenciesForRelationship
            (
            const bvector<ChangeGroupPtr>& changeGroups,
            ChangeGroupPtr relationshipChangeGroup
            ) const;

    public:
        WSCACHE_EXPORT ChangesGraph(ChangeManager::ChangesCR changes);
        WSCACHE_EXPORT bvector<ChangeGroupPtr> BuildChangeGroups();
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChangeGroup
    {
    private:
        bool m_isSynced;
        ChangeManager::ObjectChange m_objectChange;
        ChangeManager::RelationshipChange m_relationshipChange;
        ChangeManager::FileChange m_fileChange;
        bset<ChangeGroupPtr> m_dependsOn;

    public:
        WSCACHE_EXPORT ChangeGroup();

        WSCACHE_EXPORT ChangeManager::ObjectChangeCR GetObjectChange() const;
        WSCACHE_EXPORT ChangeManager::ObjectChangeR GetObjectChange();
        WSCACHE_EXPORT void SetObjectChange(ChangeManager::ObjectChangeCR change);

        WSCACHE_EXPORT ChangeManager::RelationshipChangeCR GetRelationshipChange() const;
        WSCACHE_EXPORT ChangeManager::RelationshipChangeR GetRelationshipChange();
        WSCACHE_EXPORT void SetRelationshipChange(ChangeManager::RelationshipChangeCR change);

        WSCACHE_EXPORT ChangeManager::FileChangeCR GetFileChange() const;
        WSCACHE_EXPORT ChangeManager::FileChangeR GetFileChange();
        WSCACHE_EXPORT void SetFileChange(ChangeManager::FileChangeCR change);

        WSCACHE_EXPORT bool DoesDependOn(ChangeGroupPtr other) const;
        WSCACHE_EXPORT void AddDependency(ChangeGroupPtr other);
        WSCACHE_EXPORT bool AreAllDependenciesSynced() const;

        WSCACHE_EXPORT void SetSynced(bool isSynced);
        WSCACHE_EXPORT bool IsSynced() const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
