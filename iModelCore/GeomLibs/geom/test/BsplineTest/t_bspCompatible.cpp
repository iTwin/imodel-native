/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

void OutputCurves (bvector<MSBsplineCurvePtr> &curves, double shiftX = 0.0, double shiftY = 0.0, double shiftZ = 0.0)
    {
    Check::Shift (shiftX, shiftY, shiftZ);
    for (auto &curve : curves)
        {
        Check::SaveTransformed (curve);
        Check::True (curve->HasValidCountsAndAllocations ());
        }
    Check::Shift (-shiftX, -shiftY, -shiftZ);
    }
//static int s_noisy = 0;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineCurve,MakeCompatible)
    {
    auto inputShift = Transform::From (DVec3d::From (0,0,1));
    bvector<double> knotsToAdd {0.10, 0.456, 0.69};
    for (bool closed : {false, true})
        {
        SaveAndRestoreCheckTransform shifter1 (0,100,0);
        for (uint32_t numAddedCurve : { 1, 4})
            {
            SaveAndRestoreCheckTransform shifter2 (100,0,0);
            for (int order : {3,4,5})
                {
                SaveAndRestoreCheckTransform shifter3 (0, 20,0);
                for (size_t numAdd = 0; numAdd <= knotsToAdd.size (); numAdd++)
                    {
                    SaveAndRestoreCheckTransform shifter4 (10,0,0);
                    auto curveA = MSBsplineCurve::CreateFromPolesAndOrder (
                                    bvector<DPoint3d> {
                                            DPoint3d::From (0,0,0),
                                            DPoint3d::From (1,0,0),
                                            DPoint3d::From (2,1,0),
                                            DPoint3d::From (3,1,0),
                                            DPoint3d::From (3,2,0),
                                            DPoint3d::From (0,1,0)},
                                            nullptr, nullptr,
                                            order, closed, true);
                    auto curveB = curveA->CreateCopyTransformed (inputShift);
                    for (size_t i = 0; i < numAdd; i++)
                        curveB->AddKnot (knotsToAdd[i], 1);
                    auto shiftK = inputShift;
                    bvector<MSBsplineCurvePtr> curves {curveA, curveB};
                    for (uint32_t k = 2; k <= numAddedCurve; k++)
                        {
                        shiftK = inputShift * shiftK;
                        curves.push_back (curveA->CreateCopyTransformed (shiftK));
                        }
                    bvector<MSBsplineCurvePtr> compatibleCurves;
                    MSBsplineCurve::CloneCompatibleCurves (compatibleCurves, curves, true, false);
                    OutputCurves (curves);
                    OutputCurves (compatibleCurves, 0, 4, 0);
                    }
                }
            }
        }
    Check::ClearGeometry ("MSBsplineCurve.MakeCompatible");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineCurve,CloneArcLengthCompatible)
    {
    auto inputShift = Transform::From (DVec3d::From (0,0,1));
    bvector<double> knotsToAdd {0.10, 0.456, 0.69};
    for (bool closed : {false, true})
        {
        SaveAndRestoreCheckTransform shifter1 (0,100,0);
        for (uint32_t numAddedCurve : { 1, 4})
            {
            SaveAndRestoreCheckTransform shifter2 (100,0,0);
            for (int order : {3,4,5})
                {
                SaveAndRestoreCheckTransform shifter3 (0, 20,0);
                for (size_t numAdd = 0; numAdd <= knotsToAdd.size (); numAdd++)
                    {
                    SaveAndRestoreCheckTransform shifter4 (10,0,0);
                    auto curveA = MSBsplineCurve::CreateFromPolesAndOrder (
                                    bvector<DPoint3d> {
                                            DPoint3d::From (0,0,0),
                                            DPoint3d::From (1,0,0),
                                            DPoint3d::From (2,1,0),
                                            DPoint3d::From (3,1,0),
                                            DPoint3d::From (3,2,0),
                                            DPoint3d::From (0,1,0)},
                                            nullptr, nullptr,
                                            order, closed, true);
                    auto curveB = curveA->CreateCopyTransformed (inputShift);
                    for (size_t i = 0; i < numAdd; i++)
                        curveB->AddKnot (knotsToAdd[i], 1);
                    auto shiftK = inputShift;
                    bvector<MSBsplineCurvePtr> curves {curveA, curveB};
                    for (uint32_t k = 2; k <= numAddedCurve; k++)
                        {
                        shiftK = inputShift * shiftK;
                        curves.push_back (curveA->CreateCopyTransformed (shiftK));
                        }
                    bvector<MSBsplineCurvePtr> compatibleCurves;
                    MSBsplineCurve::CloneArcLengthCompatibleCurves (compatibleCurves, curves, true, false);
                    OutputCurves (curves);
                    OutputCurves (compatibleCurves, 0, 4, 0);
                    }
                }
            }
        }
    Check::ClearGeometry ("MSBsplineCurve.CloneArcLengthCompatible");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineCurve,SurfaceKnots)
    {
    for (double weight : { 0.0, 0.9})
        {
        SaveAndRestoreCheckTransform shifter (20,0);
        auto surface = SurfaceWithSinusoidalControlPolygon (3, 4,
                    10, 7,
                    0.1, 0.76,
                    -0.2, 1.2, weight);
        Check::True (surface->HasValidCountsAndAllocations (), "Initial Surface");
        Check::SaveTransformed (surface);
        Check::Shift (0,15);
        surface->AddKnot (0.48, 1, 0);
        surface->AddKnot (0.94, 2, 1);
        Check::True (surface->HasValidCountsAndAllocations (), "After knot insertion");
        Check::SaveTransformed (surface);
        double tol = 1.0e-6;
        surface->RemoveKnotsBounded (1, tol);
        surface->RemoveKnotsBounded (2, tol);
        Check::True (surface->HasValidCountsAndAllocations (), "After knot removal");
        Check::Shift (0,15);
        Check::SaveTransformed (surface);
        }
    Check::ClearGeometry ("MSBsplineCurve.SurfaceKnots");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineCurve,CreateLoft)
    {
    auto inputShift = Transform::From (DVec3d::From (0,0,1));
    bvector<double> knotsToAdd {0.10, 0.456, 0.69};
    for (bool chordLength : {false, true})
        {
        SaveAndRestoreCheckTransform shifter1 (0,100,0);
        for (uint32_t numAddedCurve : { 1, 4})
            {
            SaveAndRestoreCheckTransform shifter2 (100,0,0);
            // negative order triggers convert-to-rational on first curve ...
            for (int signedOrderRational : {3,4,5, -3, -4})
                {
                SaveAndRestoreCheckTransform shifter3 (0, 20,0);
                for (size_t numAdd = 0; numAdd <= knotsToAdd.size (); numAdd++)
                    {
                    SaveAndRestoreCheckTransform shifter4 (10,0,0);
                    int order = abs (signedOrderRational);
                    auto curveA = MSBsplineCurve::CreateFromPolesAndOrder (
                                    bvector<DPoint3d> {
                                            DPoint3d::From (0,0,0),
                                            DPoint3d::From (1,0,0),
                                            DPoint3d::From (2,1,0),
                                            DPoint3d::From (3,1,0),
                                            DPoint3d::From (3,2,0),
                                            DPoint3d::From (0,1,0)},
                                            nullptr, nullptr,
                                            order, false, true);
                    if (signedOrderRational < 0)
                        curveA->MakeRational ();
                    auto curveB = curveA->CreateCopyTransformed (inputShift);
                    for (size_t i = 0; i < numAdd; i++)
                        curveB->AddKnot (knotsToAdd[i], 1);
                    auto shiftK = inputShift;
                    bvector<MSBsplineCurvePtr> curves {curveA, curveB};
                    for (uint32_t k = 2; k <= numAddedCurve; k++)
                        {
                        shiftK = inputShift * shiftK;
                        curves.push_back (curveA->CreateCopyTransformed (shiftK));
                        }
                    MSBsplineSurface surface;
                    OutputCurves (curves);
                    double tolerance = 1.0e-3;
                    if (Check::True (SUCCESS == surface.InitLoftingSurface (curves, nullptr, nullptr,
                            true, false, false, false, chordLength, false, tolerance),
                            "Construct surface loft"))
                        {
                        auto surfacePtr = surface.CreateCapture ();
                        Check::SaveTransformed (surfacePtr);
                        }
                    }
                }
            }
        }
    Check::ClearGeometry ("MSBsplineCurve.CreateLoft");
    }
struct PointMatrix
{
bvector<DPoint3d> xyz;
uint32_t numX;
uint32_t numY;
PointMatrix (uint32_t _numX, uint32_t _numY) : numX (_numX), numY (_numY) {}
// Initialize as grid with spacing dx,dy and constant z
PointMatrix(uint32_t _numX, uint32_t _numY, double dx, double dy, double z) : numX(_numX), numY(_numY)
    {
    for (uint32_t j = 0; j < numY; j++)
        for (uint32_t i = 0; i < numX; i++)
            xyz.push_back (DPoint3d::From (i * dx, j * dy, z));
    }
void SetZ (uint32_t i, uint32_t j, double z)
    {
    if (i < numX && j < numY)
        xyz[i + j * numX].z = z;
    }
MSBsplineSurfacePtr CreateSurface (int orderX, int orderY)
    {
    return MSBsplineSurface::CreateFromPolesAndOrder(xyz, nullptr,
        nullptr, orderX, numX, false,
        nullptr, orderY, numY, false,
        true);
    }
};

struct IntIntInt
{
int dataA;
int dataB;
int dataC;
IntIntInt (int a, int b, int c): dataA(a), dataB(b), dataC(c){}
};
// ASSUME adjacent triangles share an edge which is the xy sequence in the second triangle
size_t NumInCircle(bvector<Point3d> const &triangles, bvector<DPoint3d> const &points, int numA, int numB)
    {
    size_t n = 0;
    for (size_t i = 0; i + 1 < triangles.size(); i++)
        {
        if (DPoint3dOps::InCircleXY (
            points[triangles[i].x],
            points[triangles[i].y],
            points[triangles[i].z],
            points[triangles[i+1].z], false))
            {
            n++;
            double q, c;
            DPoint3dOps::InCircleXY (
                points[triangles[i].x],
                points[triangles[i].y],
                points[triangles[i].z],
                points[triangles[i + 1].z], false, q, c);
            printf (" (q %lg) (c %lg)\n", q, c);
            }
        }
    if (n != 0)
        printf (" (numAB %d %d)  (numTriangle %d) (numIn %d", numA, numB, (int)triangles.size(), (int)points.size ());
    return n;
    }
void Load(bvector<DPoint3d> &xyz, bvector<DPoint2d> &xy, bvector<int> &index, DPoint2dCR xy0, DPoint2dCR xy1, int n)
    {
    xy.push_back (xy0);
    index.push_back((int)xyz.size ());
    xyz.push_back (DPoint3d::From (xy.back ()));
    for (int i = 1; i < n; i++)
        {
        xy.push_back(DPoint2d::FromInterpolate (xy0, (double)(i) / (double)(n - 1), xy1));
        index.push_back((int)xyz.size());
        xyz.push_back(DPoint3d::From(xy.back()));
        }
    }
void TestGreedyIndexedTriangulation (DPoint2dCR pointA0, DPoint2dCR pointA1, DPoint2dCR pointB0, DPoint2dCR pointB1)
    {
    for (size_t numA : {1, 2, 3, 5, 8})
        {
        SaveAndRestoreCheckTransform shifterA (20,0,0);
        for (size_t numB : {1, 2, 5, 12})
            {
            SaveAndRestoreCheckTransform shifterB(0, 10, 0);
            bvector<DPoint3d> points;
            bvector<DPoint2d> pointA, pointB;
            bvector<int> indexA, indexB;
            bvector<Point3d> triangleIndices;
            Load (points, pointA, indexA, pointA0, pointA1, (int)numA);
            Load (points, pointB, indexB, pointB0, pointB1, (int)numB);
            Check::SaveTransformed (points);
            Check::Shift (3,0,0);
            PolylineOps::GreedyTriangulateBetweenLinestringsWithIndices(pointA, indexA, pointB, indexB, &triangleIndices, nullptr);
            Check::Size (0, NumInCircle(triangleIndices, points, (int)numA, (int)numB), "InCircle Failures in triangulation");
            bvector<DPoint3d> triXYZ;
            for (auto &tri : triangleIndices)
                {
                triXYZ.clear ();
                triXYZ.push_back (points[(uint32_t)tri.x]);
                triXYZ.push_back(points[(uint32_t)tri.y]);
                triXYZ.push_back(points[(uint32_t)tri.z]);
                triXYZ.push_back(points[(uint32_t)tri.x]);
                Check::SaveTransformed (triXYZ);
                }
            }
        }
    }

