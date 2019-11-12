/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static size_t CountZeros(int32_t const *index, size_t n)
    {
    size_t numZero = 0;
    for (size_t i = 0; i < n; i++)
        {
        if (index[i] == 0)
            numZero++;
        }
    return numZero;
    }

bool   PolyfaceToMTG
(
MTGFacets*                      pFacets,
bvector<MTGNodeId>      *pNodeIdFromMeshVertex,
bvector<size_t>         *pPolyfaceArrayIndexFromNodeId,
PolyfaceQueryCR             polyface,
bool                        dropToSingleFace,
double                          absTol,
double                          relTol
)
    {
    return PolyfaceToMTG (pFacets, pNodeIdFromMeshVertex, pPolyfaceArrayIndexFromNodeId, polyface, dropToSingleFace, absTol, relTol, 0);
    }
/*-----------------------------------------------------------------*//**
* @description Create an MTGFacets set from polyface mesh arrays using the double face
*       stitcher.  Partner faces are found via the MTG_NODE_PARTNER_TAG label of each
*       MTG node.  Relevant edges are masked with MTG_BOUNDARY_MASK and/or
*       MTG_EXTERIOR_MASK.  Optionally return maps for MTGFacets to/from mesh.
* @param pFacets                        <= facet set
* @param pNodeIdFromMeshVertex          <= an MTG nodeId in the vertex loop at mesh vertex i (or NULL)
* @param pPolyfaceArrayIndexFromNodeId  <= index in pIndexArray to face loop entry corresponding to MTG nodeId i (or NULL)
* @param polyface => source mesh
* @param absTol                         => absolute tolerance for identical point test
* @param relTol                         => relative tolerance for identical point test
* @return true if MTG was successfully stitched
* @bsihdr                                       DavidAssaf      06/02
+---------------+---------------+---------------+---------------+------*/
bool   PolyfaceToMTG
(
MTGFacets*                      pFacets,
bvector<MTGNodeId>      *pNodeIdFromMeshVertex,
bvector<size_t>         *pPolyfaceArrayIndexFromNodeId,
PolyfaceQueryCR             polyface,
bool                        dropToSingleFace,
double                          absTol,
double                          relTol,
int                    stitchSelect
)
    {
    size_t invalidIndex = SIZE_MAX;
    size_t numPoint = polyface.GetPointCount ();
    DPoint3dCP polyfacePoints = polyface.GetPointCP ();
    double tolerance = DPoint3dOps::Tolerance (polyfacePoints, numPoint, absTol, relTol);

    size_t numIndex = polyface.GetPointIndexCount ();
    int32_t const *polyfaceIndex = polyface.GetPointIndexCP ();

    size_t numVertex = polyface.GetPointCount ();
    MTGGraphP pGraph = jmdlMTGFacets_getGraph (pFacets);
    jmdlMTGFacets_empty (pFacets);
    jmdlMTGFacets_setNormalMode (pFacets, MTG_Facets_SeparateNormals, (int)numVertex, (int)numVertex);
    int thruOffset = jmdlMTGFacets_defineLabel (pFacets, MTG_NODE_PARTNER_TAG, MTG_LabelMask_SectorProperty, -1);
    
    // Copy coordinates into facets ..
    DPoint3dOps::Copy (&pFacets->vertexArrayHdr, polyfacePoints, numPoint);

    size_t numPrimary = 0;
    size_t numEdge = 0;


    if (pNodeIdFromMeshVertex)
        {
        pNodeIdFromMeshVertex->clear ();
        for (size_t i = 0; i < numVertex; i++)
            pNodeIdFromMeshVertex->push_back (MTG_NULL_NODEID);
        }

    if (pPolyfaceArrayIndexFromNodeId)
        {
        size_t nZero  = CountZeros (polyfaceIndex, numIndex);
        size_t nNodeId = 2 * (numIndex - nZero);     // upper bound on InvMap size (some faces may be degenerate)
        pPolyfaceArrayIndexFromNodeId->clear ();    // um ... what is -1 in size_t?
        for (size_t i = 0; i < nNodeId; i++)
            pPolyfaceArrayIndexFromNodeId->push_back (invalidIndex);
        }

    PolyfaceVisitorPtr visitorPtr = PolyfaceVisitor::Attach (polyface, false);
    PolyfaceVisitor & visitor = *visitorPtr.get ();
    size_t numFacets = 0;
    for (visitor.Reset ();visitor.AdvanceToNextFace ();)
        {

        visitor.CompressClosePoints (tolerance);
        int numThisFace = (int)visitor.NumEdgesThisFace ();
        if (numThisFace < 3)
            continue;

        MTGNodeId baseNodeId = jmdlMTGFacets_addIndexedDoubleFace (pFacets, thruOffset, visitor.ClientPointIndex().GetCP (), NULL, numThisFace);
        if (MTG_NULL_NODEID == baseNodeId)
            continue;   /* ignore degenerate faces */
        numFacets++;

        if (pNodeIdFromMeshVertex)
            {
            // face loop indices are already compressed
            size_t i = 0;
            MTGARRAY_FACE_LOOP (nodeId, pGraph, baseNodeId)
                {
                /* baseNodeId is in MTG face loop w/ same orientation as Mesh face */
                pNodeIdFromMeshVertex->at(visitor.ClientPointIndex()[i]) = nodeId;
                numEdge++;
                if (visitor.Visible ()[i])
                    {
                    pGraph->SetMaskAt (nodeId, MTG_DIRECTED_EDGE_MASK);
                    MTGNodeId mateId = pGraph->EdgeMate(nodeId);
                    pGraph->SetMaskAt (mateId, MTG_DIRECTED_EDGE_MASK);
                    numPrimary++;
                    }
                i++;
                }
            MTGARRAY_END_FACE_LOOP (nodeId, pGraph, baseNodeId)
            }

        if (pPolyfaceArrayIndexFromNodeId)
            {
            size_t i = 0;
            MTGARRAY_FACE_LOOP (nodeId, pGraph, baseNodeId)
                {
                /* both sides of manifold get mapped to same read index */
                MTGNodeId partnerNodeId = jmdlMTGGraph_getFacePartner (pGraph, nodeId, thruOffset);
                size_t position = visitor.IndexPosition ()[i];
                pPolyfaceArrayIndexFromNodeId->at((size_t)nodeId) = position;
                pPolyfaceArrayIndexFromNodeId->at((size_t)partnerNodeId) = position;
                i++;
                }
            MTGARRAY_END_FACE_LOOP (nodeId, pGraph, baseNodeId)
            }
        }
    if (numFacets == 0)
        return false;
    /* stitch (use same tolerance as jmdlMTGFacets_addDoubleFace) */
    if (stitchSelect == 0)
        {
        if (!jmdlMTGFacets_stitchFacets (pFacets, tolerance, 0.0))
            return false;
        }
    else  // default to modernized stich  . . . 
        {
        if (!jmdlMTGFacets_stitchFacetsForMultiVolume (pFacets, tolerance, 0.0))
            return false;
        }

    /* mark backsides: faces whose nodes are odd & interior are flipped from their original orientation */
    jmdlMTGGraph_setEdgeStarClassificationMasks (pGraph, thruOffset, MTG_BOUNDARY_MASK, MTG_NULL_MASK, MTG_BOUNDARY_MASK, MTG_EXTERIOR_MASK);

    /* update map to move nodeIds on back side (face is flipped) to the front */
    if (pNodeIdFromMeshVertex != NULL)
        {
        for (size_t i = 0; i < numVertex; i++)
            {
            MTGNodeId frontNodeId = pNodeIdFromMeshVertex->at(i);
            if (jmdlMTGGraph_getMask (pGraph, frontNodeId, MTG_EXTERIOR_MASK))
                pNodeIdFromMeshVertex->at(i) = jmdlMTGGraph_getFacePartner (pGraph, frontNodeId, thruOffset);
            }
        }

    /* delete edges fully on the back side */
    if (dropToSingleFace)
        {
        int nDrop = jmdlMTGGraph_dropByDoubleMask (pGraph, MTG_EXTERIOR_MASK, true);

        /* update Inv map to reflect deleted nodes */
        if (pPolyfaceArrayIndexFromNodeId != NULL && nDrop > 0)
            {
            size_t numNode = pPolyfaceArrayIndexFromNodeId->size ();
            for (size_t i = 0; i < numNode; i++)
                if (!jmdlMTGGraph_isValidNodeId (pGraph, (MTGNodeId)i))
                    pPolyfaceArrayIndexFromNodeId->at (i) = invalidIndex;
            }
        }
    return true;
    }

/*-----------------------------------------------------------------*//**
* @description Add MTGFacets to an indexed polyface.
* @param [in] pFacets source facets
* @param [in,out] polyface destination polyfaace
* @return number of facets added to polyface
* @bsihdr                                       DavidAssaf      06/02
+---------------+---------------+---------------+---------------+------*/
size_t   AddMTGFacetsToIndexedPolyface
(
MTGFacets*                      pFacets,
PolyfaceHeaderR             polyface,
MTGMask                     exclusionMask,
MTGMask                     visibleEdgeMask
)
    {
    MTGGraph * pGraph = jmdlMTGFacets_getGraph (pFacets);
    MTGMask visitMask = jmdlMTGGraph_grabMask (pGraph);
    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);
    bvector<DPoint3d>&point = polyface.Point ();
    bvector<int>&pointIndex = polyface.PointIndex ();
    int vertexLabelOffset = pFacets->vertexLabelOffset;    
    size_t baseVertexId = point.size ();
    size_t facetCount = 0;
    int maxMTGVertexIndex = -1;
    MTGMask skipMask = visitMask | exclusionMask;
    size_t numPointIndex = 0;
    size_t numVisible = 0;
    MTGARRAY_SET_LOOP (seedNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, skipMask))
            {
            jmdlMTGGraph_setMaskAroundFace (pGraph, seedNodeId, visitMask);
            int mtgVertexId;
            MTGARRAY_FACE_LOOP (faceNodeId, pGraph, seedNodeId)
                {
                jmdlMTGGraph_getLabel (pGraph, &mtgVertexId, faceNodeId, vertexLabelOffset);
                if (mtgVertexId > maxMTGVertexIndex)
                    maxMTGVertexIndex = mtgVertexId;
                size_t polyfaceVertexId = baseVertexId + (size_t)mtgVertexId;
                int signedPointIndex = static_cast <int> (polyfaceVertexId + 1);
                if (visibleEdgeMask != MTG_NULL_MASK && !pGraph->GetMaskAt (faceNodeId, visibleEdgeMask))
                    signedPointIndex *= -1;
                numPointIndex++;
                if (signedPointIndex > 0)
                    numVisible++;
                pointIndex.push_back (signedPointIndex);
                }
            MTGARRAY_END_FACE_LOOP (faceNodeId, pGraph, seedNodeId)
            facetCount++;
            pointIndex.push_back (0);
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, pGraph)
    jmdlMTGGraph_dropMask (pGraph, visitMask);

    for (size_t i = 0; i <= (size_t)maxMTGVertexIndex; i++)
        {
        point.push_back (pFacets->vertexArrayHdr[i]);        
        }
    return facetCount;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
