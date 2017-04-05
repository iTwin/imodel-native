/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/Events/DgnDbServerDeletedEvent.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/Client/Events/DgnDbServerEvent.h>
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

struct EXPORT_VTABLE_ATTRIBUTE DgnDbServerDeletedEvent : public DgnDbServerEvent::GenericEvent
{
private:
    Utf8String m_eventTopic;
    Utf8String m_fromEventSubscriptionId;
    int        m_briefcaseId;
    DgnDbServerEvent::DgnDbServerEventType m_deletedEventType;

    DgnDbServerDeletedEvent
        (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        int        briefcaseId,
        DgnDbServerEvent::DgnDbServerEventType deletedEventType
        );

public:
    DGNDBSERVERCLIENT_EXPORT static RefCountedPtr<struct DgnDbServerDeletedEvent> Create
        (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        int        briefcaseId,
        DgnDbServerEvent::DgnDbServerEventType deletedEventType
        );

    Utf8String GetEventTopic() override {return m_eventTopic;}
    Utf8String GetFromEventSubscriptionId() override {return m_fromEventSubscriptionId;}
    DgnDbServerEvent::DgnDbServerEventType GetEventType() override {return m_deletedEventType;}
    int GetBriefcaseId() const {return m_briefcaseId;}
    DgnDbServerEvent::DgnDbServerEventType GetDeletedEventType() const {return m_deletedEventType;}
};

END_BENTLEY_DGNDBSERVER_NAMESPACE
