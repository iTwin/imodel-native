/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/Events/DgnDbServerEvent.h $
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

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Arvind.Venkateswaran   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
namespace DgnDbServerEvent
    {
    static Utf8CP EventTopic = "EventTopic";
    static Utf8CP FromEventSubscriptionId = "FromEventSubscriptionId";
    namespace LockEventProperties
        {
        static Utf8CP ObjectIds = "ObjectIds";
        static Utf8CP LockType = "LockType";
        static Utf8CP LockLevel = "LockLevel";
        static Utf8CP BriefcaseId = "BriefcaseId";
        static Utf8CP ReleasedWithRevision = "ReleasedWithRevision";
        }
    namespace RevisionEventProperties
        {
        static Utf8CP RevisionId = "RevisionId";
        static Utf8CP RevisionIndex = "RevisionIndex";
        }
    namespace CodeEventProperties
        {
        static Utf8CP CodeAuthorityId = "CodeAuthorityId";
        static Utf8CP Namespace = "Namespace";
        static Utf8CP Values = "Values";
        static Utf8CP Reserved = "Reserved";
        static Utf8CP Used = "Used";
        static Utf8CP BriefcaseId = "BriefcaseId";
        static Utf8CP UsedWithRevision = "UsedWithRevision";
        }
    namespace DeletedEventProperties
        {
        static Utf8CP AllLocksDeletedEvent = "AllLocksDeletedEvent";
        static Utf8CP AllCodesDeletedEvent = "AllCodesDeletedEvent";
        static Utf8CP BriefcaseId = "BriefcaseId";
        }

    enum DgnDbServerEventType
        {
        LockEvent,
        RevisionEvent,
        CodeEvent,
        AllLocksDeletedEvent,
        AllCodesDeletedEvent,
        UnknownEventType
        };

    /*--------------------------------------------------------------------------------------+
    * @bsiclass                                              Arvind.Venkateswaran   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    struct Helper
        {
        DGNDBSERVERCLIENT_EXPORT static Utf8String GetEventNameFromEventType(DgnDbServerEventType eventType)
            {
            switch (eventType)
                {
                    case DgnDbServerEventType::LockEvent:             return "LockEvent";
                    case DgnDbServerEventType::RevisionEvent:         return "RevisionEvent";
                    case DgnDbServerEventType::CodeEvent:             return "CodeEvent";
                    case DgnDbServerEventType::AllLocksDeletedEvent:  return "AllLocksDeletedEvent";
                    case DgnDbServerEventType::AllCodesDeletedEvent:  return "AllCodesDeletedEvent";
                    case DgnDbServerEventType::UnknownEventType:
                    default:      return "UnknownEventType";
                }
            }

        DGNDBSERVERCLIENT_EXPORT static DgnDbServerEventType GetEventTypeFromEventName(Utf8CP eventName)
            {
            if (0 == (BeStringUtilities::Stricmp("LockEvent", eventName)))
                return DgnDbServerEventType::LockEvent;
            else if (0 == (BeStringUtilities::Stricmp("RevisionEvent", eventName)))
                return DgnDbServerEventType::RevisionEvent;
            else if (0 == (BeStringUtilities::Stricmp("CodeEvent", eventName)))
                return DgnDbServerEventType::CodeEvent;
            else if (0 == (BeStringUtilities::Stricmp("AllLocksDeletedEvent", eventName)))
                return DgnDbServerEventType::AllLocksDeletedEvent;
            else if (0 == (BeStringUtilities::Stricmp("AllCodesDeletedEvent", eventName)))
                return DgnDbServerEventType::AllCodesDeletedEvent;
            else
                return DgnDbServerEventType::UnknownEventType;
            }
        };

    /*--------------------------------------------------------------------------------------+
    * @bsiclass                                              Arvind.Venkateswaran   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    struct GenericEvent
        {
        public:
            DGNDBSERVERCLIENT_EXPORT virtual DgnDbServerEventType GetEventType() { return UnknownEventType; }
            DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetEventTopic() { return ""; }
            DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetFromEventSubscriptionId() { return ""; }
        };
    }

typedef std::shared_ptr<struct DgnDbServerEvent::GenericEvent> DgnDbServerEventPtr;
DEFINE_TASK_TYPEDEFS(DgnDbServerEventPtr, DgnDbServerEvent);

END_BENTLEY_DGNDBSERVER_NAMESPACE