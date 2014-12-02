/*----------------------------------------------------------------------+
|
|     $Source: DgnCore/nonport_utilities.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32) // WIP_NONPORT
#include <Windows.h>
#include <objbase.h>
#elif defined (BENTLEY_WINRT)
    // WIP_WINRT
#endif

#include "DgnPlatformInternal.h"

#if defined (BENTLEY_WIN32) // WIP_NONPORT
#include <io.h>
#endif

#if defined (BENTLEY_WIN32)
#include <Wininet.h>

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// checkCanAccessUrl ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
// NEEDS_WORK: Find a home for this non-portable stuff
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       05/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void    ie_logError
(
void
)
    {
#if defined (DEBUG)
    wchar_t    errText[500];
    DWORD   len = _countof(errText), err2;

    InternetGetLastResponseInfo (&err2, errText, &len);
    printf ("...FAILED! GLE %#x/%d, IGLRI %#x, '%ls'\n", GetLastError(), GetLastError()-INTERNET_ERROR_BASE, err2, errText);
    DebugBreak();
#endif
    }


struct InternetHandleHolder
{
    HINTERNET   m_h;
    InternetHandleHolder (HINTERNET h) : m_h(h) {;}
    ~InternetHandleHolder () {InternetCloseHandle (m_h);}
};

enum { MAX_AUTHORIZATION_URL_RETRIES = 5 };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/03
+---------------+---------------+---------------+---------------+---------------+------*/
EXPORT_ATTRIBUTE StatusInt checkCanAccessUrl (char const* url)
    {
    //  Get host, filename, scheme, port, etc. from input URL
    //
    UInt32      serviceType = INTERNET_SERVICE_HTTP;
    bool        usingSSL = false;
    INTERNET_PORT port = INTERNET_DEFAULT_HTTP_PORT;
    char        hostname[256], filename[256];
    *hostname = *filename = 0;
        {
        URL_COMPONENTSA uc;
        memset (&uc, 0, sizeof(uc));
        uc.dwStructSize     = sizeof(uc);
        uc.lpszHostName     = hostname;
        uc.dwHostNameLength = sizeof(hostname);
        uc.lpszUrlPath      = filename;
        uc.dwUrlPathLength  = sizeof(filename);

        if (!::InternetCrackUrlA (url, 0, 0, &uc))
            {
/*<=*/      return ERROR;
            }

        if (INTERNET_SCHEME_FTP == uc.nScheme)
            {
#if defined (NEEDS_WORK)
            serviceType = INTERNET_SERVICE_FTP;
#endif
/*<=*/      return ERROR;
            }

        if (INTERNET_SCHEME_HTTPS == uc.nScheme)
            usingSSL = true;

        port = uc.nPort;
        }

    //  Try to access server
    //
    HINTERNET hOpenHandle = ::InternetOpenA ("DgnV8FileIOLicense", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    InternetHandleHolder _closeOpenHandle (hOpenHandle);

    HINTERNET hConnectHandle = ::InternetConnectA (hOpenHandle, hostname, port, NULL, NULL, serviceType, 0, 0);
    InternetHandleHolder _closeConnectHandle (hConnectHandle);

    DWORD   const iReqFlags = INTERNET_FLAG_KEEP_CONNECTION | (usingSSL ? INTERNET_FLAG_SECURE : 0);
    HINTERNET hResourceHandle = ::HttpOpenRequestA (hConnectHandle, "GET", filename, NULL, NULL, NULL, iReqFlags, 0);
    InternetHandleHolder _closeResourceHandle (hResourceHandle);

    for (int hope=0; hope < MAX_AUTHORIZATION_URL_RETRIES; hope++)
        {
        if (!HttpSendRequest (hResourceHandle, NULL, 0, NULL, 0)) // Get to the machine/server
            {
            ie_logError ();
/*<=*/      return ERROR;
            }

        DWORD dwHttpStatCode, dwHttpStatCodeSize = sizeof (DWORD);
        if (!HttpQueryInfo (hResourceHandle, HTTP_QUERY_STATUS_CODE | // Check access to page/file/resource
                                             HTTP_QUERY_FLAG_NUMBER,
                                             &dwHttpStatCode,
                                             &dwHttpStatCodeSize, 0))
            {
            ie_logError ();
/*<=*/      return ERROR;
            }

        switch (dwHttpStatCode)
            {
            case HTTP_STATUS_OK:
/*<=*/          return SUCCESS;

            case HTTP_STATUS_NOT_FOUND:
/*<=*/          return ERROR;

            case HTTP_STATUS_DENIED:
            case HTTP_STATUS_PROXY_AUTH_REQ:
                {
                //  Prompt the user for password, etc.
                #if defined (NEEDS_WORK)
                    HWND        hwndUstnTop;
                    mdlWin32_getInternalData (&hwndUstnTop, sizeof(hwndUstnTop), 110, 0, 0);
                #else
                    HWND const hwndUstnTop = GetActiveWindow ();
                #endif

                DWORD dwError = InternetErrorDlg (hwndUstnTop, hResourceHandle, ERROR_INTERNET_INCORRECT_PASSWORD,
                                           FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
                                           FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS |
                                           FLAGS_ERROR_UI_FLAGS_GENERATE_DATA,
                                           NULL);

                //  Retry the request with authentication info collected from the user
                if (ERROR_INTERNET_FORCE_RETRY == dwError)
                    continue;

                break;
                }

            default:
                ie_logError ();
/*<=*/          return ERROR;
            }
        }

    return ERROR;
    }

#elif defined (__unix__) || defined (BENTLEY_WINRT)

StatusInt checkCanAccessUrl (char const* url)
    {
    /*WIP_NONPORT*/
    return ERROR;
    }

#endif
