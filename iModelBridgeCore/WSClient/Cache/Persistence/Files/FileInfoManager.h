/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Persistence/CacheEnvironment.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>
#include <Bentley/Tasks/AsyncError.h>

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

        ECClassCP               m_staleFileInfoClass;
        ECRelationshipClassCP   m_staleFileInfoToFileInfoClass;

        ECClassCP               m_externalFileInfoClass;
        ECClassCP               m_externalFileInfoOwnershipClass;

        ECSqlAdapterLoader<JsonInserter>                        m_cachedFileInfoInserter;
        ECSqlAdapterLoader<JsonUpdater, JsonUpdater::Options>   m_cachedFileInfoUpdater;
        ECSqlAdapterLoader<JsonInserter>                        m_staleFileInfoInserter;
        ECSqlAdapterLoader<JsonInserter>                        m_externalFileInfoInserter;
        ECSqlAdapterLoader<JsonUpdater, JsonUpdater::Options>   m_externalFileInfoUpdater;

    private:
        Json::Value ReadCachedFileInfo(CachedInstanceKeyCR cachedKey);
        Json::Value ReadExternalFileInfo(CachedInstanceKeyCR cachedKey);
        ECInstanceKey InsertFileInfoOwnership(ECInstanceKeyCR ownerKey, ECInstanceKeyCR fileInfoKey);
        BentleyStatus CheckMaxLastAccessDate(BeFileNameCR fileName, DateTimeCP maxLastAccessDate, bool &shouldSkip);
        BentleyStatus RemoveFileGracefully(FileInfoR info);
        BentleyStatus InsertStaleFileInfo(FileInfoCR info);

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

        CacheStatus DeleteFilesNotHeldByNodes
            (
            const ECInstanceKeyMultiMap& holdingNodes,
            DateTimeCP maxLastAccessDate = nullptr
            );

        BeFileName ReadFilePath(CachedInstanceKeyCR cachedKey);

        BentleyStatus ReadStaleFilePaths(bset<BeFileName>& pathsOut);
        BentleyStatus CleanupStaleFiles();

        // FileInfo::IAbsolutePathProvider
        BeFileName GetAbsoluteFilePath(FileCache location, BeFileNameCR relativePath) const override;

        //! IECDbAdapter::DeleteListener
        BentleyStatus OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId, bset<ECInstanceKey>& additionalInstancesOut) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
