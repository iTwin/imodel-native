/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/RepositoryInfoStore.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "RepositoryInfoStore.h"

#define ECDbProperty_NAMESPACE      "RepositoryInfoStore"
#define ECDbProperty_ServerInfo     "Info"
#define ECDbProperty_Initialized    "Initialized"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryInfoStore::RepositoryInfoStore
(
ICacheTransactionManager* cacheTxnManager,
IWSRepositoryClientPtr client,
WorkerThreadPtr thread
) :
m_cacheTxnManager(cacheTxnManager),
m_client(client),
m_infoListener(std::make_shared<InfoListener>(this)),
m_thread(thread)
    {
    if (m_client)
        {
        m_client->GetWSClient()->RegisterServerInfoListener(m_infoListener);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryInfoStore::~RepositoryInfoStore()
    {
    if (m_client)
        {
        m_client->GetWSClient()->UnregisterServerInfoListener(m_infoListener);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryInfoStore::CacheServerInfo(WSInfoCR info)
    {
    m_thread->ExecuteAsync([=]
        {
        auto txn = m_cacheTxnManager->StartCacheTransaction();
        if (SUCCESS != CacheServerInfo(txn.GetCache(), info))
            {
            return;
            }
        txn.Commit();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RepositoryInfoStore::CacheServerInfo(IDataSourceCache& cache, WSInfoCR info)
    {
    m_cachedInfo = info;

    PropertySpec prop(ECDbProperty_ServerInfo, ECDbProperty_NAMESPACE);
    if (DbResult::BE_SQLITE_OK != cache.GetAdapter().GetECDb().SavePropertyString(prop, m_cachedInfo.ToString()))
        {
        BeAssert(false);
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfoCR RepositoryInfoStore::GetServerInfo(IDataSourceCache& cache)
    {
    if (!m_cachedInfo.IsValid())
        {
        m_cachedInfo = ReadServerInfo(cache);
        }
    return m_cachedInfo;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo RepositoryInfoStore::ReadServerInfo(IDataSourceCache& cache)
    {
    PropertySpec prop(ECDbProperty_ServerInfo, ECDbProperty_NAMESPACE);
    Utf8String infoStr;
    cache.GetAdapter().GetECDb().QueryProperty(infoStr, prop);
    return WSInfo(infoStr);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RepositoryInfoStore::SetCacheInitialized(IDataSourceCache& cache)
    {
    PropertySpec prop(ECDbProperty_Initialized, ECDbProperty_NAMESPACE);
    if (DbResult::BE_SQLITE_OK != cache.GetAdapter().GetECDb().SavePropertyString(prop, ""))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryInfoStore::IsCacheInitialized(IDataSourceCache& cache)
    {
    PropertySpec prop(ECDbProperty_Initialized, ECDbProperty_NAMESPACE);
    return cache.GetAdapter().GetECDb().HasProperty(prop);
    }
