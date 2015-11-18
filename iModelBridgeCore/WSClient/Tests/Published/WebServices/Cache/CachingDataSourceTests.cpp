/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/CachingDataSourceTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CachingDataSourceTests.h"

#include <WebServices/Cache/Persistence/DataSourceCache.h>
#include <WebServices/Cache/Persistence/DataReadOptions.h>
#include <Bentley/BeDebugLog.h>
#include "MockCachingDataSource.h"
#include "MockQueryProvider.h"

#ifdef USE_GTEST
USING_NAMESPACE_BENTLEY_WEBSERVICES

CachedResponseKey CreateTestResponseKey(ICachingDataSourcePtr ds, Utf8StringCR rootName = "StubResponseKeyRoot", Utf8StringCR keyName = BeGuid().ToString())
    {
    auto txn = ds->StartCacheTransaction();
    CachedResponseKey key(txn.GetCache().FindOrCreateRoot(rootName.c_str()), keyName);
    txn.Commit();
    return key;
    }

TEST_F(CachingDataSourceTests, DISABLED_OpenOrCreate_CalledSecondTimeAfterCacheWasCreated_OpensAndSucceeds)
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

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "TestSchema"), _, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        GetTestSchema()->WriteToXmlFile(filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));

    // Create
    auto thread = WorkerThread::Create("DS1");
    auto ds1 = CachingDataSource::OpenOrCreate(client, path, StubCacheEnvironemnt(), thread)->GetResult().GetValue();
    ASSERT_TRUE(nullptr != ds1);

    // Ensure that cache is closed
    ds1 = nullptr;
    thread->OnEmpty()->Wait();

    // Open
    auto ds2 = CachingDataSource::OpenOrCreate(client, path, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_TRUE(nullptr != ds2);
    EXPECT_TRUE(nullptr != ds2->StartCacheTransaction().GetCache().GetAdapter().GetECSchema("TestSchema"));
    }

TEST_F(CachingDataSourceTests, OpenOrCreate_NonECDbFileExists_Error)
    {
    BeFileName path = StubFile("NotECDbFileContents");

    auto client = MockWSRepositoryClient::Create();
    auto result = CachingDataSource::OpenOrCreate(client, path, StubCacheEnvironemnt())->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    }

TEST_F(CachingDataSourceTests, OpenOrCreate_NonDataSourceCacheDbExists_OpensAndStartsUpdatingWithRemoteSchemas)
    {
    BeFileName path = StubFilePath();

    ECDb db;
    db.CreateNewDb(path);

    auto client = MockWSRepositoryClient::Create();

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError()))));

    CachingDataSource::OpenOrCreate(client, path, StubCacheEnvironemnt())->Wait();
    }

TEST_F(CachingDataSourceTests, OpenOrCreate_DataSourceCacheDbExists_StartsUpdatingWithRemoteSchemas)
    {
    BeFileName path = StubFilePath();

    DataSourceCache db;
    ASSERT_EQ(SUCCESS, db.Create(path, StubCacheEnvironemnt()));
    ASSERT_EQ(SUCCESS, db.Close());

    auto client = MockWSRepositoryClient::Create();

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError()))));

    CachingDataSource::OpenOrCreate(client, path, StubCacheEnvironemnt())->Wait();
    }

TEST_F(CachingDataSourceTests, OpenOrCreate_SchemaPathNotPassedAndServerDoesNotReturnMetaSchema_GetsSchemasAndImportsThemWithMetaSchema)
    {
    auto client = MockWSRepositoryClient::Create();

    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "SchemaId"}, {{"Name", "UserSchema"}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(_, _, _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
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

TEST_F(CachingDataSourceTests, OpenOrCreate_SchemaPathNotPassedAndServerRetursMetaSchema_GetsAllSchemasAndMetaSchemaFromServer)
    {
    auto client = MockWSRepositoryClient::Create();

    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "MetaSchemaId"}, {{"Name", "MetaSchema"}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(_, _, _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("MetaSchema.ECSchemaDef", "MetaSchemaId"), objectId);
        return CreateCompletedAsyncTask(WSFileResult());
        }));

    CachingDataSource::OpenOrCreate(client, BeFileName(":memory:"), StubCacheEnvironemnt())->Wait();
    }

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

    EXPECT_CALL(*client, SendGetFileRequest(_, _, _, _, _)).Times(2)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("MetaSchema.ECSchemaDef", "SchemaId"), objectId);
        Utf8String schemaXml(
            R"(<ECSchema schemaName="UserSchema" nameSpacePrefix="US" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                       </ECSchema>)");
        SimpleWriteToFile(schemaXml, filePath);
        return CreateCompletedAsyncTask(WSFileResult::Success(WSFileResponse(filePath, HttpStatus::OK, "SchemaFileETag")));
        }))
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR, Utf8StringCR eTag, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
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

TEST_F(CachingDataSourceTests, UpdateSchemas_CacheCreatedWithLocalSchema_QueriesServerForRemoteSchemas)
    {
    // Arrange
    auto client = MockWSRepositoryClient::Create();
    auto ds = CreateNewTestDataSource(client);
    ASSERT_TRUE(nullptr != ds);

    // Act & Assert
    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "SchemaId"}, {{"Name", "TestSchema"}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse("SchemaListETag")))));

    EXPECT_CALL(*client, SendGetFileRequest(_, _, _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
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

TEST_F(CachingDataSourceTests, UpdateSchemas_SchemaWithReferancedSchema_ImportsBothSchemas)
    {
    // Arrange
    auto client = MockWSRepositoryClient::Create();
    auto ds = CreateNewTestDataSource(client);

    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "A"}, {{"Name", "SchemaWithReferance"}, {"VersionMajor", 1}, {"VersionMinor", 0}});
    schemas.Add({"MetaSchema.ECSchemaDef", "B"}, {{"Name", "ReferancedSchema"}, {"VersionMajor", 1}, {"VersionMinor", 456}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "A"), _, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        Utf8String schemaXml =
            R"( <ECSchema schemaName="SchemaWithReferance" nameSpacePrefix="A" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                    <ECSchemaReference name="ReferancedSchema" version="1.456" prefix="B" />
                </ECSchema>)";
        SimpleWriteToFile(schemaXml, filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));
    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "B"), _, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
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

TEST_F(CachingDataSourceTests, UpdateSchemas_NewSchemaWithExistingReferancedSchema_ImportsNewSchema)
    {
    // Arrange
    auto client = MockWSRepositoryClient::Create();
    auto ds = CreateNewTestDataSource(client);

    // Initial schema
    StubInstances schemas1;
    schemas1.Add({"MetaSchema.ECSchemaDef", "B"}, {{"Name", "ReferancedSchema"}, {"VersionMajor", 1}, {"VersionMinor", 0}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas1.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "B"), _, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
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

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas2.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "A"), _, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        Utf8String schemaXml =
            R"( <ECSchema schemaName="SchemaWithReferance" nameSpacePrefix="A" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                    <ECSchemaReference name="ReferancedSchema" version="01.00" prefix="B" />
                </ECSchema>)";
        SimpleWriteToFile(schemaXml, filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));
    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "B"), _, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
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

TEST_F(CachingDataSourceTests, UpdateSchemas_SchemasIncludeStandardSchemas_SkipsStandardSchemas)
    {
    // Arrange
    auto client = MockWSRepositoryClient::Create();
    auto ds = CreateNewTestDataSource(client);
    ASSERT_TRUE(nullptr != ds);

    // Act & Assert
    StubInstances schemas;
    schemas.Add({"MetaSchema.ECSchemaDef", "A"}, {{"Name", "Bentley_Standard_CustomAttributes"}, {"VersionMajor", 1}, {"VersionMinor", 0}});
    schemas.Add({"MetaSchema.ECSchemaDef", "B"}, {{"Name", "CustomSchema"}, {"VersionMajor", 1}, {"VersionMinor", 0}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemas.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(_, _, _, _, _)).Times(1)
        .WillRepeatedly(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("MetaSchema.ECSchemaDef", "B"), objectId);
        Utf8String schemaXml;
        StubSchema("CustomSchema", "CS")->WriteToXmlString(schemaXml);
        SimpleWriteToFile(schemaXml, filePath);
        return CreateCompletedAsyncTask(WSFileResult::Success(WSFileResponse(filePath, HttpStatus::OK, nullptr)));
        }));

    auto result = ds->UpdateSchemas(nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

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

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "A"), _, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        SimpleWriteToFile(StubSchemaXml("A"), filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "B"), _, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        SimpleWriteToFile(StubSchemaXml("B"), filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
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
        })->Wait();
    }

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

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "A"), _, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        SimpleWriteToFile(StubSchemaXml("A"), filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "B"), _, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        SimpleWriteToFile(StubSchemaXml("B"), filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
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
    EXPECT_THAT(schemaKeys, Contains(SchemaKey(L"A", 1, 0)));
    EXPECT_THAT(schemaKeys, Contains(SchemaKey(L"B", 4, 2)));
    }
    }

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

    listenerWeakPtr.lock()->OnServerInfoReceived(StubWSInfoWebApi(BeVersion(1, 3)));
    ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        auto txn = ds->StartCacheTransaction();
        EXPECT_EQ(BeVersion(1, 3), ds->GetServerInfo(txn).GetWebApiVersion());
        })->Wait();

        listenerWeakPtr.lock()->OnServerInfoReceived(StubWSInfoWebApi(BeVersion(1, 3, 1, 0)));
        ds->GetCacheAccessThread()->ExecuteAsync([=]
            {
            auto txn = ds->StartCacheTransaction();
            EXPECT_EQ(BeVersion(1, 3, 1, 0), ds->GetServerInfo(txn).GetWebApiVersion());
            })->Wait();
    }

TEST_F(CachingDataSourceTests, GetFile_InstanceIsNotCached_ErrorStatus)
    {
    auto ds = GetTestDataSourceV1();

    auto result = ds->GetFile({"TestSchema.TestClass", "Foo"}, CachingDataSource::DataOrigin::CachedData, nullptr, nullptr)->GetResult();

    EXPECT_EQ(CachingDataSource::Status::DataNotCached, result.GetError().GetStatus());
    }

TEST_F(CachingDataSourceTests, GetFile_FileInstanceIsCached_ProgressIsCalledWithNameAndSize)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    ObjectId fileId {"TestSchema.TestFileClass", "TestId"};

    StubInstances fileInstances;
    fileInstances.Add(fileId, {{"TestSize", "42"}, {"TestName", "TestFileName"}});

    auto txn = ds->StartCacheTransaction();
    auto s = ds->GetRepositorySchemas(txn);
    txn.GetCache().CacheInstanceAndLinkToRoot(fileId, fileInstances.ToWSObjectsResponse(), "root");
    txn.Commit();

    // Act & Assert
    int onProgressCalled = 0;
    CachingDataSource::LabeledProgressCallback onProgress =
        [&] (double bytesTransfered, double bytesTotal, Utf8StringCR label)
        {
        EXPECT_EQ(0, bytesTransfered);
        EXPECT_EQ(42, bytesTotal);
        EXPECT_EQ("TestFileName", label);
        onProgressCalled++;
        };

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, _, _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR, Utf8StringCR, HttpRequest::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        progress(0, 0);
        return CreateCompletedAsyncTask(WSFileResult());
        }));

    ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, onProgress, nullptr)->Wait();
    EXPECT_EQ(1, onProgressCalled);
    }

