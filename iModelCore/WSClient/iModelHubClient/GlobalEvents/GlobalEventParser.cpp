/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/GlobalEvents/GlobalEventParser.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/GlobalEvents/GlobalEventParser.h>
#include <WebServices/iModelHub/GlobalEvents/ChangeSetCreatedEvent.h>
#include <WebServices/iModelHub/GlobalEvents/HardiModelDeleteEvent.h>
#include <WebServices/iModelHub/GlobalEvents/iModelCreatedEvent.h>
#include <WebServices/iModelHub/GlobalEvents/NamedVersionCreatedEvent.h>
#include <WebServices/iModelHub/GlobalEvents/SoftiModelDeleteEvent.h>
#include "../Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
std::shared_ptr<Json::Value> EventPropertiesToJSON(const Utf8String jsonString, const GlobalEvent::GlobalEventType eventType)
    {
    Json::Reader reader;
    Json::Value data(Json::objectValue);
    if (!reader.parse(jsonString, data) && !data.isArray())
        return nullptr;

    switch (eventType)
        {
        case GlobalEvent::ChangeSetCreatedEvent:
            {
            if (
                data.isMember(GlobalEvent::EventTopic) &&
                data.isMember(GlobalEvent::FromEventSubscriptionId) &&
                data.isMember(GlobalEvent::ToEventSubscriptionId) &&
                data.isMember(GlobalEvent::iModelId) &&
                data.isMember(GlobalEvent::ProjectId) &&
                data.isMember(GlobalEvent::ChangeSetCreatedEventProperties::BriefcaseId) &&
                data.isMember(GlobalEvent::ChangeSetCreatedEventProperties::ChangeSetId) &&
                data.isMember(GlobalEvent::ChangeSetCreatedEventProperties::ChangeSetIndex)
                )
            return std::make_shared<Json::Value>(data);
            break;
            }
        case GlobalEvent::HardiModelDeleteEvent:
            {
            if (
                data.isMember(GlobalEvent::EventTopic) &&
                data.isMember(GlobalEvent::FromEventSubscriptionId) &&
                data.isMember(GlobalEvent::ToEventSubscriptionId) &&
                data.isMember(GlobalEvent::iModelId) &&
                data.isMember(GlobalEvent::ProjectId)
                )
            return std::make_shared<Json::Value>(data);
            break;
            }
        case GlobalEvent::iModelCreatedEvent:
            {
            if (
                data.isMember(GlobalEvent::EventTopic) &&
                data.isMember(GlobalEvent::FromEventSubscriptionId) &&
                data.isMember(GlobalEvent::ToEventSubscriptionId) &&
                data.isMember(GlobalEvent::iModelId) &&
                data.isMember(GlobalEvent::ProjectId)
                )
            return std::make_shared<Json::Value>(data);
            break;
            }
        case GlobalEvent::NamedVersionCreatedEvent:
            {
            if (
                data.isMember(GlobalEvent::EventTopic) &&
                data.isMember(GlobalEvent::FromEventSubscriptionId) &&
                data.isMember(GlobalEvent::ToEventSubscriptionId) &&
                data.isMember(GlobalEvent::iModelId) &&
                data.isMember(GlobalEvent::ProjectId) &&
                data.isMember(GlobalEvent::NamedVersionCreatedEventProperties::ChangeSetId) &&
                data.isMember(GlobalEvent::NamedVersionCreatedEventProperties::VersionId) &&
                data.isMember(GlobalEvent::NamedVersionCreatedEventProperties::VersionName)
                )
            return std::make_shared<Json::Value>(data);
            break;
            }
        case GlobalEvent::SoftiModelDeleteEvent:
            {
            if (
                data.isMember(GlobalEvent::EventTopic) &&
                data.isMember(GlobalEvent::FromEventSubscriptionId) &&
                data.isMember(GlobalEvent::ToEventSubscriptionId) &&
                data.isMember(GlobalEvent::iModelId) &&
                data.isMember(GlobalEvent::ProjectId)
                )
            return std::make_shared<Json::Value>(data);
            break;
            }
        case GlobalEvent::UnknownEventType:
        default:
            {
            break;
            }
        }
    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalEventPtr ParseIntoChangeSetCreatedEvent(const Utf8String jsonString)
    {
    const std::shared_ptr<Json::Value> data = EventPropertiesToJSON(jsonString, GlobalEvent::ChangeSetCreatedEvent);
    if (data == nullptr)
        return nullptr;

    return ChangeSetCreatedEvent::Create
    (
        (*data)[GlobalEvent::EventTopic].asString(),
        (*data)[GlobalEvent::FromEventSubscriptionId].asString(),
        (*data)[GlobalEvent::ToEventSubscriptionId].asString(),
        (*data)[GlobalEvent::ProjectId].asString(),
        (*data)[GlobalEvent::iModelId].asString(),
        (*data)[GlobalEvent::ChangeSetCreatedEventProperties::ChangeSetId].asString(),
        (*data)[GlobalEvent::ChangeSetCreatedEventProperties::ChangeSetIndex].asString(),
        (*data)[GlobalEvent::ChangeSetCreatedEventProperties::BriefcaseId].asInt()
    );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalEventPtr ParseIntoHardiModelDeleteEvent(const Utf8String jsonString)
    {
    const std::shared_ptr<Json::Value> data = EventPropertiesToJSON(jsonString, GlobalEvent::HardiModelDeleteEvent);
    if (data == nullptr)
        return nullptr;

    return HardiModelDeleteEvent::Create
    (
        (*data)[GlobalEvent::EventTopic].asString(),
        (*data)[GlobalEvent::FromEventSubscriptionId].asString(),
        (*data)[GlobalEvent::ToEventSubscriptionId].asString(),
        (*data)[GlobalEvent::ProjectId].asString(),
        (*data)[GlobalEvent::iModelId].asString()
    );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalEventPtr ParseIntoiModelCreatedEvent(const Utf8String jsonString)
    {
    const std::shared_ptr<Json::Value> data = EventPropertiesToJSON(jsonString, GlobalEvent::iModelCreatedEvent);
    if (data == nullptr)
        return nullptr;
    return iModelCreatedEvent::Create
    (
        (*data)[GlobalEvent::EventTopic].asString(),
        (*data)[GlobalEvent::FromEventSubscriptionId].asString(),
        (*data)[GlobalEvent::ToEventSubscriptionId].asString(),
        (*data)[GlobalEvent::ProjectId].asString(),
        (*data)[GlobalEvent::iModelId].asString()
    );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalEventPtr ParseIntoNamedVersionCreatedEvent(const Utf8String jsonString)
    {
    const std::shared_ptr<Json::Value> data = EventPropertiesToJSON(jsonString, GlobalEvent::NamedVersionCreatedEvent);
    if (data == nullptr)
        return nullptr;

    return NamedVersionCreatedEvent::Create
    (
        (*data)[GlobalEvent::EventTopic].asString(),
        (*data)[GlobalEvent::FromEventSubscriptionId].asString(),
        (*data)[GlobalEvent::ToEventSubscriptionId].asString(),
        (*data)[GlobalEvent::ProjectId].asString(),
        (*data)[GlobalEvent::iModelId].asString(),
        (*data)[GlobalEvent::NamedVersionCreatedEventProperties::VersionId].asString(),
        (*data)[GlobalEvent::NamedVersionCreatedEventProperties::VersionName].asString(),
        (*data)[GlobalEvent::NamedVersionCreatedEventProperties::ChangeSetId].asString()
    );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalEventPtr ParseIntoSoftiModelDeleteEvent(const Utf8String jsonString)
    {
    const std::shared_ptr<Json::Value> data = EventPropertiesToJSON(jsonString, GlobalEvent::SoftiModelDeleteEvent);
    if (data == nullptr)
        return nullptr;

    return SoftiModelDeleteEvent::Create
    (
        (*data)[GlobalEvent::EventTopic].asString(),
        (*data)[GlobalEvent::FromEventSubscriptionId].asString(),
        (*data)[GlobalEvent::ToEventSubscriptionId].asString(),
        (*data)[GlobalEvent::ProjectId].asString(),
        (*data)[GlobalEvent::iModelId].asString()
    );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalEventPtr GlobalEventParser::ParseEvent
(
    const Utf8CP responseContentType,
    Utf8String responseString
)
    {
    if (Utf8String::IsNullOrEmpty(responseString.c_str()) || Utf8String::IsNullOrEmpty(responseContentType))
        return nullptr;

    const size_t jsonPosStart = responseString.find_first_of('{');
    const size_t jsonPosEnd = responseString.find_last_of('}');
    if (jsonPosStart == Utf8String::npos || jsonPosEnd == Utf8String::npos)
        return nullptr;

    const Utf8String actualJsonPart = responseString.substr(jsonPosStart, jsonPosEnd);
    const GlobalEvent::GlobalEventType eventType = GlobalEvent::Helper::GetEventTypeFromEventName(responseContentType);
    switch (eventType)
        {
            case GlobalEvent::GlobalEventType::ChangeSetCreatedEvent:       return ParseIntoChangeSetCreatedEvent(actualJsonPart);
            case GlobalEvent::GlobalEventType::HardiModelDeleteEvent:       return ParseIntoHardiModelDeleteEvent(actualJsonPart);
            case GlobalEvent::GlobalEventType::iModelCreatedEvent:          return ParseIntoiModelCreatedEvent(actualJsonPart);
            case GlobalEvent::GlobalEventType::NamedVersionCreatedEvent:    return ParseIntoNamedVersionCreatedEvent(actualJsonPart);
            case GlobalEvent::GlobalEventType::SoftiModelDeleteEvent:       return ParseIntoSoftiModelDeleteEvent(actualJsonPart);
            default:
            case GlobalEvent::GlobalEventType::UnknownEventType:            return nullptr;
        }
    }
