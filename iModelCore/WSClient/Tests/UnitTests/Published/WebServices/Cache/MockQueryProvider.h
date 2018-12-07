/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/MockQueryProvider.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    MOCK_CONST_METHOD3 (GetQueries, bvector<IQueryProvider::Query>
        (
        CacheTransactionCR txn,
        ECInstanceKeyCR instanceKey,
        bool isPersistent
        ));

    MOCK_CONST_METHOD3(IsFileRetrievalNeeded, ICancellationTokenPtr
        (
        CacheTransactionCR txn,
        ECInstanceKeyCR instanceKey,
        bool isPersistent
        ));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
