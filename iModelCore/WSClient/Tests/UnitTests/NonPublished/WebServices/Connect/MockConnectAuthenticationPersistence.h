/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>

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
