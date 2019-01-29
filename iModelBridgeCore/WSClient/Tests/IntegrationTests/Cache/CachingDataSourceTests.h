/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Cache/CachingDataSourceTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServicesTestsHelper.h>

struct CachingDataSourceTests : public WSClientBaseTest
    {
    StubLocalState m_localState;
    void SetUp();
    };
