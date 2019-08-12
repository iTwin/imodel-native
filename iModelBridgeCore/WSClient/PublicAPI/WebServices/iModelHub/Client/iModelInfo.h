/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/Result.h>
#include <WebServices/Client/Response/WSObjectsReader.h>
#include <Bentley/DateTime.h>
#include <WebServices/iModelHub/Client/UserInfo.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

USING_NAMESPACE_BENTLEY_WEBSERVICES

typedef RefCountedPtr<struct iModelInfo> iModelInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(iModelInfo);
DEFINE_TASK_TYPEDEFS(iModelInfoPtr, iModel);

//=======================================================================================
//! Information about changeSet file that is on server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct iModelInfo : RefCountedBase
{
friend struct Client;
private:
    Utf8String m_serverUrl;
    Utf8String m_id;
    Utf8String m_name;
    Utf8String m_description;
    Utf8String m_userCreated;
    DateTime   m_createdDate;
    UserInfoPtr m_ownerInfo;
    bool       m_isInitialized;

    iModelInfo() {}
    iModelInfo(Utf8StringCR serverUrl, Utf8StringCR id) : m_serverUrl(serverUrl), m_id(id) {}
    iModelInfo(Utf8StringCR serverUrl, Utf8StringCR id, Utf8StringCR name, Utf8StringCR description, Utf8StringCR user, 
               DateTimeCR date, UserInfoPtr ownerInfo, bool isInitialized)
        : m_serverUrl(serverUrl), m_id(id), m_name(name), m_description(description), m_userCreated(user), m_createdDate(date), 
        m_ownerInfo(ownerInfo), m_isInitialized(isInitialized) {}

    bool operator==(iModelInfoCR rhs) const {return rhs.GetId() == GetId() && rhs.GetServerURL() == GetServerURL();}
    static iModelInfoPtr Parse(WSObjectsReader::Instance instance, Utf8StringCR url);
    static iModelInfoPtr Parse(RapidJsonValueCR properties, Utf8StringCR iModelInstanceId, UserInfoPtr ownerInfo, Utf8StringCR url);
    static iModelResult ReadFromLocalValues(Dgn::DgnDbCR db);
    BeSQLite::DbResult WriteiModelProperties(Dgn::DgnDbR db) const;
    static iModelInfoPtr Create(Utf8StringCR serverUrl, Utf8StringCR id) { return iModelInfoPtr(new iModelInfo(serverUrl, id)); }
public:
    IMODELHUBCLIENT_EXPORT static iModelResult ReadiModelInfo(Dgn::DgnDbCR db);
    StatusResult WriteiModelInfo(Dgn::DgnDbR db, BeSQLite::BeBriefcaseId const& briefcaseId) const;

    Utf8StringCR GetDescription() const {return m_description;}
    Utf8StringCR GetServerURL() const {return m_serverUrl;}
    Utf8StringCR GetName() const {return m_name;}
    Utf8StringCR GetId() const {return m_id;}
    IMODELHUBCLIENT_EXPORT Utf8String GetWSRepositoryName() const;
    Utf8StringCR GetUserCreated() const {return m_userCreated;}
    DateTimeCR GetCreatedDate() const {return m_createdDate;}
    UserInfoPtr GetOwnerInfo() const {return m_ownerInfo;}
    bool IsInitialized() const { return m_isInitialized; }

    void SetName(Utf8String name) { m_name = name; }
    void SetDescription(Utf8String description) { m_description = description; }

    static void AddHasCreatorInfoSelect(Utf8StringR);
};
END_BENTLEY_IMODELHUB_NAMESPACE
