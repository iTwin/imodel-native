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
    SetupProject(L"3dMetricGeneral.idgndb", L"ElemGeometryBuilder.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnElementPtr el = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "Test1");

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    GeometricElementP geomElem = const_cast<GeometricElementP>(el->ToGeometricElement());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));
    CurveVectorPtr curveVector =  GeomHelper::computeShape();
    builder->Append(*curveVector);
    EXPECT_EQ(SUCCESS, builder->SetGeomStreamAndPlacement(*geomElem));
    EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeometryBuilderTests, CreateElement2d)
{
    SetupProject(L"2dMetricGeneral.idgndb", L"ElemGeometryBuilder.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnElementPtr el = TestElement2d::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "Test1");

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    GeometricElementP geomElem = const_cast<GeometricElementP>(el->ToGeometricElement());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, m_defaultCategoryId, DPoint2d::From(0.0, 0.0));
    TextString textStringElem;
    builder->Append(textStringElem);
    EXPECT_EQ(SUCCESS, builder->SetGeomStreamAndPlacement(*geomElem));
    EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());
}
