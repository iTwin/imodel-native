/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/CachingDataSourceTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CachingDataSourceTests.h"

#include <WebServices/Cache/Persistence/DataSourceCache.h>
#include <WebServices/Cache/Persistence/DataReadOptions.h>
#include <WebServices/Cache/SyncNotifier.h>
#include <Bentley/BeDebugLog.h>
#include "MockCachingDataSource.h"
#include "MockQueryProvider.h"

#ifdef USE_GTEST
USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

#define EXPECT_PROGRESS_EQ(expectedProgress, actualProgress)                    \
    EXPECT_EQ(expectedProgress.GetBytes(), actualProgress.GetBytes());          \
    EXPECT_EQ(expectedProgress.GetInstances(), actualProgress.GetInstances());  \
    EXPECT_NEAR(expectedProgress.GetSynced(), actualProgress.GetSynced(), 0.01);

CachedResponseKey CreateTestResponseKey(ICachingDataSourcePtr ds, Utf8StringCR rootName = "StubResponseKeyRoot", Utf8StringCR keyName = BeGuid(true).ToString())
    {
    auto txn = ds->StartCacheTransaction();
    CachedResponseKey key(txn.GetCache().FindOrCreateRoot(rootName.c_str()), keyName);
    txn.Commit();
    return key;
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_CalledSecondTimeAfterCacheWasCreated_OpensAndSucceeds)
// TODO: fix test - crashes as first cache is still closing when second one is opening.
    {
    BeFileName path = StubFilePath();
    auto client = MockWSRepositoryClient::Create();

    StubInstances schemaDefs;
    schemaDefs.Add({"MetaSchema.ECSchemaDef", "TestSchema"}, {{"Name", "TestSchema"}, {"VersionMajor", 1}, {"VersionMinor", 0}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemaDefs.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "TestSchema"), ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        GetTestSchema()->WriteToXmlFile(filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));

    // Create
    auto ds1 = CachingDataSource::OpenOrCreate(client, path, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_TRUE(nullptr != ds1);

    // Ensure that cache is closed
    ds1->Close();
    ds1 = nullptr;

    // Open
    auto ds2 = CachingDataSource::OpenOrCreate(client, path, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_TRUE(nullptr != ds2);
    EXPECT_TRUE(nullptr != ds2->StartCacheTransaction().GetCache().GetAdapter().GetECSchema("TestSchema"));
    }
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_NonECDbFileExists_Error)
    {
    BeFileName path = StubFile("NotECDbFileContents");

    auto client = MockWSRepositoryClient::Create();
    auto result = CachingDataSource::OpenOrCreate(client, path, StubCacheEnvironemnt())->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    }
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_NonDataSourceCacheDbExists_OpensAndStartsUpdatingWithRemoteSchemas)
    {
    BeFileName path = StubFilePath();

    ECDb db;
    db.CreateNewDb(path);

    auto client = MockWSRepositoryClient::Create();

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error({}))));

    CachingDataSource::OpenOrCreate(client, path, StubCacheEnvironemnt())->Wait();
    }
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_DataSourceCacheDbExists_StartsUpdatingWithRemoteSchemas)
    {
    BeFileName path = StubFilePath();

    DataSourceCache db;
    ASSERT_EQ(SUCCESS, db.Create(path, StubCacheEnvironemnt()));
    ASSERT_EQ(SUCCESS, db.Close());

    auto client = MockWSRepositoryClient::Create();
    auto token = SimpleCancellationToken::Create();

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error({}))));

    CachingDataSource::OpenOrCreate(client, path, StubCacheEnvironemnt(), nullptr, token)->Wait();
    }
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_NoFileAndCancelled_ReturnsCancellationError)
    {
    BeFileName path = StubFilePath();
    auto client = MockWSRepositoryClient::Create();

    auto token = SimpleCancellationToken::Create();
    token->SetCanceled();

    auto result = CachingDataSource::OpenOrCreate(client, path, StubCacheEnvironemnt(), nullptr, token)->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::Status::Canceled, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_CancelledAndSameThreadUsedInThenTask_DoesNotHang)
    {
    BeFileName path = StubFilePath();
    auto client = MockWSRepositoryClient::Create();
    auto token = SimpleCancellationToken::Create();

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Invoke([&] (Utf8StringCR, ICancellationTokenPtr)
        {
        token->SetCanceled();
        return CreateCompletedAsyncTask(WSObjectsResult::Error({}));
        }));

    auto thread = WorkerThread::Create("TestCache");
    auto result = CachingDataSource::OpenOrCreate(client, path, StubCacheEnvironemnt(), thread, token)
        ->Then<CachingDataSource::OpenResult>(thread, [=] (CachingDataSource::OpenResult result)
        {
        return result;
        })->GetResult();
        EXPECT_FALSE(result.IsSuccess());
        EXPECT_EQ(ICachingDataSource::Status::Canceled, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_FileExistsAndCancelled_ReturnsCancellationError)
    {
    BeFileName path = StubFilePath();
    auto client = MockWSRepositoryClient::Create();

    DataSourceCache db;
    ASSERT_EQ(SUCCESS, db.Create(path, StubCacheEnvironemnt()));
    ASSERT_EQ(SUCCESS, db.Close());

    auto token = SimpleCancellationToken::Create();
    token->SetCanceled();

    auto result = CachingDataSource::OpenOrCreate(client, path, StubCacheEnvironemnt(), nullptr, token)->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::Status::Canceled, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_ServerDoesNotReturnMetaSchema_GetsSchemasAndImportsThemWithMetaSchema)
    {
    auto client = MockWSRepositoryClient::Create();

    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "SchemaId"}, {{"Name", "UserSchema"}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("MetaSchema.ECSchemaDef", "SchemaId"), objectId);
        Utf8String schemaXml(
            R"(<ECSchema schemaName="UserSchema" nameSpacePrefix="US" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
               </ECSchema>)");
        SimpleWriteToFile(schemaXml, filePath);
        return CreateCompletedAsyncTask(WSFileResult::Success(WSFileResponse(filePath, HttpStatus::OK, "")));
        }));

    auto result = CachingDataSource::OpenOrCreate(client, BeFileName(":memory:"), StubCacheEnvironemnt())->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(nullptr != result.GetValue());

    auto txn = result.GetValue()->StartCacheTransaction();
    EXPECT_TRUE(nullptr != txn.GetCache().GetAdapter().GetECSchema("MetaSchema"));
    EXPECT_TRUE(nullptr != txn.GetCache().GetAdapter().GetECSchema("UserSchema"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_ServerRetursMetaSchema_GetsAllSchemasFromServer)
    {
    auto client = MockWSRepositoryClient::Create();

    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "MetaSchemaId"}, {{"Name", "MetaSchema"}});
    schemas.Add({"MetaSchema.ECSchemaDef", "TestSchemaId"}, {{"Name", "TestSchema"}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(2)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("MetaSchema.ECSchemaDef", "MetaSchemaId"), objectId);
        return CreateCompletedAsyncTask(WSFileResult());
        }));

    CachingDataSource::OpenOrCreate(client, BeFileName(":memory:"), StubCacheEnvironemnt())->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_ServerRetursUserAndDeprecatedSchemas_GetsAllSchemasFromServerButSkipsDeprecated)
    {
    auto client = MockWSRepositoryClient::Create();

    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "A"}, {{"Name", "Contents"},{"NameSpacePrefix", "rest_cnt"}});
    schemas.Add({"MetaSchema.ECSchemaDef", "B"}, {{"Name", "Views"}, {"NameSpacePrefix", "rest_view"}});

    schemas.Add({"MetaSchema.ECSchemaDef", "C"}, {{"Name", "Contents"},{"NameSpacePrefix", "foo"}});
    schemas.Add({"MetaSchema.ECSchemaDef", "D"}, {{"Name", "Views"},{"NameSpacePrefix", "foo"}});
    schemas.Add({"MetaSchema.ECSchemaDef", "E"}, {{"Name", "TestSchema"}, {"NameSpacePrefix", "foo"}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse()))));
    
    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "C"), ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSFileResult(StubFile(StubSchemaXml("Contents"), "Contents.01.00.ecschema.xml")))));
    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "D"), ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSFileResult(StubFile(StubSchemaXml("Views"), "Views.01.00.ecschema.xml")))));
    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "E"), ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSFileResult(StubFile(StubSchemaXml("TestSchema"), "TestSchema.01.00.ecschema.xml")))));
    
    auto ds = CachingDataSource::OpenOrCreate(client, BeFileName(":memory:"), StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_TRUE(nullptr != ds);

    auto txn = ds->StartCacheTransaction();
    auto schemasKeysList = ds->GetRepositorySchemaKeys(txn);
    auto schemasList = ds->GetRepositorySchemas(txn);
    EXPECT_EQ(schemasKeysList.size(), 3);
    EXPECT_EQ(schemasList.size(), 3);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, Close_Opened_SucceedsAndTransactionsCannotBeStarted)
    {
    auto client = MockWSRepositoryClient::Create();

    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "SchemaId"}, {{"Name", "UserSchema"}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("MetaSchema.ECSchemaDef", "SchemaId"), objectId);
        Utf8String schemaXml(
            R"(<ECSchema schemaName="UserSchema" nameSpacePrefix="US" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
               </ECSchema>)");
        SimpleWriteToFile(schemaXml, filePath);
        return CreateCompletedAsyncTask(WSFileResult::Success(WSFileResponse(filePath, HttpStatus::OK, "")));
        }));

    auto ds = CachingDataSource::OpenOrCreate(client, BeFileName(":memory:"), StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_NE(nullptr, ds);

    ASSERT_TRUE(ds->StartCacheTransaction().IsActive());

    ds->Close();

    BeTest::SetFailOnAssert(false);
    ASSERT_FALSE(ds->StartCacheTransaction().IsActive());
    BeTest::SetFailOnAssert(true);

    ds->Close();
    ds->Close();
    ds->Close();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, Close_Opened_CancelsServerRequest)
    {
    auto client = MockWSRepositoryClient::Create();

    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "SchemaId"}, {{"Name", "UserSchema"}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("MetaSchema.ECSchemaDef", "SchemaId"), objectId);
        Utf8String schemaXml(
            R"(<ECSchema schemaName="UserSchema" nameSpacePrefix="US" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                    <ECClass typeName="TestClass"/>
               </ECSchema>)");
        SimpleWriteToFile(schemaXml, filePath);
        return CreateCompletedAsyncTask(WSFileResult::Success(WSFileResponse(filePath, HttpStatus::OK, "")));
        }));

    auto ds = CachingDataSource::OpenOrCreate(client, BeFileName(":memory:"), StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_NE(nullptr, ds);

    ICancellationTokenPtr token;
    AsyncTestCheckpoint cp;
    EXPECT_CALL(*client, SendGetObjectRequest(_, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, Utf8String, ICancellationTokenPtr ct)
        {
        token = ct;
        cp.CheckinAndWait();
        return CreateCompletedAsyncTask(WSObjectsResult::Error({}));
        }));

    auto task = ds->GetObject({"UserSchema.TestClass", "Foo"}, ICachingDataSource::DataOrigin::RemoteData);
    cp.WaitUntilReached();
    cp.Continue();

    ds->GetCacheAccessThread()->ExecuteAsync([] { BeThreadUtilities::BeSleep(100); });
    ds->GetCacheAccessThread()->ExecuteAsync([] { BeThreadUtilities::BeSleep(100); });
    ds->GetCacheAccessThread()->ExecuteAsync([] { BeThreadUtilities::BeSleep(100); });

    ds->Close();

    task->Wait();
    ASSERT_EQ(0, ds->GetCacheAccessThread()->GetQueueTaskCount());
    EXPECT_TRUE(token->IsCanceled());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, FailedObjects_AppendFailures_ItemsAddedAtTheEnd)
    {
    ICachingDataSource::FailedObjects failedObjects;
    failedObjects.push_back({{"Schema.Class", "A"}, "ItemA", ICachingDataSource::Error()});
    EXPECT_THAT(failedObjects, SizeIs(1));

    ICachingDataSource::FailedObjects moreFailedObjects;
    moreFailedObjects.push_back({{"Schema.Class", "B"}, "ItemB", ICachingDataSource::Error()});
    moreFailedObjects.push_back({{"Schema.Class", "C"}, "ItemC", ICachingDataSource::Error()});
    EXPECT_THAT(moreFailedObjects, SizeIs(2));

    failedObjects.AppendFailures(moreFailedObjects);
    EXPECT_THAT(failedObjects, SizeIs(3));
    EXPECT_EQ(failedObjects.at(2).GetObjectLabel(), "ItemC");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, UpdateSchemas_CacheCreatedWithRemoteSchemas_UsesETagsForRequests)
    {
    auto client = MockWSRepositoryClient::Create();

    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "SchemaId"}, {{"Name", "UserSchema"}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(2)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse("SchemaListETag")))))
        .WillOnce(Invoke([&] (Utf8StringCR eTag, ICancellationTokenPtr)
        {
        EXPECT_EQ("SchemaListETag", eTag);
        return CreateCompletedAsyncTask(StubWSObjectsResultNotModified());
        }));

    EXPECT_CALL(*client, SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(2)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("MetaSchema.ECSchemaDef", "SchemaId"), objectId);
        Utf8String schemaXml(
            R"(<ECSchema schemaName="UserSchema" nameSpacePrefix="US" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                       </ECSchema>)");
        SimpleWriteToFile(schemaXml, filePath);
        return CreateCompletedAsyncTask(WSFileResult::Success(WSFileResponse(filePath, HttpStatus::OK, "SchemaFileETag")));
        }))
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR, Utf8StringCR eTag, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
            {
            EXPECT_EQ(ObjectId("MetaSchema.ECSchemaDef", "SchemaId"), objectId);
            EXPECT_EQ("SchemaFileETag", eTag);
            return CreateCompletedAsyncTask(WSFileResult::Success(StubWSFileResponseNotModified()));
            }));

        auto ds = CachingDataSource::OpenOrCreate(client, BeFileName(":memory:"), StubCacheEnvironemnt())->GetResult().GetValue();
        ASSERT_TRUE(nullptr != ds);

        auto result = ds->UpdateSchemas(nullptr)->GetResult();
        ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, UpdateSchemas_CacheCreatedWithLocalSchema_QueriesServerForRemoteSchemas)
    {
    auto ds = GetTestDataSourceV2();

    // Act & Assert
    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "SchemaId"}, {{"Name", "TestSchema"}});

    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(GetMockClient(), SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse("SchemaListETag")))));

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("MetaSchema.ECSchemaDef", "SchemaId"), objectId);
        Utf8String schemaXml;
        GetTestSchema()->WriteToXmlString(schemaXml);
        SimpleWriteToFile(schemaXml, filePath);
        return CreateCompletedAsyncTask(WSFileResult::Success(WSFileResponse(filePath, HttpStatus::OK, "SchemaFileETag")));
        }));

    auto result = ds->UpdateSchemas(nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, UpdateSchemas_SchemaWithReferancedSchema_ImportsBothSchemas)
    {
    auto ds = GetTestDataSourceV2();

    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "A"}, {{"Name", "SchemaWithReferance"}, {"VersionMajor", 1}, {"VersionMinor", 0}});
    schemas.Add({"MetaSchema.ECSchemaDef", "B"}, {{"Name", "ReferancedSchema"}, {"VersionMajor", 1}, {"VersionMinor", 456}});

    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(GetMockClient(), SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse()))));

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "A"), ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        Utf8String schemaXml =
            R"( <ECSchema schemaName="SchemaWithReferance" nameSpacePrefix="A" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                    <ECSchemaReference name="ReferancedSchema" version="1.456" prefix="B" />
                </ECSchema>)";
        SimpleWriteToFile(schemaXml, filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));
    EXPECT_CALL(GetMockClient(), SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "B"), ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        Utf8String schemaXml =
            R"( <ECSchema schemaName="ReferancedSchema" nameSpacePrefix="B" version="1.456" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                </ECSchema>)";
        SimpleWriteToFile(schemaXml, filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));

    // Act
    auto result = ds->UpdateSchemas(nullptr)->GetResult();

    // Assert
    ASSERT_TRUE(result.IsSuccess());

    ASSERT_TRUE(nullptr != ds->StartCacheTransaction().GetCache().GetAdapter().GetECSchema("SchemaWithReferance"));
    ASSERT_TRUE(nullptr != ds->StartCacheTransaction().GetCache().GetAdapter().GetECSchema("ReferancedSchema"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, UpdateSchemas_NewSchemaWithExistingReferancedSchema_ImportsNewSchema)
    {
    auto ds = GetTestDataSourceV2();

    // Initial schema
    StubInstances schemas1;
    schemas1.Add({"MetaSchema.ECSchemaDef", "B"}, {{"Name", "ReferancedSchema"}, {"VersionMajor", 1}, {"VersionMinor", 0}});

    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(GetMockClient(), SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas1.ToWSObjectsResponse()))));

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "B"), ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        Utf8String schemaXml =
            R"( <ECSchema schemaName="ReferancedSchema" nameSpacePrefix="B" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                </ECSchema>)";
        SimpleWriteToFile(schemaXml, filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));

    ASSERT_TRUE(ds->UpdateSchemas(nullptr)->GetResult().IsSuccess());

    // Test updated
    StubInstances schemas2;
    schemas2.Add({"MetaSchema.ECSchemaDef", "A"}, {{"Name", "SchemaWithReferance"}, {"VersionMajor", 1}, {"VersionMinor", 0}});
    schemas2.Add({"MetaSchema.ECSchemaDef", "B"}, {{"Name", "ReferancedSchema"}, {"VersionMajor", 1}, {"VersionMinor", 0}});

    EXPECT_CALL(GetMockClient(), SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas2.ToWSObjectsResponse()))));

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "A"), ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        Utf8String schemaXml =
            R"( <ECSchema schemaName="SchemaWithReferance" nameSpacePrefix="A" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                    <ECSchemaReference name="ReferancedSchema" version="01.00" prefix="B" />
                </ECSchema>)";
        SimpleWriteToFile(schemaXml, filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));
    EXPECT_CALL(GetMockClient(), SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "B"), ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(StubWSFileResultNotModified());
        }));

    // Act
    auto result = ds->UpdateSchemas(nullptr)->GetResult();

    // Assert
    ASSERT_TRUE(result.IsSuccess());

    ASSERT_TRUE(nullptr != ds->StartCacheTransaction().GetCache().GetAdapter().GetECSchema("SchemaWithReferance"));
    ASSERT_TRUE(nullptr != ds->StartCacheTransaction().GetCache().GetAdapter().GetECSchema("ReferancedSchema"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, UpdateSchemas_SchemasIncludeStandardSchemas_SkipsStandardSchemas)
    {
    auto ds = GetTestDataSourceV2();

    // Act & Assert
    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "A"}, {{"Name", "Bentley_Standard_CustomAttributes"}, {"VersionMajor", 1}, {"VersionMinor", 0}});
    schemas.Add({"MetaSchema.ECSchemaDef", "B"}, {{"Name", "CustomSchema"}, {"VersionMajor", 1}, {"VersionMinor", 0}});
    schemas.Add({"MetaSchema.ECSchemaDef", "C"}, {{"Name", "ECDbMap"}, {"VersionMajor", 1}, {"VersionMinor", 0}});

    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(GetMockClient(), SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse()))));

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillRepeatedly(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("MetaSchema.ECSchemaDef", "B"), objectId);
        Utf8String schemaXml;
        StubSchema("CustomSchema", "CS")->WriteToXmlString(schemaXml);
        return CreateCompletedAsyncTask(StubWSFileResult(StubFile(schemaXml)));
        }));

    auto result = ds->UpdateSchemas(nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, UpdateSchemas_InvalidSchemaGotFromServer_ReturnsError)
    {
    auto ds = GetTestDataSourceV2();

    // Act & Assert
    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "Foo"}, {{"Name", "Foo"}});

    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(GetMockClient(), SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse()))));

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillRepeatedly(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(StubWSFileResult(StubFile("Not-a-schema")));
        }));

    BeTest::SetFailOnAssert(false);
    auto result = ds->UpdateSchemas(nullptr)->GetResult();
    BeTest::SetFailOnAssert(true);

    ASSERT_FALSE(result.IsSuccess());
    ASSERT_EQ(ICachingDataSource::Status::SchemaError, result.GetError().GetStatus());
    ASSERT_FALSE(result.GetError().GetMessage().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, UpdateSchemas_InvalidSchemaGotFromServer_ReturnsRepositorySchemaError)
    {  
    // Arrange
    auto client = MockWSRepositoryClient::Create();
    auto ds = CreateNewTestDataSource(client);
    ASSERT_TRUE(nullptr != ds);

    // Act & Assert
    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "Foo"}, {{"Name", "Foo"}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi(BeVersion(2,1))))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillRepeatedly(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        SimpleWriteToFile("Not-a-schema", filePath);
        return CreateCompletedAsyncTask(WSFileResult::Success(WSFileResponse(filePath, HttpStatus::OK, nullptr)));
        }));

    BeTest::SetFailOnAssert(false);
    auto result = ds->UpdateSchemas(nullptr)->GetResult();
    BeTest::SetFailOnAssert(true);

    ASSERT_FALSE(result.IsSuccess());
    ASSERT_EQ(ICachingDataSource::Status::SchemaError, result.GetError().GetStatus());
    ASSERT_FALSE(result.GetError().GetMessage().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetRepositorySchemas_CacheContainsNonRepositorySchema_ReturnsOnlyRepositorySchemas)
    {
    // Arrange
    auto client = MockWSRepositoryClient::Create();

    StubInstances schemaDefs;
    schemaDefs.Add({"MetaSchema.ECSchemaDef", "A"}, {{"Name", "A"}, {"VersionMajor", 1}, {"VersionMinor", 0}});
    schemaDefs.Add({"MetaSchema.ECSchemaDef", "B"}, {{"Name", "B"}, {"VersionMajor", 1}, {"VersionMinor", 0}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemaDefs.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "A"), ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(StubWSFileResult(StubFile(StubSchemaXml("A"))));
        }));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "B"), ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(StubWSFileResult(StubFile(StubSchemaXml("B"))));
        }));

    // Act
    auto ds = CachingDataSource::OpenOrCreate(client, BeFileName(":memory:"), StubCacheEnvironemnt())->GetResult().GetValue();

    std::vector<ECSchemaPtr> schemas;
    schemas.push_back(StubSchema("Foo"));

    auto txn = ds->StartCacheTransaction();
    BentleyStatus result = txn.GetCache().UpdateSchemas(schemas);
    ASSERT_EQ(SUCCESS, result);
    txn.Commit();

    // Act 2
    ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        auto txn = ds->StartCacheTransaction();
        auto schemas = ds->GetRepositorySchemas(txn);
        EXPECT_EQ(2, schemas.size());
        EXPECT_CONTAINS(schemas, txn.GetCache().GetAdapter().GetECSchema("A"));
        EXPECT_CONTAINS(schemas, txn.GetCache().GetAdapter().GetECSchema("B"));
        EXPECT_NE(nullptr, txn.GetCache().GetAdapter().GetECSchema("A"));
        EXPECT_NE(nullptr, txn.GetCache().GetAdapter().GetECSchema("B"));
        })->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetRepositorySchemaKeys_CacheContainsNonRepositorySchema_ReturnsOnlyRepositorySchemas)
    {
    // Arrange
    auto client = MockWSRepositoryClient::Create();

    StubInstances schemaDefs;
    schemaDefs.Add({"MetaSchema.ECSchemaDef", "A"}, {{"Name", "A"}, {"VersionMajor", 1}, {"VersionMinor", 0}});
    schemaDefs.Add({"MetaSchema.ECSchemaDef", "B"}, {{"Name", "B"}, {"VersionMajor", 4}, {"VersionMinor", 2}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemaDefs.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "A"), ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(StubWSFileResult(StubFile(StubSchemaXml("A"))));
        }));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "B"), ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(StubWSFileResult(StubFile(StubSchemaXml("B"))));
        }));

    // Act
    auto ds = CachingDataSource::OpenOrCreate(client, BeFileName(":memory:"), StubCacheEnvironemnt())->GetResult().GetValue();

    std::vector<ECSchemaPtr> schemas;
    schemas.push_back(StubSchema("Foo"));

    auto txn = ds->StartCacheTransaction();
    BentleyStatus result = txn.GetCache().UpdateSchemas(schemas);
    ASSERT_EQ(SUCCESS, result);
    txn.Commit();

    // Act 2
    {
    auto txn = ds->StartCacheTransaction();
    auto schemaKeys = ds->GetRepositorySchemaKeys(txn);
    EXPECT_THAT(schemaKeys, SizeIs(2));
    EXPECT_THAT(schemaKeys, Contains(SchemaKey("A", 1, 0)));
    EXPECT_THAT(schemaKeys, Contains(SchemaKey("B", 4, 2)));
    }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetServerInfo_CreatedCacheAndCalledWithTransaction_ReturnsInfoReturnedForListener)
    {
    // Arrange
    auto client = MockWSRepositoryClient::Create();

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubInstances().ToWSObjectsResponse()))));

    std::weak_ptr<IWSClient::IServerInfoListener> listenerWeakPtr;
    EXPECT_CALL(client->GetMockWSClient(), RegisterServerInfoListener(_))
        .WillOnce(Invoke([&] (std::weak_ptr<IWSClient::IServerInfoListener> providedListener)
        {
        listenerWeakPtr = providedListener;
        }));

    // Act
    auto ds = CachingDataSource::OpenOrCreate(client, BeFileName(":memory:"), StubCacheEnvironemnt())->GetResult().GetValue();

    listenerWeakPtr.lock()->OnServerInfoReceived(StubWSInfoWebApi(BeVersion(2, 3)));
    ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        auto txn = ds->StartCacheTransaction();
        EXPECT_EQ(BeVersion(2, 3), ds->GetServerInfo(txn).GetWebApiVersion());
        })->Wait();

        listenerWeakPtr.lock()->OnServerInfoReceived(StubWSInfoWebApi(BeVersion(2, 3, 1, 0)));
        ds->GetCacheAccessThread()->ExecuteAsync([=]
            {
            auto txn = ds->StartCacheTransaction();
            EXPECT_EQ(BeVersion(2, 3, 1, 0), ds->GetServerInfo(txn).GetWebApiVersion());
            })->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetServerInfo_CreatedCache_ReturnsInfoReturnedForListener)
    {
    // Arrange
    auto client = MockWSRepositoryClient::Create();

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubInstances().ToWSObjectsResponse()))));

    std::weak_ptr<IWSClient::IServerInfoListener> listenerWeakPtr;
    EXPECT_CALL(client->GetMockWSClient(), RegisterServerInfoListener(_))
        .WillOnce(Invoke([&] (std::weak_ptr<IWSClient::IServerInfoListener> providedListener)
        {
        listenerWeakPtr = providedListener;
        }));

    // Act
    auto ds = CachingDataSource::OpenOrCreate(client, BeFileName(":memory:"), StubCacheEnvironemnt())->GetResult().GetValue();

    listenerWeakPtr.lock()->OnServerInfoReceived(StubWSInfoWebApi(BeVersion(2, 3)));
    ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        EXPECT_EQ(BeVersion(2, 3), ds->GetServerInfo().GetWebApiVersion());
        })->Wait();

        listenerWeakPtr.lock()->OnServerInfoReceived(StubWSInfoWebApi(BeVersion(2, 3, 1, 0)));
        ds->GetCacheAccessThread()->ExecuteAsync([=]
            {
            EXPECT_EQ(BeVersion(2, 3, 1, 0), ds->GetServerInfo().GetWebApiVersion());
            })->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetRepositoryInfo_CreatedCacheAndCalledWithTransaction_ReturnsInfoReturnedForListener)
    {
    // Arrange
    auto client = MockWSRepositoryClient::Create();

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubInstances().ToWSObjectsResponse()))));

    std::weak_ptr<IWSRepositoryClient::IRepositoryInfoListener> listenerWeakPtr;
    EXPECT_CALL(*client, RegisterRepositoryInfoListener(_))
        .WillOnce(Invoke([&] (std::weak_ptr<IWSRepositoryClient::IRepositoryInfoListener> providedListener)
        {
        listenerWeakPtr = providedListener;
        }));

    // Act
    auto ds = CachingDataSource::OpenOrCreate(client, BeFileName(":memory:"), StubCacheEnvironemnt())->GetResult().GetValue();

    listenerWeakPtr.lock()->OnInfoReceived(StubWSRepository("testServer", "repositoryId"));
    ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        EXPECT_STREQ("repositoryId", ds->GetRepositoryInfo().GetId().c_str());
        })->Wait();

        listenerWeakPtr.lock()->OnInfoReceived(StubWSRepository("testServer", "repositoryId2"));
        ds->GetCacheAccessThread()->ExecuteAsync([=]
            {
            EXPECT_STREQ("repositoryId2", ds->GetRepositoryInfo().GetId().c_str());
            })->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetFile_InstanceIsNotCached_ErrorStatus)
    {
    auto ds = GetTestDataSourceV2();

    auto result = ds->GetFile({"TestSchema.TestClass", "Foo"}, CachingDataSource::DataOrigin::CachedData, nullptr, nullptr)->GetResult();

    EXPECT_EQ(CachingDataSource::Status::DataNotCached, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetFile_FileInstanceIsCached_ProgressIsCalledWithNameAndSize)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    ObjectId fileId {"TestSchema.TestFileClass", "TestId"};
    std::map<Utf8String, Json::Value> properties = {{"TestSize", "42"}, {"TestName", "TestFileName"}};
    StubInstanceInCache(txn.GetCache(), fileId, properties);
    txn.Commit();

    // Act & Assert
    int onProgressCalled = 0;
    ICachingDataSource::Progress expectedProgress({0, 42}, std::make_shared<Utf8String>("TestFileName"));
    CachingDataSource::ProgressCallback onProgress =
        [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_EQ(expectedProgress, progress);
        onProgressCalled++;
        };

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        progress(0, 0);
        return CreateCompletedAsyncTask(WSFileResult());
        }));

    ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, onProgress, nullptr)->Wait();
    EXPECT_EQ(1, onProgressCalled);
    }
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Benediktas.Lipnickas              09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetFile_CalledMultipleTimes_ProgressIsReportedForAllCallers)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    ObjectId fileId {"TestSchema.TestFileClass", "TestId"};

    StubInstances fileInstances;
    fileInstances.Add(fileId, {{"TestSize", "42"}, {"TestName", "TestFileName"}});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstanceAndLinkToRoot(fileId, fileInstances.ToWSObjectsResponse(), "root"));
    txn.Commit();

    // Act & Assert
    BeFileName filePath;
    auto downloadTask = std::make_shared<PackagedAsyncTask<WSFileResult>>([&]
        {
        return StubWSFileResult(StubFile("TestContent"));
        });

    AsyncTestCheckpoint check1;

    Http::Request::ProgressCallback onProgress;
    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR path, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        filePath = path;
        onProgress = progress;

        check1.Checkin();
        return downloadTask;
        }));

    bool progressReported1 = false;
    double bytesTransfered1 = 0;
    double bytesTotal1 = 0;
    CachingDataSource::ProgressCallback onProgress1 =
        [&] (CachingDataSource::ProgressCR progress)
        {
        progressReported1 = true;
        bytesTransfered1 = progress.GetBytes().current;
        bytesTotal1 = progress.GetBytes().total;
        };

    auto lastTask = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, onProgress1, nullptr)
        ->Then([&] (CachingDataSource::FileResult result)
        {
        });

    bool progressReported2 = false;
    double bytesTransfered2 = 0;
    double bytesTotal2 = 0;
    CachingDataSource::ProgressCallback onProgress2 =
        [&] (CachingDataSource::ProgressCR progress)
        {
        progressReported2 = true;
        bytesTransfered2 = progress.GetBytes().current;
        bytesTotal2 = progress.GetBytes().total;
        };
    auto lastTask2 = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, onProgress2, nullptr)
        ->Then([&] (CachingDataSource::FileResult result)
        {
        });

    check1.WaitUntilReached();
    onProgress(2, 42);

    downloadTask->Execute();

    lastTask->Wait();
    lastTask2->Wait();

    ASSERT_TRUE(progressReported1);
    ASSERT_TRUE(progressReported2);
    EXPECT_EQ(bytesTransfered1, bytesTransfered2);
    EXPECT_EQ(bytesTotal2, bytesTotal2);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetFile_CalledMultipleTimesFirstCancelled_FirstCallbackIsCancelledSecondFinishes)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    ObjectId fileId {"TestSchema.TestFileClass", "TestId"};

    StubInstances fileInstances;
    fileInstances.Add(fileId, {{"TestSize", "42"}, {"TestName", "TestFileName"}});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstanceAndLinkToRoot(fileId, fileInstances.ToWSObjectsResponse(), "root"));
    txn.Commit();

    // Act & Assert
    CachingDataSource::ProgressCallback onProgress =
        [&] (CachingDataSource::ProgressCR progress)
        {
        };

    BeFileName filePath;
    auto downloadTask = std::make_shared<PackagedAsyncTask<WSFileResult>>([&]
        {
        return StubWSFileResult(StubFile("TestContent"));
        });

    bool downloadStarted = false;

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR path, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        filePath = path;
        downloadStarted = true;

        return downloadTask;
        }));


    bool task1Finished = false;
    auto ct1 = SimpleCancellationToken::Create();
    auto lastTask = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, onProgress, ct1)
        ->Then([&] (CachingDataSource::FileResult result)
        {
        task1Finished = true;
        EXPECT_EQ(ICachingDataSource::Status::Canceled, result.GetError().GetStatus());
        });

    while (!downloadStarted);

    bool task2Finished = false;
    auto ct2 = SimpleCancellationToken::Create();
    auto lastTask2 = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, onProgress, ct2)
        ->Then([&] (CachingDataSource::FileResult result)
        {
        task2Finished = true;
        EXPECT_TRUE(result.IsSuccess());
        });

    ct1->SetCanceled();

    lastTask->Wait();

    ASSERT_TRUE(task1Finished);

    downloadTask->Execute();

    lastTask2->Wait();

    ASSERT_TRUE(task2Finished);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetFile_CalledMultipleTimesSecondCancelled_SecondCallbackIsCancelledFirstFinishes)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    ObjectId fileId {"TestSchema.TestFileClass", "TestId"};

    StubInstances fileInstances;
    fileInstances.Add(fileId, {{"TestSize", "42"}, {"TestName", "TestFileName"}});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstanceAndLinkToRoot(fileId, fileInstances.ToWSObjectsResponse(), "root"));
    txn.Commit();

    // Act & Assert
    CachingDataSource::ProgressCallback onProgress =
        [&] (CachingDataSource::ProgressCR progress)
        {
        };

    BeFileName filePath;
    auto downloadTask = std::make_shared<PackagedAsyncTask<WSFileResult>>([&]
        {
        return StubWSFileResult(StubFile("TestContent"));
        });

    bool downloadStarted = false;

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR path, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        filePath = path;
        downloadStarted = true;

        return downloadTask;
        }));


    bool task1Finished = false;
    auto ct1 = SimpleCancellationToken::Create();
    auto lastTask = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, onProgress, ct1)
        ->Then([&] (CachingDataSource::FileResult result)
        {
        task1Finished = true;
        EXPECT_TRUE(result.IsSuccess());
        });

    while (!downloadStarted);

    bool task2Finished = false;
    auto ct2 = SimpleCancellationToken::Create();
    auto lastTask2 = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, onProgress, ct2)
        ->Then([&] (CachingDataSource::FileResult result)
        {
        task2Finished = true;
        EXPECT_EQ(ICachingDataSource::Status::Canceled, result.GetError().GetStatus());
        });

    ct2->SetCanceled();

    lastTask2->Wait();

    ASSERT_TRUE(task2Finished);

    downloadTask->Execute();

    lastTask->Wait();

    ASSERT_TRUE(task1Finished);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetFile_ClassDoesNotHaveFileDependentPropertiesCA_ProgressIsCalledWithNoNameAndNoSizeAndFileHasDefaultName)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    ObjectId fileId {"TestSchema.TestClass", "TestId"};

    StubInstances fileInstances;
    fileInstances.Add(fileId);

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstanceAndLinkToRoot(fileId, fileInstances.ToWSObjectsResponse(), "root"));
    txn.Commit();

    // Act & Assert
    int onProgressCalled = 0;
    CachingDataSource::ProgressCallback onProgress =
        [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_PROGRESS_EQ(ICachingDataSource::Progress(), progress);
        onProgressCalled++;
        };

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        EXPECT_EQ(L"TestClass_TestId", filePath.GetFileNameAndExtension());
        progress(0, 42);
        return CreateCompletedAsyncTask(WSFileResult());
        }));

    ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, onProgress, nullptr)->Wait();
    EXPECT_EQ(1, onProgressCalled);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetFile_InstanceHasVeryLongRemoteIdAndNoFileDependentPropertiesCA_FileHasTruncatedNameAndCanBeWrittenTo)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    ObjectId fileId {"TestSchema.TestClass", Utf8String(10000, 'x')};

    StubInstances fileInstances;
    fileInstances.Add(fileId);

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstanceAndLinkToRoot(fileId, fileInstances.ToWSObjectsResponse(), "root"));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(StubWSFileResult(StubFile("TestContent")));
        }));

    auto result = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("TestContent", SimpleReadFile(result.GetValue().GetFilePath()));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetFile_ClassDoesNotHaveFileDependentPropertiesCAButHasLabel_ProgressIsCalledWithGeneratedFileNameAsLabelMightBeNotSuitable)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    ObjectId fileId {"TestSchema.TestLabeledClass", "TestId"};

    StubInstances fileInstances;
    fileInstances.Add(fileId, {{"Name", "TestLabel"}});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstanceAndLinkToRoot(fileId, fileInstances.ToWSObjectsResponse(), "root"));
    txn.Commit();

    // Act & Assert
    int onProgressCalled = 0;
    CachingDataSource::ProgressCallback onProgress =
        [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_EQ(0, progress.GetBytes().current);
        EXPECT_EQ(0, progress.GetBytes().total);
        EXPECT_EQ(0, progress.GetInstances().current);
        EXPECT_EQ(0, progress.GetInstances().total);
        EXPECT_EQ("", progress.GetLabel());
        onProgressCalled++;
        };

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        EXPECT_EQ(L"TestLabeledClass_TestId", filePath.GetFileNameAndExtension());
        progress(0, 42);
        return CreateCompletedAsyncTask(WSFileResult());
        }));

    ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, onProgress, nullptr)->Wait();
    EXPECT_EQ(1, onProgressCalled);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetFile_RemoteOrCachedDataAndFileNotCachedAndConnectionError_ReturnsError)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    auto txn = ds->StartCacheTransaction();
    ObjectId fileId {"TestSchema.TestLabeledClass", "TestId"};
    ECInstanceKey fileKey = StubInstanceInCache(txn.GetCache(), fileId);
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(WSFileResult::Error(StubHttpResponse()));
        }));

    auto result = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteOrCachedData)->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Status::ConnectionError, result.GetError().GetWSError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetFile_RemoteOrCachedDataAndFileNotCachedAndServerReturnsFile_CachesFile)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    auto txn = ds->StartCacheTransaction();
    ObjectId fileId {"TestSchema.TestLabeledClass", "TestId"};
    ECInstanceKey fileKey = StubInstanceInCache(txn.GetCache(), fileId);
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(StubWSFileResult(StubFile("Foo")));
        }));

    auto result = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteOrCachedData)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ("Foo", SimpleReadFile(result.GetValue().GetFilePath()));
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetFile_RemoteOrCachedDataAndFileCachedAndConnectionError_ReturnsCachedFile)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    auto txn = ds->StartCacheTransaction();
    ObjectId fileId {"TestSchema.TestLabeledClass", "TestId"};
    ECInstanceKey fileKey = StubInstanceInCache(txn.GetCache(), fileId);
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheFile(fileId, StubWSFileResponse(StubFile("Foo"))));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(WSFileResult::Error(StubHttpResponse()));
        }));

    auto result = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteOrCachedData)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ("Foo", SimpleReadFile(result.GetValue().GetFilePath()));
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetFile_RemoteOrCachedDataAndFileCachedAndServerReturnsNewFile_CachesNewFile)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    auto txn = ds->StartCacheTransaction();
    ObjectId fileId {"TestSchema.TestLabeledClass", "TestId"};
    ECInstanceKey fileKey = StubInstanceInCache(txn.GetCache(), fileId);
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheFile(fileId, StubWSFileResponse(StubFile("OldFile"))));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(StubWSFileResult(StubFile("NewFile")));
        }));

    auto result = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteOrCachedData)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ("NewFile", SimpleReadFile(result.GetValue().GetFilePath()));
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetFile_RemoteOrCachedDataAndFileCachedAndServerReturnsNotModified_LeavesCachedFile)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    auto txn = ds->StartCacheTransaction();
    ObjectId fileId {"TestSchema.TestLabeledClass", "TestId"};
    ECInstanceKey fileKey = StubInstanceInCache(txn.GetCache(), fileId);
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheFile(fileId, StubWSFileResponse(StubFile("OldFile"))));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(WSFileResult::Success(StubWSFileResponseNotModified()));
        }));

    auto result = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteOrCachedData)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ("OldFile", SimpleReadFile(result.GetValue().GetFilePath()));
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, CacheFiles_BothFilesCachedAndSkipCached_NoFileRequestAndSuccess)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    ObjectId fileId {"TestSchema.TestFileClass", "TestId"};
    ObjectId file2Id {"TestSchema.TestFileClass", "TestId2"};
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, fileId));
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheFile(fileId, StubWSFileResponse(StubFile()), FileCache::Persistent));
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, file2Id));
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheFile(file2Id, StubWSFileResponse(StubFile()), FileCache::Persistent));
    txn.Commit();

    bvector<ObjectId> files;
    files.push_back(fileId);
    files.push_back(file2Id);

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(0);

    auto result = ds->CacheFiles(files, true, FileCache::Persistent, nullptr, nullptr)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, CacheFiles_OneFileCachedAndSkipCached_OneFileRequestAndSuccess)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    ObjectId fileId {"TestSchema.TestFileClass", "TestId"};
    ObjectId file2Id {"TestSchema.TestFileClass", "TestId2"};
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, fileId));
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, file2Id));
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheFile(file2Id, StubWSFileResponse(StubFile()), FileCache::Persistent));
    txn.Commit();

    bvector<ObjectId> files;
    files.push_back(fileId);
    files.push_back(file2Id);

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR fileName, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(StubWSFileResult());
        }));

    auto result = ds->CacheFiles(files, true, FileCache::Persistent, nullptr, nullptr)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, CacheFiles_OneFileCachedAndNoSkipCached_TwoFileRequestsAndSuccess)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    ObjectId fileId {"TestSchema.TestFileClass", "TestId"};
    ObjectId file2Id {"TestSchema.TestFileClass", "TestId2"};
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, fileId));
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, file2Id));
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheFile(file2Id, StubWSFileResponse(StubFile()), FileCache::Persistent));
    txn.Commit();

    bvector<ObjectId> files;
    files.push_back(fileId);
    files.push_back(file2Id);

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(2)
        .WillRepeatedly(Invoke([&] (ObjectIdCR, BeFileNameCR fileName, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(StubWSFileResult());
        }));

    auto result = ds->CacheFiles(files, false, FileCache::Persistent, nullptr, nullptr)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, CacheFiles_FileDownloadRestarts_ProgressReportsSmallerValue)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    ds->SetMinTimeBetweenProgressCalls(0);

    auto txn = ds->StartCacheTransaction();
    ObjectId fileId {"TestSchema.TestFileClass", "TestId"};
    StubInstanceInCache(txn.GetCache(), fileId, {{"TestSize", "42"}});
    txn.Commit();

    // Act & Assert
    int onProgressCount = 0;
    bvector<CachingDataSource::Progress::State> expectedBytes = {{0, 42}, {20, 42}, {10, 42}, {40, 42}, {42, 42}};
    auto onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_EQ(expectedBytes[onProgressCount], progress.GetBytes()) << "Iteration:" << onProgressCount;
        onProgressCount++;
        };

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, ::testing::An<BeFileNameCR>(), _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        progress(0, 42);
        progress(20, 42);
        progress(10, 42);
        progress(40, 42);
        return CreateCompletedAsyncTask(StubWSFileResult());
        }));

    ds->CacheFiles({fileId}, false, FileCache::Auto, onProgress, nullptr)->Wait();
    EXPECT_EQ(expectedBytes.size(), onProgressCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, CacheFiles_TwoFilesDownloading_ProgressReportsSumOfBothDownloads)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    ds->SetMinTimeBetweenProgressCalls(0);

    auto txn = ds->StartCacheTransaction();
    ObjectId fileId1 {"TestSchema.TestFileClass", "A"};
    ObjectId fileId2 {"TestSchema.TestFileClass", "B"};
    StubInstanceInCache(txn.GetCache(), fileId1, {{"TestSize", "10"}});
    StubInstanceInCache(txn.GetCache(), fileId2, {{"TestSize", "1000"}});
    txn.Commit();

    // Act & Assert
    AsyncTestCheckpoint c1, c2;
    Http::Request::ProgressCallback progress1, progress2;
    EXPECT_CALL(GetMockClient(), SendGetFileRequest(fileId1, ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        progress1 = progress;
        return WorkerThread::Create()->ExecuteAsync<WSFileResult>([&]
            {
            c1.CheckinAndWait();
            return StubWSFileResult();
            });
        }));

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(fileId2, ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        progress2 = progress;
        return WorkerThread::Create()->ExecuteAsync<WSFileResult>([&]
            {
            c2.CheckinAndWait();
            return StubWSFileResult();
            });
        }));

    int onProgressCount = 0;
    bvector<CachingDataSource::Progress::State> expectedBytes = {{1, 1010}, {101, 1010}, {102, 1010}, {202, 1010}, {210, 1010}, {1010, 1010}};
    auto onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_EQ(expectedBytes[onProgressCount], progress.GetBytes()) << "Iteration:" << onProgressCount;
        onProgressCount++;
        };

    auto task = ds->CacheFiles({fileId1, fileId2}, false, FileCache::Auto, onProgress, nullptr);

    c1.WaitUntilReached();
    c2.WaitUntilReached();

    progress1(1, 10);
    progress2(100, 1000);
    progress1(2, 10);
    progress2(200, 1000);

    c1.Continue();
    while (onProgressCount < 5); // Wait for File 1 to send completion progress
    c2.Continue();

    task->Wait();

    EXPECT_EQ(expectedBytes.size(), onProgressCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, CacheFiles_TwoFilesAreDownloadingWhenMaxParalelDownloadsIsOne_DownloadsAndReportsProgressInChunks)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    ds->SetMinTimeBetweenProgressCalls(0);
    ds->SetMaxParalelFileDownloadLimit(1);

    auto txn = ds->StartCacheTransaction();
    ObjectId fileId1 {"TestSchema.TestFileClass", "A"};
    ObjectId fileId2 {"TestSchema.TestFileClass", "B"};
    StubInstanceInCache(txn.GetCache(), fileId1, {{"TestSize", "10"}});
    StubInstanceInCache(txn.GetCache(), fileId2, {{"TestSize", "1000"}});
    txn.Commit();

    // Act & Assert
    AsyncTestCheckpoint c1, c2;
    Http::Request::ProgressCallback progress1, progress2;
    EXPECT_CALL(GetMockClient(), SendGetFileRequest(fileId1, ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        progress1 = progress;
        return WorkerThread::Create()->ExecuteAsync<WSFileResult>([&]
            {
            c1.CheckinAndWait();
            return StubWSFileResult();
            });
        }));

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(fileId2, ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR, Utf8StringCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        progress2 = progress;
        return WorkerThread::Create()->ExecuteAsync<WSFileResult>([&]
            {
            c2.CheckinAndWait();
            return StubWSFileResult();
            });
        }));

    int onProgressCount = 0;
    bvector<CachingDataSource::Progress::State> expectedBytes = {{1, 1010}, {2, 1010}, {10, 1010}, {110, 1010}, {210, 1010}, {1010, 1010}};
    auto onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_EQ(expectedBytes[onProgressCount], progress.GetBytes()) << "Iteration:" << onProgressCount;
        onProgressCount++;
        };

    auto task = ds->CacheFiles({fileId1, fileId2}, false, FileCache::Auto, onProgress, nullptr);

    c1.WaitUntilReached();

    progress1(1, 10);
    progress1(2, 10);

    EXPECT_FALSE(c2.WasReached());
    c1.Continue();
    c2.WaitUntilReached();

    progress2(100, 1000);
    progress2(200, 1000);

    c2.Continue();

    task->Wait();

    EXPECT_EQ(expectedBytes.size(), onProgressCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, DownloadAndCacheChildren_SpecificParent_ChildIsCached)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Parent"}));
    txn.Commit();

    // Act & Assert
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Child"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    bvector<ObjectId> parents;
    parents.push_back({"TestSchema.TestClass", "Parent"});
    auto result = ds->DownloadAndCacheChildren(parents, nullptr)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "Child"}).IsInCache());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetNavigationChildren_SpecificParentInstance_ChildIsCachedAndReturned)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Parent"}));
    txn.Commit();

    // Act & Assert
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Child"});
    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubWSInfoResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    auto result = ds->GetNavigationChildren({"TestSchema.TestClass", "Parent"}, CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(1, result.GetValue().GetJson().size());

        {
        auto txn = ds->StartCacheTransaction();
        EXPECT_EQ(ObjectId("TestSchema.TestClass", "Child"), txn.GetCache().ObjectIdFromJsonInstance(result.GetValue().GetJson()[0]));
        EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
        EXPECT_TRUE(txn.GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "Child"}).IsInCache());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetNavigationChildren_GettingRemoteData_ObjectIsCachedAndReturned)
    {
    auto ds = GetTestDataSourceV2();

    // Act & Assert
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId()));
    txn.Commit();

    auto result = ds->GetNavigationChildren(ObjectId(), CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ(1, result.GetValue().GetJson().size());

        {
        auto txn = ds->StartCacheTransaction();
        EXPECT_EQ(ObjectId("TestSchema.TestClass", "Foo"), txn.GetCache().ObjectIdFromJsonInstance(result.GetValue().GetJson()[0]));
        EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
        EXPECT_TRUE(txn.GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsInCache());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetNavigationChildren_GettingCachedDataAfterCached_ObjectIsReturned)
    {
    auto ds = GetTestDataSourceV2();

    // Act & Assert
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId()));
    txn.Commit();

    ds->GetNavigationChildren(ObjectId(), CachingDataSource::DataOrigin::RemoteData, nullptr)->Wait();
    auto result = ds->GetNavigationChildren(ObjectId(), CachingDataSource::DataOrigin::CachedData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ(1, result.GetValue().GetJson().size());

        {
        auto txn = ds->StartCacheTransaction();
        EXPECT_EQ(ObjectId("TestSchema.TestClass", "Foo"), txn.GetCache().ObjectIdFromJsonInstance(result.GetValue().GetJson()[0]));
        EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetNavigationChildrenKeys_SpecificParentInstance_ChildIsCachedAndKeyReturned)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Parent"}));
    txn.Commit();

    // Act & Assert
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Child"});
    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubWSInfoResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    auto result = ds->GetNavigationChildrenKeys({"TestSchema.TestClass", "Parent"}, CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ECInstanceKey cachedChildKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "Child"});
    EXPECT_TRUE(cachedChildKey.IsValid());
    ASSERT_EQ(1, result.GetValue().GetKeys().size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cachedChildKey, result.GetValue().GetKeys()));
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetNavigationChildrenKeys_GettingRemoteData_ObjectIsCachedAndReturned)
    {
    auto ds = GetTestDataSourceV2();

    // Act & Assert
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubWSInfoResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId()));
    txn.Commit();

    auto result = ds->GetNavigationChildrenKeys(ObjectId(), CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ECInstanceKey cachedChildKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(cachedChildKey.IsValid());
    ASSERT_EQ(1, result.GetValue().GetKeys().size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cachedChildKey, result.GetValue().GetKeys()));
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetNavigationChildrenKeys_GettingCachedDataAfterCached_ObjectIsReturned)
    {
    auto ds = GetTestDataSourceV2();

    // Act & Assert
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId()));
    txn.Commit();

    ds->GetNavigationChildrenKeys(ObjectId(), CachingDataSource::DataOrigin::RemoteData, nullptr)->Wait();
    auto result = ds->GetNavigationChildrenKeys(ObjectId(), CachingDataSource::DataOrigin::CachedData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ECInstanceKey cachedChildKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(cachedChildKey.IsValid());
    ASSERT_EQ(1, result.GetValue().GetKeys().size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cachedChildKey, result.GetValue().GetKeys()));
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, CacheNavigation_TwoLevelsCachedPreviouslyAsTemporary_RepeatsSameQueries)
    {
    auto ds = GetTestDataSourceV2();

    // Act & Assert
    StubInstances instances1;
    instances1.Add({"TestSchema.TestClass", "A"});
    StubInstances instances2;
    instances2.Add({"TestSchema.TestClass", "B"});

    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubWSInfoResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances1.ToWSObjectsResponse("TagA")))))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances2.ToWSObjectsResponse("TagB")))));
    EXPECT_CALL(GetMockClient(), SendGetChildrenRequest(_, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR parentObjectId, const bset<Utf8String>&, Utf8StringCR eTag, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId(), parentObjectId);
        EXPECT_EQ("TagA", eTag);
        return CreateCompletedAsyncTask(StubWSObjectsResultNotModified());
        }))
            .WillOnce(Invoke([&] (ObjectIdCR parentObjectId, const bset<Utf8String>&, Utf8StringCR eTag, ICancellationTokenPtr)
            {
            EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), parentObjectId);
            EXPECT_EQ("TagB", eTag);
            return CreateCompletedAsyncTask(StubWSObjectsResultNotModified());
            }));

        auto txn = ds->StartCacheTransaction();
        ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId()));
        txn.Commit();

        ds->GetNavigationChildren(ObjectId(), CachingDataSource::DataOrigin::RemoteData, nullptr)->Wait();
        ds->GetNavigationChildren({"TestSchema.TestClass", "A"}, CachingDataSource::DataOrigin::RemoteData, nullptr)->Wait();

        bvector<ObjectId> navigationTreesToCacheFully;
        bvector<ObjectId> navigationTreesToUpdateOnly;
        navigationTreesToUpdateOnly.push_back(ObjectId());

        auto result = ds->CacheNavigation(navigationTreesToCacheFully, navigationTreesToUpdateOnly, nullptr, nullptr, nullptr)->GetResult();
        EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, CacheNavigation_OneLevelCachedPreviouslyAsTemporary_RepeatsSameQueryAndCachesResults)
    {
    auto ds = GetTestDataSourceV2();

    // Act & Assert
    StubInstances instances1;
    instances1.Add({"TestSchema.TestClass", "A"});

    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubWSInfoResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances1.ToWSObjectsResponse("TagA")))));
    EXPECT_CALL(GetMockClient(), SendGetChildrenRequest(_, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR parentObjectId, const bset<Utf8String>&, Utf8StringCR eTag, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId(), parentObjectId);
        EXPECT_EQ("TagA", eTag);
        return CreateCompletedAsyncTask(StubWSObjectsResultNotModified());
        }));

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId()));
    txn.Commit();

    ds->GetNavigationChildren(ObjectId(), CachingDataSource::DataOrigin::RemoteData, nullptr)->Wait();

    bvector<ObjectId> navigationTreesToCacheFully;
    bvector<ObjectId> navigationTreesToUpdateOnly;
    navigationTreesToUpdateOnly.push_back(ObjectId());

    auto result = ds->CacheNavigation(navigationTreesToCacheFully, navigationTreesToUpdateOnly, nullptr, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, CacheNavigation_TemporaryNavigationNotCached_DoesNothing)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    // Act & Assert
    bvector<ObjectId> navigationTreesToCacheFully;
    bvector<ObjectId> navigationTreesToUpdateOnly;
    navigationTreesToUpdateOnly.push_back(ObjectId());

    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubWSInfoResult())));
    EXPECT_CALL(GetMockClient(), SendGetChildrenRequest(_, _, _, _)).Times(0);

    auto result = ds->CacheNavigation(navigationTreesToCacheFully, navigationTreesToUpdateOnly, nullptr, nullptr, nullptr)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, CacheNavigation_NotCachedRootPassedToBeFullyCached_QueriesChildrenRecursivelyForRootAndCachesResult)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    // Act & Assert
    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubWSInfoResult())));
    EXPECT_CALL(GetMockClient(), SendGetChildrenRequest(_, _, _, _)).Times(2)
        .WillOnce(Invoke([&] (ObjectIdCR parentObjectId, const bset<Utf8String>&, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId(), parentObjectId);
        StubInstances rootInstances;
        rootInstances.Add({"TestSchema.TestClass", "A"});
        return CreateCompletedAsyncTask(WSObjectsResult::Success(rootInstances.ToWSObjectsResponse()));
        }))
            .WillOnce(Invoke([&] (ObjectIdCR parentObjectId, const bset<Utf8String>&, Utf8StringCR, ICancellationTokenPtr)
            {
            EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), parentObjectId);
            return CreateCompletedAsyncTask(WSObjectsResult::Success(StubInstances().ToWSObjectsResponse()));
            }));

        bvector<ObjectId> navigationTreesToCacheFully;
        bvector<ObjectId> navigationTreesToUpdateOnly;
        navigationTreesToCacheFully.push_back(ObjectId());

        auto txn = ds->StartCacheTransaction();
        ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId()));
        txn.Commit();

        auto result = ds->CacheNavigation(navigationTreesToCacheFully, navigationTreesToUpdateOnly, nullptr, nullptr, nullptr)->GetResult();

        EXPECT_TRUE(result.IsSuccess());
        EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_ObjectNotCached_RetrievesRemoteObjectAndReturnsError)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(objectId, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto result = ds->GetObject(objectId, CachingDataSource::DataOrigin::CachedOrRemoteData)->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_ObjectLinkedButNotCached_RetrievesRemoteObject)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, objectId));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(objectId, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto result = ds->GetObject(objectId, CachingDataSource::DataOrigin::CachedOrRemoteData)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_ObjectNotCachedAndResponseHasInstance_ReturnsError)
    {
    auto ds = GetTestDataSourceV2();
    ObjectId objectIdA("TestSchema.TestClass", "Foo");
    StubInstances instances;
    instances.Add(objectIdA);

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(objectIdA, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto result = ds->GetObject(objectIdA, CachingDataSource::DataOrigin::CachedOrRemoteData)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::DataNotCached, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_CachedDataAndQueryResponseNotCached_ReturnsError)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::CachedData, nullptr, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::DataNotCached, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_CachedOrRemoteDataAndQueryResponseNotCached_SendsQueryRequest)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke([&] (WSQueryCR passedQuery, Utf8StringCR, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_THAT(passedQuery.GetSchemaName(), Eq(query.GetSchemaName()));
        EXPECT_THAT(passedQuery.GetClasses(), ContainerEq(query.GetClasses()));
        return CreateCompletedAsyncTask(WSObjectsResult());
        }));

    ds->GetObjects(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_CachedOrRemoteDataAndQueryResponseNotCachedAndNetworkError_ReturnsError)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult())));

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_CachedOrRemoteDataAndQueryResponseNotCached_CachesQueryResponseAndReturnsInstances)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    EXPECT_EQ(1, result.GetValue().GetJson().size());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[0][DataSourceCache_PROPERTY_RemoteId].asString());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_CachedOrRemoteDataAndQueryResponseIsCached_ReturnsCached)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_EQ(1, result.GetValue().GetJson().size());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[0][DataSourceCache_PROPERTY_RemoteId].asString());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_RemoteOrCachedDataAndConnectionError_ReturnsNetworkError)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteOrCachedData, nullptr, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_RemoteOrCachedDataAndQueryResponseIsCachedAndConnectionError_ReturnsCached)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteOrCachedData, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_EQ(1, result.GetValue().GetJson().size());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[0][DataSourceCache_PROPERTY_RemoteId].asString());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_RemoteOrCachedDataAndQueryResponseIsCachedAndNewData_ReturnsNew)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    StubInstances newInstances;
    newInstances.Add({"TestSchema.TestClass", "B"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(newInstances.ToWSObjectsResult())));

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteOrCachedData, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    EXPECT_EQ(1, result.GetValue().GetJson().size());
    EXPECT_EQ("B", result.GetValue().GetJson()[0][DataSourceCache_PROPERTY_RemoteId].asString());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_RemoteDataAndQueryResponseIsCached_SendsQueryRequestWithETag)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, StubInstances().ToWSObjectsResponse("TestEtag")));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, Utf8String("TestEtag"), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult())));

    ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_RemoteDataAndQueryResponseIsCachedAndNetworkErrors_ReturnsError)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, StubInstances().ToWSObjectsResponse("TestEtag")));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult())));

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_ResponseDoesNotContainPreviouslyCachedObject_RemovesObjectFromCachedResponse)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    CachedResponseKey key = CreateTestResponseKey(ds);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).Times(1)
        .WillOnce(InvokeWithoutArgs([&]
        {
        StubInstances newInstances;
        newInstances.Add({"TestSchema.TestClass", "A"});
        return CreateCompletedAsyncTask(WSObjectsResult::Success(newInstances.ToWSObjectsResponse()));
        }));

    WSQuery query("TestSchema", "TestClass");
    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_DataReadOptionsSpecified_ReturnsOnlyPropertiesSpecifiedByOptions)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances remoteInstances;
    remoteInstances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Foo"}, {"TestProperty2", "Boo"}});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(remoteInstances.ToWSObjectsResult())));

    auto options = std::make_shared<DataReadOptions>();
    options->SelectClassAndProperty("TestSchema.TestClass", "TestProperty");

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteData, options, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(1, result.GetValue().GetJson().size());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[0]["TestProperty"].asString());
    EXPECT_TRUE(result.GetValue().GetJson()[0]["TestProperty2"].isNull());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_QueryIncludesPartialInstancesThatAreInFullyPersisted_QueriesAndCachesRejectedSeparatelly)
    {
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    StubInstances fullInstances;
    fullInstances.Add({"TestSchema.TestClass", "A"});
    fullInstances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(fullInstances.ToWSObjectsResponse(), "SomePersistentRoot"));
    txn.Commit();

    StubInstances remoteInstances;
    remoteInstances.Add({"TestSchema.TestClass", "A"});
    remoteInstances.Add({"TestSchema.TestClass", "B"});
    remoteInstances.Add({"TestSchema.TestClass", "C"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(remoteInstances.ToWSObjectsResult())))
        .WillOnce(Invoke([=] (WSQueryCR query, Utf8StringCR, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_THAT(query.GetSchemaName(), Eq("TestSchema"));
        EXPECT_THAT(query.GetClasses(), ContainerEq(std::set<Utf8String> {"TestClass"}));
        EXPECT_THAT(query.GetFilter(), Eq("$id+in+['A','B']"));
        EXPECT_THAT(query.GetSelect(), Eq(""));

        StubInstances instances;
        instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Foo"}});
        instances.Add({"TestSchema.TestClass", "B"}, {{"TestProperty", "Boo"}});
        return CreateCompletedAsyncTask(instances.ToWSObjectsResult());
        }));

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");
    query.SetSelect("TestProperty");

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue().GetJson().size(), 3);

    EXPECT_THAT(result.GetValue().GetJson()[0][DataSourceCache_PROPERTY_RemoteId], Eq("A"));
    EXPECT_THAT(result.GetValue().GetJson()[1][DataSourceCache_PROPERTY_RemoteId], Eq("B"));
    EXPECT_THAT(result.GetValue().GetJson()[2][DataSourceCache_PROPERTY_RemoteId], Eq("C"));

    EXPECT_THAT(result.GetValue().GetJson()[0]["TestProperty"], Eq("Foo"));
    EXPECT_THAT(result.GetValue().GetJson()[1]["TestProperty"], Eq("Boo"));
    EXPECT_THAT(result.GetValue().GetJson()[2]["TestProperty"], Eq(Json::nullValue));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedDataAndQueryResponseNotCached_ReturnsError)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::CachedData, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::DataNotCached, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedOrRemoteDataAndQueryResponseNotCached_SendsQueryRequest)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke([&] (WSQueryCR passedQuery, Utf8StringCR, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_THAT(passedQuery.GetSchemaName(), Eq(query.GetSchemaName()));
        EXPECT_THAT(passedQuery.GetClasses(), ContainerEq(query.GetClasses()));
        return CreateCompletedAsyncTask(WSObjectsResult());
        }));

    ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedOrRemoteDataAndQueryResponseNotCachedAndNetworkError_ReturnsError)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult())));

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedOrRemoteDataAndQueryResponseNotCached_CachesQueryResponseAndReturnsInstances)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    EXPECT_EQ(ICachingDataSource::SyncStatus::Synced, result.GetValue().GetSyncStatus());
    EXPECT_EQ(1, result.GetValue().GetKeys().size());
    auto cachedInstanceKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cachedInstanceKey, result.GetValue().GetKeys()));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedOrRemoteDataAndQueryResponseIsCached_ReturnsCached)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, result.GetValue().GetSyncStatus());
    EXPECT_EQ(1, result.GetValue().GetKeys().size());
    auto cachedInstanceKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cachedInstanceKey, result.GetValue().GetKeys()));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteOrCachedDataAndConnectionError_ReturnsNetworkError)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteOrCachedData, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteOrCachedDataAndQueryResponseIsCachedAndConnectionError_ReturnsCached)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteOrCachedData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_EQ(ICachingDataSource::SyncStatus::SyncError, result.GetValue().GetSyncStatus());
    EXPECT_EQ(1, result.GetValue().GetKeys().size());
    auto cachedInstanceKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cachedInstanceKey, result.GetValue().GetKeys()));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteOrCachedDataAndQueryResponseIsCachedAndNewData_ReturnsNew)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    StubInstances newInstances;
    newInstances.Add({"TestSchema.TestClass", "B"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(newInstances.ToWSObjectsResult())));

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteOrCachedData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    EXPECT_EQ(ICachingDataSource::SyncStatus::Synced, result.GetValue().GetSyncStatus());
    EXPECT_EQ(1, result.GetValue().GetKeys().size());
    auto cachedInstanceKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "B"});
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cachedInstanceKey, result.GetValue().GetKeys()));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteDataAndQueryResponseIsCached_SendsQueryRequestWithETag)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, StubInstances().ToWSObjectsResponse("TestEtag")));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, Utf8String("TestEtag"), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult())));

    ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteDataAndQueryResponseIsCachedAndNetworkErrors_ReturnsError)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, StubInstances().ToWSObjectsResponse("TestEtag")));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult())));

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_ResponseDoesNotContainPreviouslyCachedObject_RemovesObjectFromCachedResponse)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    CachedResponseKey key = CreateTestResponseKey(ds);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).Times(1)
        .WillOnce(InvokeWithoutArgs([&]
        {
        StubInstances newInstances;
        newInstances.Add({"TestSchema.TestClass", "A"});
        return CreateCompletedAsyncTask(WSObjectsResult::Success(newInstances.ToWSObjectsResponse()));
        }));

    WSQuery query("TestSchema", "TestClass");
    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    EXPECT_EQ(ICachingDataSource::SyncStatus::Synced, result.GetValue().GetSyncStatus());
    EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteDataAndResponseNotModified_ReturnsCachedData)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    CachedResponseKey key = CreateTestResponseKey(ds);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse("TestTag")));
    txn.Commit();

    ECInstanceKeyMultiMap expectedInstances;
    expectedInstances.insert(ECDbHelper::ToPair(ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "A"})));
    expectedInstances.insert(ECDbHelper::ToPair(ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "B"})));

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, Utf8String("TestTag"), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubWSObjectsResponseNotModified()))));

    WSQuery query("TestSchema", "TestClass");
    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotModified, result.GetValue().GetSyncStatus());
    EXPECT_THAT(result.GetValue().GetKeys(), ContainerEq(expectedInstances));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_SkipTokensNotEnabled_SkipTokenNotTest)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    CachedResponseKey key = CreateTestResponseKey(ds);

    // Expect
    auto query = StubWSQuery();
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    EXPECT_CALL(GetMockClient(), SendQueryRequest(query, _, _, _))
        .WillOnce(Invoke([] (WSQueryCR, Utf8StringCR, Utf8StringCR skipToken, ICancellationTokenPtr)
        {
        EXPECT_EQ("", skipToken);
        return CreateCompletedAsyncTask(WSObjectsResult::Error({}));
        }));

    // Act
    ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_SkipTokensEnabled_InitialSkipTokenSent)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    CachedResponseKey key = CreateTestResponseKey(ds);

    ds->EnableSkipTokens(true);

    // Expect
    auto query = StubWSQuery();
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    EXPECT_CALL(GetMockClient(), SendQueryRequest(query, _, WSRepositoryClient::InitialSkipToken, _))
        .WillOnce(Invoke([] (WSQueryCR, Utf8StringCR, Utf8StringCR skipToken, ICancellationTokenPtr)
        {
        EXPECT_EQ("0", skipToken);
        return CreateCompletedAsyncTask(WSObjectsResult::Error({}));
        }));

    // Act
    ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjects_ClientRespondsWithSkipTokens_QueriesAndCachesMultiplePages)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    ds->EnableSkipTokens(true);
    CachedResponseKey key = CreateTestResponseKey(ds);

    // Expect
    auto query = StubWSQuery();

    InSequence callsInSeq;

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    EXPECT_CALL(GetMockClient(), SendQueryRequest(query, Utf8String(""), WSRepositoryClient::InitialSkipToken, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse("", "SkipTokenA")))));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    EXPECT_CALL(GetMockClient(), SendQueryRequest(query, Utf8String(""), Utf8String("SkipTokenA"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse("", "SkipTokenB")))));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "C"});
    EXPECT_CALL(GetMockClient(), SendQueryRequest(query, Utf8String(""), Utf8String("SkipTokenB"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse("", "")))));

    // Act
    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());

    auto txn = ds->StartCacheTransaction();
    EXPECT_TRUE(txn.GetCache().FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_TRUE(txn.GetCache().FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    EXPECT_TRUE(txn.GetCache().FindInstance({"TestSchema.TestClass", "C"}).IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_ClientRespondsWithSkipTokens_QueriesAndCachesMultiplePages)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    ds->EnableSkipTokens(true);
    CachedResponseKey key = CreateTestResponseKey(ds);

    // Expect
    auto query = StubWSQuery();

    InSequence callsInSeq;

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    EXPECT_CALL(GetMockClient(), SendQueryRequest(query, Utf8String(""), WSRepositoryClient::InitialSkipToken, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse("", "SkipTokenA")))));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "B"});
    EXPECT_CALL(GetMockClient(), SendQueryRequest(query, Utf8String(""), Utf8String("SkipTokenA"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse("", "SkipTokenB")))));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "C"});
    EXPECT_CALL(GetMockClient(), SendQueryRequest(query, Utf8String(""), Utf8String("SkipTokenB"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse("", "")))));

    // Act
    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    EXPECT_EQ(ICachingDataSource::SyncStatus::Synced, result.GetValue().GetSyncStatus());

    auto txn = ds->StartCacheTransaction();

    auto keys = result.GetValue().GetKeys();
    ASSERT_EQ(3, keys.size());
    auto it = keys.begin();
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), txn.GetCache().FindInstance(ECInstanceKey(it->first, it->second)));
    it++;
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "B"), txn.GetCache().FindInstance(ECInstanceKey(it->first, it->second)));
    it++;
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "C"), txn.GetCache().FindInstance(ECInstanceKey(it->first, it->second)));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_ClientRespondsWithSkipTokensAndCalledSecondTime_UsesPreviousPageETagsAndNewSkipTokens)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    ds->EnableSkipTokens(true);
    CachedResponseKey key = CreateTestResponseKey(ds);

    ON_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillByDefault(Return(CreateCompletedAsyncTask(WSObjectsResult::Error({}))));

    InSequence callsInSeq;

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, Utf8String(""), WSRepositoryClient::InitialSkipToken, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubInstances().ToWSObjectsResponse("ETagA", "SkipToken1")))));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, Utf8String(""), Utf8String("SkipToken1"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubInstances().ToWSObjectsResponse("ETagB", "")))));

    ASSERT_TRUE(ds->GetObjectsKeys(key, StubWSQuery(), CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult().IsSuccess());

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, Utf8String("ETagA"), WSRepositoryClient::InitialSkipToken, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubInstances().ToWSObjectsResponse("Foo", "SkipToken2")))));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, Utf8String("ETagB"), Utf8String("SkipToken2"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubInstances().ToWSObjectsResponse("Foo", "")))));

    ASSERT_TRUE(ds->GetObjectsKeys(key, StubWSQuery(), CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult().IsSuccess());
    }

// CachedData

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedDataAndQueryResponseNotCachedBackgroundSync_ErrorDoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedData, backgroundSync), nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::Status::DataNotCached, result.GetError().GetStatus());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedDataAndQueryResponseCachedBackgroundSync_BackgroundSyncUpdatesInstance)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto backgroundSync = SyncNotifier::Create();

    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedData, backgroundSync), nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, result.GetValue().GetSyncStatus());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::Synced, backgroundSyncResult.GetValue());

    auto cachedInstanceKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(cachedInstanceKey.IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedDataAndQueryResponseCachedBackgroundSyncNotChanged_BackgroundSyncReturnsNotModified)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubWSObjectsResponseNotModified()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedData, backgroundSync), nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, result.GetValue().GetSyncStatus());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotModified, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedDataAndQueryResponseCachedBackgroundSyncError_BackgroundSyncReturnsError)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedData, backgroundSync), nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, result.GetValue().GetSyncStatus());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_FALSE(backgroundSyncResult.IsSuccess());
    }

// RemoteData

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteDataNetworkErrorsBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteData, backgroundSync), nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteDataNotModifiedBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubWSObjectsResponseNotModified()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteData, backgroundSync), nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteDataBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto backgroundSync = SyncNotifier::Create(); 
    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteData, backgroundSync), nullptr)->GetResult();


    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

// CachedOrRemoteData

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedOrRemoteDataAndQueryResponseCachedBackgroundSync_BackgroundSyncUpdatesInstance)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedOrRemoteData, backgroundSync), nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::Synced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedOrRemoteDataAndQueryResponseNotCachedAndNetworkErrorBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedOrRemoteData, backgroundSync), nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedOrRemoteDataAndQueryResponseNotCachedBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedOrRemoteData, backgroundSync), nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

// RemoteOrCachedData

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteOrCachedDataResponseCachedAndNetworkErrorsBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteOrCachedData, backgroundSync), nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteOrCachedDataResponseNotCachedAndNetworkErrorsBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");
    
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteOrCachedData, backgroundSync), nullptr)->GetResult();
    ASSERT_FALSE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteOrCachedDataNotModifiedBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubWSObjectsResponseNotModified()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteOrCachedData, backgroundSync), nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteOrCachedDataBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse()));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObjectsKeys(key, query, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteOrCachedData, backgroundSync), nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_RemoteOrCachedDataAndConnectionError_ReturnsNetworkError)
    {
    auto ds = GetTestDataSourceV2();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    ObjectId objectId("TestSchema.TestClass", "Foo");
    auto result = ds->GetObject(objectId, CachingDataSource::DataOrigin::RemoteOrCachedData)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_RemoteOrCachedDataAndInstanceIsCachedAndConnectionError_ReturnsCached)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");
    StubInstances instances;
    instances.Add(objectId);

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto result = ds->GetObject(objectId, CachingDataSource::DataOrigin::RemoteOrCachedData)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[DataSourceCache_PROPERTY_RemoteId].asString());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_RemoteOrCachedDataAndInstanceIsCachedAndServerReturnsNewData_ReturnsNew)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, {{"TestProperty", "A"}});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    StubInstances newInstances;
    newInstances.Add(objectId, {{"TestProperty", "B"}});

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(newInstances.ToWSObjectsResult())));

    auto result = ds->GetObject(objectId, CachingDataSource::DataOrigin::RemoteOrCachedData)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[DataSourceCache_PROPERTY_RemoteId].asString());
    EXPECT_EQ("B", result.GetValue().GetJson()["TestProperty"].asString());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_RemoteDataAndNotModfieid_ReturnsCached)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, {{"TestProperty", "A"}}, "TestTag");

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, Utf8String("TestTag"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubWSObjectsResponseNotModified()))));

    auto result = ds->GetObject(objectId, CachingDataSource::DataOrigin::RemoteData)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[DataSourceCache_PROPERTY_RemoteId].asString());
    EXPECT_EQ("A", result.GetValue().GetJson()["TestProperty"].asString());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_RemoteDataAndNotEnoughRights_RemovesInstanceFromCache)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } }, "TestTag");

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));

    Json::Value instance;
    txn.GetCache().ReadInstance(objectId, instance);
    ASSERT_FALSE(instance.isNull());
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, Utf8String("TestTag"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError(WSError::Id::NotEnoughRights)))));

    auto result = ds->GetObject(objectId, CachingDataSource::DataOrigin::RemoteData)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Id::NotEnoughRights, result.GetError().GetWSError().GetId());
    txn = ds->StartCacheTransaction();
    txn.GetCache().ReadInstance(objectId, instance);
    txn.Commit();
    ASSERT_TRUE(instance.isNull());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_RemoteDataAndInstanceNotFound_RemovesInstanceFromCache)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } }, "TestTag");

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));

    Json::Value instance;
    txn.GetCache().ReadInstance(objectId, instance);
    ASSERT_FALSE(instance.isNull());
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, Utf8String("TestTag"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError(WSError::Id::InstanceNotFound)))));

    auto result = ds->GetObject(objectId, CachingDataSource::DataOrigin::RemoteData)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Id::InstanceNotFound, result.GetError().GetWSError().GetId());
    txn = ds->StartCacheTransaction();
    txn.GetCache().ReadInstance(objectId, instance);
    txn.Commit();
    ASSERT_TRUE(instance.isNull());
    }

//Cached Data

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_CachedDataAndQueryResponseNotCachedBackgroundSync_ErrorDoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedData, backgroundSync))->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::Status::DataNotCached, result.GetError().GetStatus());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_CachedDataAndQueryResponseCachedBackgroundSync_BackgroundSyncUpdatesInstance)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, {{ "TestProperty", "A" }});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    StubInstances newInstances;
    newInstances.Add(objectId, {{ "TestProperty", "B" }});

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(newInstances.ToWSObjectsResult())));

    auto backgroundSync = SyncNotifier::Create();

    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedData, backgroundSync))->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::Synced, backgroundSyncResult.GetValue());

    auto cachedInstanceKey = ds->StartCacheTransaction().GetCache().FindInstance({ "TestSchema.TestClass", "Foo" });
    EXPECT_TRUE(cachedInstanceKey.IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_CachedDataAndQueryResponseCachedBackgroundSyncNotChanged_BackgroundSyncReturnsNotModified)
    {
    auto ds = GetTestDataSourceV2();
    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubWSObjectsResponseNotModified()))));

    auto backgroundSync = SyncNotifier::Create();

    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedData, backgroundSync))->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::CachedData, result.GetValue().GetOrigin());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotModified, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_CachedDataAndQueryResponseCachedBackgroundSyncError_BackgroundSyncReturnsError)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, {{ "TestProperty", "A" }});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedData, backgroundSync))->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_FALSE(backgroundSyncResult.IsSuccess());
    }

// RemoteData

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_RemoteDataNetworkErrorsBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteData, backgroundSync))->GetResult();

    ASSERT_FALSE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_RemoteDataNotModifiedBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubWSObjectsResponseNotModified()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteData, backgroundSync))->GetResult();

    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_RemoteDataBackgroundSync_DoesNotSyncInBackground)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto ds = CreateMockedCachingDataSource(nullptr, cache);

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    EXPECT_CALL(*cache, ReadInstanceCacheTag(_)).WillOnce(Return("TagA"));

    EXPECT_CALL(*cache, UpdateInstance(_, _)).WillOnce(Return(CacheStatus::OK));

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    EXPECT_CALL(*cache, ReadInstance(_, _)).WillOnce(DoAll(SetArgReferee<1>("instance"), Return(CacheStatus::OK)));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteData, backgroundSync))->GetResult();

    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

// CachedOrRemoteData

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_CachedOrRemoteDataAndQueryResponseCachedBackgroundSync_BackgroundSyncUpdatesInstance)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedOrRemoteData, backgroundSync))->GetResult();

    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::Synced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_CachedOrRemoteDataAndQueryResponseNotCachedAndNetworkErrorBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedOrRemoteData, backgroundSync))->GetResult();

    ASSERT_FALSE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_CachedOrRemoteDataAndQueryResponseNotCachedBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedOrRemoteData, backgroundSync))->GetResult();
    ASSERT_FALSE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_CachedOrRemoteDataRemoteInstanceNotFound_BackgroundSyncReturnSynced)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError(WSError::Id::InstanceNotFound)))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedOrRemoteData, backgroundSync))->GetResult();

    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::Synced, backgroundSyncResult.GetValue());

    txn = ds->StartCacheTransaction();
    auto instanceKey = txn.GetCache().FindInstance(objectId);
    ASSERT_FALSE(instanceKey.IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_CachedOrRemoteDataRemoteNotEnoughRights_BackgroundSyncReturnSynced)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError(WSError::Id::NotEnoughRights)))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::CachedOrRemoteData, backgroundSync))->GetResult();

    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::Synced, backgroundSyncResult.GetValue());

    txn = ds->StartCacheTransaction();
    auto instanceKey = txn.GetCache().FindInstance(objectId);
    ASSERT_FALSE(instanceKey.IsValid());
    }
// RemoteOrCachedData

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_RemoteOrCachedDataResponseCachedAndNetworkErrorsBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteOrCachedData, backgroundSync))->GetResult();

    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_RemoteOrCachedDataResponseNotCachedAndNetworkErrorsBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteOrCachedData, backgroundSync))->GetResult();
    ASSERT_FALSE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_RemoteOrCachedDataNotModifiedBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubWSObjectsResponseNotModified()))));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteOrCachedData, backgroundSync))->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, GetObject_RemoteOrCachedDataBackgroundSync_DoesNotSyncInBackground)
    {
    auto ds = GetTestDataSourceV2();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, { { "TestProperty", "A" } });

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto backgroundSync = SyncNotifier::Create();
    auto result = ds->GetObject(objectId, ICachingDataSource::RetrieveOptions(CachingDataSource::DataOrigin::RemoteOrCachedData, backgroundSync))->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    auto backgroundSyncResult = backgroundSync->OnComplete()->GetResult();
    ASSERT_TRUE(backgroundSyncResult.IsSuccess());
    EXPECT_EQ(ICachingDataSource::SyncStatus::NotSynced, backgroundSyncResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_Default_CallsCommitLocalDeletionsBeforeGettingChanges)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    InSequence callsInSequence;
    EXPECT_CALL(cache->GetChangeManagerMock(), CommitLocalDeletions()).Times(1).WillOnce(Return(SUCCESS));
    EXPECT_CALL(cache->GetChangeManagerMock(), HasChanges()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(cache->GetChangeManagerMock(), GetChanges(An<IChangeManager::Changes&>(), _)).Times(1).WillOnce(Return(SUCCESS));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.GetValue().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedDeletedObject_CommitsLocalChangeAndDoesNoRequests)
    {
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto ecClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*ecClass, Json::objectValue);
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().DeleteObject(instance));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, txn.GetCache().GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    txn.Commit();

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.GetValue().empty());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, ds->StartCacheTransaction().GetCache().GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_NoChanges_DoesNoRequestsAndSucceeds)
    {
    auto ds = GetTestDataSourceV2();

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.GetValue().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
// TEST_F(CachingDataSourceTests, SyncLocalChanges_LaunchedFromTwoConnectionsToSameDb_SecondCallReturnsErrorFunctionalityNotSupported)
//    {
//    auto cache1 = std::make_shared<NiceMock<MockDataSourceCache>>();
//    auto cache2 = std::make_shared<NiceMock<MockDataSourceCache>>();
//    BeMutex c;
//    auto ds1 = CreateMockedCachingDataSource(nullptr, cache1);
//    auto ds2 = CreateMockedCachingDataSource(nullptr, cache2);
//
//    EXPECT_CALL(cache1->GetChangeManagerMock(), HasChanges()).WillOnce(Return(true));
//    EXPECT_CALL(cache2->GetChangeManagerMock(), HasChanges()).WillOnce(Return(true));
//
//    EXPECT_CALL(cache1->GetChangeManagerMock(), CommitLocalDeletions()).WillOnce(Return(SUCCESS));
//    EXPECT_CALL(cache2->GetChangeManagerMock(), CommitLocalDeletions()).WillOnce(Return(SUCCESS));
//
//    EXPECT_CALL(*cache1, GetCacheDatabasePath()).WillOnce(Return(BeFileName(L"samePath")));
//    EXPECT_CALL(*cache2, GetCacheDatabasePath()).WillOnce(Return(BeFileName(L"samePath")));
//
//    AsyncTestCheckpoint check1;
//    EXPECT_CALL(cache1->GetChangeManagerMock(), GetChanges(An<IChangeManager::Changes&>(), _)).WillOnce(Invoke([&] (IChangeManager::Changes&, bool)
//        {
//        check1.CheckinAndWait();
//        return SUCCESS;
//        }));
//    ON_CALL(cache2->GetChangeManagerMock(), GetChanges(An<IChangeManager::Changes&>(), _)).WillByDefault(Return(SUCCESS));
//
//    auto t1 = ds1->SyncLocalChanges(nullptr, nullptr);
//    check1.WaitUntilReached();
//    auto r2 = ds2->SyncLocalChanges(nullptr, nullptr)->GetResult();
//    check1.Continue();
//    auto r1 = t1->GetResult();
//
//    ASSERT_TRUE(r1.IsSuccess());
//    ASSERT_FALSE(r2.IsSuccess());
//    EXPECT_EQ(ICachingDataSource::Status::FunctionalityNotSupported, r2.GetError().GetStatus());
//    EXPECT_NE("", r2.GetError().GetMessage());
//    }
//

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F(CachingDataSourceTests, SyncLocalChanges_LaunchedFromTwoConnectionsToDifferentFiles_BothSucceeds_KnownIssue)
//    {
//    auto cache1 = std::make_shared<NiceMock<MockDataSourceCache>>();
//    auto cache2 = std::make_shared<NiceMock<MockDataSourceCache>>();
//
//    auto ds1 = CreateMockedCachingDataSource(nullptr, cache1);
//    auto ds2 = CreateMockedCachingDataSource(nullptr, cache2);
//
//    EXPECT_CALL(cache1->GetChangeManagerMock(), HasChanges()).WillOnce(Return(true));
//    EXPECT_CALL(cache2->GetChangeManagerMock(), HasChanges()).WillOnce(Return(true));
//
//    EXPECT_CALL(cache1->GetChangeManagerMock(), CommitLocalDeletions()).WillOnce(Return(SUCCESS));
//    EXPECT_CALL(cache2->GetChangeManagerMock(), CommitLocalDeletions()).WillOnce(Return(SUCCESS));
//
//    EXPECT_CALL(*cache1, GetCacheDatabasePath()).WillOnce(Return(BeFileName(L"someFilePath")));
//    EXPECT_CALL(*cache2, GetCacheDatabasePath()).WillOnce(Return(BeFileName(L"otherFilePath")));
//
//    AsyncTestCheckpoint check1;
//    EXPECT_CALL(cache1->GetChangeManagerMock(), GetChanges(An<IChangeManager::Changes&>(), _)).WillOnce(Invoke([&] (IChangeManager::Changes&, bool)
//        {
//        check1.CheckinAndWait();
//        return SUCCESS;
//        }));
//    ON_CALL(cache2->GetChangeManagerMock(), GetChanges(An<IChangeManager::Changes&>(), _)).WillByDefault(Return(SUCCESS));
//
//    auto t1 = ds1->SyncLocalChanges(nullptr, nullptr);
//    check1.WaitUntilReached();
//    auto r2 = ds2->SyncLocalChanges(nullptr, nullptr)->GetResult();
//    check1.Continue();
//    auto r1 = t1->GetResult();
//
//    ASSERT_TRUE(r1.IsSuccess());
//    ASSERT_TRUE(r2.IsSuccess());
//    }
//

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F(CachingDataSourceTests, SyncLocalChanges_LaunchedFromTwoConnectionsToMemmoryDb_BothSucceeds_KnownIssue)
//    {
//    auto cache1 = std::make_shared<NiceMock<MockDataSourceCache>>();
//    auto cache2 = std::make_shared<NiceMock<MockDataSourceCache>>();
//
//    auto ds1 = CreateMockedCachingDataSource(nullptr, cache1);
//    auto ds2 = CreateMockedCachingDataSource(nullptr, cache2);
//
//    EXPECT_CALL(cache1->GetChangeManagerMock(), HasChanges()).WillOnce(Return(true));
//    EXPECT_CALL(cache2->GetChangeManagerMock(), HasChanges()).WillOnce(Return(true));
//
//    EXPECT_CALL(cache1->GetChangeManagerMock(), CommitLocalDeletions()).WillOnce(Return(SUCCESS));
//    EXPECT_CALL(cache2->GetChangeManagerMock(), CommitLocalDeletions()).WillOnce(Return(SUCCESS));
//
//    EXPECT_CALL(*cache1, GetCacheDatabasePath()).WillOnce(Return(BeFileName(L":memory:")));
//    EXPECT_CALL(*cache2, GetCacheDatabasePath()).WillOnce(Return(BeFileName(L":memory:")));
//
//    AsyncTestCheckpoint check1;
//    EXPECT_CALL(cache1->GetChangeManagerMock(), GetChanges(An<IChangeManager::Changes&>(), _)).WillOnce(Invoke([&] (IChangeManager::Changes&, bool)
//        {
//        check1.CheckinAndWait();
//        return SUCCESS;
//        }));
//    ON_CALL(cache2->GetChangeManagerMock(), GetChanges(An<IChangeManager::Changes&>(), _)).WillByDefault(Return(SUCCESS));
//
//    auto t1 = ds1->SyncLocalChanges(nullptr, nullptr);
//    check1.WaitUntilReached();
//    auto r2 = ds2->SyncLocalChanges(nullptr, nullptr)->GetResult();
//    check1.Continue();
//    auto r1 = t1->GetResult();
//
//    ASSERT_TRUE(r1.IsSuccess());
//    ASSERT_TRUE(r2.IsSuccess());
//    }


/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CancelSync_AllFileTokensCancelled)
    {
    auto ds = GetTestDataSourceV2();
    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instance, StubFile(), true));
    txn.Commit();
    auto mainCt = SimpleCancellationToken::Create();
    auto fileCt = SimpleCancellationToken::Create();
    auto options = SyncOptions();
    options.AddFileCancellationToken(instance, fileCt);
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR creationJson, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr ct)
        {
        mainCt->SetCanceled();
        EXPECT_TRUE(ct->IsCanceled());
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(StubWSCanceledError()));
        }));
    auto result = ds->SyncLocalChanges(nullptr, mainCt, options)->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CancelSingleFileSync_AllFilesExceptOneSynced)
    {
    auto ds = GetTestDataSourceV2();
    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance1 = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    auto instance2 = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instance1, StubFile(), true));
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instance2, StubFile(), true));
    txn.Commit();
    SyncOptions options;
    options.AddFileCancellationToken(instance1, SimpleCancellationToken::Create(true));
    options.AddFileCancellationToken(instance2, SimpleCancellationToken::Create());

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _)).Times(2)
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr ct)
        {
        EXPECT_TRUE(ct->IsCanceled());
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "TestId"}));
        }))
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr ct)
        {
        EXPECT_FALSE(ct->IsCanceled());
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "TestId"}));
        }));
    ON_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubWSObjectsResult(StubObjectId()))));

    auto result = ds->SyncLocalChanges(nullptr, SimpleCancellationToken::Create(), options)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(options.GetFileCancellationToken(instance1)->IsCanceled());
    EXPECT_FALSE(options.GetFileCancellationToken(instance2)->IsCanceled());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreateObjectWithFiles_CallbacksCalled)
    {
    auto ds = GetTestDataSourceV2();
    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance1 = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    auto instance2 = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instance1, StubFile(), true));
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instance2, StubFile(), true));
    txn.Commit();

    auto mainCt = SimpleCancellationToken::Create();
    auto fileCt1 = SimpleCancellationToken::Create();
    auto fileCt2 = SimpleCancellationToken::Create();
    auto options = SyncOptions();
    options.AddFileCancellationToken(instance1, fileCt1);
    options.AddFileCancellationToken(instance2, fileCt2);

    MockFunction<void(ECInstanceKeyCR)> mockFunction;
    options.SetFileUploadFinishCallback(std::function<void(ECInstanceKeyCR)>([&] (ECInstanceKeyCR key)
        {
        mockFunction.Call(key);
        }));

    EXPECT_CALL(mockFunction, Call(_)).Times(2)
    .WillOnce(Invoke([&] (ECInstanceKeyCR key)
        {
        EXPECT_EQ(instance1, key);
        }))
    .WillOnce(Invoke([&] (ECInstanceKeyCR key)
        {
        EXPECT_EQ(instance2, key);
        }));

    ON_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "TestId"}))));
    ON_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubWSObjectsResult(StubObjectId()))));

    auto result = ds->SyncLocalChanges(nullptr, mainCt, options)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_NoFileCancellationTokens_UsesMainCancellationToken)
    {
    auto ds = GetTestDataSourceV2();
    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance1 = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instance1, StubFile(), true));
    txn.Commit();
    SyncOptions options;
    auto mainCt = SimpleCancellationToken::Create();

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr ct)
        {
        EXPECT_FALSE(ct->IsCanceled());
        mainCt->SetCanceled();
        EXPECT_TRUE(ct->IsCanceled());
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "TestId"}));
        }));
    ON_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubWSObjectsResult(StubObjectId()))));

    auto result = ds->SyncLocalChanges(nullptr, mainCt, options)->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_NonUploadingFileCancelled_Success)
    {
    auto ds = GetTestDataSourceV2();
    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance1 = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    auto instance2 = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instance1, StubFile(), true));
    txn.Commit();
    SyncOptions options;
    auto mainCt = SimpleCancellationToken::Create();
    auto fileCt2 = SimpleCancellationToken::Create();
    options.AddFileCancellationToken(instance2, fileCt2);
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _)).Times(2)
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr ct)
        {
        EXPECT_FALSE(ct->IsCanceled());
        EXPECT_FALSE(fileCt2->IsCanceled());
        fileCt2->SetCanceled();
        EXPECT_FALSE(ct->IsCanceled());
        EXPECT_TRUE(fileCt2->IsCanceled());
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "TestId"}));
        }))
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "TestId"}))));
    ON_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubWSObjectsResult(StubObjectId()))));

    auto result = ds->SyncLocalChanges(nullptr, mainCt, options)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_FileCancelled_FailureRegistered)
    {
    auto ds = GetTestDataSourceV2();
    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance1 = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instance1, StubFile(), true));
    txn.Commit();
    SyncOptions options;
    auto mainCt = SimpleCancellationToken::Create();
    auto fileCt1 = SimpleCancellationToken::Create(true);
    options.AddFileCancellationToken(instance1, fileCt1);
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr ct)
        {
        EXPECT_TRUE(ct->IsCanceled());
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(StubWSCanceledError()));
        }));
    ON_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubWSObjectsResult(StubObjectId()))));

    auto result = ds->SyncLocalChanges(nullptr, mainCt, options)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    auto resultValues = result.GetValue();
    ASSERT_EQ(1, resultValues.size());
    auto resultValue = resultValues[0];
    ASSERT_EQ(ICachingDataSource::Status::FileCancelled, resultValue.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObject_SetsSyncActiveFlagAndResetsItAfterSuccessfulSync)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_TRUE(instance.IsValid());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .Times(1)
        .WillOnce(InvokeWithoutArgs([&] ()
        {
        ds->GetCacheAccessThread()->ExecuteAsync([&]
            {
            EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(instance));
            });
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "Created"}));
        }));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Created"});
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(instance));
    ds->SyncLocalChanges(nullptr, nullptr)->Wait();
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(instance));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedRelationship_SetsSyncActiveFlagAndResetsItAfterSuccessfulSync)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto relationship = StubCreatedRelationshipInCache(txn.GetCache(),
                                   "TestSchema.TestRelationshipClass", {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"});
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(relationship));
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);

    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(relationship));
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(relationship));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CancelSync_IsActiveSyncResets)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto relationship = StubCreatedRelationshipInCache(txn.GetCache(),
                                                       "TestSchema.TestRelationshipClass", {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"});
    auto instance = StubCreatedObjectInCache(txn.GetCache());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(relationship));
        EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(instance));
        ds->CancelAllTasks();
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);

    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(relationship));
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(instance));
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(relationship));
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(instance));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObject_SetsSyncActiveFlagAndResetsItAfterFailedSync)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_TRUE(instance.IsValid());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .Times(1)
        .WillOnce(InvokeWithoutArgs([&] ()
        {
        ds->GetCacheAccessThread()->ExecuteAsync([=]
            {
            EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(instance));
            });
        return CreateCompletedAsyncTask(StubWSCreateObjectResult());
        }));

    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(instance));
    ds->SyncLocalChanges(nullptr, nullptr)->Wait();
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsUploadActive(instance));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObject_SendsCreateObjectRequestWithCorrectParameters)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    ASSERT_TRUE(txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty" : "42"})")).IsValid());
    txn.Commit();

    // Act & Assert
    Json::Value expectedCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "changeState": "new",
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "properties" :
                    {
                    "TestProperty" : "42"
                    }
                }
            })");

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, BeFileName(), _, _))
        .Times(1)
        .WillOnce(Invoke([=] (JsonValueCR json, BeFileNameCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(expectedCreationJson, json);
        return CreateCompletedAsyncTask(WSCreateObjectResult());
        }));

    ds->SyncLocalChanges(nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_ServerV2CreatedObject_SendsQueryRequestAndUpdatesInstanceClassAndProperties)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    ASSERT_TRUE(txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue).IsValid());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, BeFileName(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "NewId"}))));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillOnce(Invoke([=] (WSQueryCR query, Utf8StringCR, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_THAT(query.ToQueryString(), Eq("$filter=$id+eq+'NewId'"));
        EXPECT_THAT(query.GetSchemaName(), Eq("TestSchema"));
        EXPECT_THAT(query.GetClasses(), SizeIs(1));
        EXPECT_THAT(query.GetClasses(), Contains("TestClass!poly"));

        StubInstances instances;
        instances.Add({"TestSchema.TestDerivedClass", "NewId"}, {{"TestProperty", "TestValue"}});
        return CreateCompletedAsyncTask(instances.ToWSObjectsResult());
        }));

    ASSERT_TRUE(ds->SyncLocalChanges(nullptr, nullptr)->GetResult().IsSuccess());

    EXPECT_THAT(ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "NewId"}).IsValid(), false);
    EXPECT_THAT(ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestDerivedClass", "NewId"}).IsValid(), true);

    Json::Value jsonInstance;
    ds->StartCacheTransaction().GetCache().ReadInstance({"TestSchema.TestDerivedClass", "NewId"}, jsonInstance);
    EXPECT_THAT(jsonInstance["TestProperty"], Eq("TestValue"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_ServerV2CreatedObjectAndResponseContainsChangesetError_ErrorAsChangesetErrorsAreNotSupportedWithNonChangesetRequest)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_TRUE(instance.IsValid());
    txn.Commit();

    Json::Value responseJson = ToJson(R"({
        "changedInstance" :
            {
            "instanceAfterChange" : {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "error" : { "errorMessage" : "TestError", "httpStatusCode" : 409 }
                }
            }
        })");

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, BeFileName(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSCreateObjectResult::Success(WSUploadResponse(HttpStringBody::Create(responseJson.toStyledString()))))));
    ON_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));

    BeTest::SetFailOnAssert(false);
    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    BeTest::SetFailOnAssert(true);

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::InternalCacheError, result.GetError().GetStatus());
    txn = ds->StartCacheTransaction();
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, txn.GetCache().GetChangeManager().GetObjectChangeStatus(instance));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndNoChanges_DoesNoRequestsAndSucceeds)
    {
    auto ds = GetTestDataSource({2, 1});

    SyncOptions options;
    options.SetUseChangesets(true);
    auto result = ds->SyncLocalChanges(nullptr, nullptr, options)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.GetValue().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledCreatedModifiedDeletedObjects_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");

    auto instanceB = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "ToModify"});
    auto instanceC = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "ToDelete"});
    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty" : "NewValue"})"));
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyObject(instanceB, ToJson(R"({"TestProperty" : "ModifiedValue"})")));
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().DeleteObject(instanceC));

    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        expected.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty" : "NewValue"})"));
        expected.AddInstance({"TestSchema.TestClass", "ToModify"}, WSChangeset::Modified, ToJsonPtr(R"({"TestProperty" : "ModifiedValue"})"));
        expected.AddInstance({"TestSchema.TestClass", "ToDelete"}, WSChangeset::Deleted, nullptr);

        EXPECT_EQ(ToJson(expected.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedRelationship_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    StubCreatedRelationshipInCache(txn.GetCache(),
        "TestSchema.TestRelationshipClass", {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"});
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        expected.AddInstance({"TestSchema.TestClassA", "A"}, WSChangeset::Existing, nullptr)
            .AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
            {"TestSchema.TestClassB", "B"}, WSChangeset::Existing, nullptr);

        EXPECT_EQ(ToJson(expected.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndDeletedRelationship_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    StubInstances instances;
    instances.Add({"TestSchema.TestClassA", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClassB", "B"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(StubCachedResponseKey(txn.GetCache()), instances.ToWSObjectsResponse()));
    auto relClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = txn.GetCache().FindRelationship(*relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().DeleteRelationship(relationship));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        expected.AddInstance({"TestSchema.TestRelationshipClass", "AB"}, WSChangeset::Deleted, nullptr);

        EXPECT_EQ(ToJson(expected.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedRelatedTargetObject_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClassB = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClassB");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto source = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClassA", "ExistingId"});
    auto target = txn.GetCache().GetChangeManager().CreateObject(*testClassB, ToJson(R"({"TestProperty":"B"})"));
    auto relationship = txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, source, target);
    EXPECT_TRUE(relationship.IsValid());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        expected.AddInstance({"TestSchema.TestClassA", "ExistingId"}, WSChangeset::Existing, nullptr)
            .AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
            {"TestSchema.TestClassB", "B"}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"B"})"));

        EXPECT_EQ(ToJson(expected.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedRelatedSourceObject_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClassA = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClassA");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto source = txn.GetCache().GetChangeManager().CreateObject(*testClassA, ToJson(R"({"TestProperty":"A"})"));
    auto target = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClassB", "ExistingId"});
    auto relationship = txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, source, target);
    EXPECT_TRUE(relationship.IsValid());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        expected.AddInstance({"TestSchema.TestClassA", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"))
            .AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
            {"TestSchema.TestClassB", "ExistingId"}, WSChangeset::Existing, nullptr);

        EXPECT_EQ(ToJson(expected.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedObjectsWithForwardRelationship_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClassA = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClassA");
    auto testClassB = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClassB");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClassA, ToJson(R"({"TestProperty":"A"})"));
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClassB, ToJson(R"({"TestProperty":"B"})"));
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceB).IsValid());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        expected.AddInstance({"TestSchema.TestClassA", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"))
            .AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
            {"TestSchema.TestClassB", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"B"})"));

        EXPECT_EQ(ToJson(expected.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedObjectsWithBackwardRelationship_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClassA = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClassA");
    auto testClassB = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClassB");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClassA, ToJson(R"({"TestProperty":"A"})"));
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClassB, ToJson(R"({"TestProperty":"B"})"));
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceB, instanceA).IsValid());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        expected.AddInstance({"TestSchema.TestClassA", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"))
            .AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Backward,
            {"TestSchema.TestClassB", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"B"})"));

        EXPECT_EQ(ToJson(expected.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedObjectsWithManyRelatedObjects_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A"})"));

    // Related to "A"
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"B"})"));
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceB).IsValid());

    auto instanceC = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"C"})"));
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceC).IsValid());

    auto instanceE = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"D"})"));
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceE).IsValid());

    // Related to "C"
    auto instanceD = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"E"})"));
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceC, instanceD).IsValid());

    auto instanceF = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"F"})"));
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceC, instanceF).IsValid());

    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        auto& a = expected.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"));

        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"B"})"));

        auto& c = a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"C"})"));

        c.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"E"})"));

        c.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"F"})"));

        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"D"})"));

        EXPECT_EQ(ToJson(expected.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedRelatedObjectsAndSuccessfulResponse_CommitsRemoteIdsToInstances)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClassA = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClassA");
    auto testClassB = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClassB");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClassA, ToJson(R"({"TestProperty":"A"})"));
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClassB, ToJson(R"({"TestProperty":"B"})"));
    auto relationshipAB = txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceB);
    EXPECT_TRUE(relationshipAB.IsValid());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        StubInstances instances;
        auto instance = instances.Add({"TestSchema.TestClassA", "RemoteIdA"});
        instance.AddRelated({"TestSchema.TestRelationshipClass", "RemoteIdAB"}, {"TestSchema.TestClassB", "RemoteIdB"});
        return CreateCompletedAsyncTask(instances.ToWSChangesetResult());
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();

        {
        auto txn = ds->StartCacheTransaction();
        EXPECT_EQ(ObjectId("TestSchema.TestClassA", "RemoteIdA"), txn.GetCache().FindInstance(instanceA));
        EXPECT_EQ(ObjectId("TestSchema.TestClassB", "RemoteIdB"), txn.GetCache().FindInstance(instanceB));
        EXPECT_EQ(ObjectId("TestSchema.TestRelationshipClass", "RemoteIdAB"), txn.GetCache().FindRelationship(relationshipAB));
        EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetObjectChange(instanceA).GetChangeStatus());
        EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetObjectChange(instanceB).GetChangeStatus());
        EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetRelationshipChange(relationshipAB).GetChangeStatus());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedRelatedObjectToExistingAndSuccessfulResponse_CommitsRemoteIdsToInstances)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClassB = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClassB");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto instanceA = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClassA", "ExistingIdA"});
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClassB, ToJson(R"({"TestProperty":"B"})"));
    auto relationshipAB = txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceB);
    EXPECT_TRUE(relationshipAB.IsValid());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        StubInstances instances;
        auto instance = instances.Add({"TestSchema.TestClassA", "ExistingIdA"});
        instance.AddRelated({"TestSchema.TestRelationshipClass", "RemoteIdAB"}, {"TestSchema.TestClassB", "RemoteIdB"});

        return CreateCompletedAsyncTask(instances.ToWSChangesetResult());
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();

        {
        auto txn = ds->StartCacheTransaction();
        EXPECT_EQ(ObjectId("TestSchema.TestClassA", "ExistingIdA"), txn.GetCache().FindInstance(instanceA));
        EXPECT_EQ(ObjectId("TestSchema.TestClassB", "RemoteIdB"), txn.GetCache().FindInstance(instanceB));
        EXPECT_EQ(ObjectId("TestSchema.TestRelationshipClass", "RemoteIdAB"), txn.GetCache().FindRelationship(relationshipAB));
        EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetObjectChange(instanceA).GetChangeStatus());
        EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetObjectChange(instanceB).GetChangeStatus());
        EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetRelationshipChange(relationshipAB).GetChangeStatus());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledModifiedObjectAndSuccessfulResponse_CommitsChanges)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyObject(instance, ToJson(R"({"TestProperty" : "ModifiedValue"})")));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        auto body = HttpStringBody::Create(R"({"changedInstances" : [{"instanceAfterChange" : {}}]})");
        return CreateCompletedAsyncTask(WSChangesetResult::Success(body));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();

        {
        auto txn = ds->StartCacheTransaction();
        EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), txn.GetCache().FindInstance(instance));
        EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetObjectChange(instance).GetChangeStatus());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledDeletedObjectAndSuccessfulResponse_CommitsChanges)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().DeleteObject(instance));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        auto body = HttpStringBody::Create(R"({"changedInstances" : [{"instanceAfterChange" : {}}]})");
        return CreateCompletedAsyncTask(WSChangesetResult::Success(body));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();

        {
        auto txn = ds->StartCacheTransaction();
        EXPECT_FALSE(txn.GetCache().FindInstance({"TestSchema.TestClass", "A"}).IsValid());
        EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetObjectChange(instance).GetChangeStatus());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledDeletedRelationshipAndSuccessfulResponse_CommitsChanges)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    StubInstances instances;
    instances.Add({"TestSchema.TestClassA", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClassB", "B"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(StubCachedResponseKey(txn.GetCache()), instances.ToWSObjectsResponse()));
    auto relClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = txn.GetCache().FindRelationship(*relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().DeleteRelationship(relationship));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        auto body = HttpStringBody::Create(R"({"changedInstances" : [{"instanceAfterChange" : {}}]})");
        return CreateCompletedAsyncTask(WSChangesetResult::Success(body));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();

        {
        auto txn = ds->StartCacheTransaction();
        EXPECT_TRUE(txn.GetCache().FindInstance({"TestSchema.TestClassA", "A"}).IsValid());
        EXPECT_TRUE(txn.GetCache().FindInstance({"TestSchema.TestClassB", "B"}).IsValid());
        EXPECT_FALSE(txn.GetCache().FindRelationship(*relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"}).IsValid());
        EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndChangesetSizeLimited_SendsTwoChangesetsSoTheyWouldFitIntoLimit)
    {
    int maximumRequestSize = 800;

    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    ECInstanceKey source, target;

    source = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A"})"));

    target = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"B"})"));
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, source, target).IsValid());

    target = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"C"})"));
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, source, target).IsValid());

    target = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"D"})"));
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, source, target).IsValid());

    txn.Commit();

    // Act & Assert
    InSequence callsInSequence;

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        auto& a = expected.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"));
        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"B"})"));
        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"C"})"));
        EXPECT_LE(changesetBody->GetLength(), maximumRequestSize);
        EXPECT_EQ(ToJson(expected.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));

        StubInstances instances;
        auto instance = instances.Add({"TestSchema.TestClass", "RemoteIdA"});
        instance.AddRelated({"TestSchema.TestRelationshipClass", "RemoteIdAB"}, {"TestSchema.TestClass", "RemoteIdB"});
        instance.AddRelated({"TestSchema.TestRelationshipClass", "RemoteIdAC"}, {"TestSchema.TestClass", "RemoteIdC"});
        return CreateCompletedAsyncTask(instances.ToWSChangesetResult());
        }));

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        auto& a = expected.AddInstance({"TestSchema.TestClass", "RemoteIdA"}, WSChangeset::Existing, nullptr);
        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"D"})"));
        EXPECT_LE(changesetBody->GetLength(), maximumRequestSize);
        EXPECT_EQ(ToJson(expected.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));

        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    options.SetMaxChangesetSize(maximumRequestSize);
    auto result = ds->SyncLocalChanges(nullptr, nullptr, options)->GetResult();
    EXPECT_EQ(ICachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndChangesetSizeLimitIsSmallerThanInstance_ReturnsError)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A"})")).IsValid());
    txn.Commit();

    // Act & Assert
    SyncOptions options;
    options.SetUseChangesets(true);
    options.SetMaxChangesetSize(10);

    BeTest::SetFailOnAssert(false);
    auto result = ds->SyncLocalChanges(nullptr, nullptr, options)->GetResult();
    BeTest::SetFailOnAssert(true);

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(ICachingDataSource::Status::InternalCacheError, result.GetError().GetStatus());
    }
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndChangesetInstanceCountLimited_SendsTwoChangesetsSoTheyWouldFitIntoLimit)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    ECInstanceKey source, target;

    source = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A"})"));

    target = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"B"})"));
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, source, target).IsValid());

    target = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"C"})"));
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, source, target).IsValid());

    target = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"D"})"));
    EXPECT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, source, target).IsValid());

    txn.Commit();

    // Act & Assert
    InSequence callsInSequence;

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        auto& a = expected.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"));
        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"B"})"));
        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"C"})"));
        EXPECT_EQ(ToJson(expected.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));

        StubInstances instances;
        auto instance = instances.Add({"TestSchema.TestClass", "RemoteIdA"});
        instance.AddRelated({"TestSchema.TestRelationshipClass", "RemoteIdAB"}, {"TestSchema.TestClass", "RemoteIdB"});
        instance.AddRelated({"TestSchema.TestRelationshipClass", "RemoteIdAC"}, {"TestSchema.TestClass", "RemoteIdC"});

        return CreateCompletedAsyncTask(instances.ToWSChangesetResult());
        }));

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        auto& a = expected.AddInstance({"TestSchema.TestClass", "RemoteIdA"}, WSChangeset::Existing, nullptr);
        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"D"})"));
        EXPECT_EQ(ToJson(expected.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));

        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    options.SetMaxChangesetInstanceCount(3);
    auto result = ds->SyncLocalChanges(nullptr, nullptr, options)->GetResult();
    EXPECT_EQ(ICachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndOneObjectWithFile_InterruptsChangesetsWithCreateObjectRequestForFile)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");

    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A"})"));
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"B"})"));
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instanceB, StubFile(), true));
    auto filePath = txn.GetCache().ReadFilePath(instanceB);
    auto instanceC = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"C"})"));

    txn.Commit();

    WSChangeset expected1;
    WSChangeset expected2(WSChangeset::Format::SingeInstance);
    WSChangeset expected3;
    expected1.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
    expected3.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
    expected1.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::ChangeState::Created, ToJsonPtr(R"({"TestProperty":"A"})"));
    expected2.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::ChangeState::Created, ToJsonPtr(R"({"TestProperty":"B"})"));
    expected3.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::ChangeState::Created, ToJsonPtr(R"({"TestProperty":"C"})"));

    {
    InSequence callsInSequence;

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ToJson(expected1.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));

        StubInstances instances;
        instances.Add({"TestSchema.TestClass", "RemoteIdA"});

        return CreateCompletedAsyncTask(instances.ToWSChangesetResult());
        }));

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR creationJson, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ToJson(expected2.ToRequestString()), creationJson);
        EXPECT_EQ(filePath, path);
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "RemoteIdB"}));
        }));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillOnce(Invoke([&] (WSQueryCR query, Utf8StringCR, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_THAT(query.ToQueryString(), Eq("$filter=$id+eq+'RemoteIdB'"));

        StubInstances instances;
        instances.Add({"TestSchema.TestDerivedClass", "RemoteIdB"});

        return CreateCompletedAsyncTask(instances.ToWSObjectsResult());
        }));

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ToJson(expected3.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));

        StubInstances instances;
        instances.Add({"TestSchema.TestClass", "RemoteIdC"});

        return CreateCompletedAsyncTask(instances.ToWSChangesetResult());
        }));
    }

    SyncOptions options;
    options.SetUseChangesets(true);
    auto result = ds->SyncLocalChanges(nullptr, nullptr, options)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(0, result.GetValue().size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndRelatedObjectWithFile_SendsCreateObjectRequestForFile)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto relClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A"})"));
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"B"})"));
    auto relationshipAB = txn.GetCache().GetChangeManager().CreateRelationship(*relClass, instanceA, instanceB);
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instanceB, StubFile(), true));
    auto filePath = txn.GetCache().ReadFilePath(instanceB);

    txn.Commit();

    // Request 1
    WSChangeset expected1;
    expected1.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
    expected1.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::ChangeState::Created, ToJsonPtr(R"({"TestProperty":"A"})"));

    WSChangeset expected2(WSChangeset::SingeInstance);
    expected2.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::ChangeState::Created, ToJsonPtr(R"({"TestProperty":"B"})"))
        .AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::ChangeState::Created, ECRelatedInstanceDirection::Backward,
        {"TestSchema.TestClass", "RemoteIdA"}, WSChangeset::ChangeState::Existing, nullptr);

    {
    InSequence callsInSequence;

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ToJson(expected1.ToRequestString()), Json::Reader::DoParse(changesetBody->AsString()));

        StubInstances instances;
        instances.Add({"TestSchema.TestClass", "RemoteIdA"});

        return CreateCompletedAsyncTask(instances.ToWSChangesetResult());
        }));

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR creationJson, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ToJson(expected2.ToRequestString()), creationJson);
        EXPECT_EQ(filePath, path);
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(StubWSConnectionError()));
        }));
    }

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V20WithChangesetEnabledAndCreatedObject_SendsCreateObjectRequestsBecauseChangesetsAreNotSupported)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 0});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    ASSERT_TRUE(txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A"})")).IsValid());
    txn.Commit();

    WSChangeset expected(WSChangeset::Format::SingeInstance);
    expected.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::ChangeState::Created, ToJsonPtr(R"({"TestProperty":"A"})"));

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ToJson(expected.ToRequestString()), json);
        EXPECT_EQ(L"", path);
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndFailureStrategySet_SetsFailureStrategyInBody)
    {
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    txn.Commit();

    SyncOptions options;
    options.SetUseChangesets(true);

    // Default
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        Json::Value requestBody = ToJson(changesetBody->AsString());
        EXPECT_EQ("Stop", requestBody["requestOptions"]["FailureStrategy"].asString());
        return CreateCompletedAsyncTask(WSChangesetResult::Error({}));
        }));
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();

    // "Stop"
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        Json::Value requestBody = ToJson(changesetBody->AsString());
        EXPECT_EQ("Stop", requestBody["requestOptions"]["FailureStrategy"].asString());
        return CreateCompletedAsyncTask(WSChangesetResult::Error({}));
        }));
    options.SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();

    // "Continue"
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        Json::Value requestBody = ToJson(changesetBody->AsString());
        EXPECT_EQ("Continue", requestBody["requestOptions"]["FailureStrategy"].asString());
        return CreateCompletedAsyncTask(WSChangesetResult::Error({}));
        }));
    options.SetFailureStrategy(WSChangeset::Options::FailureStrategy::Continue);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndErrorsReceivedForInstance_FullErrorReturnedForFailedInstance)
    {
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    auto objectId = txn.GetCache().FindInstance(instance);
    EXPECT_TRUE(objectId.IsValid());
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        Json::Value responseJson = ToJson(R"({
        "changedInstances" :
            [{
            "instanceAfterChange" : {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "error" : { 
                    "errorId" : null,
                    "errorMessage" : "TestErrorMessage",
                    "errorDescription" : "TestErrorDescription",
                    "httpStatusCode" : 409 
                    }
                }
            }]
        })");
        return CreateCompletedAsyncTask(WSChangesetResult::Success(HttpStringBody::Create(responseJson.toStyledString())));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    options.SetFailureStrategy(WSChangeset::Options::FailureStrategy::Continue);
    auto result = ds->SyncLocalChanges(nullptr, nullptr, options)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(1, result.GetValue().size());
    auto failure = result.GetValue()[0];
    EXPECT_EQ(objectId, failure.GetObjectId());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, failure.GetError().GetStatus());
    EXPECT_EQ(WSError::Status::ReceivedError, failure.GetError().GetWSError().GetStatus());
    EXPECT_EQ(WSError::Id::Conflict, failure.GetError().GetWSError().GetId());
    EXPECT_EQ("TestErrorMessage", failure.GetError().GetMessage());
    EXPECT_EQ("TestErrorDescription", failure.GetError().GetDescription());

    txn = ds->StartCacheTransaction();
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, txn.GetCache().GetChangeManager().GetObjectChangeStatus(instance));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndErrorsReceivedForMultipleInstances_ErrorReturnedAndFailedInstancesNotSynced)
    {
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A"})"));
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"B"})"));
    auto instanceC = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"C"})"));
    auto instanceD = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"D"})"));
    auto objectIdB = txn.GetCache().FindInstance(instanceB);
    auto objectIdD = txn.GetCache().FindInstance(instanceD);
    EXPECT_TRUE(objectIdB.IsValid());
    EXPECT_TRUE(objectIdD.IsValid());
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        Json::Value responseJson = ToJson(R"({
        "changedInstances" :
            [{
            "instanceAfterChange" : {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "instanceId" : "NewIdA"
                }
            },{
            "instanceAfterChange" : {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "error" : { "errorMessage" : "TestErrorB", "httpStatusCode" : 409 }
                }
            },{
            "instanceAfterChange" : {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "instanceId" : "NewIdC"
                }
            },{
            "instanceAfterChange" : {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "error" : { "errorMessage" : "TestErrorD", "httpStatusCode" : 409 }
                }
            }]
        })");
        return CreateCompletedAsyncTask(WSChangesetResult::Success(HttpStringBody::Create(responseJson.toStyledString())));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    auto result = ds->SyncLocalChanges(nullptr, nullptr, options)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    auto failures = result.GetValue();
    ASSERT_EQ(2, failures.size());
    EXPECT_EQ(objectIdB, failures[0].GetObjectId());
    EXPECT_EQ(objectIdD, failures[1].GetObjectId());
    EXPECT_EQ("TestErrorB", failures[0].GetError().GetMessage());
    EXPECT_EQ("TestErrorD", failures[1].GetError().GetMessage());

    txn = ds->StartCacheTransaction();
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetObjectChangeStatus(instanceA));
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, txn.GetCache().GetChangeManager().GetObjectChangeStatus(instanceB));
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetObjectChangeStatus(instanceC));
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, txn.GetCache().GetChangeManager().GetObjectChangeStatus(instanceD));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndFirstResponseHasInstanceError_AddsFailureAndContinuesWithNextChangeset)
    {
    int maximumRequestSize = 300;

    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A"})"));
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"B"})"));
    auto instanceC = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"C"})"));
    auto instanceD = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"D"})"));
    auto objectIdB = txn.GetCache().FindInstance(instanceB);
    auto objectIdD = txn.GetCache().FindInstance(instanceD);
    EXPECT_TRUE(objectIdB.IsValid());
    EXPECT_TRUE(objectIdD.IsValid());
    txn.Commit();

    // Act & Assert
    InSequence callsInSequence;

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        expected.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"));
        expected.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"B"})"));
        EXPECT_EQ(ToJson(expected.ToRequestString()), ToJson(changesetBody->AsString()));

        Json::Value responseJson = ToJson(R"({
        "changedInstances" :
            [{
            "instanceAfterChange" : {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "instanceId" : "NewIdA"
                }
            },{
            "instanceAfterChange" : {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "error" : { "errorMessage" : "TestErrorB", "httpStatusCode" : 409 }
                }
            }]
        })");
        return CreateCompletedAsyncTask(WSChangesetResult::Success(HttpStringBody::Create(responseJson.toStyledString())));
        }));

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        expected.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"C"})"));
        expected.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"D"})"));
        EXPECT_EQ(ToJson(expected.ToRequestString()), ToJson(changesetBody->AsString()));

        StubInstances instances;
        instances.Add({"TestSchema.TestClass", "NewIdC"});
        instances.Add({"TestSchema.TestClass", "NewIdD"});
        return CreateCompletedAsyncTask(instances.ToWSChangesetResult());
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    options.SetMaxChangesetSize(maximumRequestSize);
    auto result = ds->SyncLocalChanges(nullptr, nullptr, options)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    auto failures = result.GetValue();
    ASSERT_EQ(1, failures.size());
    EXPECT_EQ(objectIdB, failures[0].GetObjectId());
    EXPECT_EQ("TestErrorB", failures[0].GetError().GetMessage());

    txn = ds->StartCacheTransaction();
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetObjectChangeStatus(instanceA));
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, txn.GetCache().GetChangeManager().GetObjectChangeStatus(instanceB));
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetObjectChangeStatus(instanceC));
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetObjectChangeStatus(instanceD));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndFirstResponseHasDependencyInstanceError_AddsFailureWithDependendInstancesAndContinues)
    {
    int maximumRequestSize = 600;

    // Arrange
    auto ds = GetTestDataSource({2, 3});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    ECInstanceKey parent = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A"})"));

    ECInstanceKey child1 = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"B"})"));
    ECInstanceKey rel1 = txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, parent, child1);

    ECInstanceKey child2 = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"C"})"));
    ECInstanceKey rel2 = txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, parent, child2);

    ECInstanceKey standalone = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"D"})"));

    EXPECT_TRUE(parent.IsValid());
    EXPECT_TRUE(rel1.IsValid());
    EXPECT_TRUE(rel2.IsValid());
    EXPECT_TRUE(standalone.IsValid());

    txn.Commit();

    // Act & Assert
    InSequence callsInSequence;

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        auto& a = expected.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"));
        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"B"})"));
        EXPECT_EQ(ToJson(expected.ToRequestString()), ToJson(changesetBody->AsString()));

        auto responseJson = ToJson(R"({
        "changedInstances" :
            [{
            "instanceAfterChange" :
                {
                "className" : "TestClass",
                "schemaName" : "TestSchema",
                "error" : { "errorMessage" : "TestErrorA", "httpStatusCode" : 409 },
                "relationshipInstances" :
                    [{
                    "className" : "TestRelationshipClass",
                    "schemaName" : "TestSchema",
                    "error" : { "errorMessage" : "TestErrorAB", "httpStatusCode" : 409 },
                    "relatedInstance" :
                        {
                        "className" : "TestClass",
                        "schemaName" : "TestSchema",
                        "error" : { "errorMessage" : "TestErrorB", "httpStatusCode" : 409 }
                        }
                    }]
                }
            }]
        })");
        return CreateCompletedAsyncTask(WSChangesetResult::Success(HttpStringBody::Create(responseJson.toStyledString())));
        }));

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset expected;
        expected.GetRequestOptions().SetFailureStrategy(WSChangeset::Options::FailureStrategy::Stop);
        expected.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"D"})"));
        // C should not be synced as it depended on failed parent.
        EXPECT_EQ(ToJson(expected.ToRequestString()), ToJson(changesetBody->AsString()));

        Json::Value responseJson = ToJson(R"({
        "changedInstances" :
            [{
            "instanceAfterChange" : {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "instanceId" : "NewIdD"
                }
            }]
        })");
        return CreateCompletedAsyncTask(WSChangesetResult::Success(HttpStringBody::Create(responseJson.toStyledString())));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    options.SetMaxChangesetSize(maximumRequestSize);
    auto result = ds->SyncLocalChanges(nullptr, nullptr, options)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    auto failures = result.GetValue();
    ASSERT_EQ(5, failures.size());
    txn = ds->StartCacheTransaction();
    EXPECT_EQ("TestErrorA", failures[0].GetError().GetMessage());
    EXPECT_EQ("TestErrorAB", failures[1].GetError().GetMessage());
    EXPECT_EQ("TestErrorB", failures[2].GetError().GetMessage());
    EXPECT_EQ("", failures[3].GetError().GetMessage());
    EXPECT_EQ("", failures[4].GetError().GetMessage());

    EXPECT_EQ(txn.GetCache().FindInstance(parent), failures[0].GetObjectId());
    EXPECT_EQ(txn.GetCache().FindRelationship(rel1), failures[1].GetObjectId());
    EXPECT_EQ(txn.GetCache().FindInstance(child1), failures[2].GetObjectId());
    EXPECT_EQ(txn.GetCache().FindRelationship(rel2), failures[3].GetObjectId());
    EXPECT_EQ(txn.GetCache().FindInstance(child2), failures[4].GetObjectId());

    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, failures[0].GetError().GetStatus());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, failures[1].GetError().GetStatus());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, failures[2].GetError().GetStatus());
    EXPECT_EQ(CachingDataSource::Status::DependencyNotSynced, failures[3].GetError().GetStatus());
    EXPECT_EQ(CachingDataSource::Status::DependencyNotSynced, failures[4].GetError().GetStatus());

    EXPECT_EQ(IChangeManager::ChangeStatus::Created, txn.GetCache().GetChangeManager().GetObjectChangeStatus(parent));
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, txn.GetCache().GetChangeManager().GetObjectChangeStatus(child1));
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, txn.GetCache().GetChangeManager().GetObjectChangeStatus(child2));
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, txn.GetCache().GetChangeManager().GetRelationshipChange(rel1).GetChangeStatus());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, txn.GetCache().GetChangeManager().GetRelationshipChange(rel2).GetChangeStatus());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, txn.GetCache().GetChangeManager().GetObjectChangeStatus(standalone));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_V2CreatedRelatedObjectsWithFile_SendsSeperateRequestsForEachNewObjectAndRelationship)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto instanceA = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty" : "ValB"})"));
    auto instanceC = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty" : "ValC"})"));
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instanceC, StubFile(), false));

    ASSERT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceB).IsValid());
    ASSERT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceB, instanceC).IsValid());
    txn.Commit();

    // Act & Assert
    Json::Value expectedCreationJson1 = ToJson(
        R"( {
            "instance" :
                {
                "changeState": "new",
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "properties" :
                    {
                    "TestProperty" : "ValB"
                    },
                "relationshipInstances" :
                    [{
                    "changeState": "new",
                    "schemaName" : "TestSchema",
                    "className" : "TestRelationshipClass",
                    "direction" : "backward",
                    "relatedInstance" :
                        {
                        "changeState" : "existing",
                        "schemaName" : "TestSchema",
                        "className" : "TestClass",
                        "instanceId" : "A"
                        }
                    }]
                }
            })");
    Json::Value expectedCreationJson2 = ToJson(
        R"( {
            "instance" :
                {
                "changeState": "new",
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "properties" :
                    {
                    "TestProperty" : "ValC"
                    },
                "relationshipInstances" :
                    [{
                    "changeState": "new",
                    "schemaName" : "TestSchema",
                    "className" : "TestRelationshipClass",
                    "direction" : "backward",
                    "relatedInstance" :
                        {
                        "changeState" : "existing",
                        "schemaName" : "TestSchema",
                        "className" : "TestDerivedClass",
                        "instanceId" : "NewB"
                        }
                    }]
                }
            })");
    BeFileName filePath2 = ds->StartCacheTransaction().GetCache().ReadFilePath(instanceC);

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(expectedCreationJson1, json);
        EXPECT_EQ(L"", path);
        return CreateCompletedAsyncTask(StubWSCreateObjectResult(
            {"TestSchema.TestClass", "NewB"}, {"TestSchema.TestRelationshipClass", ""}, {"TestSchema.TestClass", "A"}));
        }))
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
            {
            EXPECT_EQ(expectedCreationJson2, json);
            EXPECT_EQ(filePath2, path);
            return CreateCompletedAsyncTask(StubWSCreateObjectResult(
                {"TestSchema.TestClass", "NewC"}, {"TestSchema.TestRelationshipClass", ""}, {"TestSchema.TestClass", "NewB"}));
            }));

        EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
            .WillOnce(Invoke([=] (WSQueryCR query, Utf8StringCR, Utf8StringCR, ICancellationTokenPtr)
            {
            EXPECT_THAT(query.ToQueryString(), Eq("$filter=$id+eq+'NewB'"));
            StubInstances instances;
            instances.Add({"TestSchema.TestDerivedClass", "NewB"});
            return CreateCompletedAsyncTask(instances.ToWSObjectsResult());
            }))
            .WillOnce(Invoke([=] (WSQueryCR query, Utf8StringCR, Utf8StringCR, ICancellationTokenPtr)
                {
                EXPECT_THAT(query.ToQueryString(), Eq("$filter=$id+eq+'NewC'"));
                StubInstances instances;
                instances.Add({"TestSchema.TestDerivedClass", "NewC"});
                return CreateCompletedAsyncTask(instances.ToWSObjectsResult());
                }));

            auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
            EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedTwoRelatedInstancesAndFirstOneFails_SecondOneFailureHasDependencySyncFailedStatus)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto instanceA = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    auto instanceC = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);

    ASSERT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceB).IsValid());
    ASSERT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceB, instanceC).IsValid());
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSCreateObjectResult::Error(WSError(WSError::Id::Conflict)))));

    BeTest::SetFailOnAssert(false);
    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    BeTest::SetFailOnAssert(true);
    EXPECT_TRUE(result.IsSuccess());

    ASSERT_THAT(result.GetValue(), SizeIs(4));
    EXPECT_THAT(result.GetValue()[1].GetObjectId(), Eq(ds->StartCacheTransaction().GetCache().FindInstance(instanceB)));
    EXPECT_THAT(result.GetValue()[1].GetError().GetStatus(), ICachingDataSource::Status::NetworkErrorsOccured);
    EXPECT_THAT(result.GetValue()[3].GetObjectId(), Eq(ds->StartCacheTransaction().GetCache().FindInstance(instanceC)));
    EXPECT_THAT(result.GetValue()[3].GetError().GetStatus(), ICachingDataSource::Status::DependencyNotSynced);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectWithTwoRelationships_SecondRelationshipCreationSentSeperately)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto instanceA = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    auto instanceB = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "B"});
    auto instanceC = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty" : "ValC"})"));

    ASSERT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceC).IsValid());
    ASSERT_TRUE(txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceB, instanceC).IsValid());
    txn.Commit();

    // Act & Assert
    Json::Value expectedCreationJson1 = ToJson(
        R"( {
            "instance" :
                {
                "changeState": "new",
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "properties" :
                    {
                    "TestProperty" : "ValC"
                    },
                "relationshipInstances" :
                    [{
                    "changeState": "new",
                    "schemaName" : "TestSchema",
                    "className" : "TestRelationshipClass",
                    "direction" : "backward",
                    "relatedInstance" :
                        {
                        "changeState" : "existing",
                        "schemaName" : "TestSchema",
                        "className" : "TestClass",
                        "instanceId" : "A"
                        }
                    }]
                }
            })");

    Json::Value expectedCreationJson2 = ToJson(
        R"( {
            "instance" :
                {
                "changeState" : "existing",
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "instanceId" : "B",
                "relationshipInstances" :
                    [{
                    "changeState": "new",
                    "schemaName" : "TestSchema",
                    "className" : "TestRelationshipClass",
                    "direction" : "forward",
                    "relatedInstance" :
                        {
                        "changeState" : "existing",
                        "schemaName" : "TestSchema",
                        "className" : "TestClass",
                        "instanceId" : "NewC"
                        }
                    }]
                }
            })");

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .Times(2)
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(expectedCreationJson1, json);
        return CreateCompletedAsyncTask(StubWSCreateObjectResult(
            {"TestSchema.TestClass", "NewC"}, {"TestSchema.TestRelationshipClass", ""}, {"TestSchema.TestClass", "A"}));
        }))
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
            {
            EXPECT_EQ(expectedCreationJson2, json);
            return CreateCompletedAsyncTask(StubWSCreateObjectResult
                (
                {"TestSchema.TestClass", "B"},
                {"TestSchema.TestRelationshipClass", "BC"},
                {"TestSchema.TestClass", "NewC"})
                );
            }));

        EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
            .Times(1)
            .WillOnce(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "NewC"}))));

        auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
        EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjet_SetsNewRemoteIdAndCommits)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "CreatedObjectId"}))));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "CreatedObjectId"}))));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo(instance).IsInCache());
    EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "CreatedObjectId"}).IsInCache());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "CreatedObjectId"}).GetChangeStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectWithReadOnlyProperties_SendsReadOnlyButNotCalculatedProperty)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass3");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({ "TestReadOnlyProperty" : "42" })"));
    ASSERT_TRUE(instance.IsValid());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, BeFileName(), _, _))
        .Times(1)
        .WillOnce(Invoke([=] (JsonValueCR json, BeFileNameCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_TRUE(json["instance"]["properties"].isMember("TestReadOnlyProperty"));
        EXPECT_FALSE(json["instance"]["properties"].isMember("TestCalculatedProperty"));
        return CreateCompletedAsyncTask(WSCreateObjectResult());
        }));
    ds->SyncLocalChanges(nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedObjectWithReadOnlyProperties_DoesNotSendAnyReadOnlyProperties)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass3", "Foo"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyObject(instance, ToJson(R"({ "TestReadOnlyProperty" : "42" })")));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateObjectRequest(_, _, _, _, _, _))
        .Times(1)
        .WillOnce(Invoke([=] (ObjectIdCR, JsonValueCR properties, Utf8String, BeFileName, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_FALSE(properties.isMember("TestReadOnlyProperty"));
        EXPECT_FALSE(properties.isMember("TestCalculatedProperty"));
        return CreateCompletedAsyncTask(WSUpdateObjectResult());
        }));

    ds->SyncLocalChanges(nullptr, nullptr)->Wait();
    }
