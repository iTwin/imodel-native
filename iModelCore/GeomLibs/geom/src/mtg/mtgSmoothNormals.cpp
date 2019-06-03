/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------------------*//**
* @description Set the normal for each interior node of the facet set.  If no default normal
*              is given, degenerate faces' nodes have no normal (normal label is the default).
* @param pFacets                IN OUT  facet set with room for separate normals
* @param pDefaultNormalIndex    OUT     index of the default normal (or NULL)
* @param pDefaultNormal         IN      default normal to give to each degenerate interior face (or NULL)
* @param pFaceXYZ               IN      scratch array (emptied on input, but not output)
* @return true if normals computed successfully
* @bsimethod                                    DavidAssaf      08/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool   jmdlMTGFacets_computeSeparateNormals
(
        MTGFacets*              pFacets,
        int*                    pDefaultNormalIndex,
const   DPoint3d*               pDefaultNormal,
        bvector<DPoint3d>   *pFaceXYZ
)
    {
    EmbeddedDPoint3dArray*  pFacetsXYZ;
    MTGMask                 faceVisited;
    DPoint3d                normal;
    bool                 bNonDegenerateFace;
    int                     nOffset, vOffset, nIndex0;

    if (!pFacets || !pFaceXYZ)
        return false;

    pFacetsXYZ = &pFacets->vertexArrayHdr;
    MTGGraph* pGraph = jmdlMTGFacets_getGraph (pFacets);
    nOffset = pFacets->normalLabelOffset;
    vOffset = pFacets->vertexLabelOffset;

    // add normal for degenerate faces if desired
    nIndex0 = pDefaultNormal ? jmdlMTGFacets_addNormal (pFacets, pDefaultNormal) : pGraph->GetLabelDefaultAt(nOffset);

    // set up visited mask
    faceVisited = pGraph->GrabMask ();
    pGraph->ClearMask ( faceVisited);

    jmdlEmbeddedDPoint3dArray_empty (&pFacets->normalArrayHdr);
    // set normals at interior nodes
    MTGARRAY_SET_LOOP (nodeId, pGraph)
        {
        if (!pGraph->GetMaskAt (nodeId, faceVisited))
            {
            pGraph->SetMaskAroundFace (nodeId, faceVisited); // exterior face nodes have the defaultLabelValue for normals
            if (pGraph->GetMaskAt (nodeId, MTG_EXTERIOR_MASK))
                {
                }
            else
                {
                jmdlMTGFacets_getFaceCoordinates (pGraph, pFaceXYZ, pFacetsXYZ, nodeId, vOffset);

                // assume all interior faces have same orientation (this is true if we have used the MTG stitcher)
                bNonDegenerateFace = bsiGeom_polygonNormal (&normal, NULL, jmdlEmbeddedDPoint3dArray_getPtr (pFaceXYZ, 0),
                                                                     jmdlEmbeddedDPoint3dArray_getCount (pFaceXYZ));

                // add the normal for EVERY node of an interior face
                MTGARRAY_FACE_LOOP (nnodeId, pGraph, nodeId)
                    {
                    if (bNonDegenerateFace)
                        jmdlMTGGraph_setLabel (pGraph, nnodeId, nOffset, jmdlMTGFacets_addNormal (pFacets, &normal));
                    else
                        jmdlMTGGraph_setLabel (pGraph, nnodeId, nOffset, nIndex0);  // degenerate face gets default normal
                    }
                MTGARRAY_END_FACE_LOOP (nnodeId, pGraph, nodeId)
                }
            }
        }
    MTGARRAY_END_SET_LOOP (nodeId, pGraph)

    pGraph->DropMask (faceVisited);

    if (pDefaultNormalIndex)
        *pDefaultNormalIndex = nIndex0;

    return true;
    }

static bool GetSectorAngle
(
MTGFacets* pFacets,
MTGGraph*  pGraph,
MTGNodeId  nodeId,
DVec3dR    normal,
double    &angle
)
    {
    MTGNodeId rightNodeId = pGraph->FSucc (nodeId);
    MTGNodeId leftNodeId  = pGraph->FPred (nodeId);
    DPoint3d xyz, rightXYZ, leftXYZ;
    if (   jmdlMTGFacets_getNodeCoordinates (pFacets, &xyz, nodeId)
        && jmdlMTGFacets_getNodeCoordinates (pFacets, &rightXYZ, rightNodeId)
        && jmdlMTGFacets_getNodeCoordinates (pFacets, &leftXYZ, leftNodeId))
        {
        DVec3d rightVector, leftVector;
        rightVector.differenceOf (&rightXYZ, &xyz);
        leftVector.differenceOf (&leftXYZ, &xyz);
        angle = rightVector.signedAngleTo (&leftVector, &normal);
        if (angle < 0.0)
            angle += msGeomConst_2pi;
        return true;
        }
    return false;
    }

static bool GetIndexedInteriorNormal
(
MTGGraph*  pGraph,
DVec3dR    normal,
MTGNodeId  nodeId,
int        labelOffset,
EmbeddedDPoint3dArray* pNormals
)
    {
    int index;
    if (!pGraph->GetMaskAt (nodeId, MTG_EXTERIOR_MASK)
        && jmdlMTGGraph_getLabel (pGraph, &index, nodeId, labelOffset)
        && index >= 0
        &&  jmdlEmbeddedDPoint3dArray_getDPoint3d (pNormals, &normal, index))
        return true;
    return false;
    }


static void DoNormalAveraging
(
MTGFacets *pFacets,
MTGGraph *pGraph,
EmbeddedDPoint3dArray* pOldNormals,
EmbeddedDPoint3dArray *pNewNormals,
MTGNodeId startNodeId,
int numNode,
MTGMask visitMask,
int normalLabelOffset,
int noNormal
)
    {

    DVec3d averageNormal, currNormal;
    averageNormal.zero ();
    MTGNodeId currNodeId;
    int i;
    double angleSum = 0.0;
    int numOK = 0;
    for (i = 0, currNodeId = startNodeId;
         i < numNode;
         i++, currNodeId = pGraph->VSucc (currNodeId)
        )
        {
        double angle;
        if (GetIndexedInteriorNormal (pGraph, currNormal, currNodeId, normalLabelOffset, pOldNormals)
            && GetSectorAngle (pFacets, pGraph, currNodeId, currNormal, angle)
            )
            {
            angleSum += angle;
            averageNormal.sumOf (&averageNormal, &currNormal, angle);
            numOK++;
            }
        pGraph->SetMaskAt (currNodeId, visitMask);
        }
    bsiDPoint3d_normalizeInPlace (&averageNormal);

    int newNormalIndex = noNormal;
    if (numOK > 0)
        {
        newNormalIndex = jmdlEmbeddedDPoint3dArray_getCount (pNewNormals);
        jmdlEmbeddedDPoint3dArray_addDPoint3d (pNewNormals, &averageNormal);
        }
    // index to the new normal
    for (i = 0, currNodeId = startNodeId;
         i < numNode;
         i++, currNodeId = pGraph->VSucc (currNodeId)
        )
        {
        jmdlMTGGraph_setLabel (pGraph, currNodeId, normalLabelOffset, newNormalIndex);
        }
    }


/*----------------------------------------------------------------------------------*//**
* @description Set the average normal for each interior node of the facet set.
* On input each node has a normal in the facets' own array.
* On output, the facets' array is updated with new normals.
* @param pFacets                IN OUT  facet set with precomputed separate normals
* @param defaultNormalIndex     IN      index of the default normal (or the default normal label)
* @param minCosine              IN      cosine of minimum radian angle between preserved normals of adjacent faces
* @param pNewNormalArray        IN      scratch array (emptied on input, but not output)
* @return true if normals averaged successfully
* @bsimethod                                    DavidAssaf      08/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool   averageSeparateNormals
(
MTGFacets*              pFacets,
int                     defaultNormalIndex,
double                  minCosine,
EmbeddedDPoint3dArray*  pNewNormalArray
)
    {
    if (NULL == pFacets || NULL == pNewNormalArray)
        return false;

    EmbeddedDPoint3dArray* pOldNormalArray = &pFacets->normalArrayHdr;
    MTGGraph* pGraph = jmdlMTGFacets_getGraph (pFacets);
    int normalLabelOffset = pFacets->normalLabelOffset;
    int noNormal = pGraph->GetLabelDefaultAt (normalLabelOffset);

    // set up visited mask
    MTGMask vMask = pGraph->GrabMask ();
    MTGMask sharpEdgeMask = pGraph->GrabMask ();
    pGraph->ClearMask ( vMask);
    pGraph->ClearMask ( sharpEdgeMask);
    jmdlEmbeddedDPoint3dArray_empty (pNewNormalArray);

    int numSharp  = 0;
    int numSmooth = 0;
    // We know .... Every sector has its own face's outward normal.
    // Mark left side of edge if either side is exterior or if dihedral angle is sharp.
    MTGARRAY_SET_LOOP (node0Id, pGraph)
        {
        DVec3d normal0, normal1;
        bool breakEdge = true;
        MTGNodeId node1Id = pGraph->VPred (node0Id);
        if (   GetIndexedInteriorNormal (pGraph, normal0, node0Id, normalLabelOffset, pOldNormalArray)
            && GetIndexedInteriorNormal (pGraph, normal1, node1Id, normalLabelOffset, pOldNormalArray)
            )
            {
            if (normal0.dotProduct (&normal1) >= minCosine)
                breakEdge = false;
            }
        if (breakEdge)
            {
            numSharp++;
            pGraph->SetMaskAt (node0Id, sharpEdgeMask);
            }
        else
            numSmooth++;
        }
    MTGARRAY_END_SET_LOOP (node0Id, pGraph)

    // compute averages from each break edge to successor.
    // Although it does so piecewise, it is guaranteed to set the vMask completely around
    // any vertex that has 1 or more break edges, and leave vMask unset at any other node.
    MTGARRAY_SET_LOOP (breakNodeId, pGraph)
        {
        if (pGraph->GetMaskAt (breakNodeId, sharpEdgeMask))
            {
            int numNode = 1;
            for (MTGNodeId nextNodeId = pGraph->VSucc (breakNodeId);
                    !pGraph->GetMaskAt (nextNodeId, sharpEdgeMask);
                    nextNodeId = pGraph->VSucc (nextNodeId))
                numNode++;
            DoNormalAveraging (pFacets, pGraph, pOldNormalArray, pNewNormalArray, breakNodeId, numNode, vMask, normalLabelOffset,
                        noNormal);
            }
        }
    MTGARRAY_END_SET_LOOP (breakNodeId, pGraph)

    // Go back around to catch vertices that are completely smooth.
    MTGARRAY_SET_LOOP (startNodeId, pGraph)
        {
        if (!pGraph->GetMaskAt (startNodeId, vMask))
            {
            int numNode = pGraph->CountNodesAroundVertex (startNodeId);
            DoNormalAveraging (pFacets, pGraph, pOldNormalArray, pNewNormalArray, startNodeId, numNode, vMask, normalLabelOffset,
                        noNormal);
            }
        }
    MTGARRAY_END_SET_LOOP (startNodeId, pGraph)

    pGraph->DropMask (sharpEdgeMask);
    pGraph->DropMask (vMask);

    jmdlEmbeddedDPoint3dArray_empty (pOldNormalArray);
    jmdlEmbeddedDPoint3dArray_addDPoint3dArray (pOldNormalArray,
                jmdlEmbeddedDPoint3dArray_getPtr (pNewNormalArray, 0),
                jmdlEmbeddedDPoint3dArray_getCount (pNewNormalArray));
    return true;
    }

// FAIL ... if ... numPerFace > 2 && any face has more than numPerface verts.
static bool ExractPolyface
(
MTGFacets*pFacets,
PolyfaceHeaderR polyface
)
    {
    static int s_allPositive = 0;
    MTGGraph* pGraph = jmdlMTGFacets_getGraph (pFacets);
    MTGMask visitMask = pGraph->GrabMask ();
    pGraph->ClearMask ( visitMask);

    jmdlEmbeddedDPoint3dArray_swapContents (pXYZ, &pFacets->vertexArrayHdr);
    jmdlEmbeddedDPoint3dArray_swapContents (pNormal, &pFacets->normalArrayHdr);
    jmdlEmbeddedIntArray_empty (pXYZIndex);
    jmdlEmbeddedIntArray_empty (pNormalIndex);
    int normalLabelOffset = pFacets->normalLabelOffset;
    int vertexLabelOffset = pFacets->vertexLabelOffset;
    MTGARRAY_SET_LOOP (faceSeed, pGraph)
        {
        if (   !pGraph->GetMaskAt (faceSeed, visitMask)
            && !pGraph->GetMaskAt (faceSeed, MTG_EXTERIOR_MASK))
            {
            // First visit to this facet ...
            pGraph->SetMaskAroundFace (faceSeed, visitMask);
            int numThisFace = 0;
            MTGARRAY_FACE_LOOP (currNode, pGraph, faceSeed)
                {
                MTGNodeId predNode = pGraph->VPred (currNode);
                int vertexIndex, normalIndex, normalIndex1;
                jmdlMTGGraph_getLabel (pGraph, &vertexIndex, currNode, vertexLabelOffset);
                jmdlMTGGraph_getLabel (pGraph, &normalIndex, currNode, normalLabelOffset);
                jmdlMTGGraph_getLabel (pGraph, &normalIndex1, predNode, normalLabelOffset);
                int xyzIndex = vertexIndex + 1;
                if (normalIndex1 == normalIndex && s_allPositive == 0)
                    xyzIndex = - xyzIndex;
                int normalIndexA = normalIndex + 1;
                jmdlEmbeddedIntArray_addInt (pXYZIndex, xyzIndex);
                jmdlEmbeddedIntArray_addInt (pNormalIndex, normalIndexA);
                numThisFace ++;
                }
            MTGARRAY_END_FACE_LOOP (currNode, pGraph, faceSeed)
            if (numPerFace > 1)
                {
                if (numThisFace > numPerFace)
                    return false;
                // Pad to blocked face
                for (int i = numThisFace; i < numPerFace; i++)
                    {
                    jmdlEmbeddedIntArray_addInt (pXYZIndex, 0);
                    jmdlEmbeddedIntArray_addInt (pNormalIndex, 0);
                    }
                }
            else
                {
                jmdlEmbeddedIntArray_addInt (pXYZIndex, 0);
                jmdlEmbeddedIntArray_addInt (pNormalIndex, 0);
                }
            }
        }
    MTGARRAY_END_SET_LOOP (faceSeed, pGraph)
    pGraph->DropMask (visitMask);
    return true;
    }

void FixupOrientation
(
MTGFacets* pFacets,
double &originalVolume
)
    {
    originalVolume = 0.0;
    MTGGraph* pGraph = jmdlMTGFacets_getGraph (pFacets);
    MTGARRAY_SET_LOOP (curr, pGraph)
        {
        if (pGraph->GetMaskAt (curr, MTG_EXTERIOR_MASK))
            return;
        }
    MTGARRAY_END_SET_LOOP (curr, pGraph)

    jmdlMTGFacets_volume (pFacets, &originalVolume);
    if (originalVolume < 0.0)
        jmdlMTGFacets_reverseOrientation (pFacets);
    }

static double s_absTol = 1.0e-8;
static double s_relTol = 1.0e-8;
/*----------------------------------------------------------------------------------*//**
* @description Given the polyface arrays of a mesh, stitch and constuct smoothed normals.
*   Return
* @remarks Consistently oriented normals are computed at each sector of each face.
   If the mesh is closed and the volume is negative, order is reversed.
* @param [in] polyface mesh to be modified.
* @param dihedralAngleTol   IN      minimum radian angle between preserved normals of adjacent faces
* @return true if normals were computed.
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr mdlMesh_smoothMeshNormalsFromPolyfaceArraysExt
(
PolyfaceHeaderR         polyface,
double dihedralAngleRadiansTol
)
    {
    MTGFacetsP facets = jmdlMTGFacets_grab ();
    bvector<MTGNodeId> readIndexToNodeId;
    bvector<DPoint3d> faceXYZArray;
    bvector<size_t>    nodeIdToReadIndex;
    PolyfaceToMTG (facets, &readIndexToNodeId, &nodeIdToReadIndex, polyface, true, s_absTol, s_relTol);

    double originalVolume;
    FixupOrientation (facets, originalVolume);
    int nIndex0 = 0;
    DVec3d normal0;
    // compute per-node normals and set default normal (NOT zero normal---crashes renderer!)
    jmdlMTGFacets_computeSeparateNormals (facets, &nIndex0, &normal0, &faceXYZArray);

    double angleTol = 0.32;
    // compute average facet normals
    averageSeparateNormals (facets, nIndex0, cos (angleTol), &faceXYZArray);

    if (ExractPolyface (facets, polyface))
        status = SUCCESS;

wrapup:
    jmdlMTGFacets_drop (facets);
    return status;
    }

END_BENTLEY_GEOMETRY_NAMESPACE