/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../RealityPlatformToolsCurl/WSGServices.cpp"
#include <CCApi/CCPublic.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

PUSH_DISABLE_DEPRECATION_WARNINGS
//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void ConnectTokenManager::RefreshToken() const
    {
    if(m_tokenCallback)
        return m_tokenCallback(m_token, m_tokenRefreshTimer);

    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled(api, &installed);
    if (!installed)
        {
        std::cout << "Connection client does not seem to be installed" << std::endl;
        CCApi_FreeApi(api);
        return;
        }
    bool running = false;
    status = CCApi_IsRunning(api, &running);
    if (status != APIERR_SUCCESS || !running)
        {
        std::cout << "Connection client does not seem to be running" << std::endl;
        CCApi_FreeApi(api);
        return;
        }
    bool loggedIn = false;
    status = CCApi_IsLoggedIn(api, &loggedIn);
    if (status != APIERR_SUCCESS || !loggedIn)
        {
        std::cout << "Connection client does not seem to be logged in" << std::endl;
        CCApi_FreeApi(api);
        return;
        }
    bool acceptedEula = false;
    status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
    if (status != APIERR_SUCCESS || !acceptedEula)
        {
        std::cout << "Connection client user does not seem to have accepted EULA" << std::endl;
        CCApi_FreeApi(api);
        return;
        }
    bool sessionActive = false;
    status = CCApi_IsUserSessionActive(api, &sessionActive);
    if (status != APIERR_SUCCESS || !sessionActive)
        {
        std::cout << "Connection client does not seem to have an active session" << std::endl;
        CCApi_FreeApi(api);
        return;
        }

    LPCWSTR relyingParty = L"https://connect-wsg20.bentley.com";//;L"https:://qa-ims.bentley.com"
    UINT32 maxTokenLength = 16384;
    LPWSTR lpwstrToken = new WCHAR[maxTokenLength];

    status = CCApi_GetSerializedDelegateSecurityToken(api, relyingParty, lpwstrToken, maxTokenLength);
    if (status != APIERR_SUCCESS)
        {
        CCApi_FreeApi(api);
        return;
        }

    char* charToken = new char[maxTokenLength];
    wcstombs(charToken, lpwstrToken, maxTokenLength);

    m_token = "Authorization: Token ";
    m_token.append(charToken);
    m_tokenRefreshTimer = std::time(nullptr);

    delete lpwstrToken;
    delete charToken;
    CCApi_FreeApi(api);
    }
POP_DISABLE_DEPRECATION_WARNINGS
