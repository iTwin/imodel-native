/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Instances/RelationshipInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "RelationshipInfo.h"

#include <WebServices/Cache/Util/ECDbHelper.h>
#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
RelationshipInfo::RelationshipInfo() :
m_relationshipClass(nullptr),
m_infoClassId(0)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
RelationshipInfo::RelationshipInfo(JsonValueCR infoJson, ECRelationshipClassCP relationshipClass, ECInstanceId relationshipId, ECClassId infoClassId) :
ChangeInfo(infoJson),
m_relationshipClass(relationshipClass),
m_relationshipKey(relationshipClass->GetId(), relationshipId),
m_infoClassId(infoClassId)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKeyCR RelationshipInfo::GetRelationshipKey() const
    {
    return m_relationshipKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void RelationshipInfo::SetRelationshipInstanceId(ECInstanceId instanceId)
    {
    m_relationshipKey = ECInstanceKey(m_relationshipKey.GetECClassId(), instanceId);
    m_infoJson[CLASS_CachedRelationshipInfo_PROPERTY_RelInstanceId] = ECDbHelper::StringFromECInstanceId(instanceId);
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey RelationshipInfo::GetInfoKey() const
    {
    return ECInstanceKey(m_infoClassId, ECDbHelper::ECInstanceIdFromJsonInstance(m_infoJson));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ObjectIdCR RelationshipInfo::GetRelationshipId() const
    {
    if (m_relationshipId.IsEmpty() && nullptr != m_relationshipClass)
        {
        m_relationshipId.schemaName = Utf8String(m_relationshipClass->GetSchema().GetName());
        m_relationshipId.className = Utf8String(m_relationshipClass->GetName());
        m_relationshipId.remoteId = m_infoJson[CLASS_CachedRelationshipInfo_PROPERTY_RemoteId].asString();
        }
    return m_relationshipId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String RelationshipInfo::GetRemoteId() const
    {
    return m_infoJson[CLASS_CachedRelationshipInfo_PROPERTY_RemoteId].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void RelationshipInfo::SetRemoteId(Utf8StringCR remoteId)
    {
    m_infoJson[CLASS_CachedRelationshipInfo_PROPERTY_RemoteId] = remoteId;
    }
