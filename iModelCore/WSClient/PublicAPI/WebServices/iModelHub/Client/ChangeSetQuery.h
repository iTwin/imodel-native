/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/Client/WSRepositoryClient.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

USING_NAMESPACE_BENTLEY_WEBSERVICES

typedef struct ChangeSetQuery& ChangeSetQueryR;

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             10/2019
//=======================================================================================
struct ChangeSetQuery
{
private:
    friend struct iModelConnection;
    friend struct VersionsManager;
    friend struct Briefcase;
    friend struct Client;
    WSQuery m_wsQuery;

    static void AppendFilter(Utf8StringR filterString, Utf8CP format, ...);
    void SetSelect(Utf8StringCR select) { m_wsQuery.SetSelect(select); };
    void SelectDownloadAccessKey();

    bool FilterChangeSetsBetweenVersions(Utf8StringCR firstVersionId, Utf8StringCR secondVersionId);
    bool FilterChangeSetsAfterVersion(Utf8StringCR versionId);
    bool FilterChangeSetsBetweenVersionAndChangeSet(Utf8StringCR versionId, Utf8StringCR changeSetId);
    bool FilterCumulativeChangeSetsByChangeSetId(Utf8StringCR changeSetId);
    bool FilterCumulativeChangeSetsByVersionId(Utf8StringCR versionId);

public:
    //! Create an instance of bridge properties.
    IMODELHUBCLIENT_EXPORT ChangeSetQuery();

    //! Returns WSQuery for current ChangeSet query
    //! @return WSQuery object
    IMODELHUBCLIENT_EXPORT WSQuery GetWSQuery();

    //! Sets filter to get ChangeSets between two specified ChangeSets.
    //! @param[in] firstChangeSetId If empty gets all changeSets before secondChangeSetId
    //! @param[in] secondChangeSetId If empty gets all changeSets before firstChangeSetId.
    IMODELHUBCLIENT_EXPORT bool FilterChangeSetsBetween(Utf8StringCR firstChangeSetId, Utf8StringCR secondChangeSetId);

    //! Sets filter to get all changeSets after the specific ChangeSetId.
    //! @param[in] changeSetId Id of the parent ChangeSet for the first ChangeSet in the resulting collection. If empty gets all changeSets on server.
    IMODELHUBCLIENT_EXPORT bool FilterChangeSetsAfterId(Utf8StringCR changeSetId);

    //! Sets filter to get a ChangeSet for the specific ChangeSetId.
    //! @param[in] changeSetId Id of the ChangeSet to retrieve.
    IMODELHUBCLIENT_EXPORT bool FilterById(Utf8StringCR changeSetId);

    //! Sets filter to get ChangeSets for the specific ChangeSetIds.
    //! @param[in] changeSetIds Ids of ChangeSets to retrieve.
    IMODELHUBCLIENT_EXPORT bool FilterByIds(std::deque<ObjectId>& changeSetIds);

    //! Sets selector to query bridge properties
    IMODELHUBCLIENT_EXPORT void SelectBridgeProperties();
};

END_BENTLEY_IMODELHUB_NAMESPACE
