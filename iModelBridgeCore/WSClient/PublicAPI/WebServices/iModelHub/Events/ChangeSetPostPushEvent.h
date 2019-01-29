/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Events/ChangeSetPostPushEvent.h $
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

struct EXPORT_VTABLE_ATTRIBUTE ChangeSetPostPushEvent : public Event::GenericEvent
{
private:
    Utf8String m_eventTopic;
    Utf8String m_fromEventSubscriptionId;
    Utf8String m_changeSetId;
    Utf8String m_changeSetIndex;
    int        m_briefcaseId;

    ChangeSetPostPushEvent
                            (
                            Utf8String eventTopic,
                            Utf8String fromEventSubscriptionId,
                            Utf8String changeSetId,
                            Utf8String changeSetIndex,
                            int        briefcaseId
                            );

public:
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct ChangeSetPostPushEvent> Create
                                                                                        (
                                                                                        Utf8String eventTopic,
                                                                                        Utf8String fromEventSubscriptionId,
                                                                                        Utf8String changeSetId,
                                                                                        Utf8String changeSetIndex,
                                                                                        int        briefcaseId
                                                                                        );
    Utf8String GetEventTopic() override {return m_eventTopic;}
    Utf8String GetFromEventSubscriptionId() override {return m_fromEventSubscriptionId;}
    Event::EventType GetEventType() override {return Event::EventType::ChangeSetPostPushEvent;}
    Utf8String GetChangeSetId() const {return m_changeSetId;}
    Utf8String GetChangeSetIndex() const {return m_changeSetIndex;}
    int        GetBriefcaseId() const {return m_briefcaseId;}
};

END_BENTLEY_IMODELHUB_NAMESPACE
