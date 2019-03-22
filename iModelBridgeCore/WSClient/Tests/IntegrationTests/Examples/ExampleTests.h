/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Examples/ExampleTests.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServicesTestsHelper.h>

struct ExampleTests : public WSClientBaseTest
    {
    RuntimeJsonLocalState m_localState;
    void SetUp();
    };
