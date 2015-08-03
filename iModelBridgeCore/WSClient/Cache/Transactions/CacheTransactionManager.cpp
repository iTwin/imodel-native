/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Transactions/CacheTransactionManager.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Transactions/CacheTransactionManager.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

bool CacheTransactionManager::s_allowUnsafeAccess = false;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CacheTransactionManager::CacheTransactionManager
(
std::unique_ptr<IDataSourceCache> cache,
WorkerThreadPtr accessThread
) :
m_cache(std::move(cache)),
m_accessThread(accessThread),
m_transactionHandler(m_cache->GetECDb())
    {
    BeAssert(nullptr != m_cache);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheTransactionManager::SetAllowUnsafeAccess(bool allow)
    {
    s_allowUnsafeAccess = allow;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CacheTransaction CacheTransactionManager::StartCacheTransaction()
    {
    if (!s_allowUnsafeAccess && m_accessThread->GetThreadId() != BeThreadUtilities::GetCurrentThreadId())
        {
        BeAssert(false && "Starting cache transactions is only allowed in cache access thread");
        return CacheTransaction(*m_cache, nullptr);
        }

    if (SUCCESS != m_transactionHandler.BeginTransaction())
        {
        BeAssert(false && "Failed to start cache transaction");
        return CacheTransaction(*m_cache, nullptr);
        }

    return CacheTransaction(*m_cache, &m_transactionHandler);
    }