bool PrintEvaluation(DPoint2dCR point00, DPoint2dCR point10, DPoint2dCR point01, DPoint2dCR point11, double f, bool emitPoint = true, bool print = true)
    {
    auto point = DPoint2d::FromInterpolate(point00, f, point11);
    double q, c;
    bool in = DPoint2dOps::InCircle(point00, point10, point01, point, true, q, c);
    if (print)
        {
        if (f <= 0.5)
            printf(" (fxy %le %le %le) (q  %le) (c %lg) (in %d)\n", f, point.x, point.y, q, c, in ? 1 : 0);
        else
            printf(" ((fxy-1) %le %le %le) (q  %le) (c %lg) (in %d)\n", f - 1.0, point.x, point.y, q, c, in ? 1 : 0);
        }
    if (emitPoint)
        {
        double markerSize = 0.01 * point00.Distance (point11);
        if (!in)
            markerSize *= -1.0;
        Check::SaveTransformedMarker (DPoint3d::From(point), markerSize);
        }
    return in;
    }
double InsideEvaluationFraction(DPoint2dCR point00, DPoint2dCR point10, DPoint2dCR point01, DPoint2dCR point11, bvector<double> const &fractions,
        bool emitPoint = true,
        bool print = true)
    {
    size_t numIn = 0;
    for (auto f : fractions)
        {
        if (PrintEvaluation (point00, point10, point01, point11, f, emitPoint, print))
            numIn++;
        }
    return numIn / (double)fractions.size ();
    }
bvector<double> AddScaled(bvector<double> const &values, double constant, double scale)
    {
    bvector<double> result;
    for (double v : values)
        result.push_back (constant + scale * v);
    return result;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, GreedyIndexedTriangulation)
    {

    DPoint2d point00 = DPoint2d::From(0, 0);
    DPoint2d point10 = DPoint2d::From(1, 0);
    DPoint2d point01 = DPoint2d::From(0, 1);
    DPoint2d point11 = DPoint2d::From(1, 1);
    DPoint2d point22 = DPoint2d::From(2, 2);
    Check::False(DPoint2dOps::InCircle(point00, point10, point01, point22));
    Check::True(DPoint2dOps::InCircle(point00, point10, point22, point01));
    Check::True (DPoint2dOps::InCircle (point00, point10, point01, point11));

    double epsilon = 1.0e-12;
    Check::True(DPoint2dOps::InCircle(point00, point10, point01, DPoint2d::FromInterpolate (point00, 1.0-epsilon, point11)));
    Check::True(DPoint2dOps::InCircle(point00, point10, point01, DPoint2d::FromInterpolate(point00, 1.0 + epsilon, point11)));

    TestGreedyIndexedTriangulation (
        DPoint2d::From(2, 0),  DPoint2d::From(2, 5),
        DPoint2d::From(0, 1), DPoint2d::From(0, 3)
        );
    Check::Shift (20,0,0);
    TestGreedyIndexedTriangulation(
        DPoint2d::From(0, 1), DPoint2d::From(3, 0),
        DPoint2d::From(0,2), DPoint2d::From(3,5)
        );
    Check::Shift(20, 0, 0);
    TestGreedyIndexedTriangulation(
        DPoint2d::From(0, 2), DPoint2d::From(3, 5),
        DPoint2d::From(0, 1), DPoint2d::From(3, 0)
        );
    Check::ClearGeometry("DPoint2d.GreedyIndexedTriangulation");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
* Geometry output:
* a) A circle
*    a1) key points marked with a circle.
* b) Various points are tested for in/out
*    b1) inside points marked with plus
*    b2) outside points marked iwth small circle.
* c) repeat with circle scaled by 1000X and 1/1000X
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, GreedyIndexedTriangulationLimits)
    {
    auto arc0 = DEllipse3d::From (2,5,6,   6,7,0, -7,6,0, 0.0, Angle::TwoPi ());
    for (double centerScale : {1.0, 1000.0, 0.001})
        {
        auto arc = arc0;
        arc.center.Scale (centerScale);
        auto point00 = DPoint2d::From (arc.FractionToPoint (0.4));
        auto point10 = DPoint2d::From(arc.FractionToPoint(0.89));
        auto point01 = DPoint2d::From(arc.FractionToPoint(0.33));
        auto point11 = DPoint2d::From(arc.FractionToPoint(0.79));

        for (auto f : { -2.0, -1.0, 0.0, 0.25, 0.5, 0.75, 0.95, 0.98, 1.0, 1.02, 2.0, 5.0, 10.0 })
            PrintEvaluation(point00, point10, point01, point11, f, true, false);
        bvector<double> baseFractions;
        for (double f = 1.0e-12; f < 0.011; f *= 10.0)
            baseFractions.push_back (f);
        // full inside close to limits ...
        Check::Near (1.0, InsideEvaluationFraction (point00, point10, point01, point11, AddScaled (baseFractions, 0.0, 1.0), true, false), "inside points near a keypoint");
        Check::Near (1.0, InsideEvaluationFraction(point00, point10, point01, point11, AddScaled(baseFractions, 1.0, -1.0), true, false), "inside points NOT near a keypoint");

        DRange1d fuzzyRange (0.2, 0.6);
        Check::Contains (fuzzyRange, InsideEvaluationFraction(point00, point10, point01, point11, AddScaled(baseFractions, 0.0, -1.0), true, false), "outside points mear a keypoint");
        Check::Contains(fuzzyRange, InsideEvaluationFraction(point00, point10, point01, point11, AddScaled(baseFractions, 1.0, 1.0), true, false), "outside points far from a keypoint");

        double markerSize = -0.05 * arc.vector0.Magnitude ();
        Check::SaveTransformed (arc);
        // The first thre points are the ones defining the circle in the test.
        Check::SaveTransformedMarker (DPoint3d::From (point00), markerSize);
        Check::SaveTransformedMarker(DPoint3d::From(point10), markerSize);
        Check::SaveTransformedMarker(DPoint3d::From(point01), markerSize);
        // 4th point is target for line of test points.
        Check::SaveTransformedMarker(DPoint3d::From(point11), 3 * markerSize);
        }
    Check::ClearGeometry("DPoint2d.GreedyIndexedTriangulationLimits");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineSurface, BilinearPatchBlend)
    {
    uint32_t numX = 15;
    uint32_t numY = 5;
    PointMatrix poles(numX, numY, 1.0, 1.0, 0.0);
#define MultipleConfigurations
#ifdef MultipleConfigurations
    for (uint32_t j : bvector<uint32_t>{2, numY - 1})
        {
        poles.SetZ(3, j, 1.0);
        poles.SetZ(6, j, 3.0);
        poles.SetZ(7, j, -1.0);
        poles.SetZ(9, j, 10.0);
        poles.SetZ(12, j, 2.0);
        poles.SetZ(13, j, -8.0);
        }
    poles.SetZ(2, 0, 8.0);
#else
    poles.SetZ(1, 2, 10.0);
    poles.SetZ (0, 1, 2.0);
#endif
    for (int order : {2, 3})
        {
        SaveAndRestoreCheckTransform shifter (2 * numX, 0, 0);
        auto surface = poles.CreateSurface(order, order);
        Check::SaveTransformed(*surface);
        bvector<double> tolerances;
        tolerances.push_back (0.125);
        double factor = 8.0;
        for (int i = 0; i < 1; i++)
            tolerances.push_back (tolerances.back () / factor);
        
        for (double chordTolerance : tolerances)
            {
            Check::Shift (0, numY, 0);
            auto options = IFacetOptions::CreateForSurfaces();
            options->SetAngleTolerance(0.0);
            options->SetChordTolerance(chordTolerance);
            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
            builder->Add(*surface);
            auto mesh = builder->GetClientMeshPtr();
            Check::SaveTransformed (*mesh);
            }
        }
    Check::ClearGeometry("MSBsplineCurve.BilinearPatchBlend");
    }
