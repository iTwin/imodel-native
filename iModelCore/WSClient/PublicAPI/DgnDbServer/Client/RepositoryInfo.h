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
#include <WebServices/Client/Response/WSObjectsReader.h>
#include <Bentley/DateTime.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES
typedef std::shared_ptr<struct RepositoryInfo> RepositoryInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(RepositoryInfo);
DEFINE_TASK_TYPEDEFS(RepositoryInfoPtr, DgnDbServerRepository);

//=======================================================================================
//! Information about revision file that is on server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct RepositoryInfo
{
//__PUBLISH_SECTION_END__
private:
    Utf8String m_serverUrl;
    Utf8String m_id;
    Utf8String m_name;
    Utf8String m_description;
    Utf8String m_userCreated;
    DateTime   m_createdDate;

    RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id);
    RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR name, Utf8StringCR description, Utf8StringCR user, DateTimeCR date);

    static RepositoryInfoPtr Parse(RapidJsonValueCR properties, Utf8StringCR repositoryInstanceId, Utf8StringCR url);
//__PUBLISH_SECTION_START__
public:
    //__PUBLISH_SECTION_END__
    RepositoryInfo();

    bool operator==(RepositoryInfoCR rhs) const;
    //! DEPRECATED: Use Parsing from Instance
    static RepositoryInfoPtr Parse(JsonValueCR json, Utf8StringCR url);
    static RepositoryInfoPtr Parse(WSObjectsReader::Instance instnace, Utf8StringCR url);
    static RepositoryInfoPtr Create (Utf8StringCR serverUrl, Utf8StringCR id);
    //__PUBLISH_SECTION_START__


    DGNDBSERVERCLIENT_EXPORT static DgnDbServerRepositoryResult ReadRepositoryInfo(Dgn::DgnDbCR db);
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusResult WriteRepositoryInfo(Dgn::DgnDbR db, BeSQLite::BeBriefcaseId const& briefcaseId, bool clearLastPulledRevisionId = false) const;

    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetDescription() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetServerURL() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetName() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetId() const;
    DGNDBSERVERCLIENT_EXPORT Utf8String   GetWSRepositoryName() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetUserCreated() const;
    DGNDBSERVERCLIENT_EXPORT DateTimeCR   GetCreatedDate() const;
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
