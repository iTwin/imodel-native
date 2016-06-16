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
std::shared_ptr<DgnDbServerEventParser> DgnDbServerEventParser::Create()
    {
    return std::shared_ptr<DgnDbServerEventParser>(new DgnDbServerEventParser());
    }

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
DgnDbServerEventPtr DgnDbServerEventParser::ParseEventasJson
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
    if (0 == (BeStringUtilities::Stricmp(DgnDbServerEvent::GenericEvent::GetEventName(DgnDbServerEvent::DgnDbServerEventType::LockEvent).c_str(), responseContentType)))
        {
        Json::Reader reader;
        Json::Value data(Json::objectValue);
        if (
            reader.parse(actualJsonPart, data) &&
            !data.isArray() &&
            data.isMember(DgnDbServerEvent::RepoId) &&
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
                                                data[DgnDbServerEvent::RepoId].asString(),
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
    else if (0 == (BeStringUtilities::Stricmp(DgnDbServerEvent::GenericEvent::GetEventName(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent).c_str(), responseContentType)))
        {
        Json::Reader reader;
        Json::Value data(Json::objectValue);
        if (
            reader.parse(actualJsonPart, data) &&
            !data.isArray() &&
            data.isMember(DgnDbServerEvent::RepoId) &&
            data.isMember(DgnDbServerEvent::UserId) &&
            data.isMember(DgnDbServerEvent::RevisionEventProperties::RevisionId) &&
            data.isMember(DgnDbServerEvent::RevisionEventProperties::RevisionIndex) &&
            data.isMember(DgnDbServerEvent::RevisionEventProperties::Date)
            )
            return DgnDbServerRevisionEvent::Create
                                                   (
                                                    data[DgnDbServerEvent::RepoId].asString(),
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
DgnDbServerEventPtr DgnDbServerEventParser::ParseEventasString
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
    if (0 == (BeStringUtilities::Stricmp(DgnDbServerEvent::GenericEvent::GetEventName(DgnDbServerEvent::DgnDbServerEventType::LockEvent).c_str(), responseContentType)))
        {
        bmap<Utf8String, Utf8String> entitymap;
        entitymap.Insert(DgnDbServerEvent::RepoId, "");
        entitymap.Insert(DgnDbServerEvent::UserId, "");
        entitymap.Insert(DgnDbServerEvent::LockEventProperties::ObjectId, "");
        entitymap.Insert(DgnDbServerEvent::LockEventProperties::LockType, "");
        entitymap.Insert(DgnDbServerEvent::LockEventProperties::LockLevel, "");
        entitymap.Insert(DgnDbServerEvent::LockEventProperties::BriefcaseId, "");
        entitymap.Insert(DgnDbServerEvent::LockEventProperties::ReleasedWithRevision, "");
        entitymap.Insert(DgnDbServerEvent::LockEventProperties::Date, "");

        if (!StringHelper(entitymap, actualJsonPart))
            return nullptr;
        return DgnDbServerLockEvent::Create
                                           (
                                            entitymap[DgnDbServerEvent::RepoId],
                                            entitymap[DgnDbServerEvent::UserId],
                                            entitymap[DgnDbServerEvent::LockEventProperties::ObjectId],
                                            entitymap[DgnDbServerEvent::LockEventProperties::LockType],
                                            entitymap[DgnDbServerEvent::LockEventProperties::LockLevel],
                                            entitymap[DgnDbServerEvent::LockEventProperties::BriefcaseId],
                                            entitymap[DgnDbServerEvent::LockEventProperties::ReleasedWithRevision],
                                            entitymap[DgnDbServerEvent::LockEventProperties::Date]
                                           );
        }
    else if (0 == (BeStringUtilities::Stricmp(DgnDbServerEvent::GenericEvent::GetEventName(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent).c_str(), responseContentType)))
        {
        bmap<Utf8String, Utf8String> entitymap;
        entitymap.Insert(DgnDbServerEvent::RepoId, "");
        entitymap.Insert(DgnDbServerEvent::UserId, "");
        entitymap.Insert(DgnDbServerEvent::RevisionEventProperties::RevisionId, "");
        entitymap.Insert(DgnDbServerEvent::RevisionEventProperties::RevisionIndex, "");
        entitymap.Insert(DgnDbServerEvent::RevisionEventProperties::Date, "");

        if (!StringHelper(entitymap, actualJsonPart))
            return nullptr;
        return DgnDbServerRevisionEvent::Create
                                               (
                                                entitymap[DgnDbServerEvent::RepoId],
                                                entitymap[DgnDbServerEvent::UserId],
                                                entitymap[DgnDbServerEvent::RevisionEventProperties::RevisionId],
                                                entitymap[DgnDbServerEvent::RevisionEventProperties::RevisionIndex],
                                                entitymap[DgnDbServerEvent::RevisionEventProperties::Date]
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
