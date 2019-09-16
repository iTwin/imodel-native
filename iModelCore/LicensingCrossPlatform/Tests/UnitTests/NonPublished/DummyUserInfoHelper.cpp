/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/NonPublished/DummyUserInfoHelper.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
