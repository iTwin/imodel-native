/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Util/ExtendedData.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ExtendedData.h>

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExtendedData::ExtendedData()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExtendedData::ExtendedData(ECInstanceKey instanceKey, ECInstanceKey extendedDataKey, std::shared_ptr<Json::Value> extendedData) :
m_instanceKey(instanceKey),
m_extendedDataKey(extendedDataKey),
m_extendedData(nullptr == extendedData ? std::make_shared<Json::Value>() : extendedData)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueCR ExtendedData::GetData() const
    {
    return *m_extendedData;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExtendedData::HasValue(Utf8StringCR key) const
    {
    return m_extendedData->isMember(key);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueCR ExtendedData::GetValue(Utf8StringCR key) const
    {
    return (*m_extendedData)[key];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedData::SetValue(Utf8StringCR key, JsonValueCR value)
    {
    (*m_extendedData)[key] = value;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedData::RemoveValue(Utf8StringCR key)
    {
    m_extendedData->removeMember(key);
    }
