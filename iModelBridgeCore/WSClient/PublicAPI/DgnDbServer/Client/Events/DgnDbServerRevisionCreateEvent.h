/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/Events/DgnDbServerRevisionCreateEvent.h $
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

struct EXPORT_VTABLE_ATTRIBUTE DgnDbServerRevisionCreateEvent : public DgnDbServerEvent::GenericEvent
{
private:
    Utf8String m_eventTopic;
    Utf8String m_fromEventSubscriptionId;

    DgnDbServerRevisionCreateEvent(Utf8String eventTopic, Utf8String fromEventSubscriptionId);

public:
    DGNDBSERVERCLIENT_EXPORT static RefCountedPtr<struct DgnDbServerRevisionCreateEvent> Create (Utf8String eventTopic, Utf8String fromEventSubscriptionId);
    Utf8String GetEventTopic() {return m_eventTopic;}
    Utf8String GetFromEventSubscriptionId() {return m_fromEventSubscriptionId;}
    DgnDbServerEvent::DgnDbServerEventType GetEventType() {return DgnDbServerEvent::DgnDbServerEventType::RevisionCreateEvent;}
};

END_BENTLEY_DGNDBSERVER_NAMESPACE
