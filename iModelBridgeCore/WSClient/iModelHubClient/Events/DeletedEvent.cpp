/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Events/DeletedEvent.h>
#include "../Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             07/2016
//---------------------------------------------------------------------------------------
DeletedEvent::DeletedEvent
(
Utf8String eventTopic,
Utf8String fromEventSubscriptionId,
int        briefcaseId,
Event::EventType deletedEventType
)
    {
    m_eventTopic = eventTopic;
    m_fromEventSubscriptionId = fromEventSubscriptionId;
    m_briefcaseId = briefcaseId;
    m_deletedEventType = deletedEventType;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             07/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct DeletedEvent> DeletedEvent::Create
(
Utf8String eventTopic,
Utf8String fromEventSubscriptionId,
int        briefcaseId,
Event::EventType deletedEventType
)
    {
    Utf8String eventName = Event::Helper::GetEventNameFromEventType(deletedEventType);
    if (
        0 == eventName.CompareToI("AllLocksDeletedEvent") ||
        0 == eventName.CompareToI("AllCodesDeletedEvent")
       )
        return new DeletedEvent
            (
            eventTopic,
            fromEventSubscriptionId,
            briefcaseId,
            deletedEventType
            );       
    return nullptr;    
    }
