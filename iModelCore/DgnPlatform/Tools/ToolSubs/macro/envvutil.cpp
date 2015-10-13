/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/macro/envvutil.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DesktopTools/envvutil.h>
#include "macro.h"

#if defined (__unix__)

#include <stdlib.h>

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


//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
#elif defined (BENTLEY_WINRT)
// WIP_WINRT
#include "../../../DgnPlatformInternal.h"
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

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
#elif defined (BENTLEY_WIN32)
#include  <windows.h>
#include  <ShlObj.h>
#include  <stdlib.h>
#include  <objbase.h>
#undef DGN_PLATFORM_MT
#include  <malloc.h>
#include  <Shlwapi.h>
#pragma comment (lib, "shell32")

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM


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

/*=================================================================================**//**
* @bsiclass                                                     Philip.McGraw   10/2004
+===============+===============+===============+===============+===============+======*/
struct  CsidlMap
{
bmap <WString, int> m_csidl;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    10/2004
+---------------+---------------+---------------+---------------+---------------+------*/
int GetCsidl (WCharCP name)
    {
    // only consider names with prefix
    if (0 != wcsncmp (name, L"CSIDL_", 6))
        return -1;

    int csidl = -1;

    // first lookup name in the map
    bmap<WString, int>::iterator pos = m_csidl.find(WString(name));
    if (m_csidl.end() != pos)
        return pos->second;

    WCharP endptr;
    csidl = (int)wcstoul(name + 6, &endptr, 0);
    // docs say strtoul failure may return 0, LONG_MIN, or LONG_MAX
    if ((0 == csidl) || (LONG_MIN == csidl) || (LONG_MAX == csidl))
        csidl = (-1);

    return csidl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    10/2004
+---------------+---------------+---------------+---------------+---------------+------*/
CsidlMap()
    {
    // initialize map with known CSIDL names from VC7.1 ShlObj.h:
    m_csidl[L"CSIDL_DESKTOP"]                    = 0x0000;        // <desktop>
    m_csidl[L"CSIDL_INTERNET"]                   = 0x0001;        // Internet Explorer (icon on desktop)
    m_csidl[L"CSIDL_PROGRAMS"]                   = 0x0002;        // Start Menu\Programs
    m_csidl[L"CSIDL_CONTROLS"]                   = 0x0003;        // My Computer\Control Panel
    m_csidl[L"CSIDL_PRINTERS"]                   = 0x0004;        // My Computer\Printers
    m_csidl[L"CSIDL_PERSONAL"]                   = 0x0005;        // My Documents
    m_csidl[L"CSIDL_FAVORITES"]                  = 0x0006;        // <user name>\Favorites
    m_csidl[L"CSIDL_STARTUP"]                    = 0x0007;        // Start Menu\Programs\Startup
    m_csidl[L"CSIDL_RECENT"]                     = 0x0008;        // <user name>\Recent
    m_csidl[L"CSIDL_SENDTO"]                     = 0x0009;        // <user name>\SendTo
    m_csidl[L"CSIDL_BITBUCKET"]                  = 0x000a;        // <desktop>\Recycle Bin
    m_csidl[L"CSIDL_STARTMENU"]                  = 0x000b;        // <user name>\Start Menu
    m_csidl[L"CSIDL_MYDOCUMENTS"]                = 0x000c;        // logical "My Documents" desktop icon
    m_csidl[L"CSIDL_MYMUSIC"]                    = 0x000d;        // "My Music" folder
    m_csidl[L"CSIDL_MYVIDEO"]                    = 0x000e;        // "My Videos" folder
    m_csidl[L"CSIDL_DESKTOPDIRECTORY"]           = 0x0010;        // <user name>\Desktop
    m_csidl[L"CSIDL_DRIVES"]                     = 0x0011;        // My Computer
    m_csidl[L"CSIDL_NETWORK"]                    = 0x0012;        // Network Neighborhood (My Network Places)
    m_csidl[L"CSIDL_NETHOOD"]                    = 0x0013;        // <user name>\nethood
    m_csidl[L"CSIDL_FONTS"]                      = 0x0014;        // windows\fonts
    m_csidl[L"CSIDL_TEMPLATES"]                  = 0x0015;
    m_csidl[L"CSIDL_COMMON_STARTMENU"]           = 0x0016;        // All Users\Start Menu
    m_csidl[L"CSIDL_COMMON_PROGRAMS"]            = 0x0017;        // All Users\Start Menu\Programs
    m_csidl[L"CSIDL_COMMON_STARTUP"]             = 0x0018;        // All Users\Startup
    m_csidl[L"CSIDL_COMMON_DESKTOPDIRECTORY"]    = 0x0019;        // All Users\Desktop
    m_csidl[L"CSIDL_APPDATA"]                    = 0x001a;        // <user name>\Application Data
    m_csidl[L"CSIDL_PRINTHOOD"]                  = 0x001b;        // <user name>\PrintHood
    m_csidl[L"CSIDL_LOCAL_APPDATA"]              = 0x001c;        // <user name>\Local Settings\Applicaiton Data (non roaming)
    m_csidl[L"CSIDL_ALTSTARTUP"]                 = 0x001d;        // non localized startup
    m_csidl[L"CSIDL_COMMON_ALTSTARTUP"]          = 0x001e;        // non localized common startup
    m_csidl[L"CSIDL_COMMON_FAVORITES"]           = 0x001f;
    m_csidl[L"CSIDL_INTERNET_CACHE"]             = 0x0020;
    m_csidl[L"CSIDL_COOKIES"]                    = 0x0021;
    m_csidl[L"CSIDL_HISTORY"]                    = 0x0022;
    m_csidl[L"CSIDL_COMMON_APPDATA"]             = 0x0023;        // All Users\Application Data
    m_csidl[L"CSIDL_WINDOWS"]                    = 0x0024;        // GetWindowsDirectory()
    m_csidl[L"CSIDL_SYSTEM"]                     = 0x0025;        // GetSystemDirectory()
    m_csidl[L"CSIDL_PROGRAM_FILES"]              = 0x0026;        // C:\Program Files
    m_csidl[L"CSIDL_MYPICTURES"]                 = 0x0027;        // C:\Program Files\My Pictures
    m_csidl[L"CSIDL_PROFILE"]                    = 0x0028;        // USERPROFILE
    m_csidl[L"CSIDL_SYSTEMX86"]                  = 0x0029;        // x86 system directory on RISC
    m_csidl[L"CSIDL_PROGRAM_FILESX86"]           = 0x002a;        // x86 C:\Program Files on RISC
    m_csidl[L"CSIDL_PROGRAM_FILES_COMMON"]       = 0x002b;        // C:\Program Files\Common
    m_csidl[L"CSIDL_PROGRAM_FILES_COMMONX86"]    = 0x002c;        // x86 Program Files\Common on RISC
    m_csidl[L"CSIDL_COMMON_TEMPLATES"]           = 0x002d;        // All Users\Templates
    m_csidl[L"CSIDL_COMMON_DOCUMENTS"]           = 0x002e;        // All Users\Documents
    m_csidl[L"CSIDL_COMMON_ADMINTOOLS"]          = 0x002f;        // All Users\Start Menu\Programs\Administrative Tools
    m_csidl[L"CSIDL_ADMINTOOLS"]                 = 0x0030;        // <user name>\Start Menu\Programs\Administrative Tools
    m_csidl[L"CSIDL_CONNECTIONS"]                = 0x0031;        // Network and Dial-up Connections
    m_csidl[L"CSIDL_COMMON_MUSIC"]               = 0x0035;        // All Users\My Music
    m_csidl[L"CSIDL_COMMON_PICTURES"]            = 0x0036;        // All Users\My Pictures
    m_csidl[L"CSIDL_COMMON_VIDEO"]               = 0x0037;        // All Users\My Video
    m_csidl[L"CSIDL_RESOURCES"]                  = 0x0038;        // Resource Direcotry
    m_csidl[L"CSIDL_RESOURCES_LOCALIZED"]        = 0x0039;        // Localized Resource Direcotry
    m_csidl[L"CSIDL_COMMON_OEM_LINKS"]           = 0x003a;        // Links to All Users OEM specific apps
    m_csidl[L"CSIDL_CDBURN_AREA"]                = 0x003b;        // USERPROFILE\Local Settings\Application Data\Microsoft\CD Burning
    m_csidl[L"CSIDL_COMPUTERSNEARME"]            = 0x003d;        // Computers Near Me (computered from Workgroup membership)
    }
};

static CsidlMap*    s_csidlMap;

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
        if (nullptr == s_csidlMap)
            s_csidlMap = new CsidlMap;

        int csidl = s_csidlMap->GetCsidl (pName);
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

#endif
