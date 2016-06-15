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
#include <DgnDbServer/Client/DgnDbServerResult.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

typedef std::shared_ptr<struct IDgnDbServerEvent> IDgnDbServerEventPtr;

DEFINE_TASK_TYPEDEFS(bvector<IDgnDbServerEventPtr>, IDgnDbServerEventCollection);
DEFINE_TASK_TYPEDEFS(IDgnDbServerEventPtr, IDgnDbServerEvent);

namespace DgnDbServerEvent
    {
    static Utf8CP RepoId = "repoId";
    static Utf8CP UserId = "userId";
    namespace LockEvent
        {
        static Utf8CP ObjectId = "ObjectId";
        static Utf8CP LockType = "LockType";
        static Utf8CP LockLevel = "LockLevel";
        static Utf8CP BriefcaseId = "BriefcaseId";
        static Utf8CP ReleasedWithRevision = "ReleasedWithRevision";
        static Utf8CP Date = "Date";
        }
    namespace RevisionEvent
        {
        static Utf8CP RevisionId = "RevisionId";
        static Utf8CP RevisionIndex = "RevisionIndex";
        static Utf8CP Date = "Date";
        }
    }

enum DgnDbServerEventType
    {
    LockEvent,
    RevisionEvent,
    UnknownEventType
    };

struct IDgnDbServerEvent
    {
    public:
        DGNDBSERVERCLIENT_EXPORT static Utf8String GetEventName(DgnDbServerEventType eventType)
                                                {
                                                switch (eventType)
                                                    {
                                                        case DgnDbServerEventType::LockEvent:   return "LockEvent";
                                                        case DgnDbServerEventType::RevisionEvent:   return "RevisionEvent";
                                                        case DgnDbServerEventType::UnknownEventType:
                                                        default:      return "UnknownEventType";
                                                    }
                                                }
        DGNDBSERVERCLIENT_EXPORT virtual const type_info& GetEventType() { return typeid(this); }
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetRepoId() { return ""; }
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetUserId() { return ""; }
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE