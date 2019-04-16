/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "ChangeInfo.h"

#include <WebServices/Cache/Util/ECDbHelper.h>
#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ChangeInfo::ChangeInfo()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ChangeInfo::ChangeInfo(JsonValueCR infoJson) :
m_infoJson(infoJson)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool ChangeInfo::IsInCache() const
    {
    return !m_infoJson[ECJsonUtilities::json_id()].isNull();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::ChangeStatus ChangeInfo::GetChangeStatus() const
    {
    int statusInt = m_infoJson[CLASS_ChangeInfo_PROPERTY_ChangeStatus].asInt();
    return static_cast<IChangeManager::ChangeStatus>(statusInt);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::SyncStatus ChangeInfo::GetSyncStatus() const
    {
    int statusInt = m_infoJson[CLASS_ChangeInfo_PROPERTY_SyncStatus].asInt();
    return static_cast<IChangeManager::SyncStatus>(statusInt);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ChangeInfo::SetChangeStatus(IChangeManager::ChangeStatus status)
    {
    m_infoJson[CLASS_ChangeInfo_PROPERTY_ChangeStatus] = static_cast<int>(status);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ChangeInfo::SetSyncStatus(IChangeManager::SyncStatus status)
    {
    m_infoJson[CLASS_ChangeInfo_PROPERTY_SyncStatus] = static_cast<int>(status);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
uint64_t ChangeInfo::GetChangeNumber() const
    {
    return BeJsonUtilities::Int64FromValue(m_infoJson[CLASS_ChangeInfo_PROPERTY_ChangeNumber], 0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ChangeInfo::SetChangeNumber(uint64_t number)
    {
    m_infoJson[CLASS_ChangeInfo_PROPERTY_ChangeNumber] = BeJsonUtilities::StringValueFromInt64(number);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
uint64_t ChangeInfo::GetRevision() const
    {
    return BeJsonUtilities::Int64FromValue(m_infoJson[CLASS_ChangeInfo_PROPERTY_Revision], 0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ChangeInfo::IncrementRevision()
    {
    auto revision = ChangeInfo::GetRevision();
    m_infoJson[CLASS_ChangeInfo_PROPERTY_Revision] = BeJsonUtilities::StringValueFromInt64(++revision);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool ChangeInfo::IsLocal() const
    {
    return m_infoJson[CLASS_ChangeInfo_PROPERTY_IsLocal].asBool();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ChangeInfo::SetIsLocal(bool value)
    {
    m_infoJson[CLASS_ChangeInfo_PROPERTY_IsLocal] = value;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ChangeInfo::ClearChangeInfo()
    {
    m_infoJson[CLASS_ChangeInfo_PROPERTY_ChangeStatus] = Json::nullValue;
    m_infoJson[CLASS_ChangeInfo_PROPERTY_SyncStatus] = Json::nullValue;
    m_infoJson[CLASS_ChangeInfo_PROPERTY_ChangeNumber] = Json::nullValue;
    m_infoJson[CLASS_ChangeInfo_PROPERTY_Revision] = Json::nullValue;
    m_infoJson[CLASS_ChangeInfo_PROPERTY_IsLocal] = Json::nullValue;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
JsonValueCR ChangeInfo::GetJsonInfo() const
    {
    return m_infoJson;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
JsonValueR ChangeInfo::GetJsonInfo()
    {
    return m_infoJson;
    }
