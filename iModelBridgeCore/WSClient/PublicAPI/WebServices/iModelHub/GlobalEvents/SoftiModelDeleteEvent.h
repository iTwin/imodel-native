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
struct EXPORT_VTABLE_ATTRIBUTE SoftiModelDeleteEvent : public GlobalEvent::GenericGlobalEvent
    {
protected:
    SoftiModelDeleteEvent
    (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        Utf8String toEventSubscriptionId,
        Utf8String projectId,
        Utf8String iModelId,
        Utf8String lockUrl
    );

public:
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct SoftiModelDeleteEvent> Create
    (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        Utf8String toEventSubscriptionId,
        Utf8String projectId,
        Utf8String iModelId,
        Utf8String lockUrl
    );

    GlobalEvent::GlobalEventType GetEventType() override { return GlobalEvent::GlobalEventType::SoftiModelDeleteEvent; }
    };

END_BENTLEY_IMODELHUB_NAMESPACE
