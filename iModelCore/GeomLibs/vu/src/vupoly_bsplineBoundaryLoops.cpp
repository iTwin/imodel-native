/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @description Remove all edges in a graph, except those with the given mask on exactly one side.
* @param graphP IN OUT  graph header
* @param mask   IN      mask to query
* @group "VU Edges"
* @bsimethod                                                    EarlinLutz      8/96
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_removeAllButSingleMarkedEdges
(
VuSetP  graphP,
VuMask  mask
)
    {
    int mateFlag;

    VuP mateP;
    VuMask myMask = vu_grabMask (graphP);
    vu_clearMaskInSet (graphP, myMask);

    VU_SET_LOOP(currP, graphP)
        {
        mateP = vu_edgeMate (currP);
        mateFlag = vu_getMask (mateP, mask);
        if (vu_getMask (currP, mask))
            {
            if (mateFlag)
                {
                // doubly-marked edge
                vu_setMask (currP, myMask);
                vu_setMask (mateP, myMask);
                }
            }
        else
            {
            if (!mateFlag)
                {
                // unmarked edge
                vu_setMask (currP, myMask);
                vu_setMask (mateP, myMask);
                }
            }
        }
    END_VU_SET_LOOP (currP, graphP)

    vu_freeMarkedEdges (graphP, myMask);
    vu_returnMask (graphP, myMask);
    }


typedef struct
    {
    BsurfBoundary *boundP;
    int     *parentIdP;
    int     numBounds;
    int     direction;
    bool    duplicateStart;
    } BoundaryArray;

typedef StatusInt (*VuFaceOutputFunction)
(
VuSetP          graphP,             /* => containing graph */
VuP             startP,             /* => start node on face */
int             id,                 /* => loop index */
int             totalLoops,         /* => total loops to come */
int             parentId,           /* => id of containing outer loop */
BoundaryArray   *contextP,          /* => header for output collection */
VuMask          seamJumpMask        /* => mask for seam jump edges. */
);


