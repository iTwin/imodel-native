/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/IConnectSignInManager.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/IConnectSignInManager.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ImsClient.h>

// These should be removed from public API in future. Currently FieldApps/MobileUtils depend on those APIs.
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectTokenProvider.h>

#include "Connect.xliff.h"

#define USERINFO_Serialized_Username        "username"
#define USERINFO_Serialized_FirstName       "firstName"
#define USERINFO_Serialized_LastName        "lastName"
#define USERINFO_Serialized_UserId          "userId"
#define USERINFO_Serialized_OrganizationId  "organizationId"

#define LOCALSTATE_Namespace                "Connect"
#define LOCALSTATE_SignedInUser             "SignedInUser"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> IConnectSignInManager::CheckAndUpdateToken()
    {
    BeMutexHolder lock(m_mutex);
    return _CheckAndUpdateToken()->Then<void>([=] (WSConnectVoidResult result)
        {
        if (!result.IsSuccess())
            {
            LOG.infov("Force UpdateteToken failed");
            }

        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> IConnectSignInManager::SignOut()
    {
    BeMutexHolder lock(m_mutex);
    return _SignOut()->Then<void>([=] (WSConnectVoidResult result)
        {
        if (!result.IsSuccess())
            {
            LOG.infov("ConnectSignOut failed");
            return;
            }

        LOG.infov("ConnectSignOut");
        OnUserSignedOut();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool IConnectSignInManager::IsSignedIn() const
    {
    BeMutexHolder lock(m_mutex);
    return _IsSignedIn();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectSignInManager::UserInfo IConnectSignInManager::GetUserInfo() const
    {
    BeMutexHolder lock(m_mutex);

    if (!_IsSignedIn())
        return UserInfo();

    return _GetUserInfo();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                           Andrius.Paulauskas     06/2016
//--------------------------------------------------------------------------------------
Utf8String IConnectSignInManager::GetLastUsername() const
    {
    BeMutexHolder lock(m_mutex);
    return m_secureLocalState->GetValue(LOCALSTATE_Namespace, LOCALSTATE_SignedInUser);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectTokenProviderPtr IConnectSignInManager::GetTokenProvider(Utf8StringCR rpUri) const
    {
    BeMutexHolder lock(m_mutex);
    return _GetTokenProvider(rpUri);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AuthenticationHandlerPtr IConnectSignInManager::GetAuthenticationHandler
    (
    Utf8StringCR serverUrl,
    IHttpHandlerPtr httpHandler,
    HeaderPrefix prefix
    ) const
    {
    BeMutexHolder lock(m_mutex);
    return _GetAuthenticationHandler(serverUrl, httpHandler, prefix);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::RegisterListener(IListener* listener)
    {
    BeMutexHolder lock(m_mutex);
    if (listener == nullptr)
        return;
    m_listeners.insert(listener);
    CheckUserChange();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::UnregisterListener(IListener* listener)
    {
    BeMutexHolder lock(m_mutex);
    m_listeners.erase(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::OnUserTokenExpired() const
    {
    auto listeners = m_listeners;
    for (auto listener : listeners)
        listener->_OnUserTokenExpired();

    if (m_tokenExpiredHandler)
        m_tokenExpiredHandler();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::OnUserTokenRenew(bool success, int64_t tokenExpireTimestamp) const
    {
    auto listeners = m_listeners;
    for (auto listener : listeners)
        listener->_OnUserTokenRenew(success, tokenExpireTimestamp);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::OnUserChanged() const
    {
    auto listeners = m_listeners;
    for (auto listener : listeners)
        listener->_OnUserChanged();

    if (m_userChangeHandler)
        m_userChangeHandler();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::OnUserSignedIn() const
    {
    auto listeners = m_listeners;
    for (auto listener : listeners)
        listener->_OnUserSignedIn();

    if (m_userSignInHandler)
        m_userSignInHandler();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::OnUserSignedOut() const
    {
    auto listeners = m_listeners;
    for (auto listener : listeners)
        listener->_OnUserSignedOut();

    if (m_userSignOutHandler)
        m_userSignOutHandler();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::OnUserSignedInViaConnectionClient() const
    {
    auto listeners = m_listeners;
    for (auto listener : listeners)
        listener->_OnUserSignedInViaConnectionClient();

    if (m_connectionClientSignInHandler)
        m_connectionClientSignInHandler();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::SetTokenExpiredHandler(std::function<void()> handler)
    {
    BeMutexHolder lock(m_mutex);
    m_tokenExpiredHandler = handler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::SetUserChangeHandler(std::function<void()> handler)
    {
    BeMutexHolder lock(m_mutex);
    m_userChangeHandler = handler;
    CheckUserChange();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::SetUserSignInHandler(std::function<void()> handler)
    {
    BeMutexHolder lock(m_mutex);
    m_userSignInHandler = handler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::SetUserSignOutHandler(std::function<void()> handler)
    {
    BeMutexHolder lock(m_mutex);
    m_userSignOutHandler = handler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::SetConnectionClientSignInHandler(std::function<void()> handler)
    {
    BeMutexHolder lock(m_mutex);
    m_connectionClientSignInHandler = handler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::CheckUserChange()
    {
    if (!_IsSignedIn())
        return;

    Utf8String storedUsername = GetLastUsername();
    if (storedUsername.empty())
        return;

    UserInfo info = GetUserInfo();
    BeAssert(!info.username.empty());
    if (info.username == storedUsername)
        return;

    OnUserChanged();
    StoreSignedInUser();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectSignInManager::UserInfo::UserInfo(Utf8StringCR serialized)
    {
    Json::Value json;
    if (!Json::Reader::Parse(serialized, json))
        return;

    username = json[USERINFO_Serialized_Username].asString().c_str();
    firstName = json[USERINFO_Serialized_FirstName].asString().c_str();
    lastName = json[USERINFO_Serialized_LastName].asString().c_str();
    userId = json[USERINFO_Serialized_UserId].asString().c_str();
    organizationId = json[USERINFO_Serialized_OrganizationId].asString().c_str();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IConnectSignInManager::StoreSignedInUser()
    {
    BeMutexHolder lock(m_mutex);
    UserInfo info = _GetUserInfo();
    BeAssert(!info.username.empty());
    m_secureLocalState->SaveValue(LOCALSTATE_Namespace, LOCALSTATE_SignedInUser, info.username.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IConnectSignInManager::UserInfo::IsComplete() const
    {
    return !(username.Equals("") || firstName.Equals("") || lastName.Equals("") || userId.Equals("") || organizationId.Equals(""));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IConnectSignInManager::UserInfo::ToString() const
    {
    Json::Value json;

    json[USERINFO_Serialized_Username] = username;
    json[USERINFO_Serialized_FirstName] = firstName;
    json[USERINFO_Serialized_LastName] = lastName;
    json[USERINFO_Serialized_UserId] = userId;
    json[USERINFO_Serialized_OrganizationId] = organizationId;

    return Json::FastWriter::ToString(json);
    }
