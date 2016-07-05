/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Hierarchy/HierarchyManager.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <ECDb/ECDbApi.h>
#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>
#include "../Instances/CachedInstanceKey.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

struct ChangeInfoManager;
struct ObjectInfoManager;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct HierarchyManager
    {
    private:
        ECDbAdapter&            m_dbAdapter;
        ECSqlStatementCache&    m_statementCache;

    private:
        BentleyStatus DeleteRelationships(ECInstanceKeyCR source, const bvector<ECInstanceKey>& targets, ECRelationshipClassCP relationshipClass);

    public:
        HierarchyManager
            (
            ECDbAdapterR ecdbAdapter,
            ECSqlStatementCache& statementCache
            );
        ~HierarchyManager();

        ECInstanceKey RelateInstances(ECInstanceKeyCR source, ECInstanceKeyCR target, ECRelationshipClassCP relationshipClass);
        BentleyStatus CopyRelationshipsBySource(ECInstanceKeyCR instanceFrom, ECInstanceKeyCR instanceTo, ECRelationshipClassCP relationshipClass);

        BentleyStatus RelateCachedInstancesToHolder
            (
            CacheNodeKeyCR holder,
            ECRelationshipClassCP holderToInfoRelClass,
            const bset<CachedInstanceKey>& cachedInstances
            );

        BentleyStatus RemoveCachedInstancesFromHolder
            (
            CacheNodeKeyCR holder,
            ECRelationshipClassCP holderToInfoRelClass,
            const bset<CachedInstanceKey>& cachedInstances
            );

        //! Will only delete relationship without affecting source or target
        BentleyStatus DeleteRelationship(ECInstanceKeyCR source, ECInstanceKeyCR target, ECRelationshipClassCP relationshipClass);
        //! Will only delete relationship without affecting source or target
        BentleyStatus DeleteRelationship(ECInstanceKeyCR relationship);

        //! Check if any holding or embedding relationships exists to this instance
        bool IsInstanceHeldByOtherInstances(ECInstanceKeyCR instance);

        //! Delete ECInstance and all held hierarchy of instances if any
        BentleyStatus DeleteInstance(ECInstanceKeyCR instance);
        //! Delete cached instance and all metadata
        BentleyStatus DeleteInstance(CachedInstanceKeyCR instance);
        //! Delete ECInstances and all held hierarchies of instances if any
        BentleyStatus DeleteInstances(bset<ECInstanceKey> instances);
        //! Delete ECInstance and all held hierarchy of instnaes if any. Column 0 - ECClassId, Column 1 - ECInstanceId.
        BentleyStatus DeleteInstances(ECSqlStatement& ecInstanceKeyStatement);

        //! If instance has no parents, it will be deleted with possible hiearchy cleanup
        BentleyStatus CheckAndCleanupHiearchy(ECInstanceKeyCR instance);

        //! Removes child from instance and cleanups obsolete tree. Will succeed and do nothing if relationship does not exist
        BentleyStatus RemoveChildFromParent(ECInstanceKeyCR parent, ECInstanceKeyCR child, ECRelationshipClassCP relationshipClass);

        //! Removes all children from instance and cleanups obsolete trees
        BentleyStatus RemoveAllChildrenFromParent(ECInstanceKeyCR parent, ECRelationshipClassCP relationshipClass);

        BentleyStatus ReadTargetKeys(ECInstanceKeyCR source, ECRelationshipClassCP relationshipClass, bvector<ECInstanceKey>& targetsOut);
        BentleyStatus ReadTargetKeys(ECInstanceKeyCR source, ECRelationshipClassCP relationshipClass, ECInstanceKeyMultiMap& targetsOut);

        BentleyStatus ReadSourceKeys(ECInstanceKeyCR target, ECRelationshipClassCP relationshipClass, bvector<ECInstanceKey>& sourcesOut);
        BentleyStatus ReadSourceKeys(ECInstanceKeyCR target, ECRelationshipClassCP relationshipClass, ECInstanceKeyMultiMap& sourcesOut);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
