/*--------------------------------------------------------------------------------------+
|
|     $Source: src/FileUtilities.h $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects\ECObjects.h>

BEGIN_BENTLEY_EC_NAMESPACE

struct ECFileNameIterator
    {
    WIN32_FIND_DATAW m_findData;
    HANDLE           m_findHandle;
    bool             m_valid;
    wchar_t          m_deviceName[_MAX_DRIVE];
    wchar_t          m_dirName[_MAX_DIR];

public:
    ECFileNameIterator (WCharCP path);
    ~ECFileNameIterator ();
    BentleyStatus GetNextFileName (WCharP name);
     };

struct ECFileUtilities
    {
private:
    static WString s_dllPath;
    ECFileUtilities(void) {}
    
public:
    static WString GetDllPath();
    };

END_BENTLEY_EC_NAMESPACE