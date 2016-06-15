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
RepositoryInfo::RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR mergedRevisionId)
    : m_serverUrl(serverUrl), m_id(id), m_mergedRevisionId(mergedRevisionId)
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
RepositoryInfo::RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR name, Utf8StringCR fileId, Utf8StringCR fileUrl,
                               Utf8StringCR fileName, Utf8StringCR description, Utf8StringCR mergedRevisionId, Utf8StringCR user, DateTimeCR date)
    : m_serverUrl(serverUrl), m_id(id), m_name(name), m_fileId(fileId), m_fileUrl(fileUrl), m_fileName(fileName), m_description(description),
    m_mergedRevisionId(mergedRevisionId), m_userCreated(user), m_createdDate(date)
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
Utf8StringCR RepositoryInfo::GetFileId() const
    {
    return m_fileId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
Utf8StringCR RepositoryInfo::GetFileURL() const
    {
    return m_fileUrl;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
Utf8StringCR RepositoryInfo::GetFileName() const
    {
    return m_fileName;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Utf8String RepositoryInfo::GetWSRepositoryName() const
    {
    return ServerSchema::Plugin::Repository + ("--" + m_id);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             05/2016
//---------------------------------------------------------------------------------------
Utf8StringCR RepositoryInfo::GetMergedRevisionId() const
    {
    return m_mergedRevisionId;
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
    Utf8String parentRevisionId;
    BeSQLite::DbResult status;
    status = db.QueryBriefcaseLocalValue(Db::Local::RepositoryURL, serverUrl);
    if (BeSQLite::DbResult::BE_SQLITE_ROW == status)
        status = db.QueryBriefcaseLocalValue(Db::Local::RepositoryId, id);
    if (BeSQLite::DbResult::BE_SQLITE_ROW == status)
        status = db.QueryBriefcaseLocalValue(Db::Local::ParentRevisionId, parentRevisionId);
    if (BeSQLite::DbResult::BE_SQLITE_ROW == status)
        {
        repositoryInfo = RepositoryInfo(serverUrl, id, parentRevisionId);
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
    status = db.ChangeBriefcaseId(briefcaseId);
    if (BeSQLite::DbResult::BE_SQLITE_OK == status)
        status = db.SaveBriefcaseLocalValue(Db::Local::RepositoryURL, repositoryInfo.GetServerURL());
    if (BeSQLite::DbResult::BE_SQLITE_DONE == status)
        status = db.SaveBriefcaseLocalValue(Db::Local::RepositoryId, repositoryInfo.GetId());
    if (BeSQLite::DbResult::BE_SQLITE_DONE == status)
        status = db.SaveBriefcaseLocalValue(Db::Local::ParentRevisionId, repositoryInfo.GetMergedRevisionId());
    if (BeSQLite::DbResult::BE_SQLITE_DONE == status)
        return DgnDbServerStatusResult::Success();
    return DgnDbServerStatusResult::Error(DgnDbServerError(db, status));
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
