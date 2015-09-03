/*--------------------------------------------------------------------------------------+
|
|     $Source: test/TestFixture/TestFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include "../ECObjectsTestPCH.h"

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECTestFixture : ::testing::Test
    {
protected:
    ECTestFixture();
    
public:

    static WString GetTestDataPath(WCharCP fileName);
    static WString GetTempDataPath(WCharCP fileName);
    static Utf8String GetDateTime();
    };

END_BENTLEY_ECN_TEST_NAMESPACE
