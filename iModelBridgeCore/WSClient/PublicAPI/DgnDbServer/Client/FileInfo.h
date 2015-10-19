/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/FileInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
struct FileInfo
{
//__PUBLISH_SECTION_END__
private:
    Utf8String m_localPath;
    Utf8String m_fileName;
    Utf8String m_url;
    uint64_t   m_fileSize;
//__PUBLISH_SECTION_START__
public:
    FileInfo(Utf8StringCR localPath);
    FileInfo(Utf8StringCR fileName, Utf8StringCR url, uint64_t fileSize);

    void SetLocalPath(Utf8StringCR localPath);
    void SetFileName(Utf8StringCR fileName);
    void SetURL(Utf8StringCR url);
    void SetFileSize(uint64_t fileSize);

    Utf8StringCR GetLocalPath();
    Utf8StringCR GetFileName();
    Utf8StringCR GetURL();
    uint64_t     GetFileSize();
};

typedef FileInfo& FileInfoR;
typedef const FileInfo& FileInfoCR;
END_BENTLEY_DGNDBSERVER_NAMESPACE
