/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbSchemaUpgrade.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

TEST_F(ECDbTestFixture, SchemaUpgrade)
    {
    ECDbTestFixture::Initialize();

    ECDb ecdb;
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"ecdb_ecschema_upgrade.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);
        {
        ECSchemaPtr test_1_0_0;
        ECSchema::CreateSchema(test_1_0_0, "test", "t", 1, 0, 0);
        test_1_0_0->SetDisplayLabel("Test ECSchema");
        test_1_0_0->SetDescription("This is a test schema");

        ECEntityClassP test_foo;
        test_1_0_0->CreateEntityClass(test_foo, "Book");
        test_foo->SetDisplayLabel("Book");
        test_foo->SetDescription("Represent a generic book");
        test_foo->SetClassModifier(ECClassModifier::Abstract);

        ECSchemaCachePtr cache1 = ECSchemaCache::Create();
        cache1->AddSchema(*test_1_0_0);

        ASSERT_TRUE(ecdb.Schemas().ImportECSchemas(*cache1) == SUCCESS);
        }
        //Upgrade with some changes
        {
        ECSchemaPtr test_1_0_0;
        ECSchema::CreateSchema(test_1_0_0, "test", "t", 1, 1, 1);
        test_1_0_0->SetDisplayLabel("Test ECSchema (Modified)");
        test_1_0_0->SetDescription("This is a test schema  (Modified)");

        ECEntityClassP test_foo;
        test_1_0_0->CreateEntityClass(test_foo, "Book");
        test_foo->SetDisplayLabel("Book");
        test_foo->SetDescription("Represent a generic book");
        test_foo->SetClassModifier(ECClassModifier::Abstract);

        ECSchemaCachePtr cache1 = ECSchemaCache::Create();
        cache1->AddSchema(*test_1_0_0);

        ASSERT_TRUE(ecdb.Schemas().ImportECSchemas(*cache1) == SUCCESS);
        }

        ECSchemaCP test = ecdb.Schemas().GetECSchema("test");
        ASSERT_TRUE(test != nullptr);
        ASSERT_TRUE(test->GetDisplayLabel() == "Test ECSchema (Modified)");
        ASSERT_TRUE(test->GetDescription() == "This is a test schema  (Modified)");
    }

END_ECDBUNITTESTS_NAMESPACE
