/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <ECObjects/ECJsonUtilities.h>
USING_NAMESPACE_BENTLEY_DPTEST

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct DgnElementTests : public DgnDbTestFixture
    {
    TestElementCPtr AddChild(DgnElementCR parent);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnElementTests, InsertElement)
    {
    SetupSeedProject();

    //Inserts a model
    PhysicalModelPtr m1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Model1");
    EXPECT_TRUE(m1.IsValid());
    m_db->SaveChanges("changeSet1");

    auto keyE1 = InsertElement(m1->GetModelId());
    DgnElementId e1id = keyE1->GetElementId();
    DgnElementCPtr e1 = m_db->Elements().GetElement(e1id);
    EXPECT_TRUE(e1 != nullptr);

    // Now update without making any change => LASTMOD should change anyway.
    uint64_t insertTime;
    ASSERT_EQ(BSISUCCESS, e1->QueryLastModifyTime().ToJulianDay(insertTime));

    BeThreadUtilities::BeSleep(1); // NB! LASTMOD resolution is 1 millisecond!

    auto ed1 = e1->CopyForEdit();
    ed1->Update();
    e1 = m_db->Elements().GetElement(e1id);
    uint64_t updateTime;
    ASSERT_EQ(BSISUCCESS, e1->QueryLastModifyTime().ToJulianDay(updateTime));
    EXPECT_NE(insertTime, updateTime);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementCPtr DgnElementTests::AddChild(DgnElementCR parent)
    {
    TestElementPtr child = TestElement::Create(*m_db, parent.GetModelId(), m_defaultCategoryId);
    child->SetParentId(parent.GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
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
    DgnClassId classId = m_db->Domains().GetClassId(generic_ElementHandler::PhysicalObject::GetHandler());
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

    m_db->Elements().ClearCache();

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

    DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    TestElement el(params);
    EXPECT_TRUE(el.Insert().IsValid());

    EXPECT_EQ(0, countElementsOfClass(TestElement::QueryClassId(*m_db), *m_db));
    EXPECT_EQ(1, countElementsOfClass(classId, *m_db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, TestAutoHandledPropertiesCA)
    {
    SetupSeedProject();
    // *** test Custom Attributes when we get them
    DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
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

    for (int i = 0; i<_countof(s_autoHandledPropNames); ++i)
    {
        ECN::ECValue checkValue;
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, s_autoHandledPropNames[i]));
        EXPECT_TRUE(checkValue.IsNull());
    }

    // Check a few non-auto-handled props
    ECN::ECValue checkValue;
    EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "TestIntegerProperty1"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, GetSetAutoHandledProperties)
    {
    SetupSeedProject();
    DgnElementId elementId;
    uint32_t datetime;
    uint32_t datetimeutc;
    uint32_t boolean;
    uint32_t p2d;
    uint32_t p3d;
    DateTime dateTime = DateTime(2016, 9, 24);
    ECN::ECValue checkValue;
    if (true)
        {
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);

        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(boolean, "b"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(datetime, "dt"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(datetimeutc, "dtUtc"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(p2d, "p2d"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(p3d, "p3d"));

        // get properties
        ASSERT_EQ(false, el.GetPropertyValueBoolean("b"));
        ASSERT_TRUE(!(el.GetPropertyValueDateTime("dt").IsValid()));
        ASSERT_TRUE(!(el.GetPropertyValueDateTime("dtUtc").IsValid()));
        ASSERT_TRUE(el.GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 0))));
        ASSERT_TRUE(el.GetPropertyValueDPoint3d("p3d").IsEqual((DPoint3d::From(0, 0, 0))));

        // No unhandled properties yet
        ECN::ECValue checkValue;
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "StringProperty"));
        EXPECT_TRUE(checkValue.IsNull());

        // Set unhandled property (in memory)
        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue("StringProperty", ECN::ECValue("initial value")));

        // check that we see the pending value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "StringProperty"));
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());

        // Set properties
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("b", false));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("dt", dateTime));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("dtUtc", dateTime));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("p2d", DPoint2d::From(0, 9)));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("p3d", DPoint3d::From(0, 9, 9)));
        // Insert the element
        DgnDbStatus stat;
        DgnElementCPtr persistentEl = el.Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(persistentEl.IsValid());

        // Check that we see the stored value in memory
        EXPECT_EQ(false, persistentEl->GetPropertyValueBoolean("b"));

        EXPECT_TRUE(persistentEl->GetPropertyValueDateTime("dt").Equals(dateTime, true));

        EXPECT_TRUE(persistentEl->GetPropertyValueDateTime("dtUtc").Equals(dateTime, true));

        EXPECT_EQ(DPoint2d::From(0, 9), persistentEl->GetPropertyValueDPoint2d("p2d"));

        EXPECT_EQ(DPoint3d::From(0, 9, 9), persistentEl->GetPropertyValueDPoint3d("p3d"));

        elementId = persistentEl->GetElementId();
        m_db->SaveChanges();
        }

    // Before updating the element check what is stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    TestElementPtr element = m_db->Elements().GetForEdit<TestElement>(elementId);
    EXPECT_EQ(false, element->GetPropertyValueBoolean("b"));

    EXPECT_TRUE(element->GetPropertyValueDateTime("dt").Equals(dateTime, true));

    EXPECT_TRUE(element->GetPropertyValueDateTime("dtUtc").Equals(dateTime, true));

    EXPECT_EQ(DPoint2d::From(0, 9), element->GetPropertyValueDPoint2d("p2d"));

    EXPECT_EQ(DPoint3d::From(0, 9, 9), element->GetPropertyValueDPoint3d("p3d"));
    // Get some non-auto-handled properties using the same dynamic property API
    EXPECT_EQ(element->GetModelId(), element->GetPropertyValueId<DgnModelId>("Model"));
    EXPECT_EQ(element->GetCategoryId(), element->GetPropertyValueId<DgnCategoryId>("Category"));
    EXPECT_STREQ(element->GetUserLabel(), element->GetPropertyValueString("UserLabel").c_str());
    EXPECT_EQ(element->GetCode().GetCodeSpecId(), element->GetPropertyValueId<CodeSpecId>("CodeSpec"));
    EXPECT_EQ(element->GetCode().GetScopeElementId(*m_db), element->GetPropertyValueId<DgnElementId>("CodeScope"));
    EXPECT_STREQ(element->GetCode().GetValueUtf8().c_str(), element->GetPropertyValueString("CodeValue").c_str());
    }

    if (true)
        {
        // Get ready to modify the element
         TestElementPtr editEl = m_db->Elements().GetForEdit<TestElement>(elementId);

        // initially we still see the initial/stored value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "StringProperty"));
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());
        EXPECT_EQ(false, editEl->GetPropertyValueBoolean("b"));
        // Set a new value (in memory)
        EXPECT_EQ(DgnDbStatus::Success, editEl->SetPropertyValue("StringProperty", ECN::ECValue("changed value")));
        EXPECT_EQ(DgnDbStatus::Success, editEl->SetPropertyValue("b", true));
        // check that we now see the pending value on the edited copy ...
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "StringProperty"));
        EXPECT_STREQ("changed value", checkValue.ToString().c_str());
        EXPECT_EQ(true, editEl->GetPropertyValueBoolean("b"));
        // Update the element
        DgnDbStatus stat;
        DgnElementCPtr updated_element = editEl->Update(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(updated_element.IsValid());
        m_db->SaveChanges();
        }

    // REALLY check that the stored value was changed
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    TestElementCPtr Element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(Element.IsValid());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, Element->GetPropertyValue(checkValue, "StringProperty"));
    EXPECT_STREQ("changed value", checkValue.ToString().c_str());
    EXPECT_EQ(true, Element->GetPropertyValueBoolean("b"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan        1/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, FederationGuid_Update)
    {
    SetupSeedProject();
    DgnElementId elementId;
    DgnDbStatus stat;
    //Insert element
        {
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);
        DgnElementCPtr persistentEl = el.Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(persistentEl.IsValid());
        elementId = persistentEl->GetElementId();
        }

    BeGuid fedId(true);
        {
        // Get ready to modify the element
        TestElementPtr editEl = m_db->Elements().GetForEdit<TestElement>(elementId);
        ASSERT_TRUE(editEl.IsValid());

        editEl->SetFederationGuid(fedId);
        DgnDbStatus stat;
        DgnElementCPtr updated_element = editEl->Update(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(updated_element.IsValid());
        EXPECT_EQ(fedId, updated_element->GetFederationGuid());
        }

    TestElementCPtr el = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(el.IsValid());
    EXPECT_EQ(fedId, el->GetFederationGuid());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, GetSetAutoHandledStructProperties)
    {
    SetupSeedProject();
    DgnElementId elementId;
    uint32_t LocStruct;
    ECN::ECValue checkValue;
    if (true)
        {
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);

        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(LocStruct, "Location"));

        // get Struct properties before setting
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "Location"));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "Location.Street"));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "Location.City.Name"));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "Location.City.State"));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "Location.City.Country"));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "Location.City.Zip"));
        EXPECT_TRUE(checkValue.IsNull());

        // check that we see the pending value
        checkValue.Clear();
        // Set a struct valued property in memory
        BeTest::SetFailOnAssert(false);
        EXPECT_NE(DgnDbStatus::Success, el.SetPropertyValue("Location", ECN::ECValue("<<you cannot set a struct directly>>")));
        BeTest::SetFailOnAssert(true);
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.Street", ECN::ECValue("690 Pennsylvania Drive")));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.City.Name", ECN::ECValue("Exton")));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.City.State", ECN::ECValue("PA")));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.City.Country", ECN::ECValue("US")));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.City.Zip", ECN::ECValue(19341)));
        // Insert the element
        DgnDbStatus stat;
        DgnElementCPtr persistentEl = el.Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(persistentEl.IsValid());
        // Check that we see the stored value in memory

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.Street"));
        EXPECT_STREQ("690 Pennsylvania Drive", checkValue.ToString().c_str());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.City.Name"));
        EXPECT_STREQ("Exton", checkValue.ToString().c_str());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.City.State"));
        EXPECT_STREQ("PA", checkValue.ToString().c_str());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.City.Country"));
        EXPECT_STREQ("US", checkValue.ToString().c_str());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.City.Zip"));
        EXPECT_EQ(19341, checkValue.GetInteger());
        elementId = persistentEl->GetElementId();
        m_db->SaveChanges();
        }
    // Before updatation of element check what stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    TestElementPtr element = m_db->Elements().GetForEdit<TestElement>(elementId);
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "Location.Street"));
    EXPECT_STREQ("690 Pennsylvania Drive", checkValue.ToString().c_str());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "Location.City.Name"));
    EXPECT_STREQ("Exton", checkValue.ToString().c_str());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "Location.City.State"));
    EXPECT_STREQ("PA", checkValue.ToString().c_str());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "Location.City.Country"));
    EXPECT_STREQ("US", checkValue.ToString().c_str());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "Location.City.Zip"));
    EXPECT_EQ(19341, checkValue.GetInteger());
    }
    if (true)
        {
        // Get ready to modify the element
        TestElementPtr editEl = m_db->Elements().GetForEdit<TestElement>(elementId);

        // initially we still see the initial/stored value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "Location.City.Zip"));
        EXPECT_EQ(19341, checkValue.GetInteger());

        // Set a new value (in memory)
        EXPECT_EQ(DgnDbStatus::Success, editEl->SetPropertyValue("Location.City.Zip", ECN::ECValue(19342)));

        // check that we now see the pending value on the edited copy ...
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "Location.City.Zip"));
        EXPECT_EQ(19342, checkValue.GetInteger());
        // Update the element
        DgnDbStatus stat;
        DgnElementCPtr updated_element = editEl->Update(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(updated_element.IsValid());
        m_db->SaveChanges();
        }

    // REALLY check that the stored value was changed
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    TestElementCPtr Element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(Element.IsValid());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, Element->GetPropertyValue(checkValue, "Location.City.Zip"));
    EXPECT_EQ(19342, checkValue.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, GetSetAutoHandledArrayProperties)
    {
    SetupSeedProject();
    DgnElementId elementId;
    uint32_t iArrayOfPoint3d;
    uint32_t iArrayOfString;
    uint32_t iArrayOfInt;
    if (true)
    {
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);

        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(iArrayOfPoint3d, "ArrayOfPoint3d"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(iArrayOfString, "ArrayOfString"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(iArrayOfInt, "ArrayOfInt"));

        // Set an array property
        EXPECT_EQ(DgnDbStatus::Success, el.InsertPropertyArrayItems(iArrayOfString, 0, 4));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfString", ECN::ECValue("first"), PropertyArrayIndex(1)));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfString", ECN::ECValue("second"), PropertyArrayIndex(2)));

        EXPECT_EQ(DgnDbStatus::Success, el.AddPropertyArrayItems(iArrayOfPoint3d, 2));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(1, 1, 1)), PropertyArrayIndex(0)));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(2, 2, 2)), PropertyArrayIndex(1)));
        EXPECT_NE(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(2, 2, 2)), PropertyArrayIndex(2)));

        EXPECT_EQ(DgnDbStatus::Success, el.AddPropertyArrayItems(iArrayOfInt, 300));
        for (auto i = 0; i < 300; ++i)
        {
            EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfInt", ECN::ECValue(i), PropertyArrayIndex(i)));
        }
        // Clear all array property index from memory before clear first Get them
        ECN::ECValue checkValue;
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(0)));
        EXPECT_TRUE(checkValue.GetPoint3d().IsEqual(DPoint3d::From(1, 1, 1)));
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(1)));
        EXPECT_TRUE(checkValue.GetPoint3d().IsEqual(DPoint3d::From(2, 2, 2)));
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, el.ClearPropertyArray(iArrayOfPoint3d));
        EXPECT_NE(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(0)));
        EXPECT_TRUE(checkValue.IsNull());
        EXPECT_NE(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(1)));
        EXPECT_TRUE(checkValue.IsNull());
        EXPECT_EQ(DgnDbStatus::Success, el.InsertPropertyArrayItems(iArrayOfPoint3d, 0, 1));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(1, 1, 1)), PropertyArrayIndex(0)));
        EXPECT_NE(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(2, 2, 2)), PropertyArrayIndex(1)));

        // Insert the element
        DgnDbStatus stat;
        DgnElementCPtr persistentEl = el.Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);

        ASSERT_TRUE(persistentEl.IsValid());

        // Check that we see the stored value in memory
        checkValue.Clear();

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(0)));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(1)));
        EXPECT_STREQ("first", checkValue.ToString().c_str());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(2)));
        EXPECT_STREQ("second", checkValue.ToString().c_str());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(3)));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(0)));
        EXPECT_TRUE(checkValue.GetPoint3d().IsEqual(DPoint3d::From(1, 1, 1)));

        EXPECT_NE(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(1)));

        for (auto i = 0; i < 300; ++i)
        {
            EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfInt", PropertyArrayIndex(i)));
            EXPECT_EQ(i, checkValue.GetInteger());
        }

        elementId = persistentEl->GetElementId();
        m_db->SaveChanges();
    }
    // Before updatation of element check what stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    ECN::ECValue checkValue;
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    TestElementPtr element = m_db->Elements().GetForEdit<TestElement>(elementId);
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(0)));
    EXPECT_TRUE(checkValue.IsNull());
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(1)));
    EXPECT_STREQ("first", checkValue.ToString().c_str());
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(2)));
    EXPECT_STREQ("second", checkValue.ToString().c_str());
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(3)));
    ASSERT_TRUE(checkValue.IsNull());
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(0)));
    ASSERT_TRUE(checkValue.GetPoint3d().IsEqual(DPoint3d::From(1, 1, 1)));
    checkValue.Clear();
    ASSERT_NE(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(1)));

    for (auto i = 0; i < 300; ++i)
         {
         ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfInt", PropertyArrayIndex(i)));
         ASSERT_EQ(i, checkValue.GetInteger());
        }
    }
    if (true)
        {
        // Get ready to modify the element
        TestElementPtr editEl = m_db->Elements().GetForEdit<TestElement>(elementId);
        ASSERT_TRUE(editEl.IsValid());
        // initially we still see the initial/stored value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(0)));
        EXPECT_TRUE(checkValue.IsNull());
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(1)));
        EXPECT_STREQ("first", checkValue.ToString().c_str());
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(2)));
        EXPECT_STREQ("second", checkValue.ToString().c_str());
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(3)));
        EXPECT_TRUE(checkValue.IsNull());

        // Remove array item from memory
        EXPECT_EQ(DgnDbStatus::Success, editEl->RemovePropertyArrayItem(iArrayOfString, 1));
        //Verfiy the array item is removed from memory by getting its value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(0)));
        EXPECT_TRUE(checkValue.IsNull());
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(1)));
        EXPECT_STREQ("second", checkValue.ToString().c_str());
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(2)));
        EXPECT_TRUE(checkValue.IsNull());
        EXPECT_NE(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(3)));
        // Update the element
        DgnDbStatus stat;
        DgnElementCPtr updated_element=editEl->Update(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(updated_element.IsValid());
        m_db->SaveChanges();
        }

     // REALLY check that the stored value was changed
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    TestElementCPtr Element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(Element.IsValid());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, Element->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(1)));
    EXPECT_STREQ("second", checkValue.ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Caleb.Shafer      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, GetSetAutoHandledStructArrayProperties)
    {
    SetupSeedProject();

    DgnElementId elementId;
    uint32_t iArrayOfStructs;
    ECN::ECValue checkValue;
    ECN::IECInstancePtr checkInstance;
    {
    DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    TestElement el(params);

    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(iArrayOfStructs, "ArrayOfStructs"));

    // Set an array property
    EXPECT_EQ(DgnDbStatus::Success, el.InsertPropertyArrayItems(iArrayOfStructs, 0, 4));

    EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "ArrayOfStructs"));
    EXPECT_TRUE(checkValue.IsArray());
    EXPECT_EQ(4, checkValue.GetArrayInfo().GetCount());

    for (int i = 0; i < 4; i++)
        {
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(i)));
        EXPECT_TRUE(checkValue.IsNull());
        }

    ECN::ECClassCP locationStruct = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_LOCATION_STRUCT_CLASS_NAME);
    ASSERT_TRUE(nullptr != locationStruct);
    ECN::StandaloneECEnablerPtr locationEnabler = locationStruct->GetDefaultStandaloneEnabler();

    ECN::IECInstancePtr extonLocInstance = locationEnabler->CreateInstance().get();
    extonLocInstance->SetValue("Street", ECN::ECValue("690 Pennsylvania Drive"));
    extonLocInstance->SetValue("City.Name", ECN::ECValue("Exton"));
    extonLocInstance->SetValue("City.State", ECN::ECValue("PA"));
    extonLocInstance->SetValue("City.Country", ECN::ECValue("US"));
    extonLocInstance->SetValue("City.Zip", ECN::ECValue(19341));

    ECN::ECValue extonLocValue;
    extonLocValue.SetStruct(extonLocInstance.get());
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfStructs", extonLocValue, PropertyArrayIndex(1)));

    ECN::IECInstancePtr phillyLocInstance = locationEnabler->CreateInstance().get();
    phillyLocInstance->SetValue("Street", ECN::ECValue("1601 Cherry Street"));
    phillyLocInstance->SetValue("City.Name", ECN::ECValue("Philadelphia"));
    phillyLocInstance->SetValue("City.State", ECN::ECValue("PA"));
    phillyLocInstance->SetValue("City.Country", ECN::ECValue("US"));
    phillyLocInstance->SetValue("City.Zip", ECN::ECValue(19102));

    ECN::ECValue phillyLocValue;
    phillyLocValue.SetStruct(phillyLocInstance.get());
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfStructs", phillyLocValue, PropertyArrayIndex(2)));

    // Insert the element
    DgnDbStatus stat;
    DgnElementCPtr persistentEl = el.Insert(&stat);
    ASSERT_EQ(DgnDbStatus::Success, stat);

    ASSERT_TRUE(persistentEl.IsValid());

    // Check that we see the stored value in memory
    checkValue.Clear();

    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(0)));
    EXPECT_TRUE(checkValue.IsNull());

    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1)));
    EXPECT_FALSE(checkValue.IsNull());
    EXPECT_TRUE(checkValue.IsStruct());

    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("690 Pennsylvania Drive", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Exton", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19341, checkValue.GetInteger());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(2)));
    EXPECT_FALSE(checkValue.IsNull());
    EXPECT_TRUE(checkValue.IsStruct());

    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Philadelphia", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19102, checkValue.GetInteger());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(3)));
    EXPECT_TRUE(checkValue.IsNull());

    elementId = persistentEl->GetElementId();
    m_db->SaveChanges();
    }

    // Before updating the element check what is stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    TestElementPtr element = m_db->Elements().GetForEdit<TestElement>(elementId);
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(0)));
    EXPECT_TRUE(checkValue.IsNull());
    EXPECT_TRUE(checkValue.IsStruct());

    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1)));
    EXPECT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("690 Pennsylvania Drive", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Exton", checkValue.GetUtf8CP());

    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(2)));
    EXPECT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Philadelphia", checkValue.GetUtf8CP());

    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(3)));
    EXPECT_TRUE(checkValue.IsNull());
    EXPECT_TRUE(checkValue.IsStruct());
    }

    {
    // Get ready to modify the element
    TestElementPtr editEl = m_db->Elements().GetForEdit<TestElement>(elementId);
    ASSERT_TRUE(editEl.IsValid());
    // initially we still see the initial/stored value
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(0)));
    EXPECT_TRUE(checkValue.IsNull());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1)));
    EXPECT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("690 Pennsylvania Drive", checkValue.GetUtf8CP());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(2)));
    EXPECT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(3)));
    EXPECT_TRUE(checkValue.IsNull());

    // Remove array item from memory
    EXPECT_EQ(DgnDbStatus::Success, editEl->RemovePropertyArrayItem(iArrayOfStructs, 1));
    //Verfiy the array item is removed from memory by getting its value
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(0)));
    EXPECT_TRUE(checkValue.IsNull());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1)));
    EXPECT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(2)));
    EXPECT_TRUE(checkValue.IsNull());
    EXPECT_NE(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(3)));

    // Update the element
    DgnDbStatus stat;
    DgnElementCPtr updated_element=editEl->Update(&stat);
    ASSERT_EQ(DgnDbStatus::Success, stat);
    ASSERT_TRUE(updated_element.IsValid());
    m_db->SaveChanges();
    }

    // REALLY check that the stored value was changed
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    TestElementCPtr Element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(Element.IsValid());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, Element->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1)));
    EXPECT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Philadelphia", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19102, checkValue.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, OverrideAutohandledproperites)
    {
    SetupSeedProject();
    DgnElementId elementId;
    uint32_t boolean;
    if (true)
    {
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_OVERRIDE_AUTOHADLEPROPERTIES));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(boolean, "p2d"));
        // get property before setting
        ASSERT_TRUE(el.GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 0))));
        // set property
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("p2d", DPoint2d::From(0, 9)));
        //get property after setting
        ASSERT_TRUE(el.GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 9))));
        // Insert the element
        DgnDbStatus stat;
        DgnElementCPtr persistentEl = el.Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(persistentEl.IsValid());
        elementId = persistentEl->GetElementId();
        // Check that what stored value in memory
        ASSERT_TRUE(persistentEl->GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 9))));
     }
    BeFileName fileName = m_db->GetFileName();
    m_db->SaveChanges();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    // Check that what stored value in DB
    TestElementCPtr element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(element->GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 9))));
    }
    if (true)
       {
       // Get ready to modify the element
       TestElementPtr editEl = m_db->Elements().GetForEdit<TestElement>(elementId);
       // initially we still see the initial/stored value
       ASSERT_TRUE(editEl->GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 9))));
       EXPECT_EQ(DgnDbStatus::Success, editEl->SetPropertyValue("p2d", DPoint2d::From(0, 10)));
       // Update the element
       DgnDbStatus stat;
       DgnElementCPtr updated_Element = editEl->Update(&stat);
       ASSERT_EQ(DgnDbStatus::Success, stat);
       ASSERT_TRUE(updated_Element.IsValid());
       }
    // check that the stored value
    m_db->SaveChanges();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    TestElementCPtr element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(element.IsValid());
    ASSERT_TRUE(element->GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 10))));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, CreateFromECInstance)
    {
    SetupSeedProject();

    DgnElementId eid;
        {
        ECN::ECClassCP testClass = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME);
        auto testClassInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
        DgnCode code = DgnCode::CreateEmpty();
        // custom-handled properties
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("Model", ECN::ECValue(m_defaultModelId)));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("Category", ECN::ECValue(m_defaultCategoryId)));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("UserLabel", ECN::ECValue("my label")));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("CodeSpec", ECN::ECValue(code.GetCodeSpecId())));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("CodeScope", ECN::ECValue(code.GetScopeElementId(*m_db))));
        ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("CodeValue", ECN::ECValue(code.GetValueUtf8CP())));
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
        auto ele = m_db->Elements().CreateElement(*testClassInstance, &status);
        ASSERT_TRUE(ele.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, status);

        ASSERT_EQ((int64_t)m_defaultModelId.GetValue(), ele->GetModelId().GetValue());
        ECN::ECValue v;
        ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "Model"));
        ASSERT_EQ(m_defaultModelId, v.GetNavigationInfo().GetId<DgnModelId>());
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
    ASSERT_EQ(DgnDbStatus::Success, ele->GetPropertyValue(v, "Model"));
    ASSERT_EQ(m_defaultModelId, v.GetNavigationInfo().GetId<DgnModelId>());
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
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnElementTests, GetSetPropertyValues)
    {
    SetupSeedProject();

    DateTime dateTime = DateTime(DateTime::Kind::Utc, 2016, 2, 14, 9, 58, 17, 456);
    DateTime dateTimeUtc = DateTime::GetCurrentTimeUtc();
    DPoint2d point2d = {123.456, 456.789};
    DPoint3d point3d = {1.2, -3.4, 5.6};

    DgnElementId persistentId;
    if (true)
        {
        DgnElementCPtr persistentEl;

        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);

        auto testProps = el.GetUserProperties("testProps");

        // set user properties
        ASSERT_EQ(SUCCESS, testProps.SetValueEC("b", ECN::ECValue(false)));
        ASSERT_EQ(SUCCESS, testProps.SetValueEC("d", ECN::ECValue(1.0001)));
        ASSERT_EQ(SUCCESS, testProps.SetValueEC("date", ECN::ECValue(dateTime)));
        ASSERT_EQ(SUCCESS, testProps.SetValueEC("utc", ECN::ECValue(dateTimeUtc)));
        ASSERT_EQ(SUCCESS, testProps.SetValueEC("i", ECN::ECValue(1)));
        ASSERT_EQ(SUCCESS, testProps.SetValueEC("l", ECN::ECValue((uint64_t)1000000000001)));
        ASSERT_EQ(SUCCESS, testProps.SetValueEC("s", ECN::ECValue("StringVal")));
        ASSERT_EQ(SUCCESS, testProps.SetValueEC("p2d", ECN::ECValue(point2d)));
        ASSERT_EQ(SUCCESS, testProps.SetValueEC("p3d", ECN::ECValue(point3d)));

        //get property values
        ECN::ECValue checkValue = testProps.GetValueEC("b");
        EXPECT_FALSE(checkValue.GetBoolean());

        checkValue = testProps.GetValueEC("d");
        EXPECT_EQ(1.0001, checkValue.GetDouble());

        checkValue = testProps.GetValueEC("date");
        EXPECT_FALSE(checkValue.IsNull());
        compareDateTimes(dateTime, checkValue.GetDateTime());

        checkValue = testProps.GetValueEC("utc");
        EXPECT_FALSE(checkValue.IsNull());
        compareDateTimes(dateTimeUtc, checkValue.GetDateTime());

        checkValue = testProps.GetValueEC("i");
        EXPECT_EQ(1, checkValue.GetInteger());

        checkValue = testProps.GetValueEC("l");
        EXPECT_EQ(1000000000001, checkValue.GetLong());

        checkValue = testProps.GetValueEC("s");
        testProps.SetHidden("s", true);
        testProps.SetCategory("s", "blah");
        testProps.SetReadOnly("s", true);
        testProps.SetExtendedType("s", "myType");
        EXPECT_STREQ("StringVal", checkValue.GetUtf8CP());

        checkValue = testProps.GetValueEC("p2d");
        EXPECT_EQ(point2d.x, checkValue.GetPoint2d().x);
        EXPECT_EQ(point2d.y, checkValue.GetPoint2d().y);

        checkValue = testProps.GetValueEC("p3d");
        EXPECT_EQ(point3d.x, checkValue.GetPoint3d().x);
        EXPECT_EQ(point3d.y, checkValue.GetPoint3d().y);
        EXPECT_EQ(point3d.z, checkValue.GetPoint3d().z);

        el.SetUserProperties("testProps", testProps);

        //  Insert the element
        persistentEl = el.Insert();
        persistentId = persistentEl->GetElementId();
        }

    m_db->SaveChanges("");
    m_db->Elements().ClearCache();

    ASSERT_TRUE(persistentId.IsValid());

    if (true)
        {
        DgnElementCPtr persistentEl = m_db->Elements().Get<DgnElement>(persistentId);

        auto& testProps = persistentEl->GetUserProperties("testProps");
        // verify property value is persisted
        ECN::ECValue checkValue = testProps.GetValueEC("s");
        EXPECT_STREQ("StringVal", checkValue.GetUtf8CP());

        // get priority
        int priority = testProps.GetPriority("s");
        EXPECT_EQ(0, priority);

        // get is hidden
        bool isHidden = testProps.GetHidden("s");
        EXPECT_TRUE(isHidden);

        // get extended type
        Utf8String extendedType = testProps.GetExtendedType("s");
        EXPECT_TRUE("myType" == extendedType);

        // get category
        Utf8String category = testProps.GetCategory("s");
        EXPECT_TRUE(category == "blah");

        bool readonly = testProps.GetReadOnly("s");
        EXPECT_TRUE(readonly);
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

        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);

        ECN::AdHocJsonValue testProps;

        //  Set user property (in memory)
        testProps.SetValueEC("stringProperty", ECN::ECValue("initial value"));

        el.SetUserProperties("testProps", testProps);

        Json::Value val(23.0);
        el.SetUserProperties("test2", val);

        //  Insert the element
        persistentEl = el.Insert();
        persistentId = persistentEl->GetElementId();
        }

    m_db->SaveChanges("");
    m_db->Elements().ClearCache();

    ASSERT_TRUE(persistentId.IsValid());

    if (true)
        {
        DgnElementCPtr persistentEl = m_db->Elements().Get<DgnElement>(persistentId);

        auto testProps = persistentEl->GetUserProperties("testProps");

        // Check that we see the stored value
        ECN::ECValue checkValue = testProps.GetValueEC("stringProperty");
        EXPECT_FALSE(checkValue.IsNull());
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());

        //  Get ready to modify the element
        RefCountedPtr<TestElement> editEl = persistentEl->MakeCopy<TestElement>();

        //      ... initially we still see the initial/stored value
        //  Set a new value (in memory)
        testProps.SetValueEC("stringProperty", ECN::ECValue("changed value"));

        editEl->SetUserProperties("testProps", testProps);

        //  Update the element
        persistentEl = editEl->Update();
        }

    m_db->Elements().ClearCache();

    if (true)
        {
        DgnElementCPtr persistentEl = m_db->Elements().Get<DgnElement>(persistentId);
        auto& testProps = persistentEl->GetUserProperties("testProps");

        // Check that the stored value was changed
        ECN::ECValue checkValue = testProps.GetValueEC("stringProperty");
        EXPECT_FALSE(checkValue.IsNull());
        EXPECT_STREQ("changed value", checkValue.ToString().c_str());

        auto test2 = persistentEl->GetUserProperties("test2");
        EXPECT_TRUE(test2.asInt() == 23);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, ParentChildSameModel)
    {
    SetupSeedProject();

    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "testCategory");
    PhysicalModelPtr modelA = DgnDbTestUtils::InsertPhysicalModel(*m_db, "modelA");
    PhysicalModelPtr modelB = DgnDbTestUtils::InsertPhysicalModel(*m_db, "modelB");

    GenericPhysicalObjectPtr parentA = GenericPhysicalObject::Create(*modelA, categoryId);
    GenericPhysicalObjectPtr parentB = GenericPhysicalObject::Create(*modelB, categoryId);
    EXPECT_TRUE(parentA.IsValid());
    EXPECT_TRUE(parentB.IsValid());
    EXPECT_TRUE(parentA->Insert().IsValid());
    EXPECT_TRUE(parentB->Insert().IsValid());
    DgnElementId childIdA, childIdB, childIdC;
    DgnClassId parentRelClassId = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);

    // test DgnElement::_OnChildInsert success
        {
        GenericPhysicalObjectPtr childA = GenericPhysicalObject::Create(*modelA, categoryId);
        GenericPhysicalObjectPtr childB = GenericPhysicalObject::Create(*modelB, categoryId);
        GenericPhysicalObjectPtr childC = GenericPhysicalObject::Create(*modelB, categoryId);
        EXPECT_TRUE(childA.IsValid());
        EXPECT_TRUE(childB.IsValid());
        EXPECT_TRUE(childC.IsValid());
        childA->SetParentId(parentA->GetElementId(), parentRelClassId); // Match
        childB->SetParentId(parentB->GetElementId(), parentRelClassId); // Match
        EXPECT_TRUE(childA->Insert().IsValid()) << "Expecting success because models of parent and child are the same";
        EXPECT_TRUE(childB->Insert().IsValid()) << "Expecting success because models of parent and child are the same";
        EXPECT_TRUE(childC->Insert().IsValid()) << "Expecting success because childC has no parent";
        EXPECT_TRUE(childA->GetParentId().IsValid());
        EXPECT_TRUE(childB->GetParentId().IsValid());
        EXPECT_FALSE(childC->GetParentId().IsValid());
        EXPECT_TRUE(childA->GetParentRelClassId().IsValid());
        EXPECT_TRUE(childB->GetParentRelClassId().IsValid());
        EXPECT_FALSE(childC->GetParentRelClassId().IsValid());
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
        childA->SetParentId(parentB->GetElementId(), parentRelClassId); // Mismatch
        childB->SetParentId(parentA->GetElementId(), parentRelClassId); // Mismatch
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
        childA->SetParentId(parentB->GetElementId(), parentRelClassId); // Mismatch
        childB->SetParentId(parentA->GetElementId(), parentRelClassId); // Mismatch
        childC->SetParentId(parentA->GetElementId(), parentRelClassId); // Mismatch
        DgnDbStatus updateStatusA, updateStatusB, updateStatusC;
        BeTest::SetFailOnAssert(false);
        EXPECT_FALSE(childA->Update(&updateStatusA).IsValid()) << "Expecting failure because models of parent and child are different";
        EXPECT_FALSE(childB->Update(&updateStatusB).IsValid()) << "Expecting failure because models of parent and child are different";
        EXPECT_FALSE(childC->Update(&updateStatusC).IsValid()) << "Expecting failure because models of parent and child are different";
        BeTest::SetFailOnAssert(true);
        EXPECT_EQ(DgnDbStatus::WrongModel, updateStatusA);
        EXPECT_EQ(DgnDbStatus::WrongModel, updateStatusB);
        EXPECT_EQ(DgnDbStatus::WrongModel, updateStatusC);

        childC->SetParentId(parentB->GetElementId(), parentRelClassId); // Match
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

    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestModel");
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory");

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
        EXPECT_FALSE(element->GetPropertyValueGuid("FederationGuid").IsValid()) << "FederationGuid expected to be initialized as invalid";
        EXPECT_TRUE(element->Insert().IsValid());
        elementId = element->GetElementId();
        EXPECT_FALSE(element->GetFederationGuid().IsValid()) << "FederationGuid expected to be initialized as invalid";
        }

    // test error conditions for QueryElementByFederationGuid
    EXPECT_FALSE(m_db->Elements().QueryElementByFederationGuid(BeGuid()).IsValid()) << "Should not find an element by an invalid FederationGuid";
    EXPECT_FALSE(m_db->Elements().QueryElementByFederationGuid(BeGuid(true)).IsValid()) << "Should not find an element by an unused FederationGuid";

    // flush cache and re-check element
        {
        m_db->Elements().ClearCache();
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
        EXPECT_EQ(element->GetFederationGuid(), federationGuid);
        EXPECT_EQ(element->GetPropertyValueGuid("FederationGuid"), federationGuid);
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
        m_db->Elements().ClearCache();
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
        m_db->Elements().ClearCache();
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
        m_db->Elements().ClearCache();
        DgnElementCPtr element = m_db->Elements().GetElement(elementId);
        EXPECT_TRUE(element.IsValid());
        EXPECT_FALSE(element->GetFederationGuid().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, PhysicalTypeCRUD)
    {
    SetupSeedProject();
    DefinitionModelPtr typeModel = DgnDbTestUtils::InsertDefinitionModel(*m_db, "PhysicalTypes");

    DgnClassId physicalTypeRelClassId = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_PhysicalElementIsOfType);
    ASSERT_TRUE(physicalTypeRelClassId.IsValid());

    DgnElementId physicalTypeId[3];
    DgnElementId elementId;

    // insert some sample PhysicalTypes
    for (int32_t i=0; i<_countof(physicalTypeId); i++)
        {
        Utf8PrintfString name("PhysicalType%" PRIi32, i);
        TestPhysicalTypePtr physicalType = TestPhysicalType::Create(*typeModel, name.c_str());
        ASSERT_TRUE(physicalType.IsValid());
        physicalType->SetStringProperty(Utf8PrintfString("String%d", i).c_str());
        physicalType->SetIntProperty(i);
        ASSERT_TRUE(physicalType->Insert().IsValid());
        physicalTypeId[i] = physicalType->GetElementId();
        }

    // flush cache to make sure PhysicalTypes were inserted properly
        {
        m_db->Elements().ClearCache();
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
        m_db->Elements().ClearCache();
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
        DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory");
        PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestModel");

        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(*model, categoryId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->GetTypeDefinitionId().IsValid());
        ASSERT_FALSE(element->GetPhysicalType().IsValid());
        element->SetTypeDefinition(physicalTypeId[0], physicalTypeRelClassId);
        ASSERT_TRUE(element->GetTypeDefinitionId().IsValid());
        ASSERT_TRUE(element->GetPhysicalType().IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        elementId = element->GetElementId();
        }

    // flush cache to make sure the PhysicalElement's PhysicalType was inserted properly
        {
        m_db->Elements().ClearCache();
        GenericPhysicalObjectPtr element = m_db->Elements().GetForEdit<GenericPhysicalObject>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->GetPhysicalType().IsValid());
        ASSERT_EQ(element->GetTypeDefinitionId().GetValue(), physicalTypeId[0].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, element->SetTypeDefinition(physicalTypeId[1], physicalTypeRelClassId));
        ASSERT_TRUE(element->Update().IsValid());
        }

    // flush cache to make sure the PhysicalElement's PhysicalType was updated properly
        {
        m_db->Elements().ClearCache();
        GenericPhysicalObjectPtr element = m_db->Elements().GetForEdit<GenericPhysicalObject>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->GetPhysicalType().IsValid());
        ASSERT_EQ(element->GetTypeDefinitionId().GetValue(), physicalTypeId[1].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, element->SetTypeDefinition(DgnElementId(), physicalTypeRelClassId));
        ASSERT_TRUE(element->Update().IsValid());
        }

    // flush cache to make sure the PhysicalElement's PhysicalType was cleared properly
        {
        m_db->Elements().ClearCache();
        GenericPhysicalObjectPtr element = m_db->Elements().GetForEdit<GenericPhysicalObject>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->GetPhysicalType().IsValid());
        ASSERT_FALSE(element->GetTypeDefinitionId().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, GraphicalType2dCRUD)
    {
    SetupSeedProject();
    DefinitionModelPtr typesModel = DgnDbTestUtils::InsertDefinitionModel(*m_db, "GraphicalTypes");

    DgnClassId graphicalTypeRelClassId = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_GraphicalElement2dIsOfType);
    ASSERT_TRUE(graphicalTypeRelClassId.IsValid());

    DgnElementId graphicalTypeId[3];
    DgnElementId elementId;

    // insert some sample GraphicalTypes
    for (int32_t i=0; i<_countof(graphicalTypeId); i++)
        {
        TestGraphicalType2dPtr graphicalType = TestGraphicalType2d::Create(*typesModel, Utf8PrintfString("GraphicalType%d", i).c_str());
        ASSERT_TRUE(graphicalType.IsValid());
        graphicalType->SetStringProperty(Utf8PrintfString("String%d", i).c_str());
        graphicalType->SetIntProperty(i);
        ASSERT_TRUE(graphicalType->Insert().IsValid());
        graphicalTypeId[i] = graphicalType->GetElementId();
        }

    // flush cache to make sure GraphicalTypes were inserted properly
        {
        m_db->Elements().ClearCache();
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
        m_db->Elements().ClearCache();
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
        DgnCategoryId categoryId = DgnDbTestUtils::InsertDrawingCategory(GetDgnDb(), "TestCategory");
        DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(GetDgnDb(), "DrawingListModel");
        DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "Drawing");
        DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);

        TestElement2dPtr element = TestElement2d::Create(GetDgnDb(), drawingModel->GetModelId(), categoryId, DgnCode(), 2.0);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->GetTypeDefinitionId().IsValid());
        ASSERT_FALSE(element->GetGraphicalType().IsValid());
        element->SetTypeDefinition(graphicalTypeId[0], graphicalTypeRelClassId);
        ASSERT_TRUE(element->GetTypeDefinitionId().IsValid());
        ASSERT_TRUE(element->GetGraphicalType().IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        elementId = element->GetElementId();
        }

    // flush cache to make sure the TestElement2d's GraphicalType was inserted properly
        {
        m_db->Elements().ClearCache();
        TestElement2dPtr element = m_db->Elements().GetForEdit<TestElement2d>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->GetGraphicalType().IsValid());
        ASSERT_EQ(element->GetTypeDefinitionId().GetValue(), graphicalTypeId[0].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, element->SetTypeDefinition(graphicalTypeId[1], graphicalTypeRelClassId));
        ASSERT_TRUE(element->Update().IsValid());
        }

    // flush cache to make sure the TestElement2d's GraphicalType was updated properly
        {
        m_db->Elements().ClearCache();
        TestElement2dPtr element = m_db->Elements().GetForEdit<TestElement2d>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->GetGraphicalType().IsValid());
        ASSERT_EQ(element->GetTypeDefinitionId().GetValue(), graphicalTypeId[1].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, element->SetTypeDefinition(DgnElementId(), graphicalTypeRelClassId));
        ASSERT_TRUE(element->Update().IsValid());
        }

    // flush cache to make sure the TestElement2d's GraphicalType was cleared properly
        {
        m_db->Elements().ClearCache();
        TestElement2dPtr element = m_db->Elements().GetForEdit<TestElement2d>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->GetGraphicalType().IsValid());
        ASSERT_FALSE(element->GetTypeDefinitionId().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, EqualsTests)
    {
    SetupSeedProject();

    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "testCategory");
    PhysicalModelPtr modelA = DgnDbTestUtils::InsertPhysicalModel(*m_db, "modelA");
    PhysicalModelPtr modelB = DgnDbTestUtils::InsertPhysicalModel(*m_db, "modelB");

    GenericPhysicalObjectPtr elementA = GenericPhysicalObject::Create(*modelA, categoryId);
    ASSERT_TRUE(elementA.IsValid());

    ASSERT_TRUE(elementA->Equals(*elementA)) << " An element should be equivalent to itself";
    ASSERT_TRUE(elementA->Equals(*elementA->CopyForEdit())) << " An element should be equivalent to a copy of itself";

    GenericPhysicalObjectPtr elementB = GenericPhysicalObject::Create(*modelB, categoryId);
    ASSERT_TRUE(elementB.IsValid());
    ASSERT_FALSE(elementA->Equals(*elementB)) << " ModelIds should differ";
    bset<Utf8String> ignoreProps;
    ignoreProps.insert("Model");
    DgnElement::ComparePropertyFilter filter(ignoreProps);
    ASSERT_TRUE(elementA->Equals(*elementB, filter));

    elementB->SetUserLabel("label for b");
    ASSERT_FALSE(elementA->Equals(*elementB, filter)) << " UserLabels should differ";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, ElementIterator)
    {
    SetupSeedProject();
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory");
    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestPhysicalModel");
    CodeSpecId codeSpecId = DgnDbTestUtils::InsertCodeSpec(*m_db, "TestCodeSpec");
    const int numPhysicalObjects=5;

    for (int i=0; i<numPhysicalObjects; i++)
        {
        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(*model, categoryId);
        ASSERT_TRUE(element.IsValid());
        Utf8PrintfString userLabel("UserLabel%d", i);
        element->SetUserLabel(userLabel.c_str());
        Utf8PrintfString codeValue("CodeValue%d", i);
        element->SetCode(CodeSpec::CreateCode(*m_db, "TestCodeSpec", codeValue));
        ASSERT_TRUE(element->Insert().IsValid());
        }

    ElementIterator iterator = m_db->Elements().MakeIterator(GENERIC_SCHEMA(GENERIC_CLASS_PhysicalObject));
    ASSERT_EQ(numPhysicalObjects, iterator.BuildIdSet<DgnElementId>().size());
    ASSERT_EQ(numPhysicalObjects, iterator.BuildIdList<DgnElementId>().size());

    bvector<DgnElementId> idList;
    iterator.BuildIdList(idList);
    ASSERT_EQ(numPhysicalObjects, idList.size());

    iterator = m_db->Elements().MakeIterator(GENERIC_SCHEMA(GENERIC_CLASS_PhysicalObject), "WHERE [UserLabel]='UserLabel1'");
    ASSERT_EQ(1, iterator.BuildIdSet<DgnElementId>().size());
    ASSERT_EQ(1, iterator.BuildIdList<DgnElementId>().size());

    int count=0;
    for (ElementIteratorEntryCR entry : m_db->Elements().MakeIterator(GENERIC_SCHEMA(GENERIC_CLASS_PhysicalObject), nullptr, "ORDER BY ECInstanceId"))
        {
        ASSERT_TRUE(entry.GetClassId().IsValid());
        ASSERT_EQ(entry.GetModelId(), model->GetModelId());
        Utf8PrintfString userLabel("UserLabel%d", count);
        ASSERT_STREQ(entry.GetUserLabel(), userLabel.c_str());
        Utf8PrintfString codeValue("CodeValue%d", count);
        ASSERT_STREQ(entry.GetCodeValue(), codeValue.c_str());
        ASSERT_STREQ(entry.GetCode().GetValueUtf8CP(), codeValue.c_str());
        ASSERT_EQ(entry.GetCode().GetCodeSpecId(), codeSpecId);
        count++;
        }
    ASSERT_EQ(numPhysicalObjects, count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, SimpleInsert)
    {
    SetupSeedProject();
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory");
    SpatialLocationModelPtr spatialLocationModel = DgnDbTestUtils::InsertSpatialLocationModel(*m_db, "TestSpatialLocationModel");
    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestPhysicalModel");
    InformationRecordModelPtr informationRecordModel = DgnDbTestUtils::InsertInformationRecordModel(*m_db, "TestInformationRecordModel");

    TestSpatialLocationPtr spatialLocation1 = TestSpatialLocation::Create(*spatialLocationModel, categoryId);
    ASSERT_TRUE(spatialLocation1.IsValid());
    ASSERT_TRUE(spatialLocation1->Insert().IsValid()) << "SpatialLocationElements should be able to be inserted into a SpatialLocationModel";

    TestSpatialLocationPtr spatialLocation2 = TestSpatialLocation::Create(*physicalModel, categoryId);
    ASSERT_TRUE(spatialLocation2.IsValid());
    ASSERT_TRUE(spatialLocation2->Insert().IsValid()) << "SpatialLocationElements should be able to be inserted into a PhysicalModel";

    TestInformationRecordPtr informationRecord = TestInformationRecord::Create(*informationRecordModel);
    ASSERT_TRUE(informationRecord.IsValid());
    ASSERT_TRUE(informationRecord->Insert().IsValid()) << "InformationRecordElements should be able to be inserted into an InformationRecordModel";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, NavigationPropertyOnTargetDelete)
    {
    SetupSeedProject();
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory");
    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestPhysicalModel");
    auto analyticalPipeClass = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, "AnalyticalPipe");
    auto physicalPipeClass = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, "PhysicalPipe");
    ASSERT_TRUE(nullptr != analyticalPipeClass);
    ASSERT_TRUE(nullptr != physicalPipeClass);
    PhysicalElementPtr physical = TestElement::Create(*m_db, *physicalPipeClass, model->GetModelId(), categoryId);
    ASSERT_TRUE(physical.IsValid());
    ASSERT_TRUE(physical->Insert().IsValid());
    PhysicalElementPtr analytical = TestElement::Create(*m_db, *analyticalPipeClass, model->GetModelId(), categoryId);
    ASSERT_TRUE(analytical.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, analytical->SetPropertyValue("PhysicalPipe", physical->GetElementId()));
    ASSERT_TRUE(analytical->Insert().IsValid());
    m_db->SaveChanges();
    ASSERT_TRUE(analytical->GetPropertyValueId<DgnElementId>("PhysicalPipe") == physical->GetElementId());
    ASSERT_EQ(DgnDbStatus::Success, physical->Delete());
    ASSERT_TRUE(analytical->GetPropertyValueId<DgnElementId>("PhysicalPipe") == DgnElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, CreateSubjectChildElemet)
    {
    SetupSeedProject();
    SubjectCPtr rootSubject = m_db->Elements().GetRootSubject();
    ASSERT_TRUE(rootSubject.IsValid());
    SubjectPtr subele=Subject::Create(*rootSubject, "SubjectChild","Child1");
    ASSERT_TRUE(subele.IsValid());
    DgnElementCPtr ele=subele->Insert();
    ASSERT_TRUE(ele.IsValid());
    ASSERT_EQ(ele->GetModelId(), rootSubject->GetModelId());
    DgnElementCPtr elep = m_db->Elements().GetElement(ele->GetElementId());
    ASSERT_EQ(elep->GetDisplayLabel(),"SubjectChild");
    RefCountedCPtr<Subject> info=m_db->Elements().Get<Subject>(ele->GetElementId());
    ASSERT_EQ(info->GetDescription(),"Child1");

    SubjectCPtr subele2 = Subject::CreateAndInsert(*rootSubject, "SubjectChild2", "Child2");
    ASSERT_TRUE(subele2.IsValid());
    ASSERT_EQ(subele2->GetModelId(), rootSubject->GetModelId());
    elep = m_db->Elements().GetElement(subele2->GetElementId());
    ASSERT_EQ(elep->GetDisplayLabel(), "SubjectChild2");
    info = m_db->Elements().Get<Subject>(subele2->GetElementId());
    ASSERT_EQ(info->GetDescription(), "Child2");
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                   02/2018
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(DgnElementTests, FromJson)
    {
    SetupSeedProject();

    DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    ECN::ECValue checkValue;
    ECN::IECInstancePtr checkInstance = nullptr;
    DgnElementId elementId;

    bool const b = false;
    double const d = 3.14;
    int32_t const i = 50;
    int64_t const l = 100;
    DateTime const dt(2018, 2, 4);
    DateTime const dtUtc(2018, 2, 5);
    auto const p2d = DPoint2d::From(3, 5);
    auto const p3d = DPoint3d::From(3, 5, 7);
    Utf8String const s("test string");

    Utf8String const invalidProp("this should not be converted");
    Utf8String const customHandledProp("and neither should this");

    // Create JSON and set properties
    {
    TestElement el(params);

    Json::Value json(Json::ValueType::objectValue);
    json["b"] = b;
    json["d"] = d;
    json["i"] = i;
    ECN::ECJsonUtilities::Int64ToJson(json["l"], l);

    ECN::ECJsonUtilities::DateTimeToJson(json["dt"], dt);
    ECN::ECJsonUtilities::DateTimeToJson(json["dtUtc"], dtUtc);

    ECN::ECJsonUtilities::Point2dToJson(json["p2d"], p2d);
    ECN::ECJsonUtilities::Point3dToJson(json["p3d"], p3d);

    json["s"] = s;

    json["ArrayOfStructs"] = Json::Value(Json::ValueType::arrayValue);
    Utf8String tmp;
    Json::Value extonOffice(Json::ValueType::objectValue);
    ASSERT_TRUE(ECN::ECValue("690 Pennsylvania Drive").ConvertPrimitiveToString(tmp));
    extonOffice["Street"] = tmp;
    ASSERT_TRUE(ECN::ECValue("Exton").ConvertPrimitiveToString(tmp));
    extonOffice["City"] = Json::Value(Json::ValueType::objectValue);
    extonOffice["City"]["Name"] = tmp;
    ASSERT_TRUE(ECN::ECValue("PA").ConvertPrimitiveToString(tmp));
    extonOffice["City"]["State"] = tmp;
    ASSERT_TRUE(ECN::ECValue("US").ConvertPrimitiveToString(tmp));
    extonOffice["City"]["Country"] = tmp;
    ASSERT_TRUE(ECN::ECValue(19341).ConvertPrimitiveToString(tmp));
    extonOffice["City"]["Zip"] = tmp;
    json["ArrayOfStructs"][0u] = extonOffice;
    Json::Value phillyOffice(Json::ValueType::objectValue);
    ASSERT_TRUE(ECN::ECValue("1601 Cherry Street").ConvertPrimitiveToString(tmp));
    phillyOffice["Street"] = tmp;
    ASSERT_TRUE(ECN::ECValue("Philadelphia").ConvertPrimitiveToString(tmp));
    phillyOffice["City"]["Name"] = tmp;
    ASSERT_TRUE(ECN::ECValue("PA").ConvertPrimitiveToString(tmp));
    phillyOffice["City"]["State"] = tmp;
    ASSERT_TRUE(ECN::ECValue("US").ConvertPrimitiveToString(tmp));
    phillyOffice["City"]["Country"] = tmp;
    ASSERT_TRUE(ECN::ECValue(19102).ConvertPrimitiveToString(tmp));
    phillyOffice["City"]["Zip"] = tmp;
    json["ArrayOfStructs"][1u] = phillyOffice;

    ASSERT_FALSE(ElementECPropertyAccessor(el, "invalidProp").IsValid());
    ASSERT_TRUE(ElementECPropertyAccessor(el, "invalidProp").IsAutoHandled());
    json["invalidProp"] = invalidProp;

    ASSERT_TRUE(ElementECPropertyAccessor(el, "TestElementProperty").IsValid());
    ASSERT_FALSE(ElementECPropertyAccessor(el, "TestElementProperty").IsAutoHandled());
    json["TestElementProperty"] = customHandledProp;

    // Populate element with json
    el.FromJson(json);

    // Insert the element
    DgnDbStatus stat;
    DgnElementCPtr persistentEl = el.Insert(&stat);
    ASSERT_EQ(DgnDbStatus::Success, stat);
    ASSERT_TRUE(persistentEl.IsValid());

    // Check in-memory Element that will be persisted
    // Test persisted values
    EXPECT_EQ(b, persistentEl->GetPropertyValueBoolean("b"));
    EXPECT_DOUBLE_EQ(d, persistentEl->GetPropertyValueDouble("d"));
    EXPECT_EQ(i, persistentEl->GetPropertyValueInt32("i"));
    ECN::ECValue longVal;
    persistentEl->GetPropertyValue(longVal, "l");
    ASSERT_TRUE(longVal.IsLong());
    EXPECT_EQ(l, longVal.GetLong());
    EXPECT_TRUE(persistentEl->GetPropertyValueDateTime("dt").Equals(dt, true));
    EXPECT_TRUE(persistentEl->GetPropertyValueDateTime("dtUtc").Equals(dtUtc, true));
    EXPECT_EQ(p2d, persistentEl->GetPropertyValueDPoint2d("p2d"));
    EXPECT_EQ(p3d, persistentEl->GetPropertyValueDPoint3d("p3d"));
    EXPECT_STREQ(s.c_str(), persistentEl->GetPropertyValueString("s").c_str());

    checkValue.Clear();
    persistentEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(0u));
    ASSERT_FALSE(checkValue.IsNull());
    ASSERT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("690 Pennsylvania Drive", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Exton", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19341, checkValue.GetInteger());

    checkValue.Clear();
    persistentEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1u));
    ASSERT_FALSE(checkValue.IsNull());
    ASSERT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Philadelphia", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19102, checkValue.GetInteger());

    checkValue.Clear();
    persistentEl->GetPropertyValue(checkValue, "invalidProp");
    EXPECT_TRUE(checkValue.IsNull());

    checkValue.Clear();
    persistentEl->GetPropertyValue(checkValue, "TestElementProperty");
    EXPECT_STREQ("", checkValue.GetUtf8CP());

    // Get Persistent ElementId and save to db
    elementId = persistentEl->GetElementId();
    m_db->SaveChanges();
    }

    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);

    // Retrieve properties
    {
    TestElementCPtr retrievedElement = m_db->Elements().Get<TestElement>(elementId);

    EXPECT_EQ(b, retrievedElement->GetPropertyValueBoolean("b"));
    EXPECT_DOUBLE_EQ(d, retrievedElement->GetPropertyValueDouble("d"));
    EXPECT_TRUE(retrievedElement->GetPropertyValueDateTime("dt").Equals(dt, true));
    EXPECT_TRUE(retrievedElement->GetPropertyValueDateTime("dtUtc").Equals(dtUtc, true));
    EXPECT_EQ(p2d, retrievedElement->GetPropertyValueDPoint2d("p2d"));
    EXPECT_EQ(p3d, retrievedElement->GetPropertyValueDPoint3d("p3d"));
    EXPECT_STREQ(s.c_str(), retrievedElement->GetPropertyValueString("s").c_str());

    checkValue.Clear();
    retrievedElement->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(0u));
    ASSERT_FALSE(checkValue.IsNull());
    ASSERT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("690 Pennsylvania Drive", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Exton", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19341, checkValue.GetInteger());

    checkValue.Clear();
    retrievedElement->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1u));
    ASSERT_FALSE(checkValue.IsNull());
    ASSERT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Philadelphia", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19102, checkValue.GetInteger());

    checkValue.Clear();
    checkInstance->GetValue(checkValue, "TestElementProperty");
    EXPECT_TRUE(checkValue.IsNull());
    }

    m_db->CloseDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bill.Goehrig     03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnElementTests, RelatedElementToJson)
    {
    SetupSeedProject();

    DgnElementId elementId((uint64_t)0x123456789u);
    DgnClassId relClassId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME));
    DgnElement::RelatedElement related(elementId, relClassId);

    Json::Value actualJson = related.ToJson(*m_db);
    EXPECT_TRUE(actualJson.isObject());

    Utf8String expectedId = elementId.ToHexStr();
    EXPECT_TRUE(actualJson[ECN::ECJsonUtilities::json_navId()].isString());
    EXPECT_STREQ(expectedId.c_str(), actualJson[ECN::ECJsonUtilities::json_navId()].asCString());

    Utf8CP expectedRelClassName = DPTEST_SCHEMA_NAME ":" DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME;
    EXPECT_TRUE(actualJson[ECN::ECJsonUtilities::json_navRelClassName()].isString());
    EXPECT_STREQ(expectedRelClassName, actualJson[ECN::ECJsonUtilities::json_navRelClassName()].asCString());

    m_db->CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bill.Goehrig     03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnElementTests, RelatedElementFromJson)
    {
    SetupSeedProject();

    DgnElementId expectedElementId((uint64_t)0x987654321u);
    DgnClassId expectedRelClassId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME));

    Utf8String serializedElementId = expectedElementId.ToHexStr();
    Json::Value json(Json::ValueType::objectValue);
    json[ECN::ECJsonUtilities::json_navId()] = serializedElementId;

    // Normal relClassName format - {schemaName}.{className}
        {
        json[ECN::ECJsonUtilities::json_navRelClassName()] = DPTEST_SCHEMA_NAME "." DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME;
        DgnElement::RelatedElement related;
        related.FromJson(*m_db, json);
        EXPECT_TRUE(expectedElementId == related.m_id);
        EXPECT_EQ(expectedRelClassId, related.m_relClassId);
        }

    // Should also support legacy relClassName format - {schemaName}:{className}
        {
        json[ECN::ECJsonUtilities::json_navRelClassName()] = DPTEST_SCHEMA_NAME ":" DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME;
        DgnElement::RelatedElement related;
        related.FromJson(*m_db, json);
        EXPECT_TRUE(expectedElementId == related.m_id);
        EXPECT_EQ(expectedRelClassId, related.m_relClassId);
        }

    m_db->CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Caleb.Shafer      07/2019
