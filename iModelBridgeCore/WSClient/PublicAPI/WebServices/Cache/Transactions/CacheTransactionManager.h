/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Transactions/CacheTransactionManager.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
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
        static bool s_allowUnsafeAccess;
        std::unique_ptr<IDataSourceCache> m_cache;
        WorkerThreadPtr m_accessThread;
        BeSQLiteDbTransactionHandler m_transactionHandler;

    public:
        WSCACHE_EXPORT CacheTransactionManager
            (
            std::unique_ptr<IDataSourceCache> cache,
            WorkerThreadPtr accessThread
            );

        //! Enable or disable correct thread checking when starting transactions.
        //! Only useful for testing purposes and should never be used in production.
        WSCACHE_EXPORT static void SetAllowUnsafeAccess(bool allow);

        //! Start active transaction. Can be called only in cache access thread. Will assert and return inactive transaction if an error occurs.
        WSCACHE_EXPORT CacheTransaction StartCacheTransaction() override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
