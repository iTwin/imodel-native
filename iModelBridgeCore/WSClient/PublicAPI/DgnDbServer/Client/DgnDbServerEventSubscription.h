/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerEventSubscription.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerResult.h>
#include <DgnDbServer/Client/Events/DgnDbServerEvent.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

typedef RefCountedPtr<struct DgnDbServerEventSubscription> DgnDbServerEventSubscriptionPtr;
DEFINE_TASK_TYPEDEFS(DgnDbServerEventSubscriptionPtr, DgnDbServerEventSubscription);

struct DgnDbServerEventSubscription : RefCountedBase
{
private:
    Utf8String m_subscriptionId;
    DgnDbServerEventTypeSet m_eventTypes;

    DgnDbServerEventSubscription(Utf8String subscriptionId, DgnDbServerEventTypeSet eventTypes) : m_subscriptionId(subscriptionId), m_eventTypes(eventTypes) {}
public:
    static DgnDbServerEventSubscriptionPtr Create(Utf8String subscriptionId, DgnDbServerEventTypeSet eventTypes) {return new DgnDbServerEventSubscription(subscriptionId, eventTypes);}
    Utf8String GetSubscriptionId() const {return m_subscriptionId;}
    DgnDbServerEventTypeSet GetEventTypes() const {return m_eventTypes;}
};

END_BENTLEY_DGNDBSERVER_NAMESPACE
