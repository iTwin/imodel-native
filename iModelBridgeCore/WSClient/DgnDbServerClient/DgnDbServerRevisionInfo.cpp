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
DgnDbServerRevisionInfo::DgnDbServerRevisionInfo(Utf8String id, Utf8String parentRevisionId, Utf8String dbGuid, int64_t index, Utf8String url)
    : m_id(id), m_parentRevisionId(parentRevisionId), m_dbGuid(dbGuid), m_index(index), m_url(url)
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
int64_t DgnDbServerRevisionInfo::GetIndex() const
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

// avoid collision of a static function with the same name in another CPP file in this compiland...
BEGIN_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          01/2017
//---------------------------------------------------------------------------------------
DgnDbServerRevisionInfoPtr ParseRapidJson(RapidJsonValueCR properties)
    {
    auto id = properties[ServerSchema::Property::Id].GetString();
    auto dbGuid = properties[ServerSchema::Property::MasterFileId].GetString();
    auto parentRevisionId = properties[ServerSchema::Property::ParentId].GetString();

    Utf8String url = properties[ServerSchema::Property::URL].IsString() ? properties[ServerSchema::Property::URL].GetString() : "";
    Utf8String revIndex = properties[ServerSchema::Property::Index].GetString();
    uint64_t index = -1;
    if (!revIndex.empty())
        {
        BeStringUtilities::ParseUInt64(index, revIndex.c_str());
        }

    return std::make_shared<DgnDbServerRevisionInfo>(id, parentRevisionId, dbGuid, index, url);
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
