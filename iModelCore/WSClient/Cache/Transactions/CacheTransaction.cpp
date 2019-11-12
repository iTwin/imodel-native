/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Transactions/CacheTransaction.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheTransaction::CacheTransaction(IDataSourceCache& cache, ITransactionHandler* handler) :
Transaction(handler),
m_cache(&cache)
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
CacheTransaction::~CacheTransaction() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CacheTransaction& CacheTransaction::operator=(CacheTransaction&& other)
    {
    Transaction::operator=(std::move(other));

    m_cache = other.m_cache;
    other.m_cache = nullptr;

    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IDataSourceCache& CacheTransaction::GetCache() const
    {
    if (!IsActive())
        BeAssert(IsActive());

    return *m_cache;
    }
