/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ElementGroup_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Element Group 
* @bsimethod                                                    Umar.Hayat      09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementGroupTests : public DgnDbTestFixture
{
    PhysicalElementPtr CreateAndInsertElement(DgnModelP model);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalElementPtr ElementGroupTests::CreateAndInsertElement(DgnModelP model)
    {
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));
    PhysicalElementPtr testElement = PhysicalElement::Create(*(model->ToPhysicalModelP()), m_defaultCategoryId);

    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,/**/  0, 0, 2, /**/ 0, 3, 0, /**/ 0.0, Angle::TwoPi());
    ICurvePrimitivePtr curvePrimitive = ICurvePrimitive::CreateArc(ellipseData);
    builder->Append(*curvePrimitive);
    BentleyStatus status = builder->SetGeomStreamAndPlacement(*testElement);
    if (SUCCESS != status)
        return NULL;

    DgnDbStatus statusInsert;
    auto member = testElement->Insert(&statusInsert);
    if (DgnDbStatus::Success != statusInsert)
        return NULL;

    return testElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGroupTests, CRUD)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementGroupTests_CRUD.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    auto member1 = CreateAndInsertElement(model);
    ASSERT_TRUE(member1.IsValid());
    auto member2 = CreateAndInsertElement(model);
    ASSERT_TRUE(member2.IsValid());
    auto member3 = CreateAndInsertElement(model);
    ASSERT_TRUE(member3.IsValid());

    // Create Element Group
    TestGroupPtr group = TestGroup::Create(*m_db, m_defaultModelId, m_defaultCategoryId);
    ASSERT_TRUE(group.IsValid());
    ASSERT_TRUE(group->Insert().IsValid());

    // Insert Elements
    ASSERT_TRUE(DgnDbStatus::Success == group->AddMember(*member1));
    ASSERT_TRUE(DgnDbStatus::Success == group->AddMember(*member2));
    ASSERT_TRUE(DgnDbStatus::Success == group->AddMember(*member3));
    // Insert Duplicate
    ASSERT_FALSE(DgnDbStatus::Success == group->AddMember(*member3));

    //  Query
    EXPECT_TRUE (3 == group->QueryMembers().size());
    
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementGroupsMembers)));
    int relationshipCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ++relationshipCount;
        }
    EXPECT_TRUE(3 == relationshipCount);
    m_db->SaveChanges();

    DgnElementIdSet allMembers = group->QueryMembers();
    for (DgnElementId id : allMembers)
        {
        if (id == member1->GetElementId() || id == member2->GetElementId() || id == member3->GetElementId())
            continue;
        else
            EXPECT_TRUE(false) << "This element id should not be here ";
        }
    
    // Delete Members
    EXPECT_TRUE(DgnDbStatus::Success == group->RemoveMember(*member2));
    EXPECT_TRUE(2 == group->QueryMembers().size());
        // Check only relationship is deleted , not the element
        
    // Already deleted
    EXPECT_TRUE(DgnDbStatus::Success == group->RemoveMember(*member2));
    EXPECT_TRUE(2 == group->QueryMembers().size());

    EXPECT_TRUE(m_db->Elements().GetElement(member1->GetElementId()).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(member2->GetElementId()).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(member3->GetElementId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGroupTests, ElementCrossMembershipOfGroups)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementGroupTests_ElementCrossMembershipOfGroups.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    auto member1 = CreateAndInsertElement(model);
    ASSERT_TRUE(member1.IsValid());
    auto member2 = CreateAndInsertElement(model);
    ASSERT_TRUE(member2.IsValid());
    auto member3 = CreateAndInsertElement(model);
    ASSERT_TRUE(member3.IsValid());

    // Create Element Group
    TestGroupPtr group1 = TestGroup::Create(*m_db, m_defaultModelId, m_defaultCategoryId);
    ASSERT_TRUE(group1.IsValid());
    ASSERT_TRUE(group1->Insert().IsValid());

    TestGroupPtr group2 = TestGroup::Create(*m_db, m_defaultModelId, m_defaultCategoryId);
    ASSERT_TRUE(group2.IsValid());
    ASSERT_TRUE(group2->Insert().IsValid());

    // Insert Elements
    // { in group 1 }
    ASSERT_TRUE(DgnDbStatus::Success == group1->AddMember(*member1));
    ASSERT_TRUE(DgnDbStatus::Success == group1->AddMember(*member2));
    ASSERT_TRUE(DgnDbStatus::Success == group1->AddMember(*member3));
    // { in groupt 2 }
    ASSERT_TRUE(DgnDbStatus::Success == group2->AddMember(*member2));
    ASSERT_TRUE(DgnDbStatus::Success == group2->AddMember(*member3));

    //  Query
    ASSERT_EQ(3, group1->QueryMembers().size());
    ASSERT_EQ(2, group2->QueryMembers().size());
    
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementGroupsMembers)));
    int relationshipCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ++relationshipCount;
        }
    EXPECT_TRUE(5 == relationshipCount);

    DgnElementIdSet allMembers = group1->QueryMembers();
    for (DgnElementId id : allMembers)
        {
        if (id == member1->GetElementId() || id == member2->GetElementId() || id == member3->GetElementId())
            continue;
        else
            EXPECT_TRUE(false) << "This element id should not be here ";
        }

    EXPECT_TRUE(m_db->Elements().GetElement(member1->GetElementId()).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(member2->GetElementId()).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(member3->GetElementId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGroupTests, NestedGroups)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementGroupTests_ElementCrossMembershipOfGroups.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    auto member1 = CreateAndInsertElement(model);
    ASSERT_TRUE(member1.IsValid());
    auto member2 = CreateAndInsertElement(model);
    ASSERT_TRUE(member2.IsValid());
    auto member3 = CreateAndInsertElement(model);
    ASSERT_TRUE(member3.IsValid());

    // Create Element Group
    TestGroupPtr group1 = TestGroup::Create(*m_db, m_defaultModelId, m_defaultCategoryId);
    ASSERT_TRUE(group1.IsValid());
    ASSERT_TRUE(group1->Insert().IsValid());

    TestGroupPtr group2 = TestGroup::Create(*m_db, m_defaultModelId, m_defaultCategoryId);
    ASSERT_TRUE(group2.IsValid());
    ASSERT_TRUE(group2->Insert().IsValid());

    // Insert Elements
    // { in group 1 }
    ASSERT_TRUE(DgnDbStatus::Success == group1->AddMember(*member1));
    ASSERT_TRUE(DgnDbStatus::Success == group1->AddMember(*member2));
    // { in groupt 2 }
    ASSERT_TRUE(DgnDbStatus::Success == group2->AddMember(*member2));
    ASSERT_TRUE(DgnDbStatus::Success == group2->AddMember(*member3));
    ASSERT_TRUE(DgnDbStatus::Success == group2->AddMember(*group1));

    //  Query
    ASSERT_EQ(2, group1->QueryMembers().size());
    ASSERT_EQ(3, group2->QueryMembers().size());
    
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementGroupsMembers)));
    int relationshipCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ++relationshipCount;
        }
    ASSERT_EQ(5, relationshipCount);

    EXPECT_TRUE(m_db->Elements().GetElement(member1->GetElementId()).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(member2->GetElementId()).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(member3->GetElementId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGroupTests, DeleteMemberElement)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementGroupTests_DeleteMemberElement.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    auto member1 = CreateAndInsertElement(model);
    ASSERT_TRUE(member1.IsValid());

    // Create Element Group
    TestGroupPtr group1 = TestGroup::Create(*m_db, m_defaultModelId, m_defaultCategoryId);
    ASSERT_TRUE(group1.IsValid());
    ASSERT_TRUE(group1->Insert().IsValid());

    // Insert Element
    ASSERT_TRUE(DgnDbStatus::Success == group1->AddMember(*member1));
    ASSERT_EQ(1, group1->QueryMembers().size());

    ASSERT_TRUE(DgnDbStatus::Success == m_db->Elements().Delete(member1->GetElementId()));
    ASSERT_EQ(0, group1->QueryMembers().size());

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementGroupsMembers)));
    int relationshipCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ++relationshipCount;
        }
    ASSERT_TRUE(0 == relationshipCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGroupTests, DeleteElementGroup)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementGroupTests_CRUD.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    auto member1 = CreateAndInsertElement(model);
    ASSERT_TRUE(member1.IsValid());

    // Create Element Group
    TestGroupPtr group = TestGroup::Create(*m_db, m_defaultModelId, m_defaultCategoryId);
    ASSERT_TRUE(group.IsValid());
    ASSERT_TRUE(group->Insert().IsValid());

    // Insert Element
    ASSERT_TRUE(DgnDbStatus::Success == group->AddMember(*member1));
    ASSERT_EQ(1, group->QueryMembers().size());

    // Delete Element Group
    ASSERT_TRUE(DgnDbStatus::Success == m_db->Elements().Delete(group->GetElementId()));
    
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementGroupsMembers)));
    int relationshipCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ++relationshipCount;
        }
    // relationshipt table should be empty
    ASSERT_EQ(0, relationshipCount);

    EXPECT_TRUE(m_db->Elements().GetElement(member1->GetElementId()).IsValid());
    }