TEST_F(CachingDataSourceTests, GetFile_ClassDoesNotHaveFileDependentPropertiesCA_ProgressIsCalledWithNoNameAndNoSizeAndFileHasDefaultName)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    ObjectId fileId {"TestSchema.TestClass", "TestId"};

    StubInstances fileInstances;
    fileInstances.Add(fileId);

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheInstanceAndLinkToRoot(fileId, fileInstances.ToWSObjectsResponse(), "root");
    txn.Commit();

    // Act & Assert
    int onProgressCalled = 0;
    CachingDataSource::LabeledProgressCallback onProgress =
        [&] (double bytesTransfered, double bytesTotal, Utf8StringCR label)
        {
        EXPECT_EQ(0, bytesTransfered);
        EXPECT_EQ(0, bytesTotal);
        EXPECT_EQ("", label);
        onProgressCalled++;
        };

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, _, _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        EXPECT_EQ(L"TestClass_TestId", filePath.GetFileNameAndExtension());
        progress(0, 42);
        return CreateCompletedAsyncTask(WSFileResult());
        }));

    ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, onProgress, nullptr)->Wait();
    EXPECT_EQ(1, onProgressCalled);
    }

TEST_F(CachingDataSourceTests, GetFile_InstanceHasVeryLongRemoteIdAndNoFileDependentPropertiesCA_FileHasTruncatedNameAndCanBeWrittenTo)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    ObjectId fileId {"TestSchema.TestClass", Utf8String(10000, 'x')};

    StubInstances fileInstances;
    fileInstances.Add(fileId);

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheInstanceAndLinkToRoot(fileId, fileInstances.ToWSObjectsResponse(), "root");
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, _, _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        SimpleWriteToFile("TestContent", filePath);
        return CreateCompletedAsyncTask(WSFileResult::Success(WSFileResponse(filePath, HttpStatus::OK, "")));
        }));

    auto result = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("TestContent", SimpleReadFile(result.GetValue().GetFilePath()));
    }

TEST_F(CachingDataSourceTests, GetFile_ClassDoesNotHaveFileDependentPropertiesCAButHasLabel_ProgressIsCalledWithoutNameAndSizeAndFileHasInstanceLabel)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    ObjectId fileId {"TestSchema.TestLabeledClass", "TestId"};

    StubInstances fileInstances;
    fileInstances.Add(fileId, {{"Name", "TestLabel"}});

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheInstanceAndLinkToRoot(fileId, fileInstances.ToWSObjectsResponse(), "root");
    txn.Commit();

    // Act & Assert
    int onProgressCalled = 0;
    CachingDataSource::LabeledProgressCallback onProgress =
        [&] (double bytesTransfered, double bytesTotal, Utf8StringCR label)
        {
        EXPECT_EQ(0, bytesTransfered);
        EXPECT_EQ(0, bytesTotal);
        EXPECT_EQ("TestLabel", label);
        onProgressCalled++;
        };

    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, _, _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, HttpRequest::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        EXPECT_EQ(L"TestLabel", filePath.GetFileNameAndExtension());
        progress(0, 42);
        return CreateCompletedAsyncTask(WSFileResult());
        }));

    ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, onProgress, nullptr)->Wait();
    EXPECT_EQ(1, onProgressCalled);
    }

TEST_F(CachingDataSourceTests, CacheFiles_BothFilesCachedAndSkipCached_NoFileRequestAndSuccess)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    ObjectId fileId {"TestSchema.TestFileClass", "TestId"};
    ObjectId file2Id {"TestSchema.TestFileClass", "TestId2"};
    txn.GetCache().LinkInstanceToRoot(nullptr, fileId);
    txn.GetCache().CacheFile(fileId, StubWSFileResponse(StubFile()), FileCache::Persistent);
    txn.GetCache().LinkInstanceToRoot(nullptr, file2Id);
    txn.GetCache().CacheFile(file2Id, StubWSFileResponse(StubFile()), FileCache::Persistent);
    txn.Commit();

    bvector<ObjectId> files;
    files.push_back(fileId);
    files.push_back(file2Id);

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, _, _, _, _)).Times(0);

    auto result = ds->CacheFiles(files, true, FileCache::ExistingOrTemporary, nullptr, nullptr)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(CachingDataSourceTests, CacheFiles_OneFileCachedAndSkipCached_OneFileRequestAndSuccess)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    ObjectId fileId {"TestSchema.TestFileClass", "TestId"};
    ObjectId file2Id {"TestSchema.TestFileClass", "TestId2"};
    txn.GetCache().LinkInstanceToRoot(nullptr, fileId);
    txn.GetCache().LinkInstanceToRoot(nullptr, file2Id);
    txn.GetCache().CacheFile(file2Id, StubWSFileResponse(StubFile()), FileCache::Persistent);
    txn.Commit();

    bvector<ObjectId> files;
    files.push_back(fileId);
    files.push_back(file2Id);

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, _, _, _, _)).Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR fileName, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        SimpleWriteToFile("", fileName);
        return CreateCompletedAsyncTask(WSFileResult::Success(StubWSFileResponse(fileName, "")));
        }));

    auto result = ds->CacheFiles(files, true, FileCache::ExistingOrTemporary, nullptr, nullptr)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(CachingDataSourceTests, CacheFiles_OneFileCachedAndNoSkipCached_TwoFileRequestsAndSuccess)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    ObjectId fileId {"TestSchema.TestFileClass", "TestId"};
    ObjectId file2Id {"TestSchema.TestFileClass", "TestId2"};
    txn.GetCache().LinkInstanceToRoot(nullptr, fileId);
    txn.GetCache().LinkInstanceToRoot(nullptr, file2Id);
    txn.GetCache().CacheFile(file2Id, StubWSFileResponse(StubFile()), FileCache::Persistent);
    txn.Commit();

    bvector<ObjectId> files;
    files.push_back(fileId);
    files.push_back(file2Id);

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendGetFileRequest(_, _, _, _, _)).Times(2)
        .WillRepeatedly(Invoke([&] (ObjectIdCR, BeFileNameCR fileName, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        SimpleWriteToFile("", fileName);
        return CreateCompletedAsyncTask(WSFileResult::Success(StubWSFileResponse(fileName, "")));
        }));

    auto result = ds->CacheFiles(files, false, FileCache::ExistingOrTemporary, nullptr, nullptr)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(CachingDataSourceTests, DownloadAndCacheChildren_SpecificParent_ChildIsCached)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Parent"});
    txn.Commit();

    // Act & Assert    
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Child"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    bvector<ObjectId> parents;
    parents.push_back({"TestSchema.TestClass", "Parent"});
    auto result = ds->DownloadAndCacheChildren(parents, nullptr)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "Child"}).IsInCache());
    }

TEST_F(CachingDataSourceTests, GetNavigationChildren_SpecificParentInstance_ChildIsCachedAndReturned)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Parent"}));
    txn.Commit();

    // Act & Assert  
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Child"});
    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubWSInfoResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _)).Times(1)
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

TEST_F(CachingDataSourceTests, GetNavigationChildren_GettingRemoteData_ObjectIsCachedAndReturned)
    {
    // Arrange
    auto ds = CreateNewTestDataSource();

    // Act & Assert  
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId());
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

TEST_F(CachingDataSourceTests, GetNavigationChildren_GettingCachedDataAfterCached_ObjectIsReturned)
    {
    // Arrange
    auto ds = CreateNewTestDataSource();

    // Act & Assert  
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId());
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

TEST_F(CachingDataSourceTests, GetNavigationChildrenKeys_SpecificParentInstance_ChildIsCachedAndKeyReturned)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Parent"});
    txn.Commit();

    // Act & Assert  
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Child"});
    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubWSInfoResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    auto result = ds->GetNavigationChildrenKeys({"TestSchema.TestClass", "Parent"}, CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ECInstanceKey cachedChildKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "Child"});
    EXPECT_TRUE(cachedChildKey.IsValid());
    ASSERT_EQ(1, result.GetValue().GetKeys().size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cachedChildKey, result.GetValue().GetKeys()));
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    }

TEST_F(CachingDataSourceTests, GetNavigationChildrenKeys_GettingRemoteData_ObjectIsCachedAndReturned)
    {
    // Arrange
    auto ds = CreateNewTestDataSource();

    // Act & Assert  
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubWSInfoResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId());
    txn.Commit();

    auto result = ds->GetNavigationChildrenKeys(ObjectId(), CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ECInstanceKey cachedChildKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(cachedChildKey.IsValid());
    ASSERT_EQ(1, result.GetValue().GetKeys().size());
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cachedChildKey, result.GetValue().GetKeys()));
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    }

TEST_F(CachingDataSourceTests, GetNavigationChildrenKeys_GettingCachedDataAfterCached_ObjectIsReturned)
    {
    // Arrange
    auto ds = CreateNewTestDataSource();

    // Act & Assert  
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId());
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

TEST_F(CachingDataSourceTests, CacheNavigation_TwoLevelsCachedPreviouslyAsTemporary_RepeatsSameQueries)
    {
    // Arrange
    auto ds = CreateNewTestDataSource();

    // Act & Assert 
    StubInstances instances1;
    instances1.Add({"TestSchema.TestClass", "A"});
    StubInstances instances2;
    instances2.Add({"TestSchema.TestClass", "B"});

    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubWSInfoResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
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
        txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId());
        txn.Commit();

        ds->GetNavigationChildren(ObjectId(), CachingDataSource::DataOrigin::RemoteData, nullptr)->Wait();
        ds->GetNavigationChildren({"TestSchema.TestClass", "A"}, CachingDataSource::DataOrigin::RemoteData, nullptr)->Wait();

        bvector<ObjectId> navigationTreesToCacheFully;
        bvector<ObjectId> navigationTreesToUpdateOnly;
        navigationTreesToUpdateOnly.push_back(ObjectId());

        auto result = ds->CacheNavigation(navigationTreesToCacheFully, navigationTreesToUpdateOnly, nullptr, nullptr, nullptr)->GetResult();
        EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(CachingDataSourceTests, CacheNavigation_OneLevelCachedPreviouslyAsTemporary_RepeatsSameQueryAndCachesResults)
    {
    // Arrange
    auto ds = CreateNewTestDataSource();

    // Act & Assert 
    StubInstances instances1;
    instances1.Add({"TestSchema.TestClass", "A"});

    EXPECT_CALL(GetMockClient().GetMockWSClient(), GetServerInfo(_))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(StubWSInfoResult())));
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances1.ToWSObjectsResponse("TagA")))));
    EXPECT_CALL(GetMockClient(), SendGetChildrenRequest(_, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR parentObjectId, const bset<Utf8String>&, Utf8StringCR eTag, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId(), parentObjectId);
        EXPECT_EQ("TagA", eTag);
        return CreateCompletedAsyncTask(StubWSObjectsResultNotModified());
        }));

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId());
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

TEST_F(CachingDataSourceTests, CacheNavigation_TemporaryNavigationNotCached_DoesNothing)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

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

TEST_F(CachingDataSourceTests, CacheNavigation_NotCachedRootPassedToBeFullyCached_QueriesChildrenRecursivelyForRootAndCachesResult)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

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
        txn.GetCache().LinkInstanceToRoot(nullptr, ObjectId());
        txn.Commit();

        auto result = ds->CacheNavigation(navigationTreesToCacheFully, navigationTreesToUpdateOnly, nullptr, nullptr, nullptr)->GetResult();

        EXPECT_TRUE(result.IsSuccess());
        EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    }

TEST_F(CachingDataSourceTests, GetObject_ObjectNotCached_RetrievesRemoteObject)
    {
    auto ds = GetTestDataSourceV1();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(ObjectId("TestSchema.TestClass", "Foo"), _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult())));

    ds->GetObject(ObjectId("TestSchema.TestClass", "Foo"), CachingDataSource::DataOrigin::CachedOrRemoteData, IDataSourceCache::JsonFormat::Raw)->Wait();
    }

TEST_F(CachingDataSourceTests, GetObjects_CachedDataAndQueryResponseNotCached_ReturnsError)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::CachedData, nullptr, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::DataNotCached, result.GetError().GetStatus());
    }

TEST_F(CachingDataSourceTests, GetObjects_CachedOrRemoteDataAndQueryResponseNotCached_SendsQueryRequest)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .Times(1)
        .WillOnce(Invoke([&] (WSQueryCR passedQuery, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_THAT(passedQuery.GetSchemaName(), Eq(query.GetSchemaName()));
        EXPECT_THAT(passedQuery.GetClasses(), ContainerEq(query.GetClasses()));
        return CreateCompletedAsyncTask(WSObjectsResult());
        }));

    ds->GetObjects(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr, nullptr)->Wait();
    }

TEST_F(CachingDataSourceTests, GetObjects_CachedOrRemoteDataAndQueryResponseNotCachedAndNetworkError_ReturnsError)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult())));

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

TEST_F(CachingDataSourceTests, GetObjects_CachedOrRemoteDataAndQueryResponseNotCached_CachesQueryResponseAndReturnsInstances)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    EXPECT_EQ(1, result.GetValue().GetJson().size());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[0][DataSourceCache_PROPERTY_RemoteId].asString());
    }

TEST_F(CachingDataSourceTests, GetObjects_CachedOrRemoteDataAndQueryResponseIsCached_ReturnsCached)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse());
    txn.Commit();

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_EQ(1, result.GetValue().GetJson().size());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[0][DataSourceCache_PROPERTY_RemoteId].asString());
    }

TEST_F(CachingDataSourceTests, GetObjects_RemoteOrCachedDataAndConnectionError_ReturnsNetworkError)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteOrCachedData, nullptr, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

TEST_F(CachingDataSourceTests, GetObjects_RemoteOrCachedDataAndQueryResponseIsCachedAndConnectionError_ReturnsCached)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse());
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteOrCachedData, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_EQ(1, result.GetValue().GetJson().size());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[0][DataSourceCache_PROPERTY_RemoteId].asString());
    }

TEST_F(CachingDataSourceTests, GetObjects_RemoteOrCachedDataAndQueryResponseIsCachedAndNewData_ReturnsNew)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse());
    txn.Commit();

    StubInstances newInstances;
    newInstances.Add({"TestSchema.TestClass", "B"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(newInstances.ToWSObjectsResult())));

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteOrCachedData, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    EXPECT_EQ(1, result.GetValue().GetJson().size());
    EXPECT_EQ("B", result.GetValue().GetJson()[0][DataSourceCache_PROPERTY_RemoteId].asString());
    }

TEST_F(CachingDataSourceTests, GetObjects_RemoteDataAndQueryResponseIsCached_SendsQueryRequestWithETag)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheResponse(key, StubInstances().ToWSObjectsResponse("TestEtag"));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, Utf8String("TestEtag"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult())));

    ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->Wait();
    }

TEST_F(CachingDataSourceTests, GetObjects_RemoteDataAndQueryResponseIsCachedAndNetworkErrors_ReturnsError)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheResponse(key, StubInstances().ToWSObjectsResponse("TestEtag"));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _)).WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult())));

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

TEST_F(CachingDataSourceTests, GetObjects_ResponseDoesNotContainPreviouslyCachedObject_RemovesObjectFromCachedResponse)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();
    CachedResponseKey key = CreateTestResponseKey(ds);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse());
    txn.Commit();

    // Act & Assert    
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _)).Times(1)
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

TEST_F(CachingDataSourceTests, GetObjects_DataReadOptionsSpecified_ReturnsOnlyPropertiesSpecifiedByOptions)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances remoteInstances;
    remoteInstances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Foo"}, {"TestProperty2", "Boo"}});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _)).WillOnce(Return(CreateCompletedAsyncTask(remoteInstances.ToWSObjectsResult())));

    auto options = std::make_shared<DataReadOptions>();
    options->SelectClassAndProperty("TestSchema.TestClass", "TestProperty");

    auto result = ds->GetObjects(key, query, CachingDataSource::DataOrigin::RemoteData, options, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(1, result.GetValue().GetJson().size());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[0]["TestProperty"].asString());
    EXPECT_TRUE(result.GetValue().GetJson()[0]["TestProperty2"].isNull());
    }

TEST_F(CachingDataSourceTests, GetObjects_QueryIncludesPartialInstancesThatAreInFullyPersisted_QueriesAndCachesRejectedSeparatelly)
    {
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    StubInstances fullInstances;
    fullInstances.Add({"TestSchema.TestClass", "A"});
    fullInstances.Add({"TestSchema.TestClass", "B"});
    txn.GetCache().CacheInstancesAndLinkToRoot(fullInstances.ToWSObjectsResponse(), "SomePersistentRoot");
    txn.Commit();

    StubInstances remoteInstances;
    remoteInstances.Add({"TestSchema.TestClass", "A"});
    remoteInstances.Add({"TestSchema.TestClass", "B"});
    remoteInstances.Add({"TestSchema.TestClass", "C"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(remoteInstances.ToWSObjectsResult())))
        .WillOnce(Invoke([=] (WSQueryCR query, Utf8StringCR, ICancellationTokenPtr)
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
    query.SetSelect("$id");

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

TEST_F(CachingDataSourceTests, GetObjects_WSGV1NavigationQueryIncludesPartialInstancesThatAreInFullyPersisted_QueriesAndCachesRejectedSeparatelly)
    {
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    StubInstances fullInstances;
    fullInstances.Add({"TestSchema.TestClass", "A"});
    fullInstances.Add({"TestSchema.TestClass", "B"});
    txn.GetCache().CacheInstancesAndLinkToRoot(fullInstances.ToWSObjectsResponse(), "SomePersistentRoot");
    txn.Commit();

    StubInstances remoteInstances;
    remoteInstances.Add({"TestSchema.TestClass", "A"});
    remoteInstances.Add({"TestSchema.TestClass", "B"});
    remoteInstances.Add({"TestSchema.TestClass", "C"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(remoteInstances.ToWSObjectsResult())))
        .WillOnce(Invoke([=] (WSQueryCR query, Utf8StringCR, ICancellationTokenPtr)
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
    query.SetSelect("Name,OtherProperty");
    query.SetCustomParameter(WSQuery_CustomParameter_NavigationParentId, "Parent");

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

TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedDataAndQueryResponseNotCached_ReturnsError)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::CachedData, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::DataNotCached, result.GetError().GetStatus());
    }

TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedOrRemoteDataAndQueryResponseNotCached_SendsQueryRequest)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .Times(1)
        .WillOnce(Invoke([&] (WSQueryCR passedQuery, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_THAT(passedQuery.GetSchemaName(), Eq(query.GetSchemaName()));
        EXPECT_THAT(passedQuery.GetClasses(), ContainerEq(query.GetClasses()));
        return CreateCompletedAsyncTask(WSObjectsResult());
        }));

    ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr)->Wait();
    }

TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedOrRemoteDataAndQueryResponseNotCachedAndNetworkError_ReturnsError)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult())));

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedOrRemoteDataAndQueryResponseNotCached_CachesQueryResponseAndReturnsInstances)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(instances.ToWSObjectsResult())));

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    EXPECT_EQ(1, result.GetValue().GetKeys().size());
    auto cachedInstanceKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cachedInstanceKey, result.GetValue().GetKeys()));
    }

TEST_F(CachingDataSourceTests, GetObjectsKeys_CachedOrRemoteDataAndQueryResponseIsCached_ReturnsCached)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse());
    txn.Commit();

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_EQ(1, result.GetValue().GetKeys().size());
    auto cachedInstanceKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cachedInstanceKey, result.GetValue().GetKeys()));
    }

TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteOrCachedDataAndConnectionError_ReturnsNetworkError)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteOrCachedData, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteOrCachedDataAndQueryResponseIsCachedAndConnectionError_ReturnsCached)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse());
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteOrCachedData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_EQ(1, result.GetValue().GetKeys().size());
    auto cachedInstanceKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "Foo"});
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cachedInstanceKey, result.GetValue().GetKeys()));
    }

TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteOrCachedDataAndQueryResponseIsCachedAndNewData_ReturnsNew)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse());
    txn.Commit();

    StubInstances newInstances;
    newInstances.Add({"TestSchema.TestClass", "B"});

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(newInstances.ToWSObjectsResult())));

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteOrCachedData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    EXPECT_EQ(1, result.GetValue().GetKeys().size());
    auto cachedInstanceKey = ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "B"});
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cachedInstanceKey, result.GetValue().GetKeys()));
    }

TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteDataAndQueryResponseIsCached_SendsQueryRequestWithETag)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheResponse(key, StubInstances().ToWSObjectsResponse("TestEtag"));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, Utf8String("TestEtag"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult())));

    ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr)->Wait();
    }

TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteDataAndQueryResponseIsCachedAndNetworkErrors_ReturnsError)
    {
    auto ds = GetTestDataSourceV1();

    CachedResponseKey key = CreateTestResponseKey(ds);
    WSQuery query("TestSchema", "TestClass");

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheResponse(key, StubInstances().ToWSObjectsResponse("TestEtag"));
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _)).WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult())));

    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

TEST_F(CachingDataSourceTests, GetObjectsKeys_ResponseDoesNotContainPreviouslyCachedObject_RemovesObjectFromCachedResponse)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();
    CachedResponseKey key = CreateTestResponseKey(ds);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _)).Times(1)
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
    EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsInCache());
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "B"}).IsInCache());
    }

TEST_F(CachingDataSourceTests, GetObjectsKeys_RemoteDataAndResponseNotModified_ReturnsCachedData)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();
    CachedResponseKey key = CreateTestResponseKey(ds);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse("TestTag"));
    txn.Commit();

    ECInstanceKeyMultiMap expectedInstances;
    expectedInstances.insert(ECDbHelper::ToPair(ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "A"})));
    expectedInstances.insert(ECDbHelper::ToPair(ds->StartCacheTransaction().GetCache().FindInstance({"TestSchema.TestClass", "B"})));

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, Utf8String("TestTag"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubWSObjectsResponseNotModified()))));

    WSQuery query("TestSchema", "TestClass");
    auto result = ds->GetObjectsKeys(key, query, CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_THAT(result.GetValue().GetKeys(), ContainerEq(expectedInstances));
    }

TEST_F(CachingDataSourceTests, GetObject_RemoteOrCachedDataAndConnectionError_ReturnsNetworkError)
    {
    auto ds = GetTestDataSourceV1();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    ObjectId objectId("TestSchema.TestClass", "Foo");
    auto result = ds->GetObject(objectId, CachingDataSource::DataOrigin::RemoteOrCachedData, IDataSourceCache::JsonFormat::Raw)->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

TEST_F(CachingDataSourceTests, GetObject_RemoteOrCachedDataAndInstanceIsCachedAndConnectionError_ReturnsCached)
    {
    auto ds = GetTestDataSourceV1();

    ObjectId objectId("TestSchema.TestClass", "Foo");
    StubInstances instances;
    instances.Add(objectId);

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr);
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubWSConnectionError()))));

    auto result = ds->GetObject(objectId, CachingDataSource::DataOrigin::RemoteOrCachedData, IDataSourceCache::JsonFormat::Raw)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[DataSourceCache_PROPERTY_RemoteId].asString());
    }

TEST_F(CachingDataSourceTests, GetObject_RemoteOrCachedDataAndInstanceIsCachedAndServerReturnsNewData_ReturnsNew)
    {
    auto ds = GetTestDataSourceV1();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, {{"TestProperty", "A"}});

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr);
    txn.Commit();

    StubInstances newInstances;
    newInstances.Add(objectId, {{"TestProperty", "B"}});

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(newInstances.ToWSObjectsResult())));

    auto result = ds->GetObject(objectId, CachingDataSource::DataOrigin::RemoteOrCachedData, IDataSourceCache::JsonFormat::Raw)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::RemoteData, result.GetValue().GetOrigin());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[DataSourceCache_PROPERTY_RemoteId].asString());
    EXPECT_EQ("B", result.GetValue().GetJson()["TestProperty"].asString());
    }

