/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/ArbitraryCenterLineProfileGeometryTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ArbitraryGeometryProfileTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct ArbitraryCenterLineProfileGeometryTestCase : ArbitraryGeometryProfileTestCase
    {
    public:
        typedef ArbitraryCenterLineProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_Line_CurvePrimitive_Success)
    {
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 1, 1, 0 });
    ASSERT_TRUE (line.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line->GetCurvePrimitiveType());

    IGeometryPtr shape = IGeometry::Create (line);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurvePrimitive, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_Line_CurveVector_Success)
    {
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 1, 1, 0 });
    ASSERT_TRUE (line.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line->GetCurvePrimitiveType());

    CurveVectorPtr asCurveVector = CurveVector::Create (line);
    ASSERT_CURVEVECTOR (asCurveVector, CurveVector::BOUNDARY_TYPE_Open, 1);

    IGeometryPtr shape = IGeometry::Create (asCurveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_OpenLineString_CurvePrimitive_Success)
    {
    ICurvePrimitivePtr lineString = ICurvePrimitive::CreateLineString ({ { 0, 0, 0 }, { 1, 1, 0 }, {1, 0, 0} });
    ASSERT_LINESTRING (lineString, 3);

    IGeometryPtr shape = IGeometry::Create (lineString);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurvePrimitive, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_OpenLineString_CurveVector_Success)
    {
    ICurvePrimitivePtr lineString = ICurvePrimitive::CreateLineString ({ { 0, 0, 0 }, { 1, 1, 0 }, {1, 0, 0} });
    ASSERT_LINESTRING (lineString, 3);

    CurveVectorPtr asCurveVector = CurveVector::Create (lineString);
    ASSERT_CURVEVECTOR (asCurveVector, CurveVector::BOUNDARY_TYPE_Open, 1);

    IGeometryPtr shape = IGeometry::Create (asCurveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_ClosedLineString_CurvePrimitive_Success)
    {
    ICurvePrimitivePtr lineString = ICurvePrimitive::CreateLineString ({ { 0, 0, 0 }, { 1, 1, 0 }, {1, 0, 0}, {0, 0, 0} });
    ASSERT_LINESTRING (lineString, 4);

    IGeometryPtr shape = IGeometry::Create (lineString);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurvePrimitive, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_OpenLineStringAsRegion_CurveVector_Success)
    {
    ICurvePrimitivePtr lineString = ICurvePrimitive::CreateLineString ({ { 0, 0, 0 }, { 1, 1, 0 }, {1, 0, 0} });
    ASSERT_LINESTRING (lineString, 3);

    CurveVectorPtr asCurveVector = CurveVector::Create (lineString, CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_CURVEVECTOR (asCurveVector, CurveVector::BOUNDARY_TYPE_Outer, 1);

    IGeometryPtr shape = IGeometry::Create (asCurveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_ClosedLineString_CurveVector_Success)
    {
    ICurvePrimitivePtr lineString = ICurvePrimitive::CreateLineString ({ { 0, 0, 0 }, { 1, 1, 0 }, {1, 0, 0}, {0, 0, 0} });
    ASSERT_LINESTRING (lineString, 4);

    CurveVectorPtr asCurveVector = CurveVector::Create (lineString);
    ASSERT_CURVEVECTOR (asCurveVector, CurveVector::BOUNDARY_TYPE_Open, 1);

    IGeometryPtr shape = IGeometry::Create (asCurveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_OpenArc_CurvePrimitive_Success)
    {
    DEllipse3d ellipse = DEllipse3d::FromArcCenterStartEnd ({ 0, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 });
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (ellipse);
    ASSERT_ARC (arc, DPoint3d::From (0, 0, 0), 1.0);

    IGeometryPtr shape = IGeometry::Create (arc);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurvePrimitive, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_OpenArc_CurveVector_Success)
    {
    DEllipse3d ellipse = DEllipse3d::FromArcCenterStartEnd ({ 0, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 });
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (ellipse);
    ASSERT_ARC (arc, DPoint3d::From (0, 0, 0), 1.0);

    CurveVectorPtr asCurveVector = CurveVector::Create (arc);
    ASSERT_CURVEVECTOR (asCurveVector, CurveVector::BOUNDARY_TYPE_Open, 1);

    IGeometryPtr shape = IGeometry::Create (asCurveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_ClosedArc_CurvePrimitive_Success)
    {
    DEllipse3d ellipse = DEllipse3d::FromCenterRadiusXY ({ 0, 0, 0 }, 1.0);
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (ellipse);
    ASSERT_ARC (arc, DPoint3d::From (0, 0, 0), 1.0);

    IGeometryPtr shape = IGeometry::Create (arc);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurvePrimitive, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_OpenArcAsRegion_CurveVector_Success)
    {
    DEllipse3d ellipse = DEllipse3d::FromArcCenterStartEnd ({ 0, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 });
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (ellipse);
    ASSERT_ARC (arc, DPoint3d::From (0, 0, 0), 1.0);

    CurveVectorPtr asCurveVector = CurveVector::Create (arc, CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_CURVEVECTOR (asCurveVector, CurveVector::BOUNDARY_TYPE_Outer, 1);

    IGeometryPtr shape = IGeometry::Create (asCurveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_ClosedArc_CurveVector_Success)
    {
    DEllipse3d ellipse = DEllipse3d::FromCenterRadiusXY ({ 0, 0, 0 }, 1.0);
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (ellipse);
    ASSERT_ARC (arc, DPoint3d::From (0, 0, 0), 1.0);

    CurveVectorPtr asCurveVector = CurveVector::Create (arc);
    ASSERT_CURVEVECTOR (asCurveVector, CurveVector::BOUNDARY_TYPE_Open, 1);

    IGeometryPtr shape = IGeometry::Create (asCurveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_MultipleConnectedPrimitivesOpen_CurveVector_Success)
    {
    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine ({ 2, 0, 0 }, { 1, 0, 0 });
    ASSERT_TRUE (line1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line1->GetCurvePrimitiveType());

    DEllipse3d ellipse = DEllipse3d::FromArcCenterStartEnd ({ 0, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 });
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (ellipse);
    ASSERT_ARC (arc, DPoint3d::From (0, 0, 0), 1.0);

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine ({ 0, 1, 0 }, { 0, 2, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    CurveVectorPtr asCurveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (asCurveVector, CurveVector::BOUNDARY_TYPE_Open, 0);
    asCurveVector->Add (line1);
    asCurveVector->Add (arc);
    asCurveVector->Add (line2);
    ASSERT_CURVEVECTOR (asCurveVector, CurveVector::BOUNDARY_TYPE_Open, 3);

    IGeometryPtr shape = IGeometry::Create (asCurveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_MultipleConnectedPrimitivesClosed_CurveVector_Success)
    {
    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 1, 0, 0 });
    ASSERT_TRUE (line1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line1->GetCurvePrimitiveType());

    DEllipse3d ellipse = DEllipse3d::FromArcCenterStartEnd ({ 0, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 });
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (ellipse);
    ASSERT_ARC (arc, DPoint3d::From (0, 0, 0), 1.0);

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine ({ 0, 1, 0 }, { 0, 0, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    CurveVectorPtr asCurveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (asCurveVector, CurveVector::BOUNDARY_TYPE_Open, 0);
    asCurveVector->Add (line1);
    asCurveVector->Add (arc);
    asCurveVector->Add (line2);
    ASSERT_CURVEVECTOR (asCurveVector, CurveVector::BOUNDARY_TYPE_Open, 3);

    IGeometryPtr shape = IGeometry::Create (asCurveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_MultipleDisconnectedPrimitivesOpen_CurveVector_Fail)
    {
    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine ({ 2, 0, 0 }, { 1.5, 0, 0 });
    ASSERT_TRUE (line1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line1->GetCurvePrimitiveType());

    DEllipse3d ellipse = DEllipse3d::FromArcCenterStartEnd ({ 0, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 });
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (ellipse);
    ASSERT_ARC (arc, DPoint3d::From (0, 0, 0), 1.0);

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine ({ 0, 1.5, 0 }, { 0, 2, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    CurveVectorPtr asCurveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (asCurveVector, CurveVector::BOUNDARY_TYPE_Open, 0);
    asCurveVector->Add (line1);
    asCurveVector->Add (arc);
    asCurveVector->Add (line2);
    ASSERT_CURVEVECTOR (asCurveVector, CurveVector::BOUNDARY_TYPE_Open, 3);

    IGeometryPtr shape = IGeometry::Create (asCurveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_2DSolidPrimitive_Fail)
    {
    DgnExtrusionDetail extrusion = DgnExtrusionDetail (CurveVector::CreateRectangle (0, 0, 1, 1, 0), DVec3d::From (0, 0, 0), true);
    ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnExtrusion (extrusion);

    IGeometryPtr shape = IGeometry::Create (solid);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::SolidPrimitive, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_3DSolidPrimitive_Fail)
    {
    DgnExtrusionDetail extrusion = DgnExtrusionDetail (CurveVector::CreateRectangle (0, 0, 1, 1, 0), DVec3d::From (0, 0, 1), true);
    ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnExtrusion (extrusion);

    IGeometryPtr shape = IGeometry::Create (solid);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::SolidPrimitive, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_3DCurveVector_Fail)
    {
    CurveVectorPtr curveVector = CurveVector::CreateLinear ({ {0, 0, 0}, {0, 1, 1}, {1, 1, 0}, {1, 2, 1} });
    ASSERT_LINESTRING_CV (curveVector, CurveVector::BOUNDARY_TYPE_Open, 0, 4);

    IGeometryPtr shape = IGeometry::Create (curveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_ElevatedCurveVector_Fail)
    {
    CurveVectorPtr curveVector = CurveVector::CreateLinear ({ {0, 0, 0.1}, {0, 1, 0.1}, {1, 1, 0.1}, {1, 2, 0.1} });
    ASSERT_LINESTRING_CV (curveVector, CurveVector::BOUNDARY_TYPE_Open, 0, 4);

    IGeometryPtr shape = IGeometry::Create (curveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_SelfIntersectingArcs_Fail)
    {
    DEllipse3d ellipse1 = DEllipse3d::FromArcCenterStartEnd ({ 0, 0, 0 }, { 0, 1, 0 }, { -1, 0, 0 });
    ICurvePrimitivePtr arc1 = ICurvePrimitive::CreateArc (ellipse1);
    ASSERT_ARC (arc1, DPoint3d::From (0, 0, 0), 1.0);

    DEllipse3d ellipse2 = DEllipse3d::FromArcCenterStartEnd ({ 0, 0, 0 }, { 1, 0, 0 }, { 0, -1, 0 });
    ICurvePrimitivePtr arc2 = ICurvePrimitive::CreateArc (ellipse2);
    ASSERT_ARC (arc2, DPoint3d::From (0, 0, 0), 1.0);

    DEllipse3d ellipse3 = DEllipse3d::FromArcCenterStartEnd ({ -1, -1, 0 }, { 0, -1, 0 }, { -1, 0, 0 });
    ICurvePrimitivePtr arc3 = ICurvePrimitive::CreateArc (ellipse3);
    ASSERT_ARC (arc3, DPoint3d::From (-1, -1, 0), 1.0);

    DEllipse3d ellipse4 = DEllipse3d::FromArcCenterStartEnd ({ -1, -1, 0 }, { -1, 0, 0 }, { -2, -1, 0 });
    ICurvePrimitivePtr arc4 = ICurvePrimitive::CreateArc (ellipse4);
    ASSERT_ARC (arc4, DPoint3d::From (-1, -1, 0), 1.0);

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 0);
    curveVector->Add (arc1);
    curveVector->Add (arc2);
    curveVector->Add (arc3);
    curveVector->Add (arc4);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 4);

    IGeometryPtr shape = IGeometry::Create (curveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_SelfIntersectingLines_Fail)
    {
    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 2, 2, 0 });
    ASSERT_TRUE (line1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line1->GetCurvePrimitiveType());

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine ({ 2, 2, 0 }, { 2, 0, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    ICurvePrimitivePtr line3 = ICurvePrimitive::CreateLine ({ 2, 0, 0 }, { 0, 2, 0 });
    ASSERT_TRUE (line3.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line3->GetCurvePrimitiveType());

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 0);
    curveVector->Add (line1);
    curveVector->Add (line2);
    curveVector->Add (line3);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 3);

    IGeometryPtr shape = IGeometry::Create (curveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_AlmostSelfIntersectingLines_Fail)
    {
    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 2, 2, 0 });
    ASSERT_TRUE (line1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line1->GetCurvePrimitiveType());

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine ({ 2, 2, 0 }, { 2, 0, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    ICurvePrimitivePtr line3 = ICurvePrimitive::CreateLine ({ 2, 0, 0 }, { 1, 1, 0 });
    ASSERT_TRUE (line3.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line3->GetCurvePrimitiveType());

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 0);
    curveVector->Add (line1);
    curveVector->Add (line2);
    curveVector->Add (line3);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 3);

    IGeometryPtr shape = IGeometry::Create (curveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_AlmostSelfIntersectingLinesWhenIntersectingPointIsNotTheShapeEnd_Fail)
    {
    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 2, 2, 0 });
    ASSERT_TRUE (line1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line1->GetCurvePrimitiveType());

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine ({ 2, 2, 0 }, { 2, 0, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    ICurvePrimitivePtr line3 = ICurvePrimitive::CreateLine ({ 2, 0, 0 }, { 1, 1, 0 });
    ASSERT_TRUE (line3.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line3->GetCurvePrimitiveType());

    ICurvePrimitivePtr line4 = ICurvePrimitive::CreateLine ({ 1, 1, 0 }, { 0, 2, 0 });
    ASSERT_TRUE (line4.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line4->GetCurvePrimitiveType());

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 0);
    curveVector->Add (line1);
    curveVector->Add (line2);
    curveVector->Add (line3);
    curveVector->Add (line4);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 4);

    IGeometryPtr shape = IGeometry::Create (curveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_SelfIntersectingChildCurveAndPrimitives_Fail)
    {
    ICurvePrimitivePtr start = ICurvePrimitive::CreateLine ({ -1, 0, 0 }, { 0, 0, 0 });
    ASSERT_TRUE (start.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, start->GetCurvePrimitiveType());

    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 2, 2, 0 });
    ASSERT_TRUE (line1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line1->GetCurvePrimitiveType());

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine ({ 2, 2, 0 }, { 2, 0, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    ICurvePrimitivePtr line3 = ICurvePrimitive::CreateLine ({ 2, 0, 0 }, { 0, 2, 0 });
    ASSERT_TRUE (line3.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line3->GetCurvePrimitiveType());

    CurveVectorPtr middle = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (middle, CurveVector::BOUNDARY_TYPE_Open, 0);
    middle->Add (line1);
    middle->Add (line2);
    middle->Add (line3);
    ASSERT_CURVEVECTOR (middle, CurveVector::BOUNDARY_TYPE_Open, 3);

    ICurvePrimitivePtr end = ICurvePrimitive::CreateLine ({ 0, 2, 0 }, { 0, 4, 0 });
    ASSERT_TRUE (end.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, end->GetCurvePrimitiveType());

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 0);
    curveVector->Add (start);
    curveVector->Add (middle);
    curveVector->Add (end);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 3);

    IGeometryPtr shape = IGeometry::Create (curveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_SelfIntersectingChildCurveAndChildren_Fail)
    {
    CurveVectorPtr start = CurveVector::CreateLinear ({ { -1, 0, 0 }, { 0, 0, 0 } });
    ASSERT_LINESTRING_CV (start, CurveVector::BOUNDARY_TYPE_Open, 0, 2);

    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 2, 2, 0 });
    ASSERT_TRUE (line1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line1->GetCurvePrimitiveType());

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine ({ 2, 2, 0 }, { 2, 0, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    ICurvePrimitivePtr line3 = ICurvePrimitive::CreateLine ({ 2, 0, 0 }, { 0, 2, 0 });
    ASSERT_TRUE (line3.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line3->GetCurvePrimitiveType());

    CurveVectorPtr middle = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (middle, CurveVector::BOUNDARY_TYPE_Open, 0);
    middle->Add (line1);
    middle->Add (line2);
    middle->Add (line3);
    ASSERT_CURVEVECTOR (middle, CurveVector::BOUNDARY_TYPE_Open, 3);

    CurveVectorPtr end = CurveVector::CreateLinear ({ { 0, 2, 0 }, { 0, 4, 0 } });
    ASSERT_LINESTRING_CV (end, CurveVector::BOUNDARY_TYPE_Open, 0, 2);

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 0);
    curveVector->Add (start);
    curveVector->Add (middle);
    curveVector->Add (end);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 3);

    IGeometryPtr shape = IGeometry::Create (curveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_ChildCurveLineIntersectingOtherPrimitive_Fail)
    {
    ICurvePrimitivePtr start = ICurvePrimitive::CreateLine ({ 1, 2, 0 }, { 0, 0, 0 });
    ASSERT_TRUE (start.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, start->GetCurvePrimitiveType());

    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 2, 0, 0 });
    ASSERT_TRUE (line1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line1->GetCurvePrimitiveType());

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine ({ 2, 0, 0 }, { 2, 1, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    ICurvePrimitivePtr line3 = ICurvePrimitive::CreateLine ({ 2, 1, 0 }, { 0, 1, 0 });
    ASSERT_TRUE (line3.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line3->GetCurvePrimitiveType());

    CurveVectorPtr middle = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (middle, CurveVector::BOUNDARY_TYPE_Open, 0);
    middle->Add (line1);
    middle->Add (line2);
    middle->Add (line3);
    ASSERT_CURVEVECTOR (middle, CurveVector::BOUNDARY_TYPE_Open, 3);

    ICurvePrimitivePtr end = ICurvePrimitive::CreateLine ({ 0, 1, 0 }, { 0, 4, 0 });
    ASSERT_TRUE (end.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, end->GetCurvePrimitiveType());

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 0);
    curveVector->Add (start);
    curveVector->Add (middle);
    curveVector->Add (end);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 3);

    IGeometryPtr shape = IGeometry::Create (curveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_ChildCurveLineIntersectingOtherChild_Fail)
    {
    CurveVectorPtr start = CurveVector::CreateLinear ({ { 1, 2, 0 }, { 0, 0, 0 } });
    ASSERT_LINESTRING_CV (start, CurveVector::BOUNDARY_TYPE_Open, 0, 2);

    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 2, 0, 0 });
    ASSERT_TRUE (line1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line1->GetCurvePrimitiveType());

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine ({ 2, 0, 0 }, { 2, 1, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    ICurvePrimitivePtr line3 = ICurvePrimitive::CreateLine ({ 2, 1, 0 }, { 0, 1, 0 });
    ASSERT_TRUE (line3.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line3->GetCurvePrimitiveType());

    CurveVectorPtr middle = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (middle, CurveVector::BOUNDARY_TYPE_Open, 0);
    middle->Add (line1);
    middle->Add (line2);
    middle->Add (line3);
    ASSERT_CURVEVECTOR (middle, CurveVector::BOUNDARY_TYPE_Open, 3);

    CurveVectorPtr end = CurveVector::CreateLinear ({ { 0, 1, 0 }, { 0, 4, 0 } });
    ASSERT_LINESTRING_CV (end, CurveVector::BOUNDARY_TYPE_Open, 0, 2);

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 0);
    curveVector->Add (start);
    curveVector->Add (middle);
    curveVector->Add (end);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 3);

    IGeometryPtr shape = IGeometry::Create (curveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_SelfIntersectingThroughKeyPoint_Fail)
    {
    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 0, 2, 0 });
    ASSERT_TRUE (line1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line1->GetCurvePrimitiveType());

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine ({ 0, 2, 0 }, { 3, 2, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    ICurvePrimitivePtr line3 = ICurvePrimitive::CreateLine ({ 3, 2, 0 }, { 3, 0, 0 });
    ASSERT_TRUE (line3.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line3->GetCurvePrimitiveType());

    ICurvePrimitivePtr line4 = ICurvePrimitive::CreateLine ({ 3, 0, 0 }, { 0, 2, 0 });
    ASSERT_TRUE (line4.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line4->GetCurvePrimitiveType());

    ICurvePrimitivePtr line5 = ICurvePrimitive::CreateLine ({ 0, 2, 0 }, { 0, 4, 0 });
    ASSERT_TRUE (line5.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line5->GetCurvePrimitiveType());

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 0);
    curveVector->Add (line1);
    curveVector->Add (line2);
    curveVector->Add (line3);
    curveVector->Add (line4);
    curveVector->Add (line5);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Open, 5);

    IGeometryPtr shape = IGeometry::Create (curveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_SelfIntersectingLineString_Fail)
    {
    ICurvePrimitivePtr lineString = ICurvePrimitive::CreateLineString ({ {0, 0, 0}, {0, 2, 0}, {3, 2, 0}, {3, 0, 0}, {1, 3, 0} });
    ASSERT_LINESTRING (lineString, 5);

    IGeometryPtr shape = IGeometry::Create (lineString);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurvePrimitive, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileGeometryTestCase, Validate_SelfIntersectingLineStringThroughKeyPoint_Fail)
    {
    ICurvePrimitivePtr lineString = ICurvePrimitive::CreateLineString ({ {0, 0, 0}, {0, 2, 0}, {3, 2, 0}, {3, 0, 0}, {0, 2, 0}, {0, 4, 0} });
    ASSERT_LINESTRING (lineString, 6);

    IGeometryPtr shape = IGeometry::Create (lineString);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurvePrimitive, shape->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", shape, 0.5);
    ArbitraryCenterLineProfilePtr profile = ArbitraryCenterLineProfile::Create (params);
    ASSERT_TRUE (profile.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profile->Validate());
    }
