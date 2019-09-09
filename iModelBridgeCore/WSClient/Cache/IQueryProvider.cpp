/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/IQueryProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<IQueryProvider::Query> IQueryProvider::GetQueries
(
CacheTransactionCR txn,
ECInstanceKeyCR instanceKey,
bool isPersistent
) const
    {
    return bvector<IQueryProvider::Query>();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Petras.Sukys    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ICancellationTokenPtr IQueryProvider::IsFileRetrievalNeeded
(
CacheTransactionCR txn,
ECInstanceKeyCR instanceKey,
bool isPersistent
) const
    {
    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool IQueryProvider::DoUpdateFile
(
CacheTransactionCR txn,
ECInstanceKeyCR instanceKey,
bool isPersistent
) const
    {
    auto ct = IsFileRetrievalNeeded(txn, instanceKey, isPersistent);
    return nullptr != ct && !ct->IsCanceled();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IQueryProvider::Query::Query() :
key({}, {}),
syncRecursively(false)
    {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IQueryProvider::Query::Query(CachedResponseKey key, WSQueryPtr query, bool syncRecursively) :
key(key),
query(query),
syncRecursively(syncRecursively)
    {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool IQueryProvider::Query::IsValid() const
    {
    return key.IsValid() && nullptr != query;
    };
