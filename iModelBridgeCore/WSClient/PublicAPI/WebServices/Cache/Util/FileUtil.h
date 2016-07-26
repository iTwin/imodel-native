/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Util/FileUtil.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <Bentley/Tasks/CancellationToken.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeFileName.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileUtil
    {
    public:
        typedef std::function<void(double bytesTransfered, double bytesTotal)> ProgressCallback;

    private:
        static BentleyStatus CopyFileContent
            (
            BeFileNameCR sourcePath,
            BeFile& source,
            BeFileNameCR targetPath,
            BeFile& target,
            ProgressCallback onProgress,
            ICancellationTokenPtr ct
            );

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

        //! Removes invalid characters in file name so it can be saved to disk.
        WSCACHE_EXPORT static Utf8String SanitizeFileName(Utf8StringCR fileName);

        //! Copy file content from destanation file to target file to override it.
        WSCACHE_EXPORT static BentleyStatus CopyFileContent
            (
            BeFileNameCR source,
            BeFileNameCR target,
            ProgressCallback onProgress = nullptr,
            ICancellationTokenPtr ct = nullptr
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
