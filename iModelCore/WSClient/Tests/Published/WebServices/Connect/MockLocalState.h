/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Connect/MockLocalState.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <DgnClientFx/DgnClientApp.h>

using namespace ::testing;

BEGIN_BENTLEY_DGNCLIENTFX_NAMESPACE

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockLocalState : public ILocalState
    {
    public:
        MOCK_METHOD3 (SaveValue, void (Utf8CP nameSpace, Utf8CP key, JsonValueCR value));
        MOCK_CONST_METHOD2 (GetValue, Json::Value (Utf8CP nameSpace, Utf8CP key));
    };
#endif

END_BENTLEY_DGNCLIENTFX_NAMESPACE
