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

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubTokenStore : public ITokenStore
    {
    SamlTokenPtr token;
    DateTime tokenSetTime;

    void SetToken(SamlTokenPtr newToken) override { token = newToken; tokenSetTime = BeClock::Get().GetSystemTime(); }
    SamlTokenPtr GetToken() const override { return token; }
    DateTime GetTokenSetTime() const override { return tokenSetTime; }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE