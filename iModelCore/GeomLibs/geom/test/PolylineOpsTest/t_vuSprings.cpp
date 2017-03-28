#include "testHarness.h"

typedef VuSpringModel<uint64_t> BCSSpringModel;


void SaveZones (bvector<DPoint3d> &wall, BCSSpringModel &sm)
    {
    Check::ShiftToLowerRight (10.0);
    Check::SaveTransformed (wall);
    bvector<BCSSpringModel::StationPolygon> zones;
    sm.CollectStationAreas (zones, false, 0.01, 0.10);
    for (auto &zone : zones)
        Check::SaveTransformed (zone.m_xyz);
    Check::ShiftToLowerRight (10.0);
    sm.CollectStationAreas (zones, true, 0.01, 0.10);
    for (auto &zone : zones)
        Check::SaveTransformed (zone.m_xyz);

    }

TEST(BCS,SpringModelA)
    {
    BCSSpringModel sm;

    auto wall = bvector<DPoint3d>
        {
        DPoint3d::From (0,0),
        DPoint3d::From (20,0),
        DPoint3d::From (20,10),
        DPoint3d::From ( 0,10),
        DPoint3d::From (0,0)
        };

    sm.AddWall (wall, 10.0);
    sm.AddStation (DPoint3d::From (8,5),  20.0);  // big blob
    sm.AddStation (DPoint3d::From (12,5), 1.0); // small blob

    Check::SaveTransformed (wall);
    sm.SolveSprings ();
    SaveZones (wall, sm);
    Check::SaveTransformed (wall);
    Check::ClearGeometry ("BCS.SpringModelA");
    }

TEST(BCS,SpringModelB)
    {
    BCSSpringModel sm;
    auto wall = bvector<DPoint3d>
        {
        DPoint3d::From (0,0),
        DPoint3d::From (80,0),
        DPoint3d::From (80,30),
        DPoint3d::From (30,30),
        DPoint3d::From (30,35),
        DPoint3d::From (50,35),
        DPoint3d::From (50,53),
        DPoint3d::From (0,53),
        DPoint3d::From (0,30),
        DPoint3d::From (10,30),
        DPoint3d::From (10,23),
        DPoint3d::From (0,23),
        DPoint3d::From (0,0)
        };

    Check::SaveTransformed (wall);


    sm.AddWall (wall, 10.0);

    // Along lower wall
    sm.AddStation (DPoint3d::From (10,5), 15.0);
    sm.AddStation (DPoint3d::From (25,5), 5.0);
    sm.AddStation (DPoint3d::From (35,5), 5.0);
    sm.AddStation (DPoint3d::From (45,5), 5.0);
    sm.AddStation (DPoint3d::From (55,5), 5.0);
    sm.AddStation (DPoint3d::From (65,5), 5.0);
    sm.AddStation (DPoint3d::From (75,5), 5.0);

    sm.AddStation (DPoint3d::From (15,20), 15); // lobby/reception
    sm.AddStation (DPoint3d::From (28,28), 7); // musuc lounge stage

    // below upper wall of large section
    sm.AddStation (DPoint3d::From (45,25), 5);
    sm.AddStation (DPoint3d::From (55,25), 5);
    sm.AddStation (DPoint3d::From (65,25), 5);
    sm.AddStation (DPoint3d::From (75,25), 5);
    sm.AddStation (DPoint3d::From (10,38), 15);  // restaurant

    sm.AddStation (DPoint3d::From (30,38), 3);  // upper alcove restrooms
    sm.AddStation (DPoint3d::From (35,38), 3);

    sm.AddStation (DPoint3d::From (40, 42), 5);    // upper alcove seating
    sm.AddStation (DPoint3d::From (30, 48), 5);
    sm.AddStation (DPoint3d::From (40, 48), 8);     // kitchen

    // Walkway. These need to be coupled
    sm.AddStation (DPoint3d::From (35,15), 8.0);
    sm.AddStation (DPoint3d::From (45,15), 8.0);
    sm.AddStation (DPoint3d::From (55,15), 8.0);
    sm.AddStation (DPoint3d::From (65,15), 8.0);

    sm.SolveSprings (true);
    Check::SaveTransformed (wall);
    SaveZones (wall, sm);
    Check::ClearGeometry ("BCS.SpringModelB");
    }




static bvector<bvector <DPoint3d>> s_testFloorPlanParityLoops
    {
    bvector<DPoint3d>
        {
        DPoint3d::From (0,0),
        DPoint3d::From (10,0),
        DPoint3d::From (10,10),
        DPoint3d::From (20,10),
        DPoint3d::From (20,30),
        DPoint3d::From (0,30),
        DPoint3d::From (0,0)
        },
    bvector<DPoint3d>
        {
        DPoint3d::From (5,5),
        DPoint3d::From (8,5),
        DPoint3d::From (8,11),
        DPoint3d::From (5,11),
        DPoint3d::From (5,5)
        }
    };

bvector<bvector <DPoint3d>> s_testFloorPlanOpenChains
    {
    bvector<DPoint3d>
        {
        DPoint3d::From (2,15),
        DPoint3d::From (11,15),
        DPoint3d::From (15,12)
        }
    };

bool TryLoadTestFloorPlan (GriddedSpaceManager &manager, double meshSize)
    {
    //bvector<DPoint3d> isolatedPoints;
    //bvector<double> uBreaks;
    //bvector<double> vBreaks;
    return manager.TryLoad (s_testFloorPlanParityLoops, s_testFloorPlanOpenChains);
    }

void TestGriddedSpaceManager (double meshSize, bool isoGrid, bool smoothGrid)
    {
    GriddedSpaceManager manager;
    GriddedSpaceQueries queries (manager);
    GriddedSpace_FloodFromSingleNode flooder (manager);
    double ax = 35.0;
    double ay = 35.0;
    SaveAndRestoreCheckTransform shifter (0, ay, 0);

    manager.SetMeshParams (meshSize, isoGrid, smoothGrid);
    if (TryLoadTestFloorPlan (manager, 1.0))
        {
        // output the raw grid ...
        TaggedPolygonVector polygons;
        VuOps::CollectLoopsWithMaskSummary (manager.Graph (), polygons, VU_EXTERIOR_EDGE, true);
        for (auto &loop : polygons)
            {
            if (!((int)loop.GetIndexA () & VU_EXTERIOR_EDGE))
                Check::SaveTransformed (loop.GetPointsCR ());
            }
        Check::Shift (ax, 0,0);
        bvector<int> spaceIds;
        bvector<double> baseAreas;
        // create spaces with minimal area ..
        baseAreas.push_back (20.0);
        spaceIds.push_back (manager.CreateSpace (DPoint3d::From (15,16,0), baseAreas.back ()));
        baseAreas.push_back (15.0);
        spaceIds.push_back (manager.CreateSpace (DPoint3d::From (10,21,0), baseAreas.back ()));

        baseAreas.push_back (20.0);
        spaceIds.push_back (manager.CreateSpace (DPoint3d::From (5,18,0), baseAreas.back ()));
        baseAreas.push_back (15.0);
        spaceIds.push_back (manager.CreateSpace (DPoint3d::From (5,24,0), baseAreas.back ()));


        // successively grow each space to larger and larger multiples of the original ....
        queries.SaveWalls ();
        double dz = 0.2;
        Check::Shift (0,0,10.0 * dz);
        queries.SaveSpaceBoundaries ();
        Check::Shift (0,0,-dz);     // smallest area comes out at top for downward code effect.
        bvector<double> spaceFactors {1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0};
        for (size_t k = 0; k < spaceFactors.size (); k++)
            {
            double spaceFactor = spaceFactors[k];
            for (size_t i = 0; i < baseAreas.size (); i++)
                {
                flooder.ExpandSingleSpaceIdToTargetArea (spaceIds[i], spaceFactor * baseAreas[i]);
                }
            queries.SaveSpaceBoundaries (k + 1 == spaceFactors.size ());
            Check::Shift (0,0,-dz);     // smallest area comes out at top for downward code effect.
            }
        }
    }

TEST(GriddedSpaceManager,VaryMeshSizeWithSquareGrid)
    {
    for (double meshSize : bvector<double>{1,2,3,4})
        TestGriddedSpaceManager (meshSize, false, false);
    Check::ClearGeometry ("GriddedSpaceManager.VaryMeshSizeWithSquareGrid");
    }

TEST(GriddedSpaceManager,VaryMeshSizeWithIsoGrid)
    {
    // Preshift to be friends with peers ...
    Check::Shift (100,0,0);
    for (double meshSize : bvector<double>{1,2,3,4})
        TestGriddedSpaceManager (meshSize, true, false);
    Check::ClearGeometry ("GriddedSpaceManager.VaryMeshSizeWithIsoGrid");
    }

TEST(GriddedSpaceManager,VaryMeshSizeWithSmoothGrid)
    {
    // Preshift to be friends with peers ...
    Check::Shift (200,0,0);
    for (double meshSize : bvector<double>{1,2,3,4})
        TestGriddedSpaceManager (meshSize, true, true);
    Check::ClearGeometry ("GriddedSpaceManager.VaryMeshSizeWithSmoothGrid");
    }




//---------------------------------------------------------------------------------------------------------------------------------

ValidatedDVec3d SinglePointAreaShift
(
DPoint3dCR xyz,
bvector<DPoint3d> const &neighborXYZ,
bvector<double> const &sectorArea,
bvector<double> const &sectorTargetArea
)
    {
// Let U and V be outbound vectors from A to neighbors B and C in an area.
//     C
//     |\R
//     |\
//     |  \
//     |   \
//     |    \
// W<--A-----B      (W is an arbitrary displacement vector from A)
//
// U = A-B
// R = C-B
// W is an additional vector of offset from A.
// The area is a(W) = R cross (U+W) = (R cross U + R cross W) / 2
// da(W) / dwx = -Ry/2
// da(W) / dwy = Rx/2
// Note that a second derivative wrt either var is identically 0.
// We are given a target area for each sector around A.
// Let EE(W) = sum of squared differences from the target = SUM (ee(W)) over all ee
//  ee(W) = ( a(w) -  targetArea) ^2
//  d ee(W) / dx = 2 da(W)/dx   * (a(w) - targetArea)
//  d ee(w) / dx dx = 2 [ da(W)/dx * da(w)/dx ]
//  d ee(w) / dx dy = 2 [ da(W)/dx * da(w)/dy ]
// etc.
// This is a 2-var newton raphson with analytic derivatives.
// No, its easier.  da(S) /dwx is a constant function.
// d ee(W)/dx is linear in W.
// So it solves in one step !!!
    BeAssert (neighborXYZ.size () == sectorArea.size ());
    BeAssert (neighborXYZ.size () == sectorTargetArea.size ());
    DVec3d F, dFdx, dFdy;
    F.Zero ();
    dFdx.Zero ();
    dFdy.Zero ();
    for (size_t i = 0, n = neighborXYZ.size (); i < n; i++)
        {
        size_t j = i + 1;
        if (j >= n)
            j = 0;
        double e0 = sectorArea[i] - sectorTargetArea[i];// Assume R cross U is included in current sectorArea !!!
        DVec3d R = neighborXYZ[j] - neighborXYZ[i];
        DVec3d dRcrossW = DVec3d::From (-0.5 * R.y, 0.5 * R.x );
        F = F + e0 * dRcrossW;
        dFdx.x += dRcrossW.x * dRcrossW.x;
        dFdx.y += dRcrossW.x * dRcrossW.y;
        dFdy.x += dRcrossW.y * dRcrossW.x;
        dFdy.y += dRcrossW.y * dRcrossW.y;
        }
    double dx, dy;
    if (bsiSVD_solve2x2 (&dx, &dy,
            dFdx.x, dFdx.y,
            dFdy.x, dFdy.y,
            F.x, F.y
            ))
        {
        return ValidatedDVec3d (DVec3d::From (dx, dy, 0.0), true);
        }
    return ValidatedDVec3d (DVec3d::From (0.0, 0.0, 0.0), true);
    }

// return each area[i] = additionalArea + area of triangle xyz0 to 2 neighbors.
void FillAreasFromNeighbors (bvector<double> &areas, DPoint3dCR xyz0, bvector<DPoint3d> &neighborXYZ, double additionalArea)
    {
    size_t n = neighborXYZ.size ();
    areas.clear ();
    for (size_t i = 0; i < n; i++)
        {
        size_t i1 = (i + 1) % n;
        areas.push_back (additionalArea + 0.5 * xyz0.CrossProductToPointsXY (neighborXYZ[i], neighborXYZ[i1]));
        }
    }

void RunSinglePointAreaShiftTest (
bvector<DPoint3d> &neighbors,
double spreadAreaFraction = 0.0,   // In area targets, this fraction of area0 is subtracted from area0 and equal parts added to others.
                        // This creates area distributions that do not have exact solution.
                        // (Expected values are smallish fractions == maybe up to .25 in extreme?)
double areaLossFactor = 0.0     // This fraction of the spreadArea is lost.
)
    {
    auto loop = neighbors;
    loop.push_back (neighbors.front ());
    bvector<DPoint3d> path;
    DPoint3d xyz0;
    xyz0.Zero ();
    
    double additionalArea = 0.0;
    double relaxationFactor = 1.0;
    SaveAndRestoreCheckTransform shifter (4.0, 0,0);
    Check::SaveTransformed (loop);
    // additionalArea = 0 ==> true diamond.
    // additionalArea = a ==> each quadrant has additional area a beyond its diagonal.
    bvector<double> sectorTargetArea, sectorArea;
    double b = 0.1;
    FillAreasFromNeighbors (sectorTargetArea, xyz0, neighbors, additionalArea);
    if (spreadAreaFraction != 0.0)
        {
        double shiftedArea = spreadAreaFraction * sectorTargetArea[0];
        sectorTargetArea[0] -= shiftedArea;
        double dA = shiftedArea * (1.0 - areaLossFactor) / (sectorTargetArea.size () - 1.0);
        for (size_t i = 1; i < sectorTargetArea.size (); i++)
            sectorTargetArea[i] +=dA;
        }

    // To acheive area A in a triangle with base b, the altitude is 2*A/b.
    // Draw parallel lines at target altitude.   Their failure to intersect makes the problem an optimization rather than exact solution.
    bvector<DSegment3d> segments;
    size_t n = sectorTargetArea.size ();
    static double s_displayFraction0 = 0.25;
    for (size_t i0 = 0; i0 < n; i0++)
        {
        size_t i1 = (i0 + 1) % n;
        DVec3d edgeVector = neighbors[i1] - neighbors[i0];
        double b = edgeVector.Normalize ();     // normalie in place, return prior length.
        double h = sectorTargetArea[i0] * 2.0 / b;
        DVec3d perpVector;
        perpVector.UnitPerpendicularXY (edgeVector);
        DPoint3d xyzA = DPoint3d::FromInterpolate (neighbors[i0], s_displayFraction0, neighbors[i1]);
        DPoint3d xyzB = DPoint3d::FromInterpolate (neighbors[i1], s_displayFraction0, neighbors[i0]);
        segments.push_back (DSegment3d::From (
                            xyzA + h * perpVector,
                            xyzB + h * perpVector
                            ));
        }
    Check::SaveTransformed (segments);

    static double s_expectedConvergenceFactor = 0.8;
    static double s_distanceTol = 1.0e-8;
    for (auto theta : bvector<Angle> {
                Angle::FromDegrees (0),
                Angle::FromDegrees (90),
                Angle::FromDegrees (45),
                Angle::FromDegrees (135.0),
                Angle::FromDegrees (108.0),
                Angle::FromDegrees (200.0),
                Angle::FromDegrees (270.0),
                Angle::FromDegrees (345.0),

                })
        {
        path.clear ();
        auto xyz = DPoint3d::From (b * theta.Cos (), b * theta.Sin (), 0.0);
        path.push_back (xyz);
#define IterativeCalls
#ifdef IterativeCalls
        double d0 = xyz0.Distance (xyz);
        double d1 = d0;
        double f = 1.5;
        size_t iteration = 0;
        for (iteration = 0; iteration < 8; iteration++)
            {
            FillAreasFromNeighbors (sectorArea, xyz, neighbors, additionalArea);
            auto delta = SinglePointAreaShift (xyz, neighbors, sectorArea, sectorTargetArea);
            if (!delta.IsValid ())
                break;
            xyz = xyz - delta * relaxationFactor;
            path.push_back (xyz);
            d1 = delta.Value ().Magnitude ();
            if (d1 < s_distanceTol)
                break;
            f *= s_expectedConvergenceFactor;
            }
        Check::True (d1 < f * d0, "Area shift converging?");
#else
    FillAreasFromNeighbors (sectorArea, xyz, neighbors, additionalArea);
    auto delta = SinglePointAreaShift (xyz, neighbors, sectorArea, sectorTargetArea);
    if (Check::True (delta.IsValid ()))
        {
        auto xyz1 = xyz - delta;
        Check::Near (xyz0, xyz1);
        path.push_back (xyz1);
        }
#endif
        Check::SaveTransformed (path);
        }
    }
TEST(SinglePointAreaShift,Diamond)
    {
    for (double shiftFactor : bvector<double> {0.0, 0.05, 0.10, -0.5, -0.10})
        {
        SaveAndRestoreCheckTransform shifter (0.0, 2.5,0.0);
        bvector<DPoint3d> neighbors
            {
            DPoint3d::From (1,0,0),
            DPoint3d::From (0,1,0),
            DPoint3d::From (-1,0,0),
            DPoint3d::From (0,-1,0),
            };
        RunSinglePointAreaShiftTest (neighbors, shiftFactor);
        neighbors[0].x += 0.25;
        RunSinglePointAreaShiftTest (neighbors, shiftFactor);
        neighbors[1].x -= 0.15;
        RunSinglePointAreaShiftTest (neighbors, shiftFactor);
        neighbors[2].y += 0.45;
        RunSinglePointAreaShiftTest (neighbors, shiftFactor);
        }
    Check::ClearGeometry ("SinglePointAreaShift.Diamond");
    }

TEST(SinglePointAreaShift,RegularNGon)
    {
    for (int numEdge : bvector<int> {3, 4, 5, 8})
        {
        SaveAndRestoreCheckTransform shifter (3.0, 0.0, 0.0);
        auto cv = CurveVector::CreateRegularPolygonXY (DPoint3d::From (0,0,0), 1.0, numEdge, true, CurveVector::BOUNDARY_TYPE_Outer);
        for (double shiftFactor : bvector<double> {0.0, 0.05, 0.10, -0.5, -0.10})
            {
            SaveAndRestoreCheckTransform shifter (0.0, 2.5,0.0);
            bvector<DPoint3d> points = *cv->at (0)->GetLineStringCP ();
            points.pop_back ();     // eliminate the closure point
            RunSinglePointAreaShiftTest (points, shiftFactor, 0.0);
            }
        }
    Check::ClearGeometry ("SinglePointAreaShift.RegularNgon");
    }

TEST(SinglePointAreaShift,RegularNGonAreaImbalance)
    {
    int numEdge = 5;
    for (double areaLossFactor : bvector<double> {0, -0.5, 0.5, 0.1, 0.2})
        {
        SaveAndRestoreCheckTransform shifter (3.0, 0.0, 0.0);
        auto cv = CurveVector::CreateRegularPolygonXY (DPoint3d::From (0,0,0), 1.0, numEdge, true, CurveVector::BOUNDARY_TYPE_Outer);
        for (double shiftFactor : bvector<double> {0.0, 0.05, 0.10, -0.5, -0.10})
            {
            SaveAndRestoreCheckTransform shifter (0.0, 2.5,0.0);
            bvector<DPoint3d> points = *cv->at (0)->GetLineStringCP ();
            points.pop_back ();     // eliminate the closure point
            RunSinglePointAreaShiftTest (points, shiftFactor, areaLossFactor);
            }
        }
    Check::ClearGeometry ("SinglePointAreaShift.RegularNGonAreaImbalance");
    }

struct BubblePhysics
{
// m_lambda = 0 is dry -- favors close packing, spheres distort to polyhedra.
// m_lambda = 1 is wet -- favors undeformed spheres.
double m_lambda;
double m_wetnessFactor; // (3.0 * m_wetnessfactor - 1)

BubblePhysics (double lambda)
    : m_lambda (lambda), m_wetnessFactor (3.0 * lambda - 1.0)
    {
    }

// nominal distance between bubbles of radius ri and rj with wetness factor lambda.
double Lij (double ri, double rj)
    {
    double a = ri * ri + rj * rj + m_wetnessFactor * ri * rj;
    return a >= 0.0? sqrt (a) : 0.0;
    }
// Return force vector between two bubble centers 
DVec3d Fij (DPoint3dCR Xi, DPoint3dCR Xj, double ri, double rj)
    {
    DVec3d delta = Xi - Xj;
    delta.Normalize ();
    return (1.0 - Lij (ri, rj)) * delta;
    }
};

//! Create a triangulation of points.
//! 
VuSetP CreateDelauney
(
bvector<DPoint3d> const points
)
    {
    if (points.size () < 3)
        return false;
    VuSetP graph = vu_newVuSet (0);
    DRange3d worldRange = DRange3d::From (points);
    double localAbsTol = 1.0e-8;
    auto localRange = DRange3d::From (-1,-1,-1,1,1,1);
    BentleyApi::Transform localToWorld, worldToLocal;

    if (!Transform::TryUniformScaleXYRangeFit (worldRange, localRange, worldToLocal, localToWorld))
        return nullptr;

    bvector<DPoint3d> localPoints;
    worldToLocal.Multiply (localPoints, points);

    // Trivial triangulation of the convex hull.
    bvector<DPoint3d> xyzHull (points.size () + 2);
    int numOut;
    bsiDPoint3dArray_convexHullXY (&xyzHull[0], &numOut, (DPoint3d*)&localPoints[0], (int)localPoints.size ());    
    xyzHull.resize (numOut);
    vu_addEdgesXYTol (graph, nullptr, xyzHull, true, localAbsTol, VU_BOUNDARY_EDGE, VU_BOUNDARY_EDGE);

    vu_mergeOrUnionLoops (graph, VUUNION_UNION);
    vu_regularizeGraph (graph);
    vu_parityFloodFromNegativeAreaFaces (graph, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);
    vu_splitMonotoneFacesToEdgeLimit (graph, 3);
    // final flip for true delauney condition . . .
    vu_flipTrianglesForIncircle (graph);
    
    vu_insertAndRetriangulate (graph, &localPoints[0], (int)localPoints.size (), false);
    // this should not be needed ... but retriangulate seems wrong..
    vu_flipTrianglesForIncircle (graph);

    PolyfaceHeaderPtr polyface = vu_toPolyface (graph, VU_EXTERIOR_EDGE);
    bvector<DPoint3d> &outputPoints = polyface->Point ();
    vu_transform (graph, &localToWorld);
    return graph;
    }

void AddPoints (bvector<DPoint3d> &points, DEllipse3dCR arc, size_t numEdge)
    {
    double df = 1.0 / (double)numEdge;
    for (size_t i = 0; i < numEdge; i++)
        points.push_back (arc.FractionToPoint (i * df));
    }


void OutputBisectorSplits (VuSetP graph)
    {
    bvector<DPoint3d> out;
    VU_SET_LOOP (nodeB, graph)
        {
        auto nodeA = nodeB->FSucc ();
        auto nodeC = nodeB->FPred ();
        auto xyzA = nodeA->GetXYZ ();
        auto xyzB = nodeB->GetXYZ ();
        auto xyzC = nodeC->GetXYZ ();
        ValidatedDPoint3d voronoiPoint;
        if (xyzB.CrossProductToPointsXY (xyzA, xyzC) > 0.0)
            {
            auto voronoiPoint = DPoint3d::FromIntersectPerpendicularsXY (xyzB, xyzA, 0.5, xyzC, 0.5);
            if (voronoiPoint.IsValid ())
                {
                out.clear ();
                out.push_back (DPoint3d::FromInterpolate (xyzB, 0.5, xyzA));
                out.push_back (voronoiPoint.Value ());
                out.push_back (DPoint3d::FromInterpolate (xyzB, 0.5, xyzC));
                Check::SaveTransformed (out);
                }            
            }
        }
    END_VU_SET_LOOP (nodeA, graph)
    }

DPoint3d GetPseudoCenter (VuP nodeA)
    {
    auto nodeB = nodeA->FSucc ();
    auto nodeC = nodeB->FSucc ();
    auto xyzA = nodeA->GetXYZ ();
    auto xyzB = nodeB->GetXYZ ();
    auto xyzC = nodeC->GetXYZ ();
    if (xyzA.CrossProductToPointsXY (xyzB, xyzC) > 0.0)
        {
        auto voronoiPoint = DPoint3d::FromIntersectPerpendicularsXY (xyzB, xyzA, 0.5, xyzC, 0.5);
        if (voronoiPoint.IsValid ())
            return voronoiPoint.Value ();
        }
    DVec3d vectorAB = xyzB - xyzA;
    DPoint3d xyzMid = DPoint3d::FromInterpolate (xyzA, 0.5, xyzB);
    DVec3d perp = DVec3d::FromCCWPerpendicularXY (vectorAB);
    return xyzMid + perp;
    }

void AppendPseudoCenters (VuP nodeA, bvector<DPoint3d> &points)
    {
    static double s_exteriorFactor = 2.0;
    auto nodeB = nodeA->FSucc ();
    auto nodeC = nodeB->FSucc ();
    auto xyzA = nodeA->GetXYZ ();
    auto xyzB = nodeB->GetXYZ ();
    auto xyzC = nodeC->GetXYZ ();
    if (xyzA.CrossProductToPointsXY (xyzB, xyzC) > 0.0)
        {
        auto voronoiPoint = DPoint3d::FromIntersectPerpendicularsXY (xyzB, xyzA, 0.5, xyzC, 0.5);
        if (voronoiPoint.IsValid ())
            points.push_back (voronoiPoint.Value ());
        }
    else
        {
        //  we appear to be on the outside ...
        // output a point some distance away on this edge's left bisector, and perhaps likewise at the FPred edge ....
        points.push_back (DPoint3d::FromInterpolateAndPerpendicularXY (xyzA, 0.5, xyzB, s_exteriorFactor));
        auto xyzD = nodeA->FPred ()->GetXYZ ();
        if (xyzD.CrossProductToPointsXY (xyzA, xyzB) <= 0.0)
            points.push_back (DPoint3d::FromInterpolateAndPerpendicularXY (xyzD, 0.5, xyzA, s_exteriorFactor));
        }
    }
void OutputCircumcenterChords (VuSetP graph)
    {
    bvector<DPoint3d> out;
    VU_SET_LOOP (nodeA, graph)
        {
        if (nodeA->GetId () > nodeA->EdgeMate ()->GetId ())
            {
            DPoint3d xyzA = GetPseudoCenter (nodeA);
            DPoint3d xyzB = GetPseudoCenter (nodeA->EdgeMate ());
            out.clear ();
            out.push_back (xyzA);
            out.push_back (xyzB);
            Check::SaveTransformed (out);
            }
        }
    END_VU_SET_LOOP (nodeA, graph)
    }

PolyfaceHeaderPtr CreateVoronoi (VuSetP graph)
    {
    _VuSet::TempMask visitMask (graph);
    auto facets = PolyfaceHeader::CreateVariableSizeIndexed ();
    VU_SET_LOOP (vertexSeed, graph)
        {
        vertexSeed->SetMaskAroundVertex (visitMask.Mask ());
        bvector<DPoint3d> points;
        VU_VERTEX_LOOP (sector, vertexSeed)
            {
            AppendPseudoCenters (sector, points);
            }
        END_VU_VERTEX_LOOP (sector, vertexSeed)
        DPoint3dOps::Compress (points, DoubleOps::SmallMetricDistance ());
        facets->AddPolygon (points);
        }
    END_VU_SET_LOOP (vertexSeed, graph)
    facets->Compress ();
    return facets;
    }

bool vu_createDelauneyAndVoronoi (bvector<DPoint3d> const &points, PolyfaceHeaderPtr &delauney, PolyfaceHeaderPtr &voronoi)
    {
    VuSetP graph = CreateDelauney (points);
    if (graph != nullptr)
        {
        delauney = vu_toPolyface (graph, VU_EXTERIOR_EDGE);
        voronoi = CreateVoronoi (graph);
        return true;
        }
    return false;
    }
TEST(Vu,CreateDelauney)
    {
    double dy = 50.0;
    bvector<DPoint3d> points;
    bvector<DEllipse3d> arcs 
        {
        DEllipse3d::From (0,0,0,    10,0,0,  0,10,0,   0, Angle::TwoPi ()),
        DEllipse3d::From (5,4,0,    11,0,0,  0,3,0,   0, Angle::TwoPi ()),
        DEllipse3d::From (1,3,0,   6,6,0,   -2,-3,0,  0, Angle::Pi ()),
        DEllipse3d::From (-2,1,0,   4,6,0,   -2,-3,0,  0, Angle::DegreesToRadians (75.0)),
        DEllipse3d::From (4,-1,0,   1,2,0,   -2,3,0,  0, Angle::DegreesToRadians (355.0))
        };

    bvector<size_t> edgeCount {7,9,12, 6, 11};

    for (size_t i = 0; i < arcs.size (); i++)
        {
        SaveAndRestoreCheckTransform shifter (80.0,0,0);
        AddPoints (points, arcs[i], i< edgeCount.size () ? edgeCount[i] : 9);
        Check::SaveTransformed (points);
        PolyfaceHeaderPtr delauney, voronoi;
        if (Check::True (vu_createDelauneyAndVoronoi (points, delauney, voronoi)))
            {
            Check::Shift (0,dy,0);
            Check::SaveTransformed (*delauney);
            Check::SaveTransformed (*voronoi);
            }
        }
    Check::ClearGeometry ("Vu.CreateDelauney");
    }


TEST(Vu,CreateDelauneySkew)
    {

    double dy = 30.0;
    double a = 1.0;
    size_t numX = 7;
    size_t numY = 5;
    for (double degrees : bvector<double> {60.0, 90.0, 80.0, 50.0, 40.0, 30.0, 100.0, 130.0})
        {
        Angle theta = Angle::FromDegrees (degrees);
        DPoint3dDVec3dDVec3d frame (0,0,0, a,0,0,  a * theta.Cos (), a * theta.Sin (), 0);
        bvector<DPoint3d> points;
        for (size_t j = 0; j <= numY; j++)
            for (size_t i = 0; i <= numX; i++)
                points.push_back (frame.Evaluate ((double) i, (double) j));

        Check::SaveTransformed (points);
        PolyfaceHeaderPtr delauney, voronoi;
        if (Check::True (vu_createDelauneyAndVoronoi (points, delauney, voronoi)))
            {
            Check::Shift (0,dy,0);
            Check::SaveTransformed (*delauney);
            Check::SaveTransformed (*voronoi);
            }
        Check::Shift (points.back ().x + 3.0 * a, 0, 0);
        }
    Check::ClearGeometry ("Vu.CreateDelauneySkew");
    }


TEST(Vu,CreateDelauneyCircle)
    {
    double dy = 30.0;
    bvector<DPoint3d> points;
    auto ellipse0 = DEllipse3d::From (0,0,0,    10,0,0,  0,10,0,   0, Angle::TwoPi ());
    AddPoints (points, ellipse0, 7);
    Check::SaveTransformed (points);
    Check::Shift (0,dy,0);
    auto delauney = CreateDelauney (points);
    if (delauney != nullptr)
        {
        PolyfaceHeaderPtr polyface = vu_toPolyface (delauney, VU_EXTERIOR_EDGE);
        Check::SaveTransformed (*polyface);
        //OutputCircumcenterChords (delauney);
        auto voronoi = CreateVoronoi (delauney);
        Check::SaveTransformed (*voronoi);
        }
    vu_freeVuSet (delauney);
    Check::ClearGeometry ("Vu.CreateDelauneyCircle");
    }

TEST(Vu,IncircleFlipProblem)
    {
    bvector<DPoint3d> points
        {
        DPoint3d::From (-78.00884047, 35.92836283), 
        DPoint3d::From (-84.52519939, 36.95442326),
        DPoint3d::From (-84.52519939, 31.04557674),
        DPoint3d::From (-81.43532935, 30.00000000) 
        };
    auto delauney = CreateDelauney (points);
    if (delauney != nullptr)
        {
        PolyfaceHeaderPtr polyface = vu_toPolyface (delauney, VU_EXTERIOR_EDGE);
        Check::SaveTransformed (*polyface);
        //Check::Shift (0,dy,0);
        //OutputCircumcenterChords (delauney);
        auto voronoi = CreateVoronoi (delauney);
        Check::SaveTransformed (*voronoi);
        }
    vu_freeVuSet (delauney);
    Check::ClearGeometry ("Vu.IncircleFlipProblem");
    
    }