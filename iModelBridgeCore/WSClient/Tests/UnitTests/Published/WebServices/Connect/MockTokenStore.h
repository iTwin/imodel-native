/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <WebServices/Connect/ITokenStore.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

using namespace ::testing;
#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockTokenStore : ITokenStore
    {
    MOCK_METHOD1(SetToken, void (SamlTokenPtr));
    MOCK_CONST_METHOD0(GetToken, SamlTokenPtr());
    MOCK_CONST_METHOD0(GetTokenSetTime, DateTime());
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE