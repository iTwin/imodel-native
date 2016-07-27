/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/Events/DgnDbServerEventParser.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/Events/DgnDbServerEvent.h>
#include <DgnDbServer/Client/DgnDbServerEventSubscription.h>
#include <DgnDbServer/Client/DgnDbServerEventSAS.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Arvind.Venkateswaran   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnDbServerEventParser 
    {
    public:
        DGNDBSERVERCLIENT_EXPORT static DgnDbServerEventPtr ParseEvent
            (
            Utf8CP responseContentType,
            Utf8String responseString
            );

        DGNDBSERVERCLIENT_EXPORT static std::shared_ptr<struct DgnDbServerLockEvent> GetLockEvent(DgnDbServerEventPtr eventPtr);

        DGNDBSERVERCLIENT_EXPORT static std::shared_ptr<struct DgnDbServerRevisionEvent> GetRevisionEvent(DgnDbServerEventPtr eventPtr);

        DGNDBSERVERCLIENT_EXPORT static std::shared_ptr<struct DgnDbServerCodeEvent> GetCodeEvent(DgnDbServerEventPtr eventPtr);

        DGNDBSERVERCLIENT_EXPORT static std::shared_ptr<struct DgnDbServerDeletedEvent> GetDeletedEvent(DgnDbServerEventPtr eventPtr);
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE