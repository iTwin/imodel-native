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
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int CacheEnvironment::GetPersistentRootFolderId()
    {
    return static_cast<int>(FileCache::Persistent);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int CacheEnvironment::GetTemporaryRootFolderId()
    {
    return static_cast<int>(FileCache::Temporary);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int CacheEnvironment::GetExternalRootFolderId()
    {
    return static_cast<int>(FileCache::External);
    }