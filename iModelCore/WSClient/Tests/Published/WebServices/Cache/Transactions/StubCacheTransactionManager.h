/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Transactions/StubCacheTransactionManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Transactions/ICacheTransactionManager.h>
#include "MockTransactionHandler.h"

#ifdef USE_GTEST
using namespace ::testing;
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubCacheTransactionManager : public ICacheTransactionManager
    {
    private:
        std::shared_ptr<IDataSourceCache> m_cache;
        NiceMock<MockTransactionHandler> m_handler;

    public:
        StubCacheTransactionManager(std::shared_ptr<IDataSourceCache> cache) : m_cache(cache)
            {
            ON_CALL(m_handler, CommitTransaction()).WillByDefault(Return(SUCCESS));
            ON_CALL(m_handler, RollbackTransaction()).WillByDefault(Return(SUCCESS));
            };

        CacheTransaction StartCacheTransaction() override
            {
            return CacheTransaction(*m_cache, &m_handler);
            };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
