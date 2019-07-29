/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
void vu_recycleNodeLoop (VuP *listPP, VuP *loopPP);


/*---------------------------------------------------------------------------------**//**
* @nodoc
* @description Perform a merge/boolean operation on (disjoint) face loops in the graph.
* @remarks The graph may have intersections and containments other than those indicated by explicit graph topology.
*       This function finds the intersections/containments using the given rule and augments the graph structure accordingly.
* @remarks This function uses the graph tolerances set by ~mvu_setTol.
* @param graphP     IN OUT  graph header
* @param mergeType  IN      rule to apply to graph
* @group "VU Booleans"
* @bsimethod                                                    EarlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_mergeOrUnionLoops
(
VuSetP graphP,
VuMergeType mergeType
)
    {
    double abstol, reltol;
    vu_getTol (graphP, &abstol, &reltol);
    vu_merge2002 (graphP, mergeType, abstol, reltol);
    }

/*---------------------------------------------------------------------------------**//**
* @description Perform a merge operation on (disjoint) face loops in the graph, with all duplicate edges preserved.
* @remarks The graph may have intersections and containments other than those indicated by explicit graph topology.
*       This function finds the intersections/containments using the given rule and augments the graph structure accordingly.
* @remarks This function uses the graph tolerances set by ~mvu_setTol.
* @remarks This is a shortcut for calling ~mvu_merge2002 with ~tVuMergeType VUUNION_UNION.
* @param graphP     IN OUT  graph header
* @group "VU Booleans"
* @see vu_orLoops
* @bsimethod                                                    EarlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_unionLoops
(
VuSetP graphP
)
    {
    vu_mergeOrUnionLoops (graphP, VUUNION_UNION);
    }

/*---------------------------------------------------------------------------------**//**
* @description Find the largest face of negative area in the graph.
* @param arrayP IN OUT  array to collect faces with negative area
* @param graphP IN      graph header
* @return node in the maximally negative area face
* @group "VU Coordinates"
* @bsimethod                                                    BrianPeters     10/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP      vu_getNegativeAreaFace
(
VuArrayP        arrayP,
VuSetP          graphP
)
    {
    double      area, maximallyNegativeArea;
    VuP         maximallyNegativeFace;
    VuMask      mVisited  = vu_grabMask (graphP);

    maximallyNegativeArea = 0.0;
    maximallyNegativeFace = VU_NULL;
    vu_clearMaskInSet (graphP, mVisited);
    VU_SET_LOOP (seedP, graphP)
        {
        if (! VU_GETMASK (seedP, mVisited))
            {
            vu_setMaskAroundFace (seedP, mVisited);
            if (0.0 > (area = vu_area (seedP)))
                {
                if (maximallyNegativeArea > area)
                    {
                    maximallyNegativeArea = area;
                    maximallyNegativeFace = seedP;
                    }
                if (arrayP)
                    vu_arrayAdd (arrayP, seedP);
                }
            }
        }
    END_VU_SET_LOOP (seedP, graphP)

    vu_returnMask (graphP,mVisited);
    return  maximallyNegativeFace;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vuunion_extendWIndingNumberSearchAcrossEdge             |
|                                                                       |
| author        EarlinLutz                          01/07               |
|                                                                       |
+----------------------------------------------------------------------*/
static void vuunion_extendWIndingNumberSearchAcrossEdge
(
VuP startP,     /* An already visited edge, depth available from label field. */
VuArrayP stackP,
VuMask mCrossing,
VuMask mVisited
)
    {
    /* Jump to the other side */
    int depth = vu_getInternalDataPAsInt (startP);
    VuP mateP = VU_EDGE_MATE (startP);
    VuP farP;
    if( VU_GETMASK(startP, mCrossing) )
        depth++;

    if( VU_GETMASK(mateP, mCrossing) )
        depth--;
    if( !VU_GETMASK(mateP, mVisited))
        {
        // Mark the mate face ...
        VU_FACE_LOOP (currP, mateP)
            {
            VU_SETMASK (currP, mVisited);
            vu_setInternalDataPAsInt(currP, depth);
            farP = VU_EDGE_MATE (currP);
            // Look ahead for visit mask -- bypassing the push keeps the stack a little smaller.
            if (!VU_GETMASK (farP, mVisited))
                vu_arrayAdd (stackP, currP);
            }
        END_VU_FACE_LOOP (currP, mateP)
        }
    }

/*---------------------------------------------------------------------------------**//**
* @nodoc
* @remarks Assumption: the graph is merged and connected.
* @bsimethod                                                    EarlinLutz     10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markWindingNumber
(
VuSetP          graphP,
VuMask          maskToCheck
)
    {
    /*----------------------------------------------------------------------+
    | These former statics are shared among all levels of the winding number|
    | recursion.                                                            |
    | Rule: Let nodeP and mateP be two Vu's on opposite sides of an edge    |
    |   If the face containing nodeP is at depth d, then when jumping       |
    |   across to the face containing mateP we INCREASE depth if            |
    |   mCrossing is set on nodeP, and decrease it if set                   |
    |   on mateP.                                                           |
    +----------------------------------------------------------------------*/
    VuMask mCrossing;
    VuMask mVisited;

    VuP         maxNegFaceP;
    mCrossing = maskToCheck;
    mVisited  = vu_grabMask (graphP);

    vu_clearMaskInSet (graphP, mVisited);

    if (VU_NULL != (maxNegFaceP = vu_getNegativeAreaFace (NULL, graphP)))
        {
        VuArrayP stackP = vu_grabArray (graphP);
        VuP seedP;
        VU_FACE_LOOP (currP, maxNegFaceP)
            {
            VU_SETMASK (currP, mVisited);
            vu_setInternalDataPAsInt (currP, 0);
            vu_arrayAdd (stackP, currP);
            }
        END_VU_FACE_LOOP (currP, maxNegFaceP)

        while (NULL != (seedP = vu_arrayRemoveLast (stackP)))
            {
            vuunion_extendWIndingNumberSearchAcrossEdge (seedP, stackP, mCrossing, mVisited);
            }

        vu_returnArray (graphP, stackP);
        }
    else
        {
        /* If there is no negative area loop then something is rotten in Denmark! */
        vu_recycleNodeLoop (&graphP->freeNodeP, &graphP->lastNodeP);
        }

    vu_returnMask(graphP,mVisited);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_markComponent                                        |
|                                                                       |
| author        EarlinLutz                             10/95           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void    vu_markComponent
(
VuSetP          graphP,
VuP             seedP,
VuMask          mVisited
)
    {
    VuArrayP stackP = vu_grabArray(graphP);
    VuP faceSeedP;
    vu_arrayAdd (stackP, seedP);
    while (NULL != (faceSeedP = vu_arrayRemoveLast (stackP)))
        {
        if (! VU_GETMASK (faceSeedP, mVisited))
            {
            // Prevent reentry to this face ....
            vu_setMaskAroundFace (faceSeedP, mVisited);
            // Explore fresh neighbors ....
            VU_FACE_LOOP (faceP, faceSeedP)
                {
                VuP mateP = VU_EDGE_MATE (faceP);
                if (!VU_GETMASK (mateP, mVisited))
                    vu_arrayAdd (stackP, mateP);
                }
            END_VU_FACE_LOOP (faceP, faceSeedP)
            }
        }
    vu_returnArray (graphP, stackP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_connectGraph                                         |
|                                                                       |
| author        BrianPeters                             10/95           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void    vu_connectGraph
(
VuSetP          graphP
)
    {
    int         connected = true;
    VuMask      mVisited;
    mVisited  = vu_grabMask (graphP);

    vu_clearMaskInSet (graphP, mVisited);
    vu_markComponent (graphP, VU_ANY_NODE_IN_GRAPH (graphP), mVisited);

    VU_SET_LOOP (P, graphP)
        {
        if (! VU_GETMASK (P, mVisited))
            {
            connected = false;
            break;
            }
        }
    END_VU_SET_LOOP (P, graphP)

    if (! connected)
        vureg_regularizeGraph (graphP);

    vu_returnMask (graphP, mVisited);
    }


/*----------------------------------------------------------------------+
|                                                                       |
|   2D Boolean Operations                                               |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_freeNonSeparatingEdges                               |
|                                                                       |
| author        EarlinLutz                              09/01           |
|                                                                       |
+----------------------------------------------------------------------*/
static void    vu_freeNonSeparatingEdges
(
VuSetP          graphP
)
    {
    vu_freeEdgesByMaskCount (graphP, VU_EXTERIOR_EDGE, true, false, true);
    }

/*---------------------------------------------------------------------------------**//**
* @nodoc
* @bsimethod                                                    BrianPeters     07/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markExteriorBoundariesFromWindingNumberAndRule
(
VuSetP          graphP,
VuMergeType     mergeType
)
    {

    vu_clearMaskInSet (graphP, VU_EXTERIOR_EDGE);

    /* Set exterior mark based on type of operation being performed */
    VU_SET_LOOP (nodeP, graphP)
        {
        switch (mergeType)
            {
            case VUUNION_UNION:
                VU_WRITEMASK (nodeP, VU_EXTERIOR_EDGE, vu_getInternalDataPAsInt (nodeP) <= 0);
                break;
            case VUUNION_PARITY:
                VU_WRITEMASK (nodeP, VU_EXTERIOR_EDGE, vu_getInternalDataPAsInt (nodeP) & 0x1);
                break;
            case VUUNION_INTERSECTION:
                VU_WRITEMASK (nodeP, VU_EXTERIOR_EDGE, vu_getInternalDataPAsInt (nodeP) >= 2);
                break;
            case VUUNION_UNION_COMPLEMENT:
                VU_WRITEMASK (nodeP, VU_EXTERIOR_EDGE, vu_getInternalDataPAsInt (nodeP) >= 1);
                break;
            }
        }
    END_VU_SET_LOOP (nodeP, graphP)

#ifdef CHECK_AFTER_EXTERIOR_MASK_ASSIGNMENT
    vu_countInconsistentExteriorFaceMasks (graphP, "XOR -- after parity assignment");
#endif

#ifdef regularizeAwayNullFaces
    /* Eliminate exterior marks on trivial faces */
    VU_SET_LOOP (nodeP, graphP)
        {
        if (VU_FSUCC (VU_FSUCC (nodeP)) == nodeP)
            {
            VU_CLRMASK (nodeP, VU_EXTERIOR_EDGE);
            VU_CLRMASK (VU_FSUCC (nodeP) , VU_EXTERIOR_EDGE);
            }
        }
    END_VU_SET_LOOP (nodeP, graphP)
#endif
    }

static void vu_mergeAndConnectForBoolop
(
VuSetP graphP
)
    {
    int trapMask = 110000;
    vu_postGraphToTrapFunc (graphP, "vu_mergeAndConnectForBoolop", 0, trapMask++); // 000
    vu_mergeOrUnionLoops (graphP, VUUNION_UNION);
    vu_postGraphToTrapFunc (graphP, "vu_mergeAndConnectForBoolop", 0, trapMask++); // 001
    vu_connectGraph (graphP);
    vu_postGraphToTrapFunc (graphP, "vu_mergeAndConnectForBoolop", 0, trapMask++); // 002


#ifdef ATTEMPT_FIXUP
        {
        static double s_relTol = 1.0e-8;
        int passCount = 0;
        static int maxPass = 3;
        int euler, euler1, euler2, euler3, euler4, euler5, euler6;
        euler = vu_EulerCharacteristic (graphP);
        while (euler != 2 && passCount++ < maxPass)
            {
            euler1 = euler;
            vu_copyUVAroundAllVertices (graphP);
            vu_mergeOrUnionLoops (graphP, VUUNION_UNION);
            vu_connectGraph (graphP);
            euler2 = vu_EulerCharacteristic (graphP);
            vu_countVertexSweepErrors (graphP);
            euler3 = vu_EulerCharacteristic (graphP);
            vu_realignParallelEdges (graphP);
            euler4 = vu_EulerCharacteristic (graphP);
            vu_countVertexSweepErrors (graphP);
            euler5 = vu_EulerCharacteristic (graphP);
            vu_collapseShortEdges (graphP, s_relTol);
            vu_realignParallelEdges (graphP);
            euler6 = vu_EulerCharacteristic (graphP);
            printf (" Pre/post fixup euler characteristics %d \n"
                    "\tpaste & merge  %d\n"
                    "\tcheck ccw sort %d\n"
                    "\trealign        %d\n"
                    "\trecheck ccw    %d\n"
                    "\tcollapse edges %d\n",
                    euler,
                    euler2,
                    euler3,
                    euler4,
                    euler5,
                    euler6
                    );
            if (euler6 == euler1)
                break;
            euler = euler6;
            }
        }
#endif

    }

/*---------------------------------------------------------------------------------**//**
* @description Perform a boolean union on the face loops of the graph.
* @remarks Before applying the boolean operation, this function first merges the graph (~mvu_unionLoops),
*   then regularizes it (~mvureg_regularizeGraph) if it is disconnected.
* @param graphP IN OUT graph header
* @group "VU Booleans"
* @bsimethod                                                    BrianPeters     07/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_orLoops
(
VuSetP          graphP
)
    {
    if (! vu_emptyGraph (graphP))
        {
        vu_mergeAndConnectForBoolop (graphP);
        vu_markWindingNumber (graphP, VU_EXTERIOR_EDGE);
        vu_markExteriorBoundariesFromWindingNumberAndRule (graphP, VUUNION_UNION);
        vu_freeNonSeparatingEdges (graphP);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Perform a boolean symmetric difference (logical XOR) on the face loops of the graph.
* @remarks Before applying the boolean operation, this function first merges the graph (~mvu_unionLoops),
*   then regularizes it (~mvureg_regularizeGraph) if it is disconnected.
* @param graphP IN OUT graph header
* @group "VU Booleans"
* @bsimethod                                                    BrianPeters     07/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_xorLoops
(
VuSetP          graphP
)
    {
    if (! vu_emptyGraph (graphP))
        {
        vu_mergeAndConnectForBoolop (graphP);
        vu_markWindingNumber (graphP, VU_EXTERIOR_EDGE);
        vu_markExteriorBoundariesFromWindingNumberAndRule (graphP, VUUNION_PARITY);
#ifdef CHECK_GRAPH
        vu_checkGraph (graphP, "XOR -- after parity assignment");
#endif
        vu_freeNonSeparatingEdges (graphP);

#ifdef CHECK_GRAPH
        vu_checkGraph (graphP, "XOR -- after separator cleanup");
#endif
        vu_notLoops (graphP);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Perform a boolean intersection on the face loops of the graph.
* @remarks Before applying the boolean operation, this function first merges the graph (~mvu_unionLoops),
*   then regularizes it (~mvureg_regularizeGraph) if it is disconnected.
* @param graphP IN OUT graph header
* @group "VU Booleans"
* @bsimethod                                                    BrianPeters     07/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_andLoops
(
VuSetP          graphP
)
    {
    int trapMask = 100000;
#define CHECK_GRAPHnot
    if (! vu_emptyGraph (graphP))
        {
        vu_postGraphToTrapFunc (graphP, "vu_andLoops", 0, trapMask++); // 000
        vu_mergeAndConnectForBoolop (graphP);
        vu_postGraphToTrapFunc (graphP, "vu_andLoops", 0, trapMask++); // 001
        vu_markWindingNumber (graphP, VU_EXTERIOR_EDGE);
        vu_postGraphToTrapFunc (graphP, "vu_andLoops", 0, trapMask++); // 002
        vu_markExteriorBoundariesFromWindingNumberAndRule (graphP, VUUNION_INTERSECTION);
#ifdef CHECK_GRAPH
    vu_checkGraph (graphP, "INT -- after parity assignment");
#endif
        vu_freeNonSeparatingEdges (graphP);
        vu_postGraphToTrapFunc (graphP, "vu_andLoops", 0, trapMask++); // 003

#ifdef CHECK_GRAPH
        vu_checkGraph (graphP, "INT -- after separator cleanup");
#endif
        vu_notLoops (graphP);
        vu_postGraphToTrapFunc (graphP, "vu_andLoops", 0, trapMask++); // 004

#ifdef CHECK_GRAPH
        vu_checkGraph (graphP, "INT -- after NOT");
#endif
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Perform a boolean subtraction (A intersect the complement of B) on the face loops of the graph.
* @remarks Before applying the boolean operation, this function first merges the graph (~mvu_unionLoops),
*   then regularizes it (~mvureg_regularizeGraph) if it is disconnected.
* @param graphP IN OUT graph header
* @group "VU Booleans"
* @bsimethod                                                    BrianPeters     07/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_andComplementLoops
(
VuSetP          graphP
)
    {
    if (! vu_emptyGraph (graphP))
        {
        vu_mergeAndConnectForBoolop (graphP);
        vu_markWindingNumber (graphP, VU_EXTERIOR_EDGE);
        vu_markExteriorBoundariesFromWindingNumberAndRule (graphP, VUUNION_UNION_COMPLEMENT);
        vu_freeNonSeparatingEdges (graphP);
        vu_notLoops (graphP);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Invert the face loops of the graph.
* @remarks This routine moves the VU_EXTERIOR_EDGE mask to the edge mate, essentially turning the faces inside out.
* @param graphP IN OUT graph header
* @group "VU Booleans"
* @bsimethod                                                    BrianPeters     07/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_notLoops
(
VuSetP          graphP
)
    {
    if (! vu_emptyGraph (graphP))
        {
        vu_freeNonSeparatingEdges (graphP);

        VU_SET_LOOP (currP, graphP)
            {
            VU_WRITEMASK (currP, VU_EXTERIOR_EDGE, ! VU_GETMASK (currP, VU_EXTERIOR_EDGE));
            }
        END_VU_SET_LOOP (currP, graphP)
        }
    }


END_BENTLEY_GEOMETRY_NAMESPACE
