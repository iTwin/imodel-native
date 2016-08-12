/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnElement_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DPTEST

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct DgnElementTests : public DgnDbTestFixture
    {
    TestElementCPtr AddChild(DgnElementCR parent);
    void TestAutoHandledPropertiesGetSet();
    void TestAutoHandledPropertiesCA();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnElementTests, ResetStatistics)
    {
    SetupSeedProject();

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelPtr seedModel = m_db->Models().GetModel(m_defaultModelId);
    seedModel->FillModel();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts a model
    DgnModelPtr m1 = seedModel->Clone(DgnModel::CreateModelCode("Model1"));
    m1->SetUserLabel("Test Model 1");
    m1->Insert();
    EXPECT_TRUE (m1 != nullptr);
    m_db->SaveChanges("changeSet1");

    DgnModelId m1id = m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1"));
    EXPECT_TRUE (m1id.IsValid());

    //Inserts 2 elements.
    auto keyE1 = InsertElement(m1id);
    DgnElementId E1id = keyE1->GetElementId();
    DgnElementCPtr E1 = m_db->Elements().GetElement(E1id);
    EXPECT_TRUE (E1 != nullptr);

    auto keyE2 = InsertElement(m1id);
    DgnElementId E2id = keyE2->GetElementId();
    DgnElementCPtr E2 = m_db->Elements().GetElement(E2id);
    EXPECT_TRUE (E2 != nullptr);

    DgnModelId model_id = m_db->Elements().QueryModelId(E1id);
    EXPECT_EQ (m1id, model_id);

    //Deletes the first element.
    DgnDbStatus status=E2->Delete();
    EXPECT_EQ ((DgnDbStatus)SUCCESS, status);
    m_db->SaveChanges();

    uint64_t memTarget = 0;
    m_db->Memory().PurgeUntil(memTarget);

    //Get stats of the element pool.
    DgnElements::Statistics stats = m_db->Elements().GetStatistics();

    uint32_t NewElements = stats.m_newElements;
    EXPECT_EQ (2, NewElements);

    uint32_t RefElements = stats.m_reReferenced;
    EXPECT_EQ (0, RefElements);

    uint32_t UnrefElements = stats.m_unReferenced;
    EXPECT_EQ (0, UnrefElements);

    uint32_t PurgedElements = stats.m_purged;
    EXPECT_EQ (1, PurgedElements);

    m_db->Elements().ResetStatistics();

    stats = m_db->Elements().GetStatistics();

    //Statistics after reset.
    NewElements = stats.m_newElements;
    EXPECT_EQ (0, NewElements);

    PurgedElements = stats.m_purged;
    EXPECT_EQ (0, PurgedElements);

    RefElements = stats.m_reReferenced;
    EXPECT_EQ (0, RefElements);

    UnrefElements = stats.m_unReferenced;
    EXPECT_EQ (0, UnrefElements);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnElementTests, UpdateElement)
    {
    SetupSeedProject();

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelPtr seedModel = m_db->Models().GetModel(m_defaultModelId);
    seedModel->FillModel();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts a model
    DgnModelPtr m1 = seedModel->Clone(DgnModel::CreateModelCode("Model1"));
    m1->SetUserLabel("Test Model 1");
    m1->Insert();
    EXPECT_TRUE(m1 != nullptr);
    m_db->SaveChanges("changeSet1");

    DgnModelId m1id = m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1"));
    EXPECT_TRUE(m1id.IsValid());

    auto keyE1 = InsertElement(m1id);
    DgnElementId e1id = keyE1->GetElementId();
    DgnElementCPtr e1 = m_db->Elements().GetElement(e1id);
    EXPECT_TRUE(e1 != nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementCPtr DgnElementTests::AddChild(DgnElementCR parent)
    {
    TestElementPtr child = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId);
    child->SetParentId(parent.GetElementId());
    auto el = child->Insert();
    if (!el.IsValid())
        return nullptr;
    return dynamic_cast<TestElement const*>(el.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, DgnElementTransformer)
    {
    SetupSeedProject();

    if (true)
        {
        DgnElementCPtr parent1 = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId)->Insert();
        TestElementCPtr c11 = AddChild(*parent1);
        TestElementCPtr c12 = AddChild(*parent1);

        DgnEditElementCollector all;
        all.EditAssembly(*parent1);

        for (auto el : all)
            {
            ASSERT_TRUE(el->ToGeometrySource3d()->GetPlacement().GetOrigin().IsEqual(DPoint3d::FromZero()));
            ASSERT_EQ(0, el->ToGeometrySource3d()->GetPlacement().GetAngles().GetYaw().Degrees());
            }

        Transform offsetby1 = Transform::FromIdentity();
        offsetby1.SetTranslation(DPoint3d::From(1,0,0));

        DgnElementTransformer::ApplyTransformToAll(all, offsetby1);

        for (auto el : all)
            {
            ASSERT_TRUE(el->ToGeometrySource3d()->GetPlacement().GetOrigin().IsEqual(DPoint3d::From(1,0,0)));
            ASSERT_EQ(0, el->ToGeometrySource3d()->GetPlacement().GetAngles().GetYaw().Degrees()) << "yaw should be unaffected";
            ASSERT_EQ(0, el->ToGeometrySource3d()->GetPlacement().GetAngles().GetPitch().Degrees()) << "pitch should be unaffected";
            ASSERT_EQ(0, el->ToGeometrySource3d()->GetPlacement().GetAngles().GetRoll().Degrees()) << "roll should be unaffected";
            }
        }

    if (true)
        {
        DgnElementCPtr parent1 = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId)->Insert();
        TestElementCPtr c11 = AddChild(*parent1);
        TestElementCPtr c12 = AddChild(*parent1);

        DgnEditElementCollector all;
        all.EditAssembly(*parent1);

        for (auto el : all)
            {
            ASSERT_TRUE(el->ToGeometrySource3d()->GetPlacement().GetOrigin().IsEqual(DPoint3d::From(0,0,0)));
            ASSERT_EQ(0, el->ToGeometrySource3d()->GetPlacement().GetAngles().GetYaw().Degrees());
            }

        DgnElementTransformer::ApplyTransformToAll(all, Transform::FromPrincipleAxisRotations(Transform::FromIdentity(), 0, 0, msGeomConst_piOver4));

        for (auto el : all)
            {
            ASSERT_TRUE(el->ToGeometrySource3d()->GetPlacement().GetOrigin().IsEqual(DPoint3d::From(0,0,0)));
            EXPECT_DOUBLE_EQ(45, el->ToGeometrySource3d()->GetPlacement().GetAngles().GetYaw().Degrees());
            EXPECT_DOUBLE_EQ(0, el->ToGeometrySource3d()->GetPlacement().GetAngles().GetPitch().Degrees()) << "pitch should be unaffected";
            EXPECT_DOUBLE_EQ(0, el->ToGeometrySource3d()->GetPlacement().GetAngles().GetRoll().Degrees()) << "roll should be unaffected";
            }
        }


    //  Now try a more interesting assembly
    if (true)
        {
        DgnElementCPtr parent1 = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId)->Insert();
        TestElementCPtr c11 = AddChild(*parent1);
            {
            DgnElementPtr ec11 = c11->CopyForEdit();
            Transform offsetUpAndOver = Transform::FromIdentity();
            offsetUpAndOver.SetTranslation(DPoint3d::From(1,1,0));
            DgnElementTransformer::ApplyTransformTo(*ec11, offsetUpAndOver);
            ec11->Update();
            }
        //     [c]
        //  [p]
        ASSERT_TRUE(c11->GetPlacement().GetOrigin().IsEqual(DPoint3d::From(1,1,0)));
        ASSERT_EQ(0, c11->GetPlacement().GetAngles().GetYaw().Degrees());
        ASSERT_EQ(0, c11->GetPlacement().GetAngles().GetPitch().Degrees());
        ASSERT_EQ(0, c11->GetPlacement().GetAngles().GetRoll().Degrees());

        DgnEditElementCollector all;
        all.EditAssembly(*parent1);

        //  Rotate them around the zaxis, so that child swings up and around to the left.
        //    \c
        //  \p
        DRay3d flagPole = DRay3d::FromOriginAndVector(DPoint3d::FromZero(), DVec3d::From(0,0,1));
        Transform rotateAroundFlagPole = Transform::FromAxisAndRotationAngle(flagPole, msGeomConst_piOver4);
        DgnElementTransformer::ApplyTransformToAll(all, rotateAroundFlagPole);

        DgnElementPtr eparent1 = all.FindElementById(parent1->GetElementId());
        DgnElementPtr ec11 = all.FindElementById(c11->GetElementId());
        Placement3d eparentplacement = eparent1->ToGeometrySource3dP()->GetPlacement();
        Placement3d ec11placement = ec11->ToGeometrySource3dP()->GetPlacement();
        EXPECT_DOUBLE_EQ(45, eparentplacement.GetAngles().GetYaw().Degrees());
        EXPECT_DOUBLE_EQ(45, ec11placement.GetAngles().GetYaw().Degrees());
        EXPECT_DOUBLE_EQ( 0, ec11placement.GetAngles().GetPitch().Degrees()) << "pitch should have been unaffected";
        EXPECT_DOUBLE_EQ( 0, ec11placement.GetAngles().GetRoll().Degrees()) << "roll should have been unaffected";
        ASSERT_TRUE(eparentplacement.GetOrigin().AlmostEqual(DPoint3d::FromZero()));
        ASSERT_TRUE(ec11placement.GetOrigin().AlmostEqual(DPoint3d::From(0,sqrt(2),0)));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, DgnEditElementCollector)
    {
    SetupSeedProject();

    DgnElementCPtr parent1 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId)->Insert();
    ASSERT_TRUE(parent1.IsValid());

    // single element
        {
        DgnEditElementCollector coll;
        auto eparent1 = coll.EditElement(*parent1);
        ASSERT_TRUE(eparent1.IsValid());
        ASSERT_TRUE(eparent1->GetElementId() == parent1->GetElementId());

        auto eparent1cc1 = coll.EditElement(*eparent1);
        ASSERT_TRUE(eparent1cc1.get() == eparent1.get()) << "no dups allowed in the collection";
        ASSERT_TRUE(eparent1cc1 == eparent1) << "no dups allowed in the collection";

        auto eparent1cc2 = coll.EditElement(*parent1);
        ASSERT_TRUE(eparent1cc2.get() == eparent1.get()) << "no dups allowed in the collection";
        ASSERT_TRUE(eparent1cc2 == eparent1) << "no dups allowed in the collection";

        ASSERT_EQ(1, coll.size());

        // FindByElementId
        auto found = coll.FindElementById(parent1->GetElementId());
        ASSERT_TRUE(found.get() == eparent1.get());
        ASSERT_TRUE(found->GetElementId() == parent1->GetElementId());

        // Remove
        coll.RemoveElement(*found);
        ASSERT_EQ(0, coll.size());
        coll.RemoveElement(*found);
        ASSERT_EQ(0, coll.size());
        }

    // single element (childless)
        {
        DgnEditElementCollector coll;
        coll.EditAssembly(*parent1);
        ASSERT_EQ(1, coll.size());
        }

    // Add some children
    TestElementCPtr c11 = AddChild(*parent1);
    ASSERT_EQ(c11->GetParentId(), parent1->GetElementId());
    TestElementCPtr c12 = AddChild(*parent1);
    ASSERT_EQ(c12->GetParentId(), parent1->GetElementId());
    ASSERT_EQ(2, parent1->QueryChildren().size());

    //  element with children
        {
        DgnEditElementCollector all;
        all.EditAssembly(*parent1);
        ASSERT_EQ(3, all.size());

        DgnEditElementCollector noChildren;
        noChildren.EditAssembly(*parent1, 0);
        ASSERT_EQ(1, noChildren.size());

        DgnEditElementCollector onlyElement;
        onlyElement.EditElement(*parent1);
        ASSERT_EQ(1, onlyElement.size());

        DgnEditElementCollector onlyChildren;
        onlyChildren.AddChildren(*parent1);
        ASSERT_EQ(2, onlyChildren.size());
        onlyChildren.AddChildren(*parent1);
        ASSERT_EQ(2, onlyChildren.size()) << "no dup children allowed in the collection";
        }

    // Add nested children
    TestElementCPtr c111 = AddChild(*c11);
    ASSERT_EQ(c111->GetParentId(), c11->GetElementId());
    TestElementCPtr c112 = AddChild(*c11);
    ASSERT_EQ(c112->GetParentId(), c11->GetElementId());
    ASSERT_EQ(2, c11->QueryChildren().size());

    //  element with children
        {
        DgnEditElementCollector all;
        all.EditAssembly(*parent1);
        ASSERT_EQ(5, all.size());

        DgnEditElementCollector noChildren;
        noChildren.EditAssembly(*parent1, 0);
        ASSERT_EQ(1, noChildren.size());

        DgnEditElementCollector onlyElement;
        onlyElement.EditElement(*parent1);
        ASSERT_EQ(1, onlyElement.size());

        DgnEditElementCollector onlyChildren1;
        onlyChildren1.AddChildren(*parent1, 1);
        ASSERT_EQ(2, onlyChildren1.size());
        onlyChildren1.AddChildren(*parent1, 1);
        ASSERT_EQ(2, onlyChildren1.size()) << "no dup children allowed in the collection";

        DgnEditElementCollector onlyChildrenAll;
        onlyChildrenAll.AddChildren(*parent1);
        ASSERT_EQ(4, onlyChildrenAll.size());
        onlyChildrenAll.AddChildren(*parent1);
        ASSERT_EQ(4, onlyChildrenAll.size()) << "no dup children allowed in the collection";

        // Test iterator
        size_t count = 0;
        for (auto el : all)
            {
            ASSERT_TRUE(el != nullptr);
            ++count;
            }
        ASSERT_EQ(all.size(), count);

        // apply various std algorithms to the collection
        auto eparent1 = all.FindElementById(parent1->GetElementId());
        auto ifind = std::find(all.begin(), all.end(), eparent1.get());
        ASSERT_TRUE(ifind != all.end());
        ASSERT_EQ(*ifind, eparent1.get());

        // Test removal of children
        all.RemoveChildren(*c11);
        ASSERT_EQ(3, all.size());
        all.RemoveChildren(*c11);
        ASSERT_EQ(3, all.size());

        all.RemoveChildren(*parent1);
        ASSERT_EQ(1, all.size());
        all.RemoveChildren(*parent1);
        ASSERT_EQ(1, all.size());

        all.RemoveElement(*all.FindElementById(parent1->GetElementId()));
        ASSERT_EQ(0, all.size());
        all.RemoveElement(*all.FindElementById(parent1->GetElementId()));
        ASSERT_EQ(0, all.size());
        all.RemoveChildren(*c11);
        ASSERT_EQ(0, all.size());
        }

    // mixture of persistent and non-persistent elements
        {
        DgnElementPtr nonPersistent = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId);

        DgnEditElementCollector coll;
        DgnElementPtr eparent1 = coll.EditElement(*parent1);
        DgnElementPtr enonPersistent = coll.AddElement(*nonPersistent);
        ASSERT_EQ(enonPersistent.get(), nonPersistent.get());
        ASSERT_EQ(2, coll.size());
        DgnElementPtr enonPersistentcc = coll.EditElement(*nonPersistent);
        ASSERT_NE(enonPersistentcc.get(), nonPersistent.get()) << "you can add a second copy of a non-persistent element -- we can't tell it's a copy.";
        ASSERT_EQ(3, coll.size());

        ASSERT_TRUE(coll.FindElementById(parent1->GetElementId()).IsValid());
        ASSERT_FALSE(coll.FindElementById(nonPersistent->GetElementId()).IsValid());
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, ElementCopierTests)
    {
    SetupSeedProject();

    DgnElementCPtr parent = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId)->Insert();
    TestElementCPtr c1 = AddChild(*parent);
    TestElementCPtr c2 = AddChild(*parent);

    DgnModelPtr destModel = parent->GetModel();

    // Verify that children are copied
        {
        DgnCloneContext ccontext;
        ElementCopier copier(ccontext);
        copier.SetCopyChildren(true);
        DgnElementCPtr parent_cc = copier.MakeCopy(nullptr, *destModel, *parent, DgnCode());
        ASSERT_TRUE(parent_cc.IsValid());
        auto c1ccId = copier.GetCloneContext().FindElementId(c1->GetElementId());
        ASSERT_TRUE(c1ccId.IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(c1ccId).IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(c1ccId)->GetModel() == destModel);
        auto c2ccId = copier.GetCloneContext().FindElementId(c2->GetElementId());
        ASSERT_TRUE(c2ccId.IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(c2ccId).IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(c2ccId)->GetModel() == destModel);
        size_t cccount = 0;
        for (auto childid : parent_cc->QueryChildren())
            {
            ASSERT_TRUE(childid == c1ccId || childid == c2ccId);
            ++cccount;
            }
        ASSERT_EQ(2, cccount);

        // Verify that a second attempt to copy an already copied element does nothing
        ASSERT_TRUE(parent_cc.get() == copier.MakeCopy(nullptr, *destModel, *parent, DgnCode()).get());
        ASSERT_TRUE(c1ccId == copier.MakeCopy(nullptr, *destModel, *c1, DgnCode())->GetElementId());
        ASSERT_TRUE(c2ccId == copier.MakeCopy(nullptr, *destModel, *c2, DgnCode())->GetElementId());
        }

    // Verify that children are NOT copied
        {
        DgnCloneContext ccontext;
        ElementCopier copier(ccontext);
        copier.SetCopyChildren(false);
        DgnElementCPtr parent_cc = copier.MakeCopy(nullptr, *destModel, *parent, DgnCode());
        ASSERT_TRUE(parent_cc.IsValid());
        auto c1ccId = copier.GetCloneContext().FindElementId(c1->GetElementId());
        ASSERT_FALSE(c1ccId.IsValid());
        auto c2ccId = copier.GetCloneContext().FindElementId(c2->GetElementId());
        ASSERT_FALSE(c2ccId.IsValid());
        ASSERT_EQ(0, parent_cc->QueryChildren().size());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, ElementCopierTests_Group)
    {
    SetupSeedProject();

    DgnElementCPtr group = TestGroup::Create(*m_db, m_defaultModelId, m_defaultCategoryId)->Insert();
    DgnElementCPtr m1 = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId)->Insert();
    DgnElementCPtr m2 = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId)->Insert();
    ElementGroupsMembers::Insert(*group, *m1, 0);
    ElementGroupsMembers::Insert(*group, *m2, 0);
    ASSERT_TRUE(group->ToIElementGroup()->QueryMembers().size() == 2);

    DgnModelPtr destModel = group->GetModel();

    // Verify that members are copied
        {
        DgnCloneContext ccontext;
        ElementCopier copier(ccontext);
        copier.SetCopyGroups(true);
        DgnElementCPtr group_cc = copier.MakeCopy(nullptr, *destModel, *group, DgnCode());
        ASSERT_TRUE(group_cc.IsValid());
        auto m1ccId = copier.GetCloneContext().FindElementId(m1->GetElementId());
        ASSERT_TRUE(m1ccId.IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(m1ccId).IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(m1ccId)->GetModel() == destModel);
        auto m2ccId = copier.GetCloneContext().FindElementId(m2->GetElementId());
        ASSERT_TRUE(m2ccId.IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(m2ccId).IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(m2ccId)->GetModel() == destModel);
        size_t cccount = 0;
        for (auto memberid : group_cc->ToIElementGroup()->QueryMembers())
            {
            ASSERT_TRUE(memberid == m1ccId || memberid == m2ccId);
            ++cccount;
            }
        ASSERT_EQ(2, cccount);

        // Verify that a second attempt to copy an already copied element does nothing
        ASSERT_TRUE(group_cc.get() == copier.MakeCopy(nullptr, *destModel, *group, DgnCode()).get());
        ASSERT_TRUE(m1ccId == copier.MakeCopy(nullptr, *destModel, *m1, DgnCode())->GetElementId());
        ASSERT_TRUE(m2ccId == copier.MakeCopy(nullptr, *destModel, *m2, DgnCode())->GetElementId());
        }

    // Verify that members are NOT copied
        {
        DgnCloneContext ccontext;
        ElementCopier copier(ccontext);
        copier.SetCopyGroups(false);
        DgnElementCPtr group_cc = copier.MakeCopy(nullptr, *destModel, *group, DgnCode());
        ASSERT_TRUE(group_cc.IsValid());
        ASSERT_EQ(0, group_cc->ToIElementGroup()->QueryMembers().size());
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnElementTests, ForceElementIdForInsert)
    {
    SetupSeedProject();

    DgnModelId modelId = m_db->Models().QueryFirstModelId();
    DgnClassId classId = m_db->Domains().GetClassId(generic_ElementHandler::GenericPhysicalObjectHandler::GetHandler());
    DgnElementId elementId;

    // Test creating an element the "normal" way (by letting the DgnElementId be assigned by the framework)
        {
        GenericPhysicalObjectPtr element = new GenericPhysicalObject(GenericPhysicalObject::CreateParams(*m_db, modelId, classId, m_defaultCategoryId));
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        ASSERT_TRUE(element->GetElementId().IsValid());
        ASSERT_FALSE(element->Insert().IsValid()) << "Second insert of the same element should fail";
        elementId = element->GetElementId();
        }

    DgnElementId forcedElementId(elementId.GetValue() + 100);

    // Confirm that supplying a DgnElementId in CreateParams for Insert does not work (not intended to work)
        {
        GenericPhysicalObject::CreateParams createParams(*m_db, modelId, classId, m_defaultCategoryId);
        createParams.SetElementId(DgnElementId(elementId.GetValue() + 100));
    
        GenericPhysicalObjectPtr element = new GenericPhysicalObject(createParams);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->Insert().IsValid()) << "It is not valid to supply a DgnElementId for Insert via CreateParams";
        }

    // Test PKPM's synchronization workflow where they must force a DgnElementId on Insert.
        {
        GenericPhysicalObjectPtr element = new GenericPhysicalObject(GenericPhysicalObject::CreateParams(*m_db, modelId, classId, m_defaultCategoryId));
        ASSERT_TRUE(element.IsValid());
        element->ForceElementIdForInsert(forcedElementId);
        ASSERT_EQ(element->GetElementId(), forcedElementId);
        ASSERT_TRUE(element->Insert().IsValid());
        ASSERT_EQ(element->GetElementId(), forcedElementId);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnElementTests, GenericDomainElements)
    {
    SetupSeedProject();

    // GenericSpatialLocation
        {
        DgnModelId modelId = m_db->Models().QueryFirstModelId();
        DgnClassId classId = m_db->Domains().GetClassId(generic_ElementHandler::GenericSpatialLocationHandler::GetHandler());
        GenericSpatialLocationPtr element = new GenericSpatialLocation(GenericSpatialLocation::CreateParams(*m_db, modelId, classId, m_defaultCategoryId));
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        ASSERT_TRUE(element->GetElementId().IsValid());
        }

    // GenericPhysicalObject
        {
        DgnModelId modelId = m_db->Models().QueryFirstModelId();
        DgnClassId classId = m_db->Domains().GetClassId(generic_ElementHandler::GenericPhysicalObjectHandler::GetHandler());
        GenericPhysicalObjectPtr element = new GenericPhysicalObject(GenericPhysicalObject::CreateParams(*m_db, modelId, classId, m_defaultCategoryId));
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        ASSERT_TRUE(element->GetElementId().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementGeomAndPlacementTests : DgnElementTests
{
    GeometryBuilderPtr CreateGeom();
    RefCountedPtr<PhysicalElement> CreateElement(bool wantPlacement, bool wantGeom);
    static bool AreEqualPlacements(Placement3dCR a, Placement3dCR b);
    void TestLoadElem(DgnElementId id, Placement3d const* placement, bool hasGeometry);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilderPtr ElementGeomAndPlacementTests::CreateGeom()
    {
    DgnModelPtr model = m_db->Models().GetModel(m_defaultModelId);
    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));
    CurveVectorPtr curveVector = GeomHelper::computeShape();
    builder->Append(*curveVector);
    double dz = 3.0, radius = 1.5;
    DgnConeDetail cylinderDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius, true);
    ISolidPrimitivePtr cylinder = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    builder->Append(*cylinder);

    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,
        0, 0, 2,
        0, 3, 0,
        0.0, Angle::TwoPi());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc(ellipseData);
    builder->Append(*ellipse);

    Render::GeometryParams elemDisplayParams;
    elemDisplayParams.SetCategoryId(m_defaultCategoryId);
    elemDisplayParams.SetWeight(2);
    builder->Append(elemDisplayParams);

    return builder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomAndPlacementTests::AreEqualPlacements(Placement3dCR a, Placement3dCR b)
    {
    return DoubleOps::AlmostEqual(a.GetOrigin().x, b.GetOrigin().x)
        && DoubleOps::AlmostEqual(a.GetOrigin().y, b.GetOrigin().y)
        && DoubleOps::AlmostEqual(a.GetOrigin().z, b.GetOrigin().z)
        && DoubleOps::AlmostEqual(a.GetAngles().GetYaw().Degrees(), b.GetAngles().GetYaw().Degrees())
        && DoubleOps::AlmostEqual(a.GetAngles().GetPitch().Degrees(), b.GetAngles().GetPitch().Degrees())
        && DoubleOps::AlmostEqual(a.GetAngles().GetRoll().Degrees(), b.GetAngles().GetRoll().Degrees());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomAndPlacementTests::TestLoadElem(DgnElementId id, Placement3d const* placement, bool hasGeometry)
    {
    auto el = m_db->Elements().Get<PhysicalElement>(id);
    ASSERT_TRUE(el.IsValid());
    EXPECT_EQ(nullptr != placement, el->GetPlacement().IsValid());
    if (nullptr != placement)
        {
        EXPECT_TRUE(placement->IsValid());
        EXPECT_TRUE(el->GetPlacement().IsValid());
        EXPECT_TRUE(AreEqualPlacements(*placement, el->GetPlacement()));
        }

    EXPECT_EQ(hasGeometry, el->HasGeometry());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomAndPlacementTests, ValidateOnInsert)
    {
    SetupSeedProject();
    m_defaultModelId = m_db->Models().QueryFirstModelId();

    DgnElementId noPlacementNoGeomId, placementAndGeomId, placementAndNoGeomId;
    Placement3d placement;

        {
        // Null placement + null geom
        TestElementPtr el = TestElement::CreateWithoutGeometry(*m_db, m_defaultModelId, m_defaultCategoryId);
        EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());
        TestLoadElem(el->GetElementId(), nullptr, false);
        noPlacementNoGeomId = el->GetElementId();

        // Non-null placement + non-null geom
        el = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnCode());
        auto geom = CreateGeom();
        EXPECT_EQ(SUCCESS, geom->Finish(*el));
        placement = el->GetPlacement();
        EXPECT_TRUE(placement.IsValid());
        EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());
        placementAndGeomId = el->GetElementId();

        // Non-null placement + null geom
        el = TestElement::CreateWithoutGeometry(*m_db, m_defaultModelId, m_defaultCategoryId);
        el->SetPlacement(placement);
        EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());
        placementAndNoGeomId = el->GetElementId();

        // Null placement + non-null geom
        el = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnCode());
        EXPECT_EQ(SUCCESS, geom->Finish(*el));
        el->SetPlacement(Placement3d());
        DgnDbStatus status;
        EXPECT_FALSE(m_db->Elements().Insert(*el, &status).IsValid());
        EXPECT_EQ(DgnDbStatus::BadElement, status);
        }

    m_db->Memory().PurgeUntil(0);

    TestLoadElem(noPlacementNoGeomId, nullptr, false);
    TestLoadElem(placementAndGeomId, &placement, true);
    TestLoadElem(placementAndNoGeomId, &placement, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
static int32_t countElementsOfClass(DgnClassId classId, DgnDbR db)
    {
    CachedStatementPtr stmt = db.Elements().GetStatement("SELECT count(*) FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ECClassId=?");
    stmt->BindUInt64(1, classId.GetValue());
    stmt->Step();
    return stmt->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* INSERT statements on element table were using the ClassId of the base class if
* the derived class did not define its own Handler subclass.
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, HandlerlessClass)
    {
    SetupSeedProject();

    DgnClassId classId(m_db->Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    TestElement el(params);
    EXPECT_TRUE(el.Insert().IsValid());

    EXPECT_EQ(0, countElementsOfClass(TestElement::QueryClassId(*m_db), *m_db));
    EXPECT_EQ(1, countElementsOfClass(classId, *m_db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementTests::TestAutoHandledPropertiesCA()
    {
#ifdef WIP_AUTO_HANDLED_PROPERTIES // *** test Custom Attributes when we get them
    DgnClassId classId(m_db->Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    TestElement el(params);

    static Utf8CP s_autoHandledPropNames[] = {
        "IntegerProperty1"
        ,"IntegerProperty2"
        ,"IntegerProperty3"
        ,"IntegerProperty4"
        ,"DoubleProperty1"
        ,"DoubleProperty2"
        ,"DoubleProperty3"
        ,"DoubleProperty4"
        ,"PointProperty1"
        ,"PointProperty2"
        ,"PointProperty3"
        ,"PointProperty4"
        ,"StringProperty"
        ,"IntProperty"
        };

    for (int i=0; i<_countof(s_autoHandledPropNames); ++i)
        {
        ECN::ECValue checkValue;
        EXPECT_EQ(DgnDbStatus::Success, el.GetProperty(checkValue, s_autoHandledPropNames[i]));
        EXPECT_TRUE(checkValue.IsNull());
        }

    // Check a few non-auto-handled props
    ECN::ECValue checkValue;
    EXPECT_EQ(DgnDbStatus::Success, el.GetProperty(checkValue, "TestIntegerProperty1"));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementTests::TestAutoHandledPropertiesGetSet()
    {
    DgnElementCPtr persistentEl;
    if (true)
        {
        DgnClassId classId(m_db->Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);

        //  No unhandled properties yet
        ECN::ECValue checkValue;
        EXPECT_EQ(DgnDbStatus::Success, el.GetProperty(checkValue, "StringProperty"));
        EXPECT_TRUE(checkValue.IsNull());

        //  Set unhandled property (in memory)
        ASSERT_EQ(DgnDbStatus::Success, el.SetProperty("StringProperty", ECN::ECValue("initial value")));

        //      ... check that we see the pending value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, el.GetProperty(checkValue, "StringProperty"));
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());

        //  Insert the element
        persistentEl = el.Insert();
        }
    
    ASSERT_TRUE(persistentEl.IsValid());

    // Check that we see the stored value
    ECN::ECValue checkValue;
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetProperty(checkValue, "StringProperty"));
    EXPECT_STREQ("initial value", checkValue.ToString().c_str());

    if (true)
        {
        //  Get ready to modify the element
        RefCountedPtr<TestElement> editEl = persistentEl->MakeCopy<TestElement>();

        //      ... initially we still see the initial/stored value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetProperty(checkValue, "StringProperty"));
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());

        //  Set a new value (in memory)
        EXPECT_EQ(DgnDbStatus::Success, editEl->SetProperty("StringProperty", ECN::ECValue("changed value")));

        //      ... check that we now see the pending value on the edited copy ...
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetProperty(checkValue, "StringProperty"));
        EXPECT_STREQ("changed value", checkValue.ToString().c_str());

        //      ... but no change on the persistent element
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetProperty(checkValue, "StringProperty"));
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());

        //  Update the element
        persistentEl = editEl->Update();
        }

    // Check that the stored value was changed
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetProperty(checkValue, "StringProperty"));
    EXPECT_STREQ("changed value", checkValue.ToString().c_str());

    // REALLY check that the stored value was changed
    auto fileName = m_db->GetFileName();
    auto elid = persistentEl->GetElementId();
    persistentEl = nullptr;
    m_db->SaveChanges();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    persistentEl = m_db->Elements().GetElement(elid);
    ASSERT_TRUE(persistentEl.IsValid());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetProperty(checkValue, "StringProperty"));
    EXPECT_STREQ("changed value", checkValue.ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, TestAutoHandledProperties)
    {
    SetupSeedProject();
    TestAutoHandledPropertiesCA();
    TestAutoHandledPropertiesGetSet();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, CreateFromECInstance)
    {
    SetupSeedProject();

    DgnElementId eid;
        {
        ECN::ECClassCP testClass = m_db->Schemas().GetECClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME);
        auto testClassInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
        // custom-handled properties
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("ModelId", ECN::ECValue((int64_t)m_defaultModelId.GetValue())));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("CategoryId", ECN::ECValue(m_defaultCategoryId.GetValue())));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("UserLabel", ECN::ECValue("my label")));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue(DPTEST_TEST_ELEMENT_TestElementProperty, ECN::ECValue("a string")));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue(DPTEST_TEST_ELEMENT_IntegerProperty1, ECN::ECValue(99)));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue(DPTEST_TEST_ELEMENT_DoubleProperty1, ECN::ECValue(99.99)));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue(DPTEST_TEST_ELEMENT_PointProperty1, ECN::ECValue(DPoint3d::From(99, 99, 99))));
        // auto-handled properties
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("IntegerProperty1", ECN::ECValue(199)));

        DgnDbStatus status;
        auto ele = m_db->Elements().CreateElement(&status, *testClassInstance);
        ASSERT_TRUE(ele.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, status);

        ASSERT_EQ((int64_t)m_defaultModelId.GetValue(), ele->GetModelId().GetValue());
        ECN::ECValue v;
        ASSERT_EQ(DgnDbStatus::Success, ele->GetProperty(v, "ModelId"));
        ASSERT_EQ((int64_t)m_defaultModelId.GetValue(), v.GetLong());
        ASSERT_EQ(DgnDbStatus::Success, ele->GetProperty(v, "UserLabel"));
        ASSERT_STREQ("my label", v.ToString().c_str());
        ASSERT_STREQ("my label", ele->GetUserLabel());
        ASSERT_EQ(DgnDbStatus::Success, ele->GetProperty(v, DPTEST_TEST_ELEMENT_TestElementProperty));
        ASSERT_STREQ("a string", v.ToString().c_str());
        ASSERT_EQ(DgnDbStatus::Success, ele->GetProperty(v, DPTEST_TEST_ELEMENT_IntegerProperty1));
        ASSERT_EQ(99, v.GetInteger());
        ASSERT_EQ(DgnDbStatus::Success, ele->GetProperty(v, DPTEST_TEST_ELEMENT_DoubleProperty1));
        ASSERT_DOUBLE_EQ(99.99, v.GetDouble());
        ASSERT_EQ(DgnDbStatus::Success, ele->GetProperty(v, DPTEST_TEST_ELEMENT_PointProperty1));
        ASSERT_TRUE(DPoint3d::From(99, 99, 99).IsEqual(v.GetPoint3D()));

        // Now, make sure that we can actually insert the element
        auto persistentEl = ele->Insert();
        ASSERT_TRUE(persistentEl.IsValid());
        
        eid = persistentEl->GetElementId();
        }

    // Now close and re-open, so that we read the element from the disk.
    auto filename = m_db->GetFileName();
    CloseDb();
    m_db = nullptr;
    OpenDb(m_db, filename, BeSQLite::Db::OpenMode::Readonly);
    auto ele = m_db->Elements().GetElement(eid);
    ASSERT_TRUE(ele.IsValid());

    ASSERT_EQ((int64_t)m_defaultModelId.GetValue(), ele->GetModelId().GetValue());
    ECN::ECValue v;
    ASSERT_EQ(DgnDbStatus::Success, ele->GetProperty(v, "ModelId"));
    ASSERT_EQ((int64_t)m_defaultModelId.GetValue(), v.GetLong());
    ASSERT_EQ(DgnDbStatus::Success, ele->GetProperty(v, "UserLabel"));
    ASSERT_STREQ("my label", v.ToString().c_str());
    ASSERT_STREQ("my label", ele->GetUserLabel());
    ASSERT_EQ(DgnDbStatus::Success, ele->GetProperty(v, DPTEST_TEST_ELEMENT_TestElementProperty));
    ASSERT_STREQ("a string", v.ToString().c_str());
    ASSERT_EQ(DgnDbStatus::Success, ele->GetProperty(v, DPTEST_TEST_ELEMENT_IntegerProperty1));
    ASSERT_EQ(99, v.GetInteger());
    ASSERT_EQ(DgnDbStatus::Success, ele->GetProperty(v, DPTEST_TEST_ELEMENT_DoubleProperty1));
    ASSERT_DOUBLE_EQ(99.99, v.GetDouble());
    ASSERT_EQ(DgnDbStatus::Success, ele->GetProperty(v, DPTEST_TEST_ELEMENT_PointProperty1));
    ASSERT_TRUE(DPoint3d::From(99, 99, 99).IsEqual(v.GetPoint3D()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
void compareDateTimes(DateTimeCR dateTime1, DateTimeCR dateTime2)
    {
    EXPECT_TRUE(dateTime1.Equals(dateTime2, false));
    EXPECT_EQ(dateTime1.GetYear(), dateTime2.GetYear());
    EXPECT_EQ(dateTime1.GetMonth(), dateTime2.GetMonth());
    EXPECT_EQ(dateTime1.GetDay(), dateTime2.GetDay());
    EXPECT_EQ(dateTime1.GetHour(), dateTime2.GetHour());
    EXPECT_EQ(dateTime1.GetMinute(), dateTime2.GetMinute());
    EXPECT_EQ(dateTime1.GetSecond(), dateTime2.GetSecond());
    EXPECT_EQ(dateTime1.GetMillisecond(), dateTime2.GetMillisecond());
    EXPECT_EQ(dateTime1.GetHectoNanosecond(), dateTime2.GetHectoNanosecond());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnElementTests, GetSetPropertyValues)
    {
    SetupSeedProject();


    DateTime dateTime = DateTime(DateTime::Kind::Utc, 2016, 2, 14, 9, 58, 17, 4560000);
    DateTime dateTimeUtc = DateTime::GetCurrentTimeUtc();
    DPoint2d point2d = {123.456, 456.789};
    DPoint3d point3d = {1.2, -3.4, 5.6};

    DgnElementId persistentId;
    if (true)
        {
        DgnElementCPtr persistentEl;

        DgnClassId classId(m_db->Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);

        ECN::AdHocJsonPropertyValue boolProperty = el.GetUserProperty("b");
        //  No user properties yet
        ECN::ECValue checkValue = boolProperty.GetValueEC();
        EXPECT_TRUE(checkValue.IsNull());

        ECN::AdHocJsonPropertyValue doubleProperty = el.GetUserProperty("d");
        checkValue = doubleProperty.GetValueEC();
        EXPECT_TRUE(checkValue.IsNull());

        ECN::AdHocJsonPropertyValue dateTimeProperty = el.GetUserProperty("dt");
        checkValue = dateTimeProperty.GetValueEC();
        EXPECT_TRUE(checkValue.IsNull());

        ECN::AdHocJsonPropertyValue dateTimeUtcProperty = el.GetUserProperty("dtUtc");
        checkValue = dateTimeUtcProperty.GetValueEC();
        EXPECT_TRUE(checkValue.IsNull());

        ECN::AdHocJsonPropertyValue intProperty = el.GetUserProperty("i");
        checkValue = intProperty.GetValueEC();
        EXPECT_TRUE(checkValue.IsNull());

        ECN::AdHocJsonPropertyValue longProperty = el.GetUserProperty("l");
        checkValue = longProperty.GetValueEC();
        EXPECT_TRUE(checkValue.IsNull());

        ECN::AdHocJsonPropertyValue stringProperty = el.GetUserProperty("s");
        checkValue = longProperty.GetValueEC();
        EXPECT_TRUE(checkValue.IsNull());

        ECN::AdHocJsonPropertyValue point2dProperty = el.GetUserProperty("p2d");
        checkValue = point2dProperty.GetValueEC();
        EXPECT_TRUE(checkValue.IsNull());

        ECN::AdHocJsonPropertyValue point3dProperty = el.GetUserProperty("p3d");
        checkValue = point3dProperty.GetValueEC();
        EXPECT_TRUE(checkValue.IsNull());

        // set user properties
        ASSERT_EQ(SUCCESS, boolProperty.SetValueEC(ECN::ECValue(false)));
        ASSERT_EQ(SUCCESS, doubleProperty.SetValueEC(ECN::ECValue(1.0001)));
        ASSERT_EQ(SUCCESS, dateTimeProperty.SetValueEC(ECN::ECValue(dateTime)));
        ASSERT_EQ(SUCCESS, dateTimeUtcProperty.SetValueEC(ECN::ECValue(dateTimeUtc)));
        ASSERT_EQ(SUCCESS, intProperty.SetValueEC(ECN::ECValue(1)));
        ASSERT_EQ(SUCCESS, longProperty.SetValueEC(ECN::ECValue(1000000000001ULL)));
        ASSERT_EQ(SUCCESS, stringProperty.SetValueEC(ECN::ECValue("StringVal")));
        ASSERT_EQ(SUCCESS, point2dProperty.SetValueEC(ECN::ECValue(point2d)));
        ASSERT_EQ(SUCCESS, point3dProperty.SetValueEC(ECN::ECValue(point3d)));

        //get property values
        checkValue = boolProperty.GetValueEC();
        EXPECT_FALSE(checkValue.GetBoolean());

        checkValue = doubleProperty.GetValueEC();
        EXPECT_EQ(1.0001, checkValue.GetDouble());

        checkValue = dateTimeProperty.GetValueEC();
        EXPECT_FALSE(checkValue.IsNull());
        compareDateTimes(dateTime, checkValue.GetDateTime());

        checkValue = dateTimeUtcProperty.GetValueEC();
        EXPECT_FALSE(checkValue.IsNull());
        compareDateTimes(dateTimeUtc, checkValue.GetDateTime());

        checkValue = intProperty.GetValueEC();
        EXPECT_EQ(1, checkValue.GetInteger());

        checkValue = longProperty.GetValueEC();
        EXPECT_EQ(1000000000001, checkValue.GetLong());

        checkValue = stringProperty.GetValueEC();
        EXPECT_STREQ("StringVal", checkValue.GetUtf8CP());

        checkValue = point2dProperty.GetValueEC();
        EXPECT_EQ(point2d.x, checkValue.GetPoint2D().x);
        EXPECT_EQ(point2d.y, checkValue.GetPoint2D().y);

        checkValue = point3dProperty.GetValueEC();
        EXPECT_EQ(point3d.x, checkValue.GetPoint3D().x);
        EXPECT_EQ(point3d.y, checkValue.GetPoint3D().y);
        EXPECT_EQ(point3d.z, checkValue.GetPoint3D().z);

        //  Insert the element
        persistentEl = el.Insert();
        persistentId = persistentEl->GetElementId();
        }

    m_db->SaveChanges("");

    m_defaultModelP->EmptyModel();
    m_db->Memory().PurgeUntil(0);

    ASSERT_TRUE(persistentId.IsValid());

    if (true)
        {
        DgnElementCPtr persistentEl = m_db->Elements().Get<DgnElement>(persistentId);

        // verify property value is persisted
        ECN::AdHocJsonPropertyValue persistedStringProperty = persistentEl->GetUserProperty("s");
        ECN::ECValue checkValue = persistedStringProperty.GetValueEC();
        EXPECT_STREQ("StringVal", checkValue.GetUtf8CP());

        // get priority
        int priority = 0;
        EXPECT_EQ(ECN::AdHocJsonPropertyValue::GetStatus::NotFound, persistedStringProperty.GetPriority(priority));
        EXPECT_EQ(0, priority);

        // get is hidden
        bool isHidden = false;
        EXPECT_EQ(ECN::AdHocJsonPropertyValue::GetStatus::NotFound, persistedStringProperty.GetHidden(isHidden));
        EXPECT_FALSE(isHidden);

        // get extended type
        Utf8String extendedType = "";
        EXPECT_EQ(ECN::AdHocJsonPropertyValue::GetStatus::NotFound, persistedStringProperty.GetExtendedType(extendedType));
        EXPECT_STREQ("", extendedType.c_str());

        // get category
        Utf8String category = "";
        EXPECT_EQ(ECN::AdHocJsonPropertyValue::GetStatus::NotFound, persistedStringProperty.GetCategory(category));
        EXPECT_STREQ("", category.c_str());

        persistedStringProperty.SetValueNull();
        checkValue = persistedStringProperty.GetValueEC();
        EXPECT_TRUE(checkValue.IsNull());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Ramanujam.Raman      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, TestUserProperties)
    {
    SetupSeedProject();
    
    // Note: Even if this test uses SetValueEC() and GetValueEC() for setting the user properties, the preference
    // would be to use the primitive setters and getters. The choice here was to just meant to get more coverage
    // by testing the outermost wrappers. 

    DgnElementId persistentId;
    if (true)
        {
        DgnElementCPtr persistentEl;

        DgnClassId classId(m_db->Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);

        ECN::AdHocJsonPropertyValue stringProperty = el.GetUserProperty("StringProperty");

        //  No user properties yet
        ECN::ECValue checkValue = stringProperty.GetValueEC();
        EXPECT_TRUE(checkValue.IsNull());

        //  Set user property (in memory)
        ASSERT_EQ(SUCCESS, stringProperty.SetValueEC(ECN::ECValue("initial value")));

        //      ... check that we see the pending value
        checkValue = stringProperty.GetValueEC();
        EXPECT_FALSE(checkValue.IsNull());
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());

        //  Insert the element
        persistentEl = el.Insert();
        persistentId = persistentEl->GetElementId();
        }
    
    m_db->SaveChanges("");

    m_defaultModelP->EmptyModel();
    m_db->Memory().PurgeUntil(0);

    ASSERT_TRUE(persistentId.IsValid());

    if (true)
        {
        DgnElementCPtr persistentEl = m_db->Elements().Get<DgnElement>(persistentId);

        ECN::AdHocJsonPropertyValue persistedStringProperty = persistentEl->GetUserProperty("StringProperty");

        // Check that we see the stored value
        ECN::ECValue checkValue = persistedStringProperty.GetValueEC();
        EXPECT_FALSE(checkValue.IsNull());
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());

        //  Get ready to modify the element
        RefCountedPtr<TestElement> editEl = persistentEl->MakeCopy<TestElement>();

        //      ... initially we still see the initial/stored value
        ECN::AdHocJsonPropertyValue  editedStringProperty = editEl->GetUserProperty("StringProperty");
        checkValue = editedStringProperty.GetValueEC();
        EXPECT_FALSE(checkValue.IsNull());
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());

        //  Set a new value (in memory)
        EXPECT_EQ(SUCCESS, editedStringProperty.SetValueEC(ECN::ECValue("changed value")));

        //      ... check that we now see the pending value on the edited copy ...
        checkValue = editedStringProperty.GetValueEC();
        EXPECT_FALSE(checkValue.IsNull());
        EXPECT_STREQ("changed value", checkValue.ToString().c_str());

        //      ... but no change on the persistent element
        checkValue = persistedStringProperty.GetValueEC();
        EXPECT_FALSE(checkValue.IsNull());
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());

        //  Update the element
        persistentEl = editEl->Update();
        }

    m_defaultModelP->EmptyModel();
    m_db->Memory().PurgeUntil(0);

    if (true)
        {
        DgnElementCPtr persistentEl = m_db->Elements().Get<DgnElement>(persistentId);
        ECN::AdHocJsonPropertyValue persistedStringProperty = persistentEl->GetUserProperty("StringProperty");

        // Check that the stored value was changed
        ECN::ECValue checkValue;
        checkValue = persistedStringProperty.GetValueEC();
        EXPECT_FALSE(checkValue.IsNull());
        EXPECT_STREQ("changed value", checkValue.ToString().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, ParentChildSameModel)
    {
    SetupSeedProject();
    
    DgnCategoryId categoryId = DgnDbTestUtils::InsertCategory(*m_db, "testCategory");
    EXPECT_TRUE(categoryId.IsValid());

    SpatialModelPtr modelA = DgnDbTestUtils::InsertSpatialModel(*m_db, DgnModel::CreateModelCode("modelA"));
    SpatialModelPtr modelB = DgnDbTestUtils::InsertSpatialModel(*m_db, DgnModel::CreateModelCode("modelB"));
    EXPECT_TRUE(modelA.IsValid());
    EXPECT_TRUE(modelB.IsValid());

    GenericPhysicalObjectPtr parentA = GenericPhysicalObject::Create(*modelA, categoryId);
    GenericPhysicalObjectPtr parentB = GenericPhysicalObject::Create(*modelB, categoryId);
    EXPECT_TRUE(parentA.IsValid());
    EXPECT_TRUE(parentB.IsValid());
    EXPECT_TRUE(parentA->Insert().IsValid());
    EXPECT_TRUE(parentB->Insert().IsValid());
    DgnElementId childIdA, childIdB, childIdC;

    // test DgnElement::_OnChildInsert success
        {
        GenericPhysicalObjectPtr childA = GenericPhysicalObject::Create(*modelA, categoryId);
        GenericPhysicalObjectPtr childB = GenericPhysicalObject::Create(*modelB, categoryId);
        GenericPhysicalObjectPtr childC = GenericPhysicalObject::Create(*modelB, categoryId);
        EXPECT_TRUE(childA.IsValid());
        EXPECT_TRUE(childB.IsValid());
        EXPECT_TRUE(childC.IsValid());
        childA->SetParentId(parentA->GetElementId()); // Match
        childB->SetParentId(parentB->GetElementId()); // Match
        EXPECT_TRUE(childA->Insert().IsValid()) << "Expecting success because models of parent and child are the same";
        EXPECT_TRUE(childB->Insert().IsValid()) << "Expecting success because models of parent and child are the same";
        EXPECT_TRUE(childC->Insert().IsValid()) << "Expecting success because childC has no parent";
        EXPECT_TRUE(childA->GetParentId().IsValid());
        EXPECT_TRUE(childB->GetParentId().IsValid());
        EXPECT_FALSE(childC->GetParentId().IsValid());
        childIdA = childA->GetElementId();
        childIdB = childB->GetElementId();
        childIdC = childC->GetElementId();
        }

    // test DgnElement::_OnChildInsert failure
        {
        GenericPhysicalObjectPtr childA = GenericPhysicalObject::Create(*modelA, categoryId);
        GenericPhysicalObjectPtr childB = GenericPhysicalObject::Create(*modelB, categoryId);
        EXPECT_TRUE(childA.IsValid());
        EXPECT_TRUE(childB.IsValid());
        childA->SetParentId(parentB->GetElementId()); // Mismatch 
        childB->SetParentId(parentA->GetElementId()); // Mismatch
        DgnDbStatus insertStatusA, insertStatusB;
        BeTest::SetFailOnAssert(false);
        EXPECT_FALSE(childA->Insert(&insertStatusA).IsValid()) << "Expecting failure because models of parent and child are different";
        EXPECT_FALSE(childB->Insert(&insertStatusB).IsValid()) << "Expecting failure because models of parent and child are different";
        BeTest::SetFailOnAssert(true);
        EXPECT_EQ(DgnDbStatus::WrongModel, insertStatusA);
        EXPECT_EQ(DgnDbStatus::WrongModel, insertStatusB);
        }

    // test SetParentId on existing elements
        {
        GenericPhysicalObjectPtr childA = m_db->Elements().GetForEdit<GenericPhysicalObject>(childIdA);
        GenericPhysicalObjectPtr childB = m_db->Elements().GetForEdit<GenericPhysicalObject>(childIdB);
        GenericPhysicalObjectPtr childC = m_db->Elements().GetForEdit<GenericPhysicalObject>(childIdC);
        EXPECT_TRUE(childA.IsValid());
        EXPECT_TRUE(childB.IsValid());
        EXPECT_TRUE(childC.IsValid());
        childA->SetParentId(parentB->GetElementId()); // Mismatch
        childB->SetParentId(parentA->GetElementId()); // Mismatch
        childC->SetParentId(parentA->GetElementId()); // Mismatch
        DgnDbStatus updateStatusA, updateStatusB, updateStatusC;
        BeTest::SetFailOnAssert(false);
        EXPECT_FALSE(childA->Update(&updateStatusA).IsValid()) << "Expecting failure because models of parent and child are different";
        EXPECT_FALSE(childB->Update(&updateStatusB).IsValid()) << "Expecting failure because models of parent and child are different";
        EXPECT_FALSE(childC->Update(&updateStatusC).IsValid()) << "Expecting failure because models of parent and child are different";
        BeTest::SetFailOnAssert(true);
        EXPECT_EQ(DgnDbStatus::WrongModel, updateStatusA);
        EXPECT_EQ(DgnDbStatus::WrongModel, updateStatusB);
        EXPECT_EQ(DgnDbStatus::WrongModel, updateStatusC);

        childC->SetParentId(parentB->GetElementId()); // Match
        EXPECT_TRUE(childC->Update(&updateStatusC).IsValid()) << "Expecting success because models of parent and child are the same";
        EXPECT_EQ(DgnDbStatus::Success, updateStatusC);
        }
    }
