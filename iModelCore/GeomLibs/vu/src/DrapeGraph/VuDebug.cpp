/*--------------------------------------------------------------------------------------+
|    $RCSfile: vudebug.cpp,v $
|   $Revision: 1.2 $
|       $Date: 2010/10/28 14:03:49 $
|     $Author: Earlin.Lutz $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include    <Vu/DrapeGraph.h>
#define   EXPOSE_PUBLISHED_STRUCTURE
#define   EXPOSE_DIALOG_IDS
#define   EXPOSE_FDF
#define   BUFFER_SIZE         10
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static int s_debug = 0;


DRange3d VuDebug::s_outputRange = {0.0, 0.0, 0.0, 0.0};
void VuDebug::PrintMaskChars (VuP pNode)
    {
    GEOMAPI_PRINTF (vu_getMask (pNode, VU_EXTERIOR_EDGE) ? "X" : " ");
    GEOMAPI_PRINTF (vu_getMask (pNode, VU_BOUNDARY_EDGE) ? "B" : " ");
    GEOMAPI_PRINTF (vu_getMask (pNode, VU_RULE_EDGE) ? "R" : " ");
    GEOMAPI_PRINTF (vu_getMask (pNode, VU_SEAM_EDGE) ? "S" : " ");
    GEOMAPI_PRINTF (vu_getMask (pNode, VU_KNOT_EDGE) ? "K" : " ");
    GEOMAPI_PRINTF (vu_getMask (pNode, VU_GRID_EDGE) ? "G" : " ");
    }

void VuDebug::ShowVertex (VuSetP pGraph, VuP pSeed, char const * title)
    {
    if (s_debug > 0)
        {
        static double scale = 1e-4;
        GEOMAPI_PRINTF ("VERTEX from %d %s\n", vu_getIndex (pSeed), title != NULL ? title : "");
        VU_VERTEX_LOOP (pCurr, pSeed)
            {
            GEOMAPI_PRINTF ("    %3d (fs%3d vs%3d) (vtx %3d) %s",
                vu_getIndex (pCurr),
                vu_getIndex (vu_fsucc (pCurr)),
                vu_getIndex (vu_vsucc (pCurr)),
                vu_getUserDataPAsInt (pCurr),
                (IsRealSector (pCurr) ? "<" : "="));
            PrintMaskChars (pCurr);
            DPoint3d xyz;
            vu_getDPoint3d (&xyz, pCurr);
            GEOMAPI_PRINTF ("uv %10.6g, %10.6g, %10.6g)\n", xyz.x * scale, xyz.y * scale, xyz.z * scale);
            }
        END_VU_VERTEX_LOOP (pCurr, pSeed)
        }
    }



void VuDebug::ShowRange (char const * name, DRange3d range)
    {
    GEOMAPI_PRINTF ("(RANGE %s (%g,%g,%g) (%g,%g,%g))\n", name,
                range.low.x, range.low.y, range.low.z,
                range.high.x, range.high.y, range.high.z);
    }

void VuDebug::ShowVertices (VuSetP pGraph, char const * title)
    {
    if (s_debug > 0)
        {
        GEOMAPI_PRINTF (" GRAPH BY VERTEX %s\n", title);
        VuMask visitMask = vu_grabMask (pGraph);
        vu_clearMaskInSet (pGraph, visitMask);
        GEOMAPI_PRINTF ("\nNODE DATA");
        VU_SET_LOOP (pSeed, pGraph)
            {
            if (!vu_getMask (pSeed, visitMask))
                {
                vu_setMaskAroundVertex (pSeed, visitMask);
                ShowVertex (pGraph, pSeed);
                }
            }
        END_VU_SET_LOOP (pSeed, pGraph)
        vu_returnMask (pGraph, visitMask);
        }
    }
extern double vu_perimeter (VuP);
void VuDebug::ShowFace (VuSetP pGraph, VuP pFace, char const * name, bool showVertexLoops, bool linefeed)
    {
    if (s_debug > 1)
        {
        static double areaScale = 1.0e6;
        static int s_printEdgeLengths = 0;
        size_t count = 0;
        double area = vu_area (pFace);
        double perimeter = vu_perimeter (pFace);
        double ratioFactor = 4.0 * msGeomConst_pi;    // A/P^2 for circle
        if (showVertexLoops)
            {
            double aspectRatio;
            bsiTrig_safeDivide (&aspectRatio, area , perimeter * perimeter * ratioFactor, 0.0);
            GEOMAPI_PRINTF ("   (%s  FACE LOOP (area %.15lg) (aspectRatio %.2lg)\n", name ? name : "", area / areaScale, aspectRatio);
            VU_FACE_LOOP (pCurr, pFace)
                {
                GEOMAPI_PRINTF ("   (%3d", vu_getIndex (pCurr));
                PrintMaskChars (pCurr);
                DPoint3d xyz;
                vu_getDPoint3d (&xyz, pCurr);
                GEOMAPI_PRINTF (" %.15lg,%.15g,%.15g", xyz.x, xyz.y, xyz.z);
                VuP pMate = vu_edgeMate (pCurr);
                GEOMAPI_PRINTF (" data (%3d,%3d)  mate %3d",
                            vu_getUserDataPAsInt (pCurr),
                            (int)vu_getUserData1 (pCurr),
                            vu_getIndex (pMate)
                            );
                PrintMaskChars (pMate);
                GEOMAPI_PRINTF (" (V: ");
                VU_VERTEX_LOOP (pAroundVertex, pCurr)
                    {
                    int n = vu_countEdgesAroundFace (pAroundVertex);
                    if (n == 2)
                        GEOMAPI_PRINTF ("n");   // null face
                    else if (n == 3)
                        GEOMAPI_PRINTF ("t");
                    else if (n == 4)
                        GEOMAPI_PRINTF ("q");
                    else
                        GEOMAPI_PRINTF (" ");
                    GEOMAPI_PRINTF ("%d", vu_getIndex (pAroundVertex));
                    //PrintMaskChars (pAroundVertex);
                    }
                END_VU_VERTEX_LOOP (pAroundVertex, pCurr)
                if (s_printEdgeLengths)
                    GEOMAPI_PRINTF ("(L%g)", vu_edgeLengthXY (pCurr));
                GEOMAPI_PRINTF (")\n");
                }
            END_VU_FACE_LOOP (pCurr, pFace)
            GEOMAPI_PRINTF ("   )\n");
            }
        else
            {
            GEOMAPI_PRINTF ("(%s (area %g)", name ? name : "", area);
            VU_FACE_LOOP (pCurr, pFace)
                {
                GEOMAPI_PRINTF (" %d", vu_getIndex (pCurr));
                PrintMaskChars (pCurr);
                count++;
                if ((count % 8) == 0)
                    GEOMAPI_PRINTF ("\n     ");
                }
            END_VU_FACE_LOOP (pCurr, pFace)
            GEOMAPI_PRINTF (")");
            if (linefeed)
                GEOMAPI_PRINTF ("\n");
            }
        }
    }

void VuDebug::ShowFaces (VuSetP pGraph, char const * title, bool detailed, bool forceOutput)
    {
    if (s_debug > 0 || forceOutput )
        {
        int debug = s_debug;
        s_debug = 1000;
        GEOMAPI_PRINTF (" GRAPH BY FACE %s\n", title);
        VuMask visitMask = vu_grabMask (pGraph);
        vu_clearMaskInSet (pGraph, visitMask);
        VU_SET_LOOP (pSeed, pGraph)
            {
            if (!vu_getMask (pSeed, visitMask))
                {
                vu_setMaskAroundFace (pSeed, visitMask);
                ShowFace (pGraph, pSeed, NULL, detailed, true);
                }
            }
        END_VU_SET_LOOP (pSeed, pGraph)
        vu_returnMask (pGraph, visitMask);
        s_debug = debug;
        }
    }


void VuDebug::MoveToNewOutputRange ()
    {
    // Shift output low y to current high y
    s_outputRange.low.y = s_outputRange.high.y;
    s_outputRange.high = s_outputRange.low;
    }

void VuDebug::SetOutputOrigin (DRange3dCR dataRange)
    {
    DPoint3d origin = dataRange.low;
    origin.y = dataRange.high.y;
    s_outputRange.low = s_outputRange.high = origin;
    }


END_BENTLEY_GEOMETRY_NAMESPACE