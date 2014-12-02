/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/nonport/csidl.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#define NOMINMAX
#include <windows.h>
#include <Bentley/Bentley.h>
#include <Bentley/bmap.h>
#include <DgnPlatform/DesktopTools/envvutil.h>

/*=================================================================================**//**
* @bsiclass                                                     Philip.McGraw   10/2004
+===============+===============+===============+===============+===============+======*/
class CsidlMap
    {
private:
    bmap <WString, int> m_csidl;
public:
    CsidlMap();
    int GetCsidl(WCharCP name);
    };

static CsidlMap csidlMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    10/2004
+---------------+---------------+---------------+---------------+---------------+------*/
int             util_getCSIDL (WCharCP name)
    {
    // only consider names with prefix
    if (0 != wcsncmp (name, L"CSIDL_", 6))
        return -1;

    return csidlMap.GetCsidl (name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    10/2004
+---------------+---------------+---------------+---------------+---------------+------*/
int CsidlMap::GetCsidl (WCharCP name)
    {
    int csidl = (-1);
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
CsidlMap::CsidlMap()
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
