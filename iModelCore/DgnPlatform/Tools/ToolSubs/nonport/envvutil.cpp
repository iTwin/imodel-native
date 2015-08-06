/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/nonport/envvutil.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/BeAssert.h>

#if defined (__unix__)

#if !defined (__APPLE__)
    #include "../../../DgnCore/DgnPlatformInternal.h"
#endif

#include <stdlib.h>
#include  <DgnPlatform/DesktopTools/envvutil.h>

BentleyStatus   util_getSysEnv (WStringP envValue, WCharCP pName) 
    {
    // *** WIP_NONPORT: Convert to UTF-8 or to locale?
    Utf8String utf8 (pName);

    char const* v = getenv (utf8.c_str());
    if (NULL == v)
        return BSIERROR;

    if (NULL != envValue)
        envValue->AssignA (v);

    return BSISUCCESS;
    }

BentleyStatus   util_putenv (WCharCP varName, WCharCP value)
    {
    if ( (NULL == varName) || (0 == *varName) )
        return BSIERROR;

    WString     formatted = varName;
    formatted.append (L"=");
    formatted.append (value);

    size_t  maxLocaleBytes = formatted.GetMaxLocaleCharBytes();
    char*   localeString = (char*) _alloca (maxLocaleBytes);
    // *** WIP_NONPORT: Convert to UTF-8 or to locale?
    formatted.ConvertToLocaleChars (localeString, maxLocaleBytes);

    putenv (localeString);
    }

StatusInt util_readRegistry (WStringR, WCharCP) {return ERROR;} // ***WIP_NONPORT

#elif defined (BENTLEY_WINRT)
// WIP_WINRT
#include "../../../DgnCore/DgnPlatformInternal.h"
#include <stdlib.h>
#include  <DgnPlatform/DesktopTools/envvutil.h>
BentleyStatus   util_getSysEnv (WStringP envValue, WCharCP pName) 
    {
BeDebugLog ("util_getSysEnv -- WIP_WINRT not supported on WinRT");
    return ERROR;
    }

BentleyStatus   util_putenv (WCharCP varName, WCharCP value)
    {
BeAssert (false && "WIP_WINRT - putenv not supported on WinRT");
    return ERROR;
    }

StatusInt util_readRegistry (WStringR, WCharCP) {BeAssert (false); return ERROR;} // ***WIP_NONPORT

#elif defined (BENTLEY_WIN32)
#define NOMINMAX
#include  <windows.h>
#include  <ShlObj.h>
#include  <stdlib.h>
#include  <objbase.h>
#include "..\DgnCore\DgnPlatformInternal.h"
#undef DGN_PLATFORM_MT
#include  <malloc.h>
#include  <Shlwapi.h>
#include  <DgnPlatform/Tools/ToolsAPI.h>
#include  <DgnPlatform/DesktopTools/envvutil.h>
#include <RmgrTools/Tools/UglyStrings.h>

#pragma comment (lib, "shell32")

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
*   pString <= Environment string (or NULL)
*   pName   => Environment variable name
*   maxLen  => Max output string length
* @bsimethod                                                    JVB             03/89
+---------------+---------------+---------------+---------------+---------------+------*/
Public BentleyStatus    util_getSysEnv (WStringP envValue, WCharCP pName)
    {
    wchar_t     tmpBuf[8192];               // Increased from 4 * MAXFILE Len - RBB 3/00
    DWORD       resultCode;

    if (NULL != envValue)
        envValue->clear();

    tmpBuf[0]  = 0;
    resultCode = ::GetEnvironmentVariableW (pName, tmpBuf, _countof (tmpBuf));
    BeAssert(0 != resultCode || ERROR_ENVVAR_NOT_FOUND == ::GetLastError());

    // didn't find it in environment. Is it a "CSIDL_"
    if (0 == tmpBuf[0])
        {
        int csidl = util_getCSIDL (pName);
        if (csidl >= 0)
            {
            HRESULT hr = ::SHGetFolderPathW (NULL, csidl, NULL, SHGFP_TYPE_CURRENT, tmpBuf);
            if (!SUCCEEDED (hr))
                tmpBuf[0] = 0;
            }
        }

    if (0 == tmpBuf[0])
        return BSIERROR;
    
    if (NULL != envValue)
        envValue->assign (tmpBuf);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             07/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public BentleyStatus    util_putenv (WCharCP varName, WCharCP value)
    {
    if ( (NULL == varName) || (0 == varName) )
        return BSIERROR;

    return  ::SetEnvironmentVariableW (varName, value) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt util_readRegistry (WStringR value, WCharCP data)
    {
    WChar   szFromCfg[MAX_PATH];
    WChar   buf[MAX_PATH];
    WCharP  pLastPart;
    WCharP  szKey;
    WCharP  szName;
    WCharP  pFirstPart;
    WCharCP keyName[] = {L"HKEY_CLASSES_ROOT",L"HKEY_CURRENT_USER",L"HKEY_USERS",L"HKEY_LOCAL_MACHINE",L"HKEY_CURRENT_CONFIG"};
    HKEY    keyValue[] = {HKEY_CLASSES_ROOT,HKEY_CURRENT_USER,HKEY_USERS,HKEY_LOCAL_MACHINE,HKEY_CURRENT_CONFIG};

    // __asm int 3;
    // Make a copy of the data to work on. Need orig for error msg.
    wcscpy (szFromCfg, data);
    // Crack the string into three parts.
    pLastPart =  ::wcsrchr (szFromCfg, L'\\');
    if (NULL == pLastPart)
        return ERROR;
    szName = pLastPart + 1;
    pFirstPart =  ::wcschr (szFromCfg, L'\\');
    szKey = pFirstPart +1;
    // Fire null chars into the string to crack it into three parts.
    *pFirstPart = '\0';
    *pLastPart  = '\0';

    // Translate the hive name.
    int     iKey;
    for (iKey=0; iKey < _countof (keyName); iKey++)
        {
        if (0 == wcscmp (keyName[iKey], szFromCfg))
            break;
        }

    if (iKey >= _countof (keyName))
        return ERROR;

    if (ERROR_SUCCESS != ::SHRegGetPathW (keyValue[iKey], szKey, szName, buf, 0))
        return ERROR;

    ::PathAddBackslashW (buf);
    value.assign (buf);

    return SUCCESS;    
    }


#endif
