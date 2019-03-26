/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Cache/CachingDataSourceTests.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServicesTestsHelper.h>

struct CachingDataSourceTests : public WSClientBaseTest
    {
    RuntimeJsonLocalState m_localState;
    void SetUp();
    };
