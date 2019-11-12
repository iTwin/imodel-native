/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/ChangeSetInfo.h>
#include <WebServices/iModelHub/Client/VersionInfo.h>
#include <WebServices/iModelHub/Client/GlobalRequestOptions.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

USING_NAMESPACE_BENTLEY_WEBSERVICES

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
    GlobalRequestOptionsPtr m_globalRequestOptionsPtr = nullptr;
    VersionsManager(IWSRepositoryClientPtr reposiroryClient, GlobalRequestOptionsPtr globalRequestOptionsPtr, iModelConnectionCP connection)
        : m_connection(connection), m_wsRepositoryClient(reposiroryClient), m_globalRequestOptionsPtr(globalRequestOptionsPtr){};

    WSQuery CreateChangeSetsBetweenVersionsQuery(Utf8StringCR sourceVersionId, Utf8String destinationVersionsId, BeSQLite::BeGuidCR fileId) const;

    WSQuery CreateVersionChangeSetsQuery(Utf8StringCR versionId, BeSQLite::BeGuidCR fileId) const;

    WSQuery CreateChangeSetsAfterVersionQuery(Utf8StringCR versionId, BeSQLite::BeGuidCR fileId) const;

    WSQuery CreateChangeSetsBetweenVersionAndChangeSetQuery(Utf8StringCR versionId, Utf8StringCR changeSetId, BeSQLite::BeGuidCR fileId) const;

    VersionsManager() {};

public:
    //! Returns all versions available in the server.
    //! @param[in] cancellationToken
    //! @param[in] thumbnailsToSelect Thumbnails sizes to select
    //! @return Asynchronous task that has the collection of Versions information as the result.
    IMODELHUBCLIENT_EXPORT VersionsInfoTaskPtr GetAllVersions(ICancellationTokenPtr cancellationToken = nullptr, 
                                                              Thumbnail::Size thumbnailsToSelect = Thumbnail::Size::None) const;

    //! Returns version with specified id
    //! @param[in] versionId
    //! @param[in] cancellationToken
    //! @param[in] thumbnailsToSelect Thumbnails sizes to select
    //! @return Asynchronous task that has the Version information as the result.
    IMODELHUBCLIENT_EXPORT VersionInfoTaskPtr GetVersionById(Utf8StringCR versionId, ICancellationTokenPtr cancellationToken = nullptr, 
                                                             Thumbnail::Size thumbnailsToSelect = Thumbnail::Size::None) const;

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
    //! @param[in] fileId file id of the seed file changeSets belong to
    //! @param[in] cancellationToken
    //! @returns Asynchronous task that has ChangeSets information as the result.
    IMODELHUBCLIENT_EXPORT ChangeSetsInfoTaskPtr GetVersionChangeSets(Utf8String versionId, BeSQLite::BeGuid fileId = BeSQLite::BeGuid(), 
                                                                      ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns ChangeSets between two Versions.
    //! @param[in] firstVersionId
    //! @param[in] secondVersionId
    //! @param[in] fileId file id of the seed file changeSets belong to
    //! @param[in] cancellationToken
    //! @returns Asynchronous task that has ChangeSets information as the result.
    IMODELHUBCLIENT_EXPORT ChangeSetsInfoTaskPtr GetChangeSetsBetweenVersions(Utf8String firstVersionId, Utf8String secondVersionId, 
                                                                              BeSQLite::BeGuid fileId = BeSQLite::BeGuid(), 
                                                                              ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns ChangeSets following Version's ChangeSet.
    //! @param[in] versionId
    //! @param[in] fileId file id of the seed file changeSets belong to
    //! @param[in] cancellationToken
    //! @returns Asynchronous task that has ChangeSets information as the result.
    IMODELHUBCLIENT_EXPORT ChangeSetsInfoTaskPtr GetChangeSetsAfterVersion(Utf8String versionId, BeSQLite::BeGuid fileId = BeSQLite::BeGuid(), 
                                                                           ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns ChangeSets between version's ChangeSet and specified ChangeSet 
    //! @param[in] versionId
    //! @param[in] changeSetId
    //! @param[in] fileId file id of the seed file changeSets belong to
    //! @param[in] cancellationToken
    //! @returns Asynchronous task that has ChangeSets information as the result.
    IMODELHUBCLIENT_EXPORT ChangeSetsInfoTaskPtr GetChangeSetsBetweenVersionAndChangeSet(Utf8String versionId, Utf8String changeSetId, 
                                                                                         BeSQLite::BeGuid fileId = BeSQLite::BeGuid(), 
                                                                                         ICancellationTokenPtr cancellationToken = nullptr) const;
    };

END_BENTLEY_IMODELHUB_NAMESPACE
