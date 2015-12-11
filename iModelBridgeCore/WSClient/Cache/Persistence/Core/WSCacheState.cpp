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
m_dbAdapter(db),
m_statementCache(db),

m_objectInfoManager(m_dbAdapter, m_statementCache, m_hierarchyManager),
m_relationshipInfoManager(m_dbAdapter, m_statementCache, m_hierarchyManager),
m_fileInfoManager(m_dbAdapter, m_statementCache, m_fileStorage, m_objectInfoManager, m_hierarchyManager),
#if defined (NEEDS_WORK_PORT_GRA06_ECDbDeleteHandler) // Port 0503 to 06,
m_hierarchyManager(m_dbAdapter, m_statementCache, m_changeInfoManager),
#else
m_hierarchyManager(m_dbAdapter, m_statementCache, m_objectInfoManager, m_changeInfoManager),
#endif
m_instanceHelper(m_dbAdapter, m_hierarchyManager, m_objectInfoManager, m_relationshipInfoManager, m_changeInfoManager),
m_rootManager(m_dbAdapter, m_statementCache, m_instanceHelper, m_hierarchyManager, m_objectInfoManager),
m_cachedQueryManager(m_dbAdapter, m_statementCache, m_hierarchyManager, m_relationshipInfoManager, m_objectInfoManager),
m_navigationBaseManager(m_dbAdapter, m_statementCache),
m_changeInfoManager(m_dbAdapter, m_statementCache, m_hierarchyManager, m_objectInfoManager, m_relationshipInfoManager, m_fileInfoManager),
m_changeManager(m_dbAdapter, m_instanceHelper, m_hierarchyManager, m_cachedQueryManager, m_objectInfoManager, m_relationshipInfoManager,
m_fileInfoManager, m_changeInfoManager, m_fileStorage, m_rootManager),
m_fileStorage(m_dbAdapter, m_statementCache, environment),

m_cacheSchema(nullptr),
m_extendedDataAdapter(db)
    {}

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
    m_isSyncActive = m_core ? m_core->m_changeManager.IsSyncActive() : m_isSyncActive;
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
        m_isSyncActive = m_core->m_changeManager.IsSyncActive();
        }

    m_core = std::make_shared<Core>(m_db, m_environment);

    m_core->m_changeManager.SetSyncActive(m_isSyncActive);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+--------------------------------------------------------------------------------------*/
ECSchemaCP WSCacheState::GetCacheSchema()
    {
    Core& core = GetCore();
    if (nullptr == core.m_cacheSchema)
        {
        core.m_cacheSchema = core.m_dbAdapter.GetECSchema(SCHEMA_CacheSchema);
        }
    return core.m_cacheSchema;
    }