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
#include "FileStorage.h"
#include "FileInfo.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

struct HierarchyManager;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileInfoManager : public IECDbAdapter::DeleteListener, public FileInfo::IAbsolutePathProvider
    {
    protected:
        ECDbAdapter&            m_dbAdapter;
        ECSqlStatementCache&    m_statementCache;
        FileStorage&            m_fileStorage;

        HierarchyManager&       m_hierarchyManager;
        ObjectInfoManager&      m_objectInfoManager;

        ECClassCP               m_cachedFileInfoClass;
        ECRelationshipClassCP   m_objectInfoToCachedFileInfoClass;

        ECRelationshipClassCP   m_cachedFileInfoToFileInfoClass;
        ECRelationshipClassCP   m_cachedFileInfoToFileInfoOwnershipClass;

        ECClassCP               m_externalFileInfoClass;
        ECClassCP               m_externalFileInfoOwnershipClass;

        ECSqlAdapterLoader<JsonInserter>    m_cachedFileInfoInserter;
        ECSqlAdapterLoader<JsonUpdater>     m_cachedFileInfoUpdater;
        ECSqlAdapterLoader<JsonInserter>    m_externalFileInfoInserter;
        ECSqlAdapterLoader<JsonUpdater>     m_externalFileInfoUpdater;

    private:
        Json::Value ReadCachedFileInfo(CachedInstanceKeyCR cachedKey);
        Json::Value ReadExternalFileInfo(CachedInstanceKeyCR cachedKey);
        ECInstanceKey InsertFileInfoOwnership(ECInstanceKeyCR ownerKey, ECInstanceKeyCR fileInfoKey);
        BentleyStatus CleanupExternalFile(JsonValueCR externalFileInfoJson);

    public:
        FileInfoManager
            (
            ECDbAdapter& dbAdapter,
            WebServices::ECSqlStatementCache& statementCache,
            FileStorage& fileStorage,
            ObjectInfoManager& objectInfoManager,
            HierarchyManager& hierarchyManager
            );

        ECClassCP GetInfoClass() const;

        FileInfo ReadInfo(ObjectIdCR objectId);
        FileInfo ReadInfo(CachedInstanceKeyCR cachedKey);
        FileInfo ReadInfo(JsonValueCR infoJson);

        BentleyStatus SaveInfo(FileInfoR info);

        BentleyStatus DeleteFilesNotHeldByNodes(const ECInstanceKeyMultiMap& holdingNodes);

        BeFileName ReadFilePath(CachedInstanceKeyCR cachedKey);

        // FileInfo::IAbsolutePathProvider
        BeFileName GetAbsoluteFilePath(FileCache location, BeFileNameCR relativePath) const override;

        //! IECDbAdapter::DeleteListener
        BentleyStatus OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId, bset<ECInstanceKey>& additionalInstancesOut) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
