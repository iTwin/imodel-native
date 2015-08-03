/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/IQueryProvider.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool IQueryProvider::DoUpdateFile
(
CacheTransactionCR txn,
ECInstanceKeyCR instanceKey,
bool isPersistent
) const
    {
    return false;
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