/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/IDgnDbServerEvent.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

typedef std::shared_ptr<struct IDgnDbServerEvent> IDgnDbServerEventPtr;
typedef bvector<IDgnDbServerEvent> IDgnDbServerEvents;
typedef std::tuple<Utf8String, IDgnDbServerEvent> IDgnDbServerGenericEvent;

struct IDgnDbServerEvent
    {
    public:
        virtual Utf8String GetEventType() = 0;
        virtual Utf8String GetRepoId() = 0;
        virtual Utf8String GetUserId() = 0;
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE