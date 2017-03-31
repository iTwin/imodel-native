/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerRevisionInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerRevisionInfo.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
DgnDbServerRevisionInfo::DgnDbServerRevisionInfo(Utf8String id, Utf8String parentRevisionId, Utf8String dbGuid, int64_t index,
    Utf8String description, int64_t fileSize, BeSQLite::BeBriefcaseId briefcaseId, Utf8String userCreated, DateTime pushDate, ContainingChanges containingChanges)
    : m_id(id), m_parentRevisionId(parentRevisionId), m_dbGuid(dbGuid), m_index(index),
     m_description(description), m_fileSize(fileSize), m_briefcaseId(briefcaseId), m_userCreated(userCreated), m_pushDate(pushDate), m_containingChanges(containingChanges)
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionInfo::GetId() const
    {
    return m_id;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionInfo::GetParentRevisionId() const
    {
    return m_parentRevisionId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionInfo::GetDbGuid() const
    {
    return m_dbGuid;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
uint64_t DgnDbServerRevisionInfo::GetIndex() const
    {
    return m_index;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionInfo::GetDescription() const
    {
    return m_description;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
uint64_t DgnDbServerRevisionInfo::GetFileSize() const
    {
    return m_fileSize;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionInfo::GetUserCreated() const
    {
    return m_userCreated;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
DateTime DgnDbServerRevisionInfo::GetPushDate() const
    {
    return m_pushDate;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
DgnDbServerRevisionInfo::ContainingChanges DgnDbServerRevisionInfo::GetContainingChanges() const
    {
    return m_containingChanges;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
BeSQLite::BeBriefcaseId DgnDbServerRevisionInfo::GetBriefcaseId() const
    {
    return m_briefcaseId;
    }

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

    return std::make_shared<DgnDbServerRevisionInfo>(id, parentRevisionId, dbGuid, index, description, fileSize, briefcaseId, userCreated, pushDate, containingChanges);
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

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
bool DgnDbServerRevisionInfo::operator==(DgnDbServerRevisionInfoCR revision) const
    {
    if (revision.GetId() == GetId())
        return true;

    return false;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          02/2017
//---------------------------------------------------------------------------------------
bool DgnDbServerRevisionInfo::GetContainsFileAccessKey()
    {
    return m_containsFileAccessKey;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          02/2017
//---------------------------------------------------------------------------------------
DgnDbServerFileAccessKeyPtr DgnDbServerRevisionInfo::GetFileAccessKey()
    {
    return m_fileAccessKey;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          02/2017
//---------------------------------------------------------------------------------------
void DgnDbServerRevisionInfo::SetFileAccessKey(DgnDbServerFileAccessKeyPtr fileAccessKey)
    {
    m_fileAccessKey = fileAccessKey;
    m_containsFileAccessKey = true;
    }
