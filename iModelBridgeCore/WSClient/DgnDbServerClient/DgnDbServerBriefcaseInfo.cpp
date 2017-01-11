/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerBriefcaseInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerBriefcaseInfo.h>
#include <DgnPlatform/TxnManager.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfo::DgnDbServerBriefcaseInfo()
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfo::DgnDbServerBriefcaseInfo(BeSQLite::BeBriefcaseId id)
    : m_id(id)
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfo::DgnDbServerBriefcaseInfo(BeSQLite::BeBriefcaseId id, Utf8StringCR userOwned, BeGuid fileId, bool isReadOnly)
    : m_id(id), m_userOwned(userOwned), m_fileId(fileId), m_isReadOnly(isReadOnly)
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
BeSQLite::BeBriefcaseId DgnDbServerBriefcaseInfo::GetId() const
    {
    return m_id;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
Utf8StringCR DgnDbServerBriefcaseInfo::GetUserOwned() const
    {
    return m_userOwned;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis           09/2016
//---------------------------------------------------------------------------------------
BeGuid DgnDbServerBriefcaseInfo::GetFileId() const
    {
    return m_fileId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                09/2016
//---------------------------------------------------------------------------------------
BeFileName DgnDbServerBriefcaseInfo::GetLocalPath() const
    {
    return m_localPath;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                09/2016
//---------------------------------------------------------------------------------------
void DgnDbServerBriefcaseInfo::SetLocalPath(BeFileName localPath) 
    {
    m_localPath = localPath;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                09/2016
//---------------------------------------------------------------------------------------
bool DgnDbServerBriefcaseInfo::GetIsReadOnly() const
    {
    return m_isReadOnly;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis           09/2016
//---------------------------------------------------------------------------------------
bool GuidFromJson(BeGuid& guid, RapidJsonValueCR json)
    {
    Utf8String guidString = json.GetString();
    if (guidString.size() > 0)
        return BentleyStatus::SUCCESS == guid.FromString(guidString.c_str());
    else
        return false;
    }

// avoid collision of a static function with the same name in another CPP file in this compiland...
BEGIN_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfoPtr ParseRapidJson(RapidJsonValueCR json)
    {
    BeSQLite::BeBriefcaseId id;
    id = BeBriefcaseId(json[ServerSchema::Property::BriefcaseId].GetUint());
    Utf8String  userOwned = json.HasMember(ServerSchema::Property::UserOwned) ? json[ServerSchema::Property::UserOwned].GetString() : "";
    BeGuid fileId;
    GuidFromJson(fileId, json[ServerSchema::Property::FileId]);
    bool isReadOnly = json[ServerSchema::Property::IsReadOnly].GetBool();

    return std::make_shared<DgnDbServerBriefcaseInfo>(id, userOwned, fileId, isReadOnly);
    }

END_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfoPtr DgnDbServerBriefcaseInfo::Parse(JsonValueCR json)
    {
    JsonValueCR properties      = json[ServerSchema::Properties]; 

    auto rapidJson = ToRapidJson(properties);

    return ParseRapidJson(rapidJson);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfoPtr DgnDbServerBriefcaseInfo::Parse(WSObjectsReader::Instance instance)
    {
    RapidJsonValueCR properties = instance.GetProperties();
    return ParseRapidJson(properties);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
bool DgnDbServerBriefcaseInfo::operator==(DgnDbServerBriefcaseInfoCR briefcase) const
    {
    if (briefcase.GetId() == GetId())
        return true;

    return false;
    }
