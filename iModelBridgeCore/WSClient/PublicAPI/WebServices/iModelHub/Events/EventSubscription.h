/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Events/EventSubscription.h $
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

typedef RefCountedPtr<struct EventSubscription> EventSubscriptionPtr;
DEFINE_TASK_TYPEDEFS(EventSubscriptionPtr, EventSubscription);

struct EventSubscription : RefCountedBase
{
private:
    Utf8String m_subscriptionId;
    EventTypeSet m_eventTypes;

    EventSubscription(Utf8String subscriptionId, EventTypeSet eventTypes) : m_subscriptionId(subscriptionId), m_eventTypes(eventTypes) {}
public:
    static EventSubscriptionPtr Create(Utf8String subscriptionId, EventTypeSet eventTypes) {return new EventSubscription(subscriptionId, eventTypes);}
    Utf8String GetSubscriptionId() const {return m_subscriptionId;}
    EventTypeSet GetEventTypes() const {return m_eventTypes;}
};

END_BENTLEY_IMODELHUB_NAMESPACE
