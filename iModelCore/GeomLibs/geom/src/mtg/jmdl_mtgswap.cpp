/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct SwapContext;
typedef bool    (*MTGSwap_TestFunction)(SwapContext *, MTGNodeId);


struct SwapContext
    {
    MTGGraph                *m_graph;
    MTG_MarkSet             m_markSet;
    int                     m_vertexLabelOffset;
    EmbeddedDPoint3dArray   *m_coordinateArray;
    MTGMask                 m_fixedEdgeMask;
    double                  m_xScale;
    MTGSwap_TestFunction    m_testFunc;
    MTGMask                 m_railMask;

SwapContext (
    MTGGraph                *pGraph,
    int                     vertexLabelOffset,
    bvector<DPoint3d>       *pCoordinateArray,
    MTGMask                 fixedEdgeMask,
    MTGMask                 railMask,
    MTGSwap_TestFunction    testFunc,
    double                  scaleFactor
    ) : m_graph (pGraph), m_vertexLabelOffset (vertexLabelOffset), m_coordinateArray (pCoordinateArray),
        m_fixedEdgeMask (fixedEdgeMask), m_railMask (railMask), m_testFunc (testFunc),
        m_xScale (scaleFactor),
        m_markSet (pGraph, MTG_ScopeEdge)
    {
    }

void Panic () {}


bool GetXYZ (DPoint3dR xyz, MTGNodeId nodeId)
    {
    int vertexIndex;
    if (jmdlMTGGraph_getLabel (m_graph, &vertexIndex, nodeId, m_vertexLabelOffset)
        && jmdlEmbeddedDPoint3dArray_getDPoint3d
                            (
                            m_coordinateArray,
                            &xyz,
                            vertexIndex
                            ))
        {
        return true;
        }
    Panic ();
    return false;
    }




// return true if the other side of nodeIdA is a non-exterior triangle.
bool GetNormalAcrossEdge (MTGNodeId nodeIdA, DPoint3dCR xyzA, DPoint3dCR xyzB, DVec3dR normal)
    {
    normal.Zero ();
    MTGNodeId nodeIdD = m_graph->EdgeMate (nodeIdA);
    if (m_graph->GetMaskAt (nodeIdD, MTG_EXTERIOR_MASK))
        return false;
    MTGNodeId nodeIdE = m_graph->EdgeMate (nodeIdD);
    MTGNodeId nodeIdF = m_graph->FSucc (nodeIdE);
    MTGNodeId nodeIdG = m_graph->FSucc (nodeIdF);
    if (nodeIdG != nodeIdD)
        return false;
    DPoint3d xyzF;
    if (!GetXYZ (xyzF, nodeIdF))
        return false;
    normal = DVec3d::FromNormalizedCrossProductToPoints (xyzA, xyzF, xyzB);
    return true;
    }

bool AddWingAnglesToSums
(
MTGNodeId nodeIdA,      // inside edge of wing.
DPoint3dCR xyzA,        // coordinates at A
DPoint3dCR xyzB,        // coordinates at B
double &sum0,           // evolving sum
DVec3dCR normal0,       // inside normal for sum0
double &sum1,           // evolving sum
DVec3dCR normal1        // inside normal for sum1
)
    {
    DVec3d wingNormal;
    if (GetNormalAcrossEdge (nodeIdA, xyzA, xyzB, wingNormal))
        {
        sum0 += normal0.AngleTo (wingNormal);
        sum1 += normal1.AngleTo (wingNormal);
        return true;
        }
    return false;
    }

};


/************************************************************
** Node labeling in adjacent triangles being flipped
  +---------------------+
  |F                 E/A|
  |                 /   |
  |               /     |
  |             /       |
  |           /         |
  |         /           |
  |       /             |
  |     /               |
  |   /                 |
  |D/B                 C|
  +---------------------+
*************************************************************/



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGSwap_chooseAndRemove
(
SwapContext *pContext,
MTGNodeId   *pNodeId
)
    {
    *pNodeId = pContext->m_markSet.ChooseAndRemoveNode ();
    return *pNodeId != MTG_NULL_NODEID;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGSwap_isRuleEdge
(
MTGGraph    *pGraph,
MTGNodeId   nodeId,
MTGMask     railMask
)
    {
    MTGNodeId mateNodeId;
    MTGNodeId predNodeId0, succNodeId0;
    MTGNodeId predNodeId1, succNodeId1;
    mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, nodeId);
    predNodeId0 = jmdlMTGGraph_getFPred (pGraph, nodeId);
    predNodeId1 = jmdlMTGGraph_getFPred (pGraph, mateNodeId);
    succNodeId0 = jmdlMTGGraph_getFSucc (pGraph, nodeId);
    succNodeId1 = jmdlMTGGraph_getFSucc (pGraph, mateNodeId);
    return
        (  jmdlMTGGraph_getMask (pGraph, succNodeId0, railMask)
        && jmdlMTGGraph_getMask (pGraph, succNodeId1, railMask))
        ||
        (  jmdlMTGGraph_getMask (pGraph, predNodeId0, railMask)
        && jmdlMTGGraph_getMask (pGraph, predNodeId1, railMask))
        ;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlMTGSwap_testAndAdd
(
SwapContext *pContext,
MTGNodeId   nodeId
)
    {
    if (!jmdlMTGGraph_getMask (pContext->m_graph, nodeId, pContext->m_fixedEdgeMask))
        {
        if (!jmdlMTGGraph_getMask (pContext->m_graph, nodeId, pContext->m_fixedEdgeMask))
            pContext->m_markSet.AddNode (nodeId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGSwap_getUV
(
SwapContext *pContext,
DPoint2d    *pUV,
int         nodeId
)
    {
    DPoint3d xyz;
    int vertexIndex;
    if (jmdlMTGGraph_getLabel (pContext->m_graph, &vertexIndex, nodeId, pContext->m_vertexLabelOffset)
        && jmdlEmbeddedDPoint3dArray_getDPoint3d
                            (
                            pContext->m_coordinateArray,
                            &xyz,
                            vertexIndex
                            ))
        {
        pUV->x = xyz.x;
        pUV->y = xyz.y;
        return true;
        }
    pContext->Panic ();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGSwap_getXYZ
(
SwapContext *pContext,
DPoint3d    *pXYZ,
int         nodeId
)
    {
    DPoint3d xyz;
    int vertexIndex;
    if (jmdlMTGGraph_getLabel (pContext->m_graph, &vertexIndex, nodeId, pContext->m_vertexLabelOffset)
        && jmdlEmbeddedDPoint3dArray_getDPoint3d
                            (
                            pContext->m_coordinateArray,
                            &xyz,
                            vertexIndex
                            ))
        {
        *pXYZ = xyz;
        return true;
        }
    pContext->Panic ();
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlMTGSwap_transferVertexId
(
SwapContext *pContext,
MTGNodeId   fromNodeId,
MTGNodeId   toNodeId
)
    {
    MTGGraph *pGraph = pContext->m_graph;
    int offset = pContext->m_vertexLabelOffset;
    int vertexId;

    jmdlMTGGraph_getLabel (pGraph, &vertexId, fromNodeId, offset  );
    jmdlMTGGraph_setLabel (pGraph, toNodeId,  offset,     vertexId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t jmdlMTGSwap_flipEdges
(
SwapContext *pContext
)
    {
    MTGGraph *pGraph = pContext->m_graph;
    MTGNodeId nodeAId,nodeBId,nodeCId,nodeDId,nodeEId,nodeFId;
    static int s_maxFlip = 1000;
    int numFlip = 0;

    pContext->m_markSet.Clear ();

    /* Fill the new set with all allowable edges */
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        jmdlMTGSwap_testAndAdd (pContext, currNodeId);
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)

    /* Remove and flip edges until the set is exhausted */
    for(;numFlip < s_maxFlip && jmdlMTGSwap_chooseAndRemove (pContext, &nodeAId);)
        {
        if (pContext->m_railMask && !jmdlMTGSwap_isRuleEdge (pGraph, nodeAId, pContext->m_railMask))
            {
            /* Don't flip -- ruling condition not satisfied. */
            }
        else if (pContext->m_testFunc (pContext, nodeAId))
            {
            numFlip++;
            /* Identify all 6 nodes on both triangular faces */
            nodeBId = jmdlMTGGraph_getFSucc (pGraph, nodeAId);
            nodeDId = jmdlMTGGraph_getVSucc (pGraph, nodeBId);
            nodeEId = jmdlMTGGraph_getFSucc (pGraph, nodeDId);
            nodeCId = jmdlMTGGraph_getFSucc (pGraph, nodeBId);
            nodeFId = jmdlMTGGraph_getFSucc (pGraph, nodeEId);

            /* Extract the edge. */
            jmdlMTGGraph_vertexTwist (pGraph, nodeAId,nodeEId);
            jmdlMTGGraph_vertexTwist (pGraph, nodeBId,nodeDId);

            /* Mark the boundaries of the face so they get considered for later flips */
            MTGARRAY_FACE_LOOP (currNodeId, pGraph, nodeEId)
                {
                jmdlMTGSwap_testAndAdd (pContext, currNodeId);
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, nodeEId)

            /* Reinsert the edge */
            jmdlMTGGraph_vertexTwist (pGraph, nodeAId,nodeCId);
            jmdlMTGGraph_vertexTwist (pGraph, nodeDId,nodeFId);
            jmdlMTGSwap_transferVertexId (pContext, nodeCId, nodeAId);
            jmdlMTGSwap_transferVertexId (pContext, nodeFId, nodeDId);
            }
        }
    return numFlip;
    }

#define MAG2(Z) (Z.x*Z.x + Z.y*Z.y)

/*---------------------------------------------------------------------------------**//**
* Compute the aspect ratio of a triange, defined as mu = area/ sum of squared edge lengths This is large and positive for good CCW triangles,
* small positive for 'flat' (near degenerate) CCW triangles, zero for true degenerate triangle small negative for 'flat' (near degenerate) CW
* triangles, large negative for non-flat CW triangles.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGSwap_quadraticAspectRatio        /* <= false if degenerate */
(
double  *ratioP,                /* <= returned aspect ratio.  0 if degenerate */
DPoint2d *point0P,
DPoint2d *point1P,
DPoint2d *point2P
)
    {
    DPoint2d U,V,W;
    double area;        /* SIGNED area of the triangle */
    double perim;       /* sum of SQUARED edge lengths */
    bool    boolstat;
    /* Compute vectors along each edge */
    U.x = point1P->x - point0P->x;
    U.y = point1P->y - point0P->y;

    V.x = point2P->x - point1P->x;
    V.y = point2P->y - point1P->y;

    W.x = point0P->x - point2P->x;
    W.y = point0P->y - point2P->y;

    area = U.x*V.y - U.y*V.x;
    perim  = (MAG2(U) + MAG2(V) + MAG2(W));

    if (perim > 0.0)
        {
        boolstat = true;
        *ratioP = area / perim;
        }
    else
        {
        boolstat = false;
        *ratioP = 0.0;
        }
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* Compute the aspect ratio of a triange, defined as mu = area/ sum of squared edge lengths This is large and positive for good CCW triangles,
* small positive for 'flat' (near degenerate) CCW triangles, zero for true degenerate triangle small negative for 'flat' (near degenerate) CW
* triangles, large negative for non-flat CW triangles.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGSwap_quadraticXYZAspectRatio     /* <= false if degenerate */
(
double  *ratioP,                /* <= returned aspect ratio.  0 if degenerate */
DPoint3d *point0P,
DPoint3d *point1P,
DPoint3d *point2P
)
    {
    DPoint3d U,V,W;
    DPoint3d UcrossV;
    double area;        /* SIGNED area of the triangle */
    double perim;       /* sum of SQUARED edge lengths */
    bool    boolstat;
    /* Compute vectors along each edge */
    U.DifferenceOf (*point1P, *point0P);
    V.DifferenceOf (*point2P, *point1P);
    W.DifferenceOf (*point0P, *point2P);

    UcrossV.CrossProduct (U, V);
    area = 0.5 * UcrossV.Magnitude ();

    perim  = (U.Magnitude () + V.Magnitude () + W.Magnitude ());

    if (perim > 0.0)
        {
        boolstat = true;
        *ratioP = area / perim;
        }
    else
        {
        boolstat = false;
        *ratioP = 0.0;
        }
    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        jmdlMTGSwap_quadraticFlipTest
(
SwapContext     *pContext,
MTGNodeId       nodeAId
)
    {
    MTGGraph *pGraph = pContext->m_graph;
    static double s_smallRatio = 0.01;
    static double s_smallRatioImprovement = 1.25;
    /* Identify all nodes of the adjacent triangles, as labeled above */
    MTGNodeId nodeBId = jmdlMTGGraph_getFSucc (pGraph, nodeAId);
    MTGNodeId nodeCId = jmdlMTGGraph_getFSucc (pGraph, nodeBId);
    MTGNodeId nodeDId = jmdlMTGGraph_getVSucc (pGraph, nodeBId);
    MTGNodeId nodeEId = jmdlMTGGraph_getFSucc (pGraph, nodeDId);
    MTGNodeId nodeFId = jmdlMTGGraph_getFSucc (pGraph, nodeEId);
    bool        swap_required = false;

    if(    jmdlMTGGraph_getFSucc (pGraph, nodeCId) == nodeAId
        && jmdlMTGGraph_getFSucc (pGraph, nodeFId) == nodeDId
      )
        {
        DPoint2d xyAE;
        DPoint2d xyBD;
        DPoint2d xyC;
        DPoint2d xyF;

        double muABC, muDEF, muCAF, muCFB;

        jmdlMTGSwap_getUV (pContext, &xyAE, nodeAId);
        jmdlMTGSwap_getUV (pContext, &xyBD, nodeBId);
        jmdlMTGSwap_getUV (pContext, &xyC, nodeCId);
        jmdlMTGSwap_getUV (pContext, &xyF, nodeFId);
        /* Compute the asect ratios of each triangle under the
           current and flipped configuration.
        */
        if (
               jmdlMTGSwap_quadraticAspectRatio(&muABC, &xyAE,&xyBD,&xyC)
            && jmdlMTGSwap_quadraticAspectRatio(&muDEF, &xyBD,&xyAE,&xyF)
            && jmdlMTGSwap_quadraticAspectRatio(&muCAF, &xyC, &xyAE,&xyF)
            && jmdlMTGSwap_quadraticAspectRatio(&muCFB, &xyC, &xyF, &xyBD)
           )
            {
            /* Select the configuration with the better aspect ratio in its
               poorer triangle */
            double muCurrent = ( muABC < muDEF ? muABC : muDEF );
            double muFlipped = ( muCAF < muCFB ? muCAF : muCFB );
            if (muCurrent < s_smallRatio)
                {
                if (muFlipped > muCurrent * s_smallRatioImprovement)
                    swap_required = true;
                }
            else
                {
                if(muFlipped > muCurrent)
                    swap_required = true;
                }
            }
        }
    return swap_required;
    }

static double dihedralAngleFactor
(
const DPoint3d *pFoldPoint0,
const DPoint3d *pFoldPoint1,
const DPoint3d *pWingPoint0,
const DPoint3d *pWingPoint1
)
    {
    DPoint3d normal0, normal1;
    normal0.CrossProductToPoints (*pFoldPoint0, *pFoldPoint1, *pWingPoint0);
    normal1.CrossProductToPoints (*pFoldPoint1, *pFoldPoint0, *pWingPoint1);
    normal0.Normalize ();
    normal1.Normalize ();
    return fabs (normal0.DotProduct (normal1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        jmdlMTGSwap_quadraticXYZFlipTest
(
SwapContext     *pContext,
MTGNodeId       nodeAId
)
    {
    MTGGraph *pGraph = pContext->m_graph;
    static double s_smallRatio = 0.01;
    static double s_smallRatioImprovement = 1.25;
    static int s_minimizeArea = 1;
    static int s_penalizeLength = 1;
    /* Identify all nodes of the adjacent triangles, as labeled above */
    MTGNodeId nodeBId = jmdlMTGGraph_getFSucc (pGraph, nodeAId);
    MTGNodeId nodeCId = jmdlMTGGraph_getFSucc (pGraph, nodeBId);
    MTGNodeId nodeDId = jmdlMTGGraph_getVSucc (pGraph, nodeBId);
    MTGNodeId nodeEId = jmdlMTGGraph_getFSucc (pGraph, nodeDId);
    MTGNodeId nodeFId = jmdlMTGGraph_getFSucc (pGraph, nodeEId);
    bool      swap_required = false;

    if(    jmdlMTGGraph_getFSucc (pGraph, nodeCId) == nodeAId
        && jmdlMTGGraph_getFSucc (pGraph, nodeFId) == nodeDId
      )
        {
        DPoint3d xyzAE;
        DPoint3d xyzBD;
        DPoint3d xyzC;
        DPoint3d xyzF;

        double muABC, muDEF, muCAF, muCFB;

        jmdlMTGSwap_getXYZ (pContext, &xyzAE, nodeAId);
        jmdlMTGSwap_getXYZ (pContext, &xyzBD, nodeBId);
        jmdlMTGSwap_getXYZ (pContext, &xyzC, nodeCId);
        jmdlMTGSwap_getXYZ (pContext, &xyzF, nodeFId);
        /* Compute the asect ratios of each triangle under the
           current and flipped configuration.
        */
        if (s_minimizeArea)
            {
            DPoint3d cross00, cross01, cross10, cross11;
            double mag00, mag01, mag10, mag11;
            double mu0, mu1;
            /* Compute Area (times 2) in each configuration */
            cross00.CrossProductToPoints (xyzAE, xyzBD, xyzC);
            cross01.CrossProductToPoints (xyzAE, xyzBD, xyzF);
            cross10.CrossProductToPoints (xyzC, xyzF, xyzAE);
            cross11.CrossProductToPoints (xyzC, xyzF, xyzBD);
            mag00 = cross00.Magnitude ();
            mag01 = cross01.Magnitude ();
            mag10 = cross10.Magnitude ();
            mag11 = cross11.Magnitude ();
            mu0 = mag00 + mag01;
            mu1 = mag10 + mag11;
            /* Multiply by length to penalize long diagonals */
            if (s_penalizeLength)
                {
                mu0 *= xyzAE.Distance (xyzBD);
                mu1 *= xyzC.Distance (xyzF);
                }
            swap_required = mu1 < mu0;
            }
        else if (
               jmdlMTGSwap_quadraticXYZAspectRatio(&muABC, &xyzAE,&xyzBD,&xyzC)
            && jmdlMTGSwap_quadraticXYZAspectRatio(&muDEF, &xyzBD,&xyzAE,&xyzF)
            && jmdlMTGSwap_quadraticXYZAspectRatio(&muCAF, &xyzC, &xyzAE,&xyzF)
            && jmdlMTGSwap_quadraticXYZAspectRatio(&muCFB, &xyzC, &xyzF, &xyzBD)
           )
            {
            /* Select the configuration with the better aspect ratio in its
               poorer triangle */
            double muCurrent = ( muABC < muDEF ? muABC : muDEF );
            double muFlipped = ( muCAF < muCFB ? muCAF : muCFB );
            muCurrent *= dihedralAngleFactor (&xyzAE, &xyzBD, &xyzC, &xyzF);
            muFlipped *= dihedralAngleFactor (&xyzC, &xyzF, &xyzAE, &xyzBD);
            if (muCurrent < s_smallRatio)
                {
                if (muFlipped > muCurrent * s_smallRatioImprovement)
                    swap_required = true;
                }
            else
                {
                if(muFlipped > muCurrent)
                    swap_required = true;
                }
            }
        }
    return swap_required;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        jmdlMTGSwap_dihedralAngleFlipTest
(
SwapContext     *pContext,
MTGNodeId       nodeAId
)
    {
    static double s_minimumImprovement = 0.01;  // don't flip if angle sums don't improve much.
    MTGGraph *pGraph = pContext->m_graph;
    /* Identify all nodes of the adjacent triangles, as labeled above */
    // ABC and DEF are the curent triangles, with A and D on opposite sides of the shared edge.
    MTGNodeId nodeBId = jmdlMTGGraph_getFSucc (pGraph, nodeAId);
    MTGNodeId nodeCId = jmdlMTGGraph_getFSucc (pGraph, nodeBId);
    MTGNodeId nodeDId = jmdlMTGGraph_getVSucc (pGraph, nodeBId);
    MTGNodeId nodeEId = jmdlMTGGraph_getFSucc (pGraph, nodeDId);
    MTGNodeId nodeFId = jmdlMTGGraph_getFSucc (pGraph, nodeEId);

    bool      swap_required = false;

    if(    jmdlMTGGraph_getFSucc (pGraph, nodeCId) == nodeAId
        && jmdlMTGGraph_getFSucc (pGraph, nodeFId) == nodeDId
      )
        {
        DPoint3d xyzAE;
        DPoint3d xyzBD;
        DPoint3d xyzC;
        DPoint3d xyzF;


        jmdlMTGSwap_getXYZ (pContext, &xyzAE, nodeAId);
        jmdlMTGSwap_getXYZ (pContext, &xyzBD, nodeBId);
        jmdlMTGSwap_getXYZ (pContext, &xyzC, nodeCId);
        jmdlMTGSwap_getXYZ (pContext, &xyzF, nodeFId);
        DVec3d normalABC = DVec3d::FromNormalizedCrossProductToPoints (xyzAE, xyzBD, xyzC);
        DVec3d normalDEF = DVec3d::FromNormalizedCrossProductToPoints (xyzBD, xyzAE, xyzF);
        DVec3d normalCFB = DVec3d::FromNormalizedCrossProductToPoints (xyzC, xyzF, xyzBD);
        DVec3d normalFCA = DVec3d::FromNormalizedCrossProductToPoints (xyzF, xyzC, xyzAE);
        if (normalABC.DotProduct (normalDEF) > 0.0
            && normalCFB.DotProduct (normalFCA) > 0.0)
            {
            double sum0 = normalABC.AngleTo (normalCFB);
            double sum1 = normalCFB.AngleTo (normalFCA);
            pContext->AddWingAnglesToSums (nodeBId, xyzBD, xyzC, sum0, normalABC, sum1, normalCFB);
            pContext->AddWingAnglesToSums (nodeCId, xyzC, xyzAE, sum0, normalABC, sum1, normalFCA);
            pContext->AddWingAnglesToSums (nodeDId, xyzAE, xyzF, sum0, normalDEF, sum1, normalFCA);
            pContext->AddWingAnglesToSums (nodeFId, xyzF, xyzBD, sum0, normalDEF, sum1, normalCFB);
            swap_required = sum1 < sum0 + s_minimumImprovement;
            }
        }
    return swap_required;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        jmdlMTGSwap_scaledQuadraticFlipTest
(
SwapContext         *pContext,
MTGNodeId           nodeAId
)
    {
    MTGGraph *pGraph    = pContext->m_graph;
    double xFactor      = pContext->m_xScale;

    /* Identify all nodes of the adjacent triangles, as labeled above */
    MTGNodeId nodeBId = jmdlMTGGraph_getFSucc (pGraph, nodeAId);
    MTGNodeId nodeCId = jmdlMTGGraph_getFSucc (pGraph, nodeBId);
    MTGNodeId nodeDId = jmdlMTGGraph_getVSucc (pGraph, nodeBId);
    MTGNodeId nodeEId = jmdlMTGGraph_getFSucc (pGraph, nodeDId);
    MTGNodeId nodeFId = jmdlMTGGraph_getFSucc (pGraph, nodeEId);
    bool      swap_required = false;
    if(jmdlMTGGraph_getFSucc (pGraph, nodeCId) == nodeAId && jmdlMTGGraph_getFSucc (pGraph, nodeFId) == nodeDId)
        {
        DPoint2d xyAE;
        DPoint2d xyBD;
        DPoint2d xyC;
        DPoint2d xyF;

        double muABC, muDEF, muCAF, muCFB, muCurrent, muFlipped;

        jmdlMTGSwap_getUV (pContext, &xyAE, nodeAId);
        jmdlMTGSwap_getUV (pContext, &xyBD, nodeBId);
        jmdlMTGSwap_getUV (pContext, &xyC, nodeCId);
        jmdlMTGSwap_getUV (pContext, &xyF, nodeFId);

        xyAE.x *= xFactor;
        xyBD.x *= xFactor;
        xyC.x  *= xFactor;
        xyF.x  *= xFactor;
        /* Compute the asect ratios of each triangle under the
           current and flipped configuration.
        */
        if  (
               jmdlMTGSwap_quadraticAspectRatio(&muABC, &xyAE,&xyBD,&xyC)
            && jmdlMTGSwap_quadraticAspectRatio(&muDEF, &xyBD,&xyAE,&xyF)
            && jmdlMTGSwap_quadraticAspectRatio(&muCAF, &xyC, &xyAE,&xyF)
            && jmdlMTGSwap_quadraticAspectRatio(&muCFB, &xyC, &xyF, &xyBD)
            )
            {
            /* Select the configuration with the better aspect ratio in its
               poorer triangle */
                muCurrent = ( muABC < muDEF ? muABC : muDEF );
                muFlipped = ( muCAF < muCFB ? muCAF : muCFB );
                if (muFlipped > muCurrent)
                    swap_required = true;
            }
        }
    return swap_required;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGSwap_UFlipTest
(
SwapContext     *pContext,
MTGNodeId       nodeAId
)
    {
    MTGGraph *pGraph = pContext->m_graph;
    DPoint2d uvA, uvB, uvC, uvF;

    /* Identify all nodes of the adjacent triangles, as labeled above */
    MTGNodeId nodeBId = jmdlMTGGraph_getFSucc (pGraph, nodeAId);
    MTGNodeId nodeCId = jmdlMTGGraph_getFSucc (pGraph, nodeBId);
    MTGNodeId nodeDId = jmdlMTGGraph_getVSucc (pGraph, nodeBId);
    MTGNodeId nodeEId = jmdlMTGGraph_getFSucc (pGraph, nodeDId);
    MTGNodeId nodeFId = jmdlMTGGraph_getFSucc (pGraph, nodeEId);
    bool      swap_required = false;

    if (   jmdlMTGGraph_getFSucc (pGraph, nodeCId) == nodeAId
        && jmdlMTGGraph_getFSucc (pGraph, nodeFId) == nodeDId
        && jmdlMTGSwap_getUV (pContext, &uvA, nodeAId)
        && jmdlMTGSwap_getUV (pContext, &uvB, nodeBId)
        && jmdlMTGSwap_getUV (pContext, &uvC, nodeCId)
        && jmdlMTGSwap_getUV (pContext, &uvF, nodeFId)
       )
        {
        double muCurrent = fabs(  uvA.x - uvB.x);

        double muFlipped = fabs(  uvC.x - uvF.x);

        if(muFlipped > muCurrent)
            swap_required = true;
        }
    return swap_required;
    }

/*---------------------------------------------------------------------------------**//**
* Swap triangles to improve triangle quality, based on quadratic aspect ratio.
* aspect ratio of 3D coordinates.  (Quadratic aspect ratio is the triangle area divided by
* the sum of squared edge lengths.  This is equivalent to the Delauney circle
* center condition.)
* @param pGraph <=> graph being modified
* @param vertexLabelOffset => offset to vertex label.
* @param pCoordinates => coordinate array.
* @param fixedEdgeMask => mask identifying edges that cannot be flipped.
* @return number of flips performed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGSwap_flipTrianglesToImproveQuadraticXYZAspectRatio
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         fixedEdgeMask
)
    {
    SwapContext context (pGraph, vertexLabelOffset, pCoordinates,
                            fixedEdgeMask, MTG_NULL_MASK,
                            jmdlMTGSwap_quadraticXYZFlipTest, 1.0);
    return jmdlMTGSwap_flipEdges(&context);
    }

/*---------------------------------------------------------------------------------**//**
* Swap triangles to improve triangle quality, based on quadratic aspect ratio.
* aspect ratio of 3D coordinates.  (Quadratic aspect ratio is the triangle area divided by
* the sum of squared edge lengths.  This is equivalent to the Delauney circle
* center condition.)
* @param pGraph <=> graph being modified
* @param vertexLabelOffset => offset to vertex label.
* @param pCoordinates => coordinate array.
* @param fixedEdgeMask => mask identifying edges that cannot be flipped.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGSwap_flipTrianglesToImproveDihedralAngle
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         fixedEdgeMask
)
    {
    SwapContext context (pGraph, vertexLabelOffset, pCoordinates,
                            fixedEdgeMask, MTG_NULL_MASK,
                            jmdlMTGSwap_dihedralAngleFlipTest, 1.0);
    return jmdlMTGSwap_flipEdges(&context);
    }


/*---------------------------------------------------------------------------------**//**
* Swap triangles to improve triangle quality, based on quadratic aspect ratio.
* aspect ratio of 3D coordinates.   Maintains ruled surface conditions by requiring
* that the quadrilateral formed around a flipped edge have opposing rails identified
* by a mask.
* @param pGraph <=> graph being modified
* @param vertexLabelOffset => offset to vertex label.
* @param pCoordinates => coordinate array.
* @param fixedEdgeMask => mask identifying edges that cannot be flipped.
* @param railMask => mask identifying boundary edges on left.   Must be non-null.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGSwap_flipTrianglesToImproveQuadraticRuledXYZAspectRatio
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         fixedEdgeMask,
MTGMask         railMask
)
    {
    SwapContext context (pGraph, vertexLabelOffset, pCoordinates,
                            fixedEdgeMask, railMask,
                            jmdlMTGSwap_quadraticXYZFlipTest, 1.0);
    return jmdlMTGSwap_flipEdges(&context);
    }

/*---------------------------------------------------------------------------------**//**
* Swap triangles to improve triangle quality, based on quadratic aspect ratio.
* aspect ratio.  (Quadratic aspect ratio is the triangle area divided by
* the sum of squared edge lengths.  This is equivalent to the Delauney circle
* center condition.)
* @param pGraph <=> graph being modified
* @param vertexLabelOffset => offset to vertex label.
* @param pCoordinates => coordinate array.
* @param fixedEdgeMask => mask identifying edges that cannot be flipped.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGSwap_flipTrianglesToImproveQuadraticAspectRatio
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         fixedEdgeMask
)
    {
    SwapContext context (pGraph, vertexLabelOffset, pCoordinates,
                            fixedEdgeMask, MTG_NULL_MASK,
                            jmdlMTGSwap_quadraticFlipTest, 1.0);
    return jmdlMTGSwap_flipEdges(&context);
    }

/*---------------------------------------------------------------------------------**//**
* Swap triangles to improve triangle quality, based on quadratic aspect ratio
* after scaling.  (Quadratic aspect ratio is the triangle area divided by
* the sum of squared edge lengths.  This is equivalent to the Delauney circle
* center condition.)
* aspect ratio.
* @param pGraph <=> graph being modified
* @param vertexLabelOffset => offset to vertex label.
* @param pCoordinates => coordinate array.
* @param fixedEdgeMask => mask identifying edges that cannot be flipped.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGSwap_flipTrianglesToImproveScaledQuadraticAspectRatio
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         fixedEdgeMask,
double xScale,
double yScale
)
    {

    double xFactor = xScale / yScale;
    SwapContext context (pGraph, vertexLabelOffset, pCoordinates,
                            fixedEdgeMask, MTG_NULL_MASK,
                            jmdlMTGSwap_scaledQuadraticFlipTest, xFactor);
    return jmdlMTGSwap_flipEdges(&context);
    }

/*---------------------------------------------------------------------------------**//**
* Swap triangles to improve triangle quality, using only the x coordinate
* to define aspect ratio.
* @param pGraph <=> graph being modified
* @param vertexLabelOffset => offset to vertex label.
* @param pCoordinates => coordinate array.
* @param fixedEdgeMask => mask identifying edges that cannot be flipped.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGSwap_flipTrianglesToImproveUAspectRatio
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         fixedEdgeMask
)
    {
    SwapContext context (pGraph, vertexLabelOffset, pCoordinates,
                            fixedEdgeMask, MTG_NULL_MASK,
                            jmdlMTGSwap_UFlipTest, 1.0);
    return jmdlMTGSwap_flipEdges(&context);
    }

/*---------------------------------------------------------------------------------**//**
* Flip triangles in the the graph to improve aspect ratio using xyz coordinates
* in the vertex array.   Only "ruled" edges are flipped -- must run from one
* rail to another both before and after flip.
* @param  pFacets   <=> Containing facets
* @param railMask => mask identifying rail edges.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGFacets_flipRuledXYZ
(
MTGFacets               *pFacets,
MTGMask                 railMask
)
    {
    return jmdlMTGSwap_flipTrianglesToImproveQuadraticRuledXYZAspectRatio
                (
                &pFacets->graphHdr,
                pFacets->vertexLabelOffset,
                &pFacets->vertexArrayHdr,
                MTG_PRIMARY_EDGE_MASK,
                railMask
                );
    }
END_BENTLEY_GEOMETRY_NAMESPACE
