/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Changes/ChangeInfoManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Persistence/IChangeManager.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>
#include <WebServices/Cache/Util/ValueIncrementor.h>

#include "ChangeInfo.h"
#include "../Files/FileInfoManager.h"
#include "../Hierarchy/HierarchyManager.h"
#include "../Instances/ObjectInfoManager.h"
#include "../Instances/RelationshipInfoManager.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChangeInfoManager
    {
    private:
        IECDbAdapter*                       m_dbAdapter;
        WebServices::ECSqlStatementCache*   m_statementCache;
        HierarchyManager*                   m_hierarchyManager;
        ObjectInfoManager*                  m_objectInfoManager;
        RelationshipInfoManager*            m_relationshipInfoManager;
        FileInfoManager*                    m_fileInfoManager;
        std::shared_ptr<ValueIncrementor>   m_changeNumberIncrementor;

    private:
        BentleyStatus GetObjectChanges(IChangeManager::Changes& changesOut, bool onlyReadyToSync);
        BentleyStatus GetFileChanges(IChangeManager::Changes& changesOut, bool onlyReadyToSync);
        BentleyStatus GetRelationshipChanges(IChangeManager::Changes& changesOut, bool onlyReadyToSync);
        std::shared_ptr<ECSqlStatement> GetPreparedStatementForGetChanges(ECClassCP infoClass, bool onlyReadyToSync);
        int ReadStatusProperty(ECInstanceKeyCR instanceKey, Utf8CP statusPropertyName);

        ECInstanceId FindBackupInstance(ObjectInfoCR info);
        BentleyStatus SaveBackupInstance(ObjectInfoCR info, Utf8CP serializedInstance);

    public:
        ChangeInfoManager
            (
            ECDbAdapter& dbAdapter,
            ECSqlStatementCache& statementCache,
            HierarchyManager& hierarchyManager,
            ObjectInfoManager& objectInfoManager,
            RelationshipInfoManager& relationshipInfoManager,
            FileInfoManager& fileInfoManager
            );

        bool HasChanges();
        BentleyStatus GetChanges(IChangeManager::Changes& changesOut, bool onlyReadyToSync);

        IChangeManager::ObjectChange GetObjectChange(ECInstanceKeyCR instanceKey);
        IChangeManager::RelationshipChange GetRelationshipChange(ECInstanceKeyCR instanceKey);
        IChangeManager::FileChange GetFileChange(ECInstanceKeyCR instanceKey);

        IChangeManager::ObjectChange GetObjectChange(ObjectInfoCR info);
        IChangeManager::RelationshipChange GetRelationshipChange(RelationshipInfoCR info);
        IChangeManager::FileChange GetFileChange(FileInfoCR info);

        IChangeManager::ChangeStatus GetObjectChangeStatus(ECInstanceKeyCR instanceKey);
        IChangeManager::SyncStatus GetObjectSyncStatus(ECInstanceKeyCR instanceKey);

        BentleyStatus SetupChangeNumber(ChangeInfoR info);
        BentleyStatus RemoveLocalDeletedInfos();

        BentleyStatus ReadBackupInstance(ObjectInfoCR info, RapidJsonDocumentR instanceOut);
        BentleyStatus SaveBackupInstance(ObjectInfoCR info, RapidJsonValueCR instance);
        BentleyStatus SaveBackupInstance(ObjectInfoCR info, JsonValueR instance);
        BentleyStatus DeleteBackupInstance(ObjectInfoCR info);

        // Values will not be coppied to changesOut so keep them alive with t1 and t2 until changesOut are used
        BentleyStatus ReadInstanceChanges(ObjectInfoCR info, RapidJsonDocumentR changesOut, RapidJsonDocumentR temp1, RapidJsonDocumentR temp2);
        BentleyStatus ApplyChangesToBackup(ObjectInfoCR info, JsonValueCR changes);
        BentleyStatus ApplyChangesToInstanceAndBackupIt(ObjectInfoCR info, JsonValueCR changes);
    };

typedef ChangeInfoManager& ChangeInfoManagerR;
typedef const ChangeInfoManager& ChangeInfoManagerCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
