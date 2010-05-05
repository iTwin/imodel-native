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

struct ECTestFixture
    {
private:
    static std::wstring s_dllPath;
    ECTestFixture(void) {}
    
public:
    static std::wstring GetTestDataPath(const wchar_t *fileName);
    };

END_BENTLEY_EC_NAMESPACE
