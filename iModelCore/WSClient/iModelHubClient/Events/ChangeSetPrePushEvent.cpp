/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Events/ChangeSetPrePushEvent.cpp $
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
RefCountedPtr<struct DgnDbServerRevisionCreateEvent> DgnDbServerRevisionCreateEvent::Create
(
Utf8String eventTopic, 
Utf8String fromEventSubscriptionId
)
    {
    return new DgnDbServerRevisionCreateEvent(eventTopic, fromEventSubscriptionId);
    }
