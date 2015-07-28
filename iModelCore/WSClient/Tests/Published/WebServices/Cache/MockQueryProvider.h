/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/MockQueryProvider.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/ICachingDataSource.h>

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockQueryProvider : public IQueryProvider
    {
    public:
        MOCK_CONST_METHOD1 (GetInitialQueries, bvector<IQueryProvider::Query>
            (
            CacheTransactionCR txn
            ));

        MOCK_CONST_METHOD1 (GetInitialInstances, bvector<ECInstanceKey>
            (
            CacheTransactionCR txn
            ));

        MOCK_CONST_METHOD3 (GetQueries, bvector<IQueryProvider::Query>
            (
            CacheTransactionCR txn,
            ECInstanceKeyCR instanceKey,
            bool isPersistent
            ));

        MOCK_CONST_METHOD3 (DoUpdateFile, bool
            (
            CacheTransactionCR txn,
            ECInstanceKeyCR instanceKey,
            bool isPersistent
            ));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
