

struct IndexTriangle
{
int m_vertexIndexA;
int m_vertexIndexB;
int m_vertexIndexC;
MTGNodeId m_node;

IndexTriangle ()
    : m_vertexIndexA(-1),
      m_vertexIndexB(-1),
      m_vertexIndexC(-1),
      m_node(MTG_NULL_NODEID)
    {}

IndexTriangle (
int vertexIndexA,
int vertexIndexB,
int vertexIndexC,
MTGNodeId nodeA
)
    : m_vertexIndexA (vertexIndexA),
      m_vertexIndexB (vertexIndexB),
      m_vertexIndexC (vertexIndexC),
      m_node(nodeA)
    {
    }
// sort comparison -- lexical in A B C order.
bool operator < (IndexTriangle const &other) const
    {
    if (m_vertexIndexA < other.m_vertexIndexA)
        return true;
    if (m_vertexIndexA > other.m_vertexIndexA)
        return false;

    if (m_vertexIndexB < other.m_vertexIndexB)
        return true;
    if (m_vertexIndexB > other.m_vertexIndexB)
        return false;

    if (m_vertexIndexC < other.m_vertexIndexC)
        return true;
    if (m_vertexIndexC > other.m_vertexIndexC)
        return false;
    return false; // all equal
    }
// Create an index triangle with:
//  a) indices are sorted for simple lexical comparison
//  b) retain the MTGNodeId for only the lowest index.
static bool CreateSortable
(
int vertexIndexA,
MTGNodeId nodeA,
int vertexIndexB, 
MTGNodeId nodeB,
int vertexIndexC,
MTGNodeId nodeC,
IndexTriangle &data
)
    {
    if (vertexIndexA > vertexIndexB)
        {
        std::swap (vertexIndexA, vertexIndexB);
        std::swap (nodeA, nodeB);
        }

    if (vertexIndexA > vertexIndexC)
        {
        std::swap (vertexIndexA, vertexIndexC);
        std::swap (nodeA, nodeC);
        }

    if (vertexIndexB > vertexIndexC)
        {
        std::swap (vertexIndexB, vertexIndexC);
        std::swap (nodeB, nodeC);
        }

    if (vertexIndexA == vertexIndexB || vertexIndexB == vertexIndexC)
        return false;

    data = IndexTriangle (vertexIndexA, vertexIndexB, vertexIndexC, nodeA);
    return true;
    }
};

// CONVENTION:  The MTGGraph f,v loops are "as seen from inside the tet looking at its inner faces".
// View of the 4 faces unfolded:
//               7
//      C----------D
//   /10|4\3     11|9
//11/   |  \       |
//D7    |    \     |
//9\.   |     \    |
//  \   |      \   |
//   \  |       \  |
//    \5| 0-->  2\8|
//     \A---------B
//       \6  <-- 1|
//        \      /
//         \    /
//         7\9 /11
//           D

#define VERTEX__A 0
#define VERTEX__B 1
#define VERTEX__C 2
#define VERTEX__D 3

static int s_vertexIndexAtTetNode[12] =
    {
    VERTEX__A, VERTEX__B,
    VERTEX__B, VERTEX__C,
    VERTEX__C, VERTEX__A, 
    VERTEX__A, VERTEX__D,
    VERTEX__B, VERTEX__D,
    VERTEX__C, VERTEX__D,
    };
    
// The tet is created as floating edges.
// These are twisted together in this order (to ensure genus==2!!!)
static int s_twistOrder[8][2] =
    {
    {1,2},
    {3,4},
    {5,0},
    {5,6},
    {1,8},
    {3,10},
    {7,9},
    {11,7}
    };

// for each of the 4 vertices in user order, an index of a node at that
// vertex.  There are 3 nodes at each vertex -- this list is chosen
// so there is one node in each face.
static int s_nodeAtVertex [4] = {0, 1, 3, 7};



