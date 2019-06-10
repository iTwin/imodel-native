/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include "SampleGeometry.h"

typedef VuSpringModel<uint64_t> BCSSpringModel;


void SaveZones (bvector<DPoint3d> &wall, BCSSpringModel &sm)
    {
    auto range = sm.Range ();
    double ax = range.XLength () * 1.5;
    Check::Shift (ax, 0, 0);
    Check::SaveTransformed (wall);
    bvector<BCSSpringModel::StationPolygon> zones;
    sm.CollectStationAreas (zones, false, 0.01, 0.10);
    for (auto &zone : zones)
        Check::SaveTransformed (zone.m_xyz);
    Check::Shift (ax, 0, 0);
    sm.CollectStationAreas (zones, true, 0.01, 0.10);
    for (auto &zone : zones)
        Check::SaveTransformed (zone.m_xyz);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BCS,SpringModelA)
    {
    auto wall = bvector<DPoint3d>
        {
        DPoint3d::From (0,0),
        DPoint3d::From (20,0),
        DPoint3d::From (20,10),
        DPoint3d::From ( 0,10),
        DPoint3d::From (0,0)
        };
    double ay = 100.0;
    for (bool doProjection : bvector<bool> {/*false, */true})
        {
        SaveAndRestoreCheckTransform shifter (0, ay, 0);

        BCSSpringModel sm;

        sm.AddWall (wall, doProjection ? 0.0 : 10.0);
        sm.AddStation (DPoint3d::From (8,5),  20.0);  // big blob
        sm.AddStation (DPoint3d::From (12,5), 1.0); // small blob

        Check::SaveTransformed (wall);
        Check::Shift (30,0,0);
        sm.SolveSprings (doProjection);
        SaveZones (wall, sm);
        Check::SaveTransformed (wall);
        }
    Check::ClearGeometry ("BCS.SpringModelA");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
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
        DPoint3d::From (30,53),
        DPoint3d::From (30,60),
        DPoint3d::From (50,60),
        DPoint3d::From (50,80),
        DPoint3d::From (0, 70),
        DPoint3d::From (0,53),
        DPoint3d::From (0,30),
        DPoint3d::From (10,30),
        DPoint3d::From (10,23),
        DPoint3d::From (0,23),
        DPoint3d::From (0,0)
        };

    Check::SaveTransformed (wall);


    double ay = 400.0;
    for (bool doLaplace : bvector<bool> {false, true})
        {
        SaveAndRestoreCheckTransform shifter (0, ay, 0);
        bool doProjection = true;
        double edgeFactor = doProjection ? 0.0 : 1.0;
        sm.AddWall (wall, edgeFactor * 10.0);

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

        sm.AddStation (DPoint3d::From (35,65), 8.0);
        sm.AddStation (DPoint3d::From (45,65), 8.0);
        sm.AddStation (DPoint3d::From (45,75), 8.0);
        sm.AddStation (DPoint3d::From (25,70), 8.0);


        sm.SolveSprings (doProjection, doLaplace);
        Check::SaveTransformed (wall);
        SaveZones (wall, sm);
        }
    Check::ClearGeometry ("BCS.SpringModelB");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BCS,SpringModelC)
    {
    double rA = 2.0;
    double rB = 1.2;
    DPoint3d centerA = DPoint3d::From (0,0,0);
    DPoint3d centerB = DPoint3d::From (0,0,0);
    double shift = 100.0;
    for (size_t numA : bvector<size_t> {4, 5,6,8,11})
        {
        SaveAndRestoreCheckTransform shifter (0, shift, 0.0);
        DPoint3dDoubleArrays wall (DEllipse3d::FromCenterRadiusXY (centerA, rA), numA);
        for (size_t numB : bvector<size_t> {3,4,5,7,9})
            {
            SaveAndRestoreCheckTransform shifter (shift, 0, 0.0);
            DPoint3dDoubleArrays station (DEllipse3d::FromCenterRadiusXY (centerB, rB), numB);
            station.m_xyz.pop_back ();
            station.m_f.pop_back ();
            BCSSpringModel sm;
            sm.AddWall (wall.m_xyz);
            for (auto xyz : station.m_xyz)
                sm.AddStation (xyz, 0.3);

            Check::SaveTransformed (wall.m_xyz);
            Check::SaveTransformedMarkers (station.m_xyz, 0.01);
            Check::Shift (30,0,0);
            sm.SolveSprings (true, false);
            SaveZones (wall.m_xyz, sm);
            }
        }

    Check::ClearGeometry ("BCS.SpringModelC");
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GriddedSpaceManager,VaryMeshSizeWithSquareGrid)
    {
    for (double meshSize : bvector<double>{1,2,3,4})
        TestGriddedSpaceManager (meshSize, false, false);
    Check::ClearGeometry ("GriddedSpaceManager.VaryMeshSizeWithSquareGrid");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GriddedSpaceManager,VaryMeshSizeWithIsoGrid)
    {
    // Preshift to be friends with peers ...
    Check::Shift (100,0,0);
    for (double meshSize : bvector<double>{1,2,3,4})
        TestGriddedSpaceManager (meshSize, true, false);
    Check::ClearGeometry ("GriddedSpaceManager.VaryMeshSizeWithIsoGrid");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
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


struct Fixity
{
enum class Type
    {
    Fixed = 0,
    Free  = 2
    };
Type m_type;
DVec3d m_vector;
size_t m_xIndex, m_yIndex;
Fixity (Type type)
    : m_type(type), m_vector(DVec3d::From (0,0,0)),
    m_xIndex(SIZE_MAX),
    m_yIndex(SIZE_MAX)
    {
    }
bool IsFree  (){return m_type == Type::Free;}
bool IsFixed (){return m_type == Type::Fixed;}
};

struct ISpringQueries
{
virtual DVec3d Fij (DPoint3dCR Xi, DPoint3dCR Xj, double ri, double rj) = 0;
};
struct BubblePhysics : ISpringQueries
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
    return a >= 0.0 ? sqrt (a) : 0.0;
    }