/*----------------------------------------------------------------------+
|                                                                       |
| These functions are a convenience wrapper around common vu graph      |
| operations.                                                           |
|                                                                       |
| Typical use for Bspline loop simplification is:                       |
|                                                                       |
|   -- Initialize:                                                      |
|                                                                       |
|   VuSetP graphP = vu_newVuSet (0)                                     |
|                                                                       |
|                                                                       |
|   -- One or more calls to add loops to the graph:                     |
|       vu_addDPoint2dLoop (graphP, pointP, numPoint)                   |
|       vu_addBsurfBoundaries (graphP, boundsP, numBounds)              |
|       vu_addRange (graphP, xmin, ymin, xmax, ymax)                    |
|                                                                       |
|   -- One or more data reduction steps:                                |
|       vu_markConnectedParityGraph (graphP);                           |
|       vu_breakCompoundVertices (graphP, VU_EXTERIOR_EDGE);            |
|                                                                       |
|   -- Output:                                                          |
|       vu_extractBsurfLoops (graphP, &boundsP, &parentIdP, &numBounds, |
|                   duplicateStart, direction)                          |
|   -- Cleanup:                                                         |
|                                                                       |
|   vu_freeGraph (graphP)                                               |
|                                                                       |
| Simplest use for general polygon-polygon clip:                        |
|                                                                       |
|   -- Initialize:                                                      |
|                                                                       |
|   VuSetP graphP = vu_newVuSet (0)                                     |
|                                                                       |
|   -- Install and analyze first polygon
|   vu_addDPoint2dLoop (graphP, pointP, numPoint)                       |
|   vu_markConnectedParityGraph (graphP)                                |

|                                                                       |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_addDPoint2dLoop                                      |
|                                                                       |
| author        EarlinLutz                              03/98           |
|                                                                       |
+----------------------------------------------------------------------*/
Public void vu_addDPoint2dLoop
(
VuSetP      graphP,
DPoint2d    *pointP,
int         numPoint
)
    {
    VuP faceP;
    faceP = vu_makeLoopFromArray (graphP, pointP, numPoint, true, true);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_addBsurfBoundaries                                   |
|                                                                       |
| author        EarlinLutz                              03/98           |
|                                                                       |
+----------------------------------------------------------------------*/
Public void vu_addBsurfBoundaries
(
VuSetP          graphP,
BsurfBoundary   *boundsP,
int             numBounds
)
    {
    int i;
    VuP faceP;
    for (i = 0; i < numBounds; i++ )
        {
        faceP = vu_makeLoopFromArray
                            (
                            graphP,
                            boundsP[i].points,
                            boundsP[i].numPoints,
                            true,
                            true
                            );
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_addRange                                             |
|                                                                       |
| author        EarlinLutz                              03/98           |
|                                                                       |
| Add a rectangular loop to the graph.                                  |
+----------------------------------------------------------------------*/
Public void vu_addRange
(
VuSetP          graphP,
double          xmin,
double          ymin,
double          xmax,
double          ymax
)
    {
    DPoint2d corner[4];
    VuP faceP;
    corner[0].x = corner[3].x = xmin;
    corner[1].x = corner[2].x = xmax;
    corner[0].y = corner[1].y = ymin;
    corner[2].y = corner[3].y = ymax;

    faceP = vu_makeLoopFromArray (graphP, corner, 4, true, true);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_collectConnectedComponent                            |
|                                                                       |
| author        EarlinLutz                              03/98           |
|                                                                       |
| Add a rectangular loop to the graph.                                  |
+----------------------------------------------------------------------*/
Public void vu_collectConnectedComponent
(
VuSetP          graphP,             /* <=> graph to search and mark */
VuArrayP        componentArrayP,    /* <= array of component members */
VuArrayP        stackP,             /* <=> work array */
VuP             startP,             /* => seed of component */
VuMask          mask                /* => mask to apply to visited nodes */

)
    {
    VuP currP = startP;
    VuP nextP;
    vu_arrayAdd (stackP, startP);

    while (NULL != (currP = vu_arrayRemoveLast (stackP)))
        {
        if  (!VU_GETMASK (currP, mask))
            {
            VU_SETMASK (currP, mask);
            vu_arrayAdd (componentArrayP, currP);

            nextP = VU_FSUCC (currP);
            if  (!VU_GETMASK (nextP, mask))
                vu_arrayAdd (stackP, nextP);
            nextP = VU_VSUCC (currP);

            if  (!VU_GETMASK (nextP, mask))
                vu_arrayAdd (stackP, nextP);
            }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_markConnectedParity                                  |
|                                                                       |
| author        EarlinLutz                              03/98           |
|                                                                       |
| Fixup loops with xor (parity) merge.   Eliminate "external"           |
| edges.                                                                |
+----------------------------------------------------------------------*/
Public void vu_markConnectedParity
(
VuSetP                  graphP          /* <=> analyzed graph.  Changed by analysis */
)
    {
    VuP mateP;
    VuMask deleteMask = vu_grabMask (graphP);

    /* Find all loop crossings */
    vu_mergeLoops (graphP);
    vu_regularizeGraph (graphP);
    vu_markAlternatingExteriorBoundaries(graphP,true);

    /* Mark and eliminate fully external edges */
    vu_clearMaskInSet (graphP, deleteMask );

    VU_SET_LOOP (currP, graphP)
        {
        mateP = VU_EDGE_MATE (currP);
        if  (VU_GETMASK (currP, VU_EXTERIOR_EDGE) && VU_GETMASK (mateP, VU_EXTERIOR_EDGE))
            {
            VU_SETMASK (currP, deleteMask);
            VU_SETMASK (mateP, deleteMask);
            }
        }
    END_VU_SET_LOOP (currP, graphP)

    vu_freeMarkedEdges (graphP, deleteMask);

    vu_returnMask (graphP, deleteMask);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_emitPolygons                                         |
|                                                                       |
| author        EarlinLutz                              03/98           |
|                                                                       |
| Output polygons from a graph in which exterior edges are properly     |
| marked.                                                               |
+----------------------------------------------------------------------*/
static void vu_emitPolygons
(
VuSetP                  graphP,         /* <=> analyzed graph.  Changed by analysis */
VuFaceOutputFunction    outputFunc,
BoundaryArray       *userDataP,
VuMask                  seamJumpMask
)
    {
    VuMask outputMask;
    VuMask visitMask;
    VuP     faceP, nodeP;
    VuArrayP faceArrayP;
    VuArrayP componentArrayP;
    VuArrayP stackArrayP;

    int totalFaceId, currFaceId, baseFaceId;

    visitMask = vu_grabMask (graphP);
    outputMask = vu_grabMask (graphP);

    faceArrayP = vu_grabArray (graphP);
    componentArrayP = vu_grabArray (graphP);
    stackArrayP = vu_grabArray (graphP);

    vu_clearMaskInSet (graphP, visitMask | outputMask);

    /* Assumption: Each connected component of the graph has only one exterior loop with negative area. */
    /* Masks are clear everywhere */

    vu_collectExteriorFaceLoops (faceArrayP, graphP);
    totalFaceId = vu_arraySize (faceArrayP);
    currFaceId = 0;

    for (   vu_arrayOpen (faceArrayP);
            vu_arrayRead (faceArrayP, &faceP);
        )
        {
        double area = vu_area (faceP);
        bool    nonBoundingSeamFace = false;
        if (seamJumpMask
            && vu_getMask (faceP, VU_EXTERIOR_EDGE)
            && (NULL != vu_findMaskAroundFace (faceP, seamJumpMask))
            )
            {
            nonBoundingSeamFace = true;
            }
        if  (!VU_GETMASK (faceP, visitMask) && (area < 0.0 || nonBoundingSeamFace))
            {
            /* Find all the nodes of this connected component */
            vu_collectConnectedComponent (
                        graphP, componentArrayP, stackArrayP, faceP, visitMask);

            baseFaceId = currFaceId;
            /* Output the main face */
            vu_markFace (faceP, outputMask);
            outputFunc (graphP, faceP, currFaceId++, totalFaceId, baseFaceId, userDataP, seamJumpMask);
            /* Output the subordinate faces */
            for (   vu_arrayOpen (componentArrayP);
                    vu_arrayRead (componentArrayP, &nodeP);
                )
                {
                if  (!VU_GETMASK (nodeP, outputMask))
                    {
                    vu_markFace (nodeP, outputMask);
                    if  (VU_GETMASK (nodeP, VU_EXTERIOR_EDGE))
                        {
                        outputFunc (graphP, nodeP, currFaceId++, totalFaceId, baseFaceId, userDataP, seamJumpMask);
                        }
                    }
                }
            }
        }

    vu_returnMask (graphP, visitMask        );
    vu_returnMask (graphP, outputMask       );

    vu_returnArray (graphP, componentArrayP );
    vu_returnArray (graphP, faceArrayP      );
    vu_returnArray (graphP, stackArrayP     );
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_breakCompoundVertices                                |
|                                                                       |
| author        EarlinLutz                              03/98           |
|                                                                       |
| Split vertex loops as needed so that no more than one outgoing edge   |
| has a given mask.                                                     |
| Usual use: call with VU_EXTERIOR_MASK so that multiple polygons that  |
| share a vertx become disconnected.                                    |
| Returns the number of splits performed.                               |
+----------------------------------------------------------------------*/
Public int vu_breakCompoundVertices
(
VuSetP          graphP,             /* <=> containing graph */
VuMask          mask
)
    {
    int numSplit = 0;
    VuMask visitMask = vu_grabMask (graphP);
    VuP nextP, currP;

    vu_clearMaskInSet (graphP, visitMask);
    VU_SET_LOOP (baseP, graphP)
        {
        if  (!VU_GETMASK(baseP, visitMask) && VU_GETMASK(baseP, mask))
            {
            vu_setMaskAroundFace (baseP, visitMask);
            for (currP = VU_VSUCC(baseP); currP != baseP ; currP = nextP)
                {
                nextP = VU_VSUCC(currP);
                if  (VU_GETMASK(currP, mask))
                    {
                    vu_vertexTwist (graphP, currP, baseP);
                    numSplit++;
                    }
                }
            }
        }
    END_VU_SET_LOOP (baseP, graphP)
    vu_returnMask (graphP, visitMask);
    return  numSplit;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_transverseSeamCoordinate                             |
|                                                                       |
| author        EarlinLutz                              07/01           |
|                                                                       |
| Return the value of the coordinate going transverse to the specified  |
| seam direction.                                                       |
+----------------------------------------------------------------------*/
Public double   vu_transverseSeamCoordinate
(
VuP             nodeP,
bool            isVerticalSeam      /* True for vertical seam (left and right),
                                        False for horizontal seam (top and bottom)*/
)
    {
    return isVerticalSeam ? VU_U(nodeP) : VU_V(nodeP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_seamCoordinate                                       |
|                                                                       |
| author        EarlinLutz                              07/01           |
|                                                                       |
| Return the value of the coordinate going along the specified seam     |
| direction.                                                            |
+----------------------------------------------------------------------*/
Public double   vu_seamCoordinate
(
VuP             nodeP,
bool            isVerticalSeam      /* True for vertical seam (left and right),
                                        False for horizontal seam (top and bottom)*/
)
    {
    return isVerticalSeam ? VU_V(nodeP) : VU_U(nodeP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_sameSeamCoordinates                                  |
|                                                                       |
| author        EarlinLutz                              07/01           |
|                                                                       |
| Compare seam coordinates (only) --- no test for transverse coordinate |
+----------------------------------------------------------------------*/
static bool    vu_sameSeamCoordinates
(
VuP             node0P,
VuP             node1P,
bool            isVerticalSeam      /* True for vertical seam (left and right),
                                        False for horizontal seam (top and bottom)*/
)
    {
    double a0 = vu_seamCoordinate (node0P, isVerticalSeam);
    double a1 = vu_seamCoordinate (node1P, isVerticalSeam);
    return a0 == a1;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_isExteriorSeamNode                                   |
|                                                                       |
| author        EarlinLutz                              07/01           |
|                                                                       |
| Test if nodeP is an exterior node on the specified seam.              |
+----------------------------------------------------------------------*/
Public bool     vu_isExteriorSeamNode
(
VuP             nodeP,
double          param,
bool            isVerticalSeam      /* True for vertical seam (left and right),
                                        False for horizontal seam (top and bottom)*/
)
    {
    double a = vu_transverseSeamCoordinate (nodeP, isVerticalSeam);
    if (!VU_GETMASK (nodeP, VU_EXTERIOR_EDGE))
        return false;
    return a == param;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_partnerOfHighSeedCandidate                           |
|                                                                       |
| author        EarlinLutz                              03/98           |
|                                                                       |
+----------------------------------------------------------------------*/
static VuP vu_partnerOfSeedCandidate
(
VuSetP graphP,
VuP     highP,
double  lowParam,
double  highParam,
bool    isVerticalSeam
)
    {

    if (   VU_GETMASK (highP, VU_EXTERIOR_EDGE)
        && vu_isExteriorSeamNode (highP, highParam, isVerticalSeam))
        {
        VU_SET_LOOP (lowP, graphP)
            {
            if (   vu_isExteriorSeamNode (lowP, lowParam, isVerticalSeam)
                && vu_sameSeamCoordinates (highP, lowP, isVerticalSeam)
               )
                {
                return lowP;
                }
            }
        END_VU_SET_LOOP (lowP, graphP)
        }
    return NULL;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_pasteSeams                                           |
|                                                                       |
| author        EarlinLutz                              03/98           |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      vu_pasteSeams
(
VuSetP          graphP,             /* <=> containing graph */
VuMask          seamJumpMask,       /* => Mask to apply to (both sides of)
                                                    edges which jump the seam. */

double          lowParam,
double          highParam,
bool            isVerticalSeam      /* True for vertical seam (left and right),
                                        False for horizontal seam (top and bottom)*/
)
    {
    int seamCount = 0;
    int numDelete = 0;
    VuP high1P, low1P;
    VuP high0P, low0P;
    VuP high2P, low2P;
    VuArrayP arrayP = vu_grabArray (graphP);
    VuMask deleteMask = vu_grabMask (graphP);
    vu_clearMaskInSet (graphP, deleteMask);

    /* Accumulate pairs of nodes to connect */
    VU_SET_LOOP (high1P, graphP)
        {
        low1P = vu_partnerOfSeedCandidate (graphP, high1P,lowParam, highParam, isVerticalSeam);
        if (low1P)
            {
            high0P = vu_fpred (high1P);
            high2P = vu_fsucc (high1P);
            low0P  = vu_fpred (low1P);
            low2P  = vu_fsucc (low1P);
            if (   !vu_isExteriorSeamNode (high0P, highParam, isVerticalSeam)
                &&  vu_isExteriorSeamNode (high2P, highParam, isVerticalSeam)
                &&  vu_isExteriorSeamNode (low0P, lowParam, isVerticalSeam)
                && !vu_isExteriorSeamNode (low2P, lowParam, isVerticalSeam))
                {
                vu_arrayAdd (arrayP, high1P);
                vu_arrayAdd (arrayP, low1P);
                seamCount++;
                }
            }
        }
    END_VU_SET_LOOP (high1P, graphP)


    /* Paste together in 2-edge structure.
       The pasted edges themselves may get removed later
       but this step ensures that the real edges get joined across
       the seam.
       */
    for (vu_arrayOpen (arrayP);
            vu_arrayRead (arrayP, &high1P)
         && vu_arrayRead (arrayP, &low1P);
         )
        {
        VuP newHighNodeP, newLowNodeP;
        VuP high0P = vu_fpred (high1P);
        VuP high2P = vu_fsucc (high1P);
        VuP low0P  = vu_fpred (low1P);
        VuP low2P  = vu_fsucc (low1P);

        /* Jump across top. */
        vu_join (graphP, high1P, low1P, &newHighNodeP, &newLowNodeP);
        vu_setMask (newHighNodeP, seamJumpMask);
        vu_setMask (newLowNodeP, seamJumpMask);
        vu_setMask (newHighNodeP, VU_EXTERIOR_EDGE);
            numDelete += 2;

        /* WE TRUST ... that there are matching coordinates in the seams.
                I think it is possible for islands against the seam to violate this.
                seam to violate this. */

        do
            {
            /* Skitter down the high side until the seam disappears */
            high1P = high2P;
            high0P = vu_fpred (high1P);
            high2P = vu_fsucc (high1P);
            vu_setMask (high0P, deleteMask);
            } while (vu_isExteriorSeamNode (high2P, highParam, isVerticalSeam));

        do
            {
            /* Skitter down the low side until the seam disappears */
            low1P = low0P;
            low0P = vu_fpred (low1P);
            low2P = vu_fsucc (low1P);
            vu_setMask (low1P,  deleteMask);
            } while (vu_isExteriorSeamNode (low0P, lowParam, isVerticalSeam));


        /* Jump across bottom */
        vu_join (graphP, high1P, low1P, &newHighNodeP, &newLowNodeP);
        vu_setMask (newHighNodeP, seamJumpMask);
        vu_setMask (newLowNodeP, seamJumpMask);
        vu_setMask (newLowNodeP, VU_EXTERIOR_EDGE);

        }


    if (numDelete > 0)
        vu_freeMarkedEdges (graphP, deleteMask);

    vu_returnArray (graphP, arrayP);
    vu_returnMask (graphP, deleteMask);
    return seamCount;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_addBoundaryLoop                                      |
|                                                                       |
| author        EarlinLutz                              03/98           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt vu_addBoundaryLoop
(
VuSetP          graphP,             /* => containing graph */
VuP             startP,             /* => start node on face */
int             id,                 /* => loop index */
int             totalLoops,         /* => total loops to come */
int             parentId,           /* => id of containing outer loop */
BoundaryArray   *contextP,          /* => header for output collection */
VuMask          seamJumpMask        /* => mask for seam jump edges. */
)
    {
    StatusInt status = ERROR;
    bool    duplicateStart = contextP->duplicateStart;
    bool    startingOnSeam = false;
    if  (id == 0 && totalLoops > 0)
        {
        contextP->boundP = (BsurfBoundary *)vumemfuncs_calloc (totalLoops, sizeof(BsurfBoundary));
        contextP->parentIdP = (int *)vumemfuncs_calloc (totalLoops, sizeof(int));

        if  (contextP->boundP && contextP->parentIdP)
            {
            contextP->numBounds = totalLoops;
            }
        }

    if  (id >= 0 && id < contextP->numBounds)
        {
        int numPoint = vu_countEdgesAroundFace (startP);
        int bufferPoints = numPoint;
        DPoint2d *pointP;

        if (seamJumpMask)
            {
            VuP newStartP = vu_findMaskAroundFace (startP, seamJumpMask);
            if (newStartP)
                {
                /* Start immediately after the seam jump, and do NOT copy the point --
                    the seam jump edge provides a geometric duplicate. */
                startP = vu_fsucc (newStartP);
                duplicateStart = false;
                startingOnSeam = true;
                }
            }

        /* Actual number points copied to buffer may be smaller due to seam jump edges. */
        if  (contextP->duplicateStart)
            {
            bufferPoints++;
            }

        pointP = (DPoint2d *)vumemfuncs_malloc
                            (bufferPoints * sizeof (DPoint2d));

        if  (pointP)
            {
            bool    isOuterLoop = id == parentId;
            double  outputSign = 1.0;
            int numOut;

            double area = vu_area (startP);

            contextP->boundP[id].points     = pointP;
            if  (contextP->parentIdP)
                contextP->parentIdP[id]     = parentId;

            if (seamJumpMask)
                {
                /* Umm... big assumption -- we are called with request to
                    output an "exterior" loop, which is oriented CC,
                    and seamJumpMask overrides any computed area.
                */
                area = -1.0;
                }

            switch (contextP->direction)
                {
                case 0:
                    outputSign = -1.0;
                    break;
                case 1:
                    outputSign = 1.0;
                    break;
                case 2:
                    outputSign = isOuterLoop ? 1.0 : -1.0;
                    break;
                case 3:
                    outputSign = isOuterLoop ? -1.0 : 1.0;
                    break;
                }

            if  (outputSign * area > 0.0)
                {
                numOut = 0;
                VU_FACE_LOOP (currP, startP)
                    {
                    pointP[numOut].x = VU_U(currP);
                    pointP[numOut].y = VU_V(currP);
                    numOut++;
                    }
                END_VU_FACE_LOOP (currP, startP)
                }
            else
                {
                numOut = 0;
                if (startingOnSeam)
                    startP = vu_fpred (startP);
                VU_REVERSE_FACE_LOOP (currP, startP)
                    {
                    pointP[numOut].x = VU_U(currP);
                    pointP[numOut].y = VU_V(currP);
                    numOut++;
                    }
                END_VU_REVERSE_FACE_LOOP (currP, startP)
                }


            if  (duplicateStart)
                {
                pointP[numOut] = pointP[0];
                numOut++;
                }
            contextP->boundP[id].numPoints  = numOut;

            status = SUCCESS;
            }
        }
    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_collectPolygons                                      |
|                                                                       |
| author        EarlinLutz                              03/98           |
|                                                                       |
| Collect parent-child polygons from a graph with full parity and       |
| and connectivity.                                                     |
+----------------------------------------------------------------------*/
Public void vu_collectPolygonsExt
(
VuSetP          graphP,
BsurfBoundary   **boundsPP,     /* <= array of loops, parent first then children */
int             **parentPP,     /* <= for the boundsPP[i], index of parent loop */
int             *numBoundsP,
bool            duplicateStart, /* true to have the start point
                                    duplicated at the end of each loop. */
int             direction,       /* => 0 for all CCW
                                      1 for all CW
                                      2 for all outer CCW, inner CW
                                      3 for outer CW, inner CCW
                                */
VuMask          seamJumpMask    /* mask to mark which jump seam. */
)
    {
    BoundaryArray boundaryArray;

    boundaryArray.numBounds         = 0;
    boundaryArray.boundP            = NULL;
    boundaryArray.direction         = direction;
    boundaryArray.duplicateStart    = duplicateStart;
    boundaryArray.parentIdP         = NULL;

    vu_emitPolygons (graphP, vu_addBoundaryLoop, &boundaryArray, seamJumpMask);

    *boundsPP = boundaryArray.boundP;
    *numBoundsP = boundaryArray.numBounds;

    if  (parentPP)
        {
        *parentPP = boundaryArray.parentIdP;
        }
    else
        vumemfuncs_free (boundaryArray.parentIdP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_collectPolygons                                      |
|                                                                       |
| author        EarlinLutz                              03/98           |
|                                                                       |
| Collect parent-child polygons from a graph with full parity and       |
| and connectivity.                                                     |
+----------------------------------------------------------------------*/
Public void vu_collectPolygons
(
VuSetP          graphP,
BsurfBoundary   **boundsPP,     /* <= array of loops, parent first then children */
int             **parentPP,     /* <= for the boundsPP[i], index of parent loop */
int             *numBoundsP,
bool            duplicateStart, /* true to have the start point
                                    duplicated at the end of each loop. */
int             direction       /* => 0 for all CCW
                                      1 for all CW
                                      2 for all outer CCW, inner CW
                                      3 for outer CW, inner CCW
                                */
)
    {
    vu_collectPolygonsExt (graphP, boundsPP, parentPP, numBoundsP,
                duplicateStart, direction, 0);
    }

Public void vu_collectInteriorLoopCoordinates
(
VuSetP graph,
bvector<bvector<DPoint2d> > &loops,
size_t minCount,
size_t numWrap
)
    {
    loops.clear ();
    DPoint2d xy;
    VuArrayP seeds = vu_grabArray (graph);
    VuP faceSeed;
    if (minCount == 0)
        minCount = 1;
    vu_collectInteriorFaceLoops (seeds, graph);
    for (vu_arrayOpen (seeds); vu_arrayRead (seeds, &faceSeed);)
        {
        if (vu_faceLoopSize (faceSeed) > (int)minCount)
            {
            loops.push_back (bvector<DPoint2d> ());
            VU_FACE_LOOP (corner, faceSeed)
                {
                vu_getDPoint2d (&xy, corner);
                loops.back ().push_back (xy);
                }
            END_VU_FACE_LOOP (corner, faceSeed)
            for (size_t i = 0; i < numWrap; i++)
                {
                loops.back ()[i];
                loops.back ().push_back (xy);
                }
            }
        }
    vu_returnArray (graph, seeds);
    }

Public void vu_fixupLoopParity
(
bvector <bvector <DPoint2d> > &outLoops,
bvector <bvector <DPoint2d> > &rawLoops,
size_t minCount,
size_t numWrap
)
    {
    VuSetP graph = vu_newVuSet (0);
    outLoops.clear ();
    for (size_t i = 0; i < rawLoops.size (); i++)
        {
        if (rawLoops[i].size () > 2)
            vu_makeLoopFromArray (graph, &rawLoops[i][0], (int)rawLoops[i].size (), true, true);
        }
    vu_mergeOrUnionLoops (graph, VUUNION_UNION);
    vu_regularizeGraph (graph);
    vu_markAlternatingExteriorBoundaries(graph,true);
    vu_removeAllButSingleMarkedEdges (graph, VU_EXTERIOR_EDGE);
    vu_collectInteriorLoopCoordinates (graph, outLoops, minCount, numWrap);
    vu_freeVuSet (graph);
    }

END_BENTLEY_GEOMETRY_NAMESPACE