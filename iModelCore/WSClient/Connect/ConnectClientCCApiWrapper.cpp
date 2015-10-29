/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectClientCCApiWrapper.cpp $ 
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"

#include <BentleyDesktopClient/CCApi/CCPublic.h>
#include <WebServices/Connect/ConnectClientCCApiWrapper.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Brad.Hadden    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectClientCCApiWrapper::ConnectClientCCApiWrapper ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Brad.Hadden    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring ConnectClientCCApiWrapper::GetSerializedDelegateSecurityToken (std::wstring relyingParty)
    {
    std::wstring stsToken (L"");
    CCAPIHANDLE api = CCApi_InitializeApi (COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled (api, &installed);
    if (status != APIERR_SUCCESS || !installed)
        {
        return stsToken;
        }

    bool loggedIn;
    status = CCApi_IsLoggedIn (api, &loggedIn);
    if (status != APIERR_SUCCESS || !loggedIn)
        {
        return stsToken;
        }

    wchar_t buffer[32768];
    UINT32 bufferLength = 32768;

    status = CCApi_GetSerializedDelegateSecurityToken (api, const_cast<wchar_t*>(relyingParty.c_str ()), buffer, bufferLength);

    if (status != APIERR_SUCCESS)
        {
        return stsToken;
        }

    stsToken = buffer;
    return stsToken;
    }
