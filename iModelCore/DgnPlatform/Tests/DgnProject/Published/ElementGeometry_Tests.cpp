/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Element Geometry 
* @bsimethod                                                    Umar.Hayat      09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct GeometricPrimitiveTests : public DgnDbTestFixture
{
    static IFacetOptionsPtr CreateFacetOptions(double chordTolerance)
        {
        IFacetOptionsPtr opts = IFacetOptions::Create();

        opts->SetChordTolerance(chordTolerance);
        opts->SetMaxPerFace(3);
        opts->SetCurvedSurfaceMaxPerFace(3);
        opts->SetParamsRequired(true);
        opts->SetNormalsRequired(true);

        return opts;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeometricPrimitiveTests, Create)
    {
    SetupSeedProject();
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));

    // CurvePrimitive
    //
    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,/**/  0, 0, 2, /**/ 0, 3, 0, /**/ 0.0, Angle::TwoPi());
    ICurvePrimitivePtr curvePrimitive = ICurvePrimitive::CreateArc(ellipseData);
    GeometricPrimitivePtr elmGeom = GeometricPrimitive::Create(*curvePrimitive);
    ASSERT_TRUE(elmGeom.IsValid());
    EXPECT_TRUE(GeometricPrimitive::GeometryType::CurvePrimitive == elmGeom->GetGeometryType());
    ICurvePrimitivePtr getAsCurvePrimitive = elmGeom->GetAsICurvePrimitive();
    EXPECT_TRUE(getAsCurvePrimitive.IsValid());
    EXPECT_TRUE(elmGeom->IsWire());
    EXPECT_FALSE(elmGeom->IsSheet());
    EXPECT_FALSE(elmGeom->IsSolid());
    //Clone CurvePrimitive
    GeometricPrimitivePtr elmGeomC = elmGeom->Clone();
    EXPECT_TRUE(elmGeomC.IsValid());
    EXPECT_TRUE(elmGeomC->GetGeometryType() == elmGeom->GetGeometryType());

    // Curve Vector
    //
    CurveVectorPtr curveVector = GeomHelper::computeShape();
    GeometricPrimitivePtr elmGeom2 = GeometricPrimitive::Create(*curveVector);
    ASSERT_TRUE(elmGeom2.IsValid());
    EXPECT_TRUE(GeometricPrimitive::GeometryType::CurveVector == elmGeom2->GetGeometryType());
    CurveVectorPtr getAsCurveVector = elmGeom2->GetAsCurveVector();
    EXPECT_TRUE(getAsCurveVector.IsValid());
    EXPECT_TRUE(elmGeom2->IsWire());
    EXPECT_FALSE(elmGeom2->IsSheet());
    EXPECT_FALSE(elmGeom2->IsSolid());
    //Clone CurveVector Primitive
    GeometricPrimitivePtr elmGeom2C = elmGeom2->Clone();
    EXPECT_TRUE(elmGeom2C.IsValid());
    EXPECT_TRUE(elmGeom2C->GetGeometryType() == elmGeom2->GetGeometryType());

    // SolidPrimitive
    //
    double dz = 3.0;
    double radius = 1.5;
    DgnConeDetail cylinderDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius, true);
    ISolidPrimitivePtr solidPrimitive = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    GeometricPrimitivePtr elmGeom3 = GeometricPrimitive::Create(*solidPrimitive);
    ASSERT_TRUE(elmGeom3.IsValid());
    EXPECT_TRUE(GeometricPrimitive::GeometryType::SolidPrimitive == elmGeom3->GetGeometryType());
    ISolidPrimitivePtr getAsSolid = elmGeom3->GetAsISolidPrimitive();
    EXPECT_TRUE(getAsSolid.IsValid());
    EXPECT_FALSE(elmGeom3->IsWire());
    EXPECT_FALSE(elmGeom3->IsSheet());
    EXPECT_TRUE(elmGeom3->IsSolid());
    //Clone SolidPrimitive
    GeometricPrimitivePtr elmGeom3C = elmGeom3->Clone();
    EXPECT_TRUE(elmGeom3C.IsValid());
    EXPECT_TRUE(elmGeom3C->GetGeometryType() == elmGeom3->GetGeometryType());

    // MSBsplineSurface
    //
    double a = 1000.0 / 3.0;
    MSBsplineSurfacePtr surface = GeomHelper::CreateGridSurface(DPoint3d::From(0, 0, 0), a, a, 3, 4, 4);
    GeometricPrimitivePtr elmGeom4 = GeometricPrimitive::Create(*surface);
    ASSERT_TRUE(elmGeom4.IsValid());
    EXPECT_TRUE(GeometricPrimitive::GeometryType::BsplineSurface == elmGeom4->GetGeometryType());
    MSBsplineSurfacePtr getAsMesh = elmGeom4->GetAsMSBsplineSurface();
    EXPECT_TRUE(getAsMesh.IsValid());
    EXPECT_FALSE(elmGeom4->IsWire());
    EXPECT_TRUE(elmGeom4->IsSheet());
    EXPECT_FALSE(elmGeom4->IsSolid());
    //Clone MSBsplineSurface
    GeometricPrimitivePtr elmGeom4C = elmGeom4->Clone();
    EXPECT_TRUE(elmGeom4C.IsValid());
    EXPECT_TRUE(elmGeom4C->GetGeometryType() == elmGeom4->GetGeometryType());

    // PolyfaceQuery
    //
    IFacetOptionsPtr options = IFacetOptions::Create();
    IPolyfaceConstructionPtr faceBuilder = IPolyfaceConstruction::Create(*options);
    faceBuilder->AddSweptNGon(4, 1.0, 0.0, 2.0, true, true);
    PolyfaceHeaderPtr mesh = faceBuilder->GetClientMeshPtr();
    GeometricPrimitivePtr elmGeom5 = GeometricPrimitive::Create(*mesh);
    ASSERT_TRUE(elmGeom5.IsValid());
    EXPECT_TRUE(GeometricPrimitive::GeometryType::Polyface == elmGeom5->GetGeometryType());
    PolyfaceHeaderPtr getAsPolyFace = elmGeom5->GetAsPolyfaceHeader();
    EXPECT_TRUE(getAsPolyFace.IsValid());
    EXPECT_FALSE(elmGeom5->IsWire());
    EXPECT_FALSE(elmGeom5->IsSheet());
    EXPECT_TRUE(elmGeom5->IsSolid());
    //Clone Polyface
    GeometricPrimitivePtr elmGeom5C = elmGeom5->Clone();
    EXPECT_TRUE(elmGeom5C.IsValid());
    EXPECT_TRUE(elmGeom5C->GetGeometryType() == elmGeom5->GetGeometryType());

    #ifdef BENTLEYCONFIG_PARASOLID
    // IBRepEntityPtr
    //
    IBRepEntityPtr out;
    EXPECT_EQ(BentleyStatus::SUCCESS, BRepUtil::Create::BodyFromBSurface(out, *surface));
    GeometricPrimitivePtr elmGeom6 = GeometricPrimitive::Create(out);
    ASSERT_TRUE(elmGeom6.IsValid());
    EXPECT_TRUE(GeometricPrimitive::GeometryType::BRepEntity == elmGeom6->GetGeometryType());
    IBRepEntityPtr getAsIBRepEntity = elmGeom6->GetAsIBRepEntity();
    EXPECT_TRUE(getAsIBRepEntity.IsValid());
    EXPECT_FALSE(elmGeom6->IsWire());
    EXPECT_TRUE(elmGeom6->IsSheet());
    EXPECT_FALSE(elmGeom6->IsSolid());
    //Clone IBRepEntity
    GeometricPrimitivePtr elmGeom6C = elmGeom6->Clone();
    EXPECT_TRUE(elmGeom6C.IsValid());
    EXPECT_TRUE(elmGeom6C->GetGeometryType() == elmGeom6->GetGeometryType());
    #endif

    // TextString
    //
    TextStringPtr text = GeomHelper::CreateTextString();
    GeometricPrimitivePtr elmGeom7 = GeometricPrimitive::Create(*text);
    ASSERT_TRUE(elmGeom7.IsValid());
    EXPECT_TRUE(GeometricPrimitive::GeometryType::TextString == elmGeom7->GetGeometryType());
    TextStringPtr getAsTexTString = elmGeom7->GetAsTextString();
    ASSERT_TRUE(getAsTexTString.IsValid());
    EXPECT_STREQ(text->GetText().c_str(), getAsTexTString->GetText().c_str());
    EXPECT_FALSE(elmGeom7->IsWire());
    EXPECT_FALSE(elmGeom7->IsSheet());
    EXPECT_FALSE(elmGeom7->IsSolid());
    //Clone TestString
    GeometricPrimitivePtr elmGeom7C = elmGeom7->Clone();
    EXPECT_TRUE(elmGeom7C.IsValid());
    EXPECT_TRUE(elmGeom7C->GetGeometryType() == elmGeom7->GetGeometryType());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeometricPrimitiveTests, FacetCounts)
    {
    double dz = 3.0;
    double radius = 1.5;
    DgnConeDetail cylinderDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius, true);
    ISolidPrimitivePtr solidPrimitive = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    ASSERT_TRUE(solidPrimitive.IsValid());

    IFacetOptionsPtr opts = CreateFacetOptions(0.01);
    FacetCounter facetCounter(*opts);
    size_t facetCountApprox = facetCounter.GetFacetCount(*solidPrimitive);

    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(*opts);
    polyfaceBuilder->AddSolidPrimitive(*solidPrimitive);
    PolyfaceHeaderPtr polyface = polyfaceBuilder->GetClientMeshPtr();
    ASSERT_TRUE(polyface.IsValid());

    size_t actualFacetCount = polyface->GetNumFacet();
    EXPECT_EQ(facetCountApprox, actualFacetCount);
    }

