/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
#include <Mtg/mtgprint.fdf>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* Expand an (integer) mask into an ascii buffer, appropriate
*   for use in a GEOMAPI_PRINTF statement.
* @param pBuffer <= buffer of up to 64 ascii characeters (32 bits, various spaces)
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlMTGGraph_fillMaskString
(
    char    *pBuffer,
MTGMask     mask
)
    {
    static char mnemonic[] =
            "xxxxxxxxxxxxxxxxxxxxSHEDPVuvVUBX";
        /*  "84218421842184218421842184218421";     */
    int i, j;
    int k;
    for (i = j = 0; j < 32; j++)
        {
        k = 31 - j;
        pBuffer[i++] = (mask >> k )  & 0x01 ? mnemonic[j] : '_';
        if ((j & 0x07) == 0x07)
            pBuffer[i++] = ' ';
        }
    pBuffer[i] = 0;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGraph_printVertexLoops
(
MTGGraph  *pGraph
)
    {
    MTGMask   visitMask;
    //static int s_nodesPerLine = 1;
    int loopCount;
    char maskString[128];;

    GEOMAPI_PRINTF("   (VertexLoops (vertexCounter (nodeId masks fSucc vSucc edgeMate)\n");

    visitMask = pGraph->GrabMask ();
    pGraph->ClearMask (visitMask);

    loopCount = 0;
    MTGARRAY_SET_LOOP (vertNodeId, pGraph)
        {
        if (!pGraph->GetMaskAt (vertNodeId, visitMask))
            {
            MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, vertNodeId)
                {
                jmdlMTGGraph_fillMaskString (maskString,
                                    pGraph->GetMaskAt (currNodeId, MTG_ALL_MASK_BITS));
                if (currNodeId == vertNodeId)
                    GEOMAPI_PRINTF ("\n(%4d ", loopCount);
                else
                    GEOMAPI_PRINTF("\n      ");
                GEOMAPI_PRINTF (" %6d %s fs%d vs%d em%d)",
                        currNodeId,
                        maskString,
                        pGraph->FSucc (currNodeId),
                        pGraph->VSucc (currNodeId),
                        pGraph->EdgeMate (currNodeId)
                        );

                jmdlMTGGraph_setMask (pGraph, currNodeId, visitMask);
                }
            MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, vertNodeId)
            loopCount++;
            }
        }
    MTGARRAY_END_SET_LOOP (vertNodeId, pGraph)

    GEOMAPI_PRINTF("   ) // (%d VertexLoops)\n", loopCount);
    pGraph->DropMask (visitMask);
    }

/*---------------------------------------------------------------------------------**//**
* Print face loops for the graph, with per-node callback for application additions.
* @param pGraph => graph to search
* @param pFunc => (optional) function to be called just after each line is printed
*           (but before its linefeed.)
* @param pContext => additional arg for callback
*<pre>
*               pFunc (pGraph, nodeId, pContext)
*</pre>
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_printFaceLoopsExt
(
MTGGraph    *pGraph,
MTGNodeFunc pFunc,
void        *pContext
)
    {
    MTGMask   visitMask;
    //static int s_nodesPerLine = 1;
    int loopCount;
    int labelIndex, label;
    char maskString[128];;

    GEOMAPI_PRINTF("\n   (FaceLoops (faceCounter (nodeId masks fSucc vSucc edgeMate)\n");

    visitMask = pGraph->GrabMask ();
    pGraph->ClearMask (visitMask);

    loopCount = 0;
    MTGARRAY_SET_LOOP (startNodeId, pGraph)
        {
        if (!pGraph->GetMaskAt (startNodeId, visitMask))
            {
            MTGARRAY_FACE_LOOP (currNodeId, pGraph, startNodeId)
                {
                jmdlMTGGraph_fillMaskString (maskString,
                                    pGraph->GetMaskAt (currNodeId, MTG_ALL_MASK_BITS));
                if (currNodeId == startNodeId)
                    GEOMAPI_PRINTF ("\n(%4d ", loopCount);
                else
                    GEOMAPI_PRINTF("\n      ");
                GEOMAPI_PRINTF (" %3d %s %3dF %3dV %3dE)",
                        currNodeId,
                        maskString,
                        pGraph->FSucc (currNodeId),
                        pGraph->VSucc (currNodeId),
                        pGraph->EdgeMate (currNodeId)
                        );

                jmdlMTGGraph_setMask (pGraph, currNodeId, visitMask);

                for (labelIndex = 0; pGraph->TryGetLabel (currNodeId, labelIndex, label); labelIndex++)
                    {
                    GEOMAPI_PRINTF
                        (
                        -10 < label && label < 100 ? " %2d" : " %6d",
                        label
                        );
                    }
                GEOMAPI_PRINTF ("V:");
                MTGARRAY_VERTEX_LOOP (vertexNodeId, pGraph, currNodeId)
                    {
                    GEOMAPI_PRINTF(" %d", vertexNodeId);
                    }
                MTGARRAY_END_VERTEX_LOOP (vertexNodeId, pGraph, currNodeId)

                if (pFunc)
                    pFunc (pGraph, currNodeId, pContext);
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, startNodeId)
            loopCount++;
            GEOMAPI_PRINTF("   )\n //\t)//End face %d  edge count %d\n", loopCount,
                            (int)pGraph->CountNodesAroundFace (startNodeId));
            }
        }
    MTGARRAY_END_SET_LOOP (startNodeId, pGraph)

    GEOMAPI_PRINTF("   )\n");
    jmdlMTGGraph_printLoopCounts (pGraph);
    pGraph->DropMask (visitMask);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_printFaceLoops
(
MTGGraph  *pGraph
)
    {
    jmdlMTGGraph_printFaceLoopsExt (pGraph, NULL, NULL);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_printLoopCounts
(
MTGGraph  *pGraph
)
    {
    int numVertex = (int)pGraph->CountVertexLoops ();
    int numFace = (int)pGraph->CountFaceLoops ();
    bvector<bvector<MTGNodeId>> component;
    pGraph->CollectConnectedComponents (component);
    int numComponent = (int)component.size ();
    int numEdge = (int)(pGraph->GetActiveNodeCount () / 2);
    GEOMAPI_PRINTF ("  V = %d\n", numVertex);
    GEOMAPI_PRINTF ("  E = %d\n", numEdge);
    GEOMAPI_PRINTF ("  F = %d\n", numFace);
    GEOMAPI_PRINTF ("  C = %d\n", numComponent);
    GEOMAPI_PRINTF ("  V - E + F = %d\n", numVertex - numEdge + numFace);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
