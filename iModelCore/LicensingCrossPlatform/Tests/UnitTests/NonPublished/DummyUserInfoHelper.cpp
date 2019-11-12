/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DummyUserInfoHelper.h"

USING_NAMESPACE_BENTLEY_LICENSING
//USING_NAMESPACE_BENTLEY_WEBSERVICES

ConnectSignInManager::UserInfo DummyUserInfoHelper::CreateUserInfo(Utf8StringCR username, Utf8StringCR firstName, Utf8StringCR lastName, Utf8StringCR userId, Utf8StringCR orgId)
    {
    Json::Value infoJson(Json::objectValue);
    infoJson["username"] = username;
    infoJson["firstName"] = firstName;
    infoJson["lastName"] = lastName;
    infoJson["userId"] = userId;
    infoJson["orgId"] = orgId;

    return ConnectSignInManager::UserInfo(Json::FastWriter().write(infoJson));
    }
