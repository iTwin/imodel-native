/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/Events/DgnDbServerRevisionEvent.h $
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

struct EXPORT_VTABLE_ATTRIBUTE DgnDbServerRevisionEvent : public DgnDbServerEvent::GenericEvent
{
private:
    Utf8String m_eventTopic;
    Utf8String m_fromEventSubscriptionId;
    Utf8String m_revisionId;
    Utf8String m_revisionIndex;
    int        m_briefcaseId;

    DgnDbServerRevisionEvent
                            (
                            Utf8String eventTopic,
                            Utf8String fromEventSubscriptionId,
                            Utf8String revisionId, 
                            Utf8String revisionIndex,
                            int        briefcaseId
                            );

public:
    DGNDBSERVERCLIENT_EXPORT static RefCountedPtr<struct DgnDbServerRevisionEvent> Create
                                                                                        (
                                                                                        Utf8String eventTopic,
                                                                                        Utf8String fromEventSubscriptionId,
                                                                                        Utf8String revisionId, 
                                                                                        Utf8String revisionIndex,
                                                                                        int        briefcaseId
                                                                                        );
    Utf8String GetEventTopic() override {return m_eventTopic;}
    Utf8String GetFromEventSubscriptionId() override {return m_fromEventSubscriptionId;}
    DgnDbServerEvent::DgnDbServerEventType GetEventType() override {return DgnDbServerEvent::DgnDbServerEventType::RevisionEvent;}
    Utf8String GetRevisionId() const {return m_revisionId;}
    Utf8String GetRevisionIndex() const {return m_revisionIndex;}
    int        GetBriefcaseId() const {return m_briefcaseId;}
};

END_BENTLEY_DGNDBSERVER_NAMESPACE
