/*--------------------------------------------------------------------------------------+
|
|     $Source: test/TestFixture.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct ECTestFixture : ::testing::Test
    {
private:
    static bool s_isLoggerInitialized;
    WString GetLogConfigurationFilename();

protected:
    ECTestFixture();
    
public:
    virtual void            SetUp () override;
    virtual void            TearDown () override;

    static WString GetTestDataPath(WCharCP fileName);
    static WString GetTempDataPath(WCharCP fileName);
    static WString GetWorkingDirectoryPath(WCharCP testFixture, WCharCP fileName);
    static WString GetTestResultsFilePath (WCharCP fileName);
    static WString GetDateTime();
    };

END_BENTLEY_ECOBJECT_NAMESPACE
