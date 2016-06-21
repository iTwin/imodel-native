/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEventSubscription.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerEventSubscription.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSubscription::DgnDbServerEventSubscription
(
Utf8String topicName,
Utf8String subscriptionId,
bvector<DgnDbServerEvent::DgnDbServerEventType> eventTypes
)
    {
    m_topicName = topicName;
    m_subscriptionId = subscriptionId;
    m_eventTypes = eventTypes;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSubscriptionPtr DgnDbServerEventSubscription::Create
(
Utf8String topicName, 
Utf8String subscriptionId, 
bvector<DgnDbServerEvent::DgnDbServerEventType> eventTypes
)
    {
    return DgnDbServerEventSubscriptionPtr(new DgnDbServerEventSubscription(topicName, subscriptionId, eventTypes));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerEventSubscription::GetTopicName()
    {
    return m_topicName;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerEventSubscription::GetSubscriptionId()
    {
    return m_subscriptionId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
bvector<DgnDbServerEvent::DgnDbServerEventType> DgnDbServerEventSubscription::GetEventTypes()
    {
    return m_eventTypes;
    }