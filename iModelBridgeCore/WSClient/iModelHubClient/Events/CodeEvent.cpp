/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Events/CodeEvent.h>
#include "../Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             65/2016
//---------------------------------------------------------------------------------------
CodeEvent::CodeEvent
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
RefCountedPtr<struct CodeEvent> CodeEvent::Create
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
    return new CodeEvent
                (
                eventTopic,
                fromEventSubscriptionId,
                codeSpecId,
                codeScope,
                values,
                state,
                briefcaseId);
    }
