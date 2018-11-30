/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Client/WSRepositoryClientTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServicesTestsHelper.h>

struct WSRepositoryClientTests : public WSClientBaseTest
    {
    StubLocalState m_localState;
    void SetUp();
    };
