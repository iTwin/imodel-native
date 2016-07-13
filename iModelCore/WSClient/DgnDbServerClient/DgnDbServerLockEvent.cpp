/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerLockEvent.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerLockEvent.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             65/2016
//---------------------------------------------------------------------------------------
DgnDbServerLockEvent::DgnDbServerLockEvent
(
Utf8String eventTopic, 
Utf8String fromSubscriptionId, 
Utf8String objectId, 
Utf8String lockType, 
Utf8String lockLevel, 
Utf8String briefcaseId, 
Utf8String releasedWithRevision
)
    {
    m_eventTopic = eventTopic;
    m_fromEventSubscriptionId = fromSubscriptionId;
    m_objectId = objectId;
    m_lockType = lockType;
    m_lockLevel = lockLevel;
    m_briefcaseId = briefcaseId;
    m_releasedWithRevision = releasedWithRevision;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<struct DgnDbServerLockEvent> DgnDbServerLockEvent::Create
(
Utf8String eventTopic, 
Utf8String fromSubscriptionId, 
Utf8String objectId, 
Utf8String lockType, 
Utf8String lockLevel, 
Utf8String briefcaseId, 
Utf8String releasedWithRevision
)
    {
    return std::shared_ptr<struct DgnDbServerLockEvent>
        (new DgnDbServerLockEvent
               (
               eventTopic, 
               fromSubscriptionId, 
               objectId, 
               lockType, 
               lockLevel, 
               briefcaseId, 
               releasedWithRevision
               )
         );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerLockEvent::GetEventTopic()
    {
    return m_eventTopic;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerLockEvent::GetFromEventSubscriptionId()
    {
    return m_fromEventSubscriptionId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerLockEvent::GetObjectId()
    {
    return m_objectId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerLockEvent::GetLockType()
    {
    return m_lockType;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Caleb.Shafer						06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerLockEvent::GetLockLevel()
    {
    return m_lockLevel;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Caleb.Shafer						06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerLockEvent::GetBriefcaseId()
    {
    return m_briefcaseId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Caleb.Shafer						06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerLockEvent::GetReleasedWithRevision()
    {
    return m_releasedWithRevision;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEvent::DgnDbServerEventType DgnDbServerLockEvent::GetEventType()
    {
    /*const type_info& tp = typeid(this);
    return tp;*/
    return DgnDbServerEvent::DgnDbServerEventType::LockEvent;
    }