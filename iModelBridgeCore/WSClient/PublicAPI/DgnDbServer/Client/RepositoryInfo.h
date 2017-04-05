/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/RepositoryInfo.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
typedef RefCountedPtr<struct RepositoryInfo> RepositoryInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(RepositoryInfo);
DEFINE_TASK_TYPEDEFS(RepositoryInfoPtr, DgnDbServerRepository);

//=======================================================================================
//! Information about revision file that is on server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct RepositoryInfo : RefCountedBase
{
private:
    Utf8String m_serverUrl;
    Utf8String m_id;
    Utf8String m_name;
    Utf8String m_description;
    Utf8String m_userCreated;
    DateTime   m_createdDate;

    RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id) : m_serverUrl(serverUrl), m_id(id) {}
    RepositoryInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR name, Utf8StringCR description, Utf8StringCR user, DateTimeCR date)
        : m_serverUrl(serverUrl), m_id(id), m_name(name), m_description(description), m_userCreated(user), m_createdDate(date) {}

    static RepositoryInfoPtr Parse(RapidJsonValueCR properties, Utf8StringCR repositoryInstanceId, Utf8StringCR url);

public:
    RepositoryInfo() {}

    bool operator==(RepositoryInfoCR rhs) const {return rhs.GetId() == GetId() && rhs.GetServerURL() == GetServerURL();}
    //! DEPRECATED: Use Parsing from Instance
    static RepositoryInfoPtr Parse(JsonValueCR json, Utf8StringCR url);
    static RepositoryInfoPtr Parse(WSObjectsReader::Instance instnace, Utf8StringCR url);
    static RepositoryInfoPtr Create(Utf8StringCR serverUrl, Utf8StringCR id) { return RepositoryInfoPtr(new RepositoryInfo(serverUrl, id)); }

    DGNDBSERVERCLIENT_EXPORT static DgnDbServerRepositoryResult ReadRepositoryInfo(Dgn::DgnDbCR db);
    DgnDbServerStatusResult WriteRepositoryInfo(Dgn::DgnDbR db, BeSQLite::BeBriefcaseId const& briefcaseId, bool clearLastPulledRevisionId = false) const;

    Utf8StringCR GetDescription() const {return m_description;}
    Utf8StringCR GetServerURL() const {return m_serverUrl;}
    Utf8StringCR GetName() const {return m_name;}
    Utf8StringCR GetId() const {return m_id;}
    DGNDBSERVERCLIENT_EXPORT Utf8String GetWSRepositoryName() const;
    Utf8StringCR GetUserCreated() const {return m_userCreated;}
    DateTimeCR GetCreatedDate() const {return m_createdDate;}
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
