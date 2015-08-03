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
        WebServices::ECSqlStatementCache*   m_statementCache;
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

    public:
        ChangeInfoManager
            (
            ECDbAdapter& dbAdapter,
            ECSqlStatementCache& statementCache,
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
    };

typedef ChangeInfoManager& ChangeInfoManagerR;
typedef const ChangeInfoManager& ChangeInfoManagerCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
