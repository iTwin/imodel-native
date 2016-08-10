/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/Events/DgnDbServerLockEvent.h $
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

struct DgnDbServerLockEvent : public DgnDbServerEvent::GenericEvent
    {
    private:
        Utf8String m_eventTopic;
        Utf8String m_fromEventSubscriptionId;
        bvector<Utf8String> m_objectIds;
        Utf8String m_lockType;
        Utf8String m_lockLevel;
        Utf8String m_briefcaseId;
        Utf8String m_releasedWithRevision;

        DgnDbServerLockEvent
                            (
                            Utf8String eventTopic,
                            Utf8String fromSubscriptionId,
                            bvector<Utf8String> objectIds,
                            Utf8String lockType, 
                            Utf8String lockLevel, 
                            Utf8String briefcaseId, 
                            Utf8String releasedWithRevision
                            );

    public:
        DGNDBSERVERCLIENT_EXPORT static std::shared_ptr<struct DgnDbServerLockEvent> Create
                                                                                           (
                                                                                           Utf8String eventTopic,
                                                                                           Utf8String fromSubscriptionId,
                                                                                           bvector<Utf8String> objectIds,
                                                                                           Utf8String lockType, 
                                                                                           Utf8String lockLevel, 
                                                                                           Utf8String briefcaseId, 
                                                                                           Utf8String releasedWithRevision
                                                                                           );
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetEventTopic();
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetFromEventSubscriptionId();
        DGNDBSERVERCLIENT_EXPORT virtual DgnDbServerEvent::DgnDbServerEventType GetEventType();
        DGNDBSERVERCLIENT_EXPORT bvector<Utf8String> GetObjectIds();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetLockType();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetLockLevel();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetBriefcaseId();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetReleasedWithRevision();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE