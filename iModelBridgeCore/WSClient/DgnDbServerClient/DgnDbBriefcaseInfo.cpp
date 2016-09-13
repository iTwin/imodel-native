/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbBriefcaseInfo.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbBriefcaseInfo.h>
#include <DgnPlatform/TxnManager.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbBriefcaseInfo::DgnDbBriefcaseInfo()
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbBriefcaseInfo::DgnDbBriefcaseInfo(BeSQLite::BeBriefcaseId id)
    : m_id(id)
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbBriefcaseInfo::DgnDbBriefcaseInfo(BeSQLite::BeBriefcaseId id, Utf8StringCR userOwned, BeGuid fileId)
    : m_id(id), m_userOwned(userOwned), m_fileId(fileId)
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
BeSQLite::BeBriefcaseId DgnDbBriefcaseInfo::GetId() const
    {
    return m_id;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
Utf8StringCR DgnDbBriefcaseInfo::GetUserOwned() const
    {
    return m_userOwned;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis           09/2016
//---------------------------------------------------------------------------------------
BeGuid DgnDbBriefcaseInfo::GetFileId() const
    {
    return m_fileId;
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
DgnDbBriefcaseInfoPtr DgnDbBriefcaseInfo::FromJson(JsonValueCR json)
    {
    JsonValueCR properties      = json[ServerSchema::Properties];  

    BeSQLite::BeBriefcaseId id;
    RepositoryJson::BriefcaseIdFromJson(id, properties[ServerSchema::Property::BriefcaseId]);
    Utf8String  userOwned = properties[ServerSchema::Property::UserOwned].asString();
    BeGuid fileId;
    GuidFromJson(fileId, properties[ServerSchema::Property::FileId]);

    return std::make_shared<DgnDbBriefcaseInfo>(id, userOwned, fileId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
bool DgnDbBriefcaseInfo::operator==(DgnDbBriefcaseInfoCR briefcase) const
    {
    if (briefcase.GetId() == GetId())
        return true;

    return false;
    }
