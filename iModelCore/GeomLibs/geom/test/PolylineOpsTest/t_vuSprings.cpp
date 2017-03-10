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

/////CONCEPTSTATION_START_CUT

// Space id's are one based !!!!
static const int s_invalidSpaceId = 0;

// the "userDataPAsInt" attribute of a vu node is the space id.
// spaceId == 0 is an unassigned node.
// (i.e. first space gets id 1)
struct SpaceDescriptor
{
DPoint3d m_seedPoint;
int m_id;
double m_targetArea;
double m_currentArea;
bvector<VuP> m_nodes;   // !! Complete list of nodes.
SpaceDescriptor () : m_seedPoint (DPoint3d::From (0,0,0)), m_id(0), m_targetArea (0.0), m_currentArea (0.0) {}
// Constructor -- initialize with a reference coordinate and an id but no nodes.
SpaceDescriptor (DPoint3dCR xyz, int id, double targetArea = 0.0)
    {
    m_id = id;
    m_seedPoint = xyz;
    m_targetArea = targetArea;
    }

int GetId () const { return m_id;}
// Add a node to this space.
// If the node is already in another space, return false.
bool AddNode (VuP node)
    {
    int idInNode = node->GetUserDataAsInt ();
    if (idInNode == m_id)
        return true;
    if (idInNode != 0)
        return false;
    m_nodes.push_back (node);
    node->SetUserDataAsIntAroundFace (m_id);
    return true;
    }

};

struct GriddedSpaceManager
{
friend struct GriddedSpaceQueries;
friend struct GriddedSpace_FloodFromSingleNode;
private:
VuSetP m_graph;
bvector<bvector <DPoint3d>> m_parityLoops;
bvector<bvector <DPoint3d>> m_chains;
double m_meshSize;
bool m_isoGrid;
bool m_smoothTriangles;
int    m_lastSpaceId;
bmap<int, SpaceDescriptor> m_spaces;
public:
// Constructor -- empty manager
GriddedSpaceManager () :
    m_graph(nullptr),
    m_isoGrid (true),
    m_smoothTriangles (true),
    m_meshSize (1.0)
    {
    }
~GriddedSpaceManager ()
    {
    if (m_graph != nullptr)
        vu_freeVuSet (m_graph);
    }
void SetMeshParams (double meshSize, bool isoGrid = true, bool smoothTriangles = true)
    {
    m_meshSize = meshSize;
    m_isoGrid = isoGrid;
    m_smoothTriangles = smoothTriangles;
    }
int GetSpaceId (VuP node)
    {
    return node->GetUserDataAsInt ();
    }
bool IsBarrierEdge (VuP node)
    {
    return node->HasMask (VU_BOUNDARY_EDGE | VU_RULE_EDGE);
    }

bool IsOccupied (VuP node) {return node->GetUserDataAsInt () != 0;}
VuSetP Graph (){ return m_graph;}

// create a graph for given geometry.
bool TryLoad
(
bvector<bvector <DPoint3d>> &parityLoops,
bvector<bvector <DPoint3d>> &chains
)
    {
    if (m_graph != nullptr)
        vu_freeVuSet (m_graph);
    m_graph = nullptr;
    m_parityLoops = parityLoops;
    m_chains      = chains;
    m_lastSpaceId = 0;

    m_graph = VuOps::CreateTriangulatedGrid (m_parityLoops, m_chains, bvector<DPoint3d> (),
                        bvector<double> (), bvector<double> (),
                        m_meshSize, m_meshSize, m_meshSize, m_meshSize, m_isoGrid, m_smoothTriangles);
    return m_graph != nullptr;
    }

int GetSpaceIdAtXYZ (DPoint3dCR xyz)
    {
    VuP node = vu_findContainingFace_linearSearch (m_graph, xyz, VU_EXTERIOR_EDGE);
    if (nullptr == node)
        return s_invalidSpaceId;
    int oldId = node->GetUserDataAsInt ();
    return oldId;
    }

// Try to claim the face containing xyz as a space.
int CreateSpace (DPoint3dCR xyz, double targetArea)
    {
    VuP node = vu_findContainingFace_linearSearch (m_graph, xyz, VU_EXTERIOR_EDGE);
    if (nullptr == node)
        return s_invalidSpaceId;
    int oldId = node->GetUserDataAsInt ();
    if (oldId != 0)
        {
        // this node is already claimed
        return s_invalidSpaceId;
        }
    int id = ++m_lastSpaceId;
    m_spaces[id] = SpaceDescriptor (xyz, ++m_lastSpaceId, targetArea);
    m_spaces[id].AddNode (node);
    return id;
    }

// Set mask in each node that (a) has a nonzero UserDataPAsInt and (b) edge mate has a different UserDataPAsInt
size_t SetAllSpaceBoundaryMasks (VuMask mask)
    {
    vu_clearMaskInSet (m_graph, mask);
    size_t numBoundary = 0;
    VU_SET_LOOP(node, m_graph)
        {
        int id = node->GetUserDataAsInt ();
        if (id != s_invalidSpaceId
            && node->EdgeMate ()->GetUserDataAsInt () != id)
            {
            node->SetMask (mask);
            numBoundary++;
            }
        }
    END_VU_SET_LOOP (node, m_graph)
    return numBoundary;
    }

double SpaceIdToArea (int id)
    {
    auto space = m_spaces.find (id);
    if (space == m_spaces.end ())
        return 0.0;
    double area = 0.0;
    for (auto node : space->second.m_nodes)
        {
        area += vu_area (node);
        }
    return area;
    }
    
};

// Helper class to do flood searches in a GriddedSpaceManager
struct GriddedSpace_FloodFromSingleNode
{
GriddedSpaceManager &m_manager;
GriddedSpace_FloodFromSingleNode (GriddedSpaceManager &manager) : m_manager(manager) {}

// Floods use a priority queue (aka heap) to access prefered direction of flood
struct HeapEntry
    {
    int m_id;
    VuP m_node;
    HeapEntry (int id, VuP node) : m_id (id), m_node(node) {}
    HeapEntry () : m_id (0), m_node (nullptr) {}
    };
MinimumValuePriorityQueue <HeapEntry> m_heap;

// Visit the neighbor across each edge of a face.
// (but do not cross barriers, or cross into occupied space)
// push the neighbors onto the priority queue.
void AddNeighborsToFloodHeap_weightByDistanceFromRefPoint (SpaceDescriptor &s, VuP faceSeed)
    {
    VU_FACE_LOOP (edge, faceSeed)
        {
        auto mate = edge->EdgeMate ();
        if (!m_manager.IsBarrierEdge (edge) && m_manager.GetSpaceId (mate) == 0)
            {
            DPoint2d uv;
            int numPos, numNeg;
            double area;
            vu_centroid (&uv, &area, &numPos, &numNeg, mate);
            double d = uv.Distance (DPoint2d::From (s.m_seedPoint));
            m_heap.Insert (HeapEntry (s.m_id, mate), d);
            }
        }
    END_VU_FACE_LOOP (edge, faceSeed)
    }

// flood from a space until it reaches its target area.
// (This may be called repeatedly with larger targetAreas)
bool ExpandSingleSpaceIdToTargetArea (int id, double targetArea)
    {
    m_heap.Clear ();
    auto space = m_manager.m_spaces.find (id);
    if (space == m_manager.m_spaces.end ())
        return false;
    space->second.m_currentArea = 0.0;
    space->second.m_targetArea = targetArea;
    // Sum areas of current faces.
    // Add all neighbors to the heap.
    for (auto node : space->second.m_nodes)
        {
        space->second.m_currentArea = vu_area (node);
        AddNeighborsToFloodHeap_weightByDistanceFromRefPoint (space->second, node);
        }
    HeapEntry entry;
    double distance;
    for (;;)
        {
        double a0 = space->second.m_currentArea;
        double a1 = space->second.m_targetArea;
        if (a0 > a1)
            break;
        if (!m_heap.RemoveMin (entry, distance))
            break;
        // The facet may have been absorbed from another direction . . . 
        if (!m_manager.IsOccupied (entry.m_node))
            {
            space->second.m_currentArea += vu_area (entry.m_node);
            space->second.AddNode (entry.m_node);
            AddNeighborsToFloodHeap_weightByDistanceFromRefPoint (space->second, entry.m_node);
            }
        }
    return space->second.m_currentArea >= space->second.m_targetArea;
    }

};

// Helper class to implement queries along with a GriddedSpaceManager.
struct GriddedSpaceQueries
{
GriddedSpaceManager &m_manager;
GriddedSpaceQueries (GriddedSpaceManager &manager) : m_manager(manager){}

void SaveSpaceBoundaries ()
    {
    double tolerance = DoubleOps::SmallMetricDistance ();
    _VuSet::TempMask boundaryMask (m_manager.m_graph);
    m_manager.SetAllSpaceBoundaryMasks (boundaryMask.Mask ());
    _VuSet::TempMask visitMask (m_manager.m_graph);
    bvector<VuP> chainNodes;
    bvector<DPoint3d> chain;
    VU_SET_LOOP (seedNode, m_manager.m_graph)
        {
        if (m_manager.IsOccupied (seedNode) && !visitMask.IsSetAtNode (seedNode) && boundaryMask.IsSetAtNode (seedNode))
            {
if (seedNode->HasMask (VU_EXTERIOR_EDGE))  // Something is strange -- this should be filtered by IsOccupied
    continue;
            chainNodes.clear ();
            auto currentNode = seedNode;
            for (; nullptr != currentNode;)
                {
                visitMask.SetAtNode (currentNode);
                chainNodes.push_back (currentNode);
                currentNode->SetMask (visitMask.Mask ());
                currentNode = currentNode->FSucc ()->FindMaskAroundReverseVertex (boundaryMask.Mask ());
                if (currentNode == seedNode)
                    break;
                }
            // (flood regions are closed ... chain should always close)
            chain.clear ();
            for (auto node : chainNodes)
                {
                DPoint3d xyz0 = node->GetXYZ ();
                DPoint3d xyz1 = node->FSucc ()->GetXYZ ();
                if (m_manager.IsBarrierEdge (node))
                    {
                    chain.push_back (xyz0);
                    chain.push_back (xyz1);
                    }
                else
                    {
                    chain.push_back (DPoint3d::FromInterpolate (xyz0, 0.5, xyz1));
                    }
                }
            if (currentNode == seedNode)    // closure -- we expect this
                {
                auto xyz = chain.front ();
                chain.push_back (xyz);
                }
            DPoint3dOps::Compress (chain, tolerance);
            Check::SaveTransformed (chain);
            }
        }
    END_VU_SET_LOOP (seedNode, m_manager.m_graph)
    }
void SaveWalls ()
    {
    Check::SaveTransformed (m_manager.m_parityLoops);
    Check::SaveTransformed (m_manager.m_chains);
    }
};


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

void RunSinglePointAreaShiftTest (bvector<DPoint3d> &neighbors)
    {
    auto loop = neighbors;
    loop.push_back (neighbors.front ());
    SaveAndRestoreCheckTransform shifter (4.0,0.0,0);
    bvector<DPoint3d> path;
    DPoint3d xyz0;
    xyz0.Zero ();   // Always pin the target at 000.  Initial areas are compute from this start
    //for (double additionalArea : bvector<double> { 0/*, 0.1, 0.5 */}) // EDL March 9 2017 Additional area does not change the path.
    double additionalArea = 0.0;
    double relaxationFactor = 1.0;
    //for (double relaxationFactor : bvector<double> {1.0, 0.9, 0.8, 0.75, 0.7})
        {
        SaveAndRestoreCheckTransform shifter (0,2.5, 0);
        Check::SaveTransformed (loop);
        // additionalArea = 0 ==> true diamond.
        // additionalArea = a ==> each quadrant has additional area a beyond its diagonal.
        bvector<double> sectorTargetArea, sectorArea;
        double b = 0.1;
        FillAreasFromNeighbors (sectorTargetArea, xyz0, neighbors, additionalArea);
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
    }
TEST(SinglePointAreaShift,Diamond)
    {
    bvector<DPoint3d> neighbors
        {
        DPoint3d::From (1,0,0),
        DPoint3d::From (0,1,0),
        DPoint3d::From (-1,0,0),
        DPoint3d::From (0,-1,0),
        };
    RunSinglePointAreaShiftTest (neighbors);
    neighbors[0].x += 0.25;
    RunSinglePointAreaShiftTest (neighbors);
    neighbors[1].x -= 0.15;
    RunSinglePointAreaShiftTest (neighbors);
    neighbors[2].y += 0.45;
    RunSinglePointAreaShiftTest (neighbors);
    Check::ClearGeometry ("SinglePointAreaShift.Diamond");
    }