/*=========================================================================
// MTG Graph structure for a multicellular tetrahedral solid.
// (Only the assembly sequence relies on being tetrahedral -- traversals are still valid
//    if interior faces are missing.  Other assembly sequences in the future might produce non-tetrahedral complexes)
//
// From an interior point of any cell, the cell's surface appears to be a manifold.
// The topology (adjacency) of that manifold is carried in the conventional
// MTGGraph FSucc and VSucc.  FSucc walks around a face in forward (CCW) direction,
//    VSucc walks around a vertex in the same direction.
// Each MTG node is a "spot" sitting in the corner of the face.
//
// At the corresponding corner on the OTHER SIDE of the same face (i.e. a 
// neighboring tetrahedron) there is another MTG node. These two paired nodes
// are "FaceMate" nodes.  The MultiCellularTopology object reports this with the
// FaceMate method.
//
// If a cell is a tetrahedron:
//   There are 4 faces and 4 vertices.  The faces are triangles.
//   Each vertex has 3 incident triangles
//   Each vertx has 3 incident edges.
//   There are 12 nodes "inside".  There is one node in each corner of each triangle.
//   Each node's FSucc ("FaceSuccessor") leads to the next corner of its triangle.
/       Note that the "arrow" from a node to its successor is to the left of its edge.
//   Each node's VSucc ("VertexSuccessr") leads to the next node around its vertex.
//
// 
// The full construction sequence is:
//    MTGMultiCellularTetrahedralTopology graph;
//    for each tetrhedron
//        {
//        graph.AddTetrahedron (vertexIndexA, vertexIndexB, vertexIndexC, vertexIndexD);
//        }
//    if (!graph.WrapExteriorGraphAroundCompleteInteriorGraph ())
//        {
//        // hm... this indicates assembly failure.   Tell me if it happens ...
//        }
*=========================================================================*/
struct MTGMultiCellularTetrahedralTopology : MTGGraph
{
private:
static const int s_defaultVertexIndex = -1;
static const int s_defaultPartnerIndex = MTG_NULL_NODEID;

int m_vertexLabelOffset;
int m_partnerLabelOffset;
// During construction, faces that have NOT YET BEEN partnered are
// kept in m_faces as vertexIndex triples.
bset <IndexTriangle> m_faces;

public:
MTGMultiCellularTetrahedralTopology () : MTGGraph()
    {
    m_vertexLabelOffset = DefineLabel (1000, MTG_LabelMask_VertexProperty, s_defaultVertexIndex);
    m_partnerLabelOffset = DefineLabel (1001, MTG_LabelMask_SectorProperty, s_defaultPartnerIndex);
    }

MTGNodeId FaceMate (MTGNodeId node)
    {
    MTGNodeId mate;
    if (TryGetLabel (node, m_partnerLabelOffset, mate))
        return mate;
    return MTG_NULL_NODEID;
    }

// Set the vertexIndex (label value) in a node.
public: void SetVertexIndex (MTGNodeId node, int vertexIndex)
    {
    TrySetLabel (node, m_vertexLabelOffset, vertexIndex);
    }

// Set face mate pointers between given nodes.
// return false if FaceMates were anything other than null before.
bool SetFaceMates (MTGNodeId nodeA, MTGNodeId nodeB)
    {
    MTGNodeId mateA = FaceMate(nodeA);
    MTGNodeId mateB = FaceMate(nodeB);
    bool ok = TrySetLabel (nodeA, m_partnerLabelOffset, nodeB)
         && TrySetLabel (nodeB, m_partnerLabelOffset, nodeA)
         && mateA == MTG_NULL_NODEID
         && mateB == MTG_NULL_NODEID;
    return ok;
    }

// from starting node A (in corner of a triangle of a tetrahedron)
// find nodes B, C, D which collectively represent each face and vertex of the tetrahedron.
void GetTetrahedralRepresentativeNodes
(
MTGNodeId nodeA,
MTGNodeId &nodeB,
MTGNodeId &nodeC,
MTGNodeId &nodeD
)
    {
    nodeB = VSucc (FSucc (nodeA));
    nodeC = FSucc (VSucc (nodeA));
    nodeD = FSucc (VSucc (nodeB));
    }
// Return true if anyNodeOnSharedFace is the shared face of a interior tetrahedron.
// 
// If fullTetrahedralTest is false, the only checking is for exterior mask and triangular shared face.
// if fullTetrahedralTest is true, full 12-node closure tests are performed on both sides.
bool GetAdjacentTetrahedraVertexIndices
(
MTGNodeId anyNodeOnSharedFace,
int &vertexIndexA,
int &vertexIndexB,
int &vertexIndexC,
int &vertexIndexD1,
int &vertecIndexD2,
bool fullTetrahedralTest = true
)
    {
    // suffix "1" and "2" are the two tetrahedra ..
    MTGNodeId nodeA1 = anyNodeOnSharedFace;
    MTGNodeId nodeA2 = FaceMate (nodeA1);
    MTGNodeId nodeB1 = FSucc(nodeA1);
    MTGNodeId nodeC1 = FSucc (nodeB1);

    MTGNodeId nodeD1 = FSucc (VSucc (VSucc (nodeA1)));
    MTGNodeId nodeD2 = FSucc (VSucc (VSucc (nodeA2)));
    // These are well defined accesses even if not a tetrahedron ..
    TryGetVertexIndexAtNode (nodeA1, vertexIndexA);
    TryGetVertexIndexAtNode (nodeB1, vertexIndexB);
    TryGetVertexIndexAtNode (nodeC1, vertexIndexC);
    TryGetVertexIndexAtNode (nodeD1, vertexIndexD1);
    TryGetVertexIndexAtNode (nodeD2, vertecIndexD2);

    if (IsExteriorNode (nodeA1) || IsExteriorNode (nodeA2))
        return false;
    if (fullTetrahedralTest)
        return IsTetrahedralVolume (nodeA1) && IsTetrahedralVolume (nodeA2);

    if (FSucc (nodeC1) != nodeA1)
        return false;
    MTGNodeId nodeB2 = FSucc (nodeA2);
    MTGNodeId nodeC2 = FSucc (nodeB2);
    if (FSucc (nodeC2) != nodeA2)
        return false;
    return true;
    }

// Test if the face loop has only 3 nodes.
bool Is3NodeFaceLoop (MTGNodeId nodeA)
    {
    return nodeA == FSucc(FSucc(FSucc(nodeA)));
    }

// Test if the face loop has only 3 nodes.
bool Is3NodeVertexLoop (MTGNodeId nodeA)
    {
    return nodeA == VSucc(VSucc(VSucc(nodeA)));
    }


// Test if nodeA is an interior node of a tetrahedron.
bool IsTetrahedralVolume (MTGNodeId nodeA)
    {
    MTGNodeId nodeB = FSucc (nodeA);
    MTGNodeId nodeC = FSucc (nodeB);
    if (FSucc (nodeC) != nodeA)
        return false;
    // move to opposite side of edges ...
    MTGNodeId nodeA1 = VSucc (nodeA);
    MTGNodeId nodeB1 = VSucc (nodeB);
    MTGNodeId nodeC1 = VSucc (nodeC);
    // move to off-plane edges
    MTGNodeId nodeA2 = VSucc (nodeA1);
    MTGNodeId nodeB2 = VSucc (nodeB1);
    MTGNodeId nodeC2 = VSucc (nodeC1);
    // move to far vertex
    MTGNodeId nodeA3 = FSucc (nodeA2);
    MTGNodeId nodeB3 = FSucc (nodeB2);
    MTGNodeId nodeC3 = FSucc (nodeC2);
    return nodeA3 == VSucc (nodeB3)
        && nodeB3 == VSucc (nodeC3)
        && nodeC3 == VSucc (nodeA3);
    }

// Add an interior loop for a single tetrahedron.
// This is only to be called during a construction phase.
// This makes 12 MTG nodes (3 per face)
// The returned nodes are (at each vertex A,B,C,D) one of the 3 MTG nodes at that
// vertex.  They are chosen (from the 12 total nodes) so that there is one
// node from each face loop and one from each vertex loop.
// Expected orientation: Triangle 012 is the CCW from inside. 3 is "above" this plane.
void AddTetrahedron
(
int vertexIndexA,
int vertexIndexB,
int vertexIndexC,
int vertexIndexD,
MTGNodeId &nodeA,
MTGNodeId &nodeB,
MTGNodeId &nodeC,
MTGNodeId &nodeD
)
    {
    int vertexIndex[4] = {vertexIndexA, vertexIndexB, vertexIndexC, vertexIndexD};
    // Stitch the 6 edges of the tetrhedron
    MTGNodeId nodes[12];

    for (int i = 0; i < 12; i+= 2)
        {
        CreateEdge (nodes[i], nodes[i+1]);
        TrySetLabel (nodes[i], m_vertexLabelOffset, vertexIndex[s_vertexIndexAtTetNode[i]]);
        TrySetLabel (nodes[i+1], m_vertexLabelOffset, vertexIndex[s_vertexIndexAtTetNode[i+1]]);
        }
    nodeA = nodes [s_nodeAtVertex [0]];
    nodeB = nodes [s_nodeAtVertex [1]];
    nodeC = nodes [s_nodeAtVertex [2]];
    nodeD = nodes [s_nodeAtVertex [3]];

    for (int i = 0; i < 8; i++)
        VertexTwist (nodes[s_twistOrder[i][0]], nodes[s_twistOrder[i][1]]);
        
    IndexTriangle newFaces[4];
    IndexTriangle::CreateSortable (vertexIndexA, nodes[0], vertexIndexB, nodes[2], vertexIndexC, nodes[4], newFaces[0]);
    IndexTriangle::CreateSortable (vertexIndexC, nodes[10], vertexIndexD, nodes[7], vertexIndexA, nodes[5], newFaces[1]);
    IndexTriangle::CreateSortable (vertexIndexB, nodes[1], vertexIndexA, nodes[6], vertexIndexD, nodes[9], newFaces[2]);
    IndexTriangle::CreateSortable (vertexIndexB, nodes[8], vertexIndexD, nodes[11], vertexIndexC, nodes[3], newFaces[3]);

    for (int i = 0; i < 4; i++)
        {
        bset<IndexTriangle>::iterator oldFace = m_faces.find (newFaces[i]);
        if (oldFace == m_faces.end ())
            {
            m_faces.insert (newFaces[i]);
            }
        else
            {
            IndexTriangle partner = *oldFace;
            MTGNodeId nodeA = newFaces[i].m_node;
            MTGNodeId nodeB = partner.m_node;
            m_faces.erase (partner);
            // Each face has 3 nodes.
            // Walk side-by-side, moving to FPred or FSucc on respective sides.
            for (int i = 0; i < 3; i++, nodeA = FSucc (nodeA), nodeB = FPred (nodeB))
                {
                SetFaceMates (nodeA, nodeB);
                }
            }
        }
    }

// Add a tetrahedron.   This is a short arg list for callers that don't need to 
// know nodeId's returned.
// Expected orientation: Triangle 012 is the CCW from inside. 3 is "above" this plane.
void AddTetrahedron
(
int vertexIndexA,
int vertexIndexB,
int vertexIndexC,
int vertexIndexD
)
    {
    MTGNodeId nodeA, nodeB, nodeC, nodeD;
    AddTetrahedron (vertexIndexA, vertexIndexB, vertexIndexC, vertexIndexD,
          nodeA, nodeB, nodeC, nodeD);
    }

// Try to retrieve the vertex index at a node.
bool TryGetVertexIndexAtNode (MTGNodeId nodeId, int &vertexIndex)
    {
    return TryGetLabel (nodeId, m_vertexLabelOffset, vertexIndex);
    }

// Ask if this is an exterior node.
bool IsExteriorNode (MTGNodeId nodeId)
    {
    return MTG_NULL_MASK != GetMaskAt (nodeId, MTG_EXTERIOR_MASK);
    }

// Ask if this node or its partner through the face is an exterior node.
bool IsExteriorNodeOrFaceMate (MTGNodeId nodeId)
    {
    return MTG_NULL_MASK != GetMaskAt (nodeId, MTG_EXTERIOR_MASK)
        || MTG_NULL_MASK != GetMaskAt (FaceMate (nodeId), MTG_EXTERIOR_MASK);
    }

public:
// Collect one representative per face loop.
// @param includeBothSides if true, a separate node is reported for each side of each two sided face.
//             if false, only one representative is included (and you can get to the other with the FaceMate () method)
// @param includeInterior if true interior loops are included.
// @param includeExterior if true exterior loops are included.
void CollectDoubleSidedFaces (bvector<MTGNodeId> &nodes, bool includeBothSides)
    {
    MTGMask visitMask = GrabMask ();
    ClearMask (visitMask);
    MTGARRAY_SET_LOOP (seed, this)
        {
        if (!GetMaskAt (seed, visitMask))
            {
            MTGNodeId mate = FaceMate(seed);
            SetMaskAroundFace (seed, visitMask);
            SetMaskAroundFace (mate, visitMask);
            if (includeBothSides)
                {
                nodes.push_back (seed);
                nodes.push_back (mate);
                }
            else
                {
                // only want one side .. favor interior ...
                if (IsExteriorNode (seed))
                    nodes.push_back (mate);
                else
                    nodes.push_back (seed);
                }
            }
        }
    MTGARRAY_END_SET_LOOP (seed, this)
    DropMask (visitMask);
    }

// Alternately take VSucc and FaceMate steps until encountering (a) a null node (b) a node marked with visitMask
// returns terminalNode = the null or previously visited node
//         lastInteriorNode = the node for which terminalNode = graph.FaceMate (lastInteriorNode)
//   (except if the seed is visited, terminal is seed and lastInteriorNode is null.)
// This is only expected to be valid during exterior loop completion, with seedNodeId being an interior node
//   whose FaceMate is null.
void MarkVSuccFaceMateChain (MTGNodeId seedNodeId, MTGMask visitMask, MTGNodeId &lastInteriorNode, MTGNodeId &terminalNode, size_t &chainLength)
    {
    lastInteriorNode = MTG_NULL_NODEID;
    terminalNode = seedNodeId;
    chainLength = 0;
    while (MTG_NULL_NODEID != terminalNode && !GetMaskAt (terminalNode, visitMask))
        {
        SetMaskAt (terminalNode, visitMask);
        lastInteriorNode = VSucc (terminalNode);
        terminalNode = FaceMate (lastInteriorNode);
        chainLength++;
        }
    }

// Alternately take VPred and FaceMate steps until encountering (a) a null node (b) a node marked with visitMask
// returns terminalNode = the null or previously visited node
//         lastInteriorNode = the node for which terminalNode = graph.FaceMate (lastInteriorNode)
//   (except if the seed is visited, terminal is seed and lastInteriorNode is null.)
// This is only expected to be valid during exterior loop completion, with seedNodeId being an interior node
//   whose FaceMate is null.
void MarkVPredFaceMateChain (MTGNodeId seedNodeId, MTGMask visitMask, MTGNodeId &lastInteriorNode, MTGNodeId &terminalNode, size_t &chainLength)
    {
    lastInteriorNode = MTG_NULL_NODEID;
    terminalNode = seedNodeId;
    chainLength = 0;
    while (MTG_NULL_NODEID != terminalNode && !GetMaskAt (terminalNode, visitMask))
        {
        SetMaskAt (terminalNode, visitMask);
        lastInteriorNode = VPred (terminalNode);
        terminalNode = FaceMate (lastInteriorNode);
        chainLength++;
        }
    }

// ON INPUT:
//  1) the graph has been assembled from interior volumes.
//  2) Each interior volume, as viewed from its interior, is a 2-manifold.
//  3) The 2-manifold for each interior volume is a connected component of
//      the overall MTGraph of the MultiCellularTopology object.
//  4) Mating faces of these volumes have been properly stitched via the
//       FaceMate label.
//  5) On any face that does not have a mating face, all FaceMate's are null.
//  6) i.e. on true exterior faces (that back side of unmated faces) there is
//      no graph structure.
// Naive algorithm:
//  1) build nodes "opposite" each unmated face.
//  2) join node to its edge mate that is "across the edge" on an adjacent true
//      exterior face.
//  3) Remark: That cross edge mate can be found by following VSuccFaceMate chains
//      on the inside.
//  4) This violates normal MTGGraph construction because the instant you try
//   to build those nodes for a single face, there is no edge mate.
// So....
//  1) work by exterior EDGE rather than face.
//  2) call a node "interiorBoundary" if it is part of the given (interior) complex
//     but has no FaceMate.
//  3) use the VSuccFaceMate chain to find 
// On output:
//  1) Every node has a FaceMate
//  2) True extrior nodes have MTG_EXTERIOR_MASK set.
bool WrapExteriorGraphAroundCompleteInteriorGraph ()
    {
    MTGMask vSuccMask = GrabMask ();
    ClearMask (vSuccMask);
    ClearMask (MTG_EXTERIOR_MASK);
    size_t numErrors = 0;
    // Here "interior" means a node "on the inside" but just adjacent to exterior.
    // build up pairs that are "mates" in the extended sense of sharing a true exterior edge, but reachable only
    // by jumping through an interior chain
    bvector<MTGNodeIdPair> interiorPairs;

    MTGARRAY_SET_LOOP (interiorA, this)
        {
        if (MTG_NULL_NODEID == FaceMate (interiorA) && !GetMaskAt (interiorA, vSuccMask))
            {
            // A,B,C,D all have no face mate.
            // interiorA,interiorC are "mates" for A.VStar.F.VStar.FLoop -- where VStar means multiple VSuccFaceMate
            // steps, i.e. ignore interior distractions.
            MTGNodeId terminalNodeC, terminalNodeA, interiorB, interiorC, interiorD;
            size_t chainLengthA, chainLengthC;
            MarkVSuccFaceMateChain (interiorA, vSuccMask, interiorB, terminalNodeA, chainLengthA);
            interiorC = FSucc (interiorB);
            MarkVSuccFaceMateChain (interiorC, vSuccMask, interiorD, terminalNodeC, chainLengthC);
            if (chainLengthA != chainLengthC)
                numErrors++;
            MTGNodeId interiorE = FSucc (interiorD);
            if (interiorE != interiorA)
                numErrors++;
            if (terminalNodeA != MTG_NULL_NODEID || terminalNodeC != MTG_NULL_NODEID)
                numErrors++;
            MTGNodeIdPair pair;
            // nb there is a constructor, but just in case it's not on Vancouver , do the assignments ...
            pair.nodeId[0] = interiorA;
            pair.nodeId[1] = interiorC;
            interiorPairs.push_back (pair);
            }
        }
    MTGARRAY_END_SET_LOOP (interiorA, this)

    // Make exterior edges ...
    for (MTGNodeIdPair interiorPair : interiorPairs)
        {
        int vertexIndexA, vertexIndexB;
        MTGNodeId interiorA = interiorPair.nodeId[0];
        MTGNodeId interiorB = interiorPair.nodeId[1];
        TryGetVertexIndexAtNode (interiorA, vertexIndexA);
        TryGetVertexIndexAtNode (interiorB, vertexIndexB);
        MTGNodeId exteriorA, exteriorB;
        CreateEdge (exteriorA, exteriorB);
        SetVertexIndex (exteriorA, vertexIndexA);
        SetVertexIndex (exteriorB, vertexIndexB);
        SetMaskAt (exteriorA, MTG_EXTERIOR_MASK);
        SetMaskAt (exteriorB, MTG_EXTERIOR_MASK);
        SetFaceMates (interiorA, exteriorA);
        SetFaceMates (interiorB, exteriorB);
        }

    // join vertex loops ...
    // Make exterior edges ...
    // mostly: from each exteriorA navigate:
    //     through face to interiorA
    //     along edge (inside!!!) to interiorB
    //     through face to exteriorB
    //     along edge to exteriorC
    //  exteriorC should become (by twisting) VSucc of exteriorA.
    // .... but in a vertx loop with n nodes (and n edges), the first
    //   n-1 twistst that happen will finish it all, and the last is unneeded.
    for (MTGNodeIdPair interiorPair : interiorPairs)
        {
        for (int i = 0; i < 2; i++)
            {
            auto interiorA = interiorPair.nodeId[i];
            auto exteriorA = FaceMate (interiorA);
            auto interiorB = FSucc (interiorA);
            auto exteriorB = FaceMate (interiorB);
            auto exteriorC = FSucc (exteriorB);
            if (!GetMaskAt (exteriorA, MTG_EXTERIOR_MASK))
                numErrors++;
            if (!GetMaskAt (exteriorC, MTG_EXTERIOR_MASK))
                numErrors++;
            if (VSucc (exteriorA) != exteriorC)
                VertexTwist (exteriorA, exteriorC);
            }
        }

    DropMask (vSuccMask);
    return numErrors == 0;
    }
};