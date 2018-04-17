/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Disk/DiskRepositoryClientTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#define COMPILE_TESTS
#ifdef COMPILE_TESTS

#include "DiskRepositoryClient.h"
#include <WebServices/Cache/CachingDataSource.h>
#include <Bentley/BeTest.h>

using namespace std;
using namespace testing;

BeFileName s_testFolder;
BeFileName s_testTempFile;

Utf8String s_schemaXml1 =
R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">            
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="TestProperty" typeName="string" />
            </ECClass>
        </ECSchema>)xml";

Utf8String s_schemaXml2 =
R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">            
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="TestProperty" typeName="string" />
                <ECProperty propertyName="TestProperty2" typeName="string" />
            </ECClass>
        </ECSchema>)xml";

void SimpleWriteToFile(BeFileNameCR path, Utf8StringCR content)
    {
    BeFile file;
    file.Create(path);
    EXPECT_EQ(BeFileStatus::Success, file.Write(nullptr, content.c_str(), (uint32_t) content.size()));
    file.Close();
    }

struct DiskRepositoryClientTests : Test
    {
    void SetUp()
        {
        s_testFolder.clear();
        s_testTempFile.clear();

        BeTest::GetHost().GetOutputRoot(s_testFolder);
        BeTest::GetHost().GetOutputRoot(s_testTempFile);

        s_testFolder.AppendToPath(L"DiskRepositoryClientTests");
        if (s_testFolder.DoesPathExist())
            EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(s_testFolder));
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(s_testFolder));

        s_testTempFile.AppendToPath(L"TempFile");
        if (s_testTempFile.DoesPathExist())
            EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(s_testTempFile));
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskRepositoryClientTests, GetSchemas_SchemaAndOtherFilesInFolder_CacheCreationSucceeds)
    {
    BeFileName path = s_testFolder;
    path.AppendToPath(L"TestSchema.01.00.ecschema.xml");
    SimpleWriteToFile(path, s_schemaXml1);

    auto client = std::make_shared<DiskRepositoryClient>(s_testFolder);

    CacheEnvironment env;
    BeTest::GetHost().GetOutputRoot(env.temporaryFileCacheDir);
    BeTest::GetHost().GetOutputRoot(env.persistentFileCacheDir);
    BeFileName cachePath = env.persistentFileCacheDir;
    cachePath.AppendToPath(L"TestCache.ecdb");

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, env)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    result.GetValue()->GetCacheAccessThread()->ExecuteAsync([&]
        {
        EXPECT_NE(nullptr, result.GetValue()->StartCacheTransaction().GetCache().GetAdapter().GetECClass("TestSchema.TestClass"));
        })->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskRepositoryClientTests, SendGetFileRequest_SchemaFileNotChanged_SameEtags)
    {
    BeFileName path = s_testFolder;
    path.AppendToPath(L"TestSchema.01.00.ecschema.xml");
    SimpleWriteToFile(path, s_schemaXml1);
    auto client = std::make_shared<DiskRepositoryClient>(s_testFolder);
    auto schemas = client->SendGetSchemasRequest()->GetResult().GetValue();
    EXPECT_EQ(1, schemas.GetInstances().Size());
    auto schemaId = (*schemas.GetInstances().begin()).GetObjectId();

    auto fileResponse1 = client->SendGetFileRequest(schemaId, s_testTempFile)->GetResult().GetValue();
    EXPECT_TRUE(fileResponse1.IsModified());
    EXPECT_EQ(s_testTempFile, fileResponse1.GetFilePath());

    auto fileResponse2 = client->SendGetFileRequest(schemaId, s_testTempFile)->GetResult().GetValue();
    EXPECT_TRUE(fileResponse2.IsModified());
    EXPECT_EQ(s_testTempFile, fileResponse2.GetFilePath());
    EXPECT_EQ(fileResponse1.GetETag(), fileResponse2.GetETag());

    auto fileResponse3 = client->SendGetFileRequest(schemaId, s_testTempFile, fileResponse2.GetETag())->GetResult().GetValue();
    EXPECT_FALSE(fileResponse3.IsModified());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskRepositoryClientTests, SendGetFileRequest_SchemaFileChanged_DifferentEtags)
    {
    BeFileName path = s_testFolder;
    path.AppendToPath(L"TestSchema.01.00.ecschema.xml");
    SimpleWriteToFile(path, s_schemaXml1);
    auto client = std::make_shared<DiskRepositoryClient>(s_testFolder);
    auto schemas = client->SendGetSchemasRequest()->GetResult().GetValue();
    EXPECT_EQ(1, schemas.GetInstances().Size());
    auto schemaId = (*schemas.GetInstances().begin()).GetObjectId();

    auto fileResponse1 = client->SendGetFileRequest(schemaId, s_testTempFile)->GetResult().GetValue();
    EXPECT_TRUE(fileResponse1.IsModified());
    EXPECT_EQ(s_testTempFile, fileResponse1.GetFilePath());

    SimpleWriteToFile(path, s_schemaXml2);

    auto fileResponse2 = client->SendGetFileRequest(schemaId, s_testTempFile)->GetResult().GetValue();
    EXPECT_TRUE(fileResponse2.IsModified());
    EXPECT_EQ(s_testTempFile, fileResponse2.GetFilePath());
    EXPECT_NE(fileResponse1.GetETag(), fileResponse2.GetETag());

    auto fileResponse3 = client->SendGetFileRequest(schemaId, s_testTempFile, fileResponse1.GetETag())->GetResult().GetValue();
    EXPECT_TRUE(fileResponse3.IsModified());
    EXPECT_EQ(s_testTempFile, fileResponse3.GetFilePath());
    EXPECT_NE(fileResponse1.GetETag(), fileResponse3.GetETag());
    }

#endif
