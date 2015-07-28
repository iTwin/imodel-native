/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Util/FileUtil.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeFileName.h>
#include <Bentley/WString.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileUtil
    {
    private:
        static BentleyStatus CopyFileContent(BeFileNameCR sourcePath, BeFile& source, BeFileNameCR targetPath, BeFile& target);

    public:
        //! Get file size in bytes. Returns zero on failure
        WSCACHE_EXPORT static uint64_t GetFileSize(BeFileNameCR filePath);

        //! Trims long file name to platfom specific length preserving extension
        WSCACHE_EXPORT static Utf8String TruncateFileName(Utf8StringCR fileName);

        //! Trim long file name to max length in bytes preserving extension.
        //! Use TruncateFileName() for platform independent code
        WSCACHE_EXPORT static Utf8String TruncateFileNameUtf8(Utf8StringCR fileName, size_t maxLengthBytes);

        //! Trim long file name to max length in Utf16 characters preserving extension
        //! Use TruncateFileName() for platform independent code
        WSCACHE_EXPORT static Utf8String TruncateFileNameUtf16(Utf8StringCR fileName, size_t maxLengthCharacters);

        //! Trims long file name to platfom specific max path length preserving extension. If triming cannot be done error is returned
        WSCACHE_EXPORT static BentleyStatus TruncateFilePath(BeFileNameR filePath);
        WSCACHE_EXPORT static BentleyStatus TruncateFilePath(BeFileNameR filePath, size_t maxPath);

        //! Copy file content from destanation file to target file to override it.
        WSCACHE_EXPORT static BentleyStatus CopyFileContent(BeFileNameCR source, BeFileNameCR target);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
