/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Files/FileCacheManager.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ValueIncrementor.h>
#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>

#include "FileInfoManager.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileCacheManager
    {
    private:
        CacheEnvironment m_environment;
        FileInfoManager* m_fileInfoManager;
        std::shared_ptr<ValueIncrementor> m_folderNameIncrementor;

    private:
        BeFileName CreateNewRelativeCachedFilePath(BeFileNameCR currentFilePath, bool isPersistent);
        BentleyStatus CreateNewCachedFileFolderName(Utf8StringR folderNameOut);

    public:
        FileCacheManager
            (
            ECDbAdapter& dbAdapter,
            ECSqlStatementCache& statementCache,
            CacheEnvironmentCR environment,
            FileInfoManager& fileInfoManager
            );

        BentleyStatus SetFileCacheLocation(ECInstanceKeyCR instance, FileCache cacheLocation);
        BentleyStatus CacheFile
            (
            ECInstanceKeyCR instance,
            BeFileNameCR suppliedFilePath,
            Utf8CP cacheTag,
            FileCache cacheLocation,
            DateTimeCR cacheDateUtc,
            bool copyFile
            );

        static BentleyStatus DeleteFileCacheDirectories(CacheEnvironmentCR fullEnvironment);
        static CacheEnvironment CreateCacheEnvironment(BeFileNameCR cacheFilePath, CacheEnvironmentCR inputEnvironment);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
