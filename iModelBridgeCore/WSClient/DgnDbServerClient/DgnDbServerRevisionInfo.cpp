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
DgnDbServerRevisionInfo::DgnDbServerRevisionInfo(Utf8String id, Utf8String parentRevisionId, Utf8String dbGuid, int64_t index, Utf8String url,
    Utf8String description, int64_t fileSize, BeSQLite::BeBriefcaseId briefcaseId, Utf8String userCreated, DateTime pushDate, ContainingChanges containingChanges)
    : m_id(id), m_parentRevisionId(parentRevisionId), m_dbGuid(dbGuid), m_index(index), m_url(url),
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
Utf8String DgnDbServerRevisionInfo::GetUrl() const
    {
    return m_url;
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
uint64_t ParseInt(Utf8String stringValue, uint64_t defaultValue)
    {
    uint64_t returnValue = defaultValue;
    if (!stringValue.empty())
        {
        BeStringUtilities::ParseUInt64(returnValue, stringValue.c_str());
        }
    return returnValue;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
DgnDbServerRevisionInfoPtr ParseRapidJson(RapidJsonValueCR properties)
    {
    auto id = properties[ServerSchema::Property::Id].GetString();
    auto dbGuid = properties[ServerSchema::Property::MasterFileId].GetString();
    auto parentRevisionId = properties[ServerSchema::Property::ParentId].GetString();
    auto description = properties[ServerSchema::Property::Description].GetString();
    auto userCreated = properties[ServerSchema::Property::UserCreated].GetString();

    Utf8String url = properties[ServerSchema::Property::URL].IsString() ? properties[ServerSchema::Property::URL].GetString() : "";
    uint64_t index = ParseInt(properties[ServerSchema::Property::Index].GetString(), -1);
    uint64_t fileSize = ParseInt(properties[ServerSchema::Property::FileSize].GetString(), -1);
    auto briefcaseId = BeBriefcaseId(properties[ServerSchema::Property::BriefcaseId].GetInt64());
    auto pushDate = BeJsonUtilities::DateTimeFromValue(properties[ServerSchema::Property::PushDate].GetString());
    DgnDbServerRevisionInfo::ContainingChanges containingChanges = static_cast<DgnDbServerRevisionInfo::ContainingChanges>(properties[ServerSchema::Property::ContainingChanges].GetInt());

    return std::make_shared<DgnDbServerRevisionInfo>(id, parentRevisionId, dbGuid, index, url, description, fileSize, briefcaseId, userCreated, pushDate, containingChanges);
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
