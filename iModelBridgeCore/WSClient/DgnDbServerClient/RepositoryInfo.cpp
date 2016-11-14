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
#include <DgnDbServer/Client/Logging.h>

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
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
RepositoryInfo::RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR name, Utf8StringCR description, Utf8StringCR user, DateTimeCR date)
    : m_serverUrl(serverUrl), m_id(id), m_name(name), m_description(description), m_userCreated(user), m_createdDate(date)
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  10/2016
//---------------------------------------------------------------------------------------
RepositoryInfoPtr RepositoryInfo::Create(Utf8StringCR serverUrl, Utf8StringCR id)
    {
    return RepositoryInfoPtr (new RepositoryInfo(serverUrl, id));
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
DgnDbServerRepositoryResult RepositoryInfo::ReadRepositoryInfo(Dgn::DgnDbCR db)
    {
    const Utf8String methodName = "RepositoryInfo::ReadRepositoryInfo";
    Utf8String serverUrl;
    Utf8String id;
    BeSQLite::DbResult status;
    status = db.QueryBriefcaseLocalValue(serverUrl, Db::Local::RepositoryURL);
    if (BeSQLite::DbResult::BE_SQLITE_ROW == status)
        status = db.QueryBriefcaseLocalValue(id, Db::Local::RepositoryId);
    if (BeSQLite::DbResult::BE_SQLITE_ROW == status)
        {
        return DgnDbServerRepositoryResult::Success(Create(serverUrl, id));
        }
    auto error = DgnDbServerError(db, status);
    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
    return DgnDbServerRepositoryResult::Error(error);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusResult RepositoryInfo::WriteRepositoryInfo(Dgn::DgnDbR db, BeSQLite::BeBriefcaseId const& briefcaseId, bool clearLastPulledRevisionId) const
    {
    const Utf8String methodName = "RepositoryInfo::WriteRepositoryInfo";
    BeSQLite::DbResult status;
    Utf8String parentRevisionId = clearLastPulledRevisionId ? "" : db.Revisions().GetParentRevisionId();
    status = db.ChangeBriefcaseId(briefcaseId);

    //Write the RepositoryInfo properties to the file
    if (BeSQLite::DbResult::BE_SQLITE_OK == status)
        status = db.SaveBriefcaseLocalValue(Db::Local::RepositoryURL, GetServerURL());
    if (BeSQLite::DbResult::BE_SQLITE_DONE == status)
        status = db.SaveBriefcaseLocalValue(Db::Local::RepositoryId, GetId());

    //ParentRevisionId is reset when changing briefcase Id
    if (BeSQLite::DbResult::BE_SQLITE_DONE == status)
        status = db.SaveBriefcaseLocalValue(Db::Local::ParentRevisionId, parentRevisionId);

    if (BeSQLite::DbResult::BE_SQLITE_DONE == status)
        return DgnDbServerStatusResult::Success();
    auto error = DgnDbServerError(db, status);
    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
    return DgnDbServerStatusResult::Error(error);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
RepositoryInfoPtr RepositoryInfo::Parse(RapidJsonValueCR properties, Utf8StringCR repositoryInstanceId, Utf8StringCR url)
    {
    Utf8String name = properties[ServerSchema::Property::RepositoryName].GetString();
    Utf8String description = properties[ServerSchema::Property::RepositoryDescription].GetString();
    Utf8String userUploaded = properties.HasMember(ServerSchema::Property::UserCreated) ? properties[ServerSchema::Property::UserCreated].GetString() : "";
    DateTime uploadedDate;
    Utf8String dateStr = properties.HasMember(ServerSchema::Property::UploadedDate) ? properties[ServerSchema::Property::UploadedDate].GetString() : "";
    if (!dateStr.empty())
        DateTime::FromString(uploadedDate, dateStr.c_str());
    return RepositoryInfoPtr(new RepositoryInfo(url, repositoryInstanceId, name, description, userUploaded, uploadedDate));
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                    julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
RepositoryInfoPtr RepositoryInfo::Parse(JsonValueCR json, Utf8StringCR url)
    {
    if (json.isNull())
        return nullptr;
    Utf8String repositoryInstanceId = json[ServerSchema::InstanceId].asString();
    JsonValueCR properties = json[ServerSchema::Properties];

    auto rapidJson = ToRapidJson(properties);

    return Parse(rapidJson, repositoryInstanceId, url);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
RepositoryInfoPtr RepositoryInfo::Parse(WSObjectsReader::Instance instance, Utf8StringCR url)
    {
    Utf8String repositoryInstanceId = instance.GetObjectId().remoteId;
    RapidJsonValueCR properties = instance.GetProperties();
    return Parse(properties, repositoryInstanceId, url);
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
