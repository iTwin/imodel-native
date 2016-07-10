/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerCodeEvent.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerCodeEvent.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             65/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeEvent::DgnDbServerCodeEvent
(
Utf8String eventTopic,
Utf8String fromEventSubscriptionId,
Utf8String codeAuthorityId,
Utf8String codeNamespace,
Utf8String value,
Utf8String state,
Utf8String briefcaseId,
Utf8String usedWithRevision,
Utf8String date
)
    {
    m_eventTopic = eventTopic;
    m_fromEventSubscriptionId = fromEventSubscriptionId;
    m_codeAuthorityId = codeAuthorityId;
    m_codeNamespace = codeNamespace;
    m_value = value;
    m_state = state;
    m_briefcaseId = briefcaseId;
    m_usedWithRevision = usedWithRevision;
    m_date = date;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<struct DgnDbServerCodeEvent> DgnDbServerCodeEvent::Create
(
Utf8String eventTopic,
Utf8String fromEventSubscriptionId,
Utf8String codeAuthorityId,
Utf8String codeNamespace,
Utf8String value,
Utf8String state,
Utf8String briefcaseId,
Utf8String usedWithRevision,
Utf8String date
)
    {
    return std::shared_ptr<struct DgnDbServerCodeEvent>
        (new DgnDbServerCodeEvent
                (
                eventTopic,
                fromEventSubscriptionId,
                codeAuthorityId,
                codeNamespace,
                value,
                state,
                briefcaseId,
                usedWithRevision,
                date));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerCodeEvent::GetEventTopic()
    {
    return m_eventTopic;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerCodeEvent::GetFromEventSubscriptionId()
    {
    return m_fromEventSubscriptionId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerCodeEvent::GetDate()
    {
    return m_date;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerCodeEvent::GetCodeAuthorityId()
    {
    return m_codeAuthorityId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerCodeEvent::GetNamespace()
    {
    return m_codeNamespace;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerCodeEvent::GetValue()
    {
    return m_value;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerCodeEvent::GetState()
    {
    return m_state;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerCodeEvent::GetBriefcaseId()
    {
    return m_briefcaseId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerCodeEvent::GetUsedWithRevision()
    {
    return m_usedWithRevision;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             07/2016
//---------------------------------------------------------------------------------------
DgnDbServerEvent::DgnDbServerEventType DgnDbServerCodeEvent::GetEventType()
    {
    return DgnDbServerEvent::DgnDbServerEventType::CodeEvent;
    }