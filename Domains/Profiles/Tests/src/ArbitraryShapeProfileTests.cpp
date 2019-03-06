/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/ArbitraryShapeProfileTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

#define ASSERT_CURVEVECTOR(curveVector, boundaryType, minIndex)                                 AssertCurveVector (curveVector, boundaryType, minIndex, __FILE__, __LINE__)
#define ASSERT_LINESTRING_CV(curveVector, boundaryType, minIndex, pointCount)                   AssertLineStringShape (curveVector, boundaryType, minIndex, pointCount, __FILE__, __LINE__)
#define ASSERT_LINESTRING(curvePrimitive, pointCount)                                           AssertLineStringShape (curvePrimitive, pointCount, __FILE__, __LINE__)
#define ASSERT_ARC_CV(curveVector, boundaryType, minIndex, center, radius)                      AssertArcShape (curveVector, boundaryType, minIndex, center, radius, __FILE__, __LINE__)
#define ASSERT_ARC(curvePrimitive, center, radius)                                              AssertArcShape (curvePrimitive, center, radius, __FILE__, __LINE__)
#define ASSERT_CHILD_LINESTRING(curveVector, curveIndex, boundaryType, minIndex, pointCount)    AssertChildLineStringShape (curveVector, curveIndex, boundaryType, minIndex, pointCount, __FILE__, __LINE__)
#define ASSERT_CONTAINS_LINESTRING(curveVector, pointCount, expectedCount)                      AssertContainsLineStringShape (curveVector, pointCount, expectedCount, __FILE__, __LINE__)
#define ASSERT_CONTAINS_ARC(curveVector, center, radius, expectedCount)                         AssertContainsArcShape (curveVector, center, radius, expectedCount, __FILE__, __LINE__)

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct ArbitraryShapeProfileTestCase : ProfileValidationTestCase<ArbitraryShapeProfile>
    {
    public:
        typedef ArbitraryShapeProfile::CreateParams CreateParams;

    private:
        Utf8CP GetFailureMessage (Utf8CP message, CharCP file, size_t lineNo);

    public:
        void AssertCurveVector (CurveVectorCPtr const& shape, CurveVector::BoundaryType expectedBoundaryType, size_t expectedMinimumIndex, CharCP file, size_t lineNo);

        void AssertLineStringShape (CurveVectorCPtr const& shape, CurveVector::BoundaryType expectedBoundaryType, size_t expectedIndex, size_t expectedPointCount, CharCP file, size_t lineNo);
        void AssertArcShape (CurveVectorCPtr const& shape, CurveVector::BoundaryType expectedBoundaryType, size_t expectedIndex, DPoint3d expectedCenter, double expectedRadius, CharCP file, size_t lineNo);
        
        void AssertChildLineStringShape (CurveVectorCPtr const& shape, size_t expectedCurveIndex, CurveVector::BoundaryType expectedBoundaryType, size_t expectedChildIndex, size_t expectedPointCount, CharCP file, size_t lineNo);

        void AssertContainsLineStringShape (CurveVectorCPtr const& shape, size_t expectedPointCount, int minCount, CharCP file, size_t lineNo);
        void AssertContainsArcShape (CurveVectorCPtr const& shape, DPoint3d expectedCenter, double expectedRadius, int minCount, CharCP file, size_t lineNo);

        void AssertLineStringShape (ICurvePrimitiveCPtr const& shape, size_t expectedPointCount, CharCP file, size_t);
        void AssertArcShape (ICurvePrimitiveCPtr const& shape, DPoint3d expectedCenter, double expectedRadius, CharCP, size_t lineNo);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ArbitraryShapeProfileTestCase::GetFailureMessage (Utf8CP message, CharCP file, size_t lineNo)
    {
    if (nullptr == file)
        return "";

    return Utf8PrintfString ("%s(%d): %s", file, lineNo, message).c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryShapeProfileTestCase::AssertCurveVector (CurveVectorCPtr const& shape, CurveVector::BoundaryType expectedBoundaryType, size_t expectedMinimumSize, CharCP file, size_t lineNo)
    {
    ASSERT_TRUE (shape.IsValid()) << GetFailureMessage("Curve vector is null", file, lineNo);
    EXPECT_EQ (expectedBoundaryType, shape->GetBoundaryType()) << GetFailureMessage ("Curve vector boundary type is not as expected", file, lineNo);
    ASSERT_TRUE (shape->size() >= expectedMinimumSize) << GetFailureMessage ("Curve vector has too little primitives", file, lineNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryShapeProfileTestCase::AssertArcShape (CurveVectorCPtr const& shape, CurveVector::BoundaryType expectedBoundaryType, size_t expectedIndex, DPoint3d expectedCenter, double expectedRadius, CharCP file, size_t lineNo)
    {
    AssertCurveVector (shape, expectedBoundaryType, expectedIndex + 1, file, lineNo);
    AssertArcShape ((*shape)[expectedIndex], expectedCenter, expectedRadius, file, lineNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryShapeProfileTestCase::AssertArcShape (ICurvePrimitiveCPtr const& shape, DPoint3d expectedCenter, double expectedRadius, CharCP file, size_t lineNo)
    {
    ASSERT_TRUE (shape.IsValid()) << GetFailureMessage ("Curve primitive is null", file, lineNo);
    ASSERT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc, shape->GetCurvePrimitiveType()) << GetFailureMessage ("Curve primitive is not an arc", file, lineNo);
    EXPECT_TRUE (expectedCenter.AlmostEqual (shape->GetArcCP()->center)) << GetFailureMessage ("Arc's center is not as expected", file, lineNo);
    EXPECT_DOUBLE_EQ (expectedRadius, shape->GetArcCP()->vector0.Magnitude()) << GetFailureMessage ("Arc's radius is not as expected", file, lineNo);
    EXPECT_DOUBLE_EQ (expectedRadius, shape->GetArcCP()->vector90.Magnitude()) << GetFailureMessage ("Arc's radius is not as expected", file, lineNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryShapeProfileTestCase::AssertLineStringShape (CurveVectorCPtr const& shape, CurveVector::BoundaryType expectedBoundaryType, size_t expectedIndex, size_t expectedPointCount, CharCP file, size_t lineNo)
    {
    AssertCurveVector (shape, expectedBoundaryType, expectedIndex + 1, file, lineNo);
    AssertLineStringShape ((*shape)[expectedIndex], expectedPointCount, file, lineNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryShapeProfileTestCase::AssertLineStringShape (ICurvePrimitiveCPtr const& shape, size_t expectedPointCount, CharCP file, size_t lineNo)
    {
    ASSERT_TRUE (shape.IsValid()) << GetFailureMessage ("Curve primitive is null", file, lineNo);
    ASSERT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString, shape->GetCurvePrimitiveType()) << GetFailureMessage ("Curve primitive is not a line string", file, lineNo);
    EXPECT_EQ (expectedPointCount, shape->GetLineStringCP()->size()) << GetFailureMessage ("Line string has incorrect point count", file, lineNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryShapeProfileTestCase::AssertChildLineStringShape (CurveVectorCPtr const& shape, size_t expectedCurveIndex, CurveVector::BoundaryType expectedBoundaryType, size_t expectedChildIndex, size_t expectedPointCount, CharCP file, size_t lineNo)
    {
    ASSERT_TRUE (shape.IsValid()) << GetFailureMessage ("Curve vector is null", file, lineNo);
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector, (*shape)[expectedCurveIndex]->GetCurvePrimitiveType()) 
        << GetFailureMessage ("Child is not a curve vector", file, lineNo);
    AssertLineStringShape ((*shape)[expectedCurveIndex]->GetChildCurveVectorCP(), expectedBoundaryType, expectedChildIndex, expectedPointCount, file, lineNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryShapeProfileTestCase::AssertContainsLineStringShape (CurveVectorCPtr const& shape, size_t expectedPointCount, int minCount, CharCP file, size_t lineNo)
    {
    EXPECT_TRUE (std::count_if (shape->begin(), shape->end(),
        [expectedPointCount] (ICurvePrimitiveCPtr const& curve)
        {
        return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == curve->GetCurvePrimitiveType() &&
            expectedPointCount == curve->GetLineStringCP()->size();
        }) >= minCount) << GetFailureMessage ("Line string not found", file, lineNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryShapeProfileTestCase::AssertContainsArcShape (CurveVectorCPtr const& shape, DPoint3d expectedCenter, double expectedRadius, int minCount, CharCP file, size_t lineNo)
    {
    EXPECT_TRUE (std::count_if (shape->begin(), shape->end(),
        [expectedCenter, expectedRadius] (ICurvePrimitiveCPtr const& curve)
        {
        return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == curve->GetCurvePrimitiveType() &&
            curve->GetArcCP()->center.AlmostEqual (expectedCenter) &&
            BeNumerical::IsEqual (expectedRadius, curve->GetArcCP()->vector0.Magnitude()) &&
            BeNumerical::IsEqual (expectedRadius, curve->GetArcCP()->vector90.Magnitude());
        }) >= minCount) << GetFailureMessage ("Arc is not found", file, lineNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryShapeProfileTestCase, Validate_Square_AsCurvePrimitive_Success)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_Square_AsCurveVector_Success)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_SquareWithInnerSquareRemoved_Success)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_AShape_Success)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_DifferentCurvePrimitivesOnSinglePerimeter_Success)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_MultiplePerimeters_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_InnerHoleAndMultiplePerimeters_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_TwoRectanglesOverlapping_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_UnionOfTwoRectanglesAndCircle_Success)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_3dGeometry_3dCurveVector_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_3dGeometry_Box_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_3dGeometry_Cone_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_3dGeometry_Extrusion_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_3dGeometry_Extrusion2D_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_3dGeometry_RotationalSweep_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_3dGeometry_RuledSweep_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_3dGeometry_Sphere_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_3dGeometry_TorusPipe_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_3dPlane_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_Elevated_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_NotClosedShape_CurvePrimitive_Line_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_NotClosedShape_CurvePrimitive_LineString_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_NotClosedShape_CurveVector_LineString_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_NoArea_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_NoAreaArc_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_UnclosedGeometry_CurvePrimitive_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_BoundedUnclosedGeometry_CurveVector_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_ClosedArc_Success)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_ClosedFrom3Lines_Success)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_ClosedFrom4LinesWithOneOutOfPerimeter_Failed)
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
TEST_F (ArbitraryShapeProfileTestCase, Validate_TwoTouchingRectanglesOneOfWhichIsNotClosed_Failed)
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