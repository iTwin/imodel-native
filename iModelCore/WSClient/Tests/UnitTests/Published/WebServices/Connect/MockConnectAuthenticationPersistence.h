/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/MockConnectAuthenticationPersistence.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <gmock/gmock.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

#if defined (USE_GTEST)
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockConnectAuthenticationPersistence : public IConnectAuthenticationPersistence
    {
    public:
        MOCK_METHOD1 (SetCredentials, void (CredentialsCR credentials));
        MOCK_CONST_METHOD0 (GetCredentials, Credentials ());

        MOCK_METHOD1 (SetToken, void (SamlTokenPtr token));
        MOCK_CONST_METHOD0 (GetToken, SamlTokenPtr ());
        MOCK_CONST_METHOD0(GetTokenSetTime, DateTime());
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
