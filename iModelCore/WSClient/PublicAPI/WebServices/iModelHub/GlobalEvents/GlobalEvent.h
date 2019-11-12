/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Client/Result.h>
#include <WebServices/iModelHub/Common.h>

#define EVENT_PROPERTY_EVENT_TOPIC "EventTopic"
#define EVENT_PROPERTY_FROM_EVENT_SUBSCRIPTION_ID "FromEventSubscriptionId"
#define EVENT_PROPERTY_TO_EVENT_SUBSCRIPTION_ID "ToEventSubscriptionId"
#define EVENT_PROPERTY_PROJECT_ID "ProjectId"
#define EVENT_PROPERTY_IMODEL_ID "iModelId"
#define EVENT_PROPERTY_CHANGE_SET_ID "ChangeSetId"
#define EVENT_PROPERTY_CHANGE_SET_INDEX "ChangeSetIndex"
#define EVENT_PROPERTY_BRIEFCASE_ID "BriefcaseId"
#define EVENT_PROPERTY_VERSION_ID "VersionId"
#define EVENT_PROPERTY_VERSION_NAME "VersionName"
#define EVENT_PROPERTY_CONTEXT_ID "ContextId"
#define EVENT_PROPERTY_CONTEXT_TYPE_ID "ContextTypeId"

#define IMODEL_CREATED_EVENT "iModelCreatedEvent"
#define SOFT_IMODEL_DELETE_EVENT "SoftiModelDeleteEvent"
#define HARD_IMODEL_DELETE_EVENT "HardiModelDeleteEvent"
#define CHANGE_SET_CREATED_EVENT "ChangeSetCreatedEvent"
#define NAMED_VERSION_CREATED_EVENT "NamedVersionCreatedEvent"
#define UNKNOWN_EVENT_TYPE "UnknownEventType"

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
struct GlobalEventManager;
USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas            06/2019
//=======================================================================================
enum ContextType
    {
    Unknown,
    Asset = 2,
    Project = 3
    };

//=======================================================================================
//@bsiclass                                      Karolis.Uzkuraitis             04/2018
//=======================================================================================
namespace GlobalEvent
    {
    static Utf8CP Property_EventTopic = EVENT_PROPERTY_EVENT_TOPIC;
    static Utf8CP Property_FromEventSubscriptionId = EVENT_PROPERTY_FROM_EVENT_SUBSCRIPTION_ID;
    static Utf8CP Property_ToEventSubscriptionId = EVENT_PROPERTY_TO_EVENT_SUBSCRIPTION_ID;
    static Utf8CP Property_ProjectId = EVENT_PROPERTY_PROJECT_ID;
    static Utf8CP Property_iModelId = EVENT_PROPERTY_IMODEL_ID;
    static Utf8CP Property_ContextId = EVENT_PROPERTY_CONTEXT_ID;
    static Utf8CP Property_ContextTypeId = EVENT_PROPERTY_CONTEXT_TYPE_ID;

    namespace ChangeSetCreatedEventProperties
        {
        static Utf8CP Property_ChangeSetId = EVENT_PROPERTY_CHANGE_SET_ID;
        static Utf8CP Property_ChangeSetIndex = EVENT_PROPERTY_CHANGE_SET_INDEX;
        static Utf8CP Property_BriefcaseId = EVENT_PROPERTY_BRIEFCASE_ID;
        }

    namespace NamedVersionCreatedEventProperties
        {
        static Utf8CP Property_VersionId = EVENT_PROPERTY_VERSION_ID;
        static Utf8CP Property_VersionName = EVENT_PROPERTY_VERSION_NAME;
        static Utf8CP Property_ChangeSetId = EVENT_PROPERTY_CHANGE_SET_ID;
        }

    enum GlobalEventType
        {
        UnknownEventType = -1,
        iModelCreatedEvent = 0,
        SoftiModelDeleteEvent = 1,
        HardiModelDeleteEvent = 2,
        ChangeSetCreatedEvent = 3,
        NamedVersionCreatedEvent = 4
        };

    //=======================================================================================
    //@bsiclass                                      Karolis.Uzkuraitis             04/2018
    //=======================================================================================
    struct Helper
        {
        //---------------------------------------------------------------------------------------
        //@bsimethod                                     Karolis.Uzkuraitis             04/2018
        //---------------------------------------------------------------------------------------
        static Utf8String GetEventNameFromEventType(const GlobalEventType globalEventType)
            {
            switch (globalEventType)
                {
                case iModelCreatedEvent:       return IMODEL_CREATED_EVENT;
                case SoftiModelDeleteEvent:    return SOFT_IMODEL_DELETE_EVENT;
                case HardiModelDeleteEvent:    return HARD_IMODEL_DELETE_EVENT;
                case ChangeSetCreatedEvent:    return CHANGE_SET_CREATED_EVENT;
                case NamedVersionCreatedEvent: return NAMED_VERSION_CREATED_EVENT;
                default:                                        return UNKNOWN_EVENT_TYPE;
                }
            }

        //---------------------------------------------------------------------------------------
        //@bsimethod                                     Karolis.Uzkuraitis             04/2018
        //---------------------------------------------------------------------------------------
        static GlobalEventType GetEventTypeFromEventName(const Utf8CP eventName)
            {
            if (0 == BeStringUtilities::Stricmp(IMODEL_CREATED_EVENT, eventName))
                return iModelCreatedEvent;
            if (0 == BeStringUtilities::Stricmp(SOFT_IMODEL_DELETE_EVENT, eventName))
                return SoftiModelDeleteEvent;
            if (0 == BeStringUtilities::Stricmp(HARD_IMODEL_DELETE_EVENT, eventName))
                return HardiModelDeleteEvent;
            if (0 == BeStringUtilities::Stricmp(CHANGE_SET_CREATED_EVENT, eventName))
                return ChangeSetCreatedEvent;
            if (0 == BeStringUtilities::Stricmp(NAMED_VERSION_CREATED_EVENT, eventName))
                return NamedVersionCreatedEvent;

            return UnknownEventType;
            }
        };

    //=======================================================================================
    //@bsiclass                                      Karolis.Uzkuraitis             04/2018
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE GenericGlobalEvent : RefCountedBase
        {
    private:
        friend GlobalEventManager;
    protected:
        Utf8String m_eventTopic;
        Utf8String m_fromEventSubscriptionId;
        Utf8String m_toEventSubscriptionId;
        Utf8String m_projectId;
        Utf8String m_iModelId;
        Utf8String m_lockUrl;
        Utf8String m_contextId;
        ContextType m_contextType;

        GenericGlobalEvent
        (
            const Utf8String eventTopic,
            const Utf8String fromEventSubscriptionId,
            const Utf8String toEventSubscriptionId,
            const Utf8String projectId,
            const Utf8String iModelId,
            const Utf8String lockUrl,
            const Utf8String contextId,
            const ContextType contextType
        )
            {
            m_eventTopic = eventTopic;
            m_fromEventSubscriptionId = fromEventSubscriptionId;
            m_toEventSubscriptionId = toEventSubscriptionId;
            m_projectId = projectId;
            m_iModelId = iModelId;
            m_lockUrl = lockUrl;
            m_contextId = contextId;
            m_contextType = contextType;
            }

    public:
        virtual GlobalEventType GetEventType() { return UnknownEventType; }
        Utf8String GetEventTopic() const { return m_eventTopic; }
        Utf8String GetFromEventSubscriptionId() const { return m_fromEventSubscriptionId; }
        Utf8String GetToEventSubscriptionId() const { return m_toEventSubscriptionId; }
        Utf8String GetProjectId() const { return m_projectId; }
        Utf8String GetiModelId() const { return m_iModelId; }
        Utf8String GetContextId() const { return m_contextId; }
        ContextType GetContextType() const { return m_contextType; }
        };
    }

typedef RefCountedPtr<struct GlobalEvent::GenericGlobalEvent> GlobalEventPtr;
DEFINE_TASK_TYPEDEFS(GlobalEventPtr, GlobalEvent);
typedef bset<GlobalEvent::GlobalEventType> GlobalEventTypeSet;

END_BENTLEY_IMODELHUB_NAMESPACE
