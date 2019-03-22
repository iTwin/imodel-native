/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceIntersection.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/BinaryRangeHeap.h>
#define DEBUG_ONLY(_code_)
#define DEBUG_ONLY_ (_code_)  _code_
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


typedef struct PolyfaceIndexedHeapRangeTree &PolyfaceIndexedHeapRangeTreeR;
typedef struct PolyfaceIndexedHeapRangeTree const &PolyfaceIndexedHeapRangeTreeCR;


struct FaceData
{
DPlane3d plane;
size_t   nextCoplanarFaceIndex;
MTGNodeId node;
};


struct PolyfaceIntersectionPairTester : public IndexedRangeHeap::PairProcessor
{
PolyfaceVisitorPtr m_visitorA;
PolyfaceVisitorPtr m_visitorB;
bvector<DSegment3dSizeSize> &m_segments;

bool   m_looking;
size_t m_found;
PolyfaceIndexedHeapRangeTreeR m_treeA;
PolyfaceIndexedHeapRangeTreeR m_treeB;
double m_absTol;
size_t m_numReduce;
BoolCounter m_numNeedProcessing;
size_t m_numProcess;

PolyfaceIntersectionPairTester (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceQueryR polyfaceB,           //!< second polyface
PolyfaceIndexedHeapRangeTreeR treeA,
PolyfaceIndexedHeapRangeTreeR treeB,
bvector<DSegment3dSizeSize> &segments,
double absTol
)
    : 
    m_treeA (treeA),
    m_treeB (treeB),
    m_segments (segments),
    m_numReduce (0),
    m_numProcess (0),
    m_absTol (absTol)
    {
    m_visitorA = PolyfaceVisitor::Attach (polyfaceA);
    m_visitorB = PolyfaceVisitor::Attach (polyfaceB);


    m_looking = true;
    m_found = false;
    }

    bool NeedProcessing (
            DRange3dCR rangeA, size_t iA0, size_t iA1,
            DRange3dCR rangeB, size_t iB0, size_t iB1
            ) override
            {
            return m_numNeedProcessing.Count (
                //rangeA.IntersectsWith (rangeB, m_minDistanceFound, 3));
                rangeA.IntersectsWith (rangeB));
            } 

    // EDL -- this was virtual.  Does it need to be?
    void AppendAllEdges (bvector<DPoint3d> &points, DSegment3dOnFacets::HistoryBit bit, size_t readIndexA, size_t readIndexB) 
        {
        // ASSUME -- last point not doubled !!!
        DPoint3d xyzA = points.back ();
        for (DPoint3d xyzB : points)
            {
            m_segments.push_back (DSegment3dSizeSize (
            DSegment3dOnFacets (xyzA, xyzB, bit), readIndexA, readIndexB));
            xyzA = xyzB;
            }
        }

    void Process (size_t iA, size_t iB) override 
            {
            size_t readIndexA, readIndexB;
            m_numProcess++;
            if (!m_treeA.TryGetReadIndex (iA, readIndexA))
                return;
            if (!m_treeB.TryGetReadIndex (iB, readIndexB))
                return;
            if (!m_visitorA->MoveToFacetByReadIndex (readIndexA))
                return;
            if (!m_visitorB->MoveToFacetByReadIndex (readIndexB))
                return;
#define MAX_INTERSECTION_SEGMENT 100
            DSegment3d intersectionSegments[MAX_INTERSECTION_SEGMENT];
            int numSegment;
            bool parallel;
            DVec3d normalA, normalB;
            bvector<DPoint3d> &pointA = m_visitorA->Point ();
            bvector<DPoint3d> &pointB = m_visitorB->Point ();

            bsiPolygon_transverseIntersection (intersectionSegments, &numSegment, &parallel,
                        &normalA, &normalB,
                        MAX_INTERSECTION_SEGMENT,
                        pointA.data (), (int)pointA.size (),
                        pointB.data (), (int)pointB.size ()
                        );
            if (!parallel)
                {
                for (int i = 0; i < numSegment; i++)
                    m_segments.push_back (DSegment3dSizeSize (
                          DSegment3dOnFacets (intersectionSegments[i], DSegment3dOnFacets::HistoryBit::Transverse),
                              readIndexA, readIndexB));
                }
            else
                {
                double distanceBetweenPlanes = pointB.front ().DotDifference (pointA.front (), normalB);
                if (fabs (distanceBetweenPlanes) < m_absTol)
                    {
                    AppendAllEdges (m_visitorA->Point(), DSegment3dOnFacets::HistoryBit::EdgeOfA, readIndexA, readIndexB);
                    AppendAllEdges (m_visitorB->Point(), DSegment3dOnFacets::HistoryBit::EdgeOfB, readIndexA, readIndexB);
                    }
                }
            }
            
