/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/FileUtilities.h $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
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
    ECFileNameIterator (const wchar_t * path);
    ~ECFileNameIterator ();
    BentleyStatus GetNextFileName (wchar_t * name);
     };

struct ECFileUtilities
    {
private:
    static bwstring s_dllPath;
    ECFileUtilities(void) {}
    
public:
    static bwstring GetDllPath();
    };

END_BENTLEY_EC_NAMESPACE