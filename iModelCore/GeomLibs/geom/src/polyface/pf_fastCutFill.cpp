/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/pf_fastCutFill.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Bentley/BeTimeUtilities.h>


#include <Geom/XYZRangeTree.h>
#include <Vu/VuApi.h>
//#include <assert.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL;

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct SingleSheetCutFillContext;

static int s_cutFillNoisy = 0;

// objects:
// SingleSheetCutFillFloodContext -- runs DFS in graph constructed elsewhere.
// SingleSheetCutFillContext -- constructs graph, initiates cut SingleSheetCutFillFloodContext

struct IScanProcessor
{
GEOMAPI_VIRTUAL void ProcessFace (SingleSheetCutFillContext &context, VuSetP graph, VuP node, size_t dtmFace, size_t roadFace)
    {
    bool isCutFillFace = dtmFace != SIZE_MAX && roadFace != SIZE_MAX;
    if (s_cutFillNoisy > 0)
    printf (" ProcessFace (id %d %g) (dtm %d) (road %d)\n",
            vu_getIndex (node), vu_area (node), (int)dtmFace, (int)roadFace);
    if (s_cutFillNoisy > 4 || (s_cutFillNoisy > 2 && isCutFillFace))
        vu_printFaceLabelsOneFace (graph, node);
    }
GEOMAPI_VIRTUAL bool ContinueSearch () { return true;}

// Announce coordinates in paired dtm and road loops.
// <ul>
// <li>both loops are oriented CCW.
//
GEOMAPI_VIRTUAL void ProcessCutFillFacet (bvector<DPoint3d> &dtm, size_t dtmReadIndex, bvector<DPoint3d> &road, size_t roadReadIndex, bool isCut) {}

};


struct SingleSheetCutFillFloodContext
{

void PrintFace (VuP node, char const *name)
    {
    if (s_cutFillNoisy < 10)
        return;
    printf ("%s\n", name);
    vu_printFaceLabelsOneFace (m_graph, node);
    }
bvector<VuP> m_stack;
VuMask m_floodMask;        // grab/return for flood
VuMask m_faceEnteredMask;  // grab/return for flood
VuSetP m_graph;
VuMask m_roadMask;  // supplied by caller
VuMask m_dtmMask;   // supplied by caller
VuMask m_roadExteriorMask;  // supplied by caller
VuMask m_dtmExteriorMask; // supplied by caller
IScanProcessor &m_faceProcessor;
SingleSheetCutFillContext &m_cutFillContext;
MeshAnnotationVector &m_messages;

SingleSheetCutFillFloodContext (
        SingleSheetCutFillContext &cutFillContext,
        MeshAnnotationVector &messages,
        IScanProcessor &faceProcessor,
        VuSetP graph,
        VuMask dtmMask,
        VuMask dtmExteriorMask,
        VuMask roadMask,
        VuMask roadExteriorMask
        )
    : m_graph(graph),
      m_messages(messages),
      m_faceProcessor (faceProcessor),
      m_cutFillContext (cutFillContext),
      m_dtmMask (dtmMask),
      m_roadMask (roadMask),
      m_dtmExteriorMask (dtmExteriorMask),
      m_roadExteriorMask (roadExteriorMask)
    {
    m_floodMask = vu_grabMask (graph);
    m_faceEnteredMask = vu_grabMask (graph);

    vu_clearMaskInSet (graph, m_floodMask | m_faceEnteredMask);
    }

~SingleSheetCutFillFloodContext ()
    {
    vu_returnMask (m_graph, m_floodMask);
    vu_returnMask (m_graph, m_faceEnteredMask);
    }

// UNCONDITIONAL mark and push -- caller does all tests
void MaskAndPush (VuP node)
    {
    vu_setMask (node, m_floodMask);
    if (!vu_getMask (node, m_faceEnteredMask))
        vu_setMaskAroundFace (node, m_faceEnteredMask);
    m_stack.push_back (node);
    }
// Call when crossing into a face.
// The entry edge is marked as (just one of) dtm or road.
// If also marked exterior (of the dtm or road) set the respective index to SIZE_MAX
// Otherwise set the index to the plane index from the 
void MoveToFace (VuP node, size_t &dtmIndex, size_t &roadIndex)
    {
    // Are we changing face for either the road or the dtm?
    if (vu_getMask (node, m_dtmMask))
        {
        dtmIndex = vu_getMask (node, m_dtmExteriorMask) ? SIZE_MAX : vu_getUserData1 (node);
        }
    else if (vu_getMask (node, m_roadMask))
        {
        roadIndex = vu_getMask (node, m_roadExteriorMask) ? SIZE_MAX : vu_getUserData1 (node);
        }
    }

void ScanFromFloodCandidate (VuP seed)
    {
    // The search can only start from a node that is dtmExterior.
    // Once a search runs from any given dtmExterior, the complete connected component will have m_floodMask.
    // (and also m_faceEnteredMask, but we'll just test for m_floodMask)
    if (vu_getMask (seed, m_floodMask) || !vu_getMask (seed, m_dtmExteriorMask))
        return;
    m_stack.clear ();
    MaskAndPush (seed);
    PrintFace (seed, "\n\nComponent Seed");

    size_t dtmFace = SIZE_MAX;  // reset this on each advance and retreat
    size_t roadFace = SIZE_MAX; // reset this on each advance and retreat
    while (m_faceProcessor.ContinueSearch () && !m_stack.empty ())
        {
        if (s_cutFillNoisy)
            {
            printf ("stack:\n");
            for (auto node : m_stack)
                {
                printf ("(%d)", node->id);
                }
            printf ("\n");
            }
        VuP faceSeed = m_stack.back ();
        m_stack.pop_back ();
        VuP candidate = vu_edgeMate (faceSeed);
        PrintFace (faceSeed, "Pull from Stack");
        if (s_cutFillNoisy && vu_countEdgesAroundFace (faceSeed) == 2)
            printf ("!! at null face %d\n", faceSeed->id);
            
        if (!vu_getMask (candidate, m_faceEnteredMask))
            {
            PrintFace (candidate, "jump edge into candidate");
            m_stack.push_back (faceSeed);
            MaskAndPush (candidate);
            MoveToFace (candidate, dtmFace, roadFace);
            m_faceProcessor.ProcessFace (m_cutFillContext, m_graph, candidate, dtmFace, roadFace);
            }
        else
            {
            // cannot step out of this face from this edge.
            // creep forward around this face.
                if (s_cutFillNoisy > 0)
                    printf (" fs creep from %d\n", faceSeed->id);
            VuP fs = vu_fsucc (faceSeed);
            while (!vu_getMask (fs, m_floodMask)
                && vu_getMask (vu_edgeMate (fs), m_faceEnteredMask))
                {
                if (s_cutFillNoisy > 0)
                    printf ("(%d)", fs->id);
                vu_setMask (fs, m_floodMask);
                fs = vu_fsucc (fs);
                }

            if (vu_getMask (fs, m_floodMask))
                {
                if (s_cutFillNoisy > 0)
                    printf (" return to face entry\n");
                // We have gone all the way around the face to where it was entered.
                VuP mate = vu_edgeMate(fs);
                if (!m_stack.empty ())
                    {
                    if (s_cutFillNoisy > 0)
                        printf (" pop stack\n");
//                    Check::True (mate == m_stack.back ());
                    //m_stack.pop_back ();
                    MoveToFace (mate, dtmFace, roadFace);
                    }
                }
            else
                {
                if (s_cutFillNoisy > 0)
                    printf (" setup for edge jump after fs sequence\n");
                // fs is unvisited. But its edge mate might not be.
                // whose neighbors might be unvisited.
                // push back on the stack -- edge mate will be explored next.
                MaskAndPush (fs);
                }
            }
        }

    }
};


 struct ZPlaneDescriptor
 {
 size_t m_sourceId;
 size_t m_indexInSource;
DPoint3d m_xyz;
DPoint3d m_normal;
double   m_reciprocalZ;     // for z = f(x,y) = m_xyz.z + m_reciprocalZ * (...) see method
DRange1d m_zRange;
 ZPlaneDescriptor (DPoint3dCR xyz, DVec3dCR normal, DRange1dCR zRange, size_t sourceId, size_t indexInSource, double divideByZTolerance = 0.0)
    : m_sourceId (sourceId),
      m_indexInSource (indexInSource),
      m_normal(normal),
      m_xyz(xyz),
      m_zRange(zRange)
    {
    auto rz = DoubleOps::ValidatedDivide (1.0, normal.z, 0.0, divideByZTolerance);
    if (rz.IsValid ())
        m_reciprocalZ = - rz;
    else
        m_reciprocalZ = 0.0;
    }
bool IsVerticalPlane (){ return m_reciprocalZ == 0.0;}
size_t IndexInSource () const {return m_indexInSource;}
// Evaluate Z at specified x,y.  (If the face was vertical, a zero reciprocalZ was installed and this method quietly returns the reference point z)
double EvaluateZ (double x, double y) const
    {
    double z = m_xyz.z + m_reciprocalZ * ((x - m_xyz.x) * m_normal.x + (y - m_xyz.y) * m_normal.y);
    if (z > m_zRange.high)
        z = m_zRange.high;
    if (z < m_zRange.low)
        z = m_zRange.low;
    return z;
    }
DPoint4d PlaneCoefficients () const
    {
    return DPoint4d::From (
            m_reciprocalZ * m_normal.x,
            m_reciprocalZ * m_normal.y,
            -1.0,
            m_xyz.z - m_reciprocalZ * (m_normal.x * m_xyz.x + m_normal.y * m_xyz.y)
            );
    }
DPoint4d VerticalPlaneThroughIntersection (ZPlaneDescriptor const &other) const
    {
    DPoint4d planeA = PlaneCoefficients ();
    DPoint4d planeB = other.PlaneCoefficients ();
    DPoint4d difference;
    difference.DifferenceOf (planeA, planeB);
    return difference;
    }
  };

struct SingleSheetCutFillContext
{
static const VuMask s_dtmMask = VU_KNOT_EDGE;
static const VuMask s_dtmExteriorMask = VU_SEAM_EDGE;
static const VuMask s_roadMask = VU_RULE_EDGE;
static const VuMask s_roadExteriorMask = VU_GRID_EDGE;
bvector<ZPlaneDescriptor> m_facePlanes;
MeshAnnotationVector &m_messages;
VuSetP m_graph;
double m_divideByZTolerance;

SingleSheetCutFillContext (MeshAnnotationVector &messages)
    : m_messages (messages)
    {
    m_graph = vu_newVuSet (0);
    vu_setDefaultUserData1 (m_graph, -1, false, true);
    m_divideByZTolerance = 1.0e-8;//Angle::SmallAngle ();
    }
~SingleSheetCutFillContext (){vu_freeVuSet (m_graph);}

void StartLoadSequence () {}

size_t AddFace (DPoint3dCR xyz, DVec3dCR normal, DRange1dCR zRange, size_t sourceId, size_t indexInSource)
    {
    ZPlaneDescriptor face (xyz, normal, zRange, sourceId, indexInSource, m_divideByZTolerance);
    size_t index = m_facePlanes.size ();
    m_facePlanes.push_back (face);
    return index;
    }

bool HasValidLastFace (bool deleteIfNotValid)
    {
    if (m_facePlanes.empty ())
        return false;
    if (m_facePlanes.back ().IsVerticalPlane ())
        {
        if (deleteIfNotValid)
            m_facePlanes.pop_back ();
        return false;
        }
    return true;
    }
DRange1d GetPlaneZRange (size_t planeIndex) const
    {
    if (planeIndex < m_facePlanes.size ())
        return m_facePlanes[planeIndex].m_zRange;
    return DRange1d::NullRange ();
    }

DPoint4d VerticalPlaneThroughIntersection (size_t indexA, size_t indexB) const
    {
    if (indexA < m_facePlanes.size () && indexB < m_facePlanes.size ())
        {
        return m_facePlanes[indexA].VerticalPlaneThroughIntersection (m_facePlanes[indexB]);
        }
    return DPoint4d::From (0.0, 0.0, 0.0, 0.0);
    }
// return a point with x,y from the node and z from the plane.
// If the plane is invalid, the z is from the node.
ValidatedDPoint3d EvaluateNodeOnPlane (VuP node, size_t planeIndex)
    {
    DPoint3d xyz = node->GetXYZ ();
    if (planeIndex < m_facePlanes.size ())
        {
        xyz.z = m_facePlanes[planeIndex].EvaluateZ (xyz.x, xyz.y);
        return ValidatedDPoint3d (xyz, true);
        }
    return ValidatedDPoint3d (xyz, false);
    }

ValidatedSize PlaneIndexToReadIndex (size_t planeIndex) const
    {
    if (planeIndex < m_facePlanes.size ())
        {
        return m_facePlanes[planeIndex].IndexInSource ();
        }
    return ValidatedSize (SIZE_MAX, false);
    }

// Create a loop with given coordinates.
// Mark both sides with edgeMask
// mark negative area side with exteriorMask;
// NOTE -- there is no check for self intersection, non-planar, zero-length edges.
// TODO: compute and save plane coefficients.
//       save plane index in node.
double LoadPolygon (bvector<DPoint3d> const &points, VuMask edgeMask, VuMask exteriorMask, ptrdiff_t faceId)
    {
    VuP node = nullptr;
    if (points.size () < 3)
        return 0.0;
    for (DPoint3d xyz : points)
        {
        VuP newNodeA, newNodeB;
        vu_splitEdge (m_graph, node, &newNodeA, &newNodeB, edgeMask, edgeMask);
        vu_setDPoint3d (newNodeA, &xyz);
        vu_setDPoint3d (newNodeB, &xyz);
        vu_setUserData1 (newNodeA, (ptrdiff_t)faceId);
        vu_setUserData1 (newNodeB, (ptrdiff_t)faceId);
        node = newNodeA;
        }
    double area = vu_area (node);
    if (area < 0)
        vu_setMaskAroundFace (node, exteriorMask);
    else
        vu_setMaskAroundFace (vu_vsucc(node), exteriorMask);
    return area;
    }
// return false if the merge found edge intersections.
bool FinishLoadSequence (VuMask exteriorMask)
    {
    int num0 = vu_countNodesInGraph (m_graph);
    vu_mergeOrUnionLoops (m_graph, VUUNION_UNION);
    int num1 = vu_countNodesInGraph (m_graph);
    // ?? do we need to regularize?
    vu_exchangeNullFacesToBringMaskInside (m_graph, exteriorMask);
    vu_exciseNullFaces (m_graph, exteriorMask);
    return num0 == num1;
    }

ValidatedDouble LoadPolyface (PolyfaceQueryCR mesh, VuMask edgeMask, VuMask exteriorMask, size_t sourceId, IPolyfaceVisitorFilter *filter)
    {
    auto visitor = PolyfaceVisitor::Attach (mesh, false);
    int errors = 0;
    StartLoadSequence ();
    SignCounter counter;
    bvector<DPoint3d> &points = visitor->Point ();
    double totalArea = 0.0;
    DRange1d zRange;
    DRange1d zRange1;
    zRange.InitNull ();
    zRange1.InitNull ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        if (filter != nullptr && !filter->TestFacet (*visitor))
            continue;
        DPoint3d centroid;
        DVec3d normal;
        double area;
        if (visitor->TryGetFacetCentroidNormalAndArea (centroid, normal, area))
            {
            zRange.Extend (normal.z);
            DRange3d faceRange = DRange3d::From (points);
            auto faceIndex = AddFace (centroid, normal, DRange1d (faceRange.low.z, faceRange.high.z), sourceId, visitor->GetReadIndex ());
            if (HasValidLastFace (true))
                {
                zRange1.Extend (normal.z);
                area = LoadPolygon (points, edgeMask, exteriorMask, faceIndex);
                counter.Announce (area);
                totalArea += area;
#define CheckPlaneEquations_not
#ifdef CheckPlaneEquations
                DPoint4d planeCoffs = m_facePlanes[faceIndex].PlaneCoefficients ();
                for (auto xyz : points)
                    {
                    double z = m_facePlanes[faceIndex].EvaluateZ (xyz.x, xyz.y);
//                    Check::Near (xyz.z, z, "face plane z validation");
//                    Check::Near (0.0, planeCoffs.DotProduct (xyz.x, xyz.y, xyz.z, 1.0));
                    }
#endif
                }
            }
        }
    if (!(counter.NumPositive () == 0 || counter.NumNegative () == 0))
        {
        m_messages.Assert (false, "Mixed area signs");
        }
    if (!FinishLoadSequence (exteriorMask))
        {
        errors++;
        m_messages.Assert (false, "FinishLoadSequence failure");
        }
    if (0 != vu_countMaskChangesAroundFaces (m_graph, exteriorMask))
        {
        m_messages.Assert (false, "exterior mask consistency after merge.");
        errors++;
        }
    return ValidatedDouble (totalArea, errors == 0);
    }

ValidatedDouble LoadDTM (PolyfaceQueryCR mesh, IPolyfaceVisitorFilter *filter)
    {
    vu_stackPush (m_graph);
    auto area = LoadPolyface (mesh, s_dtmMask, s_dtmExteriorMask, 0, filter);

    vu_stackPop (m_graph);
    return area;
    }

ValidatedDouble LoadRoad (PolyfaceQueryCR mesh, IPolyfaceVisitorFilter *filter)
    {
    vu_stackPush (m_graph);
    double area = LoadPolyface (mesh, s_roadMask, s_roadExteriorMask, 1, filter);
    vu_stackPop (m_graph);
    return area;
    }

void Merge ()
    {
    if (s_cutFillNoisy > 1000)
        vu_printFaceLabels (m_graph, "before merge");
    vu_mergeOrUnionLoops (m_graph, VUUNION_UNION);
    if (s_cutFillNoisy > 1000)
        vu_printFaceLabels (m_graph, "after merge");

    int numVertices, numEdges, numFaces, eulerCharacteristic;
    vu_countLoops (m_graph, numVertices, numEdges, numFaces);
    eulerCharacteristic = numVertices - numEdges + numFaces;
    if (s_cutFillNoisy > 0)
        printf (" V - E + F = Euler: %d - %d + %d = %d\n",
                numVertices, numEdges, numFaces, eulerCharacteristic
                );
    if (eulerCharacteristic != 2)
        vu_regularizeGraph (m_graph);    
    }

void Scan (IScanProcessor &faceProcessor)
    {

    SingleSheetCutFillFloodContext floodState (
                *this,
                m_messages,
                faceProcessor, m_graph,
                s_dtmMask, s_dtmExteriorMask,
                s_roadMask, s_roadExteriorMask
                );

    VU_SET_LOOP (seed, m_graph)
        {
        floodState.ScanFromFloodCandidate (seed);
        }
    END_VU_SET_LOOP (seed, m_graph)
    }

};


void Print (UsageSums const &sums, char const *name, int printWhat = 1)
    {
    printf (" %s", name);
    if (printWhat >= 0)
        printf ("(n %g)", sums.Count ());
    if (printWhat >= 1 && sums.Count () > 0)
        printf ("(sum %g avg %g min %g max %g)", sums.Sum (), sums.Mean (), sums.Min (), sums.Max ());
    if (printWhat >= 2 && sums.Count () > 0)
        printf ("(stdDev %g)", sums.StandardDeviation ());
    printf ("\n");
    }


struct CutFillFacetSplitter : IScanProcessor
{
UsageSums m_overlapArea, m_roadArea, m_dtmArea, m_positiveArea, m_negativeArea;
FacetCutFillHandler &m_handler;
MeshAnnotationVector &m_messages;
CutFillFacetSplitter (FacetCutFillHandler &handler, MeshAnnotationVector &messages)
    : m_handler(handler),
    m_messages(messages)
    {
    }

bvector<DPoint3d> m_roadAbove;
bvector<DPoint3d> m_roadBelow;
bvector<DPoint3d> m_dtmAbove;
bvector<DPoint3d> m_dtmBelow;
bvector<bool> m_isCutBoundary, m_isFillBoundary;
void ClearLoops ()
    {
    m_roadAbove.clear ();
    m_dtmBelow.clear ();
    m_roadBelow.clear ();
    m_dtmAbove.clear ();
    m_isCutBoundary.clear ();
    m_isFillBoundary.clear ();
    }

// REMARK: node is the base of the original edge -- note xyz will be different at interpolated point.
void AddSimpleLoopPoint (SingleSheetCutFillContext &context, VuP node, DPoint3d xyzDtm, DPoint3d xyzRoad)
    {
    VuP mate = vu_edgeMate (node);
    auto isBoundary = vu_getMask (mate, context.s_roadExteriorMask) != 0;
    double dz = xyzRoad.z - xyzDtm.z;
    if (dz > 0.0)
        {
        m_roadAbove.push_back (xyzRoad);
        m_dtmBelow.push_back (xyzDtm);
        m_isFillBoundary.push_back (isBoundary);
        }
    else
        {
        m_roadBelow.push_back (xyzRoad);
        m_dtmAbove.push_back (xyzDtm);
        m_isCutBoundary.push_back (isBoundary);
        }
    }

// REMARK: node is the base of the original edge -- note xyz will be different at interpolated point.
void AddMidEdgePoint (SingleSheetCutFillContext &context, VuP node, DPoint3d xyzDtm, DPoint3d xyzRoad, double dz0, double dz1)
    {
    if (dz0 * dz1 >= 0.0)
        return;     // SHOULD NOT HAPPEN ...
    VuP mate = vu_edgeMate (node);
    auto isBoundary = vu_getMask (mate, context.s_roadExteriorMask) != 0;
    m_roadAbove.push_back (xyzRoad);
    m_dtmBelow.push_back (xyzDtm);

    m_roadBelow.push_back (xyzRoad);
    m_dtmAbove.push_back (xyzDtm);
    if (dz0 < 0.0)
        {
        // "from negative to positive" is cut to fill.   subsequent boundary is real in fill only
        m_isFillBoundary.push_back (isBoundary);
        m_isCutBoundary.push_back (false);
        }
    else
        {
        // "from negative to positive" is cut to fill.   subsequent boundary is real in cut only
        m_isFillBoundary.push_back (false);
        m_isCutBoundary.push_back (isBoundary);
        }
    }




void ProcessFace (SingleSheetCutFillContext &context, VuSetP graph, VuP faceSeed, size_t dtmFace, size_t roadFace) override
    {
    bool isCutFillFace = dtmFace != SIZE_MAX && roadFace != SIZE_MAX;
    double area = vu_area (faceSeed);
    if (dtmFace != SIZE_MAX && roadFace != SIZE_MAX)
        m_overlapArea.Accumulate (area);
    if (roadFace != SIZE_MAX)
        m_roadArea.Accumulate (area);
    if (dtmFace > 0.0)
        m_dtmArea.Accumulate (area);
    if (area > 0.0)
        m_positiveArea.Accumulate (area);
    if (area < 0.0)
        m_negativeArea.Accumulate (area);


    if (s_cutFillNoisy > 0)
    printf (" ProcessFace (id %d %g) (dtm %d) (road %d)\n",
            vu_getIndex (faceSeed), vu_area (faceSeed), (int)dtmFace, (int)roadFace);
    if (s_cutFillNoisy > 4 || (s_cutFillNoisy > 2 && isCutFillFace))
        vu_printFaceLabelsOneFace (graph, faceSeed);
    if (dtmFace != SIZE_MAX && roadFace != SIZE_MAX)
        {
        UsageSums zDiffs;

        VuP startNode = nullptr;    // start the loop somewhere other than the intersection of planes !!!
        DPoint3d dtmA, roadA;       // points at the startNode, then walking around the facet.
        double deltaA = 0.0;
        VU_FACE_LOOP (nodeA, faceSeed)
            {
            dtmA = context.EvaluateNodeOnPlane (nodeA, dtmFace);
            roadA = context.EvaluateNodeOnPlane (nodeA, roadFace);
            deltaA = roadA.z - dtmA.z;
            if (deltaA != 0.0)
                {
                startNode = nodeA;
                break;
                }
            }
        END_VU_FACE_LOOP (nodeA, faceSeed)

        if (startNode != nullptr)
            {
            VuP startNodeFSucc = faceSeed->FSucc ();

            ClearLoops ();
            // nodeA is not an intersection point.
            VuP nodeA = startNode;
            dtmA = context.EvaluateNodeOnPlane (nodeA, dtmFace);
            roadA = context.EvaluateNodeOnPlane (nodeA, roadFace);
            deltaA = roadA.z - dtmA.z;
            VU_FACE_LOOP (nodeB, startNodeFSucc)
                {
                AddSimpleLoopPoint (context, nodeA, dtmA, roadA);   // wrong -- what if "on"?
                DPoint3d dtmB = context.EvaluateNodeOnPlane (nodeB, dtmFace);
                DPoint3d roadB = context.EvaluateNodeOnPlane (nodeB, roadFace);
                double deltaB = roadB.z - dtmB.z;
                if (deltaA * deltaB < 0.0)  // strict interior crossing . . .
                    {
                    double s = DoubleOps::InverseInterpolate (deltaA, 0.0, deltaB); // deltaA, deltaB are of opposite sign -- result must be valid and in 0..1
                    DPoint3d dtmCrossing = DPoint3d::FromInterpolate (dtmA, s, dtmB);
                    DPoint3d roadCrossing = DPoint3d::FromInterpolate (roadA, s, roadB);     // this should match dtmCrossing !!!!
                    // force true zero delta . ..
                    dtmCrossing.z = roadCrossing.z;
                    AddMidEdgePoint (context, nodeA, dtmCrossing, roadCrossing, deltaA, deltaB);
                    }
                deltaA = deltaB;
                dtmA = dtmB;
                roadA = roadB;
                nodeA = nodeB;
                }
            END_VU_FACE_LOOP (nodeB, startNodeFSucc)

            //BeAssert (m_roadAbove.size () == m_dtmBelow ().size (), "Matching road, dtm loops");
            //BeAssert (m_roadBelow.size () == m_dtmAbove ().size (), "Matching road, dtm loops");

            auto dtmReadIndex = context.PlaneIndexToReadIndex (dtmFace);
            auto roadReadIndex = context.PlaneIndexToReadIndex (roadFace);
            if (m_roadAbove.size () > 1)
                {
                m_handler.ProcessCutFillFacet (m_dtmBelow, dtmReadIndex, m_roadAbove, roadReadIndex, m_isFillBoundary, false);
                }
            if (m_roadBelow.size () > 1)
                {
                m_handler.ProcessCutFillFacet (m_dtmAbove, dtmReadIndex, m_roadBelow, roadReadIndex, m_isCutBoundary, true);
                }
            }
        }
    }

void PrintAreaSummary ()
    {
    printf ("\n cut fill area summary \n");
    Print (m_positiveArea, "positive", 1);
    Print (m_negativeArea, "negative", 1);
    Print (m_dtmArea, "dtm    ", 1);
    Print (m_roadArea, "road   ", 1);
    Print (m_overlapArea, "overlap", 1);
    }
};

ValidatedDouble FacetCutFillHandler::ZVolumeBetweenFacets (bvector<DPoint3d> const &loopA, bvector<DPoint3d> const &loopB)
    {
    DPoint3d centroidA, centroidB;
    DVec3d normalA, normalB;
    double areaA, areaB;
    if (   !PolygonOps::CentroidNormalAndArea (loopA, centroidA, normalA, areaA)
        || !PolygonOps::CentroidNormalAndArea (loopB, centroidB, normalB, areaB))
            return ValidatedDouble (0.0, false);
    return ValidatedDouble (areaA * normalA.z * (centroidB.z - centroidA.z), true);
    }

void PolyfaceQuery::ComputeSingleSheetCutFill
(
PolyfaceQueryCR dtm,
PolyfaceQueryCR road,
FacetCutFillHandler &handler,
MeshAnnotationVector &messages,     //!< [out] messages about errors.
IPolyfaceVisitorFilter *dtmFilter,  //!< [in] optional filter for dtm
IPolyfaceVisitorFilter *roadFilter  //!< [in] optional filter for road  //!< [in,out] message logging structure.
)
    {
    messages.clear ();
    SingleSheetCutFillContext context (messages);
    // unused - auto dtmArea = context.LoadDTM (dtm, dtmFilter);
    // unused - auto roadArea = context.LoadRoad (road, roadFilter);
    context.Merge ();
    CutFillFacetSplitter faceProcessor (handler, messages);
    context.Scan (faceProcessor);
    if (s_cutFillNoisy > 0)
        faceProcessor.PrintAreaSummary ();
    }



struct CutFillMeshBuilder : FacetCutFillHandler
{
private:
IFacetOptionsPtr m_options;
IPolyfaceConstructionPtr m_cutBuilder;
IPolyfaceConstructionPtr m_fillBuilder;

public:
PolyfaceHeaderPtr GetCut (){return m_cutBuilder->GetClientMeshPtr ();}
PolyfaceHeaderPtr GetFill (){return m_fillBuilder->GetClientMeshPtr ();}

private:
bvector<size_t> m_newFacetIndices;
// add an index to the growing array -- but skip if already present.
void AnnounceNewFacetIndex (size_t index)
    {
    for (size_t i = 0; i < m_newFacetIndices.size(); i++)
        if (m_newFacetIndices[i] == index)
            return;
    m_newFacetIndices.push_back (index);
    }

void EmitNewFacetIndices (IPolyfaceConstructionPtr &builder)
    {
    if (m_newFacetIndices.size () >= 3)
        {
        for (auto index : m_newFacetIndices)
            builder->AddPointIndex (index, true);
        builder->AddPointIndexTerminator ();
        }
    }

void AddFacet (IPolyfaceConstructionPtr &builder, bvector<DPoint3d> const &points)
    {
    if (points.size () > 2)
        {
        m_newFacetIndices.clear ();
        for (auto xyz : points)
            {
            size_t pointIndex = builder->FindOrAddPoint (xyz);
            AnnounceNewFacetIndex (pointIndex);
            }
        EmitNewFacetIndices (builder);
        }
    }

void AddReversedFacet (IPolyfaceConstructionPtr &builder, bvector<DPoint3d> const &points)
    {
    if (points.size () > 2)
        {
        m_newFacetIndices.clear ();
        for (size_t i = points.size (); i > 0;)
            {
            i--;
            size_t pointIndex = builder->FindOrAddPoint (points[i]);
            AnnounceNewFacetIndex (pointIndex);
            }
        EmitNewFacetIndices (builder);
        }
    }

public:
// Add panels moving forward on edge of pointA, backward on edge of pointB.  i.e. pointA is lower facet.
void AddBoundaryQuads (IPolyfaceConstructionPtr &builder, bvector<DPoint3d> &pointA, bvector<DPoint3d> &pointB, bvector<bool> &isBoundary)
    {
    size_t n = pointA.size ();
    for (size_t i = 0; i < n; i++)
        {
        if (isBoundary[i])
            {
            // Normally there is a quad.
            // But it becomes a triangle if the upper, lower facets inersection along some edge.
            // And between those edges it is degenerate.
            size_t i1 = (i + 1) % n;
            m_newFacetIndices.clear ();
            AnnounceNewFacetIndex (builder->FindOrAddPoint (pointA[i]));
            AnnounceNewFacetIndex (builder->FindOrAddPoint (pointA[i1]));
            AnnounceNewFacetIndex (builder->FindOrAddPoint (pointB[i1]));
            AnnounceNewFacetIndex (builder->FindOrAddPoint (pointB[i]));
            EmitNewFacetIndices (builder);
            }
        }
    }

CutFillMeshBuilder ()
    {
    m_options = IFacetOptions::Create ();
    m_cutBuilder  = IPolyfaceConstruction::Create (*m_options);
    m_fillBuilder = IPolyfaceConstruction::Create (*m_options);
    }

void ProcessCutFillFacet (
bvector<DPoint3d> &dtm,         
size_t dtmReadIndex,
bvector<DPoint3d> &road,
size_t roadReadIndex,
bvector<bool> &roadBoundaryFlag,//! [in] true if the succeeding edge is a boundary of the cutFill component.
bool isCut
) override 
    {
    if (isCut)
        {
        // dtm on top.
        AddFacet (m_cutBuilder, dtm);
        AddReversedFacet (m_cutBuilder, road);
        AddBoundaryQuads (m_cutBuilder, road, dtm, roadBoundaryFlag);
        }
    else
        {
        // road on top.
        AddFacet (m_fillBuilder, road);
        AddReversedFacet (m_fillBuilder, dtm);
        AddBoundaryQuads (m_fillBuilder, dtm, road, roadBoundaryFlag);
        }

    }
};

void PolyfaceQuery::ComputeSingleSheetCutFillMeshes
(
PolyfaceQueryCR dtm,
PolyfaceQueryCR road,
PolyfaceHeaderPtr &cutMesh,
PolyfaceHeaderPtr &fillMesh,
MeshAnnotationVector &messages,     //!< [out] messages about errors.
IPolyfaceVisitorFilter *dtmFilter,  //!< [in] optional filter for dtm
IPolyfaceVisitorFilter *roadFilter  //!< [in] optional filter for road  //!< [in,out] message logging structure.
)
    {
    messages.clear ();
    CutFillMeshBuilder builder;
    ComputeSingleSheetCutFill (dtm, road, builder, messages, dtmFilter, roadFilter);
    cutMesh = builder.GetCut ();
    fillMesh = builder.GetFill ();
    }




struct CutFillVolumeAccumulator : FacetCutFillHandler
{
private:
UsageSums m_cutVolume, m_fillVolume;

public:
UsageSums GetCutData ()   { return m_cutVolume;}
UsageSums GetFillData ()  { return m_fillVolume;}

void Reset () override
    {
    m_cutVolume.ClearSums ();
    m_fillVolume.ClearSums ();
    }
CutFillVolumeAccumulator ()
    {
    Reset ();
    }

void ProcessCutFillFacet (
bvector<DPoint3d> &dtm,         
size_t dtmReadIndex,
bvector<DPoint3d> &road,
size_t roadReadIndex,
bvector<bool> &roadBoundaryFlag,//! [in] true if the succeeding edge is a boundary of the cutFill component.
bool isCut
) override 
    {
    if (dtm.size () > 2)
        {
        double volume = ZVolumeBetweenFacets (dtm, road);
        if (isCut)
            {
            m_cutVolume.Accumulate(volume);
            }
        else
            {
            m_fillVolume.Accumulate(volume);
            }
        }
    }

};


void PolyfaceQuery::ComputeSingleSheetCutFillVolumes
(
PolyfaceQueryCR dtm,
PolyfaceQueryCR road,
double &cutVolume,
double &fillVolume,
MeshAnnotationVector &messages,     //!< [out] messages about errors.
IPolyfaceVisitorFilter *dtmFilter,  //!< [in] optional filter for dtm
IPolyfaceVisitorFilter *roadFilter  //!< [in] optional filter for road  //!< [in,out] message logging structure.
)
    {
    messages.clear ();
    CutFillVolumeAccumulator accumulator;
    ComputeSingleSheetCutFill (dtm, road, accumulator, messages, dtmFilter, roadFilter);
    cutVolume = accumulator.GetCutData ().Sum ();
    fillVolume = accumulator.GetFillData ().Sum ();
    }

END_BENTLEY_GEOMETRY_NAMESPACE
