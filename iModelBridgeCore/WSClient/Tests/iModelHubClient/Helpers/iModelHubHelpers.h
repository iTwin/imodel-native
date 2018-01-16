/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/iModelHubHelpers.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../Common.h"
#include <WebServices/iModelHub/Client/Client.h>
#include "MockHttpHandler.h"

Utf8String GenerateErrorMessage(iModel::Hub::Error const& e);
#define EXPECT_SUCCESS(x) EXPECT_TRUE(x.IsSuccess()) << GenerateErrorMessage(x.GetError())
#define EXPECT_FAILURE(x) EXPECT_FALSE(x.IsSuccess())
#define EXPECT_RESULT(x, result) if (result) { EXPECT_SUCCESS(x); } else { EXPECT_FAILURE(x); }

#define ASSERT_SUCCESS(x) ASSERT_TRUE(x.IsSuccess()) << GenerateErrorMessage(x.GetError())
#define ASSERT_FAILURE(x) ASSERT_TRUE(!x.IsSuccess())
#define ASSERT_RESULT(x, result) if (result) { ASSERT_SUCCESS(x); } else { ASSERT_FAILURE(x); }

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
namespace iModelHubHelpers
    {
    void CreateClient(ClientPtr& client, CredentialsCR credentials);
    WebServices::IWSRepositoryClientPtr CreateWSClient(iModelInfoPtr imodel, std::shared_ptr<MockHttpHandler> mockHandler);
    void CreateProjectWSClient(IWSRepositoryClientPtr& result, ClientR client, Utf8StringCR projectId);

    iModelResult CreateNewiModel(ClientPtr client, DgnDbPtr db, Utf8StringCR projectId, bool expectSuccess);
    void LockiModel(StatusResult& result, iModelConnectionPtr connection, bool expectSuccess = true);
    void UploadNewSeedFile(FileResult& result, iModelConnectionPtr connection, DgnDbPtr db, bool expectSuccess = true);
    BeSQLite::BeGuid ReplaceSeedFile(iModelConnectionPtr connection, DgnDbPtr db);
    StatusResult DeleteiModelByName(ClientPtr client, Utf8String name);

    BriefcaseInfoResult AcquireBriefcase(ClientPtr client, iModelInfoPtr imodelInfo, bool pull = true, bool expectSuccess = true);
    BriefcaseResult OpenBriefcase(ClientPtr client, DgnDbPtr db, bool pull = false, bool expectSuccess = true);
    BriefcaseResult AcquireAndOpenBriefcase(ClientPtr client, iModelInfoPtr imodelInfo, bool pull = true, bool expectSuccess = true);
    StatusResult AbandonAllBriefcases(ClientPtr client, iModelConnectionPtr connection);

    void ExpectCodesCount(BriefcasePtr briefcase, int expectedCount);
    void ExpectLocksCount(BriefcasePtr briefcase, int expectedCount);
    void ExpectUnavailableCodesCount(BriefcasePtr briefcase, int expectedCount);
    void ExpectUnavailableLocksCount(BriefcasePtr briefcase, int expectedCount);
    void ExpectLocksCountById(BriefcasePtr briefcase, int expectedCount, bool byBriefcaseId, LockableIdSet& ids);
    void ExpectLocksCountById(BriefcasePtr briefcase, int expectedCount, bool byBriefcaseId, LockableId id1, LockableId id2);
    void ExpectLocksCountById(BriefcasePtr briefcase, int expectedCount, bool byBriefcaseId, LockableId id1, LockableId id2, LockableId id3);

    ChangeSetsResult PullMergeAndPush(BriefcasePtr briefcase, bool shouldPush, bool relinquish = true, bool expectSuccess = true);
    StatusResult AddChangeSets(BriefcasePtr briefcase, uint32_t count = 1, uint32_t statingNumber = 0, bool needsPull = false, bool expectSuccess = true);
    void AcquireAndAddChangeSets(ClientPtr client, iModelInfoPtr info, uint32_t count = 1);
    void CreateNamedVersion(VersionInfoPtr& result, iModelConnectionPtr connection, Utf8StringCR name, int index);

    void SetLastPulledChangeSetId(BriefcasePtr briefcase, Utf8StringCR changeSetId);
    ChangeSetInfoPtr GetChangeSetByIndex(iModelConnectionPtr connection, int index);
    ChangeSetInfoPtr GetLastChangeSet(iModelConnectionPtr connection);
    StatusResult UpdateToVersion(BriefcasePtr briefcase, VersionInfoPtr version, bool expectSuccess = true);
    StatusResult UpdateToChangeSet(BriefcasePtr briefcase, ChangeSetInfoPtr changeSet, bool expectSuccess = true);

    IAzureBlobStorageClientPtr CreateAzureClient(std::shared_ptr<MockHttpHandler> mockHandler);
    void ConvertToChangeSetPointersVector(ChangeSets changeSets, bvector<DgnRevisionCP>& pointersVector);
    }
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
