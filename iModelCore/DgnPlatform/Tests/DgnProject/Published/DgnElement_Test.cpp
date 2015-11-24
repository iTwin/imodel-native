/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnElement_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct DgnElementTests : public DgnDbTestFixture
    {
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
    auto keyE1 = InsertElement(DgnElement::Code(), M1id);
    DgnElementId E1id = keyE1->GetElementId();
    DgnElementCPtr E1 = m_db->Elements().GetElement(E1id);
    EXPECT_TRUE (E1 != nullptr);

    auto keyE2 = InsertElement(DgnElement::Code(), M1id);
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

    auto keyE1 = InsertElement(DgnElement::Code(), m1id);
    DgnElementId e1id = keyE1->GetElementId();
    DgnElementCPtr e1 = m_db->Elements().GetElement(e1id);
    EXPECT_TRUE(e1 != nullptr);

    //Creating a copy of element to edit.
    DgnElementPtr e1Copy = e1->CopyForEdit();
    dynamic_cast<TestElement*>(e1Copy.get())->SetTestElementProperty("Updated Test Element");

    DgnElementCPtr updatedElement = e1Copy->Update();

    EXPECT_STREQ("Updated Test Element", dynamic_cast<TestElement const*>(updatedElement.get())->GetTestElementProperty().c_str());
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

