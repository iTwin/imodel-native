/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Util/MockSelectProvider.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Util/ISelectProvider.h>

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockSelectProvider : public ISelectProvider
    {
    public:
        MOCK_CONST_METHOD1 (GetSelectProperties, std::shared_ptr<SelectProperties> (ECClassCR ecClass));
        MOCK_CONST_METHOD1 (GetSortPriority, int (ECClassCR ecClass));
        MOCK_CONST_METHOD1 (GetSortProperties, SortProperties (ECClassCR ecClass));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
