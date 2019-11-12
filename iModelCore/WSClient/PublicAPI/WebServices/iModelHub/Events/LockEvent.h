/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Events/Event.h>
#include <WebServices/iModelHub/Common.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

struct EXPORT_VTABLE_ATTRIBUTE LockEvent : public Event::GenericEvent
{
private:
    Utf8String m_eventTopic;
    Utf8String m_fromEventSubscriptionId;
    bvector<Utf8String> m_objectIds;
    Utf8String m_lockType;
    Utf8String m_lockLevel;
    int        m_briefcaseId;
    Utf8String m_releasedWithChangeSet;

    LockEvent
        (
        Utf8String eventTopic,
        Utf8String fromSubscriptionId,
        bvector<Utf8String> objectIds,
        Utf8String lockType, 
        Utf8String lockLevel, 
        int        briefcaseId, 
        Utf8String releasedWithChangeSet
        );

public:
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct LockEvent> Create
        (
        Utf8String eventTopic,
        Utf8String fromSubscriptionId,
        bvector<Utf8String> objectIds,
        Utf8String lockType, 
        Utf8String lockLevel, 
        int        briefcaseId, 
        Utf8String releasedWithChangeSet
        );
    Utf8String GetEventTopic() override {return m_eventTopic;}
    Utf8String GetFromEventSubscriptionId() override {return m_fromEventSubscriptionId;}
    Event::EventType GetEventType() override {return Event::EventType::LockEvent;}
    bvector<Utf8String> GetObjectIds() const {return m_objectIds;}
    Utf8String GetLockType() const {return m_lockType;}
    Utf8String GetLockLevel() const {return m_lockLevel;}
    int GetBriefcaseId() const {return m_briefcaseId;}
    Utf8String GetReleasedWithChangeSet() const {return m_releasedWithChangeSet;}
};

END_BENTLEY_IMODELHUB_NAMESPACE
