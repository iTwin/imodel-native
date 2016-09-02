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
DgnDbBriefcaseInfo::DgnDbBriefcaseInfo(BeSQLite::BeBriefcaseId id, Utf8StringCR userOwned)
    : m_id(id), m_userOwned(userOwned)
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
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbBriefcaseInfoPtr DgnDbBriefcaseInfo::FromJson(JsonValueCR json)
    {
    JsonValueCR properties      = json[ServerSchema::Properties];  

    BeSQLite::BeBriefcaseId id;
    RepositoryJson::BriefcaseIdFromJson(id, properties[ServerSchema::Property::BriefcaseId]);
    Utf8String  userOwned = properties[ServerSchema::Property::UserOwned].asString();

    return std::make_shared<DgnDbBriefcaseInfo>(id, userOwned);
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
