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

struct EXPORT_VTABLE_ATTRIBUTE CodeEvent : public Event::GenericEvent
{
private:
    Utf8String m_eventTopic;
    Utf8String m_fromEventSubscriptionId;
    Utf8String m_codeSpecId;
    Utf8String m_codeScope;
    bvector<Utf8String> m_values;
    int        m_state;
    int        m_briefcaseId;

    CodeEvent
        (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        Utf8String codeSpecId,
        Utf8String codeScope,
        bvector<Utf8String> values,
        int        state,
        int        briefcaseId
        );

public:
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct CodeEvent> Create
        (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        Utf8String codeSpecId,
        Utf8String codeScope,
        bvector<Utf8String> values,
        int        state,
        int        briefcaseId
        );

    Utf8String GetEventTopic() override {return m_eventTopic;}
    Utf8String GetFromEventSubscriptionId() override {return m_fromEventSubscriptionId;}
    Event::EventType GetEventType() override {return Event::EventType::CodeEvent;}
    Utf8String GetCodeSpecId() const {return m_codeSpecId;}
    Utf8String GetCodeScope() const {return m_codeScope;}
    bvector<Utf8String> GetValues() const {return m_values;}
    int GetState() const {return m_state;}
    int GetBriefcaseId() const {return m_briefcaseId;}
};

END_BENTLEY_IMODELHUB_NAMESPACE
