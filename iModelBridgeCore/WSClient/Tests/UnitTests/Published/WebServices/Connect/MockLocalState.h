/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/MockLocalState.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>

using namespace ::testing;

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockLocalState : public IJsonLocalState
    {
    public:
        MOCK_METHOD3 (_SaveValue, void (Utf8CP nameSpace, Utf8CP key, Utf8StringCR value));
        MOCK_CONST_METHOD2 (_GetValue, Utf8String (Utf8CP nameSpace, Utf8CP key));
    };
#endif
