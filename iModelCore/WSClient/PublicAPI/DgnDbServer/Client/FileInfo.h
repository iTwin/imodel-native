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
    DGNDBSERVERCLIENT_EXPORT FileInfo();
    DGNDBSERVERCLIENT_EXPORT FileInfo(Utf8StringCR fileName, Utf8StringCR url, uint64_t fileSize);
    DGNDBSERVERCLIENT_EXPORT FileInfo(Utf8StringCR localPath, Utf8StringCR fileName, Utf8StringCR url, uint64_t fileSize);

    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetLocalPath() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetFileName() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetURL() const;
    DGNDBSERVERCLIENT_EXPORT uint64_t     GetFileSize() const;
};

typedef FileInfo& FileInfoR;
typedef const FileInfo& FileInfoCR;
END_BENTLEY_DGNDBSERVER_NAMESPACE
