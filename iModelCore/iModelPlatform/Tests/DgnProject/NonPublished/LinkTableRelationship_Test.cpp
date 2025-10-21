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
{};

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
TEST_F(LinkTableRelationshipTests, TestBisCore17WithUpdatedSystemIndex)
    {
    SetupSeedProject();
    const auto bisCoreSchema = m_db->Schemas().GetSchema("BisCore");
    ASSERT_GE(bisCoreSchema->GetVersionRead(), 1U);
    ASSERT_GE(bisCoreSchema->GetVersionWrite(), 0U);

    if (bisCoreSchema->GetVersionMinor() < 17U)
        {
        ECSchemaPtr schema = nullptr;
        auto context = ECSchemaReadContext::CreateContext();
        context->AddSchemaLocater(m_db->GetSchemaLocater());
        auto bisCoreSchemaPath = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
        bisCoreSchemaPath.AppendToPath(L"ECSchemas\\BisCoreDummy.01.00.17.ecschema.xml");
        EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlFile(schema, bisCoreSchemaPath.GetName(), *context));
        ASSERT_EQ(SchemaStatus::Success, m_db->ImportSchemas(context->GetCache().GetSchemas(), true));
        }

    auto relationshipClass = m_db->Schemas().GetClass(BIS_ECSCHEMA_NAME, "CategorySelectorRefersToCategories")->GetRelationshipClassCP();
    ASSERT_NE(nullptr, relationshipClass);

    CategorySelectorPtr categorySelector = new CategorySelector(m_db->GetDictionaryModel(), "TestCategorySelector");
    ASSERT_TRUE(categorySelector.IsValid());
    ASSERT_TRUE(categorySelector->Insert().IsValid());

    SpatialCategory category(m_db->GetDictionaryModel(), "TestCategory", DgnCategory::Rank::Application);
    auto spatialCategory = category.Insert(DgnSubCategory::Appearance());
    EXPECT_TRUE(spatialCategory.IsValid());

    // Try to insert duplicate link table relationships with different MemberPriority values
    // Should succeed
    auto relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relationshipClass);
    ASSERT_NE(nullptr, relationshipEnabler);

    const bvector<int> expectedMemberPriorityValues = { 0, 1, 2, 3 };
    ECInstanceKey relationshipInstanceKeys[5];
    for (const auto& memberPriorityValue : expectedMemberPriorityValues)
        {
        IECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance();
        ASSERT_NE(nullptr, relationshipInstance);

        ECValue value;
        value.SetInteger(memberPriorityValue);
        relationshipInstance->SetValue("MemberPriority", value);

        ASSERT_EQ(BE_SQLITE_OK, m_db->InsertLinkTableRelationship(relationshipInstanceKeys[memberPriorityValue], *relationshipClass, ECInstanceId(categorySelector->GetElementId().GetValue()), 
            ECInstanceId(spatialCategory->GetCategoryId().GetValue()), relationshipInstance.get())) << "Failed when inserting element with member priority value " << memberPriorityValue << "\n";
        }

    // Check if the all valid relationships were inserted correctly
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_db, "SELECT * FROM " BIS_SCHEMA("CategorySelectorRefersToCategories") " ORDER BY ECInstanceId"));
    auto rowCount = 0;
    
    while (BE_SQLITE_ROW == statement.Step())
        {
        EXPECT_EQ(statement.GetValueId<ECInstanceId>(0), relationshipInstanceKeys[rowCount].GetInstanceId());
        EXPECT_EQ(statement.GetValueId<ECClassId>(1), relationshipInstanceKeys[rowCount].GetClassId());
        EXPECT_EQ(statement.GetValueInt(2), expectedMemberPriorityValues[rowCount]);
        EXPECT_EQ(statement.GetValueId<DgnElementId>(3), categorySelector->GetElementId());
        EXPECT_EQ(statement.GetValueId<DgnClassId>(4), categorySelector->GetElementClassId());
        EXPECT_EQ(statement.GetValueId<DgnElementId>(5), spatialCategory->GetCategoryId());
        EXPECT_EQ(statement.GetValueId<DgnElementId>(6), spatialCategory->GetElementClassId());
        ++rowCount;
        }
    EXPECT_EQ(rowCount, expectedMemberPriorityValues.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(LinkTableRelationshipTests, DeleteLinkTableRelationships)
    {
    SetupSeedProject();
    ASSERT_EQ(BentleyStatus::SUCCESS, m_db->Schemas().CreateClassViewsInDb());

    const auto relationshipClass = m_db->Schemas().GetClass(BIS_ECSCHEMA_NAME, "CategorySelectorRefersToCategories")->GetRelationshipClassCP();
    ASSERT_NE(relationshipClass, nullptr);

    CategorySelectorPtr categorySelector = new CategorySelector(m_db->GetDictionaryModel(), "TestCategorySelector");
    ASSERT_TRUE(categorySelector.IsValid());
    ASSERT_TRUE(categorySelector->Insert().IsValid());

    // Create multiple categories and relationships
    const auto numRelationships = 8;
    DgnCategoryId categoryIds[numRelationships];
    ECInstanceKey instanceKeys[numRelationships];

    for (int i = 0; i < numRelationships; i++)
        {
        Utf8PrintfString categoryName("TestCategory%d", i);
        categoryIds[i] = DgnDbTestUtils::InsertSpatialCategory(*m_db, categoryName.c_str());
        ASSERT_TRUE(categoryIds[i].IsValid());
        
        ASSERT_EQ(BE_SQLITE_OK, m_db->InsertLinkTableRelationship(instanceKeys[i], *relationshipClass, categorySelector->GetElementId(), categoryIds[i]));
        ASSERT_TRUE(instanceKeys[i].IsValid());
        }

    // Verify all relationships were inserted
    ASSERT_EQ(numRelationships, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA("CategorySelectorRefersToCategories")));

    // Delete the odd indexed relationships
    DgnElementIdSet relationshipIdsToDelete;
    relationshipIdsToDelete.insert(DgnElementId(instanceKeys[1].GetInstanceId().GetValue()));
    relationshipIdsToDelete.insert(DgnElementId(instanceKeys[3].GetInstanceId().GetValue())); 
    relationshipIdsToDelete.insert(DgnElementId(instanceKeys[5].GetInstanceId().GetValue()));
    relationshipIdsToDelete.insert(DgnElementId(instanceKeys[7].GetInstanceId().GetValue()));

    ASSERT_EQ(BE_SQLITE_DONE, m_db->DeleteLinkTableRelationships(BIS_SCHEMA("CategorySelectorRefersToCategories"), relationshipIdsToDelete));

    // Verify that correct relationships remain
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_db, "SELECT ECInstanceId FROM " BIS_SCHEMA("CategorySelectorRefersToCategories") " ORDER BY ECInstanceId"));
    
    bvector<ECInstanceId> remainingIds;
    while (BE_SQLITE_ROW == statement.Step())
        remainingIds.push_back(statement.GetValueId<ECInstanceId>(0));
    
    ASSERT_EQ(4, remainingIds.size());
    EXPECT_EQ(instanceKeys[0].GetInstanceId(), remainingIds[0]);
    EXPECT_EQ(instanceKeys[2].GetInstanceId(), remainingIds[1]);
    EXPECT_EQ(instanceKeys[4].GetInstanceId(), remainingIds[2]);
    EXPECT_EQ(instanceKeys[6].GetInstanceId(), remainingIds[3]);

    DgnElementIdSet remainingRelationshipIds;
    for (const auto& id : remainingIds)
        remainingRelationshipIds.insert(DgnElementId(id.GetValue()));

    ASSERT_EQ(BE_SQLITE_DONE, m_db->DeleteLinkTableRelationships(BIS_SCHEMA("CategorySelectorRefersToCategories"), remainingRelationshipIds));

    // Verify all relationships are deleted
    ASSERT_EQ(0, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA("CategorySelectorRefersToCategories")));
    }
