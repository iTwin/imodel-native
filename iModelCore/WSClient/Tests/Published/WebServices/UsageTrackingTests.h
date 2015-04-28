/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/UsageTrackingTests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../MobileUtilsTests.h"
#include "WebServicesTestsHelper.h"

class UsageTrackingTests : public BaseMockHttpHandlerTest
    {
    public:
        virtual void SetUp () override;
        virtual void TearDown () override;
    };
