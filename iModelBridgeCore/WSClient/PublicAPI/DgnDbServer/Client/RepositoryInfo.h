/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/RepositoryInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/FileInfo.h>
#include <Bentley/DateTime.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
typedef std::shared_ptr<struct RepositoryInfo> RepositoryInfoPtr;
typedef struct RepositoryInfo& RepositoryInfoR;
typedef const struct RepositoryInfo& RepositoryInfoCR;

struct RepositoryInfo
{
//__PUBLISH_SECTION_END__
private:
    FileInfo   m_fileInfo;
    Utf8String m_serverUrl;
    Utf8String m_description;
    Utf8String m_id;
    Utf8String m_userUploaded;
    DateTime   m_uploadedDate;
protected:
    RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id);
    RepositoryInfo(FileInfoCR fileInfo, Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR description, Utf8StringCR user, DateTimeCR date);
 //__PUBLISH_SECTION_START__
public:
    DGNDBSERVERCLIENT_EXPORT static RepositoryInfoPtr Create(Utf8StringCR serverUrl, Utf8StringCR id);
    DGNDBSERVERCLIENT_EXPORT static RepositoryInfoPtr Create(FileInfoCR fileInfo, Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR description, Utf8StringCR user, DateTimeCR date);


    DGNDBSERVERCLIENT_EXPORT FileInfoCR   GetFileInfo() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetDescription() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetServerURL() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetId() const;
    DGNDBSERVERCLIENT_EXPORT Utf8String   GetWSRepositoryName() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetUserUploaded() const;
    DGNDBSERVERCLIENT_EXPORT DateTimeCR   GetUploadedDate() const;

    static RepositoryInfoPtr ReadRepositoryInfo(Dgn::DgnDbCR db);
    static BeSQLite::DbResult WriteRepositoryInfo(Dgn::DgnDbR db, const RepositoryInfo& repositoryInfo);
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
