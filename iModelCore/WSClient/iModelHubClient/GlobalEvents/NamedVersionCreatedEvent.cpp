/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/GlobalEvents/NamedVersionCreatedEvent.h>
#include "../Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             04/2018
//---------------------------------------------------------------------------------------
NamedVersionCreatedEvent::NamedVersionCreatedEvent
(
    const Utf8String eventTopic,
    const Utf8String fromEventSubscriptionId,
    const Utf8String toEventSubscriptionId,
    const Utf8String projectId,
    const Utf8String iModelId,
    const Utf8String versionId,
    const Utf8String versionName,
    const Utf8String changeSetId,
    const Utf8String lockUrl,
    const Utf8String contextId,
    const ContextType contextType
) : GenericGlobalEvent(eventTopic, fromEventSubscriptionId, toEventSubscriptionId, projectId, iModelId, lockUrl, contextId, contextType)
    {
    m_versionId = versionId;
    m_versionName = versionName;
    m_changeSetId = changeSetId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             04/2018
//---------------------------------------------------------------------------------------
RefCountedPtr<struct NamedVersionCreatedEvent> NamedVersionCreatedEvent::Create
(
    const Utf8String eventTopic,
    const Utf8String fromEventSubscriptionId,
    const Utf8String toEventSubscriptionId,
    const Utf8String projectId,
    const Utf8String iModelId,
    const Utf8String versionId,
    const Utf8String versionName,
    const Utf8String changeSetId,
    const Utf8String lockUrl,
    const Utf8String contextId,
    const ContextType contextType
)
    {
    return new NamedVersionCreatedEvent
    (
        eventTopic,
        fromEventSubscriptionId,
        toEventSubscriptionId,
        projectId,
        iModelId,
        versionId,
        versionName,
        changeSetId,
        lockUrl, 
        contextId, 
        contextType
    );
    }
