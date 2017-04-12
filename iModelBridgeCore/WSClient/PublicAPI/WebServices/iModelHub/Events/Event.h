/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Events/Event.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerResult.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

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
        static Utf8CP BriefcaseId = "BriefcaseId";
        }
    namespace CodeEventProperties
        {
        static Utf8CP CodeSpecId = "CodeSpecId";
        static Utf8CP CodeScope = "CodeScope";
        static Utf8CP Values = "Values";
        static Utf8CP State = "State";
        static Utf8CP BriefcaseId = "BriefcaseId";
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
        RevisionCreateEvent,
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
    public:
        static Utf8String GetEventNameFromEventType(DgnDbServerEventType eventType)
            {
            switch (eventType)
                {
                    case DgnDbServerEventType::LockEvent:             return "LockEvent";
                    case DgnDbServerEventType::RevisionEvent:         return "RevisionEvent";
                    case DgnDbServerEventType::RevisionCreateEvent:   return "RevisionCreateEvent";
                    case DgnDbServerEventType::CodeEvent:             return "CodeEvent";
                    case DgnDbServerEventType::AllLocksDeletedEvent:  return "AllLocksDeletedEvent";
                    case DgnDbServerEventType::AllCodesDeletedEvent:  return "AllCodesDeletedEvent";
                    case DgnDbServerEventType::UnknownEventType:
                    default:      return "UnknownEventType";
                }
            }

        static DgnDbServerEventType GetEventTypeFromEventName(Utf8CP eventName)
            {
            if (0 == (BeStringUtilities::Stricmp("LockEvent", eventName)))
                return DgnDbServerEventType::LockEvent;
            if (0 == (BeStringUtilities::Stricmp("RevisionEvent", eventName)))
                return DgnDbServerEventType::RevisionEvent;
            if (0 == (BeStringUtilities::Stricmp("RevisionCreateEvent", eventName)))
                return DgnDbServerEventType::RevisionCreateEvent;
            if (0 == (BeStringUtilities::Stricmp("CodeEvent", eventName)))
                return DgnDbServerEventType::CodeEvent;
            if (0 == (BeStringUtilities::Stricmp("AllLocksDeletedEvent", eventName)))
                return DgnDbServerEventType::AllLocksDeletedEvent;
            if (0 == (BeStringUtilities::Stricmp("AllCodesDeletedEvent", eventName)))
                return DgnDbServerEventType::AllCodesDeletedEvent;
        
            return DgnDbServerEventType::UnknownEventType;
            }
    };

    /*--------------------------------------------------------------------------------------+
    * @bsiclass                                              Arvind.Venkateswaran   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    struct EXPORT_VTABLE_ATTRIBUTE GenericEvent : RefCountedBase
    {
    public:
        virtual DgnDbServerEventType GetEventType() {return UnknownEventType;}
        virtual Utf8String GetEventTopic() {return "";}
        virtual Utf8String GetFromEventSubscriptionId() {return "";}
    };
    }

typedef RefCountedPtr<struct DgnDbServerEvent::GenericEvent> DgnDbServerEventPtr;
DEFINE_TASK_TYPEDEFS(DgnDbServerEventPtr, DgnDbServerEvent);
typedef bset<DgnDbServerEvent::DgnDbServerEventType> DgnDbServerEventTypeSet;

END_BENTLEY_DGNDBSERVER_NAMESPACE