    bool IsLive () const override
        {
        return m_looking;
        }

};
// Search for closest approach between two meshes.
//
bool PolyfaceQuery::SearchIntersectionSegments (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceQueryR polyfaceB,           //!< second polyface
struct PolyfaceIndexedHeapRangeTree &treeA,    //! optional range tree for polyfaceA
struct PolyfaceIndexedHeapRangeTree &treeB,     //! optional range tree for polyfaceB
bvector<DSegment3dSizeSize> &segments
)
    {
    DRange3d rangeA, rangeB;
    treeA.GetHeapR ().Get(0, rangeA);
    treeB.GetHeapR ().Get(0, rangeB);
    rangeB.Extend (rangeA);
    double largestCoordinate = rangeB.LargestCoordinate ();
    double absTol = Angle::SmallAngle () * largestCoordinate;
    PolyfaceIntersectionPairTester tester (polyfaceA, polyfaceB,treeA, treeB, segments, absTol);
    IndexedRangeHeap::Search (treeA.GetHeapR (), treeB.GetHeapR (), tester, 1);
    return segments.size () > 0;
    }

// Search for closest approach between two meshes.
//
GEOMDLLIMPEXP bool PolyfaceQuery::SearchIntersectionSegments (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceQueryR polyfaceB,           //!< second polyface
bvector<DSegment3dSizeSize> &segments
)
    {
    PolyfaceIndexedHeapRangeTreePtr treeA = PolyfaceIndexedHeapRangeTree::CreateForPolyface (polyfaceA, true, true, true);
    PolyfaceIndexedHeapRangeTreePtr treeB = PolyfaceIndexedHeapRangeTree::CreateForPolyface (polyfaceB, true, true, true);
    return SearchIntersectionSegments (polyfaceA, polyfaceB, *treeA.get (), *treeB.get (), segments);
    }

struct FacetSplitProcessor
{
IPolyfaceConstructionR m_builder;
bvector<DSegment3dSizeSize> &m_segments;
bvector<DPoint3d> m_points;
PolyfaceVisitorPtr m_visitor;
bvector<DSegment3dSizeSize> m_currentSegments;
bool m_byTagA;
double m_vuTolerance;

VuSetP m_graph;
int m_maxPerFace;
// bind to builder, mesh and segment data.  Must invoke sorter before processing.
~FacetSplitProcessor ()
    {
    vu_freeVuSet (m_graph);
    }

// bind to builder, mesh and segment data.  Must invoke sorter before processing.
FacetSplitProcessor
(
IPolyfaceConstructionR builder,
PolyfaceQueryR sourceMesh,
bvector<DSegment3dSizeSize> &segments
)
    : m_segments (segments),
      m_builder (builder)
    {
    m_visitor = PolyfaceVisitor::Attach (sourceMesh);
    m_graph = vu_newVuSet (0);
    m_vuTolerance = 1.0e-8; // should come from caller, considering multiple meshes in play !!!
    m_maxPerFace = m_builder.GetFacetOptionsR ().GetMaxPerFace ();
    }

size_t GetTag (size_t index)
    {
    if (index < m_segments.size ())
        return m_segments[index].GetTag (m_byTagA);
    return SIZE_MAX;
    }


// ASSUME ... builder is just points and point indices ...
// All edges visible -- maybe interior triangulations don't need that?
void AddPolygon (bvector<DPoint3d> &points, TransformCP localToWorld)
    {
    if (points.size () >= 3)
        {
        for (DPoint3d xyz : points)
            {
            if (nullptr != localToWorld)
                localToWorld->Multiply (xyz);
            size_t index = m_builder.FindOrAddPoint (xyz);
            m_builder.AddPointIndex (index, true);
            }
        m_builder.AddPointIndexTerminator ();
        }
    }

// Segments on the visitor's current facet are collected in m_segments.
// combine the facet boundary with the segments in a vu graph.
// copy
void ProcessCurrentSegments ()
    {
    // ASSUME -- visitor is on the right facet.
    // COPY to local points ....
    m_points = m_visitor->Point ();
    if (m_segments.size () > 0)
        {
        Transform localToWorld, worldToLocal;
        if (PolygonOps::CoordinateFrame (&m_points, localToWorld, worldToLocal))
            {
            worldToLocal.Multiply (m_points, m_points);
            vu_reinitializeVuSet (m_graph);
            VuOps::MakeIndexedLoopFromPartialArray (m_graph, &m_points, 0, m_points.size () - 1,
                    VU_BOUNDARY_EDGE, VU_BOUNDARY_EDGE, m_vuTolerance);
            for (DSegment3dSizeSize &segmentData : m_currentSegments)
                {
                VuP nodeA, nodeB;
                DSegment3d segment = segmentData.Get ();
                worldToLocal.Multiply (segment);
                vu_makePair (m_graph, &nodeA, &nodeB);
                vu_setDPoint3d (nodeA, &segment.point[0]);
                vu_setDPoint3d (nodeB, &segment.point[1]);
                }
            vu_mergeOrUnionLoops (m_graph, VUUNION_UNION);
            vu_regularizeGraph (m_graph);
            vu_markAlternatingExteriorBoundaries(m_graph,true);
            vu_splitMonotoneFacesToEdgeLimit (m_graph, m_maxPerFace);
            
            VuMask visitMask = vu_grabMask (m_graph);
            vu_clearMaskInSet (m_graph, visitMask);
            VU_SET_LOOP (faceSeed, m_graph)
                {
                if (!vu_getMask (faceSeed, visitMask))
                    {
                    vu_setMaskAroundFace (faceSeed, visitMask);
                    if (!vu_getMask (faceSeed, VU_EXTERIOR_EDGE))
                        {
                        m_points.clear ();
                        VU_FACE_LOOP (node, faceSeed)
                            {
                            DPoint3d xyz;
                            vu_getDPoint3d (&xyz, node);
                            m_points.push_back (xyz);
                            }
                        END_VU_FACE_LOOP (node, faceSeed)
                        AddPolygon (m_points, &localToWorld);
                        }
                    }
                }
            END_VU_SET_LOOP (faceSeed, m_graph)
            vu_returnMask (m_graph, visitMask);
            return;
            }
        }
    // fall through if no splitting happened ...
    AddPolygon (m_visitor->Point (), nullptr);
    }


// Sort the intersection segments to cluster by tagA or tagB.
// loop over all facets.
//  Apply incident segments to each facet.
void ProcessByTag (bool byTagA)
    {
    m_byTagA = byTagA;
    DSegment3dSizeSize::SortByTags (m_segments, byTagA);
    size_t segmentIndex = 0;
    for (m_visitor->Reset (); m_visitor->AdvanceToNextFace ();)
        {
        size_t meshReadIndex = m_visitor->GetReadIndex ();
        // skip over splits below this read index.  (These probably should not happen)
        while (GetTag (segmentIndex) < meshReadIndex)
            segmentIndex++;
        m_currentSegments.clear ();
        for (;GetTag (segmentIndex) == meshReadIndex; segmentIndex++)
            m_currentSegments.push_back (m_segments[segmentIndex]);
        ProcessCurrentSegments ();
        }
    }
};


