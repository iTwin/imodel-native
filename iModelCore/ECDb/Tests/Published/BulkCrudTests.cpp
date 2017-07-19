/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BulkCrudTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BulkCrudTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkBisDomainCrudTestFixture, Test)
    {
    BeFileName bisSchemaFolder;
    BeFileName domainSchemaFolder = GetDomainSchemaFolder(bisSchemaFolder);
    if (!domainSchemaFolder.DoesPathExist())
        return; 

    ASSERT_EQ(SUCCESS, SetupDomainBimFile("bulkcrud_domainschemas.ecdb", domainSchemaFolder, bisSchemaFolder));

    EXPECT_EQ(ExpectedColumn("bis_InformationReferenceElement", "js1"), GetHelper().GetPropertyMapColumn(AccessString("bis", "UrlLink", "Url")));
    EXPECT_EQ(ExpectedColumn("bis_InformationReferenceElement", "js1"), GetHelper().GetPropertyMapColumn(AccessString("bis", "EmbeddedFileLink", "Name")));

    {
    //Search for non-TPH classes
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM meta.ECClassDef WHERE Type IN (0,1) AND Modifier IN(0,1) ORDER BY Schema.Id, Name"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECClassId classId = stmt.GetValueId<ECClassId>(0);
        ASSERT_TRUE(classId.IsValid());
        ECClassCP ecClass = m_ecdb.Schemas().GetClass(classId);
        ASSERT_TRUE(ecClass != nullptr);

        MapStrategyInfo mapStrategy = GetHelper().GetMapStrategy(classId);
        ASSERT_TRUE(mapStrategy.Exists());

        if (mapStrategy.GetStrategy() != MapStrategy::OwnTable)
            continue;

        Utf8CP modifierStr = "-";
        switch (ecClass->GetClassModifier())
            {
                case ECClassModifier::Abstract:
                    modifierStr = ecClass->GetEntityClassCP()->IsMixin() ? "Mixin" : "Abstract";
                    break;

                case ECClassModifier::Sealed:
                    modifierStr = "Sealed";
                    break;

                default:
                    break;
            }

        printf("ECClass: %s - Modifier: %s - Base classes: %d (first base class: %s) \r\n", ecClass->GetFullName(), modifierStr,
               (int) ecClass->GetBaseClasses().size(), ecClass->HasBaseClasses() ? ecClass->GetBaseClasses()[0]->GetFullName() : "-");
        }
    }
    /*   TestDataset testDataset;
    ASSERT_EQ(SUCCESS, testDataset.Populate(m_ecdb)) << domainSchemaFolder.GetNameUtf8().c_str();

    AssertInsert(testDataset);
    */
    }

END_ECDBUNITTESTS_NAMESPACE
