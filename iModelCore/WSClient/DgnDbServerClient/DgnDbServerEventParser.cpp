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
#include <DgnDbServer/Client/DgnDbServerCodeEvent.h>
#include <DgnDbServer/Client/DgnDbServerDeletedEvent.h>
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
Json::Value DgnDbServerEventParser::GenerateEventSubscriptionWSChangeSetJson
(
	bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes
) const
{
	Json::Value properties;
	properties[ServerSchema::Property::EventTypes] = Json::arrayValue;
	if (eventTypes != nullptr)
	{
		bmap<DgnDbServerEvent::DgnDbServerEventType, Utf8CP> repetitiveMap;
		for (int i = 0; i < (int)eventTypes->size(); i++)
		{
			//Check for repetition
			if (repetitiveMap.end() != repetitiveMap.find(eventTypes->at(i)))
				continue;
			repetitiveMap.Insert(eventTypes->at(i), "");
			properties[ServerSchema::Property::EventTypes][i] = DgnDbServerEvent::Helper::GetEventNameFromEventType(eventTypes->at(i));
		}
	}
	return properties;
}

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Json::Value DgnDbServerEventParser::GenerateEventSubscriptionWSObjectJson
(
	bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes,
	Utf8String eventSubscriptionId
) const
{
	Json::Value request = Json::objectValue;
	JsonValueR instance = request[ServerSchema::Instance] = Json::objectValue;
	instance[ServerSchema::InstanceId] = eventSubscriptionId;
	instance[ServerSchema::SchemaName] = ServerSchema::Schema::Repository;
	instance[ServerSchema::ClassName] = ServerSchema::Class::EventSubscription;
	instance[ServerSchema::Properties] = GenerateEventSubscriptionWSChangeSetJson(eventTypes);
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
std::shared_ptr<Json::Value> GetProperties
(
	Json::Value jsonResponse,
	bool isEventSubscription,
	Utf8String *eventSubscriptionId = nullptr
)
{
	if (jsonResponse.empty() || jsonResponse.isNull())
		return nullptr;

	Json::Value changedInstance;
	if (jsonResponse.isMember(ServerSchema::ChangedInstances) && jsonResponse[ServerSchema::ChangedInstances].isArray() && !jsonResponse[ServerSchema::ChangedInstances].empty())
		changedInstance = jsonResponse[ServerSchema::ChangedInstances][0];
	else if (jsonResponse.isMember(ServerSchema::ChangedInstance))
		changedInstance = jsonResponse[ServerSchema::ChangedInstance];
	else
		return nullptr;

	if (
		!changedInstance.isMember(ServerSchema::InstanceAfterChange) ||
		!changedInstance[ServerSchema::InstanceAfterChange].isMember(ServerSchema::Properties) ||
		!changedInstance[ServerSchema::InstanceAfterChange].isMember(ServerSchema::InstanceId)
		)
		return nullptr;

	std::shared_ptr<Json::Value> properties = std::make_shared<Json::Value>(changedInstance[ServerSchema::InstanceAfterChange][ServerSchema::Properties]);
	if (properties->isNull() || properties->empty())
		return nullptr;

	if (isEventSubscription)
	{
		*eventSubscriptionId = changedInstance[ServerSchema::InstanceAfterChange][ServerSchema::InstanceId].asCString();
		if (
			!properties->isMember(ServerSchema::Property::EventTypes) ||
			!(*properties)[ServerSchema::Property::EventTypes].isArray() ||
			Utf8String::IsNullOrEmpty(eventSubscriptionId->c_str())
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
DgnDbServerEventSubscriptionPtr DgnDbServerEventParser::ParseEventSubscription
(
	Json::Value jsonResponse
) const
{
	Utf8String eventSubscriptionId = "";
	std::shared_ptr<Json::Value> propertiesPtr = GetProperties(jsonResponse, true, &eventSubscriptionId);
	if (propertiesPtr == nullptr)
		return nullptr;

	Json::Value properties = *propertiesPtr;

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

	return DgnDbServerEventSubscription::Create(eventSubscriptionId, eventTypes);
}

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSASPtr DgnDbServerEventParser::ParseEventSAS
(
	Json::Value jsonResponse
) const
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
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<Json::Value> CheckForEventProperties(Utf8String jsonString, DgnDbServerEvent::DgnDbServerEventType eventType)
{
	Json::Reader reader;
	Json::Value data(Json::objectValue);
	if (!reader.parse(jsonString, data) && !data.isArray())
		return nullptr;

	bool isSuccess = false;
	switch (eventType)
	{
	case BentleyG0601::DgnDbServer::DgnDbServerEvent::LockEvent:
	{
		if (
			data.isMember(DgnDbServerEvent::EventTopic) &&
			data.isMember(DgnDbServerEvent::FromEventSubscriptionId) &&
			data.isMember(DgnDbServerEvent::LockEventProperties::ObjectIds) && 
			data[DgnDbServerEvent::LockEventProperties::ObjectIds].isArray() &&
			data.isMember(DgnDbServerEvent::LockEventProperties::LockType) &&
			data.isMember(DgnDbServerEvent::LockEventProperties::LockLevel) &&
			data.isMember(DgnDbServerEvent::LockEventProperties::BriefcaseId) &&
			data.isMember(DgnDbServerEvent::LockEventProperties::ReleasedWithRevision)
			)
			isSuccess = true;
		break;
	}
	case BentleyG0601::DgnDbServer::DgnDbServerEvent::RevisionEvent:
	{
		if (
			data.isMember(DgnDbServerEvent::EventTopic) &&
			data.isMember(DgnDbServerEvent::FromEventSubscriptionId) &&
			data.isMember(DgnDbServerEvent::RevisionEventProperties::RevisionId) &&
			data.isMember(DgnDbServerEvent::RevisionEventProperties::RevisionIndex)
			)
			isSuccess = true;
		break;
	}
	case BentleyG0601::DgnDbServer::DgnDbServerEvent::CodeEvent:
	{
		if (
			data.isMember(DgnDbServerEvent::EventTopic) &&
			data.isMember(DgnDbServerEvent::FromEventSubscriptionId) &&
			data.isMember(DgnDbServerEvent::CodeEventProperties::CodeAuthorityId) &&
			data.isMember(DgnDbServerEvent::CodeEventProperties::Namespace) &&
			data.isMember(DgnDbServerEvent::CodeEventProperties::Values) &&
			data[DgnDbServerEvent::CodeEventProperties::Values].isArray() &&
			data.isMember(DgnDbServerEvent::CodeEventProperties::State) &&
			data.isMember(DgnDbServerEvent::CodeEventProperties::BriefcaseId) &&
			data.isMember(DgnDbServerEvent::CodeEventProperties::UsedWithRevision)
			)
			isSuccess = true;
		break;
	}
	case BentleyG0601::DgnDbServer::DgnDbServerEvent::AllLocksDeletedEvent:
	case BentleyG0601::DgnDbServer::DgnDbServerEvent::AllCodesDeletedEvent:
	{
		if (
			data.isMember(DgnDbServerEvent::EventTopic) &&
			data.isMember(DgnDbServerEvent::FromEventSubscriptionId) &&
			data.isMember(DgnDbServerEvent::DeletedEventProperties::BriefcaseId) &&
			!CheckForEventProperties(jsonString, DgnDbServerEvent::LockEvent) &&
			!CheckForEventProperties(jsonString, DgnDbServerEvent::CodeEvent)
			)
			isSuccess = true;
		break;
	}
	case BentleyG0601::DgnDbServer::DgnDbServerEvent::UnknownEventType:
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
DgnDbServerEventPtr ParseIntoLockEvent(Utf8String jsonString)
{
	std::shared_ptr<Json::Value> data = CheckForEventProperties(jsonString, DgnDbServerEvent::LockEvent);
	if (data == nullptr)
		return nullptr;

	bvector<Utf8String> objectIds;
	for
		(
		Json::ValueIterator itr = (*data)[DgnDbServerEvent::LockEventProperties::ObjectIds].begin();
		itr != (*data)[DgnDbServerEvent::LockEventProperties::ObjectIds].end();
		itr++
		)
		objectIds.push_back((*itr).asString());

	if (objectIds.size() < 1)
		return nullptr;

	return DgnDbServerLockEvent::Create
	(
		(*data)[DgnDbServerEvent::EventTopic].asString(),
		(*data)[DgnDbServerEvent::FromEventSubscriptionId].asString(),
		objectIds,
		(*data)[DgnDbServerEvent::LockEventProperties::LockType].asString(),
		(*data)[DgnDbServerEvent::LockEventProperties::LockLevel].asString(),
		(*data)[DgnDbServerEvent::LockEventProperties::BriefcaseId].asString(),
		(*data)[DgnDbServerEvent::LockEventProperties::ReleasedWithRevision].asString()
	);
}

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              07/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventPtr ParseIntoRevisionEvent(Utf8String jsonString)
{
	std::shared_ptr<Json::Value> data = CheckForEventProperties(jsonString, DgnDbServerEvent::RevisionEvent);
	if (data == nullptr)
		return nullptr;
	return DgnDbServerRevisionEvent::Create
	(
		(*data)[DgnDbServerEvent::EventTopic].asString(),
		(*data)[DgnDbServerEvent::FromEventSubscriptionId].asString(),
		(*data)[DgnDbServerEvent::RevisionEventProperties::RevisionId].asString(),
		(*data)[DgnDbServerEvent::RevisionEventProperties::RevisionIndex].asString()
	);
}

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              07/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventPtr ParseIntoCodeEvent(Utf8String jsonString)
{
	std::shared_ptr<Json::Value> data = CheckForEventProperties(jsonString, DgnDbServerEvent::CodeEvent);
	if (data == nullptr)
		return nullptr;

	bvector<Utf8String> values;
	for 
		(
		Json::ValueIterator itr = (*data)[DgnDbServerEvent::CodeEventProperties::Values].begin(); 
		itr != (*data)[DgnDbServerEvent::CodeEventProperties::Values].end(); 
		itr++
		)
		values.push_back((*itr).asString());

	if (values.size() < 1)
		return nullptr;

	return DgnDbServerCodeEvent::Create
	(
		(*data)[DgnDbServerEvent::EventTopic].asString(),
		(*data)[DgnDbServerEvent::FromEventSubscriptionId].asString(),
		(*data)[DgnDbServerEvent::CodeEventProperties::CodeAuthorityId].asString(),
		(*data)[DgnDbServerEvent::CodeEventProperties::Namespace].asString(),
		values,
		(*data)[DgnDbServerEvent::CodeEventProperties::State].asString(),
		(*data)[DgnDbServerEvent::CodeEventProperties::BriefcaseId].asString(),
		(*data)[DgnDbServerEvent::CodeEventProperties::UsedWithRevision].asString()
	);
}

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              07/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventPtr ParseIntoDeletedEvent(Utf8String jsonString, DgnDbServerEvent::DgnDbServerEventType deletedEventType)
{
	std::shared_ptr<Json::Value> data = CheckForEventProperties(jsonString, deletedEventType);
	if (data == nullptr)
		return nullptr;
	return DgnDbServerDeletedEvent::Create
	(
		(*data)[DgnDbServerEvent::EventTopic].asString(),
		(*data)[DgnDbServerEvent::FromEventSubscriptionId].asString(),
		(*data)[DgnDbServerEvent::DeletedEventProperties::BriefcaseId].asString(),
		deletedEventType
	);
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
	DgnDbServerEvent::DgnDbServerEventType eventType = DgnDbServerEvent::Helper::GetEventTypeFromEventName(responseContentType);
	switch (eventType)
	{
	case DgnDbServerEvent::DgnDbServerEventType::LockEvent:             return ParseIntoLockEvent(actualJsonPart);
	case DgnDbServerEvent::DgnDbServerEventType::RevisionEvent:         return ParseIntoRevisionEvent(actualJsonPart);
	case DgnDbServerEvent::DgnDbServerEventType::CodeEvent:             return ParseIntoCodeEvent(actualJsonPart);
	case DgnDbServerEvent::DgnDbServerEventType::AllLocksDeletedEvent:
	case DgnDbServerEvent::DgnDbServerEventType::AllCodesDeletedEvent:  return ParseIntoDeletedEvent(actualJsonPart, eventType);
	default:
	case DgnDbServerEvent::DgnDbServerEventType::UnknownEventType: return nullptr;
	}
}

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<struct DgnDbServerLockEvent> DgnDbServerEventParser::GetLockEvent(DgnDbServerEventPtr eventPtr)
{
	try
	{
		if (eventPtr == nullptr)
			return nullptr;
		return std::make_shared<DgnDbServerLockEvent>(dynamic_cast<DgnDbServerLockEvent&>(*eventPtr));
	}
	catch (const std::bad_cast&)
	{
		return nullptr;
	}
}

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<struct DgnDbServerRevisionEvent> DgnDbServerEventParser::GetRevisionEvent(DgnDbServerEventPtr eventPtr)
{
	try
	{
		if (eventPtr == nullptr)
			return nullptr;
		return std::make_shared<DgnDbServerRevisionEvent>(dynamic_cast<DgnDbServerRevisionEvent&>(*eventPtr));
	}
	catch (const std::bad_cast&)
	{
		return nullptr;
	}
}

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<struct DgnDbServerCodeEvent> DgnDbServerEventParser::GetCodeEvent(DgnDbServerEventPtr eventPtr)
{
	try
	{
		if (eventPtr == nullptr)
			return nullptr;
		return std::make_shared<DgnDbServerCodeEvent>(dynamic_cast<DgnDbServerCodeEvent&>(*eventPtr));
	}
	catch (const std::bad_cast&)
	{
		return nullptr;
	}
}

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<struct DgnDbServerDeletedEvent> DgnDbServerEventParser::GetDeletedEvent(DgnDbServerEventPtr eventPtr)
{
	try
	{
		if (eventPtr == nullptr)
			return nullptr;
		return std::make_shared<DgnDbServerDeletedEvent>(dynamic_cast<DgnDbServerDeletedEvent&>(*eventPtr));
	}
	catch (const std::bad_cast&)
	{
		return nullptr;
	}
}
