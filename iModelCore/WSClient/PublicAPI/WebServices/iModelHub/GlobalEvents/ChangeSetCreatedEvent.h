/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/GlobalEvents/GlobalEvent.h>
#include <WebServices/iModelHub/Common.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
//@bsiclass                                      Karolis.Uzkuraitis             04/2018
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ChangeSetCreatedEvent : public GlobalEvent::GenericGlobalEvent
    {
protected:
    Utf8String m_changeSetId;
    Utf8String m_changeSetIndex;
    int        m_briefcaseId;

    ChangeSetCreatedEvent
    (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        Utf8String toEventSubscriptionId,
        Utf8String projectId,
        Utf8String iModelId,
        Utf8String changeSetId,
        Utf8String changeSetIndex,
        int        briefcaseId,
        Utf8String lockUrl,
        Utf8String contextId,
        ContextType contextType
    );

public:
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct ChangeSetCreatedEvent> Create
    (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        Utf8String toEventSubscriptionId,
        Utf8String projectId,
        Utf8String iModelId,
        Utf8String changeSetId,
        Utf8String changeSetIndex,
        int        briefcaseId,
        Utf8String lockUrl,
        Utf8String contextId,
        ContextType contextType
    );

    GlobalEvent::GlobalEventType GetEventType() override { return GlobalEvent::GlobalEventType::ChangeSetCreatedEvent; }
    Utf8String GetChangeSetId() const { return m_changeSetId; }
    Utf8String GetChangeSetIndex() const { return m_changeSetIndex; }
    int GetBriefcaseId() const { return m_briefcaseId; }
    };

END_BENTLEY_IMODELHUB_NAMESPACE
