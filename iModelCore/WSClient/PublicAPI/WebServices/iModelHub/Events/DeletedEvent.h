/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Events/Event.h>
#include <WebServices/iModelHub/Common.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

struct EXPORT_VTABLE_ATTRIBUTE DeletedEvent : public Event::GenericEvent
{
private:
    Utf8String m_eventTopic;
    Utf8String m_fromEventSubscriptionId;
    int        m_briefcaseId;
    Event::EventType m_deletedEventType;

    DeletedEvent
        (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        int        briefcaseId,
        Event::EventType deletedEventType
        );

public:
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct DeletedEvent> Create
        (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        int        briefcaseId,
        Event::EventType deletedEventType
        );

    Utf8String GetEventTopic() override {return m_eventTopic;}
    Utf8String GetFromEventSubscriptionId() override {return m_fromEventSubscriptionId;}
    Event::EventType GetEventType() override {return m_deletedEventType;}
    int GetBriefcaseId() const {return m_briefcaseId;}
    Event::EventType GetDeletedEventType() const {return m_deletedEventType;}
};

END_BENTLEY_IMODELHUB_NAMESPACE
