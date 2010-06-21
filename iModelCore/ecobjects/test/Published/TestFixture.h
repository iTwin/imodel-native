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
    static std::wstring s_dllPath;

protected:
    ECTestFixture(void) {}
    
public:
    virtual void            SetUp () override;
    virtual void            TearDown () override;

    void    TestForECSchemaLeaks ();
    void    TestForIECInstanceLeaks ();

    static std::wstring GetTestDataPath(const wchar_t *fileName);
    };

END_BENTLEY_EC_NAMESPACE
