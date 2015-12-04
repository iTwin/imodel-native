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
#include <Bentley/DateTime.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
typedef std::shared_ptr<struct RepositoryInfo> RepositoryInfoPtr;
typedef struct RepositoryInfo& RepositoryInfoR;
typedef const struct RepositoryInfo& RepositoryInfoCR;

//=======================================================================================
//! Information about revision file that is on server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct RepositoryInfo
{
//__PUBLISH_SECTION_END__
private:
    Utf8String m_serverUrl;
    Utf8String m_description;
    Utf8String m_id;
    Utf8String m_fileId;
    Utf8String m_userUploaded;
    DateTime   m_uploadedDate;
protected:
    RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id);
    RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR fileId, Utf8StringCR description, Utf8StringCR user, DateTimeCR date);
 //__PUBLISH_SECTION_START__
public:
    DGNDBSERVERCLIENT_EXPORT static RepositoryInfoPtr Create(Utf8StringCR serverUrl, Utf8StringCR id);
    DGNDBSERVERCLIENT_EXPORT static RepositoryInfoPtr Create(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR fileId, Utf8StringCR description, Utf8StringCR user, DateTimeCR date);

    //__PUBLISH_SECTION_END__
    static RepositoryInfoPtr ReadRepositoryInfo(Dgn::DgnDbCR db);
    static BeSQLite::DbResult WriteRepositoryInfo(Dgn::DgnDbR db, const RepositoryInfo& repositoryInfo, const BeSQLite::BeBriefcaseId& briefcaseId);
    bool operator==(RepositoryInfoCR rhs) const;
    //__PUBLISH_SECTION_START__

    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetDescription() const; //!< Description taken from dgn_Proj Description property of the master file.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetServerURL() const; //!< URL of the server where the master file is stored.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetId() const; //!< Repository Id taken from dgn_Proj Name property of the master file.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetFileId() const; //!< Guid of the repository master file.
    DGNDBSERVERCLIENT_EXPORT Utf8String   GetWSRepositoryName() const; //!< Formatted WebServices repository id that is used in the address.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetUserUploaded() const; //!< Name of the user that uploaded the master file.
    DGNDBSERVERCLIENT_EXPORT DateTimeCR   GetUploadedDate() const; //!< Date when the file was uploaded.
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
