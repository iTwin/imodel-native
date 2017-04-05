/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEvents/DgnDbServerRevisionEvent.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/Events/DgnDbServerRevisionEvent.h>
#include "../DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerRevisionEvent::DgnDbServerRevisionEvent
(
Utf8String eventTopic, 
Utf8String fromEventSubscriptionId, 
Utf8String revisionId, 
Utf8String revisionIndex,
int        briefcaseId
)
    {
    m_eventTopic = eventTopic;
    m_fromEventSubscriptionId = fromEventSubscriptionId;
    m_revisionId = revisionId;
    m_revisionIndex = revisionIndex;
	m_briefcaseId = briefcaseId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct DgnDbServerRevisionEvent> DgnDbServerRevisionEvent::Create
(
Utf8String eventTopic, 
Utf8String fromEventSubscriptionId, 
Utf8String revisionId, 
Utf8String revisionIndex,
int        briefcaseId
)
    {
    return new DgnDbServerRevisionEvent
               (
               eventTopic,
               fromEventSubscriptionId,
               revisionId, 
               revisionIndex,
			   briefcaseId);
    }
