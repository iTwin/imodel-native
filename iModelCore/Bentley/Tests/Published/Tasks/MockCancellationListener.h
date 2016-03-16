/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Tasks/MockCancellationListener.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../TestsHelper.h"
#include <Bentley/Tasks/CancellationToken.h>

using namespace ::testing;

BEGIN_BENTLEY_UNIT_TESTS_NAMESPACE

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockCancellationListener : public ICancellationListener
    {
    public:
        MOCK_METHOD0 (OnCanceled, void ());
    };

#endif

END_BENTLEY_UNIT_TESTS_NAMESPACE