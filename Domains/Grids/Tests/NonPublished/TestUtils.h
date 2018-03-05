/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/TestUtils.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BuildingShared/BuildingSharedApi.h>

struct TestUtils
    {
    static void CompareCurves(ICurvePrimitivePtr lhs, ICurvePrimitivePtr rhs);
    static void ComparePoints(bvector<DPoint3d> lhs, bvector<DPoint3d> rhs);
    static ICurvePrimitivePtr CreateSpline(bvector<DPoint3d> poles, int order);
    static ICurvePrimitivePtr CreateInterpolationCurve(bvector<DPoint3d> poles, DVec3d startTangent = DVec3d::From(0, 0, 0), DVec3d endTangent = DVec3d::From(0, 0, 0));
    };