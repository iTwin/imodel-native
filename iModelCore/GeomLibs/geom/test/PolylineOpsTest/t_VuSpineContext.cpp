/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include "VuSpineContext.h"

// scale and translate xyzA so all fits in a range box of (targetSize,targetSize) at the origin.
bool ScaleToMaxDimension(bvector<bvector<DPoint3d>> const &xyzA, double targetSize, bvector<bvector<DPoint3d>> &xyzB, DRange3d &rangeA, DRange3d &rangeB)
    {
    xyzB.clear();
    if (xyzA.size() == 0)
        return false;
    if (xyzA.front().size() == 0)
        return false;
    rangeA = DRange3d::From(xyzA);
    double sizeA = DoubleOps::Max(rangeA.XLength(), rangeA.YLength());
    auto scale = DoubleOps::ValidatedDivideParameter(targetSize, sizeA);
    if (!scale.IsValid ())
        return false;
    for (auto & xyzA1 : xyzA)
        {
        bvector<DPoint3d> xyzB1;
        for (auto &xyz : xyzA1)
            {
            xyzB1.push_back(scale.Value() * (xyz - rangeA.low));
            }
        xyzB.push_back(xyzB1);
        }
    rangeB = DRange3d::From(xyzB);
    return true;
    }

void ShowFaces (VuSpineContext &sc)
    {
    bvector<DPoint3d> xyzOut;
    sc.InitFaceScan();
    for (; sc.GetFace(xyzOut, true);)
        {
        Check::SaveTransformed(xyzOut);
        }
    }
// byEdge: true=>load only exposed edges
//         false=>load whole quad
void testPolygons(bvector<bvector<DPoint3d>> &loopsA, int mergeSelect, double minSplitRadians = 0.3)
    {
    bvector<bvector<DPoint3d>> loopsB;
    DRange3d rangeA, rangeB;
    ScaleToMaxDimension(loopsA, 10.0, loopsB, rangeA, rangeB);
    auto range = DRange3d::From(loopsB);
    double yShift = 1.2 * range.YLength();

    VuSpineContext sc;
    bvector<bvector<DPoint3d>> loopsC;
    bvector<DRay3d> symmetryRays;
    sc.InsertLoopsWithSymmetrySplits (loopsB, loopsC, symmetryRays);
    Check::SaveTransformed (loopsB);
    bool useParity = true;
    bool includeFan = false;
    double minDiagonalAngle = 1.0;
    sc.TriangulateForSpine(useParity, minSplitRadians, 2);
    Check::Shift(0, yShift, 0);
    ShowFaces(sc);
    sc.MarkBoxes(true, minDiagonalAngle, mergeSelect);
    Check::Shift(0, yShift, 0);
    ShowFaces(sc);

    bvector<bvector<DPoint3d>> edges;
    sc.GetSpineEdges(edges, true, true, includeFan);

    Check::Shift(0, yShift, 0);
    Check::SaveTransformed (loopsC);
    Check::SaveTransformed(edges);
    }

void testPolygons(bvector<DPoint3d> &loop, int mergeSelect)
    {
    bvector<bvector<DPoint3d>> loops;
    loops.push_back (loop);
    testPolygons (loops, mergeSelect);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu, SpineContext)
    {
    VuSpineContext sc;
    bvector<DPoint3d> xyz;
    xyz.push_back(DPoint3d::From(0, 0, 0));
    xyz.push_back(DPoint3d::From(10, 0, 0));
    xyz.push_back(DPoint3d::From(20, 0, 0));
    xyz.push_back(DPoint3d::From(20, 2, 0));
    xyz.push_back(DPoint3d::From(10, 2, 0));
    xyz.push_back(DPoint3d::From(5, 2, 0));
    xyz.push_back(DPoint3d::From(5, 5, 0));
    xyz.push_back(DPoint3d::From(4, 5, 0));
    xyz.push_back(DPoint3d::From(4, 2, 0));
    xyz.push_back(DPoint3d::From(0, 2, 0));
    xyz.push_back(DPoint3d::From(0, 0, 0));

    sc.InsertEdges(xyz, true);
    bool useParity = true;
    double minSplitRadians = 0.3;
    double minDiagonalAngle = 1.0;
    sc.TriangulateForSpine(useParity, minSplitRadians);
    sc.MarkBoxes(true, minDiagonalAngle);
    bvector<bvector<DPoint3d>> edges;
    sc.GetSpineEdges(edges);
    // For Tshape where one bar has intermediate xyz (at (10,0) and (10,2))
    //           D
    //           D
    //           D
    //   AAAAAAAAJBBBBBBBCCCCCCC
    // We expect
    //   3 dead end quads (in A,C, D)
    //   3 segments at the other side of the dead end quads (midA to J, midC to B, midD to J)
    //   1 simple quad bisector (B, from J to C)
    //   3 fan edges in the joint (mid J A, mid J to D, midJ to B)
    Check::Size(10, edges.size());
    Check::SaveTransformed(xyz);
    Check::SaveTransformed(edges);
    Check::ClearGeometry("Vu.SpineContext");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu, SpineContextB)
    {
    VuSpineContext sc;
    int diagonalSelect = 1;
    bvector<DPoint3d> xyz;
    xyz.push_back(DPoint3d::From(0, 0, 0));
    xyz.push_back(DPoint3d::From(10, 0, 0));
    xyz.push_back(DPoint3d::From(20, 0, 0));
    xyz.push_back(DPoint3d::From(20, 2, 0));
    xyz.push_back(DPoint3d::From(10, 2, 0));
    xyz.push_back(DPoint3d::From(5, 2, 0));
    xyz.push_back(DPoint3d::From(5, 5, 0));
    xyz.push_back(DPoint3d::From(4, 5, 0));
    xyz.push_back(DPoint3d::From(4, 2, 0));
    xyz.push_back(DPoint3d::From(0, 2, 0));
    xyz.push_back(DPoint3d::From(0, 0, 0));

    auto cv0 = CurveVector::CreateLinear(xyz, CurveVector::BOUNDARY_TYPE_Outer);

    auto cv1 = CurveVector::CreateLinear(
        bvector<DPoint3d>
        {
        DPoint3d::From(15, -5, 0),
            DPoint3d::From(18, -5, 0),
            DPoint3d::From(18, 12, 0),
            DPoint3d::From(10, 20, 0),
            DPoint3d::From(9, 19, 0),
            DPoint3d::From(17, 11, 0),
            DPoint3d::From(15, -5, 0)
        },
        CurveVector::BOUNDARY_TYPE_Outer);

    auto cv2 = CurveVector::AreaUnion(*cv0, *cv1);
    bvector<bvector<DPoint3d>>strokes;
    cv2->CollectLinearGeometry(strokes);
    for (auto &loop : strokes)
        sc.InsertEdges(loop, true);
    bool useParity = true;
    double minSplitRadians = 0.3;
    double minDiagonalAngle = 1.0;
    sc.TriangulateForSpine(useParity, minSplitRadians, diagonalSelect);
    sc.MarkBoxes(true, minDiagonalAngle);
    bvector<bvector<DPoint3d>> edges;
    sc.GetSpineEdges(edges);

    Check::SaveTransformed(*cv2);
    Check::SaveTransformed(edges);

    Check::ClearGeometry("Vu.SpineContextB");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu, SpineContextC)
    {
    for (double f0 : bvector<double>{ 1.0, 0.8, 0.2 })
        {
        SaveAndRestoreCheckTransform shifterA(30, 0, 0);
        for (size_t n = 3; n < 12; n += 2)
            {
            SaveAndRestoreCheckTransform shifterB(0, 50, 0);
            auto centralPolygon = CurveVector::CreateRegularPolygonXY(DPoint3d::From(0, 0, 0), 10.0, (int)n, true);
            bvector<bvector<DPoint3d>> centralStrokes;
            centralPolygon->CollectLinearGeometry(centralStrokes);
            bvector<DPoint3d> xyz;
            if (centralStrokes.size() == 1)
                {
                auto &loop = centralStrokes.front();
                double a = 4.0;
                for (size_t i = 0; i + 1 < loop.size(); i++)
                    {
                    auto xyz0 = loop[i];
                    auto xyz1 = loop[i + 1];
                    auto delta = xyz1 - xyz0;
                    DVec3d perp;
                    perp.UnitPerpendicularXY(delta);
                    perp.Negate();
                    auto xyz0a = xyz0 + f0 * a * perp;
                    auto xyz1a = xyz1 + a * perp;
                    xyz.push_back(xyz0);
                    xyz.push_back(xyz0a);
                    xyz.push_back(xyz1a);
                    xyz.push_back(xyz1);
                    a += 3.0;
                    }
                }
#ifdef DirectTestC
            VuSpineContext sc;
            sc.InsertEdges(xyz, true);
            bool useParity = true;
            double minSplitRadians = 0.3;
            double minDiagonalAngle = 1.0;
            sc.TriangulateForSpine(useParity, minSplitRadians);
            sc.MarkBoxes(true, minDiagonalAngle);
            bvector<bvector<DPoint3d>> edges;
            sc.GetSpineEdges(edges);
            Check::SaveTransformed(xyz);
            Check::SaveTransformed(edges);
            Check::Shift(80, 0, 0);
#else
            bvector<bvector<DPoint3d>> loops;
            loops.push_back(xyz);
            for (int mergeSelect : {0, 1})
                {
                SaveAndRestoreCheckTransform shiftX (10, 0);
                testPolygons(loops, mergeSelect);
                }
#endif
            }
        }
    Check::ClearGeometry("Vu.SpineContextC");
    }

/*---------------------------------------------------------------------------------**//**
* Make spines for bridge pier with varying (a) skew and (b) fillet stroking.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu, SpineContextBridgePier)
    {
    double dxTotal = 10.0;
    double dyTotal = 14.0;
    double dxBar = 3.0;
    double dyBar = 2.0;
    double dxFillet = 1.0;
    int diagonalSelect = 1;
    // double markerSize = 0.15;
    auto options = IFacetOptions::CreateForCurves();
    for (double skew : {0.0, 0.1, 0.3, 0.5})
        {
        SaveAndRestoreCheckTransform shifter1(15.0 * dxTotal, 0, 0);
        for (double f : bvector<double>{ 1.0, 2.0, 0.5, 4.0 })
            {
            double dyFillet = f * dxFillet;
            auto section = CreateFilletedSymmetricT(dxTotal, dyTotal, dxBar, dyBar, dxFillet, dyFillet, false);
            auto sheer = Transform::From(RotMatrix::FromRowValues
            (
                1, 0, 0,
                skew, 1, 0,
                0, 0, 1
            ));
            section->TransformInPlace(sheer);
            SaveAndRestoreCheckTransform shifter2(2.0 * dxTotal, 0, 0);
            //for (double maxEdgeLength : bvector<double> {1000.0, 5.0, 2.0, 1.0, 0.5})
            double maxEdgeLength = 20.0;
            for (double strokeDegrees : bvector<double>{ 90,45,30,20 })
                {
                SaveAndRestoreCheckTransform shifter(0, 2.0 * dyTotal, 0);
                bvector<DPoint3d> xyz;
                options->SetMaxEdgeLength(maxEdgeLength);
                options->SetAngleTolerance(Angle::DegreesToRadians(strokeDegrees));
                section->AddStrokePoints(xyz, *options);
                {
                // Check::SaveTransformedMarkers (xyz, markerSize);
                VuSpineContext sc;
                sc.InsertEdges(xyz, true);
                bool useParity = true;
                double minSplitRadians = 0.3;
                double minDiagonalAngle = 1.0;
                sc.TriangulateForSpine(useParity, minSplitRadians);
                sc.MarkBoxes(true, minDiagonalAngle, diagonalSelect);
                bvector<bvector<DPoint3d>> edges;
                sc.GetSpineEdges(edges);
                Check::SaveTransformed(xyz);
                Check::SaveTransformed(edges);
                Check::Shift(80, 0, 0);
                }
                }
            }
        }
    Check::ClearGeometry("Vu.SpineContextBridgePier");
    }

void MoveAndScale(bvector<DPoint3d> &points, DPoint3dCR origin, double scale)
    {
    for (auto &xyz : points)
        {
        xyz.x = (xyz.x - origin.x) * scale;
        xyz.y = (xyz.y - origin.y) * scale;
        xyz.z = (xyz.z - origin.z) * scale;
        }
    }

bool isActive(int i, int numBlock)
    {
    return i == 0 || i + 1 == numBlock;
    }
bool isActiveBlock(int ix, int iy, int numXBlock, int numYBlock)
    {
    if (ix >= 0 && ix < numXBlock
        && iy >= 0 && iy < numYBlock)
        return isActive (ix, numXBlock) || isActive (iy, numYBlock);
    return false;
    }

// byEdge: true=>load only exposed edges
//         false=>load whole quad
void testBlocks(int inputSelect, int mergeSelect)
    {
    int diagonalSelect = 1; // for these tests, everything is squared up so diagonal selects have same effect.
    for (double ax : {1.0, 4.0})
        {
        SaveAndRestoreCheckTransform shifter0(0, 40, 0);
        for (int numXBlock : {2, 4, 1})
            {
            for (int numYBlock : { 1, 3})
                {
                bvector<bvector<DPoint3d>> quads;
                bvector<bvector<DPoint3d>> freeEdges;
                for (int iy = 0; iy < numYBlock; iy++)
                    {
                    double y0 = iy;
                    double y1 = y0 + 1;
                    double ym = 0.5 * (y0 + y1);
                    for (int ix = 0; ix < numXBlock; ix++)
                        {
                        double x0 = ix * ax;
                        double x1 = x0 + ax;
                        double xm = 0.5 * (x0 + x1);
                        if (isActiveBlock(ix, iy, numXBlock, numYBlock))
                            {
                            quads.push_back(bvector<DPoint3d>());
                            quads.back().push_back(DPoint3d::From(x0, y0));
                            if (inputSelect == 2)
                                quads.back().push_back(DPoint3d::From(xm, y0));
                            quads.back().push_back(DPoint3d::From(x1, y0));
                            if (inputSelect == 2)
                                quads.back().push_back(DPoint3d::From(x1, ym));
                            quads.back().push_back(DPoint3d::From(x1, y1));
                            if (inputSelect == 2)
                                quads.back().push_back(DPoint3d::From(xm, y1));
                            quads.back().push_back(DPoint3d::From(x0, y1));
                            if (inputSelect == 2)
                                quads.back().push_back(DPoint3d::From(x0, ym));
                            quads.back().push_back(DPoint3d::From(x0, y0));
                            if (!isActiveBlock(ix, iy + 1, numXBlock, numYBlock))
                                freeEdges.push_back({ DPoint3d::From(x0,y1), DPoint3d::From(x1,y1) });
                            if (!isActiveBlock(ix, iy - 1, numXBlock, numYBlock))
                                freeEdges.push_back({ DPoint3d::From(x1,y0), DPoint3d::From(x0,y0) });

                            if (!isActiveBlock(ix + 1, iy, numXBlock, numYBlock))
                                freeEdges.push_back({ DPoint3d::From(x1,y0), DPoint3d::From(x1,y1) });
                            if (!isActiveBlock(ix - 1, iy, numXBlock, numYBlock))
                                freeEdges.push_back({ DPoint3d::From(x0,y1), DPoint3d::From(x0,y0) });
                            }
                        }
                    }
                double bx = numXBlock * ax + 10.0;
                for (bool includeFan : { false, true})
                    {
                    SaveAndRestoreCheckTransform shifter1(bx, 0, 0);
                    VuSpineContext sc;
                    if (inputSelect == 0)
                        {
                        for (auto &edge : freeEdges)
                            {
                            sc.InsertEdges(edge, false);
                            Check::SaveTransformed(edge);
                            }
                        }
                    else if (inputSelect > 1)
                        {
                        for (auto &loop : quads)
                            {
                            sc.InsertEdges(loop, true);
                            Check::SaveTransformed(loop);
                            }
                        }

                    bool useParity = true;
                    double minSplitRadians = 0.3;
                    double minDiagonalAngle = 1.0;
                    sc.TriangulateForSpine(useParity, minSplitRadians, mergeSelect);
                    sc.MarkBoxes(true, minDiagonalAngle, diagonalSelect);
                    bvector<bvector<DPoint3d>> edges;
                    sc.GetSpineEdges(edges, true, true, includeFan);

                    Check::Shift(0, 5, 0);
                    bvector<DPoint3d> xyzOut;
                    sc.InitFaceScan();
                    for (; sc.GetFace(xyzOut, true);)
                        {
                        Check::SaveTransformed(xyzOut);
                        }
                    Check::Shift(0, 5, 0);
                    Check::SaveTransformed(edges);
                    }
                Check::Shift(bx, 0, 0);
                }
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu, SpineContextQuads)
    {
    testBlocks (0, 0);
    Check::ClearGeometry("Vu.SpineContextQuadsByEdge");
    testBlocks(1, 0);
    Check::Shift (0,100,0);
    testBlocks(1, 1);
    Check::Shift(0, 100, 0);
    testBlocks(1, 2);
    Check::ClearGeometry("Vu.SpineContextQuadsByBlock");

    testBlocks(2, 0);
    Check::Shift(0, 100, 0);
    testBlocks(2, 1);
    Check::Shift(0, 100, 0);
    testBlocks(2, 2);
    Check::ClearGeometry("Vu.SpineContextQuadsByBlockWithMidsides");
    }



static bvector<bvector<DPoint3d>> chiseledWasher{
    {
        {3.06162e-17, 0.5},
        {-0.353553, 0.353553},
        {-0.5, 0},
        {-0.353553, -0.353553},
        {-3.06162e-17, -0.5},
        {0.353553, -0.353553},
        {0.5, 0},
        {0.353553, 0.353553},
        {3.06162e-17, 0.5}
        },
    {
        {-0.282843, 0.282843},
        {2.44929e-17, 0.4},
        {0.282843, 0.282843},
        {0.4, 0},
        {0.282843, -0.282843},
        {-2.44929e-17, -0.4},
        {-0.282843, -0.282843},
        {-0.4, 0},
        {-0.282843, 0.282843}
    }
};

static bvector<bvector<DPoint3d>> beamSection {
    {
        {0.5334, 0}, 
        { -0.5334,0 },
        { -0.5334,-0.127 },
        { -0.1016,-0.3048 },
        { -0.1016,-1.3716 },
        { -0.3556,-1.6256 },
        { -0.3556,-1.8288 },
        { 0.3556,-1.8288 },
        { 0.3556,-1.6256 },
        { 0.1016,-1.3716 },
        { 0.1016,-0.3048 },
        { 0.5334,-0.127 },
        {0.5334, 0}
    }
};
static bvector<bvector<DPoint3d>> beamSectionT {
    {
        {0.5334, 0}, 
        { -0.5334,0 },
        { -0.5334,-0.127 },
        { -0.1016,-0.3048 },
        { -0.1016,-1.3048 },
        { 0.1016,-1.3048 },
        { 0.1016,-0.3048 },
        { 0.5334,-0.127 }   // let closure happen !!
    }
};

static bvector<bvector<DPoint3d>> beamSection1 {
    {
    {0.4572, 0},
    {-0.4572, 0}, 
    {-0.4572, -0.0889}, 
    {-0.0889, -0.1905}, 
    {-0.0889, -0.7493}, 
    {-0.4064, -0.94615},
    {-0.4064, -1.1684}, 
    {0.4064, -1.1684}, 
    {0.4064, -0.94615}, 
    {0.0889, -0.7493}, 
    {0.0889, -0.1905}, 
    {0.4572, -0.0889}       // let closure happen in the context !!!
    }
};

bvector<bvector<DPoint3d>> bridgeSection {
{
        {-932812.01962387562, 8115444.8778925976, 0.0},
        {407263.05805396568, 8164367.8014595471, 0.0},
        {1678867.1463614060, 8007814.4460453093, 0.0},
        {2168569.4166553505, 6109582.1357097412, 0.0},
        {2167945.6418642672, 5825852.0549593698, 0.0},
        {6618559.9509403110, 5816067.4702459797, 0.0},
        {6540307.3916598521, 6119389.5963610644, 0.0},
        {7009822.7473426005, 8135014.0473193768, 0.0},
        {8436084.7003240343, 8410408.6539029311, 0.0},
        {9674020.4573360179, 8439099.9723561425, 0.0},
        {9670409.7628781702, 8594889.5288486984, 0.0},
        {-883904.17007358931, 8350274.9110139534, 0.0},
        {-932812.01962387562, 8115444.8778925976, 0.0}
},
{
        {2128819.3622240396,7812122.7517775111,0.0},
        {2295106.0506950123,8017599.0307586985,0.0},
        {3419986.5903515946,8271998.2333068335,0.0},
        {5229577.0237121824,8281782.8180202227,0.0},
        {6413146.9828291088,8144798.6320327669,0.0},
        {6598996.8111201953,7939322.3530515786,0.0},
        {6227297.1545380233,6217235.4434949644,0.0},
        {6090355.1757972203,6109605.0116476752,0.0},
        {2696150.4170073578,6090035.8422208950,0.0},
        {2480955.8789861002,6187881.6893547941,0.0},
        {2128819.3622240396,7812122.7517775111,0.0}
}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu, SpineSamples)
    {
        {
        SaveAndRestoreCheckTransform sy1(100, 0, 0);
        for (int mergeSelect : {0,1})
            {
            SaveAndRestoreCheckTransform sy2(0, 60, 0);
            for (double minSplitRadians : {0.1, 0.3, 0.4, 0.5, 1.0})
                {
                SaveAndRestoreCheckTransform sy3(15, 0, 0);
                testPolygons(chiseledWasher, mergeSelect, minSplitRadians);
                }
            }
        }

    for (auto &section : {beamSection, beamSection1, beamSectionT})
        {
        SaveAndRestoreCheckTransform sy4(100, 0, 0);
        for (double yScale : {1.0, 0.5, 0.8, 1.0, 1.2, 1.5})
            {
            SaveAndRestoreCheckTransform sy5(0, 200, 0);
            for (int mergeSelect : {0, 1})
                {
                SaveAndRestoreCheckTransform sy6(0, 60, 0);
                for (double minSplitRadians : {0.3, 0.4, 0.5})
                    {
                    SaveAndRestoreCheckTransform sy7(15, 0, 0);
                    auto beamSection2 = section;
                    for (auto & loop : beamSection2)
                        for (auto &xyz : loop)
                            xyz.y *= yScale;
                    testPolygons(beamSection2, mergeSelect, minSplitRadians);
                    }
                }
            }
        }

    for (double yScale = 0.4; yScale < 2.0; yScale *= 1.25)
        {
        int mergeSelect = 1;
        double minSplitRadians = 0.4;
        SaveAndRestoreCheckTransform sx8(20, 0, 0);
        auto section = beamSection;
        for (auto & loop : section)
            for (auto &xyz : loop)
                xyz.y *= yScale;
        testPolygons(section, mergeSelect, minSplitRadians);
        }

        {
        SaveAndRestoreCheckTransform sy9(100, 0, 0);
        for (int mergeSelect : {0, 1})
            {
            SaveAndRestoreCheckTransform sy10(0, 60, 0);
            for (double minSplitRadians : {0.1, 0.3, 0.4, 0.5, 1.0})
                {
                SaveAndRestoreCheckTransform sy3(15, 0, 0);
                testPolygons(bridgeSection, mergeSelect, minSplitRadians);
                }
            }
        }
    Check::ClearGeometry("Vu.SpineContextSamples");
    }

TEST(Vu, SpineSymmetryTeaser)
    {

    int mergeSelect = 1;
    double minSplitRadians = 0.4;

    testPolygons(beamSectionT, mergeSelect, minSplitRadians);
    Check::Shift (10,0,0);
    for (double yScale = 0.4; yScale < 2.0; yScale *= 1.25)
        {
        SaveAndRestoreCheckTransform sx1(20, 0, 0);
        auto section = beamSection;
        for (auto & loop : section)
            for (auto &xyz : loop)
                xyz.y *= yScale;
        testPolygons(section, mergeSelect, minSplitRadians);
        bvector<bvector<DPoint3d>> splitSection;
        }

    Check::ClearGeometry("Vu.SpineSymmetryTeaser");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu, SpineCorners)
    {
    double b = 5.0;
    double c = 0.5;
    for (double angleFactor : {0.8, 1.0, 1.2})
        {
        SaveAndRestoreCheckTransform shifterY(0, 40, 0);
        for (double degrees1 : {70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20})
            {
            SaveAndRestoreCheckTransform shifterX(12, 0, 0);
            double theta1 = Angle::DegreesToRadians(degrees1);
            double theta2 = Angle::DegreesToRadians (degrees1 * angleFactor);
            double theta3 = theta1 + theta2;
            double ax = c * cos (theta1);
            double ay = c * sin (theta1);
            DVec3d vectorA = DVec3d::From (ax, ay);
            DVec3d vectorB = DVec3d::From (c * cos (theta3), c * sin(theta3));
            DVec3d vectorB1 = DVec3d::FromUnitPerpendicularXY (vectorB).Value ();
            DPoint3d origin = DPoint3d::From (0,0,0);
            bvector<DPoint3d> loopA;
            loopA.push_back (DPoint3d::From (0,0));
            loopA.push_back(DPoint3d::From(0, -b));
            loopA.push_back(DPoint3d::From(ax, -b));
            loopA.push_back(vectorA);
            loopA.push_back(DPoint3d::FromSumOf (vectorA, vectorB1, b + c * sin(theta2)));
            loopA.push_back(DPoint3d::FromSumOf(origin, vectorB1, b));
            loopA.push_back(DPoint3d::From(0, 0));
            bvector<bvector<DPoint3d>> loops;
            loops.push_back (loopA);
            testPolygons(loops, 0);
            }
        }
    Check::ClearGeometry("Vu.SpineContextCorners");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu, SpineQuads)
    {
    bvector<DPoint3d> xyRight;
    double ay0 = 0.2;
    double ay1 = 0.3;
    double ax = 1.1;
    double y = 0.0;
    for (double degrees = 45.0; degrees < 89.5; degrees += 5.0)
        {
        double x = ax - cos(Angle::DegreesToRadians(degrees));
        xyRight.push_back (DPoint3d::From (x,y));
        y += ay0;
        xyRight.push_back(DPoint3d::From (x, y));
        y += ay1;
        }
        // xyRight has points "to the right" of a path that moves to the right as it moves up in the 1st quadrant.
        // xyzSymmetric has corresponding reversed-order points coming down on the left.
        // xyzAxis has corresponding points directly on the y axis.
    auto xyzSymmetric = xyRight;
    auto xyzAxis = xyRight;
    auto numRight = xyRight.size ();
    for (size_t i = numRight; i-- > 0;)
        {
        DPoint3d xy = xyRight[i];
        xyzSymmetric.push_back (DPoint3d::From (-xy.x, xy.y));
        xyzAxis.push_back (DPoint3d::From (0, xy.y));
        }
    xyzSymmetric.push_back (xyRight[0]);
    xyzAxis.push_back(xyRight[0]);

    for (int mergeSelect : {0,1})
        {
        SaveAndRestoreCheckTransform shifter (10,0,0);
        testPolygons(xyzSymmetric, mergeSelect);
        }
    Check::Shift (10,0,0);
    // Make spines to axis with xyRight moving outward from test to test.
    for (double dx : {0.0, 0.5, 1.0, 2.0})
        {
        auto xyz = xyzAxis;
        for (auto k = 0; k < numRight;k++)
            xyz[k].x += dx;
        xyz.back () = xyz.front ();
        for (int mergeSelect : {0, 1})
            {
            SaveAndRestoreCheckTransform shifter(10, 0, 0);
            testPolygons(xyz, mergeSelect);
            }
        Check::Shift(10, 0, 0);
        }
    Check::ClearGeometry("Vu.SpineQuads");
    }
