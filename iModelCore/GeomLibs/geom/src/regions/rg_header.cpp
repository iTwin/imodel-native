/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "Regions/rg_intern.h"
#include "Mtg/mtgprint.fdf"
#include "../DeprecatedFunctions.h"
#include <stdio.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


static int s_globalNoisy = 0;


/*---------------------------------------------------------------------------------**//**
*
* An RG_Header is a header structure for a (probably large and complex) data structure
* describing a subdivision of the plane into regions bounded by curves.
*
* Linear boundaries are handled explicitly within this library.   The library provides the logic, but
* not geometric calculations, for curves.
* Each calling application must provide callbacks to perform various geometric calculations such as:
*<ul>
*<li>Compute the curve range.
*<li>Evaluate curve coordinates at specified parameters (0=start, 1=end).
*<li>Compute intersections of the curve with a line segment or another curve.
*<li>Create a subcurve over a specified parameter range (0=start, 1=end).
*<li>
*</ul>
*
* @bsiclass                                                     EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* Allocate a new region header.  This should be returned to jmdlRG_free
* @return pointer to region header.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public RG_Header    *jmdlRG_new
(
void
)
    {
    RG_Header *pRG = (RG_Header *)BSIBaseGeom::Malloc (sizeof (RG_Header));
    memset (pRG, 0, sizeof (RG_Header));
    pRG->pGraph = jmdlMTGGraph_newGraph ();
    pRG->vertexLabelIndex   = jmdlMTGGraph_defineLabel
                            (
                            pRG->pGraph,
                            RG_VertexLabelTag,
                            MTG_LabelMask_VertexProperty,
                            RG_NULL_VERTEXID
                            );

    pRG->edgeLabelIndex     = jmdlMTGGraph_defineLabel
                            (
                            pRG->pGraph,
                            RG_CurveLabelTag,
                            MTG_LabelMask_EdgeProperty,
                            RG_NULL_CURVEID
                            );

    pRG->parentLabelIndex   = jmdlMTGGraph_defineLabel
                            (
                            pRG->pGraph,
                            RG_ParentLabelTag,
                            MTG_LabelMask_EdgeProperty,
                            RG_NULL_CURVEID
                            );

    pRG->pVertexArray = new bvector<DPoint3d> ();
    pRG->pFaceHoleNodeIdArray = NULL;
    pRG->pFaceRangeTree = pRG->pEdgeRangeTree = NULL;
    pRG->tolerance      = 1.0e-12;
    pRG->minimumTolerance = 1.0e-12;
    pRG->relTol         = 1.0e-12;
    pRG->graphRange.Init ();

    jmdlRG_enableIncrementalEdgeRangeTree (pRG);
    return  pRG;
    }

/*---------------------------------------------------------------------------------**//**
* Free a region header and all associated memory.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRG_free
(
RG_Header       *pRG
)
    {
    if (pRG)
        {
        pRG->pGraph = jmdlMTGGraph_freeGraph (pRG->pGraph);
        delete pRG->pVertexArray;
        jmdlRG_freeFaceRangeTree (pRG);
        jmdlRG_freeEdgeRangeTree (pRG);
        if (pRG->pFaceHoleNodeIdArray)
            jmdlEmbeddedIntArray_free (pRG->pFaceHoleNodeIdArray);

        BSIBaseGeom::Free (pRG);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Set the context pointer passed back to all curve callbacks.
* @param pContext => application-specific pointer arg.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlRG_setFunctionContext
(
RG_Header   *pRG,
RIMSBS_Context   *pContext
)
    {
    pRG->funcs.pContext = pContext;
    }

/*---------------------------------------------------------------------------------**//**
* Get the context pointer passed back to all curve callbacks.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public RIMSBS_Context   * jmdlRG_getFunctionContext
(
RG_Header   *pRG
)
    {
    return pRG->funcs.pContext;
    }


/*---------------------------------------------------------------------------------**//**
* Set a static flag to generate output during merge.
* @param pContext => application-specific pointer arg.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlRG_setNoisy
(
int noisy
)
    {
    s_globalNoisy = noisy;
    }

/*---------------------------------------------------------------------------------**//**
* Return the static flag for debug output.
* @param pContext => application-specific pointer arg.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public int jmdlRG_getNoisy
(
)
    {
    return s_globalNoisy;
    }

/*---------------------------------------------------------------------------------**//**
* Set the function pointer for debug linestrings.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlRG_setOutputLinestringFunction
(
RG_Header   *pRG,
RG_OutputLinestring pFunc,
void *pDebugContext
)
    {
    pRG->pOutputLinestringFunc  = pFunc;
    pRG->pDebugContext = pDebugContext;
    }

/*---------------------------------------------------------------------------------**//**
* Set the function pointer for debug linestrings.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlRG_debugLinestring
(
RG_Header   *pRG,
const DPoint3d *pPoints,
int numPoints,
int color,
int weight,
int     drawmode
)
    {
    if (pRG->pOutputLinestringFunc)
        {
        pRG->pOutputLinestringFunc (
                    pRG->pDebugContext, pRG,
                    pPoints, numPoints,
                    color, weight, drawmode);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Set the function pointer for debug linestrings.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlRG_debugSegment
(
RG_Header   *pRG,
const DPoint3d *pPoint0,
const DPoint3d *pPoint1,
int color,
int weight,
int     drawmode
)
    {
    DPoint3d points[2];
    points[0] = *pPoint0;
    points[1] = *pPoint1;
    jmdlRG_debugLinestring (pRG, points, 2, color, weight, drawmode);
    }


/*---------------------------------------------------------------------------------**//**
* Set the function pointer for debug linestrings.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlRG_debugCircle
(
RG_Header   *pRG,
const DPoint3d *pCenter,
double radius,
int    color,
int    weight,
int     drawmode
)
    {
    double theta;
    int i;
    static int s_numTheta = 8;
    DPoint3d points[1024];
    if (pRG->pOutputLinestringFunc)
        {
        double dTheta = msGeomConst_pi / s_numTheta;
        for (i = 0; i <= s_numTheta; i++)
            {
            theta = i * dTheta;
            points[i].x = pCenter->x + radius * cos (theta);
            points[i].y = pCenter->y + radius * sin (theta);
            points[i].z = pCenter->z;
            }
        jmdlRG_debugLinestring (pRG, points, s_numTheta + 1,
                    color, weight, drawmode);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Set the minimum tolerance for the region graph.   Actual tolerance may be larger
* to accomodate large data ranges.
* @param tolerance => small distance.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlRG_setDistanceTolerance
(
RG_Header   *pRG,
double      tolerance
)
    {
    pRG->minimumTolerance = tolerance;
    if (pRG->tolerance < tolerance)
        pRG->tolerance = tolerance;
    }

/*---------------------------------------------------------------------------------**//**
* Set the abort function to be called during long computations.
* @param abortFunction => (native) test function.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlRG_setAbortFunction
(
RG_Header   *pRG,
RGC_AbortFunction abortFunction
)
    {
    pRG->funcs.abortFunction = abortFunction;
    }


/*---------------------------------------------------------------------------------**//**
* Check if an abort has been requested, either (a) as recorded in the header or
* (b) as returned by a call to the saved abort function
* BUT .. only call the abort function if the running count if
* abort checks is a multiple of the period.
* 
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlRG_checkAbort
(
RG_Header   *pRG,
int period
)
    {
    static int s_counter = 0;
    static int s_counterFrequency = 0;
    if (pRG->aborted)
        return true;
    s_counter++;
    if (period > 0 && s_counter % period != 0)
        return false;


    if (s_counterFrequency && s_counter % s_counterFrequency == 0)
            GEOMAPI_PRINTF ("abort check %d\n", s_counter++);
    if (pRG->funcs.abortFunction && pRG->funcs.abortFunction (pRG))
        {
        pRG->aborted = true;
        }
    return pRG->aborted;
    }


/*---------------------------------------------------------------------------------**//**
* Test and set the abort flag.
* @param newValue => new abort flag.
* @return prior abort flag.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlRG_setAbortFlag
(
RG_Header   *pRG,
bool        newValue
)
    {
    bool    oldValue = pRG->aborted;
    pRG->aborted =  newValue;
    return oldValue;
    }


/*---------------------------------------------------------------------------------**//**
* Return the range of the graph, as recorded by observing all incoming geometry.  Because
* this range is not recomputed if geometry is deleted, it may be larger than the
* actual geometry range after deletions.
*
* @param pRange => Range recorded with the graph.
* This may be larger than the actual range if extremal geometry was removed.
* Return true if the range is well defined.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRG_getRange
(
RG_Header   *pRG,
DRange3d    *pRange
)
    {
    *pRange = pRG->graphRange;
    return !pRange->IsNull ();
    }

/*---------------------------------------------------------------------------------**//**
* If possible, return a transform whose xy plane contains the geometry.
* @param pRG => region context.
* @param pTransform <= returned transform.
* @param pRange <= range of data in transformed system.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRG_getPlaneTransform
(
RG_Header   *pRG,
Transform   *pTransform,
DRange3d    *pRange
)
    {
    bool    bResult = false;
    Transform planeToWorld, worldToPlane;
    DRange3d range;
    DPoint3d *pBuffer;
    static double s_planeRelTol = 1.0e-6;
    int count;
    EmbeddedDPoint3dArray *pXYZArray = jmdlEmbeddedDPoint3dArray_grab ();
    planeToWorld.InitIdentity ();
    range.Init ();


    /* We always have the local point array ... */
    jmdlEmbeddedDPoint3dArray_addDPoint3dArray
                (
                pXYZArray,
                jmdlEmbeddedDPoint3dArray_getPtr (pRG->pVertexArray, 0),
                jmdlEmbeddedDPoint3dArray_getCount (pRG->pVertexArray)
                );

    /* Add more points from the curves, if available ... */
    if (pRG->funcs.appendAllCurveSamplePoints)
        {
        pRG->funcs.appendAllCurveSamplePoints (pRG->funcs.pContext, pXYZArray);
        }

    pBuffer = jmdlEmbeddedDPoint3dArray_getPtr (pXYZArray, 0);
    count = jmdlEmbeddedDPoint3dArray_getCount (pXYZArray);


    if (bsiTransform_initFromPlaneOfDPoint3dArray (&planeToWorld, *pXYZArray)
        && worldToPlane.InverseOf (planeToWorld))
        {
        double bigSize;
        worldToPlane.Multiply (pBuffer, count);
        range.Extend (pBuffer, count);
        bigSize = range.LargestCoordinate ();
        if (fabs (range.high.z - range.low.z) < bigSize * s_planeRelTol)
            bResult = true;
        }

    jmdlEmbeddedDPoint3dArray_drop (pXYZArray);

    if (pRange)
        *pRange = range;

    if (pTransform)
        *pTransform = planeToWorld;

    return bResult;
    }

/*---------------------------------------------------------------------------------**//**
* Multiply geometry (i.e. vertices and curves) by a transform.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRG_multiplyByTransform
(
RG_Header   *pRG,
Transform   *pTransform
)
    {
    pTransform->Multiply (jmdlEmbeddedDPoint3dArray_getPtr (pRG->pVertexArray,0), jmdlEmbeddedDPoint3dArray_getCount (pRG->pVertexArray));

    pTransform->Multiply (pRG->graphRange, pRG->graphRange);

    if (pRG->funcs.transformAllCurves)
        pRG->funcs.transformAllCurves (pRG->funcs.pContext, pTransform);
    }

/*---------------------------------------------------------------------------------**//**
* Resolve "any" node around a face to a consistent reference node of the face.
* @param seedNodeId => starting node for search.
* @return reference nodeId.  As long as the face is not modified, the same
*           reference node will be returned from any seed node.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public MTGNodeId    jmdlRG_resolveFaceNodeId
(
RG_Header   *pRG,
MTGNodeId   seedNodeId
)
    {
    MTGNodeId refNodeId = seedNodeId;
    MTGARRAY_FACE_LOOP (currNodeId, pRG->pGraph, seedNodeId)
        {
        if (currNodeId > refNodeId)
            refNodeId = currNodeId;
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pRG->pGraph, seedNodeId)
    return refNodeId;
    }

/*---------------------------------------------------------------------------------**//**
* Save functions to be called for geometric computations on edges.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlRG_setCurveFunctions
(
RG_Header                           *pRG,
RGC_GetCurveRangeFunction           getCurveRange,
RGC_EvaluateCurveFunction           evaluateCurve,
RGC_IntersectCurveCurveFunction     intersectCurveCurve,
RGC_IntersectSegmentCurveFunction   intersectSegmentCurve,
RGC_CreateSubcurveFunction          createSubcurve,
RGC_SweptCurvePropertiesFunction    sweptCurveProperties,
RGC_EvaluateDerivativesFunction     evaluateDerivatives,
RGC_GetClosestXYPointOnCurveFunction    getClosestXYPointOnCurve,
RGC_IntersectCurveCircleXYFunction  intesectCurveCircleXY,
RGC_GetGroupIdFunction              getGroupId,
RGC_ConsolidateCoincidentGeometryFunction     consolidateCoincidentGeometry
)
    {
    pRG->funcs.getCurveRange            = getCurveRange;
    pRG->funcs.intersectCurveCurve      = intersectCurveCurve;
    pRG->funcs.intersectSegmentCurve    = intersectSegmentCurve;
    pRG->funcs.evaluateCurve            = evaluateCurve;
    pRG->funcs.createSubcurve           = createSubcurve;
    pRG->funcs.sweptCurveProperties     = sweptCurveProperties;
    pRG->funcs.evaluateDerivatives      = evaluateDerivatives;
    pRG->funcs.getClosestXYPointOnCurve = getClosestXYPointOnCurve;
    pRG->funcs.intersectCurveCircleXY   = intesectCurveCircleXY;
    pRG->funcs.getGroupId               = getGroupId;
    pRG->funcs.consolidateCoincidentGeometry =
                                            consolidateCoincidentGeometry;
    }

/*---------------------------------------------------------------------------------**//**
* Save functions to be called for geometric computations on edges.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlRG_setCurveFunctions01
(
RG_Header                           *pRG,
RGC_AppendAllCurveSamplePointsFunction appendAllCurveSamplePoints,
RGC_TransformCurveFunction          transformCurve,
RGC_TransformAllCurvesFunction      transformAllCurves
)
    {
    pRG->funcs.appendAllCurveSamplePoints  = appendAllCurveSamplePoints;
    pRG->funcs.transformCurve           = transformCurve;
    pRG->funcs.transformAllCurves       = transformAllCurves;
    }

/*---------------------------------------------------------------------------------**//**
* Call at conclusion of defining an edge.  Updates the graph's range box to include the
* edge, and enters the edge into the incremental range tree (if the range tree is active).
* @param nodeId => node on the edge.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_completeEdgeEntry
(
RG_Header                       *pRG,
MTGNodeId                       nodeId
)
    {
    DRange3d range;
    double lowOffset = jmdlRG_getTolerance (pRG);
    double highOffset = lowOffset;

    if (jmdlRG_getEdgeRange (pRG, &range, nodeId))
        {
        pRG->graphRange.Extend (range);

        if  (   pRG->incrementalEdgeRanges
             && pRG->pEdgeRangeTree
            )
            {
            rgXYRangeTree_addToTree (pRG->pEdgeRangeTree,
                            range.low.x  - lowOffset,    range.low.y  - lowOffset,
                            range.high.x + highOffset,  range.high.y + highOffset,
                            nodeId);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Call along with other header initialization steps to indicate that edge
* range tree is to be maintained incrementally.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlRG_enableIncrementalEdgeRangeTree
(
RG_Header                       *pRG
)
    {
    jmdlRG_freeEdgeRangeTree (pRG);
    rgXYRangeTree_initializeTree ((void **)&pRG->pEdgeRangeTree);
    pRG->incrementalEdgeRanges = true;

    }

/*---------------------------------------------------------------------------------**//**
* @instance pRG <=> region context
* @param pChainEnds <= node ids at start, end of new chain
* @param pPointArray => array of polyline points.
* @param numPoint => number of points.
* @param parentId => label to store with all edges.
* @param leftMask => left-side mask
* @param rightMask => right-side mask
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_addMaskedLinear
(
RG_Header       *pRG,
MTGNodeIdPair   *pChainEnds,
DPoint3dCP      pPointArray,
int             numPoint,
int             parentId,
MTGMask         leftMask,
MTGMask         rightMask
)
    {
    int i;
    int vertexId;
    MTGNodeId nodeId0 = MTG_NULL_NODEID, nodeId1 = MTG_NULL_NODEID;
    MTGNodeId  baseNodeId = MTG_NULL_NODEID;
    MTGNodeIdPair       endPair;
    bool    funcStat = false;
    endPair.nodeId[0] = endPair.nodeId[1] = MTG_NULL_NODEID;

    if  (numPoint <= 1)
        {
        funcStat = false;
        }
    else
        {
        /* baseNodeId is the beginning of the edge being added.
            For open chains, the end (fSucc of baseNodeId) dangles awaiting
                subsequent vertex index.
            For closed chains, the end is already the wrapped back to the beginning

            Edge id's for linear edges are always null.
        */
        for (i = 0; i < numPoint; i++ )
            {
            vertexId = jmdlEmbeddedDPoint3dArray_getCount (pRG->pVertexArray);
            jmdlEmbeddedDPoint3dArray_insertDPoint3d(pRG->pVertexArray, &pPointArray[i], vertexId);
            // At disconnect, reset baseNodeId to trigger new chain.
            if (pPointArray[i].IsDisconnect ())
                {
                baseNodeId = MTG_NULL_NODEID;
                }
            else if  (baseNodeId == MTG_NULL_NODEID)
                {
                if (   i < numPoint - 1
                    && !pPointArray[i+1].IsDisconnect ())
                    {
                    /* First edge.  Create the first edge and label its start vertex. */
                    jmdlMTGGraph_createEdge (pRG->pGraph, &nodeId0, &nodeId1);
                    endPair.nodeId[0] = nodeId0;
                    endPair.nodeId[1] = nodeId1;
                    jmdlRG_labelNode (pRG, nodeId0, vertexId, RG_NULL_CURVEID, parentId,
                            jmdlRG_constructPrimaryEdgeMask (true, leftMask));
                    baseNodeId = nodeId1;
                    }
                }
            else
                {
                /* Any subsequent edge.  Set the vertex id in the dangler .... */
                jmdlRG_labelNode (pRG, baseNodeId, vertexId, RG_NULL_CURVEID, parentId,
                        jmdlRG_constructPrimaryEdgeMask (false, rightMask));
                jmdlRG_completeEdgeEntry (pRG, nodeId0);
                funcStat = true;

                /* Except for the last edge, twist a new edge into the base and label its base */
                if  (i < numPoint - 1)
                    {
                    jmdlMTGGraph_createEdge  (pRG->pGraph, &nodeId0, &nodeId1  );
                    endPair.nodeId[1] = nodeId1;
                    jmdlMTGGraph_vertexTwist (pRG->pGraph, nodeId0,  baseNodeId);
                    jmdlRG_labelNode (pRG, nodeId0, vertexId, RG_NULL_CURVEID, parentId,
                        jmdlRG_constructPrimaryEdgeMask (true, leftMask));

                    baseNodeId = nodeId1;
                    }
                }
            }
        }

    if (pChainEnds)
        *pChainEnds = endPair;

    return funcStat;
    }

/*---------------------------------------------------------------------------------**//**
* @instance pRG <=> region context
* @param pChainEnds <= node ids at start, end of new chain
* @param curveId => curve id (preexisting) to reference from the edge.
* @param parentId => label to store with all edges.
* @param leftMask => left-side mask
* @param rightMask => right-side mask
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool            jmdlRG_addMaskedCurveDirect
(
RG_Header                       *pRG,
MTGNodeIdPair                   *pChainEnds,
RG_CurveId                      curveId,
RG_CurveId                      parentId,
MTGMask                 leftMask,
MTGMask                 rightMask
)
    {
    int vertex0Id, vertex1Id;
    MTGNodeId nodeId0, nodeId1;

    DPoint3d curvePoint[2];
    double   curveParam[2];
    int      nParam = 2;

    curveParam[0] = 0.0;
    curveParam[1] = 1.0;
    pRG->funcs.evaluateCurve (pRG->funcs.pContext, curvePoint, NULL, curveParam, nParam, curveId);

    vertex0Id = jmdlEmbeddedDPoint3dArray_getCount (pRG->pVertexArray);
    jmdlEmbeddedDPoint3dArray_insertDPoint3d(pRG->pVertexArray, &curvePoint[0], vertex0Id);

    vertex1Id = jmdlEmbeddedDPoint3dArray_getCount (pRG->pVertexArray);
    jmdlEmbeddedDPoint3dArray_insertDPoint3d(pRG->pVertexArray, &curvePoint[1], vertex1Id);

    jmdlMTGGraph_createEdge (pRG->pGraph, &nodeId0, &nodeId1);

    jmdlRG_labelNode (pRG, nodeId0, vertex0Id, curveId, parentId,
                            jmdlRG_constructPrimaryEdgeMask (true, leftMask));

    jmdlRG_labelNode (pRG, nodeId1, vertex1Id, curveId, parentId,
                            jmdlRG_constructPrimaryEdgeMask (false, rightMask));

    jmdlRG_completeEdgeEntry (pRG, nodeId0);

    if (pChainEnds)
        {
        pChainEnds->nodeId[0] = nodeId0;
        pChainEnds->nodeId[1] = nodeId1;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @instance pRG <=> region context
* @param pChainEnds <= node ids at start, end of new chain
* @param curveId => curve id (preexisting) to reference from the edge.
* @param parentId => label to store with all edges.
* @param leftMask => left-side mask
* @param rightMask => right-side mask
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
bool            jmdlRG_addMaskedCurve
(
RG_Header                       *pRG,
MTGNodeIdPair                   *pChainEnds,
RG_CurveId                      curveId,
RG_CurveId                      parentId,
MTGMask                 leftMask,
MTGMask                 rightMask
)
    {
    RG_CurveId primaryCurveId;
    if (!jmdlRIMSBS_isCurveChain (pRG->funcs.pContext, &primaryCurveId, curveId))
        return jmdlRG_addMaskedCurveDirect (pRG, pChainEnds, curveId, parentId, leftMask, rightMask);
    RIMSBS_CurveId childId;
    for (int i = 0; jmdlRIMSBS_getCurveChainChild (pRG->funcs.pContext, curveId, i, &childId); i++)
        {
        jmdlRG_addMaskedCurveDirect (pRG, pChainEnds, childId, parentId, leftMask, rightMask);
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @instance pRG => region header
* @param pPointArray => linestring coordinates
* @param numPoint => number of points.
* @param closed => if true, first and last node id's are joined topologically.
* @param parentId => label to store with all edges.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_addLinear
(
RG_Header     *pRG,
DPoint3dCP    pPointArray,
int           numPoint,
bool          closed,
int           parentId
)
    {
    MTGNodeIdPair endPair;
    bool    funcStat = jmdlRG_addMaskedLinear
                                (
                                pRG,
                                &endPair,
                                pPointArray,
                                numPoint,
                                parentId,
                                MTG_NULL_MASK,
                                MTG_NULL_MASK
                                );

    if (funcStat && closed)
        {
        jmdlMTGGraph_vertexTwist (pRG->pGraph, endPair.nodeId[0], endPair.nodeId[1]);
        }
    return funcStat;
    }

/*---------------------------------------------------------------------------------**//**
* Add a curve (defined by its curve id)
* @instance pRG => region header
* @param curveId => id of (preexisting) curve.
* @param closed => if true, first and last node id's are joined topologically.
* @param parentId => label to store with all edges.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public MTGNodeId       jmdlRG_addCurve
(
RG_Header                       *pRG,
RG_CurveId                      curveId,
RG_CurveId                      parentId
)
    {
    return jmdlRG_addMaskedCurve (pRG, NULL, curveId, parentId, MTG_NULL_MASK, MTG_NULL_MASK);
    }

/*---------------------------------------------------------------------------------**//**
* Prepare for input sequence using "current pair" context.
* Pairing logic aids construction of closed loops from curve sequences which
* are in order but have no prior indication of when there is a jump from one loop to the
* next.  During pairing, a "current pair" indicates the start and end nodes of the
* current evolving open path.   As new edges are added, comparison to the current pair
* coordinates bulid extended paths.
* @instance => region context.
* @param => pair to initialize.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_initPairing
(
RG_Header                       *pRG,
MTGNodeIdPair                   *pPair
)
    {
    pPair->nodeId[0] = pPair->nodeId[1] = MTG_NULL_NODEID;
    }

/*---------------------------------------------------------------------------------**//**
* Test if head of pair0 has same coordinates as tail of pair1; if so, join
* the pairs.
* @instance pRG => region context
* @param pMergedPair <= combined pair.
* @param pPair0 => old pair
* @param pPair1 => new edge or path to be joined.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_joinPairs
(
RG_Header                       *pRG,
MTGNodeIdPair                   *pMergedPair,
const   MTGNodeIdPair           *pPair0,
const   MTGNodeIdPair           *pPair1
)
    {
    MTGNodeId nodeId01 = pPair0->nodeId[1];
    MTGNodeId nodeId10 = pPair1->nodeId[0];
    bool    funcStat = false;

    if (   nodeId01 != nodeId10
        && nodeId01 != MTG_NULL_NODEID
        && nodeId10 != MTG_NULL_NODEID
        && jmdlRG_areVerticesCloseXY (pRG, nodeId10, nodeId01)
        )
        {
        jmdlMTGGraph_vertexTwist (pRG->pGraph, nodeId01, nodeId10);
        pMergedPair->nodeId[0] = pPair0->nodeId[0];
        pMergedPair->nodeId[1] = pPair0->nodeId[1];
        funcStat = true;
        }
    else if (  pPair0->nodeId[0] == MTG_NULL_NODEID
            && pPair0->nodeId[1] == MTG_NULL_NODEID
            )
        {
        *pMergedPair = *pPair1;
        funcStat = true;
        }
    else
        {
        jmdlRG_initPairing (pRG, pMergedPair);
        }

    return funcStat;
    }

/*---------------------------------------------------------------------------------**//**
* "Close" a pairing sequence by joining its ends.
* @instance pRG => region header.
* @param pPair => start / end description of completed path (to be closed if
*           start and end coordiantes match).
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_closePairing
(
RG_Header                       *pRG,
MTGNodeIdPair                   *pPair
)
    {
    MTGNodeIdPair tempPair;
    return jmdlRG_joinPairs (pRG, &tempPair, pPair, pPair);
    }


/*---------------------------------------------------------------------------------**//**
* Query the graph structure part of the region context.
* @instace pRG => region context.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public MTGGraph        *jmdlRG_getGraph
(
RG_Header       *pRG
)
    {
    if (!pRG)
        return NULL;
    return  pRG->pGraph;
    }

/*---------------------------------------------------------------------------------**//**
* Evaluate an edge, given as edge data, at a parameter.
* @instance pRG => region context
* @param pPoint <= evaluated point
* @param pEdgeData => edge summary
* @param param => evluation parameter.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool        jmdlRG_evaluateEdgeData
(
RG_Header           *pRG,
DPoint3d            *pPoint,
RG_EdgeData         *pEdgeData,
double              param
)
    {
    bool    myStat = false;
    if (!pEdgeData)
        return false;

    if  (param <= 0.0)
        {
        *pPoint         = pEdgeData->xyz[0];
        myStat = true;
        }
    else if (param >= 1.0)
        {
        *pPoint         = pEdgeData->xyz[1];
        myStat = true;
        }
    else
        {
        if  (pEdgeData->curveIndex == RG_NULL_CURVEID)
            {
            pPoint->Interpolate (*(&pEdgeData->xyz[0]), param, *(&pEdgeData->xyz[1]));
            myStat = true;
            }
        else
            {
            myStat = jmdlRG_linkToEvaluateCurve
                    (
                    pRG,
                    pEdgeData,
                    pPoint,
                    NULL,
                    &param,
                    1
                    );
            }
        }
    return  myStat;
    }

/*---------------------------------------------------------------------------------**//**
* Evaluate length of edge as approximated by chords.  Intended for use in detecting
*    zero-length edges, hence speed more important than accuracy.
* @instance pRG => region context
* @param pEdgeData => edge summary
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public double     jmdlRG_quickChordLength
(
RG_Header           *pRG,
MTGNodeId           nodeId,
int                 numChord
)
    {
    double chordLength = 0.0;
    RG_EdgeData edgeData;
    double param;

    if (jmdlRG_getEdgeData (pRG, &edgeData, nodeId))
        {

        if  (edgeData.curveIndex == RG_NULL_CURVEID)
            {
            chordLength = edgeData.xyz[0].Distance (*(&edgeData.xyz[1]));
            }
        else
            {
            double ds = 1.0 / numChord;
            DPoint3d point0, point1;
            int i;
            point0.Zero ();
            for (i = 0; i <= numChord; i++, point0 = point1)
                {
                param = i * ds;
                if (!jmdlRG_linkToEvaluateCurve
                    (
                    pRG,
                    &edgeData,
                    &point1,
                    NULL,
                    &param,
                    1
                    ))
                    return 0.0;
                if (i > 0)
                    chordLength += point0.Distance (point1);

                }
            }
        }
    return chordLength;
    }

/*---------------------------------------------------------------------------------**//**
* @instance pRG <=> region context
* @param pPoint <= point coordinates.
* @param vertexIndex <= vertex index; may be a preexisting end vertex or a newly
*           created interior index.
* @param pEdgeData => edge summary.
* @param s => parameer where vertex is to be placed.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool        jmdlRG_findOrCreateVertexFromEdgeData
(
RG_Header           *pRG,
        DPoint3d    *pPoint,
int                 *pVertexIndex,
const   RG_EdgeData *pEdgeData,
        double      s
)
    {
    bool    myStat = false;
    if  (s <= 0.0)
        {
        *pVertexIndex   = pEdgeData->vertexIndex[0];
        *pPoint         = pEdgeData->xyz[0];
        myStat = true;
        }
    else if (s >= 1.0)
        {
        *pVertexIndex   = pEdgeData->vertexIndex[1];
        *pPoint         = pEdgeData->xyz[1];
        myStat = true;
        }
    else
        {
        if  (pEdgeData->curveIndex == RG_NULL_CURVEID)
            {
            pPoint->Interpolate (*(&pEdgeData->xyz[0]), s, *(&pEdgeData->xyz[1]));
            }
        else
            {
            jmdlRG_linkToEvaluateCurve
                    (
                    pRG,
                    pEdgeData,
                    pPoint,
                    NULL,
                    &s,
                    1
                    );
            }

        *pVertexIndex = jmdlEmbeddedDPoint3dArray_getCount (pRG->pVertexArray);
        jmdlEmbeddedDPoint3dArray_insertDPoint3d(pRG->pVertexArray, pPoint, *pVertexIndex);
        myStat = true;
        }
    return  myStat;
    }


/*---------------------------------------------------------------------------------**//**
* Set the vertex label on a given node.
* @instance pRG <=> region context
* @param nodeId => existing node id.
* @param vertexIndex => index to store. Not tested for validity in vertex table.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRG_setVertexIndex
(
RG_Header           *pRG,
MTGNodeId           nodeId,
int                 vertexIndex
)
    {
    jmdlMTGGraph_setLabel
            (
            pRG->pGraph,
            nodeId,
            pRG->vertexLabelIndex,
            vertexIndex
            );

    }

/*---------------------------------------------------------------------------------**//**
* Set the curve label on a given node.
* @instance pRG <=> region context
* @param nodeId => existing node id
* @param curveIndex => curve index to store.  Not tested for validity in curve manager.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRG_setCurveIndex
(
RG_Header           *pRG,
MTGNodeId           nodeId,
int                 curveIndex
)
    {
    jmdlMTGGraph_setLabel
            (
            pRG->pGraph,
            nodeId,
            pRG->edgeLabelIndex,
            curveIndex
            );
    }

/*---------------------------------------------------------------------------------**//**
* @description Get the curve label from a node.
* @param pRG => region context.
* @param pCurveIndex <= returned curve index.
* @param nodeId => base node id of edge.
* @return true if valid node id.
* @bsimethod                                                    David.Assaf     08/07
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRG_getCurveIndex
(
RG_Header           *pRG,
int                 *pCurveIndex,
MTGNodeId           nodeId
)
    {
    return jmdlMTGGraph_getLabel (pRG->pGraph, pCurveIndex, nodeId, pRG->edgeLabelIndex);
    }

/*---------------------------------------------------------------------------------**//**
* Set the parent curve label on a given node.
* @instance pRG <=> region context
* @param nodeId => existing node id
* @param curveIndex => parent curve index to store.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRG_setParentCurveIndex
(
RG_Header           *pRG,
MTGNodeId           nodeId,
int                 curveIndex
)
    {
    jmdlMTGGraph_setLabel
            (
            pRG->pGraph,
            nodeId,
            pRG->parentLabelIndex,
            curveIndex
            );
    }

/*---------------------------------------------------------------------------------**//**
* Get the parent curve index from a node.
* @instance pRG => region context.
* @param pParentIndex <= returned parent index.
* @param nodeId => base node id of edge.
* @return true if valid node id.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRG_getParentCurveIndex
(
RG_Header           *pRG,
int                 *pParentIndex,
MTGNodeId           nodeId
)
    {
    return jmdlMTGGraph_getLabel
            (
            pRG->pGraph,
            pParentIndex,
            nodeId,
            pRG->parentLabelIndex
            );
    }


/*---------------------------------------------------------------------------------**//**
* Set the curve label on a given node.
* @instance pRG <=> region header
* @param nodeId => node id to set
* @param value => true to indicate this node is the start (rather than end)
*                   of the (directed) edge.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRG_setEdgeStartBit
(
RG_Header           *pRG,
MTGNodeId           nodeId,
bool                value
)
    {
    jmdlMTGGraph_writeMask (pRG->pGraph, nodeId, MTG_DIRECTED_EDGE_MASK, value);
    }



/*---------------------------------------------------------------------------------**//**
* Set masks, vertex label, and edge label at a ndoe.
* @instance     pRG         => region context
* @param        nodeId      => node to label
* @param        vertexId    => vertex id to attach
* @param        curveId     => curve id to attach
* @param        isBase      => true if this node is the base of the edge.
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRG_labelNode
(
RG_Header       *pRG,
MTGNodeId      nodeId,
int             vertexId,
int             curveId,
int             parentId,
MTGMask mask
)
    {
    jmdlMTGGraph_setLabel (pRG->pGraph, nodeId, pRG->vertexLabelIndex, vertexId);
    jmdlMTGGraph_setLabel (pRG->pGraph, nodeId, pRG->edgeLabelIndex, curveId);
    jmdlMTGGraph_setLabel (pRG->pGraph, nodeId, pRG->parentLabelIndex, parentId);
    jmdlMTGGraph_setMask (pRG->pGraph, nodeId, mask);
    }

/*---------------------------------------------------------------------------------**//**
* Return the combined mask for a primary edge with optional direction indicator and user mask
* @param isBase => true if the node being labeld is the start (rather than end)
*               of the edge.
* @param userMask => additional mask.
* @return composite mask.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public MTGMask  jmdlRG_constructPrimaryEdgeMask
(
bool        isBase,
MTGMask    userMask
)
    {
    MTGMask mask = MTG_PRIMARY_EDGE_MASK | userMask;
    if (isBase)
        mask |= MTG_DIRECTED_EDGE_MASK;
    return mask;
    }

/*---------------------------------------------------------------------------------**//**
* Like crossing an edge to its mate, but also jump over null faces
*   on the mate side.
* Strictly: Cross to edge mate.  Search that vertex loop (starting at the edge mate)
*       for an edge not marked a null face.  Since null faces always have exactly two edges, each
*       step around this vertex steps over another null face. If no non-null is found,
*       give up and return original edge mate.
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public MTGNodeId    jmdlRG_skipNullFacesToEdgeMate
(
RG_Header           *pRG,
MTGNodeId           nodeId
)
    {
    MTGNodeId mateNodeId = jmdlMTGGraph_getEdgeMate (pRG->pGraph, nodeId);
    MTGARRAY_VERTEX_LOOP (currNodeId, pRG->pGraph, mateNodeId)
        {
        if (!jmdlMTGGraph_getMask (pRG->pGraph, currNodeId, RG_MTGMASK_NULL_FACE))
            return currNodeId;
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, pRG->pGraph, mateNodeId)
    /* Can only get here if entire vertex loop has null face label.
       This shouldn't happen geometrically.
    */
    return mateNodeId;
    }


/*---------------------------------------------------------------------------------**//**
* Evaluate the coordinates and vertex id for a node.  Optionally use a specified parametric
*   offset "away from" the node as evaluation point.
* @instance pRG => region header.
* @param pX <= evaluated coordinates and derivatives
* @param numDerivative => number of deriviates to evaluate. (0 fo rjust the point)
* @param pVertexId => vertex id at the node.
* @param offset => paramter space offset towards middle of edge.
* @return true if all data accessible
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_getVertexData
(
RG_Header           *pRG,
DPoint3d            *pX,
int                 numDerivative,
int                 *pVertexId,
MTGNodeId           nodeId,
double              offset
)
    {
    int vertexId;
    int i;
    bool    result = false;


    if  (jmdlMTGGraph_getLabel (pRG->pGraph, &vertexId, nodeId, pRG->vertexLabelIndex))
        {
        result = true;
        if  (pVertexId)
            *pVertexId = vertexId;

        if (pX)
            {
            /* VertexId is supposed to be good ... */
            if  (!jmdlEmbeddedDPoint3dArray_getDPoint3d (pRG->pVertexArray, pX, vertexId))
                result = false;
            if (numDerivative > 0)
                {
                RG_EdgeData edgeData;
                if (!jmdlRG_getEdgeData (pRG, &edgeData, nodeId))
                    {
                    result = false;
                    }
                else
                    {
                    if  (edgeData.curveIndex == RG_NULL_CURVEID)
                        {
                        pX[1].DifferenceOf (*(&edgeData.xyz[1]), *(&edgeData.xyz[0]));
                        for (i = 2; i <= numDerivative ; i++)
                            pX[i].Zero ();
                        }
                    else
                        {
                        double param = edgeData.isReversed ? 1.0 - offset : offset;
                        jmdlRG_linkToEvaluateDerivatives
                                (
                                pRG,
                                &edgeData,
                                pX,
                                numDerivative,
                                param
                                );

                        if (edgeData.isReversed)
                            {
                            for (i = 1; i <= numDerivative; i += 2)
                                {
                                pX[i].Negate();
                                }
                            }
#define TANGENT_CHECKS_not
#ifdef TANGENT_CHECKS
                        if (numDerivative >= 1)
                            {
                            double checkParam[2];
                            DPoint3d checkPoint[2];
                            DPoint3d checkTangent;
                            double checkMagnitude;
                            double tangentMagnitude;
                            double checkDot;
                            double dist;
                            checkParam[0] = edgeData.isReversed ? 1.0 : 0.0;
                            checkParam[1] = checkParam[0]
                                            + 0.001 * (0.5 - checkParam[0]);
                            jmdlRG_linkToEvaluateCurve
                                    (
                                    pRG,
                                    &edgeData,
                                    checkPoint,
                                    NULL,
                                    checkParam,
                                    2
                                    );
                            checkTangent.DifferenceOf (checkPoint[1], checkPoint[0]);
                            checkTangent.Scale (checkTangent, 1.0 / fabs(checkParam[1] - checkParam[0]));
                            checkMagnitude = sqrt (checkTangent.DotProductXY (checkTangent));
                            tangentMagnitude = sqrt (pX[1].DotProduct (pX[1]));
                            checkDot = checkTangent.DotProduct (pX[1]);
                            if (checkDot < 1.0001 * checkMagnitude * tangentMagnitude)
                                checkDot = checkDot / (checkMagnitude * tangentMagnitude);
                            else
                                checkDot = 2.0;

                            if (fabs (checkDot - 1.0) > 0.001)
                                {
                                GEOMAPI_PRINTF("\n Tangent comparison dot product == %lf  edge dir = %d", checkDot, edgeData.isReversed);
                                }
                            if (checkDot < 0.0)
                                pX[1].Negate();
                            dist = pX[1].Distance (checkTangent);

                            if (dist >= 0.01 * tangentMagnitude && tangentMagnitude != 0.0)
                                GEOMAPI_PRINTF ("\n    ****   Tangent error / tangent magnitude = %lf / %lf \n",
                                                dist, tangentMagnitude);
                            }

#endif
                        }
                    }
                }
            }
        }
    return  result;
    }

/*---------------------------------------------------------------------------------**//**
* Get vertex coordinates.
* @instance pRG => region header.
* @param pX <= evaluated coordinates and derivatives
* @param nodeId => node id to query
* @return true if all data accessible
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_getVertexDPoint3d
(
RG_Header           *pRG,
DPoint3d            *pX,
MTGNodeId           nodeId
)
    {
    return jmdlRG_getVertexData (pRG, pX, 0, NULL, nodeId, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @return true if the given nodes are at vertices that are within the region context's
*   proximity tolerance.
* @instance pRG => region context.
* @param nodeId0 => first node id to test.
* @param nodeId1 => second node id to test.
* @return true if all data accessible and vetices close together.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_areVerticesCloseXY
(
RG_Header           *pRG,
MTGNodeId           nodeId0,
MTGNodeId           nodeId1
)
    {
    bool    result = false;
    DPoint3d point0, point1;

    if (   jmdlRG_getVertexData (pRG, &point0, 0, NULL, nodeId0, 0.0)
        && jmdlRG_getVertexData (pRG, &point1, 0, NULL, nodeId1, 0.0)
        )
        {
        double d2 = point0.DistanceSquaredXY (point1);

        if (d2 <= pRG->tolerance * pRG->tolerance)
            result = true;
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* Move vertex coordinates.  Warning: No update of auxiliary structures
* (e.g. range tree, curve data) which may rely on the vertex coordinates.
* vertex index is absolute -- negative 1 is NOT a reference to final vertex.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_moveVertex
(
RG_Header           *pRG,
int                 vertexId,
DPoint3d            *pXYZ
)
    {
    int numVertex = jmdlEmbeddedDPoint3dArray_getCount (pRG->pVertexArray);
    if (vertexId >= 0 && vertexId < numVertex)
        {
        jmdlEmbeddedDPoint3dArray_setDPoint3d (pRG->pVertexArray, pXYZ, vertexId);
        return true;
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* Look up edge id and both vertex coordinates and ids for a given node.
* @instance pRG => region context.
* @param pEdgeData <= edge coordinate and curve id data.
* @param node0Id => node id whose edge is being inspected.
* @return true if edge data accessible.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_getEdgeData
(
RG_Header           *pRG,
RG_EdgeData         *pEdgeData,
MTGNodeId           node0Id
)
    {
    bool    result = false;
    MTGNodeId node1Id = jmdlMTGGraph_getEdgeMate (pRG->pGraph, node0Id);
    pEdgeData->nodeId[0] = node0Id;
    pEdgeData->nodeId[1] = node1Id;


    result =
            jmdlMTGGraph_getLabel
                            (
                            pRG->pGraph,
                            &pEdgeData->vertexIndex[0],
                            node0Id,
                            pRG->vertexLabelIndex
                            )

         && jmdlMTGGraph_getLabel
                            (
                            pRG->pGraph,
                            &pEdgeData->vertexIndex[1],
                            node1Id,
                            pRG->vertexLabelIndex
                            )

         && jmdlMTGGraph_getLabel
                            (
                            pRG->pGraph,
                            &pEdgeData->curveIndex,
                            node0Id,
                            pRG->edgeLabelIndex
                            )

         && jmdlMTGGraph_getLabel
                            (
                            pRG->pGraph,
                            &pEdgeData->auxIndex,
                            node0Id,
                            pRG->parentLabelIndex
                            )

         && jmdlEmbeddedDPoint3dArray_getDPoint3d
                            (
                            pRG->pVertexArray,
                            &pEdgeData->xyz[0],
                            pEdgeData->vertexIndex[0]
                            )

         && jmdlEmbeddedDPoint3dArray_getDPoint3d
                            (
                            pRG->pVertexArray,
                            &pEdgeData->xyz[1],
                            pEdgeData->vertexIndex[1]
                            );

    if (result)
        pEdgeData->isReversed =
            jmdlMTGGraph_getMask (pRG->pGraph, node0Id, MTG_DIRECTED_EDGE_MASK)
                        ? 0 : 1;

    return  result;
    }

/*---------------------------------------------------------------------------------**//**
* Look up edge id and both vertex coordinates and ids for a given node.
* @instance pRG => region context.
* @param pCurveIndex <= geometric curve index.
* @param pIsReversed <= true if this node (aka half edge, coedge, directed edge) is oriented
*           in the opposite direction of the geometric curve's parameterization
* @param pStartPoint <= start point of edge.
* @param pEndPoint <= end point of edge
* @param node0Id => node id on (half, co, directed) edge.
* @return true if edge data accessible.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_getCurveData
(
RG_Header           *pRG,
int                 *pCurveIndex,
bool                *pIsReversed,
DPoint3d            *pStartPoint,
DPoint3d            *pEndPoint,
MTGNodeId           node0Id
)
    {
    return jmdlRG_getAuxCurveData
                (
                pRG,
                pCurveIndex,
                NULL,
                pIsReversed,
                pStartPoint,
                pEndPoint,
                node0Id
                );
    }

/*---------------------------------------------------------------------------------**//**
* Look up the parent curve id for a given node, and then the group id from the curve.
* @return true if groupId is defined.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_getGroupId
(
RG_Header           *pRG,
int                 *pGroupId,
MTGNodeId           nodeId
)
    {
    int parentCurveId;
    bool    result = jmdlRG_getParentCurveIndex (pRG, &parentCurveId, nodeId);

    if (result)
        {
        RGC_GetGroupIdFunction F = pRG->funcs.getGroupId;
        if  (F)
            result = F (pRG->funcs.pContext, pGroupId, parentCurveId);
        }
    return  result;
    }

/*---------------------------------------------------------------------------------**//**
* Look up edge id and both vertex coordinates and ids for a given node.
* @instance pRG => region context.
* @param pCurveIndex    <= curve index in geometry context.
* @param pAuxIndex      <= auxiliary curve index
* @param pIsReversed    <= indicates if moving in opposite direction from original curve
* @param pStartPoint    <= start coordinates
* @param pEndPoint      <= end coordinates
* @param node0Id        => node to look up.
* @return true if edge data is accessible.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_getAuxCurveData
(
RG_Header           *pRG,
int                 *pCurveIndex,
int                 *pAuxIndex,
bool                *pIsReversed,
DPoint3d            *pStartPoint,
DPoint3d            *pEndPoint,
MTGNodeId           node0Id
)
    {

    RG_EdgeData edgeData;
    bool    result = jmdlRG_getEdgeData (pRG, &edgeData, node0Id);

    if (result)
        {
        if (pAuxIndex)
            *pAuxIndex   = edgeData.auxIndex;
        if (pCurveIndex)
            *pCurveIndex = edgeData.curveIndex;
        if (pIsReversed)
            *pIsReversed = edgeData.isReversed;
        if (pStartPoint)
            *pStartPoint = edgeData.xyz[0];
        if (pEndPoint)
            *pEndPoint   = edgeData.xyz[1];
        }
    return  result;
    }

/*---------------------------------------------------------------------------------**//**
* Compute the xy intersections of an edge with an xy circle.
*
* Returned parameter always starts at 0 at the given base node --- i.e. the fractional
*   paramerization is on this side of the edge.
* @instance pRG => region context.
* @param pParameterArray <= array of fractional parameters.
* @param pPointArray <= array of intersection points, taken from the curve to obtain a z.
* @param nodeId => base node id of (half) edge.
* @param pCenter => center of circle.
* @param radius => circle radius
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool        jmdlRG_edgeCircleXYIntersection
(
RG_Header           *pRG,
bvector<double>     *pParameterArray,
EmbeddedDPoint3dArray *pPointArray,
int                 nodeId,
DPoint3d            *pCenter,
double              radius
)
    {
    RG_EdgeData edgeData;
    bool    result = jmdlRG_getEdgeData (pRG, &edgeData, nodeId);
    if (result)
        {
        if  (edgeData.curveIndex == RG_NULL_CURVEID)
            {
            jmdlRG_intersectCircleSegmentXY (pParameterArray, pPointArray,
                                &edgeData.xyz[0], &edgeData.xyz[1], pCenter, radius, true);
            result = true;
            }
        else
            {
            result = jmdlRG_linkToCurveCircleXYIntersection
                        (
                        pRG,
                        pParameterArray,
                        pPointArray,
                        &edgeData,
                        pCenter,
                        radius
                        );
            if (edgeData.isReversed)
                {
                for (double &param : *pParameterArray)
                    param = 1.0 - param;
                }
            }
        }
    return  result;
    }




/*---------------------------------------------------------------------------------**//**
* @instance pRG => region context.
* @param nodeId => either of the two nodes on the edge to drop.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_dropEdge
(
RG_Header           *pRG,
MTGNodeId           nodeId
)
    {
    jmdlMTGGraph_dropEdge (pRG->pGraph, nodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @instance pRG => region context.
* @return proximity tolerance for the graph.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public double          jmdlRG_getTolerance
(
RG_Header           *pRG
)
    {
    return  pRG->tolerance;
    }

static double s_maxRelTol = 1.0e-7;
/*---------------------------------------------------------------------------------**//**
* Update the session's maxRelTol, this is used by jmdlRG_updateTolerance to compute the 
* maxTol (using maxTol = s_maxRelTol * maxCoordinate).
* @bsimethod                                                    Wouter.Rombouts 05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Public double          jmdlRG_setMaxRelTol
(
double newMaxRelTol
)
    {
    double oldMaxRelTol = s_maxRelTol;
    s_maxRelTol = newMaxRelTol;
    return oldMaxRelTol;
    }

/*---------------------------------------------------------------------------------**//**
* Update the graph tolerance, based on the (previously recorded) minimum tolerance and the
* relative tolerance times the data range (as recorded by the header --- not recomputed)
* @return true if a global tolerance could be determined.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_updateTolerance
(
RG_Header           *pRG
)
    {
    double maxCoordinate = 0.0;
    double maxTol;

    if (!pRG->graphRange.IsNull ())
        maxCoordinate = pRG->graphRange.LargestCoordinate ();

    maxTol = s_maxRelTol * maxCoordinate;
    pRG->tolerance = pRG->relTol * maxCoordinate + pRG->minimumTolerance;
    if (pRG->tolerance > maxTol)
        pRG->tolerance = maxTol;
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Collect the coordinates of all vertices around the given face.
* @instance pRG => region context.
* @param pPointArrayHdr <=> rubber array header to receive points.
* @param startId => any node id on the face.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public int jmdlRG_collectVerticesAroundFace
(
RG_Header                   *pRG,
EmbeddedDPoint3dArray               *pPointArrayHdr,
MTGNodeId                   startId
)
    {
    int numVertex = 0;

    DPoint3d point;
    RG_EdgeData edgeData;
    int i;
    static int s_numCurvePoint = 5;
    double s;
    double ds;
    double s0;

    jmdlEmbeddedDPoint3dArray_empty (pPointArrayHdr);

    MTGARRAY_FACE_LOOP (currNodeId, pRG->pGraph, startId)
        {

        if  (jmdlRG_getEdgeData (pRG, &edgeData, currNodeId))
            {
            if  (edgeData.curveIndex == RG_NULL_CURVEID)
                {
                jmdlEmbeddedDPoint3dArray_addDPoint3d(pPointArrayHdr, &edgeData.xyz[0]);
                numVertex++;
                }
            else
                {
                ds = (edgeData.isReversed ? -1.0 : 1.0) / (double)s_numCurvePoint;
                s0 = edgeData.isReversed ? 1.0 : 0.0;
                for (i = 0; i < s_numCurvePoint ; i++)
                    {
                    s = s0 + i * ds;
                    jmdlRG_linkToEvaluateCurve (pRG, &edgeData, &point, NULL, &s, 1);
                    jmdlEmbeddedDPoint3dArray_addDPoint3d(pPointArrayHdr, &point);
                    numVertex++;
                    }
                }
            }
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pRG->pGraph, startId)
    return  numVertex;
    }
/*---------------------------------------------------------------------------------**//**
* Pass point array for vertex coordinates around each face to a callback.
* This passes ONLY the vertices --- no subdividsion of curves.
* @instance pRG => region context
* @param F => function to call, args F(pArg0, pArg1, pUserData, pPointBuffer, numVertex startNodeId)
*               (Note: this is enough args to allow F=dlmSystem_callMdlFunction,
*               pArg0=mdl descriptor, pArg1=mdl function address, pUserData=user data in mdl app.
* @param pArg0 => caller's context.
* @param pArg1 => caller's context.
* @param pUserData => caller's context.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_emitFaceVertexLoops
(
RG_Header                       *pRG,
RG_FaceLoopFunction             F,
void                            *pArg0,
void                            *pArg1,
void *                          pUserData
)
    {
    int iFace;
    MTGNodeId startNodeId;
    int numVertex;
    DPoint3d *pPointBuffer;

    EmbeddedIntArray *pFaceStartArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedDPoint3dArray *pCoordinateHeader = jmdlEmbeddedDPoint3dArray_grab ();
    pRG->pGraph->CollectFaceLoops (*pFaceStartArray);

    for (iFace = 0; jmdlEmbeddedIntArray_getInt (pFaceStartArray, &startNodeId, iFace); iFace++)
        {
        numVertex = jmdlRG_collectVerticesAroundFace (pRG, pCoordinateHeader, startNodeId);
        pPointBuffer = jmdlEmbeddedDPoint3dArray_getPtr (pCoordinateHeader, 0);
        if (numVertex > 0)
            F (pArg0, pArg1, pUserData, pPointBuffer, numVertex, startNodeId);
        }
    jmdlEmbeddedIntArray_drop (pFaceStartArray);
    jmdlEmbeddedDPoint3dArray_drop (pCoordinateHeader);
    }

/*---------------------------------------------------------------------------------**//**
* call F(pArg0, pArg1, pUserData, &range, nodeId) with the range of each edge.
* @pRG => region context
* @param F => function to invoke per edge.
* @param pArg0 => caller's context.
* @param pArg1 => caller's context.
* @param pUserData => caller's context.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_emitEdgeRanges
(
RG_Header                       *pRG,
RG_RangeFunction                F,
void                            *pArg0,
void                            *pArg1,
void *                          pUserData
)
    {
    RG_EdgeData edgeData;
    DRange3d range;
    MTGARRAY_SET_LOOP (currNode, pRG->pGraph)
        {
        if (   jmdlRG_getEdgeData (pRG, &edgeData, currNode)
            && !edgeData.isReversed
            && jmdlRG_getEdgeRange (pRG, &range, currNode)
            )
            {
            F (pArg0, pArg1, pUserData, &range, currNode);
            }
        }
    MTGARRAY_END_SET_LOOP (currNode, pRG->pGraph)
    }


typedef struct
    {
    RG_Header *pRG;
    DPoint3d  testPoint;
    EmbeddedDPoint3dArray *pCoordinateArrayHeader;

    RG_FaceLoopFunction loopFunc;
    void        *pLoopArg0;
    void        *pLoopArg1;
    void        *pUserData;

    RG_NodeFunction     nodeFunc;
    void        *pNodeArg0;
    void        *pNodeArg1;
    void        *pUserData0;
    void        *pUserData1;
    int         userInt;
    } RG_FaceRangeParams;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRG_cb_filterPointInFace
(
HideTreeRange   *pRange,
int             nodeId,
RG_FaceRangeParams *pParams,
void            *pParent,
void            *pLeaf
)
    {
    DPoint3d *pBuffer;
    int n;
    static double angleTol = 1.0e-3;
    static int s_pmcByAngle = 0;

    if (s_pmcByAngle)
        {
        double area, angle;
        if (   jmdlRG_getFaceSweepProperties (pParams->pRG, &area, &angle, &pParams->testPoint, nodeId)
            && angle > angleTol
            )
            {
            if (pParams->loopFunc)
                {
                jmdlRG_collectVerticesAroundFace
                            (
                            pParams->pRG,
                            pParams->pCoordinateArrayHeader,
                            nodeId
                            );

                pBuffer = jmdlEmbeddedDPoint3dArray_getPtr (pParams->pCoordinateArrayHeader, 0);
                n           = jmdlEmbeddedDPoint3dArray_getCount (pParams->pCoordinateArrayHeader);

                pParams->loopFunc (pParams->pLoopArg0, pParams->pLoopArg1, pParams->pUserData, pBuffer, n, nodeId);
                }

            if (pParams->nodeFunc)
                    pParams->nodeFunc (pParams->pNodeArg0, pParams->pNodeArg1,
                                pParams->pUserData0,
                                pParams->pRG, pParams->pRG->pGraph,
                                nodeId, nodeId,
                                pParams->pUserData1, pParams->userInt,
                                NULL, NULL);
            }
        }
    else
        {
        MTGNodeId minNodeId;
        double     minParam, minDist;
        DPoint3d    minPoint;
        bool    ccw;

        if (   jmdlRG_getClosestPointAroundFace (pParams->pRG,
                        &minNodeId, &minParam, &minDist, &minPoint, &ccw,
                        &pParams->testPoint, nodeId)
             && ccw
             /*
             && jmdlRG_getFaceSweepProperties (pParams->pRG, &area,NULL, &pParams->testPoint, nodeId)
             && angle > angleTol
             */
             )
            {
            if (pParams->loopFunc)
                {
                jmdlRG_collectVerticesAroundFace
                            (
                            pParams->pRG,
                            pParams->pCoordinateArrayHeader,
                            nodeId
                            );

                pBuffer = jmdlEmbeddedDPoint3dArray_getPtr (pParams->pCoordinateArrayHeader, 0);
                n           = jmdlEmbeddedDPoint3dArray_getCount (pParams->pCoordinateArrayHeader);

                pParams->loopFunc (pParams->pLoopArg0, pParams->pLoopArg1, pParams->pUserData, pBuffer, n, nodeId);
                }

            if (pParams->nodeFunc)
                    pParams->nodeFunc
                                (
                                pParams->pNodeArg0, pParams->pNodeArg1,
                                pParams->pUserData0,
                                pParams->pRG, pParams->pRG->pGraph,
                                nodeId, minNodeId,
                                pParams->pUserData1, pParams->userInt,
                                &pParams->testPoint, &minPoint
                                );
            }
        }

    return  true;
    }


/*---------------------------------------------------------------------------------**//**
* Issue callbacks for each containing face; each callback passes an array of coordinates around
*   the face.
*
* Function call is
*<pre>
*       F(pArg0, pArg1, pUserData, pPointArray, numPoint, nodeId);
*</pre>
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_emitFaceLoopsByFaceRangeSearch
(
RG_Header                       *pRG,
DPoint3d                        *pPoint,
RG_FaceLoopFunction             F,
void                            *pArg0,
void                            *pArg1,
void *                          pUserData
)
    {
    RG_FaceRangeParams params;
    HideTreeRange treeRange;

    EmbeddedDPoint3dArray *pCoordinateHeader = jmdlEmbeddedDPoint3dArray_grab ();

    rgXYRangeTree_initRange (&treeRange, pPoint->x, pPoint->y, pPoint->x, pPoint->y);
    params.pRG                          = pRG;
    params.testPoint                    = *pPoint;
    params.pCoordinateArrayHeader       = pCoordinateHeader;
    params.pLoopArg0                    = pArg0;
    params.pLoopArg1                    = pArg1;
    params.pUserData                    = pUserData;
    params.loopFunc                     = F;

    params.nodeFunc                     = NULL;

    rgXYRangeTree_traverseTree (
                    pRG->pFaceRangeTree,
                    (HideTree_NodeFunction)rgXYRangeTree_nodeFunction_testForRangeOverlap,
                    &treeRange,
                    (HideTree_ProcessLeafFunction)jmdlRG_cb_filterPointInFace,
                    &params);

    jmdlEmbeddedDPoint3dArray_drop (pCoordinateHeader);
    }

/*---------------------------------------------------------------------------------**//**
* Issue callback for each face surrounding the pick point.   Each callback indicates one node
* of the face.
* Callback args are
*<pre>
*   F
*       (
*       void    *pArg0,             => user context (e.g. mdl descriptor)
*       void    *pArg1,             => user context (e.g. mdl function address)
*       void    *pUserData0,        => user context.
*       RG_Header   *pRG,           => region graph.
*       MTGGraph   *pGraph,         => connectivity graph for traversal.
*       MTGNodeId  nodeId,          => seed node for the face loop.
*       MTGNodeId  nearestNodeId,   => node at base of nearest edge.
*       void        *pUserData1,    => user data.
*       int         userInt,        => user data.
*       DPoint3d    *pPoint0,       => The test point.
*       DPoint3d    *pPoint1        => Nearest point.
*       );
*</pre>
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_emitFaceNodesByFaceRangeSearch
(
RG_Header                       *pRG,
DPoint3d                        *pPoint,
RG_NodeFunction                 F,
void                            *pArg0,
void                            *pArg1,
void *                          pUserData0,
void *                          pUserData1,
int                             userInt
)
    {
    RG_FaceRangeParams params;
    HideTreeRange treeRange;

    EmbeddedDPoint3dArray *pCoordinateHeader = jmdlEmbeddedDPoint3dArray_grab ();

    rgXYRangeTree_initRange (&treeRange, pPoint->x, pPoint->y, pPoint->x, pPoint->y);
    params.pRG                          = pRG;
    params.testPoint                    = *pPoint;
    params.pCoordinateArrayHeader       = pCoordinateHeader;
    params.loopFunc                     = NULL;

    params.pNodeArg0                    = pArg0;
    params.pNodeArg1                    = pArg1;
    params.pUserData0                   = pUserData0;
    params.nodeFunc                     = F;
    params.pUserData1                   = pUserData1;
    params.userInt                      = userInt;

    rgXYRangeTree_traverseTree (
                    pRG->pFaceRangeTree,
                    (HideTree_NodeFunction)rgXYRangeTree_nodeFunction_testForRangeOverlap,
                    &treeRange,
                    (HideTree_ProcessLeafFunction)jmdlRG_cb_filterPointInFace,
                    &params);

    jmdlEmbeddedDPoint3dArray_drop (pCoordinateHeader);
    }


typedef struct
    {
    RG_Header   *pRG;
    MTGNodeId   nodeId;
    MTGNodeId   minNodeId;
    double      minDist;
    DPoint3d    testPoint;
    EmbeddedIntArray  *pNodeIdToComponentArray;
    int         excludedComponent;
    } RG_SmallestFaceParams;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRG_cb_filterSmallestContainingFace
(
HideTreeRange   *pRange,
int             nodeId,
RG_SmallestFaceParams *pParams,
void            *pParent,
void            *pLeaf
)
    {

    MTGNodeId minNodeId;
    double     minParam, minDist;
    DPoint3d    minPoint;
    bool    ccw;
    int         component;

    /* Negative area face cannot contain anything. */
    if (jmdlMTGGraph_getMask (pParams->pRG->pGraph, nodeId, RG_MTGMASK_IS_NEGATIVE_AREA_FACE))
        return true;

    if (pParams->pNodeIdToComponentArray
        && jmdlEmbeddedIntArray_getInt
                            (pParams->pNodeIdToComponentArray, &component, nodeId)
        && component == pParams->excludedComponent
        )
        return true;

    if (    jmdlRG_getClosestPointAroundFace (pParams->pRG,
                    &minNodeId, &minParam, &minDist, &minPoint, &ccw,
                    &pParams->testPoint, nodeId)
         && ccw
         )
        {
        if (pParams->nodeId == MTG_NULL_NODEID || minDist < pParams->minDist)
            {
            pParams->minDist = minDist;
            pParams->nodeId = nodeId;
            pParams->minNodeId = minNodeId;
            }
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Find the smallest face surrounding a point.
* @instance pRG => region graph to search.
* @param    pNodeId <= node at the base of the edge that comes nearest to the test point.
*           MTG_NULL_NODEID if there is no surrounding face.
* @param    pPoint => coordinates of search point.
* @param    pNodeIdToComponentArray => array for exlusion check.   Faces whose
*               entry in this array are the same as the excludedComponent value
*               are excluded from the search.
* @param    exludedComponent => value for exclusion test.
*
* Remark: The array and excluded component are simply integers.  This filtering mechanism
* is potentially useful for other-than connected component exclusion.
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         void            jmdlRG_smallestContainingFaceExcludingComponent
(
RG_Header                       *pRG,
MTGNodeId                       *pMinNodeId,
MTGNodeId                       *pNodeId,
const DPoint3d                  *pPoint,
EmbeddedIntArray                        *pNodeIdToComponentArray,
int                             excludedComponent
)
    {
    RG_SmallestFaceParams params;
    HideTreeRange treeRange;

    params.pRG = pRG;
    params.nodeId = MTG_NULL_NODEID;
    params.minNodeId = MTG_NULL_NODEID;
    params.minDist = 0.0;
    params.testPoint = *pPoint;
    params.pNodeIdToComponentArray = pNodeIdToComponentArray;
    params.excludedComponent = excludedComponent;

    rgXYRangeTree_initRange (&treeRange, pPoint->x, pPoint->y, pPoint->x, pPoint->y);

    rgXYRangeTree_traverseTree (
                    pRG->pFaceRangeTree,
                    (HideTree_NodeFunction)rgXYRangeTree_nodeFunction_testForRangeOverlap,
                    &treeRange,
                    (HideTree_ProcessLeafFunction)jmdlRG_cb_filterSmallestContainingFace,
                    &params);

    if (s_globalNoisy >= 10000)
        {
        double faceArea;
        jmdlRG_getFaceSweepProperties (pRG, &faceArea, NULL, NULL, params.minNodeId);
        GEOMAPI_PRINTF  (
                " jmdlRG_smallestContainingFace.... "
                "    x=%lf y=%lf\n"
                "    near nodeId = %d ref nodeId = %d\n"
                "    nodes around face = %d    area = %le\n",
                pPoint->x, pPoint->y,
                params.minNodeId,
                params.nodeId,
                (int)pRG->pGraph->CountNodesAroundFace (params.minNodeId),
                faceArea
                );
        }
    if (pMinNodeId)
        *pMinNodeId = params.minNodeId;
    if (pNodeId)
        *pNodeId = params.nodeId;
    }



/*---------------------------------------------------------------------------------**//**
* Find the smallest face surrounding a point.
* @instance pRG => region graph to search.
* @param    pNodeId <= node at the base of the edge that comes nearest to the test point.
*           MTG_NULL_NODEID if there is no surrounding face.
* @param pPoint => search point.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_smallestContainingFace
(
RG_Header                       *pRG,
MTGNodeId                       *pNearNodeId,
const DPoint3d                  *pPoint
)
    {
    MTGNodeId refNodeId = MTG_NULL_NODEID;
    jmdlRG_smallestContainingFaceExcludingComponent (pRG, pNearNodeId, &refNodeId, pPoint, NULL, -1);
    }

/*---------------------------------------------------------------------------------**//**
* Find the smallest face surrounding a point.
* @instance pRG => region graph to search.
* @param    pNodeId <= node at the base of the edge that comes nearest to the test point.
*           MTG_NULL_NODEID if there is no surrounding face.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_smallestContainingFaceExt
(
RG_Header                       *pRG,
MTGNodeId                       *pNearNodeId,
MTGNodeId                       *pRefNodeId,
const DPoint3d                  *pPoint
)
    {
    jmdlRG_smallestContainingFaceExcludingComponent (pRG, pNearNodeId, pRefNodeId, pPoint, NULL, -1);
    }

/*---------------------------------------------------------------------------------**//**
* Get the range of all vertices on the graph.
* (Overall graph may be larger if curves extend beyond vertices)
* @param pRange => range of vertices.
* @return true if vertices are present.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_getVertexRange
(
RG_Header                       *pRG,
DRange3d                        *pRange
)
    {
    pRange->InitFrom (*pRG->pVertexArray);
    return !pRange->IsNull ();
//    return jmdlEmbeddedDPoint3dArray_getDRange3d (pRG->pVertexArray, pRange);
    }

/*---------------------------------------------------------------------------------**//**
* Return true if the nodeId is valid.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_getEdgeRange
(
RG_Header                       *pRG,
DRange3d                        *pRange,
int                             nodeId
)
    {
    RG_EdgeData edgeData;
    bool    myStat = false;
    if (jmdlRG_getEdgeData (pRG, &edgeData, nodeId))
        {
        if  (edgeData.curveIndex == RG_NULL_CURVEID)
            {
            pRange->InitFrom(edgeData.xyz[0], edgeData.xyz[1]);
            myStat = true;
            }
        else
            {
            myStat = jmdlRG_linkToGetRange (pRG, &edgeData, pRange);
            }
        }
    return myStat;
    }

/*---------------------------------------------------------------------------------**//**
* Eliminate edges shorter than tolerance.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_dropShortEdges
(
RG_Header                       *pRG,
double                          minEdgeLength
)
    {
    bool    myStat = true;
    MTGGraph *pGraph = pRG->pGraph;
    MTGNodeId mateId;
    RG_EdgeData edgeData;
    MTGARRAY_SET_LOOP (nodeId, pGraph)
        {
        mateId = jmdlMTGGraph_getEdgeMate (pGraph, nodeId);
        if (nodeId < mateId
            && jmdlRG_getEdgeData (pRG, &edgeData, nodeId)
            && edgeData.curveIndex == RG_NULL_CURVEID
            )
            {
            double edgeLength = sqrt(edgeData.xyz[0].DistanceSquaredXY (*(&edgeData.xyz[1])));
            if (edgeLength < minEdgeLength)
                {
                jmdlRG_dropEdge (pRG, nodeId);
                }
            }
        }
    MTGARRAY_END_SET_LOOP (nodeId, pGraph)

    return myStat;
    }

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.   Split edges and reorder vertex loops.
* Reconstruct edge range tree at end if in incremental merge mode
* @return true if merge completed successfully.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_merge
(
RG_Header                       *pRG
)
    {
    return jmdlRG_mergeWithGapTolerance (pRG, 0.0, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @return the vertex array header pointer.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public EmbeddedDPoint3dArray* jmdlRG_getVertexArray
(
RG_Header *pRG
)
    {
    return NULL == pRG ? NULL : pRG->pVertexArray;
    }

/*---------------------------------------------------------------------------------**//**
* Placeholder for breakpoints during merge.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    checkMergeAbort
(
RG_Header                       *pRG,
int id
)
    {
    return jmdlRG_checkAbort (pRG);
    }
static double jmdlRG_restrictSizeByFractionOfRangeDiagonal
(
RG_Header                       *pRG,
double      fraction,
double      size
)
    {
    DRange3d range;
    double maxSize;
    jmdlRG_getRange (pRG, &range);
    maxSize = fraction * range.low.Distance (range.high);
    return size > maxSize ? maxSize : size;
    }

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.   Split edges and reorder vertex loops.
* Reconstruct edge range tree at end if in incremental merge mode
* Optionally close gaps.
* @param vertexVertexTolerance => tolerance for closing large gaps between preexisting
*       vertices.   This is an application data tolerance, which may be
*       much larger than computational tolerances.
* @param vertexEdgeTolerance => tolerance for closing large gaps between a vertex and
*       and edge,   This is an application data tolerance, which may be
*       much larger than computational tolerances.
* @return true if merge completed successfully.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRG_mergeWithGapTolerance_go
(
RG_Header                       *pRG,
double                          vertexVertexTolerance,
double                          vertexEdgeTolerance
)
    {
    RG_IntersectionList intersectionList;
    bool    myStat = false;
    static double s_edgeRangeFactor = 1000.0;

    static double s_maxVertexFraction = 0.001;
    static double s_maxEdgeFraction = 0.001;
    static double s_boxFactor = 10.0;
    static double s_maxBoxFraction = 0.025;

    if  (jmdlMTGGraph_getNodeCount (pRG->pGraph) == 0)
        return  true;

    vertexVertexTolerance = jmdlRG_restrictSizeByFractionOfRangeDiagonal (pRG, s_maxVertexFraction, vertexVertexTolerance);
    vertexEdgeTolerance = jmdlRG_restrictSizeByFractionOfRangeDiagonal (pRG, s_maxEdgeFraction, vertexEdgeTolerance);


    if (s_globalNoisy > 1)
        jmdlRG_print (pRG, "Before Merge");
    jmdlRGIL_init (&intersectionList);

    if (pRG->funcs.consolidateCoincidentGeometry
        && vertexVertexTolerance > 0.0)
        pRG->funcs.consolidateCoincidentGeometry (pRG->funcs.pContext, vertexVertexTolerance);

    if (    jmdlRG_updateTolerance (pRG)
        && !checkMergeAbort (pRG, 1)
        &&  jmdlRG_closeSimpleGaps (pRG, vertexVertexTolerance)
        && !checkMergeAbort (pRG, 2)
        &&  jmdlRG_dropShortEdges (pRG, jmdlRG_getTolerance (pRG))
        && !checkMergeAbort (pRG, 3)
        &&  jmdlRG_buildEdgeRangeTree (pRG, 0.0, s_edgeRangeFactor * jmdlRG_getTolerance (pRG))
        && !checkMergeAbort (pRG, 4)
        &&  jmdlRGGapList_addGapEdgesToGraph (pRG, RG_MTGMASK_GAP_MASK,
                        0.5 * jmdlRG_getTolerance (pRG),
                        jmdlRG_getTolerance (pRG),
                        vertexVertexTolerance,
                        jmdlRG_getTolerance (pRG),
                        vertexEdgeTolerance,
                        s_boxFactor,
                        s_maxBoxFraction
                        )
            /* Yeah, already did it, but more stuff showed up.  Not sure incremental
                edge additions are there., so we do it again */
        && !checkMergeAbort (pRG, 5)
        &&  jmdlRG_buildEdgeRangeTree (pRG, 0.0, s_edgeRangeFactor * jmdlRG_getTolerance (pRG))
        && !checkMergeAbort (pRG, 6)
        &&  jmdlRGMerge_findAllIntersectionsFromRangeTree (pRG, &intersectionList)
        && !checkMergeAbort (pRG, 7)
        &&  jmdlRGMerge_mergeIntersections (pRG, &intersectionList,RG_MTGMASK_NULL_FACE)
        && !checkMergeAbort (pRG, 8)
        &&  jmdlRGMerge_connectHolesToParents (pRG)
       )
        {
        if (pRG->incrementalEdgeRanges)
            jmdlRG_enableIncrementalEdgeRangeTree (pRG);
        myStat = true;
        }

#define CompileMtgPrintNOT
#ifdef CompileMtgPrint
    if (s_globalNoisy)
        {
        GEOMAPI_PRINTF (" Post-merge loop counts");
        jmdlMTGGraph_printLoopCounts (pRG->pGraph);
        }
#endif
    if (s_globalNoisy >= 10)
        {
        jmdlRG_print (pRG, "After Merge");
        jmdlMTGGraph_printFaceLoops (pRG->pGraph);
        }
    jmdlRGIL_releaseMem (&intersectionList);

    return  myStat;
    }

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.   Split edges and reorder vertex loops.
* Reconstruct edge range tree at end if in incremental merge mode
* Optionally close gaps.
* @param vertexVertexTolerance => tolerance for closing large gaps between preexisting
*       vertices.   This is an application data tolerance, which may be
*       much larger than computational tolerances.
* @param vertexEdgeTolerance => tolerance for closing large gaps between a vertex and
*       and edge,   This is an application data tolerance, which may be
*       much larger than computational tolerances.
* @return true if merge completed successfully.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_mergeWithGapTolerance
(
RG_Header                       *pRG,
double                          vertexVertexTolerance,
double                          vertexEdgeTolerance
)
    {
    Transform worldToLocal, localToWorld;
    DRange3d range;
    DPoint3d origin;
    bool     status = false;
    if (jmdlRG_getRange (pRG, &range))
        {
        origin.Interpolate (range.low, 0.5, range.high);
        worldToLocal.InitFrom (-origin.x, -origin.y, -origin.z);
        localToWorld.InitFrom (origin.x, origin.y, origin.z);
        jmdlRG_multiplyByTransform (pRG, &worldToLocal);
        status = jmdlRG_mergeWithGapTolerance_go (pRG, vertexVertexTolerance, vertexEdgeTolerance);
        jmdlRG_multiplyByTransform (pRG, &localToWorld);
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* Return arrays showing matched vertices.
* @param pLinkedLists => array in which at each vertex index the stored int is the
*       index of a "next" vertex of a cluster.
* @param pVertexBatch => array of contiguous blocks of vertex indices, each
*               terminated by a -1 index.
* @param toleranceFactor => multiplies the overall graph tolerance.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_identifyMatchedVertices
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pLinkedLists,
EmbeddedIntArray                        *pVertexBatch,
double                          toleranceFactor
)
    {
    double tol = toleranceFactor * jmdlRG_getTolerance (pRG);
    return SUCCESS == jmdlVArrayDPoint3d_identifyMatchedVertices (pRG->pVertexArray, pLinkedLists, pVertexBatch, tol, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* Search a (merged) region structure for all faces that intersect a given polygon.
*
* @param    pFaceNodeIdArray    => Array of faces contacted by the polygon.
* @param    pEdgeNodeIdArray    => Array of edges contacted by the polygon.
* @return true if search completed successfully.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_collectUnderPolyline
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pFaceNodeIdArray,
EmbeddedIntArray                        *pEdgeNodeIdArray,
DPoint3d                        *pPointArray,
int                             numPoint
)
    {
    bool    myStat = false;

    if  (  pEdgeNodeIdArray
         && ! pFaceNodeIdArray
         && pRG->incrementalEdgeRanges
         && pRG->pEdgeRangeTree
        )
        {
        myStat = jmdlRG_collectEdgesUnderPolyline (pRG, pEdgeNodeIdArray, pPointArray, numPoint);
        }
    else
        {
        RG_IntersectionList intersectionList;

        if (pFaceNodeIdArray)
            jmdlEmbeddedIntArray_empty (pFaceNodeIdArray);
        if (pEdgeNodeIdArray)
            jmdlEmbeddedIntArray_empty (pEdgeNodeIdArray);

        if  (jmdlMTGGraph_getNodeCount (pRG->pGraph) == 0)
            return  true;

        if (s_globalNoisy > 5)
            jmdlRG_print (pRG, "Before face collect");
        jmdlRGIL_init (&intersectionList);

        jmdlRGIL_findPolylineIntersectionsFromFaceRangeTree (pRG, &intersectionList, pPointArray, numPoint);

        if (pFaceNodeIdArray)
            jmdlRGIL_getUniqueSeedNodeIdArray (pRG, pFaceNodeIdArray, &intersectionList);

        if (pEdgeNodeIdArray)
            jmdlRGIL_getUniqueNodeIdArray (pRG, pEdgeNodeIdArray, &intersectionList);


        jmdlRGIL_releaseMem (&intersectionList);

        if (s_globalNoisy > 4)
            jmdlRG_print (pRG, "After face collect");
        }

    return  myStat;
    }
/*---------------------------------------------------------------------------------**//**
* Search for edges that conflict with a given polyline.  Only valid if graph is
* "incremental" edge range mode.
*
* @param    pEdgeNodeIdArray    => Array of edges contacted by the polygon.
* @return true if search completed successfully.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_collectEdgesUnderPolyline
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pEdgeNodeIdArray,
DPoint3d                        *pPointArray,
int                             numPoint
)
    {
    RG_IntersectionList intersectionList;
    bool    myStat = false;

    if (!pRG->incrementalEdgeRanges)
        return false;

    if (pEdgeNodeIdArray)
        jmdlEmbeddedIntArray_empty (pEdgeNodeIdArray);

    if  (jmdlMTGGraph_getNodeCount (pRG->pGraph) == 0)
        return  true;

    jmdlRGIL_init (&intersectionList);

    jmdlRGIL_findPolylineIntersectionsFromEdgeRangeTree (pRG, &intersectionList, pPointArray, numPoint);

    if (pEdgeNodeIdArray)
        jmdlRGIL_getUniqueNodeIdArray (pRG, pEdgeNodeIdArray, &intersectionList);


    jmdlRGIL_releaseMem (&intersectionList);

    if (s_globalNoisy > 1)
        GEOMAPI_PRINTF (
                        "Before edge collect hit count %d\n",
                        jmdlEmbeddedIntArray_getCount (pEdgeNodeIdArray)
                     );
    return  myStat;
    }

/*---------------------------------------------------------------------------------**//**
* Toss the old range tree.
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_freeEdgeRangeTree
(
RG_Header                       *pRG
)
    {
    if (pRG->pEdgeRangeTree)
        {
        rgXYRangeTree_destroyRangeTree (&pRG->pEdgeRangeTree);
        pRG->pEdgeRangeTree = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Toss the old range tree.
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_freeFaceRangeTree
(
RG_Header                       *pRG
)
    {
    if (pRG->pFaceRangeTree)
        {
        rgXYRangeTree_destroyRangeTree (&pRG->pFaceRangeTree);
        pRG->pFaceRangeTree = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Toss the old range tree and build a new one.
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_buildEdgeRangeTree
(
RG_Header                       *pRG,
double                          lowOffset,
double                          highOffset
)
    {
    MTGNodeId mateId;
    DRange3d range;
    jmdlRG_freeEdgeRangeTree (pRG);
    rgXYRangeTree_initializeTree ((void**)&pRG->pEdgeRangeTree);
    static int s_checkStopPeriod = 200;
    MTGARRAY_SET_LOOP (nodeId, pRG->pGraph)
        {
        if (jmdlRG_checkAbort (pRG, s_checkStopPeriod))
            return false;            

        mateId = jmdlMTGGraph_getEdgeMate (pRG->pGraph, nodeId);
        if (   jmdlMTGGraph_getMask (pRG->pGraph, nodeId, MTG_DIRECTED_EDGE_MASK)
            && jmdlRG_getEdgeRange (pRG, &range, nodeId))
            {
            rgXYRangeTree_addToTree (pRG->pEdgeRangeTree,
                            range.low.x  - lowOffset,    range.low.y  - lowOffset,
                            range.high.x + highOffset,  range.high.y + highOffset,
                            nodeId);
            }
        }
    MTGARRAY_END_SET_LOOP (nodeId, pRG->pGraph)
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Return an array of all nodeId's which are base of an edge in the region graph.
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_getEdgeList
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pEdgeStart
)
    {
    jmdlEmbeddedIntArray_empty (pEdgeStart);

    MTGARRAY_SET_LOOP (nodeId, pRG->pGraph)
        {
        /* Edge start condition is indicated by MTG_DIRECTED_EDGE_MASK !!! */
        if (   jmdlMTGGraph_getMask (pRG->pGraph, nodeId, MTG_DIRECTED_EDGE_MASK))
            {
            jmdlEmbeddedIntArray_addInt(pEdgeStart, nodeId);
            }
        }
    MTGARRAY_END_SET_LOOP (nodeId, pRG->pGraph)
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Toss the old range tree and build a new one.
*
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_buildFaceRangeTree
(
RG_Header                       *pRG,
double                          lowOffset,
double                          highOffset
)
    {
    double faceArea;
    MTGGraph *pGraph = pRG->pGraph;
    bvector<int> faceStartArray;
    pGraph->CollectFaceLoops (faceStartArray);
    jmdlRG_freeFaceRangeTree (pRG);
    rgXYRangeTree_initializeTree ((void**)&pRG->pFaceRangeTree);
    jmdlMTGGraph_clearMaskInSet (pGraph, RG_MTGMASK_IS_NEGATIVE_AREA_FACE);
    for (int faceSeedNodeId : faceStartArray)
        {
        /* 'Faces between duplicate edges' and 'faces whose area cannot be computed'
                are left out. */
        if (   !jmdlMTGGraph_getMask (pGraph, faceSeedNodeId, RG_MTGMASK_NULL_FACE)
            && jmdlRG_getFaceSweepProperties (pRG, &faceArea, NULL, NULL, faceSeedNodeId))
            {
            if (faceArea == 0.0)
                {
                /* Leave zero area faces out. */
                }
            else
                {
                if (faceArea < 0.0)
                    {
                    jmdlMTGGraph_setMaskAroundFace (pGraph, faceSeedNodeId, RG_MTGMASK_IS_NEGATIVE_AREA_FACE);
                    }
                jmdlRG_addFaceToXYRangeTree
                            (
                            pRG,
                            pRG->pFaceRangeTree,
                            faceSeedNodeId,
                            lowOffset,
                            highOffset
                            );
                }
            }
        }

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Toss the old faceHole array and construct a new one.  This assumes that the face range
* tree has been constructed.
* The face hole array is a set of node id pairs.  Each pair gives (outerNodeId, innerNodeId)
* for a hole.
* In addition, each node in the graph is marked so it can be quickly tested to see
* which of the four face types it is:
*<ul>
*<li>True exterior -- negative area, not contained in any other face.</li>
*<li>Plain face -- positive area, no holes.</li>
*<li>Has holes -- positive area, has holes.</li>
*<li>Hole -- negative area, contained in another face.</li>
*</ul>
*
* Face range tree must be constructed prior to calling this function.
* (so negative area bits are set)
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_buildFaceHoleArray
(
RG_Header                       *pRG
)
    {
    EmbeddedIntArray *pFaceHoleNodeIdArray;
    MTGGraph    *pGraph = pRG->pGraph;
    MTGMask     visitMask = jmdlMTGGraph_grabMask (pGraph);
    DPoint3d    vertexCoordinates;
    MTGNodeId   containingFaceNodeId;
    int         excludedComponent;
    MTGNodeId   innerRefNodeId, outerRefNodeId;
    RG_EdgeData edgeData;
    static      double s_offsetFraction = 1.2378237432432e-3;

    bvector<bvector<MTGNodeId>> components;
    pRG->pGraph->CollectConnectedComponents (components);
    bvector<int> nodeToComponentIdArray;
    for (size_t i = 0; i < pRG->pGraph->GetNodeIdCount (); i++)
        nodeToComponentIdArray.push_back (-1);
    int numComponent = 0;
    for (auto &component : components)
        {
        for (auto nodeId : component)
            nodeToComponentIdArray[(size_t)nodeId] = numComponent;
        numComponent++;
        }

    if (pRG->pFaceHoleNodeIdArray)
        {
        pFaceHoleNodeIdArray = pRG->pFaceHoleNodeIdArray;
        jmdlEmbeddedIntArray_empty (pFaceHoleNodeIdArray);
        pRG->pFaceHoleNodeIdArray = NULL;
        }
    else
        {
        pFaceHoleNodeIdArray = jmdlEmbeddedIntArray_new ();
        }

    if (numComponent >= 1)
        {
        jmdlMTGGraph_clearMaskInSet
                (
                pGraph,
                visitMask | RG_MTGMASK_FACE_APPEARS_IN_HOLE_ARRAYS
                );

        MTGARRAY_SET_LOOP (faceSeedNodeId, pGraph)
            {
            if (!jmdlMTGGraph_getMask (pGraph, faceSeedNodeId, visitMask))
                {
                jmdlMTGGraph_setMaskAroundFace (pGraph, faceSeedNodeId, visitMask);
                jmdlRG_getEdgeData (pRG, &edgeData, faceSeedNodeId);
                jmdlRG_evaluateEdgeData (pRG, &vertexCoordinates, &edgeData, s_offsetFraction);
                jmdlEmbeddedIntArray_getInt(&nodeToComponentIdArray, &excludedComponent, faceSeedNodeId);
                if (jmdlMTGGraph_getMask (pGraph, faceSeedNodeId, RG_MTGMASK_IS_NEGATIVE_AREA_FACE))
                    {
                    innerRefNodeId = jmdlRG_resolveFaceNodeId (pRG, faceSeedNodeId);
                    /* This is either a true exterior face or a hole.
                    **  Look for a containing face on a different connectedcomponent.
                    */
                    jmdlRG_smallestContainingFaceExcludingComponent
                                        (
                                        pRG,
                                        &containingFaceNodeId,
                                        NULL,
                                        &vertexCoordinates,
                                        &nodeToComponentIdArray,
                                        excludedComponent
                                        );

                    if (containingFaceNodeId != MTG_NULL_NODEID)
                        {
                        outerRefNodeId = jmdlRG_resolveFaceNodeId (pRG, containingFaceNodeId);
                        jmdlEmbeddedIntArray_addInt(pFaceHoleNodeIdArray, outerRefNodeId);
                        jmdlEmbeddedIntArray_addInt(pFaceHoleNodeIdArray, innerRefNodeId);
                        /* This is the only time this face can ever be "inner", so we always
                            set RG_MTGMASK_FACE_APPEARS_IN_HOLE_ARRAYS. */
                        jmdlMTGGraph_setMaskAroundFace (pGraph, innerRefNodeId,
                                                    RG_MTGMASK_FACE_APPEARS_IN_HOLE_ARRAYS);
                        /* The outer face may already be a parent of another hole, so
                            the RG_MTGMASK_FACE_APPEARS_IN_HOLE_ARRAYS setting is conditional. */
                        if (!jmdlMTGGraph_getMask (pGraph, outerRefNodeId,
                                                    RG_MTGMASK_FACE_APPEARS_IN_HOLE_ARRAYS))
                            jmdlMTGGraph_setMaskAroundFace (pGraph, outerRefNodeId,
                                                    RG_MTGMASK_FACE_APPEARS_IN_HOLE_ARRAYS);
                        }
                    }
                }
            }
        MTGARRAY_END_SET_LOOP (faceSeedNodeId, pGraph)

        }

    pRG->pFaceHoleNodeIdArray = pFaceHoleNodeIdArray;
    jmdlMTGGraph_dropMask (pGraph, visitMask);
    }


/*---------------------------------------------------------------------------------**//**
* Build bridge edges as indicted by the face-hole array.
* @bsimethod                    EarlinLutz      11/08/00   (The day without a president)
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_buildBridgeEdges
(
RG_Header                       *pRG
)
    {
    int iFace, iHole;
    MTGNodeId  faceNodeId, holeNodeId;
    MTGNodeId bridgeId0, bridgeId1;
    MTGMask faceToHoleMask = RG_MTGMASK_BRIDGE_EDGE;
    MTGMask holeToFaceMask = RG_MTGMASK_BRIDGE_EDGE;

    if (pRG->pFaceHoleNodeIdArray)
        {
        for (iFace = 0; jmdlEmbeddedIntArray_getInt (pRG->pFaceHoleNodeIdArray, &faceNodeId, iFace);
                    iFace += 2)
            {
            iHole = iFace + 1;
            if (jmdlEmbeddedIntArray_getInt (pRG->pFaceHoleNodeIdArray, &holeNodeId, iHole)
               )
                {
                jmdlMTGGraph_join
                            (
                            pRG->pGraph,
                            &bridgeId0, &bridgeId1,
                            faceNodeId, holeNodeId,
                            faceToHoleMask, holeToFaceMask
                            );
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Drop (delete) all bridge edges.
* @instance pRG <=> region header.
* @bsimethod                    EarlinLutz      11/08/00   (The day without a president)
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_dropBrideEdges
(
RG_Header                       *pRG
)
    {
    MTGGraph *pGraph = pRG->pGraph;
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        if (jmdlMTGGraph_getMask (pGraph, currNodeId, RG_MTGMASK_BRIDGE_EDGE))
            jmdlMTGGraph_dropEdge (pGraph, currNodeId);
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)
    }


/*---------------------------------------------------------------------------------**//**
* Test pre-recorded hole array markup to see if face is true exterior, i.e.
*   negative area and not contained in another face.
* @instance pRG => region header.
* @param nodeId => node to test.
* @bsimethod                                                    EarlinLutz      11/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_faceIsTrueExterior
(
RG_Header                       *pRG,
MTGNodeId                       nodeId
)
    {
    MTGGraph *pGraph = pRG->pGraph;
    MTGMask mask = jmdlMTGGraph_getMask (pGraph, nodeId,
                RG_MTGMASK_IS_NEGATIVE_AREA_FACE | RG_MTGMASK_FACE_APPEARS_IN_HOLE_ARRAYS);
    return  (mask & RG_MTGMASK_IS_NEGATIVE_AREA_FACE)
        && !(mask & RG_MTGMASK_FACE_APPEARS_IN_HOLE_ARRAYS);
    }

/*---------------------------------------------------------------------------------**//**
* @description Test if nodeId has been marked as a null face, e.g., by jmdlRG_mergeWithGapTolerance.
* @param pRG => region header.
* @param nodeId => node to test.
* @bsimethod                                                    David.Assaf     07/07
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlRG_faceIsNull
(
RG_Header   *pRG,
MTGNodeId   nodeId
)
    {
    MTGGraph *pGraph = pRG->pGraph;
    MTGMask mask = jmdlMTGGraph_getMask (pGraph, nodeId, RG_MTGMASK_NULL_FACE);
    return mask ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @description Test if nodeId has been marked as the start of a directed edge.
* @param pRG => region header.
* @param nodeId => node to test.
* @bsimethod                                                    David.Assaf     08/07
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlRG_edgeIsDirected
(
RG_Header   *pRG,
MTGNodeId   nodeId
)
    {
    MTGGraph *pGraph = pRG->pGraph;
    MTGMask mask = jmdlMTGGraph_getMask (pGraph, nodeId, MTG_DIRECTED_EDGE_MASK);
    return mask ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* Test pre-recorded hole array markup to see if face has negative area, i.e. might be
*   a true exterior or hole exterior.
* @instance pRG => region header.
* @param nodeId => node to test.
* @bsimethod                                                    EarlinLutz      11/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_faceIsNegativeArea
(
RG_Header                       *pRG,
MTGNodeId                       nodeId
)
    {
    MTGGraph *pGraph = pRG->pGraph;
    MTGMask mask = jmdlMTGGraph_getMask (pGraph, nodeId, RG_MTGMASK_IS_NEGATIVE_AREA_FACE);
    return  mask ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* Test if nodeId is a bridge edge.
* @instance pRG => region header.
* @param nodeId => node to test.
* @bsimethod                                                    EarlinLutz      11/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_isBridgeEdge
(
RG_Header                       *pRG,
MTGNodeId                       nodeId
)
    {
    MTGGraph *pGraph = pRG->pGraph;
    MTGMask mask = jmdlMTGGraph_getMask (pGraph, nodeId, RG_MTGMASK_BRIDGE_EDGE);
    return  mask ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* Test pre-recorded hole array markup to see if face has holes, i.e.
*   positive area and has recorded holes.
* @instance pRG => region header.
* @param nodeId => node to test.
* @bsimethod                                                    EarlinLutz      11/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_faceHasHoles
(
RG_Header                       *pRG,
MTGNodeId                       nodeId
)
    {
    MTGGraph *pGraph = pRG->pGraph;
    MTGMask mask = jmdlMTGGraph_getMask (pGraph, nodeId,
                RG_MTGMASK_IS_NEGATIVE_AREA_FACE | RG_MTGMASK_FACE_APPEARS_IN_HOLE_ARRAYS);
    return
            (mask & RG_MTGMASK_FACE_APPEARS_IN_HOLE_ARRAYS)
        && !(mask & RG_MTGMASK_IS_NEGATIVE_AREA_FACE) ;
    }

/*---------------------------------------------------------------------------------**//**
* Test pre-recorded hole array markup to see if face is a negative area (hole)
*   loop in a larger postive face.
* @instance pRG => region header.
* @param nodeId => node to test.
* @bsimethod                                                    EarlinLutz      11/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_faceIsHoleLoop
(
RG_Header                       *pRG,
MTGNodeId                       nodeId
)
    {
    MTGGraph *pGraph = pRG->pGraph;
    MTGMask mask = jmdlMTGGraph_getMask (pGraph, nodeId,
                RG_MTGMASK_IS_NEGATIVE_AREA_FACE | RG_MTGMASK_FACE_APPEARS_IN_HOLE_ARRAYS);
    return
           (mask & RG_MTGMASK_FACE_APPEARS_IN_HOLE_ARRAYS)
        && (mask & RG_MTGMASK_IS_NEGATIVE_AREA_FACE) ;
    }


/*---------------------------------------------------------------------------------**//**
* Return the holeNodeId part of each nodeId pair of the form (outerFaceNodeId,holeNodeId)
* in the face-hole array as constructed by jmdlRG_buildFaceHoleNodeIdArray.
* @instance pRG => region header.
* @param pHoleNodeId <= array containing pairs of nodes.
* @param nodeId => any node on the outer (positive area) loop of the face.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_searchFaceHoleArray
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pHoleNodeIdArray,
MTGNodeId                       outerFaceNodeId
)
    {
    int iFace, iHole;
    MTGNodeId  faceNodeId, holeNodeId;
    jmdlEmbeddedIntArray_empty (pHoleNodeIdArray);
    if (pRG->pFaceHoleNodeIdArray)
        {
        for (iFace = 0; jmdlEmbeddedIntArray_getInt (pRG->pFaceHoleNodeIdArray, &faceNodeId, iFace);
                    iFace += 2)
            {
            iHole = iFace + 1;
            if (faceNodeId == outerFaceNodeId
                && jmdlEmbeddedIntArray_getInt (pRG->pFaceHoleNodeIdArray, &holeNodeId, iHole))
                {
                jmdlEmbeddedIntArray_addInt(pHoleNodeIdArray, holeNodeId);
                }
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* Search for outerloop and array of inner loop nodes for the face containing
* given node.   The seed node may be on any inner or outer face.
*
* @param pHoleNodeIdArray <= If the face containing seedNodeId has is not part of a
* face with holes, return in pHoleNodeIdArray a single node, which is some node on the
* face containing the seed.  (Possibly the seed, possibly elsewhere.)
*  If the face containing the seed is part of a multi-loop face, the array
*       contains the outer loop representative followed by all inner loop representatives.
* @return 0 if the seed node is not part of a multiloop face, 1 if it is an outer
*               loop, -1 if it is an inner loop.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public int             jmdlRG_resolveHoleNodeId
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pHoleNodeIdArray,
MTGNodeId                       seedNodeId
)
    {
    int index0, index1;
    MTGNodeId  seedRefNodeId = jmdlRG_resolveFaceNodeId (pRG, seedNodeId);
    int outerNodeId0, outerNodeId1, innerNodeId1;

    jmdlEmbeddedIntArray_empty (pHoleNodeIdArray);
    index0 = -1;
    outerNodeId0 = MTG_NULL_NODEID;
    if (pRG->pFaceHoleNodeIdArray)
        {
        for (index1 = 0;    jmdlEmbeddedIntArray_getInt
                            (pRG->pFaceHoleNodeIdArray, &outerNodeId1, index1)
                        && jmdlEmbeddedIntArray_getInt
                            (pRG->pFaceHoleNodeIdArray, &innerNodeId1, index1 + 1);
                    index1 += 2)
            {
            /* Does this pair begin a new block of (inner,outer) records? */
            if (outerNodeId1 != outerNodeId0)
                {
                index0 = index1;
                outerNodeId0 = outerNodeId1;
                }

            /* Does this pair match the seed? */
            if (outerNodeId1 == seedRefNodeId || innerNodeId1 == seedRefNodeId)
                {
                int retValue = seedRefNodeId == outerNodeId1 ? 1 : -1;
                /* Outer first */
                jmdlEmbeddedIntArray_addInt(pHoleNodeIdArray, outerNodeId0);
                /* All the inners ... yup, its a linear search */
                for (index1 = index0;
                           jmdlEmbeddedIntArray_getInt
                            (pRG->pFaceHoleNodeIdArray, &outerNodeId1, index1)
                        && jmdlEmbeddedIntArray_getInt
                            (pRG->pFaceHoleNodeIdArray, &innerNodeId1, index1 + 1);
                    index1 += 2)
                    {
                    if (outerNodeId0 == outerNodeId1)
                        jmdlEmbeddedIntArray_addInt(pHoleNodeIdArray, innerNodeId1);
                    }
                return retValue;
                }
            }
        }
    jmdlEmbeddedIntArray_addInt(pHoleNodeIdArray, seedRefNodeId);
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* Add a single face to a caller-supplied range tree.
*
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_addFaceToXYRangeTree
(
RG_Header                       *pRG,
XYRangeTreeNode                 *pTree,
MTGNodeId                       faceStartId,
double                          lowOffset,
double                          highOffset
)
    {
    DRange3d faceRange, edgeRange;

    faceRange.Init ();

    MTGARRAY_FACE_LOOP (currId, pRG->pGraph, faceStartId)
        {
        if (jmdlRG_getEdgeRange (pRG, &edgeRange, currId))
            faceRange.Extend (edgeRange);
        }
    MTGARRAY_END_FACE_LOOP (currId, pRG->pGraph, faceStartId)

    rgXYRangeTree_addToTree (pTree,
                        faceRange.low.x  - lowOffset,    faceRange.low.y  - lowOffset,
                        faceRange.high.x + highOffset,   faceRange.high.y + highOffset,
                        faceStartId);

    return  true;
    }


/*---------------------------------------------------------------------------------**//**
* Test for point in face, using parity rules. (i.e. do not assume face is oriented and/or
* non-selfintersecting)
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRG_faceContainsPointByParity
(
        RG_Header               *pRG,
        int                     nodeId,
const   DPoint3d                *pTestPoint,
const   DRange3d                *pFaceRange
)
    {
    RG_IntersectionList intersectionList;
    DPoint3d    rayPoint[2];
    double      theta;
    double      dTheta = 0.03435234;
    bool        parityChanged;
    double      dx, dy;
    DPoint3d    dir;
    double      scaleFactor;
    int         count = 10;
    bool        funcStat = false;

    jmdlRGIL_init (&intersectionList);

    dx = pFaceRange->high.x - pFaceRange->low.x;
    dy = pFaceRange->high.y - pFaceRange->low.y;

    /* Cast rays at assorted angles until one has only clean hits with boundary. */

    for (theta = 0.0; count-- > 0; theta += dTheta)
        {
        dir.Init ( cos (theta), sin(theta), 0.0);
        if (fabs (dir.x) > fabs (dir.y))
            {
            scaleFactor = 2.0 * fabs (dx / fabs (dir.x));
            }
        else
            {
            scaleFactor = 2.0 * fabs (dy / fabs (dir.y));
            }
        rayPoint[0] = *pTestPoint;
        rayPoint[1].SumOf (rayPoint[0], dir, scaleFactor);

        jmdlRGIL_findPolylineIntersectionsFromFace
                    (
                    pRG,
                    &intersectionList,
                    nodeId,
                    rayPoint,
                    2
                    );

        if (jmdlRGIL_intersectionListHasClearParity (&intersectionList, &parityChanged))
            {
            funcStat = parityChanged;
            break;
            }
        jmdlRGIL_empty (&intersectionList);
        }
    /* All those ray casts were confused.  Call it false. */
    jmdlRGIL_releaseMem (&intersectionList);
    return funcStat;
    }

/*---------------------------------------------------------------------------------**//**
* Test for node (point) in polygon.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRG_isNodeInPolygon
(
        RG_Header               *pRG,
        int                     nodeId,
const   DPoint3d                *pPointArray,
        int                     numPoint
)
    {
    DPoint3d vertexCoords;
    int parity;
    if (jmdlRG_getVertexData (pRG, &vertexCoords, 0, NULL, nodeId, 0.0))
        {
        parity = bsiGeom_XYPolygonParity (&vertexCoords, pPointArray, numPoint, pRG->tolerance);
        return parity >= 0;
        }
    return false;
    }

typedef struct
    {
    RG_Header       *pRG;
    DRange3d        polylineRange;
    HideTreeRange   searchRange;
    DPoint3d        *pPointArray;
    int             numPoint;
    EmbeddedIntArray      *pConflictArray;
    bool            closed;
    } RG_PolylineSearchParams;

/*---------------------------------------------------------------------------------**//**
* Search for complete containment of face in polygon or polygon in face.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRG_cb_filterPolylineInFace
(
HideTreeRange                   *pRange,
int                             nodeId,
RG_PolylineSearchParams         *pParams,
void                            *pParent,
void                            *pLeaf
)
    {
    bool    myStat = true;
    DRange3d    leafRange;

    rgXYRangeTree_getLeafRange (pLeaf, &leafRange);

    if (leafRange.IsStrictlyContainedXY (pParams->polylineRange))
        {

        RG_IntersectionList intersectionList;
        jmdlRGIL_init (&intersectionList);
        jmdlRGIL_findPolylineIntersectionsFromFace
                    (
                    pParams->pRG,
                    &intersectionList,
                    nodeId,
                    pParams->pPointArray,
                    pParams->numPoint
                    );

        if (   jmdlRGIL_getCount (&intersectionList) == 0
            && pParams->closed
            && jmdlRG_isNodeInPolygon
                            (
                            pParams->pRG,
                            nodeId,
                            pParams->pPointArray,
                            pParams->numPoint
                            )
           )
            {
            jmdlEmbeddedIntArray_addInt(pParams->pConflictArray, RG_PFC_FACE_IN_POLYGON);
            jmdlEmbeddedIntArray_addInt(pParams->pConflictArray, nodeId);
            }
        jmdlRGIL_releaseMem (&intersectionList);
        }
    else if (pParams->polylineRange.IsStrictlyContainedXY (leafRange))
        {
        RG_IntersectionList intersectionList;
        jmdlRGIL_init (&intersectionList);
        jmdlRGIL_findPolylineIntersectionsFromFace
                    (
                    pParams->pRG,
                    &intersectionList,
                    nodeId,
                    pParams->pPointArray,
                    pParams->numPoint
                    );

        if (   jmdlRGIL_getCount (&intersectionList) == 0
            && jmdlRG_faceContainsPointByParity
                        (
                        pParams->pRG,
                        nodeId,
                        &pParams->pPointArray[0],
                        &leafRange
                        )
            )
            {
            jmdlEmbeddedIntArray_addInt(pParams->pConflictArray, RG_PFC_POLYLINE_IN_FACE);
            jmdlEmbeddedIntArray_addInt(pParams->pConflictArray, nodeId);
            }
        jmdlRGIL_releaseMem (&intersectionList);
        }

    return myStat;
    }

/*---------------------------------------------------------------------------------**//**
* Search for complete containment of edge in polygon.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRG_cb_filterEdgeInPolygon
(
HideTreeRange                   *pRange,
int                             nodeId,
RG_PolylineSearchParams         *pParams,
void                            *pParent,
void                            *pLeaf
)
    {
    bool    myStat = true;
    DRange3d    leafRange;

    rgXYRangeTree_getLeafRange (pLeaf, &leafRange);

    if (leafRange.IsStrictlyContainedXY (pParams->polylineRange))
        {
        RG_IntersectionList intersectionList;
        jmdlRGIL_init (&intersectionList);
        jmdlRGIL_findPolylineIntersectionsFromEdge
                    (
                    pParams->pRG,
                    &intersectionList,
                    nodeId,
                    pParams->pPointArray,
                    pParams->numPoint
                    );

        if (   jmdlRGIL_getCount (&intersectionList) == 0
            && pParams->closed
            && jmdlRG_isNodeInPolygon
                            (
                            pParams->pRG,
                            nodeId,
                            pParams->pPointArray,
                            pParams->numPoint
                            )
           )
            {
            jmdlEmbeddedIntArray_addInt(pParams->pConflictArray, RG_PFC_FACE_IN_POLYGON);
            jmdlEmbeddedIntArray_addInt(pParams->pConflictArray, nodeId);
            }
        }
    return myStat;
    }

/*---------------------------------------------------------------------------------**//**
* Find conflicts between a polyline/polygon and faces indexed by a range tree.
*
* Each hit is recorded as two integers in the conflict array:
*       First int: One of RG_PFC_EDGE_CONFLICT, RG_PFC_POLYLINE_IN_FACE, RG_PFC_FACE_IN_POLYGON
*       Second int:  seed nodeId for the face
* @param closed => true if point array is to be closed (and hence can produce RG_PFC_FACE_IN_POLYGON)
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_searchFaceRangeTreeForPolylineConflict
(
RG_Header                       *pRG,
XYRangeTreeNode                 *pTree,
EmbeddedIntArray                        *pConflictArray,
DPoint3d                        *pPointArray,
int                             numPoint,
bool                            closed
)
    {
    RG_PolylineSearchParams params;

    jmdlEmbeddedIntArray_empty (pConflictArray);
    /* Capture parameters */
    params.pRG          = pRG;
    params.pPointArray  = pPointArray;
    params.numPoint     = numPoint;
    params.closed       = closed;
    params.pConflictArray = pConflictArray;

    /* Build ranges */
    params.polylineRange.InitFrom(pPointArray, numPoint);

    rgXYRangeTree_initRange
                    (
                    &params.searchRange,
                    params.polylineRange.low.x,
                    params.polylineRange.low.y,
                    params.polylineRange.high.x,
                    params.polylineRange.high.y
                    );

    /* Roll the searches: */

    rgXYRangeTree_traverseTree (
                    pTree,
                    (HideTree_NodeFunction)rgXYRangeTree_nodeFunction_testForRangeOverlap,
                    &params.searchRange,
                    (HideTree_ProcessLeafFunction)jmdlRG_cb_filterPolylineInFace,
                    &params);

    }

/*---------------------------------------------------------------------------------**//**
* Search the edge range tree for edges that are fully contained in a given polygon.
* Each hit is recorded as two integers in the conflict array:
*       First int:   RG_PFC_EDGE_IN_POLYTON
*       Second int:  seed nodeId for the edge
* @instance pRG => region context.
* @param pConflictArray <= array of typecode and node id pairs.
* @param pPointArray => polygon coordinates
* @param numPoint => number of points in polygon.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_searchEdgeRangeTreeForPolygonContainment
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pConflictArray,
DPoint3d                        *pPointArray,
int                             numPoint
)
    {
    if (    pRG->incrementalEdgeRanges
         && pRG->pEdgeRangeTree
       )
        {
        RG_PolylineSearchParams params;

        jmdlEmbeddedIntArray_empty (pConflictArray);
        /* Capture parameters */
        params.pRG          = pRG;
        params.pPointArray  = pPointArray;
        params.numPoint     = numPoint;
        params.closed       = true;             /* implied by caller */
        params.pConflictArray = pConflictArray;

        /* Build ranges */
        params.polylineRange.InitFrom(pPointArray, numPoint);

        rgXYRangeTree_initRange
                        (
                        &params.searchRange,
                        params.polylineRange.low.x,
                        params.polylineRange.low.y,
                        params.polylineRange.high.x,
                        params.polylineRange.high.y
                        );

        /* Roll the searches: */

        rgXYRangeTree_traverseTree (
                        pRG->pEdgeRangeTree,
                        (HideTree_NodeFunction)rgXYRangeTree_nodeFunction_testForRangeOverlap,
                        &params.searchRange,
                        (HideTree_ProcessLeafFunction)jmdlRG_cb_filterEdgeInPolygon,
                        &params);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRGMerge_cb_collectNode
(
HideTreeRange                   *pRange,
int                             nodeId,
EmbeddedIntArray                *pNodeArray,
void                            *pParent,
void                            *pLeaf
)
    {
    jmdlEmbeddedIntArray_addInt (pNodeArray, nodeId);
    return true;
    }
/*---------------------------------------------------------------------------------**//**

* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_collectXYEdgeRangeHits
(
RG_Header           *pRG,
EmbeddedIntArray    *pNodeArray,
DRange3d            *pRange
)
    {
    HideTreeRange treeRange;

    rgXYRangeTree_initRange
                    (
                    &treeRange,
                    pRange->low.x,
                    pRange->low.y,
                    pRange->high.x,
                    pRange->high.y
                    );
    jmdlEmbeddedIntArray_empty (pNodeArray);
    rgXYRangeTree_traverseTree (
                            pRG->pEdgeRangeTree,
                            (HideTree_NodeFunction)rgXYRangeTree_nodeFunction_testForRangeOverlap,
                            &treeRange,
                            (HideTree_ProcessLeafFunction)jmdlRGMerge_cb_collectNode,
                            pNodeArray);
    }

/*---------------------------------------------------------------------------------**//**
* Compute the area and angle swept by a ray from a from a given origin as it sweeps
* edge.
* @instance pRG => region context
* @param pSweptArea <= (signed) area swept by line from pPoint to edge.
* @param pSweptAngle <= (signed) angle swept by line from pPoint to edge.
* @param pPoint => fixed point of sweep.
* @param edgeNodeId => node on edge.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_getEdgeSweepProperties
(
RG_Header                       *pRG,
double                          *pSweptArea,
double                          *pSweptAngle,
DPoint3d                        *pPoint,
MTGNodeId                       edgeNodeId
)
    {
    RG_EdgeData edgeData;
    DPoint3d vector0, vector1;
    double crossXY;
    double dotXY;
    double d2start, d2end;
    double tol2 = pRG->tolerance * pRG->tolerance;
    bool    computed = false;

    if (jmdlRG_getEdgeData (pRG, &edgeData, edgeNodeId))
        {
        if (edgeData.curveIndex != RG_NULL_CURVEID)
            {
            computed = jmdlRG_linkToEdgeSweepProperties (pRG,
                            &edgeData,
                            pSweptArea,
                            pSweptAngle,
                            pPoint);
            /* GEOMAPI_PRINTF("\n Curve angle: %lf", *pSweptAngle); */
            }

        if (!computed)
            {
            vector0.DifferenceOf (*(&edgeData.xyz[0]), *pPoint);
            vector1.DifferenceOf (*(&edgeData.xyz[1]), *pPoint);
            crossXY = vector0.x * vector1.y - vector0.y * vector1.x;
            dotXY   = vector0.x * vector1.x + vector0.y * vector1.y;

            d2start = vector0.x * vector0.x + vector0.y * vector0.y;
            d2end   = vector1.x * vector1.x + vector1.y * vector1.y;

            if (d2start < tol2 || d2end < tol2)
                {
                if (pSweptArea)
                    *pSweptArea = 0.0;
                if (pSweptAngle)
                    *pSweptAngle = 0.0;
                }
            else
                {
                if (pSweptArea)
                    *pSweptArea = 0.5 * crossXY;
                if (pSweptAngle)
                    *pSweptAngle = atan2 (crossXY, dotXY);
                }
            /* GEOMAPI_PRINTF("\n  Line angle: %lf", *pSweptAngle); */
            }

        return  true;
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* Project from test point to (bounded) edge.
* @instance pRG => region context.
* @param pMinParam <= parameter of closest point.
* @param pMinDistSquared <= squared distance.
* @param pMinPont <= closest point.
* @param pMinTangent <= tangent at closest point.
* @param pPoint => test point.
* @param edgeNodeId => node on edge.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_getClosestXYPointOnEdge
(
RG_Header                       *pRG,
double                          *pMinParam,
double                          *pMinDistSquared,
DPoint3d                        *pMinPoint,
DPoint3d                        *pMinTangent,
const DPoint3d                  *pPoint,
MTGNodeId                       edgeNodeId
)
    {
    RG_EdgeData edgeData;
    DPoint3d vectorU, vectorV, minPoint;
    double dotUV, dotUU;
    double param;
    bool    computed = false;

    if (jmdlRG_getEdgeData (pRG, &edgeData, edgeNodeId))
        {
        if (edgeData.curveIndex != RG_NULL_CURVEID)
            {
            if (jmdlRG_linkToGetClosestXYPointOnEdge
                                (
                                pRG,
                                &edgeData,
                                pMinParam,
                                pMinDistSquared,
                                pMinPoint,
                                pMinTangent,
                                pPoint))
                {
                computed = true;
                if (edgeData.isReversed)
                    {
                    if (pMinTangent)
                        pMinTangent->Negate();
                    if (pMinParam)
                        *pMinParam = 1.0 - *pMinParam;
                    }
                }
            /* GEOMAPI_PRINTF("\n Curve angle: %lf", *pSweptAngle); */
            }

        if (!computed)
            {
            /* Compute closet point to the chord of the edge */
            vectorU.DifferenceOf (*(&edgeData.xyz[1]), *(&edgeData.xyz[0]));
            vectorV.DifferenceOf (*pPoint, *(&edgeData.xyz[0]));

            dotUU = vectorU.DotProductXY (vectorU);
            dotUV = vectorU.DotProductXY (vectorV);

            if (dotUV <= 0)
                {
                param = 0.0;
                }
            else if (dotUV >= dotUU)
                {
                param = 1.0;
                }
            else
                {
                param = dotUV / dotUU;
                }

            minPoint.SumOf (*(&edgeData.xyz[0]), vectorU, param);

            if (pMinParam)
                *pMinParam = param;

            if (pMinDistSquared)
                *pMinDistSquared = pPoint->DistanceSquaredXY (minPoint);

            if (pMinPoint)
                *pMinPoint = minPoint;

            if (pMinTangent)
                *pMinTangent = vectorU;

            }
        return  true;
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* Determine the direction that the face "passed by" pPoint.   Assumes the given face point
* is the result of a "nearest point on face" search, so 0 and 1 parameter cases have specific
* interpretations.
* Various input parameters are the results of prior search for minimum distance point on face.
* @instance pRG => region context.
* @param pPoint => test point.
* @param mindNodeId => node on closest edge.
* @param minParam => parameter on closest edge.
* @param pMinPoint => closest point.
* @param pMinTangent => edge tangent at closest point.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_classifyPassingDirection
(
RG_Header                       *pRG,
DPoint3d                        *pPoint,
MTGNodeId                       minNodeId,
double                          minParam,
DPoint3d                        *pMinPoint,
DPoint3d                        *pMinTangent
)
    {
    double cross;
    MTGNodeId forwardNodeId, backNodeId;
    bool    ccw;
    DPoint3d edgeToPoint;
    /* Evaluate tangents a little back from the node.  This reduces multiple pole problems (a little) */
    double s_offset = 1.0e-5;
    if (minParam <= 0.0 || minParam >= 1.0)
        {
        /* We are sitting in the funny area where the closest
            point of the geometry is a vertex, not an edge interior
            point.   Get an outgoing tangent on each of the
            two incident edges.  If the cross product of the
            forward and back directed tangents is strictly negative,
            the orientation is clear.  If that cross product
            is not strictly negative, look at crosses from edge to point.
        */
        DPoint3d XForward[2], XBack[2];
        DPoint3d xyzVertex;
        DPoint3d vertexToPoint;

        if (minParam <= 0.0)
            {
            forwardNodeId = minNodeId;
            backNodeId    = jmdlMTGGraph_getVSucc (pRG->pGraph, forwardNodeId);
            }
        else
            {
            forwardNodeId = jmdlMTGGraph_getFSucc (pRG->pGraph, minNodeId);
            backNodeId    = jmdlMTGGraph_getVSucc (pRG->pGraph, forwardNodeId);
            }

        jmdlRG_getVertexData (pRG, &xyzVertex, 0, NULL, forwardNodeId, 0.0);
        vertexToPoint.DifferenceOf (*pPoint, xyzVertex);

        jmdlRG_getVertexData (pRG, XForward, 1, NULL, forwardNodeId, s_offset);
        jmdlRG_getVertexData (pRG, XBack, 1, NULL, backNodeId, s_offset);
        cross = XForward[1].CrossProductXY (XBack[1]);
        ccw = cross < 0.0
            || (   XForward[1].CrossProductXY (vertexToPoint) >= 0.0
               &&  vertexToPoint.CrossProductXY (XBack[1]) >= 0.0
               );
        }
    else
        {
        edgeToPoint.DifferenceOf (*pPoint, *pMinPoint);
        cross = pMinTangent->CrossProductXY (edgeToPoint);
        ccw = cross >= 0.0;
        }

    if (!ccw)
        {
        /* Consider the possiblity that the opposite side of this edge is also on the face,
           In this case call the face ccw because the opposite tangent could have been used. */
        MTGNodeId mateNodeId = jmdlMTGGraph_getEdgeMate (pRG->pGraph, minNodeId);
        MTGARRAY_FACE_LOOP (currNodeId, pRG->pGraph, minNodeId)
            {
            if (currNodeId == mateNodeId)
                {
                ccw = true;
                break;
                }
            }
        MTGARRAY_END_FACE_LOOP (currNodeId, pRG->pGraph, minNodeId)

        }
    return ccw;

    }
/*---------------------------------------------------------------------------------**//**
* Search all the edges around a face for the edge point closest to the test point.
* @instance pRG => region context.
* @param pMinNodeId <= base node for closest edge.
* @param pMinParam <= parameter on closest edge.
* @param pMinDist  <= distance to closest edge.
* @param pMinPoint <= closest point.
* @param pminCCW <= true if face is passing by in CCW direction, i.e. point is
*               inside face.
* @param pPoint => test point.
* @param faceNodeId => any start node on the face loop.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_getClosestPointAroundFace
(
RG_Header                       *pRG,
MTGNodeId                       *pMinNodeId,
double                          *pMinParam,
double                          *pMinDist,
DPoint3d                        *pMinPoint,
bool                            *pMinCCW,
DPoint3d                        *pPoint,
MTGNodeId                       faceNodeId
)
    {
    MTGGraph *pGraph = pRG->pGraph;

    DPoint3d currPoint, currTangent;
    double   currParam, currDistSquared;

    DPoint3d minPoint, minTangent;
    double   minParam = 0.0, minDistSquared = DBL_MAX;
    MTGNodeId minNodeId;
    minPoint.Zero ();
    minTangent.Zero ();

    minNodeId = MTG_NULL_NODEID;
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, faceNodeId)
        {
        if (!jmdlRG_getClosestXYPointOnEdge (pRG,
                                &currParam, &currDistSquared, &currPoint, &currTangent,
                            pPoint, currNodeId))
            return  false;

        if (minNodeId == MTG_NULL_NODEID || currDistSquared < minDistSquared)
            {
            minDistSquared = currDistSquared;
            minPoint    = currPoint;
            minParam    = currParam;
            minTangent  = currTangent;
            minNodeId = currNodeId;
            }
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, faceNodeId)

    if (minNodeId == MTG_NULL_NODEID)
        {
        return false;
        }
    else
        {
        if (pMinDist)
            *pMinDist = sqrt (minDistSquared);
        if (pMinPoint)
            *pMinPoint = minPoint;
        if (pMinParam)
            *pMinParam = minParam;
        if (pMinNodeId)
            *pMinNodeId = minNodeId;

        if (pMinCCW)
            {
            *pMinCCW = jmdlRG_classifyPassingDirection (pRG, pPoint, minNodeId, minParam,
                            &minPoint, &minTangent);
            }
        return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @instance pRG => region context.
* @param pArea <= computed area
* @param pSweptAngle <= computed angle swept by the face as viewed from the point.
* @param pPoint => reference point for sweep calculations.
* @param faceNodeId => any start node on the face.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRG_getFaceSweepProperties
(
RG_Header                       *pRG,
double                          *pArea,
double                          *pSweptAngle,
DPoint3d                        *pPoint,
MTGNodeId                       faceNodeId
)
    {
    static double sAreaRelTol = 1.0e-16;
    MTGGraph *pGraph = pRG->pGraph;
    double deltaArea, deltaTheta, absArea;
    DPoint3d point;
    int numEdge = 0;


    if (pPoint)
        {
        point = *pPoint;
        }
    else
        {
        jmdlRG_getVertexData (pRG, &point, 0, NULL, faceNodeId, 0.0);
        }

    absArea = 0.0;
    if (pArea)
        *pArea = 0.0;

    if (pSweptAngle)
        *pSweptAngle = 0.0;


    MTGARRAY_FACE_LOOP (currNodeId, pGraph, faceNodeId)
        {
        if (!jmdlRG_getEdgeSweepProperties (pRG, &deltaArea, &deltaTheta, &point, currNodeId))
            return  false;
        if (pArea)
            {
            *pArea              += deltaArea;
            absArea             += fabs (deltaArea);
            }
        if (pSweptAngle)
            *pSweptAngle        += deltaTheta;
        numEdge++;
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, faceNodeId)

    if (pArea)
        {
        //
        if (fabs (*pArea) < absArea * sAreaRelTol)
            *pArea = 0.0;
        }
#define CHECK_ANGLES_not
#ifdef CHECK_ANGLES
    {
    static double angleTol = 1.0e-3;
    double twoPi = 8.0 * atan (1.0);
    if (fabs(*pSweptAngle) < angleTol)
        {
        /* We are outside the face */
        }
    else if (fabs(*pSweptAngle - twoPi) < angleTol && *pArea > 0.0)
        {
        /* We are inside a ccw face */
        }
    else if (fabs(*pSweptAngle + twoPi) < angleTol && *pArea < 0.0)
        {
        /* We are outside a cw face */
        }
    else
        {
        GEOMAPI_PRINTF ("\n ?? Face %d angle %lf area %lf edges %d\n", faceNodeId, *pSweptAngle, *pArea, numEdge);
        }
    }
#endif
    return  true;
    }


/*---------------------------------------------------------------------------------**//**
* Print a summary of the region graph.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRG_print
(
RG_Header                       *pRG,
const char                      *pTitle
)
    {
    RG_EdgeData edgeData;
    char maskChars[40];
    int numMask;
    int groupId;

    GEOMAPI_PRINTF (" *******  %s  ******** \n", pTitle);
    GEOMAPI_PRINTF (" node  vert VS   FS                 x           y           z\n");

    MTGARRAY_SET_LOOP (nodeId, pRG->pGraph)
	{
	jmdlRG_getEdgeData (pRG, &edgeData, nodeId);
	numMask = 0;
	maskChars[numMask++] = jmdlMTGGraph_getMask (pRG->pGraph, nodeId, MTG_PRIMARY_EDGE_MASK ) ? 'P' : ' ';
	maskChars[numMask++] = jmdlMTGGraph_getMask (pRG->pGraph, nodeId, MTG_DIRECTED_EDGE_MASK) ? 'D' : ' ';
	maskChars[numMask] = 0;
        jmdlRG_getGroupId (pRG, &groupId, nodeId);
	GEOMAPI_PRINTF(" %4d %s %4d %4d %4d %12.8lg %12.8lg %12.6lg G%2d E%4d V%5d C%d\n",
			    nodeId, 
			    maskChars,
			    edgeData.vertexIndex[0],
			    jmdlMTGGraph_getVSucc (pRG->pGraph, nodeId),
			    jmdlMTGGraph_getFSucc (pRG->pGraph, nodeId),
			    edgeData.xyz[0].x, edgeData.xyz[0].y, edgeData.xyz[0].z,
                            groupId,
			    edgeData.nodeId[1],
			    edgeData.vertexIndex[1],
			    edgeData.curveIndex);
	}
    MTGARRAY_END_SET_LOOP (nodeId, pRG->pGraph)
    }


/*---------------------------------------------------------------------------------**//**
*
* Link to application-supplied function to intersect a circle and an edge
* @instance pRG => region context
* @param pParameterArray <= array of intersection parameters.
* @param pPointArray <= array of intersection points.
* @param pEdgeData => edge data.
* @param pCenter => circle center.
* @param radius => circle radius.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool        jmdlRG_linkToCurveCircleXYIntersection
(
RG_Header           *pRG,
bvector<double>     *pParameterArray,
EmbeddedDPoint3dArray *pPointArray,
RG_EdgeData         *pEdgeData,
DPoint3d            *pCenter,
double              radius
)
    {
    RGC_IntersectCurveCircleXYFunction F = pRG->funcs.intersectCurveCircleXY;
    if  (!F)
        return  false;

    return F (pRG->funcs.pContext, pParameterArray, pPointArray, pEdgeData, pCenter, radius);
    }
/*---------------------------------------------------------------------------------**//**
*
* Link to application-supplied function to intersect a line segment and
* a curve.
* @instance pRG => region context
* @param    pIL => intersection list to receive announcements of intersections
*                   and near approaches.
* @param    pEdge0Data => First edge, known to be linear
* @param    pEdge1Data => Second edge, known to be curve.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool        jmdlRG_linkToSegmentCurveIntersection
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
RG_EdgeData         *pEdge0Data,
RG_EdgeData         *pEdge1Data
)
    {
    RGC_IntersectSegmentCurveFunction F = pRG->funcs.intersectSegmentCurve;
    if  (!F)
        return  false;

    return F (pRG->funcs.pContext, pRG, pIL, pEdge0Data, pEdge1Data);
    }

/*---------------------------------------------------------------------------------**//**
* @instance pRG => region context.
* @param pArea <= area of rule surface from the fixed point to the edge.
* @param pSweptAngle <= computed angle swept by the edge as viewed from the point.
* @param pPoint => reference point for sweep calculations.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool        jmdlRG_linkToEdgeSweepProperties
(
RG_Header           *pRG,
RG_EdgeData         *pEdgeData,
double              *pArea,
double              *pSweptAngle,
const DPoint3d      *pPoint
)
    {
    RGC_SweptCurvePropertiesFunction F = pRG->funcs.sweptCurveProperties;

    if  (!F)
        return  false;

    return F (pRG->funcs.pContext, pEdgeData, pArea, pSweptAngle, pPoint);
    }


/*---------------------------------------------------------------------------------**//**
* Link to application-supplied function to intersect two curves.
* @instance pRG => region context
* @param    pIL => intersection list to receive announcements of intersections
*                   and near approaches.
* @param    pEdge0Data => First edge, known to be curve
* @param    pEdge1Data => Second edge, known to be curve.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool        jmdlRG_linkToCurveCurveIntersection
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
RG_EdgeData         *pEdge0Data,
RG_EdgeData         *pEdge1Data
)
    {
    RGC_IntersectCurveCurveFunction F = pRG->funcs.intersectCurveCurve;
    if  (!F)
        return  false;

    return F (pRG->funcs.pContext, pRG, pIL, pEdge0Data, pEdge1Data);
    }

/*---------------------------------------------------------------------------------**//**
* @return true if the linkage was completed and the nearest ponit computed.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRG_linkToGetClosestXYPointOnEdge
(
RG_Header                       *pRG,
const RG_EdgeData               *pEdgeData,
double                          *pMinParam,
double                          *pMinDistSquared,
DPoint3d                        *pMinPoint,
DPoint3d                        *pMinTangent,
const DPoint3d                  *pPoint
)
    {
    DPoint3d point = *pPoint;   /* const problem */
    RGC_GetClosestXYPointOnCurveFunction F = pRG->funcs.getClosestXYPointOnCurve;

    if  (!F)
        return  false;

    return  F (pRG->funcs.pContext, pMinParam, pMinDistSquared, pMinPoint, pMinTangent, &point, pEdgeData->curveIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @param pPoint <= returned point.
* @param endIndex => 0 for start, 1 for end.
* @return true if the linkage was completed and the point and tangent computed.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRG_linkToEvaluateCurve
(
RG_Header   *pRG,
const RG_EdgeData *pEdgeData,
DPoint3d    *pPoint,
DPoint3d    *pTangent,
double      *pParameter,
int         numParam
)
    {
    RGC_EvaluateCurveFunction F = pRG->funcs.evaluateCurve;

    if  (!F)
        return  false;

    return  F (pRG->funcs.pContext, pPoint, pTangent, pParameter, numParam, pEdgeData->curveIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @param pX <= returned curve point plus derivatives
* @return true if the linkage was completed and the point and tangent computed.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRG_linkToEvaluateDerivatives
(
RG_Header   *pRG,
const RG_EdgeData *pEdgeData,
DPoint3d    *pX,
int         numDerivative,
double      param
)
    {
    RGC_EvaluateDerivativesFunction F = pRG->funcs.evaluateDerivatives;

    if  (!F)
        return  false;

    return  F (pRG->funcs.pContext, pX, numDerivative, param, pEdgeData->curveIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @instance pRG <=> region context.
* @param pEdgeData => parent edge description.
* @param pNewCurveIndex <= newly created subcurve index.
* @param s0 => start of parametric interval.
* @param s1 => end of parametric interval.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRG_linkToCreateSubcurve
(
RG_Header   *pRG,
const RG_EdgeData *pEdgeData,
RG_CurveId  *pNewCurveIndex,
double      s0,
double      s1
)
    {
    RGC_CreateSubcurveFunction F = pRG->funcs.createSubcurve;
    RG_CurveId oldCurveIndex = pEdgeData->curveIndex;

    if  (!F)
        return  false;
    *pNewCurveIndex = RG_NULL_CURVEID;
    return  F (pRG->funcs.pContext, pNewCurveIndex, oldCurveIndex, s0, s1);
    }

/*---------------------------------------------------------------------------------**//**
* Evaluate the range of a specified curve.
* @param pEdgeData => edge whose range is computed.
* @return true if the range computation succeeds.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRG_linkToGetRange
(
RG_Header   *pRG,
RG_EdgeData *pEdgeData,
DRange3d    *pRange
)
    {
    RGC_GetCurveRangeFunction F = pRG->funcs.getCurveRange;

    if  (!F)
        return  false;

    return  F (pRG->funcs.pContext, pRange, pEdgeData->curveIndex);
    }


/*---------------------------------------------------------------------------------**//**
* @return Application curve id from the edge data.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public int     jmdlRGEdge_getCurveId
(
RG_EdgeData *pEdgeData
)
    {
    return  pEdgeData->curveIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @return sense flag from edge data
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRGEdge_isReversed
(
RG_EdgeData *pEdgeData
)
    {
    return  pEdgeData->isReversed;
    }

/*---------------------------------------------------------------------------------**//**
* Extract a start or endpoint node id for a region edge.
* @param endIndex => 0 for start, 1 for end.
* @return selected node id from edge summary data.   MTG_NULL_NODEID if the end index
*           is invalid.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public MTGNodeId     jmdlRGEdge_getNodeId
(
RG_EdgeData *pEdgeData,
int         endIndex
)
    {
    if  (endIndex == 0 || endIndex == 1)
        return  pEdgeData->nodeId[endIndex];
    return  MTG_NULL_NODEID;
    }

/*---------------------------------------------------------------------------------**//**
* @param endIndex => 0 for start, 1 for end.
* @return selected vertex id from edge summary data.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public int      jmdlRGEdge_getVertexId
(
RG_EdgeData *pEdgeData,
int         endIndex
)
    {
    if  (endIndex == 0 || endIndex == 1)
        return  pEdgeData->vertexIndex[endIndex];
    return  -1;
    }


/*---------------------------------------------------------------------------------**//**
* Extract start or endpoint xyz from edge data
* @param pPoint <= returned point.
* @param endIndex => 0 for start, 1 for end.
* @return true if the end index is valid.
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlRGEdge_getXYZ
(
RG_EdgeData *pEdgeData,
DPoint3d    *pPoint,
int         endIndex
)
    {
    if  (endIndex == 0 || endIndex == 1)
        {
        *pPoint = pEdgeData->xyz[endIndex];
        return  true;
        }

    return  false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
