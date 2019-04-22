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

struct EXPORT_VTABLE_ATTRIBUTE VersionEvent : public Event::GenericEvent
    {
    private:
        Utf8String m_eventTopic;
        Utf8String m_fromEventSubscriptionId;
        Utf8String m_versionId;
        Utf8String m_versionName;
        Utf8String m_changeSetId;

        VersionEvent
        (
            Utf8String eventTopic,
            Utf8String fromSubscriptionId,
            Utf8String versionId,
            Utf8String versionName,
            Utf8String changeSetId
        );

    public:
        IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct VersionEvent> Create
        (
            Utf8String eventTopic,
            Utf8String fromSubscriptionId,
            Utf8String versionId,
            Utf8String versionName,
            Utf8String changeSetId
        );
        Utf8String GetEventTopic() override { return m_eventTopic; }
        Utf8String GetFromEventSubscriptionId() override { return m_fromEventSubscriptionId; }
        Event::EventType GetEventType() override { return Event::EventType::VersionEvent; }
        Utf8String GetVersionId() const { return m_versionId; }
        Utf8String GetVersionName() const { return m_versionName; }
        Utf8String GetChangeSetId() const { return m_changeSetId; }
    };

END_BENTLEY_IMODELHUB_NAMESPACE