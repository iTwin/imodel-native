/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/RepositoryInfo.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/RepositoryInfo.h>
#include <DgnPlatform/TxnManager.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
RepositoryInfo::RepositoryInfo()
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
RepositoryInfo::RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id)
    : m_serverUrl(serverUrl), m_id(id)
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
RepositoryInfo::RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR name, Utf8StringCR description)
    : m_serverUrl(serverUrl), m_id(id), m_name(name), m_description(description)
    {

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
RepositoryInfo::RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR name, Utf8StringCR description, Utf8StringCR user, DateTimeCR date)
    : m_serverUrl(serverUrl), m_id(id), m_name(name), m_description(description), m_userCreated(user), m_createdDate(date)
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Utf8StringCR RepositoryInfo::GetDescription() const
    {
    return m_description;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Utf8StringCR RepositoryInfo::GetServerURL() const
    {
    return m_serverUrl;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
Utf8StringCR RepositoryInfo::GetName() const
    {
    return m_name;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Utf8StringCR RepositoryInfo::GetId() const
    {
    return m_id;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Utf8String RepositoryInfo::GetWSRepositoryName() const
    {
    return ServerSchema::Plugin::Repository + ("--" + m_id);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Utf8StringCR RepositoryInfo::GetUserCreated() const
    {
    return m_userCreated;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DateTimeCR RepositoryInfo::GetCreatedDate() const
    {
    return m_createdDate;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusResult RepositoryInfo::ReadRepositoryInfo(RepositoryInfo& repositoryInfo, Dgn::DgnDbCR db)
    {
    Utf8String serverUrl;
    Utf8String id;
    BeSQLite::DbResult status;
    status = db.QueryBriefcaseLocalValue(Db::Local::RepositoryURL, serverUrl);
    if (BeSQLite::DbResult::BE_SQLITE_ROW == status)
        status = db.QueryBriefcaseLocalValue(Db::Local::RepositoryId, id);
    if (BeSQLite::DbResult::BE_SQLITE_ROW == status)
        {
        repositoryInfo = RepositoryInfo(serverUrl, id);
        return DgnDbServerStatusResult::Success();
        }
    return DgnDbServerStatusResult::Error(DgnDbServerError(db, status));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusResult RepositoryInfo::WriteRepositoryInfo(Dgn::DgnDbR db, RepositoryInfoCR repositoryInfo, BeSQLite::BeBriefcaseId const& briefcaseId)
    {
    BeSQLite::DbResult status;
    Utf8String parentRevisionId = db.Revisions().GetParentRevisionId();
    status = db.ChangeBriefcaseId(briefcaseId);

    //Write the RepositoryInfo properties to the file
    if (BeSQLite::DbResult::BE_SQLITE_OK == status)
        status = db.SaveBriefcaseLocalValue(Db::Local::RepositoryURL, repositoryInfo.GetServerURL());
    if (BeSQLite::DbResult::BE_SQLITE_DONE == status)
        status = db.SaveBriefcaseLocalValue(Db::Local::RepositoryId, repositoryInfo.GetId());

    //ParentRevisionId is reset when changing briefcase Id
    if (BeSQLite::DbResult::BE_SQLITE_DONE == status)
        status = db.SaveBriefcaseLocalValue(Db::Local::ParentRevisionId, parentRevisionId);

    if (BeSQLite::DbResult::BE_SQLITE_DONE == status)
        return DgnDbServerStatusResult::Success();
    return DgnDbServerStatusResult::Error(DgnDbServerError(db, status));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
RepositoryInfoPtr RepositoryInfo::FromJson(JsonValueCR json, Utf8StringCR url)
    {
    Utf8String repositoryInstanceId = json[ServerSchema::InstanceId].asString();
    JsonValueCR properties = json[ServerSchema::Properties];
    Utf8String name = properties[ServerSchema::Property::RepositoryName].asString();
    Utf8String description = properties[ServerSchema::Property::RepositoryDescription].asString();
    Utf8String userUploaded = properties[ServerSchema::Property::UserCreated].asString();
    DateTime uploadedDate;
    Utf8String dateStr = properties[ServerSchema::Property::UploadedDate].asString();
    if (!dateStr.empty())
        DateTime::FromString(uploadedDate, dateStr.c_str());
    return std::make_shared<RepositoryInfo>(url, repositoryInstanceId, name, description, userUploaded, uploadedDate);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
bool RepositoryInfo::operator==(RepositoryInfoCR rhs) const
    {
    if (rhs.GetId() == GetId() && rhs.GetServerURL() == GetServerURL())
        return true;
    return false;
    }
