/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/RepositoryInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/RepositoryInfo.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER

#define DGNDBSERVER_PLUGIN_NAME "Bentley.DgnDbServerECPlugin"
#define DGNDBSERVER_LOCAL_REPOSITORY_URL "dgndbserver_serverUrl"
#define DGNDBSERVER_LOCAL_REPOSITORY_ID "dgndbserver_repositoryId"

RepositoryInfo::RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id)
    : m_fileInfo(FileInfo("", "", 0)), m_serverUrl(serverUrl), m_id(id) {}

RepositoryInfo::RepositoryInfo(FileInfoCR fileInfo, Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR description, Utf8StringCR user, DateTimeCR date)
    : m_fileInfo(fileInfo), m_serverUrl(serverUrl), m_id(id), m_description(description), m_userUploaded(user), m_uploadedDate(date) {}

RepositoryInfoPtr RepositoryInfo::Create(Utf8StringCR serverUrl, Utf8StringCR id)
    {
    return std::make_shared<RepositoryInfo>(RepositoryInfo(serverUrl, id));
    }

RepositoryInfoPtr RepositoryInfo::Create(FileInfoCR fileInfo, Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR description, Utf8StringCR user, DateTimeCR date)
    {
    return std::make_shared<RepositoryInfo>(RepositoryInfo(fileInfo, serverUrl, id, description, user, date));
    }

FileInfoCR   RepositoryInfo::GetFileInfo() const
    {
    return m_fileInfo;
    }

Utf8StringCR RepositoryInfo::GetDescription() const
    {
    return m_description;
    }

Utf8StringCR RepositoryInfo::GetServerURL() const
    {
    return m_serverUrl;
    }

Utf8StringCR RepositoryInfo::GetId() const
    {
    return m_id;
    }

Utf8String RepositoryInfo::GetWSRepositoryName() const
    {
    return DGNDBSERVER_PLUGIN_NAME "--" + m_id;
    }

Utf8StringCR RepositoryInfo::GetUserUploaded() const
    {
    return m_userUploaded;
    }

DateTimeCR RepositoryInfo::GetUploadedDate() const
    {
    return m_uploadedDate;
    }

RepositoryInfoPtr RepositoryInfo::ReadRepositoryInfo(Dgn::DgnDbCR db)
    {
    Utf8String serverUrl;
    Utf8String id;
    db.QueryBriefcaseLocalValue(DGNDBSERVER_LOCAL_REPOSITORY_URL, serverUrl);
    db.QueryBriefcaseLocalValue(DGNDBSERVER_LOCAL_REPOSITORY_ID, id);
    return RepositoryInfo::Create(serverUrl, id);
    }

BeSQLite::DbResult RepositoryInfo::WriteRepositoryInfo(Dgn::DgnDbR db, const RepositoryInfo& repositoryInfo)
    {
    BeSQLite::DbResult status;
    status = db.SaveBriefcaseLocalValue(DGNDBSERVER_LOCAL_REPOSITORY_URL, repositoryInfo.GetServerURL());
    if (BeSQLite::DbResult::BE_SQLITE_OK == status)
        status = db.SaveBriefcaseLocalValue(DGNDBSERVER_LOCAL_REPOSITORY_ID, repositoryInfo.GetId());
    return status;
    }
