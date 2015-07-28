/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Transactions/Transaction.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Transactions/ITransactionHandler.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct Transaction
    {
    private:
        ITransactionHandler* m_activeTransactionHandler;

    public:
        //! Create active transaction.
        WSCACHE_EXPORT Transaction (ITransactionHandler* handler);
        //! Only move constructor is available.
        WSCACHE_EXPORT Transaction (const Transaction&) = delete;
        //! Move created transaction without ending it.
        WSCACHE_EXPORT Transaction (Transaction&&);
        //! If active, rollback transaction without commiting it.
        WSCACHE_EXPORT virtual ~Transaction ();

        //! Check if transaction is active
        WSCACHE_EXPORT bool IsActive () const;
        //! Commit whole transaction
        WSCACHE_EXPORT BentleyStatus Commit ();
        //! Rollback whole transaction
        WSCACHE_EXPORT BentleyStatus Rollback ();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
