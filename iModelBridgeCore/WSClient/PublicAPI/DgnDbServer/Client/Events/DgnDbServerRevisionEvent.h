/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/Events/DgnDbServerRevisionEvent.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/Client/Events/DgnDbServerEvent.h>
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

struct DgnDbServerRevisionEvent : public DgnDbServerEvent::GenericEvent
    {
    private:
        Utf8String m_eventTopic;
        Utf8String m_fromEventSubscriptionId;
        Utf8String m_revisionId;
        Utf8String m_revisionIndex;
		Utf8String m_briefcaseId;

        DgnDbServerRevisionEvent
                                (
                                Utf8String eventTopic,
                                Utf8String fromEventSubscriptionId,
                                Utf8String revisionId, 
                                Utf8String revisionIndex,
								Utf8String briefcaseId
                                );

    public:
        DGNDBSERVERCLIENT_EXPORT static std::shared_ptr<struct DgnDbServerRevisionEvent> Create
                                                                                               (
                                                                                               Utf8String eventTopic,
                                                                                               Utf8String fromEventSubscriptionId,
                                                                                               Utf8String revisionId, 
                                                                                               Utf8String revisionIndex,
																							   Utf8String briefcaseId
                                                                                               );
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetEventTopic();
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetFromEventSubscriptionId();
        DGNDBSERVERCLIENT_EXPORT virtual DgnDbServerEvent::DgnDbServerEventType GetEventType();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetRevisionId();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetRevisionIndex();
		DGNDBSERVERCLIENT_EXPORT Utf8String GetBriefcaseId();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE