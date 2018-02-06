#include "testHarness.h"

// ASSUME surface has functional z
void CheckRuledXYPierce
(
DPoint3dCP coffA,
DPoint3dCP coffB,
int order,
double u,
double v
)
    {
    DPoint3d xyz;
    DVec3d dXdu, dXdv;
    bsiBezier_evaluateRuled (u, v, coffA, coffB, order, xyz, dXdu, dXdv);
    bvector<SolidLocationDetail>pickData;
    DPoint2d xy = DPoint2d::From (xyz.x, xyz.y);
    bsiBezier_addRuledPatchXYIntersections (pickData, xy, coffA, coffB, order);
    Check::Size (1, pickData.size (), "pick count");
    if (pickData.size () == 1)
        {
        Check::Near (xyz, pickData[0].GetXYZ (), "pick recovery");
        }

    }

void CheckBilinear (DPoint3dCP coffA, DPoint3dCP coffB)
    {
    for (double u = 0; u <= 1.0; u+= 0.25)
        {
        for (double v = 0; v < 1.0; v += 0.25)
            {
            DPoint3d xyzBez;
            DVec3d dXBezdu, dXBezdv;
            DPoint3d xyz0v = DPoint3d::FromInterpolate (coffA[0], v, coffB[0]);
            DPoint3d xyz1v = DPoint3d::FromInterpolate (coffA[1], v, coffB[1]);
            DPoint3d xyzu0 = DPoint3d::FromInterpolate (coffA[0], u, coffA[1]);
            DPoint3d xyzu1 = DPoint3d::FromInterpolate (coffB[0], u, coffB[1]);
            DPoint3d xyz   = DPoint3d::FromInterpolate (xyz0v, u, xyz1v);
            bsiBezier_evaluateRuled (u, v, coffA, coffB, 2, xyzBez, dXBezdu, dXBezdv);
            DVec3d   dXdv  = DVec3d::FromStartEnd (xyzu0, xyzu1);
            DVec3d   dXdu  = DVec3d::FromStartEnd (xyz0v, xyz1v);
            Check::Near (xyz, xyzBez, "bilinear ruled");
            Check::Near (dXdv, dXBezdv, "bilinear ruled");
            Check::Near (dXdu, dXBezdu, "bilinear ruled");
            }
        }
    }

void CheckRuledPierce (DPoint3dCP coffA, DPoint3dCP coffB, int order)
    {
    for (double u = 0; u <= 1.0; u+= 0.25)
        {
        for (double v = 0; v < 1.0; v += 0.25)
            {
            if (u != 0.0 && v != 0.0)
                CheckRuledXYPierce (coffA, coffB, order, u, v);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RuledBezier,Bilinear0)
    {
    DPoint3d coffA[] = 
        {
        {1,2},
        {3,2}, 
        };

    DPoint3d coffB[] = 
        {
        {1,4},
        {3,4}, 
        };

    CheckBilinear (coffA, coffB);
    CheckRuledPierce (coffA, coffB, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RuledBezier,Bilinear1)
    {
    DPoint3d coffA[] = 
        {
        {1,2},
        {3,-1}, 
        };

    DPoint3d coffB[] = 
        {
        {1,5},
        {4,4}, 
        };

    CheckBilinear (coffA, coffB);
    CheckRuledPierce (coffA, coffB, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RuledBezier,Quad0)
    {
    DPoint3d coffA[] = 
        {
        {1,2},
        {3,-1}, 
        {5, -1}
        };

    DPoint3d coffB[] = 
        {
        {1,5},
        {4,4},
        {5, 4}
        };

    CheckRuledPierce (coffA, coffB, 3);
    }

//! Given the points and normals at each end of an edge, compute a point "near the mid edge" such that the quadratic bezier
//! for the 3 points passes through the ends tangent to those planes.
ValidatedDPoint3d ComputeMidEdgeBezierPoint (DPoint3dCR pointA, DVec3dCR _normalA, DPoint3dCR pointB, DVec3dCR _normalB)
    {
    static double s_scaleFactor = 1.0;
    // X is the bezier point.
    // vector from A to B is U = B-A
    // work with V = X-A
    // Note X-B = (X-(A+U)) = V -U
    // Condition 1: X is on the tangent plane at A:    V.normalA = 0
    // Condition 2: X is on the tangent plane at B:   (V-U).normalB = 0 = V.normalB - U.normalB
    //    i.e. V.normalB = U.normalB
    // Condition 3: X is the closest approach of the edge to the line of intersection of the planes.
    //   * projection of X to edge has fractional coordinate s = V.U / U.U
    //   * line of intersection has direction W = normalA.dot.normalB
    //   * need    (V - sU) dot W = 0
    //     (V dot W - V.U U dot W / (U dot U)) = 0
    //     (U dot U) (V dot W) - (V dot U) (U dot W) = 0
    //     V dot ( U.U W - U.W U) = 0
    DVec3d normalA, normalB;
    double aa;
    DVec3d vectorU = DVec3d::FromStartEnd (pointA, pointB);
    DVec3d vectorW = DVec3d::FromCrossProduct (_normalA, _normalB);
        
    if (normalA.TryNormalize (_normalA, aa) && normalB.TryNormalize (_normalB, aa) && vectorW.TryNormalize (vectorW, aa))
        {
        // error if there is an inflection !!!
        if (normalB.DotProduct (vectorU) * normalA.DotProduct (vectorU) < 0.0)
            {
            DVec3d vectorV;
            DVec3d rightHandSide = DVec3d::From (0, vectorU.DotProduct (normalB), 0.0);
            rightHandSide.Scale (s_scaleFactor); // pull in bad solutions ...
            DVec3d vectorQ = vectorU.DotProduct (vectorU) * vectorW - vectorU.DotProduct(vectorW) * vectorU;
            RotMatrix matrix = RotMatrix::FromRowValues (
                normalA.x, normalA.y, normalA.z,
                normalB.x, normalB.y, normalB.z,
                vectorQ.x, vectorQ.y, vectorQ.z);
            if (matrix.Solve (vectorV, rightHandSide))
                {
                return ValidatedDPoint3d (pointA + vectorV, true);
                }
            }
        }
    return ValidatedDPoint3d (DPoint3d::FromInterpolate (pointA, 0.5, pointB), false);
    }
TEST(BezierTriangle,CreateMidEdgePointsFromNormals)
    {
    bvector<double> positiveShifts {0.1, 1,2,3};
    bvector<double> mixedShifts {-1,0,1,2};
    double edgeLength = 10;
    double normalZ = 5.0;
    int numEdge = 16;
    bvector<double> params;
    for (int i = 0; i <= numEdge; i++)
        params.push_back ((double)i / (double) numEdge);
    int numEval = numEdge + 1;
    bvector<DPoint3d> strokes(numEval);
    for (auto dxA : positiveShifts)
        {
        for (auto dyA : mixedShifts)
            {
            SaveAndRestoreCheckTransform shiftA (2.0 * edgeLength, 0, 0);
            for (auto dxB : positiveShifts)
                {
                for (auto dyB : mixedShifts)
                    {
                    SaveAndRestoreCheckTransform shiftB (0, 0.5 * edgeLength, 0);
                    auto pointA = DPoint3d::From (0,0,0);
                    auto pointB = DPoint3d::From (edgeLength, 0, 0);
                    auto normalA = DVec3d::From (-dxA, dyA, normalZ);
                    auto normalB = DVec3d::From (dxB, dyB, normalZ);
                    auto midPoint = ComputeMidEdgeBezierPoint (pointA, normalA, pointB, normalB);
                    if (Check::True (midPoint.IsValid (), "Compute Bezier midpoint"))
                        {
                        Check::SaveTransformed (bvector<DPoint3d> {
                            pointA + normalA, pointA, pointB, pointB + normalB
                            });
                        bvector<DPoint3d> bezierPoles {pointA, midPoint.Value (), pointB};
                        Check::SaveTransformed (bezierPoles);
                        bsiBezierDPoint3d_evaluateArray (&strokes[0], nullptr, &bezierPoles[0], 3, &params[0], numEval);
                        Check::SaveTransformed (strokes);
                        }
                    }
                }
            }
        }
    Check::ClearGeometry ("BezierTriangle.CreateMidEdgePointsFromNormals");
    }

TEST(BezierTriangle,CreateQuadratic)
    {
    bvector<double> positiveShifts {1,2};
    bvector<double> mixedShifts {-1,0.5};
    double edgeLength = 10;
    double normalZ = 5.0;
    int numEdge = 16;
    bvector<double> params;
    for (int i = 0; i <= numEdge; i++)
        params.push_back ((double)i / (double) numEdge);
    int numEval = numEdge + 1;

    auto pointA = DPoint3d::From (0,0,0);
    auto pointB = DPoint3d::From (edgeLength,0,0);
    auto pointC = DPoint3d::From (0, 0.6 * edgeLength, 0);
    bvector<DPoint3d> strokes(numEval);
    for (auto dxA : positiveShifts)
        {
        for (auto dyA : mixedShifts)
            {
            SaveAndRestoreCheckTransform shiftA (2.0 * edgeLength, 0, 0);
            for (auto dxB : positiveShifts)
                {
                for (auto dyB : mixedShifts)
                    {
                    SaveAndRestoreCheckTransform shiftA (0, 2.0 * edgeLength, 0);
                    for (auto dxC : mixedShifts)
                        {
                        for (auto dyC : positiveShifts)
                            {
                            SaveAndRestoreCheckTransform shiftA (0, 0, 3.0 * edgeLength);
                            auto normalA = DVec3d::From (-dxA, dyA, normalZ);
                            auto normalB = DVec3d::From (dxB, dyB, normalZ);
                            auto normalC = DVec3d::From (dxC, dyC, normalZ);
                            auto pointAB = ComputeMidEdgeBezierPoint (pointA, normalA, pointB, normalB);
                            auto pointBC = ComputeMidEdgeBezierPoint (pointB, normalB, pointC, normalC);
                            auto pointCA = ComputeMidEdgeBezierPoint (pointC, normalC, pointA, normalA);

                            Check::SaveTransformed (bvector<DPoint3d> {
                                pointA + normalA, pointA, pointB, pointB + normalB, pointB, pointC , pointC + normalC, pointC, pointA
                                });
                            Check::SaveTransformed (bvector<DPoint3d> {pointA, pointAB, pointCA, pointA});
                            Check::SaveTransformed (bvector<DPoint3d> {pointB, pointBC, pointAB, pointB});
                            Check::SaveTransformed (bvector<DPoint3d> {pointC, pointCA, pointBC, pointC});
                            }
                        }
                    }
                }
            }
        }
    Check::ClearGeometry ("BezierTriangle.CreateQuadratic");
    }