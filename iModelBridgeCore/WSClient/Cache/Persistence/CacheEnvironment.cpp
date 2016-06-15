/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/CacheEnvironment.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Persistence/CacheEnvironment.h>

#include "Files/FileInfo.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheEnvironment::CacheEnvironment()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheEnvironment::CacheEnvironment(BeFileNameCR persistentDir, BeFileNameCR temporaryDir) :
persistentFileCacheDir(persistentDir),
temporaryFileCacheDir(temporaryDir)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR CacheEnvironment::GetPath(FileCache location)
    {
    if (FileCache::External == location)
        return externalFileCacheDir;
    if (FileCache::Persistent == location)
        return persistentFileCacheDir;
    if (FileCache::Temporary == location)
        return temporaryFileCacheDir;

    BeAssert(false);
    static BeFileName empty;
    return empty;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int CacheEnvironment::GetRootFolderId(FileCache location)
    {
    if (FileCache::External == location)
        return static_cast<int>(FileCache::External);
    if (FileCache::Persistent == location)
        return static_cast<int>(FileCache::Persistent);
    if (FileCache::Temporary == location)
        return static_cast<int>(FileCache::Temporary);

    BeAssert(false);
    return -1;
    }
