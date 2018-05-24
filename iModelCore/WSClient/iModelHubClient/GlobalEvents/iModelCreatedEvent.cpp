/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/GlobalEvents/iModelCreatedEvent.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/GlobalEvents/iModelCreatedEvent.h>
#include "../Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             04/2018
//---------------------------------------------------------------------------------------
iModelCreatedEvent::iModelCreatedEvent
(
    const Utf8String eventTopic,
    const Utf8String fromEventSubscriptionId,
    const Utf8String toEventSubscriptionId,
    const Utf8String projectId,
    const Utf8String iModelId
) : GenericGlobalEvent(eventTopic, fromEventSubscriptionId, toEventSubscriptionId, projectId, iModelId)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             04/2018
//---------------------------------------------------------------------------------------
RefCountedPtr<struct iModelCreatedEvent> iModelCreatedEvent::Create
(
    const Utf8String eventTopic,
    const Utf8String fromEventSubscriptionId,
    const Utf8String toEventSubscriptionId,
    const Utf8String projectId,
    const Utf8String iModelId
)
    {
    return new iModelCreatedEvent
    (
        eventTopic,
        fromEventSubscriptionId,
        toEventSubscriptionId,
        projectId,
        iModelId
    );
    }
