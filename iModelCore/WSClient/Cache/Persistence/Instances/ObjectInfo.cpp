/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "ObjectInfo.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ObjectInfo::ObjectInfo() :
ChangeInfo(Json::nullValue),
m_instanceClass(nullptr)
    {}

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectInfo::ObjectInfo(JsonValueCR infoJson, ECClassCP instanceClass, ECClassId infoClassId) :
ChangeInfo(infoJson),
m_instanceClass(instanceClass),
m_infoClassId(infoClassId)
    {}

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedInstanceKey ObjectInfo::GetCachedInstanceKey() const
    {
    return CachedInstanceKey(GetInfoKey(), GetInstanceKey());
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ObjectInfo::GetInstanceKey() const
    {
    ECInstanceId ecInstanceId = ECDbHelper::ECInstanceIdFromJsonValue(m_infoJson[CLASS_CachedObjectInfo_PROPERTY_InstanceId]);
    return ECInstanceKey(m_instanceClass ? m_instanceClass->GetId() : ECClassId(), ecInstanceId);
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ObjectInfo::SetInstanceId(ECInstanceId instanceId)
    {
    m_infoJson[CLASS_CachedObjectInfo_PROPERTY_InstanceId] = ECDbHelper::StringFromECInstanceId(instanceId);
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedObjectInfoKey ObjectInfo::GetInfoKey() const
    {
    return CachedObjectInfoKey(m_infoClassId, ECDbHelper::ECInstanceIdFromJsonInstance(m_infoJson));
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ObjectInfo::SetObjectCacheTag(Utf8StringCR tag)
    {
    m_infoJson[CLASS_CachedObjectInfo_PROPERTY_CacheTag] = tag.empty() ? Json::nullValue : Json::Value(tag);
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime ObjectInfo::GetObjectCacheDate() const
    {
    return BeJsonUtilities::DateTimeFromValue(m_infoJson[CLASS_CachedFileInfo_PROPERTY_CacheDate]);
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ObjectInfo::SetObjectCacheDate(DateTimeCR utcDate)
    {
    m_infoJson[CLASS_CachedObjectInfo_PROPERTY_CacheDate] = ECDbHelper::UtcDateToString(utcDate);
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ObjectInfo::ClearObjectCacheDate()
    {
    m_infoJson[CLASS_CachedObjectInfo_PROPERTY_CacheDate] = Json::nullValue;
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ObjectInfo::SetObjectState(CachedInstanceState state)
    {
    m_infoJson[CLASS_CachedObjectInfo_PROPERTY_InstanceState] = static_cast<int>(state);
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ObjectInfo::SetRemoteId(Utf8StringCR remoteId)
    {
    m_infoJson[CLASS_CachedObjectInfo_PROPERTY_RemoteId] = remoteId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ObjectInfo::IsFullyCached() const
    {
    int intState = m_infoJson[CLASS_CachedObjectInfo_PROPERTY_InstanceState].asInt();
    CachedInstanceState state = static_cast<CachedInstanceState> (intState);
    return CachedInstanceState::Full == state;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ObjectInfo::GetObjectCacheTag() const
    {
    return m_infoJson[CLASS_CachedObjectInfo_PROPERTY_CacheTag].asString();
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectIdCR ObjectInfo::GetObjectId() const
    {
    if (m_objectIdMutable.IsEmpty() && nullptr != m_instanceClass)
        {
        m_objectIdMutable.schemaName = Utf8String(m_instanceClass->GetSchema().GetName());
        m_objectIdMutable.className = Utf8String(m_instanceClass->GetName());
        m_objectIdMutable.remoteId = m_infoJson[CLASS_CachedObjectInfo_PROPERTY_RemoteId].asString();
        }
    return m_objectIdMutable;
    }
