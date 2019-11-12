/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>

using namespace ::testing;

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockLocalState : IJsonLocalState
    {
    MOCK_METHOD3 (_SaveValue, void (Utf8CP nameSpace, Utf8CP key, Utf8StringCR value));
    MOCK_CONST_METHOD2 (_GetValue, Utf8String (Utf8CP nameSpace, Utf8CP key));
    };
#endif
