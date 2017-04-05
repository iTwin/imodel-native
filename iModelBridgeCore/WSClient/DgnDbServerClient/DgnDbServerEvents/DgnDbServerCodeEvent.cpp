/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEvents/DgnDbServerCodeEvent.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/Events/DgnDbServerCodeEvent.h>
#include "../DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             65/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeEvent::DgnDbServerCodeEvent
(
Utf8String eventTopic,
Utf8String fromEventSubscriptionId,
Utf8String codeSpecId,
Utf8String codeScope,
bvector<Utf8String> values,
int        state,
int        briefcaseId
)
    {
    m_eventTopic = eventTopic;
    m_fromEventSubscriptionId = fromEventSubscriptionId;
    m_codeSpecId = codeSpecId;
    m_codeScope = codeScope;
    m_values = values;
    m_state = state;
    m_briefcaseId = briefcaseId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct DgnDbServerCodeEvent> DgnDbServerCodeEvent::Create
(
Utf8String eventTopic,
Utf8String fromEventSubscriptionId,
Utf8String codeSpecId,
Utf8String codeScope,
bvector<Utf8String> values,
int        state,
int        briefcaseId
)
    {
    return new DgnDbServerCodeEvent
                (
                eventTopic,
                fromEventSubscriptionId,
                codeSpecId,
                codeScope,
                values,
                state,
                briefcaseId);
    }
