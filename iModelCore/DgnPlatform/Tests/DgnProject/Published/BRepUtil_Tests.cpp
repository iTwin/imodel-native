/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"

#ifdef BENTLEYCONFIG_PARASOLID

//=======================================================================================
// @bsiclass                                                 Ridha.Malik   01/17
//=======================================================================================
struct BRepUtilTests : public DgnDbTestFixture
{
  
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CreateBodyFrom_SolidPrimDgncone)
    {
    //Dgncone
    double dz = 3.0;
    double radius = 1.5;
    DgnConeDetail cylinderDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius+radius, true);
    ISolidPrimitivePtr solidPrimitive = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    ASSERT_TRUE(!solidPrimitive.IsNull());
    IBRepEntityPtr brep;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromSolidPrimitive(brep, *solidPrimitive));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(3,BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(2, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(0, BRepUtil::GetBodyVertices(NULL, *brep));
    ASSERT_FALSE(BRepUtil::IsDisjointBody(*brep));

    //DgnCylinder
    DgnConeDetail coneDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius, true);
    solidPrimitive = ISolidPrimitive::CreateDgnCone(coneDetail);
    ASSERT_TRUE(!solidPrimitive.IsNull());
    brep=nullptr;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromSolidPrimitive(brep, *solidPrimitive));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(3, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(2, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(0, BRepUtil::GetBodyVertices(NULL, *brep));
    ASSERT_FALSE(BRepUtil::IsDisjointBody(*brep));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CreateBodyFrom_SolidPrimDgnSphere)
    {
    //Dgn Sphere 
    DgnSphereDetail dgnsphere(DPoint3d::From(0, 0, 0), 10);
    ISolidPrimitivePtr myDgnSphere = ISolidPrimitive::CreateDgnSphere(dgnsphere);
    ASSERT_TRUE(!myDgnSphere.IsNull());
    IBRepEntityPtr brep;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromSolidPrimitive(brep, *myDgnSphere));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(1, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(0, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(0, BRepUtil::GetBodyVertices(NULL, *brep));
    ASSERT_FALSE(BRepUtil::IsDisjointBody(*brep));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CreateBodyFrom_SolidPrimDgnbox)
    {
    DgnBoxDetail Dgnbox(
        DPoint3d::From(1, 1, 1),
        DPoint3d::From(1, 1, 2),
        DVec3d::From(1, 0, 0), DVec3d::From(0, 1, 0), 2, 2, 2, 2, true);
    ISolidPrimitivePtr myDgnbox = ISolidPrimitive::CreateDgnBox(Dgnbox);
    ASSERT_TRUE(!myDgnbox.IsNull());
    IBRepEntityPtr brep;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromSolidPrimitive(brep, *myDgnbox));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(6, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(12, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(8, BRepUtil::GetBodyVertices(NULL, *brep));
    ASSERT_FALSE(BRepUtil::IsDisjointBody(*brep));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CreateBodyFrom_SolidPrimDgnTorus)
    {
    DgnTorusPipeDetail torus(
        DPoint3d::From(0, 0, 0),
        DVec3d::From(1, 0, 0),
        DVec3d::From(0, 1, 0),
        2,
        0.5,
        Angle::PiOver2(),
        true);
    ISolidPrimitivePtr Dgntorus = ISolidPrimitive::CreateDgnTorusPipe(torus);
    ASSERT_TRUE(!Dgntorus.IsNull());
    IBRepEntityPtr brep;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromSolidPrimitive(brep, *Dgntorus));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(3, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(2, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(0, BRepUtil::GetBodyVertices(NULL, *brep));
    ASSERT_FALSE(BRepUtil::IsDisjointBody(*brep));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CreateBodyFrom_SolidPrimDgnExtrusion)
    {
    CurveVectorPtr ellipse = CurveVector::CreateDisk(DEllipse3d::From(0, 0, 0, 1.5, 0, 0, 0, 1.5, 0, 0.0, Angle::TwoPi()));
    ASSERT_TRUE(ellipse.IsValid());
    DgnExtrusionDetail DgnExtrusion(ellipse, DVec3d::From(0, 0, 3), true);
    ISolidPrimitivePtr myDgnExtrusion = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusion);
    ASSERT_TRUE(myDgnExtrusion.IsValid());
    IBRepEntityPtr brep;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromSolidPrimitive(brep, *myDgnExtrusion));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(3, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(2, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(0, BRepUtil::GetBodyVertices(NULL, *brep));
    ASSERT_FALSE(BRepUtil::IsDisjointBody(*brep));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CreateBodyFrom_SolidPrimDgnRotationalsweep)
    {
    CurveVectorPtr rect = CurveVector::CreateRectangle(1,2,3,4,0.5);
    ASSERT_TRUE(rect.IsValid());
    DPoint3d center = DPoint3d::From(0, 0, 0);
    DVec3d   axis = DVec3d::From(0, 1, 0);
    DgnRotationalSweepDetail rotationalSweepData(rect, center, axis, Angle::Pi(), false);
    ISolidPrimitivePtr mySweepData = ISolidPrimitive::CreateDgnRotationalSweep(rotationalSweepData);
    ASSERT_TRUE(mySweepData.IsValid());
    IBRepEntityPtr brep;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromSolidPrimitive(brep, *mySweepData));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(4, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(12, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(8, BRepUtil::GetBodyVertices(NULL, *brep));
    ASSERT_FALSE(BRepUtil::IsDisjointBody(*brep));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CreateBodyFrom_SolidPrimDgnRuledSweep)
    {
    double ax = 1.0;
    double ay = 2.0;
    double bx = 3.0;
    double by = 4.0;
    double z0 = 0.5;
    double dz = 4.0;
    double z1 = z0 + dz;
    CurveVectorPtr sec1 = CurveVector::CreateRectangle(ax, ay, bx, by, z0, CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_TRUE(sec1.IsValid());
    CurveVectorPtr sec2 = CurveVector::CreateRectangle(ax, ay, bx, by, z1, CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_TRUE(sec2.IsValid());
    DgnRuledSweepDetail DgnRuledSweepDetail (sec1, sec2, true);
    ISolidPrimitivePtr RuledSweep = ISolidPrimitive::CreateDgnRuledSweep(DgnRuledSweepDetail);
    ASSERT_TRUE(RuledSweep.IsValid());
    IBRepEntityPtr brep;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromSolidPrimitive(brep, *RuledSweep));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(6, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(12, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(8, BRepUtil::GetBodyVertices(NULL, *brep));
    ASSERT_FALSE(BRepUtil::IsDisjointBody(*brep));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CreateBodyFrom_BSurface)
    {
    ICurvePrimitivePtr curvep=ICurvePrimitive::CreateLine(DSegment3d::From(0,1,0,5,5,0));
    ASSERT_TRUE(curvep.IsValid());
    MSBsplineSurfacePtr sp=MSBsplineSurface::CreateLinearSweep(*curvep, DVec3d::From(0,1,0));
    ASSERT_TRUE(sp.IsValid());
    IBRepEntityPtr brep;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromBSurface(brep, *sp));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(1, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(4, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(4, BRepUtil::GetBodyVertices(NULL, *brep));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CreateBodyFromAndToCurveVector)
    {
    CurveVectorPtr rect = CurveVector::CreateRectangle(1, 2, 3, 4, 0.5);
    ASSERT_TRUE(rect.IsValid());
    IBRepEntityPtr brep;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromCurveVector(brep, *rect));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(1, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(4, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(4, BRepUtil::GetBodyVertices(NULL, *brep));
    CurveVectorPtr result =BRepUtil::Create::BodyToCurveVector(*brep);
    ASSERT_TRUE(!result.IsNull());
    ASSERT_EQ(CurveVector::BOUNDARY_TYPE_Outer,result->GetBoundaryType());
    ASSERT_EQ(rect->Length(), result->Length());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CreateBodyFromExtrusionToBody)
    {
    CurveVectorPtr rect = CurveVector::CreateRectangle(1, 2, 3, 4, 0.5);
    ASSERT_TRUE(rect.IsValid());
    IBRepEntityPtr profile;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromCurveVector(profile, *rect));
    ASSERT_TRUE(!profile.IsNull());
    CurveVectorPtr sec1 = CurveVector::CreateRectangle( 1, 2, 3, 4, 0.5);
    CurveVectorPtr sec2 = CurveVector::CreateRectangle( 1, 2, 3, 4, 4.5);
    DgnRuledSweepDetail DgnRuledSweepDetail(sec1, sec2, true);
    ISolidPrimitivePtr RuledSweep = ISolidPrimitive::CreateDgnRuledSweep(DgnRuledSweepDetail);
    IBRepEntityPtr brep, extrudeto;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromSolidPrimitive(extrudeto, *RuledSweep));
    ASSERT_TRUE(!extrudeto.IsNull());
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromExtrusionToBody(brep, *extrudeto,*profile,false));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(6, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(12, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(8, BRepUtil::GetBodyVertices(NULL, *brep));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CreateBodyFromPolyface)
    {
    IFacetOptionsPtr options = IFacetOptions::Create();
    IPolyfaceConstructionPtr faceBuilder = IPolyfaceConstruction::Create(*options);
    faceBuilder->AddSweptNGon(4, 1.0, 0.0, 2.0, true, true);
    PolyfaceHeaderPtr mesh = faceBuilder->GetClientMeshPtr();
    ASSERT_TRUE(mesh.IsValid());
    IBRepEntityPtr brep;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromPolyface(brep, *mesh));
    ASSERT_EQ(6, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(12, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(8, BRepUtil::GetBodyVertices(NULL, *brep));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CutProfileBodyFromOpenCurveVector)
    {
    CurveVectorPtr rect = CurveVector::CreateRectangle(1, 2, 3, 4, 0.5);
    ASSERT_TRUE(rect.IsValid());
    IBRepEntityPtr profile;
    IBRepEntityPtr brep;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::CutProfileBodyFromOpenCurveVector(brep, *rect, DRange3d::From(DPoint3d::From(1,1,1))));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(1, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(4, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(4, BRepUtil::GetBodyVertices(NULL, *brep));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CreateBodyFromSweep)
    {
    CurveVectorPtr rect = CurveVector::CreateRectangle(1, 2, 3, 4, 0.5);
    ASSERT_TRUE(rect.IsValid());
    bvector<DPoint3d> points;
    points.push_back(DPoint3d::From(1, 2, 0.5));
    points.push_back(DPoint3d::From(3, 4, 0.5));
    CurveVectorPtr sec2 = CurveVector::CreateLinear(points);
    IBRepEntityPtr brep,brep2;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromSweep(brep, *rect, *sec2, true,true,true));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(4, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(12, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(8, BRepUtil::GetBodyVertices(NULL, *brep));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, CreateBodyFromLoft)
    {   
    IBRepEntityPtr brep;
    bvector<CurveVectorPtr>profiles;
    bvector<DPoint3d> Points;
    Points.push_back(DPoint3d::From(1, 2, 0.5));
    Points.push_back(DPoint3d::From(3, 2, 0.5));
    CurveVectorPtr line = CurveVector::CreateLinear(Points);
    profiles.push_back(line);
    Points.clear();
    Points.push_back(DPoint3d::From(1, 5, 0.5));
    Points.push_back(DPoint3d::From(3, 5, 0.5));
    CurveVectorPtr line2 = CurveVector::CreateLinear(Points);
    profiles.push_back(line2);
    bvector<CurveVectorPtr>guides;
    ICurvePrimitivePtr arc1 = ICurvePrimitive::CreateArc(DEllipse3d::FromVectors(DPoint3d::From(1, 3.5, 0.5), DVec3d::From(0.0, -1.5, 0.0), DVec3d::From(0.0, 0.0, 1.5), 0.0, Angle::Pi()));
    guides.push_back(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, arc1));
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromLoft(brep, profiles, &guides, false));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(1, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(4, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(4, BRepUtil::GetBodyVertices(NULL, *brep));
    }

Dgn::IBRepEntityPtr createSheet()
    {
    CurveVectorPtr curve = CurveVector::CreateRectangle(0, 0, 10, 10, 0);
    Dgn::IBRepEntityPtr sheet;
    BRepUtil::Create::BodyFromCurveVector(sheet, *curve);

    return sheet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Elonas.Seviakovas 07/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, IntersectSheetFaces_ParallelSheets_ReturnsNullptr)
    {   
    Dgn::IBRepEntityPtr sheet1 = createSheet();
    sheet1->ApplyTransform(Transform::From(DPoint3d::From(0.0, 0.0, 1.0)));

    Dgn::IBRepEntityPtr sheet2 = createSheet();

    CurveVectorPtr intersectionCurve;
    BRepUtil::Modify::IntersectSheetFaces(intersectionCurve, *sheet1, *sheet2);

    ASSERT_TRUE(intersectionCurve.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Elonas.Seviakovas 07/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, IntersectSheetFaces_CrossingSheets_ReturnsAnIntersection)
    {   
    Dgn::IBRepEntityPtr sheet1 = createSheet();
    sheet1->ApplyTransform(Transform::From(RotMatrix::FromRotate90(DVec3d::UnitX())));
    sheet1->ApplyTransform(Transform::From(DPoint3d::From(-5.0, 0.0, -5.0)));

    Dgn::IBRepEntityPtr sheet2 = createSheet();
    sheet2->ApplyTransform(Transform::From(DPoint3d::From(-5.0, -5.0, 0.0)));

    auto range1 = sheet1->GetEntityRange();
    auto range2 = sheet2->GetEntityRange();

    CurveVectorPtr intersectionCurve;
    BRepUtil::Modify::IntersectSheetFaces(intersectionCurve, *sheet1, *sheet2);

    ASSERT_TRUE(intersectionCurve.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Elonas.Seviakovas 07/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, IntersectSheetFaces_PerpendicularNonCrossingSheets_ReturnsNullptr)
    {   
    Dgn::IBRepEntityPtr sheet1 = createSheet();
    sheet1->ApplyTransform(Transform::From(RotMatrix::FromRotate90(DVec3d::UnitX())));
    sheet1->ApplyTransform(Transform::From(DPoint3d::From(-5.0, 0.0, 5.0)));

    Dgn::IBRepEntityPtr sheet2 = createSheet();
    sheet2->ApplyTransform(Transform::From(DPoint3d::From(-5.0, -5.0, 0.0)));

    auto range1 = sheet1->GetEntityRange();
    auto range2 = sheet2->GetEntityRange();

    CurveVectorPtr intersectionCurve;
    BRepUtil::Modify::IntersectSheetFaces(intersectionCurve, *sheet1, *sheet2);

    ASSERT_TRUE(intersectionCurve.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Elonas.Seviakovas 07/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BRepUtilTests, IntersectSheetFaces_ParallelCloseselyAtAnAngle_ReturnsNullptr)
    {   
    Dgn::IBRepEntityPtr sheet1 = createSheet();
    sheet1->ApplyTransform(Transform::From(RotMatrix::FromAxisAndRotationAngle(0, 0.8)));
    sheet1->ApplyTransform(Transform::From(DPoint3d::From(0.0, 0.0, 1.0)));

    Dgn::IBRepEntityPtr sheet2 = createSheet();
    sheet2->ApplyTransform(Transform::From(RotMatrix::FromAxisAndRotationAngle(0, 0.8)));

    auto range1 = sheet1->GetEntityRange();
    auto range2 = sheet2->GetEntityRange();

    CurveVectorPtr intersectionCurve;
    BRepUtil::Modify::IntersectSheetFaces(intersectionCurve, *sheet1, *sheet2);

    ASSERT_TRUE(intersectionCurve.IsNull());
    }

#endif

