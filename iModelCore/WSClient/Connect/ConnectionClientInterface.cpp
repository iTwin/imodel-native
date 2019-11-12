/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "ConnectionClientInterface.h"
#include "Connect.xliff.h"

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
void ConnectionClientInterface::CCApiStatusToString(int status, Utf8StringP errorStringOut)
    {
    Utf8String statusString;
    switch (status)
        {
        case APIERR_SUCCESS:
            break; // Do nothing
        case APIERR_CLIENT_NOT_INSTALLED:
            statusString = ConnectLocalizedString(ALERT_CCNotInstalledError);
            break;
        case APIERR_CLIENT_NOT_RUNNING:
            statusString = ConnectLocalizedString(ALERT_CCNotRunningError);
            break;
        case APIERR_INVALID_USER_OR_PASSWORD:
            statusString = ConnectLocalizedString(ALERT_CCInvalidCredentialsError);
            break;
        case APIERR_UNHANDLED_EXCEPTION:
            statusString = ConnectLocalizedString(ALERT_CCUnhandledExceptionError);
            break;
        case APIERR_NOT_LOGGED_IN:
            statusString = ConnectLocalizedString(ALERT_ConnectionClientNotLoggedIn_Message);
            break;
        case APIERR_NOT_ACCEPTED_EULA:
            statusString = ConnectLocalizedString(ALERT_CCNotAcceptedEulaError);
            break;
        case APIERR_UNABLE_TO_START_APP:
            statusString = ConnectLocalizedString(ALERT_CCUnableToStartAppError);
            break;
        case APIERR_API_OBSOLETE:
            statusString = ConnectLocalizedString(ALERT_CCApiObsoleteError);
            break;
        case APIERR_SERVICE_UNAVAILABLE:
            statusString = ConnectLocalizedString(ALERT_CCServiceUnavailableError);
            break;
        case APIERR_USER_NOT_AFFILIATED:
            statusString = ConnectLocalizedString(ALERT_CCUserNotAffiliatedError);
            break;
        case APIERR_NOT_FOUND:
        case APIERR_INVALID_ARGUMENT:
        case APIERR_NOT_SUPPORTED:
        case APIERR_FOLDER_NOT_FOUND:
        case APIERR_UNKNOWN:
        default:
            statusString = ConnectLocalizedString(ALERT_CCUnknownError);
            break;
        }
    if (errorStringOut != nullptr)
        *errorStringOut = statusString;
    if (!statusString.empty())
        LOG.errorv("CC error status: %d (%s)", status, statusString.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr ConnectionClientInterface::GetSerializedDelegateSecurityToken(Utf8StringCR rpUri, Utf8StringP errorStringOut)
    {
    WString rpUriW(rpUri.c_str(), true);
    LPCWSTR uri = rpUriW.c_str();

    CCDATABUFHANDLE buffer = NULL;
    CallStatus status = CCApi_GetSerializedDelegateSecurityTokenBuffer(m_ccApi, uri, &buffer);
    if (status != APIERR_SUCCESS)
        {
        CCApi_DataBufferFree(buffer);
        CCApiStatusToString(status, errorStringOut);
        return nullptr;
        }

    size_t tokenLength = 0;
    status = CCApi_DataBufferGetStringLength(buffer, TOKEN_BUFF_TOKEN, 0, &tokenLength);
    if (status != APIERR_SUCCESS)
        {
        CCApi_DataBufferFree(buffer);
        CCApiStatusToString(status, errorStringOut);
        return nullptr;
        }

    tokenLength++;
    std::unique_ptr<WCHAR> tokenBuffer(new WCHAR[tokenLength]);

    status = CCApi_DataBufferGetStringProperty(buffer, TOKEN_BUFF_TOKEN, 0, tokenBuffer.get(), (UINT32)tokenLength);
    CCApi_DataBufferFree(buffer);
    if (status != APIERR_SUCCESS)
        {
        CCApiStatusToString(status, errorStringOut);
        return nullptr;
        }

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