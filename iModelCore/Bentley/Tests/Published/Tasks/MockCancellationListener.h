/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Tasks/MockCancellationListener.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <Bentley/Tasks/Tasks.h>
#include <Bentley/Tasks/CancellationToken.h>

using namespace ::testing;

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