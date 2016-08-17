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
    Utf8String m_id;
    Utf8String m_name;
    Utf8String m_description;
    Utf8String m_userCreated;
    DateTime   m_createdDate;
 //__PUBLISH_SECTION_START__
public:
    DGNDBSERVERCLIENT_EXPORT RepositoryInfo();
    DGNDBSERVERCLIENT_EXPORT RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id);
    DGNDBSERVERCLIENT_EXPORT RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR name, Utf8StringCR description);
    DGNDBSERVERCLIENT_EXPORT RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR name, Utf8StringCR description, Utf8StringCR user, DateTimeCR date);

    //__PUBLISH_SECTION_END__
    static DgnDbServerStatusResult ReadRepositoryInfo(RepositoryInfo& repositoryInfo, Dgn::DgnDbCR db);
    static DgnDbServerStatusResult WriteRepositoryInfo(Dgn::DgnDbR db, RepositoryInfoCR repositoryInfo, BeSQLite::BeBriefcaseId const& briefcaseId);
    bool operator==(RepositoryInfoCR rhs) const;
    static RepositoryInfoPtr FromJson(JsonValueCR json, Utf8StringCR url);
    //__PUBLISH_SECTION_START__

    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetDescription() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetServerURL() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetName() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetId() const;
    DGNDBSERVERCLIENT_EXPORT Utf8String   GetWSRepositoryName() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetUserCreated() const;
    DGNDBSERVERCLIENT_EXPORT DateTimeCR   GetCreatedDate() const;
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
