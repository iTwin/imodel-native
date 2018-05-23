/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/GlobalEvents/HardiModelDeleteEvent.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
struct EXPORT_VTABLE_ATTRIBUTE HardiModelDeleteEvent : public GlobalEvent::GenericGlobalEvent
    {
protected:
    HardiModelDeleteEvent
    (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        Utf8String toEventSubscriptionId,
        Utf8String projectId,
        Utf8String iModelId
    );

public:
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct HardiModelDeleteEvent> Create
    (
        Utf8String eventTopic,
        Utf8String fromEventSubscriptionId,
        Utf8String toEventSubscriptionId,
        Utf8String projectId,
        Utf8String iModelId
    );

    GlobalEvent::GlobalEventType GetEventType() override { return GlobalEvent::GlobalEventType::HardiModelDeleteEvent; }
    };

END_BENTLEY_IMODELHUB_NAMESPACE
