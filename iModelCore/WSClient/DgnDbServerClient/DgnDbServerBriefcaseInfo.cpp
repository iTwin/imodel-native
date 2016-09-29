/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerBriefcaseInfo.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
bool GuidFromJson(BeGuid& guid, JsonValueCR json)
    {
    Utf8String guidString = json.asString();
    if (guidString.size() > 0)
        return BentleyStatus::SUCCESS == guid.FromString(guidString.c_str());
    else
        return false;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfoPtr DgnDbServerBriefcaseInfo::FromJson(JsonValueCR json)
    {
    JsonValueCR properties      = json[ServerSchema::Properties];  

    BeSQLite::BeBriefcaseId id;
    RepositoryJson::BriefcaseIdFromJson(id, properties[ServerSchema::Property::BriefcaseId]);
    Utf8String  userOwned = properties[ServerSchema::Property::UserOwned].asString();
    BeGuid fileId;
    GuidFromJson(fileId, properties[ServerSchema::Property::FileId]);
    bool isReadOnly = properties[ServerSchema::Property::IsReadOnly].asBool();

    return std::make_shared<DgnDbServerBriefcaseInfo>(id, userOwned, fileId, isReadOnly);
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
