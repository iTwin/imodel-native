/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Connect/ConnectTokenProviderTests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../MobileUtilsTests.h"
#include "ConnectTestsHelper.h"
#include "../WebServices/WebServicesTestsHelper.h"

class ConnectTokenProviderTests : public BaseMockHttpHandlerTest
    {
    public:
        virtual void SetUp () override;
        virtual void TearDown () override;
    };
