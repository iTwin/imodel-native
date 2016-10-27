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

    //Inserts a model
    PhysicalModelPtr m1 = InsertPhysicalModel("Model1");
    EXPECT_TRUE(m1.IsValid());
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

    //Inserts a model
    PhysicalModelPtr m1 = InsertPhysicalModel("Model1");
    EXPECT_TRUE(m1.IsValid());
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
    TestElementPtr child = TestElement::Create(*m_db, parent.GetModelId(), m_defaultCategoryId);
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
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    DgnModelId modelId = model->GetModelId();

    if (true)
        {
        DgnElementCPtr parent1 = TestElement::Create(*m_db, modelId, m_defaultCategoryId)->Insert();
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
        DgnElementCPtr parent1 = TestElement::Create(*m_db, modelId, m_defaultCategoryId)->Insert();
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
        DgnElementCPtr parent1 = TestElement::Create(*m_db, modelId, m_defaultCategoryId)->Insert();
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
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    DgnModelId modelId = model->GetModelId();

    DgnElementCPtr parent1 = TestElement::Create(*m_db, modelId, m_defaultCategoryId)->Insert();
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
        DgnElementPtr nonPersistent = TestElement::Create(*m_db, modelId, m_defaultCategoryId);

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
    PhysicalModelPtr model = GetDefaultPhysicalModel();

    DgnElementCPtr parent = TestElement::Create(*m_db, model->GetModelId(), m_defaultCategoryId)->Insert();
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
    PhysicalModelPtr model = GetDefaultPhysicalModel();

    DgnElementCPtr group = TestGroup::Create(*m_db, model->GetModelId(), m_defaultCategoryId)->Insert();
    DgnElementCPtr m1 = TestElement::Create(*m_db, model->GetModelId(), m_defaultCategoryId)->Insert();
    DgnElementCPtr m2 = TestElement::Create(*m_db, model->GetModelId(), m_defaultCategoryId)->Insert();
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
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    DgnModelId modelId = model->GetModelId();
    DgnCategoryId categoryId = GetDefaultCategoryId();
    DgnClassId classId = m_db->Domains().GetClassId(generic_ElementHandler::GenericPhysicalObjectHandler::GetHandler());
    DgnElementId elementId;

    // Test creating an element the "normal" way (by letting the DgnElementId be assigned by the framework)
        {
        GenericPhysicalObjectPtr element = new GenericPhysicalObject(GenericPhysicalObject::CreateParams(*m_db, modelId, classId, categoryId));
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        ASSERT_TRUE(element->GetElementId().IsValid());
        ASSERT_FALSE(element->Insert().IsValid()) << "Second insert of the same element should fail";
        elementId = element->GetElementId();
        }

    DgnElementId forcedElementId(elementId.GetValue() + 100);

    // Confirm that supplying a DgnElementId in CreateParams for Insert does not work (not intended to work)
        {
        GenericPhysicalObject::CreateParams createParams(*m_db, modelId, classId, categoryId);
        createParams.SetElementId(DgnElementId(elementId.GetValue() + 100));
    
        GenericPhysicalObjectPtr element = new GenericPhysicalObject(createParams);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->Insert().IsValid()) << "It is not valid to supply a DgnElementId for Insert via CreateParams";
        }

    // Test PKPM's synchronization workflow where they must force a DgnElementId on Insert.
        {
        GenericPhysicalObjectPtr element = new GenericPhysicalObject(GenericPhysicalObject::CreateParams(*m_db, modelId, classId, categoryId));
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
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    DgnCategoryId categoryId = GetDefaultCategoryId();

    // GenericSpatialLocation
        {
        GenericSpatialLocationPtr element = GenericSpatialLocation::Create(*model, categoryId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        ASSERT_TRUE(element->GetElementId().IsValid());
        }

    // GenericPhysicalObject
        {
        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(*model, categoryId);
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
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, s_autoHandledPropNames[i]));
        EXPECT_TRUE(checkValue.IsNull());
        }

    // Check a few non-auto-handled props
    ECN::ECValue checkValue;
    EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "TestIntegerProperty1"));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementTests::TestAutoHandledPropertiesGetSet()
    {
    RefCountedCPtr<TestElement> persistentEl;
    uint32_t iArrayOfPoint3d;
    uint32_t iArrayOfString;
    uint32_t iArrayOfInt;
    if (true)
        {
        DgnClassId classId(m_db->Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);

        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(iArrayOfPoint3d, "ArrayOfPoint3d"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(iArrayOfString, "ArrayOfString"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(iArrayOfInt, "ArrayOfInt"));

        //  No unhandled properties yet
        ECN::ECValue checkValue;
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "StringProperty"));
        EXPECT_TRUE(checkValue.IsNull());

        //  Set unhandled property (in memory)
        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue("StringProperty", ECN::ECValue("initial value")));

        //      ... check that we see the pending value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "StringProperty"));
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());

        // Set a struct valued property
        BeTest::SetFailOnAssert(false);
        EXPECT_NE(DgnDbStatus::Success, el.SetPropertyValue("Location", ECN::ECValue("<<you cannot set a struct directly>>")));
        BeTest::SetFailOnAssert(true);
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.Street", ECN::ECValue("690 Pennsylvania Drive")));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.City.Name", ECN::ECValue("Exton")));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.City.State", ECN::ECValue("PA")));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.City.Zip", ECN::ECValue(19341)));

        // Set an array property
        EXPECT_EQ(DgnDbStatus::Success, el.AddPropertyArrayItems(iArrayOfString, 3));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfString", ECN::ECValue("first"), PropertyArrayIndex(0)));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfString", ECN::ECValue("second"), PropertyArrayIndex(1)));

        EXPECT_EQ(DgnDbStatus::Success, el.AddPropertyArrayItems(iArrayOfPoint3d, 2));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(1,1,1)), PropertyArrayIndex(0)));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(2,2,2)), PropertyArrayIndex(1)));

        EXPECT_EQ(DgnDbStatus::Success, el.AddPropertyArrayItems(iArrayOfInt, 300));
        for (auto i=0; i<300; ++i)
            {
            EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfInt", ECN::ECValue(i), PropertyArrayIndex(i)));
            }

        //  Insert the element
        persistentEl = m_db->Elements().Insert(el);
        }
    
    ASSERT_TRUE(persistentEl.IsValid());

    // Check that we see the stored value
    ECN::ECValue checkValue;
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.Street"));
    EXPECT_STREQ("690 Pennsylvania Drive", checkValue.ToString().c_str());

    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.City.Name"));
    EXPECT_STREQ("Exton", checkValue.ToString().c_str());

    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.City.Country"));
    EXPECT_TRUE(checkValue.IsNull()) << "I never set the Location.City.Country property, so it should be null";

    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.City.Zip"));
    EXPECT_EQ(19341, checkValue.GetInteger());

    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(0)));
    EXPECT_STREQ("first", checkValue.ToString().c_str());

    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(1)));
    EXPECT_STREQ("second", checkValue.ToString().c_str());

    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(2)));
    EXPECT_TRUE(checkValue.IsNull());

    EXPECT_NE(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(3)));

    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(0)));
    EXPECT_TRUE(checkValue.GetPoint3d().IsEqual(DPoint3d::From(1,1,1)));

    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(1)));
    EXPECT_TRUE(checkValue.GetPoint3d().IsEqual(DPoint3d::From(2,2,2)));

    EXPECT_NE(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(2)));

    for (auto i=0; i<300; ++i)
        {
        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfInt", PropertyArrayIndex(i)));
        EXPECT_EQ(i, checkValue.GetInteger());
        }

    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "StringProperty"));
    EXPECT_STREQ("initial value", checkValue.ToString().c_str());

    // Get some non-auto-handled properties using the same dynamic property API
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ModelId"));
    EXPECT_EQ(persistentEl->GetModelId().GetValue(), checkValue.GetLong());
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "CategoryId"));
    EXPECT_EQ(persistentEl->GetCategoryId().GetValue(), checkValue.GetLong());
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "UserLabel"));
    EXPECT_STREQ(persistentEl->GetUserLabel(), checkValue.ToString().c_str());
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "CodeAuthorityId"));
    EXPECT_EQ(persistentEl->GetCode().GetAuthority().GetValueUnchecked(), (uint64_t)checkValue.GetLong());
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "CodeNamespace"));
    EXPECT_STREQ(persistentEl->GetCode().GetNamespace().c_str(), checkValue.ToString().c_str());
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "CodeValue"));
    EXPECT_STREQ(persistentEl->GetCode().GetValue().c_str(), checkValue.ToString().c_str());

    if (true)
        {
        //  Get ready to modify the element
        RefCountedPtr<TestElement> editEl = persistentEl->MakeCopy<TestElement>();

        //      ... initially we still see the initial/stored value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "StringProperty"));
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());

        //  Set a new value (in memory)
        EXPECT_EQ(DgnDbStatus::Success, editEl->SetPropertyValue("StringProperty", ECN::ECValue("changed value")));

        //      ... check that we now see the pending value on the edited copy ...
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "StringProperty"));
        EXPECT_STREQ("changed value", checkValue.ToString().c_str());

        //      ... but no change on the persistent element
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "StringProperty"));
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());

        //  Update the element
        persistentEl = m_db->Elements().Update(*editEl);
        }

    // Check that the stored value was changed
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "StringProperty"));
    EXPECT_STREQ("changed value", checkValue.ToString().c_str());

    // REALLY check that the stored value was changed
    auto fileName = m_db->GetFileName();
    auto elid = persistentEl->GetElementId();
    persistentEl = nullptr;
    m_db->SaveChanges();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    persistentEl = m_db->Elements().Get<TestElement>(elid);
    ASSERT_TRUE(persistentEl.IsValid());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "StringProperty"));
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
        DgnCode code = DgnCode::CreateEmpty();
        // custom-handled properties
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("ModelId", ECN::ECValue((int64_t)m_defaultModelId.GetValue())));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("CategoryId", ECN::ECValue(m_defaultCategoryId.GetValue())));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("UserLabel", ECN::ECValue("my label")));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("CodeAuthorityId", ECN::ECValue(code.GetAuthority().GetValue())));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("CodeNamespace", ECN::ECValue(code.GetNamespace().c_str())));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("CodeValue", ECN::ECValue(code.GetValueCP())));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue(DPTEST_TEST_ELEMENT_TestElementProperty, ECN::ECValue("a string")));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue(DPTEST_TEST_ELEMENT_IntegerProperty1, ECN::ECValue(99)));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue(DPTEST_TEST_ELEMENT_DoubleProperty1, ECN::ECValue(99.99)));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue(DPTEST_TEST_ELEMENT_PointProperty1, ECN::ECValue(DPoint3d::From(99, 99, 99))));
        
        // auto-handled properties
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("IntegerProperty1", ECN::ECValue(199)));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("Location.Street", ECN::ECValue("690 Pennsylvania Drive")));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("Location.City.Name", ECN::ECValue("Exton")));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("Location.City.State", ECN::ECValue("PA")));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("Location.City.Zip", ECN::ECValue(19341)));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->AddArrayElements("ArrayOfString", 3));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("ArrayOfString", ECN::ECValue("first"), 0));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("ArrayOfString", ECN::ECValue("second"), 1));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->AddArrayElements("ArrayOfPoint3d", 2));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(1,1,1)), 0));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(2,2,2)), 1));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->AddArrayElements("ArrayOfInt", 300));
        for (auto i=0; i<300; ++i)
            {
            ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("ArrayOfInt", ECN::ECValue(i), i));
            }

        DgnDbStatus status;
        auto ele = m_db->Elements().CreateElement(&status, *testClassInstance);
        ASSERT_TRUE(ele.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, status);

        ASSERT_EQ((int64_t)m_defaultModelId.GetValue(), ele->GetModelId().GetValue());
        ECN::ECValue v;
        ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "ModelId"));
        ASSERT_EQ((int64_t)m_defaultModelId.GetValue(), v.GetLong());
        ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "UserLabel"));
        ASSERT_STREQ("my label", v.ToString().c_str());
        ASSERT_STREQ("my label", ele->GetUserLabel());
        ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, DPTEST_TEST_ELEMENT_TestElementProperty));
        ASSERT_STREQ("a string", v.ToString().c_str());
        ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, DPTEST_TEST_ELEMENT_IntegerProperty1));
        ASSERT_EQ(99, v.GetInteger());
        ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, DPTEST_TEST_ELEMENT_DoubleProperty1));
        ASSERT_DOUBLE_EQ(99.99, v.GetDouble());
        ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, DPTEST_TEST_ELEMENT_PointProperty1));
        ASSERT_TRUE(DPoint3d::From(99, 99, 99).IsEqual(v.GetPoint3d()));
        EXPECT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "ArrayOfPoint3d", PropertyArrayIndex(1)));
        EXPECT_TRUE(v.GetPoint3d().IsEqual(DPoint3d::From(2,2,2)));

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
    ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "ModelId"));
    ASSERT_EQ((int64_t)m_defaultModelId.GetValue(), v.GetLong());
    ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "UserLabel"));
    ASSERT_STREQ("my label", v.ToString().c_str());
    ASSERT_STREQ("my label", ele->GetUserLabel());
    ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, DPTEST_TEST_ELEMENT_TestElementProperty));
    ASSERT_STREQ("a string", v.ToString().c_str());
    ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, DPTEST_TEST_ELEMENT_IntegerProperty1));
    ASSERT_EQ(99, v.GetInteger());
    ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, DPTEST_TEST_ELEMENT_DoubleProperty1));
    ASSERT_DOUBLE_EQ(99.99, v.GetDouble());
    ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, DPTEST_TEST_ELEMENT_PointProperty1));
    ASSERT_TRUE(DPoint3d::From(99, 99, 99).IsEqual(v.GetPoint3d()));
    EXPECT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "Location.Street"));
    EXPECT_STREQ("690 Pennsylvania Drive", v.ToString().c_str());

    EXPECT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "Location.City.Name"));
    EXPECT_STREQ("Exton", v.ToString().c_str());

    EXPECT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "Location.City.Country"));
    EXPECT_TRUE(v.IsNull()) << "I never set the Location.City.Country property, so it should be null";

    EXPECT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "Location.City.Zip"));
    EXPECT_EQ(19341, v.GetInteger());

    EXPECT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "ArrayOfString", PropertyArrayIndex(0)));
    EXPECT_STREQ("first", v.ToString().c_str());

    EXPECT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "ArrayOfString", PropertyArrayIndex(1)));
    EXPECT_STREQ("second", v.ToString().c_str());

    EXPECT_NE(DgnDbStatus::Success, ele->GetPropertyValue(v, "ArrayOfString", PropertyArrayIndex(2))); // I only set two items

    EXPECT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "ArrayOfPoint3d", PropertyArrayIndex(0)));
    EXPECT_TRUE(v.GetPoint3d().IsEqual(DPoint3d::From(1,1,1)));

    EXPECT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "ArrayOfPoint3d", PropertyArrayIndex(1)));
    EXPECT_TRUE(v.GetPoint3d().IsEqual(DPoint3d::From(2,2,2)));

    EXPECT_NE(DgnDbStatus::Success, ele->GetPropertyValue(v, "ArrayOfPoint3d", PropertyArrayIndex(2)));

    for (auto i=0; i<300; ++i)
        {
        EXPECT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "ArrayOfInt", PropertyArrayIndex(i)));
        EXPECT_EQ(i, v.GetInteger());
        }
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
        EXPECT_EQ(point2d.x, checkValue.GetPoint2d().x);
        EXPECT_EQ(point2d.y, checkValue.GetPoint2d().y);

        checkValue = point3dProperty.GetValueEC();
        EXPECT_EQ(point3d.x, checkValue.GetPoint3d().x);
        EXPECT_EQ(point3d.y, checkValue.GetPoint3d().y);
        EXPECT_EQ(point3d.z, checkValue.GetPoint3d().z);

        //  Insert the element
        persistentEl = el.Insert();
        persistentId = persistentEl->GetElementId();
        }

    m_db->SaveChanges("");
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
    PhysicalModelPtr modelA = DgnDbTestUtils::InsertPhysicalModel(*m_db, DgnModel::CreateModelCode("modelA"));
    PhysicalModelPtr modelB = DgnDbTestUtils::InsertPhysicalModel(*m_db, DgnModel::CreateModelCode("modelB"));

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, FederationGuid)
    {
    SetupSeedProject();

    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, DgnModel::CreateModelCode("TestModel"));
    DgnCategoryId categoryId = DgnDbTestUtils::InsertCategory(*m_db, "TestCategory");

    DgnElementId elementId;
    BeGuid federationGuid(true);
    BeGuid updatedFederationGuid(true);

    // Quick BeGuid tests
    EXPECT_TRUE(BeGuid(true).IsValid());
    EXPECT_FALSE(BeGuid().IsValid());
    EXPECT_NE(federationGuid, updatedFederationGuid);
    EXPECT_EQ(federationGuid, BeGuid(federationGuid.m_guid.u[0], federationGuid.m_guid.u[1]));
    EXPECT_EQ(federationGuid, BeGuid(federationGuid.m_guid.i[0], federationGuid.m_guid.i[1], federationGuid.m_guid.i[2], federationGuid.m_guid.i[3]));
    
    // test that FederationGuid is initialized as invalid
        {
        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(*model, categoryId);
        EXPECT_TRUE(element.IsValid());
        EXPECT_FALSE(element->GetFederationGuid().IsValid()) << "FederationGuid expected to be initialized as invalid";
        EXPECT_TRUE(element->Insert().IsValid());
        elementId = element->GetElementId();
        EXPECT_FALSE(element->GetFederationGuid().IsValid()) << "FederationGuid expected to be initialized as invalid";
        }

    // test error conditions for QueryElementByFederationGuid
    EXPECT_FALSE(m_db->Elements().QueryElementByFederationGuid(BeGuid()).IsValid()) << "Should not find an element by an invalid FederationGuid";
    EXPECT_FALSE(m_db->Elements().QueryElementByFederationGuid(BeGuid(true)).IsValid()) << "Should not find an element by an unused FederationGuid";

    // flush cache and re-check element
        {
        m_db->Memory().PurgeUntil(0);
        DgnElementCPtr element = m_db->Elements().GetElement(elementId);
        EXPECT_TRUE(element.IsValid());
        EXPECT_FALSE(element->GetFederationGuid().IsValid());
        }

    // test when FederationGuid is specified
        {
        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(*model, categoryId);
        EXPECT_TRUE(element.IsValid());
        element->SetFederationGuid(federationGuid);
        EXPECT_TRUE(element->GetFederationGuid().IsValid()) << "FederationGuid should be valid after SetFederationGuid";
        EXPECT_TRUE(element->Insert().IsValid());
        elementId = element->GetElementId();
        EXPECT_EQ(elementId.GetValue(), m_db->Elements().QueryElementByFederationGuid(federationGuid)->GetElementId().GetValue()) << "Should be able to query for an element by its FederationGuid";

        GenericPhysicalObjectPtr element2 = GenericPhysicalObject::Create(*model, categoryId);
        EXPECT_TRUE(element2.IsValid());
        element2->SetFederationGuid(federationGuid);
        EXPECT_FALSE(element->Insert().IsValid()) << "Cannot insert a second element with a duplicate FederationGuid";
        m_db->SaveChanges();
        }

    // flush cache and re-check element
        {
        m_db->Memory().PurgeUntil(0);
        DgnElementPtr element = m_db->Elements().GetForEdit<DgnElement>(elementId);
        EXPECT_TRUE(element.IsValid());
        EXPECT_TRUE(element->GetFederationGuid().IsValid());
        EXPECT_EQ(element->GetFederationGuid(), federationGuid);

        element->SetFederationGuid(updatedFederationGuid);
        EXPECT_TRUE(element->Update().IsValid());
        m_db->SaveChanges();
        }

    // flush cache and verify set/update to updated value
        {
        m_db->Memory().PurgeUntil(0);
        DgnElementPtr element = m_db->Elements().GetForEdit<DgnElement>(elementId);
        EXPECT_TRUE(element.IsValid());
        EXPECT_TRUE(element->GetFederationGuid().IsValid());
        EXPECT_EQ(element->GetFederationGuid(), updatedFederationGuid);
        EXPECT_TRUE(m_db->Elements().QueryElementByFederationGuid(updatedFederationGuid).IsValid());
        EXPECT_FALSE(m_db->Elements().QueryElementByFederationGuid(federationGuid).IsValid());

        element->SetFederationGuid(BeGuid()); // set FederationGuid to null
        EXPECT_TRUE(element->Update().IsValid());
        m_db->SaveChanges();
        }

    // flush cache and verify set/update to null value
        {
        m_db->Memory().PurgeUntil(0);
        DgnElementCPtr element = m_db->Elements().GetElement(elementId);
        EXPECT_TRUE(element.IsValid());
        EXPECT_FALSE(element->GetFederationGuid().IsValid());
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, PhysicalTemplateCRUD)
    {
    SetupSeedProject();
    DgnElementId physicalTemplateId[3];

    // insert some sample PhysicalTemplates
    for (int32_t i=0; i<_countof(physicalTemplateId); i++)
        {
        TestPhysicalTemplatePtr physicalTemplate = TestPhysicalTemplate::Create(m_db->GetDictionaryModel());
        ASSERT_TRUE(physicalTemplate.IsValid());
        physicalTemplate->SetUserLabel(Utf8PrintfString("PhysicalTemplate%d", i).c_str());
        ASSERT_TRUE(physicalTemplate->Insert().IsValid());
        physicalTemplateId[i] = physicalTemplate->GetElementId();
        }

    // flush cache to make sure PhysicalTemplates were inserted properly
        {
        m_db->Memory().PurgeUntil(0);
        for (int32_t i=0; i<_countof(physicalTemplateId); i++)
            {
            TestPhysicalTemplateCPtr physicalTemplate = m_db->Elements().Get<TestPhysicalTemplate>(physicalTemplateId[i]);
            ASSERT_TRUE(physicalTemplate.IsValid());
            ASSERT_FALSE(physicalTemplate->GetSubModel().IsValid());

            PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*physicalTemplate, DgnModel::CreateModelCode(physicalTemplate->GetUserLabel()));
            ASSERT_TRUE(model.IsValid());
            ASSERT_TRUE(physicalTemplate->GetSubModel().IsValid());
            }
        }

    // flush cache to make sure PhysicalTemplate subModels were inserted properly
        {
        m_db->Memory().PurgeUntil(0);
        for (int32_t i=0; i<_countof(physicalTemplateId); i++)
            {
            TestPhysicalTemplateCPtr physicalTemplate = m_db->Elements().Get<TestPhysicalTemplate>(physicalTemplateId[i]);
            ASSERT_TRUE(physicalTemplate.IsValid());
            ASSERT_TRUE(physicalTemplate->GetSubModel().IsValid());
            ASSERT_TRUE(physicalTemplate->GetSubModel()->IsTemplate());
            ASSERT_TRUE(physicalTemplate->GetSub<PhysicalModel>().IsValid());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, PhysicalTypeCRUD)
    {
    SetupSeedProject();

    DgnElementId physicalTypeId[3];
    DgnElementId elementId;

    // insert some sample PhysicalTypes
    for (int32_t i=0; i<_countof(physicalTypeId); i++)
        {
        TestPhysicalTypePtr physicalType = TestPhysicalType::Create(*m_db);
        ASSERT_TRUE(physicalType.IsValid());
        physicalType->SetUserLabel(Utf8PrintfString("PhysicalType%d", i).c_str());
        physicalType->SetStringProperty(Utf8PrintfString("String%d", i).c_str());
        physicalType->SetIntProperty(i);
        ASSERT_TRUE(physicalType->Insert().IsValid());
        physicalTypeId[i] = physicalType->GetElementId();
        }

    // flush cache to make sure PhysicalTypes were inserted properly
        {
        m_db->Memory().PurgeUntil(0);
        for (int32_t i=0; i<_countof(physicalTypeId); i++)
            {
            TestPhysicalTypePtr physicalType = m_db->Elements().GetForEdit<TestPhysicalType>(physicalTypeId[i]);
            ASSERT_TRUE(physicalType.IsValid());
            ASSERT_STREQ(physicalType->GetStringProperty().c_str(), Utf8PrintfString("String%d", i).c_str());
            ASSERT_EQ(physicalType->GetIntProperty(), i);

            physicalType->SetStringProperty(Utf8PrintfString("Updated%d", i).c_str());
            physicalType->SetIntProperty(i+100);
            ASSERT_TRUE(physicalType->Update().IsValid());
            }
        }

    // flush cache to make sure PhysicalTypes were updated properly
        {
        m_db->Memory().PurgeUntil(0);
        for (int32_t i=0; i<_countof(physicalTypeId); i++)
            {
            TestPhysicalTypeCPtr physicalType = m_db->Elements().Get<TestPhysicalType>(physicalTypeId[i]);
            ASSERT_TRUE(physicalType.IsValid());
            ASSERT_STREQ(physicalType->GetStringProperty().c_str(), Utf8PrintfString("Updated%d", i).c_str());
            ASSERT_EQ(physicalType->GetIntProperty(), i+100);
            }
        }

    // create a PhysicalElement and set its PhysicalType
        {
        DgnCategoryId categoryId = DgnDbTestUtils::InsertCategory(*m_db, "TestCategory");
        PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, DgnModel::CreateModelCode("TestModel"));

        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(*model, categoryId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->GetPhysicalTypeId().IsValid());
        ASSERT_FALSE(element->GetPhysicalType().IsValid());
        element->SetPhysicalType(physicalTypeId[0]);
        ASSERT_TRUE(element->GetPhysicalTypeId().IsValid());
        ASSERT_TRUE(element->GetPhysicalType().IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        elementId = element->GetElementId();
        }

    // flush cache to make sure the PhysicalElement's PhysicalType was inserted properly
        {
        m_db->Memory().PurgeUntil(0);
        GenericPhysicalObjectPtr element = m_db->Elements().GetForEdit<GenericPhysicalObject>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->GetPhysicalType().IsValid());
        ASSERT_EQ(element->GetPhysicalTypeId().GetValue(), physicalTypeId[0].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, element->SetPhysicalType(physicalTypeId[1]));
        ASSERT_TRUE(element->Update().IsValid());
        }

    // flush cache to make sure the PhysicalElement's PhysicalType was updated properly
        {
        m_db->Memory().PurgeUntil(0);
        GenericPhysicalObjectPtr element = m_db->Elements().GetForEdit<GenericPhysicalObject>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->GetPhysicalType().IsValid());
        ASSERT_EQ(element->GetPhysicalTypeId().GetValue(), physicalTypeId[1].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, element->SetPhysicalType(DgnElementId()));
        ASSERT_TRUE(element->Update().IsValid());
        }

    // flush cache to make sure the PhysicalElement's PhysicalType was cleared properly
        {
        m_db->Memory().PurgeUntil(0);
        GenericPhysicalObjectPtr element = m_db->Elements().GetForEdit<GenericPhysicalObject>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->GetPhysicalType().IsValid());
        ASSERT_FALSE(element->GetPhysicalTypeId().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, GraphicalType2dCRUD)
    {
    SetupSeedProject();

    DgnElementId graphicalTypeId[3];
    DgnElementId elementId;

    // insert some sample GraphicalType2ds
    for (int32_t i=0; i<_countof(graphicalTypeId); i++)
        {
        TestGraphicalType2dPtr graphicalType = TestGraphicalType2d::Create(*m_db);
        ASSERT_TRUE(graphicalType.IsValid());
        graphicalType->SetUserLabel(Utf8PrintfString("GraphicalType2d%d", i).c_str());
        graphicalType->SetStringProperty(Utf8PrintfString("String%d", i).c_str());
        graphicalType->SetIntProperty(i);
        ASSERT_TRUE(graphicalType->Insert().IsValid());
        graphicalTypeId[i] = graphicalType->GetElementId();
        }

    // flush cache to make sure GraphicalTypes were inserted properly
        {
        m_db->Memory().PurgeUntil(0);
        for (int32_t i=0; i<_countof(graphicalTypeId); i++)
            {
            TestGraphicalType2dPtr graphicalType = m_db->Elements().GetForEdit<TestGraphicalType2d>(graphicalTypeId[i]);
            ASSERT_TRUE(graphicalType.IsValid());
            ASSERT_STREQ(graphicalType->GetStringProperty().c_str(), Utf8PrintfString("String%d", i).c_str());
            ASSERT_EQ(graphicalType->GetIntProperty(), i);

            graphicalType->SetStringProperty(Utf8PrintfString("Updated%d", i).c_str());
            graphicalType->SetIntProperty(i+100);
            ASSERT_TRUE(graphicalType->Update().IsValid());
            }
        }

    // flush cache to make sure GraphicalTypes were updated properly
        {
        m_db->Memory().PurgeUntil(0);
        for (int32_t i=0; i<_countof(graphicalTypeId); i++)
            {
            TestGraphicalType2dCPtr graphicalType = m_db->Elements().Get<TestGraphicalType2d>(graphicalTypeId[i]);
            ASSERT_TRUE(graphicalType.IsValid());
            ASSERT_STREQ(graphicalType->GetStringProperty().c_str(), Utf8PrintfString("Updated%d", i).c_str());
            ASSERT_EQ(graphicalType->GetIntProperty(), i+100);
            }
        }

    // create a TestElement2d and set its GraphicalType
        {
        DgnCategoryId categoryId = DgnDbTestUtils::InsertCategory(GetDgnDb(), "TestCategory");
        DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(GetDgnDb(), DgnModel::CreateModelCode("DrawingListModel"));
        DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, DgnCode(), "Drawing");
        DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing, DgnModel::CreateModelCode("DrawingModel"));

        TestElement2dPtr element = TestElement2d::Create(GetDgnDb(), drawingModel->GetModelId(), categoryId, DgnCode(), 2.0);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->GetGraphicalTypeId().IsValid());
        ASSERT_FALSE(element->GetGraphicalType().IsValid());
        element->SetGraphicalType(graphicalTypeId[0]);
        ASSERT_TRUE(element->GetGraphicalTypeId().IsValid());
        ASSERT_TRUE(element->GetGraphicalType().IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        elementId = element->GetElementId();
        }

    // flush cache to make sure the TestElement2d's GraphicalType was inserted properly
        {
        m_db->Memory().PurgeUntil(0);
        TestElement2dPtr element = m_db->Elements().GetForEdit<TestElement2d>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->GetGraphicalType().IsValid());
        ASSERT_EQ(element->GetGraphicalTypeId().GetValue(), graphicalTypeId[0].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, element->SetGraphicalType(graphicalTypeId[1]));
        ASSERT_TRUE(element->Update().IsValid());
        }

    // flush cache to make sure the TestElement2d's GraphicalType was updated properly
        {
        m_db->Memory().PurgeUntil(0);
        TestElement2dPtr element = m_db->Elements().GetForEdit<TestElement2d>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->GetGraphicalType().IsValid());
        ASSERT_EQ(element->GetGraphicalTypeId().GetValue(), graphicalTypeId[1].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, element->SetGraphicalType(DgnElementId()));
        ASSERT_TRUE(element->Update().IsValid());
        }

    // flush cache to make sure the TestElement2d's GraphicalType was cleared properly
        {
        m_db->Memory().PurgeUntil(0);
        TestElement2dPtr element = m_db->Elements().GetForEdit<TestElement2d>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->GetGraphicalType().IsValid());
        ASSERT_FALSE(element->GetGraphicalTypeId().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, EqualsTests)
    {
    SetupSeedProject();

    DgnCategoryId categoryId = DgnDbTestUtils::InsertCategory(*m_db, "testCategory");
    PhysicalModelPtr modelA = DgnDbTestUtils::InsertPhysicalModel(*m_db, DgnModel::CreateModelCode("modelA"));
    PhysicalModelPtr modelB = DgnDbTestUtils::InsertPhysicalModel(*m_db, DgnModel::CreateModelCode("modelB"));

    GenericPhysicalObjectPtr elementA = GenericPhysicalObject::Create(*modelA, categoryId);
    ASSERT_TRUE(elementA.IsValid());
    
    ASSERT_TRUE(elementA->Equals(*elementA)) << " An element should be equivalent to itself";
    ASSERT_TRUE(elementA->Equals(*elementA->CopyForEdit())) << " An element should be equivalent to a copy of itself";
    
    GenericPhysicalObjectPtr elementB = GenericPhysicalObject::Create(*modelB, categoryId);
    ASSERT_TRUE(elementB.IsValid());
    ASSERT_FALSE(elementA->Equals(*elementB)) << " ModelIds should differ";
    bset<Utf8String> ignoreProps;
    ignoreProps.insert("ModelId");
    DgnElement::ComparePropertyFilter filter(ignoreProps);
    ASSERT_TRUE(elementA->Equals(*elementB, filter));

    elementB->SetUserLabel("label for b");
    ASSERT_FALSE(elementA->Equals(*elementB, filter)) << " UserLabels should differ";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(DgnElementTests, DemoArrayProblem)
    {
    SetupSeedProject();

    ECN::ECSchemaReadContextPtr   schemaContext = ECN::ECSchemaReadContext::CreateContext();

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Arrays' version='01.00' displayLabel='Arrays' description='Schema with array properties class' nameSpacePrefix='ams' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
            "<ECSchemaReference name=\"Generic\" version=\"01.00.00\" prefix=\"generic\"/>"
            "<ECStructClass typeName=\"JDRange3d\" displayLabel=\"JDRange3d\">"
                "<ECProperty propertyName=\"xMin\" typeName=\"double\" displayLabel=\"xMin\"/>"
                "<ECProperty propertyName=\"yMin\" typeName=\"double\" displayLabel=\"yMin\"/>"
                "<ECProperty propertyName=\"zMin\" typeName=\"double\" displayLabel=\"zMin\"/>"
                "<ECProperty propertyName=\"xMax\" typeName=\"double\" displayLabel=\"xMax\"/>"
                "<ECProperty propertyName=\"yMax\" typeName=\"double\" displayLabel=\"yMax\"/>"
                "<ECProperty propertyName=\"zMax\" typeName=\"double\" displayLabel=\"zMax\"/>"
            "</ECStructClass>"
            "<ECStructClass typeName=\"JDTransform3d\" displayLabel=\"JDTransform3d\">"
                "<ECProperty propertyName=\"M00\" typeName=\"double\" displayLabel=\"M00\"/>"
                "<ECProperty propertyName=\"M01\" typeName=\"double\" displayLabel=\"M01\"/>"
                "<ECProperty propertyName=\"M02\" typeName=\"double\" displayLabel=\"M02\"/>"
                "<ECProperty propertyName=\"M03\" typeName=\"double\" displayLabel=\"M03\"/>"
                "<ECProperty propertyName=\"M04\" typeName=\"double\" displayLabel=\"M04\"/>"
                "<ECProperty propertyName=\"M05\" typeName=\"double\" displayLabel=\"M05\"/>"
                "<ECProperty propertyName=\"M06\" typeName=\"double\" displayLabel=\"M06\"/>"
                "<ECProperty propertyName=\"M07\" typeName=\"double\" displayLabel=\"M07\"/>"
                "<ECProperty propertyName=\"M08\" typeName=\"double\" displayLabel=\"M08\"/>"
                "<ECProperty propertyName=\"M09\" typeName=\"double\" displayLabel=\"M09\"/>"
                "<ECProperty propertyName=\"M10\" typeName=\"double\" displayLabel=\"M10\"/>"
                "<ECProperty propertyName=\"M11\" typeName=\"double\" displayLabel=\"M11\"/>"
                "<ECProperty propertyName=\"M12\" typeName=\"double\" displayLabel=\"M12\"/>"
                "<ECProperty propertyName=\"M13\" typeName=\"double\" displayLabel=\"M13\"/>"
                "<ECProperty propertyName=\"M14\" typeName=\"double\" displayLabel=\"M14\"/>"
                "<ECProperty propertyName=\"M15\" typeName=\"double\" displayLabel=\"M15\"/>"
            "</ECStructClass>"
            "<ECEntityClass typeName=\"AMS_BASE\" displayLabel=\"AMS_BASE\">"
                "<BaseClass>generic:PhysicalObject</BaseClass>"
                "<ECArrayProperty propertyName=\"SNAP_POINTS\" typeName=\"point3d\" displayLabel=\"SNAP_POINTS\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                "<ECStructProperty propertyName=\"TMATRIX\" typeName=\"JDTransform3d\" displayLabel=\"TMATRIX\"/>"
            "</ECEntityClass>"
            "<ECEntityClass typeName=\"PIPE_BASE\" displayLabel=\"PIPE_BASE\">"
                "<BaseClass>AMS_BASE</BaseClass>"
            "</ECEntityClass>"
            "<ECEntityClass typeName=\"JSPACE_OBJECT\" displayLabel=\"JSPACE_OBJECT\">"
                "<BaseClass>generic:PhysicalObject</BaseClass>"
            "</ECEntityClass>"
            "<ECEntityClass typeName=\"GRAPHICS\" displayLabel=\"GRAPHICS\">"
                "<BaseClass>JSPACE_OBJECT</BaseClass>"
                "<ECArrayProperty propertyName=\"ATTRIBUTES\" typeName=\"int\" displayLabel=\"ATTRIBUTES\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                "<ECStructProperty propertyName=\"RANGE\" typeName=\"JDRange3d\" displayLabel=\"RANGE\"/>"
                "<ECProperty propertyName=\"LEVEL_NAME\" typeName=\"string\" displayLabel=\"LEVEL_NAME\"/>"
            "</ECEntityClass>"
            "<ECEntityClass typeName=\"GROUP\" displayLabel=\"GROUP\">"
                "<BaseClass>GRAPHICS</BaseClass>"
                "<ECProperty propertyName=\"NAME\" typeName=\"string\" displayLabel=\"NAME\"/>"
            "</ECEntityClass>"
            "<ECEntityClass typeName=\"PHYSICAL_GROUP\" displayLabel=\"PHYSICAL_GROUP\">"
                "<BaseClass>GROUP</BaseClass>"
                "<ECStructProperty propertyName=\"MATRIX\" typeName=\"JDTransform3d\" displayLabel=\"MATRIX\"/>"
            "</ECEntityClass>"
            "<ECEntityClass typeName=\"AMS_BASE_USER\" displayLabel=\"AMS_BASE_USER\">"
                "<BaseClass>PHYSICAL_GROUP</BaseClass>"
                "<ECProperty propertyName=\"SNP_LENGTH\" typeName=\"double\" displayLabel=\"SNP_LENGTH\"/>"
                "<ECProperty propertyName=\"CLIP_LENGTH\" typeName=\"double\" displayLabel=\"CLIP_LENGTH\"/>"
                "<ECProperty propertyName=\"ELEMENT_ID\" typeName=\"string\" displayLabel=\"ELEMENT_ID\"/>"
                "<ECProperty propertyName=\"BOM_FLAG\" typeName=\"int\" displayLabel=\"BOM_FLAG\"/>"
            "</ECEntityClass>"
            "<ECEntityClass typeName=\"BOLT_BASE_USER\" displayLabel=\"BOLT_BASE_USER\">"
                "<BaseClass>generic:PhysicalObject</BaseClass>"
                "<ECProperty propertyName=\"CREATES_BOLTS\" typeName=\"boolean\" displayLabel=\"CREATES_BOLTS\"/>"
                "<ECProperty propertyName=\"NEEDS_GASKETS\" typeName=\"boolean\" displayLabel=\"NEEDS_GASKETS\"/>"
            "</ECEntityClass>"
            "<ECEntityClass typeName=\"PIPE_BASE_USER\" displayLabel=\"PIPE_BASE_USER\">"
                "<BaseClass>AMS_BASE_USER</BaseClass>"
                "<BaseClass>BOLT_BASE_USER</BaseClass>"
                "<ECProperty propertyName=\"COMP_DESC\" typeName=\"string\" displayLabel=\"COMP_DESC\"/>"
                "<ECProperty propertyName=\"ZZ_VLV_TYPE_DESC\" typeName=\"string\" displayLabel=\"ZZ_VLV_TYPE_DESC\"/>"
                "<ECProperty propertyName=\"L_DESC1\" typeName=\"string\" displayLabel=\"L_DESC1\"/>"
                "<ECProperty propertyName=\"L_DESC2\" typeName=\"string\" displayLabel=\"L_DESC2\"/>"
                "<ECProperty propertyName=\"REPLACEMENT_OBJECT\" typeName=\"string\" displayLabel=\"REPLACEMENT_OBJECT\"/>"
                "<ECProperty propertyName=\"ZZ_BR_TYPE_DESC\" typeName=\"string\" displayLabel=\"ZZ_BR_TYPE_DESC\"/>"
                "<ECProperty propertyName=\"ZZ_FL_FACE_DESC\" typeName=\"string\" displayLabel=\"ZZ_FL_FACE_DESC\"/>"
                "<ECProperty propertyName=\"ZZ_FL_TYPE_DESC\" typeName=\"string\" displayLabel=\"ZZ_FL_TYPE_DESC\"/>"
                "<ECProperty propertyName=\"ZZ_STR_TYPE_DESC\" typeName=\"string\" displayLabel=\"ZZ_STR_TYPE_DESC\"/>"
                "<ECProperty propertyName=\"ZZ_STYLE_DESC\" typeName=\"string\" displayLabel=\"ZZ_STYLE_DESC\"/>"
                "<ECProperty propertyName=\"ZZ_CONN1_DESC\" typeName=\"string\" displayLabel=\"ZZ_CONN1_DESC\"/>"
                "<ECProperty propertyName=\"ZZ_CONN2_DESC\" typeName=\"string\" displayLabel=\"ZZ_CONN2_DESC\"/>"
                "<ECProperty propertyName=\"ZZ_QUANTITY\" typeName=\"double\" displayLabel=\"ZZ_QUANTITY\"/>"
                "<ECProperty propertyName=\"NETWORK_CONNECTIVITY\" typeName=\"boolean\" displayLabel=\"NETWORK_CONNECTIVITY\"/>"
                "<ECProperty propertyName=\"OVERLAY\" typeName=\"string\" displayLabel=\"OVERLAY\"/>"
            "</ECEntityClass>"
            "<ECEntityClass typeName = \"SPECCHK_BASE_USER\" displayLabel=\"SPECCHK_BASE_USER\">"
                "<BaseClass>generic:PhysicalObject</BaseClass>"
                "<ECProperty propertyName=\"SPECCHK_OK\" typeName=\"string\" displayLabel=\"SPECCHK_OK\"/>"
                "<ECArrayProperty propertyName=\"SPECCHK\" typeName=\"string\" displayLabel=\"SPECCHK\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                "<ECProperty propertyName=\"WHERE_CLAUSE\" typeName=\"string\" displayLabel=\"WHERE_CLAUSE\"/>"
            "</ECEntityClass>"
            "<ECEntityClass typeName=\"CONN_PREP_2X2\" displayLabel=\"CONN_PREP_2X2\">"
                "<BaseClass>generic:PhysicalObject</BaseClass>"
                "<ECProperty propertyName=\"CONN_PREP\" typeName=\"int\" displayLabel=\"CONN_PREP\"/>"
            "</ECEntityClass>"
            "<ECEntityClass typeName=\"PIPE_PVLV_USER\" displayLabel=\"PIPE_PVLV_USER\">"
                "<BaseClass>PIPE_BASE_USER</BaseClass>"
                "<BaseClass>SPECCHK_BASE_USER</BaseClass>"
                "<BaseClass>CONN_PREP_2X2</BaseClass>"
                "<ECProperty propertyName=\"COMP_DESC\" typeName=\"string\" displayLabel=\"COMP_DESC\"/>"
                "<ECProperty propertyName=\"SERVICE\" typeName=\"string\" displayLabel=\"SERVICE\"/>"
                "<ECProperty propertyName=\"TAG\" typeName=\"string\" displayLabel=\"TAG\"/>"
                "<ECProperty propertyName=\"SIZE\" typeName=\"double\" displayLabel=\"SIZE\"/>"
                "<ECProperty propertyName=\"WHICH_APPL\" typeName=\"string\" displayLabel=\"WHICH_APPL\"/>"
                "<ECProperty propertyName=\"LINE\" typeName=\"string\" displayLabel=\"LINE\"/>"
                "<ECProperty propertyName=\"CONN_PREP\" typeName=\"int\" displayLabel=\"CONN_PREP\"/>"
                "<ECProperty propertyName=\"WHERE_CLAUSE\" typeName=\"string\" displayLabel=\"WHERE_CLAUSE\"/>"
                "<ECProperty propertyName=\"SH_DESC\" typeName=\"string\" displayLabel=\"SH_DESC\"/>"
                "<ECProperty propertyName=\"L_DESC1\" typeName=\"string\" displayLabel=\"L_DESC1\"/>"
                "<ECProperty propertyName=\"L_DESC2\" typeName=\"string\" displayLabel=\"L_DESC2\"/>"
                "<ECProperty propertyName=\"WEIGHT\" typeName=\"double\" displayLabel=\"WEIGHT\"/>"
                "<ECProperty propertyName=\"NOSPEC_CLAUSE\" typeName=\"string\" displayLabel=\"NOSPEC_CLAUSE\"/>"
                "<ECProperty propertyName=\"OP_TYPE\" typeName=\"string\" displayLabel=\"OP_TYPE\"/>"
                "<ECProperty propertyName=\"WHERE_CLAUSE_OP\" typeName=\"string\" displayLabel=\"WHERE_CLAUSE_OP\"/>"
                "<ECProperty propertyName=\"L_DESC3\" typeName=\"string\" displayLabel=\"L_DESC3\"/>"
                "<ECProperty propertyName=\"INLINE\" typeName=\"boolean\" displayLabel=\"INLINE\"/>"
                "<ECArrayProperty propertyName=\"FL_TYPE\" typeName=\"string\" displayLabel=\"FL_TYPE\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
                "<ECProperty propertyName=\"FOR_BOLTS_EPREP1\" typeName=\"string\" displayLabel=\"FOR_BOLTS_EPREP1\"/>"
            "</ECEntityClass>"
            "<ECEntityClass typeName=\"PIPE_PVLV\" displayLabel=\"PIPE_PVLV\">"
                "<BaseClass>PIPE_BASE</BaseClass>"
                "<BaseClass>PIPE_PVLV_USER</BaseClass>"
                "<ECProperty propertyName=\"OP_TYPE\" typeName=\"string\" displayLabel=\"OP_TYPE\"/>"
                "<ECProperty propertyName=\"BUD_TYPE\" typeName=\"string\" displayLabel=\"BUD_TYPE\"/>"
                "<ECProperty propertyName=\"COMPTYPE\" typeName=\"string\" displayLabel=\"COMPTYPE\"/>"
                "<ECProperty propertyName=\"SCH_COND\" typeName=\"string\" displayLabel=\"SCH_COND\"/>"
            "</ECEntityClass>"
        "</ECSchema>";

    schemaContext->AddSchemaLocater(m_db->GetSchemaLocater());
    ECN::ECSchemaPtr schema;
    ECN::ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);

    schemaContext->RemoveSchemaLocater(m_db->GetSchemaLocater());

    m_db->Schemas().ImportECSchemas(schemaContext->GetCache().GetSchemas());
    m_db->SaveChanges();

    Utf8CP json =
        "{"
        "\"$ECClassId\" : \"819\","
        "\"$ECClassKey\" : \"ams.PIPE_PVLV\","
        "\"$ECClassLabel\" : \"PIPE_PVLV\","
        "\"$ECInstanceId\" : \"576\","
        "\"$ECInstanceLabel\" : \"V_CHEC\","
        "\"AREA_NO\" : null,"
        "\"ATTRIBUTES\" : [0, 0, 0, 0, 0, 0, 0, -1, -1, 0],"
        "\"BBoxHigh\" : null,"
        "\"BBoxLow\" : null,"
        "\"BOM_FLAG\" : 1,"
        "\"BUD_TYPE\" : \"PIPE_PVLV\","
        "\"CLIP_LENGTH\" : 1.3333333730697632,"
        "\"COMPTYPE\" : \"PVLV\","
        "\"COMP_DESC\" : \"VALVE, , , 150# CLASS\","
        "\"CONN_PREP\" : null,"
        "\"CREATES_BOLTS\" : false,"
        "\"CategoryId\" : \"398\","
        "\"CodeAuthorityId\" : \"1\","
        "\"CodeNamespace\" : \"\","
        "\"CodeValue\" : null,"
        "\"ELEMENT_ID\" : \"22000\","
        "\"FL_TYPE\" : [\"#\"],"
        "\"FOR_BOLTS_EPREP1\" : \"FL\","
        "\"GUID\" : null,"
        "\"GeometryStream\" : null,"
        "\"INLINE\" : false,"
        "\"InSpatialIndex\" : true,"
        "\"LEVEL_NAME\" : null,"
        "\"LINE\" : \"105\","
        "\"L_DESC1\" : null,"
        "\"L_DESC2\" : \"\","
        "\"L_DESC3\" : \"\","
        "\"MATRIX\" : {"
            "\"M00\" : 1.0,"
            "\"M01\" : 0.0,"
            "\"M02\" : 0.0,"
            "\"M03\" : 0.0,"
            "\"M04\" : 0.0,"
            "\"M05\" : 1.0,"
            "\"M06\" : 0.0,"
            "\"M07\" : 0.0,"
            "\"M08\" : 0.0,"
            "\"M09\" : 0.0,"
            "\"M10\" : 1.0,"
            "\"M11\" : 0.0,"
            "\"M12\" : 0.0,"
            "\"M13\" : 0.0,"
            "\"M14\" : 0.0,"
            "\"M15\" : 1.0"
            "},"
        "\"ModelId\" : \"4\","
        "\"NAME\" : \"V_CHEC\","
        "\"NEEDS_GASKETS\" : true,"
        "\"NETWORK_CONNECTIVITY\" : true,"
        "\"NOSPEC_CLAUSE\" : \"  EPREP1 = 'FL' AND SCH_RAT1 = ' ' AND EPREP2 = 'FL' AND SCH_RAT2 = ' ' AND SIZE_1 = 6 AND SIZE_2 = 6 AND STNDRD = 'ANSI' AND CODE = 'B16.10' AND REIHE = '#' AND VFL_TYPE = '#' AND PATTERN = '#' AND TYPE = 'CHEC' AND RATING= 150 AND MAT_NAME = 'STL' AND FACE = 'RF'\","
        "\"OP_TYPE\" : \"#\","
        "\"OVERLAY\" : \"\","
        "\"Origin\" : null,"
        "\"ParentId\" : null,"
        "\"Pitch\" : null,"
        "\"RANGE\" : {"
            "\"xMax\" : -100000000000.0,"
            "\"xMin\" : 100000000000.0,"
            "\"yMax\" : -100000000000.0,"
            "\"yMin\" : 100000000000.0,"
            "\"zMax\" : -100000000000.0,"
            "\"zMin\" : 100000000000.0"
            "},"
        "\"REPLACEMENT_OBJECT\" : null,"
        "\"Roll\" : null,"
        "\"SCH_COND\" : null,"
        "\"SERVICE\" : \"W\","
        "\"SHOP_FLD\" : null,"
        "\"SH_DESC\" : null,"
        "\"SIZE\" : 6.0,"
        "\"SNAP_POINTS\" : ["
            "{"
            "\"x\" : 1170.0000000000002,"
            "\"y\" : 421.07800434914088,"
            "\"z\" : 15.499959933500962"
            "},"
            "{"
            "\"x\" : 1170.0000000000002,"
            "\"y\" : 421.07800434914088,"
            "\"z\" : 16.833293266834296"
            "}"
        "],"
        "\"SNP_LENGTH\" : 1.3333333730697632,"
        "\"SPECCHK\" : [],"
        "\"SPECCHK_OK\" : \"FALSE\","
        "\"TAG\" : \"V407\","
        "\"TMATRIX\" : {"
            "\"M00\" : 0.0,"
            "\"M01\" : 0.0,"
            "\"M02\" : 1.0,"
            "\"M03\" : 0.0,"
            "\"M04\" : 6.1230317691118863e-17,"
            "\"M05\" : 1.0,"
            "\"M06\" : 0.0,"
            "\"M07\" : 0.0,"
            "\"M08\" : -1.0,"
            "\"M09\" : 6.1230317691118863e-17,"
            "\"M10\" : 0.0,"
            "\"M11\" : 0.0,"
            "\"M12\" : 1170.0000000000002,"
            "\"M13\" : 421.07800434914088,"
            "\"M14\" : 15.499959933500962,"
            "\"M15\" : 1.0"
            "},"
        "\"UserLabel\" : \"V_CHEC\","
        "\"UserProperties\" : null,"
        "\"WEIGHT\" : null,"
        "\"WHERE_CLAUSE\" : \" WHERE SPEC = '1C' AND   EPREP1 = 'FL' AND SCH_RAT1 = ' ' AND EPREP2 = 'FL' AND SCH_RAT2 = ' ' AND SIZE_1 = 6 AND SIZE_2 = 6 AND STNDRD = 'ANSI' AND CODE = 'B16.10' AND REIHE = '#' AND VFL_TYPE = '#' AND PATTERN = '#' AND TYPE = 'CHEC' AND RATING= 150 AND MAT_NAME = 'STL' AND FACE = 'RF'\","
        "\"WHERE_CLAUSE_OP\" : \" WHERE SPEC = '1C' AND  EPREP1 = 'FL' AND SCH_RAT1 = ' ' AND  SIZE_1 = 6 AND STNDRD = 'ANSI' AND CODE = 'B16.10' AND OPERATOR= '#' AND TYPE = 'CHEC' AND RATING= 150 AND MAT_NAME = 'STL'\","
        "\"WHICH_APPL\" : \"PS Piping\","
        "\"Yaw\" : null,"
        "\"ZZ_BR_TYPE_DESC\" : null,"
        "\"ZZ_CONN1_DESC\" : null,"
        "\"ZZ_CONN2_DESC\" : null,"
        "\"ZZ_FL_FACE_DESC\" : null,"
        "\"ZZ_FL_TYPE_DESC\" : null,"
        "\"ZZ_QUANTITY\" : 1.0,"
        "\"ZZ_STR_TYPE_DESC\" : null,"
        "\"ZZ_STYLE_DESC\" : null,"
        "\"ZZ_VLV_TYPE_DESC\" : null,"
        "\"ams_CODE_\" : null"
        "}";
    Json::Value jsonInput;
    ASSERT_TRUE(Json::Reader::Parse(json, jsonInput));

    ECN::ECClassCP ecClass = schema->GetClassCP("PIPE_PVLV");
    ASSERT_TRUE(nullptr != ecClass);
    ECN::IECInstancePtr ecInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ASSERT_TRUE(ecInstance.IsValid());
    ASSERT_EQ(SUCCESS, ECN::ECJsonUtilities::ECInstanceFromJson(*ecInstance, jsonInput));

    DgnCode code = DgnCode::CreateEmpty();
    // custom-handled properties
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ecInstance->SetValue("ModelId", ECN::ECValue((int64_t) m_defaultModelId.GetValue())));
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ecInstance->SetValue("CategoryId", ECN::ECValue(m_defaultCategoryId.GetValue())));
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ecInstance->SetValue("UserLabel", ECN::ECValue("my label")));
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ecInstance->SetValue("CodeAuthorityId", ECN::ECValue(code.GetAuthority().GetValue())));
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ecInstance->SetValue("CodeNamespace", ECN::ECValue(code.GetNamespace().c_str())));
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ecInstance->SetValue("CodeValue", ECN::ECValue(code.GetValueCP())));

    DgnDbStatus stat;
    DgnElementPtr dgnElement = m_db->Elements().CreateElement(&stat, *ecInstance);
    DgnElementCPtr inserted = dgnElement->Insert(&stat);
    ASSERT_TRUE(inserted != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(DgnElementTests, AutoHandleViewDefinition)
    {
    SetupSeedProject();

    Utf8CP json =
        "{"
        "\"$ECClassId\" : \"198\","
        "\"$ECClassKey\" : \"BisCore.CameraViewDefinition\","
        "\"$ECClassLabel\" : \"CameraViewDefinition\","
        "\"$ECInstanceId\" : \"502\","
        "\"$ECInstanceLabel\" : \"CameraViewDefinition\","
        "\"CodeAuthorityId\" : \"4\","
        "\"CodeNamespace\" : \"ViewDefinition\","
        "\"CodeValue\" : \"Default - View 1\","
        "\"Descr\" : \"\","
        "\"Extents\" : {"
            "\"x\" : 85.413445258737553,"
            "\"y\" : 76.125601109667528,"
            "\"z\" : 112.79558108349732"
        "},"
        "\"EyePoint\" : {"
            "\"x\" : 293.99476935528162,"
            "\"y\" : 69.335060236322079,"
            "\"z\" : 68.339134990346963"
        "},"
        "\"FocusDistance\" : 100.61073354297713,"
        "\"LensAngle\" : 0.80285145591739238,"
        "\"ModelId\" : \"16\","
        "\"Origin\" : {"
            "\"x\" : 338.90639657040640,"
            "\"y\" : 174.64311379840612,"
            "\"z\" : -53.387925168591018"
        "},"
        "\"ParentId\" : null,"
        "\"Pitch\" : -35.264389682754654,"
        "\"Roll\" : -45.000000000000007,"
        "\"Source\" : 2,"
        "\"UserLabel\" : null,"
        "\"UserProperties\" : null,"
        "\"Yaw\" : 29.999999999999986"
    "}";

    ECN::ECClassCP viewDefClass = m_db->Schemas().GetECClass("BisCore", "CameraViewDefinition");
    ASSERT_TRUE(nullptr != viewDefClass);
    ECN::IECInstancePtr ecInstance = viewDefClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ASSERT_TRUE(ecInstance.IsValid());
    
    Json::Value jsonInput;
    ASSERT_TRUE(Json::Reader::Parse(json, jsonInput));
    ASSERT_EQ(SUCCESS, ECN::ECJsonUtilities::ECInstanceFromJson(*ecInstance, jsonInput));

    DgnDbStatus stat;
    DgnElementPtr dgnElement = m_db->Elements().CreateElement(&stat, *ecInstance);
    DgnElementCPtr inserted = dgnElement->Insert(&stat);
    ASSERT_TRUE(inserted != nullptr);
    m_db->Schemas().CreateECClassViewsInDb();
    m_db->SaveChanges();

    CameraViewDefinitionCPtr camera = m_db->Elements().Get<CameraViewDefinition>(inserted->GetElementId());
    ASSERT_TRUE(camera.IsValid());
    DPoint3d eyepoint = camera->GetEyePoint();
//    ASSERT_EQ(293.99476935528162, eyepoint.x);
    }
