/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
#include <Mtg/capi/mtgprint_capi.h>
#include <Mtg/MTGShortestPaths.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// !!! assumed setup: normals are "per face" -- comparing normal with mate defines
bool MarkPlanarBoundaries (MTGFacets &facets, MTGMask boundaryMask, MTGMask ignoreMask)
    {
    MTGGraph *graph = facets.GetGraphP ();
    graph->ClearMask (boundaryMask);
    MTGGraph::ScopedMask visitMaskA (*graph);
    MTGMask visitMask = visitMaskA.Get ();
    size_t errors = 0;
    size_t numSmooth = 0;
    size_t numAngle = 0;
    MTGARRAY_SET_LOOP (nodeIdA, graph)
        {
        if (!graph->HasMaskAt (nodeIdA, visitMask))
            {
            graph->SetMaskAroundEdge (nodeIdA, visitMask);
            MTGNodeId nodeIdB = graph->EdgeMate (nodeIdA);
            bool ignoreA = graph->HasMaskAt (nodeIdA, ignoreMask);
            bool ignoreB = graph->HasMaskAt (nodeIdB, ignoreMask);
            if (!ignoreA && !ignoreB)
                {
                DVec3d normalA, normalB;
                if (!facets.NodeToNormalCoordinates (nodeIdA, normalA)
                    || !facets.NodeToNormalCoordinates (nodeIdB, normalB)
                    )
                    {
                    errors++;
                    }
                else if (normalA.IsParallelTo (normalB))
                    {
                    numSmooth++;
                    }
                else
                    {
                    numAngle++;
                    if (!ignoreA)
                        graph->SetMaskAt (nodeIdA, boundaryMask);
                    if (!ignoreB)
                        graph->SetMaskAt (nodeIdB, boundaryMask);
                    }
                }
            else
                {
                if (!ignoreA)
                    graph->SetMaskAt (nodeIdA, boundaryMask);
                if (!ignoreB)
                    graph->SetMaskAt (nodeIdB, boundaryMask);
                }
            }
        }
    MTGARRAY_END_SET_LOOP (nodeIdA, graph)
    return errors == 0;
    }
// Callback functions to accesss coordinates in MTGFacets and edge length as search condition
struct MyMTGFacetsXYZDistanceSearchFunctions : MTGShortestPathContext::MTGFacetsSearchFunctions
    {
    private: MyMTGFacetsXYZDistanceSearchFunctions () {}
    public:
    MyMTGFacetsXYZDistanceSearchFunctions (MTGFacetsP facets)
        {
#ifdef NoisyPlanarMerge
        jmdlMTGFacets_printFaceLoops (facets);
#endif
        SetFacets (facets);
        }

    double EdgeLength (MTGNodeId node) override
        {
        MTGNodeId mate = m_graph->EdgeMate (node);
#ifdef NoisyPlanarMerge
        DPoint3d xyz0 = m_facets->GetXYZ (node);
        DPoint3d xyz1 = m_facets->GetXYZ (mate);
        printf("   EdgeLength (%d %g %g) (%d %g %g) (distance %g)\n",
                    node, xyz0.x, xyz0.y, mate, xyz1.x, xyz1.y, xyz0.Distance (xyz1));
#endif
        return m_facets->GetXYZ (node).Distance (m_facets->GetXYZ (mate));
        }
    void Announce (char const *message,  int data0, int data1) override
        {
#ifdef NoisyPlanarMerge
        printf (" %s (%d %d))\n", message, data0, data1);
#endif
        }
    };

void BuildBridgeEdges (MTGFacets &facets, MTGMask boundaryMask)
    {
    MTGGraphP graph = facets.GetGraphP ();
    
#ifdef NoisyPlanarMerge
    jmdlMTGFacets_printFaceLoops (&facets);
#endif
    MTGShortestPathContext searcher (graph);
    MyMTGFacetsXYZDistanceSearchFunctions facetDistanceFunctions (&facets);
    searcher.MarkShortestPathBetweenLoops (boundaryMask, boundaryMask, &facetDistanceFunctions);
    }
// Find nodeIdA, nodeIdB pairs that are successors in the extended face sense, and whose mates are either
//    a) exterior edges that are successors
//    b) another pair of successors in the extended sense (C = B.mate, D = A.mate)
// AND the vectors along A and B are parallel.
// Set the colinear vertex mask at the outgoing nodeId (B, D)
void MarkColinearEdges (MTGFacets &facets, MTGMask boundaryMask, MTGMask exteriorMask, MTGMask colinearVertexMask)
    {
    MTGGraph &graph = *facets.GetGraphP ();
    graph.ClearMask (colinearVertexMask);

    MTGGraph::ScopedMask visitMaskA (graph);
    MTGMask visitMask = visitMaskA.Get ();
    size_t numSmooth = 0;
    size_t numTested = 0;
    MTGARRAY_SET_LOOP (nodeIdA, &graph)
        {
        if (!graph.HasMaskAt (nodeIdA, visitMask)
            && graph.HasMaskAt (nodeIdA, boundaryMask)
            )
            {
            MTGNodeId nodeIdB = graph.FindMaskAroundVertexPred (graph.FSucc (nodeIdA), boundaryMask);
            if (graph.IsValidNodeId (nodeIdB))
                {
                MTGNodeId nodeIdC = graph.EdgeMate (nodeIdB);
                MTGNodeId nodeIdD = MTGGraph::NullNodeId;
                bool isBoundary = graph.HasMaskAt (nodeIdC, exteriorMask);
                if (isBoundary)
                    {
                    // the A.B sequential edges are against a true boundary ...
                    nodeIdD = graph.FSucc (nodeIdC);    // for exterior, only look at simple successor
                    }
                else if (graph.HasMaskAt (nodeIdC, boundaryMask))
                    {
                    // the A.B sequential edges have neighbors that are also interior ...
                    nodeIdD = graph.FindMaskAroundVertexPred (graph.FSucc (nodeIdC), boundaryMask);
                    }
                else
                    nodeIdD = MTGGraph::NullNodeId;

                if (graph.IsValidNodeId (nodeIdD) && nodeIdA == graph.EdgeMate (nodeIdD))
                    {
                    DPoint3d xyzA, xyzB, xyzB1;
                    if (   facets.NodeToVertexCoordinates (nodeIdA, xyzA)
                        && facets.NodeToVertexCoordinates (nodeIdB, xyzB)
                        && facets.NodeToVertexCoordinates (graph.FSucc (nodeIdB), xyzB1)
                        )
                        {
                        numTested++;
                        auto edgeVectorA = (xyzB - xyzA).ValidatedNormalize ();
                        auto edgeVectorB = (xyzB1 - xyzB).ValidatedNormalize ();
                        if (edgeVectorA.IsValid () && edgeVectorB.IsValid ())
                            {
                            if (edgeVectorA.Value ().IsParallelTo (edgeVectorB.Value ()))
                                {
                                graph.SetMaskAt (nodeIdB, colinearVertexMask);
                                if (!isBoundary)
                                    graph.SetMaskAt (nodeIdD, colinearVertexMask);
                                if (!isBoundary)
                                    graph.SetMaskAt (nodeIdC, visitMask);      // remark "visit mask" only needs to be set mate side
                                numSmooth++;
                                }
                            }
                        }
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP (nodeIdA, *graph)
    }




size_t CountMaskParityErrorAroundVertexLoops (MTGGraph & graph, MTGMask targetMask)
    {
    MTGGraph::ScopedMask visitMask (graph);
    size_t errors = 0;
    MTGARRAY_SET_LOOP (seedNodeId, &graph)
        {
        if (!visitMask.IsSetAt (seedNodeId))
            {
            visitMask.SetAroundVertex (seedNodeId);
            MTGNodeId nodeIdA = graph.FindMaskAroundVertex (seedNodeId, targetMask);
            if (graph.IsValidNodeId (nodeIdA))
                {
                int state = 0;
                // We want to know the partity state 
                // "on each edge" and "in each sector".
                // Call it 0 "on" the edge of nodeIdA.
                // Increase when outbound edge has targetMask
                // Decrease when inbound edge has targetMask.
                // The only valid values of the state are 0 and 1.
                MTGARRAY_VERTEX_LOOP (nodeIdB, &graph, nodeIdA)
                    {
                    if (graph.HasMaskAt (nodeIdA, targetMask))
                        state++;
                    if (state < 0 || state > 1)
                        errors++;
                    nodeIdB = graph.VPred (nodeIdA);
                    if (graph.HasMaskAt (nodeIdB, targetMask))
                        state--;
                    if (state < 0 || state > 1)
                        errors++;
                    }
                MTGARRAY_END_VERTEX_LOOP (nodeIdB, &graph, nodeIdA)
                }
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, &graph)
    return errors;
    }
//! Returns ERROR count !!!
size_t CollectExtendedFaceLoops (MTGGraph &graph, bvector<bvector<MTGNodeId>> &nodes, MTGMask extendedFaceMask, MTGMask skipMask)
    {
    MTGGraph::ScopedMask visitMask (graph);
    size_t errors = 0;
    MTGARRAY_SET_LOOP (nodeIdA, &graph)
        {
        if (!visitMask.IsSetAt (nodeIdA))
            {
            if (graph.HasMaskAt (nodeIdA, extendedFaceMask))
                {
                nodes.push_back (bvector <MTGNodeId> ());
                for (MTGNodeId nodeIdB = nodeIdA;;)
                    {
                    if (visitMask.IsSetAt (nodeIdB))
                        {
                        errors++;
                        break;
                        }
                    visitMask.SetAt (nodeIdB);
                    if (!graph.HasMaskAt (nodeIdB, skipMask))
                        nodes.back ().push_back (nodeIdB);
                    nodeIdB = graph.FindMaskAroundVertexPred (graph.FSucc (nodeIdB), extendedFaceMask);
                    if (!graph.IsValidNodeId (nodeIdB))
                        {
                        errors++;
                        break;
                        }
                    if (nodeIdB == nodeIdA)
                        break;
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP (nodeIdA, &graph)
    return errors;
    }
//! Return the number of failed label access while extracting by nodeid.
//! The returned (default) label is saved at error spots.
size_t CollectLabels (MTGGraph const &graph, int labelIndex, bvector<bvector<MTGNodeId>> const &loopNodes, bvector<bvector<int>> &labels)
    {
    labels.clear ();
    size_t numErrors = 0;
    for (auto &loop : loopNodes)
        {
        labels.push_back (bvector<int> ());
        for (auto nodeId : loop)
            {
            int value;
            if (!graph.TryGetLabel (nodeId, labelIndex, value))
                numErrors++;
            labels.back ().push_back (value);
            }
        }
    return numErrors;
    }

// Copy selected contents of from source to dest, with 0 terminators between blocks
void CopyReadIndexLoops
(
BlockedVector<int> const &source,
bvector<bvector<int>> const &sourceIndices,
BlockedVector<int> &dest
)
    {
    dest.SetActive (source.Active ());
    size_t numSource = source.size ();
    if (dest.Active ())
        {
        for (auto &loop : sourceIndices)
            {
            for (auto intIndex : loop)
                {
                if (intIndex >= 0 && intIndex < (int) numSource)
                    dest.push_back (source[(size_t) intIndex]);
                }
                dest.push_back (0);
            }
        }
    }
PolyfaceHeaderPtr CreateMeshFromReadIndexLoops (PolyfaceHeaderR sourceMesh, bvector<bvector<int>> const &readIndexIntLoops)
    {
    PolyfaceHeaderPtr result = PolyfaceHeader::CreateVariableSizeIndexed ();
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (sourceMesh);
#define CopyAndCompress_not
#ifdef CopyAndCompress
    result->Point () = sourceMesh.Point ();
    result->Param () = sourceMesh.Param ();
    result->Normal () = sourceMesh.Normal ();
    result->IntColor () = sourceMesh.IntColor ();
#ifdef ColorFlavors
    result.TableColor () = source.TableColor ();
    result->DoubleColor () = source.DoubleColor ();
#endif
    CopyReadIndexLoops (sourceMesh.PointIndex (),  readIndexIntLoops,  result->PointIndex ());
    CopyReadIndexLoops (sourceMesh.ParamIndex (),  readIndexIntLoops,  result->ParamIndex ());
    CopyReadIndexLoops (sourceMesh.NormalIndex (), readIndexIntLoops,  result->NormalIndex ());
    CopyReadIndexLoops (sourceMesh.ColorIndex (),  readIndexIntLoops,  result->ColorIndex ());
    result->Compress ();
#else
    // Copy the read index loops, then use the copies to pull selective entries from the data arrays
    // this does not provide coordinate consolidation.
    CopyReadIndexLoops (sourceMesh.PointIndex (),  readIndexIntLoops,  result->PointIndex ());
    CopyReadIndexLoops (sourceMesh.ParamIndex (),  readIndexIntLoops,  result->ParamIndex ());
    CopyReadIndexLoops (sourceMesh.NormalIndex (), readIndexIntLoops,  result->NormalIndex ());
    CopyReadIndexLoops (sourceMesh.ColorIndex (),  readIndexIntLoops,  result->ColorIndex ());
    bvector<size_t> oldToNewData;
    size_t numRangeError;
    PolyfaceVectors::ReindexOneBasedData (sourceMesh.Point (), 1, result->PointIndex (), result->Point (), oldToNewData, numRangeError);
    PolyfaceVectors::ReindexOneBasedData (sourceMesh.Param (), 1, result->ParamIndex (), result->Param (), oldToNewData, numRangeError);
    PolyfaceVectors::ReindexOneBasedData (sourceMesh.Normal (), 1, result->NormalIndex (), result->Normal (), oldToNewData, numRangeError);
    if (sourceMesh.IntColor ().Active ())
        PolyfaceVectors::ReindexOneBasedData (sourceMesh.IntColor (), 1, result->ColorIndex (), result->IntColor (), oldToNewData, numRangeError);
#ifdef ColorFlavors
    else if (sourceMesh.DoubleColor ().Active ())
        PolyfaceVectors::ReindexOneBasedData (sourceMesh.DoubleColor (), 1, result->ColorIndex (), oldToNewData, numRangeError);
    else if (sourceMesh.ColorTable ().Active ())
        PolyfaceVectors::ReindexOneBasedData (sourceMesh.ColorTable (), 1, result->ColorIndex (), oldToNewData, numRangeError);
#endif

#endif
    return result;
    }
//!-----------------------------------------------------------------
//! From MtgApi -- structure of the MTGFacets ....
//! Create MTGFacets from polyface.  Only the polyface connectivity is used -- no merging of vertex coordinates
//!   other than what is already indicated in the connectivity.
//! The graph is marked as follows:
//! 1) Any nonmanifold edge (other than two adjacent faces) is masked MTG_BOUNDARY_MASK.
//! 2) The outside of the nonmanifold edges is masked MTG_EXTERIOR_MASK.
//! 3) An outside edge with identical start and end vertex indices is also masked MTG_POLAR_LOOP_MASK
//! 4) Any edge marked visible in the polyface is masked MTG_PRIMARY_EDGE_MASK
//! 5) A label with tag MTG_LABEL_TAG_POLYFACE_READINDEX is added (if necessary) with a back index to the read index
//!      that the node came from.   This label is -1 for exterior nodes.
//! 6) facets->vertexLabelOffset is the 0 based (!!) vertex index.
//!*----------------------------------------------------------------

PolyfaceHeaderPtr PolyfaceHeader::CloneWithMaximalPlanarFacets (bool mergeCoplanarFacets, bool mergeColinearEdges)
    {
    PolyfaceHeaderPtr newMesh = nullptr;
    MTGFacets *mtgFacets = jmdlMTGFacets_new ();
    PolyfaceToMTG_FromPolyfaceConnectivity (mtgFacets, *this);
    MTGGraph *graph = mtgFacets->GetGraphP ();
    //PrintGraph (graph, mtgFacets->vertexLabelOffset);
    int readIndexOffset;
    if (!mtgFacets->GetGraphP ()->TrySearchLabelTag (MTG_LABEL_TAG_POLYFACE_READINDEX, readIndexOffset))
        return newMesh;
    size_t errors = 0;
    MTGMask planarBoundaryMask = MTG_PRIMARY_EDGE_MASK;
    if (mtgFacets->BuildFacePlaneNormals ())
        {
        MTGGraph::ScopedMask colinearEdgeScopedMask (*graph);
        MTGMask vertexInColinearEdgeMask = colinearEdgeScopedMask.Get ();
        if (MarkPlanarBoundaries (*mtgFacets, planarBoundaryMask, MTG_EXTERIOR_MASK))
            {
            BuildBridgeEdges (*mtgFacets, planarBoundaryMask);
            if (mergeColinearEdges)
                MarkColinearEdges (*mtgFacets, planarBoundaryMask, MTG_EXTERIOR_MASK, vertexInColinearEdgeMask);
            if (CountMaskParityErrorAroundVertexLoops (*mtgFacets->GetGraphP (), planarBoundaryMask) == 0)
                {
                bvector<bvector<MTGNodeId>> loopNodes;
                errors += CollectExtendedFaceLoops (*mtgFacets->GetGraphP (), loopNodes, planarBoundaryMask, vertexInColinearEdgeMask);
                bvector<bvector<int>> readIndexLoops;
                errors += CollectLabels (*mtgFacets->GetGraphP (), readIndexOffset, loopNodes, readIndexLoops);
#ifdef PrintExtendedLoops
                Check::PrintHeading ("ExtendedFaceLoops", "nodes");
                for (auto &loop : loopNodes)
                    {
                    Check::PrintIndent (2);
                    for (auto nodeId : loop)
                        Check::Print (nodeId, "");
                    }
                Check::PrintHeading ("ExtendedFaceLoops", "readIndices");
                for (auto &loop : readIndexLoops)
                    {
                    Check::PrintIndent (2);
                    for (auto ri : loop)
                        Check::Print (ri, "");
                    }
#endif
                newMesh = CreateMeshFromReadIndexLoops (*this, readIndexLoops);
                }
            }
        }
    jmdlMTGFacets_free (mtgFacets);
    return newMesh;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
