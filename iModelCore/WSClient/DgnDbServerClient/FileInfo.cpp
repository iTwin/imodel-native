/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/FileInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/FileInfo.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER

FileInfo::FileInfo()
    {};

FileInfo::FileInfo(Utf8StringCR fileName, Utf8StringCR url, uint64_t fileSize) :
    m_fileName(fileName), m_url(url), m_fileSize(fileSize)
    {};

FileInfo::FileInfo(Utf8StringCR localPath, Utf8StringCR fileName, Utf8StringCR url, uint64_t fileSize) :
    m_localPath(localPath), m_fileName(fileName), m_url(url), m_fileSize(fileSize)
    {};

Utf8StringCR FileInfo::GetLocalPath() const
    {
    return m_localPath;
    }

Utf8StringCR FileInfo::GetFileName() const
    {
    return m_fileName;
    }

Utf8StringCR FileInfo::GetURL() const
    {
    return m_url;
    }

uint64_t FileInfo::GetFileSize() const
    {
    return m_fileSize;
    }
