/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Persistence/DataSourceCacheCommon.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

//! Internal ECF defined
enum class ExternalFileInfoRootFolderId
    {
    Documents = 0,
    Temporary = 1,
    Caches = 2,
    LocalState = 3
    };

//! WSCache defined
enum class FileCache
    {
    Persistent = static_cast<int>(ExternalFileInfoRootFolderId::LocalState),
    Temporary = static_cast<int>(ExternalFileInfoRootFolderId::Temporary),
    External = static_cast<int>(ExternalFileInfoRootFolderId::Documents)
    };

enum class CacheRootPersistence
    {
    Full = 0,
    Temporary = 1,
    Default = Full
    };

// TODO: remove
// JSON specific properties
#define DataSourceCache_PROPERTY_LocalInstanceId        "$ECInstanceId"
#define DataSourceCache_PROPERTY_ClassKey               "$ECClassKey"
#define DataSourceCache_PROPERTY_RemoteId               "$RemoteId"

#define DataSourceCache_PROPERTY_DisplayInfo            "$DisplayInfo"
#define DataSourceCache_PROPERTY_DisplayData            "$DisplayData"
#define DataSourceCache_PROPERTY_RawData                "$RawData"

END_BENTLEY_WEBSERVICES_NAMESPACE
