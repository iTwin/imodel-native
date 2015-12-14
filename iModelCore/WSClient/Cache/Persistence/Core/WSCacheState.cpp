/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Core/WSCacheState.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WSCacheState.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "CacheSchema.h"
#include "ECDbFileInfoSchema.h"

#include "CacheSettings.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSCacheState::Core::Core(ObservableECDb& db, CacheEnvironmentCR environment) :
dbAdapter(db),
statementCache(db),
environment(environment),
extendedDataAdapter(db),

objectInfoManager(dbAdapter, statementCache, hierarchyManager),
relationshipInfoManager(dbAdapter, statementCache, hierarchyManager),
fileInfoManager(dbAdapter, statementCache, fileStorage, objectInfoManager, hierarchyManager),
#if defined (NEEDS_WORK_PORT_GRA06_ECDbDeleteHandler) // Port 0503 to 06,
hierarchyManager(dbAdapter, statementCache, changeInfoManager),
#else
hierarchyManager(dbAdapter, statementCache, objectInfoManager, changeInfoManager),
#endif
instanceCacheHelper(dbAdapter, hierarchyManager, objectInfoManager, relationshipInfoManager, changeInfoManager),
rootManager(dbAdapter, statementCache, instanceCacheHelper, hierarchyManager, objectInfoManager),
responseManager(dbAdapter, statementCache, hierarchyManager, relationshipInfoManager, objectInfoManager),
navigationBaseManager(dbAdapter, statementCache),
changeInfoManager(dbAdapter, statementCache, hierarchyManager, objectInfoManager, relationshipInfoManager, fileInfoManager),
fileStorage(dbAdapter, statementCache, environment),
changeManager(dbAdapter, instanceCacheHelper, hierarchyManager, responseManager, objectInfoManager, relationshipInfoManager,
fileInfoManager, changeInfoManager, fileStorage, rootManager)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECSchemaCP WSCacheState::Core::GetCacheSchema()
    {
    if (nullptr == cacheSchema)
        {
        cacheSchema = dbAdapter.GetECSchema(SCHEMA_CacheSchema);
        }
    return cacheSchema;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSCacheState::WSCacheState(ObservableECDb& db, CacheEnvironmentCR environment) :
m_db(db),
m_environment(environment),
m_isSyncActive(false)
    {
    m_db.RegisterSchemaChangeListener(this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+--------------------------------------------------------------------------------------*/
WSCacheState::~WSCacheState()
    {
    m_db.UnRegisterSchemaChangeListener(this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+--------------------------------------------------------------------------------------*/
void WSCacheState::OnSchemaChanged()
    {
    ClearRuntimeCaches();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+--------------------------------------------------------------------------------------*/
void WSCacheState::ClearRuntimeCaches()
    {
    m_isSyncActive = m_core ? m_core->changeManager.IsSyncActive() : m_isSyncActive;
    m_core = nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+--------------------------------------------------------------------------------------*/
WSCacheState::Core& WSCacheState::GetCore()
    {
    if (nullptr == m_core)
        {
        ResetCore();
        }
    return *m_core;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void WSCacheState::ResetCore()
    {
    if (m_core)
        {
        m_isSyncActive = m_core->changeManager.IsSyncActive();
        }

    m_core = std::make_shared<Core>(m_db, m_environment);

    m_core->changeManager.SetSyncActive(m_isSyncActive);
    }
