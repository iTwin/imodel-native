/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Util/ISelectProvider.h>

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockSelectProvider : ISelectProvider
    {
    MOCK_CONST_METHOD1 (GetSelectProperties, std::shared_ptr<SelectProperties> (ECClassCR ecClass));
    MOCK_CONST_METHOD1 (GetSortPriority, int (ECClassCR ecClass));
    MOCK_CONST_METHOD1 (GetSortProperties, SortProperties (ECClassCR ecClass));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