/*--------------------------------------------------------------------------------------+
* @bsitest                             Benediktas.Lipnickas                     02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_FailedToModifyObject_ReturnsFailedObject)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache());
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyObject(instance, Json::objectValue));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateObjectRequest(_, _, _, _, _, _))
        .WillOnce(Invoke([=] (ObjectIdCR, JsonValueCR properties, Utf8String, BeFileName, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        return CreateCompletedAsyncTask(WSUpdateObjectResult::Error(WSError(StubWSErrorHttpResponse(HttpStatus::BadRequest))));
        }));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    ASSERT_THAT(result.GetValue(), SizeIs(1));
    EXPECT_THAT(result.GetValue()[0].GetObjectId(), Eq(ds->StartCacheTransaction().GetCache().FindInstance(instance)));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedObject_SendUpdateObjectRequestWithOnlyChangedPropertiesAndCommits)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "Foo"}, {{"TestProperty", "OldA"}, {"TestProperty2", "OldB"}});

    Json::Value newPropertiesJson = ToJson(R"({ "TestProperty" : "NewA", "TestProperty2" : "OldB" })");
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyObject(instance, newPropertiesJson));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateObjectRequest(_, _, _, _, _, _))
        .Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, JsonValueCR propertiesJson, Utf8String, BeFileName, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("TestSchema.TestClass", "Foo"), objectId);
        EXPECT_EQ(ToJson(R"({ "TestProperty" : "NewA" })"), propertiesJson);
        return CreateCompletedAsyncTask(WSUpdateObjectResult::Success({}));
        }));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, ds->StartCacheTransaction().GetCache().GetCachedObjectInfo(instance).GetChangeStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedFile_SendUpdateFileRequestWithCorrectParametersAndCommits)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instance, StubFile(), false));
    auto cachedFilePath = txn.GetCache().ReadFilePath(instance);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateFileRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("TestSchema.TestClass", "Foo"), objectId);
        EXPECT_EQ(cachedFilePath, filePath);
        return CreateCompletedAsyncTask(WSUpdateObjectResult::Success({nullptr, "NewTag"}));
        }));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, ds->StartCacheTransaction().GetCache().GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_EQ("NewTag", ds->StartCacheTransaction().GetCache().ReadFileCacheTag({"TestSchema.TestClass", "Foo"}));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectWithFile_SendUpdateFileRequestWithCorrectParametersAndCommits)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instance, StubFile(), false));
    auto cachedFilePath = txn.GetCache().ReadFilePath(instance);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR filePath, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(cachedFilePath, filePath);
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "Foo"}, "NewTag"));
        }));

    ON_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillByDefault(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "Foo"}))));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, ds->StartCacheTransaction().GetCache().GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, ds->StartCacheTransaction().GetCache().GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_EQ("NewTag", ds->StartCacheTransaction().GetCache().ReadFileCacheTag({"TestSchema.TestClass", "Foo"}));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectWithFile_ProgressCallbackCalledWithCorrectParameters)
    {
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    auto fileContent = Utf8String("Hello world over here");
    auto file = StubFile(fileContent);
    auto filelength = (double)fileContent.size();
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instance, file, false));
    auto cachedFilePath = txn.GetCache().ReadFilePath(instance);
    txn.Commit();

    ON_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillByDefault(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "Foo"}))));

    MockFunction<void(ICachingDataSource::ProgressCR)> mockFunction;
    auto onProgress = [&] (ICachingDataSource::ProgressCR progress)
        {
        mockFunction.Call(progress);
        };

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR filePath, Http::Request::ProgressCallbackCR uploadProgressCallback, ICancellationTokenPtr)
        {
        uploadProgressCallback(5.0, filelength);
        uploadProgressCallback(10.0, filelength);
        uploadProgressCallback(filelength, filelength);

        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "Foo"}));
        }));

        {
        InSequence dummy;
        EXPECT_CALL(mockFunction, Call(_)).WillOnce(Invoke([&] (ICachingDataSource::ProgressCR progress)
            {
            EXPECT_EQ(instance, progress.GetCurrentFileKey());
            EXPECT_EQ(ICachingDataSource::Progress::State(), progress.GetCurrentFileBytes());
            }));
        EXPECT_CALL(mockFunction, Call(_)).WillOnce(Invoke([&] (ICachingDataSource::ProgressCR progress)
            {
            EXPECT_EQ(instance, progress.GetCurrentFileKey());
            EXPECT_EQ(ICachingDataSource::Progress::State(5.0, filelength), progress.GetCurrentFileBytes());
            }));
        EXPECT_CALL(mockFunction, Call(_)).WillOnce(Invoke([&] (ICachingDataSource::ProgressCR progress)
            {
            EXPECT_EQ(instance, progress.GetCurrentFileKey());
            EXPECT_EQ(ICachingDataSource::Progress::State(10.0, filelength), progress.GetCurrentFileBytes());
            }));
        EXPECT_CALL(mockFunction, Call(_)).WillOnce(Invoke([&] (ICachingDataSource::ProgressCR progress)
            {
            EXPECT_EQ(instance, progress.GetCurrentFileKey());
            EXPECT_EQ(ICachingDataSource::Progress::State(filelength, filelength), progress.GetCurrentFileBytes());
            }));
        EXPECT_CALL(mockFunction, Call(_)).WillOnce(Invoke([&] (ICachingDataSource::ProgressCR progress)
            {
            EXPECT_DOUBLE_EQ(1.0, progress.GetSynced());
            }));
        }

    auto result = ds->SyncLocalChanges(onProgress, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectWithout_ProgressCallbackCalledWithoutFileProgress)
    {
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);

    txn.Commit();

    ON_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillByDefault(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "Foo"}))));

    MockFunction<void(ICachingDataSource::ProgressCR)> mockFunction;
    auto onProgress = [&] (ICachingDataSource::ProgressCR progress)
        {
        mockFunction.Call(progress);
        };

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR filePath, Http::Request::ProgressCallbackCR uploadProgressCallback, ICancellationTokenPtr)
        {
        uploadProgressCallback(0.0, 0.0);

        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "Foo"}));
        }));

        EXPECT_CALL(mockFunction, Call(_)).Times(2).WillRepeatedly(Invoke([&] (ICachingDataSource::ProgressCR progress)
            {
            EXPECT_FALSE(progress.GetCurrentFileKey().IsValid());
            }));

        auto result = ds->SyncLocalChanges(onProgress, nullptr)->GetResult();
        ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_WebApi24AndCreatedObjectWithFile_ChecksIfRepositorySupportsFileAccessUrl)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto instance = StubCreatedFileInCache(txn.GetCache(), "TestSchema.TestClass");
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillOnce(Invoke([&] (WSQueryCR query, Utf8StringCR, Utf8StringCR, ICancellationTokenPtr ct)
        {
        EXPECT_EQ("Policies/PolicyAssertion?$select=$id&$filter=Name+eq+'SupportsFileAccessUrl'+and+Supported+eq+true", query.ToFullString());
        EXPECT_NE(nullptr, ct);
        return CreateCompletedAsyncTask(WSObjectsResult::Error({}));
        }));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_WebApi24AndCreatedObjectWithFileAndFileAccessUrlNotSupported_UploadsInstanceAndFileInOneRequest)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto instance = StubCreatedFileInCache(txn.GetCache(), "TestSchema.TestClass");
    auto filePath = txn.GetCache().ReadFilePath(instance);
    ASSERT_TRUE(filePath.DoesPathExist());
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillOnce(Invoke([&] (WSQueryCR query, Utf8StringCR, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_EQ("Policies", query.GetSchemaName());
        return CreateCompletedAsyncTask(StubWSObjectsResult());
        }));

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr ct)
        {
        EXPECT_EQ(filePath, path);
        EXPECT_NE(nullptr, ct);

        EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
            .WillOnce(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "NewRemoteId"}))));
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "NewRemoteId"}));
        }));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_WebApi24AndCreatedObjectWithFileAndFileAccessUrlSupported_UploadsInstanceAndFileSeperately)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto instance = StubCreatedFileInCache(txn.GetCache(), "TestSchema.TestClass");
    auto filePath = txn.GetCache().ReadFilePath(instance);
    ASSERT_TRUE(filePath.DoesPathExist());
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillOnce(Invoke([&] (WSQueryCR query, Utf8StringCR, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_EQ("Policies", query.GetSchemaName());
        return CreateCompletedAsyncTask(StubWSObjectsResult(StubObjectId()));
        }));

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr ct)
        {
        EXPECT_EQ(L"", path);
        EXPECT_NE(nullptr, ct);

        EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
            .WillOnce(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "NewRemoteId"}))));
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "NewRemoteId"}));
        }));

    EXPECT_CALL(GetMockClient(), SendUpdateFileRequest(_, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr ct)
        {
        EXPECT_EQ(ObjectId("TestSchema.TestClass", "NewRemoteId"), objectId);
        EXPECT_EQ(filePath, path);
        EXPECT_NE(nullptr, ct);

        EXPECT_CALL(GetMockClient(), SendGetObjectRequest(ObjectId("TestSchema.TestClass", "NewRemoteId"), _, _))
            .WillOnce(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "NewRemoteId"}))));
        return CreateCompletedAsyncTask(WSUpdateFileResult::Success({}));
        }));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_WebApi24AndCreatedObjectWithFileAndFileAccessUrlSupportedAndCreationFails_DoesNotUploadFile)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto instance = StubCreatedFileInCache(txn.GetCache(), "TestSchema.TestClass");
    auto filePath = txn.GetCache().ReadFilePath(instance);
    ASSERT_TRUE(filePath.DoesPathExist());
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillOnce(Invoke([&] (WSQueryCR query, Utf8StringCR, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_EQ("Policies", query.GetSchemaName());
        return CreateCompletedAsyncTask(StubWSObjectsResult(StubObjectId()));
        }));

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(L"", path);
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error({WSError::Id::Conflict}));
        }));

    EXPECT_CALL(GetMockClient(), SendUpdateFileRequest(_, _, _, _)).Times(0);

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ(1, result.GetValue().size());
    EXPECT_EQ(instance, ds->StartCacheTransaction().GetCache().FindInstance(result.GetValue()[0].GetObjectId()));
    EXPECT_EQ(WSError::Id::Conflict, result.GetValue()[0].GetError().GetWSError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_WebApi24AndCreatedObjectWithFileAndCalledSecondTime_PoliciesCheckDoneOnce)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto instance = StubCreatedFileInCache(txn.GetCache(), "TestSchema.TestClass");
    auto filePath = txn.GetCache().ReadFilePath(instance);
    ASSERT_TRUE(filePath.DoesPathExist());
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillOnce(Invoke([&] (WSQueryCR query, Utf8StringCR, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_EQ("Policies", query.GetSchemaName());
        return CreateCompletedAsyncTask(StubWSObjectsResult());
        }));

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(filePath, path);
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(StubHttpResponse()));
        }));

    ASSERT_FALSE(ds->SyncLocalChanges(nullptr, nullptr)->GetResult().IsSuccess());

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(filePath, path);
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(StubHttpResponse()));
        }));

    ASSERT_FALSE(ds->SyncLocalChanges(nullptr, nullptr)->GetResult().IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_WebApi24AndCreatedObjectWithFileAndFileAccessUrlIsSupportedAndCalledSecondTime_PoliciesCheckDoneOnce)
    {
    auto ds = GetTestDataSourceV24();
    auto txn = ds->StartCacheTransaction();
    auto instance = StubCreatedFileInCache(txn.GetCache(), "TestSchema.TestClass");
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillOnce(Invoke([&] (WSQueryCR query, Utf8StringCR, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_EQ("Policies", query.GetSchemaName());
        return CreateCompletedAsyncTask(StubWSObjectsResult(StubObjectId()));
        }));

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(L"", path);
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(StubHttpResponse()));
        }));

    ASSERT_FALSE(ds->SyncLocalChanges(nullptr, nullptr)->GetResult().IsSuccess());

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR path, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(L"", path);
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(StubHttpResponse()));
        }));

    ASSERT_FALSE(ds->SyncLocalChanges(nullptr, nullptr)->GetResult().IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_DeletedObject_SendsDeleteObjectRequestWithCorrectParametersAndCommits)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().DeleteObject(instance));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendDeleteObjectRequest(_, _))
        .Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("TestSchema.TestClass", "Foo"), objectId);
        return CreateCompletedAsyncTask(WSDeleteObjectResult::Success());
        }));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, ds->StartCacheTransaction().GetCache().GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_DeletedRelationship_SendsDeleteObjectRequestWithCorrectParametersAndCommits)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(StubCachedResponseKey(txn.GetCache()), instances.ToWSObjectsResponse()));
    auto relClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = txn.GetCache().FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().DeleteRelationship(relationship));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendDeleteObjectRequest(_, _))
        .Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("TestSchema.TestRelationshipClass", "AB"), objectId);
        return CreateCompletedAsyncTask(WSDeleteObjectResult::Success());
        }));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    auto changeStatus = ds->StartCacheTransaction().GetCache().GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus();
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, changeStatus);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectWithClassThatHasLabel_CallsProgressWithLabelAndWithoutBytes)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestLabeledClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"Name" : "TestLabel"})"));
    ASSERT_TRUE(instance.IsValid());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSCreateObjectResult())));

    int onProgressCount = 0;
    ICachingDataSource::Progress expectedProgress({}, std::make_shared<Utf8String>("TestLabel"));
    auto onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        onProgressCount++;
        EXPECT_EQ(expectedProgress, progress);
        };

    ds->SyncLocalChanges(onProgress, nullptr)->Wait();
    EXPECT_EQ(1, onProgressCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectWithClassWithoutLabel_CallsProgressWithFallbackLabel)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    auto objectId = txn.GetCache().FindInstance(instance);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSCreateObjectResult())));

    int onProgressCount = 0;
    auto onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        onProgressCount++;
        EXPECT_EQ("TestClass:" + objectId.remoteId, progress.GetLabel());
        };

    ds->SyncLocalChanges(onProgress, nullptr)->Wait();
    EXPECT_EQ(1, onProgressCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedObjectWithLabel_CallsProgressWithLabelAndWithoutBytes)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestLabeledClass", "Foo"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyObject(instance, ToJson(R"({"Name" : "TestLabel"})")));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateObjectRequest(_, _, _, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSUpdateObjectResult())));

    int onProgressCount = 0;
    auto onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        onProgressCount++;
        EXPECT_EQ("TestLabel", progress.GetLabel());
        };

    ds->SyncLocalChanges(onProgress, nullptr)->Wait();
    EXPECT_EQ(1, onProgressCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedAndModifiedAndDeletedObjects_CallsSyncedInstancesProgress)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    auto txn = ds->StartCacheTransaction();

    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instanceB = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "B"});
    auto instanceC = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "C"});
    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyObject(instanceB, Json::objectValue));
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().DeleteObject(instanceC));

    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "Foo"}))));

    ON_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
        .WillByDefault(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "Foo"}))));

    EXPECT_CALL(GetMockClient(), SendUpdateObjectRequest(_, _, _, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSUpdateObjectResult::Success({}))));

    EXPECT_CALL(GetMockClient(), SendDeleteObjectRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSDeleteObjectResult::Success())));

    int onProgressCount = 0;
    double expectedSyncedValues[4] = {0, 0.33, 0.66, 1};
    auto onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_EQ(expectedSyncedValues[onProgressCount], progress.GetSynced());
        onProgressCount++;
        };

    ds->SyncLocalChanges(onProgress, nullptr)->Wait();
    EXPECT_EQ(4, onProgressCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedObject_CallsSyncedInstanceProgress)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestLabeledClass", "Foo"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyObject(instance, ToJson(R"({"Name" : "TestLabelA"})")));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateObjectRequest(_, _, _, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSUpdateObjectResult::Success({}))));

    int onProgressCount = 0;
    double expectedSyncedValues[2] = {0, 1};
    auto onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_EQ(expectedSyncedValues[onProgressCount], progress.GetSynced());
        onProgressCount++;
        };

    ds->SyncLocalChanges(onProgress, nullptr)->Wait();
    EXPECT_EQ(2, onProgressCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedFileWithLabel_CallsProgressWithLabel)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheInstanceAndLinkToRoot({"TestSchema.TestLabeledClass", "Foo"}, *ToRapidJson(R"({"Name" : "TestLabel"})"), "", ""));
    auto instance = txn.GetCache().FindInstance({"TestSchema.TestLabeledClass", "Foo"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instance, StubFile("12"), false));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateFileRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        progress(0, 2);
        return CreateCompletedAsyncTask(WSUpdateFileResult());
        }));

    int onProgressCount = 0;
    auto onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        onProgressCount++;
        EXPECT_EQ("TestLabel", progress.GetLabel());
        };

    ds->SyncLocalChanges(onProgress, nullptr)->Wait();
    EXPECT_EQ(2, onProgressCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectsWithFiles_CallsProgressWithTotalBytes)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();
    ObjectId newIdA("TestSchema.TestClass", "");
    ObjectId newIdB("TestSchema.TestClass", "");

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instanceA, StubFile("12"), false));
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instanceB, StubFile("3456"), false));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .Times(2)
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        progress(0, 2);
        progress(2, 2);
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "A"}));
        }))
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
            {
            progress(0, 4);
            progress(4, 4);
            return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "B"}));
            }));

        EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _))
            .Times(2)
            .WillOnce(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "A"}))))
            .WillOnce(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "B"}))));

        int onProgressCount = 0;
        bvector<CachingDataSource::Progress::State> expectedBytes = {{0, 6}, {0, 6}, {2, 6}, {2, 6}, {2, 6}, {6, 6}, {6, 6}};
        auto onProgress = [&] (CachingDataSource::ProgressCR progress)
            {
            EXPECT_EQ(expectedBytes[onProgressCount], progress.GetBytes());
            EXPECT_EQ(CachingDataSource::Progress::State(), progress.GetInstances());
            onProgressCount++;
            };

        ds->SyncLocalChanges(onProgress, nullptr)->Wait();
        EXPECT_EQ(expectedBytes.size(), onProgressCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedFiles_CallsProgressWithTotalBytes)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto instanceA = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    auto instanceB = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "B"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instanceA, StubFile("12"), false));
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instanceB, StubFile("3456"), false));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateFileRequest(_, _, _, _))
        .Times(2)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        progress(0, 2);
        progress(2, 2);
        return CreateCompletedAsyncTask(WSUpdateFileResult::Success({}));
        }))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR, Http::Request::ProgressCallbackCR progress, ICancellationTokenPtr)
            {
            progress(0, 4);
            progress(4, 4);
            return CreateCompletedAsyncTask(WSUpdateFileResult::Success({}));
            }));

    int onProgressCount = 0;
    bvector<CachingDataSource::Progress::State> expectedBytes = {{0, 6}, {0, 6}, {2, 6}, {2, 6}, {2, 6}, {6, 6}, {6, 6}};
    bvector<CachingDataSource::Progress::State> expectedCurrentFileBytes = {{0, 0}, {0, 2}, {2, 2}, {0, 0}, {0, 4}, {4, 4}, {0, 0}};

    auto onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_EQ(expectedBytes[onProgressCount], progress.GetBytes());
        EXPECT_EQ(expectedCurrentFileBytes[onProgressCount], progress.GetCurrentFileBytes());
        onProgressCount++;
        EXPECT_EQ(6, progress.GetBytes().total);
        EXPECT_EQ(0, progress.GetInstances().current);
        EXPECT_EQ(0, progress.GetInstances().total);
        };

    ds->SyncLocalChanges(onProgress, nullptr)->Wait();
    EXPECT_EQ(expectedBytes.size(), onProgressCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_NoObjectsPassedToSync_DoesNoRequestsAndReturnsSuccess)
    {
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    ASSERT_TRUE(txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue).IsValid());
    txn.Commit();

    bset<ECInstanceKey> toSync;
    auto result = ds->SyncLocalChanges(toSync, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.GetValue().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_ObjectIdForObjectChangePassed_SyncsFile)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSCreateObjectResult::Error(StubWSConnectionError()))));

    bset<ECInstanceKey> toSync;
    toSync.insert(instance);
    ds->SyncLocalChanges(toSync, nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_ObjectIdForRelationshipChangePassed_SyncsRelationship)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto relationship = StubCreatedRelationshipInCache(txn.GetCache(),
        "TestSchema.TestRelationshipClass", {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"});
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSCreateObjectResult::Error(StubWSConnectionError()))));

    bset<ECInstanceKey> toSync;
    toSync.insert(relationship);
    ds->SyncLocalChanges(toSync, nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_ObjectIdForFileChangePassed_SyncsFile)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instance, StubFile("12"), false));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateFileRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSUpdateFileResult::Error(StubWSConnectionError()))));

    bset<ECInstanceKey> toSync;
    toSync.insert(instance);
    ds->SyncLocalChanges(toSync, nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_NoQueryProviders_DoesNothingAndReturns)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), bvector<IQueryProvider::Query>(), bvector<IQueryProviderPtr>(), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueriesSupplied_CachesInitialQueriesAndCallsGetQueriesWithDoUpdateFile)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    IQueryProvider::Query query(CachedResponseKey(StubECInstanceKey(11, 22), "Foo"), std::make_shared<WSQuery>("Schema", "Class"));

    EXPECT_CALL(*cache, ReadResponseCacheTag(query.key, _)).WillOnce(Return("TestTag"));
    EXPECT_CALL(*client, SendQueryRequest(*query.query, Utf8String("TestTag"), _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, CacheResponse(query.key, _, _, Pointee(*query.query), _, _)).WillOnce(Return(CacheStatus::OK));

    auto newInstanceKey = StubECInstanceKey(33, 44);
    auto responseKeys = StubECInstanceKeyMultiMap({newInstanceKey});
    EXPECT_CALL(*cache, ReadResponseInstanceKeys(query.key, _)).WillOnce(DoAll(SetArgReferee<1>(responseKeys), Return(CacheStatus::OK)));

    EXPECT_CALL(*provider, GetQueries(_, newInstanceKey, _)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, newInstanceKey, _)).WillOnce(Return(nullptr));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(query), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueriesSupplied_ProgressHandlerParameterInvoked)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    IQueryProvider::Query query(CachedResponseKey(StubECInstanceKey(11, 22), "Foo"), std::make_shared<WSQuery>("Schema", "Class"));

    EXPECT_CALL(*cache, ReadResponseCacheTag(query.key, _)).WillOnce(Return("TestTag"));
    EXPECT_CALL(*client, SendQueryRequest(*query.query, Utf8String("TestTag"), _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, CacheResponse(query.key, _, _, Pointee(*query.query), _, _)).WillOnce(Return(CacheStatus::OK));

    auto newInstanceKey = StubECInstanceKey(33, 44);
    auto responseKeys = StubECInstanceKeyMultiMap({newInstanceKey});
    EXPECT_CALL(*cache, ReadResponseInstanceKeys(query.key, _)).WillOnce(DoAll(SetArgReferee<1>(responseKeys), Return(CacheStatus::OK)));

    EXPECT_CALL(*provider, GetQueries(_, newInstanceKey, _)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, newInstanceKey, _)).Times(1).WillRepeatedly(Return(nullptr));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    ICachingDataSource::ProgressHandler progressHandler;

    progressHandler.progressCallback = [] (ICachingDataSource::ProgressCR) {};
    progressHandler.shouldReportInstanceProgress = [=] (ECInstanceKeyCR instanceKey, CacheTransactionCR txn)
        {
        EXPECT_EQ(newInstanceKey, instanceKey);
        return true;
        };

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(query), StubBVector<IQueryProviderPtr>(provider), progressHandler, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueriesSuppliedFilteringAllQueryProgress_ProgressNotSentForInstance)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    IQueryProvider::Query query(CachedResponseKey(StubECInstanceKey(11, 22), "Foo"), std::make_shared<WSQuery>("Schema", "Class"));

    EXPECT_CALL(*cache, ReadResponseCacheTag(query.key, _)).WillOnce(Return("TestTag"));
    EXPECT_CALL(*client, SendQueryRequest(*query.query, Utf8String("TestTag"), _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, CacheResponse(query.key, _, _, Pointee(*query.query), _, _)).WillOnce(Return(CacheStatus::OK));

    auto newInstanceKey = StubECInstanceKey(33, 44);
    auto responseKeys = StubECInstanceKeyMultiMap({newInstanceKey});
    EXPECT_CALL(*cache, ReadResponseInstanceKeys(query.key, _)).WillOnce(DoAll(SetArgReferee<1>(responseKeys), Return(CacheStatus::OK)));

    EXPECT_CALL(*provider, GetQueries(_, newInstanceKey, _)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, newInstanceKey, _)).Times(1).WillRepeatedly(Return(nullptr));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    ICachingDataSource::ProgressHandler progressHandler;
    ICachingDataSource::Progress expectedProgress[] = {
            {0, {0, 0}}, {1, {0, 0}}
        };
    uint16_t index = 0;
    progressHandler.progressCallback = [&] (ICachingDataSource::ProgressCR progress)
        {
        EXPECT_PROGRESS_EQ(expectedProgress[index], progress);
        index++;
        };
    progressHandler.shouldReportInstanceProgress = [=] (ECInstanceKeyCR instanceKey, CacheTransactionCR txn)
        {
        EXPECT_EQ(newInstanceKey, instanceKey);
        return false;
        };

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(query), StubBVector<IQueryProviderPtr>(provider), progressHandler, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    EXPECT_EQ(2, index);
    }

TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueriesWithSyncRecursivelyFalseSupplied_CachesInitialQueriesAndOnlyCallsDoUpdateFileFromProviders)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    IQueryProvider::Query query(CachedResponseKey(StubECInstanceKey(11, 22), "Foo"), std::make_shared<WSQuery>("Schema", "Class"), false);

    EXPECT_CALL(*cache, ReadResponseCacheTag(query.key, _)).WillOnce(Return("TestTag"));
    EXPECT_CALL(*client, SendQueryRequest(*query.query, Utf8String("TestTag"), _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, CacheResponse(query.key, _, _, Pointee(*query.query), _, _)).WillOnce(Return(CacheStatus::OK));

    auto newInstanceKey = StubECInstanceKey(33, 44);
    auto responseKeys = StubECInstanceKeyMultiMap({newInstanceKey});
    EXPECT_CALL(*cache, ReadResponseInstanceKeys(query.key, _)).WillOnce(DoAll(SetArgReferee<1>(responseKeys), Return(CacheStatus::OK)));
    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    EXPECT_CALL(*provider, GetQueries(_, _, _)).Times(0);
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, newInstanceKey, _)).Times(1).WillRepeatedly(Return(nullptr));

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(query), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstancesSupplied_CachesInitialInstancesAndCallsGetQueriesWithDoUpdateFile)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    auto instanceKey = StubECInstanceKey(11, 22);
    ObjectId objectId("TestSchema.TestClass", "TestId");
    EXPECT_CALL(*cache, FindInstance(instanceKey)).WillRepeatedly(Return(objectId));

    EXPECT_CALL(*client, SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillOnce(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));

    EXPECT_CALL(*provider, GetQueries(_, instanceKey, _)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, instanceKey, _)).WillOnce(Return(nullptr));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto result = ds->SyncCachedData(StubBVector(instanceKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstancesContainsInvalid_SkipsInvalidOnesAndReturnsAsFailedObjects)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    auto instanceKey = StubECInstanceKey(11, 22);
    ObjectId objectId("TestSchema.TestClass", "TestId");
    EXPECT_CALL(*cache, FindInstance(instanceKey)).WillRepeatedly(Return(objectId));

    EXPECT_CALL(*cache, FindInstance(StubECInstanceKey(1, 2))).WillRepeatedly(Return(ObjectId()));
    EXPECT_CALL(*cache, FindInstance(StubECInstanceKey(1, 3))).WillRepeatedly(Return(ObjectId()));
    EXPECT_CALL(*cache, ReadInstanceLabel(_)).WillRepeatedly(Return(Utf8String()));
    
    EXPECT_CALL(*client, SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillOnce(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));

    EXPECT_CALL(*provider, GetQueries(_, _, _)).WillRepeatedly(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, _, _)).WillRepeatedly(Return(nullptr));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));
     
    auto instances = StubBVector({StubECInstanceKey(1, 2), instanceKey, StubECInstanceKey(1, 3)});
    auto result = ds->SyncCachedData(instances, bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(2, result.GetValue().size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstancesNotReturnedInQueryRequest_QueriesAndCachesInstancesSeperatelyAndRemovesOnesThatNotAccessable)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);

    auto instanceKeyA = StubECInstanceKey(11, 22);
    auto instanceKeyB = StubECInstanceKey(11, 33);
    auto instanceKeyC = StubECInstanceKey(11, 44);
    ObjectId objectIdA("TestSchema.TestClass", "A");
    ObjectId objectIdB("TestSchema.TestClass", "B");
    ObjectId objectIdC("TestSchema.TestClass", "C");
    EXPECT_CALL(*cache, FindInstance(instanceKeyA)).WillRepeatedly(Return(objectIdA));
    EXPECT_CALL(*cache, FindInstance(instanceKeyB)).WillRepeatedly(Return(objectIdB));
    EXPECT_CALL(*cache, FindInstance(instanceKeyC)).WillRepeatedly(Return(objectIdC));

    EXPECT_CALL(*cache, FindInstance(objectIdA)).WillRepeatedly(Return(instanceKeyA));
    EXPECT_CALL(*cache, FindInstance(objectIdB)).WillRepeatedly(Return(instanceKeyB));
    EXPECT_CALL(*cache, FindInstance(objectIdC)).WillRepeatedly(Return(instanceKeyC));

    StubInstances queryInstances;
    queryInstances.Add(objectIdB);

    EXPECT_CALL(*client, SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(queryInstances.ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillOnce(DoAll(SetArgPointee<2>(StubBSet({instanceKeyB})), Return(SUCCESS)));

    StubInstances instanceC;
    instanceC.Add(objectIdC);
    EXPECT_CALL(*client, SendGetObjectRequest(objectIdC, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instanceC.ToWSObjectsResult())));
    EXPECT_CALL(*client, SendGetObjectRequest(objectIdA, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError::Id::InstanceNotFound))));

    ON_CALL(*cache, UpdateInstance(objectIdC, _)).WillByDefault(Return(CacheStatus::OK));
    EXPECT_CALL(*cache, RemoveInstance(objectIdA)).WillOnce(Return(CacheStatus::OK));

    ON_CALL(*cache, ReadInstanceLabel(_)).WillByDefault(Return(nullptr));
    ON_CALL(*cache, ReadInstanceCacheTag(_)).WillByDefault(Return(nullptr));
    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto initialInstances = StubBVector({instanceKeyA, instanceKeyB, instanceKeyC});
    auto result = ds->SyncCachedData(initialInstances, bvector<IQueryProvider::Query>(), bvector<IQueryProviderPtr>(), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), SizeIs(1));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstancesNotReturnedInQueryRequest_RemovesInstancesFromCache)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);

    auto instanceKeyA = StubECInstanceKey(11, 22);
    auto instanceKeyB = StubECInstanceKey(11, 33);
    auto instanceKeyC = StubECInstanceKey(11, 44);
    ObjectId objectIdA("TestSchema.TestClass", "A");
    ObjectId objectIdB("TestSchema.TestClass", "B");
    ObjectId objectIdC("TestSchema.TestClass", "C");
    EXPECT_CALL(*cache, FindInstance(instanceKeyA)).WillRepeatedly(Return(objectIdA));
    EXPECT_CALL(*cache, FindInstance(instanceKeyB)).WillRepeatedly(Return(objectIdB));
    EXPECT_CALL(*cache, FindInstance(instanceKeyC)).WillRepeatedly(Return(objectIdC));

    EXPECT_CALL(*client, SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillOnce(DoAll(SetArgPointee<2>(StubBSet({instanceKeyB})), Return(SUCCESS)));

    EXPECT_CALL(*client, SendGetObjectRequest(objectIdA, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError::Id::InstanceNotFound))));

    EXPECT_CALL(*client, SendGetObjectRequest(objectIdC, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError::Id::InstanceNotFound))));

    EXPECT_CALL(*cache, RemoveInstance(objectIdA)).WillOnce(Return(CacheStatus::OK));
    EXPECT_CALL(*cache, RemoveInstance(objectIdC)).WillOnce(Return(CacheStatus::OK));

    ON_CALL(*cache, ReadInstanceLabel(_)).WillByDefault(Return(nullptr));
    ON_CALL(*cache, ReadInstanceCacheTag(_)).WillByDefault(Return(nullptr));
    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto initialInstances = StubBVector({instanceKeyA, instanceKeyB, instanceKeyC});
    auto result = ds->SyncCachedData(initialInstances, bvector<IQueryProvider::Query>(), bvector<IQueryProviderPtr>(), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), SizeIs(2));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_AlreadyCachedInstanceReturnedFromQuery_QueryProviderIsNotCalledAgainWithSameInstance)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    auto instanceKey = StubECInstanceKey(11, 22);
    ObjectId objectId("TestSchema.TestClass", "TestId");
    EXPECT_CALL(*cache, FindInstance(instanceKey)).WillRepeatedly(Return(objectId));

    // Initial caching
    EXPECT_CALL(*client, SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillOnce(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));

    IQueryProvider::Query query(CachedResponseKey(instanceKey, "Foo"), std::make_shared<WSQuery>("Schema", "Class"));

    EXPECT_CALL(*provider, GetQueries(_, instanceKey, _)).WillOnce(Return(StubBVector(query)));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, instanceKey, _)).WillOnce(Return(nullptr));

    // Second query
    EXPECT_CALL(*cache, ReadResponseCacheTag(query.key, _)).WillOnce(Return(""));
    EXPECT_CALL(*client, SendQueryRequest(*query.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, CacheResponse(query.key, _, _, Pointee(*query.query), _, _)).WillOnce(Return(CacheStatus::OK));

    auto responseKeys = StubECInstanceKeyMultiMap({instanceKey});
    EXPECT_CALL(*cache, ReadResponseInstanceKeys(query.key, _)).WillOnce(DoAll(SetArgReferee<1>(responseKeys), Return(CacheStatus::OK)));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto result = ds->SyncCachedData(StubBVector(instanceKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_CachePartialInstancesRejectsInstances_InstnaceIsQueriedAndCachedSeperately)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);

    auto instanceKey = StubECInstanceKey(11, 22);
    ObjectId objectId("TestSchema.TestClass", "TestId");
    EXPECT_CALL(*cache, FindInstance(instanceKey)).WillRepeatedly(Return(objectId));
    EXPECT_CALL(*cache, FindInstance(objectId)).WillRepeatedly(Return(instanceKey));

    IQueryProvider::Query query(CachedResponseKey(StubECInstanceKey(), "Foo"), std::make_shared<WSQuery>("Schema", "Class"));

    EXPECT_CALL(*cache, ReadResponseCacheTag(query.key, _)).WillOnce(Return(nullptr));
    EXPECT_CALL(*client, SendQueryRequest(*query.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));

    EXPECT_CALL(*cache, CacheResponse(query.key, _, _, Pointee(*query.query), _, _))
        .WillOnce(DoAll(SetArgPointee<2>(StubBSet({objectId})), Return(CacheStatus::OK)));

    EXPECT_CALL(*cache, ReadResponseInstanceKeys(query.key, _)).WillOnce(Return(CacheStatus::OK));

    // Queries and caches rejected instances
    EXPECT_CALL(*client, SendQueryRequest(WSQuery(ObjectId("TestSchema.TestClass", "TestId")), _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillOnce(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(query), bvector<IQueryProviderPtr>(), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Simas.Lukenas                     01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstanceRemovesPathToChild_ChildInstanceIsRemovedAndNotCached)
    {
    auto ds = GetTestDataSourceV2();
    auto provider = std::make_shared<MockQueryProvider>();

    ObjectId objectIdParent("TestSchema.TestClass", "Parent");
    ObjectId objectIdA("TestSchema.TestClass", "A");
    ObjectId objectIdB("TestSchema.TestClass", "B");

    auto txn = ds->StartCacheTransaction();
    StubInstances instances;
    instances.Add(objectIdParent);
    CachedResponseKey responseKeyParent(txn.GetCache().FindOrCreateRoot(nullptr), nullptr);
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(responseKeyParent, instances.ToWSObjectsResponse()));
    auto instanceKeyParent = txn.GetCache().FindInstance(objectIdParent);
    ASSERT_TRUE(instanceKeyParent.IsValid());

    instances.Clear();
    instances.Add(objectIdA);
    CachedResponseKey responseKeyA(instanceKeyParent, nullptr);
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(responseKeyA, instances.ToWSObjectsResponse()));
    auto instanceKeyA = txn.GetCache().FindInstance(objectIdA);
    ASSERT_TRUE(instanceKeyA.IsValid());

    CachedResponseKey responseKeyB(instanceKeyA, nullptr);
    instances.Clear();
    instances.Add(objectIdB);
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(responseKeyB, instances.ToWSObjectsResponse()));
    auto instanceKeyB = txn.GetCache().FindInstance(objectIdB);
    ASSERT_TRUE(instanceKeyB.IsValid());
    txn.Commit();

    IQueryProvider::Query queryParent(responseKeyA, std::make_shared<WSQuery>("SchemaA", "ClassA"));
    IQueryProvider::Query queryB(CachedResponseKey(instanceKeyB, nullptr), std::make_shared<WSQuery>("SchemaB", "ClassB"));

    EXPECT_CALL(*provider, GetQueries(_, instanceKeyParent, _)).WillOnce(Return(StubBVector({queryParent})));
    EXPECT_CALL(*provider, GetQueries(_, instanceKeyB, _)).WillOnce(Return(StubBVector({queryB})));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, instanceKeyParent, _)).WillOnce(Return(nullptr));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, instanceKeyB, _)).WillOnce(Return(nullptr));

    StubInstances remoteInstances;
    remoteInstances.Add(objectIdB);
    remoteInstances.Add(objectIdParent);

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(remoteInstances.ToWSObjectsResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryParent.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryB.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));

    auto result = ds->SyncCachedData(StubBVector({instanceKeyB, instanceKeyParent}), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), SizeIs(1));
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo(objectIdB).IsInCache());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Simas.Lukenas                     02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstanceRemovesChildAndPathToIt_ChildInstanceIsRemovedAndNotCached)
    {
    auto ds = GetTestDataSourceV2();
    auto provider = std::make_shared<MockQueryProvider>();

    ObjectId objectIdA("TestSchema.TestClass", "A");
    ObjectId objectIdB("TestSchema.TestClass", "B");

    auto txn = ds->StartCacheTransaction();
    StubInstances instances;
    instances.Add(objectIdA);
    CachedResponseKey responseKeyA(txn.GetCache().FindOrCreateRoot(nullptr), nullptr);
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(responseKeyA, instances.ToWSObjectsResponse()));
    auto instanceKeyA = txn.GetCache().FindInstance(objectIdA);
    ASSERT_TRUE(instanceKeyA.IsValid());

    instances.Clear();
    instances.Add(objectIdB);
    CachedResponseKey responseKeyB(instanceKeyA, nullptr);
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(responseKeyB, instances.ToWSObjectsResponse()));
    auto instanceKeyB = txn.GetCache().FindInstance(objectIdB);
    ASSERT_TRUE(instanceKeyB.IsValid());
    txn.Commit();

    IQueryProvider::Query query(responseKeyA, std::make_shared<WSQuery>("SchemaA", "ClassA"));

    EXPECT_CALL(*provider, GetQueries(_, instanceKeyB, _)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, instanceKeyB, _)).WillOnce(Return(nullptr));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).Times(2)
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(objectIdB, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError::Id::InstanceNotFound))));

    auto result = ds->SyncCachedData(StubBVector({instanceKeyB}), StubBVector(query), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), SizeIs(1));
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo(objectIdB).IsInCache());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstanceRemovesNonExistingChildAndPathToIt_ChildInstanceIsRemovedAndNotCached)
    {
    auto ds = GetTestDataSourceV2();
    auto provider = std::make_shared<MockQueryProvider>();

    ObjectId objectIdA("TestSchema.TestClass", "A");
    ObjectId objectIdB("TestSchema.TestClass", "B");

    auto txn = ds->StartCacheTransaction();
    StubInstances instances;
    instances.Add(objectIdA);
    CachedResponseKey responseKeyA(txn.GetCache().FindOrCreateRoot(nullptr), nullptr);
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(responseKeyA, instances.ToWSObjectsResponse()));
    auto instanceKeyA = txn.GetCache().FindInstance(objectIdA);
    ASSERT_TRUE(instanceKeyA.IsValid());

    instances.Clear();
    instances.Add(objectIdB);
    CachedResponseKey responseKeyB(instanceKeyA, nullptr);
    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(responseKeyB, instances.ToWSObjectsResponse()));
    auto instanceKeyB = txn.GetCache().FindInstance(objectIdB);
    ASSERT_TRUE(instanceKeyB.IsValid());
    txn.Commit();

    IQueryProvider::Query query(responseKeyA, std::make_shared<WSQuery>("SchemaA", "ClassA"));

    EXPECT_CALL(*provider, GetQueries(_, instanceKeyB, _)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, instanceKeyB, _)).WillOnce(Return(nullptr));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).Times(2)
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(objectIdB, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto result = ds->SyncCachedData(StubBVector({instanceKeyB}), StubBVector(query), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), SizeIs(1));
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo(objectIdB).IsInCache());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_QueryProviderReturnsToUpdateFile_DownloadsAndCachesFileToSetupAutoLocation)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    auto instanceKey = StubECInstanceKey(11, 22);
    ObjectId objectId("TestSchema.TestClass", "TestId");
    EXPECT_CALL(*cache, FindInstance(instanceKey)).WillRepeatedly(Return(objectId));
    EXPECT_CALL(*cache, FindInstance(objectId)).WillRepeatedly(Return(instanceKey));

    EXPECT_CALL(*client, SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillOnce(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));

    EXPECT_CALL(*provider, GetQueries(_, instanceKey, _)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, instanceKey, _)).WillOnce(Return(SimpleCancellationToken::Create()));

    // Download & cache file
    EXPECT_CALL(*cache, ReadFileCacheTag(objectId)).WillOnce(Return("TestTag"));
    EXPECT_CALL(*cache, ReadFileProperties(instanceKey, _, _)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*client, SendGetFileRequest(objectId, ::testing::An<BeFileNameCR>(), Utf8String("TestTag"), _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR fileName, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        SimpleWriteToFile("", fileName);
        return CreateCompletedAsyncTask(WSFileResult::Success(StubWSFileResponse(fileName, "")));
        }));
    EXPECT_CALL(*cache, CacheFile(objectId, _, FileCache::Auto)).WillOnce(Return(SUCCESS));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto result = ds->SyncCachedData(StubBVector(instanceKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_QueryProviderReturnsToUpdateFileButInstanceIsThenDeleted_SkipsFileDownload)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    auto instanceKey = StubECInstanceKey(11, 22);
    ObjectId objectId("TestSchema.TestClass", "TestId");
    EXPECT_CALL(*cache, FindInstance(instanceKey)).WillOnce(Return(objectId)).WillRepeatedly(Return(ObjectId()));
    EXPECT_CALL(*cache, FindInstance(objectId)).WillRepeatedly(Return(instanceKey));

    EXPECT_CALL(*client, SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillOnce(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));

    EXPECT_CALL(*provider, GetQueries(_, instanceKey, _)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, instanceKey, _)).WillOnce(Return(SimpleCancellationToken::Create()));

    // Download & cache file
    EXPECT_CALL(*cache, ReadFileProperties(instanceKey, _, _)).WillRepeatedly(Return(SUCCESS));
    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto result = ds->SyncCachedData(StubBVector(instanceKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InstanceCachedAsPersistent_GetQueriesAndDoUpdateFileIsPersistentParameterIsTrue)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    IQueryProvider::Query query(CachedResponseKey(StubECInstanceKey(11, 22), "Foo"), std::make_shared<WSQuery>("Schema", "Class"));

    EXPECT_CALL(*cache, ReadResponseCacheTag(query.key, _)).WillOnce(Return("TestTag"));
    EXPECT_CALL(*client, SendQueryRequest(*query.query, Utf8String("TestTag"), _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, CacheResponse(query.key, _, _, Pointee(*query.query), _, _)).WillOnce(Return(CacheStatus::OK));

    auto newInstanceKey = StubECInstanceKey(33, 44);
    auto responseKeys = StubECInstanceKeyMultiMap({newInstanceKey});
    EXPECT_CALL(*cache, ReadResponseInstanceKeys(query.key, _)).WillOnce(DoAll(SetArgReferee<1>(responseKeys), Return(CacheStatus::OK)));

    // Return that all response instances are cached
    EXPECT_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillOnce(DoAll(SetArgReferee<0>(responseKeys), Return(SUCCESS)));

    EXPECT_CALL(*provider, GetQueries(_, newInstanceKey, true)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, newInstanceKey, true)).WillOnce(Return(nullptr));

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(query), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_CancellationTokenCanceled_ReturnsErrorCanceled)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    IQueryProvider::Query query(CachedResponseKey(StubECInstanceKey(11, 22), "Foo"), std::make_shared<WSQuery>("Schema", "Class"));

    auto ct = SimpleCancellationToken::Create(true);

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(query), StubBVector<IQueryProviderPtr>(provider), nullptr, ct)->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_THAT(result.GetError().GetStatus(), ICachingDataSource::Status::Canceled);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueriesSuppliedAndServerErrorReturnedForFirstOne_ContinuesWithOtherQuerysAndReturnsFailedObject)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);

    IQueryProvider::Query a(CachedResponseKey(StubECInstanceKey(11, 22), "A"), std::make_shared<WSQuery>("SchemaA", "ClassA"));
    IQueryProvider::Query b(CachedResponseKey(StubECInstanceKey(11, 33), "B"), std::make_shared<WSQuery>("SchemaB", "ClassB"));
    IQueryProvider::Query c(CachedResponseKey(StubECInstanceKey(11, 44), "C"), std::make_shared<WSQuery>("SchemaC", "ClassC"));

    EXPECT_CALL(*cache, ReadResponseCacheTag(_, _)).WillRepeatedly(Return(""));
    EXPECT_CALL(*client, SendQueryRequest(*a.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*client, SendQueryRequest(*b.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError::Id::ServerError))));
    EXPECT_CALL(*client, SendQueryRequest(*c.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, CacheResponse(a.key, _, _, Pointee(*a.query), _, _)).WillOnce(Return(CacheStatus::OK));
    EXPECT_CALL(*cache, CacheResponse(c.key, _, _, Pointee(*c.query), _, _)).WillOnce(Return(CacheStatus::OK));

    EXPECT_CALL(*cache, ReadResponseInstanceKeys(a.key, _)).WillOnce(Return(CacheStatus::OK));
    EXPECT_CALL(*cache, ReadResponseInstanceKeys(c.key, _)).WillOnce(Return(CacheStatus::OK));
    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    EXPECT_CALL(*cache, FindInstance(b.key.GetParent())).WillOnce(Return(ObjectId()));
    EXPECT_CALL(*cache, ReadInstanceLabel(_)).WillOnce(Return(""));

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector({a, b, c}), bvector<IQueryProviderPtr>(), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), SizeIs(1));
    EXPECT_THAT(result.GetValue()[0].GetError().GetWSError().GetId(), WSError::Id::ServerError);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueriesSuppliedAndConnectionError_StopsAndReturnsError)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);

    IQueryProvider::Query a(CachedResponseKey(StubECInstanceKey(11, 22), "A"), std::make_shared<WSQuery>("SchemaA", "ClassA"));
    IQueryProvider::Query b(CachedResponseKey(StubECInstanceKey(11, 33), "B"), std::make_shared<WSQuery>("SchemaB", "ClassB"));

    EXPECT_CALL(*cache, ReadResponseCacheTag(_, _)).WillRepeatedly(Return(""));
    EXPECT_CALL(*client, SendQueryRequest(*a.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubHttpResponse()))));
    EXPECT_CALL(*client, SendQueryRequest(*b.query, _, _, _)).Times(0);

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector({a, b}), bvector<IQueryProviderPtr>(), nullptr, nullptr)->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_THAT(result.GetError().GetWSError().GetStatus(), WSError::Status::ConnectionError);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_SkipTokensNotEnabledInitialQueriesSuppliedAndServerRespondsWithSkipToken_SkipTokenNotUsed)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    ds->EnableSkipTokens(false);

    IQueryProvider::Query query(CachedResponseKey(StubECInstanceKey(11, 22), "A"), std::make_shared<WSQuery>("SchemaA", "ClassA"));

    ON_CALL(*cache, ReadResponseCacheTag(_, _)).WillByDefault(Return(""));
    ON_CALL(*cache, CacheResponse(_, _, _, _, _, _)).WillByDefault(Return(CacheStatus::OK));
    ON_CALL(*cache, ReadResponseInstanceKeys(_, _)).WillByDefault(Return(CacheStatus::OK));

    EXPECT_CALL(*client, SendQueryRequest(*query.query, _, _, _))
        .WillOnce(Invoke([] (WSQueryCR, Utf8StringCR, Utf8StringCR skipToken, ICancellationTokenPtr)
        {
        EXPECT_EQ("", skipToken);
        return CreateCompletedAsyncTask(WSObjectsResult::Error({}));
        }));

    ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector({query}), bvector<IQueryProviderPtr>(), nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                   Vincas.Razma                      03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueriesSuppliedAndServerRespondsWithSkipToken_MultipleRequestsDone)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    ds->EnableSkipTokens(true);

    IQueryProvider::Query query(CachedResponseKey(StubECInstanceKey(11, 22), "A"), std::make_shared<WSQuery>("SchemaA", "ClassA"));

    ON_CALL(*cache, ReadResponseCacheTag(_, _)).WillByDefault(Return(""));
    ON_CALL(*cache, CacheResponse(_, _, _, _, _, _)).WillByDefault(Return(CacheStatus::OK));
    ON_CALL(*cache, ReadResponseInstanceKeys(_, _)).WillByDefault(Return(CacheStatus::OK));

    InSequence callsInSeq;

    EXPECT_CALL(*client, SendQueryRequest(_, _, WSRepositoryClient::InitialSkipToken, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubInstances().ToWSObjectsResponse("", "SkipToken")))));

    EXPECT_CALL(*client, SendQueryRequest(_, _, Utf8String("SkipToken"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubInstances().ToWSObjectsResponse("", "")))));

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector({query}), bvector<IQueryProviderPtr>(), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstance_CallbackCalledWithZeroProgress)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    auto instanceKey = StubECInstanceKey(11, 22);
    ON_CALL(*cache, FindInstance(instanceKey)).WillByDefault(Return(ObjectId("TestSchema.TestClass", "TestId")));
    ON_CALL(*client, SendQueryRequest(_, _, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    ON_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillByDefault(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));
    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    EXPECT_CALL(*provider, GetQueries(_, instanceKey, _)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, instanceKey, _)).WillOnce(Return(nullptr));

    int progressCalled = 0;
    double expectedSyncedValues[2] = {0, 1};

    ICachingDataSource::ProgressCallback onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_EQ(expectedSyncedValues[progressCalled], progress.GetSynced());
        EXPECT_THAT(progress.GetLabel(), IsEmpty());
        EXPECT_THAT(progress.GetBytes().current, 0);
        EXPECT_THAT(progress.GetBytes().total, 0);
        progressCalled++;
        };

    auto result = ds->SyncCachedData(StubBVector(instanceKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), onProgress, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    EXPECT_THAT(progressCalled, 2);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstancesAndQueries_OnProgressCallsWithEachpartProgress)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto store = std::make_shared<StubRepositoryInfoStore>(StubWSInfoWebApi({2, 0}));
    auto ds = CreateMockedCachingDataSource(client, cache, store);

    auto instanceA = StubECInstanceKey(1, 2);
    auto instanceB = StubECInstanceKey(3, 4);
    auto objectA = ObjectId("TestSchema.TestClass", "A");
    auto objectB = ObjectId("TestSchema.TestClass", "B");

    IQueryProvider::Query queryA(CachedResponseKey(StubECInstanceKey(5, 6), "A"), std::make_shared<WSQuery>("SchemaA", "ClassA"));
    IQueryProvider::Query queryB(CachedResponseKey(StubECInstanceKey(7, 8), "B"), std::make_shared<WSQuery>("SchemaB", "ClassB"));

    ON_CALL(*cache, CacheResponse(_, _, _, _, _, _)).WillByDefault(Return(CacheStatus::OK));
    ON_CALL(*cache, FindInstance(instanceA)).WillByDefault(Return(objectA));
    ON_CALL(*cache, FindInstance(instanceB)).WillByDefault(Return(objectB));
    ON_CALL(*cache, FindInstance(objectA)).WillByDefault(Return(instanceA));
    ON_CALL(*cache, FindInstance(objectB)).WillByDefault(Return(instanceB));
    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));
    ON_CALL(*cache, ReadInstanceCacheTag(_)).WillByDefault(Return(nullptr));
    ON_CALL(*cache, ReadResponseCacheTag(_, _)).WillByDefault(Return(nullptr));
    ON_CALL(*cache, ReadResponseInstanceKeys(_, _)).WillByDefault(Return(CacheStatus::OK));
    ON_CALL(*cache, UpdateInstance(_, _)).WillByDefault(Return(CacheStatus::OK));
    ON_CALL(*cache, UpdateInstances(_, _, _, _)).WillByDefault(Return(SUCCESS));
    ON_CALL(*client, SendGetObjectRequest(_, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    ON_CALL(*client, SendQueryRequest(_, _, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));

    int progressCalled = 0;
    double expectedSyncedValues[5] = {0, 0.25, 0.50, 0.75, 1};
    ICachingDataSource::ProgressCallback onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_EQ(expectedSyncedValues[progressCalled], progress.GetSynced());
        EXPECT_THAT(progress.GetBytes().current, 0);
        EXPECT_THAT(progress.GetBytes().total, 0);
        progressCalled++;
        };

    auto result = ds->SyncCachedData(StubBVector({instanceA, instanceB}), StubBVector({queryA, queryB}),
                                     bvector<IQueryProviderPtr>(), onProgress, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    EXPECT_THAT(progressCalled, 5);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstancesWithProviders_OnProgressCallsWithInitialAndNewQuery)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto store = std::make_shared<StubRepositoryInfoStore>(StubWSInfoWebApi({2, 0}));
    auto ds = CreateMockedCachingDataSource(client, cache, store);
    auto provider = std::make_shared<MockQueryProvider>();

    auto instanceA = StubECInstanceKey(1, 2);
    auto instanceB = StubECInstanceKey(3, 4);
    auto objectA = ObjectId("TestSchema.TestClass", "A");
    auto objectB = ObjectId("TestSchema.TestClass", "B");
    IQueryProvider::Query queryA(CachedResponseKey(StubECInstanceKey(5, 6), "A"), std::make_shared<WSQuery>("SchemaA", "ClassA"));
    IQueryProvider::Query queryB(CachedResponseKey(StubECInstanceKey(7, 8), "B"), std::make_shared<WSQuery>("SchemaB", "ClassB"));

    ON_CALL(*cache, CacheResponse(_, _, _, _, _, _)).WillByDefault(Return(CacheStatus::OK));
    ON_CALL(*cache, FindInstance(instanceA)).WillByDefault(Return(objectA));
    ON_CALL(*cache, FindInstance(instanceB)).WillByDefault(Return(objectB));
    ON_CALL(*cache, FindInstance(objectA)).WillByDefault(Return(instanceA));
    ON_CALL(*cache, FindInstance(objectB)).WillByDefault(Return(instanceB));
    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));
    ON_CALL(*cache, ReadInstanceCacheTag(_)).WillByDefault(Return(nullptr));
    ON_CALL(*cache, ReadResponseCacheTag(_, _)).WillByDefault(Return(nullptr));
    ON_CALL(*cache, ReadResponseInstanceKeys(_, _)).WillByDefault(Return(CacheStatus::OK));
    ON_CALL(*cache, UpdateInstance(_, _)).WillByDefault(Return(CacheStatus::OK));
    ON_CALL(*cache, UpdateInstances(_, _, _, _)).WillByDefault(Return(SUCCESS));
    ON_CALL(*client, SendGetObjectRequest(_, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    ON_CALL(*client, SendQueryRequest(_, _, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));

    EXPECT_CALL(*provider, GetQueries(_, instanceA, _)).WillOnce(Return(StubBVector({queryA})));
    EXPECT_CALL(*provider, GetQueries(_, instanceB, _)).WillOnce(Return(StubBVector({queryB})));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, _, _)).WillRepeatedly(Return(nullptr));

    int progressCalled = 0;
    ICachingDataSource::Progress expectedProgress[] = {
        {0,       {0, 2}},
        {0.25,    {0, 2}},
        {0.50,    {0, 2}},
        {0.75,    {1, 2}},
        {1,       {2, 2}}};
    ICachingDataSource::ProgressCallback onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_EQ(expectedProgress[progressCalled], progress);
        progressCalled++;
        };

    auto result = ds->SyncCachedData(StubBVector({instanceA, instanceB}), bvector<IQueryProvider::Query>(),
                                     StubBVector<IQueryProviderPtr>(provider), onProgress, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    EXPECT_THAT(progressCalled, 5);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_FilesBeingDownloaded_CallbackCalledWithFileProgress)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    auto instanceKey = StubECInstanceKey(11, 22);
    ObjectId objectId("TestSchema.TestClass", "TestId");
    ON_CALL(*cache, FindInstance(instanceKey)).WillByDefault(Return(objectId));
    ON_CALL(*cache, FindInstance(objectId)).WillByDefault(Return(instanceKey));
    ON_CALL(*cache, UpdateInstances(_, _, _, _)).WillByDefault(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));
    ON_CALL(*client, SendQueryRequest(_, _, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));

    EXPECT_CALL(*provider, GetQueries(_, instanceKey, _)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, instanceKey, _)).WillOnce(Return(SimpleCancellationToken::Create()));

    // Download & cache file
    EXPECT_CALL(*cache, ReadFileCacheTag(objectId)).WillOnce(Return(nullptr));
    EXPECT_CALL(*cache, ReadFileProperties(instanceKey, _, _)).WillRepeatedly(Invoke([&] (ECInstanceKeyCR, Utf8String* fileNameP, uint64_t* fileSizeP)
        {
        if (nullptr != fileNameP)
            *fileNameP = "TestFile.txt";
        *fileSizeP = 42;
        return SUCCESS;
        }));
    EXPECT_CALL(*client, SendGetFileRequest(objectId, ::testing::An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR fileName, Utf8StringCR, Http::Request::ProgressCallbackCR onProgress, ICancellationTokenPtr)
        {
        onProgress(5, 42);
        SimpleWriteToFile("", fileName);
        return CreateCompletedAsyncTask(WSFileResult::Success(StubWSFileResponse(fileName, "")));
        }));

    ON_CALL(*cache, CacheFile(objectId, _, _)).WillByDefault(Return(SUCCESS));
    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    int progressCalled = 0;
    double expectedSyncedValues[] = {0, 1, 1, 1};
    ICachingDataSource::ProgressCallback onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_EQ(expectedSyncedValues[progressCalled], progress.GetSynced());
        if (progressCalled == 2)
            {
            EXPECT_EQ("TestFile.txt", progress.GetLabel());
            EXPECT_THAT(progress.GetBytes().current, 5);
            EXPECT_THAT(progress.GetBytes().total, 42);
            }
        progressCalled++;
        };

    auto result = ds->SyncCachedData(StubBVector(instanceKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), onProgress, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    EXPECT_THAT(progressCalled, 4);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstanceAndItsQueryReturnsTwoInstances_ProgressCalledWithInstancesState)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    StubInstances instanceA;
    instanceA.Add({"TestSchema.TestClass", "A"});
    ObjectId instanceId {"TestSchema.TestClass", "A"};
    auto instanceKey = StubInstanceInCache(txn.GetCache(), instanceId);
    auto responseKey = StubCachedResponseKey(txn.GetCache());
    txn.Commit();

    // Act & Assert
    auto provider = std::make_shared<MockQueryProvider>();
    EXPECT_CALL(*provider, GetQueries(_, _, _)).WillRepeatedly(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, _, _)).WillRepeatedly(Return(nullptr));
    IQueryProvider::Query query(responseKey, std::make_shared<WSQuery>("TestSchema", "TestClass"));
    EXPECT_CALL(*provider, GetQueries(_, instanceKey, _)).WillOnce(Return(StubBVector({query})));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "B"});
    instances.Add({"TestSchema.TestClass", "C"});
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).Times(2)
        .WillOnce(Return(CreateCompletedAsyncTask(instanceA.ToWSObjectsResult())))
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    int progressCalled = 0;
    CachingDataSource::Progress expectedProgress[] = {
        {0,   {0, 1}}, 
        {0.5, {0, 1}},
        {1,   {3, 3}}};
    ICachingDataSource::ProgressCallback onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_PROGRESS_EQ(expectedProgress[progressCalled], progress);
        progressCalled++;
        };

    auto result = ds->SyncCachedData(StubBVector(instanceKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), onProgress, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstanceAndItsTwoQueriesReturnTwoInstancesEach_ProgressCalledWithInstancesState)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    StubInstances instanceA;
    instanceA.Add({"TestSchema.TestClass", "A"});
    ObjectId instanceAId {"TestSchema.TestClass", "A"};
    auto instanceAKey = StubInstanceInCache(txn.GetCache(), instanceAId);
    auto responseQAKey = StubCachedResponseKey(txn.GetCache(), "QA");
    auto responseQBKey = StubCachedResponseKey(txn.GetCache(), "QB");
    txn.Commit();

    // Act & Assert
    auto provider = std::make_shared<MockQueryProvider>();
    EXPECT_CALL(*provider, GetQueries(_, _, _)).WillRepeatedly(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, _, _)).WillRepeatedly(Return(nullptr));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instanceA.ToWSObjectsResult())));

    IQueryProvider::Query queryA(responseQAKey, std::make_shared<WSQuery>(ObjectId {"TestSchema", "TestClass", "QA"}));
    IQueryProvider::Query queryB(responseQBKey, std::make_shared<WSQuery>(ObjectId {"TestSchema", "TestClass", "QB"}));
    EXPECT_CALL(*provider, GetQueries(_, instanceAKey, _)).WillOnce(Return(StubBVector({queryA, queryB})));

    StubInstances instancesQA;
    instancesQA.Add({"TestSchema.TestClass", "AA"});
    instancesQA.Add({"TestSchema.TestClass", "AB"});
    StubInstances instancesQB;
    instancesQB.Add({"TestSchema.TestClass", "BA"});
    instancesQB.Add({"TestSchema.TestClass", "BB"});
    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryA.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instancesQA.ToWSObjectsResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryB.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instancesQB.ToWSObjectsResult())));

    int progressCalled = 0;
    CachingDataSource::Progress expectedProgress[] = {
            {0,    {0, 1}},
            {0.33, {0, 1}},
            {0.66, {2, 3}},
            {1,    {5, 5}}};
    ICachingDataSource::ProgressCallback onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_PROGRESS_EQ(expectedProgress[progressCalled], progress);
        progressCalled++;
        };

    auto result = ds->SyncCachedData(StubBVector(instanceAKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), onProgress, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstanceWithQueryThatReturnsInstanceWithQuery_ProgressCalledWithInstancesState)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    StubInstances instancesA;
    instancesA.Add({"TestSchema.TestClass", "A"});
    ObjectId instanceAId {"TestSchema.TestClass", "A"};
    auto instanceAKey = StubInstanceInCache(txn.GetCache(), instanceAId);
    auto responseQAKey = StubCachedResponseKey(txn.GetCache(), "QA");

    StubInstances instancesQA;
    instancesQA.Add({"TestSchema.TestClass", "B"});
    ObjectId instanceBId {"TestSchema.TestClass", "B"};
    auto instanceBKey = StubInstanceInCache(txn.GetCache(), instanceBId);
    auto responseQBKey = StubCachedResponseKey(txn.GetCache(), "QB");
    txn.Commit();

    // Act & Assert
    auto provider = std::make_shared<MockQueryProvider>();
    EXPECT_CALL(*provider, GetQueries(_, _, _)).WillRepeatedly(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, _, _)).WillRepeatedly(Return(nullptr));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instancesA.ToWSObjectsResult())));

    IQueryProvider::Query queryA(responseQAKey, std::make_shared<WSQuery>(ObjectId {"TestSchema", "TestClass", "QA"}));
    IQueryProvider::Query queryB(responseQBKey, std::make_shared<WSQuery>(ObjectId {"TestSchema", "TestClass", "QB"}));

    EXPECT_CALL(*provider, GetQueries(_, instanceAKey, _)).WillOnce(Return(StubBVector({queryA})));
    EXPECT_CALL(*provider, GetQueries(_, instanceBKey, _)).WillOnce(Return(StubBVector({queryB})));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryA.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instancesQA.ToWSObjectsResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryB.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));

    int progressCalled = 0;
    CachingDataSource::Progress expectedProgress[] = {
            {0,    {0, 1}},
            {0.5,  {0, 1}},
            {0.66, {1, 2}},
            {1,    {2, 2}}};
    ICachingDataSource::ProgressCallback onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_PROGRESS_EQ(expectedProgress[progressCalled], progress);
        progressCalled++;
        };

    auto result = ds->SyncCachedData(StubBVector(instanceAKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), onProgress, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstanceWithQueryThatReturnsNoInstances_ProgressCalledWithInstancesState)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    StubInstances instancesA;
    instancesA.Add({"TestSchema.TestClass", "A"});
    ObjectId instanceAId {"TestSchema.TestClass", "A"};
    auto instanceAKey = StubInstanceInCache(txn.GetCache(), instanceAId);
    auto responseQAKey = StubCachedResponseKey(txn.GetCache(), "QA");
    txn.Commit();

    // Act & Assert
    auto provider = std::make_shared<MockQueryProvider>();
    EXPECT_CALL(*provider, GetQueries(_, _, _)).WillRepeatedly(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, _, _)).WillRepeatedly(Return(nullptr));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instancesA.ToWSObjectsResult())));

    IQueryProvider::Query queryA(responseQAKey, std::make_shared<WSQuery>(ObjectId {"TestSchema", "TestClass", "QA"}));

    EXPECT_CALL(*provider, GetQueries(_, instanceAKey, _)).WillOnce(Return(StubBVector({queryA})));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryA.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));

    int progressCalled = 0;
    CachingDataSource::Progress expectedProgress[] = {
            {0,   {0, 1}},
            {0.5, {0, 1}},
            {1,   {1, 1}}};
    ICachingDataSource::ProgressCallback onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_PROGRESS_EQ(expectedProgress[progressCalled], progress);
        progressCalled++;
        };
    
    auto result = ds->SyncCachedData(StubBVector(instanceAKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), onProgress, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstanceWithNoQuery_ProgressCalledWithInstancesState)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    StubInstances instancesA;
    instancesA.Add({"TestSchema.TestClass", "A"});
    ObjectId instanceAId {"TestSchema.TestClass", "A"};
    auto instanceAKey = StubInstanceInCache(txn.GetCache(), instanceAId);
    txn.Commit();

    // Act & Assert
    auto provider = std::make_shared<MockQueryProvider>();
    EXPECT_CALL(*provider, GetQueries(_, _, _)).WillRepeatedly(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, _, _)).WillRepeatedly(Return(nullptr));

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(instanceAId, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instancesA.ToWSObjectsResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));

    int progressCalled = 0;
    CachingDataSource::Progress expectedProgress[] = {
            {0, {0, 1}},
            {1, {1, 1}}};
    ICachingDataSource::ProgressCallback onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_PROGRESS_EQ(expectedProgress[progressCalled], progress);
        progressCalled++;
        };

    auto result = ds->SyncCachedData(StubBVector(instanceAKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), onProgress, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueryWithInstance_ProgressCalledWithInstancesState)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto responseQAKey = StubCachedResponseKey(txn.GetCache(), "QA");
    IQueryProvider::Query queryA(responseQAKey, std::make_shared<WSQuery>(ObjectId {"TestSchema", "TestClass", "QA"}));
    StubInstances instancesA;
    instancesA.Add({"TestSchema.TestClass", "A"});
    txn.Commit();

    // Act & Assert
    auto provider = std::make_shared<MockQueryProvider>();
    EXPECT_CALL(*provider, GetQueries(_, _, _)).WillRepeatedly(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, _, _)).WillRepeatedly(Return(nullptr));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryA.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instancesA.ToWSObjectsResult())));

    int progressCalled = 0;
    CachingDataSource::Progress expectedProgress[] = {
            {0, {0, 0}},
            {1, {1, 1}}};
    ICachingDataSource::ProgressCallback onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_PROGRESS_EQ(expectedProgress[progressCalled], progress);
        progressCalled++;
        };

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(queryA), StubBVector<IQueryProviderPtr>(provider), onProgress, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueryWithInstanceThatReturnsQueryWithInstance_ProgressCalledWithInstancesState)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto responseQAKey = StubCachedResponseKey(txn.GetCache(), "QA");
    IQueryProvider::Query queryA(responseQAKey, std::make_shared<WSQuery>(ObjectId {"TestSchema", "TestClass", "QA"}));
    StubInstances instancesA;
    instancesA.Add({"TestSchema.TestClass", "A"});
    ObjectId instanceAId {"TestSchema.TestClass", "A"};
    auto instanceAKey = StubInstanceInCache(txn.GetCache(), instanceAId);

    auto responseQBKey = StubCachedResponseKey(txn.GetCache(), "QB");
    IQueryProvider::Query queryB(responseQBKey, std::make_shared<WSQuery>(ObjectId {"TestSchema", "TestClass", "QB"}));
    StubInstances instancesB;
    instancesB.Add({"TestSchema.TestClass", "B"});
    ObjectId instanceBId {"TestSchema.TestClass", "B"};
    auto instanceBKey = StubInstanceInCache(txn.GetCache(), instanceBId);
    txn.Commit();

    // Act & Assert
    auto provider = std::make_shared<MockQueryProvider>();
    EXPECT_CALL(*provider, GetQueries(_, _, _)).WillRepeatedly(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, _, _)).WillRepeatedly(Return(nullptr));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryA.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instancesA.ToWSObjectsResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryB.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instancesB.ToWSObjectsResult())));

    EXPECT_CALL(*provider, GetQueries(_, instanceAKey, _)).WillOnce(Return(StubBVector({queryB})));

    int progressCalled = 0;
    CachingDataSource::Progress expectedProgress[] = {
            {0,   {0, 0}},
            {0.5, {0, 1}},
            {1,   {2, 2}}};
    ICachingDataSource::ProgressCallback onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_PROGRESS_EQ(expectedProgress[progressCalled], progress);
        progressCalled++;
        };

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(queryA), StubBVector<IQueryProviderPtr>(provider), onProgress, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueryWithInstancesThatReturnQueriesWithNoInstances_ProgressCalledWithInstancesState)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto responseQKey = StubCachedResponseKey(txn.GetCache(), "Q");
    IQueryProvider::Query query(responseQKey, std::make_shared<WSQuery>(ObjectId {"TestSchema", "TestClass", "Q"}));
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});
    ObjectId instanceAId {"TestSchema.TestClass", "A"};
    ObjectId instanceBId {"TestSchema.TestClass", "B"};
    auto instanceAKey = StubInstanceInCache(txn.GetCache(), instanceAId);
    auto instanceBKey = StubInstanceInCache(txn.GetCache(), instanceBId);

    auto responseQ1AKey = StubCachedResponseKey(txn.GetCache(), "Q1A");
    auto responseQ2AKey = StubCachedResponseKey(txn.GetCache(), "Q2A");
    IQueryProvider::Query queryA1(responseQ1AKey, std::make_shared<WSQuery>(ObjectId {"TestSchema", "TestClass", "Q1A"}));
    IQueryProvider::Query queryA2(responseQ2AKey, std::make_shared<WSQuery>(ObjectId {"TestSchema", "TestClass", "Q2A"}));

    auto responseQ1BKey = StubCachedResponseKey(txn.GetCache(), "Q1B");
    auto responseQ2BKey = StubCachedResponseKey(txn.GetCache(), "Q2B");
    IQueryProvider::Query queryB1(responseQ1BKey, std::make_shared<WSQuery>(ObjectId {"TestSchema", "TestClass", "Q1B"}));
    IQueryProvider::Query queryB2(responseQ2BKey, std::make_shared<WSQuery>(ObjectId {"TestSchema", "TestClass", "Q2B"}));
    txn.Commit();

    // Act & Assert
    auto provider = std::make_shared<MockQueryProvider>();
    EXPECT_CALL(*provider, GetQueries(_, _, _)).WillRepeatedly(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, _, _)).WillRepeatedly(Return(nullptr));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(*query.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryA1.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryA2.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryB1.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(*queryB2.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));

    EXPECT_CALL(*provider, GetQueries(_, instanceAKey, _)).WillOnce(Return(StubBVector({queryA1, queryA2})));
    EXPECT_CALL(*provider, GetQueries(_, instanceBKey, _)).WillOnce(Return(StubBVector({queryB1, queryB2})));

    int progressCalled = 0;
    CachingDataSource::Progress expectedProgress[] = {
            {0,   {0, 0}},
            {0.2, {0, 2}},
            {0.4, {0, 2}},
            {0.6, {1, 2}},
            {0.8, {1, 2}},
            {1,   {2, 2}}};
    ICachingDataSource::ProgressCallback onProgress = [&] (CachingDataSource::ProgressCR progress)
        {
        EXPECT_PROGRESS_EQ(expectedProgress[progressCalled], progress);
        progressCalled++;
        };

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(query), StubBVector<IQueryProviderPtr>(provider), onProgress, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Daumantas.Kojelis                10/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncCachedData_AsyncTaskInCacheThread_AsyncTaskTerminatesWhilePreparingCachingQueries)
    {
    auto ds = GetTestDataSourceV2();
    auto txn = ds->StartCacheTransaction();
    auto provider = std::make_shared<MockQueryProvider>();

    auto responseKey = StubCachedResponseKey(txn.GetCache(), "Q");
    IQueryProvider::Query query(responseKey, std::make_shared<WSQuery>(ObjectId{ "TestSchema", "TestClass", "Q" }));
    StubInstances instances;

    instances.Add({ "TestSchema.TestClass" , "A" });
    ObjectId instanceAId{ "TestSchema.TestClass", "A" };
    ECInstanceKey instanceAKey = StubInstanceInCache(txn.GetCache(), instanceAId);

    AsyncTestCheckpoint cp;
    EXPECT_CALL(*provider, GetQueries(_, instanceAKey, _)).WillOnce(InvokeWithoutArgs([&]()
        {
        cp.CheckinAndWait();
        return bvector<IQueryProvider::Query>();
        }));

    instances.Add({ "TestSchema.TestClass" , "B" });
    ObjectId instanceBId{ "TestSchema.TestClass", "B" };
    ECInstanceKey instanceBKey = StubInstanceInCache(txn.GetCache(), instanceBId);

    bool preparedQueries = false;
    EXPECT_CALL(*provider, GetQueries(_, instanceBKey, _)).WillOnce(InvokeWithoutArgs([&]()
        {
        preparedQueries = true;
        return bvector<IQueryProvider::Query>();
        }));

    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(*query.query, _, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));
    EXPECT_CALL(*provider, IsFileRetrievalNeeded(_, _, _)).WillRepeatedly(Return(nullptr));

    auto syncCachedDataTask = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(query), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr);

    cp.WaitUntilReached();
    auto middleTask = ds->GetCacheAccessThread()->ExecuteAsync([&]
        {
        EXPECT_FALSE(preparedQueries);
        });

    cp.Continue();

    auto syncCachedDataResult = syncCachedDataTask->GetResult();
    ASSERT_TRUE(syncCachedDataResult.IsSuccess());

    middleTask->Wait();
    }

#endif // USE_GTEST
