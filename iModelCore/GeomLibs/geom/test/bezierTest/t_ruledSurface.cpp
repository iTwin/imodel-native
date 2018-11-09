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

#ifdef TEST_QUADRATIC_BEZIER_CONSTRUCTIONS
// EDL Feb 6, 2018
// These consturctions build quadratic bezier triangles using vertex normals.
// The surfaces are good for mild normals, but cannot handle inflections and strong curvature.
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
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierTriangle,CreateMidEdgePointsFromNormals)
    {
    bvector<double> positiveShifts {0.1, 1};
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
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierTriangle,CreateQuadraticSurfaceFromNormals)
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
    Check::ClearGeometry ("BezierTriangle.CreateQuadraticSurfaceFromNormals");
    }
#endif

//! Given the normal at one end of an edge, compute a cubbic bezier point.
//! The point is 
//! 1) In the plane of the edge and surface normal
//! 2) located 1/3 the edge length along that direction.
//
DPoint3d CubicBezierTargetPoint(DPoint3dCR pointA, DVec3dCR normalA, DPoint3dCR pointB)
    {
    auto edgeVector = DVec3d::FromStartEnd (pointA, pointB);
    auto perpVector = DVec3d::FromCrossProduct (edgeVector, normalA);
    auto tangentVector = DVec3d::FromCrossProduct (normalA, perpVector);

    auto d = edgeVector.Magnitude () / 3.0;

    if (tangentVector.Normalize ())
        {
        return DPoint3d::FromSumOf (pointA, tangentVector, d);
        }
    return pointA;
    }

//! Given the normal at one end of an edge, compute a cubbic bezier point.
//! The point is 
//! 1) In the plane of the edge and surface normal
//! 2) located 1/3 the edge length along that direction.
//! returns 10 points of triangle, starting with 4 points along AB edge.
void CubicBezierTriangleFromPointsAndNormals(bvector<DPoint3d> &bezierPoints, DPoint3dCR pointA, DVec3dCR normalA, DPoint3dCR pointB, DVec3dCR normalB, DPoint3dCR pointC, DVec3dCR normalC)
    {
    bezierPoints.clear ();
    bezierPoints.push_back (pointA);
    bezierPoints.push_back (CubicBezierTargetPoint (pointA, normalA, pointB));
    bezierPoints.push_back (CubicBezierTargetPoint (pointB, normalB, pointA));
    bezierPoints.push_back (pointB);
    bezierPoints.push_back (CubicBezierTargetPoint (pointA, normalA, pointC));
    bezierPoints.push_back (DPoint3d::FromZero ());
    bezierPoints.push_back (CubicBezierTargetPoint (pointB, normalB, pointC));
    bezierPoints.push_back (CubicBezierTargetPoint (pointC, normalC, pointA));
    bezierPoints.push_back (CubicBezierTargetPoint (pointC, normalC, pointB));
    bezierPoints.push_back (pointC);

    DPoint3d centralPoint = DPoint3d::FromZero ();
    for (size_t i : {1,2,4,6,7,8})
        centralPoint.Add (bezierPoints[i]);
    centralPoint.Scale (1.0 / 6.0);
    bezierPoints[5] = centralPoint;
    }
void AddScaled (DPoint3dR xyz, DPoint3dCR delta, double s)
    {
    xyz.x += delta.x * s;
    xyz.y += delta.y * s;
    xyz.z += delta.z * s;
    }

// Given the control net for a bezier cubic triangle, ADD facets to mesh . . .
void FacetCubicBezierTriangle (
uint32_t numEdge,       //!< [in] number of facet edges along each side of the triangle.  Recommended 3 to 6.
bvector<DPoint3d> const &bezier,    //!< [in] bezier control points, e.g. as produced by CubicBezierTriangleFromPointsAndNormals
PolyfaceHeaderR  mesh   //!< [in/out]  mesh to receive facets.
)
    {
    if (numEdge < 1)
        return;
    double df = 1.0 / (numEdge);
    DPoint3d sum;
    bvector<int> &pointIndex = mesh.PointIndex ();
    bvector<DPoint3d> &points = mesh.Point ();
    for (uint32_t j = 0, dropToPreviousRow = numEdge + 2; j <= numEdge; j++, dropToPreviousRow--)
        {
        double y = j * df;
        for (uint32_t i = 0, numI = numEdge - i - j; i <= numI; i++)
            {
            double x = i * df;
            double u = 1.0 - x - y;
            double v = x;
            double w = y;
            double u2 = u * u; double u3 = u * u2;
            double v2 = v * v; double v3 = v * v2;
            double w2 = w * w; double w3 = w * w2;
            sum.Zero ();
            AddScaled (sum, bezier[0], u3);
            AddScaled (sum, bezier[1], 3.0 * u2 * v);
            AddScaled (sum, bezier[2], 3.0 * u * v2);
            AddScaled (sum, bezier[3], v3);
            AddScaled (sum, bezier[4], 3.0 * w * u2);
            AddScaled (sum, bezier[5], 6.0 * w * u * v);
            AddScaled (sum, bezier[6], 3.0 * w * v2);
            AddScaled (sum, bezier[7], 3.0 * w2 * u);
            AddScaled (sum, bezier[8], 3.0 * w2 * v);
            AddScaled (sum, bezier[9], w3);
            points.push_back (sum);
            int q = (int)mesh.Point().size ();  // ONE BASED index of new point
//
//   *---p---q
//   | \ | \ | \
//   *---*---r---s
// q is new index.
// p=q-1
// r=q-dropToPReviousRow
// s=r+1
// triangle (qpr) does "is off to the left" when q==0
// triangle (qrs) is always there.
            if (j > 0)
                {
                int p = q - 1;
                int r = q - dropToPreviousRow;
                int s = r + 1;
                if (i > 0)
                    {
                    pointIndex.push_back (q);
                    pointIndex.push_back (p);
                    pointIndex.push_back (r);
                    pointIndex.push_back (0);
                    }
                pointIndex.push_back (q);
                pointIndex.push_back (r);
                pointIndex.push_back (s);
                pointIndex.push_back (0);
                }
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierTriangle,CreateCubicSurfaceFromNormals)
    {
    // non-unit normals have z=4 and all combinations of values from mixedShifts.
    // 3 shifts ==> 9 normals at each of A,B,C ==> 9*9*9 triangles faceted.  Wow.
    // 2 shifts ==> 9 normals at each of A,B,C ==> 4*4*4 triangles to facet.
    double normalZ = 4.0;
    bvector<double> mixedShifts {-1, 0.8};
    double edgeLength = 10;
    uint32_t numEdge = 6;
    auto pointA = DPoint3d::From (0,0,0);
    auto pointB = DPoint3d::From (edgeLength,0,0);
    auto pointC = DPoint3d::From (0, 0.6 * edgeLength, 0);
    for (auto dxA : mixedShifts)
        {
        for (auto dyA : mixedShifts)
            {
            SaveAndRestoreCheckTransform shiftA (2.0 * edgeLength, 0, 0);
            for (auto dxB : mixedShifts)
                {
                for (auto dyB : mixedShifts)
                    {
                    SaveAndRestoreCheckTransform shiftA (0, 2.0 * edgeLength, 0);
                    for (auto dxC : mixedShifts)
                        {
                        for (auto dyC : mixedShifts)
                            {
                            SaveAndRestoreCheckTransform shiftA (0, 0, 3.0 * edgeLength);
                            auto normalA = DVec3d::From (-dxA, dyA, normalZ);
                            auto normalB = DVec3d::From (dxB, dyB, normalZ);
                            auto normalC = DVec3d::From (dxC, dyC, normalZ);
                            bvector<DPoint3d> bezierNet;
                            CubicBezierTriangleFromPointsAndNormals (bezierNet, pointA, normalA, pointB, normalB, pointC, normalC);
                            auto mesh = PolyfaceHeader::CreateVariableSizeIndexed ();
                            FacetCubicBezierTriangle (numEdge, bezierNet, *mesh);
                            Check::SaveTransformed (bvector<DPoint3d> {
                                pointA + normalA, pointA, pointB, pointB + normalB, pointB, pointC , pointC + normalC, pointC, pointA
                                });
                            Check::SaveTransformed (*mesh);
                            }
                        }
                    }
                }
            }
        }
    Check::ClearGeometry ("BezierTriangle.CreateCubicSurfaceFromNormals");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierDPoint3d,ExactRange)
    {
    bvector<DPoint3d> poles {
        DPoint3d::From (1,0,0),
        DPoint3d::From (2,0,0),
        DPoint3d::From (5,3,1),
        DPoint3d::From (3,7,2),
        DPoint3d::From (0,1,0)
        };
    DRange3d exactRange;
    bsiBezierDPoint3d_getDRange3d (&exactRange, poles.data (), (int)poles.size ());
    DRange3d strokedRange;
    strokedRange.Init ();
    double du = 1.0/ 64.0;
    for (double u = 0; u <= 1.0; u += du)
        {
        DPoint3d xyz;
        bsiBezierDPoint3d_evaluateDPoint3d (&xyz, nullptr, poles.data (), (int)poles.size (), u);
        strokedRange.Extend (xyz);
        }
    double cornerTol = du * strokedRange.low.Distance (strokedRange.high);  // just a guess about how closely the strokes come to the exact box
    Check::LessThanOrEqual (exactRange.low.Distance (strokedRange.low), cornerTol, "stroked range close to exact");
    Check::LessThanOrEqual (exactRange.high.Distance (strokedRange.high), cornerTol, "stroked range close to exact");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierDPoint3d,Subdivide)
    {
    bvector<DPoint3d> allPoles {
        DPoint3d::From (1,0,0),
        DPoint3d::From (2,0,0),
        DPoint3d::From (5,3,1),
        DPoint3d::From (3,7,2),
        DPoint3d::From (0,1,0)
        };

    for (int order = 2; order < (int)allPoles.size (); order++)
        {
        SaveAndRestoreCheckTransform shifter (0,20,0);
        bvector<DPoint3d> poles;
        for (int i = 0; i < order; i++)
            poles.push_back (allPoles[i]);
        bvector<DPoint3d> poleALeft = poles;
        bvector<DPoint3d> poleARight = poles;
        double u = 0.498;   // close enough to midpoint for crude chord magnitude rules apply
        bsiBezierDPoint3d_subdivideLeftInPlace (poleALeft.data (), poleARight.data (), order, u);
        bvector<DPoint3d> poleBLeft = poles;
        bvector<DPoint3d> poleBRight = poles;
        bsiBezierDPoint3d_subdivideRightInPlace (poleBRight.data (), poleBLeft.data (), order, u);
        Check::Near (poleALeft, poleBLeft);
        Check::Near (poleARight, poleBRight);
        Check::SaveTransformed (poles);
        Check::Shift (0,10,0);
        Check::SaveTransformed (poleALeft);
        Check::SaveTransformed (poleARight);
        Check::Shift (0,10,0);
        Check::SaveTransformed (poleBLeft);
        Check::SaveTransformed (poleBRight);
        }
    Check::ClearGeometry ("BezierDPoint3d.Subdivide");
    }

