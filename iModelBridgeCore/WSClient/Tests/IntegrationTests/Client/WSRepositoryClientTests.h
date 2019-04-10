/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Client/WSRepositoryClientTests.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServicesTestsHelper.h>

struct WSRepositoryClientTests : public WSClientBaseTest
    {
    RuntimeJsonLocalState m_localState;
    void SetUp();
    };
