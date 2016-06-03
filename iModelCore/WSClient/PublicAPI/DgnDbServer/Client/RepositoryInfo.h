/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/RepositoryInfo.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerResult.h>
#include <Bentley/DateTime.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
typedef std::shared_ptr<struct RepositoryInfo> RepositoryInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(RepositoryInfo);

//=======================================================================================
//! Information about revision file that is on server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct RepositoryInfo
{
//__PUBLISH_SECTION_END__
private:
    Utf8String m_serverUrl;
    Utf8String m_fileUrl;
    Utf8String m_fileName;
    Utf8String m_description;
    Utf8String m_name;
    Utf8String m_id;
    Utf8String m_fileId;
    Utf8String m_mergedRevisionId;
    Utf8String m_userCreated;
    DateTime   m_createdDate;
 //__PUBLISH_SECTION_START__
public:
    DGNDBSERVERCLIENT_EXPORT RepositoryInfo();
    DGNDBSERVERCLIENT_EXPORT RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR mergedRevisionId = "");
    DGNDBSERVERCLIENT_EXPORT RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR name, Utf8StringCR description);
    DGNDBSERVERCLIENT_EXPORT RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR name, Utf8StringCR fileId, Utf8StringCR fileUrl,
                                            Utf8StringCR fileName, Utf8StringCR description, Utf8StringCR mergedRevisionId, Utf8StringCR user, DateTimeCR date);

    //__PUBLISH_SECTION_END__
    static DgnDbServerStatusResult ReadRepositoryInfo(RepositoryInfo& repositoryInfo, Dgn::DgnDbCR db);
    static DgnDbServerStatusResult WriteRepositoryInfo(Dgn::DgnDbR db, RepositoryInfoCR repositoryInfo, BeSQLite::BeBriefcaseId const& briefcaseId);
    bool operator==(RepositoryInfoCR rhs) const;
    //__PUBLISH_SECTION_START__

    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetDescription() const; //!< Description taken from dgn_Proj Description property of the master file.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetServerURL() const; //!< URL of the server where the master file is stored.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetFileURL() const; //!< URL of the master file.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetFileName() const; //!< FileName of the master file.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetName() const; //!< Repository name taken from dgn_Proj Name property of the master file or supplied by user.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetId() const; //!< Converted repository name, containing no illegal characters.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetFileId() const; //!< Guid of the repository master file.
    DGNDBSERVERCLIENT_EXPORT Utf8String   GetWSRepositoryName() const; //!< Formatted WebServices repository id that is used in the address.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetMergedRevisionId() const; //!< Get Last Revision Id merged to the repository.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetUserCreated() const; //!< Name of the user that uploaded the master file.
    DGNDBSERVERCLIENT_EXPORT DateTimeCR   GetCreatedDate() const; //!< Date when the file was uploaded.
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
