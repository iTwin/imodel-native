/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Transactions/CacheTransactionManager.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

bool CacheTransactionManager::s_safeAccessOnly = true;
intptr_t CacheTransactionManager::s_unsafeAccessThreadId = -1;

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
void CacheTransactionManager::SetAllowUnsafeAccess(bool allow, intptr_t threadId)
    {
    s_safeAccessOnly = !allow;
    s_unsafeAccessThreadId = threadId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CacheTransaction CacheTransactionManager::StartCacheTransaction()
    {
    if (!IsThreadValid())
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool CacheTransactionManager::IsThreadValid()
    {
    if (m_accessThread->GetThreadId() == BeThreadUtilities::GetCurrentThreadId())
        return true;

    if (s_safeAccessOnly)
        return false;

    if (s_unsafeAccessThreadId == BeThreadUtilities::GetCurrentThreadId())
        return true;

    return false;
    }
