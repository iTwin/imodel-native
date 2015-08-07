/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Persistence/CachedObjectInfo.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeJsonCpp/BeJsonUtilities.h>
#include <ECDb/ECDb.h>
#include <WebServices/Cache/WebServicesCache.h>

#include "ChangeManager.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    00/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct CachedObjectInfo
    {
    protected:
        std::shared_ptr<struct ObjectInfo> m_info;

    public:
        WSCACHE_EXPORT CachedObjectInfo();
        WSCACHE_EXPORT CachedObjectInfo(std::shared_ptr<struct ObjectInfo> info);

        WSCACHE_EXPORT bool IsInCache() const;
        WSCACHE_EXPORT bool IsFullyCached() const;

        WSCACHE_EXPORT Utf8String GetObjectCacheTag() const;

        WSCACHE_EXPORT IChangeManager::ChangeStatus  GetChangeStatus() const;
        WSCACHE_EXPORT ChangeManager::SyncStatus    GetSyncStatus() const;

        WSCACHE_EXPORT ECInstanceKey GetCachedInstanceKey() const;
        WSCACHE_EXPORT ObjectIdCR GetObjectId() const;
    };

typedef CachedObjectInfo& CachedObjectInfoR;
typedef const CachedObjectInfo& CachedObjectInfoCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
