/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Client/Result.h>
#include <WebServices/iModelHub/Common.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Arvind.Venkateswaran   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
namespace Event
    {
    static Utf8CP EventTopic = "EventTopic";
    static Utf8CP FromEventSubscriptionId = "FromEventSubscriptionId";
    namespace LockEventProperties
        {
        static Utf8CP ObjectIds = "ObjectIds";
        static Utf8CP LockType = "LockType";
        static Utf8CP LockLevel = "LockLevel";
        static Utf8CP BriefcaseId = "BriefcaseId";
        static Utf8CP ReleasedWithChangeSet = "ReleasedWithChangeSet";
        }
    namespace ChangeSetPostPushEventProperties
        {
        static Utf8CP ChangeSetId = "ChangeSetId";
        static Utf8CP ChangeSetIndex = "ChangeSetIndex";
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
    namespace VersionEventProperties
        {
        static Utf8CP VersionId = "VersionId";
        static Utf8CP VersionName = "VersionName";
        static Utf8CP ChangeSetId = "ChangeSetId";
        }

    enum EventType
        {
        LockEvent,
        ChangeSetPostPushEvent,
        ChangeSetPrePushEvent,
        CodeEvent,
        AllLocksDeletedEvent,
        AllCodesDeletedEvent,
        UnknownEventType,
        VersionEvent,
        VersionModifiedEvent
        };

    /*--------------------------------------------------------------------------------------+
    * @bsiclass                                              Arvind.Venkateswaran   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    struct Helper
    {
    public:
        static Utf8String GetEventNameFromEventType(EventType eventType)
            {
            switch (eventType)
                {
                    case EventType::LockEvent:               return "LockEvent";
                    case EventType::ChangeSetPostPushEvent:  return "ChangeSetPostPushEvent";
                    case EventType::ChangeSetPrePushEvent:   return "ChangeSetPrePushEvent";
                    case EventType::CodeEvent:               return "CodeEvent";
                    case EventType::AllLocksDeletedEvent:    return "AllLocksDeletedEvent";
                    case EventType::AllCodesDeletedEvent:    return "AllCodesDeletedEvent";
                    case EventType::VersionEvent:            return "VersionEvent";
                    case EventType::VersionModifiedEvent:    return "VersionModifiedEvent";
                    case EventType::UnknownEventType:
                    default:                                 return "UnknownEventType";
                }
            }

        static EventType GetEventTypeFromEventName(Utf8CP eventName)
            {
            if (0 == (BeStringUtilities::Stricmp("LockEvent", eventName)))
                return EventType::LockEvent;
            if (0 == (BeStringUtilities::Stricmp("ChangeSetPostPushEvent", eventName)))
                return EventType::ChangeSetPostPushEvent;
            if (0 == (BeStringUtilities::Stricmp("ChangeSetPrePushEvent", eventName)))
                return EventType::ChangeSetPrePushEvent;
            if (0 == (BeStringUtilities::Stricmp("CodeEvent", eventName)))
                return EventType::CodeEvent;
            if (0 == (BeStringUtilities::Stricmp("AllLocksDeletedEvent", eventName)))
                return EventType::AllLocksDeletedEvent;
            if (0 == (BeStringUtilities::Stricmp("AllCodesDeletedEvent", eventName)))
                return EventType::AllCodesDeletedEvent;
            if (0 == (BeStringUtilities::Stricmp("VersionEvent", eventName)))
                return EventType::VersionEvent;
            if (0 == (BeStringUtilities::Stricmp("VersionModifiedEvent", eventName)))
                return EventType::VersionModifiedEvent;
        
            return EventType::UnknownEventType;
            }
    };

    /*--------------------------------------------------------------------------------------+
    * @bsiclass                                              Arvind.Venkateswaran   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    struct EXPORT_VTABLE_ATTRIBUTE GenericEvent : RefCountedBase
    {
    public:
        virtual EventType GetEventType() {return UnknownEventType;}
        virtual Utf8String GetEventTopic() {return "";}
        virtual Utf8String GetFromEventSubscriptionId() {return "";}
    };
    }

typedef RefCountedPtr<struct Event::GenericEvent> EventPtr;
DEFINE_TASK_TYPEDEFS(EventPtr, Event);
typedef bset<Event::EventType> EventTypeSet;

END_BENTLEY_IMODELHUB_NAMESPACE
