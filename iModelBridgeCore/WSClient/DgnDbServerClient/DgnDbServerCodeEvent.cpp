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
bvector<Utf8String> values,
Utf8String state,
Utf8String briefcaseId,
Utf8String usedWithRevision
)
    {
    m_eventTopic = eventTopic;
    m_fromEventSubscriptionId = fromEventSubscriptionId;
    m_codeAuthorityId = codeAuthorityId;
    m_codeNamespace = codeNamespace;
    m_values = values;
    m_state = state;
    m_briefcaseId = briefcaseId;
    m_usedWithRevision = usedWithRevision;
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
bvector<Utf8String> values,
Utf8String state,
Utf8String briefcaseId,
Utf8String usedWithRevision
)
    {
    return std::shared_ptr<struct DgnDbServerCodeEvent>
        (new DgnDbServerCodeEvent
                (
                eventTopic,
                fromEventSubscriptionId,
                codeAuthorityId,
                codeNamespace,
                values,
                state,
                briefcaseId,
                usedWithRevision));
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
bvector<Utf8String> DgnDbServerCodeEvent::GetValues()
    {
    return m_values;
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