/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Transactions/MockTransactionHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Transactions/ITransactionHandler.h>

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockTransactionHandler : public ITransactionHandler
    {
    public:
        MOCK_METHOD0 (CommitTransaction, BentleyStatus ());
        MOCK_METHOD0 (RollbackTransaction, BentleyStatus ());
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
