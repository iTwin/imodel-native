/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/RepositoryInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/RepositoryInfo.h>
#include <DgnPlatform/TxnManager.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
RepositoryInfo::RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id)
    : m_serverUrl(serverUrl), m_id(id) {}

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
RepositoryInfo::RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR fileId, Utf8StringCR description, Utf8StringCR user, DateTimeCR date)
    : m_serverUrl(serverUrl), m_id(id), m_fileId(fileId), m_description(description), m_userUploaded(user), m_uploadedDate(date) {}

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
RepositoryInfoPtr RepositoryInfo::Create(Utf8StringCR serverUrl, Utf8StringCR id)
    {
    return std::make_shared<RepositoryInfo>(RepositoryInfo(serverUrl, id));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
RepositoryInfoPtr RepositoryInfo::Create(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR fileId, Utf8StringCR description, Utf8StringCR user, DateTimeCR date)
    {
    return std::make_shared<RepositoryInfo>(RepositoryInfo(serverUrl, id, fileId, description, user, date));
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
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Utf8StringCR RepositoryInfo::GetId() const
    {
    return m_id;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Utf8StringCR RepositoryInfo::GetFileId() const
    {
    return m_fileId;
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
Utf8StringCR RepositoryInfo::GetUserUploaded() const
    {
    return m_userUploaded;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DateTimeCR RepositoryInfo::GetUploadedDate() const
    {
    return m_uploadedDate;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
RepositoryInfoPtr RepositoryInfo::ReadRepositoryInfo(Dgn::DgnDbCR db)
    {
    Utf8String serverUrl;
    Utf8String id;
    db.QueryBriefcaseLocalValue(Db::Local::RepositoryURL, serverUrl);
    db.QueryBriefcaseLocalValue(Db::Local::RepositoryId, id);
    if (serverUrl.empty() || id.empty())
        return nullptr;
    return RepositoryInfo::Create(serverUrl, id);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
BeSQLite::DbResult RepositoryInfo::WriteRepositoryInfo(Dgn::DgnDbR db, const RepositoryInfo& repositoryInfo, const BeSQLite::BeBriefcaseId& briefcaseId)
    {
    BeSQLite::DbResult status;
    status = db.ChangeBriefcaseId(briefcaseId);
    if (BeSQLite::DbResult::BE_SQLITE_OK != status)
        return status;
    status = db.SaveBriefcaseLocalValue(Db::Local::RepositoryURL, repositoryInfo.GetServerURL());
    if (BeSQLite::DbResult::BE_SQLITE_DONE == status)
        status = db.SaveBriefcaseLocalValue(Db::Local::RepositoryId, repositoryInfo.GetId());
    return status;
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
