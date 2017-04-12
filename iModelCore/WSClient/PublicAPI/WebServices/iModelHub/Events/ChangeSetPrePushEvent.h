/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Events/ChangeSetPrePushEvent.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Events/Event.h>
#include <WebServices/iModelHub/Common.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

struct EXPORT_VTABLE_ATTRIBUTE ChangeSetPrePushEvent : public Event::GenericEvent
{
private:
    Utf8String m_eventTopic;
    Utf8String m_fromEventSubscriptionId;

    ChangeSetPrePushEvent(Utf8String eventTopic, Utf8String fromEventSubscriptionId);

public:
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct ChangeSetPrePushEvent> Create (Utf8String eventTopic, Utf8String fromEventSubscriptionId);
    Utf8String GetEventTopic() {return m_eventTopic;}
    Utf8String GetFromEventSubscriptionId() {return m_fromEventSubscriptionId;}
    Event::EventType GetEventType() {return Event::EventType::ChangeSetPrePushEvent;}
};

END_BENTLEY_IMODELHUB_NAMESPACE
