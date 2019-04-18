/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <Vu/VuApi.h>

#define const_2pi   6.28318530717958620000e+000
#define const_pi    3.14159265358979310000e+000

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
Private void    vu__printNode
(
VuSetP          graphP,
VuP             currP,
bool            bPrintVertexLoop
)
    {
    VuP mateP = vu_edgeMate (currP);
    printf (" %5d|%5d ", VU_ID(currP), VU_ID(mateP));
    printf ("%c", vu_getMask (currP, VU_BOUNDARY_EDGE) ? 'B' : ' ');
    printf ("%c", vu_getMask (mateP, VU_BOUNDARY_EDGE) ? 'b' : ' ');
    printf ("%c", vu_getMask (currP, VU_EXTERIOR_EDGE) ? 'X' : ' ');
    printf ("%c", vu_getMask (mateP, VU_EXTERIOR_EDGE) ? 'x' : ' ');
    printf (" %4d", VU_GET_LABEL_AS_INT (currP));
    printf (" (%10lg,%10lg)", VU_U(currP), VU_V(currP));
    printf (" (");
    if (bPrintVertexLoop)
        {
        VU_VERTEX_LOOP (vertP, currP)
            {
            printf (" %d", VU_ID(vertP));
            }
        END_VU_VERTEX_LOOP (vertP, currP)
        }
    printf (")");
    printf ("\n");
    }

/*---------------------------------------------------------------------------------**//**
* @description Print statistics for a single face.
* @param graphP     IN  vu graph
* @param faceSeedP  IN  node in face loop
* @group "VU Debugging"
* @see vu__printFaceLabels
* @bsimethod                                                    BrianPeters     10/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void     vu__printFaceLabelsOneFace
(
VuSetP          graphP,
VuP             faceSeedP
)
    {
    VU_FACE_LOOP (currP, faceSeedP)
        {
        vu__printNode (graphP, currP, true);
        }
    END_VU_FACE_LOOP (currP, faceSeedP)
    }

/*---------------------------------------------------------------------------------**//**
* @description Print statistics for masked nodes in the graph.
* @param graphP     IN  vu graph
* @param mask       IN  node mask on nodes to print
* @param pTitle     IN  unused
* @group "VU Debugging"
* @bsimethod                                                    EarlinLutz      05/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void     vu__printMaskedNodes
(
VuSetP          graphP,
VuMask          mask,
char *pTitle
)
    {
    printf (" All Nodes with mask %s (%x)\n", pTitle, mask);
    VU_SET_LOOP (pCurr, graphP)
        {
        if (vu_getMask (pCurr, mask))
            vu__printNode (graphP, pCurr, false);
        }
    END_VU_SET_LOOP (pCurr, graphP)
    }

/*---------------------------------------------------------------------------------**//**
* @description Print statistics for all faces of the graph.
* @param graphP     IN  vu graph
* @param pTitle     IN  text to print at start of report
* @group "VU Debugging"
* @see vu__printFaceLabelsOneFace
* @bsimethod                                                    BrianPeters     10/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void     vu__printFaceLabels
(
VuSetP          graphP,
char *pTitle
)
    {
    int count = 0;
    int numPositive = 0;
    int numNegative = 0;
    int numZero     = 0;
    double area;
    VuMask visitMask = vu_grabMask (graphP);

    if (pTitle)
        printf (" *********** %s ***************\n", pTitle);
    vu_clearMaskInSet (graphP, visitMask);
    VU_SET_LOOP (faceSeedP, graphP)
        {
        if (!vu_getMask (faceSeedP, visitMask))
            {
            vu_setMaskAroundFace (faceSeedP, visitMask);
            area = vu_area (faceSeedP);
            printf (" (FACE %d) (edges %d) (area %le)\n", count++,
                        vu_faceLoopSize (faceSeedP),
                        area);
            if (area > 0.0)
                numPositive++;
            else if (area < 0.0)
                numNegative++;
            else
                numZero++;
            vu__printFaceLabelsOneFace (graphP, faceSeedP);
            }
        }
    END_VU_SET_LOOP (faceSeedP, graphP)
    printf (" *** (area counts +0- %d %d %d)\n",
                    numPositive, numZero, numNegative);
    vu_returnMask (graphP, visitMask);
    }

