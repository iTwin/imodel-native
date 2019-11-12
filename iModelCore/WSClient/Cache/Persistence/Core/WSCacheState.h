/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>

#include <WebServices/Cache/Persistence/CacheEnvironment.h>
#include <WebServices/Cache/Persistence/ChangeManager.h>
#include <WebServices/Cache/Persistence/FileManager.h>
#include <WebServices/Cache/Util/ObservableECDb.h>
#include <WebServices/Cache/Util/ExtendedDataAdapter.h>

#include "../Changes/ChangeInfoManager.h"
#include "../Files/FileInfoManager.h"
#include "../Files/FileStorage.h"
#include "../Hierarchy/ExtendedDataDelegate.h"
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
                Core(ObservableECDb& db, CacheEnvironmentCR environment, ECInstanceKeyMultiMap& syncKeys, IFileManager& fileManager);
                ECSchemaCP GetCacheSchema();

                IFileManager&           fileManager;

                ECDbAdapter             dbAdapter;
                ECSqlStatementCache     statementCache;

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

                ExtendedDataDelegate    extendedDataDelegate;
                ExtendedDataAdapter     extendedDataAdapter;
            };

    private:
        ObservableECDb& m_db;
        CacheEnvironment m_environment;
        IFileManager& m_fileManager;

        std::shared_ptr<Core> m_core;
        ECInstanceKeyMultiMap m_activeSyncKeys;

    private:
        Core& GetCore();
        void ResetCore();

    public:
        WSCacheState(const WSCacheState&) = delete;
        WSCacheState(ObservableECDb& db, CacheEnvironmentCR environment, IFileManager& fileManager);
        virtual ~WSCacheState();

        virtual void OnSchemaChanged() override; // IECDbSchemaChangeListener
        void ClearRuntimeCaches();

        ECDbAdapter& GetECDbAdapter()
            {
            return GetCore().dbAdapter;
            }
        IFileManager& GetFileManager()
            {
            return GetCore().fileManager;
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
