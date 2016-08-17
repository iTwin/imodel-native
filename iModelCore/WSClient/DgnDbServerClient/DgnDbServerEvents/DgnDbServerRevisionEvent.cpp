/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEvents/DgnDbServerRevisionEvent.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
Utf8String briefcaseId
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
std::shared_ptr<struct DgnDbServerRevisionEvent> DgnDbServerRevisionEvent::Create
(
Utf8String eventTopic, 
Utf8String fromEventSubscriptionId, 
Utf8String revisionId, 
Utf8String revisionIndex,
Utf8String briefcaseId
)
    {
    return std::shared_ptr<struct DgnDbServerRevisionEvent>
        (new DgnDbServerRevisionEvent
               (
               eventTopic,
               fromEventSubscriptionId,
               revisionId, 
               revisionIndex,
			   briefcaseId));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionEvent::GetEventTopic()
    {
    return m_eventTopic;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionEvent::GetFromEventSubscriptionId()
    {
    return m_fromEventSubscriptionId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionEvent::GetRevisionId()
    {
    return m_revisionId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Caleb.Shafer						06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionEvent::GetRevisionIndex()
    {
    return m_revisionIndex;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             08/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionEvent::GetBriefcaseId()
{
	return m_briefcaseId;
}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEvent::DgnDbServerEventType DgnDbServerRevisionEvent::GetEventType()
    {
    return DgnDbServerEvent::DgnDbServerEventType::RevisionEvent;
    }