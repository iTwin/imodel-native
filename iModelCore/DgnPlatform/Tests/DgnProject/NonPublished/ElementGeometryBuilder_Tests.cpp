/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementGeometryBuilder_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Element Geometry Builder
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementGeometryBuilderTests : public DgnDbTestFixture
{

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeometryBuilderTests, CreateElement3d)
{
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementGeometryBuilderTests_CreateElement3d.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnElementPtr el = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnElement::Code());

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    GeometricElementP geomElem = const_cast<GeometricElementP>(el->ToGeometricElement());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));
    CurveVectorPtr curveVector =  GeomHelper::computeShape();
    EXPECT_TRUE(builder->Append(*curveVector));

    double dz = 3.0;
    double radius = 1.5;
    DgnConeDetail cylinderDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius, true);
    ISolidPrimitivePtr cylinder = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    EXPECT_TRUE(builder->Append(*cylinder));

    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,
        0, 0, 2,
        0, 3, 0,
        0.0, Angle::TwoPi());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc(ellipseData);
    EXPECT_TRUE(builder->Append(*ellipse));

    ElemDisplayParams elemDisplayParams;
    elemDisplayParams.SetCategoryId(m_defaultCategoryId);
    elemDisplayParams.SetWeight(2);
    EXPECT_TRUE( builder->Append(elemDisplayParams));

    EXPECT_EQ(SUCCESS, builder->SetGeomStreamAndPlacement(*geomElem));
    EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeometryBuilderTests, CreateElement2d)
{
    SetupProject(L"2dMetricGeneral.idgndb", L"ElementGeometryBuilderTests_CreateElement2d.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnElementPtr el = TestElement2d::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnElement::Code());

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    GeometricElementP geomElem = const_cast<GeometricElementP>(el->ToGeometricElement());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, m_defaultCategoryId, DPoint2d::From(0.0, 0.0));
    TextString textStringElem;
    textStringElem.SetText("If we have no text we have no range and insert fails");
    EXPECT_TRUE(builder->Append(textStringElem));

    // 3d should not be appended
    double dz = 3.0;
    double radius = 1.5;
    DgnConeDetail cylinderDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius, true);
    ISolidPrimitivePtr cylinder = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    BeTest::SetFailOnAssert(false);
    EXPECT_FALSE(builder->Append(*cylinder));
    BeTest::SetFailOnAssert(true);

    EXPECT_EQ(SUCCESS, builder->SetGeomStreamAndPlacement(*geomElem));
    EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());

    DgnElementPtr el2 = TestElement2d::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
    DgnElement2dP dgnElement2d= el2->ToElement2dP();
    DPoint2d origin = DPoint2d::From(0.0, 0.0);
    ElementGeometryBuilderPtr builder2 = ElementGeometryBuilder::Create(*dgnElement2d, origin);
}
