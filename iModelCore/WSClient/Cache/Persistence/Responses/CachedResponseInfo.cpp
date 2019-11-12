/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "CachedResponseInfo.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedResponseInfo::CachedResponseInfo() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedResponseInfo::CachedResponseInfo(ResponseKey key, JsonValueCR infoJson, ECClassId infoClassId) :
m_key(key),
m_infoJson(infoJson),
m_infoClassId(infoClassId)
    {
    BeAssert(m_key.IsValid());
    BeAssert(m_key.GetName() == m_infoJson[CLASS_CachedResponseInfo_PROPERTY_Name].asString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedResponseInfo::CachedResponseInfo(ResponseKey key, ECClassId infoClassId) :
m_key(key),
m_infoJson(),
m_infoClassId(infoClassId)
    {
    BeAssert(m_key.IsValid());
    m_infoJson[CLASS_CachedResponseInfo_PROPERTY_Name] = m_key.GetName();
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
ResponseKeyCR CachedResponseInfo::GetKey() const
    {
    return m_key;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheNodeKey CachedResponseInfo::GetInfoKey() const
    {
    return CacheNodeKey(m_infoClassId, ECDbHelper::ECInstanceIdFromJsonInstance(m_infoJson));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachedResponseInfo::IsCached() const
    {
    return !m_infoJson[ECJsonUtilities::json_id()].isNull();
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
