#include "testHarness.h"


struct Stick
{
size_t m_bandIndex;
size_t m_masterEdge;
DPoint3d m_xyz0;
DPoint3d m_xyz1;

Stick
(
size_t splitIndex,
size_t loopIndex,
size_t masterEdge,
DPoint3dCR xyz0,
DPoint3dCR xyz1
)
    : m_bandIndex (splitIndex),
      m_masterEdge (masterEdge),
      m_xyz0 (xyz0),
      m_xyz1 (xyz1)
    {

    }
// Comparator for sorting to trapezoid regions
static bool cb_less_splitIndex_x01 (Stick const &dataA, Stick const &dataB)
    {
    if (dataA.m_bandIndex < dataB.m_bandIndex)
        return true;
    if (dataA.m_bandIndex > dataB.m_bandIndex)
        return false;
    if (dataA.m_xyz0.x < dataB.m_xyz0.x)
        return true;
    if (dataA.m_xyz0.x > dataB.m_xyz0.x)
        return false;
    if (dataA.m_xyz1.x < dataB.m_xyz1.x)
        return true;
    return false;
    }
};


struct TileBracket
{
size_t m_stick0, m_stick1;  // index into array of Stick
size_t m_masterEdge0, m_masterEdge1;  // edge index in master loops
size_t m_band;
TileBracket (size_t stick0, size_t stick1, size_t masterEdge0, size_t masterEdge1, size_t band)
    : m_stick0 (stick0), m_stick1 (stick1),
    m_masterEdge0 (masterEdge0), m_masterEdge1 (masterEdge1),
    m_band (band)
    {
    }

static bool cb_sort_masterEdges_band (TileBracket const &pairA, TileBracket const &pairB)
    {
    // left edge ...
    if (pairA.m_masterEdge0 < pairB.m_masterEdge0)
        return true;
    if (pairA.m_masterEdge0 > pairB.m_masterEdge0)
        return false;
#ifdef abc
    // right edge ..
    if (pairA.m_masterEdge1 < pairB.m_masterEdge1)
        return true;
    if (pairA.m_masterEdge1 > pairB.m_masterEdge1)
        return false;
#endif
    if (pairA.m_band < pairB.m_band)
        return true;
    return false;

    }
bool SameMasterEdges (TileBracket const &other) const 
    {
    return m_masterEdge0 == other.m_masterEdge0
        && m_masterEdge1 == other.m_masterEdge1;
    }
bool IsInNextBand (TileBracket const &other) const 
    {
    return m_band + 1 == other.m_band;
    }

};


