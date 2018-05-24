/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/GlobalEvents/NamedVersionCreatedEvent.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
    const Utf8String changeSetId
) : GenericGlobalEvent(eventTopic, fromEventSubscriptionId, toEventSubscriptionId, projectId, iModelId)
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
    const Utf8String changeSetId
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
        changeSetId
    );
    }
