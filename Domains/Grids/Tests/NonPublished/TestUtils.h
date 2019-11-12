/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BuildingShared/BuildingSharedApi.h>

struct TestUtils
    {
    static void CompareCurves(ICurvePrimitivePtr lhs, ICurvePrimitivePtr rhs);
    static void ComparePoints(bvector<DPoint3d> lhs, bvector<DPoint3d> rhs);
    static ICurvePrimitivePtr CreateSpline(bvector<DPoint3d> poles, int order);
    static ICurvePrimitivePtr CreateInterpolationCurve(bvector<DPoint3d> poles, DVec3d startTangent = DVec3d::From(0, 0, 0), DVec3d endTangent = DVec3d::From(0, 0, 0));
    };