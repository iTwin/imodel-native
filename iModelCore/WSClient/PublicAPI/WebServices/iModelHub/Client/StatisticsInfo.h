/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/StatisticsInfo.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "UserInfo.h"

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
typedef RefCountedPtr<struct StatisticsInfo> StatisticsInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(StatisticsInfo);
DEFINE_TASK_TYPEDEFS(StatisticsInfoPtr, StatisticsInfo);
DEFINE_TASK_TYPEDEFS(bvector<StatisticsInfoPtr>, MultipleStatisticsInfo);

//=======================================================================================
//! Available statistics properties
//@bsiclass                                      Benas.Kikutis             12/2017
//=======================================================================================
enum StatisticsProperties ENUM_UNDERLYING_TYPE(uint32_t)
    {
    All = 0,
    BriefcaseCount = 1 << 0,
    OwnedLocksCount = 1 << 1,
    PushedChangeSetsCount = 1 << 2,
    LastChangeSetPushDate = 1 << 3
    // After updating please also update StatisticsManager::AddHasStatisticsSelect
    };

//=======================================================================================
//! Information about statistics.
//@bsiclass                                      Benas.Kikutis             11/2017
//=======================================================================================
struct StatisticsInfo : RefCountedBase
{
private:
    friend struct StatisticsManager;
    
    int32_t m_briefcaseCount;
    int32_t m_locksCount;
    int32_t m_changeSetsCount;
    DateTime m_lastChangeSetPushDate;
    UserInfoPtr m_userInfo;

    StatisticsInfo(int32_t briefcaseCount, int32_t locksCount, int32_t changeSetsCount, DateTime lastChangeSetPushDate, UserInfoPtr userInfo)
        : m_briefcaseCount(briefcaseCount), m_locksCount(locksCount), m_changeSetsCount(changeSetsCount), m_lastChangeSetPushDate(lastChangeSetPushDate), m_userInfo(userInfo) {}

    static StatisticsInfoPtr ParseFromRelated(WebServices::WSObjectsReader::Instance instance);
public:
    int32_t GetBriefcaseCount() const { return m_briefcaseCount; }
    int32_t GetLocksCount() const { return m_locksCount; }
    int32_t GetChangeSetsCount() const { return m_changeSetsCount; }
    DateTime GetLastChangeSetPushDate() const { return m_lastChangeSetPushDate; }
    UserInfoPtr GetUserInfo() const { return m_userInfo; }
};

END_BENTLEY_IMODELHUB_NAMESPACE
