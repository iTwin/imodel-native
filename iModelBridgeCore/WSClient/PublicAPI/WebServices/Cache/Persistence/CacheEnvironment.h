/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Persistence/CacheEnvironment.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/BeFileName.h>

#include <WebServices/Cache/WebServicesCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

//! EC ExternalFileInfo root folder identifiers. Internal ECF defined
enum class ExternalFileInfoRootFolderId
    {
    Documents = 0,
    Temporary = 1,
    Caches = 2,
    LocalState = 3
    };

//! WSCache environment folders identifiers
//! Correct mapping to EC should be checked with CacheEnvironment::GetRootFolderId()
enum class FileCache
    {
    Persistent = static_cast<int>(ExternalFileInfoRootFolderId::LocalState),
    Temporary = static_cast<int>(ExternalFileInfoRootFolderId::Temporary),
    External = static_cast<int>(ExternalFileInfoRootFolderId::Documents)
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct CacheEnvironment
    {
    BeFileName persistentFileCacheDir;
    BeFileName temporaryFileCacheDir;
    BeFileName externalFileCacheDir;

    WSCACHE_EXPORT CacheEnvironment();
    WSCACHE_EXPORT CacheEnvironment(BeFileNameCR persistentDir, BeFileNameCR temporaryDir);

    //! Get environment path for given location
    WSCACHE_EXPORT BeFileNameCR GetPath(FileCache location);

    //! For accessing cached files through ExternalFileInfo. Example usages include ECDbECPlugin
    WSCACHE_EXPORT static int GetRootFolderId(FileCache location);
    };

typedef const CacheEnvironment& CacheEnvironmentCR;
typedef CacheEnvironment& CacheEnvironmentR;

END_BENTLEY_WEBSERVICES_NAMESPACE
