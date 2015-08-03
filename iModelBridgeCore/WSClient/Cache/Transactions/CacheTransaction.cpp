/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Transactions/CacheTransaction.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Transactions/CacheTransaction.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheTransaction::CacheTransaction(IDataSourceCache& cache, ITransactionHandler* handler) :
Transaction(handler),
m_cache(cache)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheTransaction::CacheTransaction(CacheTransaction&& other) :
Transaction(std::move(other)),
m_cache(other.m_cache)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheTransaction::~CacheTransaction()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IDataSourceCache& CacheTransaction::GetCache() const
    {
    if (!IsActive())
        {
        BeAssert(IsActive());
        }
    return m_cache;
    }
