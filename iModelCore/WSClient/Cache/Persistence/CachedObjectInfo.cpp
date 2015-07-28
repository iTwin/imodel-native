/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/CachedObjectInfo.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Persistence/CachedObjectInfo.h>

#include "Instances/ObjectInfo.h"

using namespace std;

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CachedObjectInfo::CachedObjectInfo() :
CachedObjectInfo(std::make_shared<ObjectInfo>(ObjectInfo()))
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CachedObjectInfo::CachedObjectInfo(std::shared_ptr<ObjectInfo> info) :
m_info(info)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachedObjectInfo::IsInCache() const
    {
    return m_info->IsInCache();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachedObjectInfo::IsFullyCached() const
    {
    return m_info->IsFullyCached();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CachedObjectInfo::GetObjectCacheTag() const
    {
    return m_info->GetObjectCacheTag();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::ChangeStatus CachedObjectInfo::GetChangeStatus() const
    {
    return m_info->GetChangeStatus();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ChangeManager::SyncStatus CachedObjectInfo::GetSyncStatus() const
    {
    return m_info->GetSyncStatus();
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey CachedObjectInfo::GetCachedInstanceKey() const
    {
    return m_info->GetCachedInstanceKey();
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectIdCR CachedObjectInfo::GetObjectId() const
    {
    return m_info->GetObjectId();
    }
