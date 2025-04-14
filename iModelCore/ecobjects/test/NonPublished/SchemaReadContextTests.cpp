/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaReadContextTest : ECTestFixture 
    {
    BeFileName GenerateOutputDirectory(WCharCP directoryName)
        {
        BeFileName testData;
        BeTest::GetHost().GetOutputRoot (testData);
        BeFileName folder = testData.AppendToPath (directoryName);
        BeFileNameStatus status;
        if(folder.DoesPathExist(folder))
            status = BeFileName::EmptyDirectory(folder);
        else
            status = BeFileName::CreateNewDirectory(folder);

        EXPECT_EQ(BeFileNameStatus::Success, status);
        return folder;

        }

    void GenerateSchemaFile(BeFileName directory, SchemaKey key, Utf8String alias, bool appendVersionSuffix = true)
        {
        ECSchemaPtr schema;
        ECObjectsStatus status = ECSchema::CreateSchema(schema, key.GetName(), alias, key.GetVersionRead(), key.GetVersionWrite(), key.GetVersionMinor(), ECVersion::Latest);
        ASSERT_EQ(ECObjectsStatus::Success, status);
        ASSERT_TRUE(schema.IsValid());

        WString fileName;
        fileName.AssignUtf8(appendVersionSuffix ? key.GetFullSchemaName().c_str() : key.GetName().c_str());
        fileName.append(L".ecschema.xml");
        BeFileName fullPath = directory.AppendToPath(fileName.c_str());
        
        SchemaWriteStatus writeStatus = schema->WriteToXmlFile(fullPath);
        ASSERT_EQ(SchemaWriteStatus::Success, writeStatus);
        ASSERT_TRUE(fullPath.DoesPathExist(fullPath));
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaReadContextTest, AddSchemaPathPriority)
    {
    const Utf8String alias("ts");
    auto dir1 = GenerateOutputDirectory(L"AddSchemaPathPriority1");
    SchemaKey key1("TestSchema", 1, 0, 0);
    GenerateSchemaFile(dir1, key1, alias, false);

    auto dir2 = GenerateOutputDirectory(L"AddSchemaPathPriority2");
    SchemaKey key2("TestSchema", 1, 2, 0);
    GenerateSchemaFile(dir2, key2, alias, true);

    auto dir3 = GenerateOutputDirectory(L"AddSchemaPathPriority3");
    SchemaKey key3("TestSchema", 2, 0, 0);
    GenerateSchemaFile(dir3, key3, alias, true);

    { // Schema1 is on top
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false, true);
        schemaContext->AddSchemaPath(dir1);
        schemaContext->AddSchemaPath(dir2);
        schemaContext->AddSchemaPath(dir3);
        ECSchemaPtr schema = schemaContext->LocateSchema(key1, SchemaMatchType::LatestReadCompatible);
        ASSERT_TRUE(schema.IsValid());
        EXPECT_TRUE(key1.Matches(schema->GetSchemaKey(), SchemaMatchType::Exact));
    }

    { // Schema 2 is on top
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false, true);
        schemaContext->AddSchemaPath(dir2);
        schemaContext->AddSchemaPath(dir1);
        schemaContext->AddSchemaPath(dir3);
        ECSchemaPtr schema = schemaContext->LocateSchema(key1, SchemaMatchType::LatestReadCompatible);
        ASSERT_TRUE(schema.IsValid());
        EXPECT_TRUE(key2.Matches(schema->GetSchemaKey(), SchemaMatchType::Exact));
    }

    { // Schema 3 is on, but not read compatible. Using version 1.0.1 to locate, so Schema 1 also does not match.
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false, true);
        schemaContext->AddSchemaPath(dir3);
        schemaContext->AddSchemaPath(dir1);
        schemaContext->AddSchemaPath(dir2);
        SchemaKey desiredKey("TestSchema", 1, 0, 1);
        ECSchemaPtr schema = schemaContext->LocateSchema(desiredKey, SchemaMatchType::LatestReadCompatible);
        ASSERT_TRUE(schema.IsValid());
        EXPECT_TRUE(key2.Matches(schema->GetSchemaKey(), SchemaMatchType::Exact));
    }

    { // Put the last added schema path on top
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false, true);
        schemaContext->AddSchemaPath(dir3);
        schemaContext->AddSchemaPath(dir1);
        schemaContext->AddSchemaPath(dir2, true);
        ECSchemaPtr schema = schemaContext->LocateSchema(key1, SchemaMatchType::Latest);
        ASSERT_TRUE(schema.IsValid());
        EXPECT_TRUE(key2.Matches(schema->GetSchemaKey(), SchemaMatchType::Exact));
    }
    }

END_BENTLEY_ECN_TEST_NAMESPACE
