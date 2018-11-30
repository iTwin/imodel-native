/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Events/ChangeSetPrePushEvent.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Events/ChangeSetPrePushEvent.h>
#include "../Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
ChangeSetPrePushEvent::ChangeSetPrePushEvent
(
Utf8String eventTopic,
Utf8String fromEventSubscriptionId
)
    {
    m_eventTopic = eventTopic;
    m_fromEventSubscriptionId = fromEventSubscriptionId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct ChangeSetPrePushEvent> ChangeSetPrePushEvent::Create
(
Utf8String eventTopic,
Utf8String fromEventSubscriptionId
)
    {
    return new ChangeSetPrePushEvent(eventTopic, fromEventSubscriptionId);
    }
