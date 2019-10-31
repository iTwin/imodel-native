/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <OidcNativeClient/OidcNative.h>
#include "iModelHubHelpers.h"
#include "IntegrationTestsSettings.h"
#include "TestsProgressCallback.h"
#include "DgnPlatformHelpers.h"
#include <WebServices/iModelHub/Client/ClientHelper.h>
#include <WebServices/Connect/SimpleConnectTokenProvider.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeStringUtilities.h>
#include <BeHttp/ProxyHttpHandler.h>
#include "../../../iModelHubClient/Utils.h"
#include "Oidc/OidcSignInManager.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_IMODELHUB
using namespace OidcInterop;

#define IMODELHUB_ClientId            "imodel-hub-integration-tests-2485"
#define IMODELHUB_Scope               "openid profile email imodelhub"

Utf8String GenerateErrorMessage(Error const& e)
    {
    Utf8String errorMessage;
    errorMessage.Sprintf("\nError id: %d\nMessage: %s\nDescription: %s\n", (int) e.GetId(), e.GetMessage().c_str(), e.GetDescription().c_str());
    return errorMessage;
    }

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
namespace iModelHubHelpers
    {
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              08/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String GetServerUrl()
        {
        Utf8String serverUrl = IntegrationTestsSettings::Instance().GetServerUrl();
        if (!Utf8String::IsNullOrEmpty(serverUrl.c_str()))
            return serverUrl;
        return UrlProvider::Urls::iModelHubApi.Get();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void CreateClient(ClientPtr& client, CredentialsCR credentials)
        {
        AsyncError error;
        client = ClientHelper::GetInstance()->SignInWithCredentials(&error, credentials);
        ASSERT_TRUE(client.IsValid()) << error.GetMessage().c_str();
        ASSERT_TRUE(!Utf8String::IsNullOrEmpty(client->GetServerUrl().c_str()));
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             10/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String GenerateNewToken(CredentialsCR credentials)
        { 
        return OIDCNative::IssueToken(credentials.GetUsername().c_str(), credentials.GetPassword().c_str(), UrlProvider::Urls::IMSOpenID.Get().c_str(), IMODELHUB_ClientId, IMODELHUB_Scope);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             08/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void CreateOidcClient(ClientPtr& client, CredentialsCR credentials)
        {
        auto securityToken = GenerateNewToken(credentials);
        auto onUpdateToken = [=] { return CreateCompletedAsyncTask<Utf8String>(GenerateNewToken(credentials)); };
        auto oidcTokenProvider = std::make_shared<SimpleConnectTokenProvider>(securityToken, onUpdateToken);

        client = ClientHelper::CreateClient(oidcTokenProvider);
        ASSERT_TRUE(client.IsValid());
        ASSERT_TRUE(!Utf8String::IsNullOrEmpty(client->GetServerUrl().c_str()));
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void CreateContextWSClient(IWSRepositoryClientPtr& result, ClientR client, Utf8StringCR contextId)
        {
        Utf8StringCR serverUrl = GetServerUrl();
        ClientInfoPtr clientInfo = IntegrationTestsSettings::Instance().GetClientInfo();

        Utf8String context;
        context.Sprintf("%s--%s", ServerSchema::Plugin::Context, contextId.c_str());
        result = WSRepositoryClient::Create(serverUrl, ServerProperties::ServiceVersion(), context, clientInfo, nullptr, client.GetHttpHandler());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              03/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    Json::Value iModelCreationJson(Utf8StringCR imodelName, Utf8StringCR description)
        {
        Json::Value iModelCreation(Json::objectValue);
        JsonValueR instance = iModelCreation[ServerSchema::Instance] = Json::objectValue;
        instance[ServerSchema::SchemaName] = ServerSchema::Schema::Context;
        instance[ServerSchema::ClassName] = ServerSchema::Class::iModel;
        JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
        properties[ServerSchema::Property::iModelName] = imodelName;
        properties[ServerSchema::Property::iModelDescription] = description;
        return iModelCreation;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              03/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void CreateUninitializediModel(iModelResult& result, ClientPtr client, Utf8StringCR contextId, Utf8StringCR imodelName)
        {
        IWSRepositoryClientPtr wsClient;
        CreateContextWSClient(wsClient, *client, contextId);

        auto requestOptions = CreateiModelHubRequestOptions();
        ASSERT_SUCCESS(wsClient->SendCreateObjectRequestWithOptions(iModelCreationJson(imodelName, ""), BeFileName(), nullptr, requestOptions)->GetResult());
        result = client->GetiModelByName(contextId, imodelName)->GetResult();
        ASSERT_SUCCESS(result);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    iModelResult CreateNewiModel(ClientCR client, DgnDbR db, Utf8StringCR contextId, bool expectSuccess)
        {
        TestsProgressCallback callback;
        auto createResult = client.CreateNewiModel(contextId, db, true, callback.Get())->GetResult();
        EXPECT_RESULT(createResult, expectSuccess);
        callback.Verify(expectSuccess);
        return createResult;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    iModelResult CreateNewiModel(ClientPtr client, DgnDbPtr db, Utf8StringCR contextId, bool expectSuccess)
        {
        return CreateNewiModel(*client, *db, contextId, expectSuccess);
        }
    
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Vilius.Kazlauskas               09/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    iModelResult CreateNewiModel(ClientCR client, DgnDbR db, Utf8StringCR contextId, iModelCreateInfoPtr imodelCreateInfo, bool expectSuccess)
        {
        TestsProgressCallback callback;
        auto createResult = client.CreateNewiModel(contextId, db, imodelCreateInfo, true, callback.Get())->GetResult();
        EXPECT_RESULT(createResult, expectSuccess);
        callback.Verify(expectSuccess);
        return createResult;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Vilius.Kazlauskas               09/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    iModelResult CreateNewiModel(ClientPtr client, DgnDbPtr db, Utf8StringCR contextId, iModelCreateInfoPtr imodelCreateInfo, bool expectSuccess)
        {
        return CreateNewiModel(*client, *db, contextId, imodelCreateInfo, expectSuccess);
        }
    
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             01/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    iModelResult CreateEmptyiModel(ClientCR client, Utf8StringCR contextId, Utf8StringCR name, Utf8StringCR description, bool expectSuccess)
        {
        auto createResult = client.CreateEmptyiModel(contextId, name, description)->GetResult();
        EXPECT_RESULT(createResult, expectSuccess);
        return createResult;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Vilius.Kazlauskas               09/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    iModelResult CreateEmptyiModel(ClientCR client, Utf8StringCR contextId, iModelCreateInfoPtr imodelCreateInfo, bool expectSuccess)
        {
        auto createResult = client.CreateEmptyiModel(contextId, imodelCreateInfo)->GetResult();
        EXPECT_RESULT(createResult, expectSuccess);
        return createResult;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void LockiModel(StatusResult& result, iModelConnectionPtr connection, bool expectSuccess)
        {
        result = connection->LockiModel()->GetResult();
        ASSERT_RESULT(result, expectSuccess);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void UploadNewSeedFile(FileResult& result, iModelConnectionPtr connection, DgnDbPtr db, bool expectSuccess)
        {
        auto fileName = db->GetFileName();
        ICancellationTokenPtr cancellationToken = SimpleCancellationToken::Create();
        FileInfoPtr fileInfo = FileInfo::Create(*db, "Replacement description1");
        TestsProgressCallback callback;
        result = connection->UploadNewSeedFile(fileName, *fileInfo, true, callback.Get(), cancellationToken)->GetResult();
        ASSERT_RESULT(result, expectSuccess);
        callback.Verify(expectSuccess);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    BeSQLite::BeGuid ReplaceSeedFile(iModelConnectionPtr connection, DgnDbPtr db)
        {
        StatusResult lockResult;
        LockiModel(lockResult, connection, true);
        FileResult result;
        UploadNewSeedFile(result, connection, db, true);
        return result.GetValue()->GetFileId();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusResult DeleteiModelByName(ClientPtr client, Utf8String name, Utf8String contextId)
        {
        iModelResult getResult = client->GetiModelByName(contextId, name)->GetResult();
        if (getResult.IsSuccess())
            {
            StatusResult deleteResult = client->DeleteiModel(contextId, *getResult.GetValue())->GetResult();
            EXPECT_SUCCESS(deleteResult);
            return deleteResult;
            }
        if (Error::Id::iModelDoesNotExist == getResult.GetError().GetId())
            return StatusResult::Success();
        return StatusResult::Error(getResult.GetError());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusResult DeleteiModelByName(ClientPtr client, Utf8String name)
        {
        Utf8String contextId = IntegrationTestsSettings::Instance().GetProjectId();
        return DeleteiModelByName(client, name, contextId);
        }
    
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    BriefcaseInfoResult AcquireBriefcase(ClientCR client, iModelInfoCR imodelInfo, bool pull, bool expectSuccess)
        {
        BeFileName outputRoot;
        BeTest::GetHost().GetOutputRoot(outputRoot);
        TestsProgressCallback callback;
        BriefcaseInfoResult acquireResult = client.AcquireBriefcaseToDir(imodelInfo, outputRoot, pull, Client::DefaultFileNameCallback, callback.Get())->GetResult();
        EXPECT_RESULT(acquireResult, expectSuccess);
        if (pull)
            {
            callback.Verify(expectSuccess);
            }
        return acquireResult;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    BriefcaseInfoResult AcquireBriefcase(ClientPtr client, iModelInfoPtr imodelInfo, bool pull, bool expectSuccess)
        {
        return AcquireBriefcase(*client, *imodelInfo, pull, expectSuccess);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    BriefcaseResult OpenBriefcase(ClientCR client, DgnDbPtr db, bool pull, bool expectSuccess)
        {
        TestsProgressCallback callback;
        BriefcaseResult briefcaseResult = client.OpenBriefcase(db, pull, callback.Get())->GetResult();
        EXPECT_RESULT(briefcaseResult, expectSuccess);
        if (pull)
            {
            callback.Verify(expectSuccess);
            }
        else if (expectSuccess)
            {
            callback.Verify(false);
            }
        return briefcaseResult;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    BriefcaseResult OpenBriefcase(ClientPtr client, DgnDbPtr db, bool pull, bool expectSuccess)
        {
        return OpenBriefcase(*client, db, pull, expectSuccess);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    BriefcaseResult AcquireAndOpenBriefcase(ClientCR client, iModelInfoCR imodelInfo, bool pull, bool expectSuccess)
        {
        BriefcaseInfoResult acquireResult = AcquireBriefcase(client, imodelInfo, pull, expectSuccess);
        if (!acquireResult.IsSuccess())
            return BriefcaseResult::Error(acquireResult.GetError());
        DgnDbPtr db = nullptr;
        OpenDgnDb(db, acquireResult.GetValue()->GetLocalPath());
        return OpenBriefcase(client, db, false, expectSuccess);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    BriefcaseResult AcquireAndOpenBriefcase(ClientPtr client, iModelInfoPtr imodelInfo, bool pull, bool expectSuccess)
        {
        return AcquireAndOpenBriefcase(*client, *imodelInfo, pull, expectSuccess);
        }
    
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusResult AbandonAllBriefcases(ClientCR client, iModelConnectionCR connection)
        {
        bset<StatusTaskPtr> tasks;
        BriefcasesInfoResult queryResult = connection.QueryAllBriefcasesInfo()->GetResult();
        EXPECT_SUCCESS(queryResult);
        if (!queryResult.IsSuccess())
            {
            return StatusResult::Error(queryResult.GetError());
            }
        for (BriefcaseInfoPtr briefcase : queryResult.GetValue())
            {
            tasks.insert(client.AbandonBriefcase(connection.GetiModelInfo(), briefcase->GetId()));
            }
        PackagedAsyncTask<void>::WhenAll(tasks)->Wait();
        for (StatusTaskPtr task : tasks)
            {
            StatusResult taskResult = task->GetResult();
            EXPECT_SUCCESS(taskResult);
            if (!taskResult.IsSuccess())
                {
                return taskResult;
                }
            }
        return StatusResult::Success();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusResult AbandonAllBriefcases(ClientPtr client, iModelConnectionPtr connection)
        {
        return AbandonAllBriefcases(*client, *connection);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     Benas.Kikutis             01/2018
    //---------------------------------------------------------------------------------------
    CodeInfoSetTaskPtr QueryCodesById(BriefcaseR briefcase, bool byBriefcaseId, DgnCodeSet& codes)
        {
        if (byBriefcaseId)
            return briefcase.GetiModelConnection().QueryCodesByIds(codes, briefcase.GetBriefcaseId());

        return briefcase.GetiModelConnection().QueryCodesByIds(codes);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     Benas.Kikutis             01/2018
    //---------------------------------------------------------------------------------------
    LockInfoSetTaskPtr QueryLocksById(BriefcaseR briefcase, bool byBriefcaseId, LockableIdSet& ids)
        {
        if (byBriefcaseId)
            return briefcase.GetiModelConnection().QueryLocksByIds(ids, briefcase.GetBriefcaseId());

        return briefcase.GetiModelConnection().QueryLocksByIds(ids);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             06/2016
    //---------------------------------------------------------------------------------------
    void ExpectCodesCount(BriefcaseR briefcase, int expectedCount)
        {
        auto result = briefcase.GetiModelConnection().QueryCodesLocks(briefcase.GetBriefcaseId())->GetResult();
        ASSERT_SUCCESS(result);
        auto actualCount = result.GetValue().GetCodes().size();
        EXPECT_EQ(expectedCount, actualCount);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExpectCodesCount(BriefcasePtr briefcase, int expectedCount)
        {
        ExpectCodesCount(*briefcase, expectedCount);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Benas.Kikutis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExpectCodesCountByIds(BriefcaseR briefcase, int expectedCount, bool byBriefcaseId, DgnCodeSet& codes)
        {
        auto result = QueryCodesById(briefcase, byBriefcaseId, codes)->GetResult();

        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().size();
        EXPECT_EQ(expectedCount, actualCount);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     Eligijus.Mauragas             01/2016
    //---------------------------------------------------------------------------------------
    void ExpectLocksCount(BriefcaseR briefcase, int expectedCount)
        {
        auto result = briefcase.GetiModelConnection().QueryCodesLocks(briefcase.GetBriefcaseId())->GetResult();
        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().GetLocks().size();
        EXPECT_EQ(expectedCount, actualCount);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExpectLocksCount(BriefcasePtr briefcase, int expectedCount)
        {
        ExpectLocksCount(*briefcase, expectedCount);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                    Karolis.Dziedzelis             06/2016
    //---------------------------------------------------------------------------------------
    void ExpectUnavailableCodesCount(BriefcaseR briefcase, int expectedCount)
        {
        auto result = briefcase.GetiModelConnection().QueryUnavailableCodesLocks(briefcase.GetBriefcaseId(), briefcase.GetLastChangeSetPulled())->GetResult();
        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().GetCodes().size();
        EXPECT_EQ(expectedCount, actualCount);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExpectUnavailableCodesCount(BriefcasePtr briefcase, int expectedCount)
        {
        ExpectUnavailableCodesCount(*briefcase, expectedCount);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                    Karolis.Dziedzelis             06/2016
    //---------------------------------------------------------------------------------------
    void ExpectUnavailableLocksCount(BriefcaseR briefcase, int expectedCount)
        {
        auto result = briefcase.GetiModelConnection().QueryUnavailableCodesLocks(briefcase.GetBriefcaseId(), briefcase.GetLastChangeSetPulled())->GetResult();
        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().GetLocks().size();
        EXPECT_EQ(expectedCount, actualCount);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExpectUnavailableLocksCount(BriefcasePtr briefcase, int expectedCount)
        {
        ExpectUnavailableLocksCount(*briefcase, expectedCount);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     Eligijus.Mauragas             01/2016
    //---------------------------------------------------------------------------------------
    void ExpectLocksCountById(BriefcaseR briefcase, int expectedCount, bool byBriefcaseId, LockableIdSet& ids)
        {
        auto result = QueryLocksById(briefcase, byBriefcaseId, ids)->GetResult();

        EXPECT_SUCCESS(result);

        int locksCount = 0;
        for (DgnLockInfo lockState : result.GetValue())
            {
            if (LockLevel::Exclusive == lockState.GetOwnership().GetLockLevel())
                locksCount++;
            else
                {
                for (auto owner : lockState.GetOwnership().GetSharedOwners())
                    locksCount++;
                }
            }
        EXPECT_TRUE(expectedCount == locksCount);
        }
    
    //---------------------------------------------------------------------------------------
    //@bsimethod                                     Eligijus.Mauragas             01/2016
    //---------------------------------------------------------------------------------------
    void ExpectLocksCountByLevelAndId(BriefcasePtr briefcase, int expectedCount, bool byBriefcaseId, LockableIdSet& ids, LockLevel expectedLevel)
        {
        auto result = QueryLocksById(*briefcase, byBriefcaseId, ids)->GetResult();

        EXPECT_SUCCESS(result);

        int locksCount = 0;
        for (DgnLockInfo lockState : result.GetValue())
            {
            if (expectedLevel == LockLevel::Exclusive)
                {
                if (expectedLevel == lockState.GetOwnership().GetLockLevel())
                    locksCount++;
                }
            else
                {
                for (auto owner : lockState.GetOwnership().GetSharedOwners())
                    if (expectedLevel == lockState.GetOwnership().GetLockLevel())
                        locksCount++;
                }
            }
        EXPECT_TRUE(expectedCount == locksCount);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExpectLocksCountById(BriefcasePtr briefcase, int expectedCount, bool byBriefcaseId, LockableIdSet& ids)
        {
        ExpectLocksCountById(*briefcase, expectedCount, byBriefcaseId, ids);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     Eligijus.Mauragas             01/2016
    //---------------------------------------------------------------------------------------
    void ExpectLocksCountById(BriefcaseR briefcase, int expectedCount, bool byBriefcaseId, LockableId id1, LockableId id2)
        {
        LockableIdSet ids;
        ids.insert(id1);
        ids.insert(id2);
        ExpectLocksCountById(briefcase, expectedCount, byBriefcaseId, ids);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExpectLocksCountById(BriefcasePtr briefcase, int expectedCount, bool byBriefcaseId, LockableId id1, LockableId id2)
        {
        ExpectLocksCountById(*briefcase, expectedCount, byBriefcaseId, id1, id2);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     Eligijus.Mauragas             01/2016
    //---------------------------------------------------------------------------------------
    void ExpectLocksCountById(BriefcaseR briefcase, int expectedCount, bool byBriefcaseId, LockableId id1, LockableId id2, LockableId id3)
        {
        LockableIdSet ids;
        ids.insert(id1);
        ids.insert(id2);
        ids.insert(id3);
        ExpectLocksCountById(briefcase, expectedCount, byBriefcaseId, ids);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExpectLocksCountById(BriefcasePtr briefcase, int expectedCount, bool byBriefcaseId, LockableId id1, LockableId id2, LockableId id3)
        {
        ExpectLocksCountById(*briefcase, expectedCount, byBriefcaseId, id1, id2, id3);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ChangeSetsResult PullMergeAndPush(BriefcaseR briefcase, bool shouldPush, bool shouldPull, bool relinquish, bool expectSuccess)
        {
        TestsProgressCallback pushCallback;
        TestsProgressCallback pullCallback;

        auto pushResult = briefcase.PullMergeAndPush(nullptr, relinquish, pullCallback.Get(), pushCallback.Get())->GetResult();
        EXPECT_RESULT(pushResult, expectSuccess);
        pullCallback.Verify(shouldPull);
        pushCallback.Verify(shouldPush);
        return pushResult;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             10/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    ChangeSetsResult PullMergeAndPush(BriefcaseR briefcase, PullChangeSetsArgumentsPtr pullArguments, PushChangeSetArgumentsPtr pushArguments)
        {
        auto pushResult = briefcase.PullMergeAndPush(pullArguments, pushArguments)->GetResult();
        EXPECT_RESULT(pushResult, true);
        return pushResult;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ChangeSetsResult PullMergeAndPush(BriefcasePtr briefcase, bool shouldPush, bool shouldPull, bool relinquish, bool expectSuccess)
        {
        return PullMergeAndPush(*briefcase, shouldPush, shouldPull, relinquish, expectSuccess);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String Utf8GuidString(Utf8CP format)
        {
        return Utf8PrintfString(format, BeSQLite::BeGuid(true).ToString().c_str());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusResult AddChangeSets(BriefcaseR briefcase, uint32_t count, uint32_t startingNumber, bool needsPull, bool expectSuccess)
        {
        DgnModelPtr model = CreateModel(Utf8GuidString("AddChangeSetsModel_%s").c_str(), briefcase.GetDgnDb());
        for (uint32_t i = startingNumber; i < startingNumber + count; ++i)
            {
            CreateElement(*model);
            ChangeSetsResult result = PullMergeAndPush(briefcase, expectSuccess, false, true, expectSuccess);
            if (!result.IsSuccess())
                {
                return StatusResult::Error(result.GetError());
                }
            }
        return StatusResult::Success();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusResult AddChangeSets(BriefcasePtr briefcase, uint32_t count, uint32_t statingNumber, bool needsPull, bool expectSuccess)
        {
        return AddChangeSets(*briefcase, count, statingNumber, needsPull, expectSuccess);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    BriefcasePtr AcquireAndAddChangeSets(ClientPtr client, iModelInfoPtr info, uint32_t count)
        {
        BriefcaseResult acquireResult = AcquireAndOpenBriefcase(client, info, true, true);
        EXPECT_SUCCESS(acquireResult);
        BriefcasePtr briefcase = acquireResult.GetValue();
        EXPECT_SUCCESS(AddChangeSets(briefcase, count, true));
        return briefcase;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             01/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    void PushDefaultView(ClientPtr client, iModelInfoPtr info)
        {
        // Create model and insert view
        BriefcaseResult acquireResult = AcquireAndOpenBriefcase(client, info, true, true);
        EXPECT_SUCCESS(acquireResult);
        BriefcasePtr briefcase = acquireResult.GetValue();

        PhysicalModelPtr model = CreateModel("DefaultModel", briefcase->GetDgnDb());
        EXPECT_TRUE(model.IsValid());
        InsertSpatialView(*model, "DefaultView");
        briefcase->GetDgnDb().SaveChanges();

        ChangeSetsResult pushResult = PullMergeAndPush(briefcase, true, false);
        EXPECT_SUCCESS(pushResult);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              12/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void CreateNamedVersion(VersionInfoPtr& result, iModelConnectionCR connection, Utf8StringCR name, int index)
        {
        auto changeSetsResult = connection.GetAllChangeSets()->GetResult();
        ASSERT_SUCCESS(changeSetsResult);
        auto changeSets = changeSetsResult.GetValue();

        VersionInfoPtr version = new VersionInfo(name, "Description", index > 0 ? changeSets.at(index - 1)->GetId() : "");
        VersionInfoResult versionResult = connection.GetVersionsManager().CreateVersion(*version)->GetResult();
        ASSERT_SUCCESS(versionResult);
        result = versionResult.GetValue();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void CreateNamedVersion(VersionInfoPtr& result, iModelConnectionPtr connection, Utf8StringCR name, int index)
        {
        CreateNamedVersion(result, *connection, name, index);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Andrius.Zonys                   09/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    VersionInfoPtr GetVersionByChangeSetId(iModelConnectionPtr connection, Utf8StringCR changeSetId)
        {
        bvector<VersionInfoPtr> versions = connection->GetVersionsManager().GetAllVersions()->GetResult().GetValue();
        for (int i = 0; i < versions.size(); i++)
            {
            if (versions[i]->GetChangeSetId() == changeSetId)
                return versions[i];
            }

        return nullptr;
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     Eligijus.Mauragas             01/2016
    //---------------------------------------------------------------------------------------
    void SetLastPulledChangeSetId(BriefcaseR briefcase, Utf8StringCR changeSetId)
        {
        briefcase.GetDgnDb().SaveBriefcaseLocalValue("ParentChangeSetId", changeSetId);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetLastPulledChangeSetId(BriefcasePtr briefcase, Utf8StringCR changeSetId)
        {
        SetLastPulledChangeSetId(*briefcase, changeSetId);
        }
    
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              12/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void QueryChangeSets(ChangeSetsInfoResult& result, iModelConnectionCR connection)
        {
        result = connection.GetAllChangeSets()->GetResult();
        ASSERT_SUCCESS(result);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              12/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ChangeSetInfoPtr GetChangeSetByIndex(iModelConnectionCR connection, int index)
        {
        ChangeSetsInfoResult changeSetsResult;
        QueryChangeSets(changeSetsResult, connection);
        return changeSetsResult.GetValue().at(index - 1);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ChangeSetInfoPtr GetChangeSetByIndex(iModelConnectionPtr connection, int index)
        {
        return GetChangeSetByIndex(*connection, index);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              12/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ChangeSetInfoPtr GetLastChangeSet(iModelConnectionCR connection)
        {
        ChangeSetsInfoResult changeSetsResult;
        QueryChangeSets(changeSetsResult, connection);
        return changeSetsResult.GetValue().back();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ChangeSetInfoPtr GetLastChangeSet(iModelConnectionPtr connection)
        {
        return GetLastChangeSet(*connection);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              12/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusResult UpdateToVersion(BriefcaseCR briefcase, VersionInfoCR version, bool expectSuccess)
        {
        Utf8String expectedChangeSetId = expectSuccess ? version.GetChangeSetId() : briefcase.GetLastChangeSetPulled();
        TestsProgressCallback callback;
        StatusResult result = briefcase.UpdateToVersion(version.GetId(), callback.Get())->GetResult();
        EXPECT_RESULT(result, expectSuccess);
        callback.Verify();
        EXPECT_EQ(expectedChangeSetId, briefcase.GetLastChangeSetPulled());
        return result;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusResult UpdateToVersion(BriefcasePtr briefcase, VersionInfoPtr version, bool expectSuccess)
        {
        return UpdateToVersion(*briefcase, *version, expectSuccess);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              12/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusResult UpdateToChangeSet(BriefcaseCR briefcase, ChangeSetInfoCR changeSet, bool expectSuccess)
        {
        Utf8String expectedChangeSetId = expectSuccess ? changeSet.GetId() : briefcase.GetLastChangeSetPulled();
        TestsProgressCallback callback;
        StatusResult result = briefcase.UpdateToChangeSet(changeSet.GetId(), callback.Get())->GetResult();
        EXPECT_RESULT(result, expectSuccess);
        callback.Verify();
        EXPECT_EQ(expectedChangeSetId, briefcase.GetLastChangeSetPulled());
        return result;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusResult UpdateToChangeSet(BriefcasePtr briefcase, ChangeSetInfoPtr changeSet, bool expectSuccess)
        {
        return UpdateToChangeSet(*briefcase, *changeSet, expectSuccess);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             09/2016
    //---------------------------------------------------------------------------------------
    WebServices::IWSRepositoryClientPtr CreateWSClient(iModelInfoPtr imodel, std::shared_ptr<MockHttpHandler> mockHandler)
        {
        WebServices::ClientInfoPtr clientInfo = IntegrationTestsSettings::Instance().GetClientInfo();
        return WebServices::WSRepositoryClient::Create(imodel->GetServerURL(), ServerProperties::ServiceVersion(), imodel->GetWSRepositoryName(), clientInfo, nullptr, mockHandler);
        }
    
    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             12/2017
    //---------------------------------------------------------------------------------------
    IAzureBlobStorageClientPtr CreateAzureClient(std::shared_ptr<MockHttpHandler> mockHandler)
        {
        return AzureBlobStorageClient::Create(mockHandler);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     Algirdas.Mikoliunas             06/2017
    //---------------------------------------------------------------------------------------
    void ConvertToChangeSetPointersVector(ChangeSets changeSets, bvector<DgnRevisionCP>& pointersVector)
        {
        pointersVector.clear();
        for (auto changeSetPtr : changeSets)
            {
            pointersVector.push_back(changeSetPtr.get());
            }
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             12/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    IWSRepositoryClient::RequestOptionsPtr CreateiModelHubRequestOptions()
        {
        auto requestOptions = std::make_shared<IWSRepositoryClient::RequestOptions>();

        requestOptions->GetActivityOptions()->SetHeaderName(IWSRepositoryClient::ActivityOptions::HeaderName::XCorrelationId);
        if (!requestOptions->GetActivityOptions()->HasActivityId())
            {
            BeSQLite::BeGuid id = BeSQLite::BeGuid(true);
            requestOptions->GetActivityOptions()->SetActivityId(id.ToString());
            }

        return requestOptions;
        }
    }
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
