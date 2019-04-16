/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ArbitraryGeometryProfileTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct ArbitraryShapeProfileGeometryTestCase : ArbitraryGeometryProfileTestCase
    {
    public:
        typedef ArbitraryShapeProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_Square_AsCurvePrimitive_Success)
    {
    // Create profile geometry and assert it is has been created as expected
    ICurvePrimitivePtr squareCurve = ICurvePrimitive::CreateRectangle (-1.0, -1.0, 1.0, 1.0, 0); // (-1, -1, 0), (-1, 1, 0), (1, 1, 0), (1, -1, 0), (-1, -1, 0)
    ASSERT_LINESTRING (squareCurve, 5);

    IGeometryPtr shape = IGeometry::Create(squareCurve);
    EXPECT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurvePrimitive, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "Square_AsCurvePrimitive", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    EXPECT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_Square_AsCurveVector_Success)
    {
    // Create profile geometry and assert it is has been created as expected
    CurveVectorPtr squareCurve = CurveVector::CreateRectangle (-1.0, -1.0, 1.0, 1.0, 0); // (-1, -1, 0), (-1, 1, 0), (1, 1, 0), (1, -1, 0), (-1, -1, 0)
    ASSERT_LINESTRING_CV (squareCurve, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    IGeometryPtr shape = IGeometry::Create (squareCurve);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "Square_AsCurveVector", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_SquareWithInnerSquareRemoved_Success)
    {
    // Create profile geometry and assert it is has been created as expected
    CurveVectorPtr squareCurve = CurveVector::CreateRectangle (-1.0, -1.0, 1.0, 1.0, 0); // (-1, -1, 0), (-1, 1, 0), (1, 1, 0), (1, -1, 0), (-1, -1, 0)
    ASSERT_LINESTRING_CV (squareCurve, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    CurveVectorPtr innerSquareCurve = CurveVector::CreateRectangle (-0.5, -0.5, 0.5, 0.5, 0.0); // (-0.5, -0.5, 0), (-0.5, 0.5, 0), (0.5, 0.5, 0), (0.5, -0.5, 0), (-0.5, -0.5, 0)
    ASSERT_LINESTRING_CV (innerSquareCurve, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    CurveVectorPtr squareWithInnerSquare = CurveVector::AreaDifference (*squareCurve, *innerSquareCurve);
    ASSERT_CURVEVECTOR (squareWithInnerSquare, CurveVector::BOUNDARY_TYPE_ParityRegion, 2);
    ASSERT_CHILD_LINESTRING (squareWithInnerSquare, 0, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);
    ASSERT_CHILD_LINESTRING (squareWithInnerSquare, 1, CurveVector::BOUNDARY_TYPE_Inner, 0, 5);

    IGeometryPtr shape = IGeometry::Create (squareWithInnerSquare);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "SquareWithInnerSquareRemoved", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_AShape_Success)
    {
    // Create profile geometry and assert it is has been created as expected
    bvector<DPoint3d> outerPoints = 
        { 
            {0, 0, 0},
            {3, 0, 0},
            {4, 3, 0},
            {5, 3, 0},
            {6, 0, 0},
            {9, 0, 0},
            {6, 7, 0},
            {3, 7, 0}
        };
    bvector<DPoint3d> innerPoints =
        {
            {3, 5, 0},
            {6, 5, 0},
            {5, 6, 0},
            {4, 6, 0}
        };

    CurveVectorPtr aOuterShape = CurveVector::CreateLinear (outerPoints, CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_LINESTRING_CV (aOuterShape, CurveVector::BOUNDARY_TYPE_Outer, 0, 9);

    CurveVectorPtr aInnerShape = CurveVector::CreateLinear (innerPoints, CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_LINESTRING_CV (aInnerShape, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    CurveVectorPtr aShape = CurveVector::AreaDifference (*aOuterShape, *aInnerShape);
    ASSERT_CURVEVECTOR (aShape, CurveVector::BOUNDARY_TYPE_ParityRegion, 2);
    ASSERT_CHILD_LINESTRING (aShape, 0, CurveVector::BOUNDARY_TYPE_Outer, 0, 9);
    ASSERT_CHILD_LINESTRING (aShape, 1, CurveVector::BOUNDARY_TYPE_Inner, 0, 5);

    IGeometryPtr shape = IGeometry::Create (aShape);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "AShape", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_DifferentCurvePrimitivesOnSinglePerimeter_Success)
    {
    // Create profile geometry and assert it is has been created as expected
    CurveVectorPtr squareShape = CurveVector::CreateRectangle (0, 0, 5, 6, 0); // (0, 0, 0), (5, 0, 0), (5, 6, 0), (0, 6, 0)
    ASSERT_LINESTRING_CV (squareShape, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    DEllipse3d arcEllipse = DEllipse3d::FromCenterRadiusXY ({ 5, 3, 0 }, 3.0);
    CurveVectorPtr arcShape = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc(arcEllipse));
    ASSERT_ARC_CV (arcShape, CurveVector::BOUNDARY_TYPE_Outer, 0, DPoint3d::From (5, 3, 0), 3.0);

    CurveVectorPtr arcAndSquare = CurveVector::AreaUnion (*squareShape, *arcShape);
    ASSERT_CURVEVECTOR (arcAndSquare, CurveVector::BOUNDARY_TYPE_Outer, 2);
    ASSERT_CONTAINS_LINESTRING (arcAndSquare, 4, 1);
    ASSERT_CONTAINS_ARC (arcAndSquare, DPoint3d::From (5, 3, 0), 3, 1);

    IGeometryPtr shape = IGeometry::Create (arcAndSquare);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "DifferentCurvePrimitivesOnSinglePerimeter", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_MultiplePerimeters_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    CurveVectorPtr squareShape1 = CurveVector::CreateRectangle (0, 0, 5, 6, 0); // (0, 0, 0), (5, 0, 0), (5, 6, 0), (0, 6, 0)
    ASSERT_LINESTRING_CV (squareShape1, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    CurveVectorPtr squareShape2 = CurveVector::CreateRectangle (-1, -1, -5, -6, 0); // (-1, -1, 0), (-5, -1, 0), (-5, -6, 0), (-1, -6, 0)
    ASSERT_LINESTRING_CV (squareShape2, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    EXPECT_TRUE (CurveVector::AreaIntersection (*squareShape1, *squareShape2).IsNull());

    CurveVectorPtr untouchingSquares = CurveVector::AreaUnion (*squareShape1, *squareShape2);
    ASSERT_CURVEVECTOR (untouchingSquares, CurveVector::BOUNDARY_TYPE_UnionRegion, 2);
    ASSERT_CHILD_LINESTRING (untouchingSquares, 0, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);
    ASSERT_CHILD_LINESTRING (untouchingSquares, 1, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    IGeometryPtr shape = IGeometry::Create (untouchingSquares);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "MultiplePerimeters", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_InnerHoleAndMultiplePerimeters_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    CurveVectorPtr squareShape1 = CurveVector::CreateRectangle (0, 0, 5, 6, 0); // (0, 0, 0), (5, 0, 0), (5, 6, 0), (0, 6, 0)
    ASSERT_LINESTRING_CV (squareShape1, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    CurveVectorPtr innerSquareShape = CurveVector::CreateRectangle (2, 2, 4, 4, 0); // (2, 2, 0), (2, 4, 0), (4, 4, 0), (4, 2, 0)
    ASSERT_LINESTRING_CV (innerSquareShape, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    CurveVectorPtr squareWithInnerHole = CurveVector::AreaDifference (*squareShape1, *innerSquareShape);
    ASSERT_CURVEVECTOR (squareWithInnerHole, CurveVector::BOUNDARY_TYPE_ParityRegion, 2);
    ASSERT_CHILD_LINESTRING (squareWithInnerHole, 0, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);
    ASSERT_CHILD_LINESTRING (squareWithInnerHole, 1, CurveVector::BOUNDARY_TYPE_Inner, 0, 5);

    CurveVectorPtr squareShape2 = CurveVector::CreateRectangle (-1, -1, -5, -6, 0); // (-1, -1, 0), (-5, -1, 0), (-5, -6, 0), (-1, -6, 0)
    ASSERT_LINESTRING_CV (squareShape2, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    CurveVectorPtr untouchingSquares = CurveVector::AreaUnion (*squareWithInnerHole, *squareShape2);
    ASSERT_CURVEVECTOR (untouchingSquares, CurveVector::BOUNDARY_TYPE_UnionRegion, 2);

    EXPECT_EQ (1, std::count_if (untouchingSquares->begin(), untouchingSquares->end(), 
        [] (ICurvePrimitivePtr curve) 
        {
        return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == curve->GetCurvePrimitiveType() &&
            CurveVector::BOUNDARY_TYPE_ParityRegion == curve->GetChildCurveVectorCP()->GetBoundaryType() &&
            2 == curve->GetChildCurveVectorCP()->size() &&

            ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == (*curve->GetChildCurveVectorCP())[0]->GetCurvePrimitiveType() &&
            CurveVector::BOUNDARY_TYPE_Outer == (*curve->GetChildCurveVectorCP())[0]->GetChildCurveVectorCP()->GetBoundaryType() &&
            1 == (*curve->GetChildCurveVectorCP())[0]->GetChildCurveVectorCP()->size() &&
            ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == (*(*curve->GetChildCurveVectorCP())[0]->GetChildCurveVectorCP())[0]->GetCurvePrimitiveType() &&
            5 == (*(*curve->GetChildCurveVectorCP())[0]->GetChildCurveVectorCP())[0]->GetLineStringCP()->size() &&

            ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == (*curve->GetChildCurveVectorCP())[1]->GetCurvePrimitiveType() &&
            CurveVector::BOUNDARY_TYPE_Inner == (*curve->GetChildCurveVectorCP())[1]->GetChildCurveVectorCP()->GetBoundaryType() &&
            1 == (*curve->GetChildCurveVectorCP())[1]->GetChildCurveVectorCP()->size() &&
            ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == (*(*curve->GetChildCurveVectorCP())[1]->GetChildCurveVectorCP())[0]->GetCurvePrimitiveType() &&
            5 == (*(*curve->GetChildCurveVectorCP())[1]->GetChildCurveVectorCP())[0]->GetLineStringCP()->size();
        }));

    EXPECT_EQ(1, std::count_if (untouchingSquares->begin(), untouchingSquares->end(),
        [] (ICurvePrimitivePtr curve)
        {
        return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == curve->GetCurvePrimitiveType() &&
            CurveVector::BOUNDARY_TYPE_Outer == curve->GetChildCurveVectorCP()->GetBoundaryType() &&
            1 == curve->GetChildCurveVectorCP()->size() &&
            ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == (*curve->GetChildCurveVectorCP())[0]->GetCurvePrimitiveType() &&
            5 == (*curve->GetChildCurveVectorCP())[0]->GetLineStringCP()->size();
        }));

    IGeometryPtr shape = IGeometry::Create (untouchingSquares);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "InnerHoleAndMultiplePerimeters", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create(params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_TwoRectanglesOverlapping_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    CurveVectorPtr squareShape1 = CurveVector::CreateRectangle (-1, -1, 5, 6, 0); // (0, 0, 0), (5, 0, 0), (5, 6, 0), (0, 6, 0)
    ASSERT_LINESTRING_CV (squareShape1, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    CurveVectorPtr squareShape2 = CurveVector::CreateRectangle (-5, -6, 1, 1, 0); // (0, 0, 0), (-5, 0, 0), (-5, -6, 0), (0, -6, 0)
    ASSERT_LINESTRING_CV (squareShape2, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    CurveVectorPtr touchingSquares = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);
    touchingSquares->Add (squareShape1);
    touchingSquares->Add (squareShape2);
    ASSERT_CURVEVECTOR (touchingSquares, CurveVector::BOUNDARY_TYPE_Outer, 2);

    IGeometryPtr shape = IGeometry::Create (touchingSquares);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "TwoRectanglesOverlapping", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_UnionOfTwoRectanglesAndCircle_Success)
    {
    // Create profile geometry and assert it is has been created as expected
    CurveVectorPtr squareShape1 = CurveVector::CreateRectangle (0, 0, 4, 4, 0); // (0, 0, 0), (4, 0, 0), (4, 4, 0), (0, 4, 0)
    ASSERT_LINESTRING_CV (squareShape1, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    CurveVectorPtr squareShape2 = CurveVector::CreateRectangle (5, 5, 9, 9, 0); // (5, 5, 0), (5, 9, 0), (9, 9, 0), (9, 5, 0)
    ASSERT_LINESTRING_CV (squareShape2, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    DEllipse3d arcEllipse = DEllipse3d::FromCenterRadiusXY ({ 4.5, 4.5, 0 }, 2.5);
    CurveVectorPtr arcShape = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc (arcEllipse));
    ASSERT_ARC_CV (arcShape, CurveVector::BOUNDARY_TYPE_Outer, 0, DPoint3d::From (4.5, 4.5, 0), 2.5);

    CurveVectorPtr nonTouchingSquares = CurveVector::AreaUnion (*squareShape1, *squareShape2);
    ASSERT_CURVEVECTOR (nonTouchingSquares, CurveVector::BOUNDARY_TYPE_UnionRegion, 2);
    ASSERT_CHILD_LINESTRING (nonTouchingSquares, 0, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);
    ASSERT_CHILD_LINESTRING (nonTouchingSquares, 1, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    CurveVectorPtr singlePerimeterShape = CurveVector::AreaUnion (*nonTouchingSquares, *arcShape);
    ASSERT_CURVEVECTOR (singlePerimeterShape, CurveVector::BOUNDARY_TYPE_Outer, 3);
    ASSERT_CONTAINS_LINESTRING (singlePerimeterShape, 5, 2);
    ASSERT_CONTAINS_ARC (singlePerimeterShape, DPoint3d::From (4.5, 4.5, 0), 2.5, 1);

    IGeometryPtr shape = IGeometry::Create (singlePerimeterShape);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "UnionOfTwoRectanglesAndCircle", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_3dGeometry_3dCurveVector_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    bvector<DPoint3d> points = 
        {
            {0, 0, 0},
            {1, 0, 1},
            {1, 1, 0},
            {0, 1, 1}
        };
    CurveVectorPtr shape3d = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_LINESTRING_CV (shape3d, CurveVector::BOUNDARY_TYPE_Outer, 0, 5);

    IGeometryPtr shape = IGeometry::Create (shape3d);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "3dGeometry_3dCurveVector", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_3dGeometry_Box_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    DgnBoxDetail boxShape = DgnBoxDetail 
    (
        DPoint3d::From( 0, 0, 0 ), 
        DPoint3d::From (1, 1, 1 ), 
        DVec3d::From( 1, 0, 0 ), 
        DVec3d::From (0, 1, 0 ), 
        1.0, 
        1.0, 
        1.0, 
        1.0, 
        true
    );
    ISolidPrimitivePtr shape3d = ISolidPrimitive::CreateDgnBox (boxShape);
    EXPECT_TRUE (shape3d.IsValid());

    IGeometryPtr shape = IGeometry::Create (shape3d);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::SolidPrimitive, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "3dGeometry_Box", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_3dGeometry_Cone_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    DgnConeDetail coneShape = DgnConeDetail
    (
        DPoint3d::From (0, 0, 0),
        DPoint3d::From (0, 0, 1),
        5.0,
        1.0,
        true
    );
    ISolidPrimitivePtr shape3d = ISolidPrimitive::CreateDgnCone (coneShape);
    ASSERT_TRUE (shape3d.IsValid());

    IGeometryPtr shape = IGeometry::Create (shape3d);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::SolidPrimitive, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "3dGeometry_Cone", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_3dGeometry_Extrusion_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    DgnExtrusionDetail extrusionShape = DgnExtrusionDetail
    (
        CurveVector::CreateRectangle (0, 0, 1, 1, 0),
        DVec3d::From (0, 0, 1),
        true
    );
    ISolidPrimitivePtr shape3d = ISolidPrimitive::CreateDgnExtrusion (extrusionShape);
    ASSERT_TRUE (shape3d.IsValid());

    IGeometryPtr shape = IGeometry::Create (shape3d);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::SolidPrimitive, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "3dGeometry_Extrusion", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_3dGeometry_Extrusion2D_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    DgnExtrusionDetail extrusionShape = DgnExtrusionDetail
    (
        CurveVector::CreateRectangle (0, 0, 1, 1, 0),
        DVec3d::From (0, 0, 0),
        true
    );
    ISolidPrimitivePtr shape3d = ISolidPrimitive::CreateDgnExtrusion (extrusionShape);
    ASSERT_TRUE (shape3d.IsValid());

    IGeometryPtr shape = IGeometry::Create (shape3d);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::SolidPrimitive, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "3dGeometry_Extrusion2D", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_3dGeometry_RotationalSweep_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    DgnRotationalSweepDetail rotationalSweepShape = DgnRotationalSweepDetail
    (
        CurveVector::CreateLinear (bvector<DPoint3d>
        {
            { 0, 0, 0 },
            { 1, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 0 }
        }),
        DPoint3d::From (0, 0, 0),
        DVec3d::From (1, 0, 0),
        2 * PI,
        true
    );
    ISolidPrimitivePtr shape3d = ISolidPrimitive::CreateDgnRotationalSweep (rotationalSweepShape);
    ASSERT_TRUE (shape3d.IsValid());

    IGeometryPtr shape = IGeometry::Create (shape3d);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::SolidPrimitive, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "3dGeometry_RotationalSweep", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_3dGeometry_RuledSweep_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    DgnRuledSweepDetail ruledSweepShape = DgnRuledSweepDetail
    (
        CurveVector::CreateLinear (bvector<DPoint3d>
        {
            { 0, 0, 0 },
            { 1, 0, 0 },
            { 1, 1, 0 },
            { 0, 1, 0 },
            { 0, 0, 0 }
        }),
        CurveVector::CreateLinear (bvector<DPoint3d>
        {
            { 0, 0, 1 },
            { 1, 0, 1 },
            { 1, 1, 1 },
            { 0, 1, 1 },
            { 0, 0, 1 }
        }),
        true
    );
    ISolidPrimitivePtr shape3d = ISolidPrimitive::CreateDgnRuledSweep (ruledSweepShape);
    ASSERT_TRUE (shape3d.IsValid());

    IGeometryPtr shape = IGeometry::Create (shape3d);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::SolidPrimitive, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "3dGeometry_RuledSweep", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_3dGeometry_Sphere_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    DgnSphereDetail sphereShape = DgnSphereDetail(DPoint3d::From (0, 0, 0), 5.0);
    ISolidPrimitivePtr shape3d = ISolidPrimitive::CreateDgnSphere (sphereShape);
    ASSERT_TRUE (shape3d.IsValid());

    IGeometryPtr shape = IGeometry::Create (shape3d);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::SolidPrimitive, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "3dGeometry_Sphere", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_3dGeometry_TorusPipe_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    DgnTorusPipeDetail torusPipeShape = DgnTorusPipeDetail 
    (
        DEllipse3d::FromCenterRadiusXY (DPoint3d::From (0, 0, 0), 5.0), 
        2.0, 
        true
    );
    ISolidPrimitivePtr shape3d = ISolidPrimitive::CreateDgnTorusPipe (torusPipeShape);
    EXPECT_TRUE (shape3d.IsValid());

    IGeometryPtr shape = IGeometry::Create (shape3d);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::SolidPrimitive, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "3dGeometry_TorusPipe", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_3dPlane_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    bvector<DPoint3d> points =
        {
            {0, 0, 0},
            {1, 0, 1},
            {1, 1, 0},
            {0, 0, 0}
        };
    CurveVectorPtr shape3d = CurveVector::CreateLinear (points);
    ASSERT_TRUE (shape3d.IsValid());

    IGeometryPtr shape = IGeometry::Create (shape3d);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "3dPlane", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_Elevated_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    bvector<DPoint3d> points =
        {
            {0, 0, 0.01},
            {1, 0, 0.01},
            {1, 1, 0.01},
            {0, 0, 0.01}
        };
    CurveVectorPtr elevated = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_TRUE (elevated.IsValid());

    IGeometryPtr shape = IGeometry::Create (elevated);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "Elevated", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_NotClosedShape_CurvePrimitive_Line_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine (DPoint3d::From (0, 0, 0), DPoint3d::From (1, 1, 0));
    ASSERT_TRUE (line.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line->GetCurvePrimitiveType());

    IGeometryPtr shape = IGeometry::Create (line);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurvePrimitive, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "NotClosedShape_CurvePrimitive_Line", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_NotClosedShape_CurvePrimitive_LineString_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLineString (bvector<DPoint3d>{ DPoint3d::From (0, 0, 0), DPoint3d::From (1, 1, 0) });
    ASSERT_TRUE (line.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString, line->GetCurvePrimitiveType());

    IGeometryPtr shape = IGeometry::Create (line);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurvePrimitive, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "NotClosedShape_CurvePrimitive_LineString", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_NotClosedShape_CurveVector_LineString_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    CurveVectorPtr line = CurveVector::CreateLinear (bvector<DPoint3d>{ DPoint3d::From (0, 0, 0), DPoint3d::From (1, 1, 0) });
    ASSERT_TRUE (line.IsValid());
    EXPECT_EQ (CurveVector::BOUNDARY_TYPE_Open, line->GetBoundaryType());

    IGeometryPtr shape = IGeometry::Create (line);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "NotClosedShape_CurveVector_LineString", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_NoArea_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    CurveVectorPtr line = CurveVector::CreateLinear (bvector<DPoint3d>{ DPoint3d::From (0, 0, 0), DPoint3d::From (1, 1, 0) }, CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_TRUE (line.IsValid());
    EXPECT_EQ (CurveVector::BOUNDARY_TYPE_Outer, line->GetBoundaryType());
    
    DPoint3d centroid;
    double area;
    line->CentroidAreaXY (centroid, area);
    ASSERT_DOUBLE_EQ (0, area);

    IGeometryPtr shape = IGeometry::Create (line);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "NoArea", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_NoAreaArc_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    DEllipse3d arcEllipse = DEllipse3d::FromCenterRadiusXY ({ 0, 0, 0 }, 0);
    ICurvePrimitivePtr arcShape = ICurvePrimitive::CreateArc (arcEllipse);
    ASSERT_ARC (arcShape, DPoint3d::From (0, 0, 0), 0);

    IGeometryPtr shape = IGeometry::Create (arcShape);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurvePrimitive, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "NoAreaArc", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_UnclosedGeometry_CurvePrimitive_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    DEllipse3d arcEllipse = DEllipse3d::FromPoints ({ 0, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 }, 0, PI);
    ICurvePrimitivePtr arcShape = ICurvePrimitive::CreateArc (arcEllipse);
    ASSERT_ARC (arcShape, DPoint3d::From (0, 0, 0), 1.0);

    IGeometryPtr shape = IGeometry::Create (arcShape);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurvePrimitive, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "UnclosedGeometry_CurvePrimitive", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_BoundedUnclosedGeometry_CurveVector_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    DEllipse3d arcEllipse = DEllipse3d::FromPoints ({ 0, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 }, 0, PI);
    ICurvePrimitivePtr arcShape = ICurvePrimitive::CreateArc (arcEllipse);
    ASSERT_ARC (arcShape, DPoint3d::From (0, 0, 0), 1.0);

    CurveVectorPtr closed = CurveVector::Create (arcShape, CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_ARC_CV (closed, CurveVector::BOUNDARY_TYPE_Outer, 0, DPoint3d::From (0, 0, 0), 1.0);

    IGeometryPtr shape = IGeometry::Create (closed);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "BoundedUnclosedGeometry_CurveVector", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_ClosedArc_Success)
    {
    // Create profile geometry and assert it is has been created as expected
    DEllipse3d arcEllipse = DEllipse3d::FromPoints ({ 0, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 }, 0, PI);
    ICurvePrimitivePtr arcShape = ICurvePrimitive::CreateArc (arcEllipse);
    ASSERT_ARC (arcShape, DPoint3d::From (0, 0, 0), 1.0);

    ICurvePrimitivePtr closingLine = ICurvePrimitive::CreateLine ({ -1, 0, 0 }, { 1, 0, 0 });
    ASSERT_TRUE (closingLine.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, closingLine->GetCurvePrimitiveType());

    CurveVectorPtr closed = CurveVector::Create (arcShape, CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_TRUE (closed.IsValid());
    closed->Add (closingLine);
    ASSERT_EQ (2, closed->size());

    IGeometryPtr shape = IGeometry::Create (closed);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "ClosedArc", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_ClosedFrom3Lines_Success)
    {
    // Create profile geometry and assert it is has been created as expected
    
    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 1, 0, 0 });
    ASSERT_TRUE (line1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line1->GetCurvePrimitiveType());

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine ({ 1, 0, 0 }, { 1, 1, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    ICurvePrimitivePtr line3 = ICurvePrimitive::CreateLine ({ 1, 1, 0 }, { 0, 0, 0 });
    ASSERT_TRUE (line3.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line3->GetCurvePrimitiveType());

    CurveVectorPtr closed = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_TRUE (closed.IsValid());
    closed->Add (line1);
    closed->Add (line2);
    closed->Add (line3);
    ASSERT_EQ (3, closed->size());

    IGeometryPtr shape = IGeometry::Create (closed);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "ClosedFrom3Lines", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_ClosedFrom4LinesWithOneOutOfPerimeter_Failed)
    {
    // Create profile geometry and assert it is has been created as expected

    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 1, 0, 0 });
    ASSERT_TRUE (line1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line1->GetCurvePrimitiveType());

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine ({ 1, 0, 0 }, { 1, 1, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    ICurvePrimitivePtr line3 = ICurvePrimitive::CreateLine ({ 1, 1, 0 }, { 2, 2, 0 });
    ASSERT_TRUE (line2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line2->GetCurvePrimitiveType());

    ICurvePrimitivePtr line4 = ICurvePrimitive::CreateLine ({ 1, 1, 0 }, { 0, 0, 0 });
    ASSERT_TRUE (line4.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, line4->GetCurvePrimitiveType());

    CurveVectorPtr closed = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_TRUE (closed.IsValid());
    closed->Add (line1);
    closed->Add (line2);
    closed->Add (line3);
    closed->Add (line4);
    ASSERT_EQ (4, closed->size());

    IGeometryPtr shape = IGeometry::Create (closed);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "ClosedFrom4LinesWithOneOutOfPerimeter", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_TwoTouchingRectanglesOneOfWhichIsNotClosed_Failed)
    {
    // Create profile geometry and assert it is has been created as expected
    ICurvePrimitivePtr square1 = ICurvePrimitive::CreateRectangle (0, 0, 3, 3, 0);
    ASSERT_LINESTRING (square1, 5);

    ICurvePrimitivePtr square2 = ICurvePrimitive::CreateLineString (bvector<DPoint3d>{ {3, 1, 0}, { 4, 1, 0 }, { 4, 2, 0 }, { 3, 2, 0 }});
    ASSERT_LINESTRING (square2, 4);

    CurveVectorPtr squares = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_TRUE (squares.IsValid());
    squares->Add (square1);
    squares->Add (square2);
    EXPECT_EQ (2, squares->size());

    IGeometryPtr shape = IGeometry::Create (squares);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "TwoTouchingRectanglesOneOfWhichIsNotClosed", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::ValidationFailed, profilePtr->Validate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileGeometryTestCase, Validate_BShapeWithoutInnerHoles_Success)
    {
    // Create profile geometry and assert it is has been created as expected
    DEllipse3d topArc = DEllipse3d::FromPoints ({ 0, 3, 0 }, { 1, 3, 0 }, { 0, 4, 0 }, Angle::PiOver2(), -Angle::Pi());
    ICurvePrimitivePtr topArcPrimitive = ICurvePrimitive::CreateArc (topArc);
    ASSERT_ARC (topArcPrimitive, DPoint3d::From (0, 3, 0), 1.0);

    DEllipse3d botArc = DEllipse3d::FromPoints ({ 0, 1, 0 }, { 1, 1, 0 }, { 0, 2, 0 }, Angle::PiOver2(), -Angle::Pi());
    ICurvePrimitivePtr botArcPrimitive = ICurvePrimitive::CreateArc (botArc);
    ASSERT_ARC (botArcPrimitive, DPoint3d::From (0, 1, 0), 1.0);

    ICurvePrimitivePtr closingLine1 = ICurvePrimitive::CreateLine ({ 0, 0, 0 }, { 0, 2, 0 });
    ASSERT_TRUE (closingLine1.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, closingLine1->GetCurvePrimitiveType());

    ICurvePrimitivePtr closingLine2 = ICurvePrimitive::CreateLine ({ 0, 2, 0 }, { 0, 4, 0 });
    ASSERT_TRUE (closingLine2.IsValid());
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line, closingLine2->GetCurvePrimitiveType());

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Outer, 0);
    curveVector->Add (topArcPrimitive);
    curveVector->Add (botArcPrimitive);
    curveVector->Add (closingLine1);
    curveVector->Add (closingLine2);
    ASSERT_CURVEVECTOR (curveVector, CurveVector::BOUNDARY_TYPE_Outer, 4);

    IGeometryPtr shape = IGeometry::Create (curveVector);
    ASSERT_TRUE (shape.IsValid());
    EXPECT_EQ (IGeometry::GeometryType::CurveVector, shape->GetGeometryType());

    // Create profile and assert its valididty is as expected
    CreateParams params (GetModel(), "BShapeWithoutInnerHoles", shape);
    ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);
    ASSERT_TRUE (profilePtr.IsValid());
    EXPECT_EQ (DgnDbStatus::Success, profilePtr->Validate());
    }