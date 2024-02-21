/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_EC;

//========================================================================================
// @bsiclass
//========================================================================================
struct LinkTableRelationshipTests : public DgnDbTestFixture
{
    struct TestIssueListener : ECN::IIssueListener
    {
    mutable bvector<Utf8String> m_issues;

    void _OnIssueReported(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, ECN::IssueId id, Utf8CP message) const override
        {
        m_issues.push_back(message);
        }
    };
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(LinkTableRelationshipTests, CRUD)
    {
    SetupSeedProject();
    ASSERT_EQ(BentleyStatus::SUCCESS, m_db->Schemas().CreateClassViewsInDb());

    ECRelationshipClassCP relationshipClass = m_db->Schemas().GetClass(BIS_ECSCHEMA_NAME, "CategorySelectorRefersToCategories")->GetRelationshipClassCP();
    ASSERT_NE(relationshipClass, nullptr);

    CategorySelectorPtr categorySelector = new CategorySelector(m_db->GetDictionaryModel(), "MyCategorySelector");
    ASSERT_TRUE(categorySelector.IsValid());
    ASSERT_TRUE(categorySelector->Insert().IsValid());

    DgnCategoryId categoryId[5];
    ECInstanceKey instanceKey[5];

    for (int i=0; i<_countof(categoryId); i++)
        {
        Utf8PrintfString categoryName("MyCategory%d", i);
        categoryId[i] = DgnDbTestUtils::InsertSpatialCategory(*m_db, categoryName.c_str());
        ASSERT_EQ(BE_SQLITE_OK, m_db->InsertLinkTableRelationship(instanceKey[i], *relationshipClass, categorySelector->GetElementId(), categoryId[i]));
        }
    ASSERT_EQ(_countof(categoryId), DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA("CategorySelectorRefersToCategories")));

    int count = 0;
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_db, "SELECT ECInstanceId,SourceECInstanceId,TargetECInstanceId FROM " BIS_SCHEMA("CategorySelectorRefersToCategories") " ORDER BY ECInstanceId"));
    while (BE_SQLITE_ROW == statement.Step())
        {
        ASSERT_EQ(statement.GetValueId<ECInstanceId>(0), instanceKey[count].GetInstanceId());
        ASSERT_EQ(statement.GetValueId<DgnElementId>(1), categorySelector->GetElementId());
        ASSERT_EQ(statement.GetValueId<DgnCategoryId>(2), categoryId[count]);
        count++;
        }
    ASSERT_EQ(_countof(categoryId), count);

    ASSERT_EQ(BE_SQLITE_OK, m_db->DeleteLinkTableRelationships(BIS_SCHEMA("CategorySelectorRefersToCategories"), DgnElementId(), categoryId[3]));
    ASSERT_EQ(BE_SQLITE_OK, m_db->DeleteLinkTableRelationships(BIS_SCHEMA("CategorySelectorRefersToCategories"), DgnElementId(), categoryId[1]));
    ASSERT_EQ(_countof(categoryId)-2, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA("CategorySelectorRefersToCategories")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(LinkTableRelationshipTests, BisCore17ImportShouldFail)
    {
    SetupSeedProject();
    const auto bisCoreSchema = m_db->Schemas().GetSchema("BisCore");
    ASSERT_EQ(bisCoreSchema->GetVersionRead(), 1U);
    ASSERT_EQ(bisCoreSchema->GetVersionWrite(), 0U);
    ASSERT_LT(bisCoreSchema->GetVersionMinor(), 17U);

    // Import new dummy BisCore which has the new DbIndex added to ElementRefersToElements
    ECSchemaPtr schema = nullptr;
    auto context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(m_db->GetSchemaLocater());
    auto bisCoreSchemaPath = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    bisCoreSchemaPath.AppendToPath(L"ECSchemas\\BisCoreDummy.01.00.17.ecschema.xml");
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlFile(schema, bisCoreSchemaPath.GetName(), *context));

    TestIssueListener issueListener;
    m_db->AddIssueListener(issueListener);

    ASSERT_EQ(SchemaStatus::SchemaImportFailed, m_db->ImportSchemas(context->GetCache().GetSchemas(), true));

    EXPECT_FALSE(issueListener.m_issues.empty());
    EXPECT_STREQ(issueListener.m_issues.back().c_str(), Utf8PrintfString("ECSchema BisCore.01.00.17 requires ECDb version 4.0.0.5, but the current runtime version is only %s.", m_db->GetECDbProfileVersion().ToString().c_str()).c_str());
    }
