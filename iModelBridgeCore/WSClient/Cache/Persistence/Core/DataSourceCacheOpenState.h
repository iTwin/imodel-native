/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Core/DataSourceCacheOpenState.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>

#include <WebServices/Cache/Persistence/CacheEnvironment.h>
#include <WebServices/Cache/Persistence/ChangeManager.h>
#include <WebServices/Cache/Util/ObservableECDb.h>
#include <WebServices/Cache/Util/ExtendedDataAdapter.h>

#include "../Changes/ChangeInfoManager.h"
#include "../Files/FileInfoManager.h"
#include "../Files/FileCacheManager.h"
#include "../Hierarchy/HierarchyManager.h"
#include "../Hierarchy/RootManager.h"
#include "../Instances/NavigationBaseManager.h"
#include "../Instances/ObjectInfoManager.h"
#include "../Instances/RelationshipInfoManager.h"
#include "../Instances/InstanceCacheHelper.h"
#include "../Responses/CachedResponseManager.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataSourceCacheOpenState : public IECDbSchemaChangeListener
    {
    private:
        struct Core
            {
            Core(ObservableECDb& db, CacheEnvironmentCR environment);

            ECDbAdapter                 m_dbAdapter;
            ECSqlStatementCache         m_statementCache;

            ObjectInfoManager           m_objectInfoManager;
            RelationshipInfoManager     m_relationshipInfoManager;
            FileInfoManager             m_fileInfoManager;
            CachedResponseManager       m_cachedQueryManager;
            NavigationBaseManager       m_navigationBaseManager;
            HierarchyManager            m_hierarchyManager;
            RootManager                 m_rootManager;
            InstanceCacheHelper         m_instanceHelper;
            ChangeInfoManager           m_changeInfoManager;
            ChangeManager               m_changeManager;
            FileCacheManager            m_fileCacheManager;

            ECSchemaCP                  m_cacheSchema;

            ExtendedDataAdapter         m_extendedDataAdapter;
            };

    private:
        ObservableECDb& m_db;
        CacheEnvironment m_environment;

        bool m_isSyncActive;
        std::shared_ptr<Core> m_core;

    private:
        Core& GetCore();
        void ResetCore();

    public:
        DataSourceCacheOpenState(ObservableECDb& db, CacheEnvironmentCR environment);
        ~DataSourceCacheOpenState();

        virtual void OnSchemaChanged() override; // IECDbSchemaChangeListener
        void ClearRuntimeCaches();
        ECSchemaCP GetCacheSchema();

        ECDbAdapter& GetECDbAdapter()
            {
            return GetCore().m_dbAdapter;
            }

        ExtendedDataAdapter& GetExtendedDataAdapter()
            {
            return GetCore().m_extendedDataAdapter;
            }

        CacheEnvironmentCR GetFileCacheEnvironment()
            {
            return m_environment;
            }
        ObjectInfoManager& GetObjectInfoManager()
            {
            return GetCore().m_objectInfoManager;
            }
        RelationshipInfoManager& GetRelationshipInfoManager()
            {
            return GetCore().m_relationshipInfoManager;
            }
        FileInfoManager& GetFileInfoManager()
            {
            return GetCore().m_fileInfoManager;
            }
        CachedResponseManager& GetCachedResponseManager()
            {
            return GetCore().m_cachedQueryManager;
            }
        FileCacheManager& GetFileCacheManager()
            {
            return GetCore().m_fileCacheManager;
            }
        NavigationBaseManager& GetNavigationBaseManager()
            {
            return GetCore().m_navigationBaseManager;
            }
        HierarchyManager& GetHierarchyManager()
            {
            return GetCore().m_hierarchyManager;
            }
        RootManager& GetRootManager()
            {
            return GetCore().m_rootManager;
            }
        ChangeInfoManager& GetChangeInfoManager()
            {
            return GetCore().m_changeInfoManager;
            }
        ChangeManager& GetChangeManager()
            {
            return GetCore().m_changeManager;
            }
        InstanceCacheHelper& GetInstanceHelper()
            {
            return GetCore().m_instanceHelper;
            }
        ECSqlStatementCache& GetStatementCache()
            {
            return GetCore().m_statementCache;
            }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
