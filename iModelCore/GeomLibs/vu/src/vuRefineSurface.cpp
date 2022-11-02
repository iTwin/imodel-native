/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//  Pair with (VuP, double)
struct VuPD
{
VuP m_node;
double m_a;
VuPD (VuP node, double a) : m_node (node), m_a (a)
  {
  }
};

struct VuSurfaceRefinementContext
{
VuSetP            m_graph;
VuEdgeSubdivisionTestFunction &m_surfaceTester;

VuSurfaceRefinementContext (VuSetP m_graph, VuEdgeSubdivisionTestFunction &surfaceTester)
    : m_graph (m_graph),
    m_surfaceTester (surfaceTester)
    {
    }
~VuSurfaceRefinementContext ()
    {
    }

bvector<VuPD> m_edgesToSplit;
VuMask       m_splitEdgeMask;
// Collect one representative of each edge that should be split.

void MarkEdgesToSplit (VuMask splitMask)
    {
    static double s_minimumSplitCount = 1.0000001;
    DRange1d range;
    range.InitNull ();
    VuMask visitMask = vu_grabMask (m_graph);
    VuMask barrierMask = visitMask | VU_ALL_FIXED_EDGES_MASK;
    m_edgesToSplit.clear ();
    vu_clearMaskInSet (m_graph, visitMask);
    vu_clearMaskInSet (m_graph, splitMask);
    VU_SET_LOOP (nodeA, m_graph)
        {
        if (!vu_getMask (nodeA, barrierMask))
            {
            VuP nodeB = vu_edgeMate (nodeA);
            vu_setMask (nodeA, visitMask);
            vu_setMask (nodeB, visitMask);

            DPoint2d uvA, uvB;
            vu_getDPoint2d (&uvA, nodeA);
            vu_getDPoint2d (&uvB, nodeB);
            double requiredEdges = m_surfaceTester.ComputeNumEdgesRequired (uvA, uvB);
            range.Extend (requiredEdges);
            if (requiredEdges > s_minimumSplitCount)
                {
                vu_setMask (nodeA, splitMask);
                vu_setMask (nodeB, splitMask);
                }
            }
        }
    END_VU_SET_LOOP (nodeA, m_graph)
    vu_returnMask (m_graph, visitMask);
    }

static void CountEdgesInFace (VuP seed, VuMask mask, int &numMasked, int &numTotal)
    {
    numMasked = numTotal = 0;
    VU_FACE_LOOP (node, seed)
        {
        numTotal++;
        if (vu_getMask (node, mask))
            numMasked++;
        }
    END_VU_FACE_LOOP (node, seed)
    }

static void CountEdgesInFace (VuP seed, VuMask mask, int &numMasked, int &numTotal, VuP &lastMaskedNode, VuP &lastUnMaskedNode)
    {
    numMasked = numTotal = 0;
    VU_FACE_LOOP (node, seed)
        {
        numTotal++;
        if (vu_getMask (node, mask))
            {
            numMasked++;
            lastMaskedNode = node;
            }
        else
            {
            lastUnMaskedNode = node;
            }
        }
    END_VU_FACE_LOOP (node, seed)
    }

// A "TwoSplitTriangle" is a triangle with 2 edges marked for split.
// Within a row of such triangles, splitting all the marked edges would create
//    twice as many points as needed.
// Stepping from one to the next is subtle.
// In the figure, the long, splittable edges run verticallly.  All horizontal edges are non-splittable.
// The prefered node for any such triangle is the one between the splittable edges
// ----------X---------Y---Z---U--------
//         / x \     / y\  |  /
//        /     \ b /    \c|q/
// ------A--------B--------C----------
// If sitting in a sector along the bottom edge a move to the triangle to the right can happen in two ways:
//  From (b):  y is the edge mate of b.
//  From (c):  q is the vertex predecessor of c.
// Similarly for left moved:
//  From q: c is the vertex successor of q.
//  From b: x is the reverse edge mate of b.
// From the preferred start node, only zero or one of the two left steps can end up in another TwoSplitTriangle.  Likewise for right steps.
// In order to continue moving "to the right" (relative to the picture) we must know whether the current node is at the 
//  bottom or the top, and choose (edgeMate, vpred) or (reverseEdgeMate, vsucc) as the stepping options.
// A vertex step leaves this unchanged.  An edge step reverses it.
// Beware of the distinction between global left/right and local left/right.
//  (local is as seen when on the preferred node and looking into the triangle -- i.e. between the two long edges.
//
struct VuDirectedStepper
{
private:
VuP m_currentNode;
// indicates the preferred stepping direction as viewed locally from the currentNode:
bool m_steppingRight;
// mask that is set on the home vertex in each triangle. (And NOT set on either of the other two)
VuMask m_homeNodeMask;

// Caller suggests a candidate and indicates if taking this step would shift to the "other" side
//  in the global view.
// If the candidate is a home node, accept it and record the reversal.
bool TryAdvance (VuP candidate, bool reverse)
    {
    if (vu_getMask (candidate, m_homeNodeMask))
        {
        m_currentNode = candidate;
        if (reverse)
            m_steppingRight = !m_steppingRight;
        return true;
        }
    return false;        
    }    


public:

VuP Node () const { return m_currentNode;}
bool IsSteppingRight () const { return m_steppingRight;}
void ReverseDirection () {m_steppingRight = !m_steppingRight;}
VuP TrailingEdgeNode () const
    {
    if (m_steppingRight)
        return m_currentNode;
    return vu_fpred (m_currentNode);
    }

VuP LeadingEdgeNode () const
    {
    if (m_steppingRight)
        return vu_fpred (m_currentNode);
    return m_currentNode;
    }

VuDirectedStepper (VuP startNode, VuMask mask, bool startSteppingRight = true)
    {
    m_currentNode = startNode;
    m_steppingRight = startSteppingRight;
    m_homeNodeMask = mask;
    }

void ClearCurrentNodeMask ()
    {
    vu_clrMask (m_currentNode, m_homeNodeMask);
    }
        
bool MoveToNeighbor ()
    {
    if (m_steppingRight)
        {
        if (TryAdvance (vu_vpred (m_currentNode), false))
            return true;
        if (TryAdvance (vu_edgeMate (m_currentNode), true))
            return true;
        return false;
        }
    else
        {
        if (TryAdvance (vu_vsucc (m_currentNode), false))
            return true;
        if (TryAdvance (vu_reverseEdgeMate (m_currentNode), true))
            return true;
        return false;
        }
    }

VuP VertexNeighbor () const { return m_steppingRight ? vu_vpred (m_currentNode) : vu_vsucc(m_currentNode);}
VuP EdgeNeighbor () const { return m_steppingRight ? vu_edgeMate (m_currentNode) : vu_reverseEdgeMate (m_currentNode);}

// Count the number of vertex steps (in appropriate direction) that are preferred vertices.
int CountVertexNeighbors () const
    {
    // This walks around a vertex, hence must return to m_currentNode.
    VuP walker = m_currentNode;
    int count = 0;
    for (;;count++)
        {
        walker = VertexNeighbor ();
        if (walker == m_currentNode || !vu_getMask (walker, m_homeNodeMask))
            break;
        } 
    return count;
    }
    
// Count the number of edge steps (in appropriate direction) that are preferred vertices.
int CountEdgeNeighbors () const
    {
    // Do I know that this will terminate?
    VuP walker = m_currentNode;
    int count = 0;
    for (;;count++)
        {
        walker = EdgeNeighbor ();
        if (walker == m_currentNode || !vu_getMask (walker, m_homeNodeMask))
            break;
        } 
    return count;
    }


// Take multiple steps until reaching end of chain or return to start.    
int MoveToEndOfChain ()
    {
    VuP startNode = m_currentNode;
    int numStep = 0;
    while (MoveToNeighbor () && m_currentNode != startNode)
        {
        numStep++;
        }
    return numStep;        
    }
};


// Find an masked node that has an unmasked successor
static VuP Find10Transition (VuP seed, VuMask mask)
    {
    VU_FACE_LOOP (nodeA, seed)
        {
        if (vu_getMask (nodeA, mask) && !vu_getMask (vu_fsucc (nodeA), mask))
          return nodeA;
        }
    END_VU_FACE_LOOP (nodeA, seed)
    return NULL;
    }

// Find an masked node that has an unmasked successor
static VuP Find01Transition (VuP seed, VuMask mask)
    {
    VU_FACE_LOOP (nodeA, seed)
        {
        if (!vu_getMask (nodeA, mask) && vu_getMask (vu_fsucc (nodeA), mask))
          return nodeA;
        }
    END_VU_FACE_LOOP (nodeA, seed)
    return NULL;
    }

// Find an masked node that has an masked successor
static VuP Find11Transition (VuP seed, VuMask mask)
    {
    VU_FACE_LOOP (nodeA, seed)
        {
        if (vu_getMask (nodeA, mask) && vu_getMask (vu_fsucc (nodeA), mask))
          return nodeA;
        }
    END_VU_FACE_LOOP (nodeA, seed)
    return NULL;
    }


// If an edge is the only splittable edge in either of its faces, record it for splitting.
// If all three edges in a triangle are splittable, record all.
// (fans of triangles with 2 splits along the fan need special treatment ...)
size_t SplitEdges13 ()
    {
//    static double s_minimumSplitCount = 1.0000001;
    VuMask visitMask = vu_grabMask (m_graph);
    VuMask twoSplitMask = vu_grabMask (m_graph);
    VuMask newVertexMask = vu_grabMask (m_graph);
    VuMask splitEdgeMask = vu_grabMask (m_graph);
    vu_clearMaskInSet (m_graph, splitEdgeMask);
    // Apply splitMask to all edges that are identified by the split function.
    // (But we can't just blast ahead and split the all -- other context will control that)
    MarkEdgesToSplit (splitEdgeMask);

    // Narrow down to edges actually splittable ...
    VuMarkedEdgeSetP edgesToSplit = vu_markedEdgeSetNew (m_graph, 0);
    vu_clearMaskInSet (m_graph, newVertexMask);
    vu_clearMaskInSet (m_graph, visitMask);
    vu_clearMaskInSet (m_graph, twoSplitMask);
    int numA0 = 0;
    int numA1 = 0;
    int numA2 = 0;
    int numA3 = 0;
    VU_SET_LOOP (nodeA, m_graph)
        {
        if (!vu_getMask (nodeA, visitMask))
            {
            vu_setMaskAroundFace (nodeA, visitMask);
            int numSplit, numTotal, numSplitB, numTotalB;
            VuP splitA, splitB;
            VuP cornerA, cornerB;
            CountEdgesInFace (nodeA, splitEdgeMask, numSplit, numTotal, splitA, cornerA);
            if (numTotal == 3)
                {
                if (numSplit == 1)
                    {
                    numA1++;
                    VuP nodeB = vu_edgeMate (splitA);
                    CountEdgesInFace (nodeB, splitEdgeMask, numSplitB, numTotalB, splitB, cornerB);
                    if (numSplitB == 1 && numTotalB == 3)
                        {
                        // This edge is the only splittable edege in either adjacent face.  Accept it ....
                        vu_markedEdgeSetTestAndAdd (edgesToSplit, splitA);
                        vu_setMaskAroundFace (nodeA, visitMask);
                        }
                    }
                else if (numSplit == 2)
                    {
                    numA2++;
                    VuP homeNode = vu_fsucc (cornerA, 2);
                    vu_setMask (homeNode, twoSplitMask);
                    }
                else if (numSplit == 3)
                    {
                    numA3++;
                    // The whole face is out of tolerance.  Force the split everywhere.
                    VU_FACE_LOOP (nodeQ, nodeA)
                        {
                        vu_markedEdgeSetTestAndAdd (edgesToSplit, nodeQ);
                        }
                    END_VU_FACE_LOOP (nodeQ, nodeA)
                    }
                else
                    {
                    numA0++;
                    }
                }
            }
        }
    END_VU_SET_LOOP (nodeA, m_graph)

    // revisit TwoSplit sequences ... 
    VU_SET_LOOP (nodeA, m_graph)
        {
        if (vu_getMask (nodeA, twoSplitMask))
            {
            // We are at an unknown spot in a chain of skinny triangles that each require 2 edges
            // to be split.  Walk to one end all the way back to the other.
            VuDirectedStepper stepper (nodeA, twoSplitMask);
            stepper.MoveToEndOfChain ();
            stepper.ReverseDirection ();
            int chainSteps = stepper.MoveToEndOfChain ();
            stepper.ReverseDirection ();
            stepper.ClearCurrentNodeMask ();
            VuP nodeQ;
            for (int i = 0; i < chainSteps; i++)
                {
                if (0 == (i % 2))
                    {
                    nodeQ = stepper.TrailingEdgeNode ();
                    vu_markedEdgeSetTestAndAdd (edgesToSplit, nodeQ);
                    }
                if (!stepper.MoveToNeighbor ())
                    break;
                stepper.ClearCurrentNodeMask ();
                }
            nodeQ = stepper.LeadingEdgeNode ();
            vu_markedEdgeSetTestAndAdd (edgesToSplit, nodeQ);
            }
        }
    END_VU_SET_LOOP (nodeA, m_graph)

    // There is now a simple list of edges to split ...
    size_t numSplit1 = 0;
    for (VuP edgeBase; NULL != (edgeBase = vu_markedEdgeSetChooseAny (edgesToSplit));)
        {
        VuP nodeA, nodeB;
        DPoint3d xyz = vu_pointAtFraction (edgeBase, 0.5);
        vu_splitEdge (m_graph, edgeBase, &nodeA, &nodeB);
        vu_setMask (nodeA, newVertexMask);
        vu_setMask (nodeB, newVertexMask);
        vu_setDPoint3d (nodeA, &xyz);
        vu_setDPoint3d (nodeB, &xyz);
        numSplit1++;
        }

    // Visit all faces and add edges as needed to restore triangulation ...
    vu_clearMaskInSet (m_graph, visitMask);
    int num0 = 0;
    int num1 = 0;
    int num2 = 0;
    int num3 = 0;
    VU_SET_LOOP (nodeA, m_graph)
        {
        if (!vu_getMask (nodeA, visitMask))
            {
            vu_setMaskAroundFace (nodeA, visitMask);
            int numSplit, numTotal;
            VuP split0;
            VuP lastCorner;
            CountEdgesInFace (nodeA, newVertexMask, numSplit, numTotal, split0, lastCorner);
            if (numSplit == 1 && numTotal == 4)
                {
                VuP join0, join1;
                VuP oppositeCorner = vu_fsucc ( vu_fsucc (split0));
                vu_join (m_graph, split0, oppositeCorner, &join0, &join1);
                num1++;
                }
            else if (numSplit == 2 && numTotal == 5)
                {
                // Another walk around gets the "other" split as "last" ...
                VuP split1;
                CountEdgesInFace (split0, newVertexMask, numSplit, numTotal, split1, lastCorner);
                if (vu_fsucc (split1, 2) == split0)
                    {
                    std::swap (split0, split1);
                    }
                // reconfirm the expected sequencing...
                if (vu_fsucc (split0, 2) == split1)   // should always be true ..
                    {
                    VuP cornerAfter1 = vu_fsucc (split1);
                    VuP join0, join1;
                    VuP join2, join3;
                    vu_join (m_graph, split0, split1, &join0, &join1);
                    vu_join (m_graph, join0, cornerAfter1, &join2, &join3);
                    num2++;
                    }
                }
            else if (numSplit == 3 && numTotal == 6)
                {
                VuP split1 = vu_fsucc (split0, 2);
                VuP split2 = vu_fsucc (split1, 2);
                VuP join0A, join0B, join1A, join1B, join2A, join2B; // A edges are "inside" face, B area outside.
                num3++;
                if (vu_getMask (split1, newVertexMask) && vu_getMask (split2, newVertexMask)) // this should always pass...
                    {
                    vu_join (m_graph, split0, split1, &join0A, &join1B);
                    vu_join (m_graph, split1, split2, &join1A, &join2B);
                    vu_join (m_graph, split2, join0A, &join2A, &join0B);
                    }
                }
            else if (numSplit == 0)
                {
                num0++;
                }
            else
                {
                assert (false);
                }
            }
        }
    END_VU_SET_LOOP (nodeA, m_graph)
    

    vu_markedEdgeSetFree (edgesToSplit);
    vu_returnMask (m_graph, twoSplitMask);
    vu_returnMask (m_graph, visitMask);
    vu_returnMask (m_graph, splitEdgeMask);
    vu_returnMask (m_graph, newVertexMask);
    return  numSplit1;
    }

};

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public size_t vu_refineSurface
(
VuSetP graph,
VuEdgeSubdivisionTestFunction &surfaceTester
)
    {
    VuSurfaceRefinementContext context (graph, surfaceTester);
    size_t numSplit = context.SplitEdges13 ();
    return numSplit;
    }

END_BENTLEY_GEOMETRY_NAMESPACE