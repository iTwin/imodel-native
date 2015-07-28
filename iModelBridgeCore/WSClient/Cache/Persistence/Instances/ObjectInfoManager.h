/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Instances/ObjectInfoManager.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>
#include "ObjectInfo.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

struct HierarchyManager;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ObjectInfoManager
    {
    private:
        ECDbAdapter*            m_dbAdapter;
        ECSqlStatementCache*    m_statementCache;
        HierarchyManager*       m_hierarchyManager;

        ECClassCP               m_infoClass;
        ECRelationshipClassCP   m_infoRelationshipClass;

        ECSqlAdapterLoader<JsonInserter>    m_infoInserter;
        ECSqlAdapterLoader<JsonUpdater>     m_infoUpdater;

    public:
        ObjectInfoManager
            (
            ECDbAdapter& dbAdapter,
            ECSqlStatementCache& statementCache,
            HierarchyManager& hierarchyManager
            );

        ECClassCP GetInfoClass() const;
        ECRelationshipClassCP GetInfoRelationshipClass() const;

        BentleyStatus InsertInfo(ObjectInfoR info);
        BentleyStatus UpdateInfo(ObjectInfoCR info);

        ObjectInfo ReadInfo(ObjectIdCR objectId);
        ObjectInfo ReadInfo(ECClassCR ecClass, Utf8StringCR remoteId);
        ObjectInfo ReadInfo(ECInstanceKeyCR instance);
        ObjectInfo ReadInfo(JsonValueCR infoJson);

        ECInstanceKey FindCachedInstance(ObjectIdCR objectId);
        ECInstanceKey FindCachedInstance(ECClassCP ecClass, Utf8StringCR remoteId);
        ObjectId FindCachedInstance(ECInstanceKeyCR instance);

        ECInstanceKey ReadInfoKey(ObjectIdCR objectId);

        BentleyStatus DeleteInstanceLeavingInfo(ObjectInfoR info);
        BentleyStatus RemoveAllCachedInstances();
    };

typedef const ObjectInfoManager& ObjectInfoManagerCR;
typedef ObjectInfoManager& ObjectInfoManagerR;

END_BENTLEY_WEBSERVICES_NAMESPACE
