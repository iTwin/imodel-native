/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/Published/TestFixture.h $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <windows.h>

BEGIN_BENTLEY_EC_NAMESPACE

struct ECTestFixture : public ::testing::Test
    {
private:
    static bwstring s_dllPath;
    static bwstring GetDllPath();
    bwstring GetLogConfigurationFilename();
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

    static bwstring GetTestDataPath(const wchar_t *fileName);
    static bwstring GetWorkingDirectoryPath(const wchar_t *testFixture, const wchar_t *fileName);
    };

END_BENTLEY_EC_NAMESPACE
