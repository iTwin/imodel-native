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

    //  CurvePrimitive
    //
    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,
        0, 0, 2,
        0, 3, 0,
        0.0, Angle::TwoPi());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc(ellipseData);
    EXPECT_TRUE(builder->Append(*ellipse));
    
    // Curve Vector
    //
    CurveVectorPtr curveVector =  GeomHelper::computeShape();
    EXPECT_TRUE(builder->Append(*curveVector));

    // SolidPrimitive
    //
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

    // SubCategory
    //
    DgnSubCategory::Appearance appearence;
    appearence.SetInvisible(false);
    appearence.SetColor(ColorDef::DarkRed());
    Utf8CP sub_code = "Test SubCategory";
    Utf8CP sub_desc = "This is a test subcategory";
    DgnSubCategory subCategory(DgnSubCategory::CreateParams(*m_db, m_defaultCategoryId, sub_code, appearence, sub_desc));
    DgnDbStatus status;
    DgnSubCategoryCPtr newSubCategory =  subCategory.Insert(&status);
    EXPECT_TRUE(DgnDbStatus::Success == status);
    EXPECT_TRUE(newSubCategory.IsValid());
    EXPECT_TRUE(newSubCategory->GetSubCategoryId().IsValid());
    EXPECT_TRUE(builder->Append(newSubCategory->GetSubCategoryId()));

    // MSBsplineSurface
    //
    double a = 1000.0 / 3.0;
    MSBsplineSurfacePtr surface = GeomHelper::CreateGridSurface(DPoint3d::From(0, 0, 0), a, a, 3, 4, 4);
    EXPECT_TRUE(builder->Append(*surface));

    // PolyfaceQuery
    //
    IFacetOptionsPtr options = IFacetOptions::Create();
    IPolyfaceConstructionPtr faceBuilder = IPolyfaceConstruction::Create(*options);
    faceBuilder->AddSweptNGon(4, 1.0, 0.0, 2.0, true, true);
    PolyfaceHeaderPtr mesh = faceBuilder->GetClientMeshPtr();
    EXPECT_TRUE(builder->Append(*mesh));

    // ISolidKernelEntityPtr
    //

    // TextString
    //
    
    TextStringPtr text = GeomHelper::CreateTextString();
    EXPECT_TRUE(builder->Append(*text));

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
