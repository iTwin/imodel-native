/*-------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/FileUtilities.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <windows.h>
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

ECFileNameIterator::ECFileNameIterator (const wchar_t * path)
    {
    m_valid = true;
    m_findHandle = ::FindFirstFileW (path, &m_findData);
    _wsplitpath(path, m_deviceName, m_dirName, NULL, NULL);
    }
    
ECFileNameIterator::~ECFileNameIterator ()
    {
    if (INVALID_HANDLE_VALUE != m_findHandle)
        ::FindClose (m_findHandle);
    }
    
StatusInt ECFileNameIterator::GetNextFileName (wchar_t * name)
    {
    if (INVALID_HANDLE_VALUE == m_findHandle || !m_valid)
        return  ERROR;

    _wmakepath (name, m_deviceName, m_dirName, m_findData.cFileName, NULL);
    m_valid = 0 != ::FindNextFileW (m_findHandle, &m_findData);
    return  SUCCESS;
    }
    
END_BENTLEY_EC_NAMESPACE