/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Connect/IConnectionClientInterface.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

#if defined (USE_GTEST)
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Mark.Uvari    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockConnectionClientInterface : IConnectionClientInterface
    {
    MockConnectionClientInterface()
        {
        using namespace ::testing;
        ON_CALL(*this, IsInstalled()).WillByDefault(Return(true));
        ON_CALL(*this, IsLoggedIn()).WillByDefault(Return(true));
        ON_CALL(*this, GetUserId()).WillByDefault(Return(""));              
        ON_CALL(*this, StartClientApp()).WillByDefault(Return(BentleyStatus::SUCCESS));
        }

    MOCK_METHOD0(IsInstalled, bool());
    MOCK_METHOD0(IsRunning, bool());
    MOCK_METHOD0(IsLoggedIn, bool());
    MOCK_METHOD0(GetUserId, Utf8String());
    MOCK_METHOD2(GetSerializedDelegateSecurityToken, SamlTokenPtr(Utf8StringCR rpUri, Utf8StringP errorStringOut));
    MOCK_METHOD1(AddClientEventListener, void(event_callback callback));
    MOCK_METHOD0(StartClientApp, BentleyStatus());
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
