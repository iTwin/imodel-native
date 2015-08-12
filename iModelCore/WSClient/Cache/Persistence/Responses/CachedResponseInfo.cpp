/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Responses/CachedResponseInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CachedResponseInfo.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedResponseInfo::CachedResponseInfo() :
m_infoClassId(0)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedResponseInfo::CachedResponseInfo(ECInstanceKeyCR parent, ECInstanceKeyCR holder, JsonValueCR infoJson, ECClassId infoClassId) :
m_parent(parent),
m_holder(holder),
m_infoJson(infoJson),
m_infoClassId(infoClassId)
    {
    BeAssert(m_parent.IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedResponseInfo::CachedResponseInfo(ECInstanceKeyCR parent, ECInstanceKeyCR holder, Utf8StringCR name, ECClassId infoClassId) :
m_parent(parent),
m_holder(holder),
m_infoJson(),
m_infoClassId(infoClassId)
    {
    BeAssert(m_parent.IsValid());
    m_infoJson[CLASS_CachedResponseInfo_PROPERTY_Name] = name;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueCR CachedResponseInfo::GetJsonData() const
    {
    return m_infoJson;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueR CachedResponseInfo::GetJsonData()
    {
    return m_infoJson;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKeyCR CachedResponseInfo::GetParent() const
    {
    return m_parent;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKeyCR CachedResponseInfo::GetHolder() const
    {
    return m_holder;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey CachedResponseInfo::GetKey() const
    {
    return ECInstanceKey(m_infoClassId, ECDbHelper::ECInstanceIdFromJsonInstance(m_infoJson));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachedResponseInfo::IsCached() const
    {
    return !m_infoJson["$ECInstanceId"].isNull();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CachedResponseInfo::GetName() const
    {
    return m_infoJson[CLASS_CachedResponseInfo_PROPERTY_Name].asCString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime CachedResponseInfo::GetCacheDate() const
    {
    return BeJsonUtilities::DateTimeFromValue(m_infoJson[CLASS_CachedResponseInfo_PROPERTY_CacheDate]);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CachedResponseInfo::SetCacheDate(DateTimeCR utcDate)
    {
    m_infoJson[CLASS_CachedResponseInfo_PROPERTY_CacheDate] = ECDbHelper::UtcDateToString(utcDate);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime CachedResponseInfo::GetAccessDate() const
    {
    return BeJsonUtilities::DateTimeFromValue(m_infoJson[CLASS_CachedResponseInfo_PROPERTY_AccessDate]);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CachedResponseInfo::SetAccessDate(DateTimeCR utcDate)
    {
    m_infoJson[CLASS_CachedResponseInfo_PROPERTY_AccessDate] = ECDbHelper::UtcDateToString(utcDate);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CachedResponseInfo::GetCacheTag() const
    {
    JsonValueCR tagJson = m_infoJson[CLASS_CachedResponseInfo_PROPERTY_CacheTag];
    if (tagJson.isNull())
        {
        return nullptr;
        }
    return tagJson.asCString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CachedResponseInfo::SetCacheTag(Utf8StringCR tag)
    {
    m_infoJson[CLASS_CachedResponseInfo_PROPERTY_CacheTag] = tag;
    }
