/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Bentley/BeTimeUtilities.h>


#include <Geom/XYZRangeTree.h>
#include <Vu/VuApi.h>
//#include <assert.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL;

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_relTol = 1.0e-12;
static double s_absTol = 1.0e-8;
static void KeepAlive (double a)
    {
    }
static int s_debugUnion = 0;
static int s_debugMerge = 0;
static int s_debugFinal = 0;
static int s_noFree = 0;
static int s_recordCoplanar = 0;
static double s_lowZStep = 1000.0;
static int s_deleteDanglers = 1;
static int s_expandFaces = 1;


struct PolygonSplitIndices
    {
    private:
    ptrdiff_t m_below;
    ptrdiff_t m_on;
    ptrdiff_t m_above;
    public:
    static const ptrdiff_t InvalidIndex = -1;
    public:
    PolygonSplitIndices ()
        : m_below (InvalidIndex),
          m_on (InvalidIndex),
          m_above (InvalidIndex)
        {}
    PolygonSplitIndices (ptrdiff_t below, ptrdiff_t on, ptrdiff_t above)
        : m_below (below),
          m_on (on),
          m_above (above)
        {}
    ptrdiff_t GetAbove () const { return m_above;}
    ptrdiff_t GetBelow () const { return m_below;}
    ptrdiff_t GetOn    () const { return m_on;}
    static bool IsValidIndex (ptrdiff_t index) {return index != InvalidIndex;}
    };

void AddRangeBase (TaggedPolygonVectorR polygons, DRange3dCR range)
    {
    TaggedPolygon newPolygon;
    DPoint3d xyz;
    xyz.Init (range.low.x, range.low.y, range.low.z - s_lowZStep);
    newPolygon.Add (xyz);
    xyz.Init (range.high.x, range.low.y, range.low.z - s_lowZStep);
    newPolygon.Add (xyz);
    xyz.Init (range.high.x, range.high.y, range.low.z - s_lowZStep);
    newPolygon.Add (xyz);
    xyz.Init (range.low.x, range.high.y,range.low.z - s_lowZStep);
    newPolygon.Add (xyz);
    polygons.push_back (newPolygon);
    }
void AddFacesFromGraph (VuSetP graph, bvector<TaggedPolygonVector> &polygons, VuMask faceMask, bool outputTrue, bool outputFalse, int ticId = 0)
    {
    DRange3d range;
    vu_graphRange (graph, &range);

    polygons.push_back (TaggedPolygonVector ());
    TaggedPolygonVectorR newPolygons = polygons.back ();
    VuMask visitMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, visitMask);
    VU_SET_LOOP (seed, graph)
        {
        if (!vu_getMask (seed, visitMask))
            {
            vu_setMaskAroundFace (seed, visitMask);
            bool isExterior = NULL != vu_findMaskAroundFace (seed, faceMask);
            bool outputThisFace = isExterior ? outputTrue : outputFalse;
            if (outputThisFace)
                {
                TaggedPolygon newPolygon;
                VU_FACE_LOOP (node, seed)
                    {
                    DPoint3d xyz1;
                    vu_getDPoint3d (&xyz1, node);
                    newPolygon.Add (xyz1);
                    }
                END_VU_FACE_LOOP (node, seed)
                newPolygons.push_back (newPolygon);
                }
            }
        }
    END_VU_SET_LOOP (seed, graph)
    // Draw rectangles along bottom and left for reference..
    DRange3d outlineRange = range;
    static double s_outlineFraction = 0.02;
    outlineRange.high.y = outlineRange.low.y + s_outlineFraction * (outlineRange.high.y - outlineRange.low.y);
    AddRangeBase (newPolygons, outlineRange);
    outlineRange = range;
    outlineRange.high.x = outlineRange.low.x + s_outlineFraction * (outlineRange.high.x - outlineRange.low.x);
    AddRangeBase (newPolygons, outlineRange);


    static int s_ticDivisor = 10;
    if (ticId >= 0)
        {
        int i = ticId % s_ticDivisor;
        int j = ticId / s_ticDivisor;
        double f = 1.0 / (double) s_ticDivisor;
        DRange3d range1 = range;
        double dx = f * (range.high.x - range.low.x);
        double dy0 = f * (range.high.y - range.low.y);
        double dy = (dy0 > dx) ? dx : dy0;
        range1.low.x = range.low.x + i * dx;
        range1.high.x = range1.low.x + dx;
        range1.low.y = range.high.y + j * dy;
        range1.high.y = range1.low.y + dy;
        AddRangeBase (newPolygons, range1);
        }
    vu_returnMask (graph, visitMask);
    }

void vu_freeDoubleMarkedEdges (VuSetP graph, VuMask mask)
    {
    if (s_noFree)
        return;
    VuMask deleteMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, deleteMask);
    VU_SET_LOOP (edgeSeed, graph)
        {
        if (
            (0 != vu_getMask (edgeSeed, mask))
         && (0 != vu_getMask (vu_edgeMate (edgeSeed), mask))
         )
            vu_setMask (edgeSeed, deleteMask);
        }
    END_VU_SET_LOOP (edgeSeed, graph)
    vu_freeMarkedEdges (graph, deleteMask);
    vu_returnMask (graph, deleteMask);
    }

void vu_unmaskNullFaceParityChanges (VuSetP graph, VuMask exteriorMask, VuMask boundaryMask)
    {
    VU_SET_LOOP (nodeA, graph)
        {
        VuP nodeB = vu_fsucc (nodeA);
        VuP nodeA1, nodeB1;
        if ((size_t)nodeB > (size_t)nodeA
            && vu_fsucc(nodeB) == nodeA
            )
            {
            nodeA1 = vu_vsucc (nodeA);
            nodeB1 = vu_vsucc (nodeB);
            // Could all 4 be masked?  Dunno ... test as if they could.
            if (vu_getMask (nodeA, exteriorMask) != 0 && vu_getMask (nodeB, exteriorMask) != 0)
                {
                vu_clrMask (nodeA, exteriorMask | boundaryMask);
                vu_clrMask (nodeB, exteriorMask | boundaryMask);
                vu_clrMask (nodeA1, boundaryMask);
                vu_clrMask (nodeB1, boundaryMask);
                }
            if (vu_getMask (nodeA1, exteriorMask) != 0 && vu_getMask (nodeB1, exteriorMask) != 0)
                {
                vu_clrMask (nodeA1, exteriorMask | boundaryMask);
                vu_clrMask (nodeB1, exteriorMask | boundaryMask);
                vu_clrMask (nodeA, boundaryMask);
                vu_clrMask (nodeB, boundaryMask);
                }
            }
        }
    END_VU_SET_LOOP (nodeA, graph)
    }

// IF exteriorMask appears in opposing sense on pairs of 
void vu_deleteNullFaceParityPairs (VuSetP graph, VuMask exteriorMask, bool deletePairedExteriors, bool deleteFullInterior)
    {
    VuMask deleteMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, deleteMask);
    VU_SET_LOOP (nodeA, graph)
        {
        VuP nodeB = vu_fsucc (nodeA);
        VuP nodeA1, nodeB1;
        if ((size_t)nodeB > (size_t)nodeA
            && vu_fsucc(nodeB) == nodeA
            )
            {
            nodeA1 = vu_vsucc (nodeA);
            nodeB1 = vu_vsucc (nodeB);
            bool maskA = 0 != vu_getMask (nodeA, exteriorMask);
            bool maskB = 0 != vu_getMask (nodeB, exteriorMask);
            bool maskA1 = 0 != vu_getMask (nodeA1, exteriorMask);
            bool maskB1 = 0 != vu_getMask (nodeB1, exteriorMask);

            bool doDelete = false;

            int numMask = 0;
            if (maskA)
                numMask++;
            if (maskB)
                numMask++;
            if (maskA1)
                numMask++;
            if (maskB1)
                numMask++;
            if (numMask == 0)
                doDelete = deleteFullInterior;
            else if (numMask == 2 && deletePairedExteriors)
                {
                if (maskA && maskB)   // maskA1 and maskB1 must be false per count.
                  doDelete = true;
                else if (maskA1 && maskB1)
                  doDelete = true;
                }
            else if (numMask == 4)
                doDelete = deletePairedExteriors;

            if (doDelete)
                {
                vu_setMask (nodeA, deleteMask);
                vu_setMask (nodeA1, deleteMask);
                vu_setMask (nodeB, deleteMask);
                vu_setMask (nodeB1, deleteMask);
                // excise the bundle to higher multiplicity neighbors can participate.  (hmmm.. does that need another pass?)
                VuP nodeA2 = vu_fsucc (nodeB1);
                VuP nodeB2 = vu_fsucc (nodeA1);
                vu_vertexTwist (graph, nodeA1, nodeA2);
                vu_vertexTwist (graph, nodeB1, nodeB2);
                }
            }
        }
    END_VU_SET_LOOP (nodeA, graph)
    vu_freeMarkedEdges (graph, deleteMask);
    vu_returnMask (graph, deleteMask);
    }




void vu_checkConsistentFaceMask (VuSetP graph, VuMask mask)
    {
    int numError = 0;
    VuMask visitMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, visitMask);
    assert (visitMask != 0);
    VU_SET_LOOP (seed, graph)
        {
        if (!vu_getMask (seed, visitMask))
            {
            vu_setMaskAroundFace (seed, visitMask);
            if (NULL != vu_findMaskAroundFace (seed, mask)
              && NULL != vu_findUnMaskedAroundFace (seed, mask))
              numError++;
            }
        }
    END_VU_SET_LOOP (seed, graph)
    vu_returnMask (graph, visitMask);
    }

static UsageSums s_polygonData0;
static UsageSums s_polygonData1;
static UsageSums s_strongZ;
class MultipleCollectionClippper
{
public:
  bvector<TaggedPolygonVector> m_debugPolygons;

private:
VuSetP m_pGraph;
VuArrayP m_pFaceArray;
int    m_debug;


// m_indexedNodeLists[i] is the array (stack?) of nodes in collection i.
struct NodeCollection : bvector<VuP>
    {
    NodeCollection () : bvector<VuP> () {}
    };

bvector<NodeCollection> m_indexedNodeLists;


private:
void PrintState (char const*name, size_t i)
    {
    static int s_noisy = 0;
    if (s_noisy == 0)
        return;
    size_t n = 0;
    for (VuP node = vu_firstNodeInGraph (m_pGraph); node != NULL; node = vu_nextNodeInGraph (m_pGraph, node))
        n++;

    printf ("  %s (%d) (graph nodes %d)\n", name, (int)i, (int)n);
    if (s_noisy > 2)
        {
        for (size_t c = 0; c < m_indexedNodeLists.size (); c++)
            {
            printf ("         (collection %d fragments %d)\n", (int)c, (int)m_indexedNodeLists[c].size());
            }
        }
    }
public:
void SaveCurrentNodesInCollection (ptrdiff_t collectionIndex)
    {
    if (collectionIndex < 0)
        return;
    for (;m_indexedNodeLists.size () <= (size_t)collectionIndex;)
      m_indexedNodeLists.push_back (NodeCollection ());
    PrintState ("SaveCurrentNodesinCollection", collectionIndex);
    VuP nodes = vu_extractNodeList (m_pGraph);
    m_indexedNodeLists[collectionIndex].push_back (nodes);
    }
MultipleCollectionClippper (double abstol)
    {
    m_pGraph = vu_newVuSet (0);
    m_pFaceArray = vu_grabArray (m_pGraph);
    if (s_absTol > abstol)
        abstol = s_absTol;
    vu_setTol (m_pGraph, abstol, s_relTol); 
    m_debug = 0;
    }

~MultipleCollectionClippper ()
    {
    vu_returnArray (m_pGraph, m_pFaceArray);
    vu_freeVuSet (m_pGraph);
    }

void Initialize ()
    {
    for (size_t i = 0; i < m_indexedNodeLists.size (); i++)
        {
        for (size_t j = 0; j < m_indexedNodeLists[i].size (); j++)
            {
            vu_restoreNodeList (m_pGraph, m_indexedNodeLists[i][j]);
            m_indexedNodeLists[i][j] = NULL;
            }
        m_indexedNodeLists[i].clear ();
        }
    m_indexedNodeLists.clear ();
    vu_reinitializeVuSet (m_pGraph);
    }

void SetDebug (int level) {m_debug = level;}
VuSetP GetGraph (){ return m_pGraph;}

public:

void PushGraph ()
    {
    static int numNonNull = 0;
    if (NULL != vu_firstNodeInGraph (m_pGraph))
        numNonNull++;
    vu_stackPush (m_pGraph);
    }

void PopGraph ()
    {
    static int numNonNull = 0;
    if (NULL != vu_firstNodeInGraph (m_pGraph))
        numNonNull++;
    vu_stackPop (m_pGraph);
    }
private:
VuP AddMarkedPolygon (bvector<DPoint3d> &xyz, VuMask baseMask, size_t &numEdge)
    {
    numEdge = 0;
    VuP pBase = NULL;
    VuP pLeft, pRight;
    size_t numXYZ = xyz.size ();
    s_polygonData0.Accumulate ((double)numXYZ);
    while (numXYZ > 1
            && xyz[0].IsEqual (xyz[numXYZ-1]))
            numXYZ--;
    s_polygonData1.Accumulate ((double)numXYZ);
    if (numXYZ < 3)
        return nullptr;
    numEdge = numXYZ;
    for (size_t i = 0; i < numXYZ; i++)
        {
        vu_splitEdge (m_pGraph, pBase, &pLeft, &pRight);
        vu_setDPoint3d (pLeft,  &xyz[i]);
        vu_setDPoint3d (pRight, &xyz[i]);
        vu_setMask (pLeft, baseMask);
        vu_setMask (pRight, baseMask);
        pBase = pLeft;
        }
    return pBase;
    }
public:
static bool IsStrongZ (DVec3dCR vector, double strongZFactor)
    {
    return fabs (vector.z) > strongZFactor * (fabs (vector.x) + fabs (vector.y));
    }
// Add a polygon to specified collection.
// The polygon is "fixed" by parity rules to be connected, oriented, and non-intersecting.
// The polygon exists in the collection as a distinct node list with canonical VU_BOUNDARY_MASK and VU_EXTERIOR_MASK after fixup.
void AddPolygon (size_t collectionIndex, bvector<DPoint3d> &xyz)
    {
    PushGraph ();
    static int s_select = 0;
    size_t numEdge;
    VuP node = AddMarkedPolygon (xyz, VU_BOUNDARY_EDGE, numEdge);
    double strongZFactor = 1.0;
    if (node != nullptr)
        {
        bool directConstruction = false;
        if (s_select == 0)
            {
            DPoint3d origin = xyz[0];
            DVec3d tri0 = DVec3d::FromCrossProductToPoints (origin, xyz[1], xyz[2]);
            DVec3d sum = tri0;
            DVec3d tri1;
            bool allStrong = true;
            directConstruction = true;
            for (size_t i = 2; i + 1 < numEdge; i++)
                {
                tri1.CrossProductToPoints (origin, xyz[i], xyz[i+1]);
                sum.Add (tri1);
                allStrong &= IsStrongZ (tri1, strongZFactor);
                }
            if (!allStrong)
                directConstruction = false;
            s_strongZ.Accumulate (directConstruction ? 1.0 : 0.0);
            }
        if (directConstruction)
            {
            double area = vu_area (node);
            VuP insideNode, outsideNode;
            if (area > 0.0)
                {
                insideNode = node;
                outsideNode = vu_vsucc(node);
                }
            else
                {
                outsideNode = node;
                insideNode = vu_vsucc(node);
                }
        
            vu_setMaskAroundFace (outsideNode, VU_EXTERIOR_EDGE);
            }
        else
            {
            vu_mergeLoops (m_pGraph);
            vu_regularizeGraph (m_pGraph);

            vu_parityFloodFromNegativeAreaFaces (m_pGraph, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);

            if (m_debug > 3)
                vu_printFaceLabels (m_pGraph, "AddPolygon");
            }
        }
    SaveCurrentNodesInCollection (collectionIndex);
    PopGraph ();
    }

size_t ActivateCollection (ptrdiff_t collectionIndex)
    {
    if (collectionIndex < 0)
        return 0;
    if ((size_t)collectionIndex >= m_indexedNodeLists.size ())
        return 0;
    size_t numTerms = m_indexedNodeLists[collectionIndex].size ();        
    for (; m_indexedNodeLists[collectionIndex].size () > 0; m_indexedNodeLists[collectionIndex].pop_back ())
        vu_restoreNodeList (m_pGraph, m_indexedNodeLists[collectionIndex].back ());
    m_indexedNodeLists[collectionIndex].clear ();
    PrintState ("ActivateCollection", collectionIndex);
    return numTerms;
    }

// Combine multiple graphs to a single graph with a union operation.
// ASSUME all graphs marked with usual BOUNDARY and EXTERIOR
// ASSUME singleton graph can be marked immediately without merge.
// COPY all VU_BOUNDARY_EDGE masks to boundaryMask
// Set exteriorMask by winding sweep.
// The combined graph becomes the collection.
void UnionWithinCollection (size_t collectionIndex, CharCP name, VuMask boundaryMask, VuMask exteriorMask)
    {
    if (collectionIndex >= m_indexedNodeLists.size ())
        return;
    static int s_actionWithinCollection = 1;
    PushGraph ();

    size_t numTerms = ActivateCollection (collectionIndex);
    if (numTerms == 1)
        {
        vu_copyMaskInSet (m_pGraph, VU_EXTERIOR_EDGE, exteriorMask);
        vu_copyMaskInSet (m_pGraph, VU_BOUNDARY_EDGE, boundaryMask);
        }
    else
        {
        vu_copyMaskInSet (m_pGraph, VU_BOUNDARY_EDGE, boundaryMask);

        // Merge among all clippers ..
        vu_mergeOrUnionLoops (m_pGraph, VUUNION_UNION);
        vu_regularizeGraph (m_pGraph);
        if (s_actionWithinCollection == 0)
            vu_windingFloodFromNegativeAreaFaces (m_pGraph, VU_EXTERIOR_EDGE, exteriorMask);
        else
            vu_parityFloodFromNegativeAreaFaces (m_pGraph, VU_BOUNDARY_EDGE, exteriorMask);
        vu_deleteNullFaceParityPairs (m_pGraph, exteriorMask, true, true);

        vu_checkConsistentFaceMask (m_pGraph, exteriorMask);
        vu_freeDoubleMarkedEdges (m_pGraph, exteriorMask);
        vu_copyMaskInSet (m_pGraph, exteriorMask, VU_EXTERIOR_EDGE);
        }

    DebugGraph (s_debugUnion, 10, name, exteriorMask);
    // aggressively simplify to "just the loops that matter" -- no pure interior or exteriors.  No hole connections
    vu_freeEdgesByMaskCount (m_pGraph, exteriorMask, true, false, true);
    DebugGraph (s_debugUnion, 20, name, exteriorMask);
    SaveCurrentNodesInCollection (collectionIndex);
    PopGraph ();
    }


void SetupForLoopOverInteriorFaces ()
    {
    if (s_debugFinal)
        vu_printFaceLabels (m_pGraph, "Final output");
    vu_collectInteriorFaceLoops (m_pFaceArray, m_pGraph);
    vu_arrayOpen (m_pFaceArray);
    }

bool GetFace (bvector<DPoint3d> &xyz, bool repeatFirstPoint)
    {
    xyz.clear ();

    VuP pFaceSeed;
    if (!vu_arrayRead (m_pFaceArray, &pFaceSeed))
        return false;

    VU_FACE_LOOP (pCurr, pFaceSeed)
        {
        DPoint3d xyz1;
        vu_getDPoint3d (&xyz1, pCurr);
        xyz.push_back (xyz1);
        }
    END_VU_FACE_LOOP (pCurr, pFaceSeed)

    if (repeatFirstPoint && xyz.size () >  0)
        {
        DPoint3d xyz1 = xyz.front ();
        xyz.push_back (xyz1);
        }
    return true;
    }

void PopAndMarkCollection (size_t index, VuMask boundaryMask, VuMask exteriorMask)
    {
    PushGraph ();
    ActivateCollection (index);
    vu_copyMaskInSet (m_pGraph, VU_BOUNDARY_EDGE, boundaryMask);
    vu_copyMaskInSet (m_pGraph, VU_EXTERIOR_EDGE, exteriorMask);
    SaveCurrentNodesInCollection (index);
    PopGraph ();
    }

void DebugGraph (int flags, int ticId, CharCP name, VuMask mask = VU_EXTERIOR_EDGE)
    {
    if (flags & 0x01)
        vu_printFaceLabels (m_pGraph, name);
    if (flags & 0x02)
        AddFacesFromGraph (m_pGraph, m_debugPolygons, mask, false, true, ticId);
    if (flags & 0x04)
        AddFacesFromGraph (m_pGraph, m_debugPolygons, mask, true, false, ticId);
    }

void DoubleMaskEdges (VuMask singleMask, VuMask doubleMask)
    {
    vu_clearMaskInSet (m_pGraph, doubleMask);
    VU_SET_LOOP (edge, m_pGraph)
        {
        VuP mate = vu_edgeMate (edge);
        if (
              vu_getMask (edge,singleMask)  != 0
          ||  vu_getMask (mate, singleMask) != 0
          )
            vu_setMask (edge, doubleMask);
        }
    END_VU_SET_LOOP (edge, m_pGraph)
    }
// Collapse components A and B with a union operation.
// Activate both, with indicated masks for their boundary and exterior.
// Merge and remark winding number with respective exterior masks.
void MergeAndMarkComponents (
CharCP nameA,
size_t indexA,
VuMask boundaryA,
VuMask exteriorA,
CharCP nameB,
size_t indexB,
VuMask boundaryB,
VuMask exteriorB,
bool (*NodeTestFunc) (VuSetP, VuP, VuMask maskA, VuMask maskB),
CharCP nameC,
size_t indexC,
VuMask maskC
)
    {

    VuMask myMask = vu_grabMask (m_pGraph);

    UnionWithinCollection (indexA, "A", boundaryA, exteriorA);
    UnionWithinCollection (indexB, "B", boundaryB, exteriorB);
    PushGraph ();

    ActivateCollection (indexA);

    ActivateCollection (indexB);

    vu_mergeOrUnionLoops (m_pGraph, VUUNION_UNION);
    vu_regularizeGraph (m_pGraph);



    //DebugGraph (s_debugMerge, 1, "Regularized", exteriorA || exteriorB);

    vu_clearMaskInSet (m_pGraph, myMask);
    vu_windingFloodFromNegativeAreaFaces (m_pGraph, exteriorA, myMask);
    vu_checkConsistentFaceMask (m_pGraph, myMask);
    vu_copyMaskInSet (m_pGraph, myMask, exteriorA);
    //DebugGraph (s_debugMerge, 22, "merge + A flood", exteriorA);
    //vu_freeDoubleMarkedEdges (m_pGraph, exteriorA);
    DebugGraph (s_debugMerge, 2, "merge + A flood", exteriorA);

    vu_clearMaskInSet (m_pGraph, myMask);
    vu_windingFloodFromNegativeAreaFaces (m_pGraph, exteriorB, myMask);
    vu_checkConsistentFaceMask (m_pGraph, myMask);
    vu_copyMaskInSet (m_pGraph, myMask, exteriorB);
    //vu_freeDoubleMarkedEdges (m_pGraph, exteriorB);
    DebugGraph (s_debugMerge, 3, "merge + B flood", exteriorB);

    VU_SET_LOOP (node, m_pGraph)
        {
        if (NodeTestFunc (m_pGraph, node, exteriorA, exteriorB))
            vu_setMask (node, maskC);
        else
            vu_clrMask (node, maskC);
        }
    END_VU_SET_LOOP (node, m_pGraph)
    vu_copyMaskInSet (m_pGraph, maskC, VU_EXTERIOR_EDGE);   // This has to be there to be activated properly by subsequent users !!!
    DebugGraph (s_debugMerge, 4, "After merge + TestFunc", VU_EXTERIOR_EDGE);
    DoubleMaskEdges (VU_EXTERIOR_EDGE, myMask);
    if (s_expandFaces)
        vu_expandFacesToBarrier (m_pGraph, myMask);
    if (s_deleteDanglers)
        vu_deleteDanglingEdges (m_pGraph);
    DebugGraph (s_debugMerge, 5, "After interior edge purge", VU_EXTERIOR_EDGE);

    SaveCurrentNodesInCollection (indexC);
    PopGraph ();


    vu_returnMask  (m_pGraph, myMask);

    }


};

// EXTERIOR mask test for A INTERSECT B when exteriorA and exteriorB are marked
bool NodeTest_Mask_OR (VuSetP graph, VuP node, VuMask maskA,  VuMask maskB)
    {
    return (vu_getMask (node, maskA) != 0) || (vu_getMask (node, maskB) != 0);
    }

// EXTERIOR mask test for A UNION B when exteriorA and exteriorB are marked
bool NodeTest_Mask_AND (VuSetP graph, VuP node, VuMask maskA, VuMask maskB)
    {
    return (vu_getMask (node, maskA) != 0) && (vu_getMask (node, maskB) != 0);
    }

// EXTERIOR mask test for A-B when exteriorA and exteriorB are marked
bool NodeTest_Mask_A_OR_notB (VuSetP graph, VuP node, VuMask maskA,  VuMask maskB)
    {
    return (vu_getMask (node, maskA) != 0) || (vu_getMask (node, maskB) == 0);
    }





///-----------------------------------------------------------------------------------------------------------------------------
static double s_normalZTolerance = 5.0e-4; /// We really don't want panels that are close to vertical.

// REMARK: This code is copied from the XYVisibilitySplitter -- lots of duplication.
struct CutAndFillSplitter
{
public:
static const int s_indexPrimaryPolygon = 0;
static const int s_indexPolygonsAbove  = 1;
static const int s_indexPolygonsBelow  = 2;

static const int s_indexAminusB        = 4;
static const int s_indexFinal          = 5;
private:

bvector<DPoint3d> m_currPoints;     // current clippee
double                m_originalArea;   // as presented when loaded.

size_t m_numTrivialClip;
size_t m_numFullClip;

bvector<DPoint3d> m_clipperPoints;
bvector<DPoint3d> m_clipperPointsOnPlane;
bvector<double>   m_clipperAltitudes;
double                m_abstol;
double                m_zScale;
bvector<DPoint3d> m_clipperLoop;    // Exracted from m_clipperPointsOnPlane
size_t m_currIndex;

DPlane3d m_currPlane;   // Plane origin and normal, with normal scaled so plane evaluation gives z altitude.

TaggedPolygonVectorCR m_polygons;
DRange3d      m_searchRange;
DRange3d      m_globalRange;
MultipleCollectionClippper m_clipper;

// Resize the clipper arrays to numPrimaryPoints.
// Replicated max(numCopy0, numCopy1) points at end.
void WrapClipperData (size_t numPrimaryPoints, size_t numCopy0, size_t numCopy1)
    {
    size_t numCopy = 0;
    if (numCopy0 > numCopy)
        numCopy = numCopy0;
    if (numCopy1 > numCopy)
        numCopy = numCopy1;
    m_clipperPoints.resize (numPrimaryPoints);
    m_clipperPointsOnPlane.resize (numPrimaryPoints);
    m_clipperAltitudes.resize (numPrimaryPoints);
    for (size_t i = 0; i < numCopy; i++)
        {
        m_clipperAltitudes.push_back (m_clipperAltitudes[i]);
        // copy point structures out to avoid pointers into resized buffer ...
        DPoint3d xyz = m_clipperPoints[i];
        m_clipperPoints.push_back (xyz);
        xyz = m_clipperPointsOnPlane[i];
        m_clipperPointsOnPlane.push_back (xyz);
        }
    }

void PushInterpolatedPoint (bvector<DPoint3d> &dest,
        bvector<DPoint3d> &source,
        bvector<double> &altitude,
        size_t i0
        )
    {
    size_t i1 = i0 + 1;
    double h0 = altitude[i0];
    double h1 = altitude[i1];
    double fraction;
    if (DoubleOps::SafeDivide (fraction, -h0, h1 - h0, 0.0))
        {
        DPoint3d xyz;
        xyz.Interpolate (source[i0], fraction, source[i1]);
        dest.push_back (xyz);
        }
    }

public:
  void AddDebugPolygons (bvector<TaggedPolygonVector> &dest, DVec3dCR shift)
      {
      Transform transform;
      transform.InitFrom (shift.x, shift.y, shift.z);
      for (size_t i = 0; i < m_clipper.m_debugPolygons.size (); i++)
          {
          dest.push_back (m_clipper.m_debugPolygons[i]);
          PolygonVectorOps::Multiply (dest.back (), transform);
          }
      }

CutAndFillSplitter (TaggedPolygonVectorCR polygons, double abstol, double zScale)
    : m_polygons (polygons),
      m_numTrivialClip(0),
      m_numFullClip (0),
      m_clipper (abstol),
      m_zScale (zScale)
    {
    m_globalRange = PolygonVectorOps::GetRange (m_polygons);
    m_abstol = abstol;
    }

~CutAndFillSplitter ()
    {
    }

size_t GetNumTrivialClip() const {return m_numTrivialClip;} // wip graphite - clang complains that m_numTrivialClip is unused. Maybe this function will quiet it down.

static bool GetNonVerticalPolygonPlane(TaggedPolygonCR polygon, DPlane3dR plane, DRange3dR range)
    {
    DPoint3d centroid;
    DVec3d   normal;
    bvector<DPoint3d> const & points = polygon.GetPointsCR ();
    range = polygon.GetRange ();
    double   area, perimeter, maxPlanarError;
    if (range.IsNull ())
        return false;
    if (!bsiPolygon_centroidAreaPerimeter (const_cast <DPoint3dP>(&points[0]), (int)points.size (),
                &centroid, &normal, &area, &perimeter, &maxPlanarError))
        return false;
#ifdef ExamineNormalVariation
    DVec3d normal0, normal1, normal2;
    normal0.CrossProductToPoints(points[0], points[1], points[2]);
    normal1.CrossProductToPoints(points[1], points[2], points[0]);
    normal2.CrossProductToPoints(points[2], points[0], points[1]);
    normal0.Normalize ();
    normal1.Normalize ();
    normal2.Normalize ();
    double c0 = normal0.Distance(normal);
    double c1 = normal1.Distance(normal);
    double c2 = normal2.Distance(normal);
#endif


    if (fabs (normal.z) < s_normalZTolerance)
        return false;

    double inverseNZ;
    if (!DoubleOps::SafeDivide (inverseNZ, 1.0, normal.z, 0.0))
        return false;
    normal.Scale (inverseNZ);
    plane.InitFromOriginAndNormal (centroid, normal);
    return true;
    }

int PolygonXYOrientation(bvector<DPoint3d> const &polygon)
    {
    size_t n = polygon.size ();
    DVec3d vector0, vector1;

    while (n > 1 && polygon[n-1].IsEqual (polygon[0]))
        n--;
    if (n < 2)
        return 0;
    vector0.DifferenceOf (polygon[0], polygon[n-1]);
    size_t numPositive = 0;
    size_t numNegative = 0;
    for (size_t i = 0; i < n; i++, vector0 = vector1)
        {
        size_t j = i + 1;
        if (j == n)
            j = 0;
        vector1.DifferenceOf (polygon[j], polygon[i]);
        double a = vector0.CrossProductXY (vector1);
        if (a < 0.0)
            numNegative++;
        else if (a > 0.0)
            numPositive++;
        }
    if (numNegative == 0)
        return 1;
    if (numPositive == 0)
        return -1;
    return 0;
    }

DRange3d GetSearchRange (){ return m_searchRange;}

bool LoadPrimaryPolygon (TaggedPolygonCR polygon, size_t index)
    {
    SetPolygonSplitIndicesForSource (polygon, index);
    m_currIndex  = index;
    m_currPoints = polygon.GetPointsCR ();
//    LoadOutwardNormals ();
    if (!GetNonVerticalPolygonPlane (polygon, m_currPlane, m_searchRange))
        return false;
    if (m_zScale < 0.0)
        m_currPlane.normal.Negate ();
    m_searchRange.high.z = m_globalRange.high.z;
    m_searchRange.low.z = m_globalRange.low.z;

    m_clipper.Initialize ();
    m_clipper.AddPolygon (s_indexPrimaryPolygon, m_currPoints);
    m_originalArea = polygon.AreaXY ();
    return true;
    }

bool LoadPrimaryPolygon (size_t index)
    {
    if (index >= m_polygons.size ())
        return false;
    return LoadPrimaryPolygon (m_polygons.at(index), index);
    }


double DistanceSpacePointToPlane (DPoint3dCR spacePoint)
    {
    return spacePoint.DotDifference (m_currPlane.origin, m_currPlane.normal);
    }

DPoint3d ProjectSpacePointToPlane (DPoint3dCR spacePoint)
    {
    double a = DistanceSpacePointToPlane (spacePoint);
    DPoint3d planePoint = spacePoint;
    planePoint.z -= m_zScale * a;
    return planePoint;
    }
private:
    PolygonSplitIndices m_splitAA_A0beforeA1;
    PolygonSplitIndices m_splitAA_A0afterA1;
    PolygonSplitIndices m_splitAB;

    PolygonSplitIndices m_splitBB_B0beforeB1;
    PolygonSplitIndices m_splitBB_B0afterB1;
    PolygonSplitIndices m_splitBA;

    size_t              m_candidateCategory; // 0,1 for A,B
    size_t              m_candidateIndex;

size_t GetPolygonCategory (TaggedPolygonCR polygon)
    {
    return polygon.GetIndexA () == 0 ? 0 : 1;
    }
public:
void SetPolygonSplitIndicesForSource (TaggedPolygonCR candidate, size_t candidateIndex)
    {
    m_candidateIndex = candidateIndex;
    m_candidateCategory = GetPolygonCategory (candidate);
    static int s_selfClip = 0;
    int selfClipAboveIndex = s_indexPolygonsAbove;
    if (s_selfClip == 0)
        selfClipAboveIndex = PolygonSplitIndices::InvalidIndex;
    if (m_candidateCategory == 0)
        {
        m_splitAA_A0beforeA1 = PolygonSplitIndices (PolygonSplitIndices::InvalidIndex, PolygonSplitIndices::InvalidIndex, selfClipAboveIndex);
        m_splitAA_A0afterA1 = PolygonSplitIndices (PolygonSplitIndices::InvalidIndex, s_indexPolygonsAbove, selfClipAboveIndex);
        m_splitAB = PolygonSplitIndices (s_indexPolygonsBelow, s_indexPolygonsAbove, s_indexPolygonsAbove);
        }
    else
        {
        m_splitBB_B0beforeB1 = PolygonSplitIndices (PolygonSplitIndices::InvalidIndex, PolygonSplitIndices::InvalidIndex, selfClipAboveIndex);
        m_splitBB_B0afterB1 = PolygonSplitIndices (PolygonSplitIndices::InvalidIndex, s_indexPolygonsAbove, selfClipAboveIndex);
        m_splitBA = PolygonSplitIndices (s_indexPolygonsBelow, s_indexPolygonsAbove, s_indexPolygonsAbove);
        }
    }

PolygonSplitIndices GetClipperSplitIndices (TaggedPolygonCR clipper, size_t clipperIndex)
    {
    size_t clipperCategory = GetPolygonCategory (clipper);
    if (m_candidateCategory == 0)
        {
        if (clipperCategory == 0)
            return m_candidateIndex < clipperIndex
                  ? m_splitAA_A0beforeA1
                  : m_splitAA_A0afterA1;
        return m_splitAB;
        }
    else
        if (clipperCategory == 1)
            return m_candidateIndex < clipperIndex
                  ? m_splitBB_B0beforeB1
                  : m_splitBB_B0afterB1;
        return m_splitBA;
    }

void BuildLoops (ptrdiff_t polygonCollectionIndex, size_t firstIndexOnOtherSide, size_t numDistinctPoints, double altitudeSign)
    {
    if (!PolygonSplitIndices::IsValidIndex (polygonCollectionIndex))
        return;
    // Find strings of positive altitudes following negatives.
    size_t firstNegativeIndexPlusPeriod = numDistinctPoints + firstIndexOnOtherSide;
    for (size_t i0 = firstIndexOnOtherSide; i0 < firstNegativeIndexPlusPeriod;)
        {
        // i0 is a negative index.
        // Walk forward through nonnegatives.  Because of the wrap, we are assured
        // if finding one (without an index check)
        size_t i1;
        for (i1 = i0 + 1; m_clipperAltitudes[i1] * altitudeSign >= 0.0;)
            {
            i1++;
            }
        if (i1 > i0)
            {
            m_clipperLoop.clear ();
            if (m_clipperAltitudes[i0 + 1] * altitudeSign > 0.0)
                PushInterpolatedPoint (m_clipperLoop, m_clipperPointsOnPlane, m_clipperAltitudes, i0);
            for (size_t i = i0+1; i < i1; i++)
                m_clipperLoop.push_back (m_clipperPointsOnPlane[i]);
            if (m_clipperAltitudes[i1 - 1] * altitudeSign > 0.0)
                PushInterpolatedPoint (m_clipperLoop, m_clipperPointsOnPlane, m_clipperAltitudes, i1 - 1);
            if (m_clipperLoop.size () > 2)
                {
                DPoint3d xyz = m_clipperLoop[0];
                m_clipperLoop.push_back (xyz);
                m_clipper.AddPolygon (polygonCollectionIndex, m_clipperLoop);  
                }
            i0 = i1;
            }
        }
    }


// split a polygon (clipper) by the plane of the clippee polygon.
// distribute above, below, on parts to specified collections.
void ApplyClipper (bvector<DPoint3d> const &clipper, PolygonSplitIndices const & outputIndices)
    {
    m_numFullClip++;

    m_clipperPoints = clipper;
    m_clipperAltitudes.clear ();
    m_clipperPointsOnPlane.clear ();
    ptrdiff_t firstNegativeIndex = -1;
    ptrdiff_t firstPositiveIndex = -1;
    size_t numPositive = 0;
    size_t numNegative = 0;
    size_t numOn = 0;
    size_t numClipperPoints = m_clipperPoints.size ();
    
    while (numClipperPoints > 1 && m_clipperPoints[0].IsEqual (m_clipperPoints[numClipperPoints - 1]))
        numClipperPoints--;

    for (size_t i = 0; i < numClipperPoints; i++)
        {
        DPoint3d xyz = m_clipperPoints[i];
        double z = DistanceSpacePointToPlane (xyz);
        m_clipperPoints.push_back (xyz);
        xyz.z -= m_zScale * z;
        m_clipperPointsOnPlane.push_back (xyz);

        if (fabs (z) < m_abstol)
            {
            numOn++;
            z = 0.0;
            }
        else if (z >= 0.0)
            {
            numPositive++;
            if (firstPositiveIndex < 0)
                firstPositiveIndex = i;
            }
        else
            {
            numNegative++;
            if (firstNegativeIndex < 0)
                firstNegativeIndex = i;
            }

        m_clipperAltitudes.push_back (z);
        }

    if (numOn == numClipperPoints)
        {
        if (s_recordCoplanar)
        m_clipper.AddPolygon (outputIndices.GetOn (), m_clipperPointsOnPlane);
        }
    else if (numPositive == 0)
        {
        m_clipper.AddPolygon (outputIndices.GetBelow (), m_clipperPointsOnPlane);
        }
    else if (numNegative == 0)
        {
        m_clipper.AddPolygon (outputIndices.GetAbove (), m_clipperPointsOnPlane);
        }
    else
        {
        size_t numDistinctPoints = numClipperPoints;
        if (m_clipperPoints[0].IsEqual (m_clipperPoints[numClipperPoints - 1]))
            numDistinctPoints--;
        WrapClipperData (numDistinctPoints, firstNegativeIndex + 1, firstPositiveIndex + 1);
        BuildLoops (outputIndices.GetAbove (), firstNegativeIndex, numDistinctPoints,  1.0);
        BuildLoops (outputIndices.GetBelow (), firstPositiveIndex, numDistinctPoints, -1.0);
        }
    }


// project polygon to active plane, add to collection.
// distribute above, below, on parts to specified collections.
void ApplyXYClip (bvector<DPoint3d> const &clipper, ptrdiff_t collectionIndex)
    {
    m_numFullClip++;

    m_clipperPoints = clipper;
    m_clipperAltitudes.clear ();
    m_clipperPointsOnPlane.clear ();
    size_t numClipperPoints = m_clipperPoints.size ();
    
    while (numClipperPoints > 1 && m_clipperPoints[0].IsEqual (m_clipperPoints[numClipperPoints - 1]))
        numClipperPoints--;

    for (size_t i = 0; i < numClipperPoints; i++)
        {
        DPoint3d xyz = m_clipperPoints[i];
        double z = DistanceSpacePointToPlane (xyz);
        m_clipperPoints.push_back (xyz);
        xyz.z -= m_zScale * z;
        m_clipperPointsOnPlane.push_back (xyz);
        m_clipperAltitudes.push_back (z);
        }

    m_clipper.AddPolygon (collectionIndex, m_clipperPointsOnPlane);
    }




// restrictToBase true ==>> (A-B) ^ C
// restrictTOBase false ==>> (A-B)
void CollectClip (TaggedPolygonVectorR outputPolygons, bool subtractB, bool intersectC)
    {
    int boundaryA = VU_SILHOUETTE_EDGE;
    int exteriorA = VU_DISCONTINUITY_EDGE;
    int boundaryB = VU_RULE_EDGE;
    int exteriorB = VU_GRID_EDGE;
    int boundaryC = VU_SEAM_EDGE;
    int exteriorC = VU_KNOT_EDGE;
    int exteriorAminusB = VU_SECTION_EDGE;
    // A is the primary
    // B is hiders
    // C is below

    // CLAIM: This masking is redundant.
    // 1) boundaryA, boundaryB, boundaryC are never used
    // 2) Saved collections ALWAYS have VU_EXTERIOR_EDGE set.
    // 3) MergeAndMark can use borrowed masks for role of exteriorA and exteriorB.
    // 4) "The collections" are powerful things !!!!
          
    if (subtractB && intersectC)
        {
        m_clipper.MergeAndMarkComponents (
              "primary", s_indexPrimaryPolygon, boundaryA, exteriorA,
              "above",   s_indexPolygonsAbove,  boundaryB, exteriorB,
              NodeTest_Mask_A_OR_notB,
              "visible = primary-above",
              s_indexAminusB, exteriorAminusB);

        m_clipper.MergeAndMarkComponents (
              "visible", s_indexAminusB,        boundaryA, exteriorAminusB,
              "below",   s_indexPolygonsBelow,  boundaryC, exteriorC,
              NodeTest_Mask_OR,
              "visible ^ below", s_indexFinal,
              VU_EXTERIOR_EDGE);
        }
    else if (subtractB && !intersectC)
        {
        // B (above) is used
        // C (below) is not used ...
        m_clipper.MergeAndMarkComponents (
              "primary", s_indexPrimaryPolygon, boundaryA, exteriorA,
              "above",   s_indexPolygonsAbove,  boundaryB, exteriorB,
              NodeTest_Mask_A_OR_notB,
              "visible = primary-above", s_indexFinal, VU_EXTERIOR_EDGE);
        }
    else if (!subtractB && intersectC)
        {
        // B (above) is not used ...
        // C (below) is used
        m_clipper.MergeAndMarkComponents (
              "primary", s_indexPrimaryPolygon, boundaryA, exteriorA,
              "below",   s_indexPolygonsBelow,  boundaryC, exteriorC,
              NodeTest_Mask_OR,
              "keep = primary ^ below", s_indexFinal,
              VU_EXTERIOR_EDGE);
        }        

    bvector<DPoint3d> xyz;
    m_clipper.PushGraph ();
    m_clipper.ActivateCollection (s_indexFinal);
    m_clipper.SetupForLoopOverInteriorFaces ();
    size_t numPolygon = 0;
    for (;m_clipper.GetFace (xyz, false);)
        {
        double area = bsiGeom_getXYPolygonArea (&xyz[0], (int)xyz.size ());
        if (area * m_originalArea < 0.0)
            bsiDPoint3d_reverseArrayInPlace (&xyz[0], (int)xyz.size ());
        PolygonVectorOps::AddPolygon (outputPolygons, xyz);
        if (m_currIndex < m_polygons.size ())
            outputPolygons.at (outputPolygons.size () - 1).CopyTagsFrom (m_polygons.at (m_currIndex));
        numPolygon ++;
        }
    m_clipper.SaveCurrentNodesInCollection (s_indexFinal);
    m_clipper.PopGraph ();
    }
};
double FractionOf (StopWatch &child, StopWatch &parent)
    {
    if (parent.GetElapsedSeconds() > 0)
        return (double)child.GetElapsedSeconds() / (double)parent.GetElapsedSeconds();
    return 0.0;
    }

