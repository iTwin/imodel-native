/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/Events/DgnDbServerLockEvent.h $
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

struct EXPORT_VTABLE_ATTRIBUTE DgnDbServerLockEvent : public DgnDbServerEvent::GenericEvent
{
private:
    Utf8String m_eventTopic;
    Utf8String m_fromEventSubscriptionId;
    bvector<Utf8String> m_objectIds;
    Utf8String m_lockType;
    Utf8String m_lockLevel;
    int        m_briefcaseId;
    Utf8String m_releasedWithRevision;

    DgnDbServerLockEvent
                        (
                        Utf8String eventTopic,
                        Utf8String fromSubscriptionId,
                        bvector<Utf8String> objectIds,
                        Utf8String lockType, 
                        Utf8String lockLevel, 
                        int        briefcaseId, 
                        Utf8String releasedWithRevision
                        );

public:
    DGNDBSERVERCLIENT_EXPORT static RefCountedPtr<struct DgnDbServerLockEvent> Create
                                                                                    (
                                                                                    Utf8String eventTopic,
                                                                                    Utf8String fromSubscriptionId,
                                                                                    bvector<Utf8String> objectIds,
                                                                                    Utf8String lockType, 
                                                                                    Utf8String lockLevel, 
                                                                                    int        briefcaseId, 
                                                                                    Utf8String releasedWithRevision
                                                                                    );
    Utf8String GetEventTopic() override {return m_eventTopic;}
    Utf8String GetFromEventSubscriptionId() override {return m_fromEventSubscriptionId;}
    DgnDbServerEvent::DgnDbServerEventType GetEventType() override {return DgnDbServerEvent::DgnDbServerEventType::LockEvent;}
    bvector<Utf8String> GetObjectIds() const {return m_objectIds;}
    Utf8String GetLockType() const {return m_lockType;}
    Utf8String GetLockLevel() const {return m_lockLevel;}
    int GetBriefcaseId() const {return m_briefcaseId;}
    Utf8String GetReleasedWithRevision() const {return m_releasedWithRevision;}
};

END_BENTLEY_DGNDBSERVER_NAMESPACE
