/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ElementGeometry_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Element Geometry 
* @bsimethod                                                    Umar.Hayat      09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementGeometryTests : public DgnDbTestFixture
{

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeometryTests, Create)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementGeometryTests_CreateElement3d.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    
    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));


    // CurvePrimitive
    //
    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,/**/  0, 0, 2, /**/ 0, 3, 0, /**/ 0.0, Angle::TwoPi());
    ICurvePrimitivePtr curvePrimitive = ICurvePrimitive::CreateArc(ellipseData);
    ElementGeometryPtr elmGeom = ElementGeometry::Create(*curvePrimitive);
    EXPECT_TRUE(elmGeom.IsValid());
    EXPECT_TRUE(ElementGeometry::GeometryType::CurvePrimitive == elmGeom->GetGeometryType());
    ICurvePrimitivePtr getAsCurvePrimitive = elmGeom->GetAsICurvePrimitive();
    EXPECT_TRUE(getAsCurvePrimitive.IsValid());

    // Curve Vector
    //
    CurveVectorPtr curveVector =  GeomHelper::computeShape();
    ElementGeometryPtr elmGeom2 = ElementGeometry::Create(*curveVector);
    EXPECT_TRUE(elmGeom2.IsValid());
    EXPECT_TRUE(ElementGeometry::GeometryType::CurveVector == elmGeom2->GetGeometryType());
    CurveVectorPtr getAsCurveVector = elmGeom2->GetAsCurveVector();
    EXPECT_TRUE(getAsCurveVector.IsValid());

    // SolidPrimitive
    //
    double dz = 3.0;
    double radius = 1.5;
    DgnConeDetail cylinderDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius, true);
    ISolidPrimitivePtr solidPrimitive = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    ElementGeometryPtr elmGeom3 = ElementGeometry::Create(*solidPrimitive);
    EXPECT_TRUE(elmGeom3.IsValid());
    EXPECT_TRUE(ElementGeometry::GeometryType::SolidPrimitive == elmGeom3->GetGeometryType());
    ISolidPrimitivePtr getAsSolid = elmGeom3->GetAsISolidPrimitive();
    EXPECT_TRUE(getAsSolid.IsValid());

    // MSBsplineSurface
    //
    double a = 1000.0 / 3.0;
    MSBsplineSurfacePtr surface = GeomHelper::CreateGridSurface(DPoint3d::From(0, 0, 0), a, a, 3, 4, 4);
    ElementGeometryPtr elmGeom4 = ElementGeometry::Create(*surface);
    EXPECT_TRUE(elmGeom4.IsValid());
    EXPECT_TRUE(ElementGeometry::GeometryType::BsplineSurface == elmGeom4->GetGeometryType());
    MSBsplineSurfacePtr getAsMesh = elmGeom4->GetAsMSBsplineSurface();
    EXPECT_TRUE(getAsMesh.IsValid());

    // PolyfaceQuery
    //
    IFacetOptionsPtr options = IFacetOptions::Create();
    IPolyfaceConstructionPtr faceBuilder = IPolyfaceConstruction::Create(*options);
    faceBuilder->AddSweptNGon(4, 1.0, 0.0, 2.0, true, true);
    PolyfaceHeaderPtr mesh = faceBuilder->GetClientMeshPtr();
    ElementGeometryPtr elmGeom5 = ElementGeometry::Create(*mesh);
    EXPECT_TRUE(elmGeom5.IsValid());
    EXPECT_TRUE(ElementGeometry::GeometryType::Polyface == elmGeom5->GetGeometryType());
    PolyfaceHeaderPtr getAsPolyFace = elmGeom5->GetAsPolyfaceHeader();
    EXPECT_TRUE(getAsPolyFace.IsValid());

    // ISolidKernelEntityPtr
    //

    // TextString
    //
    
    TextStringPtr text = GeomHelper::CreateTextString();
    ElementGeometryPtr elmGeom6 = ElementGeometry::Create(*text);
    EXPECT_TRUE(elmGeom6.IsValid());
    EXPECT_TRUE(ElementGeometry::GeometryType::TextString == elmGeom6->GetGeometryType());
    TextStringPtr getAsTexTString = elmGeom6->GetAsTextString();
    EXPECT_TRUE(getAsTexTString.IsValid());
    EXPECT_STREQ(text->GetText().c_str(), getAsTexTString->GetText().c_str());


    }
