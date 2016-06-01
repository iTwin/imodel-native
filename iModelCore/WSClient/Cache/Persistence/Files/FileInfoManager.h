/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Files/FileInfoManager.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Persistence/CacheEnvironment.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>

#include "../Instances/ObjectInfoManager.h"
#include "../Hierarchy/IDeleteHandler.h"
#include "FileStorage.h"
#include "FileInfo.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

struct HierarchyManager;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileInfoManager : public IDeleteHandler, public FileInfo::IAbsolutePathProvider
    {
    protected:
        ECDbAdapter*            m_dbAdapter;
        ECSqlStatementCache*    m_statementCache;
        FileStorage*            m_fileStorage;

        HierarchyManager*       m_hierarchyManager;
        ObjectInfoManager*      m_objectInfoManager;

        ECClassCP               m_infoClass;
        ECRelationshipClassCP   m_infoRelationshipClass;
        ECClassCP               m_externalFileInfoClass;
        ECRelationshipClassCP   m_externalFileInfoRelationshipClass;

        ECSqlAdapterLoader<JsonInserter>    m_cachedInfoInserter;
        ECSqlAdapterLoader<JsonUpdater>     m_cachedInfoUpdater;
        ECSqlAdapterLoader<JsonInserter>    m_externalInfoInserter;
        ECSqlAdapterLoader<JsonUpdater>     m_externalInfoUpdater;

    private:
        ECInstanceKey FindTargetInstanceKeyForInfo(ECInstanceId infoId);
        Json::Value ReadExternalFileInfo(ECInstanceKeyCR instanceKey);
        Json::Value ReadCachedInfoJson(ECInstanceKeyCR instanceKey);

    public:
        FileInfoManager
            (
            ECDbAdapter& dbAdapter,
            ECSqlStatementCache& statementCache,
            FileStorage& fileStorage,
            ObjectInfoManager& objectInfoManager,
            HierarchyManager& hierarchyManager
            );

        ECClassCP GetInfoClass() const;
        ECRelationshipClassCP GetInfoRelationshipClass() const;

        FileInfo ReadInfo(ECInstanceKeyCR fileKey);
        FileInfo ReadInfo(ObjectIdCR fileId);
        FileInfo ReadInfo(JsonValueCR infoJson);

        BentleyStatus SaveInfo(FileInfoR info);

        BentleyStatus DeleteFilesNotHeldByInstances(const ECInstanceKeyMultiMap& holdingInstances);

        BeFileName GetAbsoluteFilePath(FileCache location, BeFileNameCR relativePath) const override;

        BeFileName ReadFilePath(ECInstanceKeyCR instance);

        //! IDeleteHandler
        BentleyStatus OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId) override;
        BentleyStatus OnAfterDelete(bset<ECInstanceKey>& instancesToDeleteOut) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
