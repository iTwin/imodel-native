/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>

#include <json/json.h>
#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>


BEGIN_BENTLEY_LICENSING_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

struct DummyUserInfoHelper
    {
    public:
        static ConnectSignInManager::UserInfo CreateUserInfo(Utf8StringCR userName, Utf8StringCR firstName, Utf8StringCR lastName, Utf8StringCR userId, Utf8StringCR orgId);
    };

END_BENTLEY_LICENSING_NAMESPACE
