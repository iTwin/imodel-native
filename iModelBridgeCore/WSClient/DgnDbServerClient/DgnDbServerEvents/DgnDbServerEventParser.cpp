/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEvents/DgnDbServerEventParser.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/Events/DgnDbServerEventParser.h>
#include <DgnDbServer/Client/Events/DgnDbServerLockEvent.h>
#include <DgnDbServer/Client/Events/DgnDbServerRevisionEvent.h>
#include <DgnDbServer/Client/Events/DgnDbServerCodeEvent.h>
#include <DgnDbServer/Client/Events/DgnDbServerDeletedEvent.h>
#include "../DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

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
			    data.isMember(DgnDbServerEvent::CodeEventProperties::Reserved) &&
			    data.isMember(DgnDbServerEvent::CodeEventProperties::Used) &&
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
		(*data)[DgnDbServerEvent::CodeEventProperties::Reserved].asString(),
		(*data)[DgnDbServerEvent::CodeEventProperties::Used].asString(),
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
)
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
