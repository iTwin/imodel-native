/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Transactions/Transaction.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Transactions/Transaction.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Transaction::Transaction(ITransactionHandler* handler) :
m_activeTransactionHandler(handler)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Transaction::Transaction(Transaction&& other) :
m_activeTransactionHandler(other.m_activeTransactionHandler)
    {
    other.m_activeTransactionHandler = nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Transaction::~Transaction()
    {
    Rollback();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool Transaction::IsActive() const
    {
    return nullptr != m_activeTransactionHandler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Transaction::Commit()
    {
    if (nullptr == m_activeTransactionHandler)
        {
        return ERROR;
        }
    auto status = m_activeTransactionHandler->CommitTransaction();
    m_activeTransactionHandler = nullptr;
    return status;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Transaction::Rollback()
    {
    if (nullptr == m_activeTransactionHandler)
        {
        return ERROR;
        }
    auto status = m_activeTransactionHandler->RollbackTransaction();
    m_activeTransactionHandler = nullptr;
    return status;
    }