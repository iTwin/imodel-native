/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ECDb/ECDb.h>

#include "../Changes/ChangeInfo.h"
#include "../Core/CacheSchema.h"
#include "CachedInstanceKey.h"

USING_NAMESPACE_BENTLEY_SQLITE_EC;
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct CachedObjectInfoKey : public CacheNodeKey
    {
    CachedObjectInfoKey() : CacheNodeKey() {}
    CachedObjectInfoKey(ECClassId ecClassId, ECInstanceId const& ecInstanceId) :
        CacheNodeKey(ecClassId, ecInstanceId) {}
    };

typedef const CachedObjectInfoKey& CachedObjectInfoKeyCR;
typedef CachedObjectInfoKey& CachedObjectInfoKeyR;

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ObjectInfo : public ChangeInfo
    {
    private:
        mutable ObjectId m_objectIdMutable;

        ECClassCP m_instanceClass;
        ECClassId m_infoClassId;

    public:
        ObjectInfo();
        ObjectInfo(JsonValueCR infoJson, ECClassCP instanceClass, ECClassId infoClassId);

        bool IsFullyCached() const;

        Utf8String GetObjectCacheTag() const;
        void SetObjectCacheTag(Utf8StringCR tag);

        DateTime GetObjectCacheDate() const;
        void SetObjectCacheDate(DateTimeCR utcDate);
        void ClearObjectCacheDate();

        void SetObjectState(CachedInstanceState state);

        CachedInstanceKey GetCachedInstanceKey() const;
        CachedObjectInfoKey GetInfoKey() const;

        ECInstanceKey GetInstanceKey() const;
        void SetInstanceId(ECInstanceId instanceId);

        ObjectIdCR GetObjectId() const;
        void SetRemoteId(Utf8StringCR remoteId);
    };

typedef ObjectInfo& ObjectInfoR;
typedef const ObjectInfo& ObjectInfoCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
