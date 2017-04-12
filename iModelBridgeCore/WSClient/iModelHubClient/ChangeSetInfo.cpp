/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/ChangeSetInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerRevisionInfo.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

// avoid collision of a static function with the same name in another CPP file in this compiland...
BEGIN_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
uint64_t ParseInt(RapidJsonValueCR properties, Utf8String stringName, uint64_t defaultValue)
    {
    if (!properties.HasMember(stringName.c_str()))
        return defaultValue;

    uint64_t returnValue = defaultValue;
    Utf8String stringValue = properties[stringName.c_str()].GetString();
    if (!stringValue.empty())
        {
        BeStringUtilities::ParseUInt64(returnValue, stringValue.c_str());
        }
    return returnValue;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
Utf8String ParseString(RapidJsonValueCR properties, Utf8String stringName, Utf8String defaultValue)
    {
    if (properties.HasMember(stringName.c_str()))
        return properties[stringName.c_str()].GetString();

    return defaultValue;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
DgnDbServerRevisionInfoPtr ParseRapidJson(RapidJsonValueCR properties)
    {
    auto id = properties[ServerSchema::Property::Id].GetString();
    auto dbGuid = ParseString(properties, ServerSchema::Property::MasterFileId, "");
    auto parentRevisionId = ParseString(properties, ServerSchema::Property::ParentId, "");
    auto description = ParseString(properties, ServerSchema::Property::Description, "");
    auto userCreated = ParseString(properties, ServerSchema::Property::UserCreated, "");

    uint64_t index = ParseInt(properties, ServerSchema::Property::Index, -1);
    uint64_t fileSize = ParseInt(properties, ServerSchema::Property::FileSize, -1);
    auto briefcaseId = properties.HasMember(ServerSchema::Property::BriefcaseId) ? BeBriefcaseId(properties[ServerSchema::Property::BriefcaseId].GetInt64()) : BeBriefcaseId(-1);
    auto pushDate = properties.HasMember(ServerSchema::Property::PushDate) ? BeJsonUtilities::DateTimeFromValue(properties[ServerSchema::Property::PushDate].GetString()) : DateTime();
    DgnDbServerRevisionInfo::ContainingChanges containingChanges = properties.HasMember(ServerSchema::Property::ContainingChanges) ?
        static_cast<DgnDbServerRevisionInfo::ContainingChanges>(properties[ServerSchema::Property::ContainingChanges].GetInt()) :
        static_cast<DgnDbServerRevisionInfo::ContainingChanges>(0);

    return new DgnDbServerRevisionInfo(id, parentRevisionId, dbGuid, index, description, fileSize, briefcaseId, userCreated, pushDate, containingChanges);
    }

END_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
DgnDbServerRevisionInfoPtr DgnDbServerRevisionInfo::Parse(JsonValueCR json)
    {
    JsonValueCR properties      = json[ServerSchema::Properties];
    auto rapidJson = ToRapidJson(properties);

    return ParseRapidJson(rapidJson);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
DgnDbServerRevisionInfoPtr DgnDbServerRevisionInfo::Parse(WSObjectsReader::Instance instance)
    {
    RapidJsonValueCR properties = instance.GetProperties();
    return ParseRapidJson(properties);
    }
