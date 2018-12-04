/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Examples/ExampleTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServicesTestsHelper.h>

struct ExampleTests : public WSClientBaseTest
    {
    StubLocalState m_localState;
    void SetUp();
    };
