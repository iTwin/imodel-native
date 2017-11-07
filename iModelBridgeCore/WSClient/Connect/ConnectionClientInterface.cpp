/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectionClientInterface.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "ConnectionClientInterface.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
const int GUID_BUFFER_SIZE = 1024;

bset<event_callback>  ConnectionClientInterface::s_listeners;
BeMutex ConnectionClientInterface::s_mutex;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Mark.Uvari    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectionClientInterface::ConnectionClientInterface()
    {
    m_ccApi = CCApi_InitializeApi(COM_THREADING_Multi);
    CCApi_AddClientEventListener(m_ccApi, EventListener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Mark.Uvari    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectionClientInterface::~ConnectionClientInterface()
    {
    CCApi_RemoveClientEventListener(m_ccApi, EventListener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectionClientInterface::IsInstalled()
    {
    bool installed;
    CallStatus status = CCApi_IsInstalled(m_ccApi, &installed);
    if (status != APIERR_SUCCESS)
        return false;
    return installed;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr ConnectionClientInterface::GetSerializedDelegateSecurityToken(Utf8StringCR rpUri)
    {
    WString rpUriW(rpUri.c_str(), true);
    LPCWSTR uri = rpUriW.c_str();

    CCDATABUFHANDLE buffer = NULL;
    CallStatus status = CCApi_GetSerializedDelegateSecurityTokenBuffer(m_ccApi, uri, &buffer);
    if (status != APIERR_SUCCESS)
        {
        CCApi_DataBufferFree(buffer);
        return nullptr;
        }

    size_t tokenLength = 0;
    status = CCApi_DataBufferGetStringLength(buffer, TOKEN_BUFF_TOKEN, 0, &tokenLength);
    if (status != APIERR_SUCCESS)
        {
        CCApi_DataBufferFree(buffer);
        return nullptr;
        }

    tokenLength++;
    std::unique_ptr<WCHAR> tokenBuffer(new WCHAR[tokenLength]);

    status = CCApi_DataBufferGetStringProperty(buffer, TOKEN_BUFF_TOKEN, 0, tokenBuffer.get(), (UINT32)tokenLength);
    CCApi_DataBufferFree(buffer);
    if (status != APIERR_SUCCESS)
        return nullptr;

    Utf8String tokenStrBase64(tokenBuffer.get());
    Utf8String tokenStr = Base64Utilities::Decode(tokenStrBase64);
    SamlTokenPtr token = std::make_shared<SamlToken>(tokenStr);

    return token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectionClientInterface::IsRunning()
    {
    bool running = false;
    CallStatus status = CCApi_IsRunning(m_ccApi, &running);
    if (status != APIERR_SUCCESS)
        return false;
    return running;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConnectionClientInterface::StartClientApp()
    {
    CallStatus status = CCApi_StartClientApp(m_ccApi, false);
    if (status != APIERR_SUCCESS)
        return BentleyStatus::ERROR;
    return BentleyStatus::SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectionClientInterface::IsLoggedIn()
    {
    bool loggedIn = false;
    CallStatus status = CCApi_IsLoggedIn(m_ccApi, &loggedIn);
    if (status != APIERR_SUCCESS)
        return false;
    return loggedIn;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CCDATABUFHANDLE ConnectionClientInterface::GetUserInformation()
    {
    CCDATABUFHANDLE buffer = NULL;
    CCApi_GetUserInformation(m_ccApi, &buffer);
    return buffer;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ConnectionClientInterface::GetUserId()
    {
    CCDATABUFHANDLE userInfo = GetUserInformation();

    wchar_t userId[GUID_BUFFER_SIZE];
    CCApi_DataBufferGetStringProperty(userInfo, USER_BUFF_BPID, 0, userId, GUID_BUFFER_SIZE);
    Utf8String utfUserId;
    BeStringUtilities::WCharToUtf8(utfUserId, userId);

    return utfUserId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionClientInterface::AddClientEventListener(event_callback callback)
    {
    s_listeners.insert(callback);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionClientInterface::EventListener(int eventId, WCharCP data)
    {
    BeMutexHolder lock(s_mutex);
    for (auto listener : s_listeners)
        (*listener)(eventId, data);
    }