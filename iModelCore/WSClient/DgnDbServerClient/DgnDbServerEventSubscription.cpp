/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEventSubscription.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
Utf8String subscriptionId,
DgnDbServerEventTypeSet eventTypes
)
    {
    m_subscriptionId = subscriptionId;
    m_eventTypes = eventTypes;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSubscriptionPtr DgnDbServerEventSubscription::Create
(
Utf8String subscriptionId, 
DgnDbServerEventTypeSet eventTypes
)
    {
    return DgnDbServerEventSubscriptionPtr(new DgnDbServerEventSubscription(subscriptionId, eventTypes));
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
DgnDbServerEventTypeSet DgnDbServerEventSubscription::GetEventTypes()
    {
    return m_eventTypes;
    }
