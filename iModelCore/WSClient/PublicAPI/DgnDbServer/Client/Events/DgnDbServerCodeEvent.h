/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/Events/DgnDbServerCodeEvent.h $
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

struct EXPORT_VTABLE_ATTRIBUTE DgnDbServerCodeEvent : public DgnDbServerEvent::GenericEvent
    {
    private:
        Utf8String m_eventTopic;
        Utf8String m_fromEventSubscriptionId;
        Utf8String m_codeSpecId;
        Utf8String m_codeScope;
        bvector<Utf8String> m_values;
        int        m_state;
        int        m_briefcaseId;

        DgnDbServerCodeEvent
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
        DGNDBSERVERCLIENT_EXPORT static std::shared_ptr<struct DgnDbServerCodeEvent> Create
            (
            Utf8String eventTopic,
            Utf8String fromEventSubscriptionId,
            Utf8String codeSpecId,
            Utf8String codeScope,
            bvector<Utf8String> values,
            int        state,
            int        briefcaseId
            );
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetEventTopic() { return m_eventTopic; }
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetFromEventSubscriptionId() { return m_fromEventSubscriptionId; }
        DGNDBSERVERCLIENT_EXPORT virtual DgnDbServerEvent::DgnDbServerEventType GetEventType() { return DgnDbServerEvent::DgnDbServerEventType::CodeEvent; }
        DGNDBSERVERCLIENT_EXPORT Utf8String GetCodeSpecId() { return m_codeSpecId; }
        DGNDBSERVERCLIENT_EXPORT Utf8String GetCodeScope() { return m_codeScope;  }
        DGNDBSERVERCLIENT_EXPORT bvector<Utf8String> GetValues() { return m_values; }
        DGNDBSERVERCLIENT_EXPORT int        GetState() { return m_state; }
        DGNDBSERVERCLIENT_EXPORT int        GetBriefcaseId() { return m_briefcaseId; }
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE
