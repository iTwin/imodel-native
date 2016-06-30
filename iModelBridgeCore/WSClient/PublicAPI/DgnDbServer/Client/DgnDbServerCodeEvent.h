/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerCodeEvent.h $
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

struct DgnDbServerCodeEvent : public DgnDbServerEvent::GenericEvent
    {
    private:
        Utf8String m_eventTopic;
        Utf8String m_fromEventSubscriptionId;
        Utf8String m_codeAuthorityId;
        Utf8String m_codeNamespace;
        Utf8String m_value;
        Utf8String m_state;
        Utf8String m_briefcaseId;
        Utf8String m_usedWithRevision;
        Utf8String m_date;

        DgnDbServerCodeEvent
            (
            Utf8String eventTopic,
            Utf8String fromEventSubscriptionId,
            Utf8String codeAuthorityId,
            Utf8String codeNamespace,
            Utf8String value,
            Utf8String state,
            Utf8String briefcaseId,
            Utf8String usedWithRevision,
            Utf8String date
            );

    public:
        DGNDBSERVERCLIENT_EXPORT static std::shared_ptr<struct DgnDbServerCodeEvent> Create
            (
            Utf8String eventTopic,
            Utf8String fromEventSubscriptionId,
            Utf8String codeAuthorityId,
            Utf8String codeNamespace,
            Utf8String value,
            Utf8String state,
            Utf8String briefcaseId,
            Utf8String usedWithRevision,
            Utf8String date
            );
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetEventTopic();
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetFromEventSubscriptionId();
        DGNDBSERVERCLIENT_EXPORT virtual const type_info& GetEventType();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetDate();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetCodeAuthorityId();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetNamespace();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetValue();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetState();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetBriefcaseId();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetUsedWithRevision();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE