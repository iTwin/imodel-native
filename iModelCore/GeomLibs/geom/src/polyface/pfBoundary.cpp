/*----------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/pfBoundary.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "pf_halfEdgeArray.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE



struct ExtractBoundaryContext
{
MTGFacetsP facets;
MTGGraphP  graph;
MTGMask barrierEdgeMask;
int     readIndexLabel;
PolyfaceHeaderR mesh;
CurveVectorPtr allBoundaries;
bvector<DPoint3d> points;

ExtractBoundaryContext (PolyfaceHeaderR mesh1)
    : mesh (mesh1)
    {
    facets = jmdlMTGFacets_new ();
    graph = jmdlMTGFacets_getGraph (facets);
    readIndexLabel = -1;
    allBoundaries = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    }
    
~ExtractBoundaryContext ()
    {
    jmdlMTGFacets_free (facets);    
    }

bool IsVisibleEdge (MTGNodeId node)
    {
    return (graph->GetMaskAt (node, MTG_PRIMARY_EDGE_MASK) || graph->GetMaskAt (graph->EdgeMate (node), MTG_PRIMARY_EDGE_MASK))
        && !graph->GetMaskAt (node, MTG_EXTERIOR_MASK);
    }
    
MTGNodeId GetVisibleSuccessor (MTGNodeId nodeA)
    {
    if (IsVisibleEdge (nodeA))
        {
        // Walk the vertex loop clockwise (vpred!!) expecting to find an outbound visible edge
        //  (but NOT an inbound visible edge!!!)
        MTGNodeId nodeB = graph->FSucc (nodeA);
        while (nodeB != nodeA)
            {
            if (IsVisibleEdge (nodeB))
                return nodeB;
            nodeB = graph->VPred (nodeB);                                
            }
        }
    return MTG_NULL_NODEID;
    }
 
void EmitAndMark (MTGNodeId node, MTGMask visitMask)
    {
    graph->SetMaskAt (node, visitMask);
    DPoint3d xyz;
    jmdlMTGFacets_getNodeCoordinates (facets, &xyz, node);
    points.push_back (xyz);
    } 
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr go (size_t &numOpen, size_t &numClosed)
    {
    numOpen = 0;
    numClosed = 0;
    if (!PolyfaceToMTG_FromPolyfaceConnectivity (facets, mesh))
        return NULL;
    if (!graph->TrySearchLabelTag (MTG_LABEL_TAG_POLYFACE_READINDEX, readIndexLabel))
        return NULL;
    static int s_noisy = 0;
    if (s_noisy)
        jmdlMTGGraph_printFaceLoops (graph);

    bvector<MTGNodeId> vertexLoops;
    MTGMask visitMask = graph->GrabMask ();
    graph->ClearMask (visitMask);
    MTGARRAY_SET_LOOP (seedNodeId, graph)
        {
        if (!graph->GetMaskAt (seedNodeId, visitMask) && IsVisibleEdge (seedNodeId))
            {
            // nodeA walks around the interior of a superface.
            // We expect each step (except the return to the seedNodeId) to be to an unvisited node.
            MTGNodeId nodeA = seedNodeId;
            points.clear ();
            EmitAndMark (nodeA, visitMask);
            MTGNodeId nodeB;
            for (;; nodeA = nodeB)
                {
                nodeB = GetVisibleSuccessor (nodeA);
                if (!graph->IsValidNodeId (nodeB))
                    {
                    EmitAndMark (graph->FSucc (nodeA), visitMask);
                    numOpen++;
                    break;
                    }
                bool alreadyVisited = graph->HasMaskAt (nodeB, visitMask);
                EmitAndMark (nodeB, visitMask);
                if (nodeB == seedNodeId)
                    {
                    numClosed++;
                    break;
                    }
                // test for loop that rejoins its own path other than at its seed .. call it open.  This is a badly marked boundary in the polyface?
                if (alreadyVisited)
                    {
                    numOpen++;
                    break;
                    }
                }
            ICurvePrimitivePtr boundary = ICurvePrimitive::CreateLineString (points);
            allBoundaries->push_back (boundary);
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, graph)
    graph->DropMask (visitMask);
    return allBoundaries;
    }
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  07/18
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP CurveVectorPtr PolyfaceHeader::ExtractBoundaryStrings (size_t &numOpen, size_t &numClosed)
    {
    ExtractBoundaryContext context (*this);
    return context.go (numOpen,  numClosed);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  07/18
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP void PolyfaceHeader::CollectEdgeMateData
(
bvector<FacetEdgeDetail> &segments,
bool includeMatched,
bool returnSingleEdgeReadIndex
)
    {
    segments.clear ();
    HalfEdgeArray halfEdges (this);
    halfEdges.BuildArray (*this, false, true, true);
    halfEdges.SortForEdgeMatching ();
    halfEdges.CollectEdgeMateData (*this, segments, includeMatched, returnSingleEdgeReadIndex);
    }
END_BENTLEY_GEOMETRY_NAMESPACE

