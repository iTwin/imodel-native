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
bool StringHelper(bmap<Utf8String, Utf8String>& entitymap, Utf8String original)
    {
    bmap<Utf8String, Utf8String>::iterator data = entitymap.begin();
    while (data != entitymap.end())
        {
        Utf8String entity = data->first;
        size_t entityPos = (0 == (BeStringUtilities::Stricmp("Date", entity.c_str()))) ? original.find_first_of(entity.c_str()) : original.rfind(entity.c_str());

        if (Utf8String::npos == entityPos)
            return false;
        Utf8String sub1 = original.substr(entityPos, original.length());
        size_t entityValueStart = sub1.find_first_of(':');
        Utf8String sub2 = sub1.substr(entityValueStart + 2, sub1.length());
        size_t entityValueStop = sub2.find_first_of('"');
        Utf8String entityValue = sub2.substr(0, entityValueStop);
        data->second = entityValue;
        ++data;
        }
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
IDgnDbServerEventPtr DgnDbServerEventParser::ParseEventasJson
(
Utf8CP responseContentType,
Utf8String responseString
) const
    {
    size_t jsonPosStart = responseString.find_first_of('{');
    size_t jsonPosEnd = responseString.find_last_of('}');
    Utf8String actualJsonPart = responseString.substr(jsonPosStart, jsonPosEnd);
    if (0 == (BeStringUtilities::Stricmp(IDgnDbServerEvent::GetEventName(DgnDbServerEventType::LockEvent).c_str(), responseContentType)))
        {
        Json::Reader reader;
        Json::Value data(Json::objectValue);
        if (
            reader.parse(actualJsonPart, data) &&
            !data.isArray() &&
            data.isMember(DgnDbServerEvent::RepoId) &&
            data.isMember(DgnDbServerEvent::UserId) &&
            data.isMember(DgnDbServerEvent::LockEvent::ObjectId) &&
            data.isMember(DgnDbServerEvent::LockEvent::LockType) &&
            data.isMember(DgnDbServerEvent::LockEvent::LockLevel) &&
            data.isMember(DgnDbServerEvent::LockEvent::BriefcaseId) &&
            data.isMember(DgnDbServerEvent::LockEvent::ReleasedWithRevision) &&
            data.isMember(DgnDbServerEvent::LockEvent::Date)
            )
            return DgnDbServerLockEvent::Create
            (
            data[DgnDbServerEvent::RepoId].asString(),
            data[DgnDbServerEvent::UserId].asString(),
            data[DgnDbServerEvent::LockEvent::ObjectId].asString(),
            data[DgnDbServerEvent::LockEvent::LockType].asString(),
            data[DgnDbServerEvent::LockEvent::LockLevel].asString(),
            data[DgnDbServerEvent::LockEvent::BriefcaseId].asString(),
            data[DgnDbServerEvent::LockEvent::ReleasedWithRevision].asString(),
            data[DgnDbServerEvent::LockEvent::Date].asString()
            );
        return nullptr;
        }
    else if (0 == (BeStringUtilities::Stricmp(IDgnDbServerEvent::GetEventName(DgnDbServerEventType::RevisionEvent).c_str(), responseContentType)))
        {
        Json::Reader reader;
        Json::Value data(Json::objectValue);
        if (
            reader.parse(actualJsonPart, data) &&
            !data.isArray() &&
            data.isMember(DgnDbServerEvent::RepoId) &&
            data.isMember(DgnDbServerEvent::UserId) &&
            data.isMember(DgnDbServerEvent::RevisionEvent::RevisionId) &&
            data.isMember(DgnDbServerEvent::RevisionEvent::RevisionIndex) &&
            data.isMember(DgnDbServerEvent::RevisionEvent::Date)
            )
            return DgnDbServerRevisionEvent::Create
            (
            data[DgnDbServerEvent::RepoId].asString(),
            data[DgnDbServerEvent::UserId].asString(),
            data[DgnDbServerEvent::RevisionEvent::RevisionId].asString(),
            data[DgnDbServerEvent::RevisionEvent::RevisionIndex].asString(),
            data[DgnDbServerEvent::RevisionEvent::Date].asString()
            );
        return nullptr;
        }
    else
        return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
IDgnDbServerEventPtr DgnDbServerEventParser::ParseEventasString
(
Utf8CP responseContentType,
Utf8String responseString
) const
    {
    size_t jsonPart1 = responseString.find_first_of('{');
    size_t jsonPart2 = responseString.find_last_of('}');
    Utf8String actualJsonPart = responseString.substr(jsonPart1, jsonPart2);
    if (0 == (BeStringUtilities::Stricmp(IDgnDbServerEvent::GetEventName(DgnDbServerEventType::LockEvent).c_str(), responseContentType)))
        {
        bmap<Utf8String, Utf8String> entitymap;
        entitymap.Insert(DgnDbServerEvent::RepoId, "");
        entitymap.Insert(DgnDbServerEvent::UserId, "");
        entitymap.Insert(DgnDbServerEvent::LockEvent::ObjectId, "");
        entitymap.Insert(DgnDbServerEvent::LockEvent::LockType, "");
        entitymap.Insert(DgnDbServerEvent::LockEvent::LockLevel, "");
        entitymap.Insert(DgnDbServerEvent::LockEvent::BriefcaseId, "");
        entitymap.Insert(DgnDbServerEvent::LockEvent::ReleasedWithRevision, "");
        entitymap.Insert(DgnDbServerEvent::LockEvent::Date, "");

        if (!StringHelper(entitymap, actualJsonPart))
            return nullptr;
        return DgnDbServerLockEvent::Create
            (
            entitymap[DgnDbServerEvent::RepoId],
            entitymap[DgnDbServerEvent::UserId],
            entitymap[DgnDbServerEvent::LockEvent::ObjectId],
            entitymap[DgnDbServerEvent::LockEvent::LockType],
            entitymap[DgnDbServerEvent::LockEvent::LockLevel],
            entitymap[DgnDbServerEvent::LockEvent::BriefcaseId],
            entitymap[DgnDbServerEvent::LockEvent::ReleasedWithRevision],
            entitymap[DgnDbServerEvent::LockEvent::Date]
            );
        }
    else if (0 == (BeStringUtilities::Stricmp(IDgnDbServerEvent::GetEventName(DgnDbServerEventType::RevisionEvent).c_str(), responseContentType)))
        {
        bmap<Utf8String, Utf8String> entitymap;
        entitymap.Insert(DgnDbServerEvent::RepoId, "");
        entitymap.Insert(DgnDbServerEvent::UserId, "");
        entitymap.Insert(DgnDbServerEvent::RevisionEvent::RevisionId, "");
        entitymap.Insert(DgnDbServerEvent::RevisionEvent::RevisionIndex, "");
        entitymap.Insert(DgnDbServerEvent::RevisionEvent::Date, "");

        if (!StringHelper(entitymap, actualJsonPart))
            return nullptr;
        return DgnDbServerRevisionEvent::Create
            (
            entitymap[DgnDbServerEvent::RepoId],
            entitymap[DgnDbServerEvent::UserId],
            entitymap[DgnDbServerEvent::RevisionEvent::RevisionId],
            entitymap[DgnDbServerEvent::RevisionEvent::RevisionIndex],
            entitymap[DgnDbServerEvent::RevisionEvent::Date]
            );
        }
    else
        {
        return nullptr;
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventType DgnDbServerEventParser::GetEventType
(
IDgnDbServerEventPtr eventPtr
) const
    {
    const type_info& eventType = eventPtr->GetEventType();
    Utf8String returnedEventName = eventType.name();
    if (returnedEventName.ContainsI(typeid(DgnDbServerLockEvent).name()))
        return DgnDbServerEventType::LockEvent;
    else if (returnedEventName.ContainsI(typeid(DgnDbServerRevisionEvent).name()))
        return DgnDbServerEventType::RevisionEvent;
    else
        return DgnDbServerEventType::UnknownEventType;
    }