TEST_F(CachingDataSourceTests, GetObject_RemoteDataAndNotModfieid_ReturnsCached)
    {
    auto ds = GetTestDataSourceV1();

    ObjectId objectId("TestSchema.TestClass", "Foo");

    StubInstances instances;
    instances.Add(objectId, {{"TestProperty", "A"}}, "TestTag");

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheInstancesAndLinkToRoot(instances.ToWSObjectsResponse(), nullptr);
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, Utf8String("TestTag"), _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(StubWSObjectsResponseNotModified()))));

    auto result = ds->GetObject(objectId, CachingDataSource::DataOrigin::RemoteData, IDataSourceCache::JsonFormat::Raw)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(CachingDataSource::DataOrigin::CachedData, result.GetValue().GetOrigin());
    EXPECT_EQ("Foo", result.GetValue().GetJson()[DataSourceCache_PROPERTY_RemoteId].asString());
    EXPECT_EQ("A", result.GetValue().GetJson()["TestProperty"].asString());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_NoChanges_DoesNoRequestsAndSucceeds)
    {
    auto ds = GetTestDataSourceV1();

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.GetValue().empty());
    }

TEST_F(CachingDataSourceTests, DISABLED_SyncLocalChanges_LaunchedFromTwoConnectionsToSameDb_SecondCallReturnsErrorFunctionalityNotSupported)
    {
    auto cache1 = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto cache2 = std::make_shared<NiceMock<MockDataSourceCache>>();
    BeCriticalSection c;
    auto ds1 = CreateMockedCachingDataSource(nullptr, cache1);
    auto ds2 = CreateMockedCachingDataSource(nullptr, cache2);

    EXPECT_CALL(cache1->GetChangeManagerMock(), HasChanges()).WillOnce(Return(true));
    EXPECT_CALL(cache2->GetChangeManagerMock(), HasChanges()).WillOnce(Return(true));

    EXPECT_CALL(*cache1, GetCacheDatabasePath()).WillOnce(Return(BeFileName(L"samePath")));
    EXPECT_CALL(*cache2, GetCacheDatabasePath()).WillOnce(Return(BeFileName(L"samePath")));

    AsyncTestCheckpoint check1;
    EXPECT_CALL(cache1->GetChangeManagerMock(), GetChanges(_, An<bool>())).WillOnce(Invoke([&] (IChangeManager::Changes&, bool)
        {
        check1.CheckinAndWait();
        return SUCCESS;
        }));
    ON_CALL(cache2->GetChangeManagerMock(), GetChanges(_, An<bool>())).WillByDefault(Return(SUCCESS));

    auto t1 = ds1->SyncLocalChanges(nullptr, nullptr);
    check1.WaitUntilReached();
    auto r2 = ds2->SyncLocalChanges(nullptr, nullptr)->GetResult();
    check1.Continue();
    auto r1 = t1->GetResult();

    ASSERT_TRUE(r1.IsSuccess());
    ASSERT_FALSE(r2.IsSuccess());
    EXPECT_EQ(ICachingDataSource::Status::FunctionalityNotSupported, r2.GetError().GetStatus());
    EXPECT_NE("", r2.GetError().GetMessage());
    }

TEST_F(CachingDataSourceTests, DISABLED_SyncLocalChanges_LaunchedFromTwoConnectionsToDifferentFiles_BothSucceeds)
    {
    auto cache1 = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto cache2 = std::make_shared<NiceMock<MockDataSourceCache>>();

    auto ds1 = CreateMockedCachingDataSource(nullptr, cache1);
    auto ds2 = CreateMockedCachingDataSource(nullptr, cache2);

    EXPECT_CALL(cache1->GetChangeManagerMock(), HasChanges()).WillOnce(Return(true));
    EXPECT_CALL(cache2->GetChangeManagerMock(), HasChanges()).WillOnce(Return(true));

    EXPECT_CALL(*cache1, GetCacheDatabasePath()).WillOnce(Return(BeFileName(L"someFilePath")));
    EXPECT_CALL(*cache2, GetCacheDatabasePath()).WillOnce(Return(BeFileName(L"otherFilePath")));

    AsyncTestCheckpoint check1;
    EXPECT_CALL(cache1->GetChangeManagerMock(), GetChanges(_, An<bool>())).WillOnce(Invoke([&] (IChangeManager::Changes&, bool)
        {
        check1.CheckinAndWait();
        return SUCCESS;
        }));
    ON_CALL(cache2->GetChangeManagerMock(), GetChanges(_, An<bool>())).WillByDefault(Return(SUCCESS));

    auto t1 = ds1->SyncLocalChanges(nullptr, nullptr);
    check1.WaitUntilReached();
    auto r2 = ds2->SyncLocalChanges(nullptr, nullptr)->GetResult();
    check1.Continue();
    auto r1 = t1->GetResult();

    ASSERT_TRUE(r1.IsSuccess());
    ASSERT_TRUE(r2.IsSuccess());
    }

TEST_F(CachingDataSourceTests, DISABLED_SyncLocalChanges_LaunchedFromTwoConnectionsToMemmoryDb_BothSucceeds)
    {
    auto cache1 = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto cache2 = std::make_shared<NiceMock<MockDataSourceCache>>();

    auto ds1 = CreateMockedCachingDataSource(nullptr, cache1);
    auto ds2 = CreateMockedCachingDataSource(nullptr, cache2);

    EXPECT_CALL(cache1->GetChangeManagerMock(), HasChanges()).WillOnce(Return(true));
    EXPECT_CALL(cache2->GetChangeManagerMock(), HasChanges()).WillOnce(Return(true));

    EXPECT_CALL(*cache1, GetCacheDatabasePath()).WillOnce(Return(BeFileName(L":memory:")));
    EXPECT_CALL(*cache2, GetCacheDatabasePath()).WillOnce(Return(BeFileName(L":memory:")));

    AsyncTestCheckpoint check1;
    EXPECT_CALL(cache1->GetChangeManagerMock(), GetChanges(_, An<bool>())).WillOnce(Invoke([&] (IChangeManager::Changes&, bool)
        {
        check1.CheckinAndWait();
        return SUCCESS;
        }));
    ON_CALL(cache2->GetChangeManagerMock(), GetChanges(_, An<bool>())).WillByDefault(Return(SUCCESS));

    auto t1 = ds1->SyncLocalChanges(nullptr, nullptr);
    check1.WaitUntilReached();
    auto r2 = ds2->SyncLocalChanges(nullptr, nullptr)->GetResult();
    check1.Continue();
    auto r1 = t1->GetResult();

    ASSERT_TRUE(r1.IsSuccess());
    ASSERT_TRUE(r2.IsSuccess());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObject_SetsSyncActiveFlagAndResetsItAfterSuccessfulSync)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .Times(1)
        .WillOnce(InvokeWithoutArgs([&] ()
        {
        ds->GetCacheAccessThread()->ExecuteAsync([&]
            {
            EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsSyncActive());
            });
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "Created"}));
        }));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Created"});
    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(instances.ToWSObjectsResponse()))));

    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsSyncActive());
    ds->SyncLocalChanges(nullptr, nullptr)->Wait();
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsSyncActive());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObject_SetsSyncActiveFlagAndResetsItAfterFailedSync)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .Times(1)
        .WillOnce(InvokeWithoutArgs([&] ()
        {
        ds->GetCacheAccessThread()->ExecuteAsync([=]
            {
            EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsSyncActive());
            });
        return CreateCompletedAsyncTask(StubWSCreateObjectResult());
        }));

    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsSyncActive());
    ds->SyncLocalChanges(nullptr, nullptr)->Wait();
    EXPECT_FALSE(ds->StartCacheTransaction().GetCache().GetChangeManager().IsSyncActive());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObject_SendsCreateObjectRequestWithCorrectParameters)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty" : "42"})"));
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
        .WillOnce(Invoke([=] (JsonValueCR json, BeFileNameCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(expectedCreationJson, json);
        return CreateCompletedAsyncTask(WSCreateObjectResult());
        }));

    ds->SyncLocalChanges(nullptr, nullptr)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_ServerV1CreatedObject_SendsQueryRequestToUpdateInstance)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, BeFileName(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask((StubWSCreateObjectResult({"TestSchema.TestClass", "NewId"})))));

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillOnce(Invoke([=] (ObjectIdCR objectId, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_THAT(objectId, Eq(ObjectId("TestSchema.TestClass", "NewId")));

        StubInstances instances;
        instances.Add({"TestSchema.TestClass", "NewId"}, {{"TestProperty", "TestValue"}});
        return CreateCompletedAsyncTask(instances.ToWSObjectsResult());
        }));

    ASSERT_TRUE(ds->SyncLocalChanges(nullptr, nullptr)->GetResult().IsSuccess());

    Json::Value jsonInstance;
    ds->StartCacheTransaction().GetCache().ReadInstance({"TestSchema.TestClass", "NewId"}, jsonInstance);
    EXPECT_THAT(jsonInstance["TestProperty"], Eq("TestValue"));
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_ServerV2CreatedObject_SendsQueryRequestAndUpdatesInstanceClassAndProperties)
    {
    // Arrange
    auto ds = GetTestDataSourceV2();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, BeFileName(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "NewId"}))));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .WillOnce(Invoke([=] (WSQueryCR query, Utf8StringCR, ICancellationTokenPtr)
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

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndNoChanges_DoesNoRequestsAndSucceeds)
    {
    auto ds = GetTestDataSource({2, 1});

    SyncOptions options;
    options.SetUseChangesets(true);
    auto result = ds->SyncLocalChanges(nullptr, nullptr, options)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.GetValue().empty());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledCreatedModifiedDeletedObjects_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

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
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset changeset;
        changeset.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty" : "NewValue"})"));
        changeset.AddInstance({"TestSchema.TestClass", "ToModify"}, WSChangeset::Modified, ToJsonPtr(R"({"TestProperty" : "ModifiedValue"})"));
        changeset.AddInstance({"TestSchema.TestClass", "ToDelete"}, WSChangeset::Deleted, nullptr);

        EXPECT_EQ(ToJson(changeset.ToRequestString()), changesetBody->AsJson());
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedRelationship_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

    auto txn = ds->StartCacheTransaction();
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto instanceA = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClassA", "A"});
    auto instanceB = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClassB", "B"});
    auto relationship = txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceB);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset changeset;
        changeset.AddInstance({"TestSchema.TestClassA", "A"}, WSChangeset::Existing, nullptr)
            .AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
            {"TestSchema.TestClassB", "B"}, WSChangeset::Existing, nullptr);

        EXPECT_EQ(ToJson(changeset.ToRequestString()), changesetBody->AsJson());
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndDeletedRelationship_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

    StubInstances instances;
    instances.Add({"TestSchema.TestClassA", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClassB", "B"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheResponse(StubCachedResponseKey(txn.GetCache()), instances.ToWSObjectsResponse()));
    auto relClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = txn.GetCache().FindRelationship(*relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().DeleteRelationship(relationship));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset changeset;
        changeset.AddInstance({"TestSchema.TestRelationshipClass", "AB"}, WSChangeset::Deleted, nullptr);

        EXPECT_EQ(ToJson(changeset.ToRequestString()), changesetBody->AsJson());
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedRelatedTargetObject_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

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
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset changeset;
        changeset.AddInstance({"TestSchema.TestClassA", "ExistingId"}, WSChangeset::Existing, nullptr)
            .AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
            {"TestSchema.TestClassB", "B"}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"B"})"));

        EXPECT_EQ(ToJson(changeset.ToRequestString()), changesetBody->AsJson());
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedRelatedSourceObject_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

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
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset changeset;
        changeset.AddInstance({"TestSchema.TestClassA", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"))
            .AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
            {"TestSchema.TestClassB", "ExistingId"}, WSChangeset::Existing, nullptr);

        EXPECT_EQ(ToJson(changeset.ToRequestString()), changesetBody->AsJson());
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedObjectsWithForwardRelationship_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

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
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset changeset;
        changeset.AddInstance({"TestSchema.TestClassA", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"))
            .AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
            {"TestSchema.TestClassB", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"B"})"));

        EXPECT_EQ(ToJson(changeset.ToRequestString()), changesetBody->AsJson());
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedObjectsWithBackwardRelationship_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

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
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset changeset;
        changeset.AddInstance({"TestSchema.TestClassA", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"))
            .AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Backward,
            {"TestSchema.TestClassB", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"B"})"));

        EXPECT_EQ(ToJson(changeset.ToRequestString()), changesetBody->AsJson());
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedObjectsWithManyRelatedObjects_SendsChangeset)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

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
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset changeset;
        auto& a = changeset.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"));

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

        EXPECT_EQ(ToJson(changeset.ToRequestString()), changesetBody->AsJson());
        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedRelatedObjectsAndSuccessfulResponse_CommitsRemoteIdsToInstances)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

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
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
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

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndCreatedRelatedObjectToExistingAndSuccessfulResponse_CommitsRemoteIdsToInstances)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

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
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
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

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledModifiedObjectAndSuccessfulResponse_CommitsChanges)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyObject(instance, ToJson(R"({"TestProperty" : "ModifiedValue"})")));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
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

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledDeletedObjectAndSuccessfulResponse_CommitsChanges)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().DeleteObject(instance));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
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

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledDeletedRelationshipAndSuccessfulResponse_CommitsChanges)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

    StubInstances instances;
    instances.Add({"TestSchema.TestClassA", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClassB", "B"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheResponse(StubCachedResponseKey(txn.GetCache()), instances.ToWSObjectsResponse()));
    auto relClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = txn.GetCache().FindRelationship(*relClass, {"TestSchema.TestClassA", "A"}, {"TestSchema.TestClassB", "B"});
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().DeleteRelationship(relationship));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
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

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndChangesetSizeLimited_SendsTwoChangesetsSoTheyWouldFitIntoLimit)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

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
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset changeset;
        auto& a = changeset.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"));
        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"B"})"));
        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"C"})"));

        EXPECT_LE(changesetBody->GetLength(), 700);
        EXPECT_EQ(ToJson(changeset.ToRequestString()), changesetBody->AsJson());

        StubInstances instances;
        auto instance = instances.Add({"TestSchema.TestClass", "RemoteIdA"});
        instance.AddRelated({"TestSchema.TestRelationshipClass", "RemoteIdAB"}, {"TestSchema.TestClass", "RemoteIdB"});
        instance.AddRelated({"TestSchema.TestRelationshipClass", "RemoteIdAC"}, {"TestSchema.TestClass", "RemoteIdC"});

        return CreateCompletedAsyncTask(instances.ToWSChangesetResult());
        }));

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset changeset;
        auto& a = changeset.AddInstance({"TestSchema.TestClass", "RemoteIdA"}, WSChangeset::Existing, nullptr);
        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"D"})"));

        EXPECT_LE(changesetBody->GetLength(), 700);
        EXPECT_EQ(ToJson(changeset.ToRequestString()), changesetBody->AsJson());

        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    options.SetMaxChangesetSize(700);
    auto result = ds->SyncLocalChanges(nullptr, nullptr, options)->GetResult();
    EXPECT_EQ(ICachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndChangesetSizeLimitIsSmallerThanInstance_ReturnsError)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

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

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndChangesetInstanceCountLimited_SendsTwoChangesetsSoTheyWouldFitIntoLimit)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

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
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset changeset;
        auto& a = changeset.AddInstance({"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"A"})"));
        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"B"})"));
        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"C"})"));

        EXPECT_EQ(ToJson(changeset.ToRequestString()), changesetBody->AsJson());

        StubInstances instances;
        auto instance = instances.Add({"TestSchema.TestClass", "RemoteIdA"});
        instance.AddRelated({"TestSchema.TestRelationshipClass", "RemoteIdAB"}, {"TestSchema.TestClass", "RemoteIdB"});
        instance.AddRelated({"TestSchema.TestRelationshipClass", "RemoteIdAC"}, {"TestSchema.TestClass", "RemoteIdC"});

        return CreateCompletedAsyncTask(instances.ToWSChangesetResult());
        }));

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        WSChangeset changeset;
        auto& a = changeset.AddInstance({"TestSchema.TestClass", "RemoteIdA"}, WSChangeset::Existing, nullptr);
        a.AddRelatedInstance({"TestSchema.TestRelationshipClass", ""}, WSChangeset::Created, ECRelatedInstanceDirection::Forward,
        {"TestSchema.TestClass", ""}, WSChangeset::Created, ToJsonPtr(R"({"TestProperty":"D"})"));

        EXPECT_EQ(ToJson(changeset.ToRequestString()), changesetBody->AsJson());

        return CreateCompletedAsyncTask(WSChangesetResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    options.SetMaxChangesetInstanceCount(3);
    auto result = ds->SyncLocalChanges(nullptr, nullptr, options)->GetResult();
    EXPECT_EQ(ICachingDataSource::Status::NetworkErrorsOccured, result.GetError().GetStatus());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndOneObjectWithFile_InterruptsChangesetsWithCreateObjectRequestForFile)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");

    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A"})"));
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"B"})"));
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(instanceB, StubFile(), true));
    auto filePath = txn.GetCache().ReadFilePath(instanceB);
    auto instanceC = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"C"})"));

    txn.Commit();

    Json::Value expectedChangeset1 = ToJson(
        R"( {
            "instances" :
                [{
                "changeState": "new",
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "properties" : {"TestProperty":"A"}
                }]
            })");

    Json::Value expectedCreation2 = ToJson(
        R"( {
            "instance" :
                {
                "changeState": "new",
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "properties" : {"TestProperty":"B"}
                }
            })");

    Json::Value expectedChangeset3 = ToJson(
        R"( {
            "instances" :
                [{
                "changeState": "new",
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "properties" : {"TestProperty":"C"}
                }]
            })");

    {
    InSequence callsInSequence;

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(expectedChangeset1, changesetBody->AsJson());

        StubInstances instances;
        instances.Add({"TestSchema.TestClass", "RemoteIdA"});

        return CreateCompletedAsyncTask(instances.ToWSChangesetResult());
        }));

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR creationJson, BeFileNameCR path, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(expectedCreation2, creationJson);
        EXPECT_EQ(filePath, path);
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "RemoteIdB"}));
        }));

    EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
        .WillOnce(Invoke([&] (WSQueryCR query, Utf8StringCR, ICancellationTokenPtr)
        {
        EXPECT_THAT(query.ToQueryString(), Eq("$filter=$id+eq+'RemoteIdB'"));

        StubInstances instances;
        instances.Add({"TestSchema.TestDerivedClass", "RemoteIdB"});

        return CreateCompletedAsyncTask(instances.ToWSObjectsResult());
        }));

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(expectedChangeset3, changesetBody->AsJson());

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

TEST_F(CachingDataSourceTests, SyncLocalChanges_V21WithChangesetEnabledAndRelatedObjectWithFile_SendsCreateObjectRequestForFile)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 1});

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
    Json::Value expectedChangeset1 = ToJson(R"({
        "instances" :
            [{
            "changeState": "new",
            "schemaName" : "TestSchema",
            "className" : "TestClass",
            "properties" : {"TestProperty":"A"}
            }]})");

    // Request 2
    Json::Value expectedCreation2 = ToJson(
        R"( {
            "instance" :
                {
                "changeState": "new",
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "properties" : {"TestProperty":"B"},
                "relationshipInstances" :
                    [{
                    "changeState": "new",
                    "schemaName" : "TestSchema",
                    "className" : "TestRelationshipClass",
                    "direction" : "backward",
                    "relatedInstance" :
                        {
                        "schemaName" : "TestSchema",
                        "className" : "TestClass",
                        "instanceId" : "RemoteIdA"
                        }
                    }]
                }
            })");

    {
    InSequence callsInSequence;

    EXPECT_CALL(GetMockClient(), SendChangesetRequest(_, _, _))
        .WillOnce(Invoke([&] (HttpBodyPtr changesetBody, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(expectedChangeset1, changesetBody->AsJson());

        StubInstances instances;
        instances.Add({"TestSchema.TestClass", "RemoteIdA"});

        return CreateCompletedAsyncTask(instances.ToWSChangesetResult());
        }));

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR creationJson, BeFileNameCR path, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(expectedCreation2, creationJson);
        EXPECT_EQ(filePath, path);
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(StubWSConnectionError()));
        }));
    }

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_V20WithChangesetEnabledAndCreatedObject_SendsCreateObjectRequestsBecauseChangesetsAreNotSupported)
    {
    // Arrange
    auto ds = GetTestDataSource({2, 0});

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A"})"));
    txn.Commit();

    // Act & Assert
    Json::Value expectedCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "changeState": "new",
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "properties" : {"TestProperty":"A"}
                }
            })");

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(expectedCreationJson, json);
        EXPECT_EQ(L"", path);
        return CreateCompletedAsyncTask(WSCreateObjectResult::Error(StubWSConnectionError()));
        }));

    SyncOptions options;
    options.SetUseChangesets(true);
    ds->SyncLocalChanges(nullptr, nullptr, options)->Wait();
    }

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
    txn.GetCache().GetChangeManager().ModifyFile(instanceC, StubFile(), false);

    txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceB);
    txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceB, instanceC);
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
                        "schemaName" : "TestSchema",
                        "className" : "TestDerivedClass",
                        "instanceId" : "NewB"
                        }
                    }]
                }
            })");
    BeFileName filePath2 = ds->StartCacheTransaction().GetCache().ReadFilePath(instanceC);

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(expectedCreationJson1, json);
        EXPECT_EQ(L"", path);
        return CreateCompletedAsyncTask(StubWSCreateObjectResult(
            {"TestSchema.TestClass", "NewB"}, {"TestSchema.TestRelationshipClass", ""}, {"TestSchema.TestClass", "A"}));
        }))
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
            {
            EXPECT_EQ(expectedCreationJson2, json);
            EXPECT_EQ(filePath2, path);
            return CreateCompletedAsyncTask(StubWSCreateObjectResult(
                {"TestSchema.TestClass", "NewC"}, {"TestSchema.TestRelationshipClass", ""}, {"TestSchema.TestClass", "NewB"}));
            }));

        EXPECT_CALL(GetMockClient(), SendQueryRequest(_, _, _))
            .WillOnce(Invoke([=] (WSQueryCR query, Utf8StringCR, ICancellationTokenPtr)
            {
            EXPECT_THAT(query.ToQueryString(), Eq("$filter=$id+eq+'NewB'"));
            StubInstances instances;
            instances.Add({"TestSchema.TestDerivedClass", "NewB"});
            return CreateCompletedAsyncTask(instances.ToWSObjectsResult());
            }))
            .WillOnce(Invoke([=] (WSQueryCR query, Utf8StringCR, ICancellationTokenPtr)
                {
                EXPECT_THAT(query.ToQueryString(), Eq("$filter=$id+eq+'NewC'"));
                StubInstances instances;
                instances.Add({"TestSchema.TestDerivedClass", "NewC"});
                return CreateCompletedAsyncTask(instances.ToWSObjectsResult());
                }));

            auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
            EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_V1CreatedRelatedObjectsWithFile_SendsSeperateRequestsForEachNewObjectAndRelationship)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto instanceA = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty" : "ValB"})"));
    auto instanceC = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty" : "ValC"})"));
    txn.GetCache().GetChangeManager().ModifyFile(instanceC, StubFile(), false);

    txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceB);
    txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceB, instanceC);
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
                        "schemaName" : "TestSchema",
                        "className" : "TestClass",
                        "instanceId" : "NewB"
                        }
                    }]
                }
            })");
    BeFileName filePath2 = ds->StartCacheTransaction().GetCache().ReadFilePath(instanceC);

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .Times(2)
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(expectedCreationJson1, json);
        EXPECT_EQ(L"", path);
        return CreateCompletedAsyncTask(StubWSCreateObjectResult(
            {"TestSchema.TestClass", "NewB"}, {"TestSchema.TestRelationshipClass", ""}, {"TestSchema.TestClass", "A"}));
        }))
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
            {
            EXPECT_EQ(expectedCreationJson2, json);
            EXPECT_EQ(filePath2, path);
            return CreateCompletedAsyncTask(StubWSCreateObjectResult(
                {"TestSchema.TestClass", "NewC"}, {"TestSchema.TestRelationshipClass", ""}, {"TestSchema.TestClass", "NewB"}));
            }));

        EXPECT_CALL(GetMockClient(), SendGetObjectRequest(ObjectId {"TestSchema.TestClass", "NewB"}, _, _))
            .Times(1)
            .WillOnce(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "NewB"}))));

        EXPECT_CALL(GetMockClient(), SendGetObjectRequest(ObjectId {"TestSchema.TestClass", "NewC"}, _, _))
            .Times(1)
            .WillOnce(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "NewC"}))));

        auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
        EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedTwoRelatedInstancesAndFirstOneFails_SecondOneFailureHasDependencySyncFailedStatus)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto instanceA = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    auto instanceC = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);

    txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceB);
    txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceB, instanceC);
    txn.Commit();

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSCreateObjectResult::Error(WSError(WSError::Id::Conflict)))));

    BeTest::SetFailOnAssert(false);
    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    BeTest::SetFailOnAssert(true);
    EXPECT_TRUE(result.IsSuccess());

    ASSERT_THAT(result.GetValue(), SizeIs(4));
    EXPECT_THAT(result.GetValue()[0].GetObjectId(), Eq(ds->StartCacheTransaction().GetCache().FindInstance(instanceB)));
    EXPECT_THAT(result.GetValue()[0].GetError().GetStatus(), ICachingDataSource::Status::NetworkErrorsOccured);
    EXPECT_THAT(result.GetValue()[2].GetObjectId(), Eq(ds->StartCacheTransaction().GetCache().FindInstance(instanceC)));
    EXPECT_THAT(result.GetValue()[2].GetError().GetStatus(), ICachingDataSource::Status::DependencyNotSynced);
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectWithTwoRelationships_SecondRelationshipCreationSentSeperately)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");

    auto instanceA = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    auto instanceB = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "B"});
    auto instanceC = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty" : "ValC"})"));

    txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceC);
    txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceB, instanceC);
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
                        "schemaName" : "TestSchema",
                        "className" : "TestClass",
                        "instanceId" : "NewC"
                        }
                    }]
                }
            })");

    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .Times(2)
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(expectedCreationJson1, json);
        return CreateCompletedAsyncTask(StubWSCreateObjectResult(
            {"TestSchema.TestClass", "NewC"}, {"TestSchema.TestRelationshipClass", ""}, {"TestSchema.TestClass", "A"}));
        }))
        .WillOnce(Invoke([&] (JsonValueCR json, BeFileNameCR path, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
            {
            EXPECT_EQ(expectedCreationJson2, json);
            return CreateCompletedAsyncTask(StubWSCreateObjectResult
                (
                {"TestSchema.TestClass", "B"},
                {"TestSchema.TestRelationshipClass", "BC"},
                {"TestSchema.TestClass", "NewC"})
                );
            }));

        EXPECT_CALL(GetMockClient(), SendGetObjectRequest(ObjectId {"TestSchema.TestClass", "NewC"}, _, _))
            .Times(1)
            .WillOnce(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "NewC"}))));

        auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
        EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjet_SetsNewRemoteIdAndCommits)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "CreatedObjectId"}))));

    EXPECT_CALL(GetMockClient(), SendGetObjectRequest(ObjectId("TestSchema.TestClass", "CreatedObjectId"), _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "CreatedObjectId"}))));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo(instance).IsInCache());
    EXPECT_TRUE(ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "CreatedObjectId"}).IsInCache());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, ds->StartCacheTransaction().GetCache().GetCachedObjectInfo({"TestSchema.TestClass", "CreatedObjectId"}).GetChangeStatus());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectWithReadOnlyProperties_SendsReadOnlyButNotCalculatedProperty)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass3");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, ToJson(R"({ "TestReadOnlyProperty" : "42" })"));
    ASSERT_TRUE(instance.IsValid());
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, BeFileName(), _, _))
        .Times(1)
        .WillOnce(Invoke([=] (JsonValueCR json, BeFileNameCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_TRUE(json["instance"]["properties"].isMember("TestReadOnlyProperty"));
        EXPECT_FALSE(json["instance"]["properties"].isMember("TestCalculatedProperty"));
        return CreateCompletedAsyncTask(WSCreateObjectResult());
        }));
    ds->SyncLocalChanges(nullptr, nullptr)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedObjectWithReadOnlyProperties_DoesNotSendAnyReadOnlyProperties)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass3", "Foo"});
    txn.GetCache().GetChangeManager().ModifyObject(instance, ToJson(R"({ "TestReadOnlyProperty" : "42" })"));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateObjectRequest(_, _, _, _, _))
        .Times(1)
        .WillOnce(Invoke([=] (ObjectIdCR, JsonValueCR properties, Utf8String, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_FALSE(properties.isMember("TestReadOnlyProperty"));
        EXPECT_FALSE(properties.isMember("TestCalculatedProperty"));
        return CreateCompletedAsyncTask(WSUpdateObjectResult());
        }));

    ds->SyncLocalChanges(nullptr, nullptr)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedObject_SendUpdateObjectRequestWithOnlyChangedPropertiesAndCommits)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "Foo"}, {{"TestProperty", "OldA"},{ "TestProperty2", "OldB"}});

    Json::Value newPropertiesJson = ToJson(R"({ "TestProperty" : "NewA", "TestProperty2" : "OldB" })");
    txn.GetCache().GetChangeManager().ModifyObject(instance, newPropertiesJson);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateObjectRequest(_, _, _, _, _))
        .Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, JsonValueCR propertiesJson, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("TestSchema.TestClass", "Foo"), objectId);
        EXPECT_EQ(ToJson(R"({ "TestProperty" : "NewA" })"), propertiesJson);
        return CreateCompletedAsyncTask(WSUpdateObjectResult::Success());
        }));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, ds->StartCacheTransaction().GetCache().GetCachedObjectInfo(instance).GetChangeStatus());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedFile_SendUpdateFileRequestWithCorrectParametersAndCommits)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "Foo"});
    txn.GetCache().GetChangeManager().ModifyFile(instance, StubFile(), false);
    auto cachedFilePath = txn.GetCache().ReadFilePath(instance);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateFileRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR objectId, BeFileNameCR filePath, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(ObjectId("TestSchema.TestClass", "Foo"), objectId);
        EXPECT_EQ(cachedFilePath, filePath);
        return CreateCompletedAsyncTask(WSUpdateObjectResult::Success());
        }));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, ds->StartCacheTransaction().GetCache().GetChangeManager().GetFileChange(instance).GetChangeStatus());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectWithFile_SendUpdateFileRequestWithCorrectParametersAndCommits)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    txn.GetCache().GetChangeManager().ModifyFile(instance, StubFile(), false);
    auto cachedFilePath = txn.GetCache().ReadFilePath(instance);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR filePath, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        EXPECT_EQ(cachedFilePath, filePath);
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "Foo"}));
        }));

    ON_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillByDefault(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "Foo"}))));

    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, ds->StartCacheTransaction().GetCache().GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, ds->StartCacheTransaction().GetCache().GetChangeManager().GetFileChange(instance).GetChangeStatus());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_DeletedObject_SendsDeleteObjectRequestWithCorrectParametersAndCommits)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "Foo"});
    txn.GetCache().GetChangeManager().DeleteObject(instance);
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
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, ds->StartCacheTransaction().GetCache().GetChangeManager().GetFileChange(instance).GetChangeStatus());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_DeletedRelationship_SendsDeleteObjectRequestWithCorrectParametersAndCommits)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});

    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().CacheResponse(StubCachedResponseKey(txn.GetCache()), instances.ToWSObjectsResponse()));
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

TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectWithClassThatHasLabel_CallsProgressWithLabelAndWithoutBytes)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

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
    auto onProgress = [&] (double, Utf8StringCR taskLabel, double bytesTransfered, double bytesTotal)
        {
        onProgressCount++;
        EXPECT_EQ(0, bytesTransfered);
        EXPECT_EQ(0, bytesTotal);
        EXPECT_EQ("TestLabel", taskLabel);
        };

    ds->SyncLocalChanges(onProgress, nullptr)->Wait();
    EXPECT_EQ(1, onProgressCount);
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectWithClassWithoutLabel_CallsProgressWithFallbackLabel)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

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
    auto onProgress = [&] (double, Utf8StringCR taskLabel, double, double)
        {
        onProgressCount++;
        EXPECT_EQ("TestClass:" + objectId.remoteId, taskLabel);
        };

    ds->SyncLocalChanges(onProgress, nullptr)->Wait();
    EXPECT_EQ(1, onProgressCount);
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedObjectWithLabel_CallsProgressWithLabelAndWithoutBytes)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestLabeledClass", "Foo"});
    txn.GetCache().GetChangeManager().ModifyObject(instance, ToJson(R"({"Name" : "TestLabel"})"));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateObjectRequest(_, _, _, _, _))
        .Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(WSUpdateObjectResult())));

    int onProgressCount = 0;
    auto onProgress = [&] (double synced, Utf8StringCR taskLabel, double, double)
        {
        onProgressCount++;
        EXPECT_EQ("TestLabel", taskLabel);
        };

    ds->SyncLocalChanges(onProgress, nullptr)->Wait();
    EXPECT_EQ(1, onProgressCount);
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedAndModifiedAndDeletedObjects_CallsSyncedInstancesProgress)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();
    auto txn = ds->StartCacheTransaction();

    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instanceB = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "B"});
    auto instanceC = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "C"});
    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    txn.GetCache().GetChangeManager().ModifyObject(instanceB, Json::objectValue);
    txn.GetCache().GetChangeManager().DeleteObject(instanceC);

    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "Foo"}))));

    ON_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
        .WillByDefault(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "Foo"}))));

    EXPECT_CALL(GetMockClient(), SendUpdateObjectRequest(_, _, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSUpdateObjectResult::Success())));

    EXPECT_CALL(GetMockClient(), SendDeleteObjectRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSDeleteObjectResult::Success())));

    int onProgressCount = 0;
    double expectedSyncedValues[4] = {0, 0.33, 0.66, 1};
    auto onProgress = [&] (double synced, Utf8StringCR, double, double)
        {
        EXPECT_EQ(expectedSyncedValues[onProgressCount], synced);
        onProgressCount++;
        };

    ds->SyncLocalChanges(onProgress, nullptr)->Wait();
    EXPECT_EQ(4, onProgressCount);
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedObject_CallsSyncedInstanceProgress)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestLabeledClass", "Foo"});
    txn.GetCache().GetChangeManager().ModifyObject(instance, ToJson(R"({"Name" : "TestLabelA"})"));
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateObjectRequest(_, _, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSUpdateObjectResult::Success())));

    int onProgressCount = 0;
    double expectedSyncedValues[2] = {0, 1};
    auto onProgress = [&] (double synced, Utf8StringCR, double, double)
        {
        EXPECT_EQ(expectedSyncedValues[onProgressCount], synced);
        onProgressCount++;
        };

    ds->SyncLocalChanges(onProgress, nullptr)->Wait();
    EXPECT_EQ(2, onProgressCount);
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedFileWithLabel_CallsProgressWithLabel)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    txn.GetCache().CacheInstanceAndLinkToRoot({"TestSchema.TestLabeledClass", "Foo"}, *ToRapidJson(R"({"Name" : "TestLabel"})"), "", "");
    auto instance = txn.GetCache().FindInstance({"TestSchema.TestLabeledClass", "Foo"});
    txn.GetCache().GetChangeManager().ModifyFile(instance, StubFile("12"), false);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateFileRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR, HttpRequest::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        progress(0, 2);
        return CreateCompletedAsyncTask(WSUpdateFileResult());
        }));

    int onProgressCount = 0;
    auto onProgress = [&] (double, Utf8StringCR taskLabel, double, double)
        {
        onProgressCount++;
        EXPECT_EQ("TestLabel", taskLabel);
        };

    ds->SyncLocalChanges(onProgress, nullptr)->Wait();
    EXPECT_EQ(2, onProgressCount);
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_CreatedObjectsWithFiles_CallsProgressWithTotalBytes)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();
    ObjectId newIdA("TestSchema.TestClass", "");
    ObjectId newIdB("TestSchema.TestClass", "");

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    auto instanceA = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    auto instanceB = txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    txn.GetCache().GetChangeManager().ModifyFile(instanceA, StubFile("12"), false);
    txn.GetCache().GetChangeManager().ModifyFile(instanceB, StubFile("3456"), false);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .Times(2)
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR, HttpRequest::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        progress(0, 2);
        progress(2, 2);
        return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "A"}));
        }))
        .WillOnce(Invoke([&] (JsonValueCR, BeFileNameCR, HttpRequest::ProgressCallbackCR progress, ICancellationTokenPtr)
            {
            progress(0, 4);
            progress(4, 4);
            return CreateCompletedAsyncTask(StubWSCreateObjectResult({"TestSchema.TestClass", "B"}));
            }));

        EXPECT_CALL(GetMockClient(), SendGetObjectRequest(_, _, _))
            .Times(2)
            .WillOnce(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "A"}))))
            .WillOnce(Return(CreateCompletedAsyncTask(StubWSObjectsResult({"TestSchema.TestClass", "B"}))));

        int onProgressCount = 0;
        double expectedBytesTransfered[7] = {0, 0, 2, 2, 2, 6, 6};

        auto onProgress = [&] (double, Utf8StringCR, double bytesTransfered, double bytesTotal)
            {
            EXPECT_EQ(expectedBytesTransfered[onProgressCount], bytesTransfered);
            onProgressCount++;
            EXPECT_EQ(6, bytesTotal);
            };

        ds->SyncLocalChanges(onProgress, nullptr)->Wait();
        EXPECT_EQ(7, onProgressCount);
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_ModifiedFiles_CallsProgressWithTotalBytes)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto instanceA = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    auto instanceB = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "B"});
    txn.GetCache().GetChangeManager().ModifyFile(instanceA, StubFile("12"), false);
    txn.GetCache().GetChangeManager().ModifyFile(instanceB, StubFile("3456"), false);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateFileRequest(_, _, _, _))
        .Times(2)
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR, HttpRequest::ProgressCallbackCR progress, ICancellationTokenPtr)
        {
        progress(0, 2);
        progress(2, 2);
        return CreateCompletedAsyncTask(WSUpdateFileResult::Success());
        }))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR, HttpRequest::ProgressCallbackCR progress, ICancellationTokenPtr)
            {
            progress(0, 4);
            progress(4, 4);
            return CreateCompletedAsyncTask(WSUpdateFileResult::Success());
            }));

        int onProgressCount = 0;
        double expectedBytesTransfered[7] = {0, 0, 2, 2, 2, 6, 6};

        auto onProgress = [&] (double, Utf8StringCR, double bytesTransfered, double bytesTotal)
            {
            EXPECT_EQ(expectedBytesTransfered[onProgressCount], bytesTransfered);
            onProgressCount++;
            EXPECT_EQ(6, bytesTotal);
            };

        ds->SyncLocalChanges(onProgress, nullptr)->Wait();
        EXPECT_EQ(7, onProgressCount);
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_NoObjectsPassedToSync_DoesNoRequestsAndReturnsSuccess)
    {
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto testClass = txn.GetCache().GetAdapter().GetECClass("TestSchema.TestClass");
    txn.GetCache().GetChangeManager().CreateObject(*testClass, Json::objectValue);
    txn.Commit();

    bset<ECInstanceKey> toSync;
    auto result = ds->SyncLocalChanges(toSync, nullptr, nullptr)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.GetValue().empty());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_ObjectIdForObjectChangePassed_SyncsFile)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

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

