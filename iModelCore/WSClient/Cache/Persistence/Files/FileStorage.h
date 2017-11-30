/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Files/FileStorage.h $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ValueIncrementor.h>
#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>
#include <WebServices/Cache/Persistence/IDataSourceCache.h>
#include "FileInfo.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileStorage
    {
    private:
        CacheEnvironment m_environment;
        std::shared_ptr<ValueIncrementor> m_folderNameIncrementor;

    private:
        static BeFileName CreateFileStoragePath(BeFileName rootDir, WStringCR cacheName);
        BeFileName CreateNewRelativeCacheDir();

        BentleyStatus StoreFile(FileInfoR info, BeFileNameCR filePath, FileCache location, BeFileNameCP relativeDir, bool copyFile);
        BentleyStatus FixFileNameIfNeeded(FileCache location, BeFileNameCR relativeDir, Utf8String& fileName);

        static BentleyStatus RollbackFile(BeFileNameCR backupPath, BeFileNameCR originalPath);
        static BentleyStatus ReplaceFileWithRollback(BeFileNameCR fileToRollback, BeFileNameCR moveFromFile, BeFileNameCR moveToFile, bool copyFile);

        //! Remove file and do parent folder cleanup
        CacheStatus RemoveStoredFile(BeFileNameCR filePath, FileCache location, BeFileNameCR relativePath, BeFileNameCP newFilePath = nullptr) const;

        //! Remove directory relative paths if they don't contain any files.
        static void CleanupDirsNotContainingFiles(BeFileNameCR baseDir, BeFileName relativeDir);
        //! Check if directory or any sub-directories contain any files
        static bool DoesDirContainFiles(BeFileNameCR dir);

    public:
        FileStorage
            (
            ECDbAdapter& dbAdapter,
            ECSqlStatementCache& statementCache,
            CacheEnvironmentCR environment
            );

        BentleyStatus SetFileCacheLocation(FileInfo& info, FileCache location, BeFileNameCP externalRelativeDir = nullptr);
        BentleyStatus CacheFile(FileInfo& info, BeFileNameCR filePath, Utf8CP cacheTag, FileCache location, bool copyFile);

        static BentleyStatus DeleteFileCacheDirectories(CacheEnvironmentCR fullEnvironment);
        static CacheEnvironment CreateCacheEnvironment(BeFileNameCR cacheFilePath, CacheEnvironmentCR baseEnvironment);

        BeFileName GetAbsoluteFilePath(FileCache location, BeFileNameCR relativePath);

        CacheStatus RemoveStoredFile(FileInfoCR info);
        BentleyStatus RenameCachedFile(FileInfoR info, Utf8String newFileName);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
