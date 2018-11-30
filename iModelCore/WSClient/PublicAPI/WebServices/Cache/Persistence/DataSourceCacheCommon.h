/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Persistence/DataSourceCacheCommon.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

enum class CacheRootPersistence
    {
    Full = 0,
    Temporary = 1,
    Default = Full
    };

// JSON specific properties
#define DataSourceCache_PROPERTY_LocalInstanceId        "id"
#define DataSourceCache_PROPERTY_ClassKey               "className"
#define DataSourceCache_PROPERTY_RemoteId               "$RemoteId"

END_BENTLEY_WEBSERVICES_NAMESPACE
