/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/ArbitraryGeometryProfileTestCase.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ArbitraryGeometryProfileTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryGeometryProfileTestCase::AssertCurveVector (CurveVectorCPtr const& shape, CurveVector::BoundaryType expectedBoundaryType, size_t expectedMinimumSize, CharCP file, size_t lineNo)
    {
    ASSERT_TRUE (shape.IsValid()) << GetFailureMessage ("Curve vector is null", file, lineNo);
    EXPECT_EQ (expectedBoundaryType, shape->GetBoundaryType()) << GetFailureMessage ("Curve vector boundary type is not as expected", file, lineNo);
    ASSERT_TRUE (shape->size() >= expectedMinimumSize) << GetFailureMessage ("Curve vector has too little primitives", file, lineNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryGeometryProfileTestCase::AssertArcShape (CurveVectorCPtr const& shape, CurveVector::BoundaryType expectedBoundaryType, size_t expectedIndex, DPoint3d expectedCenter, double expectedRadius, CharCP file, size_t lineNo)
    {
    AssertCurveVector (shape, expectedBoundaryType, expectedIndex + 1, file, lineNo);
    AssertArcShape ((*shape)[expectedIndex], expectedCenter, expectedRadius, file, lineNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryGeometryProfileTestCase::AssertArcShape (ICurvePrimitiveCPtr const& shape, DPoint3d expectedCenter, double expectedRadius, CharCP file, size_t lineNo)
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
void ArbitraryGeometryProfileTestCase::AssertLineStringShape (CurveVectorCPtr const& shape, CurveVector::BoundaryType expectedBoundaryType, size_t expectedIndex, size_t expectedPointCount, CharCP file, size_t lineNo)
    {
    AssertCurveVector (shape, expectedBoundaryType, expectedIndex + 1, file, lineNo);
    AssertLineStringShape ((*shape)[expectedIndex], expectedPointCount, file, lineNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryGeometryProfileTestCase::AssertLineStringShape (ICurvePrimitiveCPtr const& shape, size_t expectedPointCount, CharCP file, size_t lineNo)
    {
    ASSERT_TRUE (shape.IsValid()) << GetFailureMessage ("Curve primitive is null", file, lineNo);
    ASSERT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString, shape->GetCurvePrimitiveType()) << GetFailureMessage ("Curve primitive is not a line string", file, lineNo);
    EXPECT_EQ (expectedPointCount, shape->GetLineStringCP()->size()) << GetFailureMessage ("Line string has incorrect point count", file, lineNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryGeometryProfileTestCase::AssertChildLineStringShape (CurveVectorCPtr const& shape, size_t expectedCurveIndex, CurveVector::BoundaryType expectedBoundaryType, size_t expectedChildIndex, size_t expectedPointCount, CharCP file, size_t lineNo)
    {
    ASSERT_TRUE (shape.IsValid()) << GetFailureMessage ("Curve vector is null", file, lineNo);
    EXPECT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector, (*shape)[expectedCurveIndex]->GetCurvePrimitiveType())
        << GetFailureMessage ("Child is not a curve vector", file, lineNo);
    AssertLineStringShape ((*shape)[expectedCurveIndex]->GetChildCurveVectorCP(), expectedBoundaryType, expectedChildIndex, expectedPointCount, file, lineNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryGeometryProfileTestCase::AssertContainsLineStringShape (CurveVectorCPtr const& shape, size_t expectedPointCount, int minCount, CharCP file, size_t lineNo)
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
void ArbitraryGeometryProfileTestCase::AssertContainsArcShape (CurveVectorCPtr const& shape, DPoint3d expectedCenter, double expectedRadius, int minCount, CharCP file, size_t lineNo)
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