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

enum class FileCache
    {
    Temporary,
    Persistent
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
