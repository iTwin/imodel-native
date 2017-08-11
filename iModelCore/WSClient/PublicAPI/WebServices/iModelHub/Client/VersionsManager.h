/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/VersionsManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/ChangeSetInfo.h>
#include <WebServices/iModelHub/Client/VersionInfo.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(VersionsManager);
//WIP remove then relationship to get changeSets implemented
DEFINE_POINTER_SUFFIX_TYPEDEFS(iModelConnection);

struct VersionsManager
    {
private:
    friend struct iModelConnection;

    //WIP remove then relationship to get changeSets implemented
    iModelConnectionCP m_connection;
    IWSRepositoryClientPtr m_wsRepositoryClient = nullptr;
    VersionsManager(IWSRepositoryClientPtr reposiroryClient, iModelConnectionCP connection)
        : m_wsRepositoryClient(reposiroryClient), m_connection(connection){};

    VersionsManager() {};
public:
    //! Returns all versions available in the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of Versions information as the result.
    IMODELHUBCLIENT_EXPORT VersionsInfoTaskPtr GetAllVersions(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns version with specified id
    //! @param[in] versionId
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the Version information as the result.
    IMODELHUBCLIENT_EXPORT VersionInfoTaskPtr GetVersionById(Utf8StringCR versionId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Creates new version associated with ChangeSet
    //! @param[in] version
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the Version information as the result.
    IMODELHUBCLIENT_EXPORT VersionInfoTaskPtr CreateVersion(VersionInfoR version, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Modifies Versions Name and/or Description
    //! @param[in] version
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT StatusTaskPtr UpdateVersion(VersionInfoCR version, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns ChangeSets from first ChangeSet to Version.
    //! @param[in] versionId
    //! @param[in] cancellationToken
    //! @returns Asynchronous task that has ChangeSets information as the result.
    IMODELHUBCLIENT_EXPORT ChangeSetsInfoTaskPtr GetVersionChangeSets(Utf8String versionId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns ChangeSets used to get one Version from another.
    //! @param[in] firstVersionId
    //! @param[in] secondVersionId
    //! @param[in] cancellationToken
    //! @returns Asynchronous task that has ChangeSets information as the result.
    IMODELHUBCLIENT_EXPORT ChangeSetsInfoTaskPtr GetChangeSetsBetweenVersions(Utf8String firstVersionId, Utf8String secondVersionId, ICancellationTokenPtr cancellationToken = nullptr) const;
    };

END_BENTLEY_IMODELHUB_NAMESPACE
