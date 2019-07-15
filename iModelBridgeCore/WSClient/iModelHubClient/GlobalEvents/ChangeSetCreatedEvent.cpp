/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/GlobalEvents/ChangeSetCreatedEvent.h>
#include "../Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             04/2018
//---------------------------------------------------------------------------------------
ChangeSetCreatedEvent::ChangeSetCreatedEvent
(
    const Utf8String eventTopic,
    const Utf8String fromEventSubscriptionId,
    const Utf8String toEventSubscriptionId,
    const Utf8String projectId,
    const Utf8String iModelId,
    const Utf8String changeSetId,
    const Utf8String changeSetIndex,
    const int        briefcaseId,
    const Utf8String lockUrl,
    const Utf8String contextId,
    const ContextType contextType
) : GenericGlobalEvent(eventTopic, fromEventSubscriptionId, toEventSubscriptionId, projectId, iModelId, lockUrl, contextId, contextType)
    {
    m_changeSetId = changeSetId;
    m_changeSetIndex = changeSetIndex;
    m_briefcaseId = briefcaseId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             04/2018
//---------------------------------------------------------------------------------------
RefCountedPtr<struct ChangeSetCreatedEvent> ChangeSetCreatedEvent::Create
(
    const Utf8String eventTopic,
    const Utf8String fromEventSubscriptionId,
    const Utf8String toEventSubscriptionId,
    const Utf8String projectId,
    const Utf8String iModelId,
    const Utf8String changeSetId,
    const Utf8String changeSetIndex,
    const int        briefcaseId,
    const Utf8String lockUrl,
    const Utf8String contextId,
    const ContextType contextType
)
    {
    return new ChangeSetCreatedEvent
    (
        eventTopic,
        fromEventSubscriptionId,
        toEventSubscriptionId,
        projectId,
        iModelId,
        changeSetId,
        changeSetIndex,
        briefcaseId,
        lockUrl,
        contextId,
        contextType
    );
    }
