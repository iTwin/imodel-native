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

typedef std::shared_ptr<struct DgnDbServerEventSubscription> DgnDbServerEventSubscriptionPtr;

DEFINE_TASK_TYPEDEFS(DgnDbServerEventSubscriptionPtr, DgnDbServerEventSubscription);

struct DgnDbServerEventSubscription
    {
    //__PUBLISH_SECTION_END__
    private:
        Utf8String m_subscriptionId;
        DgnDbServerEventTypeSet m_eventTypes;

        DgnDbServerEventSubscription(Utf8String subscriptionId, DgnDbServerEventTypeSet eventTypes);
        //__PUBLISH_SECTION_START__
    public:
        DGNDBSERVERCLIENT_EXPORT static DgnDbServerEventSubscriptionPtr Create(Utf8String subscriptionId, DgnDbServerEventTypeSet eventTypes);
        DGNDBSERVERCLIENT_EXPORT Utf8String GetSubscriptionId();
        DGNDBSERVERCLIENT_EXPORT DgnDbServerEventTypeSet GetEventTypes();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE
