/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Events/EventParser.h>
#include <WebServices/iModelHub/Events/LockEvent.h>
#include <WebServices/iModelHub/Events/ChangeSetPostPushEvent.h>
#include <WebServices/iModelHub/Events/CodeEvent.h>
#include <WebServices/iModelHub/Events/DeletedEvent.h>
#include <WebServices/iModelHub/Events/VersionEvent.h>
#include "../Utils.h"
#include <WebServices/iModelHub/Events/ChangeSetPrePushEvent.h>

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<Json::Value> EventPropertiesToJSON(Utf8String jsonString, Event::EventType eventType)
    {
    Json::Reader reader;
    Json::Value data(Json::objectValue);
    if (!reader.parse(jsonString, data) && !data.isArray())
        return nullptr;

    bool isSuccess = false;
    switch (eventType)
        {
        case BentleyM0200::iModel::Hub::Event::LockEvent:
            {
            if (
                data.isMember(Event::EventTopic) &&
                data.isMember(Event::FromEventSubscriptionId) &&
                data.isMember(Event::LockEventProperties::ObjectIds) &&
                data[Event::LockEventProperties::ObjectIds].isArray() &&
                data.isMember(Event::LockEventProperties::LockType) &&
                data.isMember(Event::LockEventProperties::LockLevel) &&
                data.isMember(Event::LockEventProperties::BriefcaseId) &&
                data.isMember(Event::LockEventProperties::ReleasedWithChangeSet)
                )
                isSuccess = true;
            break;
            }
        case BentleyM0200::iModel::Hub::Event::ChangeSetPostPushEvent:
            {
            if (
                data.isMember(Event::EventTopic) &&
                data.isMember(Event::FromEventSubscriptionId) &&
                data.isMember(Event::ChangeSetPostPushEventProperties::ChangeSetId) &&
                data.isMember(Event::ChangeSetPostPushEventProperties::ChangeSetIndex) &&
                data.isMember(Event::ChangeSetPostPushEventProperties::BriefcaseId)
                )
                isSuccess = true;
            break;
            }
        case BentleyM0200::iModel::Hub::Event::ChangeSetPrePushEvent:
            {
            if (
                data.isMember(Event::EventTopic) &&
                data.isMember(Event::FromEventSubscriptionId)
                )
                isSuccess = true;
            break;
            }
        case BentleyM0200::iModel::Hub::Event::CodeEvent:
            {
            if (
                data.isMember(Event::EventTopic) &&
                data.isMember(Event::FromEventSubscriptionId) &&
                data.isMember(Event::CodeEventProperties::CodeSpecId) &&
                data.isMember(Event::CodeEventProperties::CodeScope) &&
                data.isMember(Event::CodeEventProperties::Values) &&
                data[Event::CodeEventProperties::Values].isArray() &&
                data.isMember(Event::CodeEventProperties::State) &&
                data.isMember(Event::CodeEventProperties::BriefcaseId)
                )
                isSuccess = true;
            break;
            }
        case BentleyM0200::iModel::Hub::Event::AllLocksDeletedEvent:
        case BentleyM0200::iModel::Hub::Event::AllCodesDeletedEvent:
            {
            if (
                data.isMember(Event::EventTopic) &&
                data.isMember(Event::FromEventSubscriptionId) &&
                data.isMember(Event::DeletedEventProperties::BriefcaseId) &&
                !EventPropertiesToJSON(jsonString, Event::LockEvent) &&
                !EventPropertiesToJSON(jsonString, Event::CodeEvent) &&
                !EventPropertiesToJSON(jsonString, Event::ChangeSetPostPushEvent)
                )
                isSuccess = true;
            break;
            }
        case BentleyM0200::iModel::Hub::Event::VersionEvent:
        case BentleyM0200::iModel::Hub::Event::VersionModifiedEvent:
            {
            if (
                data.isMember(Event::EventTopic) &&
                data.isMember(Event::FromEventSubscriptionId) &&
                data.isMember(Event::VersionEventProperties::VersionId) &&
                data.isMember(Event::VersionEventProperties::VersionName) &&
                data.isMember(Event::VersionEventProperties::ChangeSetId)
                )
                isSuccess = true;
            break;
            }
        case BentleyM0200::iModel::Hub::Event::UnknownEventType:
        default:
            {
            break;
            }
        }
    return (isSuccess) ? std::make_shared<Json::Value>(data) : nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              07/2016
//---------------------------------------------------------------------------------------
EventPtr ParseIntoLockEvent(Utf8String jsonString)
    {
	std::shared_ptr<Json::Value> data = EventPropertiesToJSON(jsonString, Event::LockEvent);
	if (data == nullptr)
		return nullptr;

	bvector<Utf8String> objectIds;
	for
		(
		Json::ValueIterator itr = (*data)[Event::LockEventProperties::ObjectIds].begin();
		itr != (*data)[Event::LockEventProperties::ObjectIds].end();
		itr++
		)
		objectIds.push_back((*itr).asString());

	if (objectIds.size() < 1)
		return nullptr;

	return LockEvent::Create
	    (
		(*data)[Event::EventTopic].asString(),
		(*data)[Event::FromEventSubscriptionId].asString(),
		objectIds,
		(*data)[Event::LockEventProperties::LockType].asString(),
		(*data)[Event::LockEventProperties::LockLevel].asString(),
		(*data)[Event::LockEventProperties::BriefcaseId].asInt(),
		(*data)[Event::LockEventProperties::ReleasedWithChangeSet].asString()
	    );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              07/2016
//---------------------------------------------------------------------------------------
EventPtr ParseIntoChangeSetPostPushEvent(Utf8String jsonString)
    {
	std::shared_ptr<Json::Value> data = EventPropertiesToJSON(jsonString, Event::ChangeSetPostPushEvent);
	if (data == nullptr)
		return nullptr;
	return ChangeSetPostPushEvent::Create
	    (
		(*data)[Event::EventTopic].asString(),
		(*data)[Event::FromEventSubscriptionId].asString(),
		(*data)[Event::ChangeSetPostPushEventProperties::ChangeSetId].asString(),
		(*data)[Event::ChangeSetPostPushEventProperties::ChangeSetIndex].asString(),
	    (*data)[Event::ChangeSetPostPushEventProperties::BriefcaseId].asInt()
	    );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Algirdas.Mikoliunas	              12/2016
//---------------------------------------------------------------------------------------
EventPtr ParseIntoChangeSetPrePushEvent(Utf8String jsonString)
    {
    std::shared_ptr<Json::Value> data = EventPropertiesToJSON(jsonString, Event::ChangeSetPrePushEvent);
    if (data == nullptr)
        return nullptr;
    return ChangeSetPrePushEvent::Create
        (
        (*data)[Event::EventTopic].asString(),
        (*data)[Event::FromEventSubscriptionId].asString()
        );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              07/2016
//---------------------------------------------------------------------------------------
EventPtr ParseIntoCodeEvent(Utf8String jsonString)
    {
	std::shared_ptr<Json::Value> data = EventPropertiesToJSON(jsonString, Event::CodeEvent);
	if (data == nullptr)
		return nullptr;

	bvector<Utf8String> values;
	for 
		(
		Json::ValueIterator itr = (*data)[Event::CodeEventProperties::Values].begin(); 
		itr != (*data)[Event::CodeEventProperties::Values].end(); 
		itr++
		)
		values.push_back((*itr).asString());

	if (values.size() < 1)
		return nullptr;

	return CodeEvent::Create
	    (
		(*data)[Event::EventTopic].asString(),
		(*data)[Event::FromEventSubscriptionId].asString(),
		(*data)[Event::CodeEventProperties::CodeSpecId].asString(),
		(*data)[Event::CodeEventProperties::CodeScope].asString(),
		values,
		(*data)[Event::CodeEventProperties::State].asInt(),
		(*data)[Event::CodeEventProperties::BriefcaseId].asInt()
	    );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              07/2016
//---------------------------------------------------------------------------------------
EventPtr ParseIntoDeletedEvent(Utf8String jsonString, Event::EventType deletedEventType)
    {
	std::shared_ptr<Json::Value> data = EventPropertiesToJSON(jsonString, deletedEventType);
	if (data == nullptr)
		return nullptr;
	return DeletedEvent::Create
	    (
		(*data)[Event::EventTopic].asString(),
		(*data)[Event::FromEventSubscriptionId].asString(),
		(*data)[Event::DeletedEventProperties::BriefcaseId].asInt(),
		deletedEventType
	    );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Viktorija.Adomauskaite               06/2016
//---------------------------------------------------------------------------------------
EventPtr ParseIntoVersionEvent(Utf8String jsonString, Event::EventType versionEventType)
    {
    std::shared_ptr < Json::Value> data = EventPropertiesToJSON(jsonString, versionEventType);
    if (data == nullptr)
        return nullptr;
    return VersionEvent::Create
        (
        (*data)[Event::EventTopic].asString(),
        (*data)[Event::FromEventSubscriptionId].asString(),
        (*data)[Event::VersionEventProperties::VersionId].asString(),
        (*data)[Event::VersionEventProperties::VersionName].asString(),
        (*data)[Event::VersionEventProperties::ChangeSetId].asString(),
        versionEventType
        );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
EventPtr EventParser::ParseEvent
(
Utf8CP responseContentType,
Utf8String responseString
)
    {
	if (Utf8String::IsNullOrEmpty(responseString.c_str()) || Utf8String::IsNullOrEmpty(responseContentType))
		return nullptr;

	size_t jsonPosStart = responseString.find_first_of('{');
	size_t jsonPosEnd = responseString.find_last_of('}');
	if (jsonPosStart == Utf8String::npos || jsonPosEnd == Utf8String::npos)
		return nullptr;

	Utf8String actualJsonPart = responseString.substr(jsonPosStart, jsonPosEnd);
	Event::EventType eventType = Event::Helper::GetEventTypeFromEventName(responseContentType);
	switch (eventType)
	    {
        case Event::EventType::LockEvent:                   return ParseIntoLockEvent(actualJsonPart);
        case Event::EventType::ChangeSetPostPushEvent:      return ParseIntoChangeSetPostPushEvent(actualJsonPart);
        case Event::EventType::ChangeSetPrePushEvent:       return ParseIntoChangeSetPrePushEvent(actualJsonPart);
	    case Event::EventType::CodeEvent:                   return ParseIntoCodeEvent(actualJsonPart);
	    case Event::EventType::AllLocksDeletedEvent:
	    case Event::EventType::AllCodesDeletedEvent:        return ParseIntoDeletedEvent(actualJsonPart, eventType);
        case Event::EventType::VersionEvent:
        case Event::EventType::VersionModifiedEvent:        return ParseIntoVersionEvent(actualJsonPart, eventType);
	    default:
	    case Event::EventType::UnknownEventType:            return nullptr;
	    }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct LockEvent> EventParser::GetLockEvent(EventPtr eventPtr)
    {
	try
	    {
		if (eventPtr == nullptr)
			return nullptr;
		return new LockEvent(dynamic_cast<LockEvent&>(*eventPtr));
	    }
	catch (const std::bad_cast&)
	    {
		return nullptr;
	    }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct ChangeSetPostPushEvent> EventParser::GetChangeSetPostPushEvent(EventPtr eventPtr)
    {
	try
	    {
		if (eventPtr == nullptr)
			return nullptr;
		return new ChangeSetPostPushEvent(dynamic_cast<ChangeSetPostPushEvent&>(*eventPtr));
	    }
	catch (const std::bad_cast&)
	    {
		return nullptr;
	    }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Algirdas.Mikoliunas	              12/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct ChangeSetPrePushEvent> EventParser::GetChangeSetPrePushEvent(EventPtr eventPtr)
    {
    try
        {
        if (eventPtr == nullptr)
            return nullptr;
        return new ChangeSetPrePushEvent(dynamic_cast<ChangeSetPrePushEvent&>(*eventPtr));
        }
    catch (const std::bad_cast&)
        {
        return nullptr;
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct CodeEvent> EventParser::GetCodeEvent(EventPtr eventPtr)
    {
	try
	    {
		if (eventPtr == nullptr)
			return nullptr;
		return new CodeEvent(dynamic_cast<CodeEvent&>(*eventPtr));
	    }
	catch (const std::bad_cast&)
	    {
		return nullptr;
	    }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct DeletedEvent> EventParser::GetDeletedEvent(EventPtr eventPtr)
    {
	try
	    {
		if (eventPtr == nullptr)
			return nullptr;
		return new DeletedEvent(dynamic_cast<DeletedEvent&>(*eventPtr));
	    }
	catch (const std::bad_cast&)
	    {
		return nullptr;
	    }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Viktorija.Adomauskaite               06/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct VersionEvent> EventParser::GetVersionEvent(EventPtr eventPtr)
    {
    try
        {
        if (eventPtr == nullptr)
            return nullptr;
        return new VersionEvent(dynamic_cast<VersionEvent&>(*eventPtr));
        }
    catch (const std::bad_cast&)
        {
        return nullptr;
        }
    }