//=====================================================================================================

struct CutAndFillSearcher : XYZRangeTreeHandler
{
private:
CutAndFillSplitter m_upperSurfaceSplitter;
CutAndFillSplitter m_lowerSurfaceSplitter;
DRange3d      m_globalRange;
DRange3d      m_searchRange;
TaggedPolygonVectorCR m_polygons;

double m_absTol;
size_t m_currIndex;

public:

CutAndFillSplitter &UpperSplitter (){ return m_upperSurfaceSplitter;}
CutAndFillSplitter &LowerSplitter (){ return m_lowerSurfaceSplitter;}


CutAndFillSearcher (TaggedPolygonVectorCR polygons, double abstol)
    : m_polygons (polygons),
      m_upperSurfaceSplitter (polygons, abstol, 1.0),
      m_lowerSurfaceSplitter (polygons, abstol, -1.0)
    {
    m_globalRange = PolygonVectorOps::GetRange (m_polygons);
    m_absTol = abstol;
    ClearCounts ();
    }

~CutAndFillSearcher ()
    {
    }


public:
    size_t m_leafHit;
    size_t m_leafSkip;
    size_t m_subtreeHit;
    size_t m_subtreeSkip;

void ClearCounts ()
    {
    m_leafHit       = 0;
    m_leafSkip      = 0;
    m_subtreeHit    = 0;
    m_subtreeSkip   = 0;
    }

bool LoadPrimaryPolygon (size_t index)
    {
    m_currIndex = index;
    bool stat = UpperSplitter ().LoadPrimaryPolygon (index) & LowerSplitter ().LoadPrimaryPolygon(index);
    m_searchRange = UpperSplitter ().GetSearchRange ();
    return stat;
    }





bool ShouldRecurseIntoSubtree (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior) override 
    {
    DRange3d nodeRange = pInterior->Range ();
    if (m_searchRange.IntersectsWith (nodeRange))
        {
        m_subtreeHit++;
        return true;
        }
    else
        {
        m_subtreeSkip++;
        return false;
        }
    }

bool ShouldContinueAfterSubtree      (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior)    override {return true;}

void ApplyClipperByIndex (size_t leafIndex)
    {
    m_upperSurfaceSplitter.ApplyClipper (m_polygons[leafIndex].GetPointsCR (), m_upperSurfaceSplitter.GetClipperSplitIndices (m_polygons[leafIndex], leafIndex));
    m_lowerSurfaceSplitter.ApplyClipper (m_polygons[leafIndex].GetPointsCR (), m_lowerSurfaceSplitter.GetClipperSplitIndices (m_polygons[leafIndex], leafIndex));
    }

bool ShouldContinueAfterLeaf         (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior, XYZRangeTreeLeafP pLeaf) override 
    {
    size_t leafIndex = (size_t)pLeaf->GetData ();
    if (leafIndex != m_currIndex)
        {
        DRange3d nodeRange = pLeaf->Range ();
        if (m_searchRange.IntersectsWith (nodeRange))
            {
            ApplyClipperByIndex (leafIndex);
//            m_upperSurfaceSplitter.ApplyClipper (m_polygons[leafIndex].GetPointsCR (), m_upperSurfaceSplitter.GetClipperSplitIndices (m_polygons[leafIndex], leafIndex));
//            m_lowerSurfaceSplitter.ApplyClipper (m_polygons[leafIndex].GetPointsCR (), m_lowerSurfaceSplitter.GetClipperSplitIndices (m_polygons[leafIndex], leafIndex));
            m_leafHit++;
            }
        else
            {
            m_leafSkip++;
            }
        }
    return true;
    }

void CollectClip
(
bool isUpperSurface,
TaggedPolygonVectorR surfaceAaboveB,
TaggedPolygonVectorR surfaceAbelowB,
TaggedPolygonVectorR surfaceBaboveA,
TaggedPolygonVectorR surfaceBbelowA
)
    {
    if (isUpperSurface)
        {
        UpperSplitter ().CollectClip (surfaceAaboveB, true, true);
        LowerSplitter ().CollectClip (surfaceAbelowB, true, true);
        }
    else
        {
        UpperSplitter ().CollectClip (surfaceBaboveA, true, true);
        LowerSplitter ().CollectClip (surfaceBbelowA, true, true);
        }
    }

void AddDebugPolygons (bvector<TaggedPolygonVector> &dest, double dyLowerSurface)
    {
    DVec3d upperShift, lowerShift;
    upperShift.Zero ();
    lowerShift.Init (0, dyLowerSurface, 0);
    UpperSplitter ().AddDebugPolygons (dest, upperShift);
    LowerSplitter ().AddDebugPolygons (dest, lowerShift);
    }
  
};


void FixArea (TaggedPolygonVectorR data, double sign)
    {
    double area = PolygonVectorOps::GetSummedAreaXY (data);
    if (sign * area < 0.0)
        PolygonVectorOps::ReverseEachPolygon (data);
    }

 //! ASSUME tagged polygons have IndexA() = read index
 //! ASSUME tagged polygons have IndexB() = 0 or 1 for parent mesh indicator (e.g. dtm or roadway)
void PolygonVectorOps::CutAndFill_IndexedHeap
(
TaggedPolygonVectorCR source,
TaggedPolygonVectorR surfaceAaboveB,
TaggedPolygonVectorR surfaceAbelowB,
TaggedPolygonVectorR surfaceBaboveA,
TaggedPolygonVectorR surfaceBbelowA,
bvector<TaggedPolygonVector> &debugShapes
)
    {
    surfaceAaboveB.clear ();
    surfaceAbelowB.clear ();
    surfaceBaboveA.clear ();
    surfaceBbelowA.clear ();

    DRange3d totalRange = PolygonVectorOps::GetRange (source);
    double abstol = s_relTol * totalRange.LargestCoordinate ();
    auto heap = PolyfaceIndexedHeapRangeTree::CreateForPolygons (source,  true, true, false);
    size_t n = source.size ();

#ifdef PRINT_STATS
    if (s_debug > 2)
        printf ("(polygons %d\n", n);
#endif

    CutAndFillSearcher searcher (source, abstol);

    StopWatch totalSearchTime;
    StopWatch traverseTime;
    StopWatch collectTime;
    StopWatch loadTime;

    totalSearchTime.Start ();
    searcher.ClearCounts ();
    bvector<size_t> hits;
    static bool s_checkAll = false;
    for (size_t i = 0; i < n; i++)
        {
        loadTime.Start ();
        bool loaded = searcher.LoadPrimaryPolygon (i);
        if (loaded)
            {
            loadTime.Stop ();
            DRange3d range = source[i].GetRange ();
            range.low.z = -DBL_MAX;
            range.high.z = DBL_MAX;
            heap->CollectInRange (hits, range, 0.0);
            for (size_t hitIndex : hits )
                {
                if (hitIndex != i)
                    {
                    if (s_checkAll
                        || source[i].GetIndexA () != source[hitIndex].GetIndexA ()
                        )
                    searcher.ApplyClipperByIndex (hitIndex);
                    }
                }
            collectTime.Start ();
            searcher.CollectClip (source[i].GetIndexA () == 0,
                surfaceAaboveB, surfaceAbelowB, surfaceBaboveA, surfaceBbelowA);
            collectTime.Stop ();
            }
            collectTime.Stop ();
        }
    totalSearchTime.Stop ();
    FixArea (surfaceAaboveB, 1.0);
    FixArea (surfaceBbelowA, -1.0);
    FixArea (surfaceBaboveA, 1.0);
    FixArea (surfaceAbelowB, -1.0); 
    }


void PolygonVectorOps::CutAndFill
(
TaggedPolygonVectorCR source,
TaggedPolygonVectorR surfaceAaboveB,
TaggedPolygonVectorR surfaceAbelowB,
TaggedPolygonVectorR surfaceBaboveA,
TaggedPolygonVectorR surfaceBbelowA,
bvector<TaggedPolygonVector> &debugShapes
)
    {
    surfaceAaboveB.clear ();
    surfaceAbelowB.clear ();
    surfaceBaboveA.clear ();
    surfaceBbelowA.clear ();

    DRange3d totalRange = PolygonVectorOps::GetRange (source);
    double abstol = s_relTol * totalRange.LargestCoordinate ();
    XYZRangeTreeRootP rangeTree = XYZRangeTreeRoot::Allocate ();
    size_t n = source.size ();

#ifdef PRINT_STATS
    if (s_debug > 2)
        printf ("(polygons %d\n", n);
#endif

    CutAndFillSearcher searcher (source, abstol);
    for (size_t i = 0; i < n; i++)
        {
        DRange3d range;
        DPlane3d plane;
        if (CutAndFillSplitter::GetNonVerticalPolygonPlane (source[i], plane, range))
            {
            rangeTree->Add ((void*)i, range);
            }
        }

    StopWatch totalSearchTime;
    StopWatch traverseTime;
    StopWatch collectTime;
    StopWatch loadTime;
#ifdef PRINT_STATS
    XYZRangeTreeCounter counter = XYZRangeTreeCounter ();
    rangeTree->Traverse (counter);
    printf ("(RangeTree (#leaf %d) (#fringe %d) (#interior %d) (#maxDepth %d) (sumLeafSquared %ld) \n",
                    counter.mNumLeaf, counter.mNumFringe,
                    counter.mNumInterior, counter.mMaxDepth,
                    counter.mSumFringeLeafCountSquared
                    );
#endif
    static ptrdiff_t s_singlePolygonSelector = -1;
    if (s_singlePolygonSelector >= 0 && s_singlePolygonSelector < (ptrdiff_t)n)
        {
        size_t i = (size_t)s_singlePolygonSelector;
        searcher.LoadPrimaryPolygon (i);
        rangeTree->Traverse (searcher);
        searcher.CollectClip (source[i].GetIndexA () == 0,
                    surfaceAaboveB, surfaceAbelowB, surfaceBaboveA, surfaceBbelowA);
        }
    else
        {
        totalSearchTime.Start ();
        searcher.ClearCounts ();
        for (size_t i = 0; i < n; i++)
            {
            loadTime.Start ();
            bool loaded = searcher.LoadPrimaryPolygon (i);
            loadTime.Stop ();
            if (loaded)
                {
                traverseTime.Start ();
                rangeTree->Traverse (searcher);
                traverseTime.Stop ();
                collectTime.Start ();
                searcher.CollectClip (source[i].GetIndexA () == 0,
                    surfaceAaboveB, surfaceAbelowB, surfaceBaboveA, surfaceBbelowA);
                collectTime.Stop ();
                }
            }
        totalSearchTime.Stop ();
    #ifdef PRINT_STATS
            printf ("(Subtree %d %d) (Leaf %d %d)\n",
                        searcher.m_subtreeHit,
                        searcher.m_subtreeSkip,
                        searcher.m_leafHit,
                        searcher.m_leafSkip
                        );
    #endif
        }

    FixArea (surfaceAaboveB, 1.0);
    FixArea (surfaceBbelowA, -1.0);
    FixArea (surfaceBaboveA, 1.0);
    FixArea (surfaceAbelowB, -1.0);        
    static double s_debugShiftFactor = 1.4;
    searcher.AddDebugPolygons (debugShapes, (totalRange.high.y - totalRange.low.y) * s_debugShiftFactor);
    double loadFraction = FractionOf (loadTime, totalSearchTime);
    double traverseFraction = FractionOf (traverseTime, totalSearchTime);
    double collectFraction = FractionOf (collectTime, totalSearchTime);
    KeepAlive (loadFraction);
    KeepAlive (traverseFraction);
    KeepAlive (collectFraction);
    XYZRangeTreeRoot::Free (rangeTree);
    }
    
static void AddPolygons (
IPolyfaceConstructionR builder,
TaggedPolygonVectorR polygons
)
    {
    // We know this is variable indexed ...
    size_t indexShift = 1;
    bvector<size_t> indices;
    bvector<int> &pointIndex = builder.GetClientMeshR ().PointIndex ();
    for (size_t i = 0; i < polygons.size (); i++)
        {
        builder.FindOrAddPoints (
                polygons[i].GetPointsR (),
                polygons[i].GetPointSize (), 0, indices);
        // purge duplicates ... this is very rare, so don't worry about repeated erase ...
        while (indices.size () > 2 && indices.back () == indices.front ())
            indices.pop_back ();
        for (size_t i = 1; i < indices.size ();)
            {
            if (indices[i] == indices[i-1])
                {
                indices.erase (indices.begin () + i);
                }
            else
                {
                i++;
                }
            }
        for (size_t i = 0; i < indices.size (); i++)
            pointIndex.push_back ((int)(indices[i] + indexShift));
        pointIndex.push_back (0);
        indices.clear ();
    }

    }
    
static void SavePolygons (
bvector<PolyfaceHeaderPtr> &result,
TaggedPolygonVectorR polygons,
TaggedPolygonVectorP polygonB
)
    {
    IFacetOptionsPtr facetOptions = IFacetOptions::Create ();
    facetOptions->SetParamsRequired (false);
    facetOptions->SetNormalsRequired (false);
    IPolyfaceConstructionPtr builder = PolyfaceConstruction::Create (*facetOptions);
    builder->GetClientMeshR ().SetNumPerFace (0);
    AddPolygons (*builder, polygons);
    if (NULL != polygonB)
        AddPolygons (*builder, *polygonB);
    PolyfaceHeaderPtr headerPtr = builder->GetClientMeshPtr ();
    if (headerPtr->Point ().size () > 0)
        result.push_back (headerPtr);
    }
	

static PolyfaceHeaderPtr PolygonsToMesh (TaggedPolygonVectorR polygons, TransformCP localToWorld)
    {
    IFacetOptionsPtr facetOptions = IFacetOptions::Create ();
    facetOptions->SetParamsRequired (false);
    facetOptions->SetNormalsRequired (false);
    IPolyfaceConstructionPtr builder = PolyfaceConstruction::Create (*facetOptions);
    builder->GetClientMeshR ().SetNumPerFace (0);
    AddPolygons (*builder, polygons);
    PolyfaceHeaderPtr mesh = builder->GetClientMeshPtr ();
    if (nullptr != localToWorld)
        mesh->Transform (*localToWorld);
    return mesh;
    }
		
static double   s_planarityLocalRelTol = 1.0e-13;

void PolyfaceQuery::ComputeCutAndFill
(
bvector<PolyfaceHeaderPtr> polyfaceA,
bvector<PolyfaceHeaderPtr> polyfaceB,
bvector<PolyfaceHeaderPtr> &resultA,
bvector<PolyfaceHeaderPtr> &resultB
)
    {
    TaggedPolygonVector allInputPolygons;
    // In allInputPolygons, IndexA = 0 or 1 selecting primary or barrier as parent.
    //    IndexB = parent mesh index.  (This better not get used in the algorithm -- the single-polyface method makes this a facet index.
    for (size_t i = 0; i < polyfaceA.size (); i++)
        polyfaceA[i]->AddToTaggedPolygons (allInputPolygons, 0, 0);
    
    for (size_t i = 0; i < polyfaceB.size (); i++)
        polyfaceB[i]->AddToTaggedPolygons (allInputPolygons, 1, 0);
        
    DRange3d inputRange = PolygonVectorOps::GetRange (allInputPolygons);


    double planarityAbsTol = s_planarityLocalRelTol * inputRange.low.Distance (inputRange.high);
    DPoint3d rangeCenter;
    rangeCenter.Interpolate (inputRange.low, 0.5, inputRange.high);

    Transform localToWorld, worldToLocal;
    localToWorld.InitFrom (rangeCenter);
    worldToLocal.InitFrom (-rangeCenter.x, -rangeCenter.y, -rangeCenter.z);

    PolygonVectorOps::Multiply (allInputPolygons, worldToLocal);

    worldToLocal.Multiply (inputRange, inputRange);    // worldToLocal is just translation, so this is exact.

    TaggedPolygonVector visiblePolygons;

    static int s_mergeTopAndBottom = 3;
    //static int s_saveFillTop = 1;                             unused var removed in Graphite
    //static int s_saveFillBottom = 1;                          unused var removed in Graphite
    //static int s_saveCutTop = 1;                              unused var removed in Graphite
    //static int s_saveCutBottom = 2;                           unused var removed in Graphite

    PolygonVectorOps::TriangulateNonPlanarPolygons (allInputPolygons, planarityAbsTol);
    bvector <TaggedPolygonVector> debugPolygons;
    TaggedPolygonVector out1, out2, out3, out4;
    PolygonVectorOps::CutAndFill (allInputPolygons, out1, out2, out3, out4, debugPolygons);

    if (s_mergeTopAndBottom != 0)
        {
        SavePolygons (resultA, out1, &out4);
        SavePolygons (resultB, out2, &out3);
        }
    else
        {
        SavePolygons (resultA, out1, NULL);
        SavePolygons (resultB, out2, NULL);
        SavePolygons (resultB, out3, NULL);
        SavePolygons (resultA, out4, NULL);
       }
    PolyfaceHeader::Transform (resultA, localToWorld);
    PolyfaceHeader::Transform (resultB, localToWorld);
   
    }
