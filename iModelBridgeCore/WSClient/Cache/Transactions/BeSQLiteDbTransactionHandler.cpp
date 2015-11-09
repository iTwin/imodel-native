/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Transactions/BeSQLiteDbTransactionHandler.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Transactions/BeSQLiteDbTransactionHandler.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLiteDbTransactionHandler::BeSQLiteDbTransactionHandler(BeSQLite::Db& db) :
m_db(&db)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLiteDbTransactionHandler::~BeSQLiteDbTransactionHandler()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeSQLiteDbTransactionHandler::OnBusy(uint64_t count)
    {
    if (std::numeric_limits<uint64_t>::max() == count)
        {
        return false;
        }

    BeThreadUtilities::BeSleep(100);
    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BeSQLiteDbTransactionHandler::RetryDbOperation(std::function<DbResult()> operation)
    {
    uint64_t count = 0;
    DbResult result;
    while (DbResult::BE_SQLITE_BUSY == (result = operation()))
        {
        OnBusy(count);
        count++;
        }
    return result;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeSQLiteDbTransactionHandler::BeginTransaction()
    {
    if (m_transactionSavepoint || 0 != m_db->GetCurrentSavepointDepth())
        {
        BeAssert(false &&
            "Transaction is already active. Nesting transactions are not allowed. "
            "Commit existing transaction or extend its lifetime. "
            "Check if there are no transactions used in paralel in other threads.");
        return ERROR;
        }

    auto savepoint = std::make_shared<Savepoint>(*m_db, "BeSQLiteDbTransactionHandler", false, BeSQLiteTxnMode::Immediate);

    DbResult result = RetryDbOperation([&]
        {
        return savepoint->Begin();
        });
    if (DbResult::BE_SQLITE_OK != result)
        {
        BeAssert(false);
        return ERROR;
        }

    m_transactionSavepoint = savepoint;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeSQLiteDbTransactionHandler::CommitTransaction()
    {
    if (!m_transactionSavepoint)
        {
        BeAssert(false && "Transaction is not active");
        return ERROR;
        }

    auto savepoint = m_transactionSavepoint;
    m_transactionSavepoint = nullptr;

    DbResult result = RetryDbOperation([&]
        {
        return savepoint->Commit();
        });

    if (DbResult::BE_SQLITE_OK != result)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeSQLiteDbTransactionHandler::RollbackTransaction()
    {
    if (!m_transactionSavepoint)
        {
        BeAssert(false && "Transaction is not active");
        return ERROR;
        }

    auto savepoint = m_transactionSavepoint;
    m_transactionSavepoint = nullptr;

    DbResult result = RetryDbOperation([&]
        {
        return savepoint->Cancel();
        });
    if (DbResult::BE_SQLITE_OK != result)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }
