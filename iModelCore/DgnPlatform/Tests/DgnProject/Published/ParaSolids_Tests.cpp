/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ParaSolids_Tests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"

#ifdef BENTLEYCONFIG_PARASOLID

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct ParaSolidsTests : public DgnDbTestFixture
{
  
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ParaSolidsTests, CreateBodyFrom_SolidPrimDgncone)
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
TEST_F(ParaSolidsTests, CreateBodyFrom_SolidPrimDgnSphere)
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
TEST_F(ParaSolidsTests, CreateBodyFrom_SolidPrimDgnbox)
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
TEST_F(ParaSolidsTests, CreateBodyFrom_SolidPrimDgnTorus)
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
TEST_F(ParaSolidsTests, CreateBodyFrom_SolidPrimDgnExtrusion)
    {
    CurveVectorPtr ellipse = CurveVector::CreateDisk(DEllipse3d::From(0, 0, 0, 1.5, 0, 0, 0, 1.5, 0, 0.0, Angle::TwoPi()));
    DgnExtrusionDetail DgnExtrusion(ellipse, DVec3d::From(0, 0, 3), true);
    ISolidPrimitivePtr myDgnExtrusion = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusion);
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
TEST_F(ParaSolidsTests, CreateBodyFrom_SolidPrimDgnRotationalsweep)
    {
    
    CurveVectorPtr ellipse = CurveVector::CreateRectangle(1,2,3,4,0.5);
    DPoint3d center = DPoint3d::From(0, 0, 0);
    DVec3d   axis = DVec3d::From(0, 1, 0);
    DgnRotationalSweepDetail rotationalSweepData(ellipse, center, axis, Angle::Pi(), false);
    ISolidPrimitivePtr mySweepData = ISolidPrimitive::CreateDgnRotationalSweep(rotationalSweepData);
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
TEST_F(ParaSolidsTests, CreateBodyFrom_SolidPrimDgnRuledSweep)
    {
    double ax = 1.0;
    double ay = 2.0;
    double bx = 3.0;
    double by = 4.0;
    double z0 = 0.5;
    double dz = 4.0;
    double z1 = z0 + dz;
    CurveVectorPtr sec1 = CurveVector::CreateRectangle(ax, ay, bx, by, z0, CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr sec2 = CurveVector::CreateRectangle(ax, ay, bx, by, z1, CurveVector::BOUNDARY_TYPE_Outer);
    DgnRuledSweepDetail DgnRuledSweepDetail (sec1, sec2, true);
    ISolidPrimitivePtr RuledSweep = ISolidPrimitive::CreateDgnRuledSweep(DgnRuledSweepDetail);
    IBRepEntityPtr brep;
    ASSERT_EQ(SUCCESS, BRepUtil::Create::BodyFromSolidPrimitive(brep, *RuledSweep));
    ASSERT_TRUE(!brep.IsNull());
    ASSERT_EQ(6, BRepUtil::GetBodyFaces(NULL, *brep));
    ASSERT_EQ(12, BRepUtil::GetBodyEdges(NULL, *brep));
    ASSERT_EQ(8, BRepUtil::GetBodyVertices(NULL, *brep));
    ASSERT_FALSE(BRepUtil::IsDisjointBody(*brep));
    }
#endif