//---------------------------------------------------------------------------------------
TEST_F(DgnElementTests, ToJson)
{
    Utf8String validJsonString(R"json({
  "arrayOfStructs": [
    {
      "city": {
        "country": "US",
        "name": "Exton",
        "state": "PA",
        "zip": 19341.0
      },
      "street": "690 Pennsylvania Drive"
    },
    {
      "city": {
        "country": "US",
        "name": "Philadelphia",
        "state": "PA",
        "zip": 19102.0
      },
      "street": "1601 Cherry Street"
    }
  ],
  "b": false,
  "category": "0x12",
  "classFullName": "DgnPlatformTest:TestElementWithNoHandler",
  "code": {
    "scope": "0x1",
    "spec": "0x1",
    "value": ""
  },
  "d": 3.14,
  "dt": "2018-02-04T00:00:00.000",
  "dtUtc": "2018-02-05T00:00:00.000Z",
  "i": 50.0,
  "iGeom": {
    "lineSegment": [
      [
        -21908.999,
        4111.625,
        0.0
      ],
      [
        -22956.749,
        4111.625,
        0.0
      ]
    ]
  },
  "id": "0x14",
  "l": 100.0,
  "model": "0x11",
  "p2d": {
    "x": 3.0,
    "y": 5.0
  },
  "p3d": {
    "x": 3.0,
    "y": 5.0,
    "z": 7.0
  },
  "placement": {
    "angles": null,
    "bbox": {
      "high": [
        -1.7976931348623157e+308,
        -1.7976931348623157e+308,
        -1.7976931348623157e+308
      ],
      "low": [
        1.7976931348623157e+308,
        1.7976931348623157e+308,
        1.7976931348623157e+308
      ]
    },
    "origin": [
      0.0,
      0.0,
      0.0
    ]
  },
  "s": "test string"
})json");

    Json::Value validJson;
    EXPECT_TRUE(Json::Reader::Parse(validJsonString, validJson));

    SetupSeedProject();

    DgnElementId elementId;
    DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());

    bool const b = false;
    double const d = 3.14;
    int32_t const i = 50;
    int64_t const l = 100;
    DateTime const dt(2018, 2, 4);
    DateTime const dtUtc(2018, 2, 5);
    auto const p2d = DPoint2d::From(3, 5);
    auto const p3d = DPoint3d::From(3, 5, 7);
    uint32_t iArrayOfStructs;

    { // Create Element
    TestElement el(params);

    Json::Value lineSegmentObj(Json::ValueType::objectValue);
    Json::Value lineSegments(Json::ValueType::arrayValue);
    Json::Value lineSegment(Json::ValueType::arrayValue);
    lineSegment[0u] = -21908.999;
    lineSegment[1u] = 4111.625;
    lineSegment[2u] = 0.0;

    lineSegments[0u] = lineSegment;

    Json::Value lineSegment2(Json::ValueType::arrayValue);
    lineSegment2[0u] = -22956.749;
    lineSegment2[1u] = 4111.625;
    lineSegment2[2u] = 0.0;

    lineSegments[1u] = lineSegment2;

    lineSegmentObj["lineSegment"] = lineSegments;

    IGeometryPtr geom = ECN::ECJsonUtilities::JsonToIGeometry(lineSegmentObj);

    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("b", b));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("d", d));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("dt", dt));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("dtUtc", dtUtc));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("i", i));
    ECN::ECValue val;
    val.SetIGeometry(*geom);
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("IGeom", val));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("l", l));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("s", "test string"));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("p2d", p2d));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("p3d", p3d));

    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(iArrayOfStructs, "ArrayOfStructs"));

    EXPECT_EQ(DgnDbStatus::Success, el.InsertPropertyArrayItems(iArrayOfStructs, 0, 4));

    ECN::ECClassCP locationStruct = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_LOCATION_STRUCT_CLASS_NAME);
    ASSERT_TRUE(nullptr != locationStruct);
    ECN::StandaloneECEnablerPtr locationEnabler = locationStruct->GetDefaultStandaloneEnabler();

    ECN::IECInstancePtr extonLocInstance = locationEnabler->CreateInstance().get();
    extonLocInstance->SetValue("Street", ECN::ECValue("690 Pennsylvania Drive"));
    extonLocInstance->SetValue("City.Name", ECN::ECValue("Exton"));
    extonLocInstance->SetValue("City.State", ECN::ECValue("PA"));
    extonLocInstance->SetValue("City.Country", ECN::ECValue("US"));
    extonLocInstance->SetValue("City.Zip", ECN::ECValue(19341));

    ECN::ECValue extonLocValue;
    extonLocValue.SetStruct(extonLocInstance.get());
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfStructs", extonLocValue, PropertyArrayIndex(1)));

    ECN::IECInstancePtr phillyLocInstance = locationEnabler->CreateInstance().get();
    phillyLocInstance->SetValue("Street", ECN::ECValue("1601 Cherry Street"));
    phillyLocInstance->SetValue("City.Name", ECN::ECValue("Philadelphia"));
    phillyLocInstance->SetValue("City.State", ECN::ECValue("PA"));
    phillyLocInstance->SetValue("City.Country", ECN::ECValue("US"));
    phillyLocInstance->SetValue("City.Zip", ECN::ECValue(19102));

    ECN::ECValue phillyLocValue;
    phillyLocValue.SetStruct(phillyLocInstance.get());
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfStructs", phillyLocValue, PropertyArrayIndex(2)));

    // Check that we see the stored value in memory in the correct JSON format.
    //   Needed because we handle in-memory element data using ECJsonUtilities and from Db with JsonECSqlSelectAdapter for auto-handled properties.
    // TODO: Fix ECJsonUtilities
    // EXPECT_STREQ(el.ToJson().ToString().c_str(), validJson.ToString().c_str()); // Uses ECJsonUtilities

    // Insert the element
    DgnDbStatus stat;
    DgnElementCPtr persistentEl = el.Insert(&stat);
    ASSERT_EQ(DgnDbStatus::Success, stat);
    ASSERT_TRUE(persistentEl.IsValid());

    // Check that we see the stored value in memory in the correct JSON format.
    //   Needed because we handle in-memory element data using ECJsonUtilities and from Db with JsonECSqlSelectAdapter for auto-handled properties.
    EXPECT_STREQ(persistentEl->ToJson().ToString().c_str(), validJson.ToString().c_str()); // Uses JsonECSqlSelectAdapter

    // persist the element into the db
    elementId = persistentEl->GetElementId();
    m_db->SaveChanges();
    }

    // Before updating the element check what is stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);

    TestElementCPtr element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(element.IsValid());

    // Check that we see the stored value in memory in the correct JSON format.
    //   Needed because we handle in-memory element data using ECJsonUtilities and from Db with JsonECSqlSelectAdapter for auto-handled properties.
    EXPECT_STREQ(element->ToJson().ToString().c_str(), validJson.ToString().c_str()); // Uses JsonECSqlSelectAdapter
    m_db->CloseDb();
    }

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Sam.Wilson      10/17
//----------------------------------------------------------------------------------------
struct ImodelJsTest : public DgnDbTestFixture
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImodelJsTest, MeasureInsertPerformance)
    {
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true); // Imports the DgnPlatformTest schema and sets the briefcaseid=standalone

    StopWatch timer(true);

    PhysicalModelPtr model = m_db->Models().Get<PhysicalModel>(m_defaultModelId);
    DgnModelId modelId = model->GetModelId();

    auto defaultCategoryId = SpatialCategory::QueryCategoryId(m_db->GetDictionaryModel(), "DefaultCategory");

    int elementCount = 10000;
    for (int i=0; i<elementCount; ++i)
        {
        DgnClassId ecClassId(m_db->Schemas().GetClassId("DgnPlatformTest", "ImodelJsTestElement").GetValue());
        DgnElementPtr el = new ImodelJsTestElement(ImodelJsTestElement::CreateParams(*m_db, modelId, ecClassId, defaultCategoryId, Placement3d()));
        el->SetPropertyValue("IntegerProperty1", i);
        el->SetPropertyValue("IntegerProperty2", i);
        el->SetPropertyValue("IntegerProperty3", i);
        el->SetPropertyValue("IntegerProperty4", i);
        el->SetPropertyValue("DoubleProperty1", (double)i);
        el->SetPropertyValue("DoubleProperty2", (double)i);
        el->SetPropertyValue("DoubleProperty3", (double)i);
        el->SetPropertyValue("DoubleProperty4", (double)i);
        el->SetPropertyValue("b", (0 == (i % 100)));
        DPoint3d pt = DPoint3d::From(i, 0, 0);
        el->SetPropertyValue("PointProperty1", pt);
        el->SetPropertyValue("PointProperty2", pt);
        el->SetPropertyValue("PointProperty3", pt);
        el->SetPropertyValue("PointProperty4", pt);
        //DateTime dtUtc;
        //DateTime::FromString(dtUtc, "2013-09-15 12:05:39Z");
        //el->SetPropertyValue("dtUtc", dtUtc);

        ASSERT_TRUE(el->Insert().IsValid());
        if (0 == (i % 100))
            m_db->SaveChanges();
        }

    m_db->SaveChanges();

    ECSqlStatement stmt;
    stmt.Prepare(*m_db, "select count(*) from DgnPlatformTest.ImodelJsTestElement");
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementCount, stmt.GetValueInt(0));

    timer.Stop();
    printf("ImodelJsTest.MeasureInsertPerformance %lf\n", timer.GetElapsedSeconds());
    }
