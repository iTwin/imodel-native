/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/src/vupointin.cpp $
|    $RCSfile: vupointin.c,v $
|   $Revision: 1.2 $
|       $Date: 2006/09/14 14:56:12 $
|     $Author: DavidAssaf $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
static bool unitNormal
(
DPoint2d *normalP,
DPoint2d *point0P,
DPoint2d *point1P
)
    {
    double length, a;
    normalP->x = - (point1P->y - point0P->y);
    normalP->y = point1P->x - point0P->x;
    length = sqrt (normalP->x * normalP->x + normalP->y * normalP->y);
    if (length == 0.0)
        return false;
    a = 1.0 / length;
    normalP->x *= a;
    normalP->y *= a;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
static double dotVectorDifference
(
DPoint2d *vectorP,
DPoint2d *point0P,
DPoint2d *point1P
)
    {
    return vectorP->x * (point1P->x - point0P->x)
        +  vectorP->y * (point1P->y - point0P->y);
    }

/*-----------------------------------------------------------------*//**
* @description Over all edges of the face, search for the edge with <em>smallest</em> max distance to any vertex.
* @remarks For a triangle, this distance is the smallest of the altitudes.  For a convex face, this is the smallest vertical height as the
*       shape is rolled from one edge to another along a horizontal line.
* @param seedP  IN  node in face
* @bsimethod                            EarlinLutz      07/01
+----------------------------------------------------------------------*/
Public double vu_minEdgeVertexDistanceAroundFace
(
VuP seedP
)
    {
    DPoint2d baseXY, normal, currXY;
    VuP currP;
    double currDist, currBaseMaxDist, result = 1000000.0;
    int numBase = 0;
    VU_FACE_LOOP (baseP, seedP)
        {
        vu_getXY (&baseXY.x, &baseXY.y, baseP);
        currP = vu_fsucc (baseP);
        vu_getXY (&currXY.x, &currXY.y, currP);
        if (unitNormal (&normal, &baseXY, &currXY))
            {
            currBaseMaxDist = 0.0;
            for (currP = vu_fsucc(currP); currP != baseP; currP = vu_fsucc(currP))
                {
                vu_getXY (&currXY.x, &currXY.y, currP);
                currDist = fabs (dotVectorDifference (&normal, &baseXY, &currXY));
                if (currDist > currBaseMaxDist)
                    currBaseMaxDist = currDist;
                }

            if (numBase == 0 || currBaseMaxDist < result)
                {
                result = currBaseMaxDist;
                }
            numBase++;
            }
        }
    END_VU_FACE_LOOP (baseP, seedP)
    return result;
    }

/*-----------------------------------------------------------------*//**
* @description Find any point strictly interior to a polygon.  Only xy coordinates are examined.
* @param pXYOut <= coordinates of point in polygon.
* @param pLoopPoints => array of points in polygon.   Multiple loops
*               may be entered with the value DISCONNECT as x and y parts
*               of a separator point.
* @param numLoopPoints => number of points in array.
* @return SUCCESS if point found.
* @bsimethod                            EarlinLutz      07/01
+----------------------------------------------------------------------*/
Public StatusInt vu_anyInteriorPointInPolygon
(
DPoint3d                *pXYOut,
DPoint3dCP              pLoopPoints,
int                     numLoopPoints
)
    {
    StatusInt status = SUCCESS;
    VuSetP      graphP;
    VuArrayP    faceArrayP;
    VuP         faceP;
    int         i0, i1;
    int         numThisLoop;
    double      maxDist, minDist;
    DPoint3d    xySave;
    int numFace = 0;

    if ( ! (graphP = vu_newVuSet (0)) )
        return ERROR;


    for ( i0 = 0 ; i0 < numLoopPoints ;)
        {
        for (i1 = i0; i1 < numLoopPoints && pLoopPoints[i1].x != DISCONNECT;)
            {
            i1++;
            }
        numThisLoop = i1 - i0;
        if (numThisLoop > 2)
            vu_makeLoopFromArray3d (graphP, const_cast <DPoint3d*> (pLoopPoints + i0), numThisLoop, true, true);
        i0 = i1 + 1;
        }

    /*-------------------------------------------------------------------
    Merge loops is supposed to fix up all the criss-crosses and whatnot
    that might be here.  Sometimes it does, sometimes it has tolerance
    problems.  So we run it twice -- the second time cleans up what the
    first time doesn't catch.  Yup, that's strange.  But it works. (EDL)
    -------------------------------------------------------------------*/
    vu_mergeLoops (graphP);
    vu_mergeLoops (graphP);

    vu_regularizeGraph (graphP);
    vu_markAlternatingExteriorBoundaries(graphP,true);

    faceArrayP = vu_grabArray (graphP);

    vu_triangulateMonotoneInteriorFaces (graphP, false);
    vu_flipTrianglesToImproveQuadraticAspectRatio (graphP);
    vu_collectInteriorFaceLoops (faceArrayP, graphP);

    vu_arrayOpen (faceArrayP);

    maxDist = -1.0;
    memset (&xySave, 0, sizeof (xySave));
    for (;vu_arrayRead (faceArrayP, &faceP);)
        {
        double xSum = 0.0;
        double ySum = 0.0;
        numThisLoop = 0;
        VU_FACE_LOOP (P, faceP)
            {
            xSum += vu_getX (P);
            ySum += vu_getY (P);
            numThisLoop++;
            }
        END_VU_FACE_LOOP (P, faceP)
        xSum /= (double)numThisLoop;
        ySum /= (double)numThisLoop;

        minDist = vu_minEdgeVertexDistanceAroundFace (faceP);
        if (minDist > maxDist)
            {
            xySave.x = xSum;
            xySave.y = ySum;
            maxDist = minDist;
            }
        numFace++;
        }

    if (graphP)
        {
        if (faceArrayP)
            vu_returnArray (graphP, faceArrayP);
        vu_freeVuSet (graphP);
        }

    if (status == SUCCESS && numFace > 0 && pXYOut)
        *pXYOut = xySave;
    return status;
    }
/*
* If vector from x0 to x1 crossed with ux,uy is positive,
* create a parallelogram with first edge along (x0,y0) to (x1,y1) and
*  opposite edge shifted by ux,uy
*/
static void addOffsetParallelogram(
    VuSetP graph,
    bvector<DPoint3d> &work,
    double x0,
    double y0,
    double x1,
    double y1,
    double ux,
    double uy,
    VuMask leftMask,
    VuMask rightMask
)
    {
    double vx = x1 - x0;
    double vy = y1 - y0;
    if ((ux != 0.0 || uy != 0.0) && DoubleOps::DeterminantXYXY(vx, vy, ux, uy) > 0.0)
        {
        work.clear ();
        work.push_back (DPoint3d::From (x0, y0));
        work.push_back(DPoint3d::From(x1, y1));
        work.push_back(DPoint3d::From(x1 + ux, y1 + uy));
        work.push_back(DPoint3d::From(x0 + ux, y0 + uy));
        vu_addEdgesXYTol(graph, nullptr, work, true, 0.0, leftMask, rightMask);
        }
    }
static void addRectangle (
    VuSetP graph,
    bvector<DPoint3d> &work,
    double x0,
    double y0,
    double x1,
    double y1,
    VuMask leftMask,
    VuMask rightMask
)
    {
    if (!DoubleOps::AlmostEqual (x0, x1)  && !DoubleOps::AlmostEqual (y0, y1))
        {
        work.clear();
        work.push_back(DPoint3d::From(x0, y0));
        work.push_back(DPoint3d::From(x1, y0));
        work.push_back(DPoint3d::From(x1, y1));
        work.push_back(DPoint3d::From(x0, y1));
        work.push_back(DPoint3d::From(x0, y0));
        vu_addEdgesXYTol(graph, nullptr, work, true, 0.0, leftMask, rightMask);
        }
    }

/*-----------------------------------------------------------------*//**
* @description Return a graph whose interior points are at least (ax,bx) distance from 
*     loop interior points to the left and right, and (ay, by) below and above.
* @param loops > array of loops.  In these loops, inside is defined by parity.
* @param ax => rightward offset from an edge on the left side.
* @param bx => leftward offset from edge on the right side.
* @param ax => rightward offset from an edge on the top side.
* @param bx => leftward offset from edge on the bottom side.
* @param interiorLoops <= loops containing valid region.
* @bsimethod                            EarlinLutz      07/01
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_createXYOffsetLoops
(
bvector<bvector<DPoint3d>> &loops,
double ax,
double bx, 
double ay,
double by,
bvector<bvector<DPoint3d>> &interiorLoops
)
    {
    interiorLoops.clear ();
    bvector<DPoint3d> work;
    VuSetP      graph = vu_newVuSet (0);
    bvector<DPoint3d> workPoints;
    VuArrayP    faceArrayP;
   
    for (size_t loopIndex = 0; loopIndex < loops.size (); loopIndex++)
        {
        if (loops[loopIndex].size () > 0)
            vu_makeLoopFromArray3d(graph, const_cast <DPoint3d*> (loops[loopIndex].data ()),
                        (int)loops[loopIndex].size (), true, true);
        }

    VuMask offsetFragmentBoundaryMask = VU_BOUNDARY_EDGE;
    VuMask offsetFragmentExteriorMask = VU_RULE_EDGE;
    vu_mergeOrUnionLoops(graph, VUUNION_UNION);

    vu_regularizeGraph(graph);
    vu_markAlternatingExteriorBoundaries(graph, true);
    // record one node per face in the primary loops.
    faceArrayP = vu_grabArray(graph);
    vu_collectInteriorFaceLoops(faceArrayP, graph);
    // push the primary graph while we work on the offsets.
    // (but note tha the primary face nodes are still valid for traversals)
    vu_stackPush (graph);
    static int s_doRectangles = 0;
    vu_arrayOpen(faceArrayP);
    static double s_vertexFactor = 1.01;
    VuMask maskA = offsetFragmentBoundaryMask;
    VuMask maskB = offsetFragmentBoundaryMask | offsetFragmentExteriorMask;
    for (VuP faceP; vu_arrayRead(faceArrayP, &faceP);)
        {
        VU_FACE_LOOP(nodeA, faceP)
            {
            if (vu_getMask (nodeA, VU_BOUNDARY_EDGE))
                {
                auto nodeB = vu_fsucc (nodeA);
                double xA = vu_getX (nodeA);
                double yA = vu_getY(nodeA);
                double xB = vu_getX(nodeB);
                double yB = vu_getY(nodeB);
                addOffsetParallelogram(graph, workPoints, xA, yA, xB, yB, ax, 0, maskA, maskB);
                addOffsetParallelogram(graph, workPoints, xA, yA, xB, yB, -bx, 0, maskA, maskB);
                addOffsetParallelogram(graph, workPoints, xA, yA, xB, yB, 0, ay, maskA, maskB);
                addOffsetParallelogram(graph, workPoints, xA, yA, xB, yB, 0, -ay, maskA, maskB);
                if (s_doRectangles)
                    addRectangle (graph, workPoints, xA - s_vertexFactor * bx, yA - s_vertexFactor * by, xA + s_vertexFactor * ax, yA + s_vertexFactor * ay, maskA, maskB);
                }
            }
        END_VU_FACE_LOOP(nodeA, faceP)
        }

    // the active graph has overlapping areas.
    // get the union of these.  (No need to add connects?)
    vu_mergeOrUnionLoops(graph, VUUNION_UNION); (graph);
    vu_windingFloodFromNegativeAreaFaces (graph, offsetFragmentExteriorMask, VU_EXTERIOR_EDGE);
    static int doSubtract = 2;
    VuMask outputExterior = VU_EXTERIOR_EDGE;
    if (doSubtract == 2)
        {
        vu_toggleMaskInSet(graph, VU_EXTERIOR_EDGE);
        vu_stackPop (graph);
        VuMask compositeExteriorMask = VU_KNOT_EDGE;
        vu_mergeOrUnionLoops(graph, VUUNION_UNION);
        vu_windingFloodFromNegativeAreaFaces(graph, VU_EXTERIOR_EDGE, compositeExteriorMask);
        }
    else if (doSubtract == 1)
        {
        vu_toggleMaskInSet(graph, VU_EXTERIOR_EDGE);
        }

    bvector<bvector<VuP>> loopNodes;
    vu_collectMaskedFaces (graph, outputExterior, false, loopNodes);
    for (auto &loop : loopNodes)
        {
        if (vu_area (loop[0]) > 0.0)
            {
            interiorLoops.push_back (bvector<DPoint3d> ());
            DPoint3d xyz;
            for (auto &node : loop)
                {
                vu_getDPoint3d(&xyz, node);
                interiorLoops.back ().push_back (xyz);
                }
            vu_getDPoint3d(&xyz, loop.front());
            interiorLoops.back ().push_back (xyz);
            }
        }
    vu_returnArray(graph, faceArrayP);
    vu_freeVuSet(graph);
    }
END_BENTLEY_GEOMETRY_NAMESPACE