// Return force vector between two bubble centers 
DVec3d Fij (DPoint3dCR Xi, DPoint3dCR Xj, double ri, double rj) override
    {
    DVec3d delta = Xi - Xj;
    delta.Normalize ();
    return (1.0 - Lij (ri, rj)) * delta;
    }

};

struct SpringPhysics : ISpringQueries
{

DVec3d Fij (DPoint3dCR Xi, DPoint3dCR Xj, double ri, double rj) override
    {
    DVec3d delta = Xi - Xj;
    double d = delta.Normalize ();
    return (d - (ri + rj)) *delta;
    }

};

struct BubbleNetwork
{
bvector<DPoint3d> m_points;
bvector<double> m_radii;
bvector<bvector<size_t>> m_neighbors;
bvector<Fixity> m_fixity;
ISpringQueries &m_physics;


// COPY physics model ....
BubbleNetwork (ISpringQueries &physics)
    : m_physics (physics)
    {}

size_t AddPoint (double x, double y, double radius, Fixity fixity)
    {
    size_t index = m_points.size ();
    m_points.push_back (DPoint3d::From (x, y));
    m_radii.push_back (radius);
    m_fixity.push_back (fixity);
    m_neighbors.push_back (bvector<size_t> ());
    return index;
    }

RowMajorMatrix m_A;
RowMajorMatrix m_F;
void AssignDOFs ()
    {
    size_t numDOF = 0;
    for (auto &fixity : m_fixity)
        {
        if (fixity.IsFree ())
            {
            fixity.m_xIndex = numDOF++;
            fixity.m_yIndex = numDOF++;
            }
        else
            {
            fixity.m_xIndex = fixity.m_yIndex = SIZE_MAX;
            }
        }
    m_A.SetSizes (numDOF, numDOF);
    m_F.SetSizes (numDOF, 1);
    ClearSystem ();
    }
// Look up dof indices for vertex.
// return true if BOTH are valid.
bool TryVertexIndexToDOFs (size_t index, size_t &iX, size_t &iY)
    {
    size_t numDOF = m_F.size ();
    if (index < m_fixity.size ())
        {
        iX = m_fixity[index].m_xIndex;
        iY = m_fixity[index].m_yIndex;
        return iX < numDOF && iY < numDOF;
        }
    iX = SIZE_MAX;
    iY = SIZE_MAX;
    return false;
    }
void ClearSystem ()
    {
    m_A.SetZero ();
    m_F.SetZero ();
    }
// Assemble
void AssembleSymmetricMemberForces
(
size_t indexA,      // central node where sums are being formed.
size_t indexB,      // far node.
DVec3dCR FA,         // Force vector contribution at A
DVec3dCR dFdxA,     // derivative wrt Ax (derivative wrt Bx is negative)
DVec3dCR dFdyA      // derivative wrt A (derivative wrt By is negative)
)
    {
    if (m_fixity[indexA].IsFree ()) // There are two good matrix indices there !!
        {
        size_t ix = m_fixity[indexA].m_xIndex;
        size_t iy = m_fixity[indexA].m_yIndex;
        m_F.At (ix, 0) += FA.x;
        m_F.At (iy, 0) += FA.y;

        m_A.At (ix, ix) += dFdxA.x;
        m_A.At (ix, iy) += dFdxA.y;
        m_A.At (iy, ix) += dFdyA.x;
        m_A.At (iy, iy) += dFdyA.y;

        if (m_fixity[indexB].IsFree ()) // And two more here
            {
            size_t jx = m_fixity[indexB].m_xIndex;
            size_t jy = m_fixity[indexB].m_yIndex;
            m_A.At (ix, jx) -= dFdxA.x;
            m_A.At (ix, jy) -= dFdxA.y;
            m_A.At (iy, jx) -= dFdyA.x;
            m_A.At (iy, jy) -= dFdyA.y;
            }
        }
    }

bool Connect (size_t index0, size_t index1)
    {
    if (index0 < m_points.size () && index1 < m_points.size ())
        {
        m_neighbors[index0].push_back (index1);
        m_neighbors[index1].push_back (index0);
        return true;
        }
    return false;
    }
// sum forces from neighbors of m_points[index] + DVec3d::From (dx,dy,0), using m_points for neighbor coordinates
DVec3d SumForces (size_t index, double dx = 0.0, double dy = 0.0)
    {
    DVec3d F = DVec3d::From (0, 0, 0);
    auto xyz = DPoint3d::FromShift (m_points[index], dx, dy);
    for (auto neighborIndex : m_neighbors[index])
        {
        F = F + m_physics.Fij (xyz, m_points[neighborIndex], m_radii[index], m_radii[neighborIndex]);
        }
    return F;
    }

void AssembleNeighborhoodForces (
size_t indexA,  // central index
double delta    // step size for central differencing
)
    {
    if (m_fixity[indexA].IsFree () && delta > 0.0)
        {
        double divDelta = 0.5 / delta;
        auto xyzA = m_points[indexA];
        double rA = m_radii[indexA];
        for (auto indexB : m_neighbors[indexA])
            {
            DPoint3d xyzB = m_points[indexB];
            double rB = m_radii[indexB];
            auto F = m_physics.Fij (xyzA, xyzB, rA, rB);
            auto dFdx = divDelta *
                     (  m_physics.Fij (DPoint3d::FromShift (xyzA,  delta, 0, 0), xyzB, rA, rB)
                      - m_physics.Fij (DPoint3d::FromShift (xyzA, -delta, 0, 0), xyzB, rA, rB)
                     );
            auto dFdy = divDelta *
                    (  m_physics.Fij (DPoint3d::FromShift (xyzA, 0,  delta, 0), xyzB, rA, rB)
                     - m_physics.Fij (DPoint3d::FromShift (xyzA, 0, -delta, 0), xyzB, rA, rB)
                    );
            AssembleSymmetricMemberForces (indexA, indexB, F, dFdx, dFdy);
            }
        }
    }

void ShiftWithFrictionFactor (double frictionFactor, size_t index, double dx, double dy, double dz = 0.0)
    {
    m_points[index] = m_points[index] + DVec3d::From (frictionFactor * dx, frictionFactor * dy, frictionFactor * dz);
    }
DRange3d NeighborRange (size_t index)
    {
    auto range = DRange3d::NullRange ();
    for (auto neighborIndex : m_neighbors[index])
        range.Extend (m_points[neighborIndex]);
    return range;
    }

// Iteratively move each point to stable point.
bool Solve_global (
size_t maxIterations = 100,
double fractionalStep = 1.0e-3,
double fractionalStepTolerance = 1.0e-4,
double frictionFactor = 0.8,
size_t numConvergedRequired = 2
)
    {
    AssignDOFs ();
    size_t numXYZ = m_points.size ();
    static double s_distanceFactor = 1000.0;
    double distanceTol = DoubleOps::SmallMetricDistance () * s_distanceFactor;
    // unused - size_t numConverged = 0;
    for (size_t numIteration = 0; numIteration < maxIterations; numIteration++)
        {
        ClearSystem ();
        // unused - size_t numFail = 0;
        for (size_t i = 0; i < numXYZ; i++)
            {
            if (m_fixity[i].IsFree())
                {
                DRange3d neighborRange = NeighborRange (i);
                double deltaRef = neighborRange.low.DistanceXY (neighborRange.high);
                double delta = fractionalStep * deltaRef;
                if (delta == 0.0)
                    return false;
                AssembleNeighborhoodForces (i, delta);
                }
            }
        for (size_t pivot = 0; pivot < m_A.NumRows (); pivot++)
            {
            if (!m_A.BlockElimination (pivot, m_F))
                return false;
            }
        
        m_A.BackSubstitute (m_F);   // Can't fail -- elimination left 1 on diagonal

        size_t iMax, iX, iY;
        double maxU = m_F.MaxAbsBelowPivot (0, iMax);
        for (size_t i = 0; i < numXYZ; i++)
            {
            if (TryVertexIndexToDOFs (i, iX, iY))
                {
                m_points[i].x -= m_F.At (iX,0);
                m_points[i].y -= m_F.At (iY,0);
                }
            }
        if (maxU <= distanceTol)
            return true;
        }
    return false;
    }



void EmitSprings ()
    {
    bvector<DSegment3d> segments;
    for (size_t i0 = 0; i0 < m_neighbors.size (); i0++)
        {
        auto arc = ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (m_points[i0], m_radii[i0]));
        Check::SaveTransformed (*arc);

        DPoint3d xyz0 = m_points[i0];
        for (size_t i1 : m_neighbors[i0])
            {
            DPoint3d xyz1 = m_points[i1];
            segments.push_back (DSegment3d::From (xyz0, xyz1));
            }
        }
    Check::SaveTransformed (segments);
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BubblePhysics,Triangle1)
    {
    for (double mu : bvector<double> {1.0, 1.2, 1.5})
        {
        SaveAndRestoreCheckTransform shifter (0, 30.0, 00);
        for (double r1 : bvector<double> {1, 0.9, 0.5})
            {
            for (double r3 : bvector<double> {1,0.9, 0.6, 0.4})
                {
                SaveAndRestoreCheckTransform shifter (10.0,0,0);
                SpringPhysics physics;
                BubbleNetwork network (physics);
                size_t i0 = network.AddPoint (0,0,  r1, Fixity (Fixity::Type::Fixed));
                size_t i1 = network.AddPoint (2,0,  mu * r1, Fixity (Fixity::Type::Fixed));
                size_t i2 = network.AddPoint (1,2,  mu * mu * r1, Fixity (Fixity::Type::Fixed));
                size_t i3 = network.AddPoint (1.1, 1, r3, Fixity (Fixity::Type::Free));
                network.Connect (i0, i3);
                network.Connect (i1, i3);
                network.Connect (i2, i3);
                network.EmitSprings ();
                bool stat = network.Solve_global ();

                Check::Shift (0,10,0);
                network.EmitSprings ();
                if (!stat)
                    Check::SaveTransformed (*ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (DPoint3d::From (1.1, 1.0), 5)));
                }
            }
        }
    Check::ClearGeometry ("BubblePhysics.Triangele1");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BubblePhysics,Quad2)
    {
    for (double r1 : bvector<double> {1, 0.9, 0.5})
        {
        SaveAndRestoreCheckTransform shifter (0, 40,0);

        for (double r2 : bvector<double> {3, 2, 0.6, 0.4})
            {
            SaveAndRestoreCheckTransform shifter (10.0,0,0);
            SpringPhysics physics;
            BubbleNetwork network (physics);
            size_t i0 = network.AddPoint (0,0,  r1, Fixity (Fixity::Type::Fixed));
            size_t i1 = network.AddPoint (5,0,  r1, Fixity (Fixity::Type::Fixed));
            size_t i2 = network.AddPoint (5,4,  0.5 * r1, Fixity (Fixity::Type::Fixed));
            size_t i3 = network.AddPoint (0,4,  0.5 * r1, Fixity (Fixity::Type::Fixed));

            size_t i4 = network.AddPoint (1,2,  r2, Fixity (Fixity::Type::Free));
            size_t i5 = network.AddPoint (4,2,  r2, Fixity (Fixity::Type::Free));
            network.Connect (i0, i4);
            network.Connect (i3, i4);
            network.Connect (i4, i5);
            network.Connect (i1, i5);
            network.Connect (i2, i5);

            network.EmitSprings ();

            bool stat = network.Solve_global ();

            Check::Shift (0,10,0);
            network.EmitSprings ();
            if (!stat)
                Check::SaveTransformed (*ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (DPoint3d::From (2.5,2), 5)));
            }
        }
    Check::ClearGeometry ("BubblePhysics.Quad2");
    }



void AddPoints (bvector<DPoint3d> &points, DEllipse3dCR arc, size_t numEdge)
    {
    double df = 1.0 / (double)numEdge;
    for (size_t i = 0; i < numEdge; i++)
        points.push_back (arc.FractionToPoint (i * df));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu,CreateDelauney)
    {
    double dy = 50.0;
    bvector<DPoint3d> points;

    bvector<size_t> edgeCount {7,9,12, 6, 11};

    for (size_t i = 0; i < s_arcSamples.size (); i++)
        {
        SaveAndRestoreCheckTransform shifter (80.0,0,0);
        AddPoints (points, s_arcSamples[i], i < edgeCount.size () ? edgeCount[i] : 9);
        Check::SaveTransformed (points);
        PolyfaceHeaderPtr delauney, voronoi;
        if (Check::True (PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY (points, delauney, voronoi)))
            {
            Check::Shift (0,dy,0);
            Check::SaveTransformed (*delauney);
            Check::SaveTransformed (*voronoi);
            }
        }
    Check::ClearGeometry ("Vu.CreateDelauney");
    }

double AssignRadiusByRow (double a0, double a1, size_t i, size_t j, size_t numI, size_t numJ, size_t period)
    {
    return DoubleOps::Interpolate (a0, ((j * numI + i ) % period) / (double) period, a1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu,CreateDelauneySkew)
    {
    // unused - double dy = 80.0;
    double a = 1.0;
    double r0 = 0.2 * a, r1 = 0.7 * a;
    size_t numX = 7;
    size_t numY = 5;
    static size_t s_period = 11;
    for (int distanceSelect = 3; distanceSelect < 4; distanceSelect++)
        {
        double yMax = 10.0;
        SaveAndRestoreCheckTransform shifter0 (0.0, yMax, 0.0);

        for (double radiusFactor : bvector<double> {1.0, 1.2})// : bvector<double> {90.0})//60.0, 90.0, 80.0, 50.0, 40.0, 30.0, 100.0, 130.0})
            {
            double degrees = 80.0;
            Angle theta = Angle::FromDegrees (degrees);
            bvector<double> radii;
            DPoint3dDVec3dDVec3d frame (0,0,0, a,0,0,  a * theta.Cos (), a * theta.Sin (), 0);
            bvector<DPoint3d> points;
            for (size_t j = 0; j <= numY; j++)
                for (size_t i = 0; i <= numX; i++)
                    {
                    points.push_back (frame.Evaluate ((double) i, (double) j));
                    radii.push_back (AssignRadiusByRow (r0 * radiusFactor, r1 * radiusFactor, i, j, numX, numY, s_period));
                    }

            SaveAndRestoreCheckTransform shifter (points.back ().x + 50.0 * a,0,0);
            yMax = DoubleOps::Max (yMax, points.back ().y);
            PolyfaceHeaderPtr delauney, voronoi;
            bvector<NeighborIndices> cellData;
            if (Check::True (PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY (points, radii, distanceSelect, delauney, voronoi, &cellData)))
                {
                Check::SaveTransformed (*delauney);
                Check::SaveTransformed (*voronoi);
                if (distanceSelect != 0)
                    {
                    for (size_t i = 0; i < points.size (); i++)
                        {
                        Check::SaveTransformed (DEllipse3d::FromCenterRadiusXY (points[i], radii[i]));
                        }
                    }
                //Check::ValidIndex (SIZE_MAX, cellData, "Verify ValidIndex logic");
                if (Check::True (points.size () == cellData.size ()))
                    {
                    auto visitor = PolyfaceVisitor::Attach (*voronoi);
                    visitor->SetNumWrap (1);
                    for (size_t i = 0; i < cellData.size (); i++)
                        {
                        size_t readIndex = cellData[i].GetAuxIndex ();
                        size_t pointIndex = cellData[i].GetSiteIndex ();
                        // verify that the siteIndex in the neighbor arrays agree with the site index in the neighbor entry
                        for (auto &neighborEntry : cellData[i].Neighbors ())
                            {
                            size_t neighborSite = neighborEntry.siteIndex;
                            size_t k = neighborEntry.neighborIndex;
                            if (Check::ValidIndex (k, cellData, "cellData neighborIndex"))
                                Check::Size (neighborSite, cellData[k].GetSiteIndex (), "siteIndex match");
                            }
                        if (   Check::True (pointIndex < points.size (), "each voronoi cell must have a point index")
                            && Check::True (readIndex != SIZE_MAX, "Each point must have a voronoi region")
                            && Check::True (visitor->MoveToFacetByReadIndex (readIndex), "Indexed facet access")
                            )
                            {
                            int q = bsiGeom_XYPolygonParity
                                    (
                                    &points[pointIndex],
                                    &visitor->Point ()[0],
                                    (int)visitor->Point ().size (),
                                    DoubleOps::SmallMetricDistance ()
                                    );
                            if (q != 1)
                                {
                                auto v = Check::SetMaxVolume (1000);
                                Check::PrintIndent (0);
                                Check::Print (points[i], "seed");
                                Check::Print (q, "INOUT");
                                Check::Print (visitor->Point (), "Voronoi");
                                Check::Int (1, q, "Voronoi region contains its seed point");
                                Check::SetMaxVolume (v);
                                }

                            }
                        }
                    }


                }
            shifter0.SetShift (0, yMax * 3.0 + 5.0, 0.0);
            }
        }
    Check::ClearGeometry ("Vu.CreateDelauneySkew");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  04/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu,CreateDelauneyPointsOnLine)
    {
    // 3 input points on x axis, same spacing, much larger radii (problem from Mindaugus)
    double gridStep = 50.0;
    // double markerSize = 0.02;
    for (int distanceSelect : {3})
        {
        SaveAndRestoreCheckTransform shifter (3 * gridStep, 0, 0);
        for (double xCoordinate : {1.0, 0.5, 0.110, 0.108, 0.106, 0.01, 0.001})
            {
            SaveAndRestoreCheckTransform shifter (0, 3 * gridStep, 0);
            bvector<DPoint3d> points {
                DPoint3d::From (-xCoordinate, 0.00000000000000000, 0.00000000000000000),
                DPoint3d::From (0.00000000000000000, 0.00000000000000000, 0.00000000000000000),
                DPoint3d::From (xCoordinate, 0.00000000000000000, 0.00000000000000000)
                };
            bvector<double> radii { 0.95050000000000001, 0.92822500000000008, 0.91820125000000008};
         
            PolyfaceHeaderPtr delauney, voronoi;
            bvector<NeighborIndices> cellData;
            if (Check::True (PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY (points, radii, distanceSelect, delauney, voronoi, &cellData)))
                {
                // Check::SaveTransformed (*delauney);
                Check::SaveTransformed (*voronoi);
                //Check::Shift (0, gridStep, 0);
                //Check::SaveTransformed (*delauney);
                // z shift to make the point markers visible.
                Check::Shift (0, 0, 0.1);
                for (size_t i = 0; i < radii.size (); i++)
                    {
                    Check::SaveTransformedMarker (points[i], -radii[i]);
                    Check::SaveTransformed (DSegment3d::From (points[i], points[i] + DVec3d::From (0, radii[i], 0)));
                    }
                }
            }
        }
    Check::ClearGeometry ("Vu.CreateDelauneyPointsOnLine");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu,CreateDelauneyCircle)
    {
    double r = 10.0;
    auto ellipse0 = DEllipse3d::From (0,0,0,    r,0,0,  0,r,0,   0, Angle::TwoPi ());
    double delta = 10.0 * r;

    // put points on a circle or ellipse.   (The circle. This is inherently bad for delauney !!!)
    for (double eccentricity : bvector<double> {0.0, 0.001, 0.010, 0.10, 1.0})
        {
        SaveAndRestoreCheckTransform shifter (0.0, 2.5 * delta,0);
        for (size_t numPoints : bvector<size_t> { 5,7,11,16,32})
            {
            auto ellipse = ellipse0;
            ellipse.vector90.Scale (1.0 + eccentricity);
            bvector<DPoint3d> points;
            AddPoints (points, ellipse, numPoints);
            //auto range = DRange3d::From (points);
            SaveAndRestoreCheckTransform shifter (delta,0,0);
            Check::SaveTransformedMarkers (points, 0.2);
            PolyfaceHeaderPtr delauney, voronoi;
            if (Check::True (PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY (points, delauney, voronoi)))
                {
                Check::SaveTransformed (*delauney);
                Check::SaveTransformed (*voronoi);
                }
            }
        }
    Check::ClearGeometry ("Vu.CreateDelauneyCircle");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu,IncircleFlipProblem)
    {
    bvector<DPoint3d> points
        {
        DPoint3d::From (-78.00884047, 35.92836283), 
        DPoint3d::From (-84.52519939, 36.95442326),
        DPoint3d::From (-84.52519939, 31.04557674),
        DPoint3d::From (-81.43532935, 30.00000000) 
        };
    PolyfaceHeaderPtr delauney, voronoi;
    if (Check::True (PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY (points, delauney, voronoi)))
        {
        Check::SaveTransformed (*delauney);
        Check::SaveTransformed (*voronoi);
        }
    Check::ClearGeometry ("Vu.IncircleFlipProblem");
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu,TwoPoints)
    {
    bvector<DPoint3d> points
        {
        DPoint3d::From (1,4), 
        DPoint3d::From (3,1)
        };
    bvector<double> radii {2,3};
    for (int i = 0; i < 4; i++)
        {
        PolyfaceHeaderPtr delauney, voronoi;
        if (Check::True (PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY (points, radii, i, delauney, voronoi)))
            {
            Check::Shift (200,0,0);
            Check::SaveTransformed (*voronoi);
            Check::SaveTransformedMarkers (points, -0.1);
            }
        }
    Check::ClearGeometry ("Vu.TwoPoints");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu,FourPointsWeighted)
    {
    bvector<DPoint3d> points
        {
        DPoint3d::From (16.482974485129958, 8.610638157596922, 0.0),
        DPoint3d::From (20.374214571002707, 8.610638157596922, 0.0),
        DPoint3d::From (16.482974485129958, 4.3592795557477935, 0.0),
        DPoint3d::From (20.374214571002707, 4.3592795557477935, 0.0)
        };
    bvector<double> baseRadius {1.7196498565, 2.2748827835, 3.8452539636, 1.9226269818};
    double a = 20.0;
    //double b = 80.0;
    for (auto factor : bvector<double>{0.2, 0.8, 1.0, 2.0, 5.0})
        {
        SaveAndRestoreCheckTransform shifter (a, 0, 0);
        bvector<double> radius1;
        for (size_t i = 0; i < points.size (); i++)
            {
            radius1.push_back (baseRadius[i] * factor);
            }


        for (int i = 0; i < 4; i++)
            {
            SaveAndRestoreCheckTransform shifter (a, 0, 0);
            for (size_t i = 0; i < points.size (); i++)
                {
                Check::SaveTransformed (DEllipse3d::FromCenterRadiusXY (points[i], radius1[i]));
                }

            PolyfaceHeaderPtr delauney, voronoi;
            if (Check::True (PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY (points, radius1, i, delauney, voronoi)))
                {
                Check::SaveTransformed (*delauney);
                Check::SaveTransformed (*voronoi);
                }
            }
        }
    Check::ClearGeometry ("Vu.FourPointsWeighted");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu,ColinearPoints)
    {
    double b = 200.0;
    //double alpha = 0.2;
    bvector<double> radii {2,3, 1, 2};
    for (auto slope : bvector<double>{0.0, 1.0, 0.2342789123798})
        {
        SaveAndRestoreCheckTransform shifter (0, b, 0);
        bvector<DPoint3d> points;
        for (auto x : bvector<double> {1.0, 2.0, 5.0, 8.0})
            points.push_back (DPoint3d::From (x, x * slope));
       //DPoint3d pointB = points.back ();
       // points.push_back (DPoint3d::FromInterpolateAndPerpendicularXY (points.front (), 1.5, pointB, alpha));
       // points.push_back (DPoint3d::FromInterpolateAndPerpendicularXY (points.front (), 1.5, pointB, -alpha));
        for (int i = 0; i < 4; i++)
            {
            PolyfaceHeaderPtr delauney, voronoi;
            if (Check::True (PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY (points, radii, i, delauney, voronoi)))
                {
                Check::Shift (200,0,0);
                if (voronoi.IsValid ())
                    Check::SaveTransformed (*voronoi);
                Check::SaveTransformedMarkers (points, -0.1);
                }
            }
        }
    Check::ClearGeometry ("Vu.ColinearPoints");
    }



//! construct delauney triangulations and voronoi diagram for given station points (e.g. room centers)
//! clip the voronoi to the wall polygon.
bool DoClips (
bvector<DPoint3d> const &stationPoints, //!< [in] nominal room centers
bvector<DPoint3d> const &wallPoints,    //!< [in] (single) wall polygon
PolyfaceHeaderPtr &delauney,            //!<  [out] delauney triangulation of room centers
PolyfaceHeaderPtr &voronoi,             //!< [out] voronoi regions
PolyfaceHeaderPtr &voronoiInsideWalls   //!< [out] voronoi regions clipped to wall 
)
    {
    auto clipper = PolyfaceHeader::CreateVariableSizeIndexed ();
    clipper->AddPolygon (wallPoints);
    delauney = nullptr;
    voronoi = nullptr;
    voronoiInsideWalls = nullptr;
    if (PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY (stationPoints, delauney, voronoi))
        {
        PolyfaceHeaderPtr outside;
        PolyfaceHeader::ComputePunchXYByPlaneSets (*clipper, *voronoi, &voronoiInsideWalls, &outside);
        return true;
        }
    return true;
    }

void DoClips (bvector<DPoint3d> const &clipPoints)
    {
    bvector<DPoint3d> points;
    bvector<size_t> edgeCount {7,9,12, 6, 11};
    double dyMax = 10.0;
    SaveAndRestoreCheckTransform shifterY;
    /* unused - auto baseTransformA = */Check::GetTransform ();

    for (size_t i = 0; i < s_arcSamples.size (); i++)
        {
        SaveAndRestoreCheckTransform shifterX;

        /* unused - auto baseTransformB = */Check::GetTransform ();
        AddPoints (points, s_arcSamples[i], i < edgeCount.size () ? edgeCount[i] : 9);
        PolyfaceHeaderPtr delauney, voronoi, voronoiInside;
        DRange3d rangeA = DRange3d::From (points);
        double dy = rangeA.YLength ();
        double dx = rangeA.XLength ();
        if (DoClips (points, clipPoints, delauney, voronoi, voronoiInside))
            {
            DRange3d rangeB = voronoi->PointRange ();
            dx = 1.5 * rangeB.XLength ();
            dy = 1.1 * rangeB.YLength ();
            Check::SaveTransformedMarkers (points, 0.1);
            Check::Shift (0,dy,0);
            Check::SaveTransformed (clipPoints);
            Check::SaveTransformed (*delauney);
            Check::SaveTransformed (*voronoi);
            Check::Shift (0, dy, 0);
            Check::SaveTransformed (*voronoiInside);
            Check::SaveTransformedMarkers (points, 0.1);
            }
        else
            {
            Check::SaveTransformed (points);
            }
        dyMax = DoubleOps::Max (dyMax, 3.0 * dy);
        shifterX.SetShift (2.0 * dx, 0, 0);
        }
    shifterY.SetShift (0, dyMax, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu,CreateClippedVornoi)
    {


    DEllipse3d clipArc = DEllipse3d::From (
                18,0,0,
                -4,0,0,
                0,20,0,
                Angle::DegreesToRadians (-35),
                Angle::DegreesToRadians (70)
                );
    DoClips (CreateOffsetStrip (clipArc, 18.0, 22.0, 7));
    Check::Shift (200,0,0);
    auto clipT = CreateT (20, 20, 4,6, 4, 5);
    DPoint3dOps::Add (clipT, DVec3d::From (-10,-10, 0));
    DoClips (clipT);

    Check::ClearGeometry ("Vu.CreateClippedVornoi");
    }
void Stroke (DConic4dCR conic, bvector<DPoint3d> &strokes, double theta0, double theta1, int numPoint)
    {
    strokes.clear ();
    DPoint3d xyz;
    for (int i = 0; i <= numPoint; i++)
        {
        double theta =
            DoubleOps::Interpolate (theta0, (double)i / (double)numPoint, theta1);
        bsiDConic4d_angleParameterToDPoint3d (&conic, &xyz, theta);
        strokes.push_back (xyz);
        }
    }

void ShowConic (DPoint3dR xyz0, double r0, DPoint3dR xyz1, double r1, double theta0, double theta1)
    {
    int s_numPoints = 17;
    DConic4d conic;
    bvector<DPoint3d> strokes;
    bsiDConic4d_initSignedCircleTangentCenters (&conic, &xyz0, r0, &xyz1, -r1);
    Stroke (conic, strokes, theta0, theta1, s_numPoints);
    Check::SaveTransformed (strokes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Voronoi,Hyperbolas6)
    {
    bvector<DPoint3d> xyzOuter;
    DPoint3d origin = DPoint3d::FromZero ();
    bvector<double> radiusOuter {1.0, 2.0, 2.1, 3.0, 1.5, 1.1};
    double a = 10.0;
    bvector<DSegment3d> segments;
    // In a smooth triangulation, expect 6 points around each central point
    for (int i = 0; i < radiusOuter.size (); i++)
        {
        double theta = i * Angle::DegreesToRadians (60.0);
        xyzOuter.push_back (DPoint3d::From (a * cos (theta), a * sin (theta)));
        segments.push_back (DSegment3d::From (origin, xyzOuter.back ()));
        }
    segments.pop_back ();   // eliminate redundant radius
    static double theta0 = Angle::DegreesToRadians (60.0);
    static double theta1 = Angle::DegreesToRadians (120.0);
    for (double r0 : bvector<double> {0.5, 1.0, 4.0})
        {
        SaveAndRestoreCheckTransform shifter (3.0 * a, 0, 0);
        Check::SaveTransformed (xyzOuter);
        Check::SaveTransformed (segments);
        Check::SaveTransformed (DEllipse3d::FromCenterRadiusXY (origin, r0));
        for (size_t i0 = 0; i0 + 1 < xyzOuter.size (); i0++)
            {
            Check::SaveTransformed (DEllipse3d::FromCenterRadiusXY (xyzOuter[i0], radiusOuter[i0]));
            size_t i1 = i0 + 1;
            ShowConic (origin, r0, xyzOuter[i0], radiusOuter[i0], theta0, theta1);
            ShowConic (xyzOuter[i0], radiusOuter[i0], xyzOuter[i1], radiusOuter[i1], theta0, theta1);
            }
        }
    Check::ClearGeometry ("Voronoi.Hyperbolas");
    }
// intersect a convex polygon and a (stroked) circle.  The result is another convex polygon.
void intersectCircleConvexPolygon (
DPoint3dCR center,  // IN circle center
double radius,      // IN circle readius
bvector<DPoint3d> const &convexPolygonPoints,   // IN points of CONVEX polygon (not checked)
int numCircleEdges,         // IN number of edges to create on circle.
bvector<DPoint3d> &circlePoints,    // RETURNED points on circle
bvector<DPoint3d> &intersectionPolygon         // RETURNED clipped polygon
)
    {
    bvector<DPoint3d> work;
    ConvexClipPlaneSet clipPlaneSet;
    clipPlaneSet.ReloadSweptConvexPolygon (convexPolygonPoints, DVec3d::From (0,0,1), 0);
    circlePoints.clear ();
    intersectionPolygon.clear ();

    double step = Angle::TwoPi () / numCircleEdges;
    for (int i = 0; i <= numCircleEdges; i++)
        circlePoints.push_back (DPoint3d::From (center.x + radius * cos(i * step), center.y + radius * sin (i * step), 0.0));
    circlePoints.back () = circlePoints.front ();   // ensure bit closure
    clipPlaneSet.ConvexPolygonClip (circlePoints, intersectionPolygon, work);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet,ConvexPolygonClipCircleGon)
    {
    bvector<DPoint3d> polygonPoints {
    DPoint3d::From (0,0,0),
    DPoint3d::From (2,-6,0),
    DPoint3d::From (10,0,0), 
    DPoint3d::From (10,1,0), 
    DPoint3d::From (5,3,0), 
    DPoint3d::From (0,2,0), 
    DPoint3d::From (0,0,0)
    };
#ifdef inlineClip
    ConvexClipPlaneSet clipPlaneSet;
    clipPlaneSet.ReloadSweptConvexPolygon (polygonPoints, DVec3d::From (0,0,1), 0);
    double radius = 5.0;
    bvector<DPoint3d> clipped, work;
    for (auto numEdge : bvector<int> {8,16,24,32,64})
        {
        SaveAndRestoreCheckTransform shifter (4.0 * radius, 0, 0);
        bvector<DPoint3d> ngon;
        double step = Angle::TwoPi () / numEdge;
        for (int i = 0; i <= numEdge; i++)
            ngon.push_back (DPoint3d::From (radius * cos(i * step), radius * sin (i * step), 0.0));
        ngon.back () = ngon.front ();   // ensure bit closure
        clipPlaneSet.ConvexPolygonClip (ngon, clipped, work);
        Check::SaveTransformed (polygonPoints);
        Check::SaveTransformed (ngon);
        Check::Shift (0, 2.0 * radius, 0);
        Check::SaveTransformed (clipped);
        }
#else
    double radius = 5.0;
    DPoint3d center = DPoint3d::From (0.5, 1, 0);
    bvector<DPoint3d> clipped;
    for (auto numEdge : bvector<int> {8,16,24,32,64})
        {
        SaveAndRestoreCheckTransform shifter (4.0 * radius, 0, 0);
        bvector<DPoint3d> circlePoints;
        intersectCircleConvexPolygon (center, radius, polygonPoints, numEdge, circlePoints, clipped);
        Check::SaveTransformed (polygonPoints);
        Check::SaveTransformed (circlePoints);
        Check::Shift (0, 3.0 * radius, 0);
        Check::SaveTransformed (clipped);
        }
#endif
    Check::ClearGeometry ("ClipPlaneSeat.ConvexPolygonClipCircleGon");
    }


// Construct voronoi for 
void DoWallAndRadiusClips
(
bvector<DPoint3d> &stations,    // station coordinates
bvector<double> &weightRadii,   // weight to be used in voroni
bvector<double> &clipRadii,     // radius to be used for post-voronoi clip
bvector<DPoint3d> &wallPoints   // (closed) area for wall clip.
)    
    {
    static int s_numPerCircle = 32;
    PolyfaceHeaderPtr voronoi;
    PolyfaceHeaderPtr delauney;
    bvector<NeighborIndices> cellData;
    bvector<DPoint3d> clippedRegion;
    bvector<DPoint3d> circlePoints;
    PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY(stations, weightRadii, 3, delauney, voronoi, &cellData);
    auto visitor = PolyfaceVisitor::Attach (*voronoi);
    visitor->SetNumWrap (1);
    // clip each voronoi region to its circle ...
    PolyfaceHeaderPtr  voronoiClippedToCircles = PolyfaceHeader::CreateVariableSizeIndexed ();
    for (size_t i = 0; i < cellData.size (); i++)
        {
        size_t readIndex = cellData[i].GetAuxIndex ();
        size_t pointIndex = cellData[i].GetSiteIndex ();
        if (visitor->MoveToFacetByReadIndex (readIndex))
            {
            intersectCircleConvexPolygon (stations[pointIndex], clipRadii[pointIndex],
                    visitor->Point (),
                    s_numPerCircle,
                    circlePoints,
                    clippedRegion);
            voronoiClippedToCircles->AddPolygon (clippedRegion);
            }
        }

    auto wallClipper = PolyfaceHeader::CreateVariableSizeIndexed ();
    auto voronoiClippedToCirclesAndWalls = PolyfaceHeader::CreateVariableSizeIndexed ();
    auto outside = PolyfaceHeader::CreateVariableSizeIndexed ();
    wallClipper->AddPolygon (wallPoints);

    PolyfaceHeader::ComputePunchXYByPlaneSets (*wallClipper, *voronoiClippedToCircles, &voronoiClippedToCirclesAndWalls, &outside);
    Check::SaveTransformed (*voronoi);
    Check::SaveTransformedMarkers (stations, 0.2);
    Check::Shift (0, 200,0);

    Check::SaveTransformedMarkers (stations, 0.2);
    Check::SaveTransformed (*voronoiClippedToCircles);
    Check::SaveTransformed (*wallClipper);

    Check::Shift (0, 40,0);
    Check::SaveTransformed (*voronoiClippedToCirclesAndWalls);
    Check::SaveTransformedMarkers (stations, 0.2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu, WallAndRadiusClips)
    {
    bvector<DPoint3d> stations
        {
        DPoint3d::From (0,0,0),
        DPoint3d::From (10,0,0),
        DPoint3d::From (10,5,0),
        DPoint3d::From (1,8,0)
        };
    bvector<double> weightRadii { 5, 3, 4, 6 };
    bvector<double> clipRadii { 5, 4, 3, 8};
    bvector<DPoint3d> wallPoints
        {
        DPoint3d::From(-3,-3),
        DPoint3d::From (8,-3),
        DPoint3d::From (12,0),
        DPoint3d::From (12,20),
        DPoint3d::From (-2,10),
        DPoint3d::From (-3, -3)
        };
    DoWallAndRadiusClips (stations, weightRadii, clipRadii, wallPoints);
    Check::ClearGeometry ("Vu.WallAndRadiusClips");

    }

// Construct voronoi for 
void DoDirectWallAndRadiusClips
(
bvector<DPoint3d> &stations,    // station coordinates
bvector<double> &weightRadii,   // weight to be used in voroni
bvector<double> &clipRadii,     // radius to be used for post-voronoi clip
bvector<DPoint3d> &wallPoints   // (closed) area for wall clip.
)    
    {
    static int s_numPerCircle = 32;
    PolyfaceHeaderPtr voronoi;
    PolyfaceHeaderPtr delauney;
    bvector<NeighborIndices> cellData;
    bvector<DPoint3d> clippedRegion;
    bvector<DPoint3d> circlePoints;
    PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY(stations, weightRadii, 3, delauney, voronoi, &cellData);
    auto wallClip = ClipPlaneSet::FromSweptPolygon (&wallPoints[0], wallPoints.size ());
    auto visitor = PolyfaceVisitor::Attach (*voronoi);
    visitor->SetNumWrap (1);
    // clip each voronoi region to its circle ...
    PolyfaceHeaderPtr  voronoiClippedToCircles = PolyfaceHeader::CreateVariableSizeIndexed ();
    for (size_t i = 0; i < cellData.size (); i++)
        {
        size_t readIndex = cellData[i].GetAuxIndex ();
        size_t pointIndex = cellData[i].GetSiteIndex ();
        if (visitor->MoveToFacetByReadIndex (readIndex))
            {
            intersectCircleConvexPolygon (stations[pointIndex], clipRadii[pointIndex],
                    visitor->Point (),
                    s_numPerCircle,
                    circlePoints,
                    clippedRegion);
            voronoiClippedToCircles->AddPolygon (clippedRegion);


            }
        }

    auto wallClipper = PolyfaceHeader::CreateVariableSizeIndexed ();
    auto voronoiClippedToCirclesAndWalls = PolyfaceHeader::CreateVariableSizeIndexed ();
    auto outside = PolyfaceHeader::CreateVariableSizeIndexed ();
    wallClipper->AddPolygon (wallPoints);

    PolyfaceHeader::ComputePunchXYByPlaneSets (*wallClipper, *voronoiClippedToCircles, &voronoiClippedToCirclesAndWalls, &outside);
    Check::SaveTransformed (*voronoi);
    Check::SaveTransformedMarkers (stations, 0.2);
    Check::Shift (0, 200,0);

    Check::SaveTransformedMarkers (stations, 0.2);
    Check::SaveTransformed (*voronoiClippedToCircles);
    Check::SaveTransformed (*wallClipper);

    Check::Shift (0, 40,0);
    Check::SaveTransformed (*voronoiClippedToCirclesAndWalls);
    Check::SaveTransformedMarkers (stations, 0.2);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu, DirectWallAndRadiusClips)
    {
    bvector<DPoint3d> stations
        {
        DPoint3d::From (0,0,0),
        DPoint3d::From (10,0,0),
        DPoint3d::From (10,5,0),
        DPoint3d::From (1,8,0)
        };
    bvector<double> weightRadii { 5, 3, 4, 6 };
    bvector<double> clipRadii { 5, 4, 3, 8};
    bvector<DPoint3d> wallPoints
        {
        DPoint3d::From(-3,-3),
        DPoint3d::From (8,-3),
        DPoint3d::From (12,0),
        DPoint3d::From (12,20),
        DPoint3d::From (-2,10),
        DPoint3d::From (-3, -3)
        };
    DoWallAndRadiusClips (stations, weightRadii, clipRadii, wallPoints);
    Check::ClearGeometry ("Vu.WallAndRadiusClips");

    }

