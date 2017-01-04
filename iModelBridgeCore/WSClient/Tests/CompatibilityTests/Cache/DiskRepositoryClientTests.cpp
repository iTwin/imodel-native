/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Cache/DiskRepositoryClientTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

//#define COMPILE_TESTS
#ifdef DEBUG
#define COMPILE_TESTS
#endif

#ifdef COMPILE_TESTS

#include "DiskRepositoryClient.h"
#include <WebServices/Cache/CachingDataSource.h>
#include <Bentley/BeTest.h>

using namespace std;
using namespace testing;

BeFileName s_testFolder;

struct DiskRepositoryClientTests : Test 
    {
    void SetUp()
        {
        s_testFolder.clear();
        BeTest::GetHost().GetOutputRoot(s_testFolder);
        s_testFolder.AppendToPath(L"DiskRepositoryClientTests");
        if (s_testFolder.DoesPathExist())
            EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(s_testFolder));
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(s_testFolder));
        }
    };

TEST_F(DiskRepositoryClientTests, GetSchemas_SchemaAndOtherFilesInFolder_CacheCreatationSucceeds)
    {
    Utf8String schemaXml =
        R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">            
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="TestProperty" typeName="string" />
            </ECClass>
        </ECSchema>)xml";

    BeFileName path = s_testFolder;
    path.AppendToPath(L"TestSchema.01.00.ecschema.xml");
    BeFile file;
    file.Create(path);
    EXPECT_EQ(BeFileStatus::Success, file.Write(nullptr, schemaXml.c_str(), (uint32_t) schemaXml.size()));
    file.Close();

    path = path.GetDirectoryName();
    auto client = std::make_shared<DiskRepositoryClient>(path);

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

#endif