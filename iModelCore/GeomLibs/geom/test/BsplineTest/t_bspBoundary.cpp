/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
void NonConvexQuadSimpleFractal(bvector<DPoint3d> &points, int numRecursion, double perpendicularFactor);
void Fractal1(bvector<DPoint3d> &points, int numRecursion, double perpendicularFactor);
void Fractal3(bvector<DPoint3d> &points, int numRecursion, double perpendicularFactor);
// compute points on unit circle.
// Always in include primary axis points.
// apply radial factor to primary axis points
void UnitCirclePoints(bvector<DPoint3d> &points, int numInteriorPointsInQuadrant, double primaryAxisFactor)
    {
    points.clear();
    DPoint3d xyz;
    double dTheta = Angle::PiOver2() / (1.0 + numInteriorPointsInQuadrant);
    for (int quadrant = 0; quadrant < 4; quadrant++)
        {
        double theta0 = quadrant * Angle::PiOver2();
        for (int i = 0; i <= numInteriorPointsInQuadrant; i++)
            {
            double theta = theta0 + i * dTheta;
            xyz = DPoint3d::From(cos(theta), sin(theta), 0);
            if (i == 0)
                xyz.Scale(primaryAxisFactor);
            }
        points.push_back(xyz);
        }
    xyz = points.front();
    points.push_back(xyz);
    }

// static char const * s_bsurfWithoutBoundariesAt54 = R"({"bsurf":{"points":[[[-54.03111244012159,-38.15028387207429,10.369158403946198,1],[-55.09118144012159,-38.1502838720743,10.369158403946198,1]],[[-27.015556232640893,-17.55958744421396,5.184579204387358,0.5000000002328306],[-27.54559073288771,-17.559587444213964,5.184579204387358,0.5000000002328306]],[[-54.03111244012159,-36.63479287207429,7.744068403946198,1],[-55.09118144012159,-36.6347928720743,7.744068403946198,1]],[[-27.015556232640893,-19.075141944919704,2.5595527031649845,0.5000000002328306],[-27.54559073288771,-19.075141944919707,2.5595527031649845,0.5000000002328306]],[[-54.03111244012158,-39.665774872074294,7.7440684039461996,1],[-55.09118144012158,-39.6657748720743,7.7440684039461996,1]],[[-27.015556232640893,-20.590696445625433,5.184579204387358,0.5000000002328306],[-27.54559073288771,-20.590696445625436,5.184579204387358,0.5000000002328306]],[[-54.03111244012159,-38.15028387207429,10.369158403946198,1],[-55.09118144012159,-38.1502838720743,10.369158403946198,1]]],"uKnots":[0,0,1,1],"vKnots":[0,0,0,0.3333333331781129,0.3333333331781129,0.6666666668218871,0.6666666668218871,1,1,1],"orderU":2,"orderV":3}})";
void SaveBox(TransformCR transform, DRange3dCR localRange);

static char const * s_bsurfWithoutBoundaries = R"({"bsurf": {"points": [[[0.53003449999563657, -5.9193133949975163e-08, 1.7500322385228664, 1.0], [-0.53003450000436203, -5.9186263001720363e-08, 1.7500322385194309, 1.0]], [[0.26501725013104860, 1.5155544711091693, 0.87501617092811479, 0.5000000002328306], [-0.26501725011576838, 1.5155544711126048, 0.87501617092639705, 0.5000000002328306]], [[0.53003450001396857, 1.5154910295928872, -0.87505771022006051, 1.0], [-0.53003449998603003, 1.5154910295997581, -0.87505771022349599, 1.0]], [[0.26501725012973054, 5.9187311052255609e-08, -1.7500103815534769, 0.5000000002328306], [-0.26501725011708643, 5.9190746526383009e-08, -1.7500103815551946, 0.5000000002328306]], [[0.53003449999432206, -1.5154909704071073, -0.87505781273420435, 1.0], [-0.53003450000567653, -1.5154909704002364, -0.87505781273763983, 1.0]], [[0.26501725011140209, -1.5155545303022997, 0.8750160684096748, 0.50000000023283063], [-0.26501725013541488, -1.5155545302988642, 0.87501606840795709, 0.5000000002328306]], [[0.53003449999563657, -5.9193133949975163e-08, 1.7500322385228664, 1.0], [-0.53003450000436203, -5.9186263001720363e-08, 1.7500322385194309, 1.0]]], "uKnots" : [0, 0, 1, 1], "vKnots" : [0, 0, 0, 0.3333333331781129, 0.3333333331781129, 0.6666666668218871, 0.6666666668218871, 1, 1, 1], "orderU" : 2, "orderV" : 3}})";
void AddVRangeBoundary(MSBsplineSurfaceR bsurf, double u0, double v0, double u1, double v1)
    {
    bvector<DPoint2d> trimUV{
        DPoint2d::From (u0, v0),
        DPoint2d::From(u0, v1),
        DPoint2d::From(u1, v1),
        DPoint2d::From(u1, v0),
        DPoint2d::From(u0, v0)
        };
    bsurf.AddTrimBoundary (trimUV);
    }
void DoFacets(MSBsplineSurfaceCR bsurf, bool smooth, bool outputBsurf, double angleTolerance = 1.57, double chordTolerance = 0.0049329780630221216, double visibleRadians = 0.0)
    {
    static bool restrictToPoleRange = false;
    auto options = IFacetOptions::CreateForSurfaces();
    options->SetAngleTolerance (angleTolerance);
    options->SetChordTolerance (chordTolerance);
    options->SetBSurfSmoothTriangleFlowRequired(smooth);
    options->SetMaxPerFace (100);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
    builder->Add(bsurf);
    auto mesh = builder->GetClientMeshPtr();
    size_t numOpen, numClosed;
    auto edgesA = mesh->ExtractBoundaryStrings(numOpen, numClosed);
    mesh->MarkInvisibleEdges (0.001);
    auto edgesB = mesh->ExtractBoundaryStrings(numOpen, numClosed);
    DPoint3d point0, point1, point2;
    bsurf.EvaluatePoint (point0, 0,0);
    bsurf.EvaluatePoint(point1, 1, 0);
    bsurf.EvaluatePoint (point2, 0, 0.05);
    DPoint3d pointA = DPoint3d::FromInterpolate (point0, -0.25, point1);
    DPoint3d pointB = DPoint3d::FromInterpolate(point0, 1.25, point1);
    auto markup = bvector<DPoint3d> {point2, pointA, pointB};

    Transform identity = Transform::FromIdentity();
    DRange3d rangeA;
    bsurf.GetPoleRange(rangeA);
    double dx = rangeA.XLength();
    double dz = rangeA.ZLength ();
    SaveBox (identity, rangeA);
    Check::Shift (1.25 * dx, 0, 0);
    DRange3d rangeB = bsurf.FullSurfaceRange (nullptr);
    SaveBox(identity, rangeB);
    Check::Shift(1.25 * dx, 0, 0);
    if (outputBsurf)
        {
        Check::SaveTransformed(bsurf);
        Check::SaveTransformed(markup);
        for (double expansionFraction : {0.0, 0.1, 0.2})
            {
            DRange3d rangeC;
            rangeC = bsurf.TrimmedSurfaceRange (nullptr, expansionFraction, restrictToPoleRange);
            SaveBox(identity, rangeC);
            }
        }
    Check::Shift(2 * dx, 0, 0);


    Check::SaveTransformed(*mesh);
    Check::SaveTransformed(markup);
        {
        SaveAndRestoreCheckTransform shifter (0,0,0);
        for (auto &edges : {edgesA, edgesB})
            {
            Check::Shift (0,0, dz);
            if (edges.IsValid ())
                Check::SaveTransformed (*edges);
            }
        }
    Check::Shift(2 * dx, 0, 0);
    if (visibleRadians > 0.0)
    for (double visibleFraction = 0.88; visibleFraction <= 1.121; visibleFraction += 0.08)
        {
        mesh->MarkAllEdgesVisible();
        mesh->MarkInvisibleEdges(visibleFraction * visibleRadians, nullptr);
        Check::SaveTransformed(*mesh);
        Check::Shift(1.1 * dx, 0, 0);
        }
    Check::Shift(2 * dx, 0, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bsptrim, ExtraBoundariesAroundCylinder)
    {
    bvector<IGeometryPtr> allGeometry;
    static double s_chordTolerance = 0.0049329780630221216;
    static double s_angleTolerance = 1.5707;
    static double s_visibleTolerance = 0.07;
    if (Check::True(BentleyGeometryJson::TryJsonStringToGeometry(s_bsurfWithoutBoundaries, allGeometry))
        && Check::Size (1, allGeometry.size (), "Expect one surface in static string"))
        {
        auto bsurf = allGeometry[0]->GetAsMSBsplineSurface();
        DRange3d range;
        bsurf->GetPoleRange(range);
        double dx = range.XLength();
        SaveAndRestoreCheckTransform shifter (0, 3 * range.YLength ());

        DoFacets (*bsurf, false, true, s_angleTolerance, s_chordTolerance);
        DoFacets(*bsurf, true, false, s_angleTolerance, s_chordTolerance);
        Check::Shift(4.0 * dx, 0, 0);

        AddVRangeBoundary (*bsurf,  1.0, 0.35208405011896232, 0.0, 0.64791594988103762);
        AddVRangeBoundary(*bsurf, 0.0, 0.31458261623726347, 1.0, 0.018750716940849423);
        AddVRangeBoundary(*bsurf, 1.0, 0.68541738376273653, 0.0, 0.98124928305915060);
        DoFacets(*bsurf, false, true, s_angleTolerance, s_chordTolerance);
        DoFacets(*bsurf, true, false, s_angleTolerance, s_chordTolerance);
        Check::Shift(4.0 * dx, 0, 0);

        bsurf->SetOuterBoundaryActive (false);
        DoFacets(*bsurf, false, true, s_angleTolerance, s_chordTolerance, s_visibleTolerance);
        // DoFacets(*bsurf, true, false, s_angleTolerance, s_chordTolerance, s_visibleTolerance);
        Check::Shift(4.0 * dx, 0, 0);

        bsurf->SetOuterBoundaryActive(true);
        bvector<bvector<DPoint2d>> loopA;
        bsurf->GetUVBoundaryLoops (loopA, true);
        double tol = 1.0e-4;
        auto curvesA = bsurf->GetUnstructuredBoundaryCurves (tol, true, true);
        if (curvesA.IsValid ())
            Check::SaveTransformed (*curvesA);
        Check::Shift(1.5 * dx, 0, 0);

        bsurf->FixupBoundaryLoopParity ();
        bvector<bvector<DPoint2d>> loopB;
        bsurf->GetUVBoundaryLoops(loopB, true);
        auto curvesB = bsurf->GetUnstructuredBoundaryCurves(tol, true, true);
        if (curvesB.IsValid())
            Check::SaveTransformed(*curvesB);

        Check::Shift(1.5 * dx, 0, 0);
        Transform identity = Transform::FromIdentity ();
        DRange3d rangeA;
        rangeA = bsurf->TrimmedSurfaceRange (nullptr);
        SaveBox (identity, rangeA);
        Check::SaveTransformed (*bsurf);
        }
    Check::ClearGeometry ("bsptrim.ExtraBoundariesAroundCylinder");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bsptrim, CylinderBoundaryEdges)
    {
    bvector<IGeometryPtr> allGeometry;
    for (bool outerActive  : {false, true})
        {
        for (double uSetback : {0.0, 0.1})
            {
            if (Check::True(BentleyGeometryJson::TryJsonStringToGeometry(s_bsurfWithoutBoundaries, allGeometry))
                && Check::Size(1, allGeometry.size(), "Expect one surface in static string"))
                {
                auto bsurf = allGeometry[0]->GetAsMSBsplineSurface();
                double u0 = uSetback;
                double u1 = 1.0 - uSetback;
                AddVRangeBoundary(*bsurf, u0, 0.25, u1, 0.55);
                bsurf->SetOuterBoundaryActive(outerActive);

                DRange3d range;
                bsurf->GetPoleRange(range);
                double dx = range.XLength();
                SaveAndRestoreCheckTransform shifter(0, 3 * range.YLength());

                DoFacets(*bsurf, false, true);
                DoFacets(*bsurf, true, false);
                Check::Shift(4.0 * dx, 0, 0);


                bvector<bvector<DPoint2d>> loopA;
                bsurf->GetUVBoundaryLoops(loopA, true);
                double tol = 1.0e-4;
                auto curvesA = bsurf->GetUnstructuredBoundaryCurves(tol, true, true);
                if (curvesA.IsValid())
                    Check::SaveTransformed(*curvesA);
                Check::Shift(1.5 * dx, 0, 0);

                bsurf->FixupBoundaryLoopParity();
                bvector<bvector<DPoint2d>> loopB;
                bsurf->GetUVBoundaryLoops(loopB, true);
                auto curvesB = bsurf->GetUnstructuredBoundaryCurves(tol, true, true);
                if (curvesB.IsValid())
                    Check::SaveTransformed(*curvesB);
                }
            }
        }
    Check::ClearGeometry("bsptrim.CylinderBoundaryEdges");
    }

void OutputFacets(MSBsplineSurfaceR surface, double radiansTol, double maxEdgeLength, bool smoothTriangulation = false)
    {
    DRange3d poleRange;
    surface.GetPoleRange(poleRange);
    Check::SaveTransformed (surface);
    double dx = 1.1 * poleRange.XLength ();
    Check::Shift (dx, 0);
    IFacetOptionsPtr options = IFacetOptions::Create();
    options->SetAngleTolerance(radiansTol);
    options->SetMaxEdgeLength(maxEdgeLength);
    options->SetBSurfSmoothTriangleFlowRequired(smoothTriangulation);
    IPolyfaceConstructionPtr builder1 = IPolyfaceConstruction::Create(*options);
    builder1->Add(surface);
    Check::SaveTransformed(builder1->GetClientMeshR());
    Check::Shift(-dx, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bsptrim, exerciseTrimA)
    {
    auto surfaceB = HyperbolicGridSurface(2, 5, 3, 5,
        1.0, 1.0, 0.0,
        0.9, 0.95);
    double ax = 0.1;
    double bx = 0.4;
    double cx = 0.8;
    double ay = 0.2;
    double by = 0.55;
    double cy = 0.85;
    bvector<DPoint3d> diamondPoints
        {
        DPoint3d::From(bx, ay),
        DPoint3d::From(cx, by),
        DPoint3d::From(bx, cy),
        DPoint3d::From(ax, by),
        DPoint3d::From(bx, ay)
        };
    for (auto &surface : { surfaceB /* , surfaceC */ })
        {
        DRange3d poleRange;
        surface->GetPoleRange(poleRange);
        SaveAndRestoreCheckTransform shifter(3.0 * poleRange.XLength(), 0, 0);
        Check::SaveTransformed(surface);
        double dy = 1.1 * ceil(poleRange.YLength() + 0.25);
        for (auto &rawPoints : { diamondPoints })
            {
            Check::Shift(0, dy);
            bvector<DPoint3d> pointsA = rawPoints;
            Check::SaveTransformed(pointsA);
            Check::Shift(0, dy);
            surface->DeleteBoundaries();
            surface->AddTrimBoundary(pointsA);
            surface->SetOuterBoundaryActive(false);
            OutputFacets(*surface, 0.0, 0.10 * poleRange.XLength());

            Check::Shift(0, dy);
            surface->SetOuterBoundaryActive(true);
            OutputFacets(*surface, 0.0, 0.10 * poleRange.XLength());

            // Check::Shift(0, dy);
            // OutputFacets(*surface, 0.0, 0.10 * poleRange.XLength(), true);
            }
        }
    Check::ClearGeometry("bsptrim.exerciseTrimA");
    }

bvector<DPoint3d> ScalePointsToRange(bvector<DPoint3d> const &points, DRange3dCR targetRange)
    {
    DRange3d rangeA = DRange3d::From(points);
    Transform transformAToB, transformBToA;
    if (Transform::TryUniformScaleXYRangeFit(rangeA, targetRange, transformAToB, transformBToA))
        {
        bvector<DPoint3d> pointsB;
        transformAToB.Multiply(pointsB, points);
        return pointsB;
        }
    return points;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bsptrim, exerciseSurfaceTrim)
    {
    auto surfaceB = HyperbolicGridSurface(2, 3, 3, 5,
        1.2, 0.8, -0.9,
        0.9, 0.95);
    auto surfaceC = SurfaceWithSinusoidalControlPolygon(4, 4,
        12, 13,
        0.0, 0.75,
        1.5, 1.40
    );
    DRange3d targetRangeB1 = DRange3d::From(0.1, 0.1, 0.0, 0.4, 0.4, 1.0);
    DRange3d targetRangeB2 = DRange3d::From(0.4, 0.4, 0.0, 0.9, 0.9, 1.0);
    double fractalScale = 1.0;
    for (auto &surface : { surfaceB , surfaceC })
        {
        DRange3d poleRange;
        surface->GetPoleRange(poleRange);
        SaveAndRestoreCheckTransform shifter(3.0 * poleRange.XLength(), 0, 0);
        double dy = 1.1 * ceil(poleRange.YLength() + 0.25);

        OutputFacets(*surface, 0.0, 0.10 * poleRange.XLength(), false);
        Check::Shift(0, dy);
        OutputFacets(*surface, 0.0, 0.10 * poleRange.XLength(), true);
        for (auto generatorFunction : { UnitCirclePoints, NonConvexQuadSimpleFractal, Fractal1, Fractal3 })
            {
            for (int depth : {0, 2})
                {
                Check::Shift(0, dy);
                bvector<DPoint3d> pointsA;
                generatorFunction(pointsA, depth, fractalScale);
                auto pointsB1 = ScalePointsToRange(pointsA, targetRangeB1);
                Check::SaveTransformed(pointsB1);
                Check::Shift(0, dy);
                surface->DeleteBoundaries();
                surface->AddTrimBoundary(pointsB1);
                surface->SetOuterBoundaryActive(false);
                // dense facets on smallish trims
                OutputFacets(*surface, 0.0, 0.03 * poleRange.XLength());
                Check::Shift(0, dy);


                auto pointsB2 = ScalePointsToRange(pointsA, targetRangeB2);
                surface->SetOuterBoundaryActive(true);
                surface->AddTrimBoundary(pointsB2);
                Check::Shift(0, dy);
                OutputFacets(*surface, 0.0, 0.10 * poleRange.XLength());
                Check::Shift(0, dy);
                OutputFacets(*surface, 0.20, 0.0);
                }
            }
        }
    Check::ClearGeometry("bsptrim.exerciseSurfaceTrim");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bsptrim, RangeWithBoundaries)
    {
    bvector<IGeometryPtr> allGeometry;
    static double s_chordTolerance = 0.0049329780630221216;
    static double s_angleTolerance = 1.5707;
    if (Check::True(BentleyGeometryJson::TryJsonStringToGeometry(s_bsurfWithoutBoundaries, allGeometry))
        && Check::Size(1, allGeometry.size(), "Expect one surface in static string"))
        {
        auto bsurf = allGeometry[0]->GetAsMSBsplineSurface();
        DRange3d range;
        bsurf->GetPoleRange(range);
        double dx = range.XLength();
        SaveAndRestoreCheckTransform shifter(0, 3 * range.YLength());

        DoFacets(*bsurf, false, true, s_angleTolerance, s_chordTolerance);
        Check::Shift(4.0 * dx, 0, 0);

        AddVRangeBoundary(*bsurf, 1.0, 0.35208405011896232, 0.0, 0.64791594988103762);
        AddVRangeBoundary(*bsurf, 0.0, 0.31458261623726347, 1.0, 0.018750716940849423);
        AddVRangeBoundary(*bsurf, 1.0, 0.68541738376273653, 0.0, 0.98124928305915060);
        DoFacets(*bsurf, false, true, s_angleTolerance, s_chordTolerance);
        Check::Shift(4.0 * dx, 0, 0);
        }
    Check::ClearGeometry("MSBsplineSurface.RangeWithBoundaries");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineSurface, BadBoundaryMarkupOnSmallBsurf)
    {
    bvector<IGeometryPtr> allGeometry;
    static double s_chordTolerance = 0.0056532798172590076;
    static double s_angleTolerance = 1.5707;
    if (Check::True(BentleyGeometryJson::TryJsonStringToGeometry(s_bsurfWithoutBoundaries, allGeometry))
        && Check::Size(1, allGeometry.size(), "Expect one surface in static string"))
        {
        auto bsurf = allGeometry[0]->GetAsMSBsplineSurface();
        DRange3d range;
        bsurf->GetPoleRange(range);
        double dx = range.XLength();
        SaveAndRestoreCheckTransform shifter(0, 3 * range.YLength());

//        DoFacets(*bsurf, false, true, s_angleTolerance, s_chordTolerance);
//        Check::Shift(4.0 * dx, 0, 0);

        AddVRangeBoundary(*bsurf, 1.0, 0.35208405011896232, 0.0, 0.64791594988103762);
        AddVRangeBoundary(*bsurf, 0.0, 0.31458261623726347, 1.0, 0.018750716940849423);
        AddVRangeBoundary(*bsurf, 1.0, 0.68541738376273653, 0.0, 0.98124928305915060);
        bsurf->SetOuterBoundaryActive(false);
//        DoFacets(*bsurf, false, true, s_angleTolerance, s_chordTolerance);
//        Check::Shift(4.0 * dx, 0, 0);
        auto options = IFacetOptions::CreateForSurfaces();
        options->SetAngleTolerance(s_angleTolerance);
        options->SetChordTolerance(s_chordTolerance);
        options->SetBSurfSmoothTriangleFlowRequired(false);
        options->SetNormalsRequired (true);
        options->SetMaxPerFace(100);
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
        builder->Add(*bsurf);
        auto mesh = builder->GetClientMeshPtr();
        size_t numOpen, numClosed;
        auto edgesA = mesh->ExtractBoundaryStrings(numOpen, numClosed);
        Check::Size (0, numOpen);
        Check::Size (3, numClosed, "Expect each applied trim boundary to generate one loop");
        Check::SaveTransformed (*mesh);
        Check::Shift (2 * dx, 0, 0);
        Check::SaveTransformed (*edgesA);
        }
    Check::ClearGeometry("MSBsplineSurface.BadBoundaryMarkupOnSmallBsurf");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bsptrim, NonConvexBoundaries)
    {
    size_t numI = 8;
    size_t numJ = 12;
    auto bsurf = SurfaceWithSinusoidalControlPolygon(2, 2, numI, numJ, 0.0, Angle::Pi() / numI, 0.0, Angle::Pi() / numJ);

    auto options = IFacetOptions::CreateForSurfaces();
    options->SetMaxEdgeLength(0.25);
    options->SetMaxPerFace(100);
    bvector<DPoint2d> boundary6{
        DPoint2d::From(0.2, 0.4),
        DPoint2d::From(0.5, 0.5),
        DPoint2d::From(0.8, 0.3),
        DPoint2d::From(0.8, 0.8),
        DPoint2d::From(0.5, 0.7),
        DPoint2d::From(0.2, 0.8),
        DPoint2d::From(0.2, 0.4)
        };
    bsurf->AddTrimBoundary(boundary6);
    bsurf->SetOuterBoundaryActive(false);
    Check::SaveTransformed(*bsurf);

    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
    builder->Add(*bsurf);
    auto mesh = builder->GetClientMeshPtr();
    Check::Shift(0, numJ + 10.0);
    Check::SaveTransformed(*mesh);
    Check::ClearGeometry("bsptrim.NonConvexBoundaries");
    }

uint32_t EmitDegenerateFacets(PolyfaceHeaderR mesh, double areaFactor)
    {
    auto visitor = PolyfaceVisitor::Attach(mesh);
    visitor->SetNumWrap (1);
    uint32_t numError = 0;
    for (visitor->Reset(); visitor->AdvanceToNextFace();)
        {
        // hm... probably triangular .....
        double area = PolygonOps::AreaNormal (visitor->Point ()).Magnitude();
        double length = PolylineOps::Length (visitor->Point ());
        double ratio;
        DoubleOps::SafeDivide (ratio, area, length * length, 0.0);
        if (ratio < areaFactor)
            {
            Check::SaveTransformed (visitor->Point ());
            numError++;
            }
        }
    return numError;
    }
void CheckFacetTolerances(MSBsplineSurface &bsurf, double startRelTol, double toleranceFactor, int numStep, double yShiftFactor = 20.0)
    {
    DRange3d range;
    bsurf.GetPoleRange (range);
    double angleTolerance = 1.57;
    Check::SaveTransformed(bsurf);
    double tolerance = startRelTol * range.low.Distance (range.high);
    double dy = std::max (2.0 * range.YLength (), 0.02 * range.XLength ());

    Check::Shift(0, yShiftFactor * dy, 0);
    for (int i = 0; i < numStep; i++, tolerance *= toleranceFactor)
        {
        SaveAndRestoreCheckTransform shifter (1.2 * range.XLength(), 0, 0);
        auto options = IFacetOptions::CreateForSurfaces();
        options->SetAngleTolerance(angleTolerance);
        options->SetChordTolerance(tolerance);
        options->SetCurvedSurfaceMaxPerFace (3);
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
        builder->Add(bsurf);
        auto mesh = builder->GetClientMeshPtr();
        // Check::SaveTransformed (DEllipse3d::FromCenterRadiusXY(range.low, tolerance));
        Check::SaveTransformed(*mesh);
        Check::Shift (0, dy, 0);
        size_t numOpen, numClosed;
        auto edgesA = mesh->ExtractBoundaryStrings(numOpen, numClosed);
        Check::Shift(0, dy, 0);
        Check::SaveTransformed(*edgesA);

        Check::Shift(0, 10 * dy, 0);
        static double s_edgeRadians = 0.0;
        if (s_edgeRadians > 0.0)
            {
            mesh->MarkInvisibleEdges(s_edgeRadians);
            Check::SaveTransformed(*mesh);
            auto edgesB = mesh->ExtractBoundaryStrings(numOpen, numClosed);
            Check::Shift (0, dy, 0);
            Check::SaveTransformed (*edgesB);
            }
        Check::Shift(0, 2 * dy, 0);
        EmitDegenerateFacets(*mesh, 1.0e-3);
        Check::Shift(0, 2 * dy, 0);
        EmitDegenerateFacets(*mesh, 1.0e-5);
        }
    }
#include "data/boundary0.h"
#include "data/boundary1.h"
#include "data/boundary2.h"
#include "data/boundary3.h"
bvector<DPoint3d> CompressToXYZ(bvector<DPoint2d> const &xyA, double chordTol)
    {
    bvector<DPoint3d> xyzA, xyzB;
    for (auto &xy : xyA)
        xyzA.push_back(DPoint3d::From(xy));
    DPoint3dOps::CompressByChordError(xyzB, xyzA, chordTol);
    return xyzB;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bsptrim, rotor)
    {
    BeFileName dataPath;
    BeTest::GetHost().GetDocumentsRoot(dataPath);
    BeFileName outputPath;
    BeTest::GetHost().GetOutputRoot(outputPath);
    printf(" DocumnetRoot %ls\n", dataPath.c_str());
    printf(" OutputPath %ls\n", outputPath.c_str());
    dataPath.AppendToPath(L"GeomLibsTestData");
    dataPath.AppendToPath(L"Polyface");
    dataPath.AppendToPath(L"ChickenScratch");
    bvector<IGeometryPtr> g0;
    bvector<WCharCP> filenames{
        L"rotor.imjs"
        };
    bvector<PolyfaceHeaderPtr> allMesh;
    // auto range = DRange3d::NullRange();
    for (auto &filename : filenames)
        {
        bvector<IGeometryPtr> geometry;
        auto fullPath = dataPath;
        fullPath.AppendToPath(filename);
        if (GTestFileOps::JsonFileToGeometry(fullPath, geometry))
            {
            for (auto g : geometry)
                {
                auto bsurf = g->GetAsMSBsplineSurface();
                if (bsurf.IsValid())
                    {
                    DRange3d range;
                    bsurf->GetPoleRange(range);
                    DPoint3d rangeCenter = DPoint3d::FromInterpolate(range.low, 0.5, range.high);

                    static double scale = 1000.0;
                    double a = 1.0 / scale;
                    for (int i = 0; i < bsurf->GetIntNumPoles(); i++)
                        {
                        double w = 1.0;
                        if (bsurf->weights != nullptr)
                            w = bsurf->weights[i];
                        bsurf->poles[i].x -= rangeCenter.x * w;
                        bsurf->poles[i].y -= rangeCenter.y * w;
                        bsurf->poles[i].z -= rangeCenter.z * w;
                        bsurf->poles[i].Scale(a);
                        }
                    double yShift = 10.0 * range.YLength () * a;
                    double xShift = 10.0 * range.XLength() * a;
                    SaveAndRestoreCheckTransform shifter1( 0, yShift, 0);
                    // for (int loopSet : {1, 2, 106, 204, 205, 206})
                    // for (int loopSet : {1, 2, 3, 204, 205, 206, 304, 305})
                    for (int loopSet : {1, 2, 3, 4, 5, 204, 206})
                        {
                        SaveAndRestoreCheckTransform shifter2(xShift, 0, 0);
                        bsurf->DeleteBoundaries ();
                        if (loopSet == 1)
                            {
                            bsurf->SetOuterBoundaryActive(false);
                            bsurf->AddTrimBoundary(boundary0);
                            bsurf->AddTrimBoundary(boundary1);
                            bsurf->AddTrimBoundary(boundary2);
                            bsurf->AddTrimBoundary(boundary3);
                            }
                        else if (loopSet == 2)
                            {
                            bsurf->SetOuterBoundaryActive(false);
                            bsurf->AddTrimBoundary(boundary1);
                            bsurf->AddTrimBoundary(boundary2);
                            bsurf->AddTrimBoundary(boundary3);
                            }
                        else if (loopSet == 3)
                            {
                            bsurf->SetOuterBoundaryActive(false);
                            AddVRangeBoundary(*bsurf, 0, 0.25, 1, 0.35);
                            bsurf->AddTrimBoundary(boundary1);
                            bsurf->AddTrimBoundary(boundary2);
                            bsurf->AddTrimBoundary(boundary3);
                            }
                        else if (loopSet == 4)
                            {
                            bsurf->SetOuterBoundaryActive(false);
                            AddVRangeBoundary(*bsurf, 0, 0.25, 1, 0.35);
                            double v0 = 0.26;
                            double v1 = 0.30;
                            double du = 0.09;
                            for (double u0 = 0.1; u0 < 0.72; u0 += 0.10)
                                AddVRangeBoundary(*bsurf, u0, v0,u0 + du, v1);
                            }
                        else if (loopSet == 5)
                            {
                            bsurf->SetOuterBoundaryActive(false);
                            AddVRangeBoundary(*bsurf, 0, 0.0, 1, 0.35);
                            }
                        else if (loopSet > 100 && loopSet <= 120)
                            {
                            bsurf->SetOuterBoundaryActive(false);
                            double tol = pow (10.0, 100 - loopSet);
                            bsurf->AddTrimBoundary(CompressToXYZ(boundary0, tol));
                            bsurf->AddTrimBoundary(CompressToXYZ(boundary1, tol));
                            bsurf->AddTrimBoundary(CompressToXYZ(boundary2, tol));
                            bsurf->AddTrimBoundary(CompressToXYZ(boundary3, tol));
                            }
                        else if (loopSet > 200 && loopSet <= 220)
                            {
                            bsurf->SetOuterBoundaryActive(true);
                            double a1 = pow(10.0, 200 - loopSet);
                            AddVRangeBoundary(*bsurf, 0.17, 0.27, 0.23, 0.29);
                            AddVRangeBoundary(*bsurf, 0.77, 0.27 - a, 0.83, 0.29 + a1);
                            }
                        else if (loopSet > 300 && loopSet <= 320)
                            {
                            bsurf->SetOuterBoundaryActive(false);
                            double a1 = pow(10.0, 300 - loopSet);
                            AddVRangeBoundary(*bsurf, 0.17, 0.27, 0.23, 0.29);
                            AddVRangeBoundary(*bsurf, 0.77, 0.27 - a, 0.83, 0.29 + a1);
                            AddVRangeBoundary(*bsurf, 0.0, 0.0, 1.0, 0.30);
                            }
                        else
                            {
                            bsurf->SetOuterBoundaryActive(true);
                            AddVRangeBoundary(*bsurf, 0.1, 0.1, 0.15, 0.15);
                            AddVRangeBoundary(*bsurf, 0.1, 0.20, 0.15, 0.25);
                            AddVRangeBoundary(*bsurf, 0.8, 0.20, 0.8, 0.25);
                            AddVRangeBoundary(*bsurf, 0.8, 0.1, 0.9, 0.2);
                            AddVRangeBoundary(*bsurf, 0.1, 0.7, 0.2, 0.8);
                            AddVRangeBoundary(*bsurf, 0.8, 0.7, 0.9, 0.8);

                            double du0 = 0.04;
                            double v0 = 0.4;
                            double v1 = 0.43;
                            for (double u0 = 0.3; u0 < 0.62; u0 += 0.05)
                                {
                                double u1 = u0 + du0;
                                AddVRangeBoundary (*bsurf, u0, v0, u1, v1);
                                }
                            double u2 = 0.25;
                            double u3 = 0.31;
                            for (double v2 = 0.1; v2 < 0.9; v2 += 0.1)
                                AddVRangeBoundary (*bsurf, u2, v2, u3, v2 + 0.075);
                                }
                        CheckFacetTolerances (*bsurf, 0.00019 * 2, 2.0, 3);
                        }
                    }
                }
            }
        }
    Check::ClearGeometry("Polyface.ChickenScratch.rotor");
    }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TEST(bsptrim, rotorPatches)
        {
        BeFileName dataPath;
        BeTest::GetHost().GetDocumentsRoot(dataPath);
        BeFileName outputPath;
        BeTest::GetHost().GetOutputRoot(outputPath);
        printf(" DocumnetRoot %ls\n", dataPath.c_str());
        printf(" OutputPath %ls\n", outputPath.c_str());
        dataPath.AppendToPath(L"GeomLibsTestData");
        dataPath.AppendToPath(L"Polyface");
        dataPath.AppendToPath(L"ChickenScratch");
        bvector<IGeometryPtr> g0;
        bvector<WCharCP> filenames{
            L"rotor.imjs"
            };
        bvector<PolyfaceHeaderPtr> allMesh;
        // auto range = DRange3d::NullRange();
        for (auto &filename : filenames)
            {
            bvector<IGeometryPtr> geometry;
            auto fullPath = dataPath;
            fullPath.AppendToPath(filename);
            if (GTestFileOps::JsonFileToGeometry(fullPath, geometry))
                {
                for (auto g : geometry)
                    {
                    auto bsurf = g->GetAsMSBsplineSurface();
                    if (bsurf.IsValid())
                        {
                        DRange3d range;
                        bsurf->GetPoleRange(range);
                        DPoint3d rangeCenter = DPoint3d::FromInterpolate(range.low, 0.5, range.high);

                        static double scale = 1000.0;
                        double a = 1.0 / scale;
                        for (int i = 0; i < bsurf->GetIntNumPoles(); i++)
                            {
                            double w = 1.0;
                            if (bsurf->weights != nullptr)
                                w = bsurf->weights[i];
                            bsurf->poles[i].x -= rangeCenter.x * w;
                            bsurf->poles[i].y -= rangeCenter.y * w;
                            bsurf->poles[i].z -= rangeCenter.z * w;
                            bsurf->poles[i].Scale(a);
                            }
                        double yShift = 10.0 * range.YLength() * a;
                        // double xShift = 10.0 * range.XLength() * a;
                        SaveAndRestoreCheckTransform shifter1(0, yShift, 0);
                        bvector<bvector<MSBsplineSurface>> bezierSurfaces;
                        bvector<bvector<DRange2d>> bezierRanges;

                        double u0 = 0.2;
                        double u1 = 0.3;
                        double v0 = 0.35;
                        double v1 = 0.50;
                        if (SUCCESS == bsurf->MakeBeziers(bezierSurfaces, &bezierRanges))
                            {
                            for (size_t vIndex = 0; vIndex < 1; vIndex++)
                                {
                                // for (size_t uIndex = 0; uIndex < bezierSurfaces[vIndex].size(); uIndex++)
                                for (size_t uIndex = 0; uIndex < 1; uIndex++)
                                    {
                                    SaveAndRestoreCheckTransform shifter2 (2 * range.XLength(), 0, 0);
                                    AddVRangeBoundary(bezierSurfaces[vIndex][uIndex], u0, v0, u1, v1);
                                    CheckFacetTolerances(bezierSurfaces[vIndex][uIndex], 0.00019 * 2, 2.0, 1);
                                    }
                                }
                            }

                        MSBsplineSurface::ClearBeziers(bezierSurfaces);
                        }
                    }
                }
            }
        Check::ClearGeometry("Polyface.ChickenScratch.rotorPatches");
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bsptrim, rotorBoundary)
    {
    for (auto &boundary : {boundary1})
        {
        bvector<DPoint3d> boundary3d;
        for (auto &xy : boundary)
            boundary3d.push_back (DPoint3d::From (xy));
        Check::SaveTransformed(boundary3d);
        DRange3d range = DRange3d::From(boundary3d);
        double dy = 1.1 * range.YLength();
        for (double chordTol : { 1.0e-5, 1.0e-6, 1.0e-7, 1.0e-8, 1.0e-9, 1.0e-10})
            {
            Check::Shift (0, dy, 0);
            bvector<DPoint3d> loop;
            DPoint3dOps::CompressByChordError (loop, boundary3d, chordTol);
            Check::SaveTransformed (loop);
            printf ("  (chordTol %.2lg) (n %d)", chordTol, (int)loop.size ());
            }
        }
    Check::ClearGeometry("Polyface.ChickenScratch.rotorBoundary");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bsptrim, strut)
    {
    BeFileName dataPath;
    BeTest::GetHost().GetDocumentsRoot(dataPath);
    BeFileName outputPath;
    BeTest::GetHost().GetOutputRoot(outputPath);
    printf(" DocumnetRoot %ls\n", dataPath.c_str());
    printf(" OutputPath %ls\n", outputPath.c_str());
    dataPath.AppendToPath(L"GeomLibsTestData");
    dataPath.AppendToPath(L"Polyface");
    dataPath.AppendToPath(L"ChickenScratch");
    bvector<IGeometryPtr> g0;
    bvector<WCharCP> filenames{
        L"strut.imjs",
        L"strut1.imjs"
        };
    bvector<PolyfaceHeaderPtr> allMesh;
    // auto range = DRange3d::NullRange();
    for (auto &filename : filenames)
        {
        bvector<IGeometryPtr> geometry;
        auto fullPath = dataPath;
        fullPath.AppendToPath(filename);
        if (GTestFileOps::JsonFileToGeometry(fullPath, geometry))
            {
            for (auto g : geometry)
                {
                auto bsurf = g->GetAsMSBsplineSurface();
                if (bsurf.IsValid())
                    {
                    DRange3d range;
                    bsurf->GetPoleRange(range);
                    // SaveAndRestoreCheckTransform shifter1 (5 * range.XLength(), 0, 0);
                    DPoint3d rangeCenter = DPoint3d::FromInterpolate(range.low, 0.5, range.high);

                    for (int i = 0; i < bsurf->GetIntNumPoles(); i++)
                        {
                        double w = 1.0;
                        if (bsurf->weights != nullptr)
                            w = bsurf->weights[i];
                        bsurf->poles[i].x -= rangeCenter.x * w;
                        bsurf->poles[i].y -= rangeCenter.y * w;
                        bsurf->poles[i].z -= rangeCenter.z * w;
                        }
                    double yShift = 2.0 * range.YLength();
//                    double xShift = 10.0 * range.XLength();
                    SaveAndRestoreCheckTransform shifter(0, yShift, 0);
                    double relTol = 0.0072587518036889304 / range.low.Distance (range.high);
                    CheckFacetTolerances(*bsurf,  relTol, 2.0, 1, 5.0);
                    }
                }
            }
        }
    Check::ClearGeometry("Polyface.ChickenScratch.strut");
    }

TEST(bsptrim, strutBeziers)
    {
    BeFileName dataPath;
    BeTest::GetHost().GetDocumentsRoot(dataPath);
    BeFileName outputPath;
    BeTest::GetHost().GetOutputRoot(outputPath);
    printf(" DocumnetRoot %ls\n", dataPath.c_str());
    printf(" OutputPath %ls\n", outputPath.c_str());
    dataPath.AppendToPath(L"GeomLibsTestData");
    dataPath.AppendToPath(L"Polyface");
    dataPath.AppendToPath(L"ChickenScratch");
    bvector<IGeometryPtr> g0;
    // ICK!
    // These files have oversaturated bspline surfaces.
    // * order 3 in both directions
    // * all knots have multiplicity "at least 1" -- fully bezier structure right away
    // * some knots have multiplicity 3
    // * At multiplicity=3 knots, the poles themsleves must match to get continuity.  But they don't!!
    // * for test, confirm thtat there are dMax gaps between 1e-6 and 1e-4.
    // Observed dMax is 2.0951950879286212e-05.
    static double dMaxA = 1.0e-6;
    static double dMaxB = 1.0e-4;
    bvector<WCharCP> filenames{
        L"strut.imjs",
        L"strut1.imjs",
        L"fanblade.imjs"
        };
    bvector<PolyfaceHeaderPtr> allMesh;
    // auto range = DRange3d::NullRange();
    for (auto &filename : filenames)
        {
        bvector<IGeometryPtr> geometry;
        auto fullPath = dataPath;
        fullPath.AppendToPath(filename);
        if (GTestFileOps::JsonFileToGeometry(fullPath, geometry))
            {
            for (auto g : geometry)
                {
                auto bsurf = g->GetAsMSBsplineSurface();
                if (bsurf.IsValid())
                    {
                    DRange3d range;
                    bsurf->GetPoleRange(range);
                    SaveAndRestoreCheckTransform shifter1 (3 * range.XLength(), 0, 0);
                    // DPoint3d rangeCenter = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
                    double surfaceTolerance = 1.0e-12 * range.MaxAbs ();
                    Check::SaveTransformed (*bsurf);
                    Check::Shift(0, 0, 2 * range.ZLength());
                    bvector<bvector<MSBsplineSurface>> bezierSurfaces;
                    bvector<bvector<DRange2d>> bezierRanges;
                    if (SUCCESS == bsurf->MakeBeziers(bezierSurfaces, &bezierRanges))
                        {
                        for (auto &slice : bezierSurfaces)
                            {
                            for (auto &patch : slice)
                                {
                                Check::SaveTransformed (patch);
                                }
                            }
                        size_t numJ = bezierSurfaces.size ();
                        bvector<DPoint3d> badPoints;
                        double dMax = 0.0;
                        for (size_t j0 = 0; j0 < numJ; j0++)
                            {
                            for (size_t i0 = 0; i0 < bezierSurfaces[j0].size(); i0++)
                                {
                                size_t i1 = i0 + 1;
                                size_t j1 = j0 + 1;
                                // confirm matching beziers along right edge
                                if (i1 < bezierSurfaces[j0].size ())
                                    {
                                    size_t numU = bezierSurfaces[j0][i0].GetNumUPoles ();
                                    size_t numV = bezierSurfaces[j0][i0].GetNumVPoles ();

                                    for (size_t k = 0; k < numV; k++)
                                        {
                                        DPoint4d point0 = bezierSurfaces[j0][i0].GetPoleDPoint4d(numU - 1, k);
                                        DPoint4d point1 = bezierSurfaces[j0][i1].GetPoleDPoint4d(0, k);
                                        DPoint3d xyz0;
                                        point0.GetProjectedXYZ (xyz0);
                                        double d = point0.RealDistance(point1);
                                        if (d > surfaceTolerance)
                                            badPoints.push_back(xyz0);
                                        dMax = std::max(dMax, d);
                                        }
                                    }
                                // confirm matching beziers along top edge
                                if (j1 < numJ && i0 < bezierSurfaces[j1].size ())
                                    {
                                    size_t numU = bezierSurfaces[j0][i0].GetNumUPoles();
                                    size_t numV = bezierSurfaces[j0][i0].GetNumVPoles();

                                    for (size_t k = 0; k < numU; k++)
                                        {
                                        DPoint4d point0 = bezierSurfaces[j0][i0].GetPoleDPoint4d(k, numV - 1);
                                        DPoint4d point1 = bezierSurfaces[j1][i0].GetPoleDPoint4d(k, 0);
                                        DPoint3d xyz0;
                                        point0.GetProjectedXYZ(xyz0);
                                        double d = point0.RealDistance(point1);
                                        if (d > surfaceTolerance)
                                            badPoints.push_back(xyz0);
                                        dMax = std::max(dMax, d);
                                        }
                                    }
                                }
                            }
                        if (dMax > 1.0e-10)
                            {
                            Check::LessThanOrEqual (dMax, dMaxB, "Expected range of pole gaps");
                            Check::LessThanOrEqual(dMaxA, dMax,  "Expected range of pole gaps");
                            Check::SaveTransformed (badPoints);
                            }
                        }
                    MSBsplineSurface::ClearBeziers(bezierSurfaces);
                    }
                }
            }
        }
    Check::ClearGeometry("Polyface.ChickenScratch.strutBeziers");
    }

// construct a normalization transform to convert boundary curve poles into the domain of normalized knots
/* static Transform constructBoundaryPointNormalizer(MSBsplineSurfaceCR surface)
    {
    KnotData uKnotData;
    DRange1d uKnotRange;
    uKnotData.LoadSurfaceUKnots(surface);
    uKnotData.GetActiveKnotRange(uKnotRange.low, uKnotRange.high);

    KnotData vKnotData;
    DRange1d vKnotRange;
    vKnotData.LoadSurfaceVKnots(surface);
    vKnotData.GetActiveKnotRange(vKnotRange.low, vKnotRange.high);

    if (!uKnotRange.IsPositiveLength() || !vKnotRange.IsPositiveLength())
        {
        Check::Bool(false, "Invalid knot range");
        return Transform::FromIdentity();
        }

    double uScale = 1 / uKnotRange.Length();
    double vScale = 1 / vKnotRange.Length();
    return Transform::FromRowValues(uScale, 0, 0, -uKnotRange.low * uScale, 0, vScale, 0, -vKnotRange.low * vScale, 0, 0, 1, 0);
    } */

// A nice open rational B-surf (torus patch) with a 2D B-curve trim loop
TEST(bsptrim, IvanSurface)
    {
    int uPoleCount = 4;
    int vPoleCount = 9;

    bvector<DPoint3d> points;
    points.push_back(DPoint3d::From(0.048738232662344672, 0.037810853525115817, -0.12964363083517583));
    points.push_back(DPoint3d::From(0.049426499052401596, 0.0371097355053962, -0.12963534267109367));
    points.push_back(DPoint3d::From(0.049898467138177693, 0.036628956078516239, -0.1303442064247804));
    points.push_back(DPoint3d::From(0.04986738923264511, 0.036660616279618807, -0.13131650979203613));

    points.push_back(DPoint3d::From(0.0487788529729869, 0.0378564389186522, -0.12964366500237645));
    points.push_back(DPoint3d::From(0.0494555588802541, 0.037144000481021067, -0.12963287616171471));
    points.push_back(DPoint3d::From(0.049920873913038122, 0.036654161583783207, -0.1303400252086746));
    points.push_back(DPoint3d::From(0.049892051409983651, 0.036684569214571638, -0.13131244161872857));

    points.push_back(DPoint3d::From(0.049105649299235665, 0.038205279970043193, -0.12964405282359692));
    points.push_back(DPoint3d::From(0.049689783130929754, 0.037406676647151471, -0.1296137487349931));
    points.push_back(DPoint3d::From(0.050101056639732633, 0.036847138182110939, -0.13030746211618904));
    points.push_back(DPoint3d::From(0.050089252198858958, 0.0368670002202407, -0.13128068710145158));

    points.push_back(DPoint3d::From(0.049848747715827812, 0.038741321303177756, -0.12964657267327695));
    points.push_back(DPoint3d::From(0.050228938695795478, 0.037816993419482969, -0.12958055388626022));
    points.push_back(DPoint3d::From(0.050509594811387615, 0.037145049171385836, -0.13024887041546052));
    points.push_back(DPoint3d::From(0.050519558315500035, 0.037134961595597815, -0.13122247565047473));

    points.push_back(DPoint3d::From(0.051100935660656432, 0.039096783198715457, -0.12965297644706553));
    points.push_back(DPoint3d::From(0.051152994463450341, 0.038102556068224658, -0.12954452747489142));
    points.push_back(DPoint3d::From(0.051195703832263462, 0.037346711481973216, -0.13018114886983057));
    points.push_back(DPoint3d::From(0.051202889346654956, 0.037290856348931811, -0.13115313443450205));

    points.push_back(DPoint3d::From(0.0524097528676748, 0.038993937499412823, -0.12965941458290331));
    points.push_back(DPoint3d::From(0.05212894357669029, 0.038042811680043087, -0.12952087046345184));
    points.push_back(DPoint3d::From(0.051910871482732546, 0.037297298065027462, -0.13013416421466673));
    points.push_back(DPoint3d::From(0.05188885037853197, 0.037213346720250229, -0.1311038492578227));

    points.push_back(DPoint3d::From(0.053603508789706211, 0.038433759905387888, -0.12966314693494496));
    points.push_back(DPoint3d::From(0.053022340582401739, 0.037632926293440505, -0.12951009941574299));
    points.push_back(DPoint3d::From(0.052561098255864636, 0.036996472296095817, -0.13011186994276613));
    points.push_back(DPoint3d::From(0.052502034911981355, 0.036913894596409591, -0.13108006382318393));

    points.push_back(DPoint3d::From(0.054214830024989169, 0.037802258453211834, -0.12966256356600425));
    points.push_back(DPoint3d::From(0.053477337604817876, 0.037161426358807148, -0.12951125648420359));
    points.push_back(DPoint3d::From(0.052892387261749718, 0.036653546054253638, -0.13011447032329571));
    points.push_back(DPoint3d::From(0.052817924129954008, 0.0365893904420318, -0.13108292544091427));

    points.push_back(DPoint3d::From(0.0544540377593421, 0.037444079263309504, -0.12966174391673135));
    points.push_back(DPoint3d::From(0.053653762053158971, 0.036893533760576247, -0.12951409206068831));
    points.push_back(DPoint3d::From(0.053021433040612465, 0.036458526310866546, -0.13012015601047366));
    points.push_back(DPoint3d::From(0.052943920585107662, 0.036405202020773686, -0.1310889134404789));

    bvector<double> weights;
    for (size_t i = 0; i < vPoleCount; ++i)
        {
        weights.push_back(1.17207441027282);
        weights.push_back(0.93412532019840921);
        weights.push_back(0.9429579975762481);
        weights.push_back(1.19857244240634);
        }

    bvector<double> uKnots;
    uKnots.insert(uKnots.end(), 4, 0.0);
    uKnots.insert(uKnots.end(), 4, 1.0);

    bvector<double> vKnots;
    vKnots.insert(vKnots.end(), 4, 0.027830628766925845);
    vKnots.push_back(0.055661257533851691);
    vKnots.push_back(0.24527671568528703);
    vKnots.push_back(0.43326458343471508);
    vKnots.push_back(0.619491099942078);
    vKnots.push_back(0.80416809860372107);
    vKnots.insert(vKnots.end(), 4, 0.988475801421034);

    MSBsplineSurfacePtr surface = MSBsplineSurface::CreateFromPolesAndOrder(points, &weights, &uKnots, static_cast<int>(uKnots.size() - uPoleCount), uPoleCount, false, &vKnots, static_cast<int>(vKnots.size() - vPoleCount), vPoleCount, false, false);

    Check::True(surface.IsValid(), "B-spline Surface constructor succeeded");

    // We normalize this surface's knots WITHOUT similarly transforming the trim curves' poles because the latter are invalid
    // as created below: they don't live in the (non-normalized) knot space [0,1]x[0.0278,0.9885]. However, since they do live
    // in [0,1]x[0,1], normalizing surface knots makes the trim curves valid. This probably indicates an error in computing the
    // trim curves. Assuming valid trim curves, if we normalize surface knots (a linear transform), we must also hit the trim
    // curves with the same transform. See constructBoundaryPointNormalizer() above.
    surface->NormalizeKnots();

    bvector<DPoint3d> boundary1UVPoints =
        {
        DPoint3d::From(0.96310727143831165, 0.957547291449459, 0.0), DPoint3d::From(0.92375426855107046, 0.95277488062672222, 0.0),
        DPoint3d::From(0.88676760922312681, 0.94558804121026485, 0.0), DPoint3d::From(0.81226263833121171, 0.92756165223272724, 0.0),
        DPoint3d::From(0.77494367239401818, 0.91638210423976008, 0.0), DPoint3d::From(0.66677833155832889, 0.88132055442195367, 0.0),
        DPoint3d::From(0.594267409218059, 0.85413694755018377, 0.0), DPoint3d::From(0.49410978689021423, 0.81976888665679359, 0.0),
        DPoint3d::From(0.46895345650926584, 0.81136827709089387, 0.0), DPoint3d::From(0.39954773932759813, 0.78915521391992316, 0.0),
        DPoint3d::From(0.35500390781872537, 0.77596940275769177, 0.0), DPoint3d::From(0.24217404368784787, 0.74604056409664465, 0.0),
        DPoint3d::From(0.17282480326054905, 0.73056315917150816, 0.0), DPoint3d::From(0.09755108632694183, 0.717196635733034, 0.0)
        };
    bvector<double> boundary1Knots;
    boundary1Knots.insert(boundary1Knots.end(), 4, 0.0088283622970101727);
    boundary1Knots.insert(boundary1Knots.end(), 2, 0.29260291023495349);
    boundary1Knots.insert(boundary1Knots.end(), 2, 0.60778217661489387);
    boundary1Knots.insert(boundary1Knots.end(), 2, 1.2463350554406802);
    boundary1Knots.insert(boundary1Knots.end(), 2, 1.4603779060228186);
    boundary1Knots.insert(boundary1Knots.end(), 2, 1.8333993242319222);
    boundary1Knots.insert(boundary1Knots.end(), 4, 2.3819320252429055);

    bvector<DPoint3d> boundary2UVPoints =
        {
        DPoint3d::From(0.09755108632694183, 0.717196635733034, 0.0), DPoint3d::From(0.092135790144687327, 0.6040561552661563, 0.0),
        DPoint3d::From(0.08770926614675334, 0.49224357142439396, 0.0), DPoint3d::From(0.057497519373943512, 0.27289012119698919, 0.0),
        DPoint3d::From(0.03378044033517312, 0.16705682014345097, 0.0), DPoint3d::From(0.0082331818536808977, 0.040201488239094481, 0.0),
        DPoint3d::From(0.0043746620873475871, 0.020578020601520496, 0.0), DPoint3d::From(0.0006949204860723522, 0.00083332893548472076, 0.0),
        };
    bvector<double> boundary2Knots;
    boundary2Knots.insert(boundary2Knots.end(), 4, -1.0);
    boundary2Knots.insert(boundary2Knots.end(), 2, -0.52393223500467556);
    boundary2Knots.insert(boundary2Knots.end(), 2, -0.080716920174867257);
    boundary2Knots.insert(boundary2Knots.end(), 4, 0.0);

    bvector<DPoint3d> boundary3UVPoints =
        {
        DPoint3d::From(0.0006949204860723522, 0.00083332893548472076, 0.0), DPoint3d::From(0.35716863762150247, 0.00083742979983824337, 0.0),
        DPoint3d::From(0.60703266504731057, 0.00084090736229764382, 0.0), DPoint3d::From(0.96349927620607889, 0.00084041651764009927, 0.0)
        };
    bvector<double> boundary3Knots;
    boundary3Knots.insert(boundary3Knots.end(), 4, 1.0429621754748468E-13);
    boundary3Knots.insert(boundary3Knots.end(), 4, 1.5708004337757022);

    bvector<DPoint3d> boundary4UVPoints =
        {
        DPoint3d::From(0.96349927620607889, 0.00084041651764009927, 0.0), DPoint3d::From(0.96310727143831165, 0.957547291449459, 0.0)
        };
    bvector<double> boundary4Knots;
    boundary4Knots.insert(boundary4Knots.end(), 2, 0.0);
    boundary4Knots.insert(boundary4Knots.end(), 2, 1.0);

    MSBsplineCurvePtr boundary1A = MSBsplineCurve::CreateFromPolesAndOrder(boundary1UVPoints, nullptr, &boundary1Knots, static_cast<int>(boundary1Knots.size() - boundary1UVPoints.size()), false, false);
    MSBsplineCurvePtr boundary2A = MSBsplineCurve::CreateFromPolesAndOrder(boundary2UVPoints, nullptr, &boundary2Knots, static_cast<int>(boundary2Knots.size() - boundary2UVPoints.size()), false, false);
    MSBsplineCurvePtr boundary3A = MSBsplineCurve::CreateFromPolesAndOrder(boundary3UVPoints, nullptr, &boundary3Knots, static_cast<int>(boundary3Knots.size() - boundary3UVPoints.size()), false, false);
    MSBsplineCurvePtr boundary4A = MSBsplineCurve::CreateFromPolesAndOrder(boundary4UVPoints, nullptr, &boundary4Knots, static_cast<int>(boundary4Knots.size() - boundary4UVPoints.size()), false, false);

    CurveVectorPtr outerBoundary = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    outerBoundary->Add(ICurvePrimitive::CreateBsplineCurve(boundary1A));
    outerBoundary->Add(ICurvePrimitive::CreateBsplineCurve(boundary2A));
    outerBoundary->Add(ICurvePrimitive::CreateBsplineCurve(boundary3A));
    outerBoundary->Add(ICurvePrimitive::CreateBsplineCurve(boundary4A));
    surface->SetTrim(*outerBoundary);

    Check::SaveTransformed(surface);

    bvector<Byte> buffer;
    BentleyGeometryFlatBuffer::GeometryToBytes(*surface, buffer);
    MSBsplineSurfacePtr surfaceRoundtripped = BentleyGeometryFlatBuffer::BytesToMSBsplineSurface(buffer.data(), buffer.size());
    Check::True(surface->IsSameStructureAndGeometry(*surfaceRoundtripped, 0.0), "Surface successfully roundtripped through flatbuffer");

    auto range = surface->TrimmedSurfaceRange(nullptr, 0.2, true);
    Check::True(!range.IsNull(), "Surface range is valid");

    Check::ClearGeometry("bsptrim.IvanSurface");
    }

// A non-rational B-surf with trim boundary = entire domain (so boundary has no effect)
TEST(bsptrim, IvanSurface2)
    {
    int uPoleCount = 6;
    int vPoleCount = 6;

    bvector<DPoint3d> points;
    points.push_back(DPoint3d::From(0.03418321675678726, 0.047441734081417053, 0.026086542357354858));
    points.push_back(DPoint3d::From(0.034258016581816264, 0.047441734081417053, 0.02595698525990997));
    points.push_back(DPoint3d::From(0.034407330156909666, 0.047373281260320255, 0.025698366561719865));
    points.push_back(DPoint3d::From(0.03461761092330562, 0.047038732957389584, 0.025334149590449329));
    points.push_back(DPoint3d::From(0.034683216756775437, 0.0466661335566414, 0.025220516953567085));
    points.push_back(DPoint3d::From(0.034683216756775437, 0.046441734081327013, 0.025220516953567085));

    points.push_back(DPoint3d::From(0.034553593098735291, 0.047441734081417053, 0.026333171405205746));
    points.push_back(DPoint3d::From(0.034629776325687089, 0.047445994213376252, 0.02620128977942926));
    points.push_back(DPoint3d::From(0.034769698515447089, 0.047383511620637364, 0.025928784836338536));
    points.push_back(DPoint3d::From(0.034922882876344374, 0.047049431892219218, 0.025516091771436322));
    points.push_back(DPoint3d::From(0.034919977944127822, 0.04667021641773772, 0.025357118764787856));
    points.push_back(DPoint3d::From(0.03487709054672905, 0.046441734081327013, 0.025332450038369814));

    points.push_back(DPoint3d::From(0.035176937986193479, 0.047179623310967145, 0.02674722480811198));
    points.push_back(DPoint3d::From(0.035259900479218231, 0.047191144647513283, 0.026614503667445888));
    points.push_back(DPoint3d::From(0.035390486203823457, 0.0471478099630076, 0.026323063894047039));
    points.push_back(DPoint3d::From(0.035448261101237222, 0.046858472489702763, 0.025829161932087175));
    points.push_back(DPoint3d::From(0.03532234197859907, 0.046516909534034312, 0.025589592513469483));
    points.push_back(DPoint3d::From(0.035203085617581564, 0.0463106786961589, 0.025520663380312669));

    points.push_back(DPoint3d::From(0.035736943030087787, 0.0463419176012394, 0.027118143009026596));
    points.push_back(DPoint3d::From(0.035831696226409804, 0.04635106760247254, 0.026988513600507247));
    points.push_back(DPoint3d::From(0.035962061422765146, 0.0463332524739144, 0.026685340286782377));
    points.push_back(DPoint3d::From(0.035934953727178254, 0.046183890876591249, 0.026119120880181868));
    points.push_back(DPoint3d::From(0.035689661843775866, 0.046001791859112018, 0.025801487683970947));
    points.push_back(DPoint3d::From(0.03549565024968615, 0.045891825841295031, 0.025689575649437302));

    points.push_back(DPoint3d::From(0.035852982426263225, 0.045740222307472322, 0.027194919583401145));
    points.push_back(DPoint3d::From(0.035950603505739309, 0.045743292554107029, 0.027066218822277222));
    points.push_back(DPoint3d::From(0.036081621224298033, 0.045737412328549, 0.026761110966340595));
    points.push_back(DPoint3d::From(0.036037070195789056, 0.045687881260732865, 0.026179950759328108));
    points.push_back(DPoint3d::From(0.035766090427500785, 0.045627489238881935, 0.025845714358887051));
    points.push_back(DPoint3d::From(0.035556249483420288, 0.045590978194468335, 0.025724562633321568));

    points.push_back(DPoint3d::From(0.035852982426263225, 0.04544173408135066, 0.027194919583401145));
    points.push_back(DPoint3d::From(0.035950603505739309, 0.04544173408135066, 0.027066218822277222));
    points.push_back(DPoint3d::From(0.036081621224298033, 0.04544173408135066, 0.026761110966340595));
    points.push_back(DPoint3d::From(0.036037070195789056, 0.04544173408135066, 0.026179950759328108));
    points.push_back(DPoint3d::From(0.035766090427500785, 0.04544173408135066, 0.025845714358887051));
    points.push_back(DPoint3d::From(0.035556249483420288, 0.04544173408135066, 0.025724562633321568));

    bvector<double> uKnots;
    uKnots.insert(uKnots.end(), 4, 0.0);
    uKnots.push_back(0.28571428571428603);
    uKnots.push_back(0.571428571428571);
    uKnots.insert(uKnots.end(), 4, 1.0);

    bvector<double> vKnots;
    vKnots.insert(vKnots.end(), 4, -0.15670631871767599);
    vKnots.push_back(-0.0895464678386721);
    vKnots.push_back(-0.044773233919336394);
    vKnots.insert(vKnots.end(), 4, -6.677127913652331E-16);

    MSBsplineSurfacePtr surface = MSBsplineSurface::CreateFromPolesAndOrder(points, nullptr, &uKnots, static_cast<int>(uKnots.size() - uPoleCount), uPoleCount, false, &vKnots, static_cast<int>(vKnots.size() - vPoleCount), vPoleCount, false, false);

    Check::True(surface.IsValid(), "B-spline Surface constructor succeeded");

    // Note that we don't normalize surface knots. The trim curves' poles already live in the non-normalized knot space.

    bvector<DPoint3d> boundary1UVPoints = {DPoint3d::From(0.0, -0.15670631871767599), DPoint3d::From(0.0, 0.0)};
    bvector<double> boundary1Knots;
    boundary1Knots.insert(boundary1Knots.end(), 2, -0.15670631871767599);
    boundary1Knots.insert(boundary1Knots.end(), 2, 0.0);

    bvector<DPoint3d> boundary2UVPoints = {DPoint3d::From(-2.2255200281200496E-14, -6.67712791365233E-16), DPoint3d::From(0.99999999999992939, -6.67712791365233E-16)};
    bvector<double> boundary2Knots;
    boundary2Knots.insert(boundary2Knots.end(), 2, -1.696124157965186);
    boundary2Knots.insert(boundary2Knots.end(), 2, 0.0);

    bvector<DPoint3d> boundary3UVPoints = {DPoint3d::From(0.999999999999881, -6.677127913652331E-16), DPoint3d::From(0.99999999999987654, -0.15670631871767207)};
    bvector<double> boundary3Knots;
    boundary3Knots.insert(boundary3Knots.end(), 2, 0.0);
    boundary3Knots.insert(boundary3Knots.end(), 2, 0.15670631871767599);

    bvector<DPoint3d> boundary4UVPoints = {DPoint3d::From(0.99999999999987654, -0.15670631871767207), DPoint3d::From(-1.2345680033831741E-13, -0.15670631871794491)};
    bvector<double> boundary4Knots;
    boundary4Knots.insert(boundary4Knots.end(), 2, -1.5707963267948966);
    boundary4Knots.insert(boundary4Knots.end(), 2, 0.0);

    MSBsplineCurvePtr boundary1A = MSBsplineCurve::CreateFromPolesAndOrder(boundary1UVPoints, nullptr, &boundary1Knots, static_cast<int>(boundary1Knots.size() - boundary1UVPoints.size()), false, false);
    MSBsplineCurvePtr boundary2A = MSBsplineCurve::CreateFromPolesAndOrder(boundary2UVPoints, nullptr, &boundary2Knots, static_cast<int>(boundary2Knots.size() - boundary2UVPoints.size()), false, false);
    MSBsplineCurvePtr boundary3A = MSBsplineCurve::CreateFromPolesAndOrder(boundary3UVPoints, nullptr, &boundary3Knots, static_cast<int>(boundary3Knots.size() - boundary3UVPoints.size()), false, false);
    MSBsplineCurvePtr boundary4A = MSBsplineCurve::CreateFromPolesAndOrder(boundary4UVPoints, nullptr, &boundary4Knots, static_cast<int>(boundary4Knots.size() - boundary4UVPoints.size()), false, false);

    CurveVectorPtr outerBoundary = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    outerBoundary->Add(ICurvePrimitive::CreateBsplineCurve(boundary1A));
    outerBoundary->Add(ICurvePrimitive::CreateBsplineCurve(boundary2A));
    outerBoundary->Add(ICurvePrimitive::CreateBsplineCurve(boundary3A));
    outerBoundary->Add(ICurvePrimitive::CreateBsplineCurve(boundary4A));
    surface->SetTrim(*outerBoundary);

    Check::SaveTransformed(surface);

    bvector<Byte> buffer;
    BentleyGeometryFlatBuffer::GeometryToBytes(*surface, buffer);
    MSBsplineSurfacePtr surfaceRoundtripped = BentleyGeometryFlatBuffer::BytesToMSBsplineSurface(buffer.data(), buffer.size());
    Check::True(surface->IsSameStructureAndGeometry(*surfaceRoundtripped, 0.0), "Surface successfully roundtripped through flatbuffer");

    auto range = surface->TrimmedSurfaceRange(nullptr, 0.2, true);
    Check::True(!range.IsNull());

    surface->SetOuterBoundaryActive(true);
    auto range1 = surface->TrimmedSurfaceRange(nullptr, 0.2, true);
    Check::True(range1.IsNull(), "Surface is entirely trimmed out");
    surface->SetOuterBoundaryActive(false);

    surface->DeleteBoundaries();
    auto range2 = surface->TrimmedSurfaceRange(nullptr, 0.2, true);
    Check::True(!range2.IsNull());

    Check::Near(range, range2, "Range and trimmed range are same.");

    Check::ClearGeometry("bsptrim.IvanSurface2");
    }

// A rational ruled bsurf with degenerate trim boundary (which results in entire surface being trimmed). Arguably, this is bad data....
TEST(bsptrim, MikeSurface)
    {
    int uOrder = 3;
    int vOrder = 2;
    int uPoleCount = 9;
    int vPoleCount = 2;

    bvector<DPoint3d> points;       // already weighted
    points.push_back(DPoint3d::From(-0.004001524345804632, -0.004044785138670333, -0.007137399999999616));
    points.push_back(DPoint3d::From(-0.006398204999550221, -0.006428795000447286, -0.005046903940040597));
    points.push_back(DPoint3d::From(-0.009048428285207258, -0.009091689079348892, 0.0));
    points.push_back(DPoint3d::From(-0.006398204999550221, -0.006428795000447286, 0.005046903940040597));
    points.push_back(DPoint3d::From(-0.004001524345804632, -0.004044785138670333, 0.007137399999999616));
    points.push_back(DPoint3d::From(0.0007391949995981128, 0.0007086050004036228, 0.005046903940040597));
    points.push_back(DPoint3d::From(0.001045379593670025, 0.0010021188019361952, 0.0));
    points.push_back(DPoint3d::From(0.0007391949995981128, 0.0007086050004036228, -0.005046903940040597));
    points.push_back(DPoint3d::From(-0.004001524345804632, -0.004044785138670333, -0.007137399999999616));

    points.push_back(DPoint3d::From(-0.004023084031561151, -0.004023225452913814, -0.007137399999999616));
    points.push_back(DPoint3d::From(-0.006413449999548907, -0.006413550000448599, -0.005046903940040597));
    points.push_back(DPoint3d::From(-0.009069987970963778, -0.009070129393592372, 0.0));
    points.push_back(DPoint3d::From(-0.006413449999548907, -0.006413550000448599, 0.005046903940040597));
    points.push_back(DPoint3d::From(-0.004023084031561151, -0.004023225452913814, 0.007137399999999616));
    points.push_back(DPoint3d::From(0.000723949999599427, 0.0007238500004023085, 0.005046903940040597));
    points.push_back(DPoint3d::From(0.0010238199079135057, 0.0010236784876927142, 0.0));
    points.push_back(DPoint3d::From(0.000723949999599427, 0.0007238500004023085, -0.005046903940040597));
    points.push_back(DPoint3d::From(-0.004023084031561151, -0.004023225452913814, -0.007137399999999616));

    bvector<double> weights(points.size());
    for (auto j = 0; j < vPoleCount; ++j)
        for (auto i = 0; i < uPoleCount; ++i)
            weights[j * uPoleCount + i] = (i % 2) ? 0.707106781186548 : 1.0;

    bvector<double> uKnots;
    uKnots.insert(uKnots.end(), uOrder, 0.0);
    uKnots.insert(uKnots.end(), 2, 0.249999999999915);
    uKnots.insert(uKnots.end(), 2, 0.500000000000012);
    uKnots.insert(uKnots.end(), 2, 0.750000000000071);
    uKnots.insert(uKnots.end(), uOrder, 1.0);

    bvector<double> vKnots;
    vKnots.insert(vKnots.end(), vOrder, 0.0);
    vKnots.insert(vKnots.end(), vOrder, 1.0);

    MSBsplineSurfacePtr surface = MSBsplineSurface::CreateFromPolesAndOrder(points, &weights, &uKnots, uOrder, uPoleCount, false, &vKnots, vOrder, vPoleCount, false, true);

    Check::True(surface.IsValid(), "B-spline Surface constructor succeeded");

    bvector<DPoint2d> uvBoundaryPoints;
    uvBoundaryPoints.insert(uvBoundaryPoints.end(), 5, DPoint2d::From(0, 1));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.75));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.5));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.25));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.25));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.5));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.75));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 1));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.9999999967187766));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.9999999934375532));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.9999999901563298));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.9999999868751064));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.9999999835938882));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.99999998031267));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.9999999770314517));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.9999999737502335));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.9999999770314544));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.9999999803126751));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.999999983593896));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.9999999868751168));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.9999999901563377));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.9999999934375584));
    uvBoundaryPoints.push_back(DPoint2d::From(0, 0.9999999967187791));
    uvBoundaryPoints.insert(uvBoundaryPoints.end(), 9, DPoint2d::From(0, 1));

    CurveVectorPtr uvBoundary = CurveVector::CreateLinear(uvBoundaryPoints.data(), uvBoundaryPoints.size(), CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, false);
    surface->SetTrim(*uvBoundary);

    Check::SaveTransformed(surface);

    bvector<Byte> buffer;
    BentleyGeometryFlatBuffer::GeometryToBytes(*surface, buffer);
    MSBsplineSurfacePtr surfaceRoundtripped = BentleyGeometryFlatBuffer::BytesToMSBsplineSurface(buffer.data(), buffer.size());
    Check::True(surface->IsSameStructureAndGeometry(*surfaceRoundtripped, 0.0), "Surface successfully roundtripped through flatbuffer");

    // The trim boundary encloses zero area, and since holeOrigin == true, what's inside the loop survives, which is nothing.
    // If we ever ignore these loops in the mesher, this check should be inverted so as not to fail.
    auto range = surface->TrimmedSurfaceRange(nullptr, 0.2, true);
    Check::True(range.IsNull(), "Surface has been completely trimmed out");

    Check::ClearGeometry("bsptrim.MikeSurface");
    }

// A bicubic screw-thread B-surf with trim boundary (takes > 1min to run)
TEST(bsptrim, EugeneSurface)
    {
    if (!Check::GetEnableLongTests())
        return;

    BeFileName dataPath;
    BeTest::GetHost().GetDocumentsRoot(dataPath);
    dataPath.AppendToPath(L"GeomLibsTestData");
    dataPath.AppendToPath(L"BSpline");
    dataPath.AppendToPath(L"eugeneSurface.imjs");
    bvector<IGeometryPtr> geometry;
    if (Check::True(GTestFileOps::JsonFileToGeometry(dataPath, geometry), "Import geometry from JSON"))
        {
        for (auto const& g : geometry)
            {
            auto surface = g->GetAsMSBsplineSurface();
            if (Check::True(surface.IsValid(), "Geometry is a B-spline surface"))
                {
                bvector<Byte> buffer;
                BentleyGeometryFlatBuffer::GeometryToBytes(*surface, buffer);
                MSBsplineSurfacePtr surfaceRoundtripped = BentleyGeometryFlatBuffer::BytesToMSBsplineSurface(buffer.data(), buffer.size());
                Check::True(surface->IsSameStructureAndGeometry(*surfaceRoundtripped, 0.0), "Surface successfully roundtripped through flatbuffer");

                auto range = surface->TrimmedSurfaceRange(nullptr, 0.2, true);
                Check::True(!range.IsNull(), "Surface range is valid");
                }
            }
        }
    }
