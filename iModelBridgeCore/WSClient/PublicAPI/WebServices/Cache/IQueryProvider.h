/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>

#include <WebServices/Cache/Persistence/IDataSourceCache.h>
#include <WebServices/Cache/Transactions/CacheTransaction.h>
#include <WebServices/Client/WSQuery.h>
#include <Bentley/Tasks/AsyncResult.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<const struct IQueryProvider> IQueryProviderPtr;
struct EXPORT_VTABLE_ATTRIBUTE IQueryProvider
    {
    public:
        struct Query;

    public:
        virtual ~IQueryProvider()
            {};

        //! Get queries for specific instance. By default returns none.
        WSCACHE_EXPORT virtual bvector<Query> GetQueries
            (
            CacheTransactionCR txn,
            ECInstanceKeyCR instanceKey,
            bool isPersistent
            ) const;

        //! Returns non null cancellation token for file that needs retrieving from source. Returns nullptr if there is no need to retrieve file.
        //! Returned token will be cancelled if file retrieval is no longer necessary.
        WSCACHE_EXPORT virtual ICancellationTokenPtr IsFileRetrievalNeeded
            (
            CacheTransactionCR txn,
            ECInstanceKeyCR instanceKey,
            bool isPersistent
            ) const;

        //! Return true if file for this instance should be downloaded. By default returns false.
        //! DEPRECATED, use IsFileRetrievalNeeded(CacheTransactionCR, ECInstanceKeyCR, bool)
        WSCACHE_EXPORT virtual bool DoUpdateFile
            (
            CacheTransactionCR txn,
            ECInstanceKeyCR instanceKey,
            bool isPersistent
            ) const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct IQueryProvider::Query
    {
    public:
        CachedResponseKey key;
        WSQueryPtr query;
        bool syncRecursively;

    public:
        WSCACHE_EXPORT Query();
        WSCACHE_EXPORT Query(CachedResponseKey key, WSQueryPtr query, bool syncRecursively = true);
        WSCACHE_EXPORT bool IsValid() const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
