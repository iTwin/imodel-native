/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerDeletedEvent.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerDeletedEvent.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             07/2016
//---------------------------------------------------------------------------------------
DgnDbServerDeletedEvent::DgnDbServerDeletedEvent
(
Utf8String eventTopic,
Utf8String fromEventSubscriptionId,
Utf8String briefcaseId,
DgnDbServerEvent::DgnDbServerEventType deletedEventType
)
    {
    m_eventTopic = eventTopic;
    m_fromEventSubscriptionId = fromEventSubscriptionId;
    m_briefcaseId = briefcaseId;
    m_deletedEventType = deletedEventType;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             07/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<struct DgnDbServerDeletedEvent> DgnDbServerDeletedEvent::Create
(
Utf8String eventTopic,
Utf8String fromEventSubscriptionId,
Utf8String briefcaseId,
DgnDbServerEvent::DgnDbServerEventType deletedEventType
)
    {
    Utf8String eventName = DgnDbServerEvent::Helper::GetEventNameFromEventType(deletedEventType);
    if (
        0 == eventName.CompareToI("AllLocksDeletedEvent") ||
        0 == eventName.CompareToI("AllCodesDeletedEvent")
       )
        return std::shared_ptr<struct DgnDbServerDeletedEvent>
            (
            new DgnDbServerDeletedEvent
            (
            eventTopic,
            fromEventSubscriptionId,
            briefcaseId,
            deletedEventType
            )
            );       
    return nullptr;    
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             07/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerDeletedEvent::GetEventTopic()
    {
    return m_eventTopic;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             07/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerDeletedEvent::GetFromEventSubscriptionId()
    {
    return m_fromEventSubscriptionId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             07/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerDeletedEvent::GetBriefcaseId()
    {
    return m_briefcaseId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran				07/2016
//---------------------------------------------------------------------------------------
DgnDbServerEvent::DgnDbServerEventType DgnDbServerDeletedEvent::GetDeletedEventType()
    {
    return m_deletedEventType;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             07/2016
//---------------------------------------------------------------------------------------
DgnDbServerEvent::DgnDbServerEventType DgnDbServerDeletedEvent::GetEventType()
    {
    return m_deletedEventType;
    }
