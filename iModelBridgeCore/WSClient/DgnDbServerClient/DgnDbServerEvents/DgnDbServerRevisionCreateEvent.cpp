/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEvents/DgnDbServerRevisionCreateEvent.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/Events/DgnDbServerRevisionCreateEvent.h>
#include "../DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerRevisionCreateEvent::DgnDbServerRevisionCreateEvent
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
std::shared_ptr<struct DgnDbServerRevisionCreateEvent> DgnDbServerRevisionCreateEvent::Create
(
Utf8String eventTopic, 
Utf8String fromEventSubscriptionId
)
    {
    return std::shared_ptr<struct DgnDbServerRevisionCreateEvent> (new DgnDbServerRevisionCreateEvent(eventTopic, fromEventSubscriptionId));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionCreateEvent::GetEventTopic()
    {
    return m_eventTopic;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionCreateEvent::GetFromEventSubscriptionId()
    {
    return m_fromEventSubscriptionId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerEvent::DgnDbServerEventType DgnDbServerRevisionCreateEvent::GetEventType()
    {
    return DgnDbServerEvent::DgnDbServerEventType::RevisionCreateEvent;
    }
