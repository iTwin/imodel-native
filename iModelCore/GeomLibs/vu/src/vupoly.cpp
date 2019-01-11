/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/src/vupoly.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#ifndef VU_NULL_MASK
#define VU_NULL_MASK 0
#endif

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_makeIndexedLoopFromPartialArray                      |
|                                                                       |
| author        EarlinLutz                              01/01           |
|                                                                       |
| Construct loop from contiguous subset of an array of points.          |
| Omit duplicates within an xy tolerance.                               |
| No disconnect logic -- next level up must do that.
|                                                                       |
+----------------------------------------------------------------------*/
static VuP     vu_makeIndexedLoopFromPartialArray  /* <= pointer to an arbitrary vu on the loop */
(
VuSetP          graphP,                 /* <=> VU set in which a loop is to be constructed */
DPoint3d        *pointP,                /* => array of uv coordinates of the points, */
bool            *secondaryMaskFlagP,    /* => optional array of flags for secondary mask */
int             iFirst,                 /* => first index to examine. */
int             iLast,                  /* => last index to examine. */
VuMask          leftMask,               /* => mask to apply to nominal left side of loop */
VuMask          rightMask,              /* => mask to apply to nominal right side of loop */
VuMask          secondaryMask,          /* => mask to apply selectively */
double          xyTolerance             /* => tolerance for declaring continguous points
                                                identical. */
)
    {
    double      dx, dy;
    DPoint3d    lastPoint;
    DPoint3d    currPoint;
    VuP         baseP, insideP, outsideP;
    int         i;

    baseP = VU_NULL;    /* During construction, baseP is the last created VU (if any) */

    /* Strip of trailing points that duplicate first point */
    for (; iLast > iFirst; iLast--)
        {
        dx = fabs (pointP[iLast].x - pointP[iFirst].x);
        dy = fabs (pointP[iLast].y - pointP[iFirst].y);
        if (dx > xyTolerance || dy > xyTolerance)
            break;
        }

    if (iLast < iFirst)
        return NULL;

    lastPoint = pointP[iLast];

    for (i = iFirst; i <= iLast; i++)
        {
        currPoint = pointP[i];
        if (i == iFirst      /* Always put the first point in */
            || fabs (currPoint.x - lastPoint.x) > xyTolerance
            || fabs (currPoint.y - lastPoint.y) > xyTolerance)
            {
            vu_splitEdge (graphP, baseP, &insideP, &outsideP);
            baseP = insideP;
            VU_SET_UVW( insideP, currPoint.x, currPoint.y, currPoint.z);
            VU_SET_UVW(outsideP, currPoint.x, currPoint.y, currPoint.z);
            vu_setUserDataPAsInt ( insideP, i);
            vu_setUserDataPAsInt (outsideP, i);
            // Tricky tricky masking.
            // If the mask is an edge property, it gets carried through the split.
            // Soo.. explicitly clear/or set it on each inside edge.
            // post process carries it to outside
            if (NULL != secondaryMaskFlagP)
                vu_writeMask (insideP, secondaryMask, secondaryMaskFlagP[i] ? 1 : 0);
            lastPoint = currPoint;
            }
        }

    if (baseP)
        {
        vu_setMaskAroundFace (baseP, leftMask);
        vu_setMaskAroundFace (VU_VSUCC(baseP), rightMask);
        if (NULL != secondaryMaskFlagP)
            {
            VU_FACE_LOOP (currP, baseP)
                {
                VuMask insideMask = vu_getMask (currP, secondaryMask);
                vu_writeMask (vu_edgeMate (currP), secondaryMask, ((insideMask != 0) ? 1 : 0));
                }
            END_VU_FACE_LOOP (currP, baseP)
            }
        }
    return baseP;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_makeIndexedLoopFromArrayWithDisconnects              |
|                                                                       |
| author        EarlinLutz                              01/01           |
|                                                                       |
| Construct loops from an array of points.                              |
| Omit duplicates within an xy tolerance.                               |
| DISCONNECT as either x or y value indicates separator point.  This    |
|   initiates new loop; only first loop is indicated by the return      |
|   value.                                                              |
|                                                                       |
+----------------------------------------------------------------------*/
static VuP     vu_makeIndexedLoopFromArrayWithDisconnects  /* <= pointer to an arbitrary vu on the first loop */
(
VuSetP          graphP,                 /* <=> VU set in which a loop is to be constructed */
DPoint3d        *pointP,                /* => array of uv coordinates of the points, */
bool            *secondaryMaskFlagP,    /* => optional array of flags for secondary mask */
int             nPoints,                /* => number of points */
VuMask          leftMask,               /* => mask to apply to nominal left side of loop */
VuMask          rightMask,              /* => mask to apply to nominal right side of loop */
VuMask          secondaryMask,          /* => mask to apply selectively */
double          xyTolerance,            /* => tolerance for declaring continguous points
                                                identical. */
double          disconnectValue
)
    {
    int iFirst, iLast, iTest;
    VuP outputP = NULL;
    VuP currP = NULL;
    iTest = -1;
    while (iTest < nPoints)
        {
        iFirst = iTest + 1;
        iTest = iFirst;
        for (iTest = iFirst;
               iTest < nPoints
            && pointP[iTest].x != disconnectValue
            && pointP[iTest].y != disconnectValue
            ;
            iTest++)
            {
            /* Nothing to do, just looking ahead */
            }
        iLast = iTest - 1;
        if (iLast >= iFirst)
            {
            currP = vu_makeIndexedLoopFromPartialArray
                        (
                        graphP,
                        pointP,
                        secondaryMaskFlagP,
                        iFirst, iLast,
                        leftMask, rightMask, secondaryMask, xyTolerance
                        );
            if (!outputP)
                outputP = currP;
            }
        }
    return outputP;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_makeIndexedLoopFromArray                             |
|                                                                       |
| author        EarlinLutz                              01/01           |
|                                                                       |
| Construct a single loop from an array of points, omitting duplicates  |
| by an xy tolerance.                                                   |
|                                                                       |
+----------------------------------------------------------------------*/
Public VuP      vu_makeIndexedLoopFromArray  /* <= pointer to an arbitrary vu on the loop */
(
VuSetP          graphP,                 /* <=> VU set in which a loop is to be constructed */
DPoint3d        *pointP,                /* => array of uv coordinates of the points, */
int             nPoints,                /* => number of points */
VuMask          leftMask,               /* => mask to apply to nominal left side of loop */
VuMask          rightMask,              /* => mask to apply to nominal right side of loop */
double          xyTolerance             /* => tolerance for declaring continguous points
                                                identical. */
)
    {
    return vu_makeIndexedLoopFromArrayWithDisconnects (
                graphP, pointP, NULL, nPoints,
                leftMask, rightMask, VU_NULL_MASK,
                xyTolerance, DISCONNECT);
    }

/*-----------------------------------------------------------------*//**
* Triangulate a single xy polygon.  Triangulation preserves original
*   indices.
* @param pSignedOneBasedIndices <= array of indices.  Each face is added
*           as one-based indices, followed by a 0 (terminator).
*           Interior edges are optionally negated.
* @param pExteriorLoopIndices <= array of indices of actual outer loops. (optional)
*           (These are clockwise loops as viewed.)
* @param pXYZOut <= output points.  The first numPoint points are exactly
*           the input points.   If necessary and permitted, additional
*           xyz are added at crossings.  In the usual case in which crossings
*           are not expected, this array may be NULL.
* @param pointP => array of polygon points.
* @param numPoint => number of polygon points.
* @param xyTolerance => tolerance for short edges on input polygon.
* @param signedOneBasedIndices => if true, output indices.
* @param addVerticesAtCrossings => true if new coorinates can be added to pXYZOut
* @return ERROR if nonsimple polygon.
+----------------------------------------------------------------------*/
Public StatusInt vu_triangulateXYPolygonExt2
(
EmbeddedIntArray    *pIndices,
EmbeddedIntArray    *pExteriorLoopIndices,
EmbeddedDPoint3dArray *pXYZOut,
DPoint3d            *pPoints,
int                 numPoint,
double              xyTolerance,
int                 maxPerFace,
bool                signedOneBasedIndices,
bool                addVerticesAtCrossings
)
    {
    VuSetP      graphP = vu_newVuSet (0);
    VuArrayP    faceArrayP = graphP ? vu_grabArray (graphP) : NULL;
    VuMask      numberedNodeMask = graphP ? vu_grabMask (graphP) : 0;
    VuP         faceP, originalNodeP;
    StatusInt status = ERROR;
    int       i, numError;
    int       originalIndex;
    int       outputIndex;
    int     separator = signedOneBasedIndices ? 0 : -1;
    static double s_absTol = 1.0e-14;
    static double s_relTol = 1.0e-9;
    vu_setTol (graphP, s_absTol, s_relTol);
    jmdlEmbeddedIntArray_empty (pIndices);

    if (pXYZOut)
        {
        jmdlEmbeddedDPoint3dArray_empty (pXYZOut);
        jmdlEmbeddedDPoint3dArray_addDPoint3dArray (pXYZOut, pPoints, numPoint);
        }

    vu_clearMaskInSet (graphP, numberedNodeMask);

    if (graphP && faceArrayP)
        {
        VuMask newNodeMask = numberedNodeMask | VU_BOUNDARY_EDGE;
        vu_makeIndexedLoopFromArrayWithDisconnects (graphP, pPoints, NULL, numPoint,
                    newNodeMask, newNodeMask, VU_NULL_MASK,
                    xyTolerance, DISCONNECT);

        // TR #191917: keep dangling edges in triangulation
        vu_mergeOrUnionLoops (graphP, VUUNION_UNION);

        vu_regularizeGraph (graphP);
        vu_markAlternatingExteriorBoundaries(graphP,true);
        vu_splitMonotoneFacesToEdgeLimit (graphP, maxPerFace);

        if (maxPerFace == 3)
            vu_flipTrianglesToImproveQuadraticAspectRatio (graphP);

        vu_collectInteriorFaceLoops (faceArrayP, graphP);
        numError = 0;

        // force numbered node indices everywhere.  Last one seen at a vertex wins
        // (And this will do each loop at least 2X.)
        // EDL Aug 5, 2014 This corrects a problem where
        //   (1) an input vertex is at an interior point of another edge.
        //   (2) hence a node is inserted there
        //   (3) the exterior loop pass tries to get a vertex index out of the "other" node.
        //   (4) and see's 0, which becomes 1 and is a valid index but a zinger.
        // This fixes "the problem" with minimal interference.
        // After this fix, the originalNodeP logic can probably be simplified.  But I can't test that here.
        VU_SET_LOOP (seedP, graphP)
            {
            if (vu_getMask (seedP, numberedNodeMask))
                {
                int id = vu_getUserDataPAsInt (seedP);
                VU_VERTEX_LOOP (currP, seedP)
                    {
                    vu_setUserDataPAsInt (currP, id);
                    }
                END_VU_VERTEX_LOOP (currP, seedP)
                }
            }
        END_VU_SET_LOOP (seedP, graphP)

        vu_arrayOpen (faceArrayP);
        status = SUCCESS;
        for (i = 0; SUCCESS == status && vu_arrayRead (faceArrayP, &faceP); i++)
            {
            // We triangulated.  So of course there are 3 nodes per face.
            // Really?  If the input polygon retraces itself, there will be
            // sliver faces with only 2 edges.  
            if (vu_faceLoopSize (faceP) < 3)
                {
                vu_setMaskAroundFace (faceP, VU_EXTERIOR_EDGE);
                continue;
                }
                
            VU_FACE_LOOP (currP, faceP)
                {
                /* Is this an original node, or is it an intersection? */
                originalNodeP = vu_findMaskAroundVertex (currP, numberedNodeMask);
                if (originalNodeP)
                    {
                    originalIndex = vu_getUserDataPAsInt (originalNodeP);
                    if (signedOneBasedIndices)
                        {
                        outputIndex = 1 + originalIndex;
                        if (!vu_getMask (currP, newNodeMask))
                            outputIndex = -outputIndex;
                        }
                    else
                        outputIndex = originalIndex;

                    jmdlEmbeddedIntArray_addInt (pIndices, outputIndex);
                    }
                else if (addVerticesAtCrossings  && pXYZOut)
                    {
                    /* It's a new point.  Add the coordinates, assign an index,
                       and promote outgoing edges to first class status */
                    DPoint3d newXYZ;
                    int newVertexIndex;
                    vu_getDPoint3d (&newXYZ, currP);
                    jmdlEmbeddedDPoint3dArray_addDPoint3d (pXYZOut, &newXYZ);
                    newVertexIndex = jmdlEmbeddedDPoint3dArray_getCount (pXYZOut);  // 1-based

                    vu_setMask (currP, numberedNodeMask);
                    vu_setUserDataPAsInt (currP, newVertexIndex-1);

                    if (signedOneBasedIndices)
                        {
                        outputIndex = newVertexIndex;
                        if (!vu_getMask (currP, newNodeMask))
                            outputIndex = -outputIndex;
                        }
                    else
                        outputIndex = newVertexIndex - 1;

                    jmdlEmbeddedIntArray_addInt (pIndices, outputIndex);
                    }
                else
                    {
                    status = ERROR;
                    break;
                    }
                }
            END_VU_FACE_LOOP (currP, faceP)
            jmdlEmbeddedIntArray_addInt (pIndices, separator);
            }

        // Exterior loops
        if (status == SUCCESS && pExteriorLoopIndices)
            {
            // There may be edges closing over concave parts of the exterior.
            // These will be "exterior" on both sides -- delete them.
            vu_freeEdgesByMaskCount (graphP, VU_EXTERIOR_EDGE, false, false, true);

            vu_collectExteriorFaceLoops (faceArrayP, graphP);
            vu_arrayOpen (faceArrayP);
            status = SUCCESS;
            for (i = 0; SUCCESS == status && vu_arrayRead (faceArrayP, &faceP); i++)
                {
                VuP lowIndexP = faceP;
                int lowIndex = vu_getUserDataPAsInt (faceP);

                VU_FACE_LOOP (currP, faceP)
                    {
                    int index = vu_getUserDataPAsInt (currP);
                    if (index < lowIndex)
                        {
                        lowIndex = index;
                        lowIndexP = currP;                        
                        }
                    }
                END_VU_FACE_LOOP (currP, faceP)

                VU_FACE_LOOP (currP, lowIndexP)
                    {
                    int index = vu_getUserDataPAsInt (currP);
                    if (signedOneBasedIndices)
                        outputIndex = index + 1;
                    else
                        outputIndex = index;
                    jmdlEmbeddedIntArray_addInt (pExteriorLoopIndices, outputIndex);
                    }
                END_VU_FACE_LOOP (currP, lowIndexP)
                jmdlEmbeddedIntArray_addInt (pExteriorLoopIndices, separator);
                }
            }
        }

    if (numberedNodeMask)
        vu_returnMask (graphP, numberedNodeMask);
    if (faceArrayP)
        vu_returnArray (graphP, faceArrayP);
    if (graphP)
        vu_freeVuSet (graphP);

    return status;
    }

/*-----------------------------------------------------------------*//**
* Triangulate a single xy polygon.  Triangulation preserves original
*   indices.
* @param pSignedOneBasedIndices <= array of indices.  Each face is added
*           as one-based indices, followed by a 0 (terminator).
*           Interior edges are optionally negated.
* @param pXYZOut <= output points.  The first numPoint points are exactly
*           the input points.   If necessary and permitted, additional
*           xyz are added at crossings.  In the usual case in which crossings
*           are not expected, this array may be NULL.
* @param pointP => array of polygon points.
* @param numPoint => number of polygon points.
* @param xyTolerance => tolerance for short edges on input polygon.
* @param signedOneBasedIndices => if true, output indices.
* @param addVerticesAtCrossings => true if new coorinates can be added to pXYZOut
* @return ERROR if nonsimple polygon.
+----------------------------------------------------------------------*/
Public StatusInt vu_triangulateXYPolygon
(
EmbeddedIntArray    *pIndices,
EmbeddedDPoint3dArray *pXYZOut,
DPoint3d            *pPoints,
int                 numPoint,
double              xyTolerance,
int                 maxPerFace,
bool                signedOneBasedIndices,
bool                addVerticesAtCrossings
)
    {
    return vu_triangulateXYPolygonExt2 (pIndices, NULL, pXYZOut, pPoints, numPoint,
                        xyTolerance, maxPerFace, signedOneBasedIndices, addVerticesAtCrossings);
    }



/*-----------------------------------------------------------------*//**
Compute a local to world transformation for a polygon (disconnects allowed)
Favor first polygon CCW for upwards normal.
Favor the first edge as x direction.
Favor first point as origin.
If unable to do that, use GPA code which will have random
+----------------------------------------------------------------------*/
Public bool    vu_coordinateFrameOnPolygon
(
DPoint3d const *pXYZ,
int            numXYZ,
Transform       *pLocalToWorld,
Transform       *pWorldToLocal
)
    {
    Transform myWorldToLocal, myLocalToWorld;
    bool stat = PolygonOps::CoordinateFrame (pXYZ, (size_t)numXYZ, myLocalToWorld, myWorldToLocal);
    if (NULL != pLocalToWorld)
        *pLocalToWorld = myLocalToWorld;
    if (NULL != pWorldToLocal)
        *pWorldToLocal = myWorldToLocal;
    return stat;
    }
/*-----------------------------------------------------------------*//**
* Triangulate a single space polygon as projected in caller-supplied coordinate frame.
  Best effort to handle non-planar polygons.  Optionally handle self-intersecting polygons.
* @param pIndices               <=  array of output indices.  The value of bSignedOneBasedIndices determines the separator of each face loop
*                                   and whether or not interior edges are negated.
* @param pExteriorLoopIndices   <= array giving vertex sequence for each exterior loop.
* @param pXYZ                   <=  array of output points, or NULL to disallow adding points at crossings.
* @param pLocalToWorld          => local to world transformation
* @param pWorldToLoal           => world to local transformation
* @param pointP                 =>  array of input polygon points.
* @param numPoint               =>  number of polygon points.
* @param xyTolerance            =>  tolerance for short edges on input polygon.
* @param bSignedOneBasedIndices =>  if false, output indices are 0-based, with -1 as separator.  If true, indices are 1-based, and
*                                   interior edges are negated, with 0 as separator.
* @return ERROR if nonsimple polygon.
* @bsimethod                                    EarlinLutz  09/06
+----------------------------------------------------------------------*/
Public StatusInt vu_triangulateProjectedPolygonExt
(
EmbeddedIntArray*       pIndices,
EmbeddedIntArray*       pExteriorLoopIndices,
EmbeddedDPoint3dArray*  pXYZ,
Transform const         *pLocalToWorld,
Transform const         *pWorldToLocal,
DPoint3d* const         pPoints,
int                     numPoint,
double                  xyTolerance,
bool                 bSignedOneBasedIndices,
int                     maxPerFace
)
    {
    EmbeddedDPoint3dArray*      pLocalXYZ = jmdlEmbeddedDPoint3dArray_grab ();
    StatusInt                   status = ERROR;

    if (!pIndices || !pPoints || !pLocalXYZ)
        goto wrapup;

    jmdlEmbeddedIntArray_empty (pIndices);
    jmdlEmbeddedDPoint3dArray_empty (pXYZ);

    if (jmdlEmbeddedDPoint3dArray_addDPoint3dArray (pLocalXYZ, pPoints, numPoint))
        {
        DPoint3d*   pBuffer = jmdlEmbeddedDPoint3dArray_getPtr (pLocalXYZ, 0);
        int         i;

        // transform to plane
        for (i = 0; i < numPoint; i++)
            {
            if (pBuffer[i].x != DISCONNECT && pBuffer[i].y != DISCONNECT)
                pWorldToLocal->Multiply (pBuffer[i]);
            }

        status = vu_triangulateXYPolygonExt2 (pIndices, pExteriorLoopIndices, pXYZ, pBuffer, numPoint, xyTolerance, maxPerFace, bSignedOneBasedIndices, pXYZ != NULL);

        // transform back to world
        if ((SUCCESS == status) && pXYZ)
            {
            int nXYZ = jmdlEmbeddedDPoint3dArray_getCount (pXYZ);
            pBuffer = jmdlEmbeddedDPoint3dArray_getPtr (pXYZ, 0);

            for (i = 0; i < nXYZ; i++)
                if (pBuffer[i].x != DISCONNECT && pBuffer[i].y != DISCONNECT)
                    pLocalToWorld->Multiply (pBuffer[i]);
            }
        }

wrapup:
    jmdlEmbeddedDPoint3dArray_drop (pLocalXYZ);
    return status;
    }


Public StatusInt vu_triangulateProjectedPolygon
(
EmbeddedIntArray*       pIndices,
EmbeddedIntArray*       pExteriorLoopIndices,
EmbeddedDPoint3dArray*  pXYZ,
Transform const         *pLocalToWorld,
Transform const         *pWorldToLocal,
DPoint3d* const         pPoints,
int                     numPoint,
double                  xyTolerance,
bool                 bSignedOneBasedIndices,
int                     maxPerFace
)
    {
    return vu_triangulateProjectedPolygonExt (pIndices, pExteriorLoopIndices, pXYZ, pLocalToWorld, pWorldToLocal, pPoints, numPoint, xyTolerance, bSignedOneBasedIndices, maxPerFace);
    }

/*-----------------------------------------------------------------*//**
* Triangulate a single space polygon.  Best effort to handle non-planar polygons.  Optionally handle self-intersecting polygons.
* @param pIndices               <=  array of output indices.  The value of bSignedOneBasedIndices determines the separator of each face loop
*                                   and whether or not interior edges are negated.
* @param pExteriorLoopIndices   <= array giving vertex sequence for each exterior loop.
* @param pXYZ                   <=  array of output points, or NULL to disallow adding points at crossings.
* @param pLocalToWorld          <= local to world transformation
* @param pWorldToLocal          => world to local transformation
* @param pointP                 =>  array of input polygon points.
* @param numPoint               =>  number of polygon points.
* @param xyTolerance            =>  tolerance for short edges on input polygon.
* @param bSignedOneBasedIndices =>  if false, output indices are 0-based, with -1 as separator.  If true, indices are 1-based, and
*                                   interior edges are negated, with 0 as separator.
* @return ERROR if nonsimple polygon.
* @bsimethod                                    EarlinLutz  09/06
+----------------------------------------------------------------------*/
Public StatusInt vu_triangulateSpacePolygonExt2
(
EmbeddedIntArray*       pIndices,
EmbeddedIntArray*       pExteriorLoopIndices,
EmbeddedDPoint3dArray*  pXYZ,
Transform               *pLocalToWorld,
Transform               *pWorldToLocal,
DPoint3d*               pPoints,
int                     numPoint,
double                  xyTolerance,
bool                    bSignedOneBasedIndices,
int                     maxPerFace
)
    {
    Transform                   worldToLocal, localToWorld;
    StatusInt                   status = ERROR;

    if (PolygonOps::CoordinateFrame (pPoints, (size_t)numPoint, localToWorld, worldToLocal))
        {
        if (pLocalToWorld)
            *pLocalToWorld = localToWorld;
        if (pWorldToLocal)
            *pWorldToLocal = worldToLocal;

        status = vu_triangulateProjectedPolygon
                    (
                    pIndices, pExteriorLoopIndices, pXYZ,
                    &localToWorld, &worldToLocal,
                    pPoints, numPoint, xyTolerance, bSignedOneBasedIndices, maxPerFace);
        }

    return status;
    }

/*-----------------------------------------------------------------*//**
* Triangulate a single space polygon. Best effort to handle non-planar polygons.
* @param pSignedOneBasedIndices <= array of indices.  Each face is added
*           as one-based indices, followed by a 0 (terminator).
*           Interior edges are optionally negated.
* @param pointP => array of polygon points.
* @param numPoint => number of polygon points.
* @param xyTolerance => tolerance for short edges on input polygon.
* @param signedOneBasedIndices => if false, output indices are 0-based,
*           with -1 as separator.  If true, indcies are one-based, and interior
*           edges are negated, with 0 as separator.
* @return ERROR if nonsimple polygon.
+----------------------------------------------------------------------*/
Public StatusInt  vu_triangulateSpacePolygon
(
EmbeddedIntArray    *pIndices,
DPoint3d            *pPoints,
int                 numPoint,
double              xyTolerance,
int                 maxPerFace,                 /* not used */
bool                signedOneBasedIndices
)
    {
    return vu_triangulateSpacePolygonExt2 (pIndices, NULL, NULL,
                NULL, NULL,
                pPoints, numPoint, xyTolerance, signedOneBasedIndices);
    }


/*-----------------------------------------------------------------*//**
* Triangulate a single space polygon.  Best effort to handle non-planar polygons.  Optionally handle self-intersecting polygons.
* @param pIndices               <=  array of output indices.  The value of bSignedOneBasedIndices determines the separator of each face loop
*                                   and whether or not interior edges are negated.
* @param pXYZ                   <=  array of output points, or NULL to disallow adding points at crossings.
* @param pointP                 =>  array of input polygon points.
* @param numPoint               =>  number of polygon points.
* @param xyTolerance            =>  tolerance for short edges on input polygon.
* @param bSignedOneBasedIndices =>  if false, output indices are 0-based, with -1 as separator.  If true, indices are 1-based, and
*                                   interior edges are negated, with 0 as separator.
* @return ERROR if nonsimple polygon.
* @bsimethod                                    DavidAssaf      08/03
+----------------------------------------------------------------------*/
Public StatusInt vu_triangulateSpacePolygonExt
(
EmbeddedIntArray*       pIndices,
EmbeddedDPoint3dArray*  pXYZ,
DPoint3d*               pPoints,
int                     numPoint,
double                  xyTolerance,
bool                    bSignedOneBasedIndices
)
    {
    return vu_triangulateSpacePolygonExt2
                (
                pIndices,
                NULL,
                pXYZ,
                NULL,
                NULL,
                pPoints, numPoint,
                xyTolerance,
                bSignedOneBasedIndices
                );
    }

/*-----------------------------------------------------------------*//**
* @description Adjust the triangulation indices returned by ~mvu_triangulateSpacePolygon to match the orientation of the
*       original polygon.
* @remarks Although the triangles returned by ~mvu_triangulateSpacePolygon will have consistent orientation, they are not
*       guaranteed to have the same orientation as the input polygon.
* @remarks The original polygon is ASSUMED to be non-self-intersecting.  This function may incorrectly decide to reorient
*       the triangulation if the original polygon is self-intersecting.
* @param pIndices               <= array of triangulation indices, formatted as per bSignedOneBasedIndices
* @param pbReversed             <= true if orientation was reversed; false if orientation unchanged (can be NULL).
* @param bSignedOneBasedIndices => If true, indices are one-based, interior edges are negated, and 0 is the separator;
*                                  if false, indices are 0-based and -1 is the separator.
* @return ERROR if invalid index array
* @bsimethod                                    DavidAssaf      10/01
+----------------------------------------------------------------------*/
Public StatusInt  vu_reorientTriangulationIndices
(
EmbeddedIntArray    *pIndices,
bool                *pbReversed,
bool                bSignedOneBasedIndices
)
    {
    int*    pInt = jmdlEmbeddedIntArray_getPtr (pIndices, 0);
    int     i, t, n = jmdlEmbeddedIntArray_getCount (pIndices), test, succ, term = bSignedOneBasedIndices ? 0 : -1, nInc, nDec;

    if (pbReversed)
        *pbReversed = false;

    // assume triangles
    if (n % 4)
        return ERROR;

    // count original edges in original (increasing) and reversed (decreasing) orientations
    for (i = nInc = nDec = 0; i < n; i++)
        {
        if ((test = pInt[i]) > term)
            {
            succ = (((i+1) % 4) == 3) ? pInt[i - 2] : pInt[i + 1];
            if (succ < 0)
                succ = -succ;

            if (succ == test + 1)
                nInc++;
            else if (succ == test - 1)
                nDec++;
            }
        }

    // TR #191917: We used to stop at the first original edge found, however when the original edge is dangling, it may legitimately occur
    //             in the triangulation with reversed orientation (e.g., without requiring reorientation).  Without comparing vertices, we
    //             can only use a heuristic to make the decision to reorient given the possibility of dangling edges.
    if (nInc >= nDec)
        return SUCCESS;

    // rotate signs
    if (bSignedOneBasedIndices)
        {
        for (i = 0; i < n; i+=4)
            {
            if (pInt[i] < 0)
                {
                if (pInt[i+1] < 0)
                    {
                    if (pInt[i+2] < 0)
                        {
                        // all negative => no rotation
                        }
                    else
                        {
                        pInt[i]   *= -1;
                        pInt[i+2] *= -1;
                        }
                    }
                else
                    {
                    if (pInt[i+2] < 0)
                        {
                        pInt[i+1] *= -1;
                        pInt[i+2] *= -1;
                        }
                    else
                        {
                        pInt[i]   *= -1;
                        pInt[i+1] *= -1;
                        }
                    }
                }
            else
                {
                if (pInt[i+1] < 0)
                    {
                    if (pInt[i+2] < 0)
                        {
                        pInt[i]   *= -1;
                        pInt[i+1] *= -1;
                        }
                    else
                        {
                        pInt[i+1] *= -1;
                        pInt[i+2] *= -1;
                        }
                    }
                else
                    {
                    if (pInt[i+2] < 0)
                        {
                        pInt[i]   *= -1;
                        pInt[i+2] *= -1;
                        }
                    else
                        {
                        // all positive => no rotation
                        }
                    }
                }
            }
        }

    // reverse triangle orientations
    for (i = 0; i < n; i+=4)
        {
        t = pInt[i+1];
        pInt[i+1] = pInt[i+2];
        pInt[i+2] = t;
        }

    if (pbReversed)
        *pbReversed = true;

    return SUCCESS;
    }

/*-----------------------------------------------------------------*//**
* Perform a boolean union operation and return all true outer (negative area)
*   faces.
* Input and output are structured as packed arrays of coordinates with parallel count
*   arrays.
* Replication of final point in inputs optional in inputs.
* @param opCode = 0 for union of all loops, 1 for intersection of all loops
+----------------------------------------------------------------------*/
Public void             vu_polygonBoolean
(
EmbeddedDPoint3dArray   *pOutputXYZ,
EmbeddedIntArray        *pOutputCounts,
EmbeddedDPoint3dArray   *pInputXYZ,
EmbeddedIntArray        *pInputCounts,
bool                    duplicateFirstPointInOutputLoops,
int                     opCode
)
    {
    int totalXYZCount = jmdlEmbeddedDPoint3dArray_getCount (pInputXYZ);
    int iLoop;
    int numThisLoop;
    int xyzStart;
    DPoint3d *pXYZBuffer;
    VuSetP      pGraph = vu_newVuSet (0);
    VuArrayP    pFaceArray = vu_grabArray (pGraph);
    VuP         pFace;
    DPoint3d xyz0, xyz;
    VuStackOpFuncP vuOpFunc = vu_orLoops;
    if (opCode == 1)
        vuOpFunc = vu_andLoops;

    for (iLoop = 0, xyzStart = 0;
         jmdlEmbeddedIntArray_getInt (pInputCounts, &numThisLoop, iLoop);
         iLoop++, xyzStart += numThisLoop)
        {
        if (   numThisLoop > 2
            && xyzStart < totalXYZCount
            && xyzStart + numThisLoop <= totalXYZCount
            )
            {
            pXYZBuffer = jmdlEmbeddedDPoint3dArray_getPtr (pInputXYZ, xyzStart);
            /* Save the earlier graph ... */
            if (iLoop > 0)
                vu_stackPush (pGraph);
            /* Enter this loop, resolving criss-cross by parity rules */
            vu_makeLoopFromArray3d (pGraph, pXYZBuffer, numThisLoop, true, true);
            vu_mergeLoops (pGraph);

            vu_regularizeGraph (pGraph);
            vu_markAlternatingExteriorBoundaries(pGraph,true);
            /* Merge with prior loops */
            if (iLoop > 0)
                vu_stackPopWithOperation (pGraph, vuOpFunc, NULL);
            }
        }

    pFaceArray = vu_grabArray (pGraph);
    vu_collectInteriorFaceLoops (pFaceArray, pGraph);
    vu_arrayOpen (pFaceArray);

    for (;vu_arrayRead (pFaceArray, &pFace);)
        {
        numThisLoop = 0;
        vu_getDPoint3d (&xyz0, pFace);

        VU_FACE_LOOP (pNode, pFace)
            {
            vu_getDPoint3d (&xyz, pNode);
            jmdlEmbeddedDPoint3dArray_addDPoint3d (pOutputXYZ, &xyz);
            numThisLoop++;
            }
        END_VU_FACE_LOOP (pNode, pFace)
        /* Replicated first point */
        if (duplicateFirstPointInOutputLoops)
            {
            numThisLoop++;
            jmdlEmbeddedDPoint3dArray_addDPoint3d (pOutputXYZ, &xyz0);
            }
        jmdlEmbeddedIntArray_addInt (pOutputCounts, numThisLoop);
        }

    vu_returnArray (pGraph, pFaceArray);
    vu_freeVuSet (pGraph);
    }

DRange3d RangeWithDisconnects (DPoint3dCP points, size_t n)
    {
    DRange3d range;
    range.Init ();
    for (size_t i = 0; i < n; i++)
        {
        if (!points[i].IsDisconnect())
            range.Extend (points[i]);
        }
    return range;
    }

static void UpdateNumNeeded (int &num0, double d, double h)
    {
    if (h <= 0.0)
        return;
    int numNeeded = (int)(0.999 + fabs (d) / h);
    if (numNeeded > num0)
        num0 = numNeeded;
    }

static int NumEdgeNeeded (DPoint3dCR xyz0, DPoint3dCR xyz1, double h, double hx, double hy)
    {
    int numEdge = 1;
    if (h > 0.0)
        UpdateNumNeeded (numEdge, xyz0.Distance (xyz1), h);
    UpdateNumNeeded (numEdge, xyz1.x - xyz0.x, hx);
    UpdateNumNeeded (numEdge, xyz1.y - xyz0.y, hy);
    return numEdge;
    }


Public int vu_splitLongEdges (VuSetP graphP, double maxEdgeLength, int maxSubEdge)
    {
    return vu_splitLongEdges (graphP, maxEdgeLength, 0.0, 0.0, maxSubEdge);
    }

Public int vu_splitLongEdges (VuSetP graphP, double maxEdgeLength, double maxEdgeXLength, double maxEdgeYLength, int maxSubEdge)
    {
    return vu_splitLongEdges (graphP, VU_BOUNDARY_EDGE, maxEdgeLength, 0.0, 0.0, maxSubEdge);
    }

Public int vu_splitLongEdges (VuSetP graphP, VuMask candidateMask, double maxEdgeLength, double maxEdgeXLength, double maxEdgeYLength, int maxSubEdge)
    {
    if (maxEdgeLength <= 0.0 && maxEdgeXLength <= 0.0 && maxEdgeYLength <= 0.0)
        return 0;
    static int s_maxSubEdge = 50;
    if (maxSubEdge > s_maxSubEdge)
        maxSubEdge = s_maxSubEdge;
    int numSplit = 0;
    VuArrayP candidates = vu_grabArray (graphP);
    VuMask visitMask = vu_grabMask (graphP);

    // Gather all long edges ...
    vu_arrayClear (candidates);
    vu_clearMaskInSet (graphP, visitMask);

    VU_SET_LOOP (currP, graphP)
        {
        if (vu_getMask (currP, candidateMask)
            && !vu_getMask (currP, visitMask))
            {
            vu_setMask (currP, visitMask);
            vu_setMask (vu_edgeMate (currP), visitMask);
            DPoint3d xyz0, xyz1;
            vu_getDPoint3d (&xyz0, currP);
            vu_getDPoint3d (&xyz1, vu_fsucc (currP));
            if (NumEdgeNeeded (xyz0, xyz1, maxEdgeLength, maxEdgeXLength, maxEdgeYLength) > 1)
                {
                vu_arrayAdd (candidates, currP);
                }
            }        
        }
    END_VU_SET_LOOP (currP, graphP)

    // Split long edges ....
    VuP baseP;
    for (vu_arrayOpen (candidates); vu_arrayRead (candidates, &baseP);)
        {
        DPoint3d xyz0, xyz1;
        vu_getDPoint3d (&xyz0, baseP);
        vu_getDPoint3d (&xyz1, vu_fsucc (baseP));
        int numSpan = NumEdgeNeeded (xyz0, xyz1, maxEdgeLength, maxEdgeXLength, maxEdgeYLength);
        if (numSpan > maxSubEdge)
            numSpan = maxSubEdge;
        for (int i = numSpan - 1; i > 0; i--)
            {
            double f = (double) i / (double) numSpan;
            DPoint3d xyz;
            xyz.Interpolate (xyz0, f, xyz1);
            VuP leftP, rightP;
            vu_splitEdge (graphP, baseP, &leftP, &rightP);
            numSplit++;
            vu_setDPoint3d (leftP, &xyz);
            vu_setDPoint3d (rightP, &xyz);
            }
        }
    vu_returnArray (graphP, candidates);
    vu_returnMask (graphP, visitMask);
    return numSplit;
    }



static bool IsDiagonalCandidate (VuSetP graphP, VuP nodeP, VuMask candidateBarrierMask, VuMask faceBarrierMask, double rightAngleVariation, double symmetricVariationRadians)
    {
    VuP mateP = vu_edgeMate (nodeP);
    if (vu_getMask (mateP, candidateBarrierMask))
        return false;
    if (vu_getMask (nodeP, candidateBarrierMask))
        return false;

    if (vu_findMaskAroundFace (nodeP, faceBarrierMask))
        return false;
    if (vu_findMaskAroundFace (mateP, faceBarrierMask))
        return false;

    if (vu_countEdgesAroundFace (nodeP) != 3)
        return false;
    if (vu_countEdgesAroundFace (mateP) != 3)
        return false;

    VuP node[5];
    node[0] = vu_fsucc (nodeP);
    node[1] = vu_fsucc (node[0]);
    node[2] = vu_fsucc (mateP);
    node[3] = vu_fsucc (node[2]);
    node[4] = node[0];
    DPoint3d points[5];
    DVec3d  vectors[5];
    for (int i = 0; i < 4; i++)
        vu_getDPoint3d (&points[i], node[i]);
    points[4] = points[0];
    for (int i = 0; i < 4; i++)
        vectors[i].DifferenceOf (points[i+1], points[i]);
    vectors[4] = vectors[0];
    double angle[4];
    double residual [4];
    double maxResidual = 0.0;
    for (int i = 0; i < 4; i++)
        {
        angle[i] = atan2
                    (
                    vectors[i].CrossProductXY (vectors[i+1]),
                    vectors[i].DotProductXY(vectors[i+1])
                    );
        residual[i] = fabs (angle[i] - msGeomConst_piOver2);
        if (residual[i] > maxResidual)
            maxResidual = residual[i];
        }

    // Are all angles near to 90 degrees?
    if (maxResidual <= rightAngleVariation)
        return true;

    // Are opposing angles similar enought to make it symmetric?
    if (fabs (angle[0] - angle[1]) < symmetricVariationRadians
        && fabs (angle[2] - angle[3]) < symmetricVariationRadians)
        return true;

    // Are opposing angles similar enought to make it symmetric?
    if (fabs (angle[1] - angle[2]) < symmetricVariationRadians
        && fabs (angle[3] - angle[0]) < symmetricVariationRadians)
        return true;
    return false;
    }

static void vu_removeQuadDiagonals (VuSetP graphP, double rightAngleVariationRadians, double symmetricVariationRadians)
    {
    VuMask visitMask = vu_grabMask (graphP);
    VuMask deleteMask = vu_grabMask (graphP);
    VuMask barrierMask = VU_EXTERIOR_EDGE | VU_BOUNDARY_EDGE | visitMask | deleteMask;
    vu_clearMaskInSet (graphP, visitMask | deleteMask);
    VU_SET_LOOP (candidateP, graphP)
        {
        VuP mateP = vu_edgeMate (candidateP);
        if (IsDiagonalCandidate (graphP, candidateP, barrierMask, deleteMask, rightAngleVariationRadians, symmetricVariationRadians))
            {
            // Mark both sides for deletion.
            // Mark the entire faces visited.
            vu_setMask (candidateP, deleteMask);
            vu_setMask (mateP, deleteMask);
            vu_setMaskAroundFace (candidateP, visitMask);
            vu_setMaskAroundFace (mateP, visitMask);
            }
        vu_setMask (candidateP, visitMask);
        vu_setMask (mateP, visitMask);
        }
    END_VU_SET_LOOP (candidateP, graphP)

    vu_freeMarkedEdges (graphP, deleteMask);
    vu_returnMask (graphP, visitMask);
    vu_returnMask (graphP, deleteMask);    
    }

/*-----------------------------------------------------------------*//**
* Triangulate an xy polygon (optionally with DISCONNECT-deliminted hole loops), 
* with interior points added on a grid as needed.
* @param [out] pIndices signed, one-based indices for interior polygons
* @param [out] pXYZOut  output points.
* @param [in] pointP  array of polygon points.
* @param [in] pVisible optional array of visibility flags.  
* @param [in] numPoint  number of polygon points.
* @param [in] maxEdgeLength  length for edge subdivision. (Zero to ignore.)
* @param [in] meshXLength  grid size in x direction. 
* @param [in] meshYLength  grid size in y direction.
* @param [in] smoothTriangulation true to adjust mesh vertices to get smooth flow lines in the triangulation.
* @param [in] radiansTolForRemovingQuadDiagonals tolerance for removing edges to form quads.
* @param [in] meshOrigin optional mesh origin.
* @returns ERROR if internal limits on mesh counts were enforced.
* @remarks The smoothTriangulation and quad diagonal removal are performed in sequence.  Both steps are permitted,
*       but in practice callers that want "highly rectangular" quads will NOT enable triangle smoothing, and
*       callers that want "smooth triangles" will disable the quad diagonal step.
+----------------------------------------------------------------------*/
Public StatusInt vu_subtriangulateXYPolygonExt
(
EmbeddedIntArray    *pIndices,
EmbeddedDPoint3dArray *pXYZOut,
DPoint3d            *pPoints,
bool                *pVisible,
int                 numPoint,
double              maxEdgeLength,
double              meshXLength,
double              meshYLength,
bool             smoothTriangulation,
double              radiansTolForRemovingQuadDiagonals,
DPoint3d            *meshOrigin
)
    {
    // (Duplicates code in bspSmoothPolyface.cpp vu_createTriangulated -- returns graph as result)
    static int s_maxEdge = 400;
    static int s_maxSubEdge = 50;
    static double s_defaultCount = 10.0;
    VuSetP      graphP = vu_newVuSet (0);
    VuArrayP    faceArrayP = vu_grabArray (graphP);
    VuP         faceP;
    StatusInt status = SUCCESS;
    int       i, numError;
    int     separator = 0;
    int    maxPerFace = 3;
    static double s_shortEdgeToleranceFactor = 1.0e-8;
    static double s_skewAngle = 0.0;
    double skewAngle = s_skewAngle;
    if (smoothTriangulation)
        skewAngle = msGeomConst_pi / 6.0;

    DRange3d range = RangeWithDisconnects (pPoints, numPoint);
    double xyTolerance = s_shortEdgeToleranceFactor * range.low.Distance (range.high);
    VuMask visitMask = vu_grabMask(graphP);


    jmdlEmbeddedIntArray_empty (pIndices);
    jmdlEmbeddedDPoint3dArray_empty (pXYZOut);

    VuMask newNodeMask = VU_BOUNDARY_EDGE;
    VuMask secondaryMask = VU_RULE_EDGE;
    vu_makeIndexedLoopFromArrayWithDisconnects (graphP, pPoints, pVisible, numPoint,
                newNodeMask, newNodeMask, secondaryMask, xyTolerance, DISCONNECT);
    double dx = range.high.x - range.low.x;
    double dy = range.high.y - range.low.y;
    if (meshXLength <= 0.0)
        meshXLength = dy / s_defaultCount;
    if (meshYLength <= 0.0)
        meshYLength = dy / s_defaultCount;

    if (s_maxEdge * meshXLength < dx)
        {
        status = ERROR;
        meshXLength = dx / s_maxEdge;
        }
    if (s_maxEdge * meshYLength < dy)
        {
        status = ERROR;
        meshYLength = dy / s_maxEdge;
        }
    
    // Aug 19, 2014
    // EDL
    // Playing with
    //    s_shiftA1 == "direct" expansion of upper counter.
    //    s_gridScale = grid stepping factor.  Seems necessary to compensate for skew.
    // I'm going with (2,1.6).  This seems excessive, and for smooth borders it indeed makes
    //    grids just a bit small.
    // But in a 90 degree corner, less agressive choices like (1,1.4) leave a "long diagonal" in the corner.
    // Fixing the corner needs a way to make the "non-interfering grid" snuggle into the corners better.
    DPoint3d origin = NULL != meshOrigin ? *meshOrigin : range.low;
    double ax0, ay0, ax1, ay1;
    DoubleOps::SafeDivide (ax0, range.low.x - origin.x, meshXLength, 0.0);
    DoubleOps::SafeDivide (ay0, range.low.y - origin.y, meshYLength, 0.0);
    int ix0 = (int) floor (ax0);
    int iy0 = (int) floor (ay0);
    static double s_shiftA1 = 2.0;
    DoubleOps::SafeDivide (ax1, range.high.x - origin.x, meshXLength, 0.0);
    DoubleOps::SafeDivide (ay1, range.high.y - origin.y, meshYLength, 0.0);
    int ix1 = (int)ceil (ax1 + s_shiftA1);
    int iy1 = (int)ceil (ay1 + s_shiftA1);

#ifdef DIRECT_GRID
    DSegment3d lineA, lineB;
    lineA.point[0].init (range.low.x + ix0 * meshXLength, range.low.y + iy0 * meshYLength, range.low.z);
    lineA.point[1].init (range.low.x + ix1 * meshXLength, range.low.y + iy0 * meshYLength, range.low.z);
    lineB.point[0].init (range.low.x + ix0 * meshXLength, range.low.y + iy1 * meshYLength, range.low.z);
    lineB.point[1].init (range.low.x + ix1 * meshXLength, range.low.y + iy1 * meshYLength, range.low.z);

    // Build non-boundary vertical linework of grid.
    for (int ix = ix0; ix <= ix1; ix++)
        {
        DPoint3d xyzA, xyzB;
        double fx = (double)(ix-ix0) / (double)(ix1- ix0);
        xyzA.Interpolate (*(&lineA.point[0]), fx, *(&lineA.point[1]));
        xyzB.Interpolate (*(&lineB.point[0]), fx, *(&lineB.point[1]));
        VuP nodeA, nodeB;
        vu_makePair (graphP, &nodeA, &nodeB);
        vu_setDPoint3d (nodeA, &xyzA);
        vu_setDPoint3d (nodeB, &xyzB);
        for (int iy = iy0 + 1; iy < iy1; iy++)
            {
            double fy = (double)(iy - iy0) / (double)(iy1 - iy0);
            DPoint3d xyz;
            xyz.Interpolate (xyzA, fy, xyzB);
            VuP nodeC, nodeD;
            vu_splitEdge (graphP, nodeB, &nodeC, &nodeD);
            vu_setDPoint3d (nodeC, &xyz);
            vu_setDPoint3d (nodeD, &xyz);
            }
        }
    vu_mergeOrUnionLoops (graphP, VUUNION_UNION);

    vu_splitLongEdges (graphP, maxEdgeLength, s_maxSubEdge);

    vu_regularizeGraph (graphP);
    vu_markAlternatingExteriorBoundaries(graphP,true);
    vu_splitMonotoneFacesToEdgeLimit (graphP, maxPerFace);
#else
    vu_mergeOrUnionLoops (graphP, VUUNION_UNION);

    vu_splitLongEdges (graphP, maxEdgeLength, s_maxSubEdge);
    vu_regularizeGraph (graphP);
    vu_markAlternatingExteriorBoundaries(graphP,true);
    vu_freeNonMarkedEdges (graphP, VU_BOUNDARY_EDGE);

    if (skewAngle != 0.0)
        {
        Transform worldToLocal, localToWorld;
        localToWorld.InitFromRowValues (
                    1, sin(skewAngle), 0, 0,
                    0, cos(skewAngle), 0, 0,
                    0, 0,              1, 0);
        int numX = ix1 - ix0;
        int numY = iy1 - iy0;
        static double s_gridScale = 1.60;
        numX = (int) (ceil (s_gridScale * numX));
        numY = (int) (ceil (s_gridScale * numY));
        localToWorld.SetFixedPoint(range.low);
        worldToLocal.InverseOf (localToWorld);
        vu_transform (graphP, &worldToLocal);
        vu_buildNonInterferingGrid (graphP, numX, numY, 0, 0);
        vu_transform (graphP, &localToWorld);
        }
    else
        {
        vu_buildNonInterferingGrid (graphP, ix1-ix0, iy1-iy0, 0, 0);
        }
    vu_setZInGraph (graphP, range.low.z);

    vu_regularizeGraph (graphP);
    vu_markAlternatingExteriorBoundaries(graphP,true);
    vu_splitMonotoneFacesToEdgeLimit (graphP, maxPerFace);
#endif
    if (maxPerFace == 3)
        vu_flipTrianglesToImproveQuadraticAspectRatio (graphP);
// (End of near duplicate code in bspSmoothPolyface.cpp)
    static double s_symmetryFactor = 0.75;
    static double s_smoothTol = 0.001;
    static int s_maxSmoothPass = 100;
    static int s_flipInterval = 4;
    double shiftFraction;
    int numSweep;

    if (smoothTriangulation)
        vu_smoothInteriorVertices (graphP, s_smoothTol, s_maxSmoothPass,s_flipInterval, &shiftFraction, &numSweep);
    else if (radiansTolForRemovingQuadDiagonals > 0.0)
        vu_removeQuadDiagonals (graphP, radiansTolForRemovingQuadDiagonals, radiansTolForRemovingQuadDiagonals * s_symmetryFactor);

    vu_clearMaskInSet (graphP, visitMask);
    VU_SET_LOOP (currNode, graphP)
        {
        vu_setUserDataPAsInt (currNode, -1);
        }
    END_VU_SET_LOOP (currNode, graphP)
    // Assign vertex numbers.
    VU_SET_LOOP (currNode, graphP)
        {
        if (!vu_getMask (currNode, VU_EXTERIOR_EDGE)
            && !vu_getMask (currNode, visitMask))
            {
            vu_setMaskAroundVertex (currNode, visitMask);
            DPoint3d xyz;
            vu_getDPoint3d (&xyz, currNode);
            int vertexCount = jmdlEmbeddedDPoint3dArray_getCount (pXYZOut);
            jmdlEmbeddedDPoint3dArray_addDPoint3d (pXYZOut, &xyz);
            VU_VERTEX_LOOP (vertexNode, currNode)
                {
                vu_setUserDataPAsInt (vertexNode, vertexCount);
                }
            END_VU_VERTEX_LOOP (vertexNode, currNode)
            }
        }
    END_VU_SET_LOOP (currNode, graphP)
    vu_collectInteriorFaceLoops (faceArrayP, graphP);
    numError = 0;
    VuMask visibleMask = (NULL == pVisible ? VU_BOUNDARY_EDGE : secondaryMask);
    vu_arrayOpen (faceArrayP);
    for (i = 0; vu_arrayRead (faceArrayP, &faceP); i++)
        {
        // We triangulated.  So of course there are 3 nodes per face.
        // Really?  If the input polygon retraces itself, there will be
        // sliver faces with only 2 edges.  
        if (vu_faceLoopSize (faceP) > 4)
            continue;
            
        VU_FACE_LOOP (currP, faceP)
            {
            int index = vu_getUserDataPAsInt (currP);
            int outputIndex = 1 + index;
            //bool visible = false;
            if (!vu_getMask (currP, visibleMask))
                outputIndex = - outputIndex;
            jmdlEmbeddedIntArray_addInt (pIndices, outputIndex);
            }
        END_VU_FACE_LOOP (currP, faceP)
        jmdlEmbeddedIntArray_addInt (pIndices, separator);
        }

    vu_returnMask (graphP, visitMask);
    vu_returnArray (graphP, faceArrayP);
    vu_freeVuSet (graphP);

    return status;
    }
Public StatusInt vu_subtriangulateXYPolygon
(
EmbeddedIntArray    *pIndices,
EmbeddedDPoint3dArray *pXYZOut,
DPoint3d            *pPoints,
int                 numPoint,
double              maxEdgeLength,
double              meshXLength,
double              meshYLength,
bool             smoothTriangulation,
double              radiansTolForRemovingQuadDiagonals,
DPoint3d            *meshOrigin
)
    {
#ifdef TESTSETUP_makeEveryOtherEdgeVisible
    bool testFlags[MAX_VERTICES];
    for (int i = 0; i < numPoint; i++)
        testFlags[i] = (0 != (i & 0x01));
#endif
    return vu_subtriangulateXYPolygonExt (pIndices, pXYZOut, pPoints, NULL, numPoint,
                    maxEdgeLength, meshXLength, meshYLength,
                    smoothTriangulation,
                    radiansTolForRemovingQuadDiagonals,
                    meshOrigin);
    }
	
END_BENTLEY_GEOMETRY_NAMESPACE	
