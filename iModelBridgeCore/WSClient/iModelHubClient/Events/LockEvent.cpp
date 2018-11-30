/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Events/LockEvent.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Events/LockEvent.h>
#include "../Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             65/2016
//---------------------------------------------------------------------------------------
LockEvent::LockEvent
(
Utf8String eventTopic, 
Utf8String fromSubscriptionId, 
bvector<Utf8String> objectIds, 
Utf8String lockType, 
Utf8String lockLevel, 
int        briefcaseId, 
Utf8String releasedWithChangeSet
)
    {
    m_eventTopic = eventTopic;
    m_fromEventSubscriptionId = fromSubscriptionId;
    m_objectIds = objectIds;
    m_lockType = lockType;
    m_lockLevel = lockLevel;
    m_briefcaseId = briefcaseId;
    m_releasedWithChangeSet = releasedWithChangeSet;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct LockEvent> LockEvent::Create
(
Utf8String eventTopic, 
Utf8String fromSubscriptionId, 
bvector<Utf8String> objectIds, 
Utf8String lockType, 
Utf8String lockLevel, 
int        briefcaseId, 
Utf8String releasedWithChangeSet
)
    {
    return new LockEvent
               (
               eventTopic, 
               fromSubscriptionId, 
               objectIds, 
               lockType, 
               lockLevel, 
               briefcaseId, 
               releasedWithChangeSet
               );
    }
