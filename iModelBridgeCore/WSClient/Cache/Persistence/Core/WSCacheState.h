/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Core/WSCacheState.h $
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
#include "../Files/FileStorage.h"
#include "../Hierarchy/HierarchyManager.h"
#include "../Hierarchy/RootManager.h"
#include "../Instances/NavigationBaseManager.h"
#include "../Instances/ObjectInfoManager.h"
#include "../Instances/RelationshipInfoManager.h"
#include "../Instances/InstanceCacheHelper.h"
#include "../Responses/CachedResponseManager.h"
#include "../../Logging.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSCacheState : public IECDbSchemaChangeListener
    {
    private:
        struct Core
            {
            private:
                ECSchemaCP cacheSchema = nullptr;

            public:
                Core(ObservableECDb& db, CacheEnvironmentCR environment);
                ECSchemaCP GetCacheSchema();

                ECDbAdapter             dbAdapter;
                ECSqlStatementCache     statementCache;
                ExtendedDataAdapter     extendedDataAdapter;

                CacheEnvironmentCR      environment;

                ObjectInfoManager       objectInfoManager;
                RelationshipInfoManager relationshipInfoManager;
                FileInfoManager         fileInfoManager;
                CachedResponseManager   responseManager;
                NavigationBaseManager   navigationBaseManager;
                HierarchyManager        hierarchyManager;
                RootManager             rootManager;
                InstanceCacheHelper     instanceCacheHelper;
                ChangeInfoManager       changeInfoManager;
                FileStorage             fileStorage;
                ChangeManager           changeManager;
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
        WSCacheState(const WSCacheState&) = delete;
        WSCacheState(ObservableECDb& db, CacheEnvironmentCR environment);
        ~WSCacheState();

        virtual void OnSchemaChanged() override; // IECDbSchemaChangeListener
        void ClearRuntimeCaches();

        ECDbAdapter& GetECDbAdapter()
            {
            return GetCore().dbAdapter;
            }
        ExtendedDataAdapter& GetExtendedDataAdapter()
            {
            return GetCore().extendedDataAdapter;
            }
        CacheEnvironmentCR GetFileCacheEnvironment()
            {
            return GetCore().environment;
            }
        ECSchemaCP GetCacheSchema()
            {
            return GetCore().GetCacheSchema();
            }
        ObjectInfoManager& GetObjectInfoManager()
            {
            return GetCore().objectInfoManager;
            }
        RelationshipInfoManager& GetRelationshipInfoManager()
            {
            return GetCore().relationshipInfoManager;
            }
        FileInfoManager& GetFileInfoManager()
            {
            return GetCore().fileInfoManager;
            }
        CachedResponseManager& GetCachedResponseManager()
            {
            return GetCore().responseManager;
            }
        FileStorage& GetFileStorage()
            {
            return GetCore().fileStorage;
            }
        NavigationBaseManager& GetNavigationBaseManager()
            {
            return GetCore().navigationBaseManager;
            }
        HierarchyManager& GetHierarchyManager()
            {
            return GetCore().hierarchyManager;
            }
        RootManager& GetRootManager()
            {
            return GetCore().rootManager;
            }
        ChangeInfoManager& GetChangeInfoManager()
            {
            return GetCore().changeInfoManager;
            }
        ChangeManager& GetChangeManager()
            {
            return GetCore().changeManager;
            }
        InstanceCacheHelper& GetInstanceHelper()
            {
            return GetCore().instanceCacheHelper;
            }
        ECSqlStatementCache& GetStatementCache()
            {
            return GetCore().statementCache;
            }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