TEST_F(CachingDataSourceTests, SyncLocalChanges_ObjectIdForRelationshipChangePassed_SyncsFile)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto testRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto instanceA = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    auto instanceB = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "B"});
    auto relationship = txn.GetCache().GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceB);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendCreateObjectRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSCreateObjectResult::Error(StubWSConnectionError()))));

    bset<ECInstanceKey> toSync;
    toSync.insert(relationship);
    ds->SyncLocalChanges(toSync, nullptr, nullptr)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_ObjectIdForFileChangePassed_SyncsFile)
    {
    // Arrange
    auto ds = GetTestDataSourceV1();

    auto txn = ds->StartCacheTransaction();
    auto instance = StubInstanceInCache(txn.GetCache(), {"TestSchema.TestClass", "A"});
    txn.GetCache().GetChangeManager().ModifyFile(instance, StubFile("12"), false);
    txn.Commit();

    // Act & Assert
    EXPECT_CALL(GetMockClient(), SendUpdateFileRequest(_, _, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSUpdateFileResult::Error(StubWSConnectionError()))));

    bset<ECInstanceKey> toSync;
    toSync.insert(instance);
    ds->SyncLocalChanges(toSync, nullptr, nullptr)->Wait();
    }

TEST_F(CachingDataSourceTests, SyncCachedData_NoQueryProviders_DoesNothingAndReturns)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), bvector<IQueryProvider::Query>(), bvector<IQueryProviderPtr>(), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueriesSupplied_CachesInitialQueriesAndCallsGetQueriesWithDoUpdateFile)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    IQueryProvider::Query query(CachedResponseKey(StubECInstanceKey(11, 22), "Foo"), std::make_shared<WSQuery>("Schema", "Class"));

    EXPECT_CALL(*cache, ReadResponseCacheTag(query.key)).WillOnce(Return("TestTag"));
    EXPECT_CALL(*client, SendQueryRequest(*query.query, Utf8String("TestTag"), _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, CachePartialResponse(query.key, _, _, query.query.get(), _)).WillOnce(Return(SUCCESS));

    auto newInstanceKey = StubECInstanceKey(33, 44);
    auto responseKeys = StubECInstanceKeyMultiMap({newInstanceKey});
    EXPECT_CALL(*cache, ReadResponseInstanceKeys(query.key, _)).WillOnce(DoAll(SetArgReferee<1>(responseKeys), Return(CacheStatus::OK)));

    EXPECT_CALL(*provider, GetQueries(_, newInstanceKey, _)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, DoUpdateFile(_, newInstanceKey, _)).WillOnce(Return(false));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(query), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueriesWithSyncRecursivelyFalseSupplied_CachesInitialQueriesAndOnlyCallsDoUpdateFileFromProviders)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    IQueryProvider::Query query(CachedResponseKey(StubECInstanceKey(11, 22), "Foo"), std::make_shared<WSQuery>("Schema", "Class"), false);

    EXPECT_CALL(*cache, ReadResponseCacheTag(query.key)).WillOnce(Return("TestTag"));
    EXPECT_CALL(*client, SendQueryRequest(*query.query, Utf8String("TestTag"), _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, CachePartialResponse(query.key, _, _, query.query.get(), _)).WillOnce(Return(SUCCESS));

    auto newInstanceKey = StubECInstanceKey(33, 44);
    auto responseKeys = StubECInstanceKeyMultiMap({newInstanceKey});
    EXPECT_CALL(*cache, ReadResponseInstanceKeys(query.key, _)).WillOnce(DoAll(SetArgReferee<1>(responseKeys), Return(CacheStatus::OK)));
    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    EXPECT_CALL(*provider, GetQueries(_, _, _)).Times(0);
    EXPECT_CALL(*provider, DoUpdateFile(_, newInstanceKey, _)).Times(1);

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(query), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstancesSupplied_CachesInitialInstancesAndCallsGetQueriesWithDoUpdateFile)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    auto instanceKey = StubECInstanceKey(11, 22);
    ObjectId objectId("TestSchema.TestClass", "TestId");
    EXPECT_CALL(*cache, FindInstance(instanceKey)).WillRepeatedly(Return(objectId));

    EXPECT_CALL(*client, SendQueryRequest(_, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillOnce(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));

    EXPECT_CALL(*provider, GetQueries(_, instanceKey, _)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, DoUpdateFile(_, instanceKey, _)).WillOnce(Return(false));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto result = ds->SyncCachedData(StubBVector(instanceKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

TEST_F(CachingDataSourceTests, SyncCachedData_WSG1AndInitialInstancesSupplied_CachesIntialInstancesWithSeperateRequests)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto store = std::make_shared<StubRepositoryInfoStore>(StubWSInfoWebApi({1, 3}));
    auto ds = CreateMockedCachingDataSource(client, cache, store);

    auto instanceKeyA = StubECInstanceKey(11, 22);
    auto instanceKeyB = StubECInstanceKey(11, 33);
    ObjectId objectIdA("TestSchema.TestClass", "A");
    ObjectId objectIdB("TestSchema.TestClass", "B");
    EXPECT_CALL(*cache, FindInstance(instanceKeyA)).WillRepeatedly(Return(objectIdA));
    EXPECT_CALL(*cache, FindInstance(instanceKeyB)).WillRepeatedly(Return(objectIdB));
    EXPECT_CALL(*cache, FindInstance(objectIdA)).WillRepeatedly(Return(instanceKeyA));
    EXPECT_CALL(*cache, FindInstance(objectIdB)).WillRepeatedly(Return(instanceKeyB));

    EXPECT_CALL(*cache, ReadInstanceCacheTag(objectIdA)).WillOnce(Return("TagA"));
    EXPECT_CALL(*cache, ReadInstanceCacheTag(objectIdB)).WillOnce(Return("TagB"));
    EXPECT_CALL(*client, SendGetObjectRequest(objectIdA, Utf8String("TagA"), _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*client, SendGetObjectRequest(objectIdB, Utf8String("TagB"), _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstance(objectIdA, _)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*cache, UpdateInstance(objectIdB, _)).WillOnce(Return(SUCCESS));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto initialInstances = StubBVector({instanceKeyA, instanceKeyB});
    auto result = ds->SyncCachedData(initialInstances, bvector<IQueryProvider::Query>(), bvector<IQueryProviderPtr>(), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

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

    StubInstances queryInstances;
    queryInstances.Add(objectIdB);

    EXPECT_CALL(*client, SendQueryRequest(_, _, _)).WillOnce(Return(CreateCompletedAsyncTask(queryInstances.ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillOnce(DoAll(SetArgPointee<2>(StubBSet({instanceKeyB})), Return(SUCCESS)));

    StubInstances instanceC;
    instanceC.Add(objectIdC);
    EXPECT_CALL(*client, SendGetObjectRequest(objectIdC, _, _)).WillOnce(Return(CreateCompletedAsyncTask(instanceC.ToWSObjectsResult())));
    EXPECT_CALL(*client, SendGetObjectRequest(objectIdA, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError::Id::InstanceNotFound))));

    ON_CALL(*cache, UpdateInstance(objectIdC, _)).WillByDefault(Return(SUCCESS));
    EXPECT_CALL(*cache, RemoveInstance(objectIdA)).WillOnce(Return(CacheStatus::OK));

    ON_CALL(*cache, ReadInstanceLabel(_)).WillByDefault(Return(nullptr));
    ON_CALL(*cache, ReadInstanceCacheTag(_)).WillByDefault(Return(nullptr));
    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto initialInstances = StubBVector({instanceKeyA, instanceKeyB, instanceKeyC});
    auto result = ds->SyncCachedData(initialInstances, bvector<IQueryProvider::Query>(), bvector<IQueryProviderPtr>(), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), SizeIs(1));
    }

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

    EXPECT_CALL(*client, SendQueryRequest(_, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
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

TEST_F(CachingDataSourceTests, SyncCachedData_WSG1AndInitialInstancesReturnsNotFound_RemovesInstancesFromCache)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto store = std::make_shared<StubRepositoryInfoStore>(StubWSInfoWebApi({1, 3}));
    auto ds = CreateMockedCachingDataSource(client, cache, store);

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

    EXPECT_CALL(*client, SendGetObjectRequest(objectIdA, _, _)).WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError(WSError::Id::InstanceNotFound)))));
    EXPECT_CALL(*client, SendGetObjectRequest(objectIdB, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*client, SendGetObjectRequest(objectIdC, _, _)).WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError(WSError::Id::NotEnoughRights)))));

    EXPECT_CALL(*cache, UpdateInstance(objectIdB, _)).WillOnce(Return(SUCCESS));
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
    EXPECT_CALL(*client, SendQueryRequest(_, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillOnce(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));

    IQueryProvider::Query query(CachedResponseKey(instanceKey, "Foo"), std::make_shared<WSQuery>("Schema", "Class"));

    EXPECT_CALL(*provider, GetQueries(_, instanceKey, _)).WillOnce(Return(StubBVector(query)));
    EXPECT_CALL(*provider, DoUpdateFile(_, instanceKey, _)).WillOnce(Return(false));

    // Second query
    EXPECT_CALL(*cache, ReadResponseCacheTag(query.key)).WillOnce(Return(""));
    EXPECT_CALL(*client, SendQueryRequest(*query.query, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, CachePartialResponse(query.key, _, _, query.query.get(), _)).WillOnce(Return(SUCCESS));

    auto responseKeys = StubECInstanceKeyMultiMap({instanceKey});
    EXPECT_CALL(*cache, ReadResponseInstanceKeys(query.key, _)).WillOnce(DoAll(SetArgReferee<1>(responseKeys), Return(CacheStatus::OK)));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto result = ds->SyncCachedData(StubBVector(instanceKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

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

    EXPECT_CALL(*cache, ReadResponseCacheTag(query.key)).WillOnce(Return(nullptr));
    EXPECT_CALL(*client, SendQueryRequest(*query.query, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));

    EXPECT_CALL(*cache, CachePartialResponse(query.key, _, _, query.query.get(), _))
        .WillOnce(DoAll(SetArgReferee<2>(StubBSet({objectId})), Return(SUCCESS)));

    EXPECT_CALL(*cache, ReadResponseInstanceKeys(query.key, _)).WillOnce(Return(CacheStatus::OK));

    // Queries and caches rejected instances
    EXPECT_CALL(*client, SendQueryRequest(WSQuery(ObjectId("TestSchema.TestClass", "TestId")), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillOnce(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(query), bvector<IQueryProviderPtr>(), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

TEST_F(CachingDataSourceTests, SyncCachedData_QueryProviderReturnsToUpdateFile_DownloadsAndCachesFile)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    auto instanceKey = StubECInstanceKey(11, 22);
    ObjectId objectId("TestSchema.TestClass", "TestId");
    EXPECT_CALL(*cache, FindInstance(instanceKey)).WillRepeatedly(Return(objectId));
    EXPECT_CALL(*cache, FindInstance(objectId)).WillRepeatedly(Return(instanceKey));

    EXPECT_CALL(*client, SendQueryRequest(_, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillOnce(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));

    EXPECT_CALL(*provider, GetQueries(_, instanceKey, _)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, DoUpdateFile(_, instanceKey, _)).WillOnce(Return(true));

    // Download & cache file
    EXPECT_CALL(*cache, ReadFileCacheTag(objectId)).WillOnce(Return("TestTag"));
    EXPECT_CALL(*cache, ReadFileProperties(instanceKey, _, _)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*client, SendGetFileRequest(objectId, _, Utf8String("TestTag"), _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR fileName, Utf8StringCR, HttpRequest::ProgressCallbackCR, ICancellationTokenPtr)
        {
        SimpleWriteToFile("", fileName);
        return CreateCompletedAsyncTask(WSFileResult::Success(StubWSFileResponse(fileName, "")));
        }));
    EXPECT_CALL(*cache, CacheFile(objectId, _, _)).WillOnce(Return(SUCCESS));

    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    auto result = ds->SyncCachedData(StubBVector(instanceKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

TEST_F(CachingDataSourceTests, SyncCachedData_InstanceCachedAsPersistent_GetQueriesAndDoUpdateFileIsPersistentParameterIsTrue)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    IQueryProvider::Query query(CachedResponseKey(StubECInstanceKey(11, 22), "Foo"), std::make_shared<WSQuery>("Schema", "Class"));

    EXPECT_CALL(*cache, ReadResponseCacheTag(query.key)).WillOnce(Return("TestTag"));
    EXPECT_CALL(*client, SendQueryRequest(*query.query, Utf8String("TestTag"), _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, CachePartialResponse(query.key, _, _, query.query.get(), _)).WillOnce(Return(SUCCESS));

    auto newInstanceKey = StubECInstanceKey(33, 44);
    auto responseKeys = StubECInstanceKeyMultiMap({newInstanceKey});
    EXPECT_CALL(*cache, ReadResponseInstanceKeys(query.key, _)).WillOnce(DoAll(SetArgReferee<1>(responseKeys), Return(CacheStatus::OK)));

    // Return that all response instances are cached
    EXPECT_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillOnce(DoAll(SetArgReferee<0>(responseKeys), Return(SUCCESS)));

    EXPECT_CALL(*provider, GetQueries(_, newInstanceKey, true)).WillOnce(Return(bvector<IQueryProvider::Query>()));
    EXPECT_CALL(*provider, DoUpdateFile(_, newInstanceKey, true)).WillOnce(Return(false));

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector(query), StubBVector<IQueryProviderPtr>(provider), nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    }

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

TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueriesSuppliedAndServerErrorReturnedForFirstOne_ContinuesWithOtherQuerysAndReturnsFailedObject)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);

    IQueryProvider::Query a(CachedResponseKey(StubECInstanceKey(11, 22), "A"), std::make_shared<WSQuery>("SchemaA", "ClassA"));
    IQueryProvider::Query b(CachedResponseKey(StubECInstanceKey(11, 33), "B"), std::make_shared<WSQuery>("SchemaB", "ClassB"));
    IQueryProvider::Query c(CachedResponseKey(StubECInstanceKey(11, 44), "C"), std::make_shared<WSQuery>("SchemaC", "ClassC"));

    EXPECT_CALL(*cache, ReadResponseCacheTag(_)).WillRepeatedly(Return(""));
    EXPECT_CALL(*client, SendQueryRequest(*a.query, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*client, SendQueryRequest(*b.query, _, _)).WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(WSError::Id::ServerError))));
    EXPECT_CALL(*client, SendQueryRequest(*c.query, _, _)).WillOnce(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    EXPECT_CALL(*cache, CachePartialResponse(a.key, _, _, a.query.get(), _)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*cache, CachePartialResponse(c.key, _, _, c.query.get(), _)).WillOnce(Return(SUCCESS));

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

TEST_F(CachingDataSourceTests, SyncCachedData_InitialQueriesSuppliedAndConnectionError_StopsAndReturnsError)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);

    IQueryProvider::Query a(CachedResponseKey(StubECInstanceKey(11, 22), "A"), std::make_shared<WSQuery>("SchemaA", "ClassA"));
    IQueryProvider::Query b(CachedResponseKey(StubECInstanceKey(11, 33), "B"), std::make_shared<WSQuery>("SchemaB", "ClassB"));

    EXPECT_CALL(*cache, ReadResponseCacheTag(_)).WillRepeatedly(Return(""));
    EXPECT_CALL(*client, SendQueryRequest(*a.query, _, _)).WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Error(StubHttpResponse()))));
    EXPECT_CALL(*client, SendQueryRequest(*b.query, _, _)).Times(0);

    auto result = ds->SyncCachedData(bvector<ECInstanceKey>(), StubBVector({a, b}), bvector<IQueryProviderPtr>(), nullptr, nullptr)->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_THAT(result.GetError().GetWSError().GetStatus(), WSError::Status::ConnectionError);
    }

TEST_F(CachingDataSourceTests, SyncCachedData_InitialInstance_CallbackCalledWithZeroProgress)
    {
    auto cache = std::make_shared<NiceMock<MockDataSourceCache>>();
    auto client = std::make_shared<NiceMock<MockWSRepositoryClient>>();
    auto ds = CreateMockedCachingDataSource(client, cache);
    auto provider = std::make_shared<MockQueryProvider>();

    auto instanceKey = StubECInstanceKey(11, 22);
    ON_CALL(*cache, FindInstance(instanceKey)).WillByDefault(Return(ObjectId("TestSchema.TestClass", "TestId")));
    ON_CALL(*client, SendQueryRequest(_, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    ON_CALL(*cache, UpdateInstances(_, _, Not(nullptr), _)).WillByDefault(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));
    ON_CALL(*provider, GetQueries(_, instanceKey, _)).WillByDefault(Return(bvector<IQueryProvider::Query>()));
    ON_CALL(*provider, DoUpdateFile(_, instanceKey, _)).WillByDefault(Return(false));
    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    int progressCalled = 0;
    auto onProgress = [&] (double bytesTransfered, double bytesTotal)
        {
        EXPECT_THAT(progressCalled, 0);
        EXPECT_THAT(bytesTransfered, 0);
        EXPECT_THAT(bytesTotal, 0);
        progressCalled++;
        };

    auto result = ds->SyncCachedData(StubBVector(instanceKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), onProgress, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    EXPECT_THAT(progressCalled, 1);
    }

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
    ON_CALL(*client, SendQueryRequest(_, _, _)).WillByDefault(Return(CreateCompletedAsyncTask(StubInstances().ToWSObjectsResult())));
    ON_CALL(*cache, UpdateInstances(_, _, _, _)).WillByDefault(DoAll(SetArgPointee<2>(StubBSet({instanceKey})), Return(SUCCESS)));
    ON_CALL(*provider, GetQueries(_, instanceKey, _)).WillByDefault(Return(bvector<IQueryProvider::Query>()));
    ON_CALL(*provider, DoUpdateFile(_, instanceKey, _)).WillByDefault(Return(true));

    // Download & cache file
    EXPECT_CALL(*cache, ReadFileCacheTag(objectId)).WillOnce(Return(nullptr));
    EXPECT_CALL(*cache, ReadFileProperties(instanceKey, _, _)).WillOnce(DoAll(SetArgReferee<2>(42), Return(SUCCESS)));
    EXPECT_CALL(*client, SendGetFileRequest(objectId, _, _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR fileName, Utf8StringCR, HttpRequest::ProgressCallbackCR onProgress, ICancellationTokenPtr)
        {
        onProgress(5, 42);
        SimpleWriteToFile("", fileName);
        return CreateCompletedAsyncTask(WSFileResult::Success(StubWSFileResponse(fileName, "")));
        }));

    ON_CALL(*cache, CacheFile(objectId, _, _)).WillByDefault(Return(SUCCESS));
    ON_CALL(*cache, ReadFullyPersistedInstanceKeys(_)).WillByDefault(Return(SUCCESS));

    int progressCalled = 0;
    auto onProgress = [&] (double bytesTransfered, double bytesTotal)
        {
        if (progressCalled == 1)
            {
            EXPECT_THAT(bytesTransfered, 5);
            EXPECT_THAT(bytesTotal, 42);
            }
        progressCalled++;
        };

    auto result = ds->SyncCachedData(StubBVector(instanceKey), bvector<IQueryProvider::Query>(), StubBVector<IQueryProviderPtr>(provider), onProgress, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_THAT(result.GetValue(), IsEmpty());
    EXPECT_THAT(progressCalled, 2);
    }

#endif // USE_GTEST
