/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnElement_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

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
TEST_F (DgnElementTests, ResetStatistics)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"Element_Test.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelPtr seedModel = m_db->Models().GetModel(m_defaultModelId);
    seedModel->FillModel();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts a model
    DgnModelPtr M1 = seedModel->Clone(DgnModel::CreateModelCode("Model1"));
    M1->Insert("Test Model 1");
    EXPECT_TRUE (M1 != nullptr);
    m_db->SaveChanges("changeSet1");

    DgnModelId M1id = m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1"));
    EXPECT_TRUE (M1id.IsValid());

    //Inserts 2 elements.
    auto keyE1 = InsertElement(M1id);
    DgnElementId E1id = keyE1->GetElementId();
    DgnElementCPtr E1 = m_db->Elements().GetElement(E1id);
    EXPECT_TRUE (E1 != nullptr);

    auto keyE2 = InsertElement(M1id);
    DgnElementId E2id = keyE2->GetElementId();
    DgnElementCPtr E2 = m_db->Elements().GetElement(E2id);
    EXPECT_TRUE (E2 != nullptr);

    DgnModelId model_id = m_db->Elements().QueryModelId(E1id);
    EXPECT_EQ (M1id, model_id);

    //Deletes the first element.
    DgnDbStatus status=E2->Delete();
    EXPECT_EQ ((DgnDbStatus)SUCCESS, status);
    m_db->SaveChanges();

    int64_t memTarget = 0;
    m_db->Elements().Purge(memTarget);

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
    SetupProject(L"3dMetricGeneral.idgndb", L"Element_Test.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelPtr seedModel = m_db->Models().GetModel(m_defaultModelId);
    seedModel->FillModel();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts a model
    DgnModelPtr m1 = seedModel->Clone(DgnModel::CreateModelCode("Model1"));
    m1->Insert("Test Model 1");
    EXPECT_TRUE(m1 != nullptr);
    m_db->SaveChanges("changeSet1");

    DgnModelId m1id = m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1"));
    EXPECT_TRUE(m1id.IsValid());

    auto keyE1 = InsertElement(m1id);
    DgnElementId e1id = keyE1->GetElementId();
    DgnElementCPtr e1 = m_db->Elements().GetElement(e1id);
    EXPECT_TRUE(e1 != nullptr);

#ifdef WIP_ELEMENT_ITEM // *** pending redesign
    //Creating a copy of element to edit.
    DgnElementPtr e1Copy = e1->CopyForEdit();
    dynamic_cast<TestElement*>(e1Copy.get())->SetTestElementProperty("Updated Test Element");

    DgnElementCPtr updatedElement = e1Copy->Update();

    EXPECT_STREQ("Updated Test Element", dynamic_cast<TestElement const*>(updatedElement.get())->GetTestElementProperty().c_str());
#endif
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
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests_DgnEditElementCollector.idgndb", Db::OpenMode::ReadWrite);

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
            ASSERT_EQ(45, el->ToGeometrySource3d()->GetPlacement().GetAngles().GetYaw().Degrees());
            ASSERT_EQ(0, el->ToGeometrySource3d()->GetPlacement().GetAngles().GetPitch().Degrees()) << "pitch should be unaffected";
            ASSERT_EQ(0, el->ToGeometrySource3d()->GetPlacement().GetAngles().GetRoll().Degrees()) << "roll should be unaffected";
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
        ASSERT_EQ(45, eparentplacement.GetAngles().GetYaw().Degrees());
        ASSERT_EQ(45, ec11placement.GetAngles().GetYaw().Degrees());
        ASSERT_EQ( 0, ec11placement.GetAngles().GetPitch().Degrees()) << "pitch should have been unaffected";
        ASSERT_EQ( 0, ec11placement.GetAngles().GetRoll().Degrees()) << "roll should have been unaffected";
        ASSERT_TRUE(eparentplacement.GetOrigin().AlmostEqual(DPoint3d::FromZero()));
        ASSERT_TRUE(ec11placement.GetOrigin().AlmostEqual(DPoint3d::From(0,sqrt(2),0)));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, DgnEditElementCollector)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests_DgnEditElementCollector.idgndb", Db::OpenMode::ReadWrite);

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
* @bsistruct                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementGeomAndPlacementTests : DgnElementTests
{
    ElementGeometryBuilderPtr CreateGeom();
    RefCountedPtr<DgnElement3d> CreateElement(bool wantPlacement, bool wantGeom);
    static bool AreEqualPlacements(Placement3dCR a, Placement3dCR b);
    void TestLoadElem(DgnElementId id, Placement3d const* placement, bool hasGeometry);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementGeometryBuilderPtr ElementGeomAndPlacementTests::CreateGeom()
    {
    DgnModelPtr model = m_db->Models().GetModel(m_defaultModelId);
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));
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

    ElemDisplayParams elemDisplayParams;
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
    auto el = m_db->Elements().Get<DgnElement3d>(id);
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
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementGeomAndPlacement.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
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
        el = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
        auto geom = CreateGeom();
        EXPECT_EQ(SUCCESS, geom->SetGeomStreamAndPlacement(*el));
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
        el = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
        EXPECT_EQ(SUCCESS, geom->SetGeomStreamAndPlacement(*el));
        el->SetPlacement(Placement3d());
        DgnDbStatus status;
        EXPECT_FALSE(m_db->Elements().Insert(*el, &status).IsValid());
        EXPECT_EQ(DgnDbStatus::BadElement, status);
        }

    m_db->Elements().Purge(0);

    TestLoadElem(noPlacementNoGeomId, nullptr, false);
    TestLoadElem(placementAndGeomId, &placement, true);
    TestLoadElem(placementAndNoGeomId, &placement, false);
    }

