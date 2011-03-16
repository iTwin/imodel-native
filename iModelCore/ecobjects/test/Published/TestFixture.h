/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/TestFixture.h $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <windows.h>

BEGIN_BENTLEY_EC_NAMESPACE

struct ECTestFixture : public ::testing::Test
    {
private:
    static WString s_dllPath;
    static WString GetDllPath();
    WString GetLogConfigurationFilename();
    BentleyStatus CheckProcessDirectory(wchar_t *filepath, DWORD bufferSize);

protected:
    ECTestFixture(void);
    
public:
    virtual void            SetUp () override;
    virtual void            TearDown () override;

    virtual bool            _WantSchemaLeakDetection() { return true; }
    virtual bool            _WantInstanceLeakDetection() { return true; }

    void    TestForECSchemaLeaks ();
    void    TestForIECInstanceLeaks ();

    static WString GetTestDataPath(const wchar_t *fileName);
    static WString GetWorkingDirectoryPath(const wchar_t *testFixture, const wchar_t *fileName);
    };

END_BENTLEY_EC_NAMESPACE
