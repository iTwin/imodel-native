/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/GlobalEvents/GlobalEvent.h>
#include <WebServices/iModelHub/Common.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
//@bsiclass                                      Karolis.Uzkuraitis             04/2018
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE NamedVersionCreatedEvent : GlobalEvent::GenericGlobalEvent
    {
protected:
    Utf8String m_versionId;
    Utf8String m_versionName;
    Utf8String m_changeSetId;

    NamedVersionCreatedEvent
    (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        Utf8String toEventSubscriptionId,
        Utf8String projectId,
        Utf8String iModelId,
        Utf8String versionId,
        Utf8String versionName,
        Utf8String changeSetId,
        Utf8String lockUrl,
        Utf8String contextId,
        ContextType contextType
    );

public:
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct NamedVersionCreatedEvent> Create
    (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        Utf8String toEventSubscriptionId,
        Utf8String projectId,
        Utf8String iModelId,
        Utf8String versionId,
        Utf8String versionName,
        Utf8String changeSetId,
        Utf8String lockUrl,
        Utf8String contextId,
        ContextType contextType
    );

    GlobalEvent::GlobalEventType GetEventType() override { return GlobalEvent::GlobalEventType::NamedVersionCreatedEvent; }
    Utf8String GetVersionId() const { return m_versionId; }
    Utf8String GetVersionName() const { return m_versionName; }
    Utf8String GetChangeSetId() const { return m_changeSetId; }
    };

END_BENTLEY_IMODELHUB_NAMESPACE
