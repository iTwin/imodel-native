/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerDeletedEvent.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/Client/DgnDbServerEvent.h>
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

struct DgnDbServerDeletedEvent : public DgnDbServerEvent::GenericEvent
    {
    private:
        Utf8String m_eventTopic;
        Utf8String m_fromEventSubscriptionId;
        Utf8String m_briefcaseId;
        Utf8String m_date;
        DgnDbServerEvent::DgnDbServerEventType m_deletedEventType;

        DgnDbServerDeletedEvent
            (
            Utf8String eventTopic,
            Utf8String fromEventSubscriptionId,
            Utf8String briefcaseId,
            Utf8String date,
            DgnDbServerEvent::DgnDbServerEventType deletedEventType
            );

    public:
        DGNDBSERVERCLIENT_EXPORT static std::shared_ptr<struct DgnDbServerDeletedEvent> Create
            (
            Utf8String eventTopic,
            Utf8String fromEventSubscriptionId,
            Utf8String briefcaseId,
            Utf8String date,
            DgnDbServerEvent::DgnDbServerEventType deletedEventType
            );
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetEventTopic();
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetFromEventSubscriptionId();
        DGNDBSERVERCLIENT_EXPORT virtual DgnDbServerEvent::DgnDbServerEventType GetEventType();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetDate();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetBriefcaseId();
        DGNDBSERVERCLIENT_EXPORT DgnDbServerEvent::DgnDbServerEventType GetDeletedEventType();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE