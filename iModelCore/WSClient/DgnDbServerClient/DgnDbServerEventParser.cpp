/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEventParser.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerEventParser.h>
#include <DgnDbServer/Client/DgnDbServerLockevent.h>
#include <DgnDbServer/Client/DgnDbServerRevisionEvent.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventParser::DgnDbServerEventParser() {}

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventParser& DgnDbServerEventParser::GetInstance()
    {
    static DgnDbServerEventParser instance; // Guaranteed to be destroyed. Instantiated on first use.
    return instance;
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Json::Value DgnDbServerEventParser::GenerateEventSubscriptionJson(bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes, Utf8String eventSubscriptionId) const
    {
    Json::Value request = Json::objectValue;
    JsonValueR instance = request[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::InstanceId] = "";
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::Repository;
    instance[ServerSchema::ClassName] = ServerSchema::Class::EventSubscription;
    instance[ServerSchema::Properties] = Json::objectValue;
    JsonValueR properties = instance[ServerSchema::Properties];
    properties[ServerSchema::Property::Id] = eventSubscriptionId;
    properties[ServerSchema::Property::TopicName] = "";
    properties[ServerSchema::Property::EventTypes] = Json::arrayValue;
    if (eventTypes != nullptr)
        {
        bmap<DgnDbServerEvent::DgnDbServerEventType, Utf8CP> repetitiveMap;
        for (int i = 0; i < (int) eventTypes->size(); i++)
            {
            //Check for repetition
            if (repetitiveMap.end() != repetitiveMap.find(eventTypes->at(i)))
                continue;
            repetitiveMap.Insert(eventTypes->at(i), "");
            properties[ServerSchema::Property::EventTypes][i] = DgnDbServerEvent::Helper::GetEventNameFromEventType(eventTypes->at(i));
            }
        }
    return request;
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Json::Value DgnDbServerEventParser::GenerateEventSASJson() const
    {
    Json::Value request = Json::objectValue;
    JsonValueR instance = request[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::InstanceId] = "";
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::Repository;
    instance[ServerSchema::ClassName] = ServerSchema::Class::EventSAS;
    instance[ServerSchema::Properties] = Json::objectValue;
    JsonValueR properties = instance[ServerSchema::Properties];
    properties[ServerSchema::Property::BaseAddress] = "";
    properties[ServerSchema::Property::EventServiceSASToken] = "";
    return request;
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<Json::Value> GetProperties(Json::Value jsonResponse, bool isEventSubscription)
    {
    if (jsonResponse.empty() || jsonResponse.isNull())
        return nullptr;
    
    if (
        !jsonResponse.isMember(ServerSchema::ChangedInstance) ||
        !jsonResponse[ServerSchema::ChangedInstance].isMember(ServerSchema::InstanceAfterChange) ||
        !jsonResponse[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange].isMember(ServerSchema::Properties)
        )
        return nullptr;

    std::shared_ptr<Json::Value> properties = std::make_shared<Json::Value>(jsonResponse[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::Properties]);

    if (isEventSubscription)
        {
        if (
            !properties->isMember(ServerSchema::Property::Id) ||
            !properties->isMember(ServerSchema::Property::TopicName) ||
            !(*properties)[ServerSchema::Property::EventTypes].isArray()
            )
            return nullptr;
        }
    else
        {
        if (
            !properties->isMember(ServerSchema::Property::BaseAddress) ||
            !properties->isMember(ServerSchema::Property::EventServiceSASToken)
            )
            return nullptr;
        }

    return properties;
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSubscriptionPtr DgnDbServerEventParser::ParseEventSubscription(Json::Value jsonResponse) const
    {
    std::shared_ptr<Json::Value> propertiesPtr = GetProperties(jsonResponse, true);
    if (propertiesPtr == nullptr)
        return nullptr;

    Json::Value properties = *propertiesPtr;
    Utf8String topicName = properties[ServerSchema::Property::TopicName].asCString();
    Utf8String id = properties[ServerSchema::Property::Id].asCString();
    if (Utf8String::IsNullOrEmpty(topicName.c_str()) || Utf8String::IsNullOrEmpty(id.c_str()))
        return nullptr;

    bvector<DgnDbServerEvent::DgnDbServerEventType> eventTypes;
    Json::Value::ArrayIndex size = properties[ServerSchema::Property::EventTypes].size();
    if (size >= 1)
        {
        for (Json::Value::ArrayIndex i = 0; i < size; i++)
            {
            eventTypes.push_back(DgnDbServerEvent::Helper::GetEventTypeFromEventName
                                                              (
                                                              properties[ServerSchema::Property::EventTypes][i].asCString())
                                                              );
            }
        }

    return DgnDbServerEventSubscription::Create(topicName, id, eventTypes);
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSASPtr DgnDbServerEventParser::ParseEventSAS(Json::Value jsonResponse) const
    {
    std::shared_ptr<Json::Value> propertiesPtr = GetProperties(jsonResponse, false);
    if (propertiesPtr == nullptr)
        return nullptr;
        
    Json::Value properties = *propertiesPtr;
    Utf8String sasToken = properties[ServerSchema::Property::EventServiceSASToken].asCString();
    Utf8String baseAddress = properties[ServerSchema::Property::BaseAddress].asCString();
    if (Utf8String::IsNullOrEmpty(sasToken.c_str()) || Utf8String::IsNullOrEmpty(baseAddress.c_str()))
        return nullptr;
        
    return DgnDbServerEventSAS::Create(sasToken, baseAddress);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventPtr DgnDbServerEventParser::ParseEvent
(
Utf8CP responseContentType,
Utf8String responseString
) const
    {
    if (Utf8String::IsNullOrEmpty(responseString.c_str()) || Utf8String::IsNullOrEmpty(responseContentType))
        return nullptr;

    size_t jsonPosStart = responseString.find_first_of('{');
    size_t jsonPosEnd = responseString.find_last_of('}');
    if (jsonPosStart == Utf8String::npos || jsonPosEnd == Utf8String::npos)
        return nullptr;

    Utf8String actualJsonPart = responseString.substr(jsonPosStart, jsonPosEnd);
    if (0 == (BeStringUtilities::Stricmp
        (
        DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::LockEvent).c_str(), 
        responseContentType
        )))
        {
        Json::Reader reader;
        Json::Value data(Json::objectValue);
        if (
            reader.parse(actualJsonPart, data) &&
            !data.isArray() &&
            data.isMember(DgnDbServerEvent::EventTopic) &&
            data.isMember(DgnDbServerEvent::UserId) &&
            data.isMember(DgnDbServerEvent::LockEventProperties::ObjectId) &&
            data.isMember(DgnDbServerEvent::LockEventProperties::LockType) &&
            data.isMember(DgnDbServerEvent::LockEventProperties::LockLevel) &&
            data.isMember(DgnDbServerEvent::LockEventProperties::BriefcaseId) &&
            data.isMember(DgnDbServerEvent::LockEventProperties::ReleasedWithRevision) &&
            data.isMember(DgnDbServerEvent::LockEventProperties::Date)
            )
            return DgnDbServerLockEvent::Create
                                               (
                                                data[DgnDbServerEvent::EventTopic].asString(),
                                                data[DgnDbServerEvent::UserId].asString(),
                                                data[DgnDbServerEvent::LockEventProperties::ObjectId].asString(),
                                                data[DgnDbServerEvent::LockEventProperties::LockType].asString(),
                                                data[DgnDbServerEvent::LockEventProperties::LockLevel].asString(),
                                                data[DgnDbServerEvent::LockEventProperties::BriefcaseId].asString(),
                                                data[DgnDbServerEvent::LockEventProperties::ReleasedWithRevision].asString(),
                                                data[DgnDbServerEvent::LockEventProperties::Date].asString()
                                               );
        return nullptr;
        }
    else if (0 == (BeStringUtilities::Stricmp
        (
        DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent).c_str(), 
        responseContentType
        )))
        {
        Json::Reader reader;
        Json::Value data(Json::objectValue);
        if (
            reader.parse(actualJsonPart, data) &&
            !data.isArray() &&
            data.isMember(DgnDbServerEvent::EventTopic) &&
            data.isMember(DgnDbServerEvent::UserId) &&
            data.isMember(DgnDbServerEvent::RevisionEventProperties::RevisionId) &&
            data.isMember(DgnDbServerEvent::RevisionEventProperties::RevisionIndex) &&
            data.isMember(DgnDbServerEvent::RevisionEventProperties::Date)
            )
            return DgnDbServerRevisionEvent::Create
                                                   (
                                                    data[DgnDbServerEvent::EventTopic].asString(),
                                                    data[DgnDbServerEvent::UserId].asString(),
                                                    data[DgnDbServerEvent::RevisionEventProperties::RevisionId].asString(),
                                                    data[DgnDbServerEvent::RevisionEventProperties::RevisionIndex].asString(),
                                                    data[DgnDbServerEvent::RevisionEventProperties::Date].asString()
                                                   );
        return nullptr;
        }
    else
        return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEvent::DgnDbServerEventType DgnDbServerEventParser::GetEventType
(
DgnDbServerEventPtr eventPtr
) const
    {
    if (eventPtr == nullptr)
        return DgnDbServerEvent::DgnDbServerEventType::UnknownEventType;

    const type_info& eventType = eventPtr->GetEventType();
    Utf8String returnedEventName = eventType.name();
    if (returnedEventName.ContainsI(typeid(DgnDbServerLockEvent).name()))
        return DgnDbServerEvent::DgnDbServerEventType::LockEvent;
    else if (returnedEventName.ContainsI(typeid(DgnDbServerRevisionEvent).name()))
        return DgnDbServerEvent::DgnDbServerEventType::RevisionEvent;
    else
        return DgnDbServerEvent::DgnDbServerEventType::UnknownEventType;
    }
