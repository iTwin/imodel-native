/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Transactions/CacheTransaction.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Transactions/Transaction.h>
#include <WebServices/Cache/Persistence/IDataSourceCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CacheTransaction : public Transaction
    {
    private:
        IDataSourceCache& m_cache;

    public:
        //! Create active transaction.
        WSCACHE_EXPORT CacheTransaction(IDataSourceCache& cache, ITransactionHandler* handler);
        //! Copy not allowed, use explicit move.
        WSCACHE_EXPORT CacheTransaction(const CacheTransaction&) = delete;
        //! Move active transaction without ending it to prolong its scope.
        WSCACHE_EXPORT CacheTransaction(CacheTransaction&&);
        //! If active, rollback transaction without commiting it.
        WSCACHE_EXPORT virtual ~CacheTransaction();

        //! Get cache for access in transaction.
        //! Do not store referance to IDataSourceCache ouside transaction scope - only access when CacheTransaction is active.
        WSCACHE_EXPORT IDataSourceCache& GetCache() const;
    };

typedef const CacheTransaction& CacheTransactionCR;
typedef CacheTransaction& CacheTransactionR;

END_BENTLEY_WEBSERVICES_NAMESPACE
