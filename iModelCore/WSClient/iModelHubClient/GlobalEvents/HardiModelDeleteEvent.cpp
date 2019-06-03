/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/GlobalEvents/HardiModelDeleteEvent.h>
#include "../Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             04/2018
//---------------------------------------------------------------------------------------
HardiModelDeleteEvent::HardiModelDeleteEvent
(
    const Utf8String eventTopic,
    const Utf8String fromEventSubscriptionId,
    const Utf8String toEventSubscriptionId,
    const Utf8String projectId,
    const Utf8String iModelId,
    const Utf8String lockUrl,
    const Utf8String contextId,
    const ContextType contextType
) : GenericGlobalEvent(eventTopic, fromEventSubscriptionId, toEventSubscriptionId, projectId, iModelId, lockUrl, contextId, contextType)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             04/2018
//---------------------------------------------------------------------------------------
RefCountedPtr<struct HardiModelDeleteEvent> HardiModelDeleteEvent::Create
(
    const Utf8String eventTopic,
    const Utf8String fromEventSubscriptionId,
    const Utf8String toEventSubscriptionId,
    const Utf8String projectId,
    const Utf8String iModelId,
    const Utf8String lockUrl,
    const Utf8String contextId,
    const ContextType contextType
)
    {
    return new HardiModelDeleteEvent
    (
        eventTopic,
        fromEventSubscriptionId,
        toEventSubscriptionId,
        projectId,
        iModelId,
        lockUrl, 
        contextId, 
        contextType
    );
    }
