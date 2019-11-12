/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DPTEST

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Element Group 
* @bsimethod                                                    Umar.Hayat      09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementGroupTests : public DgnDbTestFixture
{
    PhysicalElementPtr CreateAndInsertElement(PhysicalModelR model);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalElementPtr ElementGroupTests::CreateAndInsertElement(PhysicalModelR model)
    {
    GeometryBuilderPtr builder = GeometryBuilder::Create(model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));
    GenericPhysicalObjectPtr testElement = GenericPhysicalObject::Create(model, m_defaultCategoryId);

    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,/**/  0, 0, 2, /**/ 0, 3, 0, /**/ 0.0, Angle::TwoPi());
    ICurvePrimitivePtr curvePrimitive = ICurvePrimitive::CreateArc(ellipseData);
    builder->Append(*curvePrimitive);
    BentleyStatus status = builder->Finish(*testElement);
    if (SUCCESS != status)
        return NULL;

    DgnDbStatus statusInsert;
    testElement->Insert(&statusInsert);
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
    PhysicalModelPtr model = GetDefaultPhysicalModel();

    PhysicalElementPtr member1 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member1.IsValid());
    PhysicalElementPtr member2 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member2.IsValid());
    PhysicalElementPtr member3 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member3.IsValid());

    // Create Element Group
    TestGroupPtr group = TestGroup::Create(*m_db, model->GetModelId(), m_defaultCategoryId);
    ASSERT_TRUE(group.IsValid());
    ASSERT_TRUE(group->Insert().IsValid());

    // Insert Elements
    ASSERT_EQ(DgnDbStatus::Success, group->AddMember(*member1, 1));
    ASSERT_EQ(DgnDbStatus::Success, group->AddMember(*member2, 2));
    ASSERT_EQ(DgnDbStatus::Success, group->AddMember(*member3));
    // Insert Duplicate
    ASSERT_NE(DgnDbStatus::Success, group->AddMember(*member3));
    // Verify MemberPriority
    ASSERT_EQ(1, group->QueryMemberPriority(*member1));
    ASSERT_EQ(2, group->QueryMemberPriority(*member2));
    ASSERT_EQ(0, group->QueryMemberPriority(*member3)) << "Expect 0 for default priority";

    EXPECT_EQ(3, group->QueryMembers().size());
    ASSERT_EQ(3, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_REL_ElementGroupsMembers)));
    
    m_db->SaveChanges();

    for (DgnElementId id : group->QueryMembers())
        {
        if (id == member1->GetElementId() || id == member2->GetElementId() || id == member3->GetElementId())
            continue;
        else
            EXPECT_TRUE(false) << "This element id should not be here ";
        }
    
    EXPECT_EQ(DgnDbStatus::Success, group->RemoveMember(*member2));
    EXPECT_EQ(2, group->QueryMembers().size());
        
    EXPECT_EQ(DgnDbStatus::Success, group->RemoveMember(*member2));
    EXPECT_EQ(2, group->QueryMembers().size());

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
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    
    PhysicalElementPtr member1 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member1.IsValid());
    PhysicalElementPtr member2 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member2.IsValid());
    PhysicalElementPtr member3 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member3.IsValid());

    // Create Element Group
    TestGroupPtr group1 = TestGroup::Create(*m_db, model->GetModelId(), m_defaultCategoryId);
    ASSERT_TRUE(group1.IsValid());
    ASSERT_TRUE(group1->Insert().IsValid());

    TestGroupPtr group2 = TestGroup::Create(*m_db, model->GetModelId(), m_defaultCategoryId);
    ASSERT_TRUE(group2.IsValid());
    ASSERT_TRUE(group2->Insert().IsValid());

    // Insert Elements
    // { in group 1 }
    ASSERT_EQ(DgnDbStatus::Success, group1->AddMember(*member1));
    ASSERT_EQ(DgnDbStatus::Success, group1->AddMember(*member2));
    ASSERT_EQ(DgnDbStatus::Success, group1->AddMember(*member3));
    // { in groupt 2 }
    ASSERT_EQ(DgnDbStatus::Success, group2->AddMember(*member2));
    ASSERT_EQ(DgnDbStatus::Success, group2->AddMember(*member3));
    // total
    ASSERT_EQ(5, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_REL_ElementGroupsMembers)));

    //  Query
    ASSERT_EQ(3, group1->QueryMembers().size());
    ASSERT_EQ(2, group2->QueryMembers().size());

    for (DgnElementId id : group1->QueryMembers())
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
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    
    PhysicalElementPtr member1 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member1.IsValid());
    PhysicalElementPtr member2 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member2.IsValid());
    PhysicalElementPtr member3 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member3.IsValid());

    // Create Element Group
    TestGroupPtr group1 = TestGroup::Create(*m_db, model->GetModelId(), m_defaultCategoryId);
    ASSERT_TRUE(group1.IsValid());
    ASSERT_TRUE(group1->Insert().IsValid());

    TestGroupPtr group2 = TestGroup::Create(*m_db, model->GetModelId(), m_defaultCategoryId);
    ASSERT_TRUE(group2.IsValid());
    ASSERT_TRUE(group2->Insert().IsValid());

    // Insert Elements
    // { in group 1 }
    ASSERT_EQ(DgnDbStatus::Success, group1->AddMember(*member2));
    ASSERT_EQ(DgnDbStatus::Success, group1->AddMember(*member1));
    // { in groupt 2 }
    ASSERT_EQ(DgnDbStatus::Success, group2->AddMember(*member2));
    ASSERT_EQ(DgnDbStatus::Success, group2->AddMember(*member3));
    ASSERT_EQ(DgnDbStatus::Success, group2->AddMember(*group1));
    // total
    ASSERT_EQ(5, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_REL_ElementGroupsMembers)));

    //  Query
    ASSERT_EQ(2, group1->QueryMembers().size());
    ASSERT_EQ(3, group2->QueryMembers().size());
    
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
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    
    PhysicalElementPtr member1 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member1.IsValid());

    TestGroupPtr group1 = TestGroup::Create(*m_db, model->GetModelId(), m_defaultCategoryId);
    ASSERT_TRUE(group1.IsValid());
    ASSERT_TRUE(group1->Insert().IsValid());

    ASSERT_TRUE(DgnDbStatus::Success == group1->AddMember(*member1));
    ASSERT_EQ(1, group1->QueryMembers().size());
    ASSERT_EQ(1, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_REL_ElementGroupsMembers)));
    ASSERT_EQ(1, DgnDbTestUtils::SelectCountFromTable(*m_db, BIS_TABLE(BIS_REL_ElementRefersToElements)));

    ASSERT_TRUE(DgnDbStatus::Success == m_db->Elements().Delete(member1->GetElementId()));
    ASSERT_EQ(0, group1->QueryMembers().size());
    ASSERT_EQ(0, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_REL_ElementGroupsMembers)));
    ASSERT_EQ(0, DgnDbTestUtils::SelectCountFromTable(*m_db, BIS_TABLE(BIS_REL_ElementRefersToElements)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGroupTests, DeleteElementGroup)
    {
    SetupSeedProject();
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    
    PhysicalElementPtr member1 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member1.IsValid());

    // Create Element Group
    TestGroupPtr group = TestGroup::Create(*m_db, model->GetModelId(), m_defaultCategoryId);
    ASSERT_TRUE(group.IsValid());
    ASSERT_TRUE(group->Insert().IsValid());

    // Insert Element
    ASSERT_TRUE(DgnDbStatus::Success == group->AddMember(*member1));
    ASSERT_EQ(1, group->QueryMembers().size());
    ASSERT_EQ(1, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_REL_ElementGroupsMembers)));
    ASSERT_EQ(1, DgnDbTestUtils::SelectCountFromTable(*m_db, BIS_TABLE(BIS_REL_ElementRefersToElements)));

    // Delete Element Group
    ASSERT_TRUE(DgnDbStatus::Success == m_db->Elements().Delete(group->GetElementId()));
    ASSERT_EQ(0, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_REL_ElementGroupsMembers)));
    ASSERT_EQ(0, DgnDbTestUtils::SelectCountFromTable(*m_db, BIS_TABLE(BIS_REL_ElementRefersToElements)));
    EXPECT_TRUE(m_db->Elements().GetElement(member1->GetElementId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGroupTests, ElementGroupsMembersHelper)
    {
    SetupSeedProject();
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    
    PhysicalElementPtr member1 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member1.IsValid());
    PhysicalElementPtr member2 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member2.IsValid());

    // Create Element Group
    TestGroupPtr group = TestGroup::Create(*m_db, model->GetModelId(), m_defaultCategoryId);
    ASSERT_TRUE(group.IsValid());
    ASSERT_TRUE(group->Insert().IsValid());
    DgnElementCR groupElement = *(m_db->Elements().GetElement(group->GetElementId()));
    TestGroupPtr group2 = TestGroup::Create(*m_db, m_defaultModelId, m_defaultCategoryId);
    ASSERT_TRUE(group2.IsValid());
    ASSERT_TRUE(group2->Insert().IsValid());
    DgnElementCR groupElement2 = *(m_db->Elements().GetElement(group2->GetElementId()));

    // Insert Elements
    ASSERT_EQ(DgnDbStatus::Success, ElementGroupsMembers::Insert(groupElement, *member1, 1));
    ASSERT_EQ(DgnDbStatus::Success, ElementGroupsMembers::Insert(groupElement, *member2, 2));
    ASSERT_EQ(DgnDbStatus::Success, ElementGroupsMembers::Insert(groupElement2, *member1, 2));
    ASSERT_EQ(3, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_REL_ElementGroupsMembers)));

    // Verify MemberPriority
    EXPECT_EQ(1, ElementGroupsMembers::QueryMemberPriority(groupElement, *member1));
    EXPECT_EQ(2, ElementGroupsMembers::QueryMemberPriority(groupElement2, *member1));

    //  Query
    EXPECT_EQ(2, ElementGroupsMembers::QueryMembers(groupElement).size());
    EXPECT_EQ(1, ElementGroupsMembers::QueryMembers(groupElement2).size());
    
    for (DgnElementId id : ElementGroupsMembers::QueryMembers(groupElement))
        {
        if (id == member1->GetElementId() || id == member2->GetElementId() )
            continue;
        else
            EXPECT_TRUE(false) << "This element id should not be here ";
        }

    // Delete Members
    EXPECT_TRUE(DgnDbStatus::Success == ElementGroupsMembers::Delete(groupElement, *member2));
    EXPECT_EQ(1, ElementGroupsMembers::QueryMembers(groupElement).size());
    ASSERT_EQ(2, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_REL_ElementGroupsMembers)));

    // Has member
    EXPECT_TRUE(ElementGroupsMembers::HasMember(groupElement, *member1));
    EXPECT_FALSE(ElementGroupsMembers::HasMember(groupElement, *member2));

    // Query Groups of element
    DgnElementIdSet allGroups = ElementGroupsMembers::QueryGroups(*member1);
    EXPECT_EQ(2, allGroups.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGroupTests, GroupInformationElementInPhysicalModel)
    {
    SetupSeedProject();
    PhysicalModelPtr model = GetDefaultPhysicalModel();

    PhysicalElementPtr member1 = CreateAndInsertElement(*model);
    PhysicalElementPtr member2 = CreateAndInsertElement(*model);
    PhysicalElementPtr member3 = CreateAndInsertElement(*model);
    ASSERT_TRUE(member1.IsValid());
    ASSERT_TRUE(member2.IsValid());
    ASSERT_TRUE(member3.IsValid());

    // Note: GroupInformationElements normally go into a separate GroupInformationModel, but being in other model types is also allowed.
    GenericGroupPtr group = GenericGroup::Create(*model); 
    DgnElementCPtr groupElement = group->Insert();
    ASSERT_TRUE(groupElement.IsValid());

    // Insert Members
    ASSERT_EQ(DgnDbStatus::Success, ElementGroupsMembers::Insert(*groupElement, *member1, 1));
    ASSERT_EQ(DgnDbStatus::Success, ElementGroupsMembers::Insert(*groupElement, *member2, 2));
    ASSERT_EQ(DgnDbStatus::Success, ElementGroupsMembers::Insert(*groupElement, *member3, 3));
    EXPECT_EQ(3, ElementGroupsMembers::QueryMembers(*groupElement).size());
    }
