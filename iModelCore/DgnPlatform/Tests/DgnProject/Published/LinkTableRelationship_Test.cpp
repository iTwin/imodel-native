/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/LinkTableRelationship_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_EC;

//========================================================================================
// @bsiclass                                                    Shaun.Sewall    07/2017
//========================================================================================
struct LinkTableRelationshipTests : public DgnDbTestFixture
{
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2017
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
