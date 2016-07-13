/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerRevisionEvent.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerRevisionEvent.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerRevisionEvent::DgnDbServerRevisionEvent
(
Utf8String eventTopic, 
Utf8String fromEventSubscriptionId, 
Utf8String revisionId, 
Utf8String revisionIndex
)
    {
    m_eventTopic = eventTopic;
    m_fromEventSubscriptionId = fromEventSubscriptionId;
    m_revisionId = revisionId;
    m_revisionIndex = revisionIndex;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<struct DgnDbServerRevisionEvent> DgnDbServerRevisionEvent::Create
(
Utf8String eventTopic, 
Utf8String fromEventSubscriptionId, 
Utf8String revisionId, 
Utf8String revisionIndex
)
    {
    return std::shared_ptr<struct DgnDbServerRevisionEvent>
        (new DgnDbServerRevisionEvent
               (
               eventTopic,
               fromEventSubscriptionId,
               revisionId, 
               revisionIndex));
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
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEvent::DgnDbServerEventType DgnDbServerRevisionEvent::GetEventType()
    {
    return DgnDbServerEvent::DgnDbServerEventType::RevisionEvent;
    }