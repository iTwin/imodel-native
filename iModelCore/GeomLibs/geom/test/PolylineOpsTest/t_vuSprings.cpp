#include "testHarness.h"

// unused - static int s_noisy = 0;

extern void SaveGraph (VuSetP graph, VuMask mask = 0);
extern void SaveGraphEdges (VuSetP graph, VuMask mask = 0);


// BCSSpringModel
// "Building ConceptStation Spring Model"
// 
// A floorplan is defined by lines for the walls.
// A station is a point with an approximate size (single number) for its expected "radius"
// The spring model will:
//   1) form a triangulation of the space, with triangle vertices at the station points.
//   2) iteratively adjust station coordinate to mimic the stations moving around to meet their "size" goals.
//
// Usage sequence:
// 1) Contructor:
//***********    BCSSpringModel sm;
// 2) Insert geometry: (Multiple calls, any order):
//***********        sm.AddWall (wall, 10.0);   // wall coordinates are bvector<DPoint3d>
//***********        sm.AddStation (xyz, size);
// 3) move station points for size conditions:
//***********        sm.SolveSprings ();
// 4) traverse the vu graph to extract area around the stations ..
//***********        sm.CollectStationAreas (bvector<bvector<DPoint3d>> &areas, double shrinkFraction = 0.0)


//


struct BCSSpringModel : private _VuSet
{
VuSetP Graph () {return this;}


// Internal struct for station data -- current xyz, original xyz, target radius.
struct StationData
{
DPoint3d m_xyz;
DPoint3d m_xyzBase;
double m_radius;
StationData (DPoint3dCR xyzBase, double radius)
    {
   m_xyz = m_xyzBase = xyzBase;
   m_radius = radius;
    }
StationData () {}
};



// a movable node stores its station.
// the station has radius.
ValidatedDouble GetStationRadius (VuP node)
    {
    ptrdiff_t index = node->GetUserData1 ();
#ifdef VerifyData1Consistency
    size_t errors = 0;
    VU_VERTEX_LOOP (nodeA, node)
        {
        if (index != nodeA->GetUserData1 ())
            errors++;
        }
    END_VU_VERTEX_LOOP (nodeA, node)
#endif
    if (index >= 0 && (size_t)index < m_station.size ())
        return ValidatedDouble (m_station[index].m_radius, true);
    return ValidatedDouble (0.0, false);
    }

// Internal structure for computing weight values used in the spring iteration.
struct TriangleWeightFunction : VuScalarFunction
{

struct CappedQuadraticFunction
{
double m_c0, m_c1, m_c2;
double m_fMin, m_fMax;
CappedQuadraticFunction (double c0, double c1, double c2, double fMin = 0.0, double fMax = 10.0)
    : m_c0 (c0), m_c1 (c1), m_c2 (c2), m_fMin (fMin), m_fMax (fMax)
    {
    }
virtual ValidatedDouble EvaluateRToR (double u)
    {
    // unused - double f = m_c0 + u * (m_c1 + u * m_c2);
    return DoubleOps::Clamp (u, m_fMin, m_fMax);
    }
};

// Internal structure for computing spring effects in triangles.
TriangleWeightFunction (
BCSSpringModel &springModel,
int factorSelect,
CappedQuadraticFunction &edgeFunction    // univariate function for correcting a single edge length ratio.
)
    : m_springModel (springModel),
      m_factorSelect (factorSelect),
      m_edgeFunction (edgeFunction)
{}

//!<ul>
//!<li>0 - average of ratios of the two edge ratios
//!<li>1 - geometric mean of two edge ratios
//!</ul>
BCSSpringModel &m_springModel;
int m_factorSelect; 
double m_power;
CappedQuadraticFunction &m_edgeFunction;

double EdgeFactor (VuP node0)
    {
    VuP node1 = node0->EdgeMate ();
    auto targetA = m_springModel.GetStationRadius(node0);
    auto targetB = m_springModel.GetStationRadius(node1);
    double factor = 1.0;
    if (targetA.IsValid () || targetB.IsValid ())
        {
        double targetLength = targetA.Value () + targetB.Value ();
        DPoint3d xyz0 = node0->GetXYZ ();
        DPoint3d xyz1 = node1->GetXYZ ();
        double factor0;
        DoubleOps::SafeDivide (factor0, xyz0.Distance (xyz1), targetLength, 1.0);
        factor = m_edgeFunction.EvaluateRToR (factor0);
        }
    return factor;
    }
// return sum of spring factors for the two outbound edges.
double Evaluate (VuSetP graph, VuP nodeA)
    {
    VuP nodeB = nodeA->VSucc ();
    double factorA = EdgeFactor (nodeA);
    double factorB = EdgeFactor (nodeB);
    if (m_factorSelect == 1)
        {
        return sqrt (factorA * factorB);
        }
    else
        return 0.5 * (factorA * factorB);
    }
};

BCSSpringModel ()
    :   m_wallMask (VU_RULE_EDGE),
        m_fringeMask (VU_BOUNDARY_EDGE),
        m_fringeExteriorMask (VU_EXTERIOR_EDGE)
    {
    m_shortWallTolerance = 1.0e-6;
    m_totalWallPoints = 0;
    // userData1 is index to station array.  We trust vu will manage this carefully ...
    vu_setDefaultUserData1 (this, -1, true, false);
    }

// SpringModel member var:  array of stations.
bvector<StationData> m_station;
// Spring model member var: total number of points entered as walls.  (Coordinates are not retained -- they are in the VU graph)
// wall point count is used to guide edge splitting of exterior rectangle
size_t m_totalWallPoints;

// Spring model member var:   mask for "wall" edges
VuMask const m_wallMask;
// Spring model member var:     mask for (both sides of ) "fringe" (outer rectangle) edges
VuMask const m_fringeMask;
// Spring model member var:     mask for outside of fringe
VuMask const m_fringeExteriorMask;
// Spring model member var:     tolerance for ignoring wall edges.
double m_shortWallTolerance;

// MAIN PUBLIC ENTRIES TO ADD GEOMETRY ....
// Define a station point and its preferred distance from neighbors.
void AddStation (DPoint3dCR xyz, double radius)
    {
    m_station.push_back (StationData (xyz, radius));
    }
// Define a wall with the default short edge tolerance.
void AddWall (bvector<DPoint3d> const &xyz)
    {
    VuP chainTail, chainHead;
    VuOps::MakeEdges (this, chainTail, chainHead, xyz, m_wallMask, m_wallMask, m_shortWallTolerance);
    m_totalWallPoints += xyz.size ();
    }
// Define a wall with the specified short edge tolerance.
void AddWall (bvector<DPoint3d> const &xyz, double maxEdgeLength)
    {
    if (xyz.size () > 1)
        {
        bvector<DPoint3d> newXYZ;
        newXYZ.push_back (xyz[0]);
        for (size_t i = 1; i < xyz.size (); i++)
            {
            DPoint3d xyzA = newXYZ.back ();
            DPoint3d xyzB = xyz[i];
            double d = xyzA.Distance (xyzB);
            if (d > maxEdgeLength)
                {
                size_t n = (int)( 0.9999999 + d / maxEdgeLength);
                for (size_t k = 1; k < n; k++)
                    newXYZ.push_back (DPoint3d::FromInterpolate (xyzA, (double)k / (double)n, xyzB));
                }
            newXYZ.push_back (xyzB);
            }
        AddWall (newXYZ);
        }
    }

// Define a wall by two points.
void AddWall (DPoint3dCR xyzA, DPoint3dCR xyzB)
    {
    VuP chainTail, chainHead;
    VuOps::MakeEdge (this, chainTail, chainHead, xyzA, xyzB, m_wallMask, m_wallMask);
    m_totalWallPoints += 2;
    }
// Define a wall by two points by coordinates
void AddWall (double xA, double yA, double xB, double yB)
    {
    VuP chainTail, chainHead;
    VuOps::MakeEdge (this, chainTail, chainHead, DPoint3d::From (xA, yA, 0.0), DPoint3d::From (xB, yB, 0.0), m_wallMask, m_wallMask);
    m_totalWallPoints += 2;
    }
// find the station closest to xyz
ptrdiff_t ClosestStation (DPoint3dCR xyz, double tol)
    {
    for (size_t i = 0, n = m_station.size (); i < n; i++)
        {
        double d = xyz.DistanceSquaredXY (m_station[i].m_xyzBase);
        if (d < m_shortWallTolerance)
            return (ptrdiff_t)i;
        }
    return -1;
    }

private:
// Search the graph.  At each vertex, find the (index of) the closest station.  Store that index in the vertex.
void IndexStations ()
    {
    // uh oh .. the insertion does not clearly record which vertices are the stations.
    // Just (!!!) search the graph and stations ....
    VuMask visitMask = GrabMask ();
    ClearMaskInSet (visitMask);
    VU_SET_LOOP (seed, this)
        {
        if (!seed->HasMask (visitMask))
            {
            seed->SetMaskAroundVertex (visitMask);
            ptrdiff_t index = ClosestStation (seed->GetXYZ (), m_shortWallTolerance);
            VU_VERTEX_LOOP (node, seed)
                node->SetUserData1 (index);
            END_VU_VERTEX_LOOP (node, seed)
            }
        }
    END_VU_SET_LOOP (node, this)
    DropMask (visitMask);
    }

void MergeAndTriangulate ()
    {
    DRange3d range = Range ();
    for (auto &station : m_station)
        range.Extend (station.m_xyzBase);

    // Create linework clearly outside the walls
    VuP outsideNode = VuOps::AddExpandedRange (this, range, m_totalWallPoints + m_station.size (), 0.25, m_fringeMask, m_fringeExteriorMask);
    // TR #191917: keep dangling edges in triangulation
    vu_mergeOrUnionLoops (this, VUUNION_UNION);

    vu_regularizeGraph (this);
    outsideNode->SetMaskAroundFace (m_fringeExteriorMask);  // no need for the usual DFS -- we really know the outside. (And the walls might not close)
    vu_splitMonotoneFacesToEdgeLimit (this, 3);
    vu_flipTrianglesToImproveQuadraticAspectRatio (this);

    bvector<DPoint3d> stationBase;
    size_t numStation = m_station.size ();
    if (numStation > 0)
        {
        for (auto &station : m_station)
            stationBase.push_back (station.m_xyzBase);
        vu_insertAndRetriangulate (this, &stationBase[0], numStation, true);
        }

    IndexStations ();
    }
void ClearMaskAroundVerticesWithOutboundMask (VuMask maskToFind, VuMask maskToClear)
    {
    VU_SET_LOOP (vertexSeed, this)
        {
        if (vertexSeed->HasMask (maskToFind))
            {
            VU_VERTEX_LOOP (outboundNode, vertexSeed)
                {
                outboundNode->ClearMaskAroundEdge (maskToClear);
                }
            END_VU_VERTEX_LOOP (outboundNode, vertexSeed)
            }
        }
    END_VU_SET_LOOP (vertexSeed, this)
    }
public:
// Triangulate the station & walls data.
//  Optionally do smoothing to simulate springs ..

void SolveSprings (bool doSmoothing = true)
    {
    for (auto &station : m_station)
        {
        auto disk = CurveVector::CreateDisk (DEllipse3d::FromCenterRadiusXY (station.m_xyzBase, station.m_radius));
        Check::SaveTransformed (*disk);
        }
    SaveGraphEdges (Graph (), m_wallMask);

    MergeAndTriangulate ();

    Check::Shift (200, 0,0);
    SaveGraphEdges (Graph (), 0);

    Check::Shift (200,0,0);

    if (doSmoothing)
        {
        double shiftFraction;
        int numSweep;
        BCSSpringModel::TriangleWeightFunction::CappedQuadraticFunction edgeWeightFunction (0,0,1, 0, 10);
        static int s_springSelect = 0;
        BCSSpringModel::TriangleWeightFunction springFunction (*this, s_springSelect, edgeWeightFunction);
        vu_smoothInteriorVertices (Graph (), &springFunction, nullptr, 1.0e-4, 10, 100, 100, &shiftFraction, &numSweep);
        if (true )  // indent for scope
            {
            _VuSet::TempMask outputMask (Graph (), true);
            ClearMaskAroundVerticesWithOutboundMask (m_fringeExteriorMask, outputMask.Mask ());
            SaveGraphEdges (Graph (), outputMask.Mask ());
            }
        }
    }
// return polygon coordinates for areas around stations.
void CollectStationAreas (bvector<bvector<DPoint3d>> &areas, double shrinkFraction = 0.0)
    {
    _VuSet::TempMask visitMask (Graph (), false);
    areas.clear ();
    VU_SET_LOOP (vertexSeedNode, Graph ())
        {
        if (!vertexSeedNode->HasMask (visitMask.Mask ()))
            {
            vertexSeedNode->SetMaskAroundVertex (visitMask.Mask ());
            auto weight0 = GetStationRadius (vertexSeedNode);
            if (weight0.IsValid ())
                {
                DPoint3d xyz0 = vertexSeedNode->GetXYZ ();
                areas.push_back (bvector<DPoint3d> ());
                VU_VERTEX_LOOP (node0, vertexSeedNode)
                    {
                    VuP node1 = node0->FSucc ();
                    // Always output a point on the edge ...
                    DPoint3d xyz1 = node1->GetXYZ ();
                    auto weight1 = GetStationRadius (node1);
                    auto xyz = DPoint3d::FromWeightedAverage (xyz0, weight1.Value (), xyz1, weight0.Value ());    // reverse weights to push away from heavy end
                    areas.back ().push_back (xyz);

                    // If it is a triangle of stations, also output weighted centroid
                    VuP node2 = node1->FSucc ();
                    auto weight2 = GetStationRadius (node2);
                    VuP node3 = node2->FSucc ();
                    if (node3 == node0 && weight1.IsValid () && weight2.IsValid ())
                        {
                        DPoint3d xyz2 = node2->GetXYZ ();
                        auto xyz = DPoint3d::FromWeightedAverage (
                                xyz0, weight1.Value () + weight2.Value (),
                                xyz1, weight2.Value () + weight0.Value (),
                                xyz2, weight0.Value () + weight1.Value ()
                                );
                        areas.back ().push_back (xyz);
                        }


                    }
                END_VU_VERTEX_LOOP (node0, vertexSeedNode)
                DPoint3d xyz = areas.back ().front ();
                areas.back ().push_back (xyz);
                if (shrinkFraction != 0.0)
                    {
                    for (auto &xyz : areas.back ())
                        xyz = DPoint3d::FromInterpolate (xyz, shrinkFraction, xyz0);
                    }
                }
            }
        }
    END_VU_SET_LOOP (vertexSeedNode, Graph ())
    }
};


void SaveZones (bvector<DPoint3d> &wall, BCSSpringModel &sm)
    {
    Check::Shift (200, 0,0);
    Check::SaveTransformed (wall);
    bvector<bvector<DPoint3d>> zones;
    sm.CollectStationAreas (zones, 0.01);
    for (auto &zone : zones)
        Check::SaveTransformed (zone);
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
    bvector<DPoint3d> isolatedPoints;
    bvector<double> uBreaks;
    bvector<double> vBreaks;
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

        // successively grow each space to larger and larger multiples of the original ....
        queries.SaveWalls ();
        double dz = 0.2;
        Check::Shift (0,0,10.0 * dz);
        queries.SaveSpaceBoundaries ();
        Check::Shift (0,0,-dz);     // smallest area comes out at top for downward code effect.

        for (double spaceFactor : bvector<double>{1.0, 1.5, 2.0, 2.5, 3.0, 3.5})
            {
            for (size_t i = 0; i < baseAreas.size (); i++)
                {
                flooder.ExpandSingleSpaceIdToTargetArea (spaceIds[i], spaceFactor * baseAreas[i]);
                }
            queries.SaveSpaceBoundaries ();
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