// test condition for distinguishing barrier polygons.
void PolyfaceQuery::ComputeCutAndFill
(
PolyfaceHeaderCR polyfaceA,
PolyfaceHeaderCR polyfaceB,
bvector<PolyfaceHeaderPtr> &resultA,
bvector<PolyfaceHeaderPtr> &resultB
)
    {
    //static int s_dumpAll = 0;                                         unused var removed in Graphite
    //static bool sbRegularize = true;                                  unused var removed in Graphite
    //static bool sbExpandRange = true;                                 unused var removed in Graphite
    //static bool s_compress = true;                                    unused var removed in Graphite
    //static bool s_doColors = false;                                   unused var removed in Graphite
    //static double   s_angleTol = 0.55;                                unused var removed in Graphite
    //static double   s_clusterRelTol = 0.0;                            unused var removed in Graphite
    //static double   s_outputRelTol  = 1.0e-6;                         unused var removed in Graphite
    //static double   s_maxFringeFraction = 10.0;                       unused var removed in Graphite
    //static double   s_areaRelTol = 0.1; // 2e-14 allows or tighter makes long slivers in TR162090 look    unused var removed in Graphite
                                        // like exterior triangle.
                                        // Because we construct the outer rectangle, there's no need to
                                        // fuss over what's "close to" zero.
    //static double   percentIncrement = -1.0 / 12.0; unused var removed in Graphite

    //static int s_addIntermediatePolygonsToFile = true;                unused var removed in Graphite


    TaggedPolygonVector allInputPolygons;
    // In allInputPolygons, IndexA = 0 or 1 selecting primary or barrier as parent.
    //    IndexB = index in parent.
    polyfaceA.AddToTaggedPolygons (allInputPolygons, 0, 0);
    DRange3d primaryRange = polyfaceA.PointRange ();
    polyfaceB.AddToTaggedPolygons (allInputPolygons, 1, 0, &primaryRange);

    DRange3d inputRange = PolygonVectorOps::GetRange (allInputPolygons);
#ifdef CompileDebugPolygons
    static int s_dumpInputs = 0;
    if (s_dumpInputs)
        {
        Transform translation;
        translation.InitFrom (0,primaryRange.high.y - primaryRange.low.x, 0);
        SavePolygons (primaryPolygons, NULL);
        SavePolygons (barrierPolygons, NULL);
        }
#endif



    double planarityAbsTol = s_planarityLocalRelTol * inputRange.low.Distance (inputRange.high);
    DPoint3d rangeCenter;
    rangeCenter.Interpolate (inputRange.low, 0.5, inputRange.high);

    Transform localToWorld, worldToLocal;
    localToWorld.InitFrom (rangeCenter);
    worldToLocal.InitFrom (-rangeCenter.x, -rangeCenter.y, -rangeCenter.z);

    PolygonVectorOps::Multiply (allInputPolygons, worldToLocal);

    worldToLocal.Multiply (inputRange, inputRange);    // worldToLocal is just translation, so this is exact.

    TaggedPolygonVector visiblePolygons;

    static int s_mergeTopAndBottom = 3;
    //static int s_saveFillTop = 1;                             unused var removed in Graphite
    //static int s_saveFillBottom = 1;                          unused var removed in Graphite
    //static int s_saveCutTop = 1;                              unused var removed in Graphite
    //static int s_saveCutBottom = 2;                           unused var removed in Graphite

    PolygonVectorOps::TriangulateNonPlanarPolygons (allInputPolygons, planarityAbsTol);
    bvector <TaggedPolygonVector> debugPolygons;
    TaggedPolygonVector out1, out2, out3, out4;
    // edl feb 9, 2016 . . .
    static int s_select = 0;
    if (s_select == 1)
        {
        PolygonVectorOps::CutAndFill_IndexedHeap (allInputPolygons, out1, out2, out3, out4, debugPolygons);
        }
    else
        {
        PolygonVectorOps::CutAndFill (allInputPolygons, out1, out2, out3, out4, debugPolygons);
        }

    if (s_mergeTopAndBottom != 0)
        {
        SavePolygons (resultA, out1, &out4);
        SavePolygons (resultB, out2, &out3);
        }
    else
        {
        SavePolygons (resultA, out1, NULL);
        SavePolygons (resultB, out2, NULL);
        SavePolygons (resultB, out3, NULL);
        SavePolygons (resultA, out4, NULL);
       }
    PolyfaceHeader::Transform (resultA, localToWorld);
    PolyfaceHeader::Transform (resultB, localToWorld);
#ifdef CompileDebugPolygons
    double dx = inputRange.high.x - inputRange.low.x;

    for (size_t i = 0; i < debugPolygons.size (); i++)
        {
        Transform localToWorldB = localToWorld;
        localToWorldB.form3d[0][3] += dx * (i + 1) * 1.2;
        SavePolygons (debugPolygons[i], NULL, localToWorldB, 1);
        }
#endif
    }


struct PunchClipSearcher : XYZRangeTreeHandler
{
private:
CutAndFillSplitter m_splitter;
DRange3d      m_punchRange, m_targetRange;
DRange3d      m_searchRange;
TaggedPolygonVectorCR m_punchPolygons;
TaggedPolygonVectorCR m_targetPolygons;
double m_absTol;
size_t m_currIndex;
bool  m_keepInside;

public:

// keepInside==true is "clip"  -- keep the parts under the puncher
// keepInside==false is "punch" -- eliminate the parts under the puncher
PunchClipSearcher (TaggedPolygonVectorCR target, TaggedPolygonVectorCR punch, bool keepInside, double abstol)
    : m_punchPolygons (punch),
      m_targetPolygons (target),
      m_splitter (punch, abstol, 1.0),
      m_keepInside (keepInside)
    {
    m_punchRange  = PolygonVectorOps::GetRange (m_punchPolygons);
    m_targetRange = PolygonVectorOps::GetRange (m_targetPolygons);
    m_absTol = abstol;
    ClearCounts ();
    }

~PunchClipSearcher ()
    {
    }


public:
    size_t m_leafHit;
    size_t m_leafSkip;
    size_t m_subtreeHit;
    size_t m_subtreeSkip;

void ClearCounts ()
    {
    m_leafHit       = 0;
    m_leafSkip      = 0;
    m_subtreeHit    = 0;
    m_subtreeSkip   = 0;
    }

bool LoadTargetPolygon (size_t index)
    {
    m_currIndex = index;
    bool stat = m_splitter.LoadPrimaryPolygon (m_targetPolygons[index], index);
    m_searchRange = m_splitter.GetSearchRange ();
    m_searchRange.high.z = DBL_MAX;
    m_searchRange.low.z  = -DBL_MAX;
    return stat;
    }

bool ShouldRecurseIntoSubtree (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior) override 
    {
    DRange3d nodeRange = pInterior->Range ();
    if (m_searchRange.IntersectsWith (nodeRange))
        {
        m_subtreeHit++;
        return true;
        }
    else
        {
        m_subtreeSkip++;
        return false;
        }
    }

bool ShouldContinueAfterSubtree      (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior)    override {return true;}

bool ShouldContinueAfterLeaf         (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior, XYZRangeTreeLeafP pLeaf) override 
    {
    size_t leafIndex = (size_t)pLeaf->GetData ();
    DRange3d nodeRange = pLeaf->Range ();
    if (m_searchRange.IntersectsWith (nodeRange))
        {
        m_splitter.ApplyXYClip (m_punchPolygons[leafIndex].GetPointsCR (),
                    m_keepInside ?  CutAndFillSplitter::s_indexPolygonsBelow : CutAndFillSplitter::s_indexPolygonsAbove);
        m_leafHit++;
        }
    else
        {
        m_leafSkip++;
        }
    return true;
    }

void CollectClip (TaggedPolygonVectorR collector)
    {
    if (m_keepInside)
        m_splitter.CollectClip (collector, false, true);
    else
        m_splitter.CollectClip (collector, true, false);
    }
    
void AddDebugPolygons (bvector<TaggedPolygonVector> &dest, double dyLowerSurface)
    {
    DVec3d upperShift;
    upperShift.Zero ();
    m_splitter.AddDebugPolygons (dest, upperShift);
    }
  
};  // PunchClipSearcher !!!!




void PolygonVectorOps::Punch
(
TaggedPolygonVectorCR targets,
TaggedPolygonVectorCR cutters,
bool keepInside,
TaggedPolygonVectorR  output,
bvector<TaggedPolygonVector> &debugShapes
)
    {
    output.clear ();

    DRange3d targetRange = PolygonVectorOps::GetRange (targets);
    //DRange3d punchRange = PolygonVectorOps::GetRange (punch);
    double abstol = s_relTol * targetRange.LargestCoordinate ();
    XYZRangeTreeRootP rangeTree = XYZRangeTreeRoot::Allocate ();
    size_t numCutters = cutters.size ();
    size_t numTargets = targets.size ();

#ifdef PRINT_STATS
    if (s_debug > 2)
        printf ("(targts %d) (cutters %d)\n", numTargets, numCutters);
#endif
    static double s_rangeAddition = 100.0;  // make punch polygons seem like top to bottom columns
    PunchClipSearcher searcher (targets, cutters, keepInside, abstol);
    for (size_t i = 0; i < numCutters; i++)
        {
        DRange3d range;
        DPlane3d plane;
        if (CutAndFillSplitter::GetNonVerticalPolygonPlane (cutters[i], plane, range))
            {
            range.low.z = targetRange.low.z - s_rangeAddition;
            range.high.z = targetRange.high.z + s_rangeAddition;
            rangeTree->Add ((void*)i, range);
            }
        }

    StopWatch totalSearchTime;
    StopWatch traverseTime;
    StopWatch collectTime;
    StopWatch loadTime;
#ifdef PRINT_STATS
    XYZRangeTreeCounter counter = XYZRangeTreeCounter ();
    rangeTree->Traverse (counter);
    printf ("(RangeTree (#leaf %d) (#fringe %d) (#interior %d) (#maxDepth %d) (sumLeafSquared %ld) \n",
                    counter.mNumLeaf, counter.mNumFringe,
                    counter.mNumInterior, counter.mMaxDepth,
                    counter.mSumFringeLeafCountSquared
                    );
#endif
    static ptrdiff_t s_singlePolygonSelector = -1;
    if (s_singlePolygonSelector >= 0 && s_singlePolygonSelector < (ptrdiff_t)numTargets)
        {
        size_t i = (size_t)s_singlePolygonSelector;
        searcher.LoadTargetPolygon (i);
        rangeTree->Traverse (searcher);
        searcher.CollectClip (output);
        }
    else
        {
        totalSearchTime.Start ();
        searcher.ClearCounts ();
        for (size_t i = 0; i < numTargets; i++)
            {
            loadTime.Start ();
            bool loaded = searcher.LoadTargetPolygon (i);
            loadTime.Stop ();
            if (loaded)
                {
                traverseTime.Start ();
                rangeTree->Traverse (searcher);
                traverseTime.Stop ();
                collectTime.Start ();
                searcher.CollectClip (output);
                collectTime.Stop ();
                }
            }
        totalSearchTime.Stop ();
    #ifdef PRINT_STATS
            printf ("(Subtree %d %d) (Leaf %d %d)\n",
                        searcher.m_subtreeHit,
                        searcher.m_subtreeSkip,
                        searcher.m_leafHit,
                        searcher.m_leafSkip
                        );
    #endif
        }

    static double s_debugShiftFactor = 1.4;
    searcher.AddDebugPolygons (debugShapes, (targetRange.high.y - targetRange.low.y) * s_debugShiftFactor);
    double loadFraction = FractionOf (loadTime, totalSearchTime);
    double traverseFraction = FractionOf (traverseTime, totalSearchTime);
    double collectFraction = FractionOf (collectTime, totalSearchTime);
    KeepAlive (loadFraction);
    KeepAlive (traverseFraction);
    KeepAlive (collectFraction);
    XYZRangeTreeRoot::Free (rangeTree);
    }

void PolyfaceQuery::ComputePunch
(
PolyfaceQueryCR punch,
PolyfaceQueryCR target,
bool keepInside,
bvector<PolyfaceHeaderPtr> &result
)
    {
    //static int s_dumpAll = 0;                                 unused var removed in Graphite
    //m_drapeGraph = new DrapeGraph ();
    //static bool sbRegularize = true;                          unused var removed in Graphite
    //static bool sbExpandRange = true;                         unused var removed in Graphite
    //static bool s_compress = true;                            unused var removed in Graphite
    //static bool s_doColors = false;                           unused var removed in Graphite
    //static double   s_angleTol = 0.55;                        unused var removed in Graphite
    //static double   s_clusterRelTol = 0.0;                    unused var removed in Graphite
    //static double   s_outputRelTol  = 1.0e-6;                 unused var removed in Graphite
    static double   s_planarityLocalRelTol = 1.0e-8;
    //static double   s_maxFringeFraction = 10.0;               unused var removed in Graphite
    //static double   s_areaRelTol = 0.1; // 2e-14 allows or tighter makes long slivers in TR162090 look    unused var removed in Graphite
                                        // like exterior triangle.
                                        // Because we construct the outer rectangle, there's no need to
                                        // fuss over what's "close to" zero.
    //static double   percentIncrement = -1.0 / 12.0; unused var removed in Graphite

    //static int s_addIntermediatePolygonsToFile = true;        unused var removed in Graphite

    TaggedPolygonVector punchPolygons, targetPolygons;
    
    punch.AddToTaggedPolygons (punchPolygons, 0, 0);
    target.AddToTaggedPolygons (targetPolygons, 0, 0);

    DRange3d inputRange = PolygonVectorOps::GetRange (targetPolygons);


    double planarityAbsTol = s_planarityLocalRelTol * inputRange.low.Distance (inputRange.high);
    DPoint3d rangeCenter;
    rangeCenter.Interpolate (inputRange.low, 0.5, inputRange.high);

    //double xyFringeSize = 0.05 * inputRange.low.Distance (inputRange.high);

    Transform localToWorld, worldToLocal;
    localToWorld.InitFrom (rangeCenter);
    worldToLocal.InitFrom (-rangeCenter.x, -rangeCenter.y, -rangeCenter.z);

    PolygonVectorOps::Multiply (targetPolygons, worldToLocal);
    PolygonVectorOps::Multiply (punchPolygons, worldToLocal);
    worldToLocal.Multiply (inputRange, inputRange);    // worldToLocal is just translation, so this is exact.


    PolygonVectorOps::TriangulateNonPlanarPolygons (targetPolygons, planarityAbsTol);
    bvector<TaggedPolygonVector> debugPolygons;
    TaggedPolygonVector outputPolygons;

    PolygonVectorOps::Punch (targetPolygons, punchPolygons, keepInside, outputPolygons, debugPolygons);

    SavePolygons (result, outputPolygons, NULL);
    PolyfaceHeader::Transform (result, localToWorld);

#ifdef CompileDebugPolygons
    double dx = inputRange.high.x - inputRange.low.x;

    for (size_t i = 0; i < debugPolygons.size (); i++)
        {
        Transform localToWorldB = localToWorld;
        localToWorldB.form3d[0][3] += dx * (i + 1) * 1.2;
        SavePolygons (debugPolygons[i], NULL, localToWorldB, 1);
        }
#endif

    }

void PolyfaceQuery::AddToTaggedPolygons
(
TaggedPolygonVectorR polygons,
ptrdiff_t indexA,
size_t numWrap,
DRange3dCP selectRange
) const
    {
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    visitor->SetNumWrap ((uint32_t)numWrap);
    bvector<DPoint3d> &points = visitor->Point ();
    if (NULL == selectRange)
        {
        for (visitor->Reset (); visitor->AdvanceToNextFace ();)
            PolygonVectorOps::AddPolygon (polygons, points, indexA, (ptrdiff_t)visitor->GetReadIndex (), 0.0);
        }
    else
        {
        DRange3d facetRange;
        for (visitor->Reset (); visitor->AdvanceToNextFace ();)
            {
            facetRange.InitFrom (points);
            if (facetRange.IntersectsWith (*selectRange, 2))
                PolygonVectorOps::AddPolygon (polygons, points, indexA, (ptrdiff_t)visitor->GetReadIndex (), 0.0);
            }
        }            
    }

void PolyfaceQuery::AddToTaggedPolygons
(
TaggedPolygonVectorR polygons,
ptrdiff_t indexA,
size_t numWrap,
IPolyfaceVisitorFilter *filter
) const
    {
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    visitor->SetNumWrap ((uint32_t)numWrap);
    bvector<DPoint3d> &points = visitor->Point ();
    if (NULL == filter)
        {
        for (visitor->Reset (); visitor->AdvanceToNextFace ();)
            PolygonVectorOps::AddPolygon (polygons, points, indexA, (ptrdiff_t)visitor->GetReadIndex (), 0.0);
        }
    else
        {
        for (visitor->Reset (); visitor->AdvanceToNextFace ();)
            {
            if (filter->TestFacet (*visitor))
                PolygonVectorOps::AddPolygon (polygons, points, indexA, (ptrdiff_t)visitor->GetReadIndex (), 0.0);
            }
        }            
    }

END_BENTLEY_GEOMETRY_NAMESPACE


#include "pf_undercut.h"

void PolyfaceQuery::ComputePunchXYByPlaneSets
(
PolyfaceQueryCR punch,
PolyfaceQueryCR target,
PolyfaceHeaderPtr *inside,
PolyfaceHeaderPtr *outside,
PolyfaceHeaderPtr *debug,
bool computeAndApplyTransform
)
    {
    TaggedPolygonVector punchPolygons, targetPolygons;
    
    punch.AddToTaggedPolygons (punchPolygons, 0, 0);
//    target.AddToTaggedPolygons (targetPolygons, 0, 0);

//    DRange3d inputRange = PolygonVectorOps::GetRange (targetPolygons);
    DRange3d inputRange = target.PointRange ();
    DPoint3d rangeCenter;
    rangeCenter.Interpolate (inputRange.low, 0.5, inputRange.high);

    Transform localToWorld, worldToLocal;
    localToWorld.InitFrom (rangeCenter);
    worldToLocal.InitFrom (-rangeCenter.x, -rangeCenter.y, -rangeCenter.z);
    static bool s_useWorld = false;
    if (s_useWorld || !computeAndApplyTransform)
        {
        worldToLocal.InitIdentity ();
        localToWorld.InitIdentity ();
        }

    //PolygonVectorOps::Multiply (targetPolygons, worldToLocal);
    PolygonVectorOps::Multiply (punchPolygons, worldToLocal);
    worldToLocal.Multiply (inputRange, inputRange);    // worldToLocal is just translation, so this is exact.
    TaggedPolygonVector insidePolygons, outsidePolygons, debugPolygons;

    PolygonVectorOps::PunchByPlaneSets (target, worldToLocal, punchPolygons,
            inside != nullptr ? &insidePolygons : nullptr,
            outside != nullptr ? &outsidePolygons : nullptr,
            debug != nullptr ? &debugPolygons : nullptr
            );

    if (inside != nullptr)
        *inside = PolygonsToMesh (insidePolygons, &localToWorld);

    if (outside != nullptr)
        *outside = PolygonsToMesh (outsidePolygons, &localToWorld);

    if (debug!= nullptr)
        *debug = PolygonsToMesh (debugPolygons, &localToWorld);

    }
// Compute what parts of meshB are hidden by meshA (the hider).   Both returned meshes are null 
// if there is no hiding.
void PolyfaceHeader::MeshHidesMeshXYByPlaneSets (
PolyfaceHeaderPtr &hider,   // 
PolyfaceHeaderPtr &hidable,
PolyfaceHeaderPtr &meshBVisible,
PolyfaceHeaderPtr &meshBHidden,
bool computeAndApplyTransform
)
    {
    PolyfaceHeaderPtr meshAOverB, meshBUnderA;
    PolyfaceHeader::ComputeOverAndUnderXY (*hider, nullptr, *hidable, nullptr, meshAOverB, meshBUnderA, computeAndApplyTransform);
    if (meshBUnderA.IsValid ())
        {
        meshBUnderA->Triangulate();
        PolyfaceHeader::ComputePunchXYByPlaneSets (*meshBUnderA, *hidable, &meshBHidden, &meshBVisible, nullptr, computeAndApplyTransform);
        }
    }
// Compute pairwise hidden-visible splits, and replace each input mesh by its visible parts.
// Note that meshes that become fully hidden become nullptr in the allMesh array.
// Each mesh is individually assumed "upward facing"
void PolyfaceHeader::MultiMeshVisiblePartsXYByPlaneSets (bvector<PolyfaceHeaderPtr> &allMesh, bvector<PolyfaceHeaderPtr> &visibleParts)
    {
    // Pretransform to an origin-centered range with sensible size ..
    BentleyApi::Transform localToWorld, worldToLocal;
    DRange3d worldRange;
    worldRange.Init ();
    for (auto &mesh : allMesh)
        worldRange.Extend (mesh->PointRange ());

    double a = 1000.0;
    DRange3d localRange = DRange3d::From (-a, -a, -a, a, a, a);
    Transform::TryUniformScaleXYRangeFit (worldRange, localRange, worldToLocal, localToWorld);
    for (auto &mesh : allMesh)
        mesh->Transform (worldToLocal);

    bvector<DRange3d> ranges;
    visibleParts.clear ();
    for (auto &mesh : allMesh)
        {
        ranges.push_back (mesh->PointRange ());
        visibleParts.push_back (mesh->Clone ());
        }
    size_t numMesh = visibleParts.size ();
    for (size_t index0 = 0; index0 < numMesh; index0++)
        {
        // index0 is the mesh being hidden.
        for (size_t index1 = 0; visibleParts[index0].IsValid () && index1 < numMesh; index1++)
            {
            // index1 is a hider mesh
            if (index0 != index1
                && visibleParts[index1].IsValid ()
                && ranges[index0].IntersectsWith (ranges[index1], 2))
                {
                PolyfaceHeaderPtr visibleFacets, hiddenFacets;
                MeshHidesMeshXYByPlaneSets (allMesh[index1], visibleParts[index0], visibleFacets, hiddenFacets, false);
                // !!! double null means nothing hidden -- leave everything alone.
                if (visibleFacets.IsValid () || hiddenFacets.IsValid ())
                    {
                    visibleParts[index0] = visibleFacets;
                    if (visibleFacets.IsValid ())
                        ranges[index0] = visibleFacets->PointRange ();
                    else
                        ranges[index0].Init ();
                    }
                }
            }
        }
    for (auto &mesh : allMesh)
        mesh->Transform (localToWorld);
    for (auto &mesh : visibleParts)
        mesh->Transform (localToWorld);
    }