bool PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (IPolyfaceConstructionR builder,
      PolyfaceQueryR source,
      bvector<DSegment3dSizeSize> &segments,
      bool byTagA
      )
    {
    FacetSplitProcessor processor (builder, source, segments);
    processor.ProcessByTag (byTagA);
    return false;
    }

static void StitchAndCollectVolumesFromImprintedMesh
(
PolyfaceQueryCR mesh,
IFacetOptionsR options,
bvector<PolyfaceHeaderPtr> &allVolumes
)
    {
    allVolumes.clear ();
    DRange3d range = mesh.PointRange ();

    if (range.IsNull ())
        return;
    static double s_absTol = 1.0e-8;
    static double s_relTol = 1.0e-11;
    MTGFacets *mtgFacets = jmdlMTGFacets_new ();
    PolyfaceToMTG (mtgFacets,
          nullptr, nullptr,
          mesh,
          false, s_absTol, s_relTol, 1
          );




    bvector<bvector<MTGNodeId>> componentFaceNodes;
    mtgFacets->GetGraphP()->CollectConnectedComponents (componentFaceNodes, MTG_ScopeFace);
    size_t numShell = componentFaceNodes.size ();
    DEBUG_ONLY(
    static double volumeScale = 1.0e12;
    DPoint3d origin;
    origin.Zero ();)
    for (size_t i = 0; i < numShell; i++)
            {
            DEBUG_ONLY(
            double volume0 = mtgFacets->SumTetrahedralVolumeFromNodesPerFace (origin, componentFaceNodes[i])
                  / volumeScale;)
            IPolyfaceConstructionPtr finalBuilder = IPolyfaceConstruction::Create (options);
            mtgFacets->AddFacesToPolyface (*finalBuilder, componentFaceNodes [i], false);
            DEBUG_ONLY (double volume1 = finalBuilder->GetClientMeshPtr ()->SumTetrahedralVolumes (origin) / volumeScale;)
            DEBUG_ONLY (GEOMAPI_PRINTF (" Component volume %g %g\n", volume0, volume1);)
            allVolumes.push_back (finalBuilder->GetClientMeshPtr ());
            }

    }


static void PushBack (bvector<PolyfaceHeaderPtr> *collection, PolyfaceHeaderPtr & value)
    {
    if (nullptr != collection)
        collection->push_back (value);
    }

void PolyfaceQuery::SelectMeshesByVolumeSign
(
bvector<PolyfaceHeaderPtr> &inputMeshes,
bvector<PolyfaceHeaderPtr> *negativeVolumeMeshes,
bvector<PolyfaceHeaderPtr> *zeroVolumeMeshes,
bvector<PolyfaceHeaderPtr> *positiveVolumeMeshes
)
    {
    DRange3d totalRange;
    totalRange.Init ();
    for (size_t i = 0; i < inputMeshes.size (); i++)
        totalRange.Extend (inputMeshes[i]->PointRange ());

    if (nullptr != negativeVolumeMeshes)
        negativeVolumeMeshes->clear ();
    if (nullptr != zeroVolumeMeshes)
        zeroVolumeMeshes->clear ();
    if (nullptr != positiveVolumeMeshes)
        positiveVolumeMeshes->clear ();

    DPoint3d origin = totalRange.low;
    double diagonal = totalRange.low.Distance (totalRange.high);
    double referenceVolume = diagonal * diagonal * diagonal;
    static double zeroVolumeRelTol = 1.0e-8;
    double absVolumeTolerance = zeroVolumeRelTol * referenceVolume;
    size_t numShell = inputMeshes.size ();
    for (size_t i = 0; i < numShell; i++)
        {
        double a = inputMeshes[i]->SumTetrahedralVolumes (origin);
        if (a > absVolumeTolerance)
            PushBack (positiveVolumeMeshes, inputMeshes[i]);
        else if (a < -absVolumeTolerance)
            PushBack (negativeVolumeMeshes, inputMeshes[i]);
        else
            PushBack (zeroVolumeMeshes, inputMeshes[i]);
        }
    }

void PolyfaceQuery::MergeAndCollectVolumes
(
PolyfaceQueryR meshA,
PolyfaceQueryR meshB,
bvector<PolyfaceHeaderPtr> &allVolumes
)
    {
    allVolumes.clear ();
    bvector<DSegment3dSizeSize> segments;
    PolyfaceQuery::SearchIntersectionSegments (meshA, meshB, segments);

    IFacetOptionsPtr options = IFacetOptions::Create ();
    static int s_maxPerFace = 4;
    options->SetMaxPerFace (s_maxPerFace);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);

    PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builder, meshA, segments, true);
    PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builder, meshB, segments, false);

    StitchAndCollectVolumesFromImprintedMesh (builder->GetClientMeshR (), *options, allVolumes);
    }


void PolyfaceQuery::MergeAndCollectVolumes
(
bvector<PolyfaceHeaderPtr> &inputMesh,
bvector<PolyfaceHeaderPtr> &allVolumes
)
    {
    allVolumes.clear ();
    // segments[i][j] of intersection segments between inputMesh[i] and inputMesh[j]
    // (But only the lower triangle is computed)
    // !! Assume no clash within any mesh -- but change that someday !!!!
    bvector<bvector<bvector<DSegment3dSizeSize>>> segments;
    size_t numMesh = inputMesh.size ();
    for (size_t i = 0; i < numMesh; i++)
        {
        segments.push_back (bvector<bvector<DSegment3dSizeSize>> ());
        for (size_t j = 0; j <= i; j++)
            {
            segments[i].push_back (bvector<DSegment3dSizeSize> ());
            if (j < i)
                {
                // TODO -- save the range trees for reuse
                PolyfaceQuery::SearchIntersectionSegments (*inputMesh[i], *inputMesh[j], segments[i][j]);
                }
            else
                {
                // self intersections go here ..
                }
            }
        }

    IFacetOptionsPtr options = IFacetOptions::Create ();
    static int s_maxPerFace = 4;
    options->SetMaxPerFace (s_maxPerFace);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);

    bvector<DSegment3dSizeSize> collectedSegments;
    for (size_t i = 0; i < numMesh; i++)
        {
        // Consolidate segments from this mesh's intersections . . .
        collectedSegments.clear ();
        // Walk ACROSS the row to the diagonal -- ReadIndices for this mesh are the TagA value 
        for (size_t j = 0; j < i; j++)
            {
            for (DSegment3dSizeSize &segment : segments[i][j])
                {
                collectedSegments.push_back (DSegment3dSizeSize (segment.Get (), segment.GetTagA (), segment.GetTagA ()));
                }
            }
          // Walk DOWN the colum below the diagonal -- ReadIndices for this mesh are the TagB value 
        for (size_t k = i + 1; k < numMesh; k++)
            {
            for (DSegment3dSizeSize &segment : segments[k][i])
                {
                collectedSegments.push_back (DSegment3dSizeSize (segment.Get (), segment.GetTagB (), segment.GetTagB ()));
                }
            }
        PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builder, *inputMesh[i], collectedSegments, true);
        }


    StitchAndCollectVolumesFromImprintedMesh (builder->GetClientMeshR (), *options, allVolumes);
    }



END_BENTLEY_GEOMETRY_NAMESPACE
