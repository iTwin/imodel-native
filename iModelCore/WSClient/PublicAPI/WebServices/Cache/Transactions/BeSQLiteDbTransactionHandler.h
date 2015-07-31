/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Transactions/BeSQLiteDbTransactionHandler.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Transactions/ITransactionHandler.h>
#include <WebServices/Cache/Transactions/Transaction.h>
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
* Class to use Transaction objects with BeSqliteDb with DefaultTx_No
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE BeSQLiteDbTransactionHandler : public ITransactionHandler
    {
    private:
        BeSQLite::Db* m_db;
        std::shared_ptr<Savepoint> m_transactionSavepoint;

    private:
        DbResult RetryDbOperation (std::function<DbResult()> operation);

    protected:
        //! Called when busy error received (other connection is using database). Return true to retry, false to cancel. 
        //! Count starts with 0 and incremented on each retry for same operation.
        virtual bool OnBusy (uint64_t count);

    public:
        WSCACHE_EXPORT BeSQLiteDbTransactionHandler (BeSQLite::Db& db);
        WSCACHE_EXPORT virtual ~BeSQLiteDbTransactionHandler ();

        //! Begin new transaction if there is no transaction started. Error if transaction is already active.
        WSCACHE_EXPORT virtual BentleyStatus BeginTransaction ();

        //! Commit active transaction.
        WSCACHE_EXPORT virtual BentleyStatus CommitTransaction () override;

        //! Rollback active transaction.
        WSCACHE_EXPORT virtual BentleyStatus RollbackTransaction () override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
