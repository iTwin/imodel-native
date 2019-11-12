/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Transactions/BeSQLiteDbTransactionHandler.h>
#include <WebServices/Cache/Transactions/CacheTransaction.h>
#include <WebServices/Cache/Transactions/ICacheTransactionManager.h>
#include <Bentley/Tasks/WorkerThread.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CacheTransactionManager : public ICacheTransactionManager
    {
    private:
        static bool s_safeAccessOnly;
        static intptr_t s_unsafeAccessThreadId;
        std::unique_ptr<IDataSourceCache> m_cache;
        WorkerThreadPtr m_accessThread;
        BeSQLiteDbTransactionHandler m_transactionHandler;

    private:
        bool IsThreadValid();

    public:
        WSCACHE_EXPORT CacheTransactionManager
            (
            std::unique_ptr<IDataSourceCache> cache,
            WorkerThreadPtr accessThread
            );

        //! Enable or disable correct thread checking when starting transactions for specified thread.
        //! WARNING: Only useful for testing purposes and should never be used in production.
        //! @param allow
        //! @param threadId - defaults to enabling unsafe access to caller thread (usually main testing thread)
        WSCACHE_EXPORT static void SetAllowUnsafeAccess(bool allow, intptr_t threadId = BeThreadUtilities::GetCurrentThreadId());

        //! Start active transaction. Can be called only in cache access thread. Will assert and return inactive transaction if an error occurs.
        WSCACHE_EXPORT CacheTransaction StartCacheTransaction() override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
