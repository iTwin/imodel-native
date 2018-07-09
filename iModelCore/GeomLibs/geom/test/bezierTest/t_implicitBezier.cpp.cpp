#include "testHarness.h"


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bezier, ImplicitXYIntersection)
    {
    bvector<bvector<DPoint4d>> curvesAlongX {
        bvector<DPoint4d>{      // line on axis
            DPoint4d::From (0,0,0,1),
            DPoint4d::From (10,0,0,1)},
        bvector<DPoint4d>{      // downward concave
            DPoint4d::From (0,0,0,1),
            DPoint4d::From (5,1,0,1),
            DPoint4d::From (10,0,0,1)},
        bvector<DPoint4d>{      // downward concave
            DPoint4d::From (0,0,0,1),
            DPoint4d::From (4,1,0,1),
            DPoint4d::From (6,1,0,1),
            DPoint4d::From (10,0,0,1)},
        bvector<DPoint4d>{      // downward concave
            DPoint4d::From (0,0,0,1),
            DPoint4d::From (4,1,0,1),
            DPoint4d::From (5,1,0,1),
            DPoint4d::From (7,1,0,1),
            DPoint4d::From (10,0,0,1)}
    };
    RotMatrix rotation = RotMatrix::FromAxisAndRotationAngle (2, 0.8);
    for (auto &xCurveA : curvesAlongX)
        {
        SaveAndRestoreCheckTransform shifter (0, 10, 0);
        int orderA = (int)xCurveA.size ();
        double paramA[2] {0.4, 0.7};
        DPoint3d xyzA[2];
        bsiBezierDPoint4d_evaluateDPoint3dArray (xyzA, nullptr, xCurveA.data (), orderA, paramA, 1);
        for (auto &xCurveB : curvesAlongX)
            {
            SaveAndRestoreCheckTransform shifter (20, 0, 0);
            auto curveB = xCurveB;
            int orderB = (int)curveB.size ();
            if (orderA + orderB > 8)        // experimentally, we observe this max order for the implicit solver ... 
                continue;
            double paramB[2] {0.3, 0.62};
            DPoint3d xyzB[2];
            bsiBezierDPoint4d_evaluateDPoint3dArray (xyzB, nullptr, xCurveB.data (), orderB, paramB, 1);
            DPoint3d pickup = xyzB[0];
            DPoint3d putdown = xyzA[0];
            Transform::From (-pickup.x, -pickup.y, -pickup.z).Multiply (curveB, curveB);
            Transform::From (rotation).Multiply (curveB, curveB);
            Transform::From (putdown.x, putdown.y, putdown.z).Multiply (curveB, curveB);
            DPoint4d xPointA[10], xPointB[10];
            double   xParamA[10], xParamB[10];
            int numIntersection;
            bvector<DPoint3d> strokeA, strokeB;
            bsiBezierDPoint4d_addStrokes (xCurveA.data (), orderA, strokeA, nullptr, nullptr, 40, true, 0, 1);
            bsiBezierDPoint4d_addStrokes (curveB.data (), orderB, strokeB, nullptr, nullptr, 40, true, 0, 1);
            Check::SaveTransformed (strokeA);
            Check::SaveTransformed (strokeB);

            if (Check::True (bsiBezierDPoint4d_intersectXY (
                        xPointA, xParamA, xPointB, xParamB, &numIntersection, 10,
                        xCurveA.data (), orderA, curveB.data (), orderB)))
                {
                if (Check::Int (1, numIntersection))
                    {
                    Check::Near (paramA[0], xParamA[0], "bezier intersection param on A");
                    Check::Near (paramB[0], xParamB[0], "bezier intersection param on B");
                    }
                for (int i = 0; i < numIntersection; i++)
                    {
                    DPoint3d xyz;
                    xPointA[0].GetProjectedXYZ (xyz);
                    Check::SaveTransformedMarker (xyz, -0.02);
                    xPointA[0].GetProjectedXYZ (xyz);
                    Check::SaveTransformedMarker (xyz, 0.02);
                    }
                }

            }
        }
    Check::ClearGeometry ("Bezier.ImplicitXYIntersection");
    
    }
