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

    // For accessing cached files through ExternalFileInfo
    WSCACHE_EXPORT static int GetPersistentRootFolderId();
    // For accessing cached files through ExternalFileInfo
    WSCACHE_EXPORT static int GetTemporaryRootFolderId();
    // For accessing cached files through ExternalFileInfo
    WSCACHE_EXPORT static int GetExternalRootFolderId();
    };

typedef const CacheEnvironment& CacheEnvironmentCR;
typedef CacheEnvironment& CacheEnvironmentR;

END_BENTLEY_WEBSERVICES_NAMESPACE
