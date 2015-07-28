/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/UpgraderFromV5ToCurrent.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Persistence/DataSourceCache.h>

#include "RawWSObjectsReader.h"
#include "UpgraderBase.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct UpgraderFromV5ToCurrent : private UpgraderBase
    {
    private:
        const CacheEnvironment m_environment;
        BeFileName m_oldCachePath;
        BeFileName m_newCachePath;
        ECSqlStatementCache m_statementCache;

    private:
        BentleyStatus CopySchema(DataSourceCache& newCache);
        BentleyStatus CopyData(DataSourceCache& newCache);
        BentleyStatus SetDefaultServerInfo(DataSourceCache& newCache);

        BentleyStatus CopyInstanceHierarchy
            (
            DataSourceCache& newCache,
            ECInstanceKeyCR newParentKey,
            ObjectIdCR oldParentObjectId,
            ECInstanceKeyCR oldParentKey,
            JsonValueCR oldParentInfo
            );

        ECInstanceKey CopyInstanceToRoot
            (
            DataSourceCache& newCache,
            const RawWSObjectsReader::RawInstance& instance,
            JsonValueCR instanceJson,
            JsonValueCR instanceInfo,
            Utf8StringCR rootName
            );

        BentleyStatus CopyCreatedInstanceToParent
            (
            DataSourceCache& newCache,
            JsonValueCR instanceJson,
            JsonValueCR instanceInfo,
            ECInstanceKeyCR newParentKey,
            ECInstanceKey& newInstanceKey
            );

        BentleyStatus CopyFile
            (
            DataSourceCache& newCache,
            ECInstanceKeyCR newInstanceKey,
            JsonValueCR fileInfo
            );

        BentleyStatus ReadInstanceData
            (
            ECInstanceKeyCR key,
            RawWSObjectsReader::RawInstance& instance,
            JsonValueR instanceJson,
            JsonValueR instanceInfo
            );

        BentleyStatus ReadFileInfo(ECInstanceKeyCR key, JsonValueR fileInfo);
        BentleyStatus ReadSchemaInfo(Utf8String& schemaName);
        BentleyStatus ReadRoots(JsonValueR roots);

        IChangeManager::ChangeStatus ConvertChangeStatus(JsonValueCR oldStatusJson);
        IChangeManager::SyncStatus ConvertSyncStatus(JsonValueCR oldStatusJson);

    public:
        UpgraderFromV5ToCurrent(ECDbAdapter& adapter, CacheEnvironmentCR environment);
        BentleyStatus Upgrade();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