void FixupLoops (bvector<bvector<DPoint3d>> const &loops, bvector<bvector<DPoint3d>> &outputTiles, bool addClosure, bool mergeStacks = true)
    {
    outputTiles.clear ();
    bvector<double> ySplit;
    bvector<Stick> sticks;
    // record all y values of points.
    for (auto &loop : loops)
        {
        for (auto &xyz : loop)
            ySplit.push_back (xyz.y);
        }
    if (ySplit.size () == 0)
        return;
    // sort and purge exact duplicates
    std::sort (ySplit.begin (), ySplit.end ());
    size_t numSplit = ySplit.size ();
    size_t lastAccepted = 0;
    for (size_t i = 1; i < numSplit; i++)
        {
        if (ySplit[i] != ySplit[lastAccepted])
            ySplit[++lastAccepted] = ySplit[i];
        }
    size_t numAccept = lastAccepted + 1;
    if (numAccept < numSplit)
        ySplit.resize (numAccept);
    size_t masterEdgeIndex = 0;
    // record portions of each stick split by the ySplit values ..
    for (size_t loopIndex = 0; loopIndex < loops.size (); loopIndex++)
        {
        bvector<DPoint3d> const &loop = loops[loopIndex];
        size_t n = loop.size ();
        if (n > 1)
            {
            for (size_t i0 = n - 1, i1 = 0; i1 < n; i0 = i1++, masterEdgeIndex++)
                {
                DPoint3d xyz0 = loop[i0];
                DPoint3d xyz1 = loop[i1];
                double y0 = xyz0.y;
                double y1 = xyz1.y;
                double yLow = y0;
                double yHigh = y1;
                size_t k0 = i0;
                size_t k1 = i1;
                if (yHigh < yLow)
                    {
                    yLow = y1;
                    yHigh = y0;
                    k0 = i1;
                    k1 = i0;
                    }

                double dy = y1 - y0;
                if (dy != 0.0)   // ensure safe divide
                    {
                    auto split = std::upper_bound (ySplit.begin (), ySplit.end (), yLow);
                    double yA = yLow;
                    double yB = yA;
                    DPoint3d xyzA = loop[k0];
                    DPoint3d xyzB = xyzA;
                    for (;split < ySplit.end () && *split <= yHigh; split++, xyzA = xyzB, yA = yB)
                        {
                        yB = *split;
                        if (yA < (*split))
                            {
                            double fB = (yB - y0 ) / dy;
                            xyzB = DPoint3d::FromInterpolate (xyz0, fB, xyz1);
                            sticks.push_back (Stick (
                                split - ySplit.begin (),
                                loopIndex,
                                masterEdgeIndex,
                                xyzA, xyzB));
                            }
                        }
                    }
                }
            }
        }

    std::sort (sticks.begin (), sticks.end (), Stick::cb_less_splitIndex_x01);

    bvector<TileBracket> tiles;
    for (size_t i0 = 0, n = sticks.size (); i0 < n; i0 += 2)   // sticks should appear in pairs at same break index
        {
        size_t i1 = i0 + 1;
        if (i1 < n && sticks[i0].m_bandIndex == sticks[i1].m_bandIndex)
            {
            tiles.push_back
                (
                TileBracket
                    (
                    i0, i1,
                    sticks[i0].m_masterEdge,
                    sticks[i1].m_masterEdge,
                    sticks[i0].m_bandIndex
                    )
                );
            }
        }
    std::sort (tiles.begin (), tiles.end (), TileBracket::cb_sort_masterEdges_band);
    SmallIntegerHistogram frequency (20);
    for (size_t i0 = 0, n = tiles.size (); i0 < n; i0++)
        {
        size_t i1 = i0;
        if (mergeStacks)
            {
            while (   i1 + 1 < n
                   && tiles[i1].SameMasterEdges(tiles[i1 + 1])
                   && tiles[i1].IsInNextBand (tiles[i1 + 1])
                   )
                {
                i1++;
                }
            }
        frequency.Record (i1 - i0);
        outputTiles.push_back (bvector<DPoint3d> ());
        outputTiles.back ().push_back (sticks[tiles[i0].m_stick0].m_xyz0);
        outputTiles.back ().push_back (sticks[tiles[i0].m_stick1].m_xyz0);
        outputTiles.back ().push_back (sticks[tiles[i1].m_stick1].m_xyz1);
        outputTiles.back ().push_back (sticks[tiles[i1].m_stick0].m_xyz1);
        if (addClosure)
            {
            auto xyz0 = outputTiles.back ().front ();
            outputTiles.back ().push_back (xyz0);
            }
        double compressionTolerance = Angle::MediumAngle ();
        DPoint3dOps::CompressCyclic (outputTiles.back (), compressionTolerance);
        i0 = i1;
        }
    }


void TestFixupLoops (bvector<bvector<DPoint3d>> &loops, bool expectAreaMatch, bool fixupStacks = true)
    {
    static int s_noisy = 0;
    auto range = DRange3d::From (loops);
    SaveAndRestoreCheckTransform shifter (2.0 * range.XLength (), 0,0);
    bvector<bvector<DPoint3d>> tiles;
    double area0 = 0.0, area1 = 0.0;
    for (auto &loop : loops)
        {
        area0 += PolygonOps::AreaXY (loop);
        if (s_noisy)
            Check::Print (loop, "LOOP");
        Check::SaveTransformed (loop);
        }
    Check::Shift (0, 1.1 * range.YLength (), 0.0);
    FixupLoops (loops, tiles, true, fixupStacks);
    for (auto &tile : tiles)
        {
        area1 += PolygonOps::AreaXY (tile);
        if (s_noisy)
            Check::Print (tile, "TILE");
        auto cv = CurveVector::CreateLinear (tile, CurveVector::BOUNDARY_TYPE_Outer);
        Check::SaveTransformed (*cv);
        }
    if (s_noisy)
        {
        Check::PrintIndent (2);
        Check::Print (area0, "loopsArea");
        Check::Print (area1, "tileArea");
        Check::Print (area1 - area0, "delta");
        if (!DoubleOps::AlmostEqual (fabs (area0), fabs (area1)))
            if (expectAreaMatch)
                Check::Print ("Area Mismatch");
            else
                Check::Print ("(this is ok -- these areas are not expected to match)");
        }
    if (expectAreaMatch)
        Check::Near (fabs (area0), fabs (area1), "tiled aream matches raw");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyfix,Test0)
    {
    Check::EnableOutputInScope outputControl (10);
    bvector<bvector<DPoint3d>> loops, tiles;
    loops.push_back (bvector<DPoint3d> ());
    loops.back ().push_back (DPoint3d::From (0,0));
    loops.back ().push_back (DPoint3d::From (1,1));
    loops.back ().push_back (DPoint3d::From (0,2));
    TestFixupLoops (loops, true);

    loops.back ().push_back (DPoint3d::From (-1,1.5));
    TestFixupLoops (loops, true);

    Check::ClearGeometry ("Polyfix.Test0");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyfix,SimpleHole)
    {
    // observed problem: hole whose sides both front on single sticks gets covered
    Check::EnableOutputInScope outputControl (10);
    bvector<bvector<DPoint3d>> loops, tiles;
    loops.push_back (bvector<DPoint3d> ());
    loops.back ().push_back (DPoint3d::From (0,0));
    loops.back ().push_back (DPoint3d::From (10,0));
    loops.back ().push_back (DPoint3d::From (10,20));
    loops.back ().push_back (DPoint3d::From ( 0,20));
    // construct hole in reverse order
    loops.push_back (bvector<DPoint3d> ());
    loops.back ().push_back (DPoint3d::From (2,5));
    loops.back ().push_back (DPoint3d::From (2,10));
    loops.back ().push_back (DPoint3d::From (7,10));
    loops.back ().push_back (DPoint3d::From (7,5));


    TestFixupLoops (loops, true);
    // now put one intermediate point on the outer
    loops.front ().push_back (DPoint3d::From (0,6));
    TestFixupLoops (loops, true);

    Check::ClearGeometry ("Polyfix.SimpleHole");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyfix,Arcs)
    {
    Check::EnableOutputInScope outputControl (10);
    auto arc = DEllipse3d::FromVectors (DPoint3d::From (1,2,0), DVec3d::From (1,-0.1,0), DVec3d::From (0,3,0), 0.0, Angle::TwoPi ());
    DPoint3dDoubleArrays strokes (arc, 15);
    bvector<bvector<DPoint3d>> loops;
    loops.push_back (strokes.m_xyz);

    TestFixupLoops (loops, true);

    loops.push_back (strokes.m_xyz);    // same points ...

    Transform::From (1.0,1,0).Multiply (loops.back (), loops.back ());  // This has overlap.
    TestFixupLoops (loops, false);

    Transform::From (5.0,1,0).Multiply (loops.back (), loops.back ());  // This has moved beyond the overlap.d
    TestFixupLoops (loops, true);

    Check::ClearGeometry ("Polyfix.Arcs");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyfix,SawTooth)
    {
    Check::EnableOutputInScope outputControl (10000);
    double dxB = 10.0;
    double dyB = 1.0;
    double dyA = 2.0;
    double dxA = 0.5;

    auto segment0 = DSegment3d::From (0,0,0, dxB,dyB,0);
    auto segment1 = DSegment3d::From (dxA,dyA,0,dxB+dxA, dyA+dyB,0);
    size_t numStrokes = 5;
    DPoint3dDoubleArrays stroke0 (segment0, numStrokes);
    DPoint3dDoubleArrays stroke1 (segment1, numStrokes);
    bvector<bvector<DPoint3d>> loops;
    loops.push_back (bvector<DPoint3d>());
    for (size_t i = 0; i < stroke0.m_xyz.size (); i++)
        {
        loops.back ().push_back (stroke0.m_xyz[i]);
        loops.back ().push_back (stroke1.m_xyz[i]);
        }
    DVec3d topShift = DVec3d::From (0,5,0);
    loops.back ().push_back (loops.back ().back () + topShift);
    loops.back ().push_back (loops.back ().front() + topShift);
    DRay3d ray = DRay3d::FromOriginAndVector (DPoint3d::From (dxB, 2.0,0), DVec3d::UnitZ ());
    for (double degrees : bvector<double> {0, 10.0, 40.0, 90.0, 170.0, -20, -200})
        {
        auto rotatedLoops = loops;
        Transform rotation = Transform::FromAxisAndRotationAngle (ray, Angle::DegreesToRadians (degrees));
        for (auto &loop : rotatedLoops)
            rotation.Multiply (loop, loop);
        TestFixupLoops (rotatedLoops, true, false);
        TestFixupLoops (rotatedLoops, true, true);
        }

    Check::ClearGeometry ("Polyfix.SawTooth");
    }

void AppendSteps (bvector<DPoint3d> &points, DVec3dCR vectorU, DVec3dCR vectorV, size_t numStep)
    {
    DPoint3d xyz = points.size () == 0 ? DPoint3d::From (0,0,0) : points.back ();
    for (size_t i = 0; i < numStep; i++)
        {
        xyz = xyz + vectorU;
        points.push_back (xyz);
        xyz = xyz + vectorV;
        points.push_back (xyz);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyfix,HorizontalSteps)
    {
    Check::EnableOutputInScope outputControl (10);
    bvector<bvector<DPoint3d>> loops;
    loops.push_back (bvector<DPoint3d>());
    loops.back ().push_back (DPoint3d::From (0,0,0));
    AppendSteps (loops.back (), DVec3d::From (1,0,0),  DVec3d::From (0,1,0),    5);
    AppendSteps (loops.back (), DVec3d::From (-1,0,0), DVec3d::From (0,-0.5,0), 5);

    for (double outerScale : bvector<double>{ 1.0, 8.0, 2.0})
        {
        SaveAndRestoreCheckTransform shifter (0, outerScale * 40.0, 0);
        DRay3d ray = DRay3d::FromOriginAndVector (DPoint3d::From (2, 2.0,0), DVec3d::UnitZ ());
        for (double degrees : bvector<double> {0, 10.0, 60.0, 90.0, 170.0, -20, -200})
            {
            auto rotatedLoops = loops;
            Transform rotation = Transform::FromAxisAndRotationAngle (ray, Angle::DegreesToRadians (degrees));
            for (auto &loop : rotatedLoops)
                rotation.Multiply (loop, loop);
            if (outerScale != 1.0)
                {
                DPoint3d center = DRange3d::From (rotatedLoops.front ()).LocalToGlobal (0.5, 0.5, 0.5);
                Transform scale = Transform::FromFixedPointAndScaleFactors (center, -outerScale, -outerScale, 0.0);
                rotatedLoops.push_back (bvector<DPoint3d> ());
                scale.Multiply (rotatedLoops.back (), rotatedLoops.front ());
                }
            TestFixupLoops (rotatedLoops, rotatedLoops.size () == 1, false);
            TestFixupLoops (rotatedLoops, rotatedLoops.size () == 1, true);
            }
        }
    Check::ClearGeometry ("Polyfix.HorizontalSteps");
    }
void SaveShapes (bvector<bvector<DPoint2d>> &loops)
    {
    for (auto &loop : loops)
        {
        bvector<DPoint3d> loop3d;
        for (auto &xy : loop)
            loop3d.push_back (DPoint3d::From (xy));
        auto shape = CurveVector::CreateLinear (loop3d, CurveVector::BOUNDARY_TYPE_Outer);
        Check::SaveTransformed (*shape);
        }
    }
TEST(Polyfix,vu_fixupLoopParity)
    {
    bvector<bvector<DPoint2d>> inputLoops;

    inputLoops.push_back (bvector<DPoint2d> ());
    SampleGeometryCreator::StrokeUnitCircle (inputLoops.back (), 6, 3.0);


    inputLoops.push_back (bvector<DPoint2d> ());
    SampleGeometryCreator::StrokeUnitCircle (inputLoops.back (), 6, 5.0);

    auto shift1 = Transform::From (1,3,0);
    shift1.Multiply (inputLoops.back (), inputLoops.back ());

    bvector<bvector<DPoint2d>> outputLoops;
    vu_fixupLoopParity (outputLoops, inputLoops, 3, 1);
    SaveShapes (inputLoops);
    Check::Shift (0,10,0);
    SaveShapes (outputLoops);
    Check::ClearGeometry ("Polyfix.vu_fixupLoopParity");
    }
