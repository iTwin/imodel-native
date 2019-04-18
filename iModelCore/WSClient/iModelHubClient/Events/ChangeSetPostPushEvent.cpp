/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Events/ChangeSetPostPushEvent.h>
#include "../Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
ChangeSetPostPushEvent::ChangeSetPostPushEvent
(
Utf8String eventTopic, 
Utf8String fromEventSubscriptionId, 
Utf8String changeSetId, 
Utf8String changeSetIndex,
int        briefcaseId
)
    {
    m_eventTopic = eventTopic;
    m_fromEventSubscriptionId = fromEventSubscriptionId;
    m_changeSetId = changeSetId;
    m_changeSetIndex = changeSetIndex;
	m_briefcaseId = briefcaseId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct ChangeSetPostPushEvent> ChangeSetPostPushEvent::Create
(
Utf8String eventTopic, 
Utf8String fromEventSubscriptionId, 
Utf8String changeSetId, 
Utf8String changeSetIndex,
int        briefcaseId
)
    {
    return new ChangeSetPostPushEvent
               (
               eventTopic,
               fromEventSubscriptionId,
               changeSetId, 
               changeSetIndex,
			   briefcaseId);
    }
