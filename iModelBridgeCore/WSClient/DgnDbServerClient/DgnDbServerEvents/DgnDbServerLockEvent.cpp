/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEvents/DgnDbServerLockEvent.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/Events/DgnDbServerLockEvent.h>
#include "../DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             65/2016
//---------------------------------------------------------------------------------------
DgnDbServerLockEvent::DgnDbServerLockEvent
(
Utf8String eventTopic, 
Utf8String fromSubscriptionId, 
bvector<Utf8String> objectIds, 
Utf8String lockType, 
Utf8String lockLevel, 
int        briefcaseId, 
Utf8String releasedWithRevision
)
    {
    m_eventTopic = eventTopic;
    m_fromEventSubscriptionId = fromSubscriptionId;
    m_objectIds = objectIds;
    m_lockType = lockType;
    m_lockLevel = lockLevel;
    m_briefcaseId = briefcaseId;
    m_releasedWithRevision = releasedWithRevision;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct DgnDbServerLockEvent> DgnDbServerLockEvent::Create
(
Utf8String eventTopic, 
Utf8String fromSubscriptionId, 
bvector<Utf8String> objectIds, 
Utf8String lockType, 
Utf8String lockLevel, 
int        briefcaseId, 
Utf8String releasedWithRevision
)
    {
    return new DgnDbServerLockEvent
               (
               eventTopic, 
               fromSubscriptionId, 
               objectIds, 
               lockType, 
               lockLevel, 
               briefcaseId, 
               releasedWithRevision
               );
    }
