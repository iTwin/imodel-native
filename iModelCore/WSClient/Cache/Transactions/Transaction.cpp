/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
    if (IsActive())
        Rollback();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                          
+---------------+---------------+---------------+---------------+---------------+------*/
Transaction& Transaction::operator = (Transaction&& other)
    {
    if (IsActive())
        Rollback();

    m_activeTransactionHandler = other.m_activeTransactionHandler;
    other.m_activeTransactionHandler = nullptr;

    return *this;
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
