#include "../RealityPlatform/SimpleRDSApi.cpp"
#include <CCApi/CCPublic.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
Utf8String RDSRequestManager::MakeBuddiCall()
    {
    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled(api, &installed);
    if (!installed)
        {
        ReportError("Connection client does not seem to be installed");
        return "";
        }
    bool running = false;
    status = CCApi_IsRunning(api, &running);
    if (status != APIERR_SUCCESS || !running)
        {
        ReportError("Connection client does not seem to be running");
        return "";
        }
    bool loggedIn = false;
    status = CCApi_IsLoggedIn(api, &loggedIn);
    if (status != APIERR_SUCCESS || !loggedIn)
        {
        ReportError("Connection client does not seem to be logged in");
        return "";
        }
    bool acceptedEula = false;
    status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
    if (status != APIERR_SUCCESS || !acceptedEula)
        {
        ReportError("Connection client user does not seem to have accepted EULA");
        return "";
        }
    bool sessionActive = false;
    status = CCApi_IsUserSessionActive(api, &sessionActive);
    if (status != APIERR_SUCCESS || !sessionActive)
        {
        ReportError("Connection client does not seem to have an active session\n");
        return "";
        }

    wchar_t* buddiUrl;
    UINT32 strlen = 0;

    CCApi_GetBuddiUrl(api, L"RealityDataServices", NULL, &strlen);
    strlen += 1;
    buddiUrl = (wchar_t*)malloc((strlen) * sizeof(wchar_t));
    CCApi_GetBuddiUrl(api, L"RealityDataServices", buddiUrl, &strlen);

    char* charServer = new char[strlen];
    wcstombs(charServer, buddiUrl, strlen);

    CCApi_FreeApi(api);

    return Utf8String(charServer);
    }