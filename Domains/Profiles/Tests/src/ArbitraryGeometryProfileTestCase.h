/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ProfilesTestCase.h"

#define ASSERT_CURVEVECTOR(curveVector, boundaryType, minIndex)                                 AssertCurveVector (curveVector, boundaryType, minIndex, __FILE__, __LINE__)
#define ASSERT_LINESTRING_CV(curveVector, boundaryType, minIndex, pointCount)                   AssertLineStringShape (curveVector, boundaryType, minIndex, pointCount, __FILE__, __LINE__)
#define ASSERT_LINESTRING(curvePrimitive, pointCount)                                           AssertLineStringShape (curvePrimitive, pointCount, __FILE__, __LINE__)
#define ASSERT_ARC_CV(curveVector, boundaryType, minIndex, center, radius)                      AssertArcShape (curveVector, boundaryType, minIndex, center, radius, __FILE__, __LINE__)
#define ASSERT_ARC(curvePrimitive, center, radius)                                              AssertArcShape (curvePrimitive, center, radius, __FILE__, __LINE__)
#define ASSERT_CHILD_LINESTRING(curveVector, curveIndex, boundaryType, minIndex, pointCount)    AssertChildLineStringShape (curveVector, curveIndex, boundaryType, minIndex, pointCount, __FILE__, __LINE__)
#define ASSERT_CONTAINS_LINESTRING(curveVector, pointCount, expectedCount)                      AssertContainsLineStringShape (curveVector, pointCount, expectedCount, __FILE__, __LINE__)
#define ASSERT_CONTAINS_ARC(curveVector, center, radius, expectedCount)                         AssertContainsArcShape (curveVector, center, radius, expectedCount, __FILE__, __LINE__)


/*---------------------------------------------------------------------------------**//**
* Base class for Arbitrary geometry test cases.
* @bsiclass                                                                      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ArbitraryGeometryProfileTestCase : ProfilesTestCase
    {
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

