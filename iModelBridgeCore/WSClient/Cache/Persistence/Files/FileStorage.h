/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Files/FileStorage.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
        BeFileName CreateNewRelativeCachedFilePath(BeFileNameCR currentFilePath, FileCache location);
        static BeFileName CreateNewFilePath(BeFileNameCR oldFilePath, Utf8String newFileName);

        static BentleyStatus CreateNewCachedFileFolderName(Utf8StringR folderNameOut);
        static BentleyStatus RollbackFile(BeFileNameCR backupPath, BeFileNameCR originalPath);
        static BentleyStatus ReplaceFileWithRollback(BeFileNameCR fileToRollback, BeFileNameCR moveFromFile, BeFileNameCR moveToFile, bool copyFile);
        static BeFileName GetFileCacheFolderPath(BeFileName rootDir, WStringCR cacheName);

    public:
        FileStorage(ECDbAdapter& dbAdapter, ECSqlStatementCache& statementCache, CacheEnvironmentCR environment);

        BentleyStatus SetFileCacheLocation(FileInfo& info, FileCache cacheLocation, BeFileNameCR externalRelativePath = BeFileName());
        BentleyStatus CacheFile(
            FileInfo& info,
            BeFileNameCR suppliedFilePath,
            Utf8CP cacheTag,
            FileCache cacheLocation,
            DateTimeCR cacheDateUtc,
            bool copyFile
            );

        static BentleyStatus DeleteFileCacheDirectories(CacheEnvironmentCR fullEnvironment);
        static CacheEnvironment CreateCacheEnvironment(BeFileNameCR cacheFilePath, CacheEnvironmentCR inputEnvironment);

        BeFileName GetAbsoluteFilePath(FileCache location, BeFileNameCR relativePath);
        BentleyStatus RemoveContainingFolder(BeFileNameCR filePath);
        BentleyStatus CleanupCachedFile(BeFileNameCR filePath);
        BentleyStatus RenameCachedFile(FileInfoR info, Utf8StringCR newFileName);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
