/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/src/vuexcise.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#include <math.h>
/*----------------------------------------------------------------------+
| name          vu_detachDuplicateEdgesAtVertex                         |
| author        EarlinLutz                                      10/95   |
+----------------------------------------------------------------------*/
static void vu_detachDuplicateEdgesAtVertex
(
VuSetP          graphP,
VuP             startP,
VuMask          discardMask
)
    {
    VuP saveP;
    int edgeCount = 0;
    int deleteCount = 0;
    if (startP)
        {
        /* Find a node that is guaranteed to remain */
        saveP = NULL;
        VU_VERTEX_LOOP (currP, startP)
            {
            if (vu_countEdgesAroundFace(currP) > 2)
                {
                saveP = currP;
                }
            else
                {
                deleteCount++;
                }
            edgeCount++;
            }
        END_VU_VERTEX_LOOP (currP, startP)

        if (saveP)
            {
            /* No really easy way to stop guard this loop against
               the fact that parts of it (possibly the saveP!!)
               are disappearing under our feet.  Use some dumb counters.
            */
            edgeCount *= 3;
            VU_VERTEX_LOOP (currP, saveP)
                {
                VuP farP = VU_FSUCC(currP);
                if (VU_FSUCC(farP) == currP)
                    {
                    vu_detachEdge(graphP, farP);
                    vu_setMaskAroundFace (farP, discardMask);
                    saveP = currP;
                    deleteCount--;
                    }
                if (deleteCount <= 0 || edgeCount-- < 0)
                    break;
                }
            END_VU_VERTEX_LOOP (currP, saveP)
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Disconnect (small) edges from the graph and mark them.
* @remarks The callback determines which edges to disconnect, performs the incident vertex re-assignment, and signals whether or not
*       resulting degenerate faces should be disconnected as well.
* @remarks On return, the marked edges remain in the graph, but are disconnected from the graph.  This allows
*       the caller to mark more edges with this mask before deleting all masked edges at once.  Thus it is
*       assumed that on input, discardMask, if not cleared in the graph, has only been applied to both or neither
*       sides of each edge of the graph, and that any edges with this mask are disconnected from the rest of the graph.
* @param graphP         IN OUT  vu graph
* @param discardMask    IN      mask to apply to discarded edges
* @param testFuncP      IN      callback for various operations
* @param userDataP      IN      callback arg
* @return number of edges marked and disconnected from the graph
* @group "VU Edges"
* @see vu_markAndExciseSmallEdges, vu_markAndExciseSliverEdges
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_testAndExciseSmallEdges
(
VuSetP              graphP,
VuMask              discardMask,
VuEdgeTestFunction  testFuncP,
void*               userDataP
)
    {
    int nDiscard = 0;
    VuP endP;
    VuP startVertP, endVertP;

    VuMask visitMask     = vu_grabMask (graphP);

    VuMask skipMask = discardMask | visitMask;

    vu_clearMaskInSet (graphP, visitMask);

    VU_SET_LOOP (startP, graphP)
        {
        if (!VU_GETMASK (startP, skipMask))
            {
            endVertP = VU_FSUCC(startP);
            endP = VU_VSUCC(endVertP);
            startVertP = VU_FSUCC(endP);

            vu_setMask (startP, visitMask);
            vu_setMask (endP, visitMask);

            if (SUCCESS == testFuncP(
                                VUF_MESSAGE_TEST_DELETED_EDGE_CANDIDATE,
                                graphP,
                                startP,
                                userDataP
                                )
                )
                {
                if (SUCCESS == testFuncP(
                                VUF_MESSAGE_RESET_DELETED_EDGE_VERTEX_COORDINATES,
                                graphP,
                                startP,
                                userDataP
                                )
                    )
                    {
                    /* Excise and mark the edge */
                    vu_vertexTwist (graphP, startVertP, startP);
                    vu_vertexTwist (graphP, endVertP, endP);
                    vu_setMask (startP, discardMask);
                    vu_setMask (endP, discardMask);

                    /* Rejoin the dangling vertices */
                    vu_vertexTwist (graphP, startVertP, endVertP);
                    if (SUCCESS == testFuncP(
                                VUF_MESSAGE_QUERY_DEGENERATE_FACE_ACTION,
                                graphP,
                                startP,
                                userDataP
                                )
                       )
                        vu_detachDuplicateEdgesAtVertex (graphP, startVertP, discardMask);
                    nDiscard++;
                    }

                }
            }
        }
    END_VU_SET_LOOP (startP, graphP)

    vu_returnMask (graphP, visitMask);
    return nDiscard;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
