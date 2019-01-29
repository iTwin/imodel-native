/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/RepositoryInfoStore.cpp $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "RepositoryInfoStore.h"

#define ECDbProperty_NAMESPACE      "RepositoryInfoStore"
#define ECDbProperty_ServerInfo     "Info"
#define ECDbProperty_RepositoryInfo "RepositoryInfo"
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
        m_client->RegisterRepositoryInfoListener(m_infoListener);
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
        m_client->UnregisterRepositoryInfoListener(m_infoListener);
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
            return;
        txn.Commit();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RepositoryInfoStore::CacheServerInfo(IDataSourceCache& cache, WSInfoCR info)
    {
    BeMutexHolder lock(m_infoMutex);
    m_info = info;

    PropertySpec prop(ECDbProperty_ServerInfo, ECDbProperty_NAMESPACE);
    if (DbResult::BE_SQLITE_OK != cache.GetAdapter().GetECDb().SavePropertyString(prop, m_info.ToString()))
        {
        BeAssert(false);
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo RepositoryInfoStore::GetServerInfo()
    {
    BeMutexHolder lock(m_infoMutex);
    BeAssert(m_info.IsValid());
    return m_info;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryInfoStore::CacheRepositoryInfo(WSRepositoryCR info)
    {
    m_thread->ExecuteAsync([=]
        {
        auto txn = m_cacheTxnManager->StartCacheTransaction();
        if (SUCCESS != CacheRepositoryInfo(txn.GetCache(), info))
            return;
        txn.Commit();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RepositoryInfoStore::CacheRepositoryInfo(IDataSourceCache& cache, WSRepositoryCR info)
    {
    BeMutexHolder lock(m_infoMutex);
    m_repositoryInfo = info;

    PropertySpec prop(ECDbProperty_RepositoryInfo, ECDbProperty_NAMESPACE);
    if (DbResult::BE_SQLITE_OK != cache.GetAdapter().GetECDb().SavePropertyString(prop, m_repositoryInfo.ToString()))
        {
        BeAssert(false);
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RepositoryInfoStore::PrepareInfo(IDataSourceCache& cache)
    {
    BeMutexHolder lock(m_infoMutex);
    m_info = ReadServerInfo(cache);
    if (!m_info.IsValid())
        return ERROR;

    m_repositoryInfo = ReadRepositoryInfo(cache);
    if (!m_repositoryInfo.IsValid())
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WSRepository RepositoryInfoStore::GetRepositoryInfo()
    {
    BeMutexHolder lock(m_infoMutex);
    BeAssert(m_repositoryInfo.IsValid());
    return m_repositoryInfo;
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
* @bsimethod                                                 julius.cepukenas   05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WSRepository RepositoryInfoStore::ReadRepositoryInfo(IDataSourceCache& cache)
    {
    PropertySpec prop(ECDbProperty_RepositoryInfo, ECDbProperty_NAMESPACE);
    Utf8String infoStr;
    cache.GetAdapter().GetECDb().QueryProperty(infoStr, prop);

    return WSRepository(infoStr);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RepositoryInfoStore::SetCacheInitialized(IDataSourceCache& cache)
    {
    Json::Value value;
    value["date"] = DateTime::GetCurrentTimeUtc().ToString();
    Utf8String valueStr = Json::FastWriter::ToString(value);

    PropertySpec prop(ECDbProperty_Initialized, ECDbProperty_NAMESPACE);
    if (DbResult::BE_SQLITE_OK != cache.GetAdapter().GetECDb().SavePropertyString(prop, valueStr))
        return ERROR;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryInfoStore::IsCacheInitialized(IDataSourceCache& cache)
    {
    PropertySpec propInitilised(ECDbProperty_Initialized, ECDbProperty_NAMESPACE);
    if (!cache.GetAdapter().GetECDb().HasProperty(propInitilised))
        return false;

    PropertySpec propServerInfo(ECDbProperty_ServerInfo, ECDbProperty_NAMESPACE);
    if (!cache.GetAdapter().GetECDb().HasProperty(propServerInfo))
        return false;

    PropertySpec propRepInfo(ECDbProperty_RepositoryInfo, ECDbProperty_NAMESPACE);
    if (!cache.GetAdapter().GetECDb().HasProperty(propRepInfo))
        return false;

    return true;
    }
