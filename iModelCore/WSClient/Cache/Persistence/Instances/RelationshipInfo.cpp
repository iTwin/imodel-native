/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Instances/RelationshipInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "RelationshipInfo.h"

#include <WebServices/Cache/Util/ECDbHelper.h>
#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
RelationshipInfo::RelationshipInfo() : m_relClass(nullptr) {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
RelationshipInfo::RelationshipInfo
(
JsonValueCR infoJson,
ECRelationshipClassCP relationshipClass,
ECInstanceId relationshipId,
ECClassId infoClassId
) :
ChangeInfo(infoJson),
m_relClass(relationshipClass),
m_instanceKey(relationshipClass->GetId(), relationshipId),
m_infoClassId(infoClassId)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKeyCR RelationshipInfo::GetInstanceKey() const
    {
    return m_instanceKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void RelationshipInfo::SetInstanceId(ECInstanceId instanceId)
    {
    m_instanceKey = ECInstanceKey(m_instanceKey.GetClassId(), instanceId);
    m_infoJson[CLASS_CachedRelationshipInfo_PROPERTY_InstanceId] = ECDbHelper::StringFromECInstanceId(instanceId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
CachedInstanceKey RelationshipInfo::GetCachedInstanceKey() const
    {
    return CachedInstanceKey(GetInfoKey(), GetInstanceKey());
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheNodeKey RelationshipInfo::GetInfoKey() const
    {
    return CacheNodeKey(m_infoClassId, ECDbHelper::ECInstanceIdFromJsonInstance(m_infoJson));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ObjectIdCR RelationshipInfo::GetObjectId() const
    {
    if (m_objectId.IsEmpty() && nullptr != m_relClass)
        {
        m_objectId.schemaName = Utf8String(m_relClass->GetSchema().GetName());
        m_objectId.className = Utf8String(m_relClass->GetName());
        m_objectId.remoteId = m_infoJson[CLASS_CachedRelationshipInfo_PROPERTY_RemoteId].asString();
        }
    return m_objectId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void RelationshipInfo::SetRemoteId(Utf8StringCR remoteId)
    {
    m_infoJson[CLASS_CachedRelationshipInfo_PROPERTY_RemoteId] = remoteId;
    }
