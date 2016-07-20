/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ElementGroup_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DPTEST

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
    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));
    GenericPhysicalObjectPtr testElement = GenericPhysicalObject::Create(*(model->ToSpatialModelP()), m_defaultCategoryId);

    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,/**/  0, 0, 2, /**/ 0, 3, 0, /**/ 0.0, Angle::TwoPi());
    ICurvePrimitivePtr curvePrimitive = ICurvePrimitive::CreateArc(ellipseData);
    builder->Append(*curvePrimitive);
    BentleyStatus status = builder->SetGeometryStreamAndPlacement(*testElement);
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
    SetupSeedProject();

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
    ASSERT_TRUE(DgnDbStatus::Success == group->AddMember(*member1, 1));
    ASSERT_TRUE(DgnDbStatus::Success == group->AddMember(*member2, 2));
    ASSERT_TRUE(DgnDbStatus::Success == group->AddMember(*member3));
    // Insert Duplicate
    ASSERT_FALSE(DgnDbStatus::Success == group->AddMember(*member3));
    // Verify MemberPriority
    ASSERT_EQ(1, group->QueryMemberPriority(*member1));
    ASSERT_EQ(2, group->QueryMemberPriority(*member2));
    ASSERT_EQ(0, group->QueryMemberPriority(*member3)) << "Expect 0 for default priority";

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
    SetupSeedProject();

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
    SetupSeedProject();

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
    SetupSeedProject();

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
    SetupSeedProject();

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGroupTests, ElementGroupsMembersHelper)
    {
    SetupSeedProject();

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    auto member1 = CreateAndInsertElement(model);
    ASSERT_TRUE(member1.IsValid());
    auto member2 = CreateAndInsertElement(model);
    ASSERT_TRUE(member2.IsValid());

    // Create Element Group
    TestGroupPtr group = TestGroup::Create(*m_db, m_defaultModelId, m_defaultCategoryId);
    ASSERT_TRUE(group.IsValid());
    ASSERT_TRUE(group->Insert().IsValid());
    DgnElementCR groupElement = *(m_db->Elements().GetElement(group->GetElementId()));
    TestGroupPtr group2 = TestGroup::Create(*m_db, m_defaultModelId, m_defaultCategoryId);
    ASSERT_TRUE(group2.IsValid());
    ASSERT_TRUE(group2->Insert().IsValid());
    DgnElementCR groupElement2 = *(m_db->Elements().GetElement(group2->GetElementId()));

    // Insert Elements
    ASSERT_TRUE(DgnDbStatus::Success == ElementGroupsMembers::Insert(groupElement, *member1->ToDgnElement(), 1));
    ASSERT_TRUE(DgnDbStatus::Success == ElementGroupsMembers::Insert(groupElement, *member2->ToDgnElement(), 2));
    ASSERT_TRUE(DgnDbStatus::Success == ElementGroupsMembers::Insert(groupElement2, *member1->ToDgnElement(), 2));

    // Verify MemberPriority
    EXPECT_EQ(1, ElementGroupsMembers::QueryMemberPriority(groupElement, *member1->ToDgnElement()));
    EXPECT_EQ(2, ElementGroupsMembers::QueryMemberPriority(groupElement2, *member1->ToDgnElement()));

    //  Query
    EXPECT_TRUE(2 == ElementGroupsMembers::QueryMembers(groupElement).size());
    
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementGroupsMembers)));
    int relationshipCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ++relationshipCount;
        }
    EXPECT_TRUE(3 == relationshipCount);
    m_db->SaveChanges();

    DgnElementIdSet allMembers = ElementGroupsMembers::QueryMembers(groupElement);
    for (DgnElementId id : allMembers)
        {
        if (id == member1->GetElementId() || id == member2->GetElementId() )
            continue;
        else
            EXPECT_TRUE(false) << "This element id should not be here ";
        }

    // Delete Members
    EXPECT_TRUE(DgnDbStatus::Success == ElementGroupsMembers::Delete(groupElement, *member2->ToDgnElement()));
    EXPECT_TRUE(1 == ElementGroupsMembers::QueryMembers(groupElement).size());

    // Has member
    EXPECT_TRUE(ElementGroupsMembers::HasMember(groupElement, *member1->ToDgnElement()));
    EXPECT_FALSE(ElementGroupsMembers::HasMember(groupElement, *member2->ToDgnElement()));

    // Query Groups of element
    DgnElementIdSet allGroups = ElementGroupsMembers::QueryGroups(*member1->ToDgnElement());
    EXPECT_TRUE(2 == allGroups.size());
    }
