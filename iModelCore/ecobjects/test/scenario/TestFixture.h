/*--------------------------------------------------------------------------------------+
|
|     $Source: test/scenario/TestFixture.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <windows.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct ECTestFixture : public ::testing::Test
    {
private:
    static WString s_dllPath;
    static bool s_isLoggerInitialized;
    static WString GetDllPath();
    WString GetLogConfigurationFilename();
    BentleyStatus CheckProcessDirectory(WCharP filepath, DWORD bufferSize);

protected:
    ECTestFixture(void);
    
public:
    virtual void            SetUp () override;
    virtual void            TearDown () override;

    virtual bool            _WantSchemaLeakDetection() { return true; }
    virtual bool            _WantInstanceLeakDetection() { return true; }

    void    TestForECSchemaLeaks ();
    void    TestForIECInstanceLeaks ();

    static WString GetTestDataPath(WCharCP fileName);
    static WString GetTempDataPath(WCharCP fileName);
    static WString GetWorkingDirectoryPath(WCharCP testFixture, WCharCP fileName);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
