/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

void CheckPierce (DPoint3dP points, int n, DPoint3dCR testPoint, DVec3dCR direction, int expectedResult)
    {
    static double d = 5.0;
    static double tolSquared = 1.0e-12;
    DPoint3d origin;
    DPoint3d piercePoint;
    double fraction;
    int baseVertexIndex;
    origin.SumOf (testPoint, direction, d);
    int result0 = bsiPolygon_piercePoint (&piercePoint, &fraction, &baseVertexIndex, points, n, NULL,
            &origin, &direction, tolSquared);
    Check::Int (expectedResult, result0, "pierce test");

    DVec3d vectorU, vectorV, vectorW;
    direction.GetTriad (vectorU, vectorV, vectorW);
    double a = 0.1; // vary direction around a small cone.
    for (double theta = 0.0; theta < 6; theta += 1.0)
        {
        DVec3d coneDirection;
        coneDirection.SumOf (vectorW, 1.0, vectorU, a * cos(theta), vectorV, a * sin(theta));
        origin.SumOf (testPoint, coneDirection, d);
        int result1 = bsiPolygon_piercePoint (&piercePoint, &fraction, &baseVertexIndex, points, n, NULL,
                &origin, &coneDirection, tolSquared);
        Check::Int (expectedResult, result1, "pierce test");
        }
    }

void Pierce (DPoint3dP points, int n, DPoint3dCR testPoint, int expectedResult)
    {
    static double s_edgeFraction0       = 0.2;
    static double s_edgeFractionStep    = 0.2;
    static double s_offsetFraction      = 0.0001;
    static double s_vertexFraction = 0.001;

    DPoint3d centroid;
    DVec3d normal;
    double area;
    bsiPolygon_centroidAreaPerimeter (points, n, &centroid, &normal, &area, NULL, NULL);
    double tolSquared = 1.0e-10 * area;
    double interiorRotationAngle = area > 0.0 ? Angle::PiOver2 () : - Angle::PiOver2 ();

    CheckPierce (points, n, testPoint, normal, expectedResult);

    for (int i = 0; i < n; i++)
        {
        // Exact hit on vertex ...
        DPoint3d point0 = points[i];
        CheckPierce (points, n, points[i], normal, 0);
        DPoint3d point1 = points[(i+1) % n];
        DVec3d tangent0 = DVec3d::FromStartEnd (point0, point1);
        DVec3d perp0;
        perp0.RotateXY (tangent0, interiorRotationAngle);
        if (tangent0.MagnitudeSquared () < tolSquared)
            continue;
        // Find "next" noncoincident vertex.  (ASSUME some point is different !!!)
        DPoint3d point2 = points[i];
        for (int k = 1; k < n; k++)
            {
            point2 = points[(i + k) % n];
            if (point2.DistanceSquared (points[i]) > tolSquared)
                break;
            }
        DVec3d tangent1 = DVec3d::FromStartEnd (point1, point2);
        DVec3d perp1;
        perp1.RotateXY (tangent1, interiorRotationAngle);

        double turnAngle = tangent0.SignedAngleTo (tangent1, normal);

        if (fabs (turnAngle) < 0.001)
            {
            // almost straight, don't try to find the vertex sector.
            }
        else if (turnAngle * area > 0.0)
            {
            // step to a point on the outside of the vertex
            DPoint3d test1 = DPoint3d::FromSumOf (point1, perp0, -s_vertexFraction, perp1, -s_vertexFraction);
            CheckPierce (points, n, test1, normal, -3);
            }
        else
            {
            // step to a point on the inside of the vertex
            DPoint3d test1 = DPoint3d::FromSumOf (point1, perp0, s_vertexFraction, perp1, s_vertexFraction);
            CheckPierce (points, n, test1, normal, 3);
            }


        // small offset left and right of some edge points ...
        for (double f = s_edgeFraction0; f <= 1.0 - 0.999999* s_edgeFraction0;
                    f += s_edgeFractionStep)
            {
            DPoint3d edgePoint    = DPoint3d::FromSumOf (points[i], tangent0, f);
            DPoint3d insidePoint  = DPoint3d::FromSumOf (edgePoint, perp0, s_offsetFraction);
            DPoint3d outsidePoint = DPoint3d::FromSumOf (edgePoint, perp0, -s_offsetFraction);
            CheckPierce (points, n, edgePoint,    normal, 1);
            CheckPierce (points, n, insidePoint,  normal, 2);
            CheckPierce (points, n, outsidePoint, normal, -2);
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolygonPierce, Square)
    {
    DPoint3d shape [] =
        {
        {0,0,0},
        {1,0,0},
        {1,1,0},
        {0,1,0},
        {0,0,0}
        };

    for (int n = 4; n <= 5; n++)
        {
        Pierce (shape, n, DPoint3d::From (0.5, 0.5), 2);
        Pierce (shape, n, DPoint3d::From (0.5, 0), 1);
        Pierce (shape, n, DPoint3d::From (1.5, 0.5), -2);
        Pierce (shape, n, DPoint3d::From (-0.1, -0.1, 0.0), -3);
        Pierce (shape, n, DPoint3d::From (1.5, 1.5),-3);
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolygonPierce, DegenerateQuad1)
    {
    DPoint3d shape [] =
        {
        {0,0,0},
        {1,0,0},
        {1,0,0},
        {1,1,0},
        {0,0,0},
        };

    for (int n = 4; n <= 5; n++)
        {
        Pierce (shape, n, DPoint3d::From (0.5, 0.2), 2);
        Pierce (shape, n, DPoint3d::From (0.5, 0), 1);
        Pierce (shape, n, DPoint3d::From (1.5, 0.5), -2);
        Pierce (shape, n, DPoint3d::From (-0.1, -0.1, 0.0), -3);
        Pierce (shape, n, DPoint3d::From (1.1, 1.1, 0.0), -3);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolygonPierce, DegenerateQuad2)
    {
    DPoint3d shape [] =
        {
        {0,0,0},
        {1,0,0},
        {1,1,0},
        {1,1,0},
        {0,0,0},
        };

    for (int n = 4; n <= 5; n++)
        {
        Pierce (shape, n, DPoint3d::From (0.5, 0.2), 2);
        Pierce (shape, n, DPoint3d::From (0.5, 0), 1);
        Pierce (shape, n, DPoint3d::From (1.5, 0.5), -2);
        Pierce (shape, n, DPoint3d::From (-0.1, -0.1, 0.0), -3);
        Pierce (shape, n, DPoint3d::From (1.1, 1.1, 0.0), -3);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolygonPierce, DegenerateQuad0)
    {
    DPoint3d shape [] =
        {
        {0,0,0},
        {0,0,0},
        {1,0,0},
        {1,1,0},
        {0,0,0},
        };

    for (int n = 4; n <= 5; n++)
        {
        Pierce (shape, n, DPoint3d::From (0.5, 0.2), 2);
        Pierce (shape, n, DPoint3d::From (0.5, 0), 1);
        Pierce (shape, n, DPoint3d::From (1.5, 0.5), -2);
        Pierce (shape, n, DPoint3d::From (-0.1, -0.1, 0.0), -3);
        Pierce (shape, n, DPoint3d::From (1.1, 1.1, 0.0), -3);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolygonPierce, DegenerateQuad3)
    {
    DPoint3d shape [] =
        {
        {0,0,0},
        {1,0,0},
        {1,1,0},
        {0,0,0},
        {0,0,0},
        };

    for (int n = 4; n <= 5; n++)
        {
        Pierce (shape, n, DPoint3d::From (0.5, 0.2), 2);
        Pierce (shape, n, DPoint3d::From (0.5, 0), 1);
        Pierce (shape, n, DPoint3d::From (1.5, 0.5), -2);
        Pierce (shape, n, DPoint3d::From (-0.1, -0.1, 0.0), -3);
        Pierce (shape, n, DPoint3d::From (1.1, 1.1, 0.0), -3);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolygonPierce, L)
    {
    DPoint3d shape [] =
        {
        {0,0,0},
        {4,0,0},
        {4,1,0},
        {1,1,0},
        {1,2,0},
        {0,1,0},
        {0,0,0}
        };

    for (int n = 6; n <= 7; n++)
        {
        Pierce (shape, n, DPoint3d::From (0.5, 0.1), 2);
        Pierce (shape, n, DPoint3d::From (0.5, 0), 1);
        Pierce (shape, n, DPoint3d::From (1.5, 0.2), 2);
        Pierce (shape, n, DPoint3d::From (1.5, -0.2), -2);
        Pierce (shape, n, DPoint3d::From (4.5, 1.5),-3);
        Pierce (shape, n, DPoint3d::From (-0.1, -0.1, 0.0), -3);
        Pierce (shape, n, DPoint3d::From (4.2, 1, 0.0), -2);
        Pierce (shape, n, DPoint3d::From (4.2, -0.1, 0.0), -3);

        }
    }

// (r cos (theta), r sin (theta)) but make near-zero values true zero.
DPoint2d PolarPoint (double r, double theta)
    {
    static double s_axisTol = 1.0e-14;
    DPoint2d xy;
    xy.Init (r * cos(theta), r * sin(theta));
    double tol = s_axisTol * fabs (r);
    if (fabs (xy.x) < tol)
        xy.x = 0.0;
    if (fabs (xy.y) < tol)
        xy.y = 0.0;
    return xy;
    }

// Make an ngon with numEdge edges, vertices equally spaced on unit circle
// First vertex is shifted by startShift steps from 0 degrees.
// optionally repeat first point as closure
// test "obvious" interior and exterior points on star from uv0 to vertices.
void TestNGon (size_t numEdge, size_t startShift, bool repeatFirstPoint, DPoint2d uv0)
    {
    bvector<DPoint2d> uvPoints;
    double dTheta = Angle::TwoPi () / (double)numEdge;
    uvPoints.clear ();
    for (size_t i = 0; i < numEdge; i++)
        {
        double theta = (startShift + i) * dTheta;
        uvPoints.push_back (PolarPoint (1.0, theta));
        }
    if (repeatFirstPoint)
        {
        DPoint2d uv1 = uvPoints[0];
        uvPoints.push_back(uv1);
        }

        DPoint2d uvInside, uvOutside, uv1;
        DPoint3d uvw, ddx, ddy;

        for (size_t i = 0; i < numEdge; i++)
            {
            size_t edgeBase;
            uvInside.Interpolate(uv0, 0.5, uvPoints[i]);
            uvOutside.Interpolate(uv0, 1.5, uvPoints[i]);

            if (Check::True(PolygonOps::PickTriangleFromStart(
                &uvPoints[0], uvPoints.size(), uvInside,
                edgeBase, uvw, ddx, ddy),
                "Convex poly inside point")
                && Check::True(edgeBase + 1 < uvPoints.size(), "valid edge selector")
                )
                {
                uv1.SumOf(uvPoints[0], uvw.x, uvPoints[edgeBase], uvw.y, uvPoints[edgeBase + 1], uvw.z);
                Check::Near(uvInside, uv1, "Inerpolate back to original point");
                }
            Check::False(PolygonOps::PickTriangleFromStart(
                &uvPoints[0], uvPoints.size(), uvOutside,
                edgeBase, uvw, ddx, ddy),
                "Convex poly outside point");
            }
    }


    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TEST(UVPierce, Convex)
        {
        bvector <DPoint2d> uvPoints;

        DPoint2d centralPoint[2];
        centralPoint[0].Zero();
        centralPoint[1].Init(0.1, 0.1);    size_t step = 1;
        for (size_t numPoint = 3; numPoint < 10; numPoint += step, step++)
            {

            for (size_t k = 0; k < 2; k++)
                {
                DPoint2d uv0 = centralPoint[k];
                TestNGon(numPoint, 0, true, uv0);
                TestNGon(numPoint, 1, true, uv0);
                }
            }
        for (size_t k = 0; k < 2; k++)
            {
            DPoint2d uv0 = centralPoint[k];
            TestNGon(4, 0, false, uv0);
            TestNGon(4, 1, false, uv0);
            TestNGon(8, 0, false, uv0);
            TestNGon(8, 3, false, uv0);
            }
        }

    // numCycle full cycles of square wave with altitudes (h1, h1, h2, h2).
    // single stroke return at h3
    void FillSquareWavePolygon(bvector<DPoint2d> &uv, double dx1, double h1, double h2, double dx2, size_t numCycle, double h3)
        {
        uv.clear();
        double x = 0.0;
        double x0 = x;
        for (size_t i = 0; i < numCycle; i++)
            {
            uv.push_back(DPoint2d::From(x, h1));
            x += dx1;
            uv.push_back(DPoint2d::From(x, h1));
            uv.push_back(DPoint2d::From(x, h2));
            x += dx2;
            uv.push_back(DPoint2d::From(x, h2));
            }
        uv.push_back(DPoint2d::From(x, h3));
        uv.push_back(DPoint2d::From(x0, h3));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TEST(UVPierce, SquareWave)
        {
        bvector<DPoint2d> polygon;
        //    FillSquareWavePolygon (polygon, 
        }

// Approximate area of a bsurf patch via gauss quadrature . . .
double PatchGaussArea(BSurfPatch const & patch, uint32_t numGauss = 5)
    {
    BSIQuadraturePoints gauss1D;
    gauss1D.InitGauss(numGauss);
    DPoint3d xyz;
    DVec3d dXdu;
    DVec3d dXdv;
    DPoint2d uv;
    double area = 0.0;
    for (uint32_t ix = 0; ix < numGauss; ix++)
        {
        for (uint32_t iy = 0; iy < numGauss; iy++)
            {
            double w;
            BSIQuadraturePoints::GetXYEval (gauss1D, ix, 0.0, 1.0,
                gauss1D, iy, 0.0, 1.0,
                uv.x, uv.y, w);
            patch.Evaluate (uv, xyz, dXdu, dXdv);
            area += w * dXdu.CrossProductMagnitude (dXdv);
            }
        }
    return area;
    }

void CheckMoments(MSBsplineSurface &surface)
    {
    size_t numU, numV;
    surface.GetIntervalCounts(numU, numV);
    BSurfPatch patch;
    double simpleArea = 0.0;
    for (size_t j = 0; j < numV; j++)
        {
        for (size_t i = 0; i < numU; i++)
            {
            surface.GetPatch (patch, i, j);
            if (!patch.isNullU && !patch.isNullV)
                {
                simpleArea += PatchGaussArea(patch);
                }
            }
        }

    DMatrix4d products[6][8];
    int       count[6][8];
    double area[6][8];
    static double s_tolerance[] = {0.1, 0.01, 0.001};
    int numTolerance = 3;
    int numGauss = 5;
    UsageSums sums;
    static double s_areaRelTol = 1.0e-5;        // a fluffy tolerance for area
    for (int i = 0; i < numTolerance; i++)
        for (int j = 0; j < numGauss; j++)
            {
            surface.ComputeSecondMomentAreaProducts (products[i][j], s_tolerance[i], j + 1, count[i][j]);
            area[i][j] = products[i][j].coff[3][3];
            sums.Accumulate(fabs (area[i][j] - simpleArea));
            }
    if (surface.numBounds == 0)
        Check::LessThanOrEqual(sums.m_min, s_areaRelTol * simpleArea, "BSurf simple area versus meshed area");
    if (Check::PrintDeepStructs ())
        {
        for (int i = 0; i < numTolerance; i++) 
            { 
            printf ("%.15lg %.15lg %.15lg %.15lg %.15lg\n", area[i][0], area[i][1], area[i][2], area[i][3], area[i][4]); 
            } 
        printf ("\n"); 

        for (int i = 0; i < numTolerance; i++) 
            { 
            printf ("%7d %7d %7d %7d %7d\n", count[i][0], count[i][1], count[i][2], count[i][3], count[i][4]); 
            } 
        printf ("\n"); 
        double refArea = area[2][4];
        for (int i = 0; i < numTolerance; i++) 
            {
            printf ("%8.2lg %8.2lg %8.2lg %8.2lg %8.2lg\n",
                fabs(area[i][0] - refArea),
                fabs(area[i][1] - refArea),
                fabs(area[i][2] - refArea),
                fabs(area[i][3] - refArea),
                fabs(area[i][4] - refArea)
                );
            } 
        printf ("\n");
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BSplineSurfaceMoments, BsplineSurface3)
    {
    //IFacetOptionsPtr options = CreateFacetOptions ();
    //IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    //varunused double mySize = SetTransformToNewGridSpot (*builder);

    bvector<DPoint3d> points;

    // almost clean 3x3 grid with edge size 10 -- middle point shifted and raised.
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (10,0,0));
    points.push_back (DPoint3d::From (20,0,0));

    points.push_back (DPoint3d::From (0,10,0));
    points.push_back (DPoint3d::From (10,11,4));
    points.push_back (DPoint3d::From (20,10,0));

    points.push_back (DPoint3d::From (0,20,0));
    points.push_back (DPoint3d::From (10,20,0));
    points.push_back (DPoint3d::From (20,20,0));

    int uPoleCount = 3;
    int vPoleCount = 3;
    bvector<double> weights;
    bvector<double> uknots;
    bvector<double> vknots;

    uknots.push_back (0);
    uknots.push_back (0);
    uknots.push_back (0);
//    uknots.push_back (0.5);
    uknots.push_back (1);
    uknots.push_back (1);
    uknots.push_back (1);

    vknots.push_back (0);
    vknots.push_back (0);
    vknots.push_back (0);
//    vknots.push_back (0.5);
    vknots.push_back (1);
    vknots.push_back (1);
    vknots.push_back (1);

    MSBsplineSurfacePtr surface = MSBsplineSurface::CreateFromPolesAndOrder (points, NULL,
                &uknots, 3, uPoleCount, false,
                &vknots, 3, vPoleCount, false, false);

    DMatrix4d products;
    surface->ComputeSecondMomentAreaProducts (products);
    
    CheckMoments (*surface);
    double r = 0.1;
    double cx = 0.2;
    double cy = 0.35;
    bvector<DPoint2d> trim;
    int n = 40;
    for (int i = 0; i < n; i++)
        {
        double theta = Angle::TwoPi () * i / (double)n;
        trim.push_back (DPoint2d::From (cx + r * cos(theta), cy + r * sin(theta)));
        }
    surface->AddTrimBoundary (trim);
    CheckMoments (*surface);
    }
    
