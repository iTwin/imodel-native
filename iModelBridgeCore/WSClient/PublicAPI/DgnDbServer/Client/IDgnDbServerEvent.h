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

struct IDgnDbServerEvent
    {
    public:
        DGNDBSERVERCLIENT_EXPORT virtual const type_info& GetEventType() { return typeid(this); }
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetRepoId() { return ""; }
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetUserId() { return ""; }
    };

namespace DgnDbServerEvent
    {
    static Utf8CP RepoId = "repoId";
    static Utf8CP UserId = "userId";
    namespace LockEvent
        {
        static Utf8CP LockId = "LockId";
        static Utf8CP LockType = "LockType";
        static Utf8CP Date = "Date";
        }
    namespace RevisionEvent
        {
        static Utf8CP RevisionId = "RevisionId";
        static Utf8CP ConnectionId = "connectionId";
        static Utf8CP Date = "Date";
        }
    }

END_BENTLEY_DGNDBSERVER_NAMESPACE