/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Instances/RelationshipInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ECDb/ECDb.h>

#include "../Changes/ChangeInfo.h"
#include "CachedInstanceKey.h"

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelationshipInfo : public ChangeInfo
    {
    public:
        ECRelationshipClassCP m_relClass;
        ECInstanceKey m_instanceKey;
        mutable ObjectId m_objectId;
        ECClassId m_infoClassId;

    public:
        RelationshipInfo();
        RelationshipInfo
            (
            JsonValueCR infoJson,
            ECRelationshipClassCP relationshipClass,
            ECInstanceId relationshipId,
            ECClassId infoClassId
            );

        ECInstanceKeyCR GetInstanceKey() const;
        void SetInstanceId(ECInstanceId instanceId);

        CachedInstanceKey GetCachedInstanceKey() const;
        CacheNodeKey GetInfoKey() const;

        ObjectIdCR GetObjectId() const;
        void SetRemoteId(Utf8StringCR remoteId);
    };

typedef RelationshipInfo& RelationshipInfoR;
typedef const RelationshipInfo& RelationshipInfoCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
