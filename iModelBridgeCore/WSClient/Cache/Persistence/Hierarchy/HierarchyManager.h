/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Hierarchy/HierarchyManager.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <ECDb/ECDbApi.h>
#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>
#include "IDeleteHandler.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

struct ChangeInfoManager;
struct ObjectInfoManager;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (NEEDS_WORK_PORT_GRA06_ECDbDeleteHandler) // Port 0504 to 06
struct HierarchyManager : public ECDbDeleteHandler
#else
struct HierarchyManager
#endif
    {
    private:
        ECDbAdapterR            m_dbAdapter;
        ECSqlStatementCache*    m_statementCache;
        ObjectInfoManager*      m_objectInfoManager;
        ChangeInfoManager*      m_changeInfoManager;

#if defined (NEEDS_WORK_PORT_GRA06_ECDbDeleteHandler) // Port 0504 to 06
        std::vector<ECInstanceDeleter> m_deleteHandlers;
#endif
    private:
        BentleyStatus DeleteRelationships(ECInstanceKeyCR source, const bvector<ECInstanceKey>& targets, ECRelationshipClassCP relationshipClass);

        //! If instance has no parents, it will be deleted with possible hiearchy cleanup
        BentleyStatus CheckAndCleanupHiearchy(ECInstanceKeyCR instance);
        BentleyStatus GetAdditonalInstancesToDelete(bset<ECInstanceKey>& instancesToDeleteOut);

    public:
        HierarchyManager
            (
            ECDbAdapterR ecdbAdapter,
            ECSqlStatementCache& statementCache,
            ObjectInfoManager& objectInfoManager,
            ChangeInfoManager& changeInfoManager
#if defined (NEEDS_WORK_PORT_GRA06_ECDbDeleteHandler) // Port 0504 to 06
            , std::vector<IDeleteHandler*> deleteHandlers
#endif
            );
        ~HierarchyManager();

#if defined (NEEDS_WORK_PORT_GRA06_ECDbDeleteHandler) // Port 0504 to 06
        //! ECDbDeleteHandler
        virtual void _OnBeforeDelete(ECN::ECClassCR ecClass, ECInstanceId ecInstanceId, ECDbR ecDb) override;
#endif

        ECInstanceKey RelateInstances(ECInstanceKeyCR source, ECInstanceKeyCR target, ECRelationshipClassCP relationshipClass);
        BentleyStatus CopyRelationshipsBySource(ECInstanceKeyCR instanceFrom, ECInstanceKeyCR instanceTo, ECRelationshipClassCP relationshipClass);

        //! Will only delete relationship without affecting source or target
        BentleyStatus DeleteRelationship(ECInstanceKeyCR source, ECInstanceKeyCR target, ECRelationshipClassCP relationshipClass);
        //! Will only delete relationship without affecting source or target
        BentleyStatus DeleteRelationship(ECInstanceKeyCR relationship);

        //! Check if any holding or embedding relationships exists to this instance
        bool IsInstanceHeldByOtherInstances(ECInstanceKeyCR instance);

        //! Delete ECInstance and all held hierarchy of instances if any
        BentleyStatus DeleteInstance(ECInstanceKeyCR instance);
        //! Delete ECInstances and all held hierarchies of instances if any
        BentleyStatus DeleteInstances(bset<ECInstanceKey> instances);
        //! Delete ECInstance and all held hierarchy of instnaes if any. Column 0 - ECClassId, Column 1 - ECInstanceId.
        BentleyStatus DeleteInstances(ECSqlStatement& ecInstanceKeyStatement);

        //! Check for children changes and cleanups obsolete children
        BentleyStatus ReleaseOldChildren
            (
            ECInstanceKeyCR parent,
            const bset<ECInstanceKey>& newChildren,
            ECRelationshipClassCP relationshipClass
            );

        //! Removes child from instance and cleanups obsolete tree. Will succeed and do nothing if relationship does not exist
        BentleyStatus RemoveChildFromParent(ECInstanceKeyCR parent, ECInstanceKeyCR child, ECRelationshipClassCP relationshipClass);

        //! Removes all children from instance and cleanups obsolete trees
        BentleyStatus RemoveAllChildrenFromParent(ECInstanceKeyCR parent, ECRelationshipClassCP relationshipClass);

        BentleyStatus ReadTargetKeys(ECInstanceKeyCR source, ECRelationshipClassCP relationshipClass, bvector<ECInstanceKey>& targetsOut);
        BentleyStatus ReadTargetKeys(ECInstanceKeyCR source, ECRelationshipClassCP relationshipClass, ECInstanceKeyMultiMap& targetsOut);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
