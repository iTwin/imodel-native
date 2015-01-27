/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Connect/MockConnectAuthenticationPersistence.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Client/Connect/ConnectAuthenticationPersistence.h>

using namespace ::testing;

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
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
