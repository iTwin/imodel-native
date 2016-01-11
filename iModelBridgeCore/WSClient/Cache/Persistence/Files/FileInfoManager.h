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

        ECClassCP               m_infoClass;
        ECRelationshipClassCP   m_infoRelationshipClass;
        ECClassCP               m_externalFileInfoClass;
        ECRelationshipClassCP   m_externalFileInfoRelationshipClass;

        ECSqlAdapterLoader<JsonInserter>    m_cachedInfoInserter;
        ECSqlAdapterLoader<JsonUpdater>     m_cachedInfoUpdater;
        ECSqlAdapterLoader<JsonInserter>    m_externalInfoInserter;
        ECSqlAdapterLoader<JsonUpdater>     m_externalInfoUpdater;

    private:
        Json::Value ReadExternalFileInfo(CachedInstanceKeyCR cachedKey);
        Json::Value ReadCachedInfoJson(CachedInstanceKeyCR cachedKey);

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

        BeFileName GetAbsoluteFilePath(bool isPersistent, BeFileNameCR relativePath) const override;
        BeFileName ReadFilePath(CachedInstanceKeyCR infoKey);

        //! IECDbAdapter::DeleteListener
        BentleyStatus OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId, bset<ECInstanceKey>& additionalInstancesOut) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